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

#include "program_service.h"
#include "mic_logger.h"

#include <cl_dev_backend_api.h>
#include <cl_dynamic_lib.h>
#include <task_executor.h>

#include "mic_dev_limits.h"
#include "mic_common_macros.h"
#include "device_service_communication.h"
#include "mic_device_interface.h"

#include <source/COIBuffer_source.h>
#include <source/COIPipeline_source.h>

#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <math.h>
#include <alloca.h>

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL;
using namespace Intel::OpenCL::TaskExecutor;
using namespace Intel::OpenCL::DeviceBackend;

// Static members
static cl_prog_binary_desc gSupportedBinTypes[] =
{
    {CL_PROG_DLL_X86, 0, 0},
    {CL_PROG_BIN_LLVM, 0, 0},
};
static    unsigned int    UNUSED(gSupportedBinTypesCount) = ARRAY_ELEMENTS(gSupportedBinTypes);

ProgramService::ProgramService(cl_int devId, IOCLFrameworkCallbacks *devCallbacks,
                                              IOCLDevLogDescriptor *logDesc,
                                              MICDeviceConfig *config,
                                              DeviceServiceCommunication& dev_service) :
    m_DevService( dev_service), m_BE_Compiler( m_DevService, MIC_BACKEND_DLL_NAME ),
    m_iDevId(devId), m_pLogDescriptor(logDesc), m_iLogHandle(0), m_progIdAlloc(1, UINT_MAX),
    m_pCallBacks(devCallbacks), m_pMICConfig(config)
{
    if ( NULL != logDesc )
    {
        cl_int ret = m_pLogDescriptor->clLogCreateClient(m_iDevId, L"MIC Device: Program Service", &m_iLogHandle);
        if(CL_DEV_SUCCESS != ret)
        {
            m_iLogHandle = 0;
        }
    }

    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("MICDevice: Program Service - Created"));
}

ProgramService::~ProgramService()
{
    TProgramMap::iterator    it;

    // Go thought the map and remove all allocated programs
    for(it = m_mapPrograms.begin(); it != m_mapPrograms.end(); ++it)
    {
        DeleteProgramEntry(it->second);
    }

    m_progIdAlloc.Clear();
    m_mapPrograms.clear();

    ReleaseBackendServices();

    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("MICDevice: Program Service - Distructed"));

    if (0 != m_iLogHandle)
    {
        m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
    }
}

/****************************************************************************************************************
 BEGIN MICBackendOptions
********************************************************************************************************************/
MICBackendOptions::MICBackendOptions( DeviceServiceCommunication& dev_service ) :
             m_dev_service( dev_service), m_bUseVectorizer(false), m_bUseVtune(false)
{
}

void MICBackendOptions::init( bool bUseVectorizer, bool bUseVtune )
{
    m_bUseVectorizer = bUseVectorizer;
    m_bUseVtune      = bUseVtune;
}

// ICLDevBackendOptions interface
// BUGBUG: DK remove this!
#define CL_DEV_BACKEND_OPTION_TARGET_DESCRIPTION 1000
#define CL_DEV_BACKEND_OPTION_TARGET_DESCRIPTION_SIZE 1000

bool MICBackendOptions::GetBooleanValue( int optionId, bool defaultValue) const
{
    return (CL_DEV_BACKEND_OPTION_USE_VTUNE == optionId) ? m_bUseVtune : defaultValue;
}

int MICBackendOptions::GetIntValue( int optionId, int defaultValue) const
{
    switch (optionId)
    {
        case CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE:
// BUGBUG: DK: Disable vectorizer
//            return !m_bUseVectorizer ? 1 : defaultValue;
            return TRANSPOSE_SIZE_1;

        case CL_DEV_BACKEND_OPTION_TARGET_DESCRIPTION_SIZE:
            return getTargetDescriptionSize();

        default:
            return defaultValue;
    }
}

const char* MICBackendOptions::GetStringValue( int optionId, const char* defaultValue)const
{
    switch (optionId)
    {
        case CL_DEV_BACKEND_OPTION_CPU_ARCH:
            return "auto-remote";

        default:
            return defaultValue;
    }
}

bool MICBackendOptions::GetValue( int optionId, void* Value, size_t* pSize) const
{
    if ((CL_DEV_BACKEND_OPTION_TARGET_DESCRIPTION == optionId) && Value && pSize && (*pSize > 0) )
    {
        return getTargetDescription( Value, pSize );
    }

    return false;
}

// Targhet Description Size should be extracted from Device
int MICBackendOptions::getTargetDescriptionSize( void ) const
{
    bool                ok;
    uint64_t            size = 0;

    // parameters
    //   pass nothing
    //   get  Value filled  size in the output_data - uint64_t. If filled size in 0 - error
    ok = m_dev_service.runServiceFunction(
                                DeviceServiceCommunication::GET_BACKEND_TARGET_DESCRIPTION_SIZE,
                                0, NULL,                             // input_data
                                sizeof(uint64_t), &size,             // ouput_data
                                0, NULL, NULL);                      // buffers passed

    assert( ok && "Cannot run Device Function to get BE options size" );
    assert( size > 0 && "GET_BACKEND_OPTIONS_SIZE in device returned 0 length!" );

    return ok ? (int)size : 0;
}

