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
//  mic_device.cpp
///////////////////////////////////////////////////////////

#include <cl_sys_info.h>
#include <mic_dev_limits.h>
#include <cl_sys_defines.h>
#include <buildversion.h>
#include <ocl_itt.h>

#include "mic_tracer.h"
#include "mic_device.h"
#include "program_service.h"
#include "mic_logger.h"
#include "cl_sys_info.h"
#include "cl_shutdown.h"
#include "mic_dev_limits.h"
#include "cl_sys_defines.h"
#include "buildversion.h"
#include "device_service_communication.h"
#include "memory_allocator.h"
#include "mic_sys_info.h"
#include "command_list.h"
#include "clang_device_info.h"

using namespace Intel::OpenCL::MICDevice;

USE_SHUTDOWN_HANDLER( MICDevice::unloadRelease );

HostTracer* MICDevice::m_tracer = NULL;

set<IOCLDeviceAgent*>* MICDevice::m_mic_instancies = NULL;
OclMutex*              MICDevice::m_mic_instancies_mutex = NULL;
Intel::OpenCL::Utils::OclDynamicLib	MICDevice::m_sDllCOILib;

typedef struct _cl_dev_internal_cmd_list
{
    void*               cmd_list;
    cl_dev_subdevice_id subdevice_id;
} cl_dev_internal_cmd_list;

static struct Intel::OpenCL::ClangFE::CLANG_DEV_INFO MICDevInfo = {NULL,0,1,0};

#ifdef USE_ITT
ocl_gpa_data* MICDevice::g_pGPAData = NULL;
#endif

///////////////////////////////////////////////////////////////////////////////
//
// BEGIN MIC global management
//
///////////////////////////////////////////////////////////////////////////////

void MICDevice::RegisterMicDevice( MICDevice* dev )
{
    OclAutoMutex lock( m_mic_instancies_mutex );
    m_mic_instancies->insert( dev );
}

bool MICDevice::UnregisterMicDevice( MICDevice* dev )
{
        OclAutoMutex lock( m_mic_instancies_mutex );
        return (0 != m_mic_instancies->erase( dev ));
}

MICDevice::TMicsSet MICDevice::GetActiveMicDevices( bool erase )
{
    TMicsSet ret_list;

    OclAutoMutex lock( m_mic_instancies_mutex );

    set<IOCLDeviceAgent*>::iterator it  = m_mic_instancies->begin();
    set<IOCLDeviceAgent*>::iterator end = m_mic_instancies->end();

    for(; it != end; ++it)
    {
        ret_list.insert( (MICDevice*)*it );
    }

    // If the client ask to erase than remove the mic devices pointers from the set.
    if (erase)
    {
        m_mic_instancies->clear();
    }

    return ret_list;
}

MICDevice::TMicsSet MICDevice::FilterMicDevices( size_t count, const IOCLDeviceAgent* const *dev_arr )
{
    TMicsSet ret_list;

    OclAutoMutex lock( m_mic_instancies_mutex );

    set<IOCLDeviceAgent*>::iterator found_end = m_mic_instancies->end();

    for(size_t i = 0; i < count; ++i)
    {
        IOCLDeviceAgent* dev = const_cast<IOCLDeviceAgent*>(dev_arr[i]);

        if (m_mic_instancies->find( dev ) != found_end)
        {
            ret_list.insert( (MICDevice*)dev );
        }
    }

    return ret_list;
}

///////////////////////////////////////////////////////////////////////////////
//
// END MIC global management
//
///////////////////////////////////////////////////////////////////////////////

MICDevice::MICDevice(cl_uint uiMicId, IOCLFrameworkCallbacks *devCallbacks, IOCLDevLogDescriptor *logDesc)
    : m_pFrameworkCallBacks(devCallbacks), m_uiMicId(uiMicId),
    m_pLogDescriptor(logDesc), m_iLogHandle (0), m_defaultCommandList(NULL), m_pDeviceServiceComm(NULL)
{
}

