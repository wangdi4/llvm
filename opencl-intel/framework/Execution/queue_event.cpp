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

///////////////////////////////////////////////////////////
//  queue_event.cpp
//  Implementation of the Class QueueEvent
//  Created on:      23-Dec-2008 3:23:06 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include <cassert>
#include "queue_event.h"
#include "command_queue.h"
#include "enqueue_commands.h"
#include <cl_sys_info.h>

// For debugging
#include <assert.h>
#include "cl_utils.h"

#include "ocl_itt.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

/******************************************************************
*
******************************************************************/
QueueEvent::QueueEvent(IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints) :
OclEvent(), m_bProfilingEnabled(false), m_pCommand(NULL), m_pEventQueue(cmdQueue)
{
	m_sProfilingInfo.m_ulCommandQueued = 0;
	m_sProfilingInfo.m_ulCommandSubmit = 0;
	m_sProfilingInfo.m_ulCommandStart = 0;
	m_sProfilingInfo.m_ulCommandEnd = 0;

	if (cmdQueue != NULL)
	{
		m_bProfilingEnabled = cmdQueue->IsProfilingEnabled() ? true : false;
	}

	m_handle.object = this;
	m_handle.dispatch = (KHRicdVendorDispatch*)pOclEntryPoints;

	m_pGPAData = cmdQueue->GetGPAData();
}

/******************************************************************
*
******************************************************************/
QueueEvent::~QueueEvent()
{
}

/******************************************************************
*
******************************************************************/
cl_err_code QueueEvent::GetInfo(cl_int paramName, size_t paramValueSize, void * paramValue, size_t * paramValueSizeRet)
{
	cl_err_code res = CL_SUCCESS;
	void* localParamValue = NULL;
	size_t outputValueSize = 0;
	cl_int eventStatus = CL_QUEUED;
	cl_command_type  cmd_type;
	cl_command_queue cmd_queue;
	cl_context       context;

	switch (paramName)
	{
	case CL_EVENT_COMMAND_QUEUE:
		cmd_queue = GetEventQueue()->GetHandle();
		localParamValue = &cmd_queue;
		outputValueSize = sizeof(cl_command_queue);
		break;
	case CL_EVENT_CONTEXT:
		context = GetContextHandle();
		localParamValue = &context;
		outputValueSize = sizeof(cl_context);
		break;
	case CL_EVENT_COMMAND_TYPE:
		if (!m_pCommand)
		{
			//Todo: need to return CL_USER_COMMAND if user event
			return CL_INVALID_VALUE;
		}
		cmd_type        = m_pCommand->GetCommandType();
		localParamValue = &cmd_type;
		outputValueSize = sizeof(cl_command_type);
		break;
	case CL_EVENT_COMMAND_EXECUTION_STATUS:
		eventStatus = GetEventCurrentStatus();
		localParamValue = &eventStatus;
		outputValueSize = sizeof(cl_int); 
		break;
	case CL_EVENT_REFERENCE_COUNT:
		localParamValue = &m_uiRefCount;
		outputValueSize = sizeof(cl_uint);
		break;
	default:
		res = CL_INVALID_VALUE;
		break;
	}

	// check param_value_size
	if ( (NULL != paramValue) && (paramValueSize < outputValueSize))
	{
		res = CL_INVALID_VALUE;
	}
	else
	{
		if ( NULL != paramValue )
		{
			memcpy(paramValue, localParamValue, outputValueSize);
		}
		if ( NULL != paramValueSizeRet )
		{
			*paramValueSizeRet = outputValueSize;
		}
	}
	return res;
}

/******************************************************************
*
******************************************************************/
cl_err_code QueueEvent::GetProfilingInfo(cl_profiling_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
	cl_err_code res = CL_SUCCESS;
	void* localParamValue = NULL;
	size_t outputValueSize = 0;
	cl_ulong ulProfilingInfo = 0;

	if (!m_bProfilingEnabled)
	{
		return CL_PROFILING_INFO_NOT_AVAILABLE;
	}

	cl_int eventStatus = GetEventCurrentStatus();

	switch (clParamName)
	{
	case CL_PROFILING_COMMAND_QUEUED:
		ulProfilingInfo = m_sProfilingInfo.m_ulCommandQueued;
		break;
	case CL_PROFILING_COMMAND_SUBMIT:
		if (eventStatus > CL_SUBMITTED)
		{
			return CL_PROFILING_INFO_NOT_AVAILABLE;
		}
		ulProfilingInfo = m_sProfilingInfo.m_ulCommandSubmit;
		break;
	case CL_PROFILING_COMMAND_START:
		if (eventStatus > CL_RUNNING)
		{
			return CL_PROFILING_INFO_NOT_AVAILABLE;
		}
		ulProfilingInfo = m_sProfilingInfo.m_ulCommandStart;
		break;
	case CL_PROFILING_COMMAND_END:
		if (eventStatus > CL_COMPLETE)
		{
			return CL_PROFILING_INFO_NOT_AVAILABLE;
		}
		ulProfilingInfo = m_sProfilingInfo.m_ulCommandEnd;
		break;
	default:
		return CL_INVALID_VALUE;
	}

	localParamValue = &ulProfilingInfo;
	outputValueSize = sizeof(cl_ulong);

	// check param_value_size
	if ( (NULL != pParamValue) && (szParamValueSize < outputValueSize))
	{
		res = CL_INVALID_VALUE;
	}
	else
	{
		if ( NULL != pParamValue )
		{
			memcpy(pParamValue, localParamValue, outputValueSize);
		}
		if ( NULL != pszParamValueSizeRet )
		{
			*pszParamValueSizeRet = outputValueSize;
		}
	}
	return res;
}

