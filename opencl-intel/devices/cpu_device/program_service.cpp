// Copyright (c) 2006-2008 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  ProgramService.cpp
//  Implementation of the Class ProgramService
//  Class Object is responsible on programs and kernels binaries
//  Intercats with the compiler backend
///////////////////////////////////////////////////////////

#include "stdafx.h"

#include "program_service.h"
#include "program_config.h"
#include "cpu_logger.h"
#include "backend_wrapper.h"

#include <cpu_dev_limits.h>
#include <cl_dynamic_lib.h>
#include <task_executor.h>

#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <math.h>

using namespace Intel::OpenCL::CPUDevice;
using namespace Intel::OpenCL;
using namespace Intel::OpenCL::TaskExecutor;

// Static members
static cl_prog_binary_desc gSupportedBinTypes[] =
{
    {CL_PROG_DLL_X86, 0, 0},
    {CL_PROG_BIN_LLVM, 0, 0},
};
static  unsigned int    UNUSED(gSupportedBinTypesCount) = sizeof(gSupportedBinTypes)/sizeof(cl_prog_binary_desc);

ProgramService::ProgramService(cl_int devId, 
                               IOCLFrameworkCallbacks *devCallbacks, 
                               IOCLDevLogDescriptor *logDesc, 
                               CPUDeviceConfig *config,
                               ICLDevBackendServiceFactory* pBackendFactory) :
    m_iDevId(devId), m_pLogDescriptor(logDesc), m_iLogHandle(0), m_progIdAlloc(1, UINT_MAX),
    m_pCallBacks(devCallbacks), m_pBackendFactory(pBackendFactory), m_pBackendCompiler(NULL),
    m_pBackendExecutor(NULL), m_pBackendImageService(NULL), m_pCPUConfig(config)
{
    assert(m_pBackendFactory && "getting backend factory assumed to allways succeed if initialization has succeeded");

    if ( NULL != logDesc )
    {
        cl_int ret = m_pLogDescriptor->clLogCreateClient(m_iDevId, L"CPU Device: Program Service", &m_iLogHandle);
        if(CL_DEV_SUCCESS != ret)
        {
            m_iLogHandle = 0;
        }
    }
    
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("CPUDevice: Program Service - Created"));
}

ProgramService::~ProgramService()
{
    TProgramMap::iterator   it;

    // Go thought the map and remove all allocated programs
    for(it = m_mapPrograms.begin(); it != m_mapPrograms.end(); ++it)
    {
        DeleteProgramEntry(it->second);
    }

    m_progIdAlloc.Clear();
//  m_kernelIdAlloc.Clear();
    m_mapPrograms.clear();

    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("CPUDevice: Program Service - Distructed"));

    if (0 != m_iLogHandle)
    {
        m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
    }

    if( m_pBackendCompiler )
    {
        m_pBackendCompiler->Release();
    }

    if( m_pBackendExecutor )
    {
        m_pBackendExecutor->Release();
    }

    if( m_pBackendImageService )
    {
        m_pBackendImageService->Release();
    }
}

/****************************************************************************************************************
 Init
    Description
        Initializes the program service. Initialize all the components that could fail - this is an only way to
        return the initialization error information when the exceptions are not used
    Input
        NONE
    Output
        NONE
    Returns
        CL_DEV_SUCCESS          The function is executed successfully.
        CL_DEV_FAILED           The function failed
********************************************************************************************************************/
cl_dev_err_code ProgramService::Init()
{
    ProgramConfig programConfig;
    programConfig.InitFromCpuConfig(*m_pCPUConfig);   
    
    ICLDevBackendCompilationService* pCompiler = NULL;
    cl_dev_err_code ret = m_pBackendFactory->GetCompilationService(&programConfig, &pCompiler);
    if( CL_DEV_FAILED(ret) )
    {
        return ret;
    }

    ICLDevBackendImageService* pImageService = NULL;
    ret = m_pBackendFactory->GetImageService(&programConfig, &pImageService);
    if( CL_DEV_FAILED(ret) )
    {
        pCompiler->Release();
        return ret;
    }

    ICLDevBackendExecutionService* pExecutor = NULL;
    ret = m_pBackendFactory->GetExecutionService(&programConfig, &pExecutor);
    if( CL_DEV_FAILED(ret) )
    {
        //Oh, where is my auto_ptr_ex :-( ?
        pImageService->Release();
        pCompiler->Release();
        return ret;
    }

    m_pBackendCompiler = pCompiler;
    m_pBackendExecutor = pExecutor;
    m_pBackendImageService = pImageService;
    return CL_DEV_SUCCESS;

}



