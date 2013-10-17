///////////////////////////////////////////////////////////
//  ContextModule.cpp
//  Implementation of the Class ContextModule
//  Created on:      10-Dec-2008 2:03:03 PM
//  Original author: Uri Levy
///////////////////////////////////////////////////////////

#include "framework_proxy.h"
#include "context_module.h"
#include "Context.hpp"
#include "program.h"
#include "kernel.h"
#include "sampler.h"
#include "MemoryAllocator/MemoryObject.h"
#include "ocl_itt.h"
#if defined (_WIN32)
#include "gl_context.h"
#include "gl_shr_utils.h"
#include "gl_mem_objects.h"
#if defined (DX_MEDIA_SHARING)
#include "d3d9_sharing.h"
#include "d3d9_context.hpp"
#endif
#endif
#include <platform_module.h>
#include <Device.h>
#include <cl_objects_map.h>
#include <cl_utils.h>
#include <assert.h>
#include <set>
#include <algorithm>
#include "GenericMemObj.h"
#include "ImageBuffer.h"
#include "cl_shared_ptr.hpp"
#include "command_queue.h"
#include "events_manager.h"
#include "user_event.h"
#include "cl_sys_info.h"
#include "svm_buffer.h"
#include "pipe.h"

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

//////////////////////////////////////////////////////////////////////////
// ContextModule C'tor
//////////////////////////////////////////////////////////////////////////
ContextModule::ContextModule(PlatformModule *pPlatformModule) : m_bIsTerminating(false)
{
    INIT_LOGGER_CLIENT(TEXT("ContextModule"),LL_DEBUG);

    LOG_INFO(TEXT("%s"), TEXT("ContextModule constructor enter"));

    m_pPlatformModule = pPlatformModule;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule D'tor
//////////////////////////////////////////////////////////////////////////
ContextModule::~ContextModule()
{
    LOG_INFO(TEXT("%s"), TEXT("ContextModule destructor enter"));

    RELEASE_LOGGER_CLIENT;
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::Initialize
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::Initialize(ocl_entry_points * pOclEntryPoints, ocl_gpa_data * pGPAData)
{
    LOG_INFO(TEXT("%s"), TEXT("ContextModule::Initialize enter"));

    m_pOclEntryPoints = pOclEntryPoints;
    m_pGPAData = pGPAData;
    
    return CL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::Release
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::Release(  bool bTerminate )
{
    m_bIsTerminating = bTerminate;
    LOG_INFO(TEXT("%s"), TEXT("ContextModule::Release enter"));

    return CL_SUCCESS;
}

 /******************************************************************
 * 
 * Forcibly shutdown all contextes 
 *
 ******************************************************************/
typedef std::list<SharedPtr<OclCommandQueue> >  QueueListType;
void ContextModule::ShutDown(bool wait_for_finish)
{
    QueueListType           queue_list;
    QueueListType::iterator queue_list_it;
    QueueListType::iterator queue_list_it_end;

    FrameworkProxy* framework_proxy     = FrameworkProxy::Instance();
    ExecutionModule*  execution_module  = framework_proxy->GetExecutionModule();
    EventsManager* eventsManager        = execution_module->GetEventsManager();

    // 1. Cancel all build tasks
    framework_proxy->CancelAllTasks(wait_for_finish);

    // 2. Delete all user-accessible queues. If queue already holds some command it will not be deleted.
    framework_proxy->GetExecutionModule()->DeleteAllActiveQueues(true);

    // 3. Loop though all existing queues and switch them to a cancel state
    m_setQueues.getObjects( queue_list );
    queue_list_it_end = queue_list.end();

    for ( queue_list_it = queue_list.begin(); queue_list_it != queue_list_it_end; ++queue_list_it)
    {
        SharedPtr<OclCommandQueue> pQueue = (*queue_list_it);
        pQueue->CancelAll();
    }

    // 4. Signal all non-completed user events to push queues forward
    //    Release all non-released user events
    execution_module->ReleaseAllUserEvents(true);
    
    // 5. clFinish() of all queueus
    if (wait_for_finish)
    {
        for ( queue_list_it = queue_list.begin(); queue_list_it != queue_list_it_end; ++queue_list_it)
        {
            SharedPtr<OclCommandQueue> pQueue = (*queue_list_it);
			if (pQueue.DynamicCast<IOclCommandQueueBase>() != NULL)
			{
				execution_module->Finish(pQueue.DynamicCast<IOclCommandQueueBase>());
			}
        }
    }

    queue_list.clear();

    // 6. Emulate Release of all objects maintained by user
    RemoveAllMemObjects(true);
    RemoveAllSamplers(true);
    RemoveAllKernels(true);
    RemoveAllPrograms(true);

    eventsManager->DisableNewEvents();
    m_mapContexts.  DisableAdding();

    eventsManager->ReleaseAllEvents(false);

    m_mapContexts.SetPreserveUserHandles();
    m_mapContexts.ReleaseAllObjects(false);

    m_pPlatformModule->RemoveAllDevices(true);
    m_pPlatformModule->ReleaseFECompilers(false);

    // 7. Ensure that all devices really closed
#ifdef _DEBUG
    const unsigned long long TIMEOUT = 100 * 1000000000LL; // 100 sec
    const unsigned long long endTime = HostTime() + TIMEOUT;
    while ( 0 < m_pPlatformModule->GetActiveDeviceCount())
    {
        if (HostTime() > endTime)
        {
            DumpSharedPts("ContextModule::ShutDown - Device Agents cannot be closed, time out. Only SharedPtrs local to intelocl DLL", true);    
            break;
        }
    }
#else
    m_pPlatformModule->WaitForAllDevices();
#endif
	
    // At that point still some internal threads in different DLLs may handle SharedPtr's destruction
    // We need to wait until all of them will end their work.
    // We will do this in the TerminateProcess() function by calling shutdown callbacks of all DLLs to ensure their
    // full shutdown
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateContext
//////////////////////////////////////////////////////////////////////////
cl_context    ContextModule::CreateContext(const cl_context_properties * clProperties,
                                         cl_uint uiNumDevices,
                                         const cl_device_id *pDevices,
                                         logging_fn pfnNotify,
                                         void *pUserData,
                                         cl_err_code *pRrrcodeRet)
{
    //cl_start;

    LOG_INFO(TEXT("Enter ContextModule::CreateContext (clProperties=%d, uiNumDevices=%d, pDevices=%d)"), 
        clProperties, uiNumDevices, pDevices);
    
    if (NULL == pDevices)
    {
        LOG_ERROR(TEXT("%s"), TEXT("(!pDevices); return CL_INVALID_VALUE"));
        if (NULL != pRrrcodeRet)
        {    
            *pRrrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }

    if (!pfnNotify && pUserData)
    {
        LOG_ERROR(TEXT("%s"), TEXT("(!pfnNotify && pUserData); return CL_INVALID_VALUE"));
        if (NULL != pRrrcodeRet)
        {    
            *pRrrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }

    if (NULL != pRrrcodeRet)
    {
        *pRrrcodeRet = CL_SUCCESS;
    }

    SharedPtr<FissionableDevice>* ppDevices = new SharedPtr<FissionableDevice>[uiNumDevices];
    if (NULL == ppDevices)
    {
        LOG_ERROR(TEXT("%s"), TEXT("Failed to allocate memory for devices: new Device[uiNumDevices] = NULL"));
        if (NULL != pRrrcodeRet)
        {    
            *pRrrcodeRet = CL_OUT_OF_HOST_MEMORY;
        }
        return CL_INVALID_HANDLE;
    }

    cl_err_code clErrRet = GetDevices(uiNumDevices, pDevices, ppDevices);
    if (CL_FAILED(clErrRet))
    {
        delete[] ppDevices;
        if (NULL != pRrrcodeRet)
        {                
            *pRrrcodeRet = CL_INVALID_DEVICE;
        }
        return CL_INVALID_HANDLE;
    }

    cl_uint numRootDevices = 0;
    for (cl_uint i = 0; i < uiNumDevices; ++i)
    {
        if (ppDevices[i]->IsRootLevelDevice())
        {
            ++numRootDevices;
        }
    }

    SharedPtr<Context> pContext;
    // check properties
    std::map<cl_context_properties, cl_context_properties> propertyMap;
    if (NULL != clProperties)
    {
        size_t i = 0;

        while (0 != clProperties[i])
        {
            if (CL_CONTEXT_PLATFORM == clProperties[i] &&
                !m_pPlatformModule->CheckPlatformId((cl_platform_id)clProperties[i + 1]))
            {
                LOG_ERROR(TEXT("%s"), TEXT("platform value specified in properties is not a valid platform"));
                delete[] ppDevices;
                if (NULL != pRrrcodeRet)
                {
                    *pRrrcodeRet = CL_INVALID_PLATFORM;
                }
                return CL_INVALID_HANDLE;
            }
            if (propertyMap.find(clProperties[i]) != propertyMap.end())
            {
                LOG_ERROR(TEXT("%s"), TEXT("the same property name is specified more than once"));
                delete[] ppDevices;
                if (NULL != pRrrcodeRet)
                {
                    *pRrrcodeRet = CL_INVALID_PROPERTY;
                }
                return CL_INVALID_HANDLE;
            }
            
            static const cl_context_properties legalProperties[] = {
                CL_CONTEXT_PLATFORM,
                CL_CONTEXT_INTEROP_USER_SYNC
#if defined (_WIN32)
                ,
                CL_GL_CONTEXT_KHR,
                CL_WGL_HDC_KHR
#if defined (DX_MEDIA_SHARING)
                ,
                CL_CONTEXT_D3D9_DEVICE_INTEL,
                CL_CONTEXT_D3D9EX_DEVICE_INTEL,
                CL_CONTEXT_DXVA_DEVICE_INTEL,
                CL_CONTEXT_ADAPTER_D3D9_KHR,
                CL_CONTEXT_ADAPTER_D3D9EX_KHR,
                CL_CONTEXT_ADAPTER_DXVA_KHR,
                CL_CONTEXT_D3D11_DEVICE_KHR
#endif
#endif
            };
            const cl_context_properties* const pEndLegalProperties = &legalProperties[sizeof(legalProperties)/sizeof(legalProperties[0])];
            if (std::find(legalProperties, pEndLegalProperties, clProperties[i]) == pEndLegalProperties)
            {
                LOG_ERROR(TEXT("%s"), TEXT("context property name in properties is not a supported property name"));
                delete[] ppDevices;
                if (NULL != pRrrcodeRet)
                {
                    *pRrrcodeRet = CL_INVALID_PROPERTY;
                }
                return CL_INVALID_HANDLE;
            }        
            propertyMap[clProperties[i]] = clProperties[i + 1];
            i += 2;
        }
    }
    
#if defined (_WIN32)  //TODO GL support for Linux
    cl_context_properties hGLCtx, hDC;
    bool bGLSharingSupported = false;
    ParseGLContextOptions(clProperties, &hGLCtx, &hDC, &bGLSharingSupported);
#if defined (DX_MEDIA_SHARING)
    IUnknown* pD3DDevice;
    cl_context_properties iDevType;
    const ID3DSharingDefinitions* pd3dDefinitions = NULL;
    clErrRet = ParseD3DContextOptions(propertyMap, pD3DDevice, iDevType, pd3dDefinitions);
    if (CL_SUCCESS != clErrRet)
    {
        assert(NULL == pd3dDefinitions);
        if (NULL != pRrrcodeRet)
        {
            *pRrrcodeRet = clErrRet;
        }
        delete[] ppDevices;
        return CL_INVALID_HANDLE;
    }
    if (NULL != pD3DDevice && bGLSharingSupported)
    {
        LOG_ERROR(TEXT("%s"), TEXT("CL_INVALID_D3D9_DEVICE_INTEL is set to a non-NULL value and interoperability with OpenGL is also specified."));
        if (NULL != pRrrcodeRet)
        {
            *pRrrcodeRet = CL_INVALID_OPERATION;
        }
        delete[] ppDevices;
        delete pd3dDefinitions;
        return CL_INVALID_HANDLE;
    }
#endif
#endif
    // Default error in case new() will fail
    clErrRet = CL_OUT_OF_HOST_MEMORY;
#if defined (_WIN32)  //TODO GL support for Linux
    if (bGLSharingSupported)
    {
        pContext =     GLContext::Allocate(clProperties, uiNumDevices, numRootDevices, ppDevices, pfnNotify, pUserData, &clErrRet, m_pOclEntryPoints, hGLCtx, hDC, m_pGPAData, *this);
    } else
#endif
#if defined (DX_MEDIA_SHARING)
    if (NULL != pD3DDevice)
    {
        const bool bIsInteropUserSync = propertyMap.find(CL_CONTEXT_INTEROP_USER_SYNC) == propertyMap.end() ? false :
            pd3dDefinitions->IsUsingContextInteropUserSync() && (cl_bool)propertyMap[CL_CONTEXT_INTEROP_USER_SYNC] != 0;
        if (pd3dDefinitions->GetVersion() == ID3DSharingDefinitions::D3D11)
        {
            pContext = D3D11Context::Allocate(clProperties, uiNumDevices, numRootDevices, ppDevices, pfnNotify, pUserData, &clErrRet, m_pOclEntryPoints, m_pGPAData, pD3DDevice,
                iDevType, pd3dDefinitions, *this, bIsInteropUserSync);
        }
        else
        {
            pContext = D3D9Context::Allocate(clProperties, uiNumDevices, numRootDevices, ppDevices, pfnNotify, pUserData, &clErrRet, m_pOclEntryPoints,
                m_pGPAData, pD3DDevice, iDevType, pd3dDefinitions, *this, bIsInteropUserSync);
        }
    }
    else
#endif
    {
        pContext =     Context::Allocate(clProperties, uiNumDevices, numRootDevices, ppDevices, pfnNotify, pUserData, &clErrRet, m_pOclEntryPoints, m_pGPAData, *this);
    }
    if (CL_FAILED(clErrRet))
    {
        LOG_ERROR(TEXT("%s"), TEXT("Create context failed"));
        if (NULL != pRrrcodeRet)
        {    
            *pRrrcodeRet = clErrRet;
        }
        delete[] ppDevices;
        if ( NULL != pContext )
        {
            pContext->Release();
        }
        return CL_INVALID_HANDLE;
    }

    delete[] ppDevices;
    
    cl_context clContextId = (cl_context)m_mapContexts.AddObject(pContext);
    LOG_INFO(TEXT("CONTEXT_TEST: New context created. (id = %d)"), clContextId);

    return clContextId;
}
cl_context ContextModule::CreateContextFromType(const cl_context_properties * clProperties, 
                                                cl_device_type clDeviceType, 
                                                logging_fn pfnNotify, 
                                                void * pUserData, 
                                                cl_int * pErrcodeRet)
{
    //cl_start;
    LOG_INFO(TEXT("Enter ContextModule::CreateContextFromType (clProperties=%d, clDeviceType=%d, pfnNotify=%d, pUserData=%d, pErrcodeRet=%d)"),
        clProperties, clDeviceType, pfnNotify, pUserData, pErrcodeRet);
    
#ifdef _DEBUG
    assert (NULL != m_pPlatformModule);
#endif
    cl_uint uiNumDevices = 0;
    
    // TODO: Handle new spec
    cl_err_code clErrRet = m_pPlatformModule->GetDeviceIDs(NULL, clDeviceType, 0, NULL, &uiNumDevices);
    if (CL_FAILED(clErrRet))
    {
        LOG_ERROR(TEXT("GetDeviceIDs(%d, 0, NULL, %d) = %s"), clDeviceType, &uiNumDevices, ClErrTxt(clErrRet));
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = clErrRet;
        }
        return CL_INVALID_HANDLE;
    }

    cl_device_id * pDevices = new cl_device_id[uiNumDevices];
    if (NULL == pDevices)
    {
        LOG_ERROR(TEXT("new cl_device_id[%d] = NULL"), uiNumDevices);
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_OUT_OF_HOST_MEMORY;
        }
        return CL_INVALID_HANDLE;
    }

    // TODO: Handle new spec
    clErrRet = m_pPlatformModule->GetDeviceIDs(NULL, clDeviceType, uiNumDevices, pDevices, NULL);
    if (CL_FAILED(clErrRet))
    {
        LOG_ERROR(TEXT("GetDeviceIDs(%d, %d, %d, NULL) = %s"), clDeviceType, uiNumDevices, pDevices, ClErrTxt(clErrRet));
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = clErrRet;
        }
        delete[] pDevices;
        return CL_INVALID_HANDLE;
    }
    cl_context clContext = CreateContext(clProperties, uiNumDevices, pDevices, pfnNotify, pUserData, pErrcodeRet);
    delete[] pDevices;
    //cl_return clContext;
    return clContext;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CheckDevices
//////////////////////////////////////////////////////////////////////////

cl_err_code ContextModule::GetRootDevices(cl_uint uiNumDevices, const cl_device_id *pclDeviceIds, SharedPtr<Device>* ppDevices)
{
    LOG_DEBUG(TEXT("ContextModule::GetRootDevices enter. uiNumDevices=%d, pclDeviceIds=%d, ppDevices=%d"), uiNumDevices, pclDeviceIds, ppDevices);
#ifdef _DEBUG
    assert ( NULL != m_pPlatformModule );
    assert ( (NULL != ppDevices) && (0 != uiNumDevices) );
#endif

    cl_uint rootId = 0;
    // go through device ids and get the device from the platform module
    
    for (cl_uint ui=0; ui<uiNumDevices; ++ui)
    {
        SharedPtr<FissionableDevice> pDevice = m_pPlatformModule->GetDevice(pclDeviceIds[ui]);
        if (NULL == pDevice)
        {
            LOG_ERROR(TEXT("m_pPlatformModule->GetDevice(%d) = NULL"), pclDeviceIds[ui]);
            return CL_ERR_KEY_NOT_FOUND;
        }
        if (pDevice->IsRootLevelDevice())
        {
            ppDevices[rootId++] = pDevice->GetRootDevice();
        }
    }
    return CL_SUCCESS;
}

cl_err_code ContextModule::GetDevices(cl_uint uiNumDevices, const cl_device_id *pclDeviceIds, SharedPtr<FissionableDevice>* ppDevices)
{
    LOG_DEBUG(TEXT("ContextModule::GetRootDevices enter. uiNumDevices=%d, pclDeviceIds=%d, ppDevices=%d"), uiNumDevices, pclDeviceIds, ppDevices);
#ifdef _DEBUG
    assert ( NULL != m_pPlatformModule );
    assert ( (NULL != ppDevices) && (0 != uiNumDevices) );
#endif

    // go through device ids and get the device from the platform module
    
    for (cl_uint ui=0; ui<uiNumDevices; ++ui)
    {
        SharedPtr<FissionableDevice> pDevice = m_pPlatformModule->GetDevice(pclDeviceIds[ui]);
        if (NULL == pDevice)
        {
            LOG_ERROR(TEXT("m_pPlatformModule->GetDevice(%d) = NULL"), pclDeviceIds[ui]);
            return CL_ERR_KEY_NOT_FOUND;
        }
        ppDevices[ui] = pDevice;
    }
    return CL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainContext
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::RetainContext(cl_context context)
{
    LOG_INFO(TEXT("ContextModule::RetainContext enter. context=%d"), context);
    cl_err_code clErrRet = CL_SUCCESS;
    SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)context).DynamicCast<Context>();
    if (NULL == pContext)
    {
        LOG_ERROR(TEXT("m_mapContexts.GetOCLObject(%d, %d) = %d"), context, &pContext, clErrRet);
        return CL_INVALID_CONTEXT;
    }
    return pContext->Retain();
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseContext
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::ReleaseContext(cl_context context)
{
    LOG_INFO(TEXT("ContextModule::ReleaseContext enter. context=%d"), context);    

    SharedPtr<OCLObject<_cl_context_int> > pContext = m_mapContexts.GetOCLObject((_cl_context_int*)context);
    if (pContext.GetRefCnt() > 2)
    {
        LOG_INFO(TEXT("Warning: context %d will not have been deleted after this call - the user might have forgotten to release some objects"), context);
    }

    cl_err_code err = m_mapContexts.ReleaseObject((_cl_context_int*)context);
    return ((CL_ERR_KEY_NOT_FOUND == err) ? CL_INVALID_CONTEXT : err);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::GetContextInfo
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::GetContextInfo(cl_context      context,
                                          cl_context_info param_name,
                                          size_t          param_value_size,
                                          void *          param_value,
                                          size_t *        param_value_size_ret)
{
    LOG_DEBUG(TEXT("ContextModule::GetContextInfo enter. context=%d, param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d"), 
        context, param_name, param_value_size, param_value, param_value_size_ret);

    cl_err_code clErrRet = CL_SUCCESS;
    SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)context).DynamicCast<Context>();
    if (NULL == pContext)
    {
        LOG_ERROR(TEXT("m_mapContexts.GetOCLObject(%d, %d) = %d"), context, &pContext, clErrRet);
        return CL_INVALID_CONTEXT;
    }
    clErrRet = pContext->GetInfo((cl_int)param_name, param_value_size, param_value, param_value_size_ret);
    if (CL_FAILED(clErrRet))
    {
        pContext->NotifyError("clGetContextInfo failed", &clErrRet, sizeof(cl_int));
    }
    return clErrRet;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateProgramWithSource
//////////////////////////////////////////////////////////////////////////
cl_program ContextModule::CreateProgramWithSource(cl_context     clContext,
                                                  cl_uint        uiCount, 
                                                  const char **  ppcStrings, 
                                                  const size_t * szLengths, 
                                                  cl_int *       pErrcodeRet)
{
    LOG_INFO(TEXT("CreateProgramWithSource enter. clContext=%d, uiCount=%d, ppcStrings=%d, szLengths=%d, pErrcodeRet=%d"), 
        clContext, uiCount, ppcStrings, szLengths, pErrcodeRet);

    cl_err_code clErrRet = CL_SUCCESS;
    // get the context from the contexts map list
    SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)clContext).DynamicCast<Context>();
    if (NULL == pContext)
    {
        LOG_ERROR(TEXT("m_mapContexts.GetOCLObject(%d, %d) = %d"), clContext, &pContext, clErrRet);
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }
    SharedPtr<Program> pProgram;
    clErrRet = pContext->CreateProgramWithSource(uiCount, ppcStrings, szLengths, &pProgram);
    if (CL_FAILED(clErrRet))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = clErrRet;
        }
        pContext->NotifyError("clCreateProgramWithSource failed", &clErrRet, sizeof(cl_int));
        return CL_INVALID_HANDLE;
    }
    clErrRet = m_mapPrograms.AddObject(pProgram, false);
    if (CL_FAILED(clErrRet))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = clErrRet;
        }
        pContext->NotifyError("clCreateProgramWithSource failed", &clErrRet, sizeof(cl_int));
        return CL_INVALID_HANDLE;
    }
    if (NULL != pErrcodeRet)
    {
        *pErrcodeRet = CL_SUCCESS;
    }
    return pProgram->GetHandle();
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateProgramWithBinary
//////////////////////////////////////////////////////////////////////////
cl_program ContextModule::CreateProgramWithBinary(cl_context                clContext,
                                                  cl_uint                    uiNumDevices,
                                                  const cl_device_id *        pclDeviceList,
                                                  const size_t *            pszLengths,
                                                  const unsigned char **    ppBinaries,
                                                  cl_int *                    piBinaryStatus,
                                                  cl_int *                    pErrRet)
{
    LOG_INFO(TEXT("CreateProgramWithBinary enter. clContext=%d, uiNumDevices=%d, pclDeviceList=%d, pszLengths=%d, ppBinaries=%d, piBinaryStatus=%d"), 
        clContext, uiNumDevices, pclDeviceList, pszLengths, ppBinaries, piBinaryStatus);
    if (NULL == pclDeviceList || 0 == uiNumDevices || NULL == pszLengths || NULL == ppBinaries)
    {
        // invalid value
        LOG_ERROR(TEXT("%s"), TEXT("NULL == pclDeviceList || 0 == uiNumDevices || NULL == pszLengths || NULL == ppBinaries"));
        if (NULL != pErrRet)
        {
            *pErrRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }
    // get the context from the contexts map list
    SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)clContext).DynamicCast<Context>();
    if (NULL == pContext)
    {
        LOG_ERROR(TEXT("m_mapContexts.GetOCLObject(%d, %d) = %d"), clContext, &pContext);
        if (NULL != pErrRet)
        {
            *pErrRet = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }
    SharedPtr<Program> pProgram;
    cl_err_code clErrRet = pContext->CreateProgramWithBinary(uiNumDevices, pclDeviceList, pszLengths, ppBinaries, piBinaryStatus, &pProgram);
    if (CL_FAILED(clErrRet))
    {
        if (NULL != pErrRet)
        {
            *pErrRet = clErrRet;
        }
        pContext->NotifyError("clCreateProgramWithBinary failed", &clErrRet, sizeof(cl_int));
        if (pProgram)
        {
            pProgram->Release();
        }        
        return CL_INVALID_HANDLE;
    }
    clErrRet = m_mapPrograms.AddObject(pProgram, false);
    if (CL_FAILED(clErrRet))
    {
        if (NULL != pErrRet)
        {
            *pErrRet = CL_OUT_OF_HOST_MEMORY;
        }
        pContext->NotifyError("clCreateProgramWithBinary failed", &clErrRet, sizeof(cl_int));
        pProgram->Release();
        return CL_INVALID_HANDLE;
    }
    if (NULL != pErrRet)
    {
        *pErrRet = CL_SUCCESS;
    }
    return pProgram->GetHandle();
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateProgramWithBuiltInKernels
//////////////////////////////////////////////////////////////////////////
cl_program ContextModule::CreateProgramWithBuiltInKernels(cl_context clContext,
        cl_uint uiNumDevices,
        const cl_device_id *  pclDeviceList,
        const char *szKernelNames,
        cl_int *pErrcodeRet)
{
    LOG_INFO(TEXT("CreateProgramWithBinary enter. clContext=%d, uiNumDevices=%d"), clContext, uiNumDevices);
    if (NULL == pclDeviceList || 0 == uiNumDevices || NULL == szKernelNames)
    {
        // invalid value
        LOG_ERROR(TEXT("%S"), TEXT("NULL == pclDeviceList || 0 == uiNumDevices || NULL == szKernelNames"));
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }
    // get the context from the contexts map list
    SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)clContext).DynamicCast<Context>();
    if (NULL == pContext)
    {
        LOG_ERROR(TEXT("m_mapContexts.GetOCLObject(%d) = NULL"), clContext);
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }
    SharedPtr<Program> pProgram = NULL;
    cl_err_code clErrRet = pContext->CreateProgramWithBuiltInKernels(uiNumDevices, pclDeviceList, szKernelNames, &pProgram);
    if (CL_FAILED(clErrRet))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = clErrRet;
        }
        pContext->NotifyError("CreateProgramWithBuiltInKernels failed", &clErrRet, sizeof(cl_int));
        if (pProgram)
        {
            pProgram->Release();
        }        
        return CL_INVALID_HANDLE;
    }
    clErrRet = m_mapPrograms.AddObject(pProgram, false);
    if (CL_FAILED(clErrRet))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_OUT_OF_HOST_MEMORY;
        }
        pContext->NotifyError("CreateProgramWithBuiltInKernels failed", &clErrRet, sizeof(cl_int));
        pProgram->Release();
        return CL_INVALID_HANDLE;
    }
    if (NULL != pErrcodeRet)
    {
        *pErrcodeRet = CL_SUCCESS;
    }
    return pProgram->GetHandle();
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainProgram
//////////////////////////////////////////////////////////////////////////
cl_err_code    ContextModule::RetainProgram(cl_program clProgram)
{
    LOG_INFO(TEXT("Enter RetainProgram (clProgram=%d)"), clProgram);
    
    SharedPtr<Program> pProgram = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram).DynamicCast<Program>();
    if (NULL == pProgram)
    {
        LOG_ERROR(TEXT("program %d is invalid program"), clProgram);
        return CL_INVALID_PROGRAM;
    }
    return pProgram->Retain();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseProgram
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::ReleaseProgram(cl_program clProgram)
{
    LOG_INFO(TEXT("Enter ReleaseProgram (clProgram=%d)"), clProgram);

    SharedPtr<Program> pProgram = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram).DynamicCast<Program>();
    if (NULL == pProgram)
    {
        LOG_ERROR(TEXT("program %d is invalid program"), clProgram);
        return CL_INVALID_PROGRAM;
    }
    SharedPtr<Context> pContext = pProgram->GetContext();
    if (NULL == pContext)
    {
        return CL_INVALID_PROGRAM;
    }

    long newRef = pProgram->Release();
    if (newRef < 0)
    {
        return CL_INVALID_PROGRAM;
    }
    else if (0 == newRef)
    {
        cl_err_code clErrRet = pContext->RemoveProgram(clProgram);
        if (CL_FAILED(clErrRet))
        {
            return CL_ERR_OUT(clErrRet);
        }

        clErrRet = m_mapPrograms.RemoveObject((_cl_program_int*)clProgram);
        if (CL_FAILED(clErrRet))
        {
            return CL_ERR_OUT(clErrRet);
        }
    }
    return CL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
