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
//  out_of_order_queue.cpp
//  Implementation of the Class OutOfOrderQueue
//  Created on:      25-Mar-2009
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include "out_of_order_command_queue.h"
#include "ocl_event.h"
#include "enqueue_commands.h"
#include "Device.h"
#include "events_manager.h"
#include <assert.h>
#include <cl_utils.h>



using namespace Intel::OpenCL::Framework;

OutOfOrderCommandQueue::OutOfOrderCommandQueue(
	Context*                    pContext,
	cl_device_id                clDefaultDeviceID, 
	cl_command_queue_properties clProperties,
	EventsManager*              pEventManager,
	ocl_entry_points *			pOclEntryPoints
	) :
	IOclCommandQueueBase(pContext, clDefaultDeviceID, clProperties, pEventManager, pOclEntryPoints),
	m_queuedQueueGuard(0),
	m_submittedQueueGuard(0),
	m_runningQueueGuard(0),
	m_commandsInExecution(0),
	m_lastBarrier(NULL)
{		
}

OutOfOrderCommandQueue::~OutOfOrderCommandQueue() 
{
	assert(m_queuedQueue.IsEmpty());
	assert(m_submittedQueue.IsEmpty());
	assert(m_runningQueue.IsEmpty());
	
	assert(m_queuedQueueGuard == 0);
	assert(m_submittedQueueGuard == 0);
	assert(m_runningQueueGuard == 0);	
}

cl_err_code OutOfOrderCommandQueue::Initialize()
{
	 cl_err_code ret = CL_SUCCESS;
	 ret = m_pDefaultDevice->GetDeviceAgent()->clDevCreateCommandList(CL_DEV_LIST_ENABLE_OOO, &m_clDevCmdListId);
	 if (CL_SUCCEEDED(ret))
	 {
		 m_bCommandListCreated = true;
	 }
	 return ret;	
}

cl_err_code OutOfOrderCommandQueue::Enqueue(Command* cmd)
{			
	Command* prev_barrier = (Command*)(m_lastBarrier.test_and_set(NULL,NULL));	
	if (prev_barrier != NULL)
	{		
		OclEvent* cmdEvent = cmd->GetEvent();		
		cmdEvent->AddDependentOn( prev_barrier->GetEvent() );
	}
	m_queuedQueue.PushBack(cmd);

	return CL_SUCCESS;
}

void OutOfOrderCommandQueue::MvQueuedToSubmitted()
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

			cmd->GetEvent()->SetColor(EVENT_STATE_RED);
			if (!cmd->GetEvent()->HasDependencies())
			{
				cmd->GetEvent()->SetColor(EVENT_STATE_YELLOW);
			}
			m_submittedQueue.PushBack(cmd);
		}
	} while (--m_queuedQueueGuard > 0);	
	SendCommandsToDevice();
}

cl_err_code OutOfOrderCommandQueue::Flush(bool bBlocking)
{	
	long prev_val = m_queuedQueueGuard++;
	
	if (0 == prev_val)
	{
		MvQueuedToSubmitted();
	}	
	else //another thread will handle the flush request
	{
		if (bBlocking) //explicit flush requests must return only when flush is complete
		{
			while (m_queuedQueueGuard > 0){}; //loop until the handling thread is done with the flush request			
		}
	}
	return CL_SUCCESS;	
}

void OutOfOrderCommandQueue::TidyRunningQueue()
{
	do
	{
		while (m_runningQueue.Top() != NULL)
		{
			Command* cmd = m_runningQueue.PopFront();
			if (!(cmd->GetEvent()->GetColor() == EVENT_STATE_BLACK))
			{
				// Command finished execution
				m_runningQueue.PushBack(cmd);
			}
			else
			{				
				cmd->GetEvent()->RemovePendency();
			}
		}
		m_runningQueue.PopFront();
	} while (--m_runningQueueGuard > 0);

	SendCommandsToDevice(); // Handle Marker in SubmittedQueue waiting for this command	
}

cl_err_code OutOfOrderCommandQueue::NotifyStateChange( const OclEvent* cpEvent, OclEventStateColor prevColor, OclEventStateColor newColor )
{	
	if (EVENT_STATE_YELLOW == newColor)
	{
		//one of the commands became ready. Trigger a SendToDevice as it may be on top of the queue. Todo: this can probably be improved
		SendCommandsToDevice();
		return CL_SUCCESS;
	} 
	else if ( EVENT_STATE_BLACK == newColor )
	{
		m_runningQueue.PushBack(NULL);
		long prev_val = m_runningQueueGuard++;
		if (prev_val == 0)
		{
			TidyRunningQueue();
		}		
		
		return  CL_SUCCESS;
	}		
	return CL_SUCCESS;
}

