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

#ifndef OPENCL_FACTORY_H
#define OPENCL_FACTORY_H

#include "IRunnerFactory.h"
#include <string>

namespace Validation {
/// @brief This class is a factory of OpenCL program, program runner and
/// reference runner
class OpenCLFactory : public IRunnerFactory {
public:
  OpenCLFactory(void);
  ~OpenCLFactory(void);

  /// @brief Creates new OpenCL program
  virtual IProgram *
  CreateProgram(IProgramConfiguration *programConfig,
                IRunConfiguration *pRunConfiguration) override;

  /// @brief Creates new program configuration
  virtual IProgramConfiguration *
  CreateProgramConfiguration(const std::string &configFile,
                             const std::string &baseDir) override;

  /// @brief Creates new run configuration
  virtual IRunConfiguration *CreateRunConfiguration() override;

  /// @brief Creates new OpenCL program runner
  virtual IProgramRunner *CreateProgramRunner(
      const IRunComponentConfiguration *pRunConfiguration) override;

  /// @brief Creates new OpenCL reference runner
  virtual IProgramRunner *CreateReferenceRunner(
      const IRunComponentConfiguration *pRunConfiguration) override;

  /// @brief Creates new run results comparator for given platform
  virtual IRunResultComparator *
  CreateComparator(IProgramConfiguration *pProgramConfiguration,
                   IRunConfiguration *pRunConfiguration) override;
};
} // namespace Validation

#endif // OPENCL_FACTORY_H
