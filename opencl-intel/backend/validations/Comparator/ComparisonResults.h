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

#ifndef COMPARISON_RESULTS_H
#define COMPARISON_RESULTS_H

#include "IComparisonResults.h"
#include <map>
#include <string>
#include <vector>

namespace Validation {
/// Comparison data for one buffer
struct CompStatistics {
  IMemoryObjectDescPtr pDesc;
  uint64_t numMismatches;
  double maxDiff;
  float maxInterval;

  CompStatistics() : numMismatches(0), maxDiff(0.0), maxInterval(0.0) {}

  CompStatistics(const IMemoryObjectDesc *in_pDesc)
      : pDesc(*in_pDesc), numMismatches(0), maxDiff(0.0), maxInterval(0.0) {}
};

/// @brief  Simple implementation of IComparisonResults interface. Stores
/// mismatches information in the vector of values.
class ComparisonResults : public IComparisonResults {
public:
  static const uint32_t MAX_MISMATCHES = 100;

  ComparisonResults(const std::string &kerName, bool statDetail)
      : m_kernelName(kerName) {}

  ComparisonResults() {}

  virtual ~ComparisonResults();

  /// @brief Adds mismatch information
  /// @param  [in]  in_Val            Mismatch value information
  /// template<typename T>
  void AddMismatch(const MismatchedVal &in_Val) override;

  /// @brief Gets mismatch information by given mismatch index
  /// @param [in] index   Index of mismatch value in mismatch container
  MismatchedVal GetMismatch(size_t index) override;

  /// @brief Number of mismatched values in mismatch container
  size_t GetMismatchCount() override;

  bool isFailed() override;

  void Report();

  void ReportDetail();

  void Clear() override;

  /// @brief returns pointer to object that contains statistics data
  StatisticsCollector *GetStatistics() override { return &mStatistics; }

private:
  std::string m_kernelName;
  /// @brief  Mismatched values container
  std::vector<MismatchedVal> mismatches;
  /// @brief  Number of mismatched values
  typedef std::map<uint32_t, CompStatistics> CompStatMap;
  CompStatMap m_statMap;
  /// @brief  Object for gathering statistical information
  StatisticsCollector mStatistics;
};
} // namespace Validation

#endif // COMPARISON_RESULTS_H
