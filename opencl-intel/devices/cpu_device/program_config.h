// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

#pragma once

#include "PipeCommon.h"
#include "cl_dev_backend_api.h"
#include "cl_device_api.h"
#include "cl_user_logger.h"
#include "cpu_dev_limits.h"
#include "cpu_logger.h"

#include <algorithm>
#include <string>

namespace Intel {
namespace OpenCL {
namespace CPUDevice {

class CPUDeviceConfig;

/**
 * Program options used upon program creation
 */
class ProgramConfig : public DeviceBackend::ICLDevBackendOptions {
public:
  ProgramConfig()
      : m_useVectorizer(false),
        m_vectorizerMode(DeviceBackend::TRANSPOSE_SIZE_NOT_SET),
        m_vectorizerType(DEFAULT_VECTORIZER), m_rtLoopUnrollFactor(0),
        m_useVTune(false), m_serializeWorkGroups(false),
        m_forcedPrivateMemorySize(0), m_useAutoMemory(false),
        m_channelDepthEmulationMode(CHANNEL_DEPTH_MODE_STRICT),
        m_targetDevice(CPU_DEVICE), m_deviceMaxWGSize(CPU_MAX_WORK_GROUP_SIZE),
        m_streamingAlways(false), m_expensiveMemOpts(0),
        m_passManagerType(DeviceBackend::PM_NONE),
        m_subGroupConstructionMode(0) {}

  void InitFromCpuConfig(const CPUDeviceConfig &cpuConfig);

  bool GetBooleanValue(int optionId, bool defaultValue) const override {
    using namespace DeviceBackend;
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_USE_VTUNE:
      return m_useVTune;
    case CL_DEV_BACKEND_OPTION_SERIALIZE_WORK_GROUPS:
      return m_serializeWorkGroups;
    case CL_DEV_BACKEND_OPTION_STREAMING_ALWAYS:
      return m_streamingAlways;
    case CL_DEV_BACKEND_OPTION_USE_AUTO_MEMORY:
      return m_useAutoMemory;
    default:
      return defaultValue;
    }
  }

  virtual int GetIntValue(int optionId, int defaultValue) const override {
    using namespace DeviceBackend;
    switch (optionId) {
    case CL_DEV_BACKEND_OPTION_DEVICE:
      return m_targetDevice;
    case CL_DEV_BACKEND_OPTION_CPU_MAX_WG_SIZE:
      return (int)m_deviceMaxWGSize;
    case CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE:
      // The transpoze size is applicable only then
      // CL_CONFIG_USE_VECTORIZER is false.
      return m_useVectorizer ? m_vectorizerMode : TRANSPOSE_SIZE_1;
    case CL_DEV_BACKEND_OPTION_RT_LOOP_UNROLL_FACTOR:
      return std::max(1, std::min(16, m_rtLoopUnrollFactor));
    case CL_DEV_BACKEND_OPTION_FORCED_PRIVATE_MEMORY_SIZE:
      return m_forcedPrivateMemorySize;
    case CL_DEV_BACKEND_OPTION_CHANNEL_DEPTH_EMULATION_MODE:
      return m_channelDepthEmulationMode;
    case CL_DEV_BACKEND_OPTION_VECTORIZER_TYPE:
      return m_vectorizerType;
    case CL_DEV_BACKEND_OPTION_EXPENSIVE_MEM_OPTS:
      return m_expensiveMemOpts;
    case CL_DEV_BACKEND_OPTION_PASS_MANAGER_TYPE:
      return (int)m_passManagerType;
    case CL_DEV_BACKEND_OPTION_SUB_GROUP_CONSTRUCTION:
      return m_subGroupConstructionMode;
    default:
      return defaultValue;
    }
  }

  virtual const char *GetStringValue(int optionId,
                                     const char *defaultValue) const override {
    return defaultValue;
  }

  virtual bool GetValue(int /*optionId*/, void * /*Value*/,
                        size_t * /*pSize*/) const override {
    return false;
  }

private:
  bool m_useVectorizer;
  int m_vectorizerMode;
  VectorizerType m_vectorizerType;
  int m_rtLoopUnrollFactor;
  bool m_useVTune;
  bool m_serializeWorkGroups;
  int m_forcedPrivateMemorySize;
  bool m_useAutoMemory;
  int m_channelDepthEmulationMode;
  DeviceMode m_targetDevice;
  size_t m_deviceMaxWGSize;
  bool m_streamingAlways;
  unsigned m_expensiveMemOpts;
  DeviceBackend::PassManagerType m_passManagerType;
  int m_subGroupConstructionMode;
};

/**
 * Options used during program code container dump
 */
class ProgramDumpConfig : public DeviceBackend::ICLDevBackendOptions {
public:
  ProgramDumpConfig(const char *options) { InitFromString(options); }

  void InitFromString(const char *options);

  bool GetBooleanValue(int /*optionId*/, bool defaultValue) const override {
    return defaultValue;
  }

  virtual int GetIntValue(int /*optionId*/, int defaultValue) const override {
    return defaultValue;
  }

  virtual const char *GetStringValue(int optionId,
                                     const char *defaultValue) const override {
    if (DeviceBackend::CL_DEV_BACKEND_OPTION_DUMPFILE != optionId) {
      return defaultValue;
    }

    return m_fileName.c_str();
  }

  virtual bool GetValue(int /*optionId*/, void * /*Value*/,
                        size_t * /*pSize*/) const override {
    return false;
  }

private:
  std::string m_fileName;
};

} // namespace CPUDevice
} // namespace OpenCL
} // namespace Intel
