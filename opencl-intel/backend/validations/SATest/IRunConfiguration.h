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

#ifndef I_RUN_CONFIGURATION_H
#define I_RUN_CONFIGURATION_H

namespace Validation {
enum TEST_MODE { VALIDATION, REFERENCE, PERFORMANCE, BUILD };

/// @brief  This class contains test run configuration for SATest components
/// like comparator, reference runner
///         and back-end runner.
class IRunComponentConfiguration {
public:
  virtual ~IRunComponentConfiguration() {}

  /// @brief Init the configuration from the command line parameters
  virtual void InitFromCommandLine() = 0;
};

/// @brief This class contains test run configuration for SATest
class IRunConfiguration {
public:
  virtual ~IRunConfiguration() {}

  /// @brief Init the configuration from the command line parameters
  virtual void InitFromCommandLine() = 0;

  /// @brief Returns true if reference should be used in validation mode
  virtual bool UseReference() const = 0;

  /// @brief Returns true if reference should unconditionally generated
  virtual bool GetForceReference() const = 0;

  /// @brief Set "force_ref" flag to passed value.
  virtual void SetForceReference(bool enable) = 0;

  /// @brief Returns current test mode.
  virtual TEST_MODE TestMode() const = 0;

  /// @brief Returns pointer to the object with comparator configuration
  virtual const IRunComponentConfiguration *
  GetComparatorConfiguration() const = 0;

  /// @brief Returns pointer to the object with reference runner configuration
  virtual const IRunComponentConfiguration *
  GetReferenceRunnerConfiguration() const = 0;

  /// @brief Returns pointer to the object with back-end runner configuration
  virtual IRunComponentConfiguration *GetBackendRunnerConfiguration() = 0;
};

} // namespace Validation

#endif // I_RUN_CONFIGURATION_H