cl_dev_err_code MICDevice::Init()
{
    m_tracer = HostTracer::getHostTracerInstace();
    if ( NULL != m_pLogDescriptor )
    {
        cl_dev_err_code ret = (cl_dev_err_code)m_pLogDescriptor->clLogCreateClient(m_uiMicId, "MIC Device", &m_iLogHandle);
        if(CL_DEV_SUCCESS != ret)
        {
            return CL_DEV_ERROR_FAIL;
        }
    }

    // Enable VTune source level profiling
    MICDevInfo.bEnableSourceLevelProfiling = MICSysInfo::getInstance().getMicDeviceConfig().UseVTune();

    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "CreateDevice function enter");

    // trying to upload next free device.
    cl_dev_err_code result = DeviceServiceCommunication::deviceSeviceCommunicationFactory(m_uiMicId, &m_pDeviceServiceComm);
    if (CL_DEV_FAILED(result))
    {
        return result;
    }
    assert(m_pDeviceServiceComm);

    // initialize the notificationPort mechanism.
    m_pNotificationPort = NotificationPort::notificationPortFactory(NOTIFICATION_PORT_MAX_BARRIERS, g_pGPAData);
    if (NULL == m_pNotificationPort)
    {
        return CL_DEV_ERROR_FAIL;
    }

    m_pProgramService = new ProgramService( m_uiMicId, m_pFrameworkCallBacks, m_pLogDescriptor,
                                           *m_pDeviceServiceComm);

    if (NULL == m_pProgramService)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    if (!m_pProgramService->Init())
    {
        delete m_pProgramService;
        m_pProgramService = NULL;
        return CL_DEV_ERROR_FAIL;
    }

    m_pMemoryAllocator = MemoryAllocator::getMemoryAllocator( m_uiMicId, m_pLogDescriptor, MIC_MAX_BUFFER_ALLOC_SIZE(m_uiMicId) );

    if (NULL == m_pMemoryAllocator)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    // record Mic device in global set
    RegisterMicDevice( this );

    return CL_DEV_SUCCESS;
}

MICDevice::~MICDevice()
{
}

bool MICDevice::loadingInit()
{
    m_mic_instancies = new set<IOCLDeviceAgent*>;
    m_mic_instancies_mutex = new OclMutex;

	if (!m_sDllCOILib.Load(COI_HOST_DLL_NAME))
	{
		return false;
	}

#ifdef USE_ITT
  if ( MICSysInfo::getInstance().getMicDeviceConfig().UseITT() )
  {
    g_pGPAData = new ocl_gpa_data;
    g_pGPAData->bUseGPA = true;
    g_pGPAData->pDeviceDomain = __itt_domain_create("OpenCL.DeviceAgent.MIC");
  }
#endif

  return true;
}

void MICDevice::unloadRelease()
{
    // Wait until all Notificatoin port threads are dieing
    NotificationPort::waitForAllNotificationPortThreads();
    assert(GetActiveMicDevices().size() == 0);

#ifdef USE_ITT
    if ( NULL != g_pGPAData )
    {
        delete g_pGPAData;
        g_pGPAData = NULL;
    }
#endif

    delete m_mic_instancies;
    m_mic_instancies = NULL;

    delete m_mic_instancies_mutex;
    m_mic_instancies_mutex = NULL;   
}


// ---------------------------------------
// Public functions / Device entry points
extern "C"
cl_dev_err_code clDevCreateDeviceInstance(  cl_uint        dev_id,
                                   IOCLFrameworkCallbacks  *pDevCallBacks,
                                   IOCLDevLogDescriptor    *pLogDesc,
                                   IOCLDeviceAgent*        *pDevice
                                   )
{
    if(NULL == pDevCallBacks || NULL == pDevice)
    {
        return CL_DEV_INVALID_OPERATION;
    }

    MICDevice *pNewDevice = new MICDevice(dev_id, pDevCallBacks, pLogDesc);
    if ( NULL == pNewDevice )
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    cl_dev_err_code rc = pNewDevice->Init();
    if ( CL_DEV_FAILED(rc) )
    {
        pNewDevice->clDevCloseDevice();
        return rc;
    }
    *pDevice = pNewDevice;
    return CL_DEV_SUCCESS;
}