void OutOfOrderCommandQueue::MvSubmittedToDevice()
{
	do
	{
		m_submittedQueue.PushBack(NULL);
		cl_uint cmdListLength = 0;
		cl_err_code res;
		while ( m_submittedQueue.Top() != NULL)
		{
			Command* cmd = m_submittedQueue.PopFront();
			OclEventStateColor color = cmd->GetEvent()->GetColor();
			if (!(color == EVENT_STATE_YELLOW))
			{
				// Command is not Ready!
				// Place it back int the submitted queue
				m_submittedQueue.PushBack(cmd);
			}
			else
			{
				
				// Command is Ready
				if ( cmd->isControlCommand() )
				{
					bool isMarker = (cmd->GetCommandType() == CL_COMMAND_MARKER);
					// Control command is Ready
					cmd->GetEvent()->SetColor(EVENT_STATE_LIME);
					cmd->Execute(); // CommandDone() and color change are applied inside Execute() call.

					if ( !isMarker )
					{							
						m_lastBarrier.test_and_set(cmd,NULL);
					}
				}
				else
				{
					// Non-control command is Ready 
					cmd->SetDevCmdListId(m_clDevCmdListId);
					cmd->GetEvent()->SetColor(EVENT_STATE_LIME);
					cmd->GetEvent()->AddPendency();					
					res = cmd->Execute();
					if (CL_SUCCEEDED(res))
					{
						if ( RUNTIME_EXECUTION_TYPE != cmd->GetExecutionType() )
						{
							m_runningQueue.PushBack(cmd);	
							++cmdListLength;
						}
						else
						{
							cmd->GetEvent()->RemovePendency(); // Runtime command done
						}
					}
					else // Not succeeded, check real value
					{
						if (res == CL_NOT_READY)
						{
							if ( EVENT_STATE_RED == cmd->GetEvent()->GetColor() )
							{
								m_submittedQueue.PushBack(cmd);	
								cmd->GetEvent()->RemovePendency();
							}
							else
							{
								m_runningQueue.PushBack(cmd);
							}
						}
						else if (res == CL_DONE_ON_RUNTIME )
						{
							cmd->GetEvent()->RemovePendency();
							cmd->CommandDone();
							cmd->GetEvent()->SetColor(EVENT_STATE_BLACK);
						}
						else
						{
							assert(0);
						}
					}
				}
			}

		}
		m_submittedQueue.PopFront();
		if (cmdListLength > 0) //any commands were submitted to the device
		{
			m_pDefaultDevice->GetDeviceAgent()->clDevFlushCommandList(m_clDevCmdListId);
		}
	}
	while ( --m_submittedQueueGuard > 0);
}

cl_err_code OutOfOrderCommandQueue::SendCommandsToDevice()
{
	
	long prev_val = m_submittedQueueGuard++;
	if (0 == prev_val)
	{		
		MvSubmittedToDevice();
	}
	return CL_SUCCESS;
}


cl_err_code OutOfOrderCommandQueue::EnqueueBarrier(Command* cmd)
{		
	m_lastBarrier.exchange(cmd);
	AddDependentOnAll(cmd);		
	return CL_SUCCESS;
}

cl_err_code OutOfOrderCommandQueue::EnqueueMarker(Command* cmd)
{		
	AddDependentOnAll(cmd);		
	return CL_SUCCESS;
}

cl_err_code OutOfOrderCommandQueue::EnqueueWaitForEvents(Command* cmd)
{		
	Command* prev_barrier = (Command*)(m_lastBarrier.exchange(cmd));
	if (prev_barrier)
	{				
		OclEvent* cmdEvent = cmd->GetEvent();		
		cmdEvent->AddDependentOn( prev_barrier->GetEvent() );
	}
	m_queuedQueue.PushBack(cmd);

	return CL_SUCCESS;	
}

void OutOfOrderCommandQueue::AddDependentOnAll(Command* cmd)
{
	while (m_runningQueueGuard.test_and_set(0,1) != 0){};
	while (m_submittedQueueGuard.test_and_set(0,1) != 0){};
	while (m_queuedQueueGuard.test_and_set(0,1) != 0){};

	// QueuedQueue
	m_queuedQueue.PushBack(cmd);
	cmd->GetEvent()->AddFloatingDependence();
	while (m_queuedQueue.Top() != cmd)
	{
		Command* iCmd = m_queuedQueue.PopFront();
		iCmd->GetCommandType();
		if (!iCmd->isControlCommand())
		{
			cmd->GetEvent()->AddDependentOn( iCmd->GetEvent() );
		}
		m_queuedQueue.PushBack(iCmd);					
	}
	m_queuedQueue.PushBack(m_queuedQueue.PopFront());

	// SubmittedQueue
	if (!m_submittedQueue.IsEmpty())
	{
		m_submittedQueue.PushBack(NULL);
		while (m_submittedQueue.Top() != NULL)
		{
			Command* iCmd = m_submittedQueue.PopFront();
			iCmd->GetCommandType();
			if (!iCmd->isControlCommand())
			{
				cmd->GetEvent()->AddDependentOn( iCmd->GetEvent() );
			}		
			m_submittedQueue.PushBack(iCmd);					
		}
		m_submittedQueue.PopFront();
	}
	// RunningQueue
	if (!m_runningQueue.IsEmpty())
	{
		m_runningQueue.PushBack(NULL);
		while (m_runningQueue.Top() != NULL)
		{
			Command* iCmd = m_runningQueue.PopFront();
			iCmd->GetCommandType();			
			cmd->GetEvent()->AddDependentOn(iCmd->GetEvent());			
			m_runningQueue.PushBack(iCmd);					
		}
		m_runningQueue.PopFront();
	}
	cmd->GetEvent()->RemoveFloatingDependence();
	
	if (--m_queuedQueueGuard > 0)
	{
		// Function decrements m_queuedQueueGuard to 0
		MvQueuedToSubmitted();
	}
	if (--m_submittedQueueGuard > 0)
	{
		// Function decrements m_submittedQueueGuard to 0
		MvSubmittedToDevice();
	}
	if (--m_runningQueueGuard > 0)
	{
		TidyRunningQueue();
	}	
}
