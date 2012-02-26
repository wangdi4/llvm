// Copyright (c) 2008-2012 Intel Corporation
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
//  in_order_command_queue.cpp
//  Implementation of the Class InOrderQueue
//  Created on:      23-Dec-2008 3:23:02 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
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
	Context*                    pContext,
	cl_device_id                clDefaultDeviceID, 
	cl_command_queue_properties clProperties,
	EventsManager*              pEventManager,
	ocl_entry_points *			pOclEntryPoints
	) :
	IOclCommandQueueBase(pContext, clDefaultDeviceID, clProperties, pEventManager, pOclEntryPoints)
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

cl_err_code ImmediateCommandQueue::EnqueueCommand(Command* pCommand, cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent)
{
    cl_err_code ret         = CL_SUCCESS;
    QueueEvent* pQueueEvent = pCommand->GetEvent();
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
    if (NULL != pEvent)
    {
        m_pEventsManager->RegisterQueueEvent(pQueueEvent, pEvent);
    }

    ret = Enqueue(pCommand);

    if (!m_bOutOfOrderEnabled)
    {
        m_CS.Unlock();
    }

    if (NULL == pEvent)
    {
        pQueueEvent->Release();
    }
    return ret;
}

cl_err_code ImmediateCommandQueue::Enqueue(Command* cmd)
{
    QueueEvent* pQueueEvent = cmd->GetEvent();
    if ( m_bProfilingEnabled )
    {
        pQueueEvent->SetProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, m_pDefaultDevice->GetDeviceAgent()->clDevGetPerformanceCounter());
    }				
    pQueueEvent->SetEventState(EVENT_STATE_READY_TO_EXECUTE);
    cmd->SetDevCmdListId(m_clDevCmdListId);
    return cmd->Execute();
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

cl_err_code ImmediateCommandQueue::NotifyStateChange( QueueEvent* pEvent, OclEventState prevColor, OclEventState newColor )
{
	return CL_SUCCESS;
}

cl_err_code ImmediateCommandQueue::SendCommandsToDevice()
{
    return CL_SUCCESS;
}
/*
bool ImmediateCommandQueue::WaitForCompletion(OclEvent* pEvent)
{

}
*/
