// Copyright (c) 2006-2007 Intel Corporation
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
//  MICDevice.cpp
///////////////////////////////////////////////////////////


#include "mic_device.h"
#include "program_service.h"
#include "mic_logger.h"
#include "cl_sys_info.h"
#include "mic_dev_limits.h"
#include "cl_sys_defines.h"
#include "buildversion.h"
#include "device_service_communication.h"
#include "memory_allocator.h"
#include "mic_sys_info.h"
#include "command_list.h"

using namespace Intel::OpenCL::MICDevice;

bool gSafeReleaseOfCoiObjects = true;

set<IOCLDeviceAgent*> MICDevice::m_mic_instancies;
OclMutex              MICDevice::m_mic_instancies_mutex;

char clMICDEVICE_CFG_PATH[MAX_PATH];

typedef struct _cl_dev_internal_cmd_list
{
    void*               cmd_list;
    cl_dev_subdevice_id subdevice_id;
} cl_dev_internal_cmd_list;

///////////////////////////////////////////////////////////////////////////////
//
// BEGIN MIC global management
//
///////////////////////////////////////////////////////////////////////////////

void MICDevice::RegisterMicDevice( MICDevice* dev )
{
    OclAutoMutex lock( &m_mic_instancies_mutex );
    m_mic_instancies.insert( dev );
}

void MICDevice::UnregisterMicDevice( MICDevice* dev )
{
		OclAutoMutex lock( &m_mic_instancies_mutex );
		m_mic_instancies.erase( dev );
}

MICDevice::TMicsSet MICDevice::GetActiveMicDevices( void )
{
    TMicsSet ret_list;

    OclAutoMutex lock( &m_mic_instancies_mutex );

    set<IOCLDeviceAgent*>::iterator it  = m_mic_instancies.end();
    set<IOCLDeviceAgent*>::iterator end = m_mic_instancies.end();

    for(; it != end; ++it)
    {
        ret_list.insert( (MICDevice*)*it );
    }

    return ret_list;
}

