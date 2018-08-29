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

#include "out_of_order_command_queue.h"
#include "ocl_event.h"
#include "enqueue_commands.h"
#include "Device.h"
#include "events_manager.h"
#include <assert.h>
#include <cl_utils.h>


using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

OutOfOrderCommandQueue::OutOfOrderCommandQueue(
    SharedPtr<Context>                    pContext,
    cl_device_id                clDefaultDeviceID, 
    cl_command_queue_properties clProperties,
    EventsManager*              pEventManager
    ) :
    IOclCommandQueueBase(pContext, clDefaultDeviceID, clProperties, pEventManager),
    m_depOnAll(nullptr),
    m_commandsInExecution(0),
    m_lastBarrier(nullptr),
    m_unflushedCommands(0)
{        
}

OutOfOrderCommandQueue::~OutOfOrderCommandQueue() 
{    
}

cl_err_code OutOfOrderCommandQueue::Initialize()
{
     cl_dev_subdevice_id subdevice_id = m_pContext->GetSubdeviceId(m_clDefaultDeviceHandle);
     cl_dev_err_code retDev = m_pDefaultDevice->GetDeviceAgent()->clDevCreateCommandList(CL_DEV_LIST_ENABLE_OOO, subdevice_id, &m_clDevCmdListId);
     if (CL_DEV_FAILED(retDev))
     {
         m_clDevCmdListId = 0;
         return CL_OUT_OF_RESOURCES;
     }

     Command* pDepOnAll = new MarkerCommand(this, 0);
     if (nullptr == pDepOnAll)
     {
         return CL_OUT_OF_HOST_MEMORY;
     }
     // This floating dependence will be resolved at the completion of clEnqueueMarker/Barrier sequence to this queue (AddDependentOnAll)
     pDepOnAll->GetEvent()->AddFloatingDependence();
     pDepOnAll->GetEvent()->Release();
     m_depOnAll = pDepOnAll;
     return CL_SUCCESS;    
}

long OutOfOrderCommandQueue::Release()
{
    const long ref = IOclCommandQueueBase::Release();
    if (0 == ref && m_depOnAll)
    {
        m_depOnAll->GetEvent()->RemoveFloatingDependence();
        m_depOnAll = nullptr;
    }
    return ref;
}

void OutOfOrderCommandQueue::Submit(Command* cmd)
{
    if ( m_bProfilingEnabled )
    {
        cmd->GetEvent()->SetProfilingInfo(CL_PROFILING_COMMAND_SUBMIT,
            m_pDefaultDevice->GetDeviceAgent()->clDevGetPerformanceCounter());
    }
    cmd->SetDevCmdListId(m_clDevCmdListId);
    cmd->GetEvent()->SetEventState(EVENT_STATE_ISSUED_TO_DEVICE);
    cl_err_code res = (m_bCancelAll) ? cmd->Cancel() : cmd->Execute();
    if (CL_SUCCEEDED(res))
    {
        if ( RUNTIME_EXECUTION_TYPE != cmd->GetExecutionType() )
        {
            ++m_unflushedCommands;
            // Must flush here or risk deadlock if the tbb task is exiting and this was the last operation done in user code
            Flush(false);
        }
    }
    else // Not succeeded, check real value
    {
        if (res == CL_DONE_ON_RUNTIME )
        {
            cmd->GetEvent()->SetEventState(EVENT_STATE_DONE);
            cmd->CommandDone();
        }
        else if(res != CL_NOT_READY)
        {
            NotifyCommandFailed(res,cmd);
        }
    }
}

cl_err_code OutOfOrderCommandQueue::Enqueue(Command* cmd)
{
    OclAutoMutex mu(&m_muLastBarrer);
    
    SharedPtr<OclEvent> cmdEvent = cmd->GetEvent();
    m_depOnAll->GetEvent()->AddDependentOn(cmdEvent);
    if (0 != m_lastBarrier)
    {
        cmdEvent->AddDependentOn( m_lastBarrier );
    }
    return CL_SUCCESS;
}