// Targhet Description should be extracted from Device
bool MICBackendOptions::getTargetDescription( void* Value, size_t* pSize) const
{
    COIRESULT           coi_err;
    COIBUFFER           coi_buffer;
    COI_ACCESS_FLAGS    coi_access = COI_SINK_WRITE_ENTIRE;
    COIMAPINSTANCE      map_instance;
    void*               data_address;

    bool                ok;
    uint64_t            filled_size = 0;
    uint64_t            original_size = *pSize;
    COIPROCESS          in_pProcesses[] = { m_dev_service.getDeviceProcessHandle() };

    //
    // Target description is going to filled into Value buffer directly and copied back using DMA
    //
    coi_err = COIBufferCreateFromMemory( original_size,                // size of buffer to be filled
                                         COI_BUFFER_NORMAL, COI_OPTIMIZE_SOURCE_READ|COI_OPTIMIZE_SINK_WRITE,
                                         Value,
                                         ARRAY_ELEMENTS(in_pProcesses), in_pProcesses,
                                         &coi_buffer );

    if (COI_SUCCESS != coi_err)
    {
        assert( false && "COIBufferCreateFromMemory failed to create buffer from memory for BE options" );
        return false;
    }

    // parameters
    //   pass Value initial size in the input_data - uint64_t
    //   get  Value filled  size in the output_data - uint64_t. If filled size in 0 - error
    //   get  Value data in the coi_buffer
    ok = m_dev_service.runServiceFunction(
                                DeviceServiceCommunication::GET_BACKEND_TARGET_DESCRIPTION,
                                0, NULL,                             // input_data
                                sizeof(uint64_t), &filled_size,      // ouput_data
                                1, &coi_buffer, &coi_access);        // buffers passed

    assert( ok && "Cannot run Device Function to get BE options" );
    assert( filled_size > 0 && "GET_BACKEND_OPTIONS in device filled 0 length!" );
    assert( filled_size <= original_size && "GET_BACKEND_TARGET_DESCRIPTION in device filled more than buffer size!" );

    if (0 != filled_size)
    {
        // map buffer to get results
        coi_err = COIBufferMap(coi_buffer, 0, *pSize, COI_MAP_READ_ONLY, 0, NULL, NULL, &map_instance, &data_address );
        assert( (COI_SUCCESS == coi_err) && "COIBufferMap failed to map buffer with BE options" );
        assert( ((size_t)Value == (size_t)data_address) && "COIBufferMap did not return original data pointer for buffer from memory" );

        // unmap buffer back - data should remain in the original memory
        coi_err = COIBufferUnmap( map_instance, 0, NULL, NULL );
        assert( (COI_SUCCESS == coi_err) && "COIBufferUnmap failed to unmap buffer with BE options" );
    }

    // remove COI Buffer structure - data should remain in the original buffer
    coi_err = COIBufferDestroy( coi_buffer );
    assert( (COI_SUCCESS == coi_err) && "COIBufferDestroy failed to destroy buffer from memory for BE options" );

    *pSize = filled_size;
    return (ok && filled_size);
}

/****************************************************************************************************************
 END MICBackendOptions
********************************************************************************************************************/

inline
ICLDevBackendCompilationService* ProgramService::GetCompilationService(void)
{
    if (NULL == m_BE_Compiler.pCompilationService)
    {
        LoadBackendServices();
    }

    return m_BE_Compiler.pCompilationService;
}

inline
ICLDevBackendSerializationService* ProgramService::GetSerializationService(void)
{
    if (NULL == m_BE_Compiler.pSerializationService)
    {
        LoadBackendServices();
    }

    return m_BE_Compiler.pSerializationService;
}

bool ProgramService::LoadBackendServices(void)
{
    cl_dev_err_code err;
    bool            ok = false;

    // local variables invisible to other threads
    ICLDevBackendCompilationService*    comp_temp = NULL;
    ICLDevBackendSerializationService*  ser_temp = NULL;


    if (NULL != m_BE_Compiler.pCompilationService)
    {
        return true;
    }

    do
    {
        // acquire creation lock
        OclAutoMutex lock(&m_BE_Compiler.creationLock);

        // check once more
        if (NULL != m_BE_Compiler.pCompilationService)
        {
            ok = true;
            break;
        }

// BUGBUG: DK test only
//        m_BE_Compiler.MICOptions.init( m_pMICConfig->UseVectorizer(),
//                                       m_pMICConfig->UseVTune()
//                                      );
//
//        unsigned char bbb[100];
//        size_t   bbb_size = 100;
//
//        int d_size = m_BE_Compiler.MICOptions.GetIntValue( CL_DEV_BACKEND_OPTION_TARGET_DESCRIPTION_SIZE, 0 );
//        printf("d_size=%d\n", d_size);
//
//        bool ee = m_BE_Compiler.MICOptions.GetValue( CL_DEV_BACKEND_OPTION_TARGET_DESCRIPTION, bbb, &bbb_size);
//        bbb[bbb_size] = '\0';
//
//        printf("ee=%s bbb=|%s| bbb_size=%u\n", ee ? "TRUE" : "FALSE", (char*)bbb, (unsigned)bbb_size);

        err = m_BE_Compiler.be_wrapper.Init();

        assert( CL_DEV_SUCCEEDED(err) && "MIC Device Agent cannot initialize BE" );

        // load Backend Compiler
        ICLDevBackendServiceFactory* be_factory = m_BE_Compiler.be_wrapper.GetBackendFactory();

        if (NULL == be_factory)
        {
            assert( false && "MIC Device Agent cannot create BE Factory" );
            MicCriticLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("MIC Device Agent cannot create BE Factory"));
            break;
        }

        // get compiler options
        m_BE_Compiler.MICOptions.init( m_pMICConfig->UseVectorizer(),
                                       m_pMICConfig->UseVTune()
                                      );

        err = be_factory->GetCompilationService( &m_BE_Compiler.MICOptions,
                                                &comp_temp );

        if (CL_DEV_FAILED( err ))
        {
            assert( false && "MIC Device Agent cannot create Backend Compilation Service" );
            MicCriticLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("MIC Device Agent cannot create Backend Compilation Service"));
            break;
        }

        err = be_factory->GetSerializationService( &m_BE_Compiler.MICOptions,
                                                  &ser_temp );

        if (CL_DEV_FAILED( err ))
        {
            assert( false && "MIC Device Agent cannot create Backend Serialization Service" );
            MicCriticLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("MIC Device Agent cannot create Backend Serialization Service"));
            break;
        }

        // set field visible to other threads
        m_BE_Compiler.pCompilationService   = comp_temp;
        m_BE_Compiler.pSerializationService = ser_temp;

        ok = true;

        // lock is released automatically here
    } while (0);

    if (!ok)
    {
        if (NULL != comp_temp)
        {
            comp_temp->Release();
        }

        if (NULL != ser_temp)
        {
            ser_temp->Release();
        }

        ReleaseBackendServices();
    }

    return ok;
}

