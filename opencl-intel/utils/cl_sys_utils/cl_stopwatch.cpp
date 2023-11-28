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

#include "cl_stopwatch.h"
#include "hw_utils.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include <assert.h>

using namespace Intel::OpenCL::Utils;

StopWatch::StopWatch() {
  assert(0 && "Deprecated code");
  m_ullTime = 0;
  m_uiCounter = 0;
}

StopWatch::~StopWatch() {}

void StopWatch::Start() { m_ullTime = RDTSC(); }

unsigned long long StopWatch::Stop() {
  unsigned long long ullPrevTime = m_ullTime;
  m_ullTime = RDTSC();
  m_uiCounter++;
  return (m_ullTime - ullPrevTime);
}

unsigned long long StopWatch::Reset() {
  unsigned long long ullPrevTime = m_ullTime;
  m_uiCounter = 0;
  return ullPrevTime;
}

unsigned long long StopWatch::GetTime() const {
  unsigned long long currTime;
  currTime = RDTSC();
  return currTime - m_ullTime;
}