// RemoveAllPrograms
//////////////////////////////////////////////////////////////////////////
void ContextModule::RemoveAllPrograms( bool preserve_user_handles )
{
    m_mapPrograms.DisableAdding();
    std::list<_cl_program_int*> to_remove;
    for (cl_uint ui=0; ui<m_mapPrograms.Count(); ++ui)
    {
        SharedPtr<Program> obj = m_mapPrograms.GetObjectByIndex(ui).DynamicCast<Program>();
        SharedPtr<Context> pContext = (NULL != obj) ? obj->GetContext() : SharedPtr<Context>(NULL);

        if (NULL != pContext)
        {
            cl_program handle = obj->GetHandle();
            pContext->RemoveProgram(handle);
            to_remove.push_back( (_cl_program_int*)handle );
        }
    }

    if (preserve_user_handles)
    {
        m_mapPrograms.SetPreserveUserHandles();
    }

    for (std::list<_cl_program_int*>::iterator it = to_remove.begin(); it != to_remove.end(); ++it)
    {
        m_mapPrograms.RemoveObject( *it );
    }

    m_mapPrograms.ReleaseAllObjects(false);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CompileProgram
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::CompileProgram(cl_program clProgram, 
                                     cl_uint uiNumDevices, 
                                     const cl_device_id * pclDeviceList, 
                                     const char * pcOptions, 
                                     cl_uint num_input_headers, 
                                     const cl_program* pclInputHeaders, 
                                     const char **header_include_names, 
                                     void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data), 
                                     void * pUserData)
{
    LOG_INFO(TEXT("CompileProgram enter. clProgram=%d, uiNumDevices=%d, pclDeviceList=%d, pcOptions=%d, num_input_headers=%d, pclInputHeaders=%d, header_include_names=%d, pUserData=%d"), 
        clProgram, uiNumDevices, pclDeviceList, pcOptions, num_input_headers, pclInputHeaders, header_include_names, pUserData);

    if ((0 == num_input_headers) && ((NULL != pclInputHeaders) || (NULL != header_include_names)))
    {
        return CL_INVALID_VALUE;
    }

    if ((0 != num_input_headers) && ((NULL == pclInputHeaders) || (NULL == header_include_names)))
    {
        return CL_INVALID_VALUE;
    }

    if ((NULL == pfn_notify) && (NULL != pUserData))
    {
        return CL_INVALID_VALUE;
    }

    // get program from programs map list
    SharedPtr<Program> pProgram = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram).DynamicCast<Program>();
    if (NULL == pProgram)
    {
        LOG_ERROR(TEXT("program %d isn't valid program"), clProgram);
        return CL_INVALID_PROGRAM;
    }

    SharedPtr<Context> pContext = pProgram->GetContext();
    cl_int clErr = pContext->CompileProgram(clProgram, uiNumDevices, pclDeviceList, num_input_headers, pclInputHeaders, header_include_names, pcOptions, pfn_notify, pUserData);

    return clErr;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::LinkProgram
