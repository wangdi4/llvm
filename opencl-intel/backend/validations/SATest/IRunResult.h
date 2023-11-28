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

#ifndef I_RUN_RESULT_H
#define I_RUN_RESULT_H

#include "IBufferContainerList.h"
#include "IComparisonResults.h"
#include "IPerformance.h"

namespace Validation {
/// @brief Contains the result of comparison for entire run result
class IRunResultComparison {
public:
  virtual ~IRunResultComparison() {}

  /// @brief Returns comparison for specific kernel
  /// @return comparison result
  virtual IComparisonResults *GetComparison(const char *name) = 0;

  /// @brief Returns the overall result of comparison
  virtual bool isFailed() const = 0;
};

/// @brief Contains information about the execution of the test.
/// The information consists of output and performance measurements.
class IRunResult {
public:
  virtual ~IRunResult() {}

  /// @brief Returns test execution output
  /// @return Test output
  virtual IBufferContainerList &GetOutput(const char *kernelName) = 0;
  virtual const IBufferContainerList &GetOutput(const char *name) const = 0;

  /// @brief Returns test execution NEAT output
  /// @return NEAT output
  virtual IBufferContainerList &GetNEATOutput(const char *kernelName) = 0;
  virtual const IBufferContainerList &GetNEATOutput(const char *name) const = 0;

  /// @brief Returns vector of flags signaling comparator to omit corresponding
  /// argument.
  virtual const std::vector<bool> *
  GetComparatorIgnoreList(const char *kernelName) = 0;

  /// @brief Set vector of flags signaling comparator to omit corresponding
  /// argument.
  virtual void SetComparatorIgnoreList(const char *kernelName,
                                       const std::vector<bool> &) = 0;

  /// @brief Returns the count of output buffers
  virtual size_t GetOutputsCount() const = 0;

  /// @brief Returns test execution performance measurements
  /// @return Test performance measurements
  virtual IPerformance &GetPerformance() = 0;
};
} // namespace Validation

#endif // I_RUN_RESULT_H
