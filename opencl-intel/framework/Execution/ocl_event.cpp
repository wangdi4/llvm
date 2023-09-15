// INTEL CONFIDENTIAL
//
// Copyright 2008 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "ocl_event.h"
#include "cl_shared_ptr.hpp"
#include "cl_sys_info.h"
#include "command_queue.h"
#include "llvm/Support/Compiler.h" // LLVM_FALLTHROUGH
#include <assert.h>

using namespace Intel::OpenCL::Framework;

#ifdef __GNUC__
#define UNUSED_ATTR __attribute__((unused))
#else
#define UNUSED_ATTR
#endif

#define OCL_EVENT_COMPLETED ((Intel::OpenCL::Utils::OclOsDependentEvent *)(-1))

static const char UNUSED_ATTR *GetEventStateName(const cl_int state) {
#define CASE_ENTRY(x)                                                          \
  case x:                                                                      \
    return #x;
  switch (state) {
    CASE_ENTRY(EVENT_STATE_CREATED)
    CASE_ENTRY(EVENT_STATE_HAS_DEPENDENCIES)
    CASE_ENTRY(EVENT_STATE_READY_TO_EXECUTE)
    CASE_ENTRY(EVENT_STATE_ISSUED_TO_DEVICE)
    CASE_ENTRY(EVENT_STATE_EXECUTING_ON_DEVICE)
    CASE_ENTRY(EVENT_STATE_DONE_EXECUTING_ON_DEVICE)
    CASE_ENTRY(EVENT_STATE_DONE)
  default:
    return "error state";
  }
}

OclEvent::OclEvent(_cl_context_int *context)
    : OCLObject<_cl_event_int>(context, "OclEvent"), m_returnCode(CL_SUCCESS),
#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
      m_pCurrentEvent(NULL),
#else
      m_complete(false),
#endif
      m_eventState(EVENT_STATE_CREATED), m_pContext(NULL) {
  if (NULL != context) {
    m_pContext = (Context *)(context->object);
  }
}

OclEvent::~OclEvent() {
  // Make sure we release all rogue dependencies. This is a backup code, for bad
  // cases!
  ExpungeObservers(m_CompleteObserversList);
  ExpungeObservers(m_SubmittedObserversList);
  ExpungeObservers(m_RunningObserversList);

#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
  if ((NULL != m_pCurrentEvent) && (OCL_EVENT_COMPLETED != m_pCurrentEvent)) {
    ((Context *)GetParentHandle()->object)->RecycleOSEvent(m_pCurrentEvent);
    m_pCurrentEvent = NULL;
  }
#endif
}

void OclEvent::ExpungeObservers(ObserversList_t &list) {
  ObserversList_t::iterator it;
  list.clear();
}

/**
 * Sugaring over AddDependentOnMulti, mainly to stay backwards compatible.
 * @param pDependsOnEvent
 */
void OclEvent::AddDependentOn(const SharedPtr<OclEvent> &pDependsOnEvent) {
  SharedPtr<OclEvent> evtList[] = {pDependsOnEvent};

  return AddDependentOnMulti(1, evtList);
}

/**
 * Add dependency on multiple events at once.
 * @param count
 * @param pDependencyList
 */
void OclEvent::AddDependentOnMulti(unsigned int count,
                                   SharedPtr<OclEvent> *pDependencyList) {
  if (0 == count) {
    return;
  }
#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
  Intel::OpenCL::Utils::OclOsDependentEvent *pPrevEvent =
      m_pCurrentEvent.test_and_set(NULL, NULL);
  assert(OCL_EVENT_COMPLETED != pPrevEvent &&
         "Event was already set as completed");
  if (OCL_EVENT_COMPLETED == pPrevEvent) {
    return;
  }
#else
  assert(!m_complete && "A weird race happened and an event finished during "
                        "AddDependentOnMulti.");
  if (m_complete) {
    return;
  }
#endif

  bool bLastWasNull = false;
  // set the counter first, to make sure we register/process all events even if
  // some instantaneously fire.
  m_numOfDependencies.fetch_add(count);
  SetEventState(EVENT_STATE_HAS_DEPENDENCIES);

  for (unsigned int i = 0; i < count; ++i) {
    SharedPtr<OclEvent> &evt = pDependencyList[i];
    if (evt.GetPtr() != NULL) {
      // Normal flow, add the dependency
      // AddPendency(evt);
      // BugFix: When command waits for queue event and user event, and user
      // event is set with failure, the second notification will fail do
      // not: SetEventState (EVENT_STATE_HAS_DEPENDENCIES);
      evt->AddObserver(
          SharedPtr<OclEvent>(this)); // might trigger this immediately, if
                                      // evt already occurred.
      bLastWasNull = false;
    } else {
      cl_int depsLeft = --m_numOfDependencies;
      if (0 == depsLeft) {
        // we dropped the list to 0 on a bogus NULL event.
        bLastWasNull = true;
      }
    }
  }

  // Special case: we thought we have at least one dependency, but we actually
  // have none. EventWasTrigerred will never be called, so change our color
  // immediately.
  if (bLastWasNull) {
    DoneWithDependencies(NULL);
  }
}