//////////////////////////////////////////////////////////////////////////
cl_program ContextModule::LinkProgram(cl_context clContext, 
                                      cl_uint uiNumDevices, 
                                      const cl_device_id * pclDeviceList, 
                                      const char * pcOptions, 
                                      cl_uint uiNumInputPrograms, 
                                      const cl_program* pclInputPrograms, 
                                      void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data), 
                                      void * pUserData, 
                                      cl_int *pErrcodeRet)
{
    LOG_INFO(TEXT("LinkProgram enter. clContext=%d, uiNumDevices=%d, pclDeviceList=%d, pcOptions=%d, uiNumInputPrograms=%d, pclInputPrograms=%d, pUserData=%d"), 
        clContext, uiNumDevices, pclDeviceList, pcOptions, uiNumInputPrograms, pclInputPrograms, pUserData);

    if ((NULL == pfn_notify) && (NULL != pUserData))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }

    if ((0 == uiNumInputPrograms) || (NULL == pclInputPrograms))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }

    if ((NULL == pclDeviceList) && (0 < uiNumDevices))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }

    if ((NULL != pclDeviceList) && (0 == uiNumDevices))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }

    // get context from contexts map list
    SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)clContext).DynamicCast<Context>();
    if (NULL == pContext)
    {
        LOG_ERROR(TEXT("context %d isn't valid context"), clContext);
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }

    SharedPtr<Program> pProgram;

    if (0 < uiNumDevices)
    {
        cl_err_code clErrRet = pContext->CreateProgramForLink(uiNumDevices, pclDeviceList, &pProgram);
        if (CL_FAILED(clErrRet))
        {
            if (NULL != pErrcodeRet)
            {
                *pErrcodeRet = clErrRet;
            }
            return CL_INVALID_HANDLE;
        }
    }
    else
    {
        cl_uint uiNumContextDevices = 0;
        cl_device_id* pContextDevices = pContext->GetDeviceIds(&uiNumContextDevices);

        cl_err_code clErrRet = pContext->CreateProgramForLink((cl_uint)uiNumContextDevices, pContextDevices, &pProgram);
        if (CL_FAILED(clErrRet))
        {
            if (NULL != pErrcodeRet)
            {
                *pErrcodeRet = clErrRet;
            }
            return CL_INVALID_HANDLE;
        }
    }

    cl_err_code clErrRet = m_mapPrograms.AddObject(pProgram, false);
    if (CL_FAILED(clErrRet))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = clErrRet;
        }
        return CL_INVALID_HANDLE;
    }

    clErrRet = pContext->LinkProgram(pProgram->GetHandle(), uiNumDevices, pclDeviceList, uiNumInputPrograms, pclInputPrograms, pcOptions, pfn_notify, pUserData);
    if (CL_FAILED(clErrRet))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = clErrRet;
        }
        // we should return a valid program handle even if the linking has failed
        return pProgram->GetHandle();
    }

    if (NULL != pErrcodeRet)
    {
        *pErrcodeRet = CL_SUCCESS;
    }
    return pProgram->GetHandle();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::BuildProgram
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::BuildProgram(cl_program clProgram, 
                                   cl_uint uiNumDevices, 
                                   const cl_device_id * pclDeviceList, 
                                   const char * pcOptions, 
                                   void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data), 
                                   void * pUserData)
{
    LOG_INFO(TEXT("BuildProgram enter. clProgram=%d, uiNumDevices=%d, pclDeviceList=%d, pcOptions=%d, pUserData=%d"), 
        clProgram, uiNumDevices, pclDeviceList, pcOptions, pUserData);

    if ((NULL == pfn_notify) && (NULL != pUserData))
    {
        return CL_INVALID_VALUE;
    }

    // get program from programs map list
    SharedPtr<Program> pProgram = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram).DynamicCast<Program>();
    if (NULL == pProgram)
    {
        LOG_ERROR(TEXT("program %d isn't valid program"), clProgram);
        return CL_INVALID_PROGRAM;
    }

    SharedPtr<Context> pContext = pProgram->GetContext();
    cl_int clErr = pContext->BuildProgram(clProgram, uiNumDevices, pclDeviceList, pcOptions, pfn_notify, pUserData);
    
    return clErr;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::GetProgramInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetProgramInfo(cl_program clProgram, 
                                     cl_program_info clParamName, 
                                     size_t szParamValueSize, 
                                     void * pParamValue, 
                                     size_t * pszParamValueSizeRet)
{
    LOG_INFO(TEXT("GetProgramInfo enter. clProgram=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d"), 
        clProgram, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
    
    cl_err_code clErrRet = CL_SUCCESS;
    SharedPtr<Program> pProgram = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram).DynamicCast<Program>();
    if (NULL == pProgram)
    {
        LOG_ERROR(TEXT("m_mapPrograms.GetOCLObject(%d, %d) = %d"), clProgram, &pProgram, clErrRet);
        return CL_INVALID_PROGRAM;
    }
    return pProgram->GetInfo((cl_int)clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::GetProgramBuildInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetProgramBuildInfo(cl_program clProgram, 
                                          cl_device_id clDevice, 
                                          cl_program_info clParamName, 
                                          size_t szParamValueSize, 
                                          void * pParamValue, 
                                          size_t * pszParamValueSizeRet)
{
    LOG_INFO(TEXT("GetProgramBuildInfo enter. clProgram=%d, clDevice=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d"), 
        clProgram, clDevice, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
    
    cl_err_code clErrRet = CL_SUCCESS;
    SharedPtr<Program> pProgram = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram).DynamicCast<Program>();
    if (NULL == pProgram)
    {
        LOG_ERROR(TEXT("m_mapPrograms.GetOCLObject(%d, %d) = %d"), clProgram, &pProgram, clErrRet);
        return CL_INVALID_PROGRAM;
    }
    return pProgram->GetBuildInfo(clDevice, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateKernel
//////////////////////////////////////////////////////////////////////////
cl_kernel ContextModule::CreateKernel(cl_program clProgram, 
                                      const char * pscKernelName, 
                                      cl_int * piErr)
{
    LOG_INFO(TEXT("CreateKernel enter. clProgram=%d, pscKernelName=%d, piErr=%d"), clProgram, pscKernelName, piErr);

    // get program object
    SharedPtr<Program> pProgram = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram).DynamicCast<Program>();
    if (NULL == pProgram)
    {
        LOG_ERROR(TEXT("%s"), TEXT("clProgram is invalid program"));
        if (NULL != piErr)
        {
            *piErr = CL_INVALID_PROGRAM;
        }
        return CL_INVALID_HANDLE;
    }

    // create new kernel
    SharedPtr<Kernel> pKernel = NULL;
    cl_err_code clErrRet = pProgram->CreateKernel(pscKernelName, &pKernel);
    if (NULL != piErr)
    {
        *piErr = CL_ERR_OUT(clErrRet);
    }


    if (NULL != pKernel)
    {
        // add new kernel to the context module's kernels list
        m_mapKernels.AddObject(pKernel, false);
        if (NULL != piErr)
        {
            *piErr = CL_SUCCESS;
        }
        // return handle
        return pKernel->GetHandle();
    }

    return CL_INVALID_HANDLE;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateKernelsInProgram
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::CreateKernelsInProgram(cl_program clProgram, 
                                             cl_uint uiNumKernels, 
                                             cl_kernel * pclKernels, 
                                             cl_uint * puiNumKernelsRet)
{
    LOG_INFO(TEXT("CreateKernelsInProgram enter. clProgram=%d, uiNumKernels=%d, pclKernels=%d, puiNumKernelsRet=%d"), 
        clProgram, uiNumKernels, pclKernels, puiNumKernelsRet);

    // get the program object
    SharedPtr<Program> pProgram = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram).DynamicCast<Program>();
    if (NULL == pProgram)
    {
        LOG_ERROR(TEXT("%s"), TEXT("clProgram is invalid program"));
        return CL_INVALID_PROGRAM;
    }

    // create all kernels for the program
    cl_err_code clErrRet = pProgram->CreateAllKernels(uiNumKernels, pclKernels, puiNumKernelsRet);
    if (CL_FAILED(clErrRet))
    {
        return CL_ERR_OUT(clErrRet);
    }
    //No point in creating user-invisible kernels
    if (NULL == pclKernels)
    {
        return CL_SUCCESS;
    }

    // get kernels and add them to the context module's map list
    cl_uint uiKerenls = 0;
    clErrRet = pProgram->GetKernels(0, NULL, &uiKerenls);
    if (CL_FAILED(clErrRet))
    {
        return CL_ERR_OUT(clErrRet);
    }
    if (uiKerenls > 0)
    {
        SharedPtr<Kernel>* ppKernels = new SharedPtr<Kernel> [uiKerenls];
        if (NULL == ppKernels)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        clErrRet = pProgram->GetKernels(uiKerenls, ppKernels, NULL);
        if (CL_FAILED(clErrRet))
        {
            delete[] ppKernels;
            return CL_ERR_OUT(clErrRet);
        }
        for (cl_uint ui=0; ui<uiKerenls; ++ui)
        {
            m_mapKernels.AddObject(ppKernels[ui], false);
        }
    
        delete[] ppKernels;
    }
    return CL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainKernel
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::RetainKernel(cl_kernel clKernel)
{
    LOG_INFO(TEXT("Enter RetainKernel (clKernel=%d)"), clKernel);

    cl_err_code clErr = CL_SUCCESS;
    SharedPtr<Kernel> pKernel = m_mapKernels.GetOCLObject((_cl_kernel_int*)clKernel).DynamicCast<Kernel>();
    if (NULL == pKernel)
    {
        LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %s"), clKernel, &pKernel, ClErrTxt(clErr));
        return CL_INVALID_KERNEL;
    }
    
    clErr = pKernel->Retain();

    return CL_ERR_OUT(clErr);
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseKernel
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::ReleaseKernel(cl_kernel clKernel)
{
    LOG_INFO(TEXT("Enter ReleaseKernel (clKernel=%d)"), clKernel);

    cl_err_code clErr = CL_SUCCESS;
    SharedPtr<Kernel> pKernel = m_mapKernels.GetOCLObject((_cl_kernel_int*)clKernel).DynamicCast<Kernel>();
    if (NULL == pKernel)
    {
        LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %s"), clKernel, &pKernel, ClErrTxt(clErr));
        return CL_INVALID_KERNEL;
    }

    SharedPtr<Program> pProgram = pKernel->GetProgram();
    if (NULL == pProgram)
    {
        return CL_INVALID_KERNEL;
    }

    long newRef = pKernel->Release();
    cl_int err = CL_SUCCESS;
    if (newRef < 0)
    {
        LOG_ERROR(TEXT("pKernel->Release() returned %ld"), newRef);
        err = CL_INVALID_KERNEL;
    }
    else if (0 == newRef)
    {
        clErr = pProgram->RemoveKernel(clKernel);
        if (CL_FAILED(clErr))
        {
            return CL_ERR_OUT(clErr);
        }
        // remove kernel form kernels list and add it to the dirty kernels list        
        clErr = m_mapKernels.RemoveObject((_cl_kernel_int*)clKernel);
        if (CL_FAILED(clErr))
        {
            return CL_ERR_OUT(clErr);
        }
    }
    return err;
}

//////////////////////////////////////////////////////////////////////////
// RemoveAllKernels
//////////////////////////////////////////////////////////////////////////
void ContextModule::RemoveAllKernels( bool preserve_user_handles )
{
    m_mapKernels.DisableAdding();
    std::list<_cl_kernel_int*> to_remove;
    for (cl_uint ui=0; ui<m_mapKernels.Count(); ++ui)
    {
        SharedPtr<Kernel> obj = m_mapKernels.GetObjectByIndex(ui).DynamicCast<Kernel>();
        SharedPtr<Program> pProgram = (NULL != obj) ? obj->GetProgram() : SharedPtr<Program>(NULL);

        if (NULL != pProgram)
        {
            cl_kernel handle = obj->GetHandle();
            pProgram->RemoveKernel(handle);
            to_remove.push_back( (_cl_kernel_int*)handle );
        }
    }

    if (preserve_user_handles)
    {
        m_mapKernels.SetPreserveUserHandles();
    }

    for (std::list<_cl_kernel_int*>::iterator it = to_remove.begin(); it != to_remove.end(); ++it)
    {
        m_mapKernels.RemoveObject( *it );
    }

    m_mapKernels.ReleaseAllObjects(false);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::SetKernelArg
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::SetKernelArg(cl_kernel clKernel, 
                                   cl_uint    uiArgIndex, 
                                   size_t szArgSize, 
                                   const void * pArgValue)
{
    LOG_DEBUG(TEXT("Enter SetKernelArg (clKernel=%d, uiArgIndex=%d, szArgSize=%d, pszArgValue=%d)"), 
        clKernel, uiArgIndex, szArgSize, pArgValue);

    cl_err_code clErr = CL_SUCCESS;
    SharedPtr<Kernel> pKernel = m_mapKernels.GetOCLObject((_cl_kernel_int*)clKernel).StaticCast<Kernel>();
    if (NULL == pKernel)
    {
        LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned NULL Object"), clKernel, &pKernel);
        return CL_INVALID_KERNEL;
    }
    clErr = pKernel->SetKernelArg(uiArgIndex, szArgSize, pArgValue);
    return CL_ERR_OUT(clErr);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::SetKernelArgSVMPointer
//////////////////////////////////////////////////////////////////////////

cl_int ContextModule::SetKernelArgSVMPointer(cl_kernel clKernel, cl_uint uiArgIndex, const void* pArgValue)
{
	SharedPtr<Kernel> pKernel = m_mapKernels.GetOCLObject((_cl_kernel_int*)clKernel).StaticCast<Kernel>();
	if (NULL == pKernel)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned NULL"), clKernel, &pKernel);
		return CL_INVALID_KERNEL;
	}
	
    cl_err_code err = pKernel->SetKernelArg(uiArgIndex, sizeof(void*), pArgValue, true);
	return CL_ERR_OUT(err);
}

cl_int ContextModule::SetKernelExecInfo(cl_kernel clKernel, cl_kernel_exec_info paramName, size_t szParamValueSize, const void* pParamValue)
{
	SharedPtr<Kernel> pKernel = m_mapKernels.GetOCLObject((_cl_kernel_int*)clKernel).StaticCast<Kernel>();
	if (NULL == pKernel)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned NULL"), clKernel, &pKernel);
		return CL_INVALID_KERNEL;
	}
	if (NULL == pParamValue)
	{
		return CL_INVALID_VALUE;
	}
	return pKernel->GetContext()->SetKernelExecInfo(pKernel, paramName, szParamValueSize, pParamValue);		
}

cl_mem ContextModule::CreatePipe(cl_context context, cl_mem_flags flags, cl_uint uiPipePacketSize, cl_uint uiPipeMaxPackets, const cl_pipe_properties* pProperties, void* pHostPtr,
    size_t* pSizeRet, cl_int* piErrcodeRet)
{
	SharedPtr<Context> pContext = GetContext(context);
	if (NULL == pContext)
	{
		if (NULL != piErrcodeRet)
		{
			*piErrcodeRet = CL_INVALID_CONTEXT;
		}
		return CL_INVALID_HANDLE;
	}		
	if (flags != (CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS) || NULL != pProperties)
	{
		if (NULL != piErrcodeRet)
		{
			*piErrcodeRet = CL_INVALID_VALUE;
		}
		return CL_INVALID_HANDLE;
	}

	cl_uint uiMinPipeMaxPacketSize = 0;
	const tSetOfDevices* pDevs = pContext->GetAllRootDevices();
	for (tSetOfDevices::const_iterator iter = pDevs->begin(); iter != pDevs->end(); iter++)
	{
		cl_uint uiPipeMaxPacketSize;
		const cl_err_code err = (*iter)->GetInfo(CL_DEVICE_PIPE_MAX_PACKET_SIZE, sizeof(uiPipeMaxPacketSize), &uiPipeMaxPacketSize, NULL);		
		if (CL_FAILED(err))
		{
			if (NULL != piErrcodeRet)
			{
				*piErrcodeRet = err;
			}
			return CL_INVALID_HANDLE;
		}
		if (0 == uiMinPipeMaxPacketSize || uiPipeMaxPacketSize < uiMinPipeMaxPacketSize)
		{
			uiMinPipeMaxPacketSize = uiPipeMaxPacketSize;
		}
	}
	if (0 == uiPipePacketSize || 0 == uiPipeMaxPackets || uiPipePacketSize > uiMinPipeMaxPacketSize)
	{
		if (NULL != piErrcodeRet)
		{
			*piErrcodeRet = CL_INVALID_PIPE_SIZE;
		}
		return CL_INVALID_HANDLE;
	}

    // handling INTEL extension for CRT
    if (NULL != pSizeRet)
    {
        if (NULL == pHostPtr)
        {
            *pSizeRet = Pipe::CalcPipeSize(uiPipePacketSize, uiPipeMaxPackets);
            if (NULL != piErrcodeRet)
            {
                *piErrcodeRet = CL_SUCCESS;
            }
            return CL_INVALID_HANDLE;
        }
        else
        {
            if (*pSizeRet != Pipe::CalcPipeSize(uiPipePacketSize, uiPipeMaxPackets))
            {
                if (NULL != piErrcodeRet)
                {
                    *piErrcodeRet = CL_OUT_OF_RESOURCES;
                }
                return CL_INVALID_HANDLE;
            }
        }
    }
    else
    {
        ASSERT_RET_VAL(NULL == pHostPtr, "this combination isn't expected from CRT", CL_INVALID_HANDLE);
    }

	SharedPtr<MemoryObject> pPipe;
	cl_err_code err = pContext->CreatePipe(uiPipePacketSize, uiPipeMaxPackets, pPipe, pHostPtr);
	if (CL_FAILED(err))
	{
		if (NULL != piErrcodeRet)
		{
			*piErrcodeRet = err;
		}
		return CL_INVALID_HANDLE;
	}
	err = m_mapMemObjects.AddObject(pPipe, false);
	if (CL_FAILED(err))
	{
		if (NULL != piErrcodeRet)
		{
			*piErrcodeRet = err;
		}
		return CL_INVALID_HANDLE;
	}
	if (NULL != piErrcodeRet)
	{
		*piErrcodeRet = CL_SUCCESS;
	}
	return pPipe->GetHandle();
}

cl_int ContextModule::GetPipeInfo(cl_mem pipe, cl_pipe_info paramName, size_t szParamValueSize, void *pParamValue, size_t* pszParamValueSizeRet)
{
	SharedPtr<Pipe> pPipe = m_mapMemObjects.GetOCLObject((_cl_mem_int*)pipe).StaticCast<Pipe>();
	if (NULL == pPipe)
	{
		return CL_INVALID_MEM_OBJECT;
	}
	return pPipe->GetPipeInfo(paramName, szParamValueSize, pParamValue, pszParamValueSizeRet);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::GetKernelInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetKernelInfo(cl_kernel clKernel, 
                                    cl_kernel_info clParamName, 
                                    size_t szParamValueSize, 
                                    void * pParamValue, 
                                    size_t * pszParamValueSizeRet)
{
    LOG_INFO(TEXT("Enter GetKernelInfo (clKernel=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"), 
        clKernel, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

    cl_err_code clErr = CL_SUCCESS;
    SharedPtr<Kernel> pKernel = m_mapKernels.GetOCLObject((_cl_kernel_int*)clKernel).DynamicCast<Kernel>();
    if (NULL == pKernel)
    {
        LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %s"), clKernel, &pKernel, ClErrTxt(clErr));
        return CL_INVALID_KERNEL;
    }

    return pKernel->GetInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::GetKernelWorkGroupInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetKernelWorkGroupInfo(cl_kernel clKernel, 
                                             cl_device_id deviceId, 
                                             cl_kernel_work_group_info clParamName, 
                                             size_t szParamValueSize, 
                                             void *    pParamValue, 
                                             size_t * pszParamValueSizeRet)
{
    LOG_INFO(TEXT("Enter GetKernelWorkGroupInfo (clKernel=%d, clDevice=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"), 
        clKernel, deviceId, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
    SharedPtr<Kernel> pKernel = m_mapKernels.GetOCLObject((_cl_kernel_int*)clKernel).DynamicCast<Kernel>();
    if (NULL == pKernel)
    {
        LOG_ERROR(TEXT("GetOCLObject(%d) returned NULL"), clKernel);
        return CL_INVALID_KERNEL;
    }    
    SharedPtr<FissionableDevice> pDevice;
    if ( NULL != deviceId )    // When deviceId is NULL, we should pass this paramter to kernel.
                            // In case of single device, it's data should be returned
    {
        pDevice = m_pPlatformModule->GetDevice(deviceId);
        if (NULL == pDevice)
        {
            LOG_ERROR(TEXT("GetDevice(%d) returned NULL"), deviceId);
            return CL_INVALID_DEVICE;
        }
    }
    return pKernel->GetWorkGroupInfo(pDevice, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateBuffer
//////////////////////////////////////////////////////////////////////////
cl_mem ContextModule::CreateBuffer(cl_context clContext, 
                                   cl_mem_flags clFlags, 
                                   size_t szSize, 
                                   void * pHostPtr, 
                                   cl_int * pErrcodeRet)
{
    LOG_DEBUG(TEXT("Enter CreateBuffer (clContext=%d, clFlags=%d, szSize=%d, pHostPtr=%d, pErrcodeRet=%d)"), 
        clContext, clFlags, szSize, pHostPtr, pErrcodeRet);

    SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)clContext).DynamicCast<Context>();    
    if (NULL == pContext)
    {
        LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d) = NULL , pContext = %d"), clContext, pContext.GetPtr())
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }

	cl_err_code clErr = CheckMemObjectParameters(clFlags, NULL, CL_MEM_OBJECT_BUFFER, 0, 0, 0, 0, 0, 0, pHostPtr);
	if ( !((CL_INVALID_IMAGE_FORMAT_DESCRIPTOR == clErr) || (CL_SUCCESS == clErr)) )
	{
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet =  clErr;
		}
		return CL_INVALID_HANDLE;
	}

	SharedPtr<MemoryObject> pBuffer;
	SharedPtr<SVMBuffer> pSvmBuf = pContext->GetSVMBufferContainingAddr(pHostPtr);	// we assume that the cl_mem and SVM buffer share the same context
	if (pSvmBuf != NULL && (clFlags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR)))
	{
		if (!pSvmBuf->IsContainedInBuffer(pHostPtr, szSize))
		{
			if (NULL != pErrcodeRet)
			{
				*pErrcodeRet = CL_INVALID_BUFFER_SIZE;	// this error code isn't specified in the spec
			}
			return CL_INVALID_HANDLE;
		}
	}
	if (pSvmBuf != NULL && (clFlags & CL_MEM_USE_HOST_PTR))
	{
		if (pSvmBuf->GetAddr() == pHostPtr && szSize == pSvmBuf->GetSize())
		{
			pBuffer = pSvmBuf;
			pContext->AddSvmBufferAsMemBuffer(pSvmBuf);
		}
		else
		{
			cl_buffer_region bufRegion;
			bufRegion.origin = (char*)pHostPtr - (char*)pSvmBuf->GetAddr();
			bufRegion.size = szSize;
			clErr = pContext->CreateSubBuffer(pSvmBuf, clFlags, CL_BUFFER_CREATE_TYPE_REGION, &bufRegion, &pBuffer);
		}		
	}
	else
	{
		clErr = pContext->CreateBuffer(clFlags, szSize, pHostPtr, &pBuffer);
	}

	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("pContext->CreateBuffer(%d, %d, %d, %d) = %s"), clFlags, szSize, pHostPtr, &pBuffer, ClErrTxt(clErr))
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_ERR_OUT(clErr);
		}
		return CL_INVALID_HANDLE;
	}	
	clErr = m_mapMemObjects.AddObject(pBuffer, false);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %S"), pBuffer.GetPtr(), pBuffer->GetHandle(), ClErrTxt(clErr))
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_ERR_OUT(clErr);
		}
		return CL_INVALID_HANDLE;
	}
	if (NULL != pErrcodeRet)
	{
		*pErrcodeRet = CL_SUCCESS;
	}
	return pBuffer->GetHandle();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateSubBuffer
//////////////////////////////////////////////////////////////////////////
cl_mem ContextModule::CreateSubBuffer(cl_mem                clBuffer,
                                   cl_mem_flags                clFlags, 
                                   cl_buffer_create_type    buffer_create_type, 
                                   const void *                buffer_create_info, 
                                   cl_int *                    pErrcodeRet)
{
    LOG_INFO(TEXT("Enter CreateSubBuffer (clFlags=%d, cl_buffer_create_type=%d, pErrcodeRet=%d)"), 
        clFlags, buffer_create_type, pErrcodeRet);

    cl_int iNullErr;
    cl_int& iErr = pErrcodeRet ? *pErrcodeRet : iNullErr;

    iErr = CheckMemObjectParameters(clFlags, NULL, CL_MEM_OBJECT_BUFFER, 0, 0, 0, 0, 0, 0, NULL);
    if (CL_FAILED(iErr))
    {
        return CL_INVALID_HANDLE;
    }
    if (!clBuffer)
    {
        iErr = CL_INVALID_MEM_OBJECT;        
        return CL_INVALID_HANDLE;
    }

    SharedPtr<MemoryObject> pMemObj = m_mapMemObjects.GetOCLObject((_cl_mem_int*)clBuffer).DynamicCast<MemoryObject>();
    if (NULL == pMemObj)
    {
        LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned NULL"), clBuffer, &pMemObj);
        iErr = CL_INVALID_MEM_OBJECT;        
        return CL_INVALID_HANDLE;
    }

    SharedPtr<Context> pContext = pMemObj->GetContext();

	// check memory object is a Buffer not Image2D/3D
	if (pMemObj->GetType() != CL_MEM_OBJECT_BUFFER)
	{
		iErr = CL_INVALID_MEM_OBJECT;		
		return CL_INVALID_HANDLE;
	}
			
	if (NULL != pMemObj->GetParent())
	{
		if (pMemObj->GetParent().DynamicCast<SVMBuffer>() == NULL)
		{
			iErr = CL_INVALID_MEM_OBJECT;
			return CL_INVALID_HANDLE;
		}
		/* When creating a cl_mem buffer from an SVM buffer, if size < SVMBuffer.m_size, we return a sub-buffer of SVMBuffer.m_memObj. However, if the user creates a sub-buffer of
		   this cl_mem buffer, we can't create a sub-buffer of a sub-buffer. So the solution is to create a sub-buffer of the SVMBuffer itself. */
		pMemObj = pMemObj->GetParent();
	}

    SharedPtr<MemoryObject> pBuffer = NULL;
    cl_err_code clErr = pContext->CreateSubBuffer(pMemObj, clFlags, buffer_create_type, buffer_create_info, &pBuffer);
    if (CL_FAILED(clErr))
    {        
        iErr = CL_ERR_OUT(clErr);        
        return CL_INVALID_HANDLE;
    }

    clErr = m_mapMemObjects.AddObject(pBuffer, false);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %S"), pBuffer.GetPtr(), pBuffer->GetHandle(), ClErrTxt(clErr))
        if (NULL != pErrcodeRet)
        {
            iErr = CL_ERR_OUT(clErr);
        }
        return CL_INVALID_HANDLE;
    }
    if (NULL != pErrcodeRet)
    {
        iErr = CL_SUCCESS;
    }

    return pBuffer->GetHandle();    
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateImage2D
//////////////////////////////////////////////////////////////////////////
cl_mem ContextModule::CreateImage2D(cl_context clContext, 
                                    cl_mem_flags clFlags, 
                                    const cl_image_format * clImageFormat, 
                                    size_t szImageWidth, 
                                    size_t szImageHeight, 
                                    size_t szImageRowPitch, 
                                    void * pHostPtr, 
                                    cl_int * pErrcodeRet)
{
    return CreateScalarImage<2, CL_MEM_OBJECT_IMAGE2D>(clContext, clFlags, clImageFormat, szImageWidth, szImageHeight, 0, szImageRowPitch, 0, pHostPtr, pErrcodeRet);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateImage3D
//////////////////////////////////////////////////////////////////////////
cl_mem ContextModule::CreateImage3D(cl_context clContext, 
                                    cl_mem_flags clFlags, 
                                    const cl_image_format * clImageFormat, 
                                    size_t szImageWidth, 
                                    size_t szImageHeight, 
                                    size_t szImageDepth, 
                                    size_t szImageRowPitch, 
                                    size_t szImageSlicePitch, 
                                    void * pHostPtr, 
                                    cl_int * pErrcodeRet)
{
    return CreateScalarImage<3, CL_MEM_OBJECT_IMAGE3D>(clContext, clFlags, clImageFormat, szImageWidth, szImageHeight, szImageDepth, szImageRowPitch, szImageSlicePitch,
        pHostPtr, pErrcodeRet);
}

/************************************************************************/
/* ContextModule::CreateImage                                              */
/************************************************************************/
cl_mem ContextModule::CreateImage(cl_context context,
                                  cl_mem_flags flags,
                                  const cl_image_format *image_format,
                                  const cl_image_desc *image_desc,
                                  void *host_ptr,
                                  cl_int *errcode_ret)
{
    LOG_DEBUG(TEXT("Enter CreateImage(context=%p, flags=%d, image_format=%p, image_desc=%p, host_ptr=%p, errcode_ret=%p"),
        context, flags, image_format, image_desc, host_ptr, errcode_ret);

    cl_mem clMemObj = CL_INVALID_HANDLE;

    if (!image_desc || 0 != image_desc->num_mip_levels || 0 != image_desc->num_samples ||
		(CL_MEM_OBJECT_IMAGE1D_BUFFER != image_desc->image_type && CL_MEM_OBJECT_IMAGE2D != image_desc->image_type && NULL != image_desc->mem_object))
    {
        if (errcode_ret)
        {
            *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
        }
        return CL_INVALID_HANDLE;
    }
    switch (image_desc->image_type)
    {
    case CL_MEM_OBJECT_IMAGE1D:
        clMemObj = CreateScalarImage<1, CL_MEM_OBJECT_IMAGE1D>(context, flags, image_format, image_desc->image_width, 0, 0, 0, 0, host_ptr, errcode_ret);
        break;
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
		clMemObj = CreateImageBuffer<1, CL_MEM_OBJECT_IMAGE1D_BUFFER>(context, flags, image_format, *image_desc, image_desc->mem_object, errcode_ret);
        break;
    case CL_MEM_OBJECT_IMAGE2D:
        if (NULL == image_desc->mem_object)
		{
			clMemObj = CreateScalarImage<2, CL_MEM_OBJECT_IMAGE2D>(context, flags, image_format, image_desc->image_width, image_desc->image_height, 0, image_desc->image_row_pitch, 0, host_ptr,
				errcode_ret);
		}
		else
		{
			cl_mem_object_type objType;
			if (CL_FAILED(GetMemObjectInfo(image_desc->mem_object, CL_MEM_TYPE, sizeof(objType), &objType, NULL)) ||
				(objType != CL_MEM_OBJECT_BUFFER && objType != CL_MEM_OBJECT_IMAGE2D))
			{
				if (errcode_ret != NULL)
				{
					*errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
				}
				return CL_INVALID_HANDLE;
			}
			if (CL_MEM_OBJECT_BUFFER == objType)
			{
				clMemObj = CreateImageBuffer<2, CL_MEM_OBJECT_IMAGE2D>(context, flags, image_format, *image_desc, image_desc->mem_object, errcode_ret);
			}
			else
			{
				clMemObj = Create2DImageFromImage(context, flags, image_format, image_desc, image_desc->mem_object, errcode_ret);
			}
		}
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        clMemObj = CreateScalarImage<3, CL_MEM_OBJECT_IMAGE3D>(context, flags, image_format, image_desc->image_width,
            image_desc->image_height, image_desc->image_depth, image_desc->image_row_pitch,
            image_desc->image_slice_pitch, host_ptr, errcode_ret);
        break;
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        clMemObj = CreateImageArray(context, flags, image_format, image_desc, host_ptr, errcode_ret);
        break;
    default:
        LOG_ERROR(TEXT("unsupported image type (%d)"), image_desc->image_type);
        if (errcode_ret)
        {
            *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
        }
    }
    return clMemObj;
}

bool ContextModule::Check2DImageFromBufferPitch(const ConstSharedPtr<GenericMemObject>& pBuffer, const cl_image_desc& desc, const cl_image_format& format) const
{
	const tSetOfDevices& devices = *pBuffer->GetContext()->GetAllRootDevices();

	cl_uint uiMaxImgPitchAlign = 0;
	for (tSetOfDevices::const_iterator iter = devices.begin(); iter != devices.end(); iter++)
	{
		cl_uint uiImgPitchAlign;
		(*iter)->GetInfo(CL_DEVICE_IMAGE_PITCH_ALIGNMENT, sizeof(uiImgPitchAlign), &uiImgPitchAlign, NULL);
		if (uiImgPitchAlign > uiMaxImgPitchAlign)
		{
			uiMaxImgPitchAlign = uiImgPitchAlign;
		}
	}
	if (0 == uiMaxImgPitchAlign)
	{
		return false;
	}

	const size_t szRowPitch = desc.image_row_pitch > 0 ? desc.image_row_pitch : desc.image_width * clGetPixelBytesCount(&format);
	return szRowPitch % uiMaxImgPitchAlign == 0;
}

cl_mem ContextModule::CreateImageArray(cl_context clContext,
                                       cl_mem_flags clFlags,
                                       const cl_image_format* clImageFormat,
                                       const cl_image_desc* pClImageDesc,
                                       void* pHostPtr,
                                       cl_int* pErrcodeRet)
{
    SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)clContext).DynamicCast<Context>();
    if (NULL == pContext)
    {
        LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d) = NULL"), clContext);
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }

    // Do some initial (not context specific) parameter checking
    // check input memory flags
    cl_err_code clErr = CheckMemObjectParameters(clFlags, clImageFormat, pClImageDesc->image_type, pClImageDesc->image_width, pClImageDesc->image_height, 0, pClImageDesc->image_row_pitch,
        pClImageDesc->image_slice_pitch, pClImageDesc->image_array_size, pHostPtr);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("%s"), TEXT("Parameter check failed"));
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = clErr;
        }
        return CL_INVALID_HANDLE;
    }
    SharedPtr<MemoryObject> pImageArr;
    // Do some context specific checks
    if ( CL_MEM_OBJECT_IMAGE1D_ARRAY == pClImageDesc->image_type)
    {
        clErr = CheckContextSpecificParameters(pContext, pClImageDesc->image_type, pClImageDesc->image_width,
                0, 0, pClImageDesc->image_array_size);
    } else if ( CL_MEM_OBJECT_IMAGE2D_ARRAY == pClImageDesc->image_type)
    {
        clErr = CheckContextSpecificParameters(pContext, pClImageDesc->image_type, pClImageDesc->image_width,
                pClImageDesc->image_height, 0, pClImageDesc->image_array_size);
    } else {
        assert(0 && "Inside CreateImageArray with non array type.");
    }
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("%S"), TEXT("Context specific parameter check failed"));
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = clErr;
        }
        return CL_INVALID_HANDLE;
    }

    // Do some context specific checks
    if ( CL_MEM_OBJECT_IMAGE1D_ARRAY == pClImageDesc->image_type)
    {
        clErr = CheckContextSpecificParameters(pContext, pClImageDesc->image_type, pClImageDesc->image_width,
                0, 0, pClImageDesc->image_array_size);
    } else if ( CL_MEM_OBJECT_IMAGE2D_ARRAY == pClImageDesc->image_type)
    {
        clErr = CheckContextSpecificParameters(pContext, pClImageDesc->image_type, pClImageDesc->image_width,
                pClImageDesc->image_height, 0, pClImageDesc->image_array_size);
    } else {
        assert(0 && "Inside CreateImageArray with non array type.");
    }
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("%s"), TEXT("Context specific parameter check failed"));
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = clErr;
        }
        return CL_INVALID_HANDLE;
    }

    clErr = pContext->CreateImageArray(clFlags, clImageFormat, pHostPtr, pClImageDesc, &pImageArr);
    if (CL_FAILED(clErr) || NULL == pImageArr)
    {
        LOG_ERROR(TEXT("pContext->CreateImage2DArray(%d, %p, %p, %p, %p) = %s"), clFlags, clImageFormat, pHostPtr, pClImageDesc, &pImageArr, ClErrTxt(clErr));
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_ERR_OUT(clErr);
        }
        return CL_INVALID_HANDLE;
    }
    clErr = m_mapMemObjects.AddObject(pImageArr, false);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %S"), pImageArr.GetPtr(), pImageArr->GetHandle(), ClErrTxt(clErr))
            if (NULL != pErrcodeRet)
            {
                *pErrcodeRet = CL_ERR_OUT(clErr);
            }
            return CL_INVALID_HANDLE;
    }
    if (NULL != pErrcodeRet)
    {
        *pErrcodeRet = CL_SUCCESS;
    }
    return pImageArr->GetHandle();
}

