// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#ifndef BACKEND_OPTIONS_H
#define BACKEND_OPTIONS_H

#include "Exception.h"
#include "OpenCLRunConfiguration.h"
#include "cl_dev_backend_api.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

namespace Validation {

// FIXME 'using namespace' shouldn't be used in header file.
using namespace Intel::OpenCL::DeviceBackend;

class GlobalBackendOptions : public ICLDevBackendOptions {
public:
  void InitFromRunConfiguration(const BERunOptions &runConfig) {
    m_transposeSize =
        runConfig.GetValue<Intel::OpenCL::DeviceBackend::ETransposeSize>(
            RC_BR_TRANSPOSE_SIZE, TRANSPOSE_SIZE_NOT_SET);
    m_TimePasses = runConfig.GetValue<std::string>(RC_BR_TIME_PASSES, "");
    m_DisableStackDump =
        runConfig.GetValue<bool>(RC_BR_USE_PIN_TRACE_MARKS, false);
    m_vectorizerType = runConfig.GetValue<VectorizerType>(RC_BR_VECTORIZER_TYPE,
                                                          DEFAULT_VECTORIZER);
    m_enableSubgroupEmulation =
        runConfig.GetValue<bool>(RC_BR_ENABLE_SUBGROUP_EMULATION, true);
    m_passManagerType =
        runConfig.GetValue<PassManagerType>(RC_BR_PASS_MANAGER_TYPE, PM_NONE);
    m_llvmOption = runConfig.GetValue<std::string>(RC_BR_LLVM_OPTION, "");
  }

  const char *GetStringValue(int optionId,
                             const char *defaultValue) const override {
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_TIME_PASSES:
      return m_TimePasses.c_str();
    case CL_DEV_BACKEND_OPTION_LLVM_OPTION:
      return m_llvmOption.c_str();
    default:
      return defaultValue;
    }
  }

  bool GetBooleanValue(int optionId, bool defaultValue) const override {
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_DISABLE_STACKDUMP:
      return m_DisableStackDump;
    case CL_DEV_BACKEND_OPTION_SUBGROUP_EMULATION:
      return m_enableSubgroupEmulation;
    default:
      return defaultValue;
    }
    return defaultValue;
  }

  virtual int GetIntValue(int optionId, int defaultValue) const override {
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE:
      return m_transposeSize;
    case CL_DEV_BACKEND_OPTION_VECTORIZER_TYPE:
      return m_vectorizerType;
    case CL_DEV_BACKEND_OPTION_PASS_MANAGER_TYPE:
      return (int)m_passManagerType;
    default:
      return defaultValue;
    }
  }

  virtual bool GetValue(int optionId, void *Value,
                        size_t *pSize) const override {
    return false;
  }

private:
  Intel::OpenCL::DeviceBackend::ETransposeSize m_transposeSize;
  std::string m_TimePasses;
  bool m_DisableStackDump;
  VectorizerType m_vectorizerType;
  PassManagerType m_passManagerType;
  std::string m_llvmOption;
  bool m_enableSubgroupEmulation;
};

class CPUBackendOptions : public ICLDevBackendOptions {
public:
  CPUBackendOptions() {}

  virtual ~CPUBackendOptions() {}

  void InitFromRunConfiguration(const BERunOptions &runConfig) {
    m_transposeSize =
        runConfig.GetValue<Intel::OpenCL::DeviceBackend::ETransposeSize>(
            RC_BR_TRANSPOSE_SIZE, TRANSPOSE_SIZE_NOT_SET);
    m_cpu = runConfig.GetValue<std::string>(RC_BR_CPU_ARCHITECTURE, "auto");
    m_cpuFeatures = runConfig.GetValue<std::string>(RC_BR_CPU_FEATURES, "");
    m_useVTune = runConfig.GetValue<bool>(RC_BR_USE_VTUNE, false);

    m_deviceMode = static_cast<DeviceMode>(
        runConfig.GetValue<int>(RC_BR_DEVICE_MODE, CPU_DEVICE));
    m_dumpHeuristcIR = runConfig.GetValue<bool>(RC_BR_DUMP_HEURISTIC_IR, false);
    m_vectorizerType = runConfig.GetValue<VectorizerType>(RC_BR_VECTORIZER_TYPE,
                                                          DEFAULT_VECTORIZER);
    m_expensiveMemOpts =
        runConfig.GetValue<unsigned>(RC_BR_EXPENSIVE_MEM_OPT, false);
    m_passManagerType =
        runConfig.GetValue<PassManagerType>(RC_BR_PASS_MANAGER_TYPE, PM_NONE);
    m_serializeWorkGroups =
        runConfig.GetValue<bool>(RC_BR_SERIALIZE_WORK_GROUPS, false);
  }