// Device Fission support
/****************************************************************************************************************
 clDevGetFECompilerDecription
    Return front-end compiler description
****************************************************************************************************************/
const char* MICDevice::clDevFEModuleName() const
{
#if defined (_WIN32)
#if defined (_M_X64)
    static const char* sFEModuleName = "clang_compiler64";
#else
    static const char* sFEModuleName = "clang_compiler32";
#endif
#else
    static const char* sFEModuleName = "clang_compiler";
#endif
    return sFEModuleName;
}

const void* MICDevice::clDevFEDeviceInfo() const
{
  if ( NULL == MICDevInfo.sExtensionStrings)
  {
    MICDevInfo.sExtensionStrings = 
      MICSysInfo::getInstance().getSupportedOclExtensions( m_uiMicId );
  }

    return &MICDevInfo;
}

size_t MICDevice::clDevFEDeviceInfoSize() const
{
    return sizeof(MICDevInfo);
}

// Device Fission support
/****************************************************************************************************************
 clDevPartition
    Calculate appropriate affinity mask to support the partitioning mode and instantiate as many dedicated
    command lists objects as needed
********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevPartition(  cl_dev_partition_prop IN props, cl_uint IN num_requested_subdevices, cl_dev_subdevice_id IN parent_id,
                                            cl_uint* INOUT num_subdevices, void* param, cl_dev_subdevice_id* OUT subdevice_ids)
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    // TODO: Not implemented yet
    assert( false && "MIC clDevPartition not implemented yet" );
    return CL_DEV_NOT_SUPPORTED;
}

/****************************************************************************************************************
 clDevReleaseSubdevice
    Release a subdevice created by a clDevPartition call. Releases the appropriate command list object
********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevReleaseSubdevice(  cl_dev_subdevice_id IN subdevice_id)
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_SUCCESS;
    }
    
    // TODO: Not implemented yet
    assert( false && "MIC clDevReleaseSubdevice not implemented yet" );
    return CL_DEV_NOT_SUPPORTED;
}

/****************************************************************************************************************
 clDevCreateCommandList
    Call commandListFactory to create command list
********************************************************************************************************************/
cl_dev_err_code MICDevice::CreateCommandList( bool external_list, 
                                              cl_dev_cmd_list_props IN props, cl_dev_subdevice_id IN subdevice_id, cl_dev_cmd_list* OUT list)
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevCreateCommandList Function enter");
    CommandList* tCommandList;
    cl_dev_err_code ret = CommandList::commandListFactory(props, subdevice_id, 
                                                          m_pNotificationPort, m_pDeviceServiceComm, 
                                                          m_pFrameworkCallBacks, m_pProgramService, &m_overhead_data,
#ifdef USE_ITT
                                                          g_pGPAData,
#endif
                                                          &tCommandList);
    if (CL_DEV_FAILED(ret))
    {
        return ret;
    }
    if (external_list)
    {
      // TODO: Why we need this, list are handled by the runtime. We can assume no bugs.
        OclAutoMutex lock( &m_commandListsSetLock );
        m_commandListsSet.insert(tCommandList);
    }
    *list = (void*)tCommandList;
    return ret;
}

cl_dev_err_code MICDevice::clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_subdevice_id IN subdevice_id, cl_dev_cmd_list* OUT list)
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    return CreateCommandList( true, props, subdevice_id, list );
}

