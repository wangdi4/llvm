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

#pragma once

#include "cl_framework.h"
#include "cl_object.h"
#include "event_observer.h"
#include <atomic>
#include <list>

namespace Intel {
namespace OpenCL {
namespace Framework {

// Forward declarations
class Command;
class Context;
class IOclCommandQueueBase;

typedef enum {
  EVENT_STATE_CREATED,          // The command was just created.
  EVENT_STATE_HAS_DEPENDENCIES, // The command is dependent on at least one
                                // other event.
  EVENT_STATE_READY_TO_EXECUTE, // The command is ready to be executed - no
                                // dependencies.
  EVENT_STATE_ISSUED_TO_DEVICE, // The command was issued to the device, but is
                                // not being processed yet.
  EVENT_STATE_EXECUTING_ON_DEVICE, // The command is currently executing on the
                                   // device.
  EVENT_STATE_DONE_EXECUTING_ON_DEVICE, // The command itself is done executing
                                        // on the device, but offspring commands
                                        // may still be running
  EVENT_STATE_DONE // The command is done, and can be deleted.
} OclEventState;

#define OCL_EVENT_WAIT_SPIN 0
#define OCL_EVENT_WAIT_YIELD 1
#define OCL_EVENT_WAIT_OS_DEPENDENT 2

// #define OCL_EVENT_WAIT_STRATEGY OCL_EVENT_WAIT_SPIN
// #define OCL_EVENT_WAIT_STRATEGY OCL_EVENT_WAIT_YIELD
#ifdef _WIN32
#define OCL_EVENT_WAIT_STRATEGY OCL_EVENT_WAIT_OS_DEPENDENT
#else // For Linux, the CFS performs much better without active wait
#define OCL_EVENT_WAIT_STRATEGY OCL_EVENT_WAIT_OS_DEPENDENT
#endif

#if OCL_EVENT_WAIT_STRATEGY != OCL_EVENT_WAIT_OS_DEPENDENT
#error "Data race in OclEvent::SetEventState, waiters on queue event are "
"released before the queue reference is dropped."
#endif

    /*******************************************************************************
     * struct name:    SProfilingInfo
     *
     * Description:
     *      Holds the profiling information of the event
     *
     ******************************************************************************/
    struct SProfilingInfo {
  cl_ulong m_ulCommandQueued; // A 64-bit value that describes the current
                              // device time
  // counter in nanoseconds when the command identified by
  // event is enqueued in a command-queue by the host.

  cl_ulong m_ulCommandSubmit; // A 64-bit value that describes the current
                              // device time
  // counter in nanoseconds when the command identified by
  // event that has been enqueued is submitted by the host
  // to the device associated with the command queue.

  cl_ulong
      m_ulCommandStart; // A 64-bit value that describes the current device time
  // counter in nanoseconds when the command identified by
  // event starts execution on the device

  cl_ulong
      m_ulCommandEnd; // A 64-bit value that describes the current device time
  // counter in nanoseconds when the command identified by
  // event has finished execution on the device

  cl_ulong m_ulCommandComplete; // A 64-bit value that describes the current
                                // device time
  // counter in nanoseconds when the command identified by
  // event and any child commands enqueued by this command on the device have
  // finished execution.
};

/*******************************************************************************
 * Class name:    OclEvent
 *
 * Description: OclEvent controls the dependencies between execution commands.
 *              The queue implementation attaches an event to each command. The
 *              event holds the execution state of the command, which is one of
 *              the following:
 *                - EVENT_STATE_HAS_DEPENDENCIES: The command is dependent on 1
 *                                                or more other commands
 *                - EVENT_STATE_READY_TO_EXECUTE: The command is ready to be
 *                                                executed - no deps.
 *                - EVENT_STATE_ISSUED_TO_DEVICE: The command is issued to the
 *                                                device, but is not processed
 *                                                yet.
 *                - EVENT_STATE_EXECUTING_ON_DEVICE: The command is been
 *                                                   executed on the device.
 *                - EVENT_STATE_DONE: Command is done and ready for deletion.
 ******************************************************************************/
class OclEvent : public OCLObject<_cl_event_int>, public IEventObserver {
public:
  OclEvent(_cl_context_int *context);
  OclEvent(const OclEvent &) = delete;
  OclEvent &operator=(const OclEvent &) = delete;

