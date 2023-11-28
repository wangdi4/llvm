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

#include <atomic>

namespace Intel {
namespace OpenCL {
namespace Utils {
enum ProcessState : unsigned int {
  Working = 0,     // still no atexit() happened
  ExitStarted = 1, // global atexit() processing started
  ExitDone = 2,    // global atexit() processing finished
};

bool IsShuttingDown(void);

void UpdateShutdownMode(ProcessState);

} // namespace Utils
} // namespace OpenCL
} // namespace Intel