cl_mem ContextModule::Create2DImageFromImage(cl_context context, cl_mem_flags flags, const cl_image_format* pImageFormat, const cl_image_desc* pImageDesc, cl_mem otherImgHandle,
	cl_int* piErrcodeRet)
{
	SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)context).DynamicCast<Context>();
	SharedPtr<MemoryObject> pOtherImg = pContext->GetMemObject(otherImgHandle);
	size_t szOtherWidth, szOtherHeight, szOtherRowPitch;
	cl_image_format otherImgFormat;

	pOtherImg->GetImageInfo(CL_IMAGE_WIDTH, sizeof(szOtherWidth), &szOtherWidth, NULL);
	pOtherImg->GetImageInfo(CL_IMAGE_HEIGHT, sizeof(szOtherHeight), &szOtherHeight, NULL);
	pOtherImg->GetImageInfo(CL_IMAGE_ROW_PITCH, sizeof(szOtherRowPitch), &szOtherRowPitch, NULL);
	pOtherImg->GetImageInfo(CL_IMAGE_FORMAT, sizeof(otherImgFormat), &otherImgFormat, NULL);

	if (pImageDesc->image_width != szOtherWidth || pImageDesc->image_height != szOtherHeight || pImageDesc->image_row_pitch != szOtherRowPitch ||
		!((pImageFormat->image_channel_order == CL_sBGRA && otherImgFormat.image_channel_order == CL_BGRA) ||
		  (pImageFormat->image_channel_order == CL_BGRA && otherImgFormat.image_channel_order == CL_sBGRA) ||
		  (pImageFormat->image_channel_order == CL_sRGBA && otherImgFormat.image_channel_order == CL_RGBA) ||
		  (pImageFormat->image_channel_order == CL_RGBA && otherImgFormat.image_channel_order == CL_sRGBA) ||
		  (pImageFormat->image_channel_order == CL_sRGB && otherImgFormat.image_channel_order == CL_RGB) ||
		  (pImageFormat->image_channel_order == CL_RGB && otherImgFormat.image_channel_order == CL_sRGB) ||
		  (pImageFormat->image_channel_order == CL_sRGBx && otherImgFormat.image_channel_order == CL_RGBx) ||
		  (pImageFormat->image_channel_order == CL_RGBx && otherImgFormat.image_channel_order == CL_sRGBx)))
	{
		if (piErrcodeRet != NULL)
		{
			*piErrcodeRet = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
		}
		return NULL;
	}
	void* const pData = pOtherImg->GetBackingStoreData();
	return CreateScalarImage<2, CL_MEM_OBJECT_IMAGE2D>(context, flags, pImageFormat, pImageDesc->image_width, pImageDesc->image_height, 1, pImageDesc->image_row_pitch, 0, pData,
		piErrcodeRet, true);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainMemObject
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::RetainMemObject(cl_mem clMemObj)
{
    LOG_DEBUG(TEXT("Enter RetainMemObject (clMemObj=%d)"), clMemObj);

    cl_err_code clErr = CL_SUCCESS;
    SharedPtr<MemoryObject> pMemObj = m_mapMemObjects.GetOCLObject((_cl_mem_int*)clMemObj).DynamicCast<MemoryObject>();
    if (NULL == pMemObj)
    {
        LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %s"), clMemObj, &pMemObj, ClErrTxt(clErr));
        return CL_INVALID_MEM_OBJECT;
    }
    return pMemObj->Retain();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseMemObject
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::ReleaseMemObject(cl_mem clMemObj)
{
    LOG_DEBUG(TEXT("Enter ReleaseMemObject (clMemObj=%d)"), clMemObj);

    cl_err_code clErr = CL_SUCCESS;
    SharedPtr<MemoryObject> pMemObj = m_mapMemObjects.GetOCLObject((_cl_mem_int*)clMemObj).DynamicCast<MemoryObject>();    
    if (NULL == pMemObj)
    {
        LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %s"), clMemObj, &pMemObj, ClErrTxt(clErr));
        return CL_INVALID_MEM_OBJECT;
    }
    SharedPtr<Context> pContext = pMemObj->GetContext();
    if (NULL == pContext)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    long newRef = pMemObj->Release();

    cl_int res = CL_SUCCESS;
    if (newRef < 0)
    {
        return CL_INVALID_MEM_OBJECT;
    }
    else if (0 == newRef)
    {
        // TODO: handle release memory object
        clErr = pContext->RemoveMemObject(clMemObj);
        if (CL_FAILED(clErr))
        {
            res = CL_ERR_OUT(clErr);
        }
        clErr = m_mapMemObjects.RemoveObject((_cl_mem_int*)clMemObj);
        if (CL_FAILED(clErr))
        {
            res = CL_ERR_OUT(clErr);
        }
    }
    return res;
}

//////////////////////////////////////////////////////////////////////////
// RemoveAllMemObjects
//////////////////////////////////////////////////////////////////////////
typedef std::list<SharedPtr<MemoryObject> >  MemObjListType;
void ContextModule::RemoveAllMemObjects( bool preserve_user_handles )
{
    m_mapMemObjects.DisableAdding();
    std::list<_cl_mem_int*> to_remove;
    for (cl_uint ui=0; ui<m_mapMemObjects.Count(); ++ui)
    {
        SharedPtr<MemoryObject> obj = m_mapMemObjects.GetObjectByIndex(ui).DynamicCast<MemoryObject>();
        SharedPtr<Context> pContext = (NULL != obj) ? obj->GetContext() : SharedPtr<Context>(NULL);

        if (NULL != pContext)
        {
            cl_mem handle = obj->GetHandle();
            pContext->RemoveMemObject(handle);
            to_remove.push_back( (_cl_mem_int*)handle );
        }
    }

    
    if (preserve_user_handles)
    {
        m_mapMemObjects.SetPreserveUserHandles();
    }

    for (std::list<_cl_mem_int*>::iterator it = to_remove.begin(); it != to_remove.end(); ++it)
    {
        m_mapMemObjects.RemoveObject( *it );
    }

    m_mapMemObjects.ReleaseAllObjects(false);

    // Remove all mapped regions
    MemObjListType mapped_list;
    m_setMappedMemObjects.getObjects( mapped_list );

    for ( MemObjListType::iterator it = mapped_list.begin(); it != mapped_list.end(); ++it)
    {
        SharedPtr<MemoryObject> obj = *it;
        obj->ReleaseAllMappedRegions();
    }
    mapped_list.clear();
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::GetSupportedImageFormats
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetSupportedImageFormats(cl_context clContext, 
                                               cl_mem_flags clFlags, 
                                               cl_mem_object_type clImageType, 
                                               cl_uint uiNumEntries, 
                                               cl_image_format * pclImageFormats, 
                                               cl_uint * puiNumImageFormats)
{
    LOG_INFO(TEXT("Enter GetSupportedImageFormats (clContext=%d, clFlags=%d, clImageType=%d, uiNumEntries=%d, pclImageFormats=%d, puiNumImageFormats=%d)"), 
        clContext, clFlags, clImageType, uiNumEntries, pclImageFormats, puiNumImageFormats);

    SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)clContext).DynamicCast<Context>();
    if (NULL == pContext)
    {
        LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d) = NULL"), clContext);
        return CL_INVALID_CONTEXT;
    }
    return pContext->GetSupportedImageFormats(clFlags, clImageType, uiNumEntries, pclImageFormats, puiNumImageFormats);
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::GetMemObjectInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetMemObjectInfo(cl_mem clMemObj, 
                                       cl_mem_info clParamName, 
                                       size_t szParamValueSize, 
                                       void * pParamValue, 
                                       size_t * pszParamValueSizeRet)
{
    LOG_INFO(TEXT("Enter GetMemObjectInfo (clMemObj=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"), 
        clMemObj, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

    cl_err_code clErr = CL_SUCCESS;
    SharedPtr<MemoryObject> pMemObj = m_mapMemObjects.GetOCLObject((_cl_mem_int*)clMemObj).DynamicCast<MemoryObject>();
    if (NULL == pMemObj)
    {
        LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %s"), clMemObj, &pMemObj, ClErrTxt(clErr));
        return CL_INVALID_MEM_OBJECT;
    }

    return pMemObj->GetInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::GetImageInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetImageInfo(cl_mem clImage, 
                                   cl_image_info clParamName, 
                                   size_t szParamValueSize, 
                                   void * pParamValue, 
                                   size_t * pszParamValueSizeRet)
{
    LOG_INFO(TEXT("Enter GetImageInfo (clImage=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"), 
        clImage, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

    cl_err_code clErr = CL_SUCCESS;
    SharedPtr<MemoryObject> pMemObj = m_mapMemObjects.GetOCLObject((_cl_mem_int*)clImage).DynamicCast<MemoryObject>();
    if (NULL == pMemObj)
    {
        LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %s"), clImage, &pMemObj, ClErrTxt(clErr));
        return CL_INVALID_MEM_OBJECT;
    }

    // If memory object doesnt support this operation it retuns CL_INVALID_MEM_OBJECT
    return pMemObj->GetImageInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateSampler
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::SetMemObjectDestructorCallback (cl_mem memObj,
                                                      mem_dtor_fn pfn_notify,                                                      
                                                      void *pUserData)
{
    cl_err_code clErr = CL_SUCCESS;

    SharedPtr<MemoryObject> pMemObj = m_mapMemObjects.GetOCLObject((_cl_mem_int*)memObj).DynamicCast<MemoryObject>();
    if (NULL == pMemObj)
    {
        LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %s"), memObj, &pMemObj, ClErrTxt(clErr));
        return CL_INVALID_MEM_OBJECT;
    }

    // if pfn_notify is NULL. the following register function will return CL_INVALID_VALUE
    clErr = pMemObj->registerDtorNotifierCallback(pfn_notify,pUserData);
    return clErr;
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateSampler
//////////////////////////////////////////////////////////////////////////
cl_sampler ContextModule::CreateSampler(cl_context clContext, 
                                        cl_bool bNormalizedCoords, 
                                        cl_addressing_mode clAddressingMode, 
                                        cl_filter_mode clFilterMode, 
                                        cl_int * pErrcodeRet)
{
    LOG_DEBUG(TEXT("Enter CreateSampler (clContext=%d, bNormalizedCoords=%d, clAddressingMode=%d, clFilterMode=%d, pErrcodeRet=%d)"), 
        clContext, bNormalizedCoords, clAddressingMode, clFilterMode, pErrcodeRet);

    SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)clContext).DynamicCast<Context>();
    if (NULL == pContext)
    {
        LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d) = NULL"), clContext);
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }
    SharedPtr<Sampler> pSampler;
    cl_err_code clErr = pContext->CreateSampler(bNormalizedCoords, clAddressingMode, clFilterMode, &pSampler);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("pContext->CreateSampler(%d, %d, %d, %d) = %s"), bNormalizedCoords, clAddressingMode, clFilterMode, &pSampler, ClErrTxt(clErr))
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_ERR_OUT(clErr);
        }
        return CL_INVALID_HANDLE;
    }
    clErr = m_mapSamplers.AddObject(pSampler, false);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %S"), pSampler.GetPtr(), pSampler->GetHandle(), ClErrTxt(clErr))
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_ERR_OUT(clErr);
        }
        return CL_INVALID_HANDLE;
    }
    if (NULL != pErrcodeRet)
    {
        *pErrcodeRet = CL_SUCCESS;
    }
    return pSampler->GetHandle();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainSampler
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::RetainSampler(cl_sampler clSampler)
{
    LOG_DEBUG(TEXT("Enter RetainSampler (clSampler=%d)"), clSampler);

    cl_err_code clErr = CL_SUCCESS;
    SharedPtr<Sampler> pSampler = m_mapSamplers.GetOCLObject((_cl_sampler_int*)clSampler).DynamicCast<Sampler>();
    if (CL_FAILED(clErr) || NULL == pSampler)
    {
        LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %s"), clSampler, &pSampler, ClErrTxt(clErr));
        return CL_INVALID_SAMPLER;
    }
    return pSampler->Retain();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseSampler
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::ReleaseSampler(cl_sampler clSampler)
{
    LOG_DEBUG(TEXT("Enter RetainMemObject (clMemObj=%d)"), clSampler);


    cl_err_code clErr = CL_SUCCESS;
    SharedPtr<Sampler> pSampler = m_mapSamplers.GetOCLObject((_cl_sampler_int*)clSampler).DynamicCast<Sampler>();
    if (NULL == pSampler)
    {
        LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %s"), clSampler, &pSampler, ClErrTxt(clErr));
        return CL_INVALID_SAMPLER;
    }
    SharedPtr<Context> pContext = pSampler->GetContext();
    if (NULL == pContext)
    {
        return CL_INVALID_SAMPLER;
    }

    long newRef = pSampler->Release();
    cl_int res = CL_SUCCESS;
    if (newRef < 0)
    {
        return CL_INVALID_SAMPLER;
    }
    else if (0 == newRef)
    {
        clErr = pContext->RemoveSampler(clSampler);
        if (CL_FAILED(clErr))
        {
            res = CL_ERR_OUT(clErr);
        }        
        clErr = m_mapSamplers.RemoveObject((_cl_sampler_int*)clSampler);
        if (CL_FAILED(clErr))
        {
            res = CL_ERR_OUT(clErr);
        }
    }

    return res;
}

///////////////////////////////////////////////////////////////////////////
// RemoveAllSamplers
//////////////////////////////////////////////////////////////////////////
void ContextModule::RemoveAllSamplers(bool preserve_user_handles)
{
    m_mapSamplers.DisableAdding();
    std::list<_cl_sampler_int*> to_remove;
    for (cl_uint ui=0; ui<m_mapSamplers.Count(); ++ui)
    {
        SharedPtr<Sampler> obj = m_mapSamplers.GetObjectByIndex(ui).DynamicCast<Sampler>();
        SharedPtr<Context> pContext = (NULL != obj) ? obj->GetContext() : SharedPtr<Context>(NULL);

        if (NULL != pContext)
        {
            cl_sampler handle = obj->GetHandle();
            pContext->RemoveSampler(handle);
            to_remove.push_back( (_cl_sampler_int*)handle );
        }
    }
    
    if (preserve_user_handles)
    {
        m_mapSamplers.SetPreserveUserHandles();
    }

    for (std::list<_cl_sampler_int*>::iterator it = to_remove.begin(); it != to_remove.end(); ++it)
    {
        m_mapSamplers.RemoveObject( *it );
    }

    m_mapSamplers.ReleaseAllObjects(false);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::GetSamplerInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetSamplerInfo(cl_sampler clSampler, 
                                     cl_sampler_info clParamName, 
                                     size_t szParamValueSize, 
                                     void * pParamValue, 
                                     size_t * pszParamValueSizeRet)
{
    LOG_INFO(TEXT("Enter GetSamplerInfo (clSampler=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"), 
        clSampler, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

    cl_err_code clErr = CL_SUCCESS;
    SharedPtr<Sampler> pSampler = m_mapSamplers.GetOCLObject((_cl_sampler_int*)clSampler).DynamicCast<Sampler>();
    if (CL_FAILED(clErr) || NULL == pSampler)
    {
        LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %s"), clSampler, &pSampler, ClErrTxt(clErr));
        return CL_INVALID_SAMPLER;
    }

    clErr = pSampler->GetInfo((cl_int)clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
    return CL_ERR_OUT(clErr);
}

SharedPtr<Context> ContextModule::GetContext(cl_context clContext)
{
    return m_mapContexts.GetOCLObject((_cl_context_int*)clContext).DynamicCast<Context>();
}

SharedPtr<Kernel> ContextModule::GetKernel(cl_kernel clKernel)
{
    return m_mapKernels.GetOCLObject((_cl_kernel_int*)clKernel).DynamicCast<Kernel>();
}

SharedPtr<MemoryObject> ContextModule::GetMemoryObject(const cl_mem clMemObjId)
{
    return m_mapMemObjects.GetOCLObject((_cl_mem_int*)clMemObjId).DynamicCast<MemoryObject>();
}

cl_mem ContextModule::CreateFromGLBuffer(cl_context clContext, 
                          cl_mem_flags clMemFlags, 
                          GLuint glBufObj, 
                          int * pErrcodeRet)
{
#if defined (_WIN32)  //TODO GL support for Linux
	LOG_INFO(TEXT("Enter CreateFromGLBuffer (clContext=%d, clFlags=%d, pErrcodeRet=%d)"), 
		clContext, clMemFlags, pErrcodeRet);
    
    cl_err_code clErr = CheckMemObjectParameters(clMemFlags, NULL, CL_GL_OBJECT_BUFFER, 0, 0, 0, 0, 0, 0, NULL);
    if (CL_FAILED(clErr))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }
    SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)clContext).DynamicCast<Context>();
    if (NULL == pContext)
    {
        LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d) = NULL"), clContext);
            if (NULL != pErrcodeRet)
            {
                *pErrcodeRet = CL_INVALID_CONTEXT;
            }
            return CL_INVALID_HANDLE;
    }

    SharedPtr<GLContext> pGLContext = pContext.DynamicCast<GLContext>();
    if (NULL == pGLContext)
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }
    if (CL_MEM_READ_ONLY != clMemFlags && CL_MEM_WRITE_ONLY != clMemFlags && CL_MEM_READ_WRITE != clMemFlags)
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }
    SharedPtr<MemoryObject> pBuffer;
    clErr = pGLContext->CreateGLBuffer(clMemFlags, glBufObj, &pBuffer);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("pGLContext->CreateGLBuffer(%d, %d, %d, %d) = %s"), clMemFlags, glBufObj, &pBuffer, ClErrTxt(clErr))
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_ERR_OUT(clErr);
        }
        return CL_INVALID_HANDLE;
    }
    clErr = m_mapMemObjects.AddObject(pBuffer, false);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %s"), pBuffer, pBuffer->GetHandle(), ClErrTxt(clErr))
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_ERR_OUT(clErr);
        }
        return CL_INVALID_HANDLE;
    }
    if (NULL != pErrcodeRet)
    {
        *pErrcodeRet = CL_SUCCESS;
    }
    return pBuffer->GetHandle();
