// Copyright (c) 2006-2013 Intel Corporation
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

#include "mic_dev_limits.h"
#include "mic_common_macros.h"
#include "device_service_communication.h"
#include "mic_device_interface.h"
#include "mic_sys_info_internal.h"
#include "mic_tracer.h"
#include "cl_sys_defines.h"

#include <source/COIBuffer_source.h>
#include <source/COIPipeline_source.h>

#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <math.h>

#ifdef WIN32
#include <Windows.h>
#endif

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL;
using namespace Intel::OpenCL::DeviceBackend;

// Static members
static cl_prog_binary_desc gSupportedBinTypes[] =
{
    {CL_PROG_DLL_X86, 0, 0},
    {CL_PROG_BIN_EXECUTABLE_LLVM, 0, 0},
};
static    unsigned int    UNUSED(gSupportedBinTypesCount) = ARRAY_ELEMENTS(gSupportedBinTypes);

ProgramService::ProgramService(cl_int devId, IOCLFrameworkCallbacks *devCallbacks,
                                              IOCLDevLogDescriptor *logDesc,
                                              DeviceServiceCommunication& dev_service) :
    m_DevService( dev_service), m_BE_Compiler( m_DevService, MIC_BACKEND_DLL_NAME ),
    m_iDevId(devId), m_pLogDescriptor(logDesc), m_iLogHandle(0),
    m_pCallBacks(devCallbacks)
{
    if ( NULL != logDesc )
    {
        cl_int ret = m_pLogDescriptor->clLogCreateClient(m_iDevId, "MIC Device: Program Service", &m_iLogHandle);
        if(CL_DEV_SUCCESS != ret)
        {
            m_iLogHandle = 0;
        }
    }

    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "MICDevice: Program Service - Created");
}

bool ProgramService::Init( void )
{
    cl_dev_err_code err;

    {
        // acquire creation lock
        OclAutoMutex lock(&m_BE_Compiler.creationLock);
        err = m_BE_Compiler.be_wrapper.Init();
    }

    if (! CL_DEV_SUCCEEDED(err))
    {
        assert( false && "MIC Device Agent cannot initialize BE" );
        MicCriticLog(m_pLogDescriptor, m_iLogHandle, "%s", "MIC Device Agent cannot initialize BE");

        return false;
    }

    return true;
}

ProgramService::~ProgramService()
{
    TProgramList::iterator    it;

    // Go thought the map and remove all allocated programs
    for(it = m_Programs.begin(); it != m_Programs.end(); ++it)
    {
        DeleteProgramEntry(*it);
    }

    m_Programs.clear();

    ReleaseBackendServices();

    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "MICDevice: Program Service - Distructed");

    if (0 != m_iLogHandle)
    {
        m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
    }
}

/****************************************************************************************************************
 BEGIN MICBackendOptions
********************************************************************************************************************/
MICBackendOptions::MICBackendOptions( DeviceServiceCommunication& dev_service ) :
             m_dev_service( dev_service), m_bUseVectorizer(false), m_bUseVtune(false),
             m_apf_level(APFLEVEL_DEFAULT)
{
}

void MICBackendOptions::init( bool bUseVectorizer, bool bUseVtune )
{
    m_bUseVectorizer = bUseVectorizer;
    m_bUseVtune      = bUseVtune;
}

void MICBackendOptions::init_cl_options ( const char *options) {
  const char *p;
  if( (NULL != options) && ('\0' != *options) )
  {
    if (NULL != (p = strstr(options, "-dump-opt-asm=")))
    {
      init_for_dump(p);
    }
    const char* opt_prefetch_level = "-auto-prefetch-level=";
    if (NULL != (p = strstr(options, opt_prefetch_level)))
    {
       init_for_apf_level(p+strlen(opt_prefetch_level));
    }
  }
}

void MICBackendOptions::init_for_dump( const char* options )
{
    // extract string withing quotas and assume it is a file name
    std::string fname(options);
    std::string::size_type pos1 = fname.find("\"", 0);
    std::string::size_type pos2 = fname.find("\"", pos1+1);

    if((pos1 != string::npos) && (pos2 != string::npos))
    {
         m_dump_file_name = fname.substr(pos1 + 1, pos2 - pos1 - 1);
    }
}

