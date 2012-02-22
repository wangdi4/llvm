/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  COIHelpers.cpp

\*****************************************************************************/

#include "COIHelpers.h"
#include "SATestException.h"
#include "CPUDetect.h"

#include <iostream>
#include <assert.h>
#include <string>
#include <limits.h>

#define DEBUG_TYPE "COIHelpers"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"

using namespace Validation;

#define CHECK_COI_RESULT(_COIFUNC)                                                  \
    {                                                                               \
    COIRESULT result = _COIFUNC;                                                    \
    if (COI_SUCCESS != result)                                                      \
    {                                                                               \
        throw Exception::COIUsageException(std::string(#_COIFUNC" retruned ") +     \
            std::string(COIResultGetName(result)));                                 \
    }                                                                               \
}

#define CHECK_COI_RESULT_NO_THROW(_COIFUNC)                                         \
{                                                                                   \
    COIRESULT result = _COIFUNC;                                                    \
    if (COI_SUCCESS != result)                                                      \
    {                                                                               \
        std::cerr << #_COIFUNC" retruned " << COIResultGetName(result) << "\n";     \
    }                                                                               \
}

#define CHECK_BACKEND_RESULT(_BEFUNC)                                           \
    {                                                                           \
    cl_dev_err_code result = _BEFUNC;                                           \
    if (CL_DEV_FAILED(result))                                                  \
    {                                                                           \
        throw Exception::BackendException(std::string(#_BEFUNC" failed"));      \
    }                                                                           \
}

COI_ISA_TYPE COIProcessAndPipelineWrapper::GetCOIISAType(std::string cpuArch)
{
    // TODO: at the moment SATest is supposed to support KNF architecture only.
    // This function have to be fixed to support other MIC architectures.
    if(std::string("auto-remote") == cpuArch) return COI_ISA_KNF;
    if(std::string("knf") == cpuArch) return COI_ISA_KNF;
    // if(std::string("knc") == cpuArch) return COI_ISA_KNC;
    return COI_ISA_INVALID;
}

void COIProcessAndPipelineWrapper::Create( COIENGINE engine, const BERunOptions *pRunConfig )
{
    // SATEST_NATIVE_NAME contains only file name. In order to be able to load SATEST_NATIVE_NAME from arbitrary directory we make obtain we full path the SATEST_NATIVE_NAME.
    // Assume that SATEST_NATIVE_NAME is located in the same directory as SATest.
    char path[PATH_MAX] = {0};
    char simlink[] = "/proc/self/exe";
    std::string fullName(""), pathName("");
    ssize_t size = readlink( simlink, path, PATH_MAX - 1);
    if (size != -1)
    {
        while(path[size-1] != '/') { --size;}
        path[size] = '\0';
        pathName = std::string(path);
        fullName = pathName;
    }
    // Remove executable name i.e. exctract path to the executable.
    fullName.append(SATEST_NATIVE_NAME);
    // Create a process on the MIC.
    CHECK_COI_RESULT(
        COIProcessCreateFromFile(
        engine,             // The engine to create the process on.
        fullName.c_str(),   // The local path to the sink side binary to launch.
        0, NULL,            // argc and argv for the sink process.
        false, NULL,        // Environment variables to set for the sink process.
        true, NULL,         // Enable the proxy but don't specify a proxy root path.
        0,                  // The amount of memory to reserve for COIBuffers. 0 - dynamic allocation.
        NULL,               // Path to search for dependencies
        &m_process          // The resulting process handle.
        ));

    COIRESULT res;
    // !!! WORKAROUND FOR LIT !!!
    // TODO: found out how to pass environment variables to the lit scripts.
    // LIT doesn't enherit environment variables, so it looses SINK_LD_LIBRARY_PATH,
    // so we will look for svml library near the executable.
    std::string svmlPath =  (SVML_LIBRARY_PATH) ? 
      std::string(SVML_LIBRARY_PATH) :
      //std::string(SVML_LIBRARY_PATH);
      pathName;

    Intel::ECPU cpuId = Intel::OpenCL::DeviceBackend::Utils::CPUDetect::GetInstance()->GetCPUByName(pRunConfig->GetValue<std::string>(RC_BR_CPU_ARCHITECTURE, "").c_str());
    const char* pCPUPrefix = Intel::OpenCL::DeviceBackend::Utils::CPUDetect::GetInstance()->GetCPUPrefix(cpuId);
    std::string svmlFileName = std::string("__ocl_svml_") + pCPUPrefix + ".so";
    // Load SVML built-ins library.
    res = COIProcessLoadLibraryFromFile(
        m_process,              // in_Process
        svmlFileName.c_str(),   // in_FileName
        NULL,                   // in_so-name if not exists in file
        svmlPath.c_str(),       // in_LibrarySearchPath
        &m_library );
    if ((COI_SUCCESS != res) && (COI_ALREADY_EXISTS != res))
    {
        // Destroy successfully created process.
        COIProcessDestroy(
            m_process,      // Process handle to be destroyed.
            -1,             // Wait indefinitely until main() (on the MIC side) returns.
            false,          // Don't force to exit. Let it finish executing functions enqueued and exit gracefully.
            NULL,           // Don't care about the exit result.
            NULL            // Also don't care what the exit reason was.
            );
        throw Exception::COIUsageException(std::string("Unable to load SVML built-ins library to the device: \n")+std::string(COIResultGetName(res)));
        return;
    }

    // Create a pipeline associated with process created earlier.
    res = 
        COIPipelineCreate(
        m_process,          // Process to associate the pipeline with
        NULL,               // Do not set any sink thread affinity for the pipeline
        0,                  // Use the default stack size for the pipeline thread
        &m_pipeline         // Handle to the new pipeline
        );
    if (COI_SUCCESS != res)
    {
        // TODO: at the moment SVML library is empty and ProcessLoadLibraryFromFile returns COI_SUCCESS and 0 library handle, but COI doesn't allow unload library with 0 handle.
        // CHECK IF IT'S LEGAL TO RETURN COI_SUCCESS AND ZERO HANDLE
        if (m_library)
        {
            // Unload SVML built-ins library
            CHECK_COI_RESULT_NO_THROW(COIProcessUnloadLibrary(m_process, m_library));
        }

        // Destroy successfully created process.
        COIProcessDestroy(
            m_process,      // Process handle to be destroyed.
            -1,             // Wait indefinitely until main() (on the MIC side) returns.
            false,          // Don't force to exit. Let it finish executing functions enqueued and exit gracefully.
            NULL,           // Don't care about the exit result.
            NULL            // Also don't care what the exit reason was.
            );
        throw Exception::COIUsageException(std::string("Unable to create COI pipeline\n")+std::string(COIResultGetName(res)));
        return;
    }

    m_created = true;
}

COIProcessAndPipelineWrapper::~COIProcessAndPipelineWrapper()
{
    DEBUG(llvm::dbgs()<< "Calling COIProcessAndPipelineWrapper desctructor... ");
    if (m_created)
    {
        // TODO: at the moment SVML library is empty and ProcessLoadLibraryFromFile returns COI_SUCCESS and 0 library handle, but COI doesn't allow unload library with 0 handle.
        // CHECK IF IT'S LEGAL TO RETURN COI_SUCCESS AND ZERO HANDLE
        if (m_library)
        {
            // Unload SVML built-ins library
            CHECK_COI_RESULT_NO_THROW(COIProcessUnloadLibrary(m_process, m_library));
        }

        // Destroy the pipeline
        CHECK_COI_RESULT_NO_THROW(COIPipelineDestroy(m_pipeline));

        // Destroy the process
        CHECK_COI_RESULT_NO_THROW(
            COIProcessDestroy(
            m_process,      // Process handle to be destroyed.
            -1,             // Wait indefinitely until main() (on the MIC side) returns.
            false,          // Don't force to exit. Let it finish executing functions enqueued and exit gracefully.
            NULL,           // Don't care about the exit result.
            NULL            // Also don't care what the exit reason was.
            ));
    }
    DEBUG(llvm::dbgs()<< "done.\n");
}

void COIBuffersWrapper::AddBuffer( size_t bufferSize, const COIPROCESS process, COI_ACCESS_FLAGS access, uint32_t flags, void* pInitData )
{
    DEBUG(llvm::dbgs()<< "Calling COIBuffersWrapper::AddBuffer.\n");
    DEBUG(llvm::dbgs()<< "Current number of buffers: " << m_buffers.size() << "\n");
    DEBUG(llvm::dbgs()<< "Buffer size: " << bufferSize << "\n");
    DEBUG(llvm::dbgs()<< "COI buffer flags: " << flags << "\n");
    COIBUFFER buff;
    CHECK_COI_RESULT(
        COIBufferCreate(
        bufferSize,             // Size of the buffer
        COI_BUFFER_NORMAL,      // Type of the buffer
        flags,
        pInitData,              // Pointer to the Initialization data
        1, &process,            // Processes that will use the buffer
        &buff                   // Buffer handle that was created
        ));
    m_buffers.push_back(buff);
    m_flags.push_back(access);
}

COIBuffersWrapper::~COIBuffersWrapper( void )
{
    DEBUG(llvm::dbgs()<< "Calling COIBuffersWrapper desctructor... ");
    for (size_t i = 0; i < m_buffers.size(); ++i)
    {
        CHECK_COI_RESULT_NO_THROW(COIBufferDestroy(m_buffers[i]));
    }
    DEBUG(llvm::dbgs()<< "done.\n");
}

// TODO: Add multiple buffer mapping support.
void COIBuffersWrapper::Map( COI_MAP_TYPE mapType, int numOfDepends, COIEVENT* dependencies, void** data, size_t id )
{
    DEBUG(llvm::dbgs()<< "Calling COIBuffersWrapper::Map... ");
    CHECK_COI_RESULT(
        COIBufferMap(
        m_buffers[id],               // Buffer handle to map
        0, 0,                        // Starting offset and number of bytes to map
        mapType,                     // Map type
        numOfDepends, dependencies,  // Input dependencies
        NULL,                        // No completion barrier indicates synchronous map
        &m_map,                      // Map instance handle
        data                         // Pointer to access the buffer data.
        ));
    DEBUG(llvm::dbgs()<< "done.\n");
}


void COIBuffersWrapper::UnMap()
{
    DEBUG(llvm::dbgs()<< "Calling COIBuffersWrapper::UnMap... ");
    // Unmap the buffer mapping Instance 'mi'
    CHECK_COI_RESULT(COIBufferUnmap(m_map, 0, NULL, NULL));
    DEBUG(llvm::dbgs()<< "done.\n");
}

void COIBuffersWrapper::CreateBufferFromMemory( size_t bufferSize, const COIPROCESS process, COI_ACCESS_FLAGS access, void* pData )
{
    DEBUG(llvm::dbgs()<< "Calling COIBuffersWrapper::CreateBufferFromMemory.\n");
    DEBUG(llvm::dbgs()<< "Current number of buffers: " << m_buffers.size() << "\n");
    DEBUG(llvm::dbgs()<< "Buffer size: " << bufferSize << "\n");
    DEBUG(llvm::dbgs()<< "COI buffer access flags: " << access << "\n");
    COIBUFFER buff;
    CHECK_COI_RESULT(
        COIBufferCreateFromMemory(
        bufferSize,                 // Size of the buffer
        COI_BUFFER_NORMAL,          // Type of the buffer
        COI_OPTIMIZE_HUGE_PAGE_SIZE,
        pData,                      // Pointer to the Initialization data
        1, &process,                // Processes that will use the buffer
        &buff                       // Buffer handle that was created
        ));
    m_buffers.push_back(buff);
    m_flags.push_back(access);
}