/****************************************************************************************************************
 clDevFlushCommandList
    flush command list
********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevFlushCommandList( cl_dev_cmd_list IN list)
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_SUCCESS;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevFlushCommandList Function enter");
    CommandList* pList = (CommandList*)list;
    if (NULL == pList)
    {
        return CL_DEV_INVALID_VALUE;
    }
    return pList->flushCommandList();
}
/****************************************************************************************************************
 clDevRetainCommandList
    retain command list
********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevRetainCommandList( cl_dev_cmd_list IN list)
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevRetainCommandList Function enter");
    CommandList* pList = (CommandList*)list;
    if (NULL == pList)
    {
        return CL_DEV_INVALID_VALUE;
    }
    return pList->retainCommandList();
}
/****************************************************************************************************************
 clDevReleaseCommandList
    release command list
********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevReleaseCommandList( cl_dev_cmd_list IN list )
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_SUCCESS;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevReleaseCommandList Function enter");
    CommandList* pList = (CommandList*)list;
    if (NULL == pList)
    {
        return CL_DEV_INVALID_VALUE;
    }
    bool removeObject = false;
    cl_dev_err_code res = pList->releaseCommandList(&removeObject);
    // If the reference counter = 0.
    if ((true == removeObject) && (CL_DEV_SUCCEEDED(res)))
    {
        OclAutoMutex lock( &m_commandListsSetLock );
        m_commandListsSet.erase(pList);
        delete(pList);
    }
    return res;
}
/****************************************************************************************************************
 clDevCommandListExecute
    execute given list of commands
********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count)
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevCommandListExecute Function enter");
    if (NULL != list)
    {
        CommandList* pList = (CommandList*)list;
        if (NULL == pList)
        {
            return CL_DEV_INVALID_VALUE;
        }
        return pList->commandListExecute(cmds,count);
    }
    else
    {
        // default list was requested for out-of-bound actions
        cl_dev_err_code err;
        
        if (NULL == m_defaultCommandList)
        {
            
            err = CreateCommandList( false, CL_DEV_LIST_ENABLE_OOO, 0, (cl_dev_cmd_list*)&m_defaultCommandList );
            if (CL_DEV_FAILED(err))
            {
                return err;
            }
        }
        
        err = m_defaultCommandList->commandListExecute(cmds,count);
        if (CL_DEV_FAILED(err))
        {
            return err;
        }

        return m_defaultCommandList->flushCommandList();
    }
}

/****************************************************************************************************************
 clDevCommandListExecute
    Call clDevCommandListWaitCompletion to add calling thread to execution pool
********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevCommandListWaitCompletion(cl_dev_cmd_list IN list, cl_dev_cmd_desc* cmdDesc)
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_NOT_SUPPORTED;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevCommandListWaitCompletion Function enter");

    if (NULL != list)
    {
        CommandList* pList = (CommandList*)list;
        if (NULL == pList)
        {
            return CL_DEV_INVALID_VALUE;
        }
        return pList->commandListWaitCompletion(cmdDesc);
    }
    return CL_DEV_NOT_SUPPORTED;
}

/****************************************************************************************************************
 clDevCommandListCancel
********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevCommandListCancel(cl_dev_cmd_list IN list)
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_NOT_SUPPORTED;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevCommandListCancel Function enter"));

    if (NULL != list)
    {
        CommandList* pList = (CommandList*)list;
        if (NULL == pList)
        {
            return CL_DEV_INVALID_VALUE;
        }
        return pList->cancelCommandList();
    }
    else
    {
        return CL_DEV_INVALID_VALUE;
    }
}

//! Release a command
    /*!
     * \param[in]   cmdToRelease the command to release
     */