void QueueEvent::SetProfilingInfo(cl_profiling_info clParamName, cl_ulong ulData)
{
	switch ( clParamName )
	{
	case CL_PROFILING_COMMAND_QUEUED:
		m_sProfilingInfo.m_ulCommandQueued = ulData;
		break;
	case CL_PROFILING_COMMAND_SUBMIT:
		m_sProfilingInfo.m_ulCommandSubmit = ulData;
		break;
	case CL_PROFILING_COMMAND_START:
		m_sProfilingInfo.m_ulCommandStart = ulData;
		break;
	case CL_PROFILING_COMMAND_END:
		m_sProfilingInfo.m_ulCommandEnd = ulData;
		break;
	default:
		break;
	}
}

/******************************************************************
*
******************************************************************/
cl_err_code QueueEvent::NotifyEventDone(Intel::OpenCL::Framework::OclEvent *pEvent, cl_int returnCode)
{
	if (CL_SUCCESS != returnCode)
	{
		m_pCommand->NotifyCmdStatusChanged(0, CL_COMPLETE, returnCode, Intel::OpenCL::Utils::HostTime());
		//Everything else will be handled from the command routine
		return CL_SUCCESS;
	}
	//Else, fall back on regular routine
	return OclEvent::NotifyEventDone(pEvent, returnCode);
}

long QueueEvent::RemovePendency(OCLObjectBase* pObj)
{
    if (NULL != pObj)
    {
        EraseFromDependecySet(pObj);
    }
	long newVal = --m_uiPendency;
	if (0 == newVal)
	{
		//m_pCommand aggregates me, so I'm also deleted as a side-effect
		delete m_pCommand;
	}
	return newVal;
}

OclEventStateColor QueueEvent::SetColor(OclEventStateColor newColor)
{
	OclEventStateColor retval = OclEvent::SetColor(newColor);
#if defined(USE_GPA)
	if (EVENT_STATE_YELLOW == newColor)
	{	
		if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA))
		{
			if (m_pGPAData->cStatusMarkerFlags & GPA_SHOW_SUBMITTED_MARKER)
			{
				// Write this data to the thread track
				__itt_set_track(NULL);

				char pMarkerString[64] = "Submitted - ";
				const char* pCommandName = m_pCommand->GetCommandName();
				strcat_s(pMarkerString, 64,pCommandName);
				
				__itt_string_handle* pMarker = __itt_string_handle_createA(pMarkerString);
				//Due to a bug in GPA 4.0 the marker is within a task
				//Should be removed in GPA 4.1 
				__itt_task_begin(m_pGPAData->pDeviceDomain, __itt_null, __itt_null, m_pGPAData->pMarkerHandle);
				
				__itt_marker(m_pGPAData->pDeviceDomain, __itt_null, pMarker, __itt_marker_scope_global);

				__itt_task_end(m_pGPAData->pDeviceDomain);
			}
		}
	}
#endif

	return retval;
}

void QueueEvent::NotifyReady(OclEvent* pEvent)
{
    if (EVENT_STATE_RED == m_color)
    {
        m_color = EVENT_STATE_YELLOW;

		QueueEvent *pQEvent = dynamic_cast<QueueEvent*>(pEvent);
        //See if I have to notify my queue or not
        if ((NULL != pQEvent) && (pQEvent->GetEventQueue()))
        {
            if (pQEvent->GetEventQueue()->GetId() == m_pEventQueue->GetId())
            {
                //that event will notify my queue for me
                if (!m_pEventQueue->IsOutOfOrderExecModeEnabled())
                {
                    //Optimization only valid for in-order queue
                    return;
                }
            }
        }
        //else, I have to notify the queue myself
        m_pEventQueue->NotifyStateChange(this, EVENT_STATE_RED, EVENT_STATE_YELLOW);
    }
}

void QueueEvent::NotifyComplete(cl_int returnCode /* = CL_SUCCESS */)
{
    OclEvent::NotifyComplete(returnCode);
    m_pEventQueue->NotifyStateChange(this, EVENT_STATE_GREEN, EVENT_STATE_BLACK);
}

cl_context QueueEvent::GetContextHandle() const
{
	return GetEventQueue()->GetContextHandle();
}
cl_command_queue QueueEvent::GetQueueHandle() const
{
	return GetEventQueue()->GetHandle();
}
cl_int QueueEvent::GetReturnCode() const
{
	return m_pCommand->GetReturnCode();
}
