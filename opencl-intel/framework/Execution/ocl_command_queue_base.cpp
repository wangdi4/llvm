// Copyright (c) 2008-2009 Intel Corporation
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

#include <cl_types.h>
#include "ocl_event.h"
#include "command_queue.h"
#include "events_manager.h"
#include "enqueue_commands.h"

using namespace Intel::OpenCL::Framework;

cl_err_code IOclCommandQueueBase::EnqueueCommand(Command* pCommand, cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pUserEvent)
{
	cl_err_code errVal = CL_SUCCESS;
	// If blocking and no event, than it is needed to create dummy cl_event for wait
	cl_event waitEvent = NULL;
	cl_event* pEvent;
	if( bBlocking && NULL == pUserEvent)
	{
		pEvent = &waitEvent;
	}
	else
	{
		pEvent = pUserEvent;
	}
	// creates the command's event
	OclEvent* pQueueEvent = m_pEventsManager->CreateOclEvent(pCommand->GetCommandType(), pEvent, this, (ocl_entry_points*)m_handle.dispatch);
	pCommand->SetEvent(pQueueEvent);

	pQueueEvent->AddFloatingDependence();
	errVal = SetDependentOnList(pCommand, uNumEventsInWaitList, cpEeventWaitList);

	if( CL_FAILED(errVal))
	{
		pQueueEvent->RemoveFloatingDependence();
		pCommand->CommandDone();
		delete pQueueEvent;
		return errVal;
	}   

	errVal = Enqueue(pCommand);
	pQueueEvent->RemoveFloatingDependence();
	if (CL_FAILED(errVal))
	{
		pCommand->CommandDone();
		delete pQueueEvent;
		return CL_ERR_FAILURE;
	}

	if (m_bProfilingEnabled)
	{
		pQueueEvent->SetProfilingInfo(CL_PROFILING_COMMAND_QUEUED, m_pDefaultDevice->GetDeviceAgent()->clDevGetPerformanceCounter());
	}

	// If blocking, wait for object
	if(bBlocking)
	{
		if ( !WaitForCompletion(pQueueEvent) )
		{
			pQueueEvent->Wait();
		}
		if ( pUserEvent == NULL )
		{
			// The case where it use temp event for blocking
			m_pEventsManager->ReleaseEvent(waitEvent);
		}
	}
	return CL_SUCCESS;
}

cl_err_code IOclCommandQueueBase::EnqueueWaitEvents(Command* wfe, cl_uint uNumEventsInWaitList, const cl_event* cpEventWaitList)
{
	cl_err_code errVal;
	//create a dummy event for the waitForEvents
	cl_event* pEvent = NULL;
	OclEvent* pQueueEvent = m_pEventsManager->CreateOclEvent(wfe->GetCommandType(), pEvent, this, (ocl_entry_points*)m_handle.dispatch);
	wfe->SetEvent(pQueueEvent);
	pQueueEvent->AddFloatingDependence();
	//wfe->SetCommandDeviceId(m_clDefaultDeviceId);
	errVal = SetDependentOnList(wfe, uNumEventsInWaitList, cpEventWaitList);

	if( CL_FAILED(errVal))
	{
		pQueueEvent->RemoveFloatingDependence();
		delete pQueueEvent;
		return errVal;
	}   

	errVal = EnqueueWaitForEvents(wfe);
	pQueueEvent->RemoveFloatingDependence();
	return errVal;
}

bool IOclCommandQueueBase::WaitForCompletion(OclEvent* pEvent)
{
	// Make blocking flush to ensure everything ends in the device's command list before we join its execution
	Flush(true);

	cl_int ret = m_pDefaultDevice->GetDeviceAgent()->clDevCommandListWaitCompletion(m_clDevCmdListId);

	QueueEventStateColor color = pEvent->GetColor();
	if ( CL_DEV_FAILED(ret) || (EVENT_STATE_BLACK != color) )
	{
		pEvent->Wait();
	}

	return true;
}

cl_err_code IOclCommandQueueBase::SetDependentOnList(Command* cmd, cl_uint uNumEventsInWaitList, const cl_event* cpEventWaitList)
{
	return m_pEventsManager->RegisterEvents(cmd->GetEvent(), uNumEventsInWaitList, cpEventWaitList);
}