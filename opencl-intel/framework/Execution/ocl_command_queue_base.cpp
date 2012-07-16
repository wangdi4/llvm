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

using namespace Intel::OpenCL::Framework;

cl_err_code IOclCommandQueueBase::EnqueueCommand(Command* pCommand, cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pUserEvent)
{
#if defined(USE_ITT)
	ocl_gpa_data* pGPAData = m_pContext->GetGPAData();
	// unique ID to pass all tasks, and markers.
	__itt_id ittID;

	if ((NULL != pGPAData) && (pGPAData->bUseGPA))
	{
        ittID = __itt_id_make(&ittID, (unsigned long long)pCommand);
	    __itt_id_create(pGPAData->pDeviceDomain, ittID);

		if (pGPAData->cStatusMarkerFlags & GPA_SHOW_QUEUED_MARKER)
		{
			#if defined(USE_GPA)
			// Write this data to the thread track
			__itt_set_track(NULL);
			#endif

			char pMarkerString[ITT_TASK_NAME_LEN];
			SPRINTF_S(pMarkerString, ITT_TASK_NAME_LEN, "Enqueued - %s", pCommand->GetCommandName());

			__itt_string_handle* pMarker = __itt_string_handle_create(pMarkerString);

			//Due to a bug in GPA 4.0 the marker is within a task
			//Should be removed in GPA 4.1  
			__itt_task_begin(pGPAData->pDeviceDomain, ittID, __itt_null, pMarker);
			__itt_marker(pGPAData->pDeviceDomain, ittID, pMarker, __itt_marker_scope_global);
			cl_ushort isBlocking = bBlocking ? 1 : 0;
			__itt_metadata_add(m_pGPAData->pDeviceDomain, ittID, m_pGPAData->pIsBlocking, __itt_metadata_u16, 1, &isBlocking);
			__itt_metadata_add(m_pGPAData->pDeviceDomain, ittID, m_pGPAData->pNumEventsInWaitList, __itt_metadata_u32 , 1, &uNumEventsInWaitList);
			__itt_task_end(pGPAData->pDeviceDomain);		
		}

		#if defined(USE_GPA)
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
		#endif
	
        __itt_id_destroy(pGPAData->pDeviceDomain, ittID);
    }


#endif // ITT
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
	SharedPtr<QueueEvent> pQueueEvent = pCommand->GetEvent();
	m_pEventsManager->RegisterQueueEvent(pQueueEvent, pEvent);

	pQueueEvent->AddFloatingDependence();
	errVal = m_pEventsManager->RegisterEvents(pQueueEvent, uNumEventsInWaitList, cpEeventWaitList);

	if( CL_FAILED(errVal))
	{
		pQueueEvent->RemoveFloatingDependence();
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
		if (NULL == pUserEvent)
		{
			m_pEventsManager->ReleaseEvent(pQueueEvent->GetHandle());
		}
		return CL_ERR_FAILURE;
	}

	// If blocking, wait for object
	if(bBlocking)
	{
		if ( ( RUNTIME_EXECUTION_TYPE == pCommand->GetExecutionType() ) || CL_FAILED(WaitForCompletion(pQueueEvent)) )
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

/**
 * @fn cl_err_code IOclCommandQueueBase::EnqueueWaitEventsProlog(Command& cmd, cl_uint uNumEventsInWaitList, const cl_event* pEventWaitList)
 */
cl_err_code IOclCommandQueueBase::EnqueueWaitEventsProlog(Command& cmd, cl_uint uNumEventsInWaitList, const cl_event* pEventWaitList)
{    
    SharedPtr<QueueEvent> pQueueEvent = cmd.GetEvent();
    m_pEventsManager->RegisterQueueEvent(pQueueEvent, NULL);    
    
    const cl_err_code errVal = m_pEventsManager->RegisterEvents(pQueueEvent, uNumEventsInWaitList, pEventWaitList);
    if(CL_FAILED(errVal))
    {
        m_pEventsManager->ReleaseEvent(pQueueEvent->GetHandle());

        return errVal;
    }
    return CL_SUCCESS;
}

cl_err_code IOclCommandQueueBase::EnqueueWaitEvents(Command* wfe, cl_uint uNumEventsInWaitList, const cl_event* cpEventWaitList)
{
    QueueEvent& event = *wfe->GetEvent();
    event.AddFloatingDependence();
    cl_err_code errVal = EnqueueWaitEventsProlog(*wfe, uNumEventsInWaitList, cpEventWaitList);
    if (CL_FAILED(errVal))
    {
        event.RemoveFloatingDependence();
        return errVal;
    }
	errVal = EnqueueWaitForEvents(wfe);
    event.RemoveFloatingDependence();
    m_pEventsManager->ReleaseEvent(event.GetHandle());
	return errVal;
}

/**
 * @fn cl_err_code IOclCommandQueueBase::EnqueueMarkerWaitEvents(Command* cmd, cl_uint uNumEvetsInWaitList, const cl_event* pEventWaitList)
 */
cl_err_code IOclCommandQueueBase::EnqueueMarkerWaitEvents(Command* cmd, cl_uint uNumEvetsInWaitList, const cl_event* pEventWaitList)
{
    QueueEvent& event = *cmd->GetEvent();
    event.AddFloatingDependence();
    cl_err_code errVal = EnqueueWaitEventsProlog(*cmd, uNumEvetsInWaitList, pEventWaitList);
    if (CL_FAILED(errVal))
    {
        event.RemoveFloatingDependence();
        return errVal;
    }
    errVal = EnqueueMarkerWaitForEvents(cmd);
    event.RemoveFloatingDependence();
    return errVal;
}

/**
 * @fn cl_err_code IOclCommandQueueBase::EnqueueBarrierWaitEvents(Command* cmd, cl_uint uNumEventsInWaitList, const cl_event* pEventWaitList)
 */
cl_err_code IOclCommandQueueBase::EnqueueBarrierWaitEvents(Command* cmd, cl_uint uNumEventsInWaitList, const cl_event* pEventWaitList)
{
    QueueEvent& event = *cmd->GetEvent();
    event.AddFloatingDependence();
    cl_err_code errVal = EnqueueWaitEventsProlog(*cmd, uNumEventsInWaitList, pEventWaitList);
    if (CL_FAILED(errVal))
    {
        event.RemoveFloatingDependence();
        return errVal;
    }
    errVal = EnqueueBarrierWaitForEvents(cmd);
    event.RemoveFloatingDependence();
    return errVal;
}

cl_err_code IOclCommandQueueBase::WaitForCompletion(SharedPtr<QueueEvent> pEvent)
{
	// Make blocking flush to ensure everything ends in the device's command list before we join its execution
	Flush(true);

	cl_dev_cmd_desc* pCmdDesc = pEvent->GetCommand()->GetDeviceCommandDescriptor();

	cl_dev_err_code ret = m_pDefaultDevice->GetDeviceAgent()->clDevCommandListWaitCompletion(
		m_clDevCmdListId, pCmdDesc);

	OclEventState color = pEvent->GetEventState();
	
	while ( CL_DEV_SUCCEEDED(ret) && (EVENT_STATE_DONE != color) )
	{
		clSleep(0);
		ret = m_pDefaultDevice->GetDeviceAgent()->clDevCommandListWaitCompletion(
				m_clDevCmdListId, pCmdDesc);
		color = pEvent->GetEventState();
	}


	return CL_DEV_SUCCEEDED(ret) ? CL_SUCCESS : CL_INVALID_OPERATION;
}