/****************************************************************************************************************
 CheckProgramBinary
    Description
        Performs syntax validation of the intermediate or binary to be built by the device during later stages.
        Call backend compiler to do the check
    Input
        bin_size                Size of the binary buffer
        bin                     A pointer to binary buffer that holds program container defined by cl_prog_container.
    Output
        NONE
    Returns
        CL_DEV_SUCCESS          The function is executed successfully.
        CL_DEV_INVALID_VALUE    If bin_size is 0 or bin is NULL.
        CL_DEV_INVALID_BINARY   If the binary is not supported by the device or program container content is invalid.
********************************************************************************************************************/
cl_dev_err_code ProgramService::CheckProgramBinary (size_t IN binSize, const void* IN bin)
{
    const cl_prog_container_header* pProgCont = (cl_prog_container_header*)bin;

    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("CheckProgramBinary enter"));

    // Check container size
    if ( sizeof(cl_prog_container_header) > binSize )
    {
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Invalid Binary Size was provided"));
        return CL_DEV_INVALID_BINARY;
    }

    // Check container mask
    if ( memcmp(_CL_CONTAINER_MASK_, pProgCont->mask, sizeof(pProgCont->mask)) )
    {
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Invalid Container Mask was provided"));
        return CL_DEV_INVALID_BINARY;
    }

    // Check supported container type
    switch ( pProgCont->container_type )
    {
    // Supported containers
    case CL_PROG_CNT_PRIVATE:
        break;

    default:
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Invalid Container Type was provided"));
        return CL_DEV_INVALID_BINARY;
    }

    // Check supported binary types
    switch ( pProgCont->description.bin_type )
    {
    // Supported program binaries
    case CL_PROG_BIN_LLVM:          // The container should contain valid LLVM-IR
        break;

    case CL_PROG_OBJ_X86:           // The container should contain binary buffer of object file
    case CL_PROG_DLL_X86:           // The container should contain a full path name to DLL file to load
    default:
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Invalid Container Type was provided<%0X>"), pProgCont->description.bin_type);
        return CL_DEV_INVALID_BINARY;
    }

    return CL_DEV_SUCCESS;
}

