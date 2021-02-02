// INTEL CONFIDENTIAL
//
// Copyright 2008-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include <algorithm>

#include "framework_proxy.h"
#include "execution_module.h"
#include "platform_module.h"
#include "context_module.h"
#include "events_manager.h"
#include "command_queue.h"
#include "in_order_command_queue.h"
#include "out_of_order_command_queue.h"
#include "immediate_command_queue.h"
#include "user_event.h"
#include "Context.h"
#include "enqueue_commands.h"
#include "MemoryAllocator/MemoryObject.h"
#include "conversion_rules.h"
#include "GenericMemObj.h"
#include "svm_commands.h"
#include "usm_commands.h"
#include "svm_buffer.h"
#include "usm_buffer.h"

#include "kernel.h"
#include "Device.h"
#include "cl_sys_defines.h"

#include <CL/cl_ext.h>
#include <cassert>
#include <cl_objects_map.h>
#include <Logger.h>
#include <cl_local_array.h>
#include "cl_shared_ptr.hpp"
#include "device_queue.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;


#define SetIfZero(X,VALUE) {if ((X)==0) (X)=(VALUE);}
#define CheckIfAnyDimIsZero(X) (((X)[0] == 0) || ((X)[1] == 0) || ((X)[2] == 0))

namespace {
/**
 * Check mutex of map flags.
 * @return CL_SUCCESS if OK.
 */
inline cl_int checkMapFlagsMutex(const cl_map_flags clMapFlags);
/**
 * Callback to update kernle-event map if event status is changed to CL_COMPLETE.
 */
void callbackForKernelEventMap(cl_event pEvent, cl_int expectedStatus, void *data);
/**
 * Sync for the access of ExecutionModule::m_OclKernelEventMap
 */
Intel::OpenCL::Utils::OclMutex KernelEventMutex;

} //anonymous namespace

/******************************************************************
 * Constructor. Only assign pointers, for objects initilaztion use
 * Initialize function immediately. otherwise, the class behaviour
 * is undefined and function calls may crash the system.
 ******************************************************************/
ExecutionModule::ExecutionModule( PlatformModule *pPlatformModule, ContextModule* pContextModule ):
    m_pPlatfromModule(pPlatformModule),
    m_pContextModule(pContextModule),
    m_pOclCommandQueueMap(NULL),
    m_pEventsManager(NULL),
    m_pOclEntryPoints(NULL),
    m_pGPAData(NULL)
{
    INIT_LOGGER_CLIENT(TEXT("ExecutionModel"),LL_DEBUG);

    LOG_DEBUG(TEXT("%s"), TEXT("ExecutionModule created"));
}

/******************************************************************
 *
 ******************************************************************/
ExecutionModule::~ExecutionModule()
{
    for (auto e : m_OclKernelEventMap)
    {
        if (e.second)
        {
            delete e.second;
        }
    }
    m_OclKernelEventMap.clear();
    RELEASE_LOGGER_CLIENT;
}

/******************************************************************
 * This function initialize the execution modeule.
 * If this function fails, the object must be released.
 * If the caller will not release it, other function will terminate
 * the application.
 ******************************************************************/
cl_err_code ExecutionModule::Initialize(ocl_entry_points * pOclEntryPoints, OCLConfig * pOclConfig, ocl_gpa_data * pGPAData)
{
    m_pOclCommandQueueMap = new OCLObjectsMap<_cl_command_queue_int>();
    m_pEventsManager = new EventsManager();

    m_pOclEntryPoints = pOclEntryPoints;

    // initialize GPA data
    m_pGPAData = pGPAData;

    m_opencl_ver = pOclConfig->GetOpenCLVersion();

    m_enableParallelCopy = pOclConfig->EnableParallelCopy();

    if ( (NULL == m_pOclCommandQueueMap) || ( NULL == m_pEventsManager))
    {
        return CL_ERR_FAILURE;
    }
    return CL_SUCCESS;
}

cl_err_code ExecutionModule::Release(bool bTerminate)
{
    if ( bTerminate )
    {
        return CL_SUCCESS;
    }

    if ( NULL!=    m_pEventsManager )
    {
        delete m_pEventsManager;
        m_pEventsManager = NULL;
    }

    if ( NULL != m_pOclCommandQueueMap )
    {
        delete m_pOclCommandQueueMap;
        m_pEventsManager = NULL;
    }

    RELEASE_LOGGER_CLIENT;

    return CL_SUCCESS;
}

cl_err_code ExecutionModule::SetDefaultDeviceCommandQueue(
                cl_context            context,
                cl_device_id          device,
                cl_command_queue      command_queue
                )
{
    SharedPtr<Context> pContext = m_pContextModule->GetContext(context);
    if (NULL == pContext)
    {
        return CL_INVALID_CONTEXT;
    }

    SharedPtr<FissionableDevice> pDevice = pContext->GetDevice(device);
    if (NULL == pDevice)
    {
        return CL_INVALID_DEVICE;
    }

    SharedPtr<DeviceQueue> pCommandQueue = GetCommandQueue(command_queue).DynamicCast<DeviceQueue>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    cl_int ret_code = pCommandQueue->SetDefaultOnDevice(pDevice);
    if (CL_FAILED(ret_code))
    {
        return ret_code;
    }

    return CL_SUCCESS;
}
/******************************************************************
 *
 ******************************************************************/
cl_command_queue ExecutionModule::CreateCommandQueue(
    cl_context                  clContext,
    cl_device_id                clDevice,
    const cl_command_queue_properties* clQueueProperties,
    cl_int*                     pErrRet
    )
{
    cl_command_queue iQueueID   = CL_INVALID_HANDLE;
    SharedPtr<Context>    pContext   = NULL;
    cl_command_queue_properties queueProps = 0;
    cl_uint uiQueueSize;
    cl_int      errVal     = CheckCreateCommandQueueParams(clContext, clDevice, clQueueProperties, &pContext, queueProps, uiQueueSize);

    // If we are here, all parameters are valid, create the queue
    if( CL_SUCCEEDED(errVal))
    {
        SharedPtr<OclCommandQueue> pCommandQueue;
        if (queueProps & CL_QUEUE_THREAD_LOCAL_EXEC_ENABLE_INTEL)
        {
            pCommandQueue = ImmediateCommandQueue::Allocate(pContext, clDevice, queueProps, m_pEventsManager);
        }
        else
        {
            if (queueProps & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)
            {
                if (queueProps & CL_QUEUE_ON_DEVICE)
                {
                    if (queueProps & CL_QUEUE_ON_DEVICE_DEFAULT)
                    {
                        // Check if command queue already exist on device...
                        SharedPtr<FissionableDevice> pDevice = pContext->GetDevice(clDevice);
                        SharedPtr<OclCommandQueue> pDefaultQueue = pDevice->SetOrReturnDefaultQueue();
                        if(pDefaultQueue)
                        {
                            if (pErrRet) *pErrRet = errVal;
                            pDefaultQueue->Retain();
                            return pDefaultQueue->GetHandle();
                        }

                    }
                    pCommandQueue = DeviceQueue::Allocate(pContext, clDevice, queueProps, m_pEventsManager, queueProps & CL_QUEUE_PROFILING_ENABLE, queueProps & CL_QUEUE_ON_DEVICE_DEFAULT, uiQueueSize);
                }
                else
                {
                    pCommandQueue = OutOfOrderCommandQueue::Allocate(pContext, clDevice, queueProps, m_pEventsManager);
                }
            }
            else
            {
                pCommandQueue = InOrderCommandQueue::Allocate(pContext, clDevice, queueProps, m_pEventsManager);
            }
        }

        if ( NULL != pCommandQueue )
        {
            errVal = pCommandQueue->Initialize();
            if(CL_SUCCEEDED(errVal))
            {
                // TODO: guard ObjMap... better doing so inside the map
                m_pOclCommandQueueMap->AddObject(pCommandQueue);
                iQueueID = pCommandQueue->GetHandle();

                // this is the first place where we are sure that the commmand queue was created
                errVal = pCommandQueue->GPA_InitializeQueue();
            }
            else
            {
                pCommandQueue->Release();
            }
        }
        else
        {
            errVal = CL_OUT_OF_HOST_MEMORY;
        }
    }
    if (pErrRet) *pErrRet = errVal;

    return iQueueID;
}

