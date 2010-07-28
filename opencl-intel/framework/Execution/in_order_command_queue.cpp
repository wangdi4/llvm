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
//  in_order_command_queue.cpp
//  Implementation of the Class InOrderQueue
//  Created on:      23-Dec-2008 3:23:02 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include "in_order_command_queue.h"
#include "enqueue_commands.h"
#include "ocl_event.h"
#include "device.h"
#include "events_manager.h"
#include "ocl_command_queue.h"
#include "context.h"

#include <assert.h>

using namespace Intel::OpenCL::Framework;
InOrderCommandQueue::InOrderCommandQueue(
	Context*                    pContext,
	cl_device_id                clDefaultDeviceID, 
	cl_command_queue_properties clProperties,
	EventsManager*              pEventManager,
	ocl_entry_points *			pOclEntryPoints
	) :
	IOclCommandQueueBase(pContext, clDefaultDeviceID, clProperties, pEventManager, pOclEntryPoints),
	m_commandsInExecution(0)
{
}
InOrderCommandQueue::~InOrderCommandQueue() 
{
	assert(m_queuedQueue.IsEmpty());
	assert(m_submittedQueue.IsEmpty());
	assert(m_queuedQueueGuard == 0);
	assert(m_submittedQueueGuard == 0);
}

cl_err_code InOrderCommandQueue::Enqueue(Command* cmd)
{
	m_queuedQueue.PushBack(cmd);
	return CL_SUCCESS;
}

cl_err_code InOrderCommandQueue::EnqueueMarker(Command* cmd)
{
	m_queuedQueue.PushBack(cmd);
	//marker-ness guaranteed implicitly by the queue being in-order
	return CL_SUCCESS;
}

cl_err_code InOrderCommandQueue::Flush(bool bBlocking)
{
	long prev_val = m_queuedQueueGuard++;
	if (0 == prev_val)
	{
		do //loop on all requests
		{
			while (!m_queuedQueue.IsEmpty())
			{
				Command* cmd = m_queuedQueue.PopFront();
				if ( m_bProfilingEnabled )
				{
					cmd->GetEvent()->SetProfilingInfo(CL_PROFILING_COMMAND_SUBMIT,
						m_pDefaultDevice->GetDeviceAgent()->clDevGetPerformanceCounter());
				}
				OclEventStateColor color = cmd->GetCommandType() == CL_COMMAND_MARKER || cmd->GetEvent()->HasDependencies() ?
										EVENT_STATE_RED : EVENT_STATE_YELLOW;
				cmd->GetEvent()->SetColor(color);
				m_submittedQueue.PushBack(cmd);
			}
			SendCommandsToDevice();
		} while (--m_queuedQueueGuard > 0);
	}
	else //another thread will handle the flush request
	{
		if (bBlocking) //explicit flush requests must return only when flush is complete
		{
			while (m_queuedQueueGuard > 0); //loop until the handling thread is done with the flush request
		}
	}
	return CL_SUCCESS;
}

cl_err_code InOrderCommandQueue::NotifyStateChange( const OclEvent* cpEvent, OclEventStateColor prevColor, OclEventStateColor newColor )
{
	if (EVENT_STATE_YELLOW == newColor)
	{
		//one of the commands became ready. Trigger a SendToDevice as it may be on top of the queue. Todo: this can probably be improved
		SendCommandsToDevice();
		return CL_SUCCESS;
	} 
	else if ( EVENT_STATE_BLACK == newColor )
	{
		--m_commandsInExecution;
		SendCommandsToDevice();
		return CL_SUCCESS;
	}

	//basically we should not be here until we use this interface for more stuff
	assert(0 && "InOrderCommandQueue::NotifyStateChange called with wrong color");
	return CL_SUCCESS;
}

cl_err_code InOrderCommandQueue::SendCommandsToDevice()
{
	long prev_val = m_submittedQueueGuard++;
	if (0 == prev_val)
	{
		//use my own Queue ID as command list ID for the device
		do //loop on all requests
		{
			cl_uint cmdListLength = 0;
			cl_err_code res;
			while (!m_submittedQueue.IsEmpty())
			{
				Command* cmd = m_submittedQueue.Top();
				OclEventStateColor color = cmd->GetEvent()->GetColor();
				if (EVENT_STATE_YELLOW == color)
				{
					//Ready for execution, schedule it										
					assert(m_pDefaultDevice == cmd->GetDevice());
					cmd->SetDevCmdListId(m_clDevCmdListId);
					cmd->GetEvent()->SetColor(EVENT_STATE_LIME);
					res = cmd->Execute();
					if (CL_SUCCEEDED(res))
					{
						m_commandsInExecution++;
						m_submittedQueue.PopFront();
						++cmdListLength;
					}					
					else if (res == CL_NOT_READY)
					{						
						// keep in the queue
					}					
					else
					{
						// there has been an error, remove from queue
						m_commandsInExecution++;
						m_submittedQueue.PopFront();						
					}
				}
				else if (EVENT_STATE_BLACK == color)
				{
					//Command already complete, flushed by previous command?
					assert(0);
				}
				else if (EVENT_STATE_RED == color)
				{
					if ( (cmd->GetCommandType() == CL_COMMAND_MARKER) && (0 == m_commandsInExecution) )
					{
						m_submittedQueue.PopFront();
						m_commandsInExecution++;
						cmd->Execute();
						continue;
					}
					break; //if a command is red and isn't a marker, there's no point in iterating further
				}
				else
				{
					//Todo: 
					assert(0);
				}
			}
			if (cmdListLength > 0) //any commands were submitted to the device
			{
				m_pDefaultDevice->GetDeviceAgent()->clDevFlushCommandList(m_clDevCmdListId);
			}
		} while (--m_submittedQueueGuard > 0);
	}
	//else return immediately
	return CL_SUCCESS;
}
