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
#include "Device.h"
#include "events_manager.h"
#include "ocl_command_queue.h"
#include "Context.h"

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
	assert(m_submittedQueue.IsEmpty());
	assert(m_submittedQueueGuard == 0);
}

cl_err_code InOrderCommandQueue::Enqueue(Command* cmd)
{
    if ( m_bProfilingEnabled )
    {
        cmd->GetEvent()->SetProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, m_pDefaultDevice->GetDeviceAgent()->clDevGetPerformanceCounter());
    }				
    cmd->GetEvent()->SetColor(EVENT_STATE_RED);
    if (!cmd->GetEvent()->HasDependencies())
    {
        cmd->GetEvent()->SetColor(EVENT_STATE_YELLOW);
    }
    m_submittedQueue.PushBack(cmd);
	return CL_SUCCESS;
}

cl_err_code InOrderCommandQueue::EnqueueMarker(Command* cmd)
{
	//marker-ness guaranteed implicitly by the queue being in-order
    Enqueue(cmd);
	return CL_SUCCESS;
}

cl_err_code InOrderCommandQueue::Flush(bool bBlocking)
{
	SendCommandsToDevice();
	return CL_SUCCESS;
}

cl_err_code InOrderCommandQueue::NotifyStateChange( QueueEvent* pEvent, OclEventStateColor prevColor, OclEventStateColor newColor )
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
					if ( RUNTIME_EXECUTION_TYPE == cmd->GetExecutionType() )
					{
						if ( m_commandsInExecution > 0)
						{
							break; // Need to wait all executing commands to complete
						}
						m_commandsInExecution++;
						cmd->GetEvent()->AddPendency(this);
						res = cmd->Execute();
						if ( CL_SUCCEEDED(res) )
						{
							m_submittedQueue.PopFront();
							cmd->GetEvent()->RemovePendency(this);
							continue;
						}
						if (res != CL_NOT_READY)
						{						
							assert(0);
							// keep in the queue
							m_submittedQueue.PopFront();
							cmd->GetEvent()->RemovePendency(this);
							m_commandsInExecution--;
							continue;
						}					
						break;
					}
					//Ready for execution, schedule it
					assert(m_pDefaultDevice == cmd->GetDevice());
					cmd->SetDevCmdListId(m_clDevCmdListId);
					cmd->GetEvent()->SetColor(EVENT_STATE_LIME);
					res = cmd->Execute();
					if (CL_SUCCEEDED(res))
					{
						m_submittedQueue.PopFront();
						m_commandsInExecution++;
						++cmdListLength;
					}					
					else if (res == CL_NOT_READY)
					{						
						// keep in the queue
					}					
					else if (res == CL_DONE_ON_RUNTIME)
					{
						// We have additional commands in execution
						cmd->SetCommandType(CL_COMMAND_MARKER);
						cmd->GetEvent()->SetColor(EVENT_STATE_YELLOW);
						if ( 0 == m_commandsInExecution )
						{
							continue;
						}
						break;
					}
					else
					{
						// there has been an error, remove from queue
						assert(0 && "Need to resolve what should be done on error");
						m_commandsInExecution++;
						m_submittedQueue.PopFront();						
					}
				}
				else if (EVENT_STATE_RED == color)
				{
					break;
				}
				else if ( (EVENT_STATE_LIME == color) && (RUNTIME_EXECUTION_TYPE == cmd->GetExecutionType()) )
				{
					break; // Runtime command is still executing
				}
				else if ( EVENT_STATE_BLACK == color )
				{
					// We have completed command in the queue
					// There is two reasons: 1. runtime command, 2. failed command(one of the dependencies failed)
					m_submittedQueue.PopFront();
					cmd->GetEvent()->RemovePendency(this);
					continue;
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
