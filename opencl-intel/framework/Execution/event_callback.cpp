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

#include "event_callback.h"
#include "cl_user_logger.h"
#include "ocl_config.h"
#include "ocl_event.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

EventCallback::EventCallback(eventCallbackFn callback, void *pUserData,
                             const cl_int expectedExecState)
    : m_callback(callback), m_pUserData(pUserData),
      m_eventCallbackExecState(expectedExecState) {}

cl_err_code
EventCallback::ObservedEventStateChanged(const SharedPtr<OclEvent> &pEvent,
                                         cl_int returnCode) {
  assert(pEvent);
  cl_int retCode = returnCode;
  if (CL_COMPLETE != pEvent->GetEventExecState()) {
    retCode = GetExpectedExecState();
  }
  if (FrameworkUserLogger::GetInstance()->IsApiLoggingEnabled()) {
    std::stringstream stream;
    stream << "EventCallback(" << pEvent->GetHandle() << ", " << m_pUserData
           << ")" << std::endl;
    FrameworkUserLogger::GetInstance()->PrintString(stream.str());
  }
  m_callback(pEvent->GetHandle(), retCode, m_pUserData);
  return CL_SUCCESS;
}