void OclEvent::AddObserver(const SharedPtr<IEventObserver> &pObserver) {
  m_ObserversListGuard.lock();
  IEventObserver *observer = pObserver.GetPtr();

  cl_int currExecState = GetEventExecState();
  cl_int expectedState = observer->GetExpectedExecState();

  if (expectedState >= currExecState) {
    // event completed while we were registering, or already happened, notify
    // immediately
    m_ObserversListGuard.unlock();
    cl_int retcode = GetReturnCode();
    // Evgeny: Should find another way to propogate the EXEC_STATE.
    //            retcode has one notation
    retcode = retcode < 0 ? retcode : expectedState;
    observer->ObservedEventStateChanged(this, retcode);
  } else {
    switch (expectedState) {
    case CL_COMPLETE:
      m_CompleteObserversList.push_back(pObserver);
      break;
    case CL_RUNNING:
      m_RunningObserversList.push_back(pObserver);
      break;
    case CL_SUBMITTED:
      m_SubmittedObserversList.push_back(pObserver);
      break;
    default:
      assert(0 && "Trying to add an observer to invalid exec state.");
    }
    m_ObserversListGuard.unlock();
  }
}

OclEventState OclEvent::SetEventState(const OclEventState newEventState) {
  // Todo: do we need atomic exchange here?
  OclEventState oldEventState = GetEventState();
  cl_int oldExecState = GetEventExecState(oldEventState);

  m_eventState = newEventState;

  assert(!((newEventState == EVENT_STATE_DONE) &&
           (oldEventState == EVENT_STATE_DONE)));

  cl_int newExecState = GetEventExecState(newEventState);

  if (newExecState != oldExecState) {
    switch (newExecState) {
    case CL_QUEUED:
      // queued, but without notification.
      // std::cerr << "SetEventState CL_QUEUED (" <<
      // GetEventStateName(m_eventState) << ") " << this << std::endl;
      break;

    case CL_SUBMITTED:
      // std::cerr << "SetEventState CL_SUBMITTED (" <<
      // GetEventStateName(m_eventState) << ") " << this << std::endl;
      NotifySubmitted();
      break;

    case CL_RUNNING:
      // std::cerr << "SetEventState CL_RUNNING (" <<
      // GetEventStateName(m_eventState) << ") " << this << std::endl;
      NotifyRunning();
      break;

    case CL_COMPLETE:
      // std::cerr << "SetEventState CL_COMPLETE (" << GetReturnCode() << ") ("
      // << GetEventStateName(m_eventState) << ") " << this << std::endl;
      /** On Windows the actual state change triggers all dependencies. On Linux
       * we use os event instead. **/
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
cl_err_code
OclEvent::ObservedEventStateChanged(const SharedPtr<OclEvent> &pEvent,
                                    cl_int returnCode) {
  assert(returnCode <= 0 && "OclEvent got a non complete return code.");

  // ALERT!!!! DK!!!!
  if (CL_SUCCESS > returnCode) {
    assert(0 && "The case with event completed with negative value should "
                "already be processed");
    // a command we depend on has finished with errors.
    m_returnCode = returnCode;
    SetEventState(EVENT_STATE_DONE);
  } else if (0 == --m_numOfDependencies) {
    DoneWithDependencies(pEvent);
  }
  return CL_SUCCESS;
}

void OclEvent::MarkAsComplete() {
#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
  Intel::OpenCL::Utils::OclOsDependentEvent *pPrevEvent =
      m_pCurrentEvent.test_and_set(NULL, OCL_EVENT_COMPLETED);
  assert(OCL_EVENT_COMPLETED != pPrevEvent &&
         "Event was already set as completed");
  // printf("this=%p, NotifyComplete() - pPrevEvent=%p\n", (void*)this,
  // (void*)pPrevEvent);
  if ((NULL != pPrevEvent) && (OCL_EVENT_COMPLETED != pPrevEvent)) {
    // printf("this=%p, Signaling event %p\n", (void*)this, (void*)pPrevEvent);
    pPrevEvent->Signal();
  }
#else
  assert(!m_complete && "Trying second notification");
  m_complete = true;
#endif
}

void OclEvent::NotifyComplete(cl_int returnCode) {
  // block further requests to add notifiers
  MarkAsComplete();
  NotifyObservers(returnCode);
}

void OclEvent::NotifySubmitted() { NotifyObservers(CL_SUBMITTED); }

void OclEvent::NotifyRunning() { NotifyObservers(CL_RUNNING); }

void OclEvent::DoneWithDependencies(const SharedPtr<OclEvent> & /*pEvent*/) {
  if (EVENT_STATE_HAS_DEPENDENCIES == GetEventState()) {
    SetEventState(EVENT_STATE_READY_TO_EXECUTE);
  }
}

void OclEvent::NotifyObservers(const cl_int retCode) {
  m_ObserversListGuard.lock();

  // If error (negative), notify (and dispatch) all.
  cl_int execState = retCode < 0 ? CL_COMPLETE : retCode;

  // Each state should also trigger previous state observers, if any, hence no
  // break.
  switch (execState) {
  case CL_COMPLETE:
    NotifyObserversOfSingleExecState(m_CompleteObserversList, retCode);
    LLVM_FALLTHROUGH;
  case CL_RUNNING:
    NotifyObserversOfSingleExecState(m_RunningObserversList, retCode);
    LLVM_FALLTHROUGH;
  case CL_SUBMITTED:
    NotifyObserversOfSingleExecState(m_SubmittedObserversList, retCode);
    break;
  default:
    m_ObserversListGuard.unlock();
    assert(0 && "invalid exec state to notify observers.");
  }

  m_ObserversListGuard.unlock();
}

/**
 * NOTE: unsafe, this function does not lock the mutex. It should ONLY be called
 * from thread-safe functions!
 * @param list
 * @param retCode
 */
void OclEvent::NotifyObserversOfSingleExecState(ObserversList_t &list,
                                                const cl_int retCode) {
  ObserversList_t::iterator it;
  for (it = list.begin(); it != list.end(); ++it) {
    IEventObserver *observer = it->GetPtr();
    observer->ObservedEventStateChanged(this, retCode);
  }
  list.clear();
}

/******************************************************************
 * This function is a synchronization point.
 * The host thread waits until this event is done
 ******************************************************************/
void OclEvent::Wait() {
#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_SPIN
  if (!m_complete) {
    WaitSpin();
  }
#elif OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_YIELD
  if (!m_complete) {
    WaitYield();
  }
#elif OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
  WaitOSEvent();
#else
#error "Please define which wait method OclEvent should use. See ocl_event.h"
#endif
}

void OclEvent::WaitSpin() {
  while (GetEventState() != EVENT_STATE_DONE) {
  }
}
void OclEvent::WaitYield() {
  while (GetEventState() != EVENT_STATE_DONE) {
    clSleep(0);
  }
}
void OclEvent::WaitOSEvent() {
#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
  // Check if event already completed, exit
  Intel::OpenCL::Utils::OclOsDependentEvent *pPrevEvent =
      m_pCurrentEvent.test_and_set(NULL, NULL);
  if (OCL_EVENT_COMPLETED == pPrevEvent) {
    return;
  }

  if (NULL != pPrevEvent) {
    // There is another OS event allocated, wait for it
    // printf("Wait(1) event %p\n", (void*)pPrevEvent);
    pPrevEvent->Wait();
    return;
  }

  // If not allocate OS event to wait on
  Intel::OpenCL::Utils::OclOsDependentEvent *pOsEvent =
      ((Context *)(GetParentHandle()->object))->GetOSEvent();
  if (NULL == pOsEvent) {
    assert(pOsEvent && "Can't retive OS event");
    WaitYield();
    return;
  }

  pPrevEvent = m_pCurrentEvent.test_and_set(NULL, pOsEvent);
  if (NULL != pPrevEvent) {
    // If previous value is set, the event is completed or already set
    // Current event should be returned to the pool
    ((Context *)(GetParentHandle()->object))->RecycleOSEvent(pOsEvent);
    if (OCL_EVENT_COMPLETED == pPrevEvent) {
      // If event completed, exit
      return;
    }
  } else {
    pPrevEvent = pOsEvent;
  }

  pPrevEvent->Wait();
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
cl_int OclEvent::GetEventExecState() const {
  return GetEventExecState(GetEventState());
}

cl_int OclEvent::GetEventExecState(const OclEventState state) const {
  switch (state) {
  case EVENT_STATE_CREATED:
    return CL_QUEUED;
  case EVENT_STATE_HAS_DEPENDENCIES:
    // Fall through
  case EVENT_STATE_READY_TO_EXECUTE:
    // Fall through
  case EVENT_STATE_ISSUED_TO_DEVICE:
    return CL_SUBMITTED;
  case EVENT_STATE_EXECUTING_ON_DEVICE:
    // Fall through
  case EVENT_STATE_DONE_EXECUTING_ON_DEVICE:
    return CL_RUNNING;
  case EVENT_STATE_DONE:
    return CL_COMPLETE;
  }

  assert(false && "Invalid event state");
  return CL_COMPLETE;
}
