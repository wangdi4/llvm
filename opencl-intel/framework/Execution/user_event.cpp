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

#include "user_event.h"
#include "cl_sys_info.h"
#include "command_queue.h"
#include "enqueue_commands.h"
#include <assert.h>

using namespace Intel::OpenCL::Framework;

UserEvent::UserEvent(_cl_context_int *context) : OclEvent(context) {
  // User events start as CL_SUBMITTED
  SetEventState(EVENT_STATE_HAS_DEPENDENCIES);
  m_returnCode = 0xdead;
}

UserEvent::~UserEvent() {}

void UserEvent::SetComplete(cl_int returnCode) {
  m_returnCode = returnCode;
  SetEventState(EVENT_STATE_DONE);
}

cl_err_code UserEvent::GetInfo(cl_int iParamName, size_t szParamValueSize,
                               void *paramValue,
                               size_t *szParamValueSizeRet) const {
  cl_err_code res = CL_SUCCESS;
  const void *localParamValue = nullptr;
  size_t outputValueSize = 0;
  cl_int eventStatus;
  cl_command_type cmd_type;
  volatile cl_command_queue cmd_queue;
  cl_context evt_contex;

  switch (iParamName) {
  case CL_EVENT_COMMAND_QUEUE:
    cmd_queue = (cl_command_queue) nullptr;
    localParamValue = const_cast<_cl_command_queue **>(&cmd_queue);
    outputValueSize = sizeof(cl_command_queue);
    break;
  case CL_EVENT_CONTEXT:
    evt_contex = GetParentHandle();
    localParamValue = (void *)(&evt_contex);
    outputValueSize = sizeof(cl_context);
    break;
  case CL_EVENT_COMMAND_TYPE:
    cmd_type = (cl_command_type)CL_COMMAND_USER;
    localParamValue = &cmd_type;
    outputValueSize = sizeof(cl_command_type);
    break;
  case CL_EVENT_REFERENCE_COUNT:
    localParamValue = &m_uiRefCount;
    outputValueSize = sizeof(cl_uint);
    break;
  case CL_EVENT_COMMAND_EXECUTION_STATUS:
    if (EVENT_STATE_DONE == GetEventState()) {
      eventStatus = m_returnCode;
    } else {
      eventStatus = CL_SUBMITTED;
    }
    localParamValue = &eventStatus;
    outputValueSize = sizeof(cl_int);
    break;
  default:
    res = CL_INVALID_VALUE;
    break;
  }

  // check param_value_size
  if ((nullptr != paramValue) && (szParamValueSize < outputValueSize)) {
    res = CL_INVALID_VALUE;
  } else {
    if (nullptr != paramValue) {
      MEMCPY_S(paramValue, szParamValueSize, localParamValue, outputValueSize);
    }
    if (nullptr != szParamValueSizeRet) {
      *szParamValueSizeRet = outputValueSize;
    }
  }
  return res;
}

void UserEvent::NotifyInvisible() {
  if (CL_COMPLETE != GetEventExecState()) {
    SetComplete(CL_DEVICE_NOT_AVAILABLE);
  }
}
