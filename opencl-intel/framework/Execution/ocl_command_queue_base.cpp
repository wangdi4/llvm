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

#include <cassert>
#include <cl_types.h>
#include "ocl_event.h"
#include "command_queue.h"
#include "events_manager.h"
#include "enqueue_commands.h"
#include "execution_module.h"
#include <cstring>
#include "ocl_itt.h"
#include "cl_shared_ptr.hpp"
#include "Context.h"
#include "context_module.h"
#include "cl_user_logger.h"


using namespace Intel::OpenCL::Framework;

cl_err_code IOclCommandQueueBase::EnqueueCommand(Command* pCommand, cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pUserEvent, ApiLogger* apiLogger)
{
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
      if ( (NULL != m_pGPAData) && m_pGPAData->bUseGPA )
      {
        static __thread __itt_string_handle* pTaskName = NULL;
        if ( NULL == pTaskName )
        {
          pTaskName = __itt_string_handle_create("IOclCommandQueueBase::EnqueueCommand()");
        }
        __itt_task_begin(m_pGPAData->pAPIDomain, __itt_null, __itt_null, pTaskName);
      }
#endif


    const SharedPtr<QueueEvent>& pQueueEvent  = pCommand->GetEvent();
    cl_event                     pEventHandle = pQueueEvent->GetHandle();
    assert(NULL != pQueueEvent);    // klocwork
    if (m_bProfilingEnabled)
    {
        pQueueEvent->SetProfilingInfo(CL_PROFILING_COMMAND_QUEUED, m_pDefaultDevice->GetDeviceAgent()->clDevGetPerformanceCounter());
    }
    pQueueEvent->AddProfilerMarker("QUEUED", ITT_SHOW_QUEUED_MARKER);

    cl_err_code errVal = CL_SUCCESS;
    cl_event waitEvent = NULL;
    cl_event* pEvent;
    if(NULL != pUserEvent)
    {
        pQueueEvent->SetVisibleToUser();
    }
    // If blocking and no event, than it is needed to create dummy cl_event for wait
    if( bBlocking && NULL == pUserEvent)
    {
        pEvent = &waitEvent;
    }
    else
    {
        pEvent = pUserEvent;
    }
    m_pEventsManager->RegisterQueueEvent(pQueueEvent, pEvent);
    if (apiLogger != NULL)
    {
        apiLogger->SetCmdId(pQueueEvent->GetId());
    }

    AddFloatingDependence(pQueueEvent);
    errVal = m_pEventsManager->RegisterEvents(pQueueEvent, uNumEventsInWaitList, cpEeventWaitList, !IsOutOfOrderExecModeEnabled(), GetId());

    if( CL_FAILED(errVal))
    {
        RemoveFloatingDependence(pQueueEvent);
        if (NULL == pUserEvent)
        {
            m_pEventsManager->ReleaseEvent(pEventHandle);
        }
        return errVal;
    }

    errVal = Enqueue(pCommand);

    // RemoveFloatingDependence() must to be after Enqueue; this prevents a situation where the current
    // command is dependent on another command which just finished (the other one) After ::RegisterEvents
    // and before ::Enqueue resulting in a situation where the same command gets submitted twice; once here
    // by ::Enqueue and the other one by ::NotifyCommandStatusChange of the other command.
    SharedPtr<QueueEvent> refCountedQueueEvent;
    if (bBlocking)
    {
        // ensure Command and Event will not disapper after execution
        refCountedQueueEvent = pQueueEvent;
    }
    RemoveFloatingDependence(pQueueEvent);

    if (CL_FAILED(errVal))
    {
        pCommand->CommandDone();
        if (NULL == pUserEvent)
        {
            m_pEventsManager->ReleaseEvent(pEventHandle);
        }
        return CL_ERR_FAILURE;
    }

    // If blocking, wait for object
    if(bBlocking)
    {
        if ( ( RUNTIME_EXECUTION_TYPE == pCommand->GetExecutionType() ) || CL_FAILED(WaitForCompletion(refCountedQueueEvent)) )
        {
            refCountedQueueEvent->Wait();
        }
        //If the event is not visible to the user, remove its floating reference count and as a result the pendency representing the object is visible to the user
        if (NULL == pUserEvent)
        {
            m_pEventsManager->ReleaseEvent(pEventHandle);
        }
    }
    else
    {
        //If the event is not visible to the user, remove its floating reference count and as a result the pendency representing the object is visible to the user
        if (NULL == pUserEvent)
        {
            m_pEventsManager->ReleaseEvent(pEventHandle);
        }
    }

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (NULL != m_pGPAData) && m_pGPAData->bUseGPA )
    {
      __itt_task_end(m_pGPAData->pAPIDomain); // "ExecutionModule::EnqueueNDRangeKernel()->CommandCreation()"
    }
#endif

    return CL_SUCCESS;
}



