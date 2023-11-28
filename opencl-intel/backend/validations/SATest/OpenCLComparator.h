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

#ifndef OPENCL_COMPARATOR_H
#define OPENCL_COMPARATOR_H

#include "IRunResultComparator.h"
#include "cl_dev_backend_api.h"
#include "cl_device_api.h"
#include <string>
#include <vector>

namespace Validation {
class OpenCLProgramConfiguration;
class ComparatorRunOptions;
class ReferenceRunOptions;
class IRunResultComparison;
class IRunResult;
class IProgram;

/// @brief Responsible for comparing the run results of the given program
class OpenCLComparator : public IRunResultComparator {
public:
  // TODO: At the moment, RunResult objects doesn't know if it contains valid
  // NEAT data, so we forced to pass Reference run options and assume if NEAT is
  // enabled then RunResult must contain NEAT data. To fix this design bug we
  // need to re-design RunResult object (including interface).
  OpenCLComparator(const OpenCLProgramConfiguration *pProgramConfig,
                   const ComparatorRunOptions *pComparatorConfig,
                   const ReferenceRunOptions *pRefConfig);

  /// @brief Compares the result of two runs
  virtual IRunResultComparison *
  Compare(IRunResult *runResults,
          IRunResult *referenceRunResults) const override;

private:
  bool m_useNeat;
  double m_ULPTolerance;
  bool m_detailedStat;
  std::vector<std::string> m_kernels;
};
} // namespace Validation

#endif // RUN_RESULTS_H