void ProgramService::ReleaseBackendServices(void)
{
    // acquire creation lock
    OclAutoMutex lock(&m_BE_Compiler.creationLock);

    if (m_BE_Compiler.pSerializationService)
    {
        m_BE_Compiler.pSerializationService->Release();
        m_BE_Compiler.pSerializationService = NULL;
    }

    if (m_BE_Compiler.pCompilationService)
    {
        m_BE_Compiler.pCompilationService->Release();
        m_BE_Compiler.pCompilationService = NULL;
    }

    m_BE_Compiler.be_wrapper.Terminate();

}

// inline utils
inline
cl_dev_kernel ProgramService::kernel_entry_2_cl_dev_kernel( const TKernelEntry* e ) const
{
    return (cl_dev_kernel)e;
}

inline
ProgramService::TKernelEntry* ProgramService::cl_dev_kernel_2_kernel_entry( cl_dev_kernel k ) const
{
    TKernelEntry* e = (TKernelEntry*)k;

    assert( TKernelEntry::marker_value == e->marker && "MICDevice: Wrong cl_dev_kernel passed!" );

    if (TKernelEntry::marker_value != e->marker)
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("MICDevice: Wrong cl_dev_kernel passed: 0x%lx"), k );
        e = NULL;
    }

    return e;
}

