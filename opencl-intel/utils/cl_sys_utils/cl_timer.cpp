// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

#include "cl_timer.h"
#include <cassert>
#include <cstdio>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

namespace Intel {
namespace OpenCL {
namespace Utils {

Timer::~Timer() {
#ifdef PRINT_TIME_IN_DESTRUCTOR
  printf("Timer %s: %ld\n", name.c_str(), total_usecs);
#endif
}

unsigned long long Timer::GetCurrentTicks() {
#ifdef _WIN32
  LARGE_INTEGER time0;
  const BOOL ret = QueryPerformanceCounter(&time0);
  (void)ret;
  assert(0 != ret);
  return time0.QuadPart;
#else
  // todo - implement for Linux
  return 0;
#endif
}

unsigned long long Timer::GetFrequency() {
#ifdef _WIN32
  static LARGE_INTEGER s_freq = {{0, 0}};
  if (0 == s_freq.QuadPart) {
    QueryPerformanceFrequency(&s_freq);
    assert(0 != s_freq.QuadPart);
  }

  return s_freq.QuadPart;
#else
  // todo - implement for Linux
  return 0;
#endif
}

unsigned long long Timer::GetTimeInUsecs() {
#ifdef _WIN32
  LARGE_INTEGER time;
  const BOOL ret = QueryPerformanceCounter(&time);
  (void)ret;
  assert(0 != ret);
  return time.QuadPart * 1000000 / GetFrequency();
#else
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return tv.tv_sec * 1000000 + tv.tv_usec;
#endif
}

void Timer::Start() {
#ifdef ENABLE_GLOBALLY
  if (!sm_bGloballyEnabled)
    return;
#endif
#ifdef _WIN32
  m_timeStart = GetTimeInUsecs();
#else
  gettimeofday(&m_TvStart, nullptr);
#endif
}

void Timer::Stop() {
#ifdef ENABLE_GLOBALLY
  if (!sm_bGloballyEnabled)
    return;
#endif
#ifdef _WIN32
  m_ulTotalUsecs += GetTimeInUsecs() - m_timeStart;
#else
  m_ulTotalUsecs +=
      GetTimeInUsecs() - (m_TvStart.tv_sec * 1000000 + m_TvStart.tv_usec);
#endif
}

} // namespace Utils
} // namespace OpenCL
} // namespace Intel