#else
    assert (0 && "NOT Implemented on Linux");
  return CL_INVALID_HANDLE; 
#endif
}

cl_mem_object_type ConvertGLTargetToCLObject(GLenum glTextureTarget)
{
#if defined(WIN32)
    switch(glTextureTarget)
    {
    case GL_TEXTURE_1D:
        return CL_GL_OBJECT_TEXTURE1D;
    case GL_TEXTURE_BUFFER:
        return CL_GL_OBJECT_TEXTURE_BUFFER;
    
    case GL_TEXTURE_2D:case GL_TEXTURE_RECTANGLE:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X: case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z: case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y: case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        return CL_GL_OBJECT_TEXTURE2D;

    case GL_TEXTURE_3D:
        return CL_GL_OBJECT_TEXTURE3D;

    case GL_TEXTURE_1D_ARRAY:
        return CL_GL_OBJECT_TEXTURE1D_ARRAY;

    case GL_TEXTURE_2D_ARRAY:
        return CL_GL_OBJECT_TEXTURE2D_ARRAY;
    }

    return CL_INVALID_GL_OBJECT;
#else // WIN32
    return CL_INVALID_GL_OBJECT;
#endif // WIN32
}

cl_mem ContextModule::CreateFromGLTexture(cl_context clContext, 
                             cl_mem_flags clMemFlags, 
                             GLenum glTextureTarget, 
                             GLint glMipLevel, 
                             GLuint glTexture, 
                             cl_int * pErrcodeRet)
{
#if defined (_WIN32)  //TODO GL support for Linux
	LOG_INFO(TEXT("Enter params(clContext=%x, clFlags=%x, glTextureTarget=%d, glMipLevel=%d, glTexture=%d)"), 
		clContext, clMemFlags, glTextureTarget, glMipLevel, glTexture);

    cl_mem_object_type clObjType = ConvertGLTargetToCLObject(glTextureTarget);
    cl_err_code clErr = CheckMemObjectParameters(clMemFlags, NULL, clObjType, 0, 0, 0, 0, 0, 0, NULL);
    if (CL_FAILED(clErr))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }
    SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)clContext).DynamicCast<Context>();
    if (NULL == pContext)
    {
        LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d) = NULL"), clContext);
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }

    SharedPtr<GLContext> pGLContext = pContext.DynamicCast<GLContext>();
    if (NULL == pGLContext)
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }
    if (CL_MEM_READ_ONLY != clMemFlags && CL_MEM_WRITE_ONLY != clMemFlags && CL_MEM_READ_WRITE != clMemFlags)
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }
    SharedPtr<MemoryObject> pMemObj;
    clErr = pGLContext->CreateGLTexture(clMemFlags, glTextureTarget, glMipLevel, glTexture, clObjType, &pMemObj);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("pGLContext->CreateGLTexture2D(%d, %d, %d, %d, %d) = %s"), clMemFlags, glTextureTarget, glMipLevel, glTexture,
            &pMemObj, ClErrTxt(clErr))
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_ERR_OUT(clErr);
        }
        return CL_INVALID_HANDLE;
    }
    clErr = m_mapMemObjects.AddObject(pMemObj, false);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %s"), pMemObj, pMemObj->GetHandle(), ClErrTxt(clErr))
            if (NULL != pErrcodeRet)
            {
                *pErrcodeRet = CL_ERR_OUT(clErr);
            }
            return CL_INVALID_HANDLE;
    }
    if (NULL != pErrcodeRet)
    {
        *pErrcodeRet = CL_SUCCESS;
    }
    return pMemObj->GetHandle();
