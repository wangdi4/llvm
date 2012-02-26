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
//  ocl_event.cpp
//  Implementation of the Class OclEvent
//  Created on:      23-Dec-2008 3:23:05 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include <assert.h>
#include <cl_sys_info.h>

#include "ocl_event.h"
#include "command_queue.h"

using namespace Intel::OpenCL::Framework;

#ifdef __GNUC__
	#define UNUSED_ATTR __attribute__((unused))
#else
	#define UNUSED_ATTR
#endif

static const char UNUSED_ATTR *GetEventStateName(const cl_int state)
{
#define CASE_ENTRY(x) case x: return #x;
	switch(state)
	{
	CASE_ENTRY(EVENT_STATE_CREATED)
	CASE_ENTRY(EVENT_STATE_HAS_DEPENDENCIES)
	CASE_ENTRY(EVENT_STATE_READY_TO_EXECUTE)
	CASE_ENTRY(EVENT_STATE_ISSUED_TO_DEVICE)
	CASE_ENTRY(EVENT_STATE_EXECUTING_ON_DEVICE)
	CASE_ENTRY(EVENT_STATE_DONE)
	default:
		return "error state";
	}
	return "error state";
}

OclEvent::OclEvent()
	: OCLObject<_cl_event_int>("OclEvent"),
	  m_numOfDependencies(0),
	  m_complete(false), m_returnCode(CL_SUCCESS),
	  m_eventState(EVENT_STATE_CREATED)
{
	/** On Windows the actual state change triggers all dependencies. On Linux we use os event instead. **/
#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
	bool UNUSED_ATTR initOK = m_osEvent.Init();
	assert(initOK && "OclEvent Failed to setup OS_DEPENDENT event.");
#endif
}

OclEvent::~OclEvent()
{
	// Make sure we release all rogue dependencies. This is a backup code, for bad cases!
	ExpungeObservers(m_CompleteObserversList);
	ExpungeObservers(m_SubmittedObserversList);
	ExpungeObservers(m_RunningObserversList);
}

void OclEvent::ExpungeObservers(ObserversList_t &list)
{
	// during debug, this is an error! We should never call expunge on non empty lists!
	assert(list.empty());
	ObserversList_t::iterator it;
	for (it = list.begin() ; it != list.end() ; ++it)
	{
		IEventObserver *observer= *it;
		OclEvent *evt = dynamic_cast<OclEvent*>(observer);
		if (evt) RemovePendency(this); // make sure we free all observers.
	}
	list.clear();
}

/**
 * Sugaring over AddDependentOnMulti, mainly to stay backwards compatible.
 * @param pDependsOnEvent
 */
void OclEvent::AddDependentOn( OclEvent* pDependsOnEvent)
{
	OclEvent *evtList[1];
	evtList[0] = pDependsOnEvent;
	return AddDependentOnMulti(1, evtList);
}

/**
 * Add dependency on multiple events at once.
 * @param count
 * @param pDependencyList
 */
void OclEvent::AddDependentOnMulti(unsigned int count, OclEvent** pDependencyList)
{
	assert(!m_complete && "A weird race happened and an event finished during AddDependentOnMulti.");
    if (m_complete)
    {
    	return;
    }

	bool bLastWasNull = false;
	// set the counter first, to make sure we register/process all events even if some instantaneously fire.
	m_numOfDependencies.add(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		OclEvent*& evt = pDependencyList[i];
		if (evt != NULL)
		{
			// Normal flow, add the dependency
			AddPendency(this); //BugFix: When command waits for queue event and user event,
			// and user event is set with failure, the second notification will fail
			// do not: SetEventState(EVENT_STATE_HAS_DEPENDENCIES);
			evt->AddObserver(this); // might trigger this immediately, if evt already occurred.
			bLastWasNull = false;
		} else {
			cl_int depsLeft = --m_numOfDependencies;
			if (0 == depsLeft)
			{
				// we dropped the list to 0 on a bogus NULL event.
				bLastWasNull = true;
			}
		}
	}

	// Special case: we thought we have at least one dependency, but we actually have none.
	// EventWasTrigerred will never be called, so change our color immediately.
	if (bLastWasNull)
	{
		DoneWithDependencies(NULL);
	}
}


void OclEvent::AddObserver(IEventObserver* observer)
{
	m_ObserversListGuard.Lock();

	if (observer->GetExpectedExecState() >= GetEventExecState())
	{
		//event completed while we were registering, or already happened, notify immediately
		m_ObserversListGuard.Unlock();
		cl_int retcode = GetReturnCode();
		retcode = retcode < 0 ? retcode : observer->GetExpectedExecState();
		observer->ObservedEventStateChanged(this, retcode);
	}
	else
	{
		switch(observer->GetExpectedExecState())
		{
		case CL_COMPLETE:
			m_CompleteObserversList.push_back(observer);
			break;
		case CL_RUNNING:
			m_RunningObserversList.push_back(observer);
			break;
		case CL_SUBMITTED:
			m_SubmittedObserversList.push_back(observer);
			break;
		default:
			assert(0 && "Trying to add an observer to invalid exec state.");
		}
		m_ObserversListGuard.Unlock();
	}
}


