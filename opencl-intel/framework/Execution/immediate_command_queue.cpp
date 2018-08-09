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

#include "immediate_command_queue.h"
#include "enqueue_commands.h"
#include "ocl_event.h"
#include "Device.h"
#include "events_manager.h"
#include "ocl_command_queue.h"
#include "Context.h"
#include <assert.h>

using namespace Intel::OpenCL::Framework;
ImmediateCommandQueue::ImmediateCommandQueue(
	SharedPtr<Context>                    pContext,
	cl_device_id                clDefaultDeviceID, 
	cl_command_queue_properties clProperties,
	EventsManager*              pEventManager
	) :
	IOclCommandQueueBase(pContext, clDefaultDeviceID, clProperties, pEventManager)
{
}
ImmediateCommandQueue::~ImmediateCommandQueue() 
{
}

cl_err_code ImmediateCommandQueue::Initialize()
{
    cl_err_code ret = CL_SUCCESS;
    ret = m_pDefaultDevice->GetDeviceAgent()->clDevCreateCommandList(CL_DEV_LIST_IN_PLACE, 0, &m_clDevCmdListId);
    return ret;
}

cl_err_code ImmediateCommandQueue::EnqueueCommand(Command* pCommand, cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger)
{
    cl_err_code ret         = CL_SUCCESS;
    SharedPtr<QueueEvent> pQueueEvent = pCommand->GetEvent();
    if (m_bProfilingEnabled)
    {
        pQueueEvent->SetProfilingInfo(CL_PROFILING_COMMAND_QUEUED, m_pDefaultDevice->GetDeviceAgent()->clDevGetPerformanceCounter());
    }
    if (!m_bOutOfOrderEnabled)
    {
        m_CS.Lock();
    }
    if (uNumEventsInWaitList > 0)
    {
        ret = m_pEventsManager->WaitForEvents(uNumEventsInWaitList, cpEeventWaitList);
    }
    if (CL_SUCCESS != ret)
    {
        if (!m_bOutOfOrderEnabled)
        {
            m_CS.Unlock();
        }
        return ret;
    }
    if (nullptr != pEvent)
    {
        pQueueEvent->SetVisibleToUser();
        m_pEventsManager->RegisterQueueEvent(pQueueEvent, pEvent);
    }

    if (apiLogger != nullptr)
    {
        apiLogger->SetCmdId(pQueueEvent->GetId());
    }

    ret = Enqueue(pCommand);

    if (!m_bOutOfOrderEnabled)
    {
        m_CS.Unlock();
    }

    if (nullptr == pEvent)
    {
        pQueueEvent->Release();
    }

    if (CL_SUCCESS != ret)
    {
        NotifyCommandFailed(ret,pCommand);
    }
    return ret;
}

cl_err_code ImmediateCommandQueue::Enqueue(Command* cmd)
{
    SharedPtr<QueueEvent> pQueueEvent = cmd->GetEvent();
    if ( m_bProfilingEnabled )
    {
        pQueueEvent->SetProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, m_pDefaultDevice->GetDeviceAgent()->clDevGetPerformanceCounter());
    }				
    pQueueEvent->SetEventState(EVENT_STATE_READY_TO_EXECUTE);
    cmd->SetDevCmdListId(m_clDevCmdListId);
    return (m_bCancelAll) ? cmd->Cancel() : cmd->Execute();
}

/**
 * @fn cl_err_code ImmediateCommandQueue::EnqueueMarkerWaitForEvents(Command* marker)
 */
cl_err_code ImmediateCommandQueue::EnqueueMarkerWaitForEvents(Command* marker)
{
    if (marker->IsDependentOnEvents())
    {
        Enqueue(marker);
    }
    else
    {
        cl_err_code ret; 
        if (m_bProfilingEnabled)
        {
            marker->GetEvent()->SetProfilingInfo(CL_PROFILING_COMMAND_QUEUED, m_pDefaultDevice->GetDeviceAgent()->clDevGetPerformanceCounter());
        }
        if (!m_bOutOfOrderEnabled)
        {
            //Must block while commands are executed
            m_CS.Lock();
            ret = Enqueue(marker);
            m_CS.Unlock();
        }
        else
        {
            ret = Enqueue(marker);
        }
        return ret;
    }
    return CL_SUCCESS;
}

/**
 * @fn cl_err_code ImmediateCommandQueue::EnqueueBarrierWaitForEvents(Command* barrier)
 */
cl_err_code ImmediateCommandQueue::EnqueueBarrierWaitForEvents(Command* barrier)
{
    return EnqueueMarkerWaitForEvents(barrier);
}

cl_err_code ImmediateCommandQueue::Flush(bool bBlocking)
{
	return CL_SUCCESS;
}

cl_err_code ImmediateCommandQueue::NotifyStateChange( const SharedPtr<QueueEvent>& pEvent, OclEventState prevColor, OclEventState newColor )
{
	return CL_SUCCESS;
}

cl_err_code ImmediateCommandQueue::SendCommandsToDevice()
{
    return CL_SUCCESS;
}
/*
bool ImmediateCommandQueue::WaitForCompletion(SharedPtr<OclEvent> pEvent)
{

}
*/
