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
//  user_event.cpp
//  Implementation of the Class UserEvent
//  Created on:      04-Jul-2010 
//  Original author: Singer, Doron
///////////////////////////////////////////////////////////

#include "user_event.h"
#include "enqueue_commands.h"
#include "command_queue.h"
#include <assert.h>
#include <cl_sys_info.h>

using namespace Intel::OpenCL::Framework;

UserEvent::UserEvent( _cl_context_int* context ) : OclEvent(context)
{
	//User events start as CL_SUBMITTED
	SetEventState(EVENT_STATE_HAS_DEPENDENCIES);
	m_returnCode = 0xdead;
}

UserEvent::~UserEvent()
{
}

void UserEvent::SetComplete(cl_int returnCode)
{
	m_returnCode = returnCode;
	SetEventState(EVENT_STATE_DONE);
}

cl_err_code UserEvent::GetInfo(cl_int iParamName, size_t szParamValueSize, void *paramValue, size_t *szParamValueSizeRet) const
{
	cl_err_code res = CL_SUCCESS;
	const void* localParamValue = NULL;
	size_t outputValueSize = 0;
	cl_int eventStatus;
	cl_command_type cmd_type;
	volatile cl_command_queue cmd_queue;
	cl_context evt_contex;

	switch (iParamName)
	{
	case CL_EVENT_COMMAND_QUEUE:
		cmd_queue = (cl_command_queue)NULL;
		localParamValue = (void*)(&cmd_queue);
		outputValueSize = sizeof(cl_command_queue);
		break;
	case CL_EVENT_CONTEXT:
		evt_contex = GetParentHandle();
		localParamValue = (void*)(&evt_contex);
		outputValueSize = sizeof(cl_context);
		break;
	case CL_EVENT_COMMAND_TYPE:
		cmd_type        = (cl_command_type)CL_COMMAND_USER;
		localParamValue = &cmd_type;
		outputValueSize = sizeof(cl_command_type);
		break;
	case CL_EVENT_REFERENCE_COUNT:
		localParamValue = &m_uiRefCount;
		outputValueSize = sizeof(cl_uint);
		break;
	case CL_EVENT_COMMAND_EXECUTION_STATUS:
		if (EVENT_STATE_DONE == GetEventState())
		{
			eventStatus = m_returnCode;
		}
		else
		{
			eventStatus = CL_SUBMITTED;
		}
		localParamValue = &eventStatus;
		outputValueSize = sizeof(cl_int); 
		break;
	default:
		res = CL_INVALID_VALUE;
		break;
	}

	// check param_value_size
	if ( (NULL != paramValue) && (szParamValueSize < outputValueSize))
	{
		res = CL_INVALID_VALUE;
	}
	else
	{
		if ( NULL != paramValue )
		{
			MEMCPY_S(paramValue, szParamValueSize, localParamValue, outputValueSize);
		}
		if ( NULL != szParamValueSizeRet )
		{
			*szParamValueSizeRet = outputValueSize;
		}
	}
	return res;
}

void UserEvent::NotifyInvisible()
{
    if (CL_COMPLETE != GetEventExecState())
	{
		SetComplete(CL_CONTEXT_CANCEL_INTEL);
	}
}