/****************************************************************************************************************
 CheckProgramBinary
    Description
        Performs syntax validation of the intermediate or binary to be built by the device during later stages.
        Call backend compiler to do the check
    Input
        bin_size                Size of the binary buffer
        bin                        A pointer to binary buffer that holds program container defined by cl_prog_container.
    Output
        NONE
    Returns
        CL_DEV_SUCCESS            The function is executed successfully.
        CL_DEV_INVALID_VALUE    If bin_size is 0 or bin is NULL.
        CL_DEV_INVALID_BINARY    If the binary is not supported by the device or program container content is invalid.
********************************************************************************************************************/
cl_dev_err_code ProgramService::CheckProgramBinary (size_t IN binSize, const void* IN bin)
{
    const cl_prog_container_header*    pProgCont = (cl_prog_container_header*)bin;

    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("CheckProgramBinary enter"));

    // Check container size
    if ( sizeof(cl_prog_container_header) > binSize )
    {
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Invalid Binary Size was provided"));
        return CL_DEV_INVALID_BINARY;
    }

    // Check container mask
    if ( memcmp(_CL_CONTAINER_MASK_, pProgCont->mask, sizeof(pProgCont->mask)) )
    {
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Invalid Container Mask was provided"));
        return CL_DEV_INVALID_BINARY;
    }

    // Check supported container type
    switch ( pProgCont->container_type )
    {
    // Supported containers
    case CL_PROG_CNT_PRIVATE:
        break;

    default:
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Invalid Container Type was provided"));
        return CL_DEV_INVALID_BINARY;
    }

    // Check supported binary types
    switch ( pProgCont->description.bin_type )
    {
    // Supported program binaries
    case CL_PROG_BIN_LLVM:            // The container should contain valid LLVM-IR
        break;

    case CL_PROG_OBJ_X86:            // The container should contain binary buffer of object file
    case CL_PROG_DLL_X86:            // The container should contain a full path name to DLL file to load
    default:
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Invalid Container Type was provided<%0X>"), pProgCont->description.bin_type);
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
        bin                                A pointer to binary buffer that holds program container defined by cl_prog_container.
        prop                            Specifies the origin of the input binary. The values is defined by cl_dev_binary_prop.
    Output
        prog                            A handle to created program object.
    Returns
        CL_DEV_SUCCESS                    The function is executed successfully.
        CL_DEV_INVALID_BINARY            If the back-end compiler failed to process binary.
        CL_DEV_OUT_OF_MEMORY            If the device failed to allocate memory for the program
***********************************************************************************************************************/
cl_dev_err_code ProgramService::CreateProgram( size_t IN binSize,
                                   const void* IN bin,
                                   cl_dev_binary_prop IN prop,
                                   cl_dev_program* OUT prog
                                   )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("CreateProgram enter"));

    // Input parameters validation
    if(0 == binSize || NULL == bin)
    {
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Invalid binSize or bin parameters"));
        return CL_DEV_INVALID_VALUE;
    }

    if ( NULL == prog )
    {
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Invalid prog parameter"));
        return CL_DEV_INVALID_VALUE;
    }

    // If the origin of binary is user loaded
    // check for rightness
    if(prop == CL_DEV_BINARY_USER)
    {
        cl_dev_err_code rc = CheckProgramBinary(binSize, bin);
        if ( CL_DEV_FAILED(rc) )
        {
            MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Check program binary failed"));
            return rc;
        }
    }

    // Create new program
    const cl_prog_container_header*    pProgCont = (cl_prog_container_header*)bin;
    TProgramEntry*    pEntry        = new TProgramEntry;
    if ( NULL == pEntry )
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Cann't allocate program entry"));
        return CL_DEV_OUT_OF_MEMORY;
    }
    pEntry->pProgram = NULL;
    pEntry->uid_program_on_device = (uint64_t)pEntry;
    pEntry->clBuildStatus = CL_BUILD_NONE;

    cl_dev_err_code ret;
    switch(pProgCont->description.bin_type)
    {
    case CL_PROG_BIN_LLVM:
        ret = GetCompilationService()->CreateProgram(pProgCont, (ICLDevBackendProgram_**)&pEntry->pProgram);
        break;
    default:
        MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Failed to find approproiate program for type<%d>"), pProgCont->description.bin_type);
        delete pEntry;
        return CL_DEV_INVALID_BINARY;
    }

    if ( CL_DEV_FAILED(ret) )
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Failed to create program from given container<%0X>"), ret);
        delete pEntry;
        return CL_DEV_INVALID_BINARY;
    }

    // Allocate new program id
    unsigned int newProgId;
    if ( !m_progIdAlloc.AllocateHandle(&newProgId) )
    {
        delete pEntry;
        MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Failed to allocate new handle"));
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
        bin                                A pointer to binary buffer that holds program container defined by cl_prog_container.
        options                            A pointer to a string that describes the build options to be used for building the program executable.
                                        The list of supported options is described in section 5.4.3 in OCL spec. document.
        user_data                        This value will be passed as an argument when clDevBuildFinished is called. Can be NULL.
        prop                            Specifies the origin of the input binary. The values is defined by cl_dev_binary_prop.
    Output
        prog                            A handle to created program object.
    Returns
        CL_DEV_SUCCESS                    The function is executed successfully.
        CL_DEV_INVALID_BUILD_OPTIONS    If build options for back-end compiler specified by options are invalid.
        CL_DEV_INVALID_BINARY            If the back-end compiler failed to process binary.
        CL_DEV_OUT_OF_MEMORY            If the device failed to allocate memory for the program
***********************************************************************************************************************/
// Program Build supporting task
namespace Intel { namespace OpenCL { namespace MICDevice {
class ProgramBuildTask : public Intel::OpenCL::TaskExecutor::ITask
{
public:
    ProgramBuildTask(ProgramService::TProgramEntry* pProgEntry, const char* pOptions,
        cl_dev_program progId, void* pUserData, ProgramService& service) :
            m_pProgEntry(pProgEntry), m_pOptions(pOptions),
            m_progId(progId), m_pUserData(pUserData), m_ProgramService(service)
    {
    }

    // ITask interface
    void Execute()
    {
        MicDbgLog(m_ProgramService.m_pLogDescriptor, m_ProgramService.m_iLogHandle, TEXT("%S"), TEXT("Enter"));

        cl_dev_err_code ret = m_ProgramService.GetCompilationService()->BuildProgram(m_pProgEntry->pProgram, NULL);

        MicDbgLog(m_ProgramService.m_pLogDescriptor, m_ProgramService.m_iLogHandle, TEXT("Build Done (%d)"), ret);

        cl_build_status status = CL_BUILD_ERROR;

        if (CL_DEV_SUCCEEDED(ret))
        {
            m_pProgEntry->copy_to_device_ok = m_ProgramService.BuildKernelData( m_pProgEntry );
            if (m_pProgEntry->copy_to_device_ok)
            {
                status = CL_BUILD_SUCCESS;
            }
        }

        m_pProgEntry->clBuildStatus = status;
        m_ProgramService.m_pCallBacks->clDevBuildStatusUpdate(m_progId, m_pUserData, status);
        MicDbgLog(m_ProgramService.m_pLogDescriptor, m_ProgramService.m_iLogHandle, TEXT("%S"), TEXT("Exit"));
    }

    void        Release() {delete this;}
protected:
    // Program to be built
    ProgramService::TProgramEntry*      m_pProgEntry;
    const char*                         m_pOptions;

    // Data to be passed to the calling party
    cl_dev_program                      m_progId;
    void*                               m_pUserData;

    ProgramService&                     m_ProgramService;
};
}}}

cl_dev_err_code ProgramService::BuildProgram( cl_dev_program OUT prog,
                                    const char* IN options,
                                    void* IN userData
                                   )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("BuildProgram enter"));

    TProgramMap::iterator    it;

    m_muProgMap.Lock();
    it = m_mapPrograms.find(prog);
    if( m_mapPrograms.end() == it )
    {
        m_muProgMap.Unlock();
        MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Requested program not found (%0X)"), (size_t)prog);
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
        MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Invalid build status(%d), should be CL_BUILD_NONE(%d)"),
            pEntry->clBuildStatus, CL_BUILD_NONE);
        //Asserting because framework should have stopped it
        assert(0);
        return CL_DEV_BUILD_IN_PROGRESS;
    }

    if ( CL_BUILD_NONE != pEntry->clBuildStatus )
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Invalid build status(%d), should be CL_BUILD_NONE(%d)"),
            pEntry->clBuildStatus, CL_BUILD_NONE);
        return CL_DEV_INVALID_OPERATION;
    }

    // Create building thread
    pEntry->clBuildStatus = CL_BUILD_IN_PROGRESS;
    ProgramBuildTask* pBuildTask = new ProgramBuildTask(pEntry, options, prog, userData, *this );

    if ( NULL == pBuildTask )
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Failed to create Backend Build task"));
        pEntry->clBuildStatus = CL_BUILD_ERROR;
        return CL_DEV_OUT_OF_MEMORY;
    }

    MicDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Submitting BuildTask"));
    int iRet = GetTaskExecutor()->Execute(pBuildTask);
    if ( 0 != iRet )
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Submission failed (%0X)"), iRet);
        pEntry->clBuildStatus = CL_BUILD_ERROR;
        return CL_DEV_ERROR_FAIL;
    }

    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Exit"));
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
        CL_DEV_SUCCESS                    The function is executed successfully.
        CL_DEV_INVALID_PROGRAM            Invalid program object was specified.