  virtual void InitTargetDescriptionSession(
      ICLDevBackendExecutionService *pExecutionService) {}

  bool GetBooleanValue(int optionId, bool defaultValue) const override {
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_USE_VTUNE:
      return m_useVTune;
    case CL_DEV_BACKEND_OPTION_DUMP_HEURISTIC_IR:
      return m_dumpHeuristcIR;
    case CL_DEV_BACKEND_OPTION_SERIALIZE_WORK_GROUPS:
      return m_serializeWorkGroups;
    default:
      return defaultValue;
    }
  }

  virtual int GetIntValue(int optionId, int defaultValue) const override {
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_DEVICE:
      return m_deviceMode;
    case CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE:
      return m_transposeSize;
    case CL_DEV_BACKEND_OPTION_VECTORIZER_TYPE:
      return m_vectorizerType;
    case CL_DEV_BACKEND_OPTION_EXPENSIVE_MEM_OPTS:
      return m_expensiveMemOpts;
    case CL_DEV_BACKEND_OPTION_PASS_MANAGER_TYPE:
      return (int)m_passManagerType;
    default:
      return defaultValue;
    }
  }

  virtual const char *GetStringValue(int optionId,
                                     const char *defaultValue) const override {
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_SUBDEVICE:
      return m_cpu.c_str();
    case CL_DEV_BACKEND_OPTION_SUBDEVICE_FEATURES:
      return m_cpuFeatures.c_str();
    default:
      return defaultValue;
    }
  }

  virtual bool GetValue(int optionId, void *Value,
                        size_t *pSize) const override {
    if (Value == NULL) {
      throw Exception::InvalidArgument("Value is not initialized");
    }
    switch (optionId) {
    default:
      return false;
    }
  }

protected:
  Intel::OpenCL::DeviceBackend::ETransposeSize m_transposeSize;
  std::string m_cpu;
  std::string m_cpuFeatures;
  DeviceMode m_deviceMode;
  bool m_useVTune;
  unsigned m_expensiveMemOpts;
  bool m_dumpHeuristcIR;
  VectorizerType m_vectorizerType;
  PassManagerType m_passManagerType;
  bool m_serializeWorkGroups;
};

/**
 * Description of ENABLE_SDE mode for MIC:
 * In the initialization flow for MIC, we need from the device (Target
 * Description), this is a buffer of target data which will contain some
 * info about the device capabilities and "execution context" (for now
 * execution context can be SVML function addresses), this needed by the
 * Compiler in order to generate the JIT and link it with the addresses
 * given from the device.
 */

class SDEBackendOptions : public CPUBackendOptions {
public:
  SDEBackendOptions() : m_targetDescSize(0), m_pTargetDesc(NULL) {}

  virtual ~SDEBackendOptions() { delete[] m_pTargetDesc; }

  SDEBackendOptions(const SDEBackendOptions &options) { copy(options); }

  SDEBackendOptions &operator=(const SDEBackendOptions &options) {
    delete[] m_pTargetDesc;
    copy(options);
    return *this;
  }

  virtual void InitTargetDescriptionSession(
      ICLDevBackendExecutionService *pExecutionService) override {
    m_targetDescSize = pExecutionService->GetTargetMachineDescriptionSize();

    if (0 != m_targetDescSize) {
      m_pTargetDesc = new char[m_targetDescSize];
      pExecutionService->GetTargetMachineDescription(m_pTargetDesc,
                                                     m_targetDescSize);
    }
  }

  virtual const char *GetStringValue(int optionId,
                                     const char *defaultValue) const override {
    return CPUBackendOptions::GetStringValue(optionId, defaultValue);
  }