void MICDevice::clDevReleaseCommand(cl_dev_cmd_desc* IN cmdToRelease)
{
    if (isDeviceLibraryUnloaded())
    {
        return;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevReleaseCommand Function enter");

    if (cmdToRelease)
    {
        Command* pCmd = (Command*)cmdToRelease->device_agent_data;

        cmdToRelease->device_agent_data = NULL;

        if (pCmd)
        {
            pCmd->releaseCommand();
        }
    }
}

//Memory API's
/****************************************************************************************************************
 clDevGetSupportedImageFormats
    Call Memory Allocator to get supported image formats
********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevGetSupportedImageFormats( cl_mem_flags IN flags, cl_mem_object_type IN imageType,
                                    cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet) const
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevGetSupportedImageFormats Function enter");
    return (cl_dev_err_code)m_pMemoryAllocator->GetSupportedImageFormats(flags, imageType,numEntries, formats, numEntriesRet);

}

cl_dev_err_code MICDevice::clDevGetMemoryAllocProperties( cl_mem_object_type IN memObjType, cl_dev_alloc_prop* OUT pAllocProp )
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevGetMemoryAllocProperties Function enter");
    return m_pMemoryAllocator->GetAllocProperties(memObjType, pAllocProp);
}

/****************************************************************************************************************
 clDevCreateMemoryObject
    Call Memory Allocator to create memory object
********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevCreateMemoryObject( cl_dev_subdevice_id IN node_id, cl_mem_flags IN flags,
                    const cl_image_format* IN format, size_t IN dim_count, const size_t* IN dim_size,
                    IOCLDevRTMemObjectService* IN pBSService, IOCLDevMemoryObject* OUT *pMemObj )
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevCreateMemoryObject Function enter");
    return m_pMemoryAllocator->CreateObject(node_id, flags, format, dim_count, dim_size, pBSService, pMemObj);
}

/****************************************************************************************************************
 clDevCheckProgramBinary
    Call Program Serice to check binaries
********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevCheckProgramBinary( size_t IN binSize, const void* IN bin )
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevCheckProgramBinary Function enter");
    return (cl_dev_err_code)m_pProgramService->CheckProgramBinary(binSize, bin );
}

/*******************************************************************************************************************
clDevCreateProgram
    Call programService to create program
**********************************************************************************************************************/

cl_dev_err_code MICDevice::clDevCreateProgram( size_t IN binSize, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog )
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevCreateProgram Function enter");
    return (cl_dev_err_code)m_pProgramService->CreateProgram(binSize, bin, prop, prog );
}

/*******************************************************************************************************************
clDevCreateProgram
    Call programService to create program
**********************************************************************************************************************/

cl_dev_err_code MICDevice::clDevCreateBuiltInKernelProgram(const char* szKernelNames, cl_dev_program* OUT prog)
{
    return CL_DEV_ERROR_FAIL;
}

/*******************************************************************************************************************
clDevBuildProgram
    Call programService to build program
**********************************************************************************************************************/

cl_dev_err_code MICDevice::clDevBuildProgram( cl_dev_program IN prog, const char* IN options, cl_build_status* OUT buildStatus )
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevBuildProgram Function enter");
    return (cl_dev_err_code)m_pProgramService->BuildProgram(prog, options, buildStatus);
}

/*******************************************************************************************************************
clDevReleaseProgram
    Call programService to release program
**********************************************************************************************************************/

cl_dev_err_code MICDevice::clDevReleaseProgram( cl_dev_program IN prog )
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_SUCCESS;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevReleaseProgram Function enter");
    return (cl_dev_err_code)m_pProgramService->ReleaseProgram( prog );
}

/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to unload the backend compiler
**********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevUnloadCompiler()
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_SUCCESS;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevUnloadCompiler Function enter");
    return (cl_dev_err_code)m_pProgramService->UnloadCompiler();
}
/*******************************************************************************************************************
clDevGetProgramBinary
    Call programService to get the program binary
**********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevGetProgramBinary( cl_dev_program IN prog, size_t IN size, void* OUT binary, size_t* OUT sizeRet )
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevGetProgramBinary Function enter");
    return (cl_dev_err_code)m_pProgramService->GetProgramBinary(prog, size, binary, sizeRet );
}
/*******************************************************************************************************************
clDevGetBuildLog
    Call programService to get the build log
**********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT sizeRet)
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevGetBuildLog Function enter");
    return (cl_dev_err_code)m_pProgramService->GetBuildLog(prog, size, log, sizeRet);
}
/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to get supported binary description
**********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevGetSupportedBinaries( size_t IN count, cl_prog_binary_desc* OUT types, size_t* OUT sizeRet )
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevGetSupportedBinaries Function enter");
    return (cl_dev_err_code)m_pProgramService->GetSupportedBinaries(count,types,sizeRet );
}
/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to get kernel id from its name
**********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId )
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevGetKernelId Function enter");
    return (cl_dev_err_code)m_pProgramService->GetKernelId(prog, name, kernelId );
}
/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to get kernels from the program
**********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN numKernels, cl_dev_kernel* OUT kernels,
                         cl_uint* OUT numKernelsRet )
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevGetProgramKernels Function enter");
    return (cl_dev_err_code)m_pProgramService->GetProgramKernels(prog, numKernels, kernels,numKernelsRet );
}
/*******************************************************************************************************************
clDevGetKernelInfo
    Call programService to get kernel info
**********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN valueSize,
                    void* OUT value, size_t* OUT valueSizeRet )
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clDevGetKernelInfo Function enter");
    return (cl_dev_err_code)m_pProgramService->GetKernelInfo(kernel, param, valueSize,value,valueSizeRet );
}

/*******************************************************************************************************************
clDevGetPerofrmanceCounter
    Get performance counter value
**********************************************************************************************************************/
cl_ulong MICDevice::clDevGetPerformanceCounter()
{
    return Intel::OpenCL::Utils::HostTime();
}

