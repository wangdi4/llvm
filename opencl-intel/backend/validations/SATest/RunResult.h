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

#ifndef RUN_RESULTS_H
#define RUN_RESULTS_H

#include "BufferContainerList.h"
#include "IBufferContainerList.h"
#include "IComparisonResults.h"
#include "IPerformance.h"
#include "IRunResult.h"
#include "Performance.h"
#include <map>
#include <string>
#include <vector>

namespace Validation {
/// @brief Responsible for holding the result of comparison
///        for entire test run ( possible for multiple kernels)
class RunResultComparison : public IRunResultComparison {
public:
  RunResultComparison() : m_failed(false) {}

  ~RunResultComparison();

  IComparisonResults *GetComparison(const char *name) override;

  bool isFailed() const override { return m_failed; }

public:
  void AddComparison(const char *name, IComparisonResults *comparison);

  void SetIsFailedStatus(bool failed) { m_failed = failed; }

private:
  typedef std::map<std::string, IComparisonResults *> CompResultsMap;
  CompResultsMap m_comparisons;
  bool m_failed;
};

/// @brief This class is responsible for containing test output and
///        performance measurements for OpenCL program
// This class is used for storing Run results both for
/// ocl_backend and OpenCL Reference
class RunResult : public IRunResult {
public:
  /// @brief Constructor
  RunResult();

  /// @brief Destructor
  virtual ~RunResult(void);

  RunResult(const RunResult &) = delete;
  RunResult &operator=(const RunResult &) = delete;

  /// @brief Returns test execution output
  /// @return Test output
  virtual IBufferContainerList &GetOutput(const char *name) override;
  virtual const IBufferContainerList &
  GetOutput(const char *name) const override;

  /// @brief Returns test execution NEAT output
  /// @return Test NEAT output
  virtual IBufferContainerList &GetNEATOutput(const char *name) override;
  virtual const IBufferContainerList &
  GetNEATOutput(const char *name) const override;

  /// @brief Returns pointer to the list of boolean values that signal
  /// comparator if it can omit corresponding kernel argument.
  /// @param name Name of the kernel.
  virtual const std::vector<bool> *
  GetComparatorIgnoreList(const char *name) override;

  /// @brief Sets ignore list for the kernel with the provided name.
  /// @param name Name of the kernel.
  /// @param ignoreList The list of boolean values that signal comparator if it
  /// can omit corresponding kernel argument.
  virtual void
  SetComparatorIgnoreList(const char *name,
                          const std::vector<bool> &ignoreList) override;

  /// @brief Returns test execution performance measurements
  /// @return Test performance measurements
  virtual IPerformance &GetPerformance() override;

  /// @brief Returns number of output buffers
  virtual size_t GetOutputsCount() const override;

private:
  typedef std::map<std::string, BufferContainerList *> OutputsMap;
  /// @brief Output buffers
  OutputsMap m_refOutputs;
  OutputsMap m_neatOutputs;
  std::map<std::string, std::vector<bool>> m_comparatorIgnoreList;

  /// @brief Output performance
  Performance m_performance;
};
} // namespace Validation

#endif // RUN_RESULTS_H