********************************************************************************************************************/
cl_dev_err_code ProgramService::ReleaseProgram( cl_dev_program IN prog )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("ReleaseProgram enter"));

    TProgramMap::iterator    it;

    m_muProgMap.Lock();
    it = m_mapPrograms.find(prog);
    if( m_mapPrograms.end() == it )
    {
        m_muProgMap.Unlock();
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Requested program not found (%0X)"), (size_t)prog);
        return CL_DEV_INVALID_PROGRAM;
    }

    TProgramEntry* pEntry = it->second;

    assert( 0 == pEntry->outanding_usages_count && "MICDevice: trying to remove program from device while kernels are still running" );

    if (0 != pEntry->outanding_usages_count)
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle,
            TEXT("MICDevice: trying to remove program from device while %d kernels are still running"), (long)pEntry->outanding_usages_count);
    }

    RemoveProgramFromDevice( pEntry );

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
        CL_DEV_SUCCESS    The function is executed successfully.
********************************************************************************************************************/
cl_dev_err_code ProgramService::UnloadCompiler()
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("UnloadCompiler enter"));
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
        binary                    A pointer to buffer wherein program binary will be stored.
        size_ret                The actual size in bytes of the returned buffer. When size is equal to 0 and binary is NULL, returns size in bytes of a program binary. If NULL the parameter is ignored.
    Returns
        CL_DEV_SUCCESS            The function is executed successfully.
        CL_DEV_INVALID_PROGRAM    If program is not valid program object.
        CL_DEV_INVALID_VALUE    If size is not enough to store the binary or binary is NULL and size is not 0.
********************************************************************************************************************/
cl_dev_err_code ProgramService::GetProgramBinary( cl_dev_program IN prog,
                                        size_t IN size,
                                        void* OUT binary,
                                        size_t* OUT sizeRet
                                        )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("GetProgramBinary enter"));

    TProgramMap::iterator    it;

    // Access program map
    m_muProgMap.Lock();
    it = m_mapPrograms.find(prog);
    if( it == m_mapPrograms.end())
    {
        m_muProgMap.Unlock();
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Requested program not found (%0X)"), (size_t)prog);
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
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("GetBuildLog enter"));

    TProgramMap::iterator    it;

    // Access program map
    m_muProgMap.Lock();
    it = m_mapPrograms.find(prog);
    if( it == m_mapPrograms.end())
    {
        m_muProgMap.Unlock();
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Requested program not found (%0X)"), (size_t)prog);
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
        count                    Size of the buffer passed to the function in terms of cl_prog_binary_desc.
    Output
        types                    A pointer to buffer wherein binary types will be stored.
        count_ret                The actual size ofthe buffer returned by the function in terms of cl_prog_binary_desc.
                                When count is equal to 0 and types is NULL, function returns a size of the list.
                                If NULL the parameter is ignored.
    Returns
        CL_DEV_SUCCESS            The function is executed successfully.
        CL_DEV_INVALID_PROGRAM    If program is not valid program object.
        CL_DEV_INVALID_VALUE    If count is not enough to store the binary or types is NULL and count is not 0.
***************************************************************************************************************/
cl_dev_err_code ProgramService::GetSupportedBinaries( size_t IN size,
                                       cl_prog_binary_desc* OUT types,
                                       size_t* OUT sizeRet
                                       )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("GetSupportedBinaries enter"));
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
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("GetKernelId enter"));

    if ( (NULL == name) || (NULL == kernelId) )
    {
        return CL_DEV_INVALID_VALUE;
    }

    TProgramMap::const_iterator    it;

    // Access program map
    m_muProgMap.Lock();
    it = m_mapPrograms.find(prog);
    if( it == m_mapPrograms.end())
    {
        m_muProgMap.Unlock();
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Requested program not found (%0X)"), (size_t)prog);
        return CL_DEV_INVALID_PROGRAM;
    }

    TProgramEntry *pEntry = it->second;
    m_muProgMap.Unlock();

    if ( pEntry->clBuildStatus != CL_BUILD_SUCCESS )
    {
        return CL_DEV_INVALID_PROGRAM;
    }

    // Find if ID is already allocated
    TKernelName2Entry::const_iterator    nameIt;
    nameIt = pEntry->mapName2Kernels.find(name);

    if ( pEntry->mapName2Kernels.end() == nameIt )
    {
        return CL_DEV_INVALID_KERNEL_NAME;
    }

    *kernelId = kernel_entry_2_cl_dev_kernel(nameIt->second);
    return CL_DEV_SUCCESS;
}

