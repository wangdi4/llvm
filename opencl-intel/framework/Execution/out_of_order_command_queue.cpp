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
	m_depOnAll(NULL),
	m_commandsInExecution(0),
	m_lastBarrier(NULL),
	m_unflushedCommands(0)
{		
}

OutOfOrderCommandQueue::~OutOfOrderCommandQueue() 
{
}

cl_err_code OutOfOrderCommandQueue::Initialize()
{
     cl_dev_subdevice_id subdevice_id = m_pContext->GetSubdeviceId(m_clDefaultDeviceHandle);
	 cl_dev_err_code retDev = m_pDefaultDevice->GetDeviceAgent()->clDevCreateCommandList(CL_DEV_LIST_ENABLE_OOO, subdevice_id, &m_clDevCmdListId);
	 if (CL_DEV_FAILED(retDev))
	 {
		 m_clDevCmdListId = 0;
		 return CL_OUT_OF_RESOURCES;
	 }

     Command* pDepOnAll = new MarkerCommand(this, (ocl_entry_points*)m_handle.dispatch);
     if (NULL == pDepOnAll)
     {
         return CL_OUT_OF_HOST_MEMORY;
     }
     pDepOnAll->GetEvent()->Release();
     // This floating dependence will be resolved at the completion of clEnqueueMarker/Barrier sequence to this queue (AddDependentOnAll)
	 m_depOnAll = pDepOnAll;
	 return CL_SUCCESS;	
}

void OutOfOrderCommandQueue::Submit(Command* cmd)
{
	if ( m_bProfilingEnabled )
	{
		cmd->GetEvent()->SetProfilingInfo(CL_PROFILING_COMMAND_SUBMIT,
			m_pDefaultDevice->GetDeviceAgent()->clDevGetPerformanceCounter());
	}
	cmd->SetDevCmdListId(m_clDevCmdListId);
	cmd->GetEvent()->SetColor(EVENT_STATE_LIME);
	cl_err_code res = cmd->Execute();
	if (CL_SUCCEEDED(res))
	{
		if ( RUNTIME_EXECUTION_TYPE != cmd->GetExecutionType() )
		{
			++m_unflushedCommands;
			// Must flush here or risk deadlock if the tbb task is exiting and this was the last operation done in user code
            Flush(false);
		}
	}
	else // Not succeeded, check real value
	{
		if (res == CL_DONE_ON_RUNTIME )
		{
			cmd->CommandDone();
			cmd->GetEvent()->SetColor(EVENT_STATE_BLACK);
		}
		else
		{
			assert(0);
		}
	}
}

cl_err_code OutOfOrderCommandQueue::Enqueue(Command* cmd)
{
	OclEvent* cmdEvent = cmd->GetEvent();
	m_depOnAll->GetEvent()->AddDependentOn(cmdEvent);
	Command* prev_barrier = (Command*)(m_lastBarrier.test_and_set(NULL,NULL));	
	if (prev_barrier != NULL)
	{		
		cmdEvent->AddDependentOn( prev_barrier->GetEvent() );
	}

  //Todo: get rid of the WHITE->RED->YELLOW color cycle by changing event's listener behaviour
    cmdEvent->AddFloatingDependence();
	cmdEvent->SetColor(EVENT_STATE_RED);
    cmdEvent->RemoveFloatingDependence();
		//Todo: remove

	return CL_SUCCESS;
}

cl_err_code OutOfOrderCommandQueue::EnqueueBarrier(Command* cmd)
{		
    cl_err_code ret = CL_SUCCESS;
	OclEvent* cmdEvent = cmd->GetEvent();
	// Prevent barrier from firing until we're done enqueuing it to avoid races
	cmdEvent->AddFloatingDependence();
	cmdEvent->SetColor(EVENT_STATE_RED);
	m_lastBarrier.exchange(cmd);
	ret = AddDependentOnAll(cmd);		
	cmdEvent->RemoveFloatingDependence();
	return ret;
}