static cl_err_code ParseQueueProperties(const cl_command_queue_properties* clQueueProperties, cl_command_queue_properties& queueProps, cl_uint& uiQueueSize,
    const ConstSharedPtr<FissionableDevice>& pDev)
{
    const cl_command_queue_properties* currProperties = clQueueProperties;
    bool bQueueSizeSpecified = false;

    while (0 != *currProperties)
    {
        const cl_command_queue_properties name = *(currProperties++);
        const cl_command_queue_properties val = *currProperties;
        switch (name)
        {
        case CL_QUEUE_PROPERTIES:
            queueProps = val;
            if (val & ~(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_THREAD_LOCAL_EXEC_ENABLE_INTEL | CL_QUEUE_ON_DEVICE | CL_QUEUE_ON_DEVICE_DEFAULT) ||
                ((val & CL_QUEUE_ON_DEVICE) && !(val & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)) ||
                ((val & CL_QUEUE_ON_DEVICE_DEFAULT) && !(val & CL_QUEUE_ON_DEVICE)))
            {
                return CL_INVALID_VALUE;
            }
            break;
        case CL_QUEUE_SIZE:
            {
                cl_uint uiMaxQueueSize;
                const cl_err_code err = pDev->GetInfo(CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE, sizeof(uiMaxQueueSize), &uiMaxQueueSize, NULL);

                if (CL_FAILED(err))
                {
                    return err;
                }
                bQueueSizeSpecified = true;
                uiQueueSize = val;
                if (uiQueueSize > uiMaxQueueSize)
                {
                    return CL_INVALID_QUEUE_PROPERTIES;
                }
            }
            break;
        default:
            return CL_INVALID_VALUE;
        }
        currProperties++;
    }
    if (bQueueSizeSpecified && !(queueProps & CL_QUEUE_ON_DEVICE))
    {
        return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::CheckCreateCommandQueueParams( cl_context clContext, cl_device_id clDevice, const cl_command_queue_properties* clQueueProperties, SharedPtr<Context>* ppContext,
                                                            cl_command_queue_properties& queueProps, cl_uint& uiQueueSize)
{
    *ppContext = m_pContextModule->GetContext(clContext);
    if (NULL == *ppContext)
    {
        return CL_INVALID_CONTEXT;
    }

    SharedPtr<FissionableDevice> pDev = (*ppContext)->GetDevice(clDevice);
    if (NULL == pDev)
    {
        return CL_INVALID_DEVICE;
    }
    if (NULL == clQueueProperties)
    {
        queueProps = 0;    // default values
        return CL_SUCCESS;
    }
    // if CL_QUEUE_SIZE isn't specified, CL_DEVICE_PREFERRED_QUEUE_SIZE is used
    const cl_int errVal = pDev->GetInfo(CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE, sizeof(uiQueueSize), &uiQueueSize, NULL);
    if (CL_FAILED(errVal))
    {
        uiQueueSize = (cl_uint)-1;    // MIC doesn't support OpenCL 2.0, so for it the query fails
    }
    return ParseQueueProperties(clQueueProperties, queueProps, uiQueueSize, pDev);
}

/******************************************************************
 * This function returns a pointer to a command queue.
 * If the command queue is not available a NULL value is returned.
 ******************************************************************/
SharedPtr<OclCommandQueue> ExecutionModule::GetCommandQueue(cl_command_queue clCommandQueue)
{
    return m_pOclCommandQueueMap->GetOCLObject((_cl_command_queue_int*)clCommandQueue).StaticCast<OclCommandQueue>();
}

bool ExecutionModule::IsValidQueueHandle(cl_command_queue clCommandQueue)
{
    return NULL != m_pOclCommandQueueMap->GetOCLObject((_cl_command_queue_int*)clCommandQueue);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::RetainCommandQueue(cl_command_queue clCommandQueue)
{
    SharedPtr<OclCommandQueue> pCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    pCommandQueue->Retain();
    return  CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::ReleaseCommandQueue(cl_command_queue clCommandQueue)
{
    cl_err_code errCode = Flush(clCommandQueue);
    if ( CL_FAILED(errCode) )
    {
        return errCode;
    }
    errCode = m_pOclCommandQueueMap->ReleaseObject((_cl_command_queue_int*)clCommandQueue);
    if (0 != errCode)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::GetCommandQueueInfo( cl_command_queue clCommandQueue, cl_command_queue_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet )
{
    cl_err_code res = CL_SUCCESS;
    SharedPtr<OclCommandQueue> pOclCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pOclCommandQueue)
    {
        res = CL_INVALID_COMMAND_QUEUE;
    }
    else
    {
        res = pOclCommandQueue->GetInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
    }
    return res;
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::SetCommandQueueProperty ( cl_command_queue clCommandQueue, cl_command_queue_properties clProperties, cl_bool bEnable, cl_command_queue_properties* pclOldProperties)
{
    SharedPtr<OclCommandQueue> pOclCommandQueue = GetCommandQueue(clCommandQueue);
    if (NULL == pOclCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    // Set old properties if not NULL
    if(NULL != pclOldProperties)
    {
        *pclOldProperties = 0x0;
        if( pOclCommandQueue->IsProfilingEnabled() )
        {
            *pclOldProperties |= CL_QUEUE_PROFILING_ENABLE;
        }
        if( pOclCommandQueue->IsOutOfOrderExecModeEnabled() )
        {
            *pclOldProperties |= CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
        }
    }

    if( (0 == clProperties) && ( CL_TRUE == bEnable))
    {
        // Disable all like in clProperties0 in clCreateQueue
        pOclCommandQueue->EnableProfiling( false );
        pOclCommandQueue->EnableOutOfOrderExecMode( false );
    }
    else
    {
        // Check that only properties that are defined by the spec are in use
        cl_command_queue_properties mask = ( CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE
                                           | CL_QUEUE_PROFILING_ENABLE );

        if ( ( clProperties & mask ) == 0 )
        {
            return CL_INVALID_VALUE;
        }

        mask ^= 0xFFFFFFFFFFFFFFFF; //mask = not(mask)

        if ( ( clProperties & mask ) != 0 )
        {
            return CL_INVALID_VALUE;
        }

        // Check that the queue supports those properties
        if( ! pOclCommandQueue->IsPropertiesSupported(clProperties) )
        {
            return CL_INVALID_QUEUE_PROPERTIES;
        }

        if(CL_QUEUE_PROFILING_ENABLE == (clProperties & CL_QUEUE_PROFILING_ENABLE))
        {
            // Asked to set profiling
            pOclCommandQueue->EnableProfiling( bEnable );
        }

        if( CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE == (clProperties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE))
        {
            // Asked to set execution mdoe
            pOclCommandQueue->EnableOutOfOrderExecMode( bEnable );
        }
    }
    return CL_SUCCESS;
}

/******************************************************************
 * On flush, the implementation always create a flush command
 * Enqueue
 ******************************************************************/
cl_err_code ExecutionModule::Flush ( cl_command_queue clCommandQueue )
{
    cl_start;
    cl_err_code res = CL_SUCCESS;
    SharedPtr<IOclCommandQueueBase> pOclCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pOclCommandQueue)
    {
        if (!IsValidQueueHandle(clCommandQueue))    // otherwise it's just a device queue, which isn't a IOclCommandQueueBase
        {
            res = CL_INVALID_COMMAND_QUEUE;
        }
    }
    else
    {
        res = pOclCommandQueue->Flush(true);
    }
    cl_return res;
}

/******************************************************************
 * This is a blocking function, works like flush, but since
 * it is blocking, the OclQueue handles the flush and wait mechanism
 ******************************************************************/
cl_err_code ExecutionModule::Finish ( cl_command_queue clCommandQueue)
{
    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    return Finish(pCommandQueue);
}

cl_err_code ExecutionModule::Finish ( const SharedPtr<IOclCommandQueueBase>& pCommandQueue)
{
    cl_err_code res = CL_SUCCESS;
    cl_event dummy = NULL;

    res = EnqueueMarker(pCommandQueue, &dummy, NULL);
    if (CL_FAILED(res))
    {
        return res;
    }

    SharedPtr<QueueEvent> pQueueEvent = m_pEventsManager->GetEventClass<QueueEvent>(dummy);
    assert(pQueueEvent && "Expecting non NULL dummy-queue event");
    if (NULL == pQueueEvent)
    {
        return CL_INVALID_VALUE;
    }
    res = pCommandQueue->WaitForCompletion(pQueueEvent);
    if ( CL_FAILED(res) )
    {
        pQueueEvent->Wait();
    }
    m_pEventsManager->ReleaseEvent(dummy);
    return CL_SUCCESS;
}

/**
 * @fn cl_err_code ExecutionModule::EnqueueMarkerWithWaitList(cl_command_queue clCommandQueue, cl_uint uiNumEvents, const cl_event* pEventList, cl_event* pEvent)
 */
cl_err_code ExecutionModule::EnqueueMarkerWithWaitList(cl_command_queue clCommandQueue, cl_uint uiNumEvents, const cl_event* pEventList, cl_event* pEvent, ApiLogger* pApiLogger)
{
    SharedPtr<IOclCommandQueueBase> const pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    return EnqueueMarkerWithWaitList(  pCommandQueue, uiNumEvents, pEventList, pEvent, pApiLogger);
}

cl_err_code ExecutionModule::EnqueueMarkerWithWaitList(const SharedPtr<IOclCommandQueueBase>& pCommandQueue, cl_uint uiNumEvents, const cl_event* pEventList, cl_event* pEvent, ApiLogger* pApiLogger)
{
    if ((NULL == pEventList && uiNumEvents > 0) || (NULL != pEventList && 0 == uiNumEvents))
    {
        return CL_INVALID_EVENT_WAIT_LIST;
    }

    MarkerCommand* const pMarkerCommand = new MarkerCommand(pCommandQueue, uiNumEvents > 0);
    if (NULL == pMarkerCommand)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_err_code err = pMarkerCommand->Init();
    if (CL_FAILED(err))
    {
        delete pMarkerCommand;
        return err;
    }

    err = pCommandQueue->EnqueueRuntimeCommandWaitEvents(IOclCommandQueueBase::MARKER, pMarkerCommand, uiNumEvents, pEventList, pEvent, pApiLogger);
    if (CL_FAILED(err))
    {
        pMarkerCommand->CommandDone();
        delete pMarkerCommand;
    }
    return err;
}

/**
 * @fn cl_err_code ExecutionModule::EnqueueBarrierWithWaitList(cl_command_queue clCommandQueue, cl_uint uiNumEvents, const cl_event* pEventList, cl_event* pEvent)
 */
cl_err_code ExecutionModule::EnqueueBarrierWithWaitList(cl_command_queue clCommandQueue, cl_uint uiNumEvents, const cl_event* pEventList, cl_event* pEvent, ApiLogger* pApiLogger)
{
    SharedPtr<IOclCommandQueueBase> const pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    if ((NULL == pEventList && uiNumEvents > 0) || (NULL != pEventList && 0 == uiNumEvents))
    {
        return CL_INVALID_EVENT_WAIT_LIST;
    }

    BarrierCommand* const pBarrierCommand = new BarrierCommand(pCommandQueue, uiNumEvents > 0);
    if (NULL == pBarrierCommand)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_err_code err = pBarrierCommand->Init();
    if (CL_FAILED(err))
    {
        delete pBarrierCommand;
        return err;
    }

    err = pCommandQueue->EnqueueRuntimeCommandWaitEvents(IOclCommandQueueBase::BARRIER, pBarrierCommand, uiNumEvents, pEventList, pEvent, pApiLogger);
    if (CL_FAILED(err))
    {
        pBarrierCommand->CommandDone();
        delete pBarrierCommand;
    }
    return err;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueMarker(cl_command_queue clCommandQueue, cl_event *pEvent, ApiLogger* pApiLogger)
{
    return EnqueueMarkerWithWaitList(clCommandQueue, 0, NULL, pEvent, pApiLogger);
}

cl_err_code ExecutionModule::EnqueueMarker(const SharedPtr<IOclCommandQueueBase>& clCommandQueue, cl_event *pEvent, ApiLogger* pApiLogger)
{
    return EnqueueMarkerWithWaitList(clCommandQueue, 0, NULL, pEvent, pApiLogger);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueWaitForEvents(cl_command_queue clCommandQueue, cl_uint uiNumEvents, const cl_event* cpEventList, ApiLogger* apiLogger)
{
    cl_err_code errVal;
    if ( (NULL == cpEventList) || (0 == uiNumEvents) )
    {
        return CL_INVALID_VALUE;
    }

    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    // Create Command
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    Command* pWaitForEventsCommand = new WaitForEventsCommand(pCommandQueue, uiNumEvents > 0);
    if (NULL == pWaitForEventsCommand)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pWaitForEventsCommand->Init();
    if ( CL_FAILED(errVal) )
    {
        delete pWaitForEventsCommand;
        return errVal;
    }

    errVal = pCommandQueue->EnqueueRuntimeCommandWaitEvents(IOclCommandQueueBase::JUST_WAIT, pWaitForEventsCommand, uiNumEvents, cpEventList, NULL, apiLogger);
    if ( CL_FAILED(errVal) )
    {
        pWaitForEventsCommand->CommandDone();
        delete pWaitForEventsCommand;
    }

    return errVal;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueBarrier(cl_command_queue clCommandQueue, ApiLogger* pApiLogger)
{
    return EnqueueBarrierWithWaitList(clCommandQueue, 0, NULL, NULL, pApiLogger);
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::WaitForEvents( cl_uint uiNumEvents, const cl_event* cpEventList )
{
    cl_start;
    cl_err_code errVal = CL_SUCCESS;
    if ( 0 == uiNumEvents || NULL == cpEventList)
        return CL_INVALID_VALUE;

    // Validate event context
    SharedPtr<OclEvent> pEvent = m_pEventsManager->GetEventClass<OclEvent>(cpEventList[0]);
    if ( NULL == pEvent )
    {
        return CL_INVALID_EVENT_WAIT_LIST;
    }

    // This call is blocking
    errVal = m_pEventsManager->WaitForEvents(uiNumEvents, cpEventList);
    if ( CL_INVALID_EVENT_WAIT_LIST == errVal )
    {
        return CL_INVALID_EVENT;
    }
    cl_return errVal;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::GetEventInfo( cl_event clEvent, cl_event_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet )
{
    cl_err_code res = m_pEventsManager->GetEventInfo(clEvent, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
    return res;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::RetainEvent(cl_event clEvent)
{
    cl_err_code res = m_pEventsManager->RetainEvent(clEvent);
    if CL_FAILED(res)
    {
        res = CL_INVALID_EVENT;
    }
    return res;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::ReleaseEvent(cl_event clEvent)
{
    cl_err_code res = m_pEventsManager->ReleaseEvent(clEvent);
    return CL_FAILED(res) ? CL_INVALID_EVENT : CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
typedef std::list<SharedPtr<UserEvent> >  EventsListType;
void ExecutionModule::ReleaseAllUserEvents( bool preserve_user_handles)
{
    EventsListType           event_list;
    EventsListType::iterator event_list_it;
    EventsListType::iterator event_list_it_end;

    m_pEventsManager->DisableNewEvents();
    m_pEventsManager->GetAllEventClass<UserEvent>( event_list );
    m_pEventsManager->EnableNewEvents(); // finish uses this

    if (preserve_user_handles)
    {
        m_pEventsManager->SetPreserveUserHandles();
    }

    event_list_it_end  = event_list.end();
    for (event_list_it=event_list.begin(); event_list_it != event_list_it_end; ++event_list_it)
    {
        SharedPtr<UserEvent>& pUserEvent = *event_list_it;
        if (pUserEvent->GetEventExecState() != CL_COMPLETE)
        {
            pUserEvent->SetComplete(CL_DEVICE_NOT_AVAILABLE);
        }

        m_pEventsManager->ReleaseEvent( pUserEvent->GetHandle() );
    }
}

/******************************************************************
*
******************************************************************/
cl_event ExecutionModule::CreateUserEvent(cl_context context, cl_int * errcode_ret)
{
    cl_int   err = CL_SUCCESS;
    cl_event evt = (cl_event)0;
    //Validate the context is legit
    SharedPtr<Context> pContext = m_pContextModule->GetContext(context);
    if (NULL == pContext)
    {
        err = CL_INVALID_CONTEXT;
    }
    else
    {
        SharedPtr<UserEvent> pUserEvent  = m_pEventsManager->CreateEventClass<UserEvent>((_cl_context_int*)context);
        if (pUserEvent)
        {
            evt = pUserEvent->GetHandle();
        }
        else
        {
            err = CL_OUT_OF_HOST_MEMORY;
        }
    }

    if (NULL != errcode_ret)
    {
        *errcode_ret = err;
    }
    return evt;

}

/******************************************************************
*
******************************************************************/
cl_int ExecutionModule::SetUserEventStatus(cl_event evt, cl_int status)
{
    SharedPtr<UserEvent> pUserEvent = m_pEventsManager->GetEventClass<UserEvent>(evt);
    if (NULL == pUserEvent)
    {
        return CL_INVALID_EVENT;
    }

    if ((status != CL_COMPLETE) && (status > 0))
    {
        return CL_INVALID_VALUE;
    }

    if (pUserEvent->GetEventExecState() != CL_SUBMITTED)
    {
        return CL_INVALID_OPERATION;
    }

    pUserEvent->SetComplete(status);
    return CL_SUCCESS;
}
/******************************************************************
*
******************************************************************/
cl_err_code ExecutionModule::SetEventCallback(cl_event evt, cl_int status, void (CL_CALLBACK *fn)(cl_event, cl_int, void *), void *userData)
{
    return m_pEventsManager->SetEventCallBack(evt, status, fn, userData);
}

/**
 * @fn cl_err_code ExecutionModule::EnqueueMigrateMemObjects(cl_command_queue clCommandQueue, cl_uint uiNumMemObjects, const cl_mem* pMemObjects, cl_mem_migration_flags clFlags, cl_uint uiNumEventsInWaitList, const cl_event* pEventWaitList, cl_event* pEvent)
 */
cl_err_code ExecutionModule::EnqueueMigrateMemObjects(cl_command_queue clCommandQueue,
                                                      cl_uint uiNumMemObjects,
                                                      const cl_mem* pMemObjects,
                                                      cl_mem_migration_flags clFlags,
                                                      cl_uint uiNumEventsInWaitList,
                                                      const cl_event* pEventWaitList,
                                                      cl_event* pEvent, ApiLogger* apiLogger)
{
    if ((NULL == pEventWaitList && uiNumEventsInWaitList > 0) || (NULL != pEventWaitList && 0 == uiNumEventsInWaitList))
    {
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    if (0 == uiNumMemObjects || NULL == pMemObjects || 0 != (clFlags & ~(CL_MIGRATE_MEM_OBJECT_HOST | CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED)))
    {
        return CL_INVALID_VALUE;
    }

    SharedPtr<IOclCommandQueueBase> const pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    MigrateMemObjCommand* pMigrateCommand = new MigrateMemObjCommand(
                                        pCommandQueue, (ocl_entry_points*)((_cl_command_queue_int*)pCommandQueue->GetHandle())->dispatch,
                                        m_pContextModule,
                                        clFlags,
                                        uiNumMemObjects, pMemObjects );
    if (NULL == pMigrateCommand)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_err_code err = pMigrateCommand->Init();
    if (CL_FAILED(err))
    {
        delete pMigrateCommand;
        return err;
    }

    err = pMigrateCommand->EnqueueSelf(CL_FALSE, uiNumEventsInWaitList, pEventWaitList, pEvent, apiLogger);
    if (CL_FAILED(err))
    {
        // Enqueue failed, free resources
        pMigrateCommand->CommandDone();
        delete pMigrateCommand;
    }

    return err;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueReadBuffer(cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlocking, size_t szOffset, size_t szCb, void* pOutData, cl_uint uNumEventsInWaitList, const cl_event* cpEventWaitList, cl_event* pEvent, ApiLogger* apiLogger)
{
    cl_err_code errVal = CL_SUCCESS;
    if (NULL == pOutData)
    {
        return CL_INVALID_VALUE;
    }

    if (m_opencl_ver < OPENCL_VERSION_2_1 && 0 == szCb)
    {
        return CL_INVALID_VALUE;
    }

    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    SharedPtr<MemoryObject> pBuffer = m_pContextModule->GetMemoryObject(clBuffer);
    if (NULL == pBuffer)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pBuffer->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    if (pBuffer->GetFlags() & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_WRITE_ONLY) )
    {
        return CL_INVALID_OPERATION;
    }

    if (CL_SUCCESS != (errVal = pBuffer->CheckBounds(&szOffset, &szCb)))
    {
        // Out of bounds check.
        return errVal;
    }

    if ((NULL == cpEventWaitList && (0 < uNumEventsInWaitList)) || (cpEventWaitList && (0 == uNumEventsInWaitList)))
    {
        return CL_INVALID_EVENT_WAIT_LIST;
    }

    // Do parallel copy if offset is 0.
    // Not supported on FPGA because FPGA doesn't support system USM.
    if (m_enableParallelCopy && szOffset == 0 &&
        !pCommandQueue->GetContext()->IsFPGAEmulator()) {
        errVal = EnqueueLibraryCopy(pCommandQueue, pOutData, &clBuffer, szCb,
                                    false, true, false, false, bBlocking,
                                    uNumEventsInWaitList, cpEventWaitList,
                                    pEvent, apiLogger, CL_COMMAND_READ_BUFFER);
        if (CL_SUCCEEDED(errVal))
            return errVal;
    }

    const size_t pszOrigin[3] = {szOffset, 0 , 0};
    const size_t pszRegion[3] = {szCb, 1, 1};

    Command* pEnqueueReadBufferCmd = new ReadBufferCommand(pCommandQueue, m_pOclEntryPoints, pBuffer, pszOrigin, pszRegion, pOutData);
    if (NULL == pEnqueueReadBufferCmd)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pEnqueueReadBufferCmd->Init();
    if ( CL_FAILED(errVal) )
    {
        delete pEnqueueReadBufferCmd;
        return  errVal;
    }

    errVal = pEnqueueReadBufferCmd->EnqueueSelf(bBlocking, uNumEventsInWaitList, cpEventWaitList, pEvent, apiLogger);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pEnqueueReadBufferCmd->CommandDone();
        delete pEnqueueReadBufferCmd;
    }
    return  errVal;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueReadBufferRect(
                        cl_command_queue    clCommandQueue,
                        cl_mem                clBuffer,
                        cl_bool                bBlocking,
                        const size_t        szBufferOrigin[MAX_WORK_DIM],
                        const size_t        szHostOrigin[MAX_WORK_DIM],
                        const size_t        region[MAX_WORK_DIM],
                        size_t                buffer_row_pitch,
                        size_t                buffer_slice_pitch,
                        size_t                host_row_pitch,
                        size_t                host_slice_pitch,
                        void*                pOutData,
                        cl_uint                uNumEventsInWaitList,
                        const cl_event*        cpEventWaitList,
                        cl_event*            pEvent,
                        ApiLogger* apiLogger)
{
    cl_err_code errVal = CL_SUCCESS;

    if (NULL == pOutData || NULL == szBufferOrigin || NULL == szHostOrigin || NULL == region)
    {
        return CL_INVALID_VALUE;
    }

    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    SharedPtr<MemoryObject> pBuffer = m_pContextModule->GetMemoryObject(clBuffer);
    if (NULL == pBuffer)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pBuffer->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    if (pBuffer->GetFlags() & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_WRITE_ONLY) )
    {
        return CL_INVALID_OPERATION;
    }

    if (CheckIfAnyDimIsZero(region)                                                        ||
        (buffer_row_pitch    !=0 && buffer_row_pitch        <region[0])                        ||
        (host_row_pitch        !=0 && host_row_pitch        <region[0])                        ||
        (buffer_slice_pitch    !=0 && buffer_slice_pitch    <(region[1]*buffer_row_pitch))    ||
        (host_slice_pitch    !=0 && host_slice_pitch        <(region[1]*host_row_pitch))
        )
    {
        return CL_INVALID_VALUE;
    }

    SetIfZero(buffer_row_pitch        , region[0]);
    SetIfZero(host_row_pitch        , region[0]);
    SetIfZero(buffer_slice_pitch    , region[1] * buffer_row_pitch);
    SetIfZero(host_slice_pitch        , region[1] * host_row_pitch);

    if (CL_SUCCESS != (errVal = pBuffer->CheckBoundsRect(szBufferOrigin, region, buffer_row_pitch, buffer_slice_pitch)))
    {
        // Out of bounds check.
        return errVal;
    }

    // Is Sub-buffer
    if ( NULL != pBuffer->GetParent() )
    {
        if (!pBuffer->IsSupportedByDevice(pCommandQueue->GetDefaultDevice()))
        {
            return CL_MISALIGNED_SUB_BUFFER_OFFSET;
        }
    }


    Command* pEnqueueReadBufferRectCmd = new ReadBufferRectCommand(pCommandQueue, m_pOclEntryPoints, pBuffer, szBufferOrigin, szHostOrigin, region, buffer_row_pitch,
        buffer_slice_pitch, host_row_pitch, host_slice_pitch, pOutData);
    if(NULL == pEnqueueReadBufferRectCmd)
    {
         return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pEnqueueReadBufferRectCmd->Init();
    if ( CL_FAILED(errVal) )
    {
        delete pEnqueueReadBufferRectCmd;
        return  errVal;
    }

    errVal = pEnqueueReadBufferRectCmd->EnqueueSelf(bBlocking, uNumEventsInWaitList, cpEventWaitList, pEvent, apiLogger);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pEnqueueReadBufferRectCmd->CommandDone();
        delete pEnqueueReadBufferRectCmd;
    }
    return  errVal;
}
/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueWriteBuffer(cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlocking, size_t szOffset, size_t szCb, const void* cpSrcData, cl_uint uNumEventsInWaitList, const cl_event* cpEventWaitList, cl_event* pEvent, ApiLogger* apiLogger)
{
    cl_start;
    cl_err_code errVal = CL_SUCCESS;
    if (NULL == cpSrcData)
    {
        return CL_INVALID_VALUE;
    }

    if (m_opencl_ver < OPENCL_VERSION_2_1 && 0 == szCb)
    {
        return CL_INVALID_VALUE;
    }


    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    SharedPtr<MemoryObject> pBuffer = m_pContextModule->GetMemoryObject(clBuffer);
    if (NULL == pBuffer)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pBuffer->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    if (pBuffer->GetFlags() & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_READ_ONLY) )
    {
        return CL_INVALID_OPERATION;
    }

    if (CL_SUCCESS != (errVal = pBuffer->CheckBounds(&szOffset, &szCb)))
    {
        // Out of bounds check.
        return errVal;
    }

    if ((NULL == cpEventWaitList && (0 < uNumEventsInWaitList)) || (cpEventWaitList && (0 == uNumEventsInWaitList)))
    {
        return CL_INVALID_EVENT_WAIT_LIST;
    }

    // Do parallel copy if offset is 0.
    // Not supported on FPGA because FPGA doesn't support system USM.
    if (m_enableParallelCopy && szOffset == 0 &&
        !pCommandQueue->GetContext()->IsFPGAEmulator()) {
        errVal = EnqueueLibraryCopy(pCommandQueue, &clBuffer, cpSrcData, szCb,
                                    false, false, false, true, bBlocking,
                                    uNumEventsInWaitList, cpEventWaitList,
                                    pEvent, apiLogger, CL_COMMAND_WRITE_BUFFER);
        if (CL_SUCCEEDED(errVal))
            return errVal;
    }

    const size_t pszOrigin[3] = {szOffset, 0 , 0};
    const size_t pszRegion[3] = {szCb, 1, 1};

    const size_t largeBufferThreshold = 1024 * 1024;
    // If the buffer big enough, there is no dependencies and it is blocking command then copy immediately.
    const bool avoidBlock = (szCb > largeBufferThreshold) && (0 == uNumEventsInWaitList);

    Command* pWriteBufferCmd = new WriteBufferCommand(pCommandQueue, m_pOclEntryPoints, avoidBlock ? CL_FALSE : bBlocking, pBuffer, pszOrigin, pszRegion, cpSrcData);
    if (NULL == pWriteBufferCmd)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pWriteBufferCmd->Init();
    if ( CL_FAILED(errVal) )
    {
        delete pWriteBufferCmd;
        return  errVal;
    }

    errVal = pWriteBufferCmd->EnqueueSelf(avoidBlock ? bBlocking : CL_FALSE, uNumEventsInWaitList, cpEventWaitList, pEvent, apiLogger);
    if(CL_FAILED(errVal))
    {
        pWriteBufferCmd->CommandDone();
        delete pWriteBufferCmd;
    }

    return  errVal;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueWriteBufferRect(
                        cl_command_queue    clCommandQueue,
                        cl_mem                clBuffer,
                        cl_bool                bBlocking,
                        const size_t        szBufferOrigin[MAX_WORK_DIM],
                        const size_t        szHostOrigin[MAX_WORK_DIM],
                        const size_t        region[MAX_WORK_DIM],
                        size_t                buffer_row_pitch,
                        size_t                buffer_slice_pitch,
                        size_t                host_row_pitch,
                        size_t                host_slice_pitch,
                        const void*            pOutData,
                        cl_uint                uNumEventsInWaitList,
                        const cl_event*        cpEventWaitList,
                        cl_event*            pEvent,
                        ApiLogger* apiLogger)
{
    cl_start;
    cl_err_code errVal = CL_SUCCESS;

    if (NULL == pOutData || NULL == szBufferOrigin || NULL == szHostOrigin || NULL == region)
    {
        return CL_INVALID_VALUE;
    }

    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    SharedPtr<MemoryObject> pBuffer = m_pContextModule->GetMemoryObject(clBuffer);
    if (NULL == pBuffer)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pBuffer->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    if (pBuffer->GetFlags() & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_READ_ONLY) )
    {
        return CL_INVALID_OPERATION;
    }

    if (CheckIfAnyDimIsZero(region)                                                        ||
        (buffer_row_pitch    !=0 && buffer_row_pitch        <region[0])                        ||
        (host_row_pitch        !=0 && host_row_pitch        <region[0])                        ||
        (buffer_slice_pitch    !=0 && buffer_slice_pitch    <(region[1]*buffer_row_pitch))    ||
        (host_slice_pitch    !=0 && host_slice_pitch        <(region[1]*host_row_pitch))
        )
    {
        return CL_INVALID_VALUE;
    }

    SetIfZero(buffer_row_pitch        , region[0]);
    SetIfZero(host_row_pitch        , region[0]);
    SetIfZero(buffer_slice_pitch    , region[1] * buffer_row_pitch);
    SetIfZero(host_slice_pitch        , region[1] * host_row_pitch);


    if (CL_SUCCESS != (errVal = pBuffer->CheckBoundsRect(szBufferOrigin, region, buffer_row_pitch, buffer_slice_pitch)))
    {
        // Out of bounds check.
        return errVal;
    }

    if ( NULL != pBuffer->GetParent() )
    {
        if (!pBuffer->IsSupportedByDevice(pCommandQueue->GetDefaultDevice()))
        {
            return CL_MISALIGNED_SUB_BUFFER_OFFSET;
        }
    }


    Command* pWriteBufferRectCmd = new WriteBufferRectCommand(pCommandQueue, m_pOclEntryPoints, bBlocking, pBuffer, szBufferOrigin, szHostOrigin, region, buffer_row_pitch,
        buffer_slice_pitch, host_row_pitch, host_slice_pitch, pOutData);

    if (NULL == pWriteBufferRectCmd)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pWriteBufferRectCmd->Init();
    if ( CL_FAILED(errVal) )
    {
        delete pWriteBufferRectCmd;
        return  errVal;
    }

    errVal = pWriteBufferRectCmd->EnqueueSelf(CL_FALSE, uNumEventsInWaitList, cpEventWaitList, pEvent, apiLogger);
    if(CL_FAILED(errVal))
    {
        pWriteBufferRectCmd->CommandDone();
        delete pWriteBufferRectCmd;
    }
    cl_return  errVal;
}


static cl_err_code CheckImageFormatSupportedByDevice(const FissionableDevice& dev, const MemoryObject& image)
{
    cl_image_format clImgFormat;
    size_t szValSize;

    const cl_err_code errVal = image.GetImageInfo(CL_IMAGE_FORMAT, sizeof(clImgFormat), &clImgFormat, &szValSize);

    assert(CL_SUCCESS == errVal);
    if (CL_FAILED(errVal))
    {
        return errVal;
    }

    assert(sizeof(clImgFormat) == szValSize);

    if (!dev.IsImageFormatSupported(clImgFormat, image.GetFlags(), image.GetType()))
    {
        return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    }
    return CL_SUCCESS;
}

cl_err_code ExecutionModule::EnqueueFillBuffer (cl_command_queue clCommandQueue,
        cl_mem clBuffer,
        const void *pattern,
        size_t pattern_size,
        size_t offset,
        size_t size,
        cl_uint num_events_in_wait_list,
        const cl_event *event_wait_list,
        cl_event *pEvent,
        ApiLogger* apiLogger)
{
    cl_start;
    cl_err_code errVal = CL_SUCCESS;

    // Only accept powers of 2, up to 128 or vectors of size 3
    if (NULL == pattern || 0 >= pattern_size || 128 < pattern_size || (pattern_size % 3 != 0 && !IsPowerOf2(pattern_size)) ||
        (pattern_size % 3 == 0 && (pattern_size > sizeof(cl_long3) || !IsPowerOf2(pattern_size / 3))))
    {
        return CL_INVALID_VALUE;
    }

    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    SharedPtr<MemoryObject> pBuffer = m_pContextModule->GetMemoryObject(clBuffer);
    if (NULL == pBuffer)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pBuffer->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    size_t szOrigin[MAX_WORK_DIM] = {offset, 0, 0};
    size_t szRegion[MAX_WORK_DIM] = {size, 1, 1};
    if (CL_SUCCESS != (errVal = pBuffer->CheckBounds(szOrigin, szRegion)))
    {
        // Out of bounds check.
        return errVal;
    }

    if ( (offset % pattern_size) || (size % pattern_size) )
    {
        return CL_INVALID_VALUE;
    }

    // check alignment with the device, just for sub-buffers.
    if (pBuffer->GetParent())
    {
        cl_uint         devAlignment;

        SharedPtr<FissionableDevice>pDevice = pCommandQueue->GetDefaultDevice();
        pDevice->GetInfo(CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(devAlignment), &devAlignment, NULL);
        // CL_DEVICE_MEM_BASE_ADDR_ALIGN is in bits, convert to bytes
        devAlignment /= 8;

        void *pData = pBuffer->GetBackingStoreData();
        // check there is BS data, and that it is not aligned with the queue device
        if (pData && ((cl_ulong)pData % devAlignment))
        {
            return CL_MISALIGNED_SUB_BUFFER_OFFSET;
        }
    }

    if ( (!event_wait_list && (0 < num_events_in_wait_list)) ||
            (event_wait_list && (0 == num_events_in_wait_list)) )
    {
        return CL_INVALID_EVENT_WAIT_LIST;
    }

    Command* pFillBufferCmd = new FillBufferCommand(pCommandQueue, m_pOclEntryPoints,
            pBuffer, pattern, pattern_size, offset, size);
    if (NULL == pFillBufferCmd)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pFillBufferCmd->Init();
    if ( CL_FAILED(errVal) )
    {
        delete pFillBufferCmd;
        return  errVal;
    }

    errVal = pCommandQueue->EnqueueCommand(pFillBufferCmd, CL_FALSE, num_events_in_wait_list, event_wait_list, pEvent, apiLogger);
    if(CL_FAILED(errVal))
    {
        pFillBufferCmd->CommandDone();
        delete pFillBufferCmd;
    }

    return  errVal;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueCopyBuffer(
    cl_command_queue    clCommandQueue,
    cl_mem              clSrcBuffer,
    cl_mem              clDstBuffer,
    size_t              szSrcOffset,
    size_t              szDstOffset,
    size_t              szCb,
    cl_uint             uNumEventsInWaitList,
    const cl_event*     cpEventWaitList,
    cl_event*           pEvent,
    ApiLogger* apiLogger
    )
{
    cl_err_code errVal = CL_SUCCESS;
    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if (m_opencl_ver < OPENCL_VERSION_2_1 && 0 == szCb)
    {
        return CL_INVALID_VALUE;
    }

    SharedPtr<MemoryObject> pSrcBuffer = m_pContextModule->GetMemoryObject(clSrcBuffer);
    SharedPtr<MemoryObject> pDstBuffer = m_pContextModule->GetMemoryObject(clDstBuffer);
    if (NULL == pSrcBuffer || NULL == pDstBuffer)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pSrcBuffer->GetContext()->GetId() != pCommandQueue->GetContextId()  ||
        pSrcBuffer->GetContext()->GetId() != pDstBuffer->GetContext()->GetId()
        )
    {
        return CL_INVALID_CONTEXT;
    }

    // Check boundaries.
    if (CL_SUCCESS != (errVal = pSrcBuffer->CheckBounds(&szSrcOffset,&szCb)))
    {
        return errVal;
    }
    if (CL_SUCCESS != (errVal = pDstBuffer->CheckBounds(&szDstOffset,&szCb)))
    {
        return errVal;
    }

    // Do parallel copy if offset is 0.
    if (m_enableParallelCopy && szSrcOffset == 0 && szDstOffset == 0) {
        errVal = EnqueueLibraryCopy(pCommandQueue, &clDstBuffer, &clSrcBuffer,
                                    szCb, false, false, false, false, CL_FALSE,
                                    uNumEventsInWaitList, cpEventWaitList,
                                    pEvent, apiLogger, CL_COMMAND_COPY_BUFFER);
        if (CL_SUCCEEDED(errVal))
            return errVal;
    }

    const size_t pszSrcOrigin[3] = { szSrcOffset, 0, 0 };
    const size_t pszDstOrigin[3] = { szDstOffset, 0, 0 };
    const size_t pszRegion[3] = { szCb, 1, 1 };

    if( clSrcBuffer == clDstBuffer)
    {
        // Check overlapping
        if (CheckMemoryObjectOverlapping(pSrcBuffer, pszSrcOrigin, pszDstOrigin, pszRegion))
        {
            return CL_MEM_COPY_OVERLAP;
        }
    }

    Command* pCopyBufferCommand = new CopyBufferCommand(pCommandQueue, m_pOclEntryPoints, pSrcBuffer, pDstBuffer, pszSrcOrigin, pszDstOrigin, pszRegion);
    if (NULL == pCopyBufferCommand)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pCopyBufferCommand->Init();
    if ( CL_FAILED(errVal) )
    {
        delete pCopyBufferCommand;
        return  errVal;
    }

    // Enqueue copy command, never blocking
    errVal = pCopyBufferCommand->EnqueueSelf(CL_FALSE, uNumEventsInWaitList, cpEventWaitList, pEvent, apiLogger);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pCopyBufferCommand->CommandDone();
        delete pCopyBufferCommand;
    }

    return  errVal;
}

/******************************************************************
 *
 ******************************************************************/

cl_err_code  ExecutionModule::EnqueueCopyBufferRect (
                        cl_command_queue    clCommandQueue,
                        cl_mem                clSrcBuffer,
                        cl_mem                clDstBuffer,
                        const size_t        szSrcOrigin[MAX_WORK_DIM],
                        const size_t        szDstOrigin[MAX_WORK_DIM],
                        const size_t        region[MAX_WORK_DIM],
                        size_t                src_buffer_row_pitch,
                        size_t                src_buffer_slice_pitch,
                        size_t                dst_buffer_row_pitch,
                        size_t                dst_buffer_slice_pitch,
                        cl_uint                uNumEventsInWaitList,
                        const cl_event*        cpEventWaitList,
                        cl_event* pEvent,
                        ApiLogger* apiLogger)
{
    cl_err_code errVal = CL_SUCCESS;
    if (NULL == szSrcOrigin || NULL == szDstOrigin || NULL == region)
    {
        return CL_INVALID_VALUE;
    }
    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    SharedPtr<MemoryObject> pSrcBuffer = m_pContextModule->GetMemoryObject(clSrcBuffer);
    SharedPtr<MemoryObject> pDstBuffer = m_pContextModule->GetMemoryObject(clDstBuffer);
    if (NULL == pSrcBuffer || NULL == pDstBuffer)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pSrcBuffer->GetContext()->GetId() != pCommandQueue->GetContextId()  ||
        pSrcBuffer->GetContext()->GetId() != pDstBuffer->GetContext()->GetId()
        )
    {
        return CL_INVALID_CONTEXT;
    }

    if (CheckIfAnyDimIsZero(region)                                                                    ||
        (src_buffer_row_pitch    !=0 && src_buffer_row_pitch        <region[0])                            ||
        (dst_buffer_row_pitch    !=0 && dst_buffer_row_pitch        <region[0])                            ||
        (src_buffer_slice_pitch    !=0 && src_buffer_slice_pitch    <(region[1]*src_buffer_row_pitch))    ||
        (dst_buffer_slice_pitch    !=0 && dst_buffer_slice_pitch    <(region[1]*dst_buffer_row_pitch))
        )
    {

        return CL_INVALID_VALUE;
    }

    // Check boundaries.
    SetIfZero(src_buffer_row_pitch        , region[0]);
    SetIfZero(dst_buffer_row_pitch        , region[0]);
    SetIfZero(src_buffer_slice_pitch    , region[1] * src_buffer_row_pitch);
    SetIfZero(dst_buffer_slice_pitch    , region[1] * dst_buffer_row_pitch);


    if (CL_SUCCESS != (errVal = pSrcBuffer->CheckBoundsRect(szSrcOrigin, region, src_buffer_row_pitch, src_buffer_slice_pitch)) ||
        CL_SUCCESS != (errVal = pDstBuffer->CheckBoundsRect(szDstOrigin, region, dst_buffer_row_pitch, dst_buffer_slice_pitch)))

    {
        // Out of bounds check.
        return errVal;
    }

    if ( NULL != pSrcBuffer->GetParent())
    {
        if (!pSrcBuffer->IsSupportedByDevice(pCommandQueue->GetDefaultDevice()))
        {
            return CL_MISALIGNED_SUB_BUFFER_OFFSET;
        }
    }

    if ( NULL != pDstBuffer->GetParent())
    {
        if (!pDstBuffer->IsSupportedByDevice(pCommandQueue->GetDefaultDevice()))
        {
            return CL_MISALIGNED_SUB_BUFFER_OFFSET;
        }
    }

    if( clSrcBuffer == clDstBuffer)
    {
        // Check overlapping
        if (CheckMemoryObjectOverlapping(pSrcBuffer, szSrcOrigin, szDstOrigin, region))
        {
            // Rami todo
            //assert(false && "Rami added: CL_MEM_COPY_OVERLAP");
            return CL_MEM_COPY_OVERLAP;
        }
    }

    Command* pCopyBufferRectCommand = new CopyBufferRectCommand(pCommandQueue, m_pOclEntryPoints, pSrcBuffer, pDstBuffer, szSrcOrigin, szDstOrigin, region,
        src_buffer_row_pitch, src_buffer_slice_pitch, dst_buffer_row_pitch, dst_buffer_slice_pitch);
    if (NULL == pCopyBufferRectCommand)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pCopyBufferRectCommand->Init();
    if ( CL_FAILED(errVal) )
    {
        delete pCopyBufferRectCommand;
        return  errVal;
    }

    // Enqueue copy command, never blocking
    errVal = pCopyBufferRectCommand->EnqueueSelf(CL_FALSE, uNumEventsInWaitList, cpEventWaitList, pEvent, apiLogger);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pCopyBufferRectCommand->CommandDone();
        delete pCopyBufferRectCommand;
    }

    return  errVal;
}

/******************************************************************
 * EnqueueFillImage
 * and help functions.
 ******************************************************************/

/**
 * Convert void* to PROPERLY aligned CL vector type.
 * It is required, since the void* input may not be aligned at all, and CL
 * vectors are aligned by definition. This may cause runtime errors -
 * see CSSD100013698 - where the compiler called movdqa on CL vector that was
 * C-style casted from a non-aligned void*.
 * @param in non-aligned pointer
 * @param out the target CL vector.
 */
template<typename TrgtCLVecType>
void voidToCLVec(const void *in, TrgtCLVecType &out)
{
    MEMCPY_S(&out, sizeof(TrgtCLVecType), in, sizeof(TrgtCLVecType));
}

/**
 * Allocate buffer, and convert origColor to the relevant format,
 * as described in clEnqueueFillImage
 * @param buf target buffer.
 * @param bufLen length of expected return.
 * @param order
 * @param type
 * @param origColor pointer to original color (cl_float4, cl_int4, cl_uint4)
 * @return
 */
static cl_uint buffer_from_converted_fill_color(
        cl_uchar         *buf,
        size_t           &bufLen,
        cl_channel_order order,
        cl_channel_type  type,
        const void *origColor)
{
    switch (type)
    {
    case CL_SNORM_INT8:
    case CL_SNORM_INT16:
    case CL_UNORM_INT8:
    case CL_UNORM_INT16:
    case CL_UNORM_SHORT_565:
    case CL_UNORM_SHORT_555:
    case CL_UNORM_INT_101010:
    case CL_HALF_FLOAT:
    case CL_FLOAT:
        cl_float4 alignedf4;
        voidToCLVec(origColor, alignedf4);
        Intel::OpenCL::Framework::norm_float_to_image(&alignedf4, order, type, buf, bufLen);
        break;

    case CL_SIGNED_INT8:
    case CL_SIGNED_INT16:
    case CL_SIGNED_INT32:
        cl_int4 alignedint4;
        voidToCLVec(origColor, alignedint4);
        Intel::OpenCL::Framework::non_norm_signed_to_image(&alignedint4, order, type, buf, bufLen);
        break;

    case CL_UNSIGNED_INT8:
    case CL_UNSIGNED_INT16:
    case CL_UNSIGNED_INT32:
        cl_uint4 aligneduint4;
        voidToCLVec(origColor, aligneduint4);
        Intel::OpenCL::Framework::non_norm_unsigned_to_image(&aligneduint4, order, type, buf, bufLen);
        break;

    default:
        return CL_DEV_INVALID_IMG_FORMAT;
    }

    return CL_SUCCESS;
}

cl_err_code ExecutionModule::EnqueueFillImage(cl_command_queue clCommandQueue,
        cl_mem clImage,
        const void *fillColor,
        const size_t *origin,
        const size_t *region,
        cl_uint num_events_in_wait_list,
        const cl_event *event_wait_list,
        cl_event *event,
        ApiLogger* apiLogger)
{
    cl_start;
    cl_err_code errVal = CL_SUCCESS;

    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    SharedPtr<MemoryObject> img = m_pContextModule->GetMemoryObject(clImage);
    if (NULL == img)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (img->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    if (CL_SUCCESS != (errVal = img->CheckBounds(origin, region)))
    {
        // Out of bounds check.
        return errVal;
    }

    if ( (!event_wait_list && (0 < num_events_in_wait_list)) ||
            (event_wait_list && (0 == num_events_in_wait_list)) )
    {
        return CL_INVALID_EVENT_WAIT_LIST;
    }

    cl_image_format format;
    errVal = img->GetImageInfo(CL_IMAGE_FORMAT, sizeof(cl_image_format), &format, NULL);
    if (CL_SUCCESS != errVal)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    errVal = CheckImageFormatSupportedByDevice(*pCommandQueue->GetDefaultDevice(), *img);
    if (CL_SUCCESS != errVal)
    {
        return errVal;
    }

    cl_uint img_dim_count = 1;
    size_t dim_sz = 0;

    errVal = img->GetImageInfo(CL_IMAGE_HEIGHT, sizeof(size_t), &dim_sz, NULL);
    if (CL_SUCCESS == errVal)
    {
        if (dim_sz) ++img_dim_count;
    } else return CL_INVALID_MEM_OBJECT;

    errVal = img->GetImageInfo(CL_IMAGE_DEPTH, sizeof(size_t), &dim_sz, NULL);
    if (CL_SUCCESS == errVal)
    {
        if (dim_sz) ++img_dim_count;
    } else return CL_INVALID_MEM_OBJECT;

    errVal = img->GetImageInfo(CL_IMAGE_ARRAY_SIZE, sizeof(size_t), &dim_sz, NULL);
    if (CL_SUCCESS == errVal)
    {
        if (dim_sz) ++img_dim_count;
    } else return CL_INVALID_MEM_OBJECT;

    cl_uchar pattern[MAX_PATTERN_SIZE];
    size_t pattern_size = GenericMemObjectBackingStore::get_element_size(&format);
    assert(MAX_PATTERN_SIZE >= pattern_size && "Trying to assign a color format too big.");

    errVal = buffer_from_converted_fill_color(pattern, pattern_size, format.image_channel_order,
            format.image_channel_data_type, fillColor);

    if (CL_SUCCESS != errVal)
    {
        return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    }

    Command* pFillBufferCmd = new FillImageCommand(pCommandQueue, m_pOclEntryPoints,
            img, pattern, pattern_size, img_dim_count, origin, region);
    if (NULL == pFillBufferCmd)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pFillBufferCmd->Init();
    if ( CL_FAILED(errVal) )
    {
        delete pFillBufferCmd;
        return  errVal;
    }

    errVal = pCommandQueue->EnqueueCommand(pFillBufferCmd, CL_FALSE, num_events_in_wait_list, event_wait_list, event, apiLogger);
    if(CL_FAILED(errVal))
    {
        pFillBufferCmd->CommandDone();
        delete pFillBufferCmd;
    }

    return  errVal;
}

/******************************************************************
 *
 ******************************************************************/
void * ExecutionModule::EnqueueMapBuffer(cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlockingMap, cl_map_flags clMapFlags, size_t szOffset, size_t szCb, cl_uint uNumEventsInWaitList, const cl_event* cpEventWaitList, cl_event* pEvent, cl_int* pErrcodeRet, ApiLogger* apiLogger)
{
    cl_int err = CL_SUCCESS;
    if (NULL == pErrcodeRet)
    {
        pErrcodeRet = &err;
    }
    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        *pErrcodeRet = CL_INVALID_COMMAND_QUEUE;
        return NULL;
    }

    // Check that flags CL_MAP_READ or CL_MAP_WRITE only
    if ( CL_SUCCESS != checkMapFlagsMutex(clMapFlags) )
    {
        *pErrcodeRet = CL_INVALID_VALUE;
        return NULL;
    }


    SharedPtr<MemoryObject> pBuffer = m_pContextModule->GetMemoryObject(clBuffer);
    if (NULL == pBuffer)
    {
        *pErrcodeRet =  CL_INVALID_MEM_OBJECT;
        return NULL;
    }

    if (pBuffer->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        *pErrcodeRet = CL_INVALID_CONTEXT;
        return NULL;
    }

    if (CL_SUCCESS != pBuffer->ValidateMapFlags(clMapFlags))
    {
        *pErrcodeRet = CL_INVALID_VALUE;
        return NULL;
    }

    if (NULL != pBuffer->GetParent())
    {
        if (!pBuffer->IsSupportedByDevice(pCommandQueue->GetDefaultDevice()))
        {
            *pErrcodeRet = CL_MISALIGNED_SUB_BUFFER_OFFSET;
            return NULL;
        }
    }

    if (pBuffer->GetSize() < (szOffset+szCb))
    {
        // Out of bounds check.
         *pErrcodeRet =  CL_INVALID_VALUE;
         return NULL;
    }
    if (false == pCommandQueue->GetEventsManager()->IsValidEventList(uNumEventsInWaitList, cpEventWaitList))
    {
        *pErrcodeRet = CL_INVALID_EVENT_WAIT_LIST;
        return NULL;
    }

    MapBufferCommand* pMapBufferCommand = new MapBufferCommand(pCommandQueue, m_pOclEntryPoints, pBuffer, clMapFlags, szOffset, szCb);
    // Must set device Id before init for buffer resource allocation.
    if (NULL == pMapBufferCommand)
    {
        *pErrcodeRet = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }

    *pErrcodeRet = pMapBufferCommand->Init();
    if ( CL_FAILED(*pErrcodeRet))
    {
        delete pMapBufferCommand;
        return  NULL;
    }

    // Get pointer for mapped region since it is allocated on init. Execute will lock the region
    // Note that if EnqueueCommand succeeded, by the time it returns, the command may be deleted already.
    void* mappedPtr = pMapBufferCommand->GetMappedPtr();

    *pErrcodeRet = pMapBufferCommand->EnqueueSelf(bBlockingMap, uNumEventsInWaitList, cpEventWaitList, pEvent, apiLogger);
    if(CL_FAILED(*pErrcodeRet))
    {
        // Enqueue failed, free resources
        pMapBufferCommand->CommandDone();
        delete pMapBufferCommand;
        return NULL;
    }
    return mappedPtr;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueUnmapMemObject(cl_command_queue clCommandQueue,cl_mem clMemObj, void* mappedPtr, cl_uint uNumEventsInWaitList, const cl_event* cpEventWaitList, cl_event* pEvent, ApiLogger* apiLogger)
{
    cl_err_code errVal;
    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    SharedPtr<MemoryObject> pMemObject = m_pContextModule->GetMemoryObject(clMemObj);
    if (NULL == pMemObject)
    {
        return  CL_INVALID_MEM_OBJECT;
    }

    if (pMemObject->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    Command* pUnmapMemObjectCommand = new UnmapMemObjectCommand(pCommandQueue, m_pOclEntryPoints, pMemObject, mappedPtr);
    // Must set device Id before init for buffer resource allocation.
    if (NULL == pUnmapMemObjectCommand)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pUnmapMemObjectCommand->Init();
    if ( CL_FAILED(errVal) )
    {
        delete pUnmapMemObjectCommand;
        return  errVal;
    }

    errVal = pUnmapMemObjectCommand->EnqueueSelf(CL_FALSE /*never blocks*/, uNumEventsInWaitList, cpEventWaitList, pEvent, apiLogger);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pUnmapMemObjectCommand->CommandDone();
        delete pUnmapMemObjectCommand;
    }

    return  errVal;
}

cl_err_code ExecutionModule::RunAutorunKernels(
    const SharedPtr<Program>& program, ApiLogger* apiLogger)
{
    std::vector<SharedPtr<Kernel>> kernels;
    cl_err_code error;
    error = program->GetAutorunKernels(kernels);
    if (CL_FAILED(error))
    {
        return error;
    }

    if (kernels.empty())
    {
        return CL_SUCCESS;
    }

    cl_uint numDevices = program->GetNumDevices();
    std::vector<cl_device_id> devices(numDevices);
    error = program->GetDevices(&devices.front());
    if (CL_FAILED(error))
    {
        return error;
    }

    std::vector<cl_command_queue> queues(numDevices);

    for (cl_uint i = 0; i < numDevices; ++i)
    {
        cl_command_queue_properties properties[] = {
            CL_QUEUE_PROPERTIES,
            CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
            (cl_command_queue_properties)0
        };
        queues[i] = CreateCommandQueue(
            program->GetContext()->GetHandle(), devices[i], properties, &error);
        if (CL_FAILED(error))
        {
            return error;
        }
    }

    for (cl_uint i = 0; i < numDevices; ++i)
    {
        for (const auto& kernel: kernels)
        {
            size_t localSize[MAX_WORK_DIM] = {0};
            error = kernel->GetWorkGroupInfo(
                program->GetContext()->GetDevice(devices[i]),
                CL_KERNEL_COMPILE_WORK_GROUP_SIZE,
                sizeof(localSize), localSize, nullptr);
            if (CL_FAILED(error))
            {
                return error;
            }

            bool isTask = kernel->IsTask(program->GetContext()->GetDevice(
                  devices[i]).GetPtr());

            // According to spec from PSG:
            //
            // The following kernel:
            // __atrribute__((reqd_work_group_size(X,Y,Z)))
            // __attribute__((autorun))
            // __kernel void test() { ... }
            // Should be launched with
            //  - global size = (2^32, 2^32, 2^32)
            //  - local size = (X, Y, Z)
            //
            // The following kernel:
            // __attribute__((max_global_work_dim(0)))
            // __attribute__((autorun))
            // __kernel void test() { ... }
            // Should be launched with
            //  - global size = (1, 1, 1)
            //  - local size = (1, 1, 1)

            size_t gsValue = (size_t)1 << (size_t)32;
            size_t globalSize[MAX_WORK_DIM] = {gsValue, gsValue, gsValue};

            if (isTask)
            {
                globalSize[0] = globalSize[1] = globalSize[2] = 1;
                localSize[0] = localSize[1] = localSize[2] = 1;
            }

            error = EnqueueNDRangeKernel(
                queues[i], kernel->GetHandle(), MAX_WORK_DIM, nullptr,
                globalSize, localSize, 0, nullptr, nullptr, apiLogger);
            if (CL_FAILED(error))
            {
                return error;
            }
        }

        Flush(queues[i]);
        ReleaseCommandQueue(queues[i]);
    }

    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueNDRangeKernel(
    cl_command_queue clCommandQueue,
    cl_kernel       clKernel,
    cl_uint         uiWorkDim,
    const size_t*   cpszGlobalWorkOffset,
    const size_t*   cpszGlobalWorkSize,
    const size_t*   cpszLocalWorkSize,
    cl_uint         uNumEventsInWaitList,
    const cl_event* cpEventWaitList,
    cl_event*       pEvent,
    ApiLogger*      apiLogger
    )
{
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
      if ( (NULL != m_pGPAData) && m_pGPAData->bUseGPA )
      {
        static __thread __itt_string_handle* pTaskName = NULL;
        if ( NULL == pTaskName )
        {
          pTaskName = __itt_string_handle_create("ExecutionModule::EnqueueNDRangeKernel()->ArgumentValidation...");
        }
        __itt_task_begin(m_pGPAData->pAPIDomain, __itt_null, __itt_null, pTaskName);
      }
#endif

    cl_err_code errVal = CL_SUCCESS;

    LOG_DEBUG(TEXT("EnqueueNDRangeKernel work dimension = %u"), uiWorkDim);
    if( uiWorkDim < 1 || uiWorkDim > 3)
    {
        return CL_INVALID_WORK_DIMENSION;
    }

    if (FrameworkProxy::Instance()->GetOCLConfig()->GetOpenCLVersion() < OPENCL_VERSION_2_1)
    {
        if( NULL == cpszGlobalWorkSize )
        {
                return CL_INVALID_GLOBAL_WORK_SIZE;
        }

        for ( cl_uint ui = 0; ui < uiWorkDim; ui++ )
        {
          LOG_DEBUG(TEXT("EnqueueNDRangeKernel global worksize dim #%u = %u"), ui, cpszGlobalWorkSize[ui]);
          if ( cpszGlobalWorkSize[ui] == 0 )
          {
            return CL_INVALID_GLOBAL_WORK_SIZE;
          }
        }
    }
    const size_t zero_size[] = {0, 0, 0};
    if( nullptr == cpszGlobalWorkSize )
    {
        cpszGlobalWorkSize = zero_size;
    }

    // TODO: Check for optimization
    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    SharedPtr<Kernel> pKernel = m_pContextModule->GetKernel(clKernel);
    if (NULL == pKernel)
    {
        return CL_INVALID_KERNEL;
    }

    if (pKernel->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    bool isFPGAEmulator = pKernel->GetContext()->IsFPGAEmulator();
    if (isFPGAEmulator &&
        !pKernel->GetProgram()->TestAndSetAutorunKernelsLaunched())
    {
        errVal = RunAutorunKernels(pKernel->GetProgram(), apiLogger);
        if (CL_FAILED(errVal))
        {
            return CL_OUT_OF_RESOURCES;
        }
    }

    const SharedPtr<FissionableDevice>& pDevice = pCommandQueue->GetDefaultDevice();

    // CL_INVALID_KERNEL_ARGS if the kernel argument values have not been specified.
    if(!pKernel->IsValidKernelArgs())
    {
        return CL_INVALID_KERNEL_ARGS;
    }

    if( NULL != cpszGlobalWorkSize && NULL != cpszLocalWorkSize )
    {
        for( unsigned int ui=0; ui<uiWorkDim; ui++)
        {
            LOG_DEBUG(TEXT("EnqueueNDRangeKernel local worksize dim #%u = %u"), ui, cpszLocalWorkSize[ui]);
            if ((cpszLocalWorkSize[ui] == 0) || ((OPENCL_VERSION_1_2 == m_opencl_ver) && (0 != (cpszGlobalWorkSize[ui] % cpszLocalWorkSize[ui]))))
            {
                return CL_INVALID_WORK_GROUP_SIZE;
            }
        }
    }

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (NULL != m_pGPAData) && m_pGPAData->bUseGPA )
    {
        __itt_task_end(m_pGPAData->pAPIDomain); // "ExecutionModule::EnqueueNDRangeKernel()->ArgumentValidation..."
    }
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
      if ( (NULL != m_pGPAData) && m_pGPAData->bUseGPA )
      {
        static __thread __itt_string_handle* pTaskName = NULL;
        if ( NULL == pTaskName )
        {
          pTaskName = __itt_string_handle_create("ExecutionModule::EnqueueNDRangeKernel()->CommandCreation()");
        }
        __itt_task_begin(m_pGPAData->pAPIDomain, __itt_null, __itt_null, pTaskName);
      }
#endif

    // TODO: create buffer resources in advance, if they are not exists,
    //      On error return: CL_OUT_OF_RESOURCES
    Command* pNDRangeKernelCmd = new NDRangeKernelCommand(pCommandQueue, m_pOclEntryPoints, pKernel, uiWorkDim, cpszGlobalWorkOffset, cpszGlobalWorkSize, cpszLocalWorkSize);
    if ( NULL == pNDRangeKernelCmd )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    // Must set device Id before init for buffer resource allocation.
    pNDRangeKernelCmd->SetDevice(pDevice);
    errVal = pNDRangeKernelCmd->Init();
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (NULL != m_pGPAData) && m_pGPAData->bUseGPA )
    {
        __itt_task_end(m_pGPAData->pAPIDomain); // "ExecutionModule::EnqueueNDRangeKernel()->CommandCreation()"
    }
#endif
    if ( CL_FAILED(errVal) )
    {
        delete pNDRangeKernelCmd;
        return  errVal;
    }
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (NULL != m_pGPAData) && m_pGPAData->bUseGPA )
    {
        static __thread __itt_string_handle* pTaskName = NULL;
        if ( NULL == pTaskName )
        {
          pTaskName = __itt_string_handle_create("ExecutionModule::EnqueueNDRangeKernel()->EnqueueSelf()");
        }
        __itt_task_begin(m_pGPAData->pAPIDomain, __itt_null, __itt_null, pTaskName);
      }
#endif

    // Kernel serialization.
    // If we have the same kernel being enqueued on FPGA emulator several
    // times - we don't want two or more instances of this kernel being
    // executed concurrently. That is needed in some cases, if the kernel
    // contains SYCL/OpenCL pipes/channels to prevent possible race condition
    // because of several instances of the same kernel are using the same
    // pipe/channel from different threads (that leads to program hang).
    // None of the specifications guarantee this execution order, but FPGA
    // hardware works that way and so the emulator shall mimic the
    // hardware's behaviour.
    // To achieve this, we create a map of events to kernels. If execution
    // of a kernel is finished, we can enqueue it one more time. Otherwise,
    // we wait until the kernel is finished.
    // Please note, that for SYCL we currently have 1:1 mapping of Q.submit
    // on enqueue_kernel OpenCL API function calls with the order of kernels
    // to be enqueued saved from SYCL to OpenCL. But it isn't necessarily
    // will be true in the future. When/if this behaviour is changed - this
    // patch must be reverted and actually I don't know if kernel
    // serialization is possible in this case.
    bool updatedEventList = false;
    // Do nothing in case of multi-device program
    // Process kernel serialization only for out-of-order queues, so the
    // runtime won't change any user's custom logic in his/her program.
    SharedPtr<Program> program = pKernel->GetProgram();
    if (isFPGAEmulator && (program->GetNumDevices() == 1) &&
        pCommandQueue->IsOutOfOrderExecModeEnabled())
    {
        std::vector<cl_event> EventListToWait;
        std::copy(cpEventWaitList,
                  cpEventWaitList + uNumEventsInWaitList,
                  back_inserter(EventListToWait));

        // Don't try to serialize autorun kernels - they are running from a
        // start of a program till its termination.
        std::vector<SharedPtr<Kernel>> autoKernels;
        errVal = program->GetAutorunKernels(autoKernels);
        if (CL_FAILED(errVal))
        {
            return errVal;
        }
        bool isAutorunKernel =
            (std::find(autoKernels.begin(), autoKernels.end(), pKernel) !=
             autoKernels.end());
        if (!isAutorunKernel)
        {
            // Query for kernel name to use in the map. With that approach a
            // case when kernel is created with clCreateKernel is also handled.
            size_t kernelNameLen = 0;
            errVal = pKernel->GetInfo(CL_KERNEL_FUNCTION_NAME, 0, nullptr,
                                      &kernelNameLen);
            if (CL_FAILED(errVal))
            {
                return errVal;
            }
            std::string kernelName(kernelNameLen, '\0');
            errVal = pKernel->GetInfo(CL_KERNEL_FUNCTION_NAME, kernelNameLen,
                                      &kernelName[0], nullptr);
            if (CL_FAILED(errVal))
            {
                return errVal;
            }

            // To track kernel execution we need to implicitly set a tracker
            // event even if a user set it as null.
            cl_event* trackerEvent = pEvent ? pEvent : ::new cl_event;

            {
                OclAutoMutex mu(&KernelEventMutex);
                auto it = m_OclKernelEventMap.find(kernelName);
                if (it != m_OclKernelEventMap.end())
                {
                    cl_event prevClEvent = it->second;
                    SharedPtr<OclEvent> prevEvent =
                        m_pEventsManager->GetEventClass<OclEvent>(prevClEvent);
                    if (prevEvent &&
                        CL_COMPLETE != prevEvent->GetEventExecState())
                    {
                        EventListToWait.push_back(prevClEvent);
                    }
                    // Update the map replacing old with a newer one.
                    m_OclKernelEventMap.erase(kernelName);
                }

                errVal = pNDRangeKernelCmd->EnqueueSelf(
                    false /*never blocking*/, EventListToWait.size(),
                    EventListToWait.empty() ? nullptr : &EventListToWait[0],
                    trackerEvent, apiLogger);
                m_OclKernelEventMap.insert(
                    std::make_pair(kernelName, *trackerEvent));
            }

            // Set call back which will erase this tracker kernel-event pair
            // from this map when the event status is changed to CL_COMPLETE.
            SetEventCallback(*trackerEvent, CL_COMPLETE,
                             (eventCallbackFn)callbackForKernelEventMap,
                             &m_OclKernelEventMap);
            updatedEventList = true;
        }
    }

    if (!updatedEventList)
    {
        errVal = pNDRangeKernelCmd->EnqueueSelf(
                           false/*never blocking*/, uNumEventsInWaitList,
                           cpEventWaitList, pEvent, apiLogger);
    }

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (NULL != m_pGPAData) && m_pGPAData->bUseGPA )
    {
      __itt_task_end(m_pGPAData->pAPIDomain); // "ExecutionModule::EnqueueNDRangeKernel()->EnqueueSelf()"
    }
#endif

    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pNDRangeKernelCmd->CommandDone();
        delete pNDRangeKernelCmd;
    }

    return  errVal;
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueTask( cl_command_queue clCommandQueue, cl_kernel clKernel, cl_uint uNumEventsInWaitList, const cl_event* cpEventWaitList, cl_event* pEvent, ApiLogger* apiLogger)
{
    cl_err_code errVal = CL_SUCCESS;

    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    SharedPtr<Kernel> pKernel = m_pContextModule->GetKernel(clKernel);
    if (NULL == pKernel)
    {
        return CL_INVALID_KERNEL;
    }

    if (pKernel->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    // CL_INVALID_PROGRAM_EXECUTABLE if there is no successfully built program
    // executable available for device associated with command_queue.
    if( NULL==pKernel->GetDeviceKernel(pCommandQueue->GetDefaultDevice().GetPtr()) )
    {
        return CL_INVALID_PROGRAM_EXECUTABLE;
    }

    // CL_INVALID_KERNEL_ARGS if the kernel argument values have not been specified.
    if(!pKernel->IsValidKernelArgs())
    {
        return CL_INVALID_KERNEL_ARGS;
    }

    if (pKernel->GetContext()->IsFPGAEmulator() &&
        !pKernel->GetProgram()->TestAndSetAutorunKernelsLaunched())
    {
        errVal = RunAutorunKernels(pKernel->GetProgram(), apiLogger);
        if (CL_FAILED(errVal))
        {
            return CL_OUT_OF_RESOURCES;
        }
    }

    // TODO: Handle those error values, probably through the kernel object...
    // CL_INVALID_WORK_GROUP_SIZE

    Command* pTaskCommand = new TaskCommand(pCommandQueue, m_pOclEntryPoints, pKernel);
    // Must set device Id before init for buffer resource allocation.
    if (NULL == pTaskCommand)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pTaskCommand->Init();
    if ( CL_FAILED(errVal) )
    {
        delete pTaskCommand;
        return  errVal;
    }

    errVal = pTaskCommand->EnqueueSelf(false/*never blocking*/, uNumEventsInWaitList, cpEventWaitList, pEvent, apiLogger);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pTaskCommand->CommandDone();
        delete pTaskCommand;
    }

    return  errVal;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueNativeKernel(cl_command_queue clCommandQueue, void (CL_CALLBACK*pUserFnc)(void *), void* pArgs, size_t szCbArgs, cl_uint uNumMemObjects, const cl_mem* clMemList, const void** ppArgsMemLoc, cl_uint uNumEventsInWaitList, const cl_event* cpEventWaitList, cl_event* pEvent, ApiLogger* apiLogger)
{
    cl_err_code errVal = CL_SUCCESS;

    // First check NULL values:
    if (    ( NULL == pUserFnc)                                                     ||
            ( NULL == pArgs && ((szCbArgs > 0) || uNumMemObjects > 0 ))             ||
            ( NULL != pArgs && 0 == szCbArgs)                                       ||
            ( (uNumMemObjects >  0) && ( NULL == clMemList || NULL == ppArgsMemLoc))||
            ( (0 == uNumMemObjects) && ( NULL != clMemList || NULL != ppArgsMemLoc)) )
    {
        return CL_INVALID_VALUE;
    }

    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    SharedPtr<MemoryObject>* pMemObjectsList = NULL;
    if (uNumMemObjects > 0)
    {
        // Create MemoryObjects references
        pMemObjectsList = new SharedPtr<MemoryObject>[uNumMemObjects];

        if(NULL == pMemObjectsList)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        cl_uint i;
        for( i=0; i < uNumMemObjects; i++ )
        {
            // Check that buffer is available
            pMemObjectsList[i] = m_pContextModule->GetMemoryObject(clMemList[i]);
            if ( NULL == pMemObjectsList[i] )
            {
                delete[] pMemObjectsList;
                return CL_INVALID_MEM_OBJECT;
            }
        }
    }

    // TODO: Handle those error values, probably through the DEVICE object...
    // CL_INVALID_OPERATION

    Command* pNativeKernelCommand = new NativeKernelCommand(pCommandQueue, m_pOclEntryPoints, pUserFnc, pArgs, szCbArgs, uNumMemObjects, pMemObjectsList, ppArgsMemLoc );
    if(NULL == pNativeKernelCommand)
    {
        if ( NULL != pMemObjectsList )
        {
            delete []pMemObjectsList;
        }
        return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pNativeKernelCommand->Init();
    if ( CL_FAILED(errVal) )
    {
        if ( NULL != pMemObjectsList )
        {
            delete []pMemObjectsList;
        }
        delete pNativeKernelCommand;
        return  errVal;
    }

    errVal = pNativeKernelCommand->EnqueueSelf(CL_FALSE/*never blocking*/, uNumEventsInWaitList, cpEventWaitList, pEvent, apiLogger);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        // pMemObjectsList is released in CommandDone()
        pNativeKernelCommand->CommandDone();
        if ( NULL != pMemObjectsList )
        {
            delete []pMemObjectsList;
        }
        delete pNativeKernelCommand;
    }

    return  errVal;
}

inline bool DimensionsOverlap(size_t d1_min, size_t d1_max, size_t d2_min, size_t d2_max)
{
    assert (d1_max >= d1_min);
    assert (d2_max >= d2_min);
    if ((d1_min == d1_max) || (d2_min == d2_max))
    {
        return false;
    }
    return !((d1_min >= d2_max) || (d2_min >= d1_max));
}

/******************************************************************
 * Returns true if regions in pMemObj overlap
 ******************************************************************/
inline bool ExecutionModule::CheckMemoryObjectOverlapping(SharedPtr<MemoryObject> pMemObj, const size_t* szSrcOrigin, const size_t* szDstOrigin, const size_t* szRegion)
{
    bool isOverlaps = true;
    cl_mem_object_type memObjType = pMemObj->GetType();
    const size_t src_min[] = {szSrcOrigin[0], szSrcOrigin[1], szSrcOrigin[2]};
    const size_t src_max[] = {szSrcOrigin[0]+szRegion[0], szSrcOrigin[1]+szRegion[1], szSrcOrigin[2]+szRegion[2]};

    const size_t dst_min[] = {szDstOrigin[0], szDstOrigin[1], szDstOrigin[2]};
    const size_t dst_max[] = {szDstOrigin[0]+szRegion[0], szDstOrigin[1]+szRegion[1], szDstOrigin[2]+szRegion[2]};

    size_t dimensionsToCompare = 0;

    switch(memObjType)
    {
    case CL_MEM_OBJECT_IMAGE3D:
        dimensionsToCompare = 3;
        break;

    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        if (szSrcOrigin[2] != szDstOrigin[2])

        {
            // For image array with different image index, no need to compare any boundaries
            // keep dimensionToCompare at 0.
            break;
        }
        dimensionsToCompare = 2;
        break;

    case CL_MEM_OBJECT_IMAGE2D:
        dimensionsToCompare = 2;
        break;

    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
        if (szSrcOrigin[1] == szDstOrigin[1])
        {
            dimensionsToCompare = 1;
        }
        break;

    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        dimensionsToCompare = 1;
        break;
    case CL_MEM_OBJECT_BUFFER:
        dimensionsToCompare = 3;
        break;

    default:
        assert(0 && "Illegal type of memory object");
        break;
    }
    for (size_t dimension = 0; dimension < dimensionsToCompare; ++dimension)
    {
        isOverlaps &= DimensionsOverlap(src_min[dimension], src_max[dimension], dst_min[dimension], dst_max[dimension]);
    }
    return isOverlaps;
}

/******************************************************************
 *
 ******************************************************************/
inline size_t ExecutionModule::CalcRegionSizeInBytes(SharedPtr<MemoryObject> pImage, const size_t* szRegion)
{
    size_t szPixelByteSize = 0;
    size_t szRegionSizeInBytes = 0;
    cl_err_code errVal  = CL_SUCCESS;
    cl_mem_object_type memObjType = pImage->GetType();
    if( memObjType != CL_MEM_OBJECT_BUFFER )
    {
        // Note: we have already checked that the appropriate elements of szRegion equal 1, so the same calculation is valid for all image types
        errVal = pImage->GetImageInfo( CL_IMAGE_ELEMENT_SIZE, sizeof(size_t), &szPixelByteSize, NULL);
        if(CL_SUCCEEDED(errVal))
        {
            szRegionSizeInBytes = szRegion[0] * szRegion[1] * szRegion[2] * szPixelByteSize;
        }
    }
    return szRegionSizeInBytes;
}

/******************************************************************
 *
 ******************************************************************/
inline cl_err_code ExecutionModule::CheckImageFormats( SharedPtr<MemoryObject> pSrcImage, SharedPtr<MemoryObject> pDstImage)
{
    cl_err_code errVal;
    cl_image_format clSrcFormat;
    cl_image_format clDstFormat;

    errVal = pSrcImage->GetImageInfo( CL_IMAGE_FORMAT, sizeof(cl_image_format), &clSrcFormat, NULL);
    if ( CL_SUCCEEDED(errVal) )
    {
        errVal = pDstImage->GetImageInfo( CL_IMAGE_FORMAT, sizeof(cl_image_format), &clDstFormat, NULL);
    }
    if ( CL_SUCCEEDED(errVal))
    {
        // Check formats
        if ( ( clSrcFormat.image_channel_order != clDstFormat.image_channel_order ) ||
             ( clSrcFormat.image_channel_data_type != clDstFormat.image_channel_data_type )
             )
        {
            errVal = CL_IMAGE_FORMAT_MISMATCH;
        }
    }
    return errVal;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueReadImage(
                                cl_command_queue clCommandQueue,
                                cl_mem           clImage,
                                cl_bool          bBlocking,
                                const size_t     szOrigin[MAX_WORK_DIM],
                                const size_t     szRegion[MAX_WORK_DIM],
                                size_t           szRowPitch,
                                size_t           szSlicePitch,
                                void*            pOutData,
                                cl_uint          uNumEventsInWaitList,
                                const cl_event*  cpEventWaitList,
                                cl_event*        pEvent,
                                ApiLogger* apiLogger
                                )
{
    cl_err_code errVal = CL_SUCCESS;
    if (NULL == pOutData)
    {
        return CL_INVALID_VALUE;
    }

    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    if (pCommandQueue->GetContext()->IsFPGAEmulator())
    {
        return CL_INVALID_OPERATION;
    }

    SharedPtr<MemoryObject> pImage = m_pContextModule->GetMemoryObject(clImage);
    if (NULL == pImage)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pImage->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    if (pImage->GetFlags() & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_WRITE_ONLY) )
    {
        return CL_INVALID_OPERATION;
    }

    if (CL_SUCCESS != (errVal = pImage->CheckBounds(szOrigin, szRegion)))
    {
        return errVal;
    }
    errVal = CheckImageFormatSupportedByDevice(*pCommandQueue->GetDefaultDevice(), *pImage);
    if (CL_SUCCESS != errVal)
    {
        return errVal;
    }
    if (pImage->GetFlags() & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS))
    {
        return CL_INVALID_OPERATION;
    }

    Command* pReadImageCmd  = new ReadImageCommand(pCommandQueue, m_pOclEntryPoints, pImage, szOrigin, szRegion, szRowPitch, szSlicePitch, pOutData);
    if (NULL == pReadImageCmd)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pReadImageCmd->Init();
    if ( CL_FAILED(errVal) )
    {
        delete pReadImageCmd;
        return  errVal;
    }

    errVal = pReadImageCmd->EnqueueSelf(bBlocking, uNumEventsInWaitList, cpEventWaitList, pEvent, apiLogger);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pReadImageCmd->CommandDone();
        delete pReadImageCmd;
    }

    return  errVal;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueWriteImage(
                                cl_command_queue clCommandQueue,
                                cl_mem           clImage,
                                cl_bool          bBlocking,
                                const size_t     szOrigin[MAX_WORK_DIM],
                                const size_t     szRegion[MAX_WORK_DIM],
                                size_t           szRowPitch,
                                size_t           szSlicePitch,
                                const void *     cpSrcData,
                                cl_uint          uNumEventsInWaitList,
                                const cl_event*  cpEventWaitList,
                                cl_event*        pEvent,
                                ApiLogger* apiLogger
                                )
{
    cl_err_code errVal = CL_SUCCESS;
    if (NULL == cpSrcData)
    {
        return CL_INVALID_VALUE;
    }

    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if (pCommandQueue->GetContext()->IsFPGAEmulator())
    {
        return CL_INVALID_OPERATION;
    }

    SharedPtr<MemoryObject> pImage = m_pContextModule->GetMemoryObject(clImage);
    if (NULL == pImage)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pImage->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        return CL_INVALID_CONTEXT;
    }

    if (pImage->GetFlags() & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_READ_ONLY) )
    {
        return CL_INVALID_OPERATION;
    }

    if (CL_SUCCESS != (errVal = pImage->CheckBounds(szOrigin, szRegion)))
    {
        return errVal;
    }
    errVal = CheckImageFormatSupportedByDevice(*pCommandQueue->GetDefaultDevice(), *pImage);
    if (CL_SUCCESS != errVal)
    {
        return errVal;
    }
    if (pImage->GetFlags() & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))
    {
        return CL_INVALID_OPERATION;
    }

    Command* pWriteImageCmd  = new WriteImageCommand(pCommandQueue, m_pOclEntryPoints, bBlocking, pImage, szOrigin, szRegion, szRowPitch, szSlicePitch, cpSrcData);
    if (NULL == pWriteImageCmd)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pWriteImageCmd->Init();
    if ( CL_FAILED(errVal) )
    {
        delete pWriteImageCmd;
        return  errVal;
    }

    errVal = pWriteImageCmd->EnqueueSelf(CL_FALSE, uNumEventsInWaitList, cpEventWaitList, pEvent, apiLogger);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pWriteImageCmd->CommandDone();
        delete pWriteImageCmd;
    }

    return  errVal;
}

static bool IsImageDimSupportedByDevice(const MemoryObject& img, const FissionableDevice& dev, cl_image_info clImgInfo, cl_int iDevInfo)
{
    size_t szImgVal, szDevVal, szValSize;

    cl_err_code clErr = img.GetImageInfo(clImgInfo, sizeof(szImgVal), &szImgVal, &szValSize);

    assert(CL_SUCCEEDED(clErr));
    if (CL_FAILED(clErr))
    {
        return false;
    }

    assert(sizeof(szImgVal) == szValSize);
    clErr = dev.GetInfo(iDevInfo, sizeof(szDevVal), &szDevVal, &szValSize);

    assert(CL_SUCCEEDED(clErr));
    if (CL_FAILED(clErr))
    {
        return false;
    }

    assert(sizeof(szDevVal) == szValSize);
    return szImgVal <= szDevVal;
}

static bool AreImageDimsSupportedByDevice(const MemoryObject& img, const FissionableDevice& dev)
{
    cl_mem_object_type clMemObjType;
    size_t szValSize;

    cl_err_code clErr = img.GetInfo(CL_MEM_TYPE, sizeof(clMemObjType), &clMemObjType, &szValSize);

    assert(CL_SUCCEEDED(clErr));
    if (CL_FAILED(clErr))
    {
        return false;
    }

    assert(sizeof(clMemObjType) == szValSize);

    switch (clMemObjType)
    {
    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        return IsImageDimSupportedByDevice(img, dev, CL_IMAGE_WIDTH, CL_DEVICE_IMAGE2D_MAX_WIDTH);
    case CL_MEM_OBJECT_IMAGE2D:
        return IsImageDimSupportedByDevice(img, dev, CL_IMAGE_WIDTH, CL_DEVICE_IMAGE2D_MAX_WIDTH) &&
            IsImageDimSupportedByDevice(img, dev, CL_IMAGE_HEIGHT, CL_DEVICE_IMAGE2D_MAX_HEIGHT);
    case CL_MEM_OBJECT_IMAGE3D:
        return IsImageDimSupportedByDevice(img, dev, CL_IMAGE_WIDTH, CL_DEVICE_IMAGE3D_MAX_WIDTH) &&
            IsImageDimSupportedByDevice(img, dev, CL_IMAGE_HEIGHT, CL_DEVICE_IMAGE3D_MAX_HEIGHT) &&
            IsImageDimSupportedByDevice(img, dev, CL_IMAGE_DEPTH, CL_DEVICE_IMAGE3D_MAX_DEPTH);
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
        return IsImageDimSupportedByDevice(img, dev, CL_IMAGE_WIDTH, CL_DEVICE_IMAGE2D_MAX_WIDTH) &&
            IsImageDimSupportedByDevice(img, dev, CL_IMAGE_ARRAY_SIZE, CL_DEVICE_IMAGE_MAX_ARRAY_SIZE);
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        return IsImageDimSupportedByDevice(img, dev, CL_IMAGE_WIDTH, CL_DEVICE_IMAGE2D_MAX_WIDTH) &&
            IsImageDimSupportedByDevice(img, dev, CL_IMAGE_HEIGHT, CL_DEVICE_IMAGE2D_MAX_HEIGHT) &&
            IsImageDimSupportedByDevice(img, dev, CL_IMAGE_ARRAY_SIZE, CL_DEVICE_IMAGE_MAX_ARRAY_SIZE);
    default:
        assert(false && "Unknown image type");
        return false;
    }
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueCopyImage(
                                cl_command_queue clCommandQueue,
                                cl_mem           clSrcImage,
                                cl_mem           clDstImage,
                                const size_t     szSrcOrigin[MAX_WORK_DIM],
                                const size_t     szDstOrigin[MAX_WORK_DIM],
                                const size_t     szRegion[MAX_WORK_DIM],
                                cl_uint          uNumEventsInWaitList,
                                const cl_event*  cpEventWaitList,
                                cl_event*        pEvent,
                                ApiLogger* apiLogger
                                )
{
    cl_err_code errVal = CL_SUCCESS;
    if (NULL == szSrcOrigin || NULL == szDstOrigin || NULL == szRegion)
    {
        return CL_INVALID_VALUE;
    }
    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    if (pCommandQueue->GetContext()->IsFPGAEmulator())
    {
        return CL_INVALID_OPERATION;
    }

    SharedPtr<MemoryObject> pSrcImage = m_pContextModule->GetMemoryObject(clSrcImage);
    SharedPtr<MemoryObject> pDstImage = m_pContextModule->GetMemoryObject(clDstImage);
    if (NULL == pSrcImage || NULL == pDstImage)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pSrcImage->GetContext()->GetId() != pCommandQueue->GetContextId()  ||
        pSrcImage->GetContext()->GetId() != pDstImage->GetContext()->GetId()
        )
    {
        return CL_INVALID_CONTEXT;
    }

    // Check format
    errVal = CheckImageFormats(pSrcImage, pDstImage);
    if(CL_FAILED(errVal))
    {
        return CL_IMAGE_FORMAT_MISMATCH;
    }

    // Check boundaries.
    if (CL_SUCCESS != (errVal = pSrcImage->CheckBounds(szSrcOrigin,szRegion)) || CL_SUCCESS != (errVal = pDstImage->CheckBounds(szDstOrigin,szRegion)))
    {
        return errVal;
    }

    // Check overlapping
    if( clSrcImage == clDstImage)
    {
        if (CheckMemoryObjectOverlapping(pSrcImage, szSrcOrigin, szDstOrigin, szRegion))
        {
            return CL_MEM_COPY_OVERLAP;
        }
    }
    if (!AreImageDimsSupportedByDevice(*pSrcImage, *pCommandQueue->GetDefaultDevice()) ||
        !AreImageDimsSupportedByDevice(*pDstImage, *pCommandQueue->GetDefaultDevice()))
    {
        return CL_INVALID_IMAGE_SIZE;
    }
    if (CL_SUCCESS != (errVal = CheckImageFormatSupportedByDevice(*pCommandQueue->GetDefaultDevice(), *pSrcImage)))
    {
        return errVal;
    }
    if (CL_SUCCESS != (errVal = CheckImageFormatSupportedByDevice(*pCommandQueue->GetDefaultDevice(), *pDstImage)))
    {
        return errVal;
    }

    //
    // Input parameters validated, enqueue the command
    //
    Command* pCopyImageCmd = new CopyImageCommand(pCommandQueue, m_pOclEntryPoints, pSrcImage, pDstImage, szSrcOrigin, szDstOrigin, szRegion);
    if (NULL == pCopyImageCmd)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pCopyImageCmd->Init();
    if ( CL_FAILED(errVal) )
    {
        delete pCopyImageCmd;
        return  errVal;
    }

    // Enqueue copy command, never blocking
    errVal = pCopyImageCmd->EnqueueSelf(CL_FALSE, uNumEventsInWaitList, cpEventWaitList, pEvent, apiLogger);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pCopyImageCmd->CommandDone();
        delete pCopyImageCmd;
    }

    return  errVal;
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueCopyImageToBuffer(
                                cl_command_queue clCommandQueue,
                                cl_mem           clSrcImage,
                                cl_mem           clDstBuffer,
                                const size_t     szSrcOrigin[MAX_WORK_DIM],
                                const size_t     szRegion[MAX_WORK_DIM],
                                size_t           szDstOffset,
                                cl_uint          uNumEventsInWaitList,
                                const cl_event*  cpEventWaitList,
                                cl_event*        pEvent,
                                ApiLogger* apiLogger
                                )
{
    cl_err_code errVal = CL_SUCCESS;
    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    if (pCommandQueue->GetContext()->IsFPGAEmulator())
    {
        return CL_INVALID_OPERATION;
    }

    SharedPtr<MemoryObject> pSrcImage = m_pContextModule->GetMemoryObject(clSrcImage);
    SharedPtr<MemoryObject> pDstBuffer = m_pContextModule->GetMemoryObject(clDstBuffer);
    if (NULL == pSrcImage || NULL == pDstBuffer)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pSrcImage->GetContext()->GetId() != pCommandQueue->GetContextId()  ||
        pSrcImage->GetContext()->GetId() != pDstBuffer->GetContext()->GetId()
        )
    {
        return CL_INVALID_CONTEXT;
    }

    // Calculate dst_cb
    size_t szDstCb = CalcRegionSizeInBytes(pSrcImage, szRegion);

    // Check boundaries.
    if (CL_SUCCESS != (errVal = pSrcImage->CheckBounds(szSrcOrigin,szRegion)) || CL_SUCCESS != (errVal = pDstBuffer->CheckBounds(&szDstOffset,&szDstCb)))
    {
        return errVal;
    }
    // check that if pSrcImage is a 1D image buffer it wasn't created from pDstBuffer
    if (pSrcImage->GetType() == CL_MEM_OBJECT_IMAGE1D_BUFFER && pSrcImage->GetBackingStoreData() == pDstBuffer->GetBackingStoreData())
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (CL_SUCCESS != (errVal = CheckImageFormatSupportedByDevice(*pCommandQueue->GetDefaultDevice(), *pSrcImage)))
    {
        return errVal;
    }

    //
    // Input parameters validated, enqueue the command
    //
    size_t    pszDstOffset[3] = {szDstOffset,0,0};
    Command* pCopyImageToBufferCmd = new CopyImageToBufferCommand(pCommandQueue, m_pOclEntryPoints, pSrcImage, pDstBuffer, szSrcOrigin, szRegion, pszDstOffset/*szDstOffset*/);
    if (NULL == pCopyImageToBufferCmd)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pCopyImageToBufferCmd->Init();
    if ( CL_FAILED(errVal) )
    {
        delete pCopyImageToBufferCmd;
        return  errVal;
    }

    // Enqueue copy command, never blocking
    errVal = pCopyImageToBufferCmd->EnqueueSelf(CL_FALSE, uNumEventsInWaitList, cpEventWaitList, pEvent, apiLogger);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pCopyImageToBufferCmd->CommandDone();
        delete pCopyImageToBufferCmd;
    }

    return  errVal;
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::EnqueueCopyBufferToImage(
                                cl_command_queue clCommandQueue,
                                cl_mem           clSrcBuffer,
                                cl_mem           clDstImage,
                                size_t           szSrcOffset,
                                const size_t     szDstOrigin[MAX_WORK_DIM],
                                const size_t     szRegion[MAX_WORK_DIM],
                                cl_uint          uNumEventsInWaitList,
                                const cl_event*  cpEventWaitList,
                                cl_event*        pEvent,
                                ApiLogger* apiLogger
                                )
{
    cl_err_code errVal = CL_SUCCESS;
    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pCommandQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    if (pCommandQueue->GetContext()->IsFPGAEmulator())
    {
        return CL_INVALID_OPERATION;
    }

    SharedPtr<MemoryObject> pSrcBuffer = m_pContextModule->GetMemoryObject(clSrcBuffer);
    SharedPtr<MemoryObject> pDstImage = m_pContextModule->GetMemoryObject(clDstImage);
    if (NULL == pSrcBuffer || NULL == pDstImage)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (pSrcBuffer->GetContext()->GetId() != pCommandQueue->GetContextId()  ||
        pSrcBuffer->GetContext()->GetId() != pDstImage->GetContext()->GetId()
        )
    {
        return CL_INVALID_CONTEXT;
    }

    // Calculate dst_cb
    size_t szDstCb = CalcRegionSizeInBytes(pDstImage, szRegion);

    // Check boundaries.
    if (CL_SUCCESS != (errVal = pSrcBuffer->CheckBounds(&szSrcOffset,&szDstCb)) || CL_SUCCESS != (errVal = pDstImage->CheckBounds(szDstOrigin,szRegion)))
    {
        return errVal;
    }
    // check that if pDstImage is a 1D image buffer, it wasn't created from pSrcBuffer
    if (pDstImage->GetType() == CL_MEM_OBJECT_IMAGE1D_BUFFER && pDstImage->GetBackingStoreData() == pSrcBuffer->GetBackingStoreData())
    {
        return CL_INVALID_MEM_OBJECT;
    }

    if (CL_SUCCESS != (errVal = CheckImageFormatSupportedByDevice(*pCommandQueue->GetDefaultDevice(), *pDstImage)))
    {
        return errVal;
    }

    //
    // Input parameters validated, enqueue the command
    //

    size_t    pszSrcOffset[3] = {szSrcOffset,0,0};
    Command* pCopyBufferToImageCmd = new CopyBufferToImageCommand(pCommandQueue, m_pOclEntryPoints, pSrcBuffer, pDstImage, pszSrcOffset, szDstOrigin, szRegion);
    if (NULL == pCopyBufferToImageCmd)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errVal = pCopyBufferToImageCmd->Init();
    if ( CL_FAILED(errVal) )
    {
        delete pCopyBufferToImageCmd;
        return  errVal;
    }

    // Enqueue copy command, never blocking
    errVal = pCopyBufferToImageCmd->EnqueueSelf(CL_FALSE, uNumEventsInWaitList, cpEventWaitList, pEvent, apiLogger);
    if(CL_FAILED(errVal))
    {
        // Enqueue failed, free resources
        pCopyBufferToImageCmd->CommandDone();
        delete pCopyBufferToImageCmd;
    }

    return  errVal;
}

/******************************************************************
 *
 ******************************************************************/
void * ExecutionModule::EnqueueMapImage(
    cl_command_queue    clCommandQueue,
    cl_mem              clImage,
    cl_bool             bBlockingMap,
    cl_map_flags        clMapFlags,
    const size_t        szOrigin[MAX_WORK_DIM],
    const size_t        szRegion[MAX_WORK_DIM],
    size_t*             pszImageRowPitch,
    size_t*             pszImageSlicePitch,
    cl_uint             uNumEventsInWaitList,
    const cl_event*     cpEventWaitList,
    cl_event*           pEvent,
    cl_int*             pErrcodeRet,
    ApiLogger* apiLogger)
{
    cl_int err = CL_SUCCESS;
    if (NULL == pErrcodeRet)
    {
        pErrcodeRet = &err;
    }
    else
    {
        *pErrcodeRet = CL_SUCCESS;
    }
    if (NULL == szOrigin || NULL == szRegion)
    {
        *pErrcodeRet = CL_INVALID_VALUE;
        return NULL;
    }
    SharedPtr<IOclCommandQueueBase> pCommandQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    SharedPtr<MemoryObject> pImage = m_pContextModule->GetMemoryObject(clImage);

    if (NULL == pCommandQueue)
    {
        *pErrcodeRet = CL_INVALID_COMMAND_QUEUE;
    }
    else if (pCommandQueue->GetContext()->IsFPGAEmulator())
    {
        *pErrcodeRet = CL_INVALID_OPERATION;
    }
    else if (NULL == pImage)
    {
        *pErrcodeRet =  CL_INVALID_MEM_OBJECT;
    }
    else if ( CL_SUCCESS != checkMapFlagsMutex(clMapFlags) )
    {
        // Check that flags CL_MAP_READ or CL_MAP_WRITE only
        *pErrcodeRet = CL_INVALID_VALUE;
    }
    else if (CL_SUCCESS != pImage->ValidateMapFlags(clMapFlags))
    {
        *pErrcodeRet = CL_INVALID_VALUE;
    }
    else if (pImage->GetContext()->GetId() != pCommandQueue->GetContextId())
    {
        *pErrcodeRet = CL_INVALID_CONTEXT;
        return NULL;
    }
    else
    {
        if (CL_SUCCESS != (err = pImage->CheckBounds(szOrigin, szRegion)))
        {
            *pErrcodeRet = err;
        }
        else
        {
            const cl_mem_object_type imgType = pImage->GetType();
            if ((NULL == pszImageRowPitch)                                      ||
                ((CL_MEM_OBJECT_IMAGE3D == imgType || CL_MEM_OBJECT_IMAGE1D_ARRAY == imgType || CL_MEM_OBJECT_IMAGE2D_ARRAY == imgType) && (NULL == pszImageSlicePitch))
                )
            {
                *pErrcodeRet = CL_INVALID_VALUE;
            }
        }
    }
    if(CL_FAILED(*pErrcodeRet))
    {
        return NULL;
    }
    if (CL_SUCCESS != (*pErrcodeRet = CheckImageFormatSupportedByDevice(*pCommandQueue->GetDefaultDevice(), *pImage)))
    {
        return NULL;
    }
    if (false == pCommandQueue->GetEventsManager()->IsValidEventList(uNumEventsInWaitList, cpEventWaitList))
    {
        *pErrcodeRet = CL_INVALID_EVENT_WAIT_LIST;
        return NULL;
    }
    MapImageCommand* pMapImageCmd = new MapImageCommand(pCommandQueue, m_pOclEntryPoints, pImage, clMapFlags, szOrigin, szRegion, pszImageRowPitch, pszImageSlicePitch);

    // Must set device Id before init for image resource allocation.
    if (NULL == pMapImageCmd)
    {
        *pErrcodeRet = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }

    *pErrcodeRet = pMapImageCmd->Init();

    if(CL_FAILED(*pErrcodeRet))
    {
        delete pMapImageCmd;
        return  NULL;
    }

    // Get pointer for mapped region since it is allocated on init. Execute will lock the region
    // Note that if EnqueueCommand succeeded, by the time it returns, the command may be deleted already.
    void* mappedPtr = pMapImageCmd->GetMappedPtr();
    *pErrcodeRet = pMapImageCmd->EnqueueSelf(bBlockingMap, uNumEventsInWaitList, cpEventWaitList, pEvent, apiLogger);
    if(CL_FAILED(*pErrcodeRet))
    {
        pMapImageCmd->CommandDone();
        delete pMapImageCmd;
        return NULL;
    }

    return mappedPtr;
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code ExecutionModule::GetEventProfilingInfo (cl_event clEvent,
                                                    cl_profiling_info clParamName,
                                                    size_t szParamValueSize,
                                                    void * pParamValue,
                                                    size_t * pszParamValueSizeRet)
{
    cl_err_code res = m_pEventsManager->GetEventProfilingInfo(clEvent, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
    return res;
}

static cl_int CheckEventList(const SharedPtr<OclCommandQueue>& pQueue, cl_uint uiNumEventsInWaitList, const cl_event* pEventWaitList)
{
    std::vector<SharedPtr<OclEvent> > eventWaitListVec;
    if (!pQueue->GetEventsManager()->IsValidEventList(uiNumEventsInWaitList, pEventWaitList, &eventWaitListVec))
    {
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    return CL_SUCCESS;
}

cl_int ExecutionModule::EnqueueSVMFree(cl_command_queue clCommandQueue, cl_uint uiNumSvmPointers, void* pSvmPointers[],
                                    void (CL_CALLBACK* pfnFreeFunc)(cl_command_queue queue, cl_uint uiNumSvmPointers, void* pSvmPointers[],    void* pUserData),
                                    void* pUserData, cl_uint uiNumEventsInWaitList,    const cl_event* pEventWaitList,    cl_event* pEvent, ApiLogger* apiLogger)
{
    SharedPtr<IOclCommandQueueBase> pQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    if (FrameworkProxy::Instance()->GetOCLConfig()->GetOpenCLVersion() < OPENCL_VERSION_2_1)
    {
        if (0 == uiNumSvmPointers || nullptr == pSvmPointers)
        {
            return CL_INVALID_VALUE;
        }
    }
    else
    {
        if ((0 == uiNumSvmPointers && nullptr != pSvmPointers) ||
            (0 != uiNumSvmPointers && nullptr == pSvmPointers))
        {
            return CL_INVALID_VALUE;
        }
    }
    for (cl_uint i = 0; i < uiNumSvmPointers; i++)
    {
        if (NULL == pSvmPointers[i])
        {
            return CL_INVALID_VALUE;
        }
    }

    cl_err_code err = CheckEventList(pQueue, uiNumEventsInWaitList, pEventWaitList);
    if (CL_FAILED(err))
    {
        return err;
    }

    SVMFreeCommand* const pSvmFreeCmd = new SVMFreeCommand(uiNumSvmPointers, pSvmPointers, pfnFreeFunc, pUserData, pQueue, uiNumEventsInWaitList > 0);
    if (NULL == pSvmFreeCmd)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    err = pSvmFreeCmd->Init();
    if (CL_FAILED(err))
    {
        delete pSvmFreeCmd;
        return err;
    }
    err = pSvmFreeCmd->EnqueueSelf(false, uiNumEventsInWaitList, pEventWaitList, pEvent, apiLogger);
    if (CL_FAILED(err))
    {
        pSvmFreeCmd->CommandDone();
        delete pSvmFreeCmd;
        return err;
    }
    return CL_SUCCESS;
}

cl_err_code ExecutionModule::EnqueueSVMMigrateMem(cl_command_queue clCommandQueue,
                                             cl_uint num_svm_pointers,
                                             const void**     svm_pointers,
                                             const size_t*    sizes,
                                             cl_mem_migration_flags flags,
                                             cl_uint uiNumEventsInWaitList,
                                             const cl_event*  pEventWaitList,
                                             cl_event*        pEvent,
                                             ApiLogger*       apiLogger)
{
    if ((NULL == pEventWaitList && uiNumEventsInWaitList > 0) || (NULL != pEventWaitList && 0 == uiNumEventsInWaitList))
    {
        return CL_INVALID_EVENT_WAIT_LIST;
    }
    if (0 == num_svm_pointers || NULL == svm_pointers)
    {
        return CL_INVALID_VALUE;
    }

    SharedPtr<IOclCommandQueueBase> pQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    MigrateSVMMemCommand* pMigrateSVMCommand = new MigrateSVMMemCommand(pQueue, m_pContextModule, flags,
                                                                    num_svm_pointers, svm_pointers, sizes);

    if (NULL == pMigrateSVMCommand)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_err_code err = pMigrateSVMCommand->Init();
    if (CL_FAILED(err))
    {
        delete pMigrateSVMCommand;
        return err;
    }

    err = pMigrateSVMCommand->EnqueueSelf(/*Blocking*/CL_FALSE, uiNumEventsInWaitList, pEventWaitList, pEvent, apiLogger);
    if(CL_FAILED(err))
    {
        // Enqueue failed, free resources
        pMigrateSVMCommand->CommandDone();
        delete pMigrateSVMCommand;
    }

    return err;
}

cl_int ExecutionModule::EnqueueSVMMemcpy(cl_command_queue clCommandQueue, cl_bool bBlockingCopy, void* pDstPtr, const void* pSrcPtr, size_t size, cl_uint uiNumEventsInWaitList,
    const cl_event* pEventWaitList, cl_event* pEvent, ApiLogger* apiLogger)
{
    // validate parameters:
    SharedPtr<IOclCommandQueueBase> pQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }
    if (NULL == pDstPtr || NULL == pSrcPtr)
    {
        return CL_INVALID_VALUE;
    }
    if (m_opencl_ver < OPENCL_VERSION_2_1 && 0 == size)
    {
        return CL_INVALID_VALUE;
    }

    if (((char*)pDstPtr >= (char*)pSrcPtr && (char*)pDstPtr < (char*)pSrcPtr + size) || ((char*)pSrcPtr >= (char*)pDstPtr && (char*)pSrcPtr < (char*)pDstPtr + size))
    {
        return CL_MEM_COPY_OVERLAP;
    }

    SharedPtr<Context> pContext = pQueue->GetContext();
    SharedPtr<SVMBuffer> pSrcSvmBuffer = pContext->GetSVMBufferContainingAddr(const_cast<void*>(pSrcPtr));
    SharedPtr<SVMBuffer> pDstSvmBuffer = pContext->GetSVMBufferContainingAddr(pDstPtr);
    if ((pSrcSvmBuffer != NULL && !pSrcSvmBuffer->IsContainedInBuffer(pSrcPtr, size)) || (pDstSvmBuffer != NULL && !pDstSvmBuffer->IsContainedInBuffer(pDstPtr, size)))
    {
        LOG_ERROR(TEXT("either source or destination pointers define a region that spans beyond an SVM buffer"), "");
        return CL_INVALID_VALUE;
    }
    if ((pSrcSvmBuffer != NULL && pSrcSvmBuffer->GetContext() != pContext) || (pDstSvmBuffer != NULL && pDstSvmBuffer->GetContext() != pContext))
    {
        return CL_INVALID_VALUE;
    }

    cl_err_code err = CheckEventList(pQueue, uiNumEventsInWaitList, pEventWaitList);
    if (CL_FAILED(err))
    {
        return err;
    }

    // Do parallel copy.
    if (m_enableParallelCopy) {
        err = EnqueueLibraryCopy(pQueue, pDstPtr, pSrcPtr, size,
                                 true, false, true, false, bBlockingCopy,
                                 uiNumEventsInWaitList,
                                 pEventWaitList, pEvent, apiLogger,
                                 CL_COMMAND_SVM_MEMCPY);
        if (CL_SUCCEEDED(err))
            return err;
    }

    // do the work:
    Command* pCmd;
    const size_t
        pszSrcOrigin[] = { pSrcSvmBuffer != NULL ? (size_t)((char*)pSrcPtr - (char*)pSrcSvmBuffer->GetAddr()) : 0, 0, 0 },
        pszDstOrigin[] = { pDstSvmBuffer != NULL ? (size_t)((char*)pDstPtr - (char*)pDstSvmBuffer->GetAddr()) : 0, 0, 0 },
        pszRegion[] = { size, 1, 1 };
    if (NULL == pSrcSvmBuffer)
    {
        if (NULL == pDstSvmBuffer)
        {
            pCmd = new RuntimeSVMMemcpyCommand(pDstPtr, pSrcPtr, size, pQueue, uiNumEventsInWaitList > 0);
        }
        else
        {
            pCmd = new WriteSvmBufferCommand(pQueue, m_pOclEntryPoints, bBlockingCopy, pDstSvmBuffer, pszDstOrigin, pszRegion, pSrcPtr);
        }
    }
    else
    {
        if (NULL == pDstSvmBuffer)
        {
            pCmd = new ReadSvmBufferCommand(pQueue, m_pOclEntryPoints, pSrcSvmBuffer, pszSrcOrigin, pszRegion, pDstPtr);
        }
        else
        {
            pCmd = new CopySvmBufferCommand(pQueue, m_pOclEntryPoints, pSrcSvmBuffer, pDstSvmBuffer, pszSrcOrigin, pszDstOrigin, pszRegion);
        }
    }
    if (NULL == pCmd)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    err = pCmd->Init();
    if (CL_FAILED(err))
    {
        delete pCmd;
        return err;
    }
    err = pCmd->EnqueueSelf(bBlockingCopy, uiNumEventsInWaitList, pEventWaitList, pEvent, apiLogger);
    if (CL_FAILED(err))
    {
        pCmd->CommandDone();
        delete pCmd;
        return err;
    }
    return CL_SUCCESS;
}

cl_int ExecutionModule::EnqueueSVMMemFill(cl_command_queue clCommandQueue, void* pSvmPtr, const void* pPattern, size_t szPatternSize, size_t size, cl_uint uiNumEventsInWaitList,
                                          const cl_event* pEventWaitList, cl_event* pEvent, ApiLogger* apiLogger)
{
    // validate parameters:
    SharedPtr<IOclCommandQueueBase> pQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if (FrameworkProxy::Instance()->GetOCLConfig()->GetOpenCLVersion() < OPENCL_VERSION_2_1
            && 0 == size)
    {
        return CL_INVALID_VALUE;
    }

    cl_err_code err = CheckEventList(pQueue, uiNumEventsInWaitList, pEventWaitList);
    if (CL_FAILED(err))
    {
        return err;
    }
    if (NULL == pSvmPtr || !IS_ALIGNED_ON(pSvmPtr, szPatternSize) || NULL == pPattern || 0 == szPatternSize || !IsPowerOf2(szPatternSize) || szPatternSize > 128 ||
        size % szPatternSize != 0)
    {
        return CL_INVALID_VALUE;
    }

    SharedPtr<SVMBuffer> pSvmBuf = pQueue->GetContext()->GetSVMBufferContainingAddr(pSvmPtr);
    if (pSvmBuf != NULL && (pSvmBuf->GetContext() != pQueue->GetContext() || !pSvmBuf->IsContainedInBuffer(pSvmPtr, size)))
    {
        return CL_INVALID_VALUE;
    }

    // Do parallel fill.
    if (m_enableParallelCopy) {
        err = EnqueueLibrarySet(
            pQueue, pSvmPtr, pPattern, szPatternSize, size, true, false,
            uiNumEventsInWaitList, pEventWaitList, pEvent, apiLogger,
            CL_COMMAND_SVM_MEMFILL);
        if (CL_SUCCEEDED(err))
            return err;
    }

    // do the work:
    Command* pCmd;
    if (pSvmBuf != NULL)
    {
        pCmd = new FillSvmBufferCommand(pQueue, m_pOclEntryPoints, pSvmBuf, pPattern, szPatternSize, (ptrdiff_t)pSvmPtr - (ptrdiff_t)pSvmBuf->GetAddr(), size);
    }
    else
    {
        pCmd = new RuntimeSVMMemFillCommand(pSvmPtr, pPattern, szPatternSize, size, pQueue, uiNumEventsInWaitList > 0);
    }
    if (NULL == pCmd)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    err = pCmd->Init();
    if (CL_FAILED(err))
    {
        delete pCmd;
        return err;
    }
    err = pCmd->EnqueueSelf(false, uiNumEventsInWaitList, pEventWaitList, pEvent, apiLogger);
    if (CL_FAILED(err))
    {
        pCmd->CommandDone();
        delete pCmd;
        return err;
    }
    return CL_SUCCESS;
}

cl_int ExecutionModule::EnqueueSVMMap(cl_command_queue clCommandQueue, cl_bool bBlockingMap, cl_map_flags mapflags, void* pSvmPtr, size_t size, cl_uint uiNumEventsInWaitList,
                                      const cl_event* pEventWaitList, cl_event* pEvent, ApiLogger* apiLogger)
{
    SharedPtr<IOclCommandQueueBase> pQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    cl_err_code err = CheckEventList(pQueue, uiNumEventsInWaitList, pEventWaitList);
    if (CL_FAILED(err))
    {
        return err;
    }
    if (NULL == pSvmPtr || 0 == size)
    {
        return CL_INVALID_VALUE;
    }
    err = checkMapFlagsMutex(mapflags);
    if (CL_FAILED(err))
    {
        return err;
    }

    SharedPtr<SVMBuffer> pSvmBuf = pQueue->GetContext()->GetSVMBufferContainingAddr(pSvmPtr);

    if (pSvmBuf && (pSvmBuf->GetContext() != pQueue->GetContext() || !pSvmBuf->IsContainedInBuffer(pSvmPtr, size)))
    {
        return CL_INVALID_VALUE;
    }

    Command* pCmd = NULL;
    if (pSvmBuf)
    {
        pCmd = new MapSvmBufferCommand(pQueue, m_pOclEntryPoints, pSvmBuf, mapflags, (char*)pSvmPtr - (char*)pSvmBuf->GetAddr(), size);
    }
    else
    {
        // if it's a system pointer, it must work like a marker.
        pCmd = new SVMMAP_Command_NOOP(pQueue, uiNumEventsInWaitList);
    }

    if (NULL == pCmd)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    err = pCmd->Init();
    if (CL_FAILED(err))
    {
        delete pCmd;
        return err;
    }
    err = pCmd->EnqueueSelf(bBlockingMap, uiNumEventsInWaitList, pEventWaitList, pEvent, apiLogger);
    if (CL_FAILED(err))
    {
        pCmd->CommandDone();
        delete pCmd;
        return err;
    }
    return CL_SUCCESS;
}

cl_int ExecutionModule::EnqueueSVMUnmap(cl_command_queue clCommandQueue, void* pSvmPtr, cl_uint uiNumEventsInWaitList, const cl_event* pEventWaitList, cl_event* pEvent, ApiLogger* apiLogger)
{
    SharedPtr<IOclCommandQueueBase> pQueue = GetCommandQueue(clCommandQueue).DynamicCast<IOclCommandQueueBase>();
    if (NULL == pQueue)
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    cl_err_code err = CheckEventList(pQueue, uiNumEventsInWaitList, pEventWaitList);
    if (CL_FAILED(err))
    {
        return err;
    }
    if (NULL == pSvmPtr)
    {
        return CL_INVALID_VALUE;
    }

    SharedPtr<SVMBuffer> pSvmBuf = pQueue->GetContext()->GetSVMBufferContainingAddr(pSvmPtr);

    if (pSvmBuf && pSvmBuf->GetContext() != pQueue->GetContext())
    {
        return CL_INVALID_VALUE;
    }

    Command* pCmd = NULL;
    if(pSvmBuf)
    {
        pCmd = new UnmapSvmBufferCommand(pQueue, m_pOclEntryPoints, pSvmBuf, pSvmPtr);
    }
    else
    {
        // if it's a system pointer, it must work like a marker.
        pCmd = new SVMUNMAP_Command_NOOP(pQueue, uiNumEventsInWaitList);
    }

    if (NULL == pCmd)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    err = pCmd->Init();
    if (CL_FAILED(err))
    {
        delete pCmd;
        return err;
    }
    err = pCmd->EnqueueSelf(false, uiNumEventsInWaitList, pEventWaitList, pEvent, apiLogger);
    if (CL_FAILED(err))
    {
        pCmd->CommandDone();
        delete pCmd;
        return err;
    }
    return CL_SUCCESS;
}

bool ExecutionModule::CanAccessUSM(SharedPtr<IOclCommandQueueBase> &queue,
                                   SharedPtr<USMBuffer> &buf) {
    cl_device_id queueDeviceId = queue->GetQueueDeviceHandle();
    const SharedPtr<Context> context = queue->GetContext();
    assert(context && "Invalid queue context");
    SharedPtr<FissionableDevice> queueDevice =
        context->GetDevice(queueDeviceId);
    assert(queueDevice && "Invalid queue device");
    if (nullptr != buf.GetPtr()) {
        auto caps = queueDevice->GetUSMCapabilities(
            CL_DEVICE_CROSS_DEVICE_SHARED_MEM_CAPABILITIES_INTEL);
        cl_device_id bufDeviceId = buf->GetDevice();
        if (!caps && bufDeviceId && queueDeviceId != bufDeviceId)
            return false;
    } else {
        auto caps = queueDevice->GetUSMCapabilities(
            CL_DEVICE_SHARED_SYSTEM_MEM_CAPABILITIES_INTEL);
        if (!caps)
            return false;
    }
    return true;
}

cl_err_code ExecutionModule::EnqueueUSMMemset(cl_command_queue command_queue,
    void* dst_ptr, cl_int value, size_t size, cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list, cl_event* event, ApiLogger* api_logger)
{
    if (nullptr == dst_ptr)
        return CL_INVALID_VALUE;
    if (0 == size)
        return CL_SUCCESS;

    SharedPtr<IOclCommandQueueBase> queue =
        GetCommandQueue(command_queue).DynamicCast<IOclCommandQueueBase>();
    if (nullptr == queue.GetPtr())
        return CL_INVALID_COMMAND_QUEUE;

    cl_err_code err = CheckEventList(queue, num_events_in_wait_list,
                                     event_wait_list);
    if (CL_FAILED(err))
        return err;

    const SharedPtr<Context> context = queue->GetContext();
    SharedPtr<USMBuffer> usmBuf = context->GetUSMBufferContainingAddr(dst_ptr);
    if (nullptr != usmBuf.GetPtr() &&
        (usmBuf->GetContext() != context ||
         !usmBuf->IsContainedInBuffer(dst_ptr, size)))
        return CL_INVALID_VALUE;

    if (!CanAccessUSM(queue, usmBuf))
        return CL_INVALID_VALUE;

    // Do parallel set.
    if (m_enableParallelCopy) {
        size_t pattern_size = 1;
        err = EnqueueLibrarySet(
            queue, dst_ptr, (void *)&value, pattern_size, size, false, true,
            num_events_in_wait_list, event_wait_list, event, api_logger,
            CL_COMMAND_MEMSET_INTEL);
        if (CL_SUCCEEDED(err))
            return err;
    }

    // do the work:
    Command* memsetCommand;
    void *pattern = &value;
    size_t pattern_size = 1;
    if (nullptr != usmBuf.GetPtr())
        memsetCommand = new MemFillUsmBufferCommand(queue, m_pOclEntryPoints,
            usmBuf, pattern, pattern_size,
            (ptrdiff_t)dst_ptr - (ptrdiff_t)usmBuf->GetAddr(), size);
    else
        memsetCommand = new RuntimeUSMMemFillCommand(dst_ptr, pattern,
            pattern_size, size, queue, num_events_in_wait_list > 0);

    err = memsetCommand->Init();
    if (CL_FAILED(err))
    {
        delete memsetCommand;
        return err;
    }
    err = memsetCommand->EnqueueSelf(false, num_events_in_wait_list,
                                     event_wait_list, event, api_logger);
    if (CL_FAILED(err))
    {
        memsetCommand->CommandDone();
        delete memsetCommand;
        return err;
    }
    return CL_SUCCESS;
}

cl_err_code ExecutionModule::EnqueueUSMMemFill(cl_command_queue command_queue,
    void* dst_ptr, const void* pattern, size_t pattern_size, size_t size,
    cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
    cl_event* event, ApiLogger* api_logger)
{
    if (nullptr == dst_ptr || !IS_ALIGNED_ON(dst_ptr, pattern_size) ||
        nullptr == pattern || (size % pattern_size) != 0)
        return CL_INVALID_VALUE;
    if (!IsPowerOf2(pattern_size) || (pattern_size > MAX_PATTERN_SIZE))
        return CL_INVALID_VALUE;
    if (0 == size)
        return CL_SUCCESS;

    SharedPtr<IOclCommandQueueBase> queue =
        GetCommandQueue(command_queue).DynamicCast<IOclCommandQueueBase>();
    if (nullptr == queue.GetPtr())
        return CL_INVALID_COMMAND_QUEUE;

    cl_err_code err = CheckEventList(queue, num_events_in_wait_list,
                                     event_wait_list);
    if (CL_FAILED(err))
        return err;

    const SharedPtr<Context> context = queue->GetContext();
    SharedPtr<USMBuffer> usmBuf = context->GetUSMBufferContainingAddr(dst_ptr);
    if (nullptr != usmBuf.GetPtr() &&
        (usmBuf->GetContext() != context ||
         !usmBuf->IsContainedInBuffer(dst_ptr, size)))
        return CL_INVALID_VALUE;

    if (!CanAccessUSM(queue, usmBuf))
        return CL_INVALID_VALUE;

    // Do parallel fill if pattern contains the same char value.
    if (m_enableParallelCopy) {
        err = EnqueueLibrarySet(
            queue, dst_ptr, pattern, pattern_size, size, false, true,
            num_events_in_wait_list, event_wait_list, event, api_logger,
            CL_COMMAND_MEMFILL_INTEL);
        if (CL_SUCCEEDED(err))
            return err;
    }

    // do the work:
    Command* memFillCommand;
    if (nullptr != usmBuf.GetPtr())
        memFillCommand = new MemFillUsmBufferCommand(queue, m_pOclEntryPoints,
            usmBuf, pattern, pattern_size,
            (ptrdiff_t)dst_ptr - (ptrdiff_t)usmBuf->GetAddr(), size);
    else
        memFillCommand = new RuntimeUSMMemFillCommand(dst_ptr, pattern,
            pattern_size, size, queue, num_events_in_wait_list > 0);

    err = memFillCommand->Init();
    if (CL_FAILED(err))
    {
        delete memFillCommand;
        return err;
    }
    err = memFillCommand->EnqueueSelf(false, num_events_in_wait_list,
                                      event_wait_list, event, api_logger);
    if (CL_FAILED(err))
    {
        memFillCommand->CommandDone();
        delete memFillCommand;
        return err;
    }
    return CL_SUCCESS;
}

cl_err_code ExecutionModule::EnqueueUSMMemcpy(cl_command_queue command_queue,
    cl_bool blocking, void* dst_ptr, const void* src_ptr, size_t size,
    cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
    cl_event* event, ApiLogger* api_logger)
{
    SharedPtr<IOclCommandQueueBase> queue =
        GetCommandQueue(command_queue).DynamicCast<IOclCommandQueueBase>();
    if (nullptr == queue.GetPtr())
        return CL_INVALID_COMMAND_QUEUE;

    if (nullptr == dst_ptr || nullptr == src_ptr)
        return CL_INVALID_MEM_OBJECT;

    if (((char*)dst_ptr >= (char*)src_ptr &&
         (char*)dst_ptr < (char*)src_ptr + size) ||
        ((char*)src_ptr >= (char*)dst_ptr &&
         (char*)src_ptr < (char*)dst_ptr + size))
        return CL_MEM_COPY_OVERLAP;

    SharedPtr<Context> context = queue->GetContext();
    SharedPtr<USMBuffer> srcBuffer = context->GetUSMBufferContainingAddr(
        const_cast<void*>(src_ptr));
    SharedPtr<USMBuffer> dstBuffer = context->GetUSMBufferContainingAddr(
        dst_ptr);
    if ((nullptr != srcBuffer.GetPtr() &&
         !srcBuffer->IsContainedInBuffer(src_ptr, size)) ||
        (nullptr != dstBuffer.GetPtr() &&
         !dstBuffer->IsContainedInBuffer(dst_ptr, size)))
    {
        LOG_ERROR(TEXT("either source or destination pointers define a region"
            " that spans beyond an USM buffer"), "");
        return CL_INVALID_VALUE;
    }
    if ((nullptr != srcBuffer.GetPtr() &&
         srcBuffer->GetContext() != context) ||
        (nullptr != dstBuffer.GetPtr() &&
         dstBuffer->GetContext() != context))
        return CL_INVALID_VALUE;

   // Always allow src or dst to be non-USM pointer.
   if ((nullptr != srcBuffer.GetPtr() && !CanAccessUSM(queue, srcBuffer)) ||
       (nullptr != dstBuffer.GetPtr() && !CanAccessUSM(queue, dstBuffer)))
        return CL_INVALID_VALUE;

    cl_err_code err = CheckEventList(queue, num_events_in_wait_list,
                                     event_wait_list);
    if (CL_FAILED(err))
        return err;

    // Do parallel copy.
    if (m_enableParallelCopy) {
        err = EnqueueLibraryCopy(
            queue, dst_ptr, src_ptr, size, false, true, false, true, blocking,
            num_events_in_wait_list, event_wait_list, event, api_logger,
            CL_COMMAND_MEMCPY_INTEL);
        if (CL_SUCCEEDED(err))
            return err;
    }

    // do the work:
    Command* memcpyCommand;
    const size_t srcOrigin[] = { nullptr != srcBuffer.GetPtr() ?
            (size_t)((char*)src_ptr - (char*)srcBuffer->GetAddr()) : 0, 0, 0 };
    const size_t dstOrigin[] = { nullptr != dstBuffer.GetPtr() ?
            (size_t)((char*)dst_ptr - (char*)dstBuffer->GetAddr()) : 0, 0, 0 };
    const size_t region[] = { size, 1, 1 };
    if (nullptr == srcBuffer.GetPtr())
    {
        if (nullptr == dstBuffer.GetPtr())
            memcpyCommand = new RuntimeUSMMemcpyCommand(dst_ptr, src_ptr, size,
                queue, num_events_in_wait_list > 0);
        else
            memcpyCommand = new WriteUsmBufferCommand(queue, m_pOclEntryPoints,
                blocking, dstBuffer, dstOrigin, region, src_ptr);
    }
    else
    {
        if (nullptr == dstBuffer.GetPtr())
            memcpyCommand = new ReadUsmBufferCommand(queue, m_pOclEntryPoints,
                srcBuffer, srcOrigin, region, dst_ptr);
        else
            memcpyCommand = new CopyUsmBufferCommand(queue, m_pOclEntryPoints,
                srcBuffer, dstBuffer, srcOrigin, dstOrigin, region);
    }

    err = memcpyCommand->Init();
    if (CL_FAILED(err))
    {
        delete memcpyCommand;
        return err;
    }
    err = memcpyCommand->EnqueueSelf(blocking, num_events_in_wait_list,
                                     event_wait_list, event, api_logger);
    if (CL_FAILED(err))
    {
        memcpyCommand->CommandDone();
        delete memcpyCommand;
        return err;
    }
    return CL_SUCCESS;
}

cl_err_code ExecutionModule::EnqueueUSMMigrateMem(
    cl_command_queue command_queue, const void* ptr, size_t size,
    cl_mem_migration_flags_intel flags, cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list, cl_event* event, ApiLogger* api_logger)
{
    // TODO: it is unresolved in spec (rev. O) whether nullptr is an invalid
    // value for ptr and whether 0 is invalid for size.
    if (nullptr == ptr || 0 == size || 0 == flags ||
        (flags & ~(CL_MIGRATE_MEM_OBJECT_HOST_INTEL |
                   CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED_INTEL)))
        return CL_INVALID_VALUE;

    SharedPtr<IOclCommandQueueBase> queue =
        GetCommandQueue(command_queue).DynamicCast<IOclCommandQueueBase>();
    if (nullptr == queue.GetPtr())
        return CL_INVALID_COMMAND_QUEUE;

    SharedPtr<Context> context = queue->GetContext();
    SharedPtr<USMBuffer> usmBuf = context->GetUSMBufferContainingAddr(
        const_cast<void*>(ptr));
    if (!CanAccessUSM(queue, usmBuf))
        return CL_INVALID_VALUE;

    cl_err_code err = CheckEventList(queue, num_events_in_wait_list,
                                     event_wait_list);
    if (CL_FAILED(err))
        return err;

    MigrateUSMMemCommand* migrateCommand = new MigrateUSMMemCommand(
        queue, m_pContextModule, flags, ptr, size );

    err = migrateCommand->Init();
    if (CL_FAILED(err))
    {
        delete migrateCommand;
        return err;
    }

    err = migrateCommand->EnqueueSelf(CL_FALSE, num_events_in_wait_list,
                                       event_wait_list, event, api_logger);
    if(CL_FAILED(err))
    {
        // Enqueue failed, free resources
        migrateCommand->CommandDone();
        delete migrateCommand;
    }

    return CL_SUCCESS;
}

cl_err_code ExecutionModule::EnqueueUSMMemAdvise(
    cl_command_queue command_queue, const void* ptr, size_t size,
    cl_mem_advice_intel advice, cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list, cl_event* event, ApiLogger* api_logger)
{
    // TODO: it is unresolved in spec (rev. H) whether nullptr is an invalid
    // value for ptr and whether 0 is invalid for size.
    if (nullptr == ptr || 0 == size)
        return CL_INVALID_VALUE;

    SharedPtr<IOclCommandQueueBase> queue =
        GetCommandQueue(command_queue).DynamicCast<IOclCommandQueueBase>();
    if (nullptr == queue.GetPtr())
        return CL_INVALID_COMMAND_QUEUE;

    SharedPtr<Context> context = queue->GetContext();
    SharedPtr<USMBuffer> usmBuf = context->GetUSMBufferContainingAddr(
        const_cast<void*>(ptr));
    if (!CanAccessUSM(queue, usmBuf))
        return CL_INVALID_VALUE;

    cl_err_code err = CheckEventList(queue, num_events_in_wait_list,
                                     event_wait_list);
    if (CL_FAILED(err))
        return err;

    // Check if advice is zero or is not supported advice for the device
    // associated with command_queue.
    if (0 == advice)
        return CL_INVALID_VALUE;

    AdviseUSMMemCommand* adviseCommand = new AdviseUSMMemCommand(
        queue, m_pContextModule, ptr, size, advice);

    err = adviseCommand->Init();
    if (CL_FAILED(err))
    {
        delete adviseCommand;
        return err;
    }

    err = adviseCommand->EnqueueSelf(CL_FALSE, num_events_in_wait_list,
                                     event_wait_list, event, api_logger);
    if(CL_FAILED(err))
    {
        // Enqueue failed, free resources
        adviseCommand->CommandDone();
        delete adviseCommand;
    }

    return CL_SUCCESS;
}

cl_err_code ExecutionModule::EnqueueLibraryCopy(
    SharedPtr<IOclCommandQueueBase> &queue,
    void *dst, const void *src, size_t size, bool is_dst_svm, bool is_dst_usm,
    bool is_src_svm, bool is_src_usm, cl_bool blocking,
    cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
    cl_event *event, ApiLogger *api_logger, cl_command_type cmdType) {
    LOG_DEBUG(TEXT("%s"), TEXT("EnqueueLibraryCopy enter"));
    if (size < DEV_PARALLEL_COPY_MIN_SIZE)
        return CL_INVALID_VALUE;

    SharedPtr<Context> context = queue->GetContext();
    // Setup kernel.
    std::string kernelName = "copy";
    SharedPtr<Kernel> kernel =
        context->GetContextModule().GetLibraryKernel(context, kernelName);
    if (!kernel) {
        LOG_ERROR(TEXT("EnqueueLibraryCopy GetLibraryKernel failed"), "");
        return CL_OUT_OF_RESOURCES;
    }
    if (kernel->GetContext()->GetId() != queue->GetContextId()) {
        LOG_ERROR(TEXT("EnqueueLibraryCopy kernel context is invalid"), "");
        return CL_INVALID_CONTEXT;
    }
    assert(kernel->GetKernelArgsCount() == 2 && "Invalid args count");
    cl_err_code err = kernel->SetKernelArg(0, sizeof(void*), dst,
                                           is_dst_svm, is_dst_usm);
    if (CL_FAILED(err)) {
        LOG_ERROR(
            TEXT("EnqueueLibraryCopy SetKernelArg 0 failed, err = %d"), err);
        return CL_INVALID_VALUE;
    }
    err = kernel->SetKernelArg(1, sizeof(void*), src, is_src_svm, is_src_usm);
    if (CL_FAILED(err)) {
        LOG_ERROR(
            TEXT("EnqueueLibraryCopy SetKernelArg 1 failed, err = %d"), err);
        return CL_INVALID_VALUE;
    }

    const size_t *offset = nullptr;
    const size_t gdim[1] = {size};
    const size_t *ldim = nullptr;
    cl_uint ndim = 1;

    const SharedPtr<FissionableDevice>& device = queue->GetDefaultDevice();
    NDRangeKernelCommand* cmd = new NDRangeKernelCommand(
        queue, m_pOclEntryPoints, kernel, ndim, offset, gdim, ldim);
    cmd->SetDevice(device);
    cmd->SetCommandType(cmdType);
    err = cmd->Init();
    if (CL_FAILED(err)) {
        LOG_ERROR(TEXT("EnqueueLibraryCopy cmd->Init failed, err = %d"), err);
        delete cmd;
        return err;
    }
    err = cmd->EnqueueSelf(blocking, num_events_in_wait_list, event_wait_list,
                           event, api_logger);
    if(CL_FAILED(err)) {
        LOG_ERROR(TEXT("EnqueueLibraryCopy EnqueueSelf failed, err = %d"), err);
        cmd->CommandDone();
        delete cmd;
        return err;
    }
    return CL_SUCCESS;
}

cl_err_code ExecutionModule::EnqueueLibrarySet(
    SharedPtr<IOclCommandQueueBase> &queue, void *dst, const void *pattern,
    size_t pattern_size, size_t size, bool is_dst_svm, bool is_dst_usm,
    cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
    cl_event *event, ApiLogger *api_logger, cl_command_type cmdType) {
    LOG_DEBUG(TEXT("%s"), TEXT("EnqueueLibrarySet enter"));
    if (size < DEV_PARALLEL_COPY_MIN_SIZE)
        return CL_INVALID_VALUE;

    // Check pattern has the same value.
    unsigned char *p = (unsigned char*)pattern;
    unsigned char value = p[0];
    if (!std::all_of(p, p + pattern_size,
                     [&](unsigned char v){ return v == value; }))
        return CL_INVALID_VALUE;

    SharedPtr<Context> context = queue->GetContext();

    // Setup kernel.
    std::string kernelName = (value == 0) ? "set_zero" : "set";
    SharedPtr<Kernel> kernel =
        context->GetContextModule().GetLibraryKernel(context, kernelName);
    if (!kernel) {
        LOG_ERROR(TEXT("EnqueueLibrarySet GetLibraryKernel failed"), "");
        return CL_OUT_OF_RESOURCES;
    }
    if (kernel->GetContext()->GetId() != queue->GetContextId()) {
        LOG_ERROR(TEXT("EnqueueLibrarySet kernel context is invalid"), "");
        return CL_INVALID_CONTEXT;
    }
    size_t argsCount = kernel->GetKernelArgsCount();
    assert((argsCount == 1 || argsCount == 2) && "Invalid args count");
    cl_err_code err = kernel->SetKernelArg(0, sizeof(void*), dst,
                                           is_dst_svm, is_dst_usm);
    if (CL_FAILED(err)) {
        LOG_ERROR(
            TEXT("EnqueueLibrarySet SetKernelArg 0 failed, err = %d"), err);
        return CL_INVALID_VALUE;
    }
    if (argsCount == 2) {
        err = kernel->SetKernelArg(1, sizeof(value), &value, false, false);
        if (CL_FAILED(err)) {
            LOG_ERROR(
                TEXT("EnqueueLibrarySet SetKernelArg 1 failed, err = %d"), err);
            return CL_INVALID_VALUE;
        }
    }

    const size_t *offset = nullptr;
    const size_t gdim[1] = {size};
    const size_t *ldim = nullptr;
    cl_uint ndim = 1;

    const SharedPtr<FissionableDevice>& device = queue->GetDefaultDevice();
    NDRangeKernelCommand* cmd = new NDRangeKernelCommand(
        queue, m_pOclEntryPoints, kernel, ndim, offset, gdim, ldim);
    cmd->SetDevice(device);
    cmd->SetCommandType(cmdType);
    err = cmd->Init();
    if (CL_FAILED(err)) {
        LOG_ERROR(TEXT("EnqueueLibrarySet cmd->Init failed, err = %d"), err);
        delete cmd;
        return err;
    }
    err = cmd->EnqueueSelf(false, num_events_in_wait_list, event_wait_list,
                           event, api_logger);
    if(CL_FAILED(err)) {
        LOG_ERROR(TEXT("EnqueueLibrarySet EnqueueSelf failed, err = %d"), err);
        cmd->CommandDone();
        delete cmd;
        return err;
    }
    return CL_SUCCESS;
}

class QueueFlusher
{
public:

    QueueFlusher(cl_context context) : m_context() { }

    bool operator()(const SharedPtr<OCLObject<_cl_command_queue_int, _cl_context_int> >& pObj)
    {
        assert(pObj != NULL && "got NULL for queue");
        if (NULL == pObj)
        {
            return false;
        }

        SharedPtr<IOclCommandQueueBase> pQueue = pObj.DynamicCast<IOclCommandQueueBase>();
        if (NULL == pQueue)
        {
            return false;
        }

        cl_context queueContext = (cl_context)pQueue->GetParentHandle();
        if (queueContext == m_context)
        {
            pQueue->Flush(false);
        }
        return true;
    }

private:

    cl_context m_context;
};

cl_err_code ExecutionModule::FlushAllQueuesForContext(cl_context clEventsContext)
{
    QueueFlusher flusher(clEventsContext);
    if (!m_pOclCommandQueueMap->ForEach(flusher))
    {
        return CL_ERR_KEY_NOT_FOUND;
    }
    return CL_SUCCESS;
}

void ExecutionModule::DeleteAllActiveQueues(bool preserve_user_handles)
{
    m_pOclCommandQueueMap->DisableAdding();
    if (preserve_user_handles)
    {
        m_pOclCommandQueueMap->SetPreserveUserHandles();
    }
    m_pOclCommandQueueMap->ReleaseAllObjects(false);
}

namespace {
/**
 * internal util function.
 */
cl_int checkMapFlagsMutex(const cl_map_flags clMapFlags)
{
    if (0 == ( clMapFlags & (CL_MAP_READ | CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION) ) )
        return CL_SUCCESS;

    if ( (clMapFlags & CL_MAP_WRITE_INVALIDATE_REGION) & (CL_MAP_READ | CL_MAP_WRITE) )
    {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}

/**
 * Callback for pEvent status change. if it's changed to CL_COMPLETE, we need to remove
 * it from kernel-event map.
 */
void callbackForKernelEventMap(cl_event pEvent, cl_int returnStatus, void *data) {
    OclAutoMutex mu(&KernelEventMutex);
    std::map<std::string, cl_event> *kernelevent_map =
        (std::map<std::string, cl_event> *)data;
    assert(kernelevent_map && "Kernel-event map should not be null.");
    for (auto it = kernelevent_map->begin(); it != kernelevent_map->end(); it++)
    {
       if (pEvent == it->second)
       {
           kernelevent_map->erase(it->first);
           break;
       }
    }
    return;
}

} //anonymous namespace