MICDevice::TMicsSet MICDevice::FilterMicDevices( size_t count, const IOCLDeviceAgent* const *dev_arr )
{
    TMicsSet ret_list;

    OclAutoMutex lock( &m_mic_instancies_mutex );

    set<IOCLDeviceAgent*>::iterator found_end = m_mic_instancies.end();

    for(size_t i = 0; i < count; ++i)
    {
        IOCLDeviceAgent* dev = const_cast<IOCLDeviceAgent*>(dev_arr[i]);

        if (m_mic_instancies.find( dev ) != found_end)
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

MICDevice::MICDevice(cl_uint uiDevId, cl_uint uiMicId, IOCLFrameworkCallbacks *devCallbacks, IOCLDevLogDescriptor *logDesc)
    : m_pMICDeviceConfig(NULL), m_pFrameworkCallBacks(devCallbacks), m_uiMicId(uiMicId),m_uiOclDevId(uiDevId),
    m_pLogDescriptor(logDesc), m_iLogHandle (0), m_pDeviceServiceComm(NULL)
{
}

cl_dev_err_code MICDevice::Init()
{
    if ( NULL != m_pLogDescriptor )
    {
        cl_dev_err_code ret = (cl_dev_err_code)m_pLogDescriptor->clLogCreateClient(m_uiOclDevId, L"MIC Device", &m_iLogHandle);
        if(CL_DEV_SUCCESS != ret)
        {
            return CL_DEV_ERROR_FAIL;
        }
    }

    m_pMICDeviceConfig = new MICDeviceConfig();
    m_pMICDeviceConfig->Initialize(clMICDEVICE_CFG_PATH);

    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("CreateDevice function enter"));

    // trying to upload next free device.
    cl_dev_err_code result = DeviceServiceCommunication::deviceSeviceCommunicationFactory(m_uiMicId, &m_pDeviceServiceComm);
    if (CL_DEV_FAILED(result))
    {
        return result;
    }
	assert(m_pDeviceServiceComm);

    // initialize the notificationPort mechanism.
    if (NotificationPort::SUCCESS != m_notificationPort.initialize(NOTIFICATION_PORT_MAX_BARRIERS))
    {
        return CL_DEV_ERROR_FAIL;
    }

    m_pProgramService = new ProgramService( m_uiOclDevId, m_pFrameworkCallBacks, m_pLogDescriptor,
                                           m_pMICDeviceConfig, *m_pDeviceServiceComm);

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

    m_pMemoryAllocator = MemoryAllocator::getMemoryAllocator( m_uiOclDevId, m_pLogDescriptor, MIC_MAX_BUFFER_ALLOC_SIZE(m_uiMicId) );

    if (NULL == m_pMemoryAllocator)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

// BUGBUG: DK - still OOO not supported
//    cl_dev_err_code ret = clDevCreateCommandList(CL_DEV_LIST_ENABLE_OOO, 0, &m_defaultCommandList);
//    if (CL_DEV_SUCCESS != ret)
//    {
//        return CL_DEV_ERROR_FAIL;
//    }

    // record Mic device in global set
    RegisterMicDevice( this );

    return CL_DEV_SUCCESS;
}

MICDevice::~MICDevice()
{
}

void MICDevice::loadingInit()
{
}

void MICDevice::unloadRelease()
{
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

    MICDevice *pNewDevice = new MICDevice(dev_id, 0, pDevCallBacks, pLogDesc);
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
	static const char* sFEModuleName = "clang_compiler";
	return sFEModuleName;
}

const void* MICDevice::clDevFEDeviceInfo() const
{
	return MICSysInfo::getInstance().getSupportedOclExtensions( m_uiMicId );
}

size_t MICDevice::clDevFEDeviceInfoSize() const
{
    const char* supported_extensions = (const char*)clDevFEDeviceInfo();

	return supported_extensions ? strlen(supported_extensions)+1 : 0;
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
    // TODO: Not implemented yet
    assert( false && "MIC clDevReleaseSubdevice not implemented yet" );
    return CL_DEV_NOT_SUPPORTED;
}

/****************************************************************************************************************
 clDevCreateCommandList
    Call commandListFactory to create command list
********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_subdevice_id IN subdevice_id, cl_dev_cmd_list* OUT list)
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCreateCommandList Function enter"));
    CommandList* tCommandList;
    cl_dev_err_code ret = CommandList::commandListFactory(props, subdevice_id, &m_notificationPort, m_pDeviceServiceComm, m_pFrameworkCallBacks, m_pProgramService, &tCommandList);
    if (CL_DEV_FAILED(ret))
    {
        return ret;
    }
    m_commandListsSet.insert(tCommandList);
    *list = (void*)tCommandList;
    return ret;
}
/****************************************************************************************************************
 clDevFlushCommandList
    flush command list
********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevFlushCommandList( cl_dev_cmd_list IN list)
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevFlushCommandList Function enter"));
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
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevRetainCommandList Function enter"));
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
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevReleaseCommandList Function enter"));
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
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCommandListExecute Function enter"));
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
        // TODO: Execute command without Command List? Immediately? NOT Implemented yet
        assert( false && "MIC: Execute command without Command List? Immediately? NOT Implemented yet" );
        return CL_DEV_NOT_SUPPORTED;
    }
}

/****************************************************************************************************************
 clDevCommandListExecute
    Call clDevCommandListWaitCompletion to add calling thread to execution pool
********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevCommandListWaitCompletion(cl_dev_cmd_list IN list)
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCommandListWaitCompletion Function enter"));

    // Nothing can be done as everything is run on the device side except of compilation.
    // Let the Runtime to wait
    return CL_DEV_NOT_SUPPORTED;
}

//Memory API's
/****************************************************************************************************************
 clDevGetSupportedImageFormats
    Call Memory Allocator to get supported image formats
********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevGetSupportedImageFormats( cl_mem_flags IN flags, cl_mem_object_type IN imageType,
                                    cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet)
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetSupportedImageFormats Function enter"));
    return (cl_dev_err_code)m_pMemoryAllocator->GetSupportedImageFormats(flags, imageType,numEntries, formats, numEntriesRet);

}
cl_dev_err_code MICDevice::clDevGetMemoryAllocProperties( cl_mem_object_type IN memObjType, cl_dev_alloc_prop* OUT pAllocProp )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetMemoryAllocProperties Function enter"));
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
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCreateMemoryObject Function enter"));
    return m_pMemoryAllocator->CreateObject(node_id, flags, format, dim_count, dim_size, pBSService, pMemObj);
}

/****************************************************************************************************************
 clDevCheckProgramBinary
    Call Program Serice to check binaries
********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevCheckProgramBinary( size_t IN binSize, const void* IN bin )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCheckProgramBinary Function enter"));
    return (cl_dev_err_code)m_pProgramService->CheckProgramBinary(binSize, bin );
}

/*******************************************************************************************************************
clDevCreateProgram
    Call programService to create program
**********************************************************************************************************************/

cl_dev_err_code MICDevice::clDevCreateProgram( size_t IN binSize, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCreateProgram Function enter"));
    return (cl_dev_err_code)m_pProgramService->CreateProgram(binSize, bin, prop, prog );
}

/*******************************************************************************************************************
clDevBuildProgram
    Call programService to build program
**********************************************************************************************************************/

cl_dev_err_code MICDevice::clDevBuildProgram( cl_dev_program IN prog, const char* IN options, void* IN userData )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevBuildProgram Function enter"));
    return (cl_dev_err_code)m_pProgramService->BuildProgram(prog, options, userData);
}

