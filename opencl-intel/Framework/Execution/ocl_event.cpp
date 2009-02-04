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
#include "queue_event.h"
#include <assert.h>

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

/******************************************************************
 *
 ******************************************************************/
OclEvent::OclEvent(QueueEvent* queueEvent, cl_command_type commandType, cl_command_queue queueId ):
    m_commandType(commandType),
    m_queueID(queueId),
    m_queueEvent(queueEvent)
{    
}

/******************************************************************
 *
 ******************************************************************/
OclEvent::~OclEvent()
{
    // Note OCL event is deleted only when is refCount is set to 0.
    // That means that none of the threads is still waiting on the condition
    assert ( 0 == m_uiRefCount ); 
    if( NULL != m_queueEvent)
    {
        m_queueEvent->UnRegisterEventDoneObserver(this);
        m_queueEvent = NULL;
    }
    // Anyhow, release all waiting... 
    m_eventDoneCond.Broadcast();    
}

/******************************************************************
 * This function is a synch point.
 * The host thread is waiting until the command associated with this event
 * is done
 ******************************************************************/
void OclEvent::Wait()
{
    COND_RESULT res = COND_RESULT_COND_BROADCASTED;
    m_eventLocker.Lock();
    // Use loop to avoid unanticipated releases.
    // Use lock to avoid waiting on m_queueEvent which was already set
    while ( NULL != m_queueEvent ||  COND_RESULT_COND_BROADCASTED != res )
    {
        res = m_eventDoneCond.Wait(&m_eventLocker);
    }
    m_eventLocker.Unlock();
}

/******************************************************************
 * This function is been called when the command has completed.
 ******************************************************************/
cl_err_code OclEvent::NotifyEventDone(QueueEvent* event)
{
    {OclAutoMutex CS(&m_eventLocker); 
    m_queueEvent = NULL;
    }
	COND_RESULT res = m_eventDoneCond.Broadcast();    
    if ( COND_RESULT_OK != res)
        return CL_ERR_EXECUTION_FAILED;
    else
        return CL_SUCCESS;
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code	OclEvent::GetInfo(cl_int paramName, size_t paramValueSize, void * paramValue, size_t * paramValueSizeRet)
{
    if ((NULL == paramValue) ||  (NULL == paramValueSizeRet ))
        return CL_INVALID_VALUE;

    cl_err_code res = CL_SUCCESS;
    void* localParamValue = NULL;
    size_t outputValueSize = 0;
    cl_int eventStatus = CL_QUEUED;

    switch (paramName)
    {
        case CL_EVENT_COMMAND_QUEUE:
            localParamValue = &m_queueID;
            outputValueSize = sizeof(cl_command_queue);
            break;
        case CL_EVENT_COMMAND_TYPE:
            localParamValue = &m_commandType;
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
	if (paramValueSize < outputValueSize)
	{
		res = CL_INVALID_VALUE;
	}
    else
    {
	    memcpy(paramValue, localParamValue, *paramValueSizeRet);
	    *paramValueSizeRet = outputValueSize;
    }
	return res;
}

/******************************************************************
 * This function returns the current status of the command that is
 * corresponding to this event.
 * The status is translated from the m_queueEvent color to OCL API
 * status
 ******************************************************************/
cl_int OclEvent::GetEventCurrentStatus()
{
    if ( (m_queueEvent->IsColor(QueueEvent::EVENT_STATE_YELLOW)) ||
         (m_queueEvent->IsColor(QueueEvent::EVENT_STATE_RED))    ||
         (m_queueEvent->IsColor(QueueEvent::EVENT_STATE_GREEN)))
            return CL_QUEUED;

    if ( m_queueEvent->IsColor(QueueEvent::EVENT_STATE_LIME)) 
            return CL_SUBMITTED;
    
    if ( m_queueEvent->IsColor(QueueEvent::EVENT_STATE_GRAY)) 
            return CL_RUNNING;

    // Case black
    return CL_COMPLETE;
}
