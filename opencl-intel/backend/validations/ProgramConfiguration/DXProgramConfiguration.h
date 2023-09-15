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

#ifndef DX_PROGRAM_CONFIGURATION_H
#define DX_PROGRAM_CONFIGURATION_H

#include "IProgramConfiguration.h"

namespace Validation {
/// @brief This class contains DirectX test run configuration
class DXProgramConfiguration : public IProgramConfiguration {
public:
  /// @brief Constructor
  /// @param [IN] configFile Name of DirectX test run configuration file
  DXProgramConfiguration(const std::string &configFile);

  /// @brief Destructor
  virtual ~DXProgramConfiguration(void);

  /// @brief Returns the program file path
  std::string GetProgramFilePath() const override { throw "Not implemented"; }

  /// @brief Returns the program name
  std::string GetProgramName() const override { throw "Not implemented"; }

  /// @brief Return number of kernel configurations to run.
  size_t GetNumberOfKernelConfigurations() const override {
    throw "Not implemented";
  }
};

} // namespace Validation

#endif // DX_RUN_CONFIGURATION_H
