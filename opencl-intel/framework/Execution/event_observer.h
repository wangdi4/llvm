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

#include "cl_shared_ptr.h"
#include "cl_types.h"

using Intel::OpenCL::Utils::SharedPtr;

namespace Intel {
namespace OpenCL {
namespace Framework {

// Forward declaration
class OclEvent;

/************************************************************************
 * Pure interface class
 *
 ************************************************************************/
class IEventObserver
    : virtual public Intel::OpenCL::Utils::ReferenceCountedObject {
public:
  PREPARE_SHARED_PTR(IEventObserver)

  virtual ~IEventObserver() {}

  virtual cl_err_code
  ObservedEventStateChanged(const SharedPtr<OclEvent> &pEvent,
                            cl_int returnCode) = 0;

  virtual cl_int GetExpectedExecState() const = 0;
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
