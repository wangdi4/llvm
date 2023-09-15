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

#include "memobj_event.h"
#include "cl_sys_info.h"
#include <assert.h>

using namespace Intel::OpenCL::Framework;

MemoryObjectEvent::MemoryObjectEvent(
    IOCLDevMemoryObject **ppDevMemObj,
    const SharedPtr<MemoryObject> &pMemObject,
    const SharedPtr<FissionableDevice> &pDevice)
    : OclEvent(pMemObject->GetParentHandle()), m_ppDevMemObj(ppDevMemObj),
      m_pMemObject(pMemObject), m_pDevice(pDevice) {
  SetEventState(EVENT_STATE_HAS_DEPENDENCIES);
}

MemoryObjectEvent::~MemoryObjectEvent() {}

cl_err_code MemoryObjectEvent::ObservedEventStateChanged(
    const SharedPtr<OclEvent> & /*pEvent*/, cl_int returnCode) {
  if (returnCode == CL_SUCCESS) {
    returnCode = m_pMemObject->UpdateDeviceDescriptor(m_pDevice, m_ppDevMemObj);
  }
  OclEvent::NotifyComplete(returnCode);
  return CL_SUCCESS;
}
