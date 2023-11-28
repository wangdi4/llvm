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

#ifndef OCL_RUN_CONFIGURATION_H
#define OCL_RUN_CONFIGURATION_H

#include "ICLDevBackendOptions.h"
#include "IRunConfiguration.h"
#include "common_dev_limits.h"
#include "llvm/Support/DataTypes.h"
#include <cstddef> // for std::size_t not included in ICLDevBackendOptions.h! TODO: Remove when the bug is fixed.
#include <string>
#include <vector>

namespace Validation {
using PassManagerType = Intel::OpenCL::DeviceBackend::PassManagerType;

enum RunConfigurationOption {
  // Common options
  RC_COMMON_DEFAULT_LOCAL_WG_SIZE,
  RC_COMMON_RUN_SINGLE_WG,
  RC_COMMON_RANDOM_DG_SEED,

  // Back-end runner specific options
  RC_BR_BUILD_ITERATIONS_COUNT,
  RC_BR_CPU_FEATURES,
  RC_BR_CPU_ARCHITECTURE,
  RC_BR_DEVICE_MODE,
  RC_BR_DUMP_OPTIMIZED_LLVM_IR,
  RC_BR_EXECUTE_ITERATIONS_COUNT,
  RC_BR_MEASURE_PERFORMANCE,
  RC_BR_VECTORIZER_TYPE,
  RC_BR_ENABLE_SUBGROUP_EMULATION,
  RC_BR_TRANSPOSE_SIZE,
  RC_BR_VERBOSE,
  RC_BR_USE_SDE,
  RC_BR_USE_PIN_TRACE_MARKS,
  RC_BR_USE_VTUNE,
  RC_BR_PRINT_BUILD_LOG,
  RC_BR_BUILD_ONLY,
  RC_BR_STOP_BEFORE_JIT,
  RC_BR_DUMP_JIT,
  RC_BR_DUMP_KERNEL_PROPERTY,
  RC_BR_TIME_PASSES,
  RC_BR_LLVM_OPTION,
  RC_BR_DUMP_HEURISTIC_IR,
  RC_BR_PERF_LOG,
  RC_BR_OBJECT_FILE,
  RC_BR_EXPENSIVE_MEM_OPT,
  RC_BR_PASS_MANAGER_TYPE,
  RC_BR_SERIALIZE_WORK_GROUPS,
  // Reference runner specific options
  RC_REF_USE_NEAT,
  RC_REF_USE_FMA_NEAT,

  // These are comparator related configurations
  RC_COMP_DETAILED_STAT,
  RC_COMP_ULP_TOLERANCE,