/*******************************************************************************************************************
clDevCreateProgram
    Description
        Creates a device specific program entity (no build is performed).
    Input
        bin_size                        Size of the binary buffer
        bin                             A pointer to binary buffer that holds program container defined by cl_prog_container.
        prop                            Specifies the origin of the input binary. The values is defined by cl_dev_binary_prop.
    Output
        prog                            A handle to created program object.
    Returns
        CL_DEV_SUCCESS                  The function is executed successfully.
        CL_DEV_INVALID_BINARY           If the back-end compiler failed to process binary.
        CL_DEV_OUT_OF_MEMORY            If the device failed to allocate memory for the program
***********************************************************************************************************************/
cl_dev_err_code ProgramService::CreateProgram( size_t IN binSize,
                                   const void* IN bin,
                                   cl_dev_binary_prop IN prop,
                                   cl_dev_program* OUT prog
                                   )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("CreateProgram enter"));

    // Input parameters validation
    if(0 == binSize || NULL == bin)
    {
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Invalid binSize or bin parameters"));
        return CL_DEV_INVALID_VALUE;
    }

    if ( NULL == prog )
    {
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Invalid prog parameter"));
        return CL_DEV_INVALID_VALUE;
    }

    // If the origin of binary is user loaded
    // check for rightness
    if(prop == CL_DEV_BINARY_USER)
    {
        cl_dev_err_code rc = CheckProgramBinary(binSize, bin);
        if ( CL_DEV_FAILED(rc) )
        {
            CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Check program binary failed"));
            return rc;
        }
    }

    // Create new program
    const cl_prog_container_header* pProgCont = (cl_prog_container_header*)bin;
    TProgramEntry*  pEntry      = new TProgramEntry;
    if ( NULL == pEntry )
    {
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Cann't allocate program entry"));
        return CL_DEV_OUT_OF_MEMORY;
    }
    pEntry->pProgram = NULL;
    pEntry->clBuildStatus = CL_BUILD_NONE;

    cl_dev_err_code ret;
    switch(pProgCont->description.bin_type)
    {
// Deprecated
//  case CL_PROG_DLL_X86:
//      pEntry->pProgram = new DLLProgram;
//      break;
    case CL_PROG_BIN_LLVM:
        assert(m_pBackendCompiler);
        ret = m_pBackendCompiler->CreateProgram(pProgCont, &pEntry->pProgram);
        break;
    default:
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Failed to find approproiate program for type<%d>"), pProgCont->description.bin_type);
        delete pEntry;
        return CL_DEV_INVALID_BINARY;
    }

    if ( CL_DEV_FAILED(ret) )
    {
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Failed to create program from given container<%0X>"), ret);
        delete pEntry;
        return CL_DEV_INVALID_BINARY;
    }

    // Allocate new program id
    unsigned int newProgId;
    if ( !m_progIdAlloc.AllocateHandle(&newProgId) )
    {
        delete pEntry;
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Failed to allocate new handle"));
        return CL_DEV_OUT_OF_MEMORY;
    }

    m_muProgMap.Lock();
    m_mapPrograms[(cl_dev_program)newProgId] = pEntry;
    m_muProgMap.Unlock();

    *prog = (cl_dev_program)newProgId;

    return CL_DEV_SUCCESS;
}


/*******************************************************************************************************************
clDevBuildProgram
    Description
        Builds (compiles & links) a program executable from the program intermediate or binary.
    Input
        bin_size                        Size of the binary buffer
        bin                             A pointer to binary buffer that holds program container defined by cl_prog_container.
        options                         A pointer to a string that describes the build options to be used for building the program executable.
                                        The list of supported options is described in section 5.4.3 in OCL spec. document.
        user_data                       This value will be passed as an argument when clDevBuildFinished is called. Can be NULL.
        prop                            Specifies the origin of the input binary. The values is defined by cl_dev_binary_prop.
    Output
        prog                            A handle to created program object.
    Returns
        CL_DEV_SUCCESS                  The function is executed successfully.
        CL_DEV_INVALID_BUILD_OPTIONS    If build options for back-end compiler specified by options are invalid.
        CL_DEV_INVALID_BINARY           If the back-end compiler failed to process binary.
        CL_DEV_OUT_OF_MEMORY            If the device failed to allocate memory for the program
***********************************************************************************************************************/
// Program Build supporting task
namespace Intel { namespace OpenCL { namespace CPUDevice {
class ProgramBuildTask : public Intel::OpenCL::TaskExecutor::ITask
{
public:
    ProgramBuildTask(ICLDevBackendCompilationService* pCompileService,
                     ProgramService::TProgramEntry* pProgEntry, 
                     const char* pOptions,
                     IOCLFrameworkCallbacks* pCallBack, 
                     cl_dev_program progId, 
                     void* pUserData,
                     IOCLDevLogDescriptor*  pDescriptor, 
                     int iLogHandle):
            m_pCompileService(pCompileService),
            m_pProgEntry(pProgEntry), m_pOptions(pOptions), m_pCallBack(pCallBack),
            m_progId(progId), m_pUserData(pUserData),
            m_pLogDescriptor(pDescriptor), m_iLogHandle(iLogHandle)
    {
        assert(m_pCompileService && "Must be initialized with valid compiler service");     
    }