#else
    assert (0 && "NOT Implemented on Linux");
  return CL_INVALID_HANDLE; 
#endif
}

cl_mem ContextModule::CreateFromGLRenderbuffer(cl_context clContext, 
                                cl_mem_flags clMemFlags, 
                                GLuint glRenderBuffer, 
                                cl_int * pErrcodeRet)
{
#if defined (_WIN32)  //TODO GL support for Linux
	LOG_INFO(TEXT("Enter CreateFromGLRenderbuffer (clContext=%d, clFlags=%d, glRenderBuffer=%d, pErrcodeRet=%d)"), 
		clContext, clMemFlags, glRenderBuffer, pErrcodeRet);

    SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)clContext).DynamicCast<Context>();
    if (NULL == pContext)
    {
        LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d) = NULL"), clContext);
            if (NULL != pErrcodeRet)
            {
                *pErrcodeRet = CL_INVALID_CONTEXT;
            }
            return CL_INVALID_HANDLE;
    }

    SharedPtr<GLContext> pGLContext = pContext.DynamicCast<GLContext>();
    if (NULL == pGLContext)
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }
    if (CL_MEM_READ_ONLY != clMemFlags && CL_MEM_WRITE_ONLY != clMemFlags && CL_MEM_READ_WRITE != clMemFlags)
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }
    SharedPtr<MemoryObject> pMemObj;
    cl_err_code clErr = pGLContext->CreateGLRenderBuffer(clMemFlags, glRenderBuffer, &pMemObj);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("pGLContext->CreateGLRenderBuffer(%d, %d, %d) = %s"), clMemFlags, glRenderBuffer, &pMemObj, ClErrTxt(clErr))
            if (NULL != pErrcodeRet)
            {
                *pErrcodeRet = CL_ERR_OUT(clErr);
            }
            return CL_INVALID_HANDLE;
    }
    clErr = m_mapMemObjects.AddObject(pMemObj, false);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %s"), pMemObj, pMemObj->GetHandle(), ClErrTxt(clErr))
            if (NULL != pErrcodeRet)
            {
                *pErrcodeRet = CL_ERR_OUT(clErr);
            }
            return CL_INVALID_HANDLE;
    }
    if (NULL != pErrcodeRet)
    {
        *pErrcodeRet = CL_SUCCESS;
    }
    return pMemObj->GetHandle();