cl_err_code OutOfOrderCommandQueue::EnqueueMarker(Command* cmd)
{
    cl_err_code ret = CL_SUCCESS;
	OclEvent* cmdEvent = cmd->GetEvent();
	// Prevent marker from firing until we're done enqueuing it to avoid races
    cmdEvent->AddFloatingDependence();
	cmdEvent->SetColor(EVENT_STATE_RED);
	ret = AddDependentOnAll(cmd);		
	cmdEvent->RemoveFloatingDependence();
	return ret;
}

cl_err_code OutOfOrderCommandQueue::EnqueueWaitForEvents(Command* cmd)
{		
	OclEvent* cmdEvent = cmd->GetEvent();
    cmdEvent->AddFloatingDependence();
	cmdEvent->SetColor(EVENT_STATE_RED);
	m_depOnAll->GetEvent()->AddDependentOn(cmdEvent);
	Command* prev_barrier = (Command*)(m_lastBarrier.exchange(cmd));
	if (prev_barrier)
	{				
		OclEvent* cmdEvent = cmd->GetEvent();		
		cmdEvent->AddDependentOn( prev_barrier->GetEvent() );
	}
	cmdEvent->RemoveFloatingDependence();
	return CL_SUCCESS;	
}

cl_err_code OutOfOrderCommandQueue::Flush(bool bBlocking)
{	
	long prev = m_unflushedCommands.exchange(0);
	if (prev > 0)
	{
		m_pDefaultDevice->GetDeviceAgent()->clDevFlushCommandList(m_clDevCmdListId);
	}

	return CL_SUCCESS;	
}

cl_err_code OutOfOrderCommandQueue::NotifyStateChange( QueueEvent* pEvent, OclEventStateColor prevColor, OclEventStateColor newColor )
{	
	if (EVENT_STATE_YELLOW == newColor)
	{
		Command* cmd = pEvent->GetCommand();
		if ( cmd->isControlCommand() )
		{
			bool isMarker = (CL_COMMAND_MARKER == cmd->GetCommandType());
			// Control command is Ready
			pEvent->SetColor(EVENT_STATE_LIME);
			cmd->Execute(); // CommandDone() and color change are applied inside Execute() call.

			if ( !isMarker )
			{							
				m_lastBarrier.test_and_set(cmd,NULL);
			}
		}
		else
		{
			Submit(cmd);
		}
    }
	return CL_SUCCESS;
}



cl_err_code OutOfOrderCommandQueue::AddDependentOnAll(Command* cmd)
{
    assert(NULL != cmd);
	Command* pNewDepOnAll = new MarkerCommand(this, (ocl_entry_points*)m_handle.dispatch);
    if (NULL == pNewDepOnAll)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    QueueEvent* pNewDepnOnAllEvent = pNewDepOnAll->GetEvent();
    QueueEvent* pCommandEvent      = cmd->GetEvent();
    pNewDepnOnAllEvent->Release();
    
    // First of all create a new "depends on all" object and put it in place.
    // Then link dependencies: new "dep on all" depends on command (marker/barrier) depends on old "dep on all"
    // Finally remove the floating dependence to allow the thing to resolve in due course.

	Command* pOldDepOnAll = (Command*)m_depOnAll.exchange(pNewDepOnAll);
    QueueEvent* pOldDepOnAllEvent = pOldDepOnAll->GetEvent();
	pOldDepOnAllEvent->AddFloatingDependence();
	pOldDepOnAllEvent->SetColor(EVENT_STATE_RED);
	pCommandEvent->AddDependentOn(pOldDepOnAllEvent);
	pOldDepOnAllEvent->RemoveFloatingDependence();
    return CL_SUCCESS;
}
void OutOfOrderCommandQueue::NotifyInvisible()
{
    m_depOnAll->GetEvent()->RemovePendency(NULL);
}
