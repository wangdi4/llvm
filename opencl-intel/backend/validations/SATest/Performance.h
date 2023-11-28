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

#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include "CL/cl.h"
#include "IPerformance.h"
#include "cl_dev_backend_api.h"
#include "cl_device_api.h"
#include <map>
#include <string>
#include <vector>

namespace Validation {
class PriorityBooster {
public:
  PriorityBooster(bool dummy);
  ~PriorityBooster();

private:
  bool m_dummy;
};

/// @brief Basic sampler. Used to measure the time intervals
class Sample {
public:
  Sample();

  /// @brief Starts/Restart sampling. Restarting the sampling
  ///        will increment the sample count (upon stop)
  cl_ulong Start();

  /// @brief Stops sampling
  cl_ulong Stop();

  /// @brief Returns the number of seconds in measured interval
  cl_ulong TotalTime() const;

  /// @brief Returns the number of ticks in measured interval
  cl_ulong TotalTicks() const;

  /// @brief Returns the number of samples (actually number of start/stop
  /// rounds)
  cl_ulong SamplesCount() const;

  /// @brief Standard comparison operator
  bool operator<(const Sample &rhs) const;

private:
  cl_ulong m_ulStart;
  cl_ulong m_ulStop;
  cl_ulong m_ulRDTSCStart;
  cl_ulong m_ulRDTSCEnd;

  cl_ulong m_ulTotalTime;
  cl_ulong m_ulTotalRDTSC;
  cl_ulong m_ulSamplesCount;
};

/// @brief Vector of samples. Provides various statistical information (min,
/// geomean, stdd)
class SampleVector {
public:
  /// @brief Adds new sample to the vector
  void AddSample(const Sample &sample);

  /// @brief Returns the sample with a minimal TotalTick count or default
  /// (zeroed) Sample in case of empty vector
  Sample MinimalSample() const;

  /// @brief Calculates the mean of all samples in the vector
  double Mean() const;

  /// @brief Calculates the standard deviation of the samples in the vector
  double StandardDeviation() const;

  void Print() const;

private:
  std::vector<Sample> m_samples;
};

/// @brief This class enables test execution performance measurements.
/// The measurements consist of build time and execution time.
class Performance : public IPerformance {
public:
  /// @brief Set the build time
  void SetBuildTime(const Sample &sample);

  /// @brief Set the program serialization time
  void SetSerializationTime(const Sample &sample);

  /// @brief Set the program de-serialization time
  void SetDeserializationTime(const Sample &sample);

  /// @brief Add the sample to the sample vector for given kernel
  void SetExecutionTime(const std::string &name, unsigned int vectorSize,
                        const Sample &sample);

  /// @brief Visits the performance data
  void Visit(IPerformanceVisitor *pVisitor) const override;

private:
  typedef std::pair<std::string, unsigned int> KernelID;
  typedef std::map<KernelID, SampleVector> Samples;

  Samples m_executionSamples;
  SampleVector m_buildSample;
  SampleVector m_serializationSample;
  SampleVector m_deserializationSample;
};
} // namespace Validation

#endif // PERFORMANCE_H