void MICBackendOptions::init_for_apf_level( const char *p )
{
  // verify option correctness
  if (p == NULL)
  {
    return;
  }
  // parse apf value and set accordingly
  int val = *p - '0';
  if (val >= APFLEVEL_MIN && val <= APFLEVEL_MAX)
  {
    m_apf_level = val;
  }
  assert(val >= APFLEVEL_MIN && val <= APFLEVEL_MAX && "Invalid value for prefetching level");
}

// ICLDevBackendOptions interface

bool MICBackendOptions::GetBooleanValue( int optionId, bool defaultValue) const
{
    return (CL_DEV_BACKEND_OPTION_USE_VTUNE == optionId) ? m_bUseVtune : defaultValue;
}

int MICBackendOptions::GetIntValue( int optionId, int defaultValue) const
{
    switch (optionId)
    {
        case CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE:
            return !m_bUseVectorizer ? TRANSPOSE_SIZE_1 : defaultValue;

        case CL_DEV_BACKEND_OPTION_TARGET_DESC_SIZE:
            {
                int size = getTargetDescriptionSize();
                return (size <= 0) ? defaultValue : size;
            }

        case CL_DEV_BACKEND_OPTION_APF_LEVEL:
            return m_apf_level;

        default:
            return defaultValue;
    }
}

const char* MICBackendOptions::GetStringValue( int optionId, const char* defaultValue)const
{
    switch (optionId)
    {
        case CL_DEV_BACKEND_OPTION_DEVICE:
            return "mic";

        case CL_DEV_BACKEND_OPTION_SUBDEVICE:
			return get_mic_cpu_arch();

        case CL_DEV_BACKEND_OPTION_DUMPFILE:
            return m_dump_file_name.c_str();

        default:
            return defaultValue;
    }
}

