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

#ifndef I_PROGRAM_RUNNER_H
#define I_PROGRAM_RUNNER_H

#include "IBufferContainerList.h"
#include "IProgram.h"
#include "IProgramConfiguration.h"
#include "IRunConfiguration.h"
#include "IRunResult.h"

namespace Validation {
/// @brief Interface for the test program runners.
class IProgramRunner {
public:
  virtual ~IProgramRunner(void) {}

  /// @brief Executes a single test program
  /// @param [OUT] runResult Result of test program execution
  /// @param [IN] program Test program to execute
  /// @param [IN] programConfig Test program configuration options
  /// @param [IN] runConfig Configuration of the test run.
  virtual void Run(IRunResult *pRunResult, const IProgram *pProgram,
                   const IProgramConfiguration *programConfig,
                   const IRunComponentConfiguration *runConfig) = 0;

  /// @brief Load pre-computed test output data from the file specified in
  /// program configuration.
  /// @param [OUT] runResult Result of test program execution
  /// @param [IN] config Configuration of the test run
  virtual void LoadOutput(IRunResult *pRunResult,
                          const IProgramConfiguration *pConfig) = 0;

  /// @brief Saves test execution results to the file specified in program
  /// configuration.
  /// @param [IN] runResult Result of test program execution
  /// @param [IN] config Configuration of the test run
  virtual void StoreOutput(const IRunResult *pRunResult,
                           const IProgramConfiguration *pConfig) const = 0;
};
} // namespace Validation

#endif // I_PROGRAM_RUNNER_H