cl_err_code IOclCommandQueueBase::EnqueueRuntimeCommandWaitEvents(RUNTIME_COMMAND_TYPE type, 
                                                    Command* pCommand, cl_uint uNumEventsInWaitList, const cl_event* pEventWaitList, cl_event* pEvent, ApiLogger* pApiLogger)
{
    const SharedPtr<QueueEvent>& pQueueEvent  = pCommand->GetEvent();
    cl_event                     pEventHandle = pQueueEvent->GetHandle();
    assert(NULL != pQueueEvent);    // klocwork

    cl_err_code errVal = CL_SUCCESS;

    m_pEventsManager->RegisterQueueEvent(pQueueEvent, pEvent);

    AddFloatingDependence(pQueueEvent);
    errVal = m_pEventsManager->RegisterEvents(pQueueEvent, uNumEventsInWaitList, pEventWaitList);
    if (NULL != pApiLogger)
    {
        pApiLogger->SetCmdId(pQueueEvent->GetId());
    }

    if( CL_FAILED(errVal))
    {
        RemoveFloatingDependence(pQueueEvent);
        if (NULL == pEvent)
        {
            m_pEventsManager->ReleaseEvent(pEventHandle);
        }
        return errVal;
    }

    switch (type)
    {
        case JUST_WAIT:
            errVal = EnqueueWaitForEvents(pCommand);
            break;

        case MARKER:
            errVal = EnqueueMarkerWaitForEvents(pCommand);
            break;

        case BARRIER:
            errVal = EnqueueBarrierWaitForEvents(pCommand);
            break;

        default:
            assert( 0 && "Unknown runtime command type" );
            errVal = CL_ERR_FAILURE;
    }

    // RemoveFloatingDependence() must to be after Enqueue; this prevents a situation where the current
    // command is dependent on another command which just finished (the other one) After ::RegisterEvents
    // and before ::Enqueue resulting in a situation where the same command gets submitted twice; once here
    // by ::Enqueue and the other one by ::NotifyCommandStatusChange of the other command.
    RemoveFloatingDependence(pQueueEvent);

    //If the event is not visible to the user, remove its floating reference count and as a result the pendency representing the object is visible to the user
    if (NULL == pEvent)
    {
        m_pEventsManager->ReleaseEvent(pEventHandle);
    }
    
    if (CL_FAILED(errVal))
    {
        return CL_ERR_FAILURE;
    }

    return CL_SUCCESS;

}

cl_err_code IOclCommandQueueBase::WaitForCompletion(const SharedPtr<QueueEvent>& pEvent)
{
    // Make blocking flush to ensure everything ends in the device's command list before we join its execution
    Flush(true);

    cl_dev_cmd_desc* pCmdDesc = pEvent->GetCommand()->GetDeviceCommandDescriptor();

    cl_dev_err_code ret = m_pDefaultDevice->GetDeviceAgent()->clDevCommandListWaitCompletion(
        m_clDevCmdListId, pCmdDesc);

    // If device does't supporting waiting, need to call explicit Wait() method
    if ( CL_DEV_NOT_SUPPORTED == ret )
    {
        pEvent->Wait();
        ret = CL_DEV_SUCCESS;
        // After wait completed we should continue to regular execution path
    }

    OclEventState state = pEvent->GetEventState();
    
    while ( CL_DEV_SUCCEEDED(ret) && (EVENT_STATE_DONE != state) )
    {
        clSleep(0);
        ret = m_pDefaultDevice->GetDeviceAgent()->clDevCommandListWaitCompletion(
                m_clDevCmdListId, pCmdDesc);
        state = pEvent->GetEventState();
    }


    return CL_DEV_SUCCEEDED(ret) ? CL_SUCCESS : CL_INVALID_OPERATION;
}

 /******************************************************************
 * This object holds extra reference count as it is recorded in ContextModule
 ******************************************************************/
void IOclCommandQueueBase::EnterZombieState( EnterZombieStateLevel call_level )
{
    m_pContext->GetContextModule().CommandQueueRemoved( this );
    OclCommandQueue::EnterZombieState(RECURSIVE_CALL);
}


void IOclCommandQueueBase::NotifyCommandFailed( cl_err_code err , const CommandSharedPtr<>& command ) const
{
    if ( NULL != command)
    {
        std::stringstream stream;
        _cl_event_int* handle = NULL;
        if(command->GetEvent()->GetVisibleToUser())
        {
            handle = command->GetEvent()->GetHandle();
        }

        if (g_pUserLogger->IsErrorLoggingEnabled())
        {
            stream << "Command failed. " << "command type: " << command->GetCommandName();
            stream << ", command id: " << command->GetEvent()->GetId();
            stream << ", result value: " << err;
            stream << ", The cl_event value associated with the command: 0x" << handle;
            g_pUserLogger->PrintError(stream.str());
            stream.str(std::string());
        }
      
        stream << "A command failed with return value: " << err;
        stream << ", the cl_event value associated with the command is in the private_info "<<
            "parameter, and its value is: 0x"<< handle <<". for more information use logging.";
        const std::string& tmp = stream.str();
        GetContext()->NotifyError( tmp.c_str() , handle , sizeof(handle) );
    }
}

void IOclCommandQueueBase::BecomeVisible()
{
    // register itself in the context_codule.
    // Note - this will increase RefCount - need to unregister when refcount drops to 1!
    m_pContext->GetContextModule().CommandQueueCreated( this );
}