#else
    assert (0 && "NOT Implemented on Linux");
  return CL_INVALID_HANDLE; 
#endif
}

cl_int ContextModule::GetGLObjectInfo(cl_mem clMemObj, 
                       cl_gl_object_type * pglObjectType, 
                       GLuint * pglObjectName)
{
#if defined (_WIN32)  //TODO GL support for Linux
    LOG_DEBUG(TEXT("Enter GetGLObjectInfo (clMemObj=%d, pglObjectType=%d, pglObjectName=%d)"), 
        clMemObj, pglObjectType, pglObjectName);

    // get program from programs map list
    SharedPtr<MemoryObject> pMemObj = m_mapMemObjects.GetOCLObject((_cl_mem_int*)clMemObj).DynamicCast<MemoryObject>();
    if (NULL == pMemObj)
    {
        LOG_ERROR(TEXT("Object %d isn't a valid object"), clMemObj);
        return CL_INVALID_MEM_OBJECT;
    }

    SharedPtr<GLMemoryObject> pGLObject = pMemObj.DynamicCast<GLMemoryObject>();
    // Check for GL object
    // Check if it's a GL object
    if (pGLObject)
    {
        return pGLObject->GetGLObjectInfo(pglObjectType, pglObjectName);
    }
    
    return CL_INVALID_GL_OBJECT;
#else
    assert (0 && "NOT Implemented on Linux");
  return CL_INVALID_GL_OBJECT; 
#endif
}

cl_int ContextModule::GetGLTextureInfo(cl_mem clMemObj, 
                                       cl_gl_texture_info clglPramName, 
                                       size_t szParamValueSize, 
                                       void * pParamValue, 
                                       size_t * pszParamValueSizeRet)
{
#if defined (_WIN32)  //TODO GL support for Linux
    LOG_DEBUG(TEXT("Enter GetGLTextureInfo (clMemObj=%d, cl_gl_texture_info=%d)"), 
        clMemObj, clglPramName);

    // get program from programs map list
    SharedPtr<MemoryObject> pMemObj = m_mapMemObjects.GetOCLObject((_cl_mem_int*)clMemObj).DynamicCast<MemoryObject>();
    if (NULL == pMemObj)
    {
        LOG_ERROR(TEXT("Object %d isn't a valid object"), clMemObj);
        return CL_INVALID_MEM_OBJECT;
    }

    SharedPtr<GLMemoryObject> pGLObject = pMemObj.DynamicCast<GLMemoryObject>();
    // Check for GL object
    // Check if it's a GL object
    if (pGLObject)
    {
        return pGLObject->GetGLTextureInfo(clglPramName, szParamValueSize, pParamValue, pszParamValueSizeRet);
    }

    return CL_INVALID_GL_OBJECT;
#else
    assert (0 && "NOT Implemented on Linux");
    return CL_INVALID_GL_OBJECT;
#endif
}

cl_err_code ContextModule::CheckMemObjectParameters(cl_mem_flags clMemFlags,
                                        const cl_image_format * clImageFormat,
                                        cl_mem_object_type clMemObjType,
                                         size_t szImageWidth,
                                         size_t szImageHeight,
                                         size_t szImageDepth,
                                         size_t szImageRowPitch,
                                         size_t szImageSlicePitch,
                                         size_t szArraySize,
                                         void * pHostPtr)
{
    // check for illegal flags
    if ((clMemFlags & ~(
        CL_MEM_READ_WRITE |
        CL_MEM_WRITE_ONLY |
        CL_MEM_READ_ONLY |
        CL_MEM_USE_HOST_PTR |
        CL_MEM_ALLOC_HOST_PTR |
        CL_MEM_COPY_HOST_PTR |
        CL_MEM_HOST_WRITE_ONLY |
        CL_MEM_HOST_READ_ONLY |
        CL_MEM_HOST_NO_ACCESS)) != 0)
    {
        return CL_INVALID_VALUE;
    }
    // check for illegal flag combinations
    if ( ((clMemFlags & CL_MEM_READ_ONLY) && (clMemFlags & CL_MEM_WRITE_ONLY)) ||
        ((clMemFlags & CL_MEM_READ_ONLY) && (clMemFlags & CL_MEM_READ_WRITE)) ||
        ((clMemFlags & CL_MEM_WRITE_ONLY) && (clMemFlags & CL_MEM_READ_WRITE))||
        ((clMemFlags & CL_MEM_USE_HOST_PTR) && (clMemFlags & CL_MEM_ALLOC_HOST_PTR)) ||
        ((clMemFlags & CL_MEM_HOST_WRITE_ONLY) && (clMemFlags & CL_MEM_HOST_READ_ONLY)) ||
        ((clMemFlags & CL_MEM_HOST_WRITE_ONLY) && (clMemFlags & CL_MEM_HOST_NO_ACCESS)) ||
        ((clMemFlags & CL_MEM_HOST_READ_ONLY) && (clMemFlags & CL_MEM_HOST_NO_ACCESS))
        )
    {
        return CL_INVALID_VALUE;
    }

    if ( (NULL == pHostPtr) && ((0 != szImageRowPitch) ||(0 != szImageSlicePitch)) )
    {
        return CL_INVALID_IMAGE_DESCRIPTOR;
     }

    if ( (NULL == pHostPtr) && ((CL_MEM_COPY_HOST_PTR|CL_MEM_USE_HOST_PTR)&clMemFlags) )
    {
        return CL_INVALID_HOST_PTR;
    }

	if (CL_MEM_OBJECT_IMAGE1D_BUFFER != clMemObjType && CL_MEM_OBJECT_IMAGE2D != clMemObjType && (NULL != pHostPtr) && !((CL_MEM_COPY_HOST_PTR|CL_MEM_USE_HOST_PTR)&clMemFlags) )
	{
		return CL_INVALID_HOST_PTR;
	}

    if (NULL != clImageFormat)
    {
        // Check if channel order and data type are in the range of valid numbers, see cl.h
        if ( !(clImageFormat->image_channel_order >= CL_R &&
               clImageFormat->image_channel_order < CL_SNORM_INT8 &&
               clImageFormat->image_channel_data_type >= CL_SNORM_INT8 &&
               clImageFormat->image_channel_data_type < CL_MEM_OBJECT_BUFFER) ) {
                return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
        }

        size_t pixelBytesCnt = clGetPixelBytesCount(clImageFormat);
        if (0 == pixelBytesCnt)
        {
            return CL_IMAGE_FORMAT_NOT_SUPPORTED;
        }
        // Check minimum row pitch size
        size_t szMinRowPitchSize = szImageWidth * pixelBytesCnt;
        if ( (NULL != pHostPtr) && (0 != szImageRowPitch) && ((szImageRowPitch<szMinRowPitchSize)||(szImageRowPitch % pixelBytesCnt)) )
        {
            return CL_INVALID_IMAGE_DESCRIPTOR;
        }
        // in 1D image array there is no row pitch, just slice pitch
        const size_t szRealRowPitch = 0 == szImageRowPitch || CL_MEM_OBJECT_IMAGE1D_ARRAY == clMemObjType ? szMinRowPitchSize : szImageRowPitch;
        const size_t szMinSlicePitchSize = CL_MEM_OBJECT_IMAGE1D_ARRAY == clMemObjType ? szRealRowPitch : szRealRowPitch * szImageHeight;
        if ( (NULL != pHostPtr) && (0 != szImageSlicePitch) && ((szImageSlicePitch<szMinSlicePitchSize)||(szImageRowPitch != 0 && szImageSlicePitch % szImageRowPitch)) )
        {
            return CL_INVALID_IMAGE_DESCRIPTOR;
        }
    }

    return CL_SUCCESS;
}