  PREPARE_SHARED_PTR(OclEvent)

  /**
   * Add an event that will observe myself, depends on my state.
   * @param observer
   */
  void AddObserver(const SharedPtr<IEventObserver> &pObserver);

  /**
   * Add events to observe, to depend on their state.
   * Sugaring over AddDependentOnMulti.
   * @param pDependsOnEvent
   */
  void AddDependentOn(const SharedPtr<OclEvent> &pDependsOnEvent);

  /**
   * Add events to observe, to depend on their state.
   * @param count
   * @param pDependencyList
   */
  void AddDependentOnMulti(unsigned int count,
                           SharedPtr<OclEvent> *pDependencyList);

  /**
   * Bogus dependency add.
   */
  void AddFloatingDependence() {
    ++m_numOfDependencies;
    SetEventState(EVENT_STATE_HAS_DEPENDENCIES);
  }
  /**
   * Bogus dependency remove.
   */
  void RemoveFloatingDependence() {
    if (0 == --m_numOfDependencies)
      DoneWithDependencies(NULL);
  }

  OclEventState SetEventState(
      const OclEventState newEventState); // returns the previous event state

  OclEventState GetEventState() const { return (OclEventState)m_eventState; }
  bool HasDependencies() const { return m_numOfDependencies > 0; }

  // Implementation IEventObserver
  virtual cl_err_code
  ObservedEventStateChanged(const SharedPtr<OclEvent> &pEvent,
                            cl_int returnCode = CL_SUCCESS) override;

  // Blocking function, returns after NotifyComplete is done
  virtual void Wait();

  cl_int GetEventExecState(const OclEventState state) const;
  cl_int GetEventExecState() const;

  virtual cl_int GetExpectedExecState() const override { return CL_COMPLETE; }

  // Get the return code of the command associated with the event.
  virtual cl_int GetReturnCode() const { return m_returnCode; }

  virtual void Cleanup(bool /*bTerminate*/ = false) override { delete this; }

protected:
  OclEvent(const std::string &typeName);
  virtual ~OclEvent();

  virtual void DoneWithDependencies(const SharedPtr<OclEvent> &pEvent);

  virtual void NotifyComplete(cl_int returnCode = CL_SUCCESS);
  virtual void NotifySubmitted();
  virtual void NotifyRunning();

  /**
   * Trigger all observers relevant to this state.
   * @param retCode exec state, or negative error code.
   */
  void NotifyObservers(const cl_int retCode);

  /**
   * Mark this event as complete, releasing all waiters on it
   */
  void MarkAsComplete();

  // Some implementations of possible methods for waiting
  void WaitSpin();
  void WaitYield();
  void WaitOSEvent();

  // typedef Intel::OpenCL::Utils::OclNaiveConcurrentQueue<IEventObserver*>
  // ObserversQ_t;
  typedef std::list<SharedPtr<IEventObserver>> ObserversList_t;
  ObserversList_t m_CompleteObserversList;
  ObserversList_t m_RunningObserversList;
  ObserversList_t m_SubmittedObserversList;
  std::mutex m_ObserversListGuard;

  std::atomic<long> m_numOfDependencies{0};

  cl_int m_returnCode;

#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
  Intel::OpenCL::Utils::AtomicPointer<Intel::OpenCL::Utils::OclOsDependentEvent>
      m_pCurrentEvent;
#else
  volatile bool m_complete;
#endif

private:
  volatile OclEventState m_eventState;
  SharedPtr<Context> m_pContext;

  /**
   * Make sure the list is empty, and all related dependencies are released.
   * @param list the list to expunge.
   */
  void ExpungeObservers(ObserversList_t &list);

  /**
   * Trigger all observers in a single exec state list, and pass retCode as
   * value. NOTE: unsafe, this function does not lock the mutex. It should ONLY
   * be called from thread-safe functions!
   * @param list
   * @param retCode can be a valid exec state, or a (negative) error code.
   */
  void NotifyObserversOfSingleExecState(ObserversList_t &list,
                                        const cl_int retCode);
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