    // ITask interface
    bool Execute()
    {
        CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Enter"));

        cl_dev_err_code ret = m_pCompileService->BuildProgram(m_pProgEntry->pProgram, NULL);

        CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Build Done (%d)"), ret);

        cl_build_status status = CL_DEV_SUCCEEDED(ret) ? CL_BUILD_SUCCESS : CL_BUILD_ERROR;
        m_pProgEntry->clBuildStatus = status;
        

        // if the user requested -dump-opt-llvm, print this module
        if( CL_DEV_SUCCEEDED(ret) && (NULL != m_pOptions) && !strncmp(m_pOptions, "-dump-opt-llvm=", 15))
        {
            assert( m_pProgEntry->pProgram && "Program must be created already");
            ProgramDumpConfig dumpOptions(m_pOptions);
            m_pCompileService->DumpCodeContainer( m_pProgEntry->pProgram->GetProgramCodeContainer(), &dumpOptions);
        }

		m_pCallBack->clDevBuildStatusUpdate(m_progId, m_pUserData, status);
        CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Exit"));
		return true;
    }

    void        Release() {delete this;}
protected:
    // Program to be built
    ICLDevBackendCompilationService* m_pCompileService;
    ProgramService::TProgramEntry*  m_pProgEntry;
    const char*                     m_pOptions;
    // Data to be passed to the calling party
    IOCLFrameworkCallbacks*         m_pCallBack;
    cl_dev_program                  m_progId;
    void*                           m_pUserData;
    IOCLDevLogDescriptor*           m_pLogDescriptor;
    cl_int                          m_iLogHandle;
};
}}}

cl_dev_err_code ProgramService::BuildProgram( cl_dev_program OUT prog,
                                    const char* IN options,
                                    void* IN userData
                                   )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("BuildProgram enter"));

    TProgramMap::iterator   it;

    m_muProgMap.Lock();
    it = m_mapPrograms.find(prog);
    if( m_mapPrograms.end() == it )
    {
        m_muProgMap.Unlock();
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Requested program not found (%0X)"), (size_t)prog);
        return CL_DEV_INVALID_PROGRAM;
    }

    TProgramEntry* pEntry = it->second;
    m_muProgMap.Unlock();

    // Program already built?
    if (CL_BUILD_SUCCESS == pEntry->clBuildStatus)
    {
        return CL_DEV_BUILD_ALREADY_COMPLETE;
    }

    // Trying two simultaneous builds of the same program?
    if ( CL_BUILD_IN_PROGRESS == pEntry->clBuildStatus )
    {
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Invalid build status(%d), should be CL_BUILD_NONE(%d)"),
            pEntry->clBuildStatus, CL_BUILD_NONE);
        //Asserting because framework should have stopped it
        assert(0);
        return CL_DEV_BUILD_IN_PROGRESS;
    }

    if ( CL_BUILD_NONE != pEntry->clBuildStatus )
    {
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Invalid build status(%d), should be CL_BUILD_NONE(%d)"),
            pEntry->clBuildStatus, CL_BUILD_NONE);
        return CL_DEV_INVALID_OPERATION;
    }

    // Create building thread
    pEntry->clBuildStatus = CL_BUILD_IN_PROGRESS;
    ProgramBuildTask* pBuildTask = new ProgramBuildTask(m_pBackendCompiler,
                                                        pEntry, 
                                                        options, 
                                                        m_pCallBacks, 
                                                        prog, 
                                                        userData,
                                                        m_pLogDescriptor, 
                                                        m_iLogHandle);
    if ( NULL == pBuildTask )
    {
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Failed to create Backend Build task"));
        pEntry->clBuildStatus = CL_BUILD_ERROR;
        return CL_DEV_OUT_OF_MEMORY;
    }

    CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Submitting BuildTask"));
    int iRet = GetTaskExecutor()->Execute(pBuildTask);
    if ( 0 != iRet )
    {
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Submission failed (%0X)"), iRet);
        pEntry->clBuildStatus = CL_BUILD_ERROR;
        return CL_DEV_ERROR_FAIL;
    }

    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Exit"));
    return CL_DEV_SUCCESS;
}

