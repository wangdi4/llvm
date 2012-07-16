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
using namespace Intel::OpenCL::Utils;

OutOfOrderCommandQueue::OutOfOrderCommandQueue(
	SharedPtr<Context>                    pContext,
	cl_device_id                clDefaultDeviceID, 
	cl_command_queue_properties clProperties,
	EventsManager*              pEventManager
	) :
	IOclCommandQueueBase(pContext, clDefaultDeviceID, clProperties, pEventManager),
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

     Command* pDepOnAll = new MarkerCommand(this, 0);
     if (NULL == pDepOnAll)
     {
         return CL_OUT_OF_HOST_MEMORY;
     }
     pDepOnAll->GetEvent()->Release();
     // This floating dependence will be resolved at the completion of clEnqueueMarker/Barrier sequence to this queue (AddDependentOnAll)
	 m_depOnAll = pDepOnAll;
	 return CL_SUCCESS;	
}

long OutOfOrderCommandQueue::Release()
{
    const long ref = IOclCommandQueueBase::Release();
    if (0 == ref && m_depOnAll)
    {
        delete m_depOnAll;
    }
    return ref;
}

void OutOfOrderCommandQueue::Submit(Command* cmd)
{
	if ( m_bProfilingEnabled )
	{
		cmd->GetEvent()->SetProfilingInfo(CL_PROFILING_COMMAND_SUBMIT,
			m_pDefaultDevice->GetDeviceAgent()->clDevGetPerformanceCounter());
	}
	cmd->SetDevCmdListId(m_clDevCmdListId);
	cmd->GetEvent()->SetEventState(EVENT_STATE_ISSUED_TO_DEVICE);
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
			cmd->GetEvent()->SetEventState(EVENT_STATE_DONE);
			cmd->CommandDone();
		}
		else
		{
			assert(res == CL_NOT_READY);
		}
	}
}

cl_err_code OutOfOrderCommandQueue::Enqueue(Command* cmd)
{
	OclAutoMutex mu(&m_muLastBarrer);
	
	SharedPtr<OclEvent> cmdEvent = cmd->GetEvent();
	m_depOnAll->GetEvent()->AddDependentOn(cmdEvent);
	Command* prev_barrier = (Command*)(m_lastBarrier.test_and_set(NULL,NULL));
	// BUBUG: The bug is here, thus I added the mutex.
	// If at this stage the barrier is completed, the prev_barrier pointer is not valid.

	// We have barrier command in the queue	
	if (prev_barrier != NULL)
	{		
		cmdEvent->AddDependentOn( prev_barrier->GetEvent() );
	}

	//Todo: get rid of the WHITE->RED->YELLOW color cycle by changing event's listener behaviour
    cmdEvent->AddFloatingDependence();
	cmdEvent->SetEventState(EVENT_STATE_HAS_DEPENDENCIES);
    cmdEvent->RemoveFloatingDependence();
	return CL_SUCCESS;
}

cl_err_code OutOfOrderCommandQueue::EnqueueMarkerWaitForEvents(Command* marker)
{
    OclEvent& cmdEvent = *marker->GetEvent();
    if (!marker->IsDependentOnEvents())
    {
        // Prevent marker from firing until we're done enqueuing it to avoid races
        cmdEvent.AddFloatingDependence();
        cmdEvent.SetEventState(EVENT_STATE_HAS_DEPENDENCIES);
        const cl_err_code ret = AddDependentOnAll(marker);
        cmdEvent.RemoveFloatingDependence();
        return ret;
    }
    cmdEvent.AddFloatingDependence();
    cmdEvent.SetEventState(EVENT_STATE_HAS_DEPENDENCIES);
    m_depOnAll->GetEvent()->AddDependentOn(&cmdEvent);
    cmdEvent.RemoveFloatingDependence();
    return CL_SUCCESS;
}

