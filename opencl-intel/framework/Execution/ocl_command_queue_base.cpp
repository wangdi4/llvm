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

#include <cassert>
#include <cl_types.h>
#include "ocl_event.h"
#include "command_queue.h"
#include "events_manager.h"
#include "enqueue_commands.h"
#include "execution_module.h"
#include <cstring>
#include "ocl_itt.h"

using namespace Intel::OpenCL::Framework;

cl_err_code IOclCommandQueueBase::EnqueueCommand(Command* pCommand, cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pUserEvent)
{
#if defined(USE_GPA)
	ocl_gpa_data* pGPAData = m_pContext->GetGPAData();

	if ((NULL != pGPAData) && (pGPAData->bUseGPA))
	{
		if (pGPAData->cStatusMarkerFlags & GPA_SHOW_QUEUED_MARKER)
		{
			// Write this data to the thread track
			__itt_set_track(NULL);

			char pMarkerString[64] = "Queued - ";
			const char* pCommandName = pCommand->GetCommandName();
			strcat_s(pMarkerString, 64,pCommandName);

			__itt_string_handle* pMarker = __itt_string_handle_createA(pMarkerString);

			//Due to a bug in GPA 4.0 the marker is within a task
			//Should be removed in GPA 4.1  
			__itt_task_begin(pGPAData->pDeviceDomain, __itt_null, __itt_null, pMarker);

			__itt_marker(pGPAData->pDeviceDomain, __itt_null, pMarker, __itt_marker_scope_global);
			
			__itt_task_end(pGPAData->pDeviceDomain);		
		}

		if ((pGPAData->bEnableContextTracing) && (NULL != pCommand->GPA_GetCommand()))
		{
			// Set custom track 
			__itt_set_track(pCommand->GetCommandQueue()->GPA_GetQueue()->m_pTrack);

			// Create id for the new task
			pCommand->GPA_GetCommand()->m_CmdId = __itt_id_make(0, (unsigned long long)pCommand);
			__itt_id_create(pGPAData->pContextDomain, pCommand->GPA_GetCommand()->m_CmdId);
        
			// Begin waiting task
			__itt_task_begin_overlapped(pGPAData->pContextDomain, pCommand->GPA_GetCommand()->m_CmdId, __itt_null, pCommand->GPA_GetCommand()->m_strCmdName);
			__ittx_task_set_state(pGPAData->pContextDomain, pCommand->GPA_GetCommand()->m_CmdId, pGPAData->pWaitingTaskState);
		}
	}
#endif
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
	QueueEvent* pQueueEvent = pCommand->GetEvent();
	m_pEventsManager->RegisterQueueEvent(pQueueEvent, pEvent);

	pQueueEvent->AddFloatingDependence();
	errVal = SetDependentOnList(pCommand, uNumEventsInWaitList, cpEeventWaitList);

	if( CL_FAILED(errVal))
	{
		pQueueEvent->RemoveFloatingDependence();
		pCommand->CommandDone();
		pQueueEvent->RemovePendency(NULL); //implicitly added by Command->SetEvent
		if (NULL == pUserEvent)
		{
			m_pEventsManager->ReleaseEvent(pQueueEvent->GetHandle());
		}
		return errVal;
	}   
	pQueueEvent->RemoveFloatingDependence();
    if (m_bProfilingEnabled)
    {
        pQueueEvent->SetProfilingInfo(CL_PROFILING_COMMAND_QUEUED, m_pDefaultDevice->GetDeviceAgent()->clDevGetPerformanceCounter());
    }

    errVal = Enqueue(pCommand);
	
	if (CL_FAILED(errVal))
	{
		pCommand->CommandDone();
		pQueueEvent->RemovePendency(NULL); //implicitly added by Command->SetEvent
		if (NULL == pUserEvent)
		{
			m_pEventsManager->ReleaseEvent(pQueueEvent->GetHandle());
		}
		return CL_ERR_FAILURE;
	}

	// If blocking, wait for object
	if(bBlocking)
	{
		if ( !WaitForCompletion(pQueueEvent) )
		{
			pQueueEvent->Wait();
		}
		//If the event is not visible to the user, remove its floating reference count and as a result the pendency representing the object is visible to the user
		if (NULL == pUserEvent) 
		{
			m_pEventsManager->ReleaseEvent(pQueueEvent->GetHandle());
		}
	}
	else
	{
		//If the event is not visible to the user, remove its floating reference count and as a result the pendency representing the object is visible to the user
		if (NULL == pUserEvent) 
		{
			m_pEventsManager->ReleaseEvent(pQueueEvent->GetHandle());
		}
	}
	return CL_SUCCESS;
}

cl_err_code IOclCommandQueueBase::EnqueueWaitEvents(Command* wfe, cl_uint uNumEventsInWaitList, const cl_event* cpEventWaitList)
{
	cl_err_code errVal;
	//create a dummy event for the waitForEvents
	cl_event* pEvent = NULL;
	QueueEvent* pQueueEvent = wfe->GetEvent();
	m_pEventsManager->RegisterQueueEvent(pQueueEvent, pEvent);
	//if(NULL == pQueueEvent) not for this commit
	//{
	//	return CL_OUT_OF_HOST_MEMORY;
	//}
	pQueueEvent->AddFloatingDependence();
	//wfe->SetCommandDeviceId(m_clDefaultDeviceId);
	errVal = SetDependentOnList(wfe, uNumEventsInWaitList, cpEventWaitList);

	if( CL_FAILED(errVal))
	{
		pQueueEvent->RemoveFloatingDependence();
		m_pEventsManager->ReleaseEvent(pQueueEvent->GetHandle());
		pQueueEvent->RemovePendency(NULL); //Added by Command->SetEvent()
		return errVal;
	}

	errVal = EnqueueWaitForEvents(wfe);
	pQueueEvent->RemoveFloatingDependence();
	m_pEventsManager->ReleaseEvent(pQueueEvent->GetHandle());
	return errVal;
}

bool IOclCommandQueueBase::WaitForCompletion(OclEvent* pEvent)
{
	pEvent->AddPendency(this);
	// Make blocking flush to ensure everything ends in the device's command list before we join its execution
	Flush(true);

	cl_int ret = m_pDefaultDevice->GetDeviceAgent()->clDevCommandListWaitCompletion(m_clDevCmdListId);

	OclEventStateColor color = pEvent->GetColor();
	if ( CL_DEV_FAILED(ret) || (EVENT_STATE_BLACK != color) )
	{
		pEvent->Wait();
	}
	pEvent->RemovePendency(this);
	return true;
}

cl_err_code IOclCommandQueueBase::SetDependentOnList(Command* cmd, cl_uint uNumEventsInWaitList, const cl_event* cpEventWaitList)
{
	return m_pEventsManager->RegisterEvents(cmd->GetEvent(), uNumEventsInWaitList, cpEventWaitList);
}