/********************************************************************************************************************
clDevReleaseProgram
    Description
        Deletes previously created program object and releases all related resources.
    Input
        prog                            A handle to program object to be deleted
    Output
        NONE
    Returns
        CL_DEV_SUCCESS                  The function is executed successfully.
        CL_DEV_INVALID_PROGRAM          Invalid program object was specified.
********************************************************************************************************************/
cl_dev_err_code ProgramService::ReleaseProgram( cl_dev_program IN prog )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("ReleaseProgram enter"));

    TProgramMap::iterator   it;

    m_muProgMap.Lock();
    it = m_mapPrograms.find(prog);
    if( m_mapPrograms.end() == it )
    {
        m_muProgMap.Unlock();
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Requested program not found (%0X)"), (size_t)prog);
        return CL_DEV_INVALID_PROGRAM;
    }

    TProgramEntry* pEntry = it->second;
    m_mapPrograms.erase(it);
    m_muProgMap.Unlock();

    DeleteProgramEntry(pEntry);

    m_progIdAlloc.FreeHandle((unsigned int)(size_t)prog);

    return CL_DEV_SUCCESS;
}

/********************************************************************************************************************
clDevUnloadCompiler
    Description
        Allows the framework to release the resources allocated by the back-end compiler.
        This is a hint from the framework and does not guarantee that the compiler will not be used in the future
        or that the compiler will actually be unloaded by the device.
    Input
        NONE
    Output
        NONE
    Returns
        CL_DEV_SUCCESS  The function is executed successfully.
********************************************************************************************************************/
cl_dev_err_code ProgramService::UnloadCompiler()
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("UnloadCompiler enter"));
    return CL_DEV_SUCCESS;
}

/********************************************************************************************************************
clDevGetProgramBinary
    Description
        Returns the compiled program binary. The output buffer contains program container as defined cl_prog_binary_desc.
    Input
        prog                    A handle to created program object.
        size                    Size in bytes of the buffer passed to the function.
    Output
        binary                  A pointer to buffer wherein program binary will be stored.
        size_ret                The actual size in bytes of the returned buffer. When size is equal to 0 and binary is NULL, returns size in bytes of a program binary. If NULL the parameter is ignored.
    Returns
        CL_DEV_SUCCESS          The function is executed successfully.
        CL_DEV_INVALID_PROGRAM  If program is not valid program object.
        CL_DEV_INVALID_VALUE    If size is not enough to store the binary or binary is NULL and size is not 0.
********************************************************************************************************************/
cl_dev_err_code ProgramService::GetProgramBinary( cl_dev_program IN prog,
                                        size_t IN size,
                                        void* OUT binary,
                                        size_t* OUT sizeRet
                                        )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("GetProgramBinary enter"));

    TProgramMap::iterator   it;

    // Access program map
    m_muProgMap.Lock();
    it = m_mapPrograms.find(prog);
    if( it == m_mapPrograms.end())
    {
        m_muProgMap.Unlock();
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Requested program not found (%0X)"), (size_t)prog);
        return CL_DEV_INVALID_PROGRAM;
    }
    ICLDevBackendProgram_ *pProg = it->second->pProgram;
    m_muProgMap.Unlock();

    const ICLDevBackendCodeContainer* pCodeContainer = pProg->GetProgramCodeContainer();
    if ( NULL == pCodeContainer )
    {
        return CL_DEV_INVALID_VALUE;
    }

    size_t stSize = pCodeContainer->GetCodeSize();
    if ( NULL != sizeRet )
    {
        *sizeRet = stSize;
    }

    if ( (0 == size) && (NULL == binary) )
    {
        return CL_DEV_SUCCESS;
    }

    if ( (NULL == binary) || (size < stSize) )
    {
        return CL_DEV_INVALID_VALUE;
    }

    memcpy( (char*)binary, pCodeContainer->GetCode(), stSize);
    return CL_DEV_SUCCESS;
}