cl_err_code OutOfOrderCommandQueue::EnqueueBarrierWaitForEvents(Command* barrier)
{
    SharedPtr<OclEvent> cmdEvent = barrier->GetEvent();
    if (!barrier->IsDependentOnEvents())
    {
		OclAutoMutex mu(&m_muLastBarrer);
		
        // Prevent barrier from firing until we're done enqueuing it to avoid races
        cmdEvent->AddFloatingDependence();
        cmdEvent->SetEventState(EVENT_STATE_HAS_DEPENDENCIES);
        cmdEvent.IncRefCnt();   // TODO: add a comment why this is necessary
        Command* prev_barrier = m_lastBarrier.exchange(barrier);
        if ( NULL != prev_barrier )
        {
        	// Need to remove pendency from the previous barrier
            prev_barrier->GetEvent().DecRefCnt();
        }
        const cl_err_code ret = AddDependentOnAll(barrier);		
        cmdEvent->RemoveFloatingDependence();
        return ret;
    }
    return EnqueueWaitForEvents(barrier);
}

cl_err_code OutOfOrderCommandQueue::EnqueueWaitForEvents(Command* cmd)
{		
	OclAutoMutex mu(&m_muLastBarrer);

	SharedPtr<OclEvent> cmdEvent = cmd->GetEvent();
    cmdEvent.IncRefCnt();   // TODO: add a comment why this is necessary
	cmdEvent->SetEventState(EVENT_STATE_HAS_DEPENDENCIES);
	m_depOnAll->GetEvent()->AddDependentOn(cmdEvent);
	Command* prev_barrier = (Command*)(m_lastBarrier.exchange(cmd));
	if ( NULL != prev_barrier)
	{					
		cmdEvent->AddDependentOn( prev_barrier->GetEvent() );
        prev_barrier->GetEvent().DecRefCnt();
	}
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

cl_err_code OutOfOrderCommandQueue::NotifyStateChange( SharedPtr<QueueEvent> pEvent, OclEventState prevColor, OclEventState newColor )
{	
	if (EVENT_STATE_READY_TO_EXECUTE == newColor)
	{
		Command* cmd = pEvent->GetCommand();
		if ( cmd->isControlCommand() )
		{
			bool isMarker = (CL_COMMAND_MARKER == cmd->GetCommandType());
			// Control command is Ready
			pEvent->SetEventState(EVENT_STATE_ISSUED_TO_DEVICE);
			cmd->Execute(); // CommandDone() and color change are applied inside Execute() call.

			if ( !isMarker )
			{							
				OclAutoMutex mu(&m_muLastBarrer);
				Command* prev_barrier = m_lastBarrier.test_and_set(cmd,NULL);
				// This is same barrier, and current lastBarrier is NULL
				if ( prev_barrier == cmd )
				{
                    //cmd->GetEvent().DecRefCnt();
				}
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

	Command* pNewDepOnAll = new MarkerCommand(this, 0);
    if (NULL == pNewDepOnAll)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    SharedPtr<QueueEvent> pNewDepnOnAllEvent = pNewDepOnAll->GetEvent();
    SharedPtr<QueueEvent> pCommandEvent      = cmd->GetEvent();
    pNewDepnOnAllEvent->Release();
    
    // First of all create a new "depends on all" object and put it in place.
    // Then link dependencies: new "dep on all" depends on command (marker/barrier) depends on old "dep on all"
    // Finally remove the floating dependence to allow the thing to resolve in due course.

	Command* pOldDepOnAll = (Command*)m_depOnAll.exchange(pNewDepOnAll);
    SharedPtr<QueueEvent> pOldDepOnAllEvent = pOldDepOnAll->GetEvent();
	pOldDepOnAllEvent->AddFloatingDependence();
	pOldDepOnAllEvent->SetEventState(EVENT_STATE_HAS_DEPENDENCIES);
	pCommandEvent->AddDependentOn(pOldDepOnAllEvent);
	pOldDepOnAllEvent->RemoveFloatingDependence();
    return CL_SUCCESS;
}
