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
//  ocl_event.cpp
//  Implementation of the Class OclEvent
//  Created on:      23-Dec-2008 3:23:05 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include "ocl_event.h"
#include "command_queue.h"
#include "enqueue_commands.h"
#include <assert.h>
#include <cl_sys_info.h>

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

/******************************************************************
 *
 ******************************************************************/
OclEvent::OclEvent(IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints) :
	QueueEvent(cmdQueue), m_bProfilingEnabled(false)
{
	m_sProfilingInfo.m_ulCommandQueued = 0;
	m_sProfilingInfo.m_ulCommandSubmit = 0;
	m_sProfilingInfo.m_ulCommandStart = 0;
	m_sProfilingInfo.m_ulCommandEnd = 0;

	if (cmdQueue)
	{
		m_bProfilingEnabled = cmdQueue->IsProfilingEnabled() ? true : false;
	}
	//Todo: workaround because OCL1.0
	m_bProfilingEnabled = true;

	m_handle.object = this;
	m_handle.dispatch = pOclEntryPoints;
}

/******************************************************************
 *
 ******************************************************************/
OclEvent::~OclEvent()
{
}

/******************************************************************
 * This function is a synch point.
 * The host thread is waiting until the command associated with this event
 * is done
 ******************************************************************/
void OclEvent::Wait()
{
#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_SPIN
	WaitSpin();
#elif OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_YIELD
	WaitYield();
#elif OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
	WaitOSEvent();
#else
#error "Please define which wait method OclEvent should use. See ocl_event.h"
#endif
}

void OclEvent::WaitSpin()
{
	while (m_color != EVENT_STATE_BLACK)
	{
	}
}
void OclEvent::WaitYield()
{
	while (m_color != EVENT_STATE_BLACK)
	{
		clSleep(0);
	}
}
void OclEvent::WaitOSEvent()
{
#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
	//This is a heavy routine, if I can early exit, all the better
	if (EVENT_STATE_BLACK == m_color) return;

	//Creating a manual reset event to prevent a race condition between event completion and waiting on OS event
	if (m_osEvent.Init())
	{
		// Adding myself as a listener to my own completion. 
		// My notification routine (called from the informing thread) will wake up the waiting thread
		AddCompleteListener(this);
		m_osEvent.Wait();
		//Disallow inconsistent results. Let the event completion routine (from another thread) finish before returning.
		WaitSpin(); 
	}
	else
	{
		WaitYield();
	}
#else
	WaitYield();
#endif
}

#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
cl_err_code OclEvent::NotifyEventDone(QueueEvent* pEvent)
{
	//Only care about notifications from myself
	if (dynamic_cast<QueueEvent *>(this) != pEvent)
	{
		return QueueEvent::NotifyEventDone(pEvent);
	}
	//We only listen on ourselves after creating a legal OS event. Make sure nothing freaky happened.
	m_osEvent.Signal();
	//And from here continue as usual
	return QueueEvent::NotifyEventDone(pEvent);
}
#endif

/******************************************************************
 *
 ******************************************************************/
cl_err_code OclEvent::GetInfo(cl_int paramName, size_t paramValueSize, void * paramValue, size_t * paramValueSizeRet)
{
    cl_err_code res = CL_SUCCESS;
    void* localParamValue = NULL;
    size_t outputValueSize = 0;
    cl_int eventStatus = CL_QUEUED;
	cl_command_type cmd_type;
	cl_command_queue cmd_queue;

    switch (paramName)
    {
        case CL_EVENT_COMMAND_QUEUE:
			cmd_queue = GetEventQueue()->GetHandle();
            localParamValue = &cmd_queue;
            outputValueSize = sizeof(cl_command_queue);
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
        case CL_EVENT_REFERENCE_COUNT:
            localParamValue = &m_uiRefCount;
            outputValueSize = sizeof(cl_uint);
            break;
        case CL_EVENT_COMMAND_EXECUTION_STATUS:
            eventStatus = GetEventCurrentStatus();
            localParamValue = &eventStatus;
            outputValueSize = sizeof(cl_int); 
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
cl_err_code OclEvent::GetProfilingInfo(cl_profiling_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
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

void OclEvent::SetProfilingInfo(cl_profiling_info clParamName, cl_ulong ulData)
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
 * This function returns the current status of the command that is
 * corresponding to this event.
 * The status is translated from the m_queueEvent color to OCL API
 * status
 ******************************************************************/
cl_int OclEvent::GetEventCurrentStatus()
{
    switch(m_color)
    {
	case EVENT_STATE_WHITE:
		return CL_QUEUED;
    case EVENT_STATE_RED:
        // Fall through
    case EVENT_STATE_YELLOW:
		// Fall through
    case EVENT_STATE_LIME:
        return CL_SUBMITTED;
    case EVENT_STATE_GREEN:
        return CL_RUNNING;
    case EVENT_STATE_BLACK:
    default:
        return CL_COMPLETE;
    }        
}

cl_context OclEvent::GetContextHandle() const
{
	return GetEventQueue()->GetContextHandle();
}
cl_command_queue OclEvent::GetQueueHandle() const
{
	return GetEventQueue()->GetHandle();
}