  virtual int GetIntValue(int optionId, int defaultValue) const override {
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_DEVICE:
      return CPU_DEVICE;
    case CL_DEV_BACKEND_OPTION_TARGET_DESC_SIZE:
      return m_targetDescSize;
    default:
      return CPUBackendOptions::GetIntValue(optionId, defaultValue);
    }
  }

  virtual bool GetValue(int optionId, void *Value,
                        size_t *pSize) const override {
    if (Value == NULL) {
      throw Exception::InvalidArgument("Value is not initialized");
    }
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_TARGET_DESC_BLOB:
      if (*pSize < m_targetDescSize)
        return false;
      memcpy(Value, m_pTargetDesc, m_targetDescSize);
      return true;
    default:
      return CPUBackendOptions::GetValue(optionId, Value, pSize);
    }
  }

private:
  void copy(const SDEBackendOptions &options) {
    m_targetDescSize = options.m_targetDescSize;
    m_pTargetDesc = NULL;

    if (0 != m_targetDescSize) {
      m_pTargetDesc = new char[m_targetDescSize];
      memcpy(m_pTargetDesc, options.m_pTargetDesc, m_targetDescSize);
    }
  }

private:
  size_t m_targetDescSize;
  char *m_pTargetDesc;
};

/**
 * Options used during program code container dump
 */
class ProgramDumpConfig : public ICLDevBackendOptions {
public:
  ProgramDumpConfig(const BERunOptions *runConfig) {
    m_fileName =
        runConfig->GetValue<std::string>(RC_BR_DUMP_OPTIMIZED_LLVM_IR, "-");
  }

  bool GetBooleanValue(int optionId, bool defaultValue) const override {
    return defaultValue;
  }

  virtual int GetIntValue(int optionId, int defaultValue) const override {
    return defaultValue;
  }

  virtual const char *GetStringValue(int optionId,
                                     const char *defaultValue) const override {
    if (CL_DEV_BACKEND_OPTION_DUMPFILE != optionId) {
      return defaultValue;
    }

    return m_fileName.c_str();
  }

  virtual bool GetValue(int optionId, void *Value,
                        size_t *pSize) const override {
    return false;
  }

private:
  std::string m_fileName;
};

/**
 * Options used during program jit code container dump
 */
class ProgramJitDumpConfig : public ICLDevBackendOptions {
public:
  ProgramJitDumpConfig(std::string fileName) { m_fileName = fileName; }

  bool GetBooleanValue(int optionId, bool defaultValue) const override {
    return defaultValue;
  }

  virtual int GetIntValue(int optionId, int defaultValue) const override {
    return defaultValue;
  }

  virtual const char *GetStringValue(int optionId,
                                     const char *defaultValue) const override {
    if (CL_DEV_BACKEND_OPTION_DUMPFILE != optionId) {
      return defaultValue;
    }

    return m_fileName.c_str();
  }

  virtual bool GetValue(int optionId, void *Value,
                        size_t *pSize) const override {
    return false;
  }

private:
  std::string m_fileName;
};

/**
 * Options used during program building
 */
class BuildProgramOptions : public ICLDevBackendOptions {
public:
  BuildProgramOptions()
      : m_pInjectedObjectStart(NULL), m_bStopBeforeJIT(false) {}

  void SetInjectedObject(const char *pInjectedObjectStart,
                         size_t injectedObjectSize) {
    m_pInjectedObjectStart = pInjectedObjectStart;
    m_injectedObjectSize = injectedObjectSize;
  }

  const char *GetStringValue(int optionId,
                             const char *defaultValue) const override {
    return defaultValue;
  }

  bool GetBooleanValue(int optionId, bool defaultValue) const override {
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_STOP_BEFORE_JIT:
      return m_bStopBeforeJIT;
    default:
      return defaultValue;
    }
  }

  int GetIntValue(int optionId, int defaultValue) const override {
    return defaultValue;
  }

  bool GetValue(int optionId, void *Value, size_t *pSize) const override {
    if (Value == NULL) {
      throw Exception::InvalidArgument("Value is not initialized");
    }

    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_INJECTED_OBJECT:
      *(static_cast<const char **>(Value)) = m_pInjectedObjectStart;
      *pSize = m_injectedObjectSize;
      return true;
    default:
      return false;
    }
  }
  void SetStopBeforeJIT() { m_bStopBeforeJIT = true; }

private:
  const char *m_pInjectedObjectStart;
  size_t m_injectedObjectSize;
  bool m_bStopBeforeJIT;
};

} // namespace Validation

#endif // BACKEND_OPTIONS_H
