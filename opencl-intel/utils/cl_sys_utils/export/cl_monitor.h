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

#ifndef _CL_MONITOR_H
#define _CL_MONITOR_H

#include "cl_stopwatch.h"
#include "cl_sys_defines.h"
#include "cl_types.h"

// #define __PERF_MONITOR__
#define MAX_SAMPLE_NAME 64
#define MAX_SAMPLES_COUNT 100

namespace Intel {
namespace OpenCL {
namespace Utils {

/*******************************************************************************
 * Class name:  Sample
 *
 * Inherit:
 * Description:  represents a sampling point object
 ******************************************************************************/
class Sample {
public:
  Sample() {}  // Constructor
  ~Sample() {} // Destructor

  // Start the sampling
  void Start(char *psName) {
    m_StopWatch.Start();
    STRCPY_S(m_sName, MAX_SAMPLE_NAME, psName);
  }

  // Stop the sampling
  unsigned long long Stop() {
    m_ullTime = m_StopWatch.Stop();
    return m_ullTime;
  }

  // get the name of the sampling object
  const char *GetName() const { return m_sName; }

  // get the total time from the start of the sampling to the end
  unsigned long long GetTime() const { return m_ullTime; }

protected:
  char m_sName[MAX_SAMPLE_NAME];
  unsigned long long m_ullTime;

  StopWatch m_StopWatch;
};

/*******************************************************************************
 * Class name:  PerformanceMeter
 *
 * Inherit:
 * Description:  represents a sampling PerformanceMeter object
 ******************************************************************************/
class PerformanceMeter {
public:
  static int Start(char *psSampleName) {
    if (MAX_SAMPLES_COUNT == g_uiSamplesCount) {
      return MAX_SAMPLES_COUNT;
    }
    g_pSamples[g_uiSamplesCount].Start(psSampleName);
    g_uiSamplesCount++;
    return g_uiSamplesCount - 1;
  }

  static void Stop(int iSampleId) {
    if (MAX_SAMPLES_COUNT == iSampleId) {
      return;
    }
    g_pSamples[iSampleId].Stop();
  }

  static Sample *GetSamples(unsigned int *puiSamplesCount) {
    *puiSamplesCount = g_uiSamplesCount;
    return g_pSamples;
  }

private:
  static unsigned int g_uiSamplesCount;
  static Sample g_pSamples[MAX_SAMPLES_COUNT];
};

} // namespace Utils
} // namespace OpenCL
} // namespace Intel

#ifdef __PERF_MONITOR__

#define __PERF_INIT int iPerf = 0;
#define __PERF_START(NAME)                                                     \
  iPerf = Intel::OpenCL::Utils::PerformanceMeter::Start(NAME);
#define __PERF_STOP Intel::OpenCL::Utils::PerformanceMeter::Stop(iPerf);
#define __PERF_MONITOR_INIT                                                    \
  unsigned int PerformanceMeter::g_uiSamplesCount = 0;                         \
  Sample PerformanceMeter::g_pSamples[100];
#define __PERF_PRINT_SUMMARY                                                   \
  unsigned int uiSamplesCount = 0;                                             \
  Sample *pSamples = PerformanceMeter::GetSamples(&uiSamplesCount);            \
  for (unsigned int ui = 0; ui < uiSamplesCount; ++ui) {                       \
    printf("%s,%llu\n", pSamples[ui].GetName(), pSamples[ui].GetTime());       \
  }

#else

#define __PERF_INIT
#define __PERF_START(NAME)
#define __PERF_STOP
#define __PERF_MONITOR_INIT
#define __PERF_PRINT_SUMMARY

#endif

#define cl_return                                                              \
  __PERF_STOP;                                                                 \
  return

#define cl_start                                                               \
  __PERF_INIT;                                                                 \
  __PERF_START(WIDEN(__FUNCTION__));

#define cl_monitor_init __PERF_MONITOR_INIT

#define cl_monitor_summary __PERF_PRINT_SUMMARY

#endif