cl_dev_err_code ProgramService::GetProgramKernels( cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels,
                         cl_uint* OUT numKernelsRet )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("GetProgramKernels enter"));

    TProgramMap::const_iterator    it;
    // Access program map
    m_muProgMap.Lock();
    it = m_mapPrograms.find(prog);
    if( it == m_mapPrograms.end())
    {
        m_muProgMap.Unlock();
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Requested program not found (%0X)"), (size_t)prog);
        return CL_DEV_INVALID_PROGRAM;
    }

    const TProgramEntry *pEntry = it->second;
    m_muProgMap.Unlock();

    if ( pEntry->clBuildStatus != CL_BUILD_SUCCESS )
    {
        return CL_DEV_INVALID_PROGRAM;
    }

    size_t          uiNumProgKernels = pEntry->mapName2Kernels.size();

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

    // Retrieve kernels
    TKernelName2Entry::const_iterator kern_it = pEntry->mapName2Kernels.begin();
    TKernelName2Entry::const_iterator kern_it_end = pEntry->mapName2Kernels.end();

    for(unsigned int i=0; kern_it != kern_it_end; ++i, ++kern_it)
    {
        assert( i < uiNumProgKernels && "TKernelName2Entry changed under the hood?");
        kernels[i] = kernel_entry_2_cl_dev_kernel(kern_it->second);
    }

    if ( NULL != numKernelsRet )
    {
        *numKernelsRet = uiNumProgKernels;
    }

    return CL_DEV_SUCCESS;
}

cl_dev_err_code ProgramService::GetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN value_size,
                    void* OUT value, size_t* OUT valueSizeRet ) const
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("GetKernelInfo enter"));

    const TKernelEntry* k_entry = cl_dev_kernel_2_kernel_entry( kernel );

    if (NULL == k_entry)
    {
        return CL_DEV_INVALID_VALUE;
    }

    const ICLDevBackendKernel_* pKernel = k_entry->pKernel;
    const ICLDevBackendKernelProporties* pKernelProps = pKernel->GetKernelProporties();

    // Set value parameters
    cl_uint stValSize;
    unsigned long long ullValue;
    const void*    pValue;
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
        {
            size_t private_mem_size = pKernelProps->GetPrivateMemorySize();
            ullValue = MIN(MIC_MAX_WORK_GROUP_SIZE, (MIC_DEV_MAX_WG_PRIVATE_SIZE /(private_mem_size ? private_mem_size : 1)) );
        }
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

    if ( NULL != valueSizeRet )
    {
        *valueSizeRet = stValSize;
    }

    if ( (0 == value_size) && (NULL == value) )
    {
        if ( NULL == valueSizeRet )
        {
            return CL_DEV_INVALID_VALUE;
        }
        return CL_DEV_SUCCESS;
    }

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

    return CL_DEV_SUCCESS;
}

const ICLDevBackendKernel_* ProgramService::GetBackendKernel( cl_dev_kernel kernel ) const
{
    TKernelEntry* k_entry = cl_dev_kernel_2_kernel_entry(kernel);

    if (NULL == k_entry)
    {
        assert( NULL != k_entry && "MICDevice: Wrong kernel passed to device" );
        return 0;
    }

    return k_entry->pKernel;
}

uint64_t ProgramService::AcquireKernelOnDevice( cl_dev_kernel kernel )
{
    TKernelEntry* k_entry = cl_dev_kernel_2_kernel_entry(kernel);

    if (NULL == k_entry)
    {
        assert( NULL != k_entry && "MICDevice: Wrong kernel passed to device" );
        return 0;
    }

    ++(k_entry->pProgEntry->outanding_usages_count);
    return k_entry->uDevKernelEntry;
}

void ProgramService::releaseKernelOnDevice( cl_dev_kernel kernel )
{
    TKernelEntry* k_entry = cl_dev_kernel_2_kernel_entry(kernel);

    if (NULL == k_entry)
    {
        assert( NULL != k_entry && "MICDevice: Wrong kernel passed to device" );
        return;
    }

    TProgramEntry* p_entry = k_entry->pProgEntry;

    long new_value = --(p_entry->outanding_usages_count);
    assert( new_value >= 0 && "MICDevice: Program usage counter underloaded" );

    if (new_value <= 0)
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("MICDevice: Program outstaning usage counter underloaded: %d"), new_value);
    }
}