  RC_END
};

class BERunOptions : public IRunComponentConfiguration {
public:
  BERunOptions();
  /// @brief Init the configuration from the command line parameters
  void InitFromCommandLine() override;
  template <typename T>
  T GetValue(RunConfigurationOption rc, T defaultValue) const {
    // TODO: notify via the logger to the user that default option value was
    // returned.
    return defaultValue;
  }
  template <typename T> void SetValue(RunConfigurationOption rc, T setValue);

private:
  bool m_measurePerformance;
  bool m_useSDE;
  bool m_useTraceMarks;
  bool m_useVTune;
  bool m_printBuildLog;
  bool m_runSingleWG;
  bool m_buildOnly;
  bool m_stopBeforeJIT;
  bool m_verbose;
  uint32_t m_expensiveMemOpts;
  DeviceMode m_deviceMode;
  uint32_t m_defaultLocalWGSize;
  uint64_t m_RandomDataGeneratorSeed;
  uint32_t m_buildIterationsCount;
  uint32_t m_executeIterationsCount;
  std::string m_cpuArch;
  std::string m_cpuFeatures;
  std::string m_optimizedLLVMIRDumpFile;
  std::string m_perfLogFile;
  Intel::OpenCL::DeviceBackend::ETransposeSize m_transposeSize;
  std::string m_DumpJIT;
  std::string m_TimePasses;
  std::string m_DebugPassManager;
  std::string m_InjectedObject;
  std::string m_LLVMOption;
  bool m_dumpHeuristcIR;
  bool m_dumpKernelProperty;
  VectorizerType m_vectorizerType;
  bool m_enableSubgroupEmulation;
  PassManagerType m_passManagerType;
  bool m_serializeWorkGroups;
};

template <>
bool BERunOptions::GetValue<bool>(RunConfigurationOption rc,
                                  bool defaultValue) const;
template <>
int BERunOptions::GetValue<int>(RunConfigurationOption rc,
                                int defaultValue) const;
template <>
uint32_t BERunOptions::GetValue<uint32_t>(RunConfigurationOption rc,
                                          uint32_t defaultValue) const;
template <>
uint64_t BERunOptions::GetValue<uint64_t>(RunConfigurationOption rc,
                                          uint64_t defaultValue) const;
template <>
std::string BERunOptions::GetValue<std::string>(RunConfigurationOption rc,
                                                std::string defaultValue) const;
template <>
VectorizerType
BERunOptions::GetValue<VectorizerType>(RunConfigurationOption rc,
                                       VectorizerType defaultValue) const;
template <>
Intel::OpenCL::DeviceBackend::ETransposeSize
BERunOptions::GetValue<Intel::OpenCL::DeviceBackend::ETransposeSize>(
    RunConfigurationOption rc,
    Intel::OpenCL::DeviceBackend::ETransposeSize defaultValue) const;
template <>
PassManagerType
BERunOptions::GetValue<PassManagerType>(RunConfigurationOption rc,
                                        PassManagerType defaultValue) const;
template <>
void BERunOptions::SetValue<int>(RunConfigurationOption rc, int setValue);

class ComparatorRunOptions : public IRunComponentConfiguration {
public:
  ComparatorRunOptions();
  /// @brief Init the configuration from the command line parameters
  void InitFromCommandLine() override;
  template <typename T>
  T GetValue(RunConfigurationOption rc, T defaultValue) const {
    // TODO: notify via the logger to the user that default option value was
    // returned.
    return defaultValue;
  }

private:
  double m_ULP_tolerance;
  bool m_detailedStat;
};

template <>
bool ComparatorRunOptions::GetValue<bool>(RunConfigurationOption rc,
                                          bool defaultValue) const;
template <>
double ComparatorRunOptions::GetValue<double>(RunConfigurationOption rc,
                                              double defaultValue) const;

class ReferenceRunOptions : public IRunComponentConfiguration {
public:
  ReferenceRunOptions();
  /// @brief Init the configuration from the command line parameters
  void InitFromCommandLine() override;
  template <typename T>
  T GetValue(RunConfigurationOption rc, T defaultValue) const {
    // TODO: notify via the logger to the user that default option value was
    // returned.
    return defaultValue;
  }

private:
  bool m_useNEAT;
  bool m_runSingleWG;
  uint32_t m_defaultLocalWGSize;
  uint64_t m_RandomDataGeneratorSeed;
};

template <>
bool ReferenceRunOptions::GetValue<bool>(RunConfigurationOption rc,
                                         bool defaultValue) const;
template <>
uint32_t ReferenceRunOptions::GetValue<uint32_t>(RunConfigurationOption rc,
                                                 uint32_t defaultValue) const;
template <>
uint64_t ReferenceRunOptions::GetValue<uint64_t>(RunConfigurationOption rc,
                                                 uint64_t defaultValue) const;

/// @brief This class contain OpenCL test run information
class OpenCLRunConfiguration : public IRunConfiguration {
public:
  /// @brief Constructor
  OpenCLRunConfiguration();

  /// @brief Init the configuration from the command line parameters
  virtual void InitFromCommandLine() override;

  /// @brief Returns true if reference should be used in validation mode
  virtual bool UseReference() const override;

  /// @brief Returns true if reference should be unconditionally generated
  virtual bool GetForceReference() const override;

  /// @brief Set "force_ref" flag to passed value.
  virtual void SetForceReference(bool enable) override;

  /// @brief Returns current test mode.
  virtual TEST_MODE TestMode() const override;

  /// @brief Returns pointer to the object with comparator configuration
  virtual const IRunComponentConfiguration *
  GetComparatorConfiguration() const override;

  /// @brief Returns pointer to the object with reference runner configuration
  virtual const IRunComponentConfiguration *
  GetReferenceRunnerConfiguration() const override;

  /// @brief Returns pointer to the object with back-end runner configuration
  virtual IRunComponentConfiguration *GetBackendRunnerConfiguration() override;

private:
  // SATest options
  bool m_useReference;
  bool m_forceReference;
  TEST_MODE m_testMode;

  // SATest components options
  BERunOptions m_backendOptions;
  ComparatorRunOptions m_comparatorOptions;
  ReferenceRunOptions m_referenceOptions;
};

} // namespace Validation

#endif // OCL_RUN_CONFIGURATION_H