cl_dev_err_code ProgramService::GetBuildLog( cl_dev_program IN prog,
                                  size_t IN size,
                                  char* OUT log,
                                  size_t* OUT sizeRet
                                  )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("GetBuildLog enter"));

    TProgramMap::iterator   it;

    // Access program map
    m_muProgMap.Lock();
    it = m_mapPrograms.find(prog);
    if( it == m_mapPrograms.end())
    {
        m_muProgMap.Unlock();
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Requested program not found (%0X)"), (size_t)prog);
        return CL_DEV_INVALID_PROGRAM;
    }
    ICLDevBackendProgram_ *pProg = it->second->pProgram;
    m_muProgMap.Unlock();

    const char* pLog = pProg->GetBuildLog();

    size_t  stLogSize = strlen(pLog) + 1;

    if ( (0 == size) && (NULL == log) )
    {
        if ( NULL == sizeRet )
        {
            return CL_DEV_INVALID_VALUE;
        }
        *sizeRet = stLogSize;
        return CL_DEV_SUCCESS;
    }

    if ( (NULL == log) || (size < stLogSize) )
    {
        return CL_DEV_INVALID_VALUE;
    }

    MEMCPY_S( log, size, pLog, stLogSize);    
    
    if ( NULL != sizeRet )
    {
        *sizeRet = stLogSize;
    }
    return CL_DEV_SUCCESS;
}
/************************************************************************************************************
    clDevGetSupportedBinaries (optional)
    Description
        Returns the list of supported binaries.
    Input
        count                   Size of the buffer passed to the function in terms of cl_prog_binary_desc.
    Output
        types                   A pointer to buffer wherein binary types will be stored.
        count_ret               The actual size ofthe buffer returned by the function in terms of cl_prog_binary_desc.
                                When count is equal to 0 and types is NULL, function returns a size of the list.
                                If NULL the parameter is ignored.
    Returns
        CL_DEV_SUCCESS          The function is executed successfully.
        CL_DEV_INVALID_PROGRAM  If program is not valid program object.
        CL_DEV_INVALID_VALUE    If count is not enough to store the binary or types is NULL and count is not 0.
***************************************************************************************************************/
cl_dev_err_code ProgramService::GetSupportedBinaries( size_t IN size,
                                       cl_prog_binary_desc* OUT types,
                                       size_t* OUT sizeRet
                                       )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("GetSupportedBinaries enter"));
    if ( NULL != sizeRet )
    {
        // TODO: Create supported list
        *sizeRet = sizeof(gSupportedBinTypes);
    }

    if ( (0 == size) && (NULL == types) )
    {
        return CL_DEV_SUCCESS;
    }

    if( (NULL == types) || (size < sizeof(gSupportedBinTypes)))
    {
        return CL_DEV_INVALID_VALUE;
    }

    //currently support only user binaries
    memcpy(types, gSupportedBinTypes, sizeof(gSupportedBinTypes));

    return CL_DEV_SUCCESS;
}


cl_dev_err_code ProgramService::GetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("GetKernelId enter"));

    if ( (NULL == name) || (NULL == kernelId) )
    {
        return CL_DEV_INVALID_VALUE;
    }

    TProgramMap::const_iterator it;

    // Access program map
    m_muProgMap.Lock();
    it = m_mapPrograms.find(prog);
    if( it == m_mapPrograms.end())
    {
        m_muProgMap.Unlock();
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Requested program not found (%0X)"), (size_t)prog);
        return CL_DEV_INVALID_PROGRAM;
    }

    TProgramEntry *pEntry = it->second;
    m_muProgMap.Unlock();

    if ( pEntry->clBuildStatus != CL_BUILD_SUCCESS )
    {
        return CL_DEV_INVALID_PROGRAM;
    }

    // Find if ID is already allocated
    TName2IdMap::const_iterator nameIt;
    pEntry->muMap.Lock();
    nameIt = pEntry->mapKernels.find(name);
    if ( pEntry->mapKernels.end() != nameIt )
    {
        *kernelId = nameIt->second;
        pEntry->muMap.Unlock();
        return CL_DEV_SUCCESS;
    }


    // Retrieve kernel from program
    cl_dev_err_code iRet;
    const ICLDevBackendKernel_* pKernel;

    iRet = pEntry->pProgram->GetKernelByName(name, &pKernel);
    if ( CL_DEV_FAILED(iRet) )
    {
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Requested kernel not found<%0X>"), iRet);
        pEntry->muMap.Unlock();
        return iRet;
    }

    // Add to program entry map
    pEntry->mapKernels[name] = (cl_dev_kernel)pKernel;
    pEntry->muMap.Unlock();

    *kernelId = (cl_dev_kernel)pKernel;

    return CL_DEV_SUCCESS;
}

