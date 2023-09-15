// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#include "Performance.h"
#include "SATestException.h"
#include <algorithm>
#include <assert.h>
#include <cmath>
#include <iostream>
#include <numeric>

#if (defined(_WIN32))
#if !defined(_M_X64)
#define _WIN32_VTUNE
#endif
#include <Windows.h>
#include <intrin.h>
#include <tchar.h>
#else
#include <sched.h>
#include <time.h>
#endif

using namespace Validation;

PriorityBooster::PriorityBooster(bool dummy) : m_dummy(dummy) {
  if (m_dummy)
    return;

#if defined(_WIN32)
  bool result = FALSE;
  result = SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
  if (!result)
    throw Exception::GeneralException(
        "OS API error:Can't set program priority class");
  result = SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
  if (!result)
    throw Exception::GeneralException("OS API error:Can't set thread priority");
#ifndef WINDOWS_ONECORE
  // Affinity is not supported on windows universal platform
  result = SetProcessAffinityMask(GetCurrentProcess(), 2);
  if (!result)
    throw Exception::GeneralException(
        "OS API error:Can't set process affinity");
#endif
  Sleep(0);
#else
  // Increase the thread priority to real time
  struct sched_param param;
  int policy = SCHED_RR;
  pthread_t thread_id = pthread_self();
  param.sched_priority = 90;
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(1, &mask);
  pthread_setschedparam(thread_id, policy, &param);
  int result = sched_setaffinity(0, sizeof(mask), &mask);
  if (0 != result)
    throw Exception::GeneralException(
        "OS API error:Can't set process affinity");
#endif
}

PriorityBooster::~PriorityBooster() {
  if (m_dummy)
    return;

#if defined(_WIN32)
  SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
  Sleep(0);
#else
  struct sched_param param;
  pthread_t thread_id = pthread_self();
  param.sched_priority = 0;
  int policy = 0; // for normal threads
  pthread_setschedparam(thread_id, policy, &param);
#endif
}

Sample::Sample()
    : m_ulStart(0), m_ulStop(0), m_ulRDTSCStart(0), m_ulTotalTime(0),
      m_ulTotalRDTSC(0), m_ulSamplesCount(0) {}

inline cl_long RDTSC() {
#ifdef _WIN32
  return __rdtsc();
#else
  if (sizeof(void *) < 8) {
    //    32 bits
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return (cl_long)((uint64_t)hi << 32 | lo);
  } else {
    // 64 bits
    uint64_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return (cl_long)(hi << 32 | lo);
  }
  return 0;

#endif
}

cl_ulong Sample::Start() {
#ifdef _WIN32
  QueryPerformanceCounter((LARGE_INTEGER *)&m_ulStart);
#else
  m_ulStart = clock();
#endif
  m_ulRDTSCStart = RDTSC();
  return m_ulStart;
}

cl_ulong Sample::Stop() {
#ifdef _WIN32
  QueryPerformanceCounter((LARGE_INTEGER *)&m_ulStop);
#else
  m_ulStop = clock();
#endif
  m_ulRDTSCEnd = RDTSC();

  m_ulTotalTime += (m_ulStop - m_ulStart);
  m_ulTotalRDTSC += (m_ulRDTSCEnd - m_ulRDTSCStart);
  ++m_ulSamplesCount;
  return m_ulStop;
}

cl_ulong Sample::SamplesCount() const { return m_ulSamplesCount; }

cl_ulong Sample::TotalTicks() const { return m_ulTotalRDTSC; }

cl_ulong Sample::TotalTime() const {
#ifdef _WIN32
  LARGE_INTEGER freq;
  QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
  double ticksPerSecond = (double)freq.QuadPart / 1000000.;
  return (cl_ulong)((double)m_ulTotalTime / ticksPerSecond);
#else
  return (m_ulTotalTime / CLOCKS_PER_SEC);
#endif
}

bool Sample::operator<(const Sample &rhs) const {
  return TotalTicks() < rhs.TotalTicks();
}

void SampleVector::AddSample(const Sample &sample) {
  m_samples.push_back(sample);
}

Sample SampleVector::MinimalSample() const {
  std::vector<Sample>::const_iterator it =
      std::min_element(m_samples.begin(), m_samples.end());
  return it == m_samples.end() ? Sample() : *it;
}

void SampleVector::Print() const {
  for (std::vector<Sample>::const_iterator it = m_samples.begin();
       it != m_samples.end(); ++it) {
    std::cout << it->TotalTicks() << ",";
  }
  std::cout << "\n";
}

cl_ulong add_ticks(cl_ulong v, const Sample &sample) {
  return sample.TotalTicks() + v;
}

struct pow_mean {
  pow_mean(double mean) : m_mean(mean) {}

  double operator()(double v, const Sample &rhs) {
    return v + std::pow((double)rhs.TotalTicks() - m_mean, 2);
  }

private:
  double m_mean;
};

double SampleVector::Mean() const {
  if (m_samples.empty()) {
    return 0.;
  }

  cl_ulong sum = std::accumulate(m_samples.begin(), m_samples.end(),
                                 (cl_ulong)0, add_ticks);
  return sum / (double)m_samples.size();
}

double SampleVector::StandardDeviation() const {
  if (m_samples.empty()) {
    return 0.;
  }
  double deviation = std::accumulate(m_samples.begin(), m_samples.end(),
                                     (double)0, pow_mean(Mean()));
  return std::sqrt(deviation / m_samples.size());
}

void Performance::SetBuildTime(const Sample &sample) {
  m_buildSample.AddSample(sample);
}

void Performance::SetSerializationTime(const Sample &sample) {
  m_serializationSample.AddSample(sample);
}

void Performance::SetDeserializationTime(const Sample &sample) {
  m_deserializationSample.AddSample(sample);
}

void Performance::SetExecutionTime(const std::string &name,
                                   unsigned int vectorSize,
                                   const Sample &sample) {
  m_executionSamples[KernelID(name, vectorSize)].AddSample(sample);
}

void Performance::Visit(IPerformanceVisitor *pVisitor) const {
  cl_long buildTicks = m_buildSample.MinimalSample().TotalTicks();
  double buildMean = m_buildSample.Mean();
  double buildSD = m_buildSample.StandardDeviation();
  double buildSDMean = buildSD / buildMean;

  cl_long serializationTicks =
      m_serializationSample.MinimalSample().TotalTicks();
  double serializationMean = m_serializationSample.Mean();
  double serializationSD = m_serializationSample.StandardDeviation();
  double serializationSDMean = serializationSD / serializationMean;

  cl_long deserializationTicks =
      m_deserializationSample.MinimalSample().TotalTicks();
  double deserializationMean = m_deserializationSample.Mean();
  double deserializationSD = m_deserializationSample.StandardDeviation();
  double deserializationSDMean = deserializationSD / deserializationMean;

  for (Samples::const_iterator it = m_executionSamples.begin();
       it != m_executionSamples.end(); ++it) {
    double mean = it->second.Mean();
    double sd = it->second.StandardDeviation();
    double sdmean = sd / mean;

    pVisitor->OnKernelSample(it->first.first,  // kernel name
                             it->first.second, // vector size used
                             buildTicks, buildSDMean,
                             it->second.MinimalSample().TotalTicks(), sdmean,
                             serializationTicks, serializationSDMean,
                             deserializationTicks, deserializationSDMean);
  }
}
