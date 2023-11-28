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

#include "build_event.h"
#include "Context.h"
#include "cl_sys_info.h"
#include <assert.h>

using namespace Intel::OpenCL::Framework;

BuildEvent::BuildEvent(_cl_context_int *context) : OclEvent(context) {
  m_pContext = (Context *)context->object;
  SetEventState(EVENT_STATE_HAS_DEPENDENCIES);
  m_returnCode = 0xdead;
}

void BuildEvent::SetComplete(cl_int returnCode) {
  m_returnCode = returnCode;
  SetEventState(EVENT_STATE_DONE);
}
