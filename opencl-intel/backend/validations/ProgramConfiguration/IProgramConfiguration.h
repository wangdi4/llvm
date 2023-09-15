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

#ifndef I_PROGRAM_CONFIGURATION_H
#define I_PROGRAM_CONFIGURATION_H

#include <string>

namespace Validation {
/// @brief This class contains test run configuration
class IProgramConfiguration {
public:
  virtual ~IProgramConfiguration(void) {}

  /// @brief Returns the program file path
  virtual std::string GetProgramFilePath() const = 0;

  /// @brief Returns the program name
  virtual std::string GetProgramName() const = 0;

  /// @brief Return number of kernel configurations to run.
  virtual size_t GetNumberOfKernelConfigurations() const = 0;
};
} // namespace Validation

#endif // I_RUN_CONFIGURATION_H
