// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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

#ifndef COMPILER_CONFIG_H
#define COMPILER_CONFIG_H

#include "ICompilerConfig.h"
#include "cpu_dev_limits.h"
#include "exceptions.h"

#include <algorithm>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

DEFINE_EXCEPTION(BadConfigException)

class GlobalCompilerConfig : public IGlobalCompilerConfig {
public:
  void LoadDefaults();
  void LoadConfig();
  void SkipBuiltins();
  void ApplyRuntimeOptions(const ICLDevBackendOptions *pBackendOptions);

  bool EnableTiming() const override { return m_enableTiming; }
  bool DisableStackDump() const override { return m_disableStackDump; }
  std::string InfoOutputFile() const override { return m_infoOutputFile; }
  const llvm::SmallVectorImpl<std::string> &LLVMOptions() const override {
    return m_LLVMOptions;
  }
  DeviceMode TargetDevice() const override { return m_targetDevice; }

private:
  bool m_enableTiming;
  bool m_disableStackDump;
  std::string m_infoOutputFile;
  llvm::SmallVector<std::string, 32> m_LLVMOptions;
  DeviceMode m_targetDevice;
};

//******************************************************************************
// CompilerConfig implementation.
//
// Responsible for loading both default configuraiton as well as runtime
// configuration passed to the backend.
//
// This class is used as an adapter from ICLDevBackendOptions to ICompilerConfig
class CompilerConfig : public virtual ICompilerConfig {
public:
  // CompilerConfiguration methods
  void LoadDefaults();
  virtual void LoadConfig();
  void SkipBuiltins() { m_loadBuiltins = false; }
  void ApplyRuntimeOptions(const ICLDevBackendOptions *pBackendOptions);

  std::string GetCpuArch() const override { return m_cpuArch; }
  std::string GetCpuFeatures() const override { return m_cpuFeatures; }
  size_t GetCpuMaxWGSize() const override { return m_cpuMaxWGSize; }
  ETransposeSize GetTransposeSize() const override { return m_transposeSize; }
  int GetRTLoopUnrollFactor() const override { return m_rtLoopUnrollFactor; }
  bool GetUseVTune() const override { return m_useVTune; }
  bool GetSerializeWorkGroups() const override { return m_serializeWorkGroups; }
  bool GetLoadBuiltins() const override { return m_loadBuiltins; }
  size_t GetForcedPrivateMemorySize() const override {
    return m_forcedPrivateMemorySize;
  }
  bool UseAutoMemory() const override { return m_useAutoMemory; }

  bool GetDumpHeuristicIRFlag() const override { return m_dumpHeuristicIR; }

  const std::string &GetDumpFilenamePrefix() const override {
    return m_dumpFilenamePrefix;
  }

  bool GetStreamingAlways() const override { return m_streamingAlways; }

  unsigned GetExpensiveMemOpts() const override { return m_expensiveMemOpts; }

  DeviceMode TargetDevice() const override { return m_targetDevice; }

  PassManagerType GetPassManagerType() const override {
    return m_passManagerType;
  }

protected:
  std::string m_cpuArch;
  std::string m_cpuFeatures;
  size_t m_cpuMaxWGSize;
  ETransposeSize m_transposeSize;
  int m_rtLoopUnrollFactor;
  bool m_useVTune;
  bool m_serializeWorkGroups;
  bool m_loadBuiltins;
  bool m_dumpHeuristicIR;
  std::string m_dumpFilenamePrefix;
  int m_forcedPrivateMemorySize;
  bool m_useAutoMemory;
  bool m_streamingAlways;
  unsigned m_expensiveMemOpts;
  DeviceMode m_targetDevice;
  PassManagerType m_passManagerType;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
#endif // COMPILER_CONFIG_H
