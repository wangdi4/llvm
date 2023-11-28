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

#ifndef SATest_H
#define SATest_H

#include "BufferContainerList.h"
#include "IProgram.h"
#include "IProgramRunner.h"
#include "IRunConfiguration.h"
#include "RunResult.h"
#include "RunnerFactoryGenerator.h"
#include <string>

namespace Validation {
/// @brief The main class that runs a single test
class SATest {
public:
  /// @brief Constructor
  /// @param [IN] configFileName Name of file that contains run configurations
  /// @param [IN] baseDirectory Directory to use in data file lookup
  /// @param [IN] pRunConfiguration Run configuration file
  SATest(const std::string &configFileName, const std::string &baseDirectory,
         IRunConfiguration *pRunConfiguration);

  /// @brief Destructor
  virtual ~SATest(void);

  /// @brief Runs a single test
  /// @throws TestFailException In case the test failed
  void Run(TEST_MODE mode, IRunConfiguration *pRunConfiguration);

  /// @brief Validates execution environment of SATest
  /// @throws InvalidEnvironmentException in case of invalid environment
  static void ValidateEnvironment();

private:
  /// @brief Loads or re-generate reference output.
  void LoadOrGenerateReference(IRunConfiguration *pRunConfiguration,
                               IRunResult *pResult);

  /// @brief re-generate reference output.
  void RunReference(const IRunComponentConfiguration *pRunConfiguration);

  /// @brief Runs a single test and measure its performance
  void RunPerformance(const IRunComponentConfiguration *pRunConfiguration);

  /// @brief Runs a single test and measure its performance
  void RunValidation(IRunConfiguration *pRunConfiguration);

  /// @brief Generate the reference output
  void GenerateReference(IRunResult *pResult, IProgramRunner *pRunner,
                         const IRunComponentConfiguration *pRunConfiguration);

  /// @brief Load the reference output
  void LoadReference(IRunResult *pResult, IProgramRunner *pRunner,
                     const IRunComponentConfiguration *pRunConfiguration);

  /// @brief Runs a single test in build only mode
  void RunBuildOnly(const IRunComponentConfiguration *pRunConfiguration);

private:
  /// @brief Runner factory generator
  IRunnerFactory &m_factory;

  /// @brief Test run configuration
  IProgramConfiguration *m_pProgramConfiguration;

  /// @brief Test program
  IProgram *m_pProgram;
};
} // namespace Validation

#endif // SATest_H
