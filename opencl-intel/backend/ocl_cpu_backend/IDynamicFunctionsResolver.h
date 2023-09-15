// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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
#include <string>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/**
 * This inetrface represent the dynamically loaded builtin functions address
 * resolver unit mainly the SVML
 */
class IDynamicFunctionsResolver {
public:
  /**
   * @returns the function address of the required function; 0 in case function
   *  not known
   */
  virtual unsigned long long int
  GetFunctionAddress(const std::string &functionName) const = 0;
  virtual ~IDynamicFunctionsResolver() {}
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