cl_dev_err_code ProgramService::GetProgramKernels( cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels,
                         cl_uint* OUT numKernelsRet )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("GetProgramKernels enter"));

    TProgramMap::const_iterator it;
    // Access program map
    m_muProgMap.Lock();
    it = m_mapPrograms.find(prog);
    if( it == m_mapPrograms.end())
    {
        m_muProgMap.Unlock();
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Requested program not found (%0X)"), (size_t)prog);
        return CL_DEV_INVALID_PROGRAM;
    }

    TProgramEntry *pEntry = it->second;
    m_muProgMap.Unlock();

    if ( pEntry->clBuildStatus != CL_BUILD_SUCCESS )
    {
        return CL_DEV_INVALID_PROGRAM;
    }

    unsigned int    uiNumProgKernels = pEntry->pProgram->GetKernelsCount();
    cl_dev_err_code         iRet;

    // Check input parameters
    if ( (0==num_kernels) && (NULL==kernels) )
    {
        if ( NULL == numKernelsRet )
        {
            return CL_DEV_INVALID_VALUE;
        }

        *numKernelsRet = uiNumProgKernels;
        return CL_DEV_SUCCESS;
    }

    if ( (NULL==kernels) || (num_kernels < uiNumProgKernels) )
    {
        return CL_DEV_INVALID_VALUE;
    }

    // Allocate buffer to store all kernels from program
    const ICLDevBackendKernel_* *pKernels;
    pKernels = new const ICLDevBackendKernel_*[uiNumProgKernels];
    if ( NULL == pKernels )
    {
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Can't allocate memory for kernels"));
        return CL_DEV_OUT_OF_MEMORY;
    }

    // Retrieve kernels from program and store internally

    for(unsigned int i=0; i<uiNumProgKernels; ++i)
    {
        iRet = pEntry->pProgram->GetKernel(i, &pKernels[i]);
        if ( CL_DEV_FAILED(iRet) )
        {
            delete []pKernels;
            CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Failed to retrive kernels<%X>"), iRet);
            return iRet;
        }
        // Check if kernel is already retrieved
        const char* szKernelName = pKernels[i]->GetKernelName();
        TName2IdMap::const_iterator nameIt;
        pEntry->muMap.Lock();
        nameIt = pEntry->mapKernels.find(szKernelName);
        pEntry->muMap.Unlock();

        // Kernel already has ID ?
        if ( pEntry->mapKernels.end() == nameIt )
        {
            // Add new ID to program's Name2ID map
            pEntry->muMap.Lock();
            pEntry->mapKernels[szKernelName] = (cl_dev_kernel)pKernels[i];
            pEntry->muMap.Unlock();
        }

        kernels[i] = (cl_dev_kernel)pKernels[i];
    }

    if ( NULL != numKernelsRet )
    {
        *numKernelsRet = uiNumProgKernels;
    }

    delete []pKernels;

    return CL_DEV_SUCCESS;
}