/*******************************************************************************************************************
clDevReleaseProgram
    Call programService to release program
**********************************************************************************************************************/

cl_dev_err_code MICDevice::clDevReleaseProgram( cl_dev_program IN prog )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevReleaseProgram Function enter"));
    return (cl_dev_err_code)m_pProgramService->ReleaseProgram( prog );
}

/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to unload the backend compiler
**********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevUnloadCompiler()
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevUnloadCompiler Function enter"));
    return (cl_dev_err_code)m_pProgramService->UnloadCompiler();
}
/*******************************************************************************************************************
clDevGetProgramBinary
    Call programService to get the program binary
**********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevGetProgramBinary( cl_dev_program IN prog, size_t IN size, void* OUT binary, size_t* OUT sizeRet )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetProgramBinary Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetProgramBinary(prog, size, binary, sizeRet );
}
/*******************************************************************************************************************
clDevGetBuildLog
    Call programService to get the build log
**********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT sizeRet)
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetBuildLog Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetBuildLog(prog, size, log, sizeRet);
}
/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to get supported binary description
**********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevGetSupportedBinaries( size_t IN count, cl_prog_binary_desc* OUT types, size_t* OUT sizeRet )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetSupportedBinaries Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetSupportedBinaries(count,types,sizeRet );
}
/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to get kernel id from its name
**********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetKernelId Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetKernelId(prog, name, kernelId );
}
/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to get kernels from the program
**********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN numKernels, cl_dev_kernel* OUT kernels,
                         cl_uint* OUT numKernelsRet )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetProgramKernels Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetProgramKernels(prog, numKernels, kernels,numKernelsRet );
}
/*******************************************************************************************************************
clDevGetKernelInfo
    Call programService to get kernel info
**********************************************************************************************************************/
cl_dev_err_code MICDevice::clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN valueSize,
                    void* OUT value, size_t* OUT valueSizeRet )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetKernelInfo Function enter"));
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

    if ( NULL != m_pLogDescriptor )
    {
        m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
    }
    m_pLogDescriptor = pLogDescriptor;
    if ( NULL != m_pLogDescriptor )
    {
        cl_dev_err_code ret = (cl_dev_err_code)m_pLogDescriptor->clLogCreateClient(m_uiOclDevId, L"MIC Device", &m_iLogHandle);
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
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clCloseDevice Function enter"));

    // remove Mic device from global set
    UnregisterMicDevice( this );

	/* TODO remove the comment from the next command when m_defaultCommandList will initialize in Init() method
    clDevReleaseCommandList(m_defaultCommandList); */
    if( NULL != m_pMICDeviceConfig)
    {
        delete m_pMICDeviceConfig;
        m_pMICDeviceConfig = NULL;
    }
    if ( 0 != m_iLogHandle)
    {
        m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
    }
    /* TODO Comment out the this comment when "ReleaseBackendServices()" will implemented - Comment it because it crash the test due to assertion
    if ( NULL != m_pProgramService )
    {
        delete m_pProgramService;
        m_pProgramService = NULL;
    } */
    if ( NULL != m_pMemoryAllocator )
    {
        m_pMemoryAllocator->Release();
        m_pMemoryAllocator = NULL;
    }

    // delete commandList objects (If not released by the client)
    set<CommandList*> ::iterator commandListsIter;
    for (commandListsIter = m_commandListsSet.begin(); commandListsIter != m_commandListsSet.end(); ++commandListsIter)
    {
        delete(*commandListsIter);
    }
    m_commandListsSet.clear();

    // release notification port
    m_notificationPort.release();

    if (m_pDeviceServiceComm)
    {
        delete(m_pDeviceServiceComm);
        m_pDeviceServiceComm = NULL;
    }

    delete this;
}

