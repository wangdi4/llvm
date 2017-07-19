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

#include <cstdio>
#include <cassert>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
#include "cl_timer.h"

namespace Intel { namespace OpenCL { namespace Utils {

Timer::~Timer()
{
#ifdef PRINT_TIME_IN_DESTRUCTOR
    printf("Timer %s: %ld\n", name.c_str(), total_usecs);
#endif
}

unsigned long long Timer::GetCurrentTicks()
{
#ifdef _WIN32
    LARGE_INTEGER time0;
    const BOOL ret = QueryPerformanceCounter(&time0);
    assert(0 != ret);
    return time0.QuadPart ;
#else
    // todo - implement for Linux
	return 0;
#endif
}


unsigned long long Timer::GetFrequency()
{
#ifdef _WIN32
    static LARGE_INTEGER s_freq = { 0 };
    if (0 == s_freq.QuadPart)
    {
        QueryPerformanceFrequency(&s_freq);
        assert(0 != s_freq.QuadPart);
    }

    return s_freq.QuadPart;
#else
     // todo - implement for Linux
	return 0;
#endif
}

unsigned long long Timer::GetTimeInUsecs()
{
#ifdef _WIN32
    LARGE_INTEGER time;
    const BOOL ret = QueryPerformanceCounter(&time);
    assert(0 != ret);
    return time.QuadPart * 1000000 / GetFrequency();
#else
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000000 + tv.tv_usec;
#endif
}


void Timer::Start()
{
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

void Timer::Stop()
{ 
#ifdef ENABLE_GLOBALLY
    if (!sm_bGloballyEnabled)
        return;
#endif
#ifdef _WIN32
    m_ulTotalUsecs += GetTimeInUsecs() - m_timeStart;
#else
    m_ulTotalUsecs += GetTimeInUsecs() - (m_TvStart.tv_sec * 1000000 + m_TvStart.tv_usec);
#endif
}

}}}
