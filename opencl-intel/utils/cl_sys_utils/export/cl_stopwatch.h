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

#include "cl_types.h"

#include <string>

namespace Intel {
namespace OpenCL {
namespace Utils {

class StopWatch {
public:
  StopWatch();

  ~StopWatch();

  // start the timer
  void Start();

  // stop the timer - return duration from last reset
  unsigned long long Stop();

  // reset the timer
  unsigned long long Reset();

  // get the current time
  unsigned long long GetTime() const;

private:
  unsigned long long m_ullTime;
  unsigned int m_uiCounter;
};

} // namespace Utils
} // namespace OpenCL
} // namespace Intel