//////////////////////////////////////////////////////////////////////////////////////
//    Private methods
void ProgramService::DeleteProgramEntry(TProgramEntry* pEntry)
{
    // Finally release the object
    pEntry->pProgram->Release();

    TKernelName2Entry::iterator it     = pEntry->mapName2Kernels.begin();
    TKernelName2Entry::iterator it_end = pEntry->mapName2Kernels.end();

    for (; it != it_end; ++it)
    {
        TKernelEntry* kernel_entry = it->second;
        delete kernel_entry;
    }

    pEntry->mapName2Kernels.clear();
    pEntry->mapId2Kernels.clear();

    delete pEntry;
}

bool ProgramService::BuildKernelData(TProgramEntry* pEntry)
{
    assert( pEntry && pEntry->pProgram );

    const ICLDevBackendProgram_* program = pEntry->pProgram;

    int kernels_count = program->GetKernelsCount();
    assert( (kernels_count >= 0) && "MIC BuildKernelData thinks program was built successfully while backend not" );

    if (0 == kernels_count)
    {
        // no kernels in program - nothing to do
        return true;
    }

    // copy program to device and get back list of kernel structs on device
    COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT input;

    input.uid_program_on_device    = pEntry->uid_program_on_device;
    input.required_executable_size = program->GetProgramCodeContainer()->GetCodeSize();
    input.number_of_kernels        = kernels_count;

    // allocate output strcut on my stack
    size_t required_output_struct_size = COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT_SIZE( kernels_count );
    COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT* output = (COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT*)alloca( required_output_struct_size );

    if (! CopyProgramToDevice( program, sizeof(input), &input, required_output_struct_size, output ))
    {
        // problem copying to device
        MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("MICDevice: Program Service failed to copy program to device."), "");
        return false;
    }

    // check that device executed ok
    if ((uint64_t)kernels_count != output->filled_kernels)
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle,
            TEXT("MICDevice: Program Service create all kernels on device: required %d created %d."),
            kernels_count, output->filled_kernels );
        return false;
    }

    // build kernel maps
    for (int i = 0; i < kernels_count; ++i)
    {
        TKernelEntry* kernel_entry = new TKernelEntry;
        assert( kernel_entry && "Cannot allocate TKernelEntry structure" );

        kernel_entry->pProgEntry = pEntry;

        cl_dev_err_code err_code = program->GetKernel( i, &(kernel_entry->pKernel) );
        assert( CL_DEV_SUCCEEDED(err_code) && kernel_entry->pKernel );

        kernel_entry->uDevKernelEntry = 0; // to be asserted later

        // insert entry to the TKernelName2Entry map
        pEntry->mapName2Kernels[ kernel_entry->pKernel->GetKernelName() ] = kernel_entry;

        // insert entry to the TKernelId2Entry map
        pEntry->mapId2Kernels[ kernel_entry->pKernel->GetKernelID() ] = kernel_entry;
    }

    // now parse the COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT and update maps with device info
    for (int i = 0; i < kernels_count; ++i)
    {
        assert( (sizeof( unsigned long long int ) <= sizeof( uint64_t )) &&
            "Assumption that uint64_t can contain unsigned long long int" );

        COPY_PROGRAM_TO_DEVICE_KERNEL_INFO* info = &(output->device_kernel_info_pts[i]);

        TKernelId2Entry::iterator it = pEntry->mapId2Kernels.find(
                ( unsigned long long int )info->kernel_id );

        assert( (it != pEntry->mapId2Kernels.end()) && "Kernel IDs are the same on host and device" );
        it->second->uDevKernelEntry = info->device_info_ptr;
    }

    return true;
}