bool MICBackendOptions::GetValue( int optionId, void* Value, size_t* pSize) const
{
    if ((CL_DEV_BACKEND_OPTION_TARGET_DESC_BLOB == optionId) && Value && pSize && (*pSize > 0) )
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

    assert( (NULL != in_pProcesses[0]) && "Device process disappeared" );

    if (NULL == in_pProcesses[0])
    {
         return false;
    }

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

    if (ok && (0 != filled_size))
    {
        // map buffer to get results
        coi_err = COIBufferMap(coi_buffer, 0, *pSize, COI_MAP_READ_ONLY, 0, NULL, NULL, &map_instance, &data_address );
        assert( (COI_SUCCESS == coi_err) && "COIBufferMap failed to map buffer with BE options" );
        assert( ((size_t)Value == (size_t)data_address) && "COIBufferMap did not return original data pointer for buffer from memory" );

        if (COI_SUCCESS != coi_err)
        {
            ok = false;
        }

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

        // load Backend Compiler
        ICLDevBackendServiceFactory* be_factory = m_BE_Compiler.be_wrapper.GetBackendFactory();

        if (NULL == be_factory)
        {
            assert( false && "MIC Device Agent cannot create BE Factory" );
            MicCriticLog(m_pLogDescriptor, m_iLogHandle, "%s", "MIC Device Agent cannot create BE Factory");
            break;
        }

		const MICDeviceConfig& tMicConfig = MICSysInfo::getInstance().getMicDeviceConfig();
        // get compiler options
        m_BE_Compiler.MICOptions.init( tMicConfig.UseVectorizer(),
                                       tMicConfig.UseVTune()
                                      );

        err = be_factory->GetCompilationService( &m_BE_Compiler.MICOptions,
                                                &comp_temp );

        if (CL_DEV_FAILED( err ))
        {
            assert( false && "MIC Device Agent cannot create Backend Compilation Service" );
            MicCriticLog(m_pLogDescriptor, m_iLogHandle, "%s", "MIC Device Agent cannot create Backend Compilation Service");
            break;
        }

        err = be_factory->GetSerializationService( &m_BE_Compiler.MICOptions,
                                                  &ser_temp );

        if (CL_DEV_FAILED( err ))
        {
            assert( false && "MIC Device Agent cannot create Backend Serialization Service" );
            MicCriticLog(m_pLogDescriptor, m_iLogHandle, "%s", "MIC Device Agent cannot create Backend Serialization Service");
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

    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "CheckProgramBinary enter");

    // Check container size
    if ( sizeof(cl_prog_container_header) > binSize )
    {
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "Invalid Binary Size was provided");
        return CL_DEV_INVALID_BINARY;
    }

    // Check container mask
    if ( memcmp(_CL_CONTAINER_MASK_, pProgCont->mask, sizeof(pProgCont->mask)) )
    {
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "Invalid Container Mask was provided");
        return CL_DEV_INVALID_BINARY;
    }

    // Check supported container type
    switch ( pProgCont->container_type )
    {
    // Supported containers
    case CL_PROG_CNT_PRIVATE:
        break;

    default:
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "Invalid Container Type was provided");
        return CL_DEV_INVALID_BINARY;
    }

    // Check supported binary types
    switch ( pProgCont->description.bin_type )
    {
    // Supported program binaries
    case CL_PROG_BIN_EXECUTABLE_LLVM:// The container should contain valid LLVM-IR
        break;

    case CL_PROG_OBJ_X86:            // The container should contain binary buffer of object file
    case CL_PROG_DLL_X86:            // The container should contain a full path name to DLL file to load
    default:
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, "Invalid Container Type was provided<%0X>", pProgCont->description.bin_type);
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
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "CreateProgram enter");

    // Input parameters validation
    if(0 == binSize || NULL == bin)
    {
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "Invalid binSize or bin parameters");
        return CL_DEV_INVALID_VALUE;
    }

    if ( NULL == prog )
    {
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "Invalid prog parameter");
        return CL_DEV_INVALID_VALUE;
    }

    // If the origin of binary is user loaded
    // check for rightness
    if(prop == CL_DEV_BINARY_USER)
    {
        cl_dev_err_code rc = CheckProgramBinary(binSize, bin);
        if ( CL_DEV_FAILED(rc) )
        {
            MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "Check program binary failed");
            return rc;
        }
    }

    // Create new program
    const cl_prog_container_header*    pProgCont = (cl_prog_container_header*)bin;
    TProgramEntry*    pEntry        = new TProgramEntry(m_iDevId);
    if ( NULL == pEntry )
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, "%s", "Cann't allocate program entry");
        return CL_DEV_OUT_OF_MEMORY;
    }
    pEntry->pProgram = NULL;
    pEntry->uid_program_on_device = (uint64_t)pEntry;
    pEntry->clBuildStatus = CL_BUILD_NONE;

    cl_dev_err_code ret;
    switch(pProgCont->description.bin_type)
    {
    case CL_PROG_BIN_EXECUTABLE_LLVM:
        {
            ICLDevBackendCompilationService* compiler = GetCompilationService();
            if (NULL == compiler)
            {
                MicErrLog(m_pLogDescriptor, m_iLogHandle, "Cannot load compilation service", "");
                return CL_DEV_OUT_OF_MEMORY;
            }
            ret = compiler->CreateProgram(pProgCont, (ICLDevBackendProgram_**)&pEntry->pProgram);
        }
        break;
    default:
        MicErrLog(m_pLogDescriptor, m_iLogHandle, "Failed to find approproiate program for type<%d>", pProgCont->description.bin_type);
        delete pEntry;
        return CL_DEV_INVALID_BINARY;
    }

    if ( CL_DEV_FAILED(ret) )
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, "Failed to create program from given container<%0X>", ret);
        delete pEntry;
        return CL_DEV_INVALID_BINARY;
    }

    cl_dev_program newProgId = pEntry;

    m_muPrograms.Lock();
    m_Programs.push_back(pEntry);
    m_muPrograms.Unlock();

    *prog = newProgId;

    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "CreateProgram Exit");
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
cl_dev_err_code ProgramService::BuildProgram( cl_dev_program OUT prog,
                                    const char* IN options,
                                    cl_build_status* OUT buildStatus
                                   )
{

    const char *p = NULL;

    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "BuildProgram enter");

    TProgramEntry* pEntry = (TProgramEntry*)prog;

    if (NULL == pEntry)
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, "Requested program not found (%0X)", (size_t)prog);
        return CL_DEV_INVALID_PROGRAM;
    }

    // Program already built?
    if (CL_BUILD_SUCCESS == pEntry->clBuildStatus)
    {
        return CL_DEV_BUILD_ALREADY_COMPLETE;
    }

    // Trying two simultaneous builds of the same program?
    if ( CL_BUILD_IN_PROGRESS == pEntry->clBuildStatus )
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, "Invalid build status(%d), should be CL_BUILD_NONE(%d)",
            pEntry->clBuildStatus, CL_BUILD_NONE);
        //Asserting because framework should have stopped it
        assert(0);
        return CL_DEV_BUILD_IN_PROGRESS;
    }

    if ( CL_BUILD_NONE != pEntry->clBuildStatus )
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, "Invalid build status(%d), should be CL_BUILD_NONE(%d)",
            pEntry->clBuildStatus, CL_BUILD_NONE);
        return CL_DEV_INVALID_OPERATION;
    }

    pEntry->clBuildStatus = CL_BUILD_IN_PROGRESS;
    MicDbgLog(m_pLogDescriptor, m_iLogHandle, "%s", "Starting build");

    CommandTracer cmdTracer;
    cmdTracer.set_command_type((char*)"Build");
    cmdTracer.set_current_time_command_host_time_start();
    
    ICLDevBackendCompilationService* compiler = GetCompilationService();
    
    cl_build_status status = CL_BUILD_ERROR;
    cl_dev_err_code ret    = CL_DEV_OUT_OF_MEMORY;
    
    if (NULL != compiler)
    {
        MICBackendOptions buildOptions( m_DevService);
        buildOptions.init_cl_options (options);

        ret = compiler->BuildProgram(pEntry->pProgram, &buildOptions);
        MicDbgLog(m_pLogDescriptor, m_iLogHandle, "Build Done (%d)", ret);
    }
    
    if (CL_DEV_SUCCEEDED(ret))
    {
        pEntry->copy_to_device_ok = BuildKernelData( pEntry );
        if (pEntry->copy_to_device_ok)
        {
            status = CL_BUILD_SUCCESS;
        }
    }

    pEntry->clBuildStatus = status;
    
    cmdTracer.set_command_id((uint64_t)pEntry);
    cmdTracer.set_current_time_command_host_time_end();

    // if the user requested -dump-opt-llvm, print this module
    if( CL_DEV_SUCCEEDED(ret) && (NULL != options) && ('\0' != *options) &&
        (NULL != (p = strstr(options, "-dump-opt-llvm="))))
    {
        assert( pEntry->pProgram && "Program must be created already");
        
        MICBackendOptions dumpOptions( m_DevService );
        dumpOptions.init_for_dump(p);
        
        compiler->DumpCodeContainer( pEntry->pProgram->GetProgramCodeContainer(), &dumpOptions);
    }

    if ( NULL != buildStatus )
    {
        *buildStatus = status;
    }

    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "BuildProgram Exit");
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
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "ReleaseProgram enter");

    TProgramEntry* pEntry = (TProgramEntry*)prog;

    if (NULL == pEntry)
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, "Requested program not found (%0X)", (size_t)prog);
        return CL_DEV_INVALID_PROGRAM;
    }

    // Here we are sure that program exists and belongs to this device. Unregister it first!
    TProgramList::iterator    it;

    m_muPrograms.Lock();

    for( it = m_Programs.begin(); it != m_Programs.end(); ++it)
    {
        if (*it == prog)
        {
            break;
        }
    }

    // conversion cl_dev_program_2_program_entry() checks that program belongs to this device - it MUST be registered!
    assert( (m_Programs.end() != it) && "MICDevice: Program not found in owner's device!");

    m_Programs.erase(it);
    m_muPrograms.Unlock();

    if (pEntry->pProgram->GetKernelsCount() > 0)
    {
        RemoveProgramFromDevice( pEntry );
    }

    DeleteProgramEntry(pEntry);
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
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "UnloadCompiler enter");
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
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "GetProgramBinary enter");

    TProgramEntry* entry = (TProgramEntry*)prog;

    if( NULL == entry )
    {
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, "Requested program not found (%0X)", (size_t)prog);
        return CL_DEV_INVALID_PROGRAM;
    }

    ICLDevBackendProgram_ *pProg = entry->pProgram;

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
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "GetBuildLog enter");

    TProgramEntry* entry = (TProgramEntry*)prog;

    if( NULL == entry )
    {
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, "Requested program not found (%0X)", (size_t)prog);
        return CL_DEV_INVALID_PROGRAM;
    }

    ICLDevBackendProgram_ *pProg = entry->pProgram;

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
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "GetSupportedBinaries enter");
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
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "GetKernelId enter");

    if ( (NULL == name) || (NULL == kernelId) )
    {
        return CL_DEV_INVALID_VALUE;
    }

    TProgramEntry* pEntry = (TProgramEntry*)prog;

    if( NULL == pEntry )
    {
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, "Requested program not found (%0X)", (size_t)prog);
        return CL_DEV_INVALID_PROGRAM;
    }

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

    *kernelId = (cl_dev_kernel)nameIt->second;
    return CL_DEV_SUCCESS;
}

