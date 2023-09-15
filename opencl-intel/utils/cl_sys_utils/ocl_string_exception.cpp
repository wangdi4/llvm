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

#include "ocl_string_exception.h"

using namespace std;
using namespace Intel::OpenCL::Utils;

ocl_string_exception::ocl_string_exception(const char *exception) throw()
    : m_exception(exception) {}

ocl_string_exception::ocl_string_exception(const std::string &exception) throw()
    : m_exception(exception) {}

const char *ocl_string_exception::what() const throw() {
  return m_exception.c_str();
}
