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
#include "cl_objects_map.h"
#include "event_callback.h"
#include <list>

namespace Intel {
namespace OpenCL {
namespace Framework {
// Forward declarations
template <class HandleType, class ObjectType> class OCLObjectsMap;
class OclEvent;
class BuildEvent;
class QueueEvent;
class UserEvent;
class IOclCommandQueueBase;

/*******************************************************************************
 * Class name:    EventsManager
 *
 * Description:
 *      TODO
 *
 ******************************************************************************/
class EventsManager {
public:
  EventsManager();
  virtual ~EventsManager();

  EventsManager(const EventsManager &) = delete;
  EventsManager &operator=(const EventsManager &) = delete;

  // OpenCL API Event related functions
  cl_err_code RetainEvent(cl_event event);
  cl_err_code ReleaseEvent(cl_event event);
  cl_err_code GetEventInfo(cl_event event, cl_int iParamName,
                           size_t szParamValueSize, void *pParamValue,
                           size_t *pszParamValueSizeRet);
  cl_err_code GetEventProfilingInfo(cl_event event,
                                    cl_profiling_info clParamName,
                                    size_t szParamValueSize, void *pParamValue,
                                    size_t *pszParamValueSizeRet);
  cl_err_code WaitForEvents(cl_uint uiNumEvents, const cl_event *eventList,
                            bool skipInvalidEvents = false);
  bool IsValidEventList(cl_uint uiNumEvents, const cl_event *eventList,
                        std::vector<SharedPtr<OclEvent>> *pvOclEvents = NULL);

  // Event handling functions
  template <class EventClass>
  SharedPtr<EventClass> CreateEventClass(_cl_context_int *context) {
    SharedPtr<EventClass> pEvent = EventClass::Allocate(context);
    m_mapEvents.AddObject(pEvent);
    return pEvent;
  }

  template <class EventClass>
  SharedPtr<EventClass> GetEventClass(cl_event clEvent) {
    SharedPtr<OCLObject<_cl_event_int>> pOclObject =
        m_mapEvents.GetOCLObject((_cl_event_int *)clEvent);
    if (0 == pOclObject) {
      return NULL;
    }
    return pOclObject.DynamicCast<EventClass>();
  }

  // return list of existing events. Assumes DisableNewEvents() was called
  // before otherwise races may happen
  template <class EventClass>
  void GetAllEventClass(std::list<SharedPtr<EventClass>> &out_list) {
    GetAllEventClassFunction<EventClass> filter(out_list);
    m_mapEvents.ForEach(filter);
  }

  void RegisterQueueEvent(const SharedPtr<QueueEvent> &pEvent,
                          cl_event *pEventHndl);
  cl_err_code RegisterEvents(const SharedPtr<OclEvent> &pEvent,
                             cl_uint uiNumEvents, const cl_event *eventList,
                             bool bRemoveEvents = false, cl_int queueId = 0);

  void ReleaseAllEvents(bool bTerminate);
  void DisableNewEvents() { m_mapEvents.DisableAdding(); };
  void EnableNewEvents() { m_mapEvents.EnableAdding(); };
  void SetPreserveUserHandles() { m_mapEvents.SetPreserveUserHandles(); }

  cl_err_code SetEventCallBack(cl_event evt, cl_int execType,
                               eventCallbackFn fn, void *pUserData);

private:
  OCLObjectsMap<_cl_event_int>
      m_mapEvents; // Holds the set of clEvents that exist.

  // Private handling functions
  bool GetEventsFromList(cl_uint uiNumEvents, const cl_event *eventList,
                         std::vector<SharedPtr<OclEvent>> *pvOclEvents,
                         bool skipInvalidEvents);

  template <class EventClass> class GetAllEventClassFunction {
  public:
    GetAllEventClassFunction(std::list<SharedPtr<EventClass>> &out_list)
        : m_out_list(out_list){};

    bool operator()(const SharedPtr<OCLObject<_cl_event_int>> &obj) {
      EventClass *pEventClass = dynamic_cast<EventClass *>(obj.GetPtr());
      if (NULL != pEventClass) {
        m_out_list.push_back(pEventClass);
      }
      return true;
    }

  private:
    std::list<SharedPtr<EventClass>> &m_out_list;
  };
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
