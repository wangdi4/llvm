// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __IMPLICIT_ARGUMENT_H__
#define __IMPLICIT_ARGUMENT_H__

#include "FunctionArgument.h"
#include "ImplicitArgProperties.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  class ImplicitArgument : public FunctionArgument {
  
  public:
    /// @brief Constructor
    /// @param pValue           Implict argument's value destination pointer
    /// @param implicitArgProps Implicit argument properties
    ImplicitArgument(char* pValue, const ImplicitArgProperties& implicitArgProps)
    : FunctionArgument(pValue, implicitArgProps.m_size, implicitArgProps.m_alignment) { }

    /// @brief Empty Constructor
    ImplicitArgument() : FunctionArgument(nullptr, 0, 0) {}
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __IMPLICIT_ARGUMENT_H__
