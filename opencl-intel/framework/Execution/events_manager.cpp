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

#include "events_manager.h"
#include "build_event.h"
#include "cl_objects_map.h"
#include "cl_shared_ptr.hpp"
#include "cl_utils.h"
#include "command_queue.h"
#include "enqueue_commands.h"
#include "ocl_event.h"
#include "queue_event.h"
#include "user_event.h"
#include <cassert>

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

/******************************************************************
 *
 ******************************************************************/
EventsManager::EventsManager() {}

/******************************************************************
 *
 ******************************************************************/
EventsManager::~EventsManager() { m_mapEvents.ReleaseAllObjects(false); }

/******************************************************************
 *
 ******************************************************************/
cl_err_code EventsManager::RetainEvent(cl_event clEvent) {
  cl_err_code res = CL_SUCCESS;

  SharedPtr<OCLObject<_cl_event_int>> pOclObject =
      m_mapEvents.GetOCLObject((_cl_event_int *)clEvent);
  if (pOclObject) {
    pOclObject->Retain();
  }
  return res;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code EventsManager::ReleaseEvent(cl_event clEvent) {
  return m_mapEvents.ReleaseObject((_cl_event_int *)clEvent);
}

void EventsManager::ReleaseAllEvents(bool bTerminate) {
  return m_mapEvents.ReleaseAllObjects(bTerminate);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code EventsManager::GetEventInfo(cl_event clEvent, cl_int iParamName,
                                        size_t szParamValueSize,
                                        void *paramValue,
                                        size_t *szParamValueSizeRet) {
  cl_err_code res = CL_SUCCESS;
  SharedPtr<OCLObject<_cl_event_int>> pOclObject =
      m_mapEvents.GetOCLObject((_cl_event_int *)clEvent);
  if (pOclObject) {
    res = pOclObject->GetInfo(iParamName, szParamValueSize, paramValue,
                              szParamValueSizeRet);
  } else {
    res = CL_INVALID_EVENT;
  }
  return res;
}
/******************************************************************
 *
 ******************************************************************/
cl_err_code EventsManager::GetEventProfilingInfo(cl_event clEvent,
                                                 cl_profiling_info clParamName,
                                                 size_t szParamValueSize,
                                                 void *pParamValue,
                                                 size_t *pszParamValueSizeRet) {
  cl_err_code res = CL_SUCCESS;
  SharedPtr<OCLObject<_cl_event_int>> pOclObject =
      m_mapEvents.GetOCLObject((_cl_event_int *)clEvent);
  if (pOclObject) {
    SharedPtr<QueueEvent> pEvent = pOclObject.DynamicCast<QueueEvent>();
    if (nullptr == pEvent.GetPtr()) // User events have no profiling info
    {
      return CL_PROFILING_INFO_NOT_AVAILABLE;
    }
    res = pEvent->GetProfilingInfo(clParamName, szParamValueSize, pParamValue,
                                   pszParamValueSizeRet);
  } else {
    res = CL_INVALID_EVENT;
  }
  return res;
}

/******************************************************************
 * This function implements the API clWaitForEvents function
 * The host thread waits for commands identified by event objects in event_list
 *to complete. The events specified in event_list act as synchronization points.
 *
 ******************************************************************/
cl_err_code EventsManager::WaitForEvents(cl_uint uiNumEvents,
                                         const cl_event *eventList,
                                         bool skipInvalidEvents) {
  if (0 == uiNumEvents || NULL == eventList)
    return CL_INVALID_EVENT_WAIT_LIST;

  // First validate that all ids in the event list exists
  std::vector<SharedPtr<OclEvent>> vOclEvents;

  if (!GetEventsFromList(uiNumEvents, eventList, &vOclEvents,
                         skipInvalidEvents))
    return CL_INVALID_EVENT_WAIT_LIST;

  // Return CL_SUCCESS if allowed to skip invalid events and all events in the
  // list are invalid.
  if (skipInvalidEvents && vOclEvents.empty())
    return CL_SUCCESS;

  cl_context eventContext;
  // Ensure all events are in the same context
  eventContext = vOclEvents[0]->GetParentHandle();
  for (cl_uint ui = 1; ui < vOclEvents.size(); ++ui) {
    if (eventContext != vOclEvents[ui]->GetParentHandle()) {
      return CL_INVALID_CONTEXT;
    }
  }

  // Wait on all events. Order doesn't matter since you always bonded to the
  // longest event. On the first stage, try to wait for events (commands) that
  // we can join the arena. For RuntimeCommand (except for MarkerCommand) or for
  // such command that we can't join execution, call Wait() method of the event
  // and wait for it's completion. OclEvent wait on event that is done do
  // nothing
  cl_err_code err = CL_SUCCESS;
  bool bWaitForEventSuccess = false;
  std::list<SharedPtr<OclEvent>> vOclEventsList(vOclEvents.begin(),
                                                vOclEvents.end());
  auto evtIt = vOclEventsList.begin();
  while (!vOclEventsList.empty()) {
    if (vOclEventsList.end() == evtIt) {
      // When one loop fished and no WaitForEvent were executed successfully
      // Need to start using explicit Wait()
      if (!bWaitForEventSuccess) {
        break;
      }

      evtIt = vOclEventsList.begin();
      bWaitForEventSuccess = false;
    }
    // Execute queue until associated command is completed

    // Don't try join master thread for user events,
    // Don't try join master thread for Runtime Commands except for marker
    // command, Or if WaitForCompletion() fails, Move event to the Explicit Wait
    // list and skip to next event
    //
    // For marker command, it is safe to join master thread because
    // clEnqueueMarker already sets marker event dependencies properly in
    // case of out of order queue and submits marker command which is ready
    // to execute in case of in order queue.
    SharedPtr<QueueEvent> pQueueEvent = evtIt->DynamicCast<QueueEvent>();
    if ((NULL != pQueueEvent.GetPtr()) &&
        (pQueueEvent->GetCommand()->GetExecutionType() ==
             DEVICE_EXECUTION_TYPE ||
         NULL != dynamic_cast<MarkerCommand *>(pQueueEvent->GetCommand()))) {
      const SharedPtr<IOclCommandQueueBase> pQueueEventQueue =
          pQueueEvent->GetEventQueue();
      if ((NULL != pQueueEventQueue.GetPtr()) &&
          !CL_FAILED(pQueueEventQueue->WaitForCompletion(
              pQueueEvent))) // CL_SUCCEDDED() != (!CL_FAILED())
      {
        bWaitForEventSuccess = true;

        // At this stage the event is completed
        assert(((*evtIt)->GetEventExecState() == CL_COMPLETE) &&
               "Event expected to be in COMPLETE state");
        // Now check even error code for failure
        if ((*evtIt)->GetReturnCode() < 0) {
          err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
        }

        evtIt = vOclEventsList.erase(evtIt);
        continue;
      }
    }
    ++evtIt;
  }

  // For rest of the events need to explicitly wait for each one, don't spent
  // CPU cycles in the loop
  for (auto evtIt = vOclEventsList.begin(); evtIt != vOclEventsList.end();
       evtIt++) {
    (*evtIt)->Wait();
    // Now check even error code for failure
    if ((*evtIt)->GetReturnCode() < 0) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
    }
  }

  return err;
}
/******************************************************************
 * This function gets an event list and returns true if it is valid
 *
 ******************************************************************/
bool EventsManager::IsValidEventList(
    cl_uint uiNumEvents, const cl_event *eventList,
    std::vector<SharedPtr<OclEvent>> *pvOclEvents) {
  if (((NULL == eventList) && (0 != uiNumEvents)) ||
      ((NULL != eventList) && (0 == uiNumEvents)))
    return false;
  if (0 != uiNumEvents) {
    if (!GetEventsFromList(uiNumEvents, eventList, pvOclEvents,
                           /*skipInvalidEvents*/ false))
      return false;
  }
  return true;
}
/******************************************************************
 * This function gets an handle to event that represents command
 * that is dependent on the command that are attached with the list
 * of events in the event list.
 * If bRemoveEvents is true, events that were allocated on queueId a
 * are removed from the list
 *
 ******************************************************************/
cl_err_code EventsManager::RegisterEvents(const SharedPtr<OclEvent> &pEvent,
                                          cl_uint uiNumEvents,
                                          const cl_event *eventList,
                                          bool bRemoveEvents, cl_int queueId) {
  cl_start;
  // Check input parameters and validate that all ids in the event list exists
  std::vector<SharedPtr<OclEvent>> vOclEvents;
  if (NULL == pEvent.GetPtr() ||
      !IsValidEventList(uiNumEvents, eventList, &vOclEvents)) {
    return CL_INVALID_EVENT_WAIT_LIST;
  }

  // If 0, no event list
  if (0 != uiNumEvents) {
    cl_uint ui;
    // Next, check that events has the same context as the queued context
    cl_context queueContext = pEvent->GetParentHandle();
    for (ui = 0; ui < uiNumEvents; ui++) {
      const SharedPtr<OclEvent> &pOclEvent = vOclEvents[ui];
      if (queueContext != pOclEvent->GetParentHandle()) {
        // Error
        return CL_INVALID_CONTEXT;
      }
    }

    // Register input event in the entire eventList
    // If bRemoveEvents is true, events that were allocated on queueId will not
    // be registered
    if (bRemoveEvents) {
      for (ui = 0; ui < uiNumEvents; ui++) {
        SharedPtr<QueueEvent> pDependOnEvent =
            vOclEvents[ui].DynamicCast<QueueEvent>();
        if (NULL != pDependOnEvent.GetPtr()) {
          if (queueId == pDependOnEvent->GetEventQueueId()) {
            // Do not register
            vOclEvents[ui] = NULL;
            continue;
          }
        }
      }
    }
    pEvent->AddDependentOnMulti(uiNumEvents, &vOclEvents[0]);
  }
  cl_return CL_SUCCESS;
}

/******************************************************************
 * This function returns list of OclEvent objects corresponding
 * to the input parameters of eventId.
 * On success a list is returned that need to be free by the caller.
 *
 ******************************************************************/
bool EventsManager::GetEventsFromList(
    cl_uint uiNumEvents, const cl_event *eventList,
    std::vector<SharedPtr<OclEvent>> *pvOclEvents, bool skipInvalidEvents) {
  if (0 == uiNumEvents)
    return false;

  for (cl_uint ui = 0; ui < uiNumEvents; ui++) {
    SharedPtr<OclEvent> pEvent =
        m_mapEvents.GetOCLObject((_cl_event_int *)eventList[ui])
            .DynamicCast<OclEvent>();

    if (NULL == pEvent.GetPtr()) {
      if (skipInvalidEvents)
        continue;
      else
        return false;
    }
    if (NULL != pvOclEvents)
      pvOclEvents->push_back(pEvent);
  }
  return true;
}

/******************************************************************
 * This function creates event object. The Queue event handle that can be used
 *by the user is assigned into the eventHndl. The function returns pointer to a
 *QueueEvent that is attached with the related Command object.
 ******************************************************************/
void EventsManager::RegisterQueueEvent(const SharedPtr<QueueEvent> &pEvent,
                                       cl_event *pEventHndl) {
  assert(pEvent);
  m_mapEvents.AddObject(pEvent);
  if (pEventHndl) {
    *pEventHndl = pEvent->GetHandle();
  }
}

/******************************************************************
 * This function registers the callback fn on the event evt when its status
 *changes to execType
 ******************************************************************/
cl_err_code
EventsManager::SetEventCallBack(cl_event evt, cl_int execType,
                                Intel::OpenCL::Framework::eventCallbackFn fn,
                                void *pUserData) {
  SharedPtr<OclEvent> pEvent = GetEventClass<OclEvent>(evt);
  if (NULL == pEvent.GetPtr()) {
    return CL_INVALID_EVENT;
  }
  if (!fn) {
    return CL_INVALID_VALUE;
  }
  if (execType != CL_SUBMITTED && execType != CL_RUNNING &&
      execType != CL_COMPLETE) {
    return CL_INVALID_VALUE;
  }
  SharedPtr<EventCallback> pNewCallback =
      EventCallback::Allocate(fn, pUserData, execType);
  if (NULL == pNewCallback.GetPtr()) {
    return CL_OUT_OF_HOST_MEMORY;
  }
  pEvent->AddObserver(SharedPtr<IEventObserver>(pNewCallback));
  return CL_SUCCESS;
}