cl_dev_err_code MICDevice::clDevSetLogger(IOCLDevLogDescriptor *pLogDescriptor)
{
    if (isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    if ( NULL != m_pLogDescriptor )
    {
        m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
    }
    m_pLogDescriptor = pLogDescriptor;
    if ( NULL != m_pLogDescriptor )
    {
        cl_dev_err_code ret = (cl_dev_err_code)m_pLogDescriptor->clLogCreateClient(m_uiMicId, "MIC Device", &m_iLogHandle);
        if(CL_DEV_SUCCESS != ret)
        {
            return CL_DEV_ERROR_FAIL;
        }
    }
    return CL_DEV_SUCCESS;
}
/*******************************************************************************************************************
clDevCloseDevice
    Close device
**********************************************************************************************************************/
void MICDevice::clDevCloseDevice(void)
{
    if (! isDeviceLibraryUnloaded())
    {
        MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "clCloseDevice Function enter");
    }

    // remove Mic device from global set
    if (isDeviceLibraryUnloaded() || UnregisterMicDevice( this ))
    {
        // If the device didn't close yet.
        clDevCloseDeviceInt();
    }
}

void MICDevice::clDevCloseDeviceInt(bool preserve_object)
{
    // release notification port
    if (NULL != m_pNotificationPort)
    {
        m_pNotificationPort->release();
        m_pNotificationPort = NULL;
    }

    if (NULL != m_defaultCommandList)
    {
        clDevReleaseCommandList((cl_dev_cmd_list*)m_defaultCommandList); 
        m_defaultCommandList = NULL;
    }

    if ( 0 != m_iLogHandle)
    {
        m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
        m_iLogHandle = 0;
    }

    if ( NULL != m_pProgramService )
    {
        delete m_pProgramService;
        m_pProgramService = NULL;
    } 
    
    if ( NULL != m_pMemoryAllocator )
    {
        m_pMemoryAllocator->Release();
        m_pMemoryAllocator = NULL;
    }

    // delete commandList objects (If not released by the client)
    set<CommandList*>::iterator commandListsIter;
    for (commandListsIter = m_commandListsSet.begin(); commandListsIter != m_commandListsSet.end(); ++commandListsIter)
    {
        delete(*commandListsIter);
    }
    m_commandListsSet.clear();

    if (m_pDeviceServiceComm)
    {
        delete(m_pDeviceServiceComm);
        m_pDeviceServiceComm = NULL;
    }

    if (! preserve_object)
    {
        delete this;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Static functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! This function initializes device agent internal data. This function should be called prior to any device agent calls.
/*!
    \retval     CL_DEV_SUCCESS          If function is executed successfully.
    \retval     CL_DEV_ERROR_FAIL        If function failed to figure the IDs of the devices.
*/
extern "C" cl_dev_err_code clDevInitDeviceAgent(void)
{
    if (!MICDevice::loadingInit())
	{
		return CL_DEV_ERROR_FAIL;
	}

#ifdef __INCLUDE_MKL__
    Intel::OpenCL::MKLKernels::InitLibrary();
#endif
    return CL_DEV_SUCCESS;
}