cl_err_code ContextModule::CheckContextSpecificParameters(SharedPtr<Context>pContext,
										const cl_mem_object_type image_type,
										const size_t image_width,
										const size_t image_height,
										const size_t image_depth,
										const size_t array_size,
										const void* pImgBufferHostPtr,
                                        cl_mem_flags bufFlags)
{
    size_t maxW = (size_t)-1;
    size_t maxH = (size_t)-1;
    size_t maxD = (size_t)-1;
    size_t maxArraySize        = (size_t)-1;
    size_t max1dFromBuffer = (size_t)-1;
    bool   isArray = (CL_MEM_OBJECT_IMAGE1D_ARRAY == image_type || CL_MEM_OBJECT_IMAGE2D_ARRAY == image_type);

    const tSetOfDevices *rootDevices = pContext->GetAllRootDevices();

    for (tSetOfDevices::const_iterator devIt = rootDevices->begin() ;
            devIt != rootDevices->end() ; ++devIt)
    {
        size_t sz;
        SharedPtr<Device> dev = *devIt;

        if (CL_MEM_OBJECT_IMAGE3D == image_type)
        {
            dev->GetInfo(CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(size_t), &sz, NULL);
        } else {
            // also applies to 1D image width not created from buffer
            dev->GetInfo(CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t), &sz, NULL);
        }
        maxW = maxW > sz ? sz : maxW;

        if (CL_MEM_OBJECT_IMAGE3D == image_type)
        {
            dev->GetInfo(CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(size_t), &sz, NULL);
        } else {
            dev->GetInfo(CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &sz, NULL);
        }
        maxH = maxH > sz ? sz : maxH;

        if (CL_MEM_OBJECT_IMAGE3D == image_type)
        {
            dev->GetInfo(CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(size_t), &sz, NULL);
            maxD = maxD > sz ? sz : maxD;
        }

        if (CL_MEM_OBJECT_IMAGE1D_BUFFER == image_type)
        {
            dev->GetInfo(CL_DEVICE_IMAGE_MAX_BUFFER_SIZE, sizeof(size_t), &sz, NULL);
            max1dFromBuffer = max1dFromBuffer > sz ? sz : max1dFromBuffer;
        }

		if (isArray)
		{
			dev->GetInfo(CL_DEVICE_IMAGE_MAX_ARRAY_SIZE, sizeof(size_t), &sz, NULL);
			maxArraySize = maxArraySize > sz ? sz : maxArraySize;
		}

        if (NULL != pImgBufferHostPtr && CL_MEM_OBJECT_IMAGE2D == image_type && (bufFlags & CL_MEM_USE_HOST_PTR))
		{
			cl_uint uiImgBaseAddrAlign;
			dev->GetInfo(CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT, sizeof(uiImgBaseAddrAlign), &uiImgBaseAddrAlign, NULL);
			if (!IS_ALIGNED_ON(pImgBufferHostPtr, uiImgBaseAddrAlign))
			{
				return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
			}
		}
	}

    // Check (the minimum of) maximum sizes and return CL_INVALID_IMAGE_SIZE if exceeding it.
    switch (image_type)
    {
        case CL_MEM_OBJECT_IMAGE3D:
            if (image_depth > maxD) return CL_INVALID_IMAGE_SIZE;

        case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        case CL_MEM_OBJECT_IMAGE2D:
            if (image_height > maxH) return CL_INVALID_IMAGE_SIZE;

        case CL_MEM_OBJECT_IMAGE1D:
            // for 1D NOT created from a buffer, use CL_DEVICE_IMAGE2D_MAX_WIDTH
            if (image_width > maxW) return CL_INVALID_IMAGE_SIZE;
            break;
        default:
            // all other cases covered outside the switch.
            break;
    }

    if (isArray)
    {
        if (array_size > maxArraySize) return CL_INVALID_IMAGE_SIZE;
    }

    if ( CL_MEM_OBJECT_IMAGE1D_BUFFER == image_type)
    {
        // for 1D created from a buffer, use CL_DEVICE_IMAGE_MAX_BUFFER_SIZE
        if (image_width > max1dFromBuffer) return CL_INVALID_IMAGE_SIZE;
    }

    return CL_SUCCESS;
}

#if defined (DX_MEDIA_SHARING)

cl_mem ContextModule::CreateFromD3D9Surface(cl_context context, cl_mem_flags flags, cl_dx9_media_adapter_type_khr adapterType, cl_dx9_surface_info_khr* pSurfaceInfo,
                                            UINT plane, cl_int *errcode_ret, const ID3DSharingDefinitions& d3d9Definitions)
{
    LOG_DEBUG(TEXT("Enter CreateFromD3D9Surface(context=%p, flags=%d, adapterType=%d, pSurfaceInfo=%p, plane=%d, errcode_ret=%p)"),
        context, flags, adapterType, pSurfaceInfo, plane, errcode_ret);

    SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)context).DynamicCast<Context>();
    if (NULL == pContext)
    {
        LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d) = %d"), context, pContext);
        if (NULL != errcode_ret)
        {
            *errcode_ret = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }
    
    SharedPtr<D3D9Context> pD3D9Context = pContext.DynamicCast<D3D9Context>();
    if (NULL == pD3D9Context)
    {
        if (NULL != errcode_ret)
        {
            *errcode_ret = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }
    if (d3d9Definitions.GetVersion() != pD3D9Context->GetD3dDefinitions().GetVersion())
    {
        LOG_ERROR(TEXT("The API call and the context are from different versions of the extension"));
        if (NULL != errcode_ret)
        {
            *errcode_ret = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }
    if (NULL == pSurfaceInfo || NULL == pSurfaceInfo->resource)
    {
        LOG_ERROR(TEXT("pSurfaceInfo or resource is NULL"));
        if (NULL != errcode_ret)
        {
            *errcode_ret = pD3D9Context->GetD3dDefinitions().GetInvalidResource();
        }
        return CL_INVALID_HANDLE;
    }
    D3DResourceInfo<IDirect3DResource9>* const pResourceInfo = new D3D9SurfaceResourceInfo(pSurfaceInfo->resource, pSurfaceInfo->shared_handle, plane, adapterType);
    if (NULL == pResourceInfo)
    {
        LOG_ERROR(TEXT("could not allocate D3DResourceInfo"));
        if (NULL != errcode_ret)
        {
            *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        }
        return CL_INVALID_HANDLE;
    }
    D3DSURFACE_DESC desc;
    HRESULT res = pSurfaceInfo->resource->GetDesc(&desc);
    assert(D3D_OK == res);
    // planar surfaces
    if (MAKEFOURCC('N', 'V', '1', '2') == desc.Format)
    {
        if (plane > 1)
        {
            LOG_ERROR(TEXT("invalid plane for format"));
            if (NULL != errcode_ret)
            {
                *errcode_ret = CL_INVALID_VALUE;
            }
            delete pResourceInfo;
            return CL_INVALID_HANDLE;
        }
        return CreateFromD3DResource<IDirect3DResource9, IDirect3DDevice9>(context, flags, pResourceInfo, errcode_ret, CL_DX9_OBJECT_SURFACE, 2, plane);
    }
    if (MAKEFOURCC('Y', 'V', '1', '2') == desc.Format)
    {
        if (plane > 2)
        {
            LOG_ERROR(TEXT("invalid plane for format"));
            if (NULL != errcode_ret)
            {
                *errcode_ret = CL_INVALID_VALUE;
            }
            delete pResourceInfo;
            return CL_INVALID_HANDLE;
        }
        return CreateFromD3DResource<IDirect3DResource9, IDirect3DDevice9>(context, flags, pResourceInfo, errcode_ret, CL_DX9_OBJECT_SURFACE, 2, plane);
    }
    // non-planar surface
    if (0 != plane)
    {
        LOG_ERROR(TEXT("invalid plane"));
        if (NULL != errcode_ret)
        {
            *errcode_ret = CL_INVALID_VALUE;
        }
        delete pResourceInfo;
        return CL_INVALID_HANDLE;
    }
    return CreateFromD3DResource<IDirect3DResource9, IDirect3DDevice9>(context, flags, pResourceInfo, errcode_ret, CL_DX9_OBJECT_SURFACE, 2, MAXUINT);
}

cl_mem ContextModule::CreateFromD3D11Buffer(cl_context context, cl_mem_flags flags, ID3D11Buffer* pResource, cl_int* pErrcodeRet)
{
    LOG_DEBUG(TEXT("Enter CreateFromD3D11Buffer(context=%p, flags=%d, resource=%p, errcode_ret=%d)"), context, flags, pResource, pErrcodeRet);
    
    D3DResourceInfo<ID3D11Resource>* const pResourceInfo = new D3DResourceInfo<ID3D11Resource>(pResource);
    if (NULL == pResourceInfo)
    {
        LOG_ERROR(TEXT("could not allocate D3DResourceInfo"));
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_OUT_OF_HOST_MEMORY;
        }
        return CL_INVALID_HANDLE;
    }
    return CreateFromD3DResource<ID3D11Resource, ID3D11Device>(context, flags, pResourceInfo, pErrcodeRet, CL_D3D11_OBJECT_BUFFER, 1);
}

cl_mem ContextModule::CreateFromD3D11Texture2D(cl_context context, cl_mem_flags flags, ID3D11Texture2D* pResource, UINT uiSubresource, cl_int* pErrcodeRet)
{
    LOG_DEBUG(TEXT("CreateFromD3D11Texture2DKHR(context=%p, flags=%d, pResource=%p, uiSubresource=%d, pErrcodeRet=%p)"), context, flags, pResource, uiSubresource,
        pErrcodeRet);
    
    D3D11TextureResourceInfo* const pResourceInfo = new D3D11TextureResourceInfo(pResource, uiSubresource);
    if (NULL == pResourceInfo)
    {
        LOG_ERROR(TEXT("could not allocate D3D11Texture2DResourceInfo"));
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_OUT_OF_HOST_MEMORY;
        }
        return CL_INVALID_HANDLE;
    }
    return CreateFromD3DResource<ID3D11Resource, ID3D11Device>(context, flags, pResourceInfo, pErrcodeRet, CL_D3D11_OBJECT_TEXTURE2D, 2);
}

cl_mem ContextModule::CreateFromD3D11Texture3D(cl_context context, cl_mem_flags flags, ID3D11Texture3D* pResource, UINT uiSubresource,
                                               cl_int* pErrcodeRet)
{
    LOG_DEBUG(TEXT("CreateFromD3D11Texture3D(context=%p, flags=%d, pResource=%p, uiSubresource=%d, pErrcodeRet=%p"), context, flags, pResource,
        uiSubresource, pErrcodeRet);

    D3D11TextureResourceInfo* const pResourceInfo = new D3D11TextureResourceInfo(pResource, uiSubresource);
    if (NULL == pResourceInfo)
    {
        LOG_ERROR(TEXT("could not allocate D3D11Texture3DResourceInfo"));
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_OUT_OF_HOST_MEMORY;
        }
        return CL_INVALID_HANDLE;
    }
    return CreateFromD3DResource<ID3D11Resource, ID3D11Device>(context, flags, pResourceInfo, pErrcodeRet, CL_D3D11_OBJECT_TEXTURE3D, 3);
}

#endif

/////////////////////////////////////////////////////////////////////
// OpenCL 1.2 functions
/////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// ContextModule::GetKernelArgInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetKernelArgInfo(cl_kernel clKernel,
                                        cl_uint argIndx,
                                        cl_kernel_arg_info paramName,
                                        size_t      szParamValueSize,
                                        void *      pParamValue,
                                        size_t *    pszParamValueSizeRet)
{
    LOG_INFO(TEXT("Enter clKernel=%X, argIndx=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%X, pszParamValueSizeRet=%X"), 
        clKernel, argIndx, paramName, szParamValueSize, pParamValue, pszParamValueSizeRet);

    cl_err_code clErr = CL_SUCCESS;
    SharedPtr<Kernel> pKernel = m_mapKernels.GetOCLObject((_cl_kernel_int*)clKernel).DynamicCast<Kernel>();
    if (NULL == pKernel)
    {
        LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %s"), clKernel, &pKernel, ClErrTxt(clErr));
        return CL_INVALID_KERNEL;
    }

    return pKernel->GetKernelArgInfo(argIndx, paramName, szParamValueSize, pParamValue, pszParamValueSizeRet);
}

void* ContextModule::SVMAlloc(cl_context context, cl_svm_mem_flags flags, size_t size, unsigned int uiAlignment)
{
	SharedPtr<Context> pContext = GetContext(context);
	if (pContext == NULL)
	{
		LOG_ERROR(TEXT("context is not a valid context"), "");
		return NULL;
	}
	if (flags & CL_MEM_SVM_ATOMICS && !(flags & CL_MEM_SVM_FINE_GRAIN_BUFFER))
	{
		LOG_ERROR(TEXT("flags does not contain CL_MEM_SVM_FINE_GRAIN_BUFFER but does contain CL_MEM_SVM_ATOMICS"), "");
		return NULL;
	}
	if ((flags & ~(CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY | CL_MEM_SVM_FINE_GRAIN_BUFFER | CL_MEM_SVM_ATOMICS)) != 0)
	{
		LOG_ERROR(TEXT("The values specified in flags are not valid i.e. don’t match those defined in table 5.13"), "");
		return NULL;
	}
	if (0 == size)
	{
		LOG_ERROR(TEXT("size is 0"), "");
		return NULL;
	}
	if (uiAlignment > 0 && (!IsPowerOf2(uiAlignment) || uiAlignment > sizeof(cl_long16)))
	{
		LOG_ERROR(TEXT("invalid alignment"), "");
		return NULL;
	}
	return pContext->SVMAlloc(flags, size, uiAlignment);	
}

void ContextModule::SVMFree(cl_context context, void* pSvmPtr)
{
	SharedPtr<Context> pContext = GetContext(context);
	if (pContext == NULL)
	{
		LOG_ERROR(TEXT("context is not a valid context"), "");
		return;
	}
	if (NULL == pSvmPtr)
	{
		LOG_INFO(TEXT("pSvmPtr is NULL"), "")
		return;
	}
	pContext->SVMFree(pSvmPtr);
}

//////////////////////////////////////////////////////////////////////////////
//
// Utility functions
//
/////////////////////////////////////////////////////////////////////////////
void ContextModule::CommandQueueCreated( OclCommandQueue* queue )
{
    m_setQueues.add( queue );
}

void ContextModule::CommandQueueRemoved( OclCommandQueue* queue )
{
    m_setQueues.remove( queue );
}

void ContextModule::RegisterMappedMemoryObject( MemoryObject* pMemObj )
{
    m_setMappedMemObjects.add( pMemObj );
}

void ContextModule::UnRegisterMappedMemoryObject( MemoryObject* pMemObj )
{
    m_setMappedMemObjects.remove( pMemObj );
}

