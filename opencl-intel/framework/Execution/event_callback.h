// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "cl_types.h"
#include "event_observer.h"

namespace Intel {
namespace OpenCL {
namespace Framework {

typedef void(CL_CALLBACK *eventCallbackFn)(cl_event, cl_int, void *);

class OclEvent;

class EventCallback : public IEventObserver {
public:
  EventCallback(eventCallbackFn callback, void *pUserData,
                const cl_int expectedExecState);
  PREPARE_SHARED_PTR(EventCallback)

  static SharedPtr<EventCallback> Allocate(eventCallbackFn callback,
                                           void *pUserData,
                                           const cl_int expectedExecState) {
    return SharedPtr<EventCallback>(
        new EventCallback(callback, pUserData, expectedExecState));
  }

  virtual ~EventCallback() {}

  // IEventObserver
  cl_err_code ObservedEventStateChanged(const SharedPtr<OclEvent> &pEvent,
                                        cl_int returnCode) override;
  cl_int GetExpectedExecState() const override {
    return m_eventCallbackExecState;
  }

private:
  eventCallbackFn m_callback;
  void *m_pUserData;
  const cl_int m_eventCallbackExecState;
};
} // namespace Framework
} // namespace OpenCL
} // namespace Intel