cl_dev_err_code ProgramService::GetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN value_size,
                    void* OUT value, size_t* OUT valueSizeRet ) const
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("GetKernelInfo enter"));

    const ICLDevBackendKernel_* pKernel = (const ICLDevBackendKernel_*)kernel;
    const ICLDevBackendKernelProporties* pKernelProps = pKernel->GetKernelProporties();

    // Set value parameters
    cl_uint stValSize;
    unsigned long long ullValue;
    const void* pValue;
    pValue = &ullValue;



    switch (param)
    {
    case CL_DEV_KERNEL_NAME:
        pValue = pKernel->GetKernelName();
        assert(strlen((const char*)pValue) + 1 <= UINT_MAX);//MAXUINT32);
        stValSize = (cl_uint)strlen((const char*)pValue)+1;
        break;

    case CL_DEV_KERNEL_PROTOTYPE:
        stValSize = pKernel->GetKernelParamsCount();
        pValue = (void*)pKernel->GetKernelParams();
        stValSize *= sizeof(cl_kernel_argument);
        break;

    case CL_DEV_KERNEL_WG_SIZE_REQUIRED:
        pValue = pKernelProps->GetRequiredWorkGroupSize();
        stValSize = sizeof(size_t)*MAX_WORK_DIM;
        break;

    case CL_DEV_KERNEL_MAX_WG_SIZE:
        ullValue = MIN(CPU_MAX_WORK_GROUP_SIZE, (CPU_DEV_MAX_WG_PRIVATE_SIZE / pKernelProps->GetPrivateMemorySize()) );
        ullValue = ((unsigned long long)1) << ((unsigned long long)(logf((float)ullValue)/logf(2.f)));
        stValSize = sizeof(size_t);
        break;

    case CL_DEV_KERNEL_WG_SIZE:
        ullValue = pKernelProps->GetKernelPackCount();
        stValSize = sizeof(size_t);
        break;

    case CL_DEV_KERNEL_IMPLICIT_LOCAL_SIZE:
        ullValue = pKernelProps->GetImplicitLocalMemoryBufferSize();
        stValSize = sizeof(cl_ulong);
        break;

    case CL_DEV_KERNEL_PRIVATE_SIZE:
        ullValue = pKernelProps->GetPrivateMemorySize();
        stValSize = sizeof(cl_ulong);
        break;

    default:
        return CL_DEV_INVALID_VALUE;
    }

    if ( (0 == value_size) && (NULL == value) )
    {
        if ( NULL == valueSizeRet )
        {
            return CL_DEV_INVALID_VALUE;
        }

		*valueSizeRet = stValSize;
        return CL_DEV_SUCCESS;
    }

	// BugFix: CSSD100011902, need first to check the error condition
    if ( (NULL == value) || (value_size < stValSize) )
    {
            return CL_DEV_INVALID_VALUE;
    }

    if ( NULL != pValue )
    {
        memcpy(value, pValue, stValSize);
    }
    else
    {
        memset(value, 0, stValSize);
    }

	if ( NULL != valueSizeRet )
    {
        *valueSizeRet = stValSize;
    }

    return CL_DEV_SUCCESS;
}

cl_dev_err_code ProgramService::GetSupportedImageFormats( cl_mem_flags IN flags, cl_mem_object_type IN imageType,
                cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet)
{
    //image_type describes the image type and must be either CL_MEM_OBJECT_IMAGE2D or
    //CL_MEM_OBJECT_IMAGE3D
    if((imageType != CL_MEM_OBJECT_IMAGE2D) && (imageType != CL_MEM_OBJECT_IMAGE3D))
    {
        return CL_DEV_INVALID_VALUE;
    }

    if(0 == numEntries && NULL != formats)
    {
        return CL_DEV_INVALID_VALUE;
    }

    unsigned int uiNumEntries;

    m_pBackendImageService->GetSupportedImageFormats(&uiNumEntries);

      if(NULL != formats)
    {
        uiNumEntries = min(uiNumEntries, numEntries);
        memcpy(formats, supportedImageFormats, uiNumEntries * sizeof(cl_image_format));
    }
    if(NULL != numEntriesRet)
    {
        *numEntriesRet = uiNumEntries;
    }

    return CL_DEV_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////////////
//  Private methods
void ProgramService::DeleteProgramEntry(TProgramEntry* pEntry)
{
    // Finally release the object
    pEntry->pProgram->Release();
    pEntry->mapKernels.clear();
    delete pEntry;
}

