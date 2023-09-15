// INTEL CONFIDENTIAL
//
// Copyright 2007 Intel Corporation.
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

#include "CL/cl.h"
#include "cl_synch_objects.h"
#include <mutex>

namespace Intel {
namespace OpenCL {
namespace Utils {

// This class helps to manage mapping between pairs of events.
// It can be used to manage wrappers for events.
//
class EventsMapper {
public:
  virtual void addEventPair(cl_event userEvent, cl_event notifierEvent) = 0;
  virtual cl_event getUserEvent(cl_event notifierEvent) = 0;
  virtual cl_event getNotifierEvent(cl_event userEvent) = 0;
  virtual void delEvent(cl_event userEvent) = 0;
  virtual ~EventsMapper() {}
};

// The class below helps the notifier collection to wrap
// each OCL write/modify event (e.g. clEnqueueWriteBuffer),
// with a custom event.
//
class NotifierEventsMapper : public EventsMapper {
public:
  virtual void addEventPair(cl_event userEvent,
                            cl_event notifierEvent) override;
  virtual cl_event getUserEvent(cl_event notifierEvent) override;
  virtual cl_event getNotifierEvent(cl_event userEvent) override;
  virtual void delEvent(cl_event userEvent) override;

private:
  std::mutex m_lock;

  typedef cl_event UserEvent;
  typedef cl_event NotifierEvent;
  typedef std::map<UserEvent, NotifierEvent> EventsMap;

  EventsMap m_eventsMap;
};

} // namespace Utils
} // namespace OpenCL
} // namespace Intel
