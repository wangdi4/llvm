// Copyright (c) 2006-2014 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#pragma once

#include <string>

namespace Intel { namespace OpenCL { namespace Utils {

/* Macros for using the timer to performance instrumentation:
   They declare a static timer in a function, which measures the total time spent in this function throughout the execution of the program. parameter x is the m_name of the timer.
   For using this you should define macro PRINT_TIME_IN_DESTRUCTOR, so that the time will be printed when the timer is destroyed.
 */
#define START(x)	static Timer timer##x(#x); timer##x.start();
#define STOP(x)		timer##x.stop();

/**
 * This class represents a timer that measures time in microseconds from the moment it is m_bStarted until it is stopped
 */
class Timer {
public:

#ifdef ENABLE_GLOBALLY  // an option for globally controlling the use of timers
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
    explicit Timer(const std::string& name = "") : m_ulTotalUsecs(0), m_name(name) { }

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
     * Reset the timer (this method has to be called before the timer can be used again)
     */
    void Reset() { m_ulTotalUsecs = 0; }

    /**
     * @return the number of microseconds elapsed since the timer was started until it was stopped
     */
    unsigned long long GetTotalUsecs() const { return m_ulTotalUsecs; }

private:

#ifdef ENABLE_GLOBALLY
    static bool sm_bGloballyEnabled;
#endif
    unsigned long long m_ulTotalUsecs;
    std::string m_name;
#ifdef _WIN32
    unsigned long long m_timeStart;
#else
    struct timeval m_TvStart;
#endif
};

}}}