cl_err_code OutOfOrderCommandQueue::EnqueueMarkerWaitForEvents(Command* marker)
{
    SharedPtr<OclEvent> cmdEvent = marker->GetEvent();
    if (!marker->IsDependentOnEvents())
    {
        // Prevent marker from firing until we're done enqueuing it to avoid races
        cmdEvent->AddFloatingDependence();
        cmdEvent->SetEventState(EVENT_STATE_HAS_DEPENDENCIES);
        const cl_err_code ret = AddDependentOnAll(marker);
        cmdEvent->RemoveFloatingDependence();
        return ret;
    }
    m_depOnAll->GetEvent()->AddDependentOn(cmdEvent);
    return CL_SUCCESS;
}

cl_err_code OutOfOrderCommandQueue::EnqueueBarrierWaitForEvents(Command* barrier)
{
    if (!barrier->IsDependentOnEvents())
    {
        OclAutoMutex mu(&m_muLastBarrer);
        m_lastBarrier = barrier->GetEvent();
        const cl_err_code ret = AddDependentOnAll(barrier);
        return ret;
    }
    return EnqueueWaitForEvents(barrier);
}

cl_err_code OutOfOrderCommandQueue::EnqueueWaitForEvents(Command* cmd)
{        
    OclAutoMutex mu(&m_muLastBarrer);

    SharedPtr<OclEvent> cmdEvent = cmd->GetEvent();
    m_depOnAll->GetEvent()->AddDependentOn(cmdEvent);
    if (0 != m_lastBarrier)
    {
        cmdEvent->AddDependentOn( m_lastBarrier );
    }
    m_lastBarrier = cmdEvent;
    return CL_SUCCESS;    
}

cl_err_code OutOfOrderCommandQueue::Flush(bool bBlocking)
{    
    long prev = m_unflushedCommands.exchange(0);
    if (prev > 0)
    {
        m_pDefaultDevice->GetDeviceAgent()->clDevFlushCommandList(m_clDevCmdListId);
    }

    return CL_SUCCESS;    
}

cl_err_code OutOfOrderCommandQueue::NotifyStateChange( const SharedPtr<QueueEvent>& pEvent, OclEventState prevColor, OclEventState newColor )
{    
    if ((EVENT_STATE_READY_TO_EXECUTE == newColor) || m_bCancelAll)
    {
        Command* cmd = pEvent->GetCommand();
        if ( cmd->isControlCommand() )
        {
            bool isMarker = (CL_COMMAND_MARKER == cmd->GetCommandType());
            // Control command is Ready
            pEvent->SetEventState(EVENT_STATE_ISSUED_TO_DEVICE);
            cmd->Execute(); // CommandDone() and color change are applied inside Execute() call.

            if ( !isMarker )
            {                            
                OclAutoMutex mu(&m_muLastBarrer);
                if (pEvent == m_lastBarrier)
                {
                    m_lastBarrier = nullptr;
                }
            }
        }
        else
        {
            Submit(cmd);
        }
    }
    return CL_SUCCESS;
}



cl_err_code OutOfOrderCommandQueue::AddDependentOnAll(Command* cmd)
{
    assert(nullptr != cmd);

    Command* pNewDepOnAll = new MarkerCommand(this, 0);
    if (nullptr == pNewDepOnAll)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    SharedPtr<QueueEvent> pNewDepnOnAllEvent = pNewDepOnAll->GetEvent();
    SharedPtr<QueueEvent> pCommandEvent      = cmd->GetEvent();

    pNewDepnOnAllEvent->AddFloatingDependence();
    pNewDepnOnAllEvent->Release();
    
    // First of all create a new "depends on all" object and put it in place.
    // Then link dependencies: new "dep on all" depends on command (marker/barrier) depends on old "dep on all"
    // Finally remove the floating dependence to allow the thing to resolve in due course.

    Command* pOldDepOnAll = (Command*)m_depOnAll.exchange(pNewDepOnAll);
    SharedPtr<QueueEvent> pOldDepOnAllEvent = pOldDepOnAll->GetEvent();
    pCommandEvent->AddDependentOn(pOldDepOnAllEvent);
    pOldDepOnAllEvent->RemoveFloatingDependence();
    return CL_SUCCESS;
}