//
// Copy compiled program to the device and get back device kernel info pointers
//
bool ProgramService::CopyProgramToDevice( const ICLDevBackendProgram_* pProgram,
                                 size_t input_size, COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT*  input,
                                 size_t output_size,COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT* output )
{
    size_t              prog_blob_size = 0;
    void*               blob;

    cl_dev_err_code     be_err;
    COIRESULT           coi_err;
    COIBUFFER           coi_buffer_prog, cio_buffer_output;
    COIMAPINSTANCE      map_instance;

    COIPROCESS          in_pProcesses[] = { m_DevService.getDeviceProcessHandle() };

    // 1. get required serialization buffer size
    be_err = GetSerializationService()->GetSerializationBlobSize(
                            SERIALIZE_TO_DEVICE, pProgram, &prog_blob_size);

    assert( CL_DEV_SUCCEEDED(be_err) && (prog_blob_size > 0) && "MIC BE GetSerializationBlobSize()" );

    // 2. Create COI buffer for program serialization
    coi_err = COIBufferCreate( prog_blob_size,
                               COI_BUFFER_NORMAL, COI_OPTIMIZE_SOURCE_WRITE|COI_OPTIMIZE_SINK_READ,
                               NULL,    // no init data
                               ARRAY_ELEMENTS(in_pProcesses), in_pProcesses,
                               &coi_buffer_prog );

    assert( COI_SUCCESS == coi_err  && "Create buffer for program serialization" );

    if ( COI_SUCCESS != coi_err )
    {

        MicErrLog(m_pLogDescriptor, m_iLogHandle,
            TEXT("MICDevice: Program Service failed to create COI buffer for program serialization. Buffer size is %d bytes. COI returned %S"),
            prog_blob_size,
            COIResultGetName( coi_err ));

        return false;
    }

    // 3. Map COI buffer for program serialization
    coi_err = COIBufferMap( coi_buffer_prog,
                            0, prog_blob_size, COI_MAP_WRITE_ENTIRE_BUFFER,
                            0, NULL,            // no dependencies
                            NULL,               // execute immediately
                            &map_instance,
                            &blob );

    assert( COI_SUCCESS == coi_err  && "Map buffer for program serialization" );

    if ( COI_SUCCESS != coi_err )
    {

        MicErrLog(m_pLogDescriptor, m_iLogHandle,
            TEXT("MICDevice: Program Service failed to map COI buffer for program serialization. Buffer size is %d bytes. COI returned %S"),
            prog_blob_size,
            COIResultGetName( coi_err ));

        COIBufferDestroy( coi_buffer_prog );
        return false;
    }

    // 4. Serialize program
    be_err = GetSerializationService()->SerializeProgram(
                            SERIALIZE_TO_DEVICE, pProgram, blob, prog_blob_size );

    assert( CL_DEV_SUCCEEDED( be_err ) && "MIC BE SerializeProgram()" );

    // 5. Unmap serialization buffer
    coi_err = COIBufferUnmap( map_instance, 0, NULL, NULL ); // execute immediately

    assert( COI_SUCCESS == coi_err  && "Unmap buffer for program serialization after it was filled" );

    if ( COI_SUCCESS != coi_err )
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle,
            TEXT("MICDevice: Program Service failed to unmap COI buffer for program serialization after it was filled. Buffer size is %d bytes. COI returned %S"),
            prog_blob_size,
            COIResultGetName( coi_err ));

        COIBufferDestroy( coi_buffer_prog );
        return false;
    }

    // 6. Create in-place COI buffer for output data
    coi_err = COIBufferCreateFromMemory( output_size,                // size of buffer to be filled
                                         COI_BUFFER_NORMAL, COI_OPTIMIZE_SOURCE_READ|COI_OPTIMIZE_SINK_WRITE,
                                         output,
                                         ARRAY_ELEMENTS(in_pProcesses), in_pProcesses,
                                         &cio_buffer_output );

    assert( COI_SUCCESS == coi_err && "COIBufferCreateFromMemory failed to create buffer from memory for device kernels list" );

    if (COI_SUCCESS != coi_err)
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle,
            TEXT("MICDevice: Program Service failed to create COI buffer from memory for device kernels list. Buffer size is %d bytes. COI returned %S"),
            output_size,
            COIResultGetName( coi_err ));

        COIBufferDestroy( coi_buffer_prog );
        return false;
    }

    // 7. Call MIC device
    COIBUFFER           buffers[]    = { coi_buffer_prog,   cio_buffer_output };
    COI_ACCESS_FLAGS    buf_access[] = { COI_SINK_READ,     COI_SINK_WRITE_ENTIRE };

    bool ok = m_DevService.runServiceFunction(
                                DeviceServiceCommunication::COPY_PROGRAM_TO_DEVICE,
                                input_size, input,                   // input_data
                                0, NULL,                             // ouput_data
                                ARRAY_ELEMENTS(buffers), buffers, buf_access); // buffers passed

    assert( ok && "Cannot run Device Function to copy program to device" );

    // map buffer to get results
    coi_err = COIBufferMap(cio_buffer_output, 0, output_size, COI_MAP_READ_ONLY, 0, NULL, NULL, &map_instance, &blob );
    assert( (COI_SUCCESS == coi_err) && "COIBufferMap failed to map buffer with device kernels list" );
    assert( ((size_t)output == (size_t)blob) && "COIBufferMap did not return original data pointer for buffer from memory" );

    if (COI_SUCCESS != coi_err)
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle,
            TEXT("MICDevice: Program Service failed to map COI buffer with device kernels list. Buffer size is %d bytes. COI returned %S"),
            output_size,
            COIResultGetName( coi_err ));

        COIBufferDestroy( coi_buffer_prog );
        COIBufferDestroy( cio_buffer_output );
        return false;
    }

    // unmap buffer back - data should remain in the original memory
    coi_err = COIBufferUnmap( map_instance, 0, NULL, NULL );
    assert( (COI_SUCCESS == coi_err) && "COIBufferUnmap failed to unmap buffer with device kernels list" );

    // remove COI Buffer structure - data should remain in the original buffer
    coi_err = COIBufferDestroy( coi_buffer_prog );
    assert( (COI_SUCCESS == coi_err) && "COIBufferDestroy failed to destroy buffer with serialized program" );

    coi_err = COIBufferDestroy( cio_buffer_output );
    assert( (COI_SUCCESS == coi_err) && "COIBufferDestroy failed to destroy buffer with device kernels list" );

    return true;
}

//
// Remove program from device
//
bool ProgramService::RemoveProgramFromDevice( const TProgramEntry* pEntry )
{
    uint64_t            input = pEntry->uid_program_on_device;

    // param:
    //  input misc   - uint64_t BE program id
    //  output miosc - no
    //  buffers      - no
    bool ok = m_DevService.runServiceFunction(
                                DeviceServiceCommunication::REMOVE_PROGRAM_FROM_DEVICE,
                                sizeof(uint64_t), &input,            // input_data
                                0, NULL,                             // ouput_data
                                0, NULL, NULL);                      // buffers passed

    assert( ok && "Cannot run Device Function to remove program from device" );

    return ok;
}

