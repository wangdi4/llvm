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

#pragma once

#include <string>

namespace Intel {
namespace OpenCL {
namespace Utils {

/* Macros for using the timer to performance instrumentation:
   They declare a static timer in a function, which measures the total time
   spent in this function throughout the execution of the program. parameter x
   is the m_name of the timer. For using this you should define macro
   PRINT_TIME_IN_DESTRUCTOR, so that the time will be printed when the timer is
   destroyed.
 */
#define START(x)                                                               \
  static Timer timer##x(#x);                                                   \
  timer##x.start();
#define STOP(x) timer##x.stop();

/**
 * This class represents a timer that measures time in microseconds from the
 * moment it is m_bStarted until it is stopped
 */
class Timer {
public:
  /**
   * @return the current time in microseconds
   */
  static unsigned long long GetTimeInUsecs();
  static unsigned long long GetCurrentTicks();
  static unsigned long long GetFrequency();

#ifdef ENABLE_GLOBALLY // an option for globally controlling the use of timers
  /**
   * Globally enable or disable the use of timers
   * @param enable whether to enable or disable the use of timers
   */
  static void enable(bool enable) { sm_bGloballyEnabled = enable; }

  /**
   * @return whether the timers are globally enabled
   */
  static bool is_enabled() { return sm_bGloballyEnabled; }
#endif

  /**
   * Constructor
   * @param name the TImer's name
   */
  explicit Timer(const std::string &name = "")
      : m_ulTotalUsecs(0), m_name(name) {}

  /**
   * Destructor
   */
  ~Timer();

  /**
   * Start the timer
   */
  void Start();

  /**
   * Stop the timer
   * @return the number of microseconds elapsed since the timer was started
   */
  void Stop();

  /**
   * Reset the timer (this method has to be called before the timer can be used
   * again)
   */
  void Reset() { m_ulTotalUsecs = 0; }

  /**
   * @return the number of microseconds elapsed since the timer was started
   * until it was stopped
   */
  unsigned long long GetTotalUsecs() const { return m_ulTotalUsecs; }

private:
#ifdef ENABLE_GLOBALLY
  static bool sm_bGloballyEnabled;
#endif
  unsigned long long m_ulTotalUsecs;
  std::string m_name;
#ifdef _WIN32
  unsigned long long m_timeStart = 0;
#else
  struct timeval m_TvStart = {0, 0};
#endif
};

} // namespace Utils
} // namespace OpenCL
} // namespace Intel
