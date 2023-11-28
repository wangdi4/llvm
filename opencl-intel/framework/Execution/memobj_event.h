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

#include "MemoryObject.h"
#include "cl_device_api.h"
#include "cl_object.h"
#include "cl_types.h"
#include "ocl_event.h"

namespace Intel {
namespace OpenCL {
namespace Framework {

class MemoryObjectEvent : public OclEvent {
public:
  PREPARE_SHARED_PTR(MemoryObjectEvent)

  MemoryObjectEvent(const MemoryObjectEvent &) = delete;
  MemoryObjectEvent &operator=(const MemoryObjectEvent &) = delete;

  static SharedPtr<MemoryObjectEvent>
  Allocate(IOCLDevMemoryObject **ppDevMemObj,
           SharedPtr<MemoryObject> pMemObject,
           SharedPtr<FissionableDevice> pDevice) {
    return new MemoryObjectEvent(ppDevMemObj, pMemObject, pDevice);
  }

  // Get the context to which the event belongs.
  // Get the return code of the command associated with the event.
  cl_int GetReturnCode() const override { return 0; }
  cl_err_code GetInfo(cl_int /*iParamName*/, size_t /*szParamValueSize*/,
                      void * /*pParamValue*/,
                      size_t * /*pszParamValueSizeRet*/) const override {
    return CL_INVALID_OPERATION;
  }

  // IEventObserver
  cl_err_code ObservedEventStateChanged(const SharedPtr<OclEvent> &pEvent,
                                        cl_int returnCode) override;

protected:
  MemoryObjectEvent(IOCLDevMemoryObject **ppDevMemObj,
                    const SharedPtr<MemoryObject> &pMemObject,
                    const SharedPtr<FissionableDevice> &pDevice);

  virtual ~MemoryObjectEvent();

  IOCLDevMemoryObject **m_ppDevMemObj;
  SharedPtr<MemoryObject> m_pMemObject;
  SharedPtr<FissionableDevice> m_pDevice;
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
