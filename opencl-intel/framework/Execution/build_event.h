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

#include "cl_object.h"
#include "cl_shared_ptr.hpp"
#include "cl_types.h"
#include "ocl_event.h"

namespace Intel {
namespace OpenCL {
namespace Framework {

class Context;

class BuildEvent : public OclEvent {
public:
  BuildEvent(_cl_context_int *context);

  SharedPtr<Context> GetContext() const { return m_pContext; }

  // OCLObject implementation
  cl_err_code GetInfo(cl_int /*iParamName*/, size_t /*szParamValueSize*/,
                      void * /*paramValue*/,
                      size_t * /*szParamValueSizeRet*/) const override {
    return CL_INVALID_VALUE;
  }

  virtual void SetComplete(cl_int returnCode);

protected:
  SharedPtr<Context> m_pContext;
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
