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

#ifndef I_RUNNER_FACTORY_H
#define I_RUNNER_FACTORY_H

#include "IProgram.h"
#include "IProgramConfiguration.h"
#include "IProgramRunner.h"
#include "IRunConfiguration.h"
#include "IRunResultComparator.h"
#include <string>

namespace Validation {
/// @brief This class is a factory of program, program runner and reference
/// runner
class IRunnerFactory {
public:
  virtual ~IRunnerFactory(void) {}

  /// @brief Creates new program
  virtual IProgram *CreateProgram(IProgramConfiguration *programConfig,
                                  IRunConfiguration *pRunConfiguration) = 0;

  /// @brief Creates new program configuration
  virtual IProgramConfiguration *
  CreateProgramConfiguration(const std::string &configFile,
                             const std::string &baseDir) = 0;

  /// @brief Creates new program configuration
  virtual IRunConfiguration *CreateRunConfiguration() = 0;

  /// @brief Creates new program runner
  virtual IProgramRunner *
  CreateProgramRunner(const IRunComponentConfiguration *pRunConfiguration) = 0;

  /// @brief Creates new reference runner
  virtual IProgramRunner *CreateReferenceRunner(
      const IRunComponentConfiguration *pRunConfiguration) = 0;

  /// @brief Creates new run results comparator
  // TODO: Once RunResult object will be re-designed, change IRunConfiguration
  // to IRunComponentConfiguration.
  virtual IRunResultComparator *
  CreateComparator(IProgramConfiguration *pProgramConfiguration,
                   IRunConfiguration *pRunConfiguration) = 0;
};
} // namespace Validation

#endif // I_RUNNER_FACTORY_H