cl_dev_err_code ProgramService::GetProgramKernels( cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels,
                         cl_uint* OUT numKernelsRet )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "GetProgramKernels enter");

    TProgramEntry* pEntry = (TProgramEntry*)prog;

    if( NULL == pEntry )
    {
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, "Requested program not found (%0X)", (size_t)prog);
        return CL_DEV_INVALID_PROGRAM;
    }

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
        kernels[i] = (cl_dev_kernel)kern_it->second;
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
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "GetKernelInfo enter");

    const TKernelEntry* k_entry = (TKernelEntry*)kernel;

    if (NULL == k_entry)
    {
        assert ( 0 && "Got NULL kernel_id from the runtime");
        return CL_DEV_INVALID_VALUE;
    }

    const ICLDevBackendKernel_* pKernel = k_entry->pKernel;
    const ICLDevBackendKernelProporties* pKernelProps = pKernel->GetKernelProporties();

    // Set value parameters
    cl_uint stValSize = 0;
    unsigned long long ullValue = 0;
    const void*    pValue;
    pValue = &ullValue;
    cl_dev_dispatch_buffer_prop dispatchProperties;

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
            ullValue = MIN(MIC_MAX_WORK_GROUP_SIZE, (MIC_DEV_MAX_WG_PRIVATE_SIZE/((private_mem_size > 0) ? private_mem_size : 1)));
            size_t packSize = pKernelProps->GetMinGroupSizeFactorial();
            if (ullValue > packSize)
                ullValue = ( ullValue ) & ~(packSize-1);
        }
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

    case CL_DEV_KERNEL_ARG_INFO:
          stValSize = pKernel->GetKernelParamsCount() * sizeof(cl_kernel_argument_info);
          pValue = (void*)pKernel->GetKernelArgInfo();
          break;

    case CL_DEV_KERNEL_MEMORY_OBJECT_INDEXES:
        stValSize = pKernel->GetMemoryObjectArgumentCount() * sizeof(unsigned int);
        pValue = (void*)pKernel->GetMemoryObjectArgumentIndexes();
        break;

    case CL_DEV_KERNEL_DISPATCH_BUFFER_PROPERTIES:
        stValSize = sizeof(cl_dev_dispatch_buffer_prop);
        dispatchProperties.size = pKernel->GetArgumentBufferSize() + sizeof(ndrange_dispatcher_data);
        dispatchProperties.argumentOffset = sizeof(ndrange_dispatcher_data);
        dispatchProperties.alignment = 64; // Currently ask for 64 byte aligment. Next BE will provide maximum aligment for the buffer.
        pValue = &dispatchProperties;
        break;

    default:
        return CL_DEV_INVALID_VALUE;
    }

    if ( NULL != value && value_size < stValSize )
    {
            return CL_DEV_INVALID_VALUE;
    }

	if ( NULL != valueSizeRet )
    {
        *valueSizeRet = stValSize;
    }

	if ( NULL != value )
	{
		if ( NULL != pValue )
		{
			memcpy(value, pValue, stValSize);
		} else {
			memset(value, 0, stValSize);
		}
	}

    return CL_DEV_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////
