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
#include <assert.h>
#include <cl_sys_info.h>

using namespace Intel::OpenCL::Framework;

OclEvent::OclEvent() : OCLObject<_cl_event_int>("OclEvent"), m_complete(false), m_color(EVENT_STATE_WHITE)
{
}

OclEvent::~OclEvent()
{
}

void OclEvent::AddDependentOn( OclEvent* pDependsOnEvent)
{
    assert(!m_complete);
	//Must increase dependency list length before adding as listener
	//The other event may have completed already, in which case our callback will be called immediately
	//Which will decrease the depListLength.

	//BugFix: When command waits for queue event and user event, and user event is set with failure, the second notification will fail
	AddPendency(this);	// There is an event that will notify us.
	++m_depListLength;
	pDependsOnEvent->AddCompleteListener(this);
}

//Important to increase the guard by count immediately, otherwise there's a race condition where the first dependency is finished before the second is registered
void OclEvent::AddDependentOnMulti(unsigned int count, OclEvent** pDependencyList)
{
    assert(!m_complete);
	m_depListLength.add(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		OclEvent*& evt = pDependencyList[i];
		if (evt != NULL)
		{
			AddPendency(this); //BugFix: When command waits for queue event and user event, and user event is set with failure, the second notification will fail
			evt->AddCompleteListener(this);
		}
		else
		{
			m_depListLength--;
		}
	}
}

void OclEvent::AddCompleteListener(IEventDoneObserver* listener)
{
	++m_CompleteListenersGuard;
	if (!m_complete)
	{
		m_CompleteListeners.PushBack(listener);
		--m_CompleteListenersGuard;
	}
	else //event completed while we were registering, notify immediately
	{
		--m_CompleteListenersGuard;
		listener->NotifyEventDone(this);
	}
}

OclEventStateColor OclEvent::SetColor(OclEventStateColor color)
{
	//Todo: do we need atomic exchange here?
	OclEventStateColor oldColor = (OclEventStateColor)m_color.exchange(color);
	assert( !((color==EVENT_STATE_BLACK) && (oldColor == EVENT_STATE_BLACK)) );
	if (EVENT_STATE_BLACK == color)
	{
		NotifyComplete(GetReturnCode());
	}
	return oldColor;
}

//Notifies us that pEvent that we were listening on, has completed
cl_err_code OclEvent::NotifyEventDone(OclEvent* pEvent, cl_int returnCode)
{
	// Should remove pendency once was notified
#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
	//Check for notifications from myself
	if (pEvent == this)
	{
		m_osEvent.Signal();
	}
	//And from here continue as usual
#endif
	if (CL_SUCCEEDED(returnCode))
	{
		if (0 == --m_depListLength)
		{
			NotifyReady(pEvent);
		}
	}
	else // a command we depend on has failed, abort with the appropriate error code
	{
		NotifyComplete(returnCode);
	}

	RemovePendency(this);

	return CL_SUCCESS;
}

void OclEvent::NotifyComplete(cl_int returnCode)
{
	//block further requests to add notifiers
	m_complete = true;
	//loop until all pending addition requests are complete
	while (m_CompleteListenersGuard > 0){};

	//Notify everyone
	while (!m_CompleteListeners.IsEmpty())
	{
		IEventDoneObserver* listener = m_CompleteListeners.PopFront();
		assert(listener);
		listener->NotifyEventDone(this, returnCode);
	}
}

void OclEvent::NotifyReady(OclEvent* pEvent)
{
	if (EVENT_STATE_RED == m_color)
	{
		m_color = EVENT_STATE_YELLOW;
    }
}

/******************************************************************
* This function is a synchronization point.
* The host thread waits until this event is done
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
		AddPendency(this);
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
