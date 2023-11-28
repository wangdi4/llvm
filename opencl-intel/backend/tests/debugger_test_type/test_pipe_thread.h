// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#ifndef TEST_PIPE_UTILS_H
#define TEST_PIPE_UTILS_H

#ifdef _WIN32
#include "cl_thread.h"
#include <cstdlib>

class NamedPipeThread : public Intel::OpenCL::Utils::OclThread {
public:
  HANDLE pipe;
  std::string portNumber;

  NamedPipeThread();

protected:
  virtual RETURN_TYPE_ENTRY_POINT Run() override;
};
#endif // _WIN32

#endif // TEST_PIPE_UTILS_H