//    Private methods
void ProgramService::DeleteProgramEntry(TProgramEntry* pEntry)
{
    // Finally release the object
    GetCompilationService()->ReleaseProgram(pEntry->pProgram);

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
    COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT* output = (COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT*)STACK_ALLOC( required_output_struct_size );

    assert( output && "Cannot allocate space using alloca()" );
    if (NULL == output)
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, "MICDevice: Program Service failed to allocate space on stack.", "");
		STACK_FREE(output);
        return false;
    }

    if (! CopyProgramToDevice( program, sizeof(input), &input, required_output_struct_size, output ))
    {
        // problem copying to device
        MicErrLog(m_pLogDescriptor, m_iLogHandle, "MICDevice: Program Service failed to copy program to device.", "");
		STACK_FREE(output);
        return false;
    }

    // check that device executed ok
    if ((uint64_t)kernels_count != output->filled_kernels)
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle,
            "MICDevice: Program Service create all kernels on device: required %d created %d.",
            kernels_count, output->filled_kernels );
		STACK_FREE(output);
        return false;
    }

    STATIC_ASSERT((sizeof(unsigned long long int)<=sizeof(uint64_t)));

    // build kernel maps
    for (int i = 0; i < kernels_count; ++i)
    {
        TKernelEntry* kernel_entry = new TKernelEntry;
        assert( (NULL!=kernel_entry) && "Cannot allocate TKernelEntry structure" );
        if ( NULL==kernel_entry )
        {
          MicErrLog(m_pLogDescriptor, m_iLogHandle,
              TEXT("%s"), "MICDevice: Failed to allocate TKernelEntry");
		  STACK_FREE(output);
          return false;
        }

        COPY_PROGRAM_TO_DEVICE_KERNEL_INFO* info = &(output->device_kernel_info_pts[i]);

        kernel_entry->pProgEntry = pEntry;

        cl_dev_err_code err_code = program->GetKernel( i, &(kernel_entry->pKernel) );
        assert( CL_DEV_SUCCEEDED(err_code) && kernel_entry->pKernel );

        if ( CL_DEV_SUCCEEDED(err_code) && (NULL != kernel_entry->pKernel) )
        {
			if ( kernel_entry->pKernel->GetKernelID() != info->kernel_id )
			{
				assert( 0 && "Kernel IDs are the same on host and device" );
				MicErrLog(m_pLogDescriptor, m_iLogHandle,
					TEXT("MICDevice: Kernel ID on teh device(%lld) and the host(%lld) don't match."),
		    		info->kernel_id, kernel_entry->pKernel->GetKernelID());
				STACK_FREE(output);
				return false;
			}
			// insert entry to the TKernelName2Entry map
			pEntry->mapName2Kernels[ kernel_entry->pKernel->GetKernelName() ] = kernel_entry;

			// insert entry to the TKernelId2Entry map
			pEntry->mapId2Kernels[ kernel_entry->pKernel->GetKernelID() ] = kernel_entry;

			kernel_entry->uDevKernelEntry = info->device_info_ptr; // to be updated later
		}
    }

	STACK_FREE(output);
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
    bool                ok = false;

    cl_dev_err_code     be_err;
    COIRESULT           coi_err;
    COIBUFFER           coi_buffer_prog = NULL, cio_buffer_output = NULL;
    COIMAPINSTANCE      map_instance = NULL;

    do {
        COIPROCESS          in_pProcesses[] = { m_DevService.getDeviceProcessHandle() };

        assert( (NULL != in_pProcesses[0]) && "Device process disappeared" );

        if (NULL == in_pProcesses[0])
        {
            MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), "MICDevice: Device process disappeared");
            break;
        }

        // 1. get required serialization buffer size
        ICLDevBackendSerializationService* serializer = GetSerializationService();

        if (NULL == serializer)
        {
            MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), "MICDevice: Cannot load Serialization Service");
            break;
        }

        be_err = serializer->GetSerializationBlobSize( SERIALIZE_TO_DEVICE, pProgram, &prog_blob_size);

        assert( CL_DEV_SUCCEEDED(be_err) && (prog_blob_size > 0) && "MIC BE GetSerializationBlobSize()" );

        if (CL_DEV_FAILED(be_err))
        {
            MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), "MICDevice: Serialization Service failed to calculate blob size");
            break;
        }

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
                "MICDevice: Program Service failed to create COI buffer for program serialization. Buffer size is %d bytes. COI returned %s",
                prog_blob_size,
                COIResultGetName( coi_err ));

            break;
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
                "MICDevice: Program Service failed to map COI buffer for program serialization. Buffer size is %d bytes. COI returned %s",
                prog_blob_size,
                COIResultGetName( coi_err ));

            break;
        }

		CommandTracer cmdTracer;
		cmdTracer.set_command_id(input->uid_program_on_device);
		cmdTracer.set_current_time_build_serialize_time_start();
        // 4. Serialize program
        be_err = serializer->SerializeProgram( SERIALIZE_TO_DEVICE, pProgram, blob, prog_blob_size );
		cmdTracer.set_current_time_build_serialize_time_end();

        assert( CL_DEV_SUCCEEDED( be_err ) && "MIC BE SerializeProgram()" );

        if (CL_DEV_FAILED(be_err))
        {
            MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"),"MICDevice: Serialization Service failed to calculate blob size");
            break;
        }


        // 5. Unmap serialization buffer
        coi_err = COIBufferUnmap( map_instance, 0, NULL, NULL ); // execute immediately

        assert( COI_SUCCESS == coi_err  && "Unmap buffer for program serialization after it was filled" );

        if ( COI_SUCCESS != coi_err )
        {
            MicErrLog(m_pLogDescriptor, m_iLogHandle,
                "MICDevice: Program Service failed to unmap COI buffer for program serialization after it was filled. Buffer size is %d bytes. COI returned %s",
                prog_blob_size,
                COIResultGetName( coi_err ));

            break;
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
                "MICDevice: Program Service failed to create COI buffer from memory for device kernels list. Buffer size is %d bytes. COI returned %s",
                output_size,
                COIResultGetName( coi_err ));

            break;
        }

        // 7. Call MIC device
        COIBUFFER           buffers[]    = { coi_buffer_prog,   cio_buffer_output };
        COI_ACCESS_FLAGS    buf_access[] = { COI_SINK_READ,     COI_SINK_WRITE_ENTIRE };

        cl_dev_err_code dev_err;
        bool done = m_DevService.runServiceFunction(
                                    DeviceServiceCommunication::COPY_PROGRAM_TO_DEVICE,
                                    input_size, input,                             // input_data
                                    sizeof(cl_dev_err_code), &dev_err,             // ouput_data
                                    ARRAY_ELEMENTS(buffers), buffers, buf_access); // buffers passed

        assert( done && "Cannot run Device Function to copy program to device");
        assert( CL_DEV_SUCCESS == dev_err && "CopyProgramOnDevice failed on device");

        if ( !done || CL_DEV_FAILED(dev_err) )
        {
            MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("MICDevice: Failed to copy program to device, done=%d, dev_err=0x%x"), done, dev_err);

            break;
        }

        // map buffer to get results
        // Since buffer was created in-place we expect to have receive the same pointer value in blob.
        coi_err = COIBufferMap(cio_buffer_output, 0, output_size, COI_MAP_READ_ONLY, 0, NULL, NULL, &map_instance, &blob );
        assert( (COI_SUCCESS == coi_err) && "COIBufferMap failed to map buffer with device kernels list" );
        assert( ((size_t)output == (size_t)blob) && "COIBufferMap did not return original data pointer for buffer from memory" );

        if ( COI_SUCCESS != coi_err )
        {
            MicErrLog(m_pLogDescriptor, m_iLogHandle,
                "MICDevice: Program Service failed to map COI buffer with device kernels list. Buffer size is %d bytes. COI returned %s",
                output_size,
                COIResultGetName( coi_err ));

            break;
        }

        if ( (size_t)output != (size_t)blob )
        {
          MicErrLog(m_pLogDescriptor, m_iLogHandle,
              TEXT("MICDevice: Program Service failed to map COI buffer with the same pointer for the output buffer(%p vs %p)"),
                  (void*)blob, (void*) output);
             break;
        }

        ok = true;
    } while (false);

    // unmap buffer back - data should remain in the original memory
    if (NULL != map_instance)
    {
        coi_err = COIBufferUnmap( map_instance, 0, NULL, NULL );
        assert( (COI_SUCCESS == coi_err) && "COIBufferUnmap failed to unmap buffer with device kernels list" );
    }

    // remove COI Buffer structure - data should remain in the original buffer
    if (NULL != coi_buffer_prog)
    {
        coi_err = COIBufferDestroy( coi_buffer_prog );
        assert( (COI_SUCCESS == coi_err) && "COIBufferDestroy failed to destroy buffer with serialized program" );
    }

    if (NULL != cio_buffer_output)
    {
        coi_err = COIBufferDestroy( cio_buffer_output );
        assert( (COI_SUCCESS == coi_err) && "COIBufferDestroy failed to destroy buffer with device kernels list" );
    }

    return ok;
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