OclEventState OclEvent::SetEventState(const OclEventState newEventState)
{
	//Todo: do we need atomic exchange here?
	OclEventState oldEventState = GetEventState();
	cl_int        oldExecState  = GetEventExecState();
	m_eventState = newEventState;
	assert( !((newEventState==EVENT_STATE_DONE) && (oldEventState == EVENT_STATE_DONE)) );

	if (GetEventExecState() != oldExecState)
	{
		switch (GetEventExecState())
		{
		case CL_QUEUED:
			// queued, but without notification.
			//std::cerr << "SetEventState CL_QUEUED (" << GetEventStateName(m_eventState) << ") " << this << std::endl;
			break;

		case CL_SUBMITTED:
			//std::cerr << "SetEventState CL_SUBMITTED (" << GetEventStateName(m_eventState) << ") " << this << std::endl;
			NotifySubmitted();
			break;

		case CL_RUNNING:
			//std::cerr << "SetEventState CL_RUNNING (" << GetEventStateName(m_eventState) << ") " << this << std::endl;
			NotifyRunning();
			break;

		case CL_COMPLETE:
			//std::cerr << "SetEventState CL_COMPLETE (" << GetReturnCode() << ") (" << GetEventStateName(m_eventState) << ") " << this << std::endl;
			/** On Windows the actual state change triggers all dependencies. On Linux we use os event instead. **/
			#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
				m_osEvent.Signal();
			#endif
			NotifyComplete(GetReturnCode());
			break;

		default:
			assert(0 && "OclEvent::SetEventState undefined new status.");
		}
	}

	return oldEventState;
}

/**
 * Notifies us that pEvent that we were listening on, has completed
 * Assumes we are called on our required event, or on error.
 */
cl_err_code OclEvent::ObservedEventStateChanged(OclEvent* pEvent, cl_int returnCode)
{
	assert(returnCode <= 0 && "OclEvent got a non complete return code.");

	if (CL_SUCCESS > returnCode)
	{
		// a command we depend on has finished with errors.
		m_returnCode = returnCode;
		SetEventState(EVENT_STATE_DONE);
	}
	else if (0 == --m_numOfDependencies)
	{
		DoneWithDependencies(pEvent);
	}

	return CL_SUCCESS;
}


void OclEvent::NotifyComplete(cl_int returnCode)
{
	//block further requests to add notifiers
	m_complete = true;

	NotifyObservers(returnCode);
}


void OclEvent::NotifySubmitted()
{
	NotifyObservers(CL_SUBMITTED);
}


void OclEvent::NotifyRunning()
{
	NotifyObservers(CL_RUNNING);
}


void OclEvent::DoneWithDependencies(OclEvent* pEvent)
{
	if (EVENT_STATE_HAS_DEPENDENCIES == GetEventState())
	{
		SetEventState(EVENT_STATE_READY_TO_EXECUTE);
    }
}


void OclEvent::NotifyObservers(const cl_int retCode)
{
	m_ObserversListGuard.Lock();

	// If error (negative), notify (and dispatch) all.
	cl_int execState = retCode < 0 ? CL_COMPLETE : retCode;

	// Each state should also trigger previous state observers, if any, hence no break.
	switch(execState)
	{
	case CL_COMPLETE:
		NotifyObserversOfSingleExecState(m_CompleteObserversList, retCode);
	case CL_RUNNING:
		NotifyObserversOfSingleExecState(m_RunningObserversList, retCode);
	case CL_SUBMITTED:
		NotifyObserversOfSingleExecState(m_SubmittedObserversList, retCode);
		break;
	default:
		m_ObserversListGuard.Unlock();
		assert(0 && "invalid exec state to notify observers.");
	}

	m_ObserversListGuard.Unlock();
}

/**
 * NOTE: unsafe, this function does not lock the mutex. It should ONLY be called from thread-safe functions!
 * @param list
 * @param retCode
 */
void OclEvent::NotifyObserversOfSingleExecState(ObserversList_t &list, const cl_int retCode)
{
	ObserversList_t::iterator it;
	for (it = list.begin() ; it != list.end() ; ++it)
	{
		IEventObserver *observer= *it;
		observer->ObservedEventStateChanged(this, retCode);
	}
	list.clear();
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
	while (GetEventState() != EVENT_STATE_DONE)
	{
	}
}
void OclEvent::WaitYield()
{
	while (GetEventState() != EVENT_STATE_DONE)
	{
		clSleep(0);
	}
}
void OclEvent::WaitOSEvent()
{
#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
	m_osEvent.Wait();
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
cl_int OclEvent::GetEventExecState() const
{
	switch(GetEventState())
	{
	case EVENT_STATE_CREATED:
		return CL_QUEUED;
	case EVENT_STATE_HAS_DEPENDENCIES:
		// Fall through
	case EVENT_STATE_READY_TO_EXECUTE:
		// Fall through
	case EVENT_STATE_ISSUED_TO_DEVICE:
		return CL_SUBMITTED;
	case EVENT_STATE_EXECUTING_ON_DEVICE:
		return CL_RUNNING;
	case EVENT_STATE_DONE:
	default:
		return CL_COMPLETE;
	}        
}
