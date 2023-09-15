// INTEL CONFIDENTIAL
//
// Copyright 2007 Intel Corporation.
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

#include <exception>
#include <string>

namespace Intel {
namespace OpenCL {
namespace Utils {

class ocl_string_exception : public std::exception {
public:
  ocl_string_exception(const char *exception) throw();
  ocl_string_exception(const std::string &exception) throw();
  virtual ~ocl_string_exception() throw() {}

  const char *what() const throw() override;

private:
  std::string m_exception;
};
} // namespace Utils
} // namespace OpenCL
} // namespace Intel
