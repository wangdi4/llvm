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

#include "cpu_config.h"
#include "cl_cpu_detect.h"
#include "cl_env.h"
#include "ocl_supported_extensions.h"
#include "opencl_c_features.h"

#include <algorithm>
#include <sstream>

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::CPUDevice;
using namespace Intel::OpenCL::DeviceBackend;

std::mutex CPUDeviceConfig::m_mutex;
std::string CPUDeviceConfig::m_extensionsName;
std::string CPUDeviceConfig::m_OpenCLCFeatureNames;
std::vector<cl_name_version> CPUDeviceConfig::m_extensions;
std::vector<cl_name_version> CPUDeviceConfig::m_c_features;

CPUDeviceConfig::CPUDeviceConfig() {
  // BasicCLConfigWrapper
}

CPUDeviceConfig::~CPUDeviceConfig() {
  // ~BasicCLConfigWrapper
}

cl_err_code CPUDeviceConfig::Initialize(std::string filename) {
  return BasicCLConfigWrapper::Initialize(filename);
}

int CPUDeviceConfig::GetNumDevices() const {
  if (FPGA_EMU_DEVICE == this->GetDeviceMode()) {
    return m_pConfigFile->Read<int>(CL_CONFIG_CPU_EMULATE_DEVICES, 1);
  }
  return 1;
}

cl_ulong CPUDeviceConfig::GetForcedGlobalMemSize() const {
  std::string strForcedSize;
  if (!m_pConfigFile->ReadInto(strForcedSize,
                               CL_CONFIG_CPU_FORCE_GLOBAL_MEM_SIZE)) {
    return 0;
  }

  return ParseStringToSize(strForcedSize);
}

cl_ulong CPUDeviceConfig::GetForcedMaxMemAllocSize() const {
  std::string strForcedSize;
  if (!m_pConfigFile->ReadInto(strForcedSize,
                               CL_CONFIG_CPU_FORCE_MAX_MEM_ALLOC_SIZE)) {
    return 0;
  }

  return ParseStringToSize(strForcedSize);
}

cl_int CPUDeviceConfig::GetVectorizerMode() const {
  return m_pConfigFile->Read(CL_CONFIG_CPU_VECTORIZER_MODE,
                             static_cast<int32_t>(TRANSPOSE_SIZE_NOT_SET));
}

VectorizerType CPUDeviceConfig::GetVectorizerType() const {
  std::string VType =
      m_pConfigFile->Read<string>(CL_CONFIG_CPU_VECTORIZER_TYPE, "default");
  std::transform(VType.begin(), VType.end(), VType.begin(), ::tolower);

  if ("vpo" == VType) {
    return VPO_VECTORIZER;
  }
  return DEFAULT_VECTORIZER;
}

PassManagerType CPUDeviceConfig::GetPassManagerType() const {
  std::string PMType = m_pConfigFile->Read<string>("CL_CONFIG_LTO_PM", "");
  std::transform(PMType.begin(), PMType.end(), PMType.begin(), ::tolower);
  if ("new" == PMType)
    return PM_LTO;
  else if ("ocl" == PMType)
    return PM_OCL;
  return PM_NONE;
}

bool CPUDeviceConfig::IsSpirSupported() const { return true; }

bool CPUDeviceConfig::IsHalfSupported() const {
  return FPGA_EMU_DEVICE != GetDeviceMode();
}

bool CPUDeviceConfig::IsDoubleSupported() const {
  // Keep original logic
  // disabled on Atom
  if (BRAND_INTEL_ATOM == CPUDetect::GetInstance()->GetHostCPUBrandFamily())
    return false;

  // enabled on non-Atom brands
  if (BRAND_UNKNOWN != CPUDetect::GetInstance()->GetHostCPUBrandFamily())
    return true;

  // if we can't detect brand, fallback to AVX support check
  // enabled on CPUs with AVX support
  bool isAVXSupported =
      CPUDetect::GetInstance()->IsFeatureSupportedOnHost(CFS_AVX10);
  if (isAVXSupported)
    return true;

  // enabled on Westmere
  if (CPUDetect::GetInstance()->isWestmereForHost())
    return true;

  // disabled in any other case
  return false;
}

static void appendExtNameVer(const char *Name, unsigned Major, unsigned Minor,
                             unsigned Patch, std::string &ExtNames,
                             std::vector<cl_name_version> &ExtNameVerions) {
  if (!ExtNames.empty())
    ExtNames.append(" ");
  ExtNames.append(Name);
  cl_name_version NameVer;
  NameVer.version = CL_MAKE_VERSION(Major, Minor, Patch);
  assert(strlen(Name) < CL_NAME_VERSION_MAX_NAME_SIZE &&
         "Extension name size invalid");
  STRNCPY_S(NameVer.name, CL_NAME_VERSION_MAX_NAME_SIZE, Name,
            strlen(Name) + 1);
  ExtNameVerions.emplace_back(NameVer);
}

const char *CPUDeviceConfig::GetExtensions() {
  std::ignore = GetExtensionsWithVersion();
  return m_extensionsName.c_str();
}

const std::vector<cl_name_version> &
CPUDeviceConfig::GetExtensionsWithVersion() {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (!m_extensions.empty())
    return m_extensions;

  m_extensionsName.reserve(1024);
  m_extensions.reserve(48);

  auto GetExtVer = [&](const char *Name, unsigned Major, unsigned Minor,
                       unsigned Patch) {
    appendExtNameVer(Name, Major, Minor, Patch, m_extensionsName, m_extensions);
  };

  GetExtVer(OCL_EXT_KHR_SPIRV_LINKONCE_ODR, 1, 0, 0);

  // double floating point extension
  if (IsDoubleSupported())
    GetExtVer(OCL_EXT_KHR_FP64, 1, 0, 0);

  // half floating point extension
  if (IsHalfSupported())
    GetExtVer(OCL_EXT_KHR_FP16, 1, 0, 0);

  GetExtVer(OCL_EXT_KHR_GLOBAL_BASE_ATOMICS, 1, 0, 0);
  GetExtVer(OCL_EXT_KHR_GLOBAL_EXTENDED_ATOMICS, 1, 0, 0);
  GetExtVer(OCL_EXT_KHR_LOCAL_BASE_ATOMICS, 1, 0, 0);
  GetExtVer(OCL_EXT_KHR_LOCAL_EXTENDED_ATOMICS, 1, 0, 0);

  GetExtVer(OCL_EXT_KHR_3D_IMAGE_WRITES, 1, 0, 0);
  GetExtVer(OCL_EXT_KHR_BYTE_ADDRESSABLE_STORE, 1, 0, 0);
  GetExtVer(OCL_EXT_KHR_DEPTH_IMAGES, 1, 0, 0);
  GetExtVer(OCL_EXT_KHR_ICD, 1, 0, 0);
  GetExtVer(OCL_EXT_KHR_IL_PROGRAM, 1, 0, 0);

  // USM extension for both ocl cpu rt and fpga emu rt
  GetExtVer(OCL_EXT_INTEL_UNIFIED_SHARED_MEMORY, 1, 0, 0);

  if (FPGA_EMU_DEVICE == GetDeviceMode()) {
    GetExtVer(OCL_EXT_INTEL_FPGA_HOST_PIPE, 1, 0, 0);
    GetExtVer(OCL_EXT_INTEL_PROGRAM_SCOPE_HOST_PIPE, 1, 0, 0);
    GetExtVer(OCL_EXT_ES_KHR_INT64, 1, 0, 0);

    // In FPGA HW: Since some of the extensions are either partially supported,
    // or are not yet conformant (have not been tested or do not pass
    // conformance tests), what we claim to support in the platform/device
    // queries should be different from what we allow and have implemented.
    // We're aligning with it's behavior in FPGA emulator.
    GetExtVer(OCL_EXT_INTEL_CHANNELS, 1, 0, 0);
    return m_extensions;
  }

  // FIXME: Re-claim cl_khr_subgroup_ballot support when we implement all
  // required builtins.
  // GetExtVer(OCL_EXT_KHR_SUBGROUP_BALLOT, 1, 0, 0);

  GetExtVer(OCL_EXT_KHR_SUBGROUP_SHUFFLE, 1, 0, 0);
  GetExtVer(OCL_EXT_KHR_SUBGROUP_SHUFFLE_RELATIVE, 1, 0, 0);
  GetExtVer(OCL_EXT_KHR_SUBGROUP_EXTENDED_TYPES, 1, 0, 0);
  GetExtVer(OCL_EXT_KHR_SUBGROUP_NON_UNIFORM_ARITHMETIC, 1, 0, 0);

  GetExtVer(OCL_EXT_INTEL_SUBGROUPS, 1, 0, 0);
  GetExtVer(OCL_EXT_INTEL_SUBGROUPS_CHAR, 1, 0, 0);
  GetExtVer(OCL_EXT_INTEL_SUBGROUPS_SHORT, 1, 0, 0);
  GetExtVer(OCL_EXT_INTEL_SUBGROUPS_LONG, 1, 0, 0);
  GetExtVer(OCL_EXT_INTEL_SUBGROUPS_REQD_SIZE, 1, 0, 0);
  GetExtVer(OCL_EXT_INTEL_SPIRV_SUBGROUPS, 1, 0, 0);

  // build the extensions list dynamically
  // common KHR extensions
  GetExtVer(OCL_EXT_KHR_INT64_BASE_ATOMICS, 1, 0, 0);
  GetExtVer(OCL_EXT_KHR_INT64_EXTENDED_ATOMICS, 1, 0, 0);

  // common Intel extensions
  GetExtVer(OCL_EXT_INTEL_DEVICE_ATTRIBUTE_QUERY, 1, 0, 0);

  // INTEL CPU execlusive extensions
  GetExtVer(OCL_EXT_INTEL_EXEC_BY_LOCAL_THREAD, 1, 0, 0);
  GetExtVer(OCL_EXT_INTEL_VEC_LEN_HINT, 1, 0, 0);
#ifndef _WIN32
  GetExtVer(OCL_EXT_INTEL_DEVICE_PARTITION_BY_NAMES, 1, 0, 0);
#endif
  // SPIR extension
  if (IsSpirSupported())
    GetExtVer(OCL_EXT_KHR_SPIR, 1, 0, 0);

  // OpenCL 2.0 extensions
  if (OPENCL_VERSION_2_0 <= GetOpenCLVersion()) {
    GetExtVer(OCL_EXT_KHR_IMAGE2D_FROM_BUFFER, 1, 0, 0);
    GetExtVer(OCL_EXT_INTEL_CONCURRENT_DISPATCH, 1, 0, 0);
  }

  return m_extensions;
}

const char *CPUDeviceConfig::GetOpenCLCFeatures() {
  std::ignore = GetOpenCLCFeaturesWithVersion();
  return m_OpenCLCFeatureNames.c_str();
}

const std::vector<cl_name_version> &
CPUDeviceConfig::GetOpenCLCFeaturesWithVersion() {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (!m_c_features.empty())
    return m_c_features;

  m_OpenCLCFeatureNames.reserve(1024);
  m_c_features.reserve(32);

  auto GetFeature = [&](const char *Name, unsigned Major, unsigned Minor,
                        unsigned Patch) {
    appendExtNameVer(Name, Major, Minor, Patch, m_OpenCLCFeatureNames,
                     m_c_features);
  };

  GetFeature(OPENCL_C_3D_IMAGE_WRITES, 3, 0, 0);
  GetFeature(OPENCL_C_ATOMIC_ORDER_ACQ_REL, 3, 0, 0);
  GetFeature(OPENCL_C_ATOMIC_ORDER_SEQ_CST, 3, 0, 0);
  GetFeature(OPENCL_C_ATOMIC_SCOPE_DEVICE, 3, 0, 0);
  GetFeature(OPENCL_C_ATOMIC_SCOPE_ALL_DEVICES, 3, 0, 0);
  GetFeature(OPENCL_C_DEVICE_ENQUEUE, 3, 0, 0);
  GetFeature(OPENCL_C_GENERIC_ADDRESS_SPACE, 3, 0, 0);
  GetFeature(OPENCL_C_FP64, 3, 0, 0);
  GetFeature(OPENCL_C_IMAGES, 3, 0, 0);
  GetFeature(OPENCL_C_INT64, 3, 0, 0);
  GetFeature(OPENCL_C_PIPES, 3, 0, 0);
  GetFeature(OPENCL_C_PROGRAM_SCOPE_GLOBAL_VARIABLES, 3, 0, 0);
  GetFeature(OPENCL_C_READ_WRITE_IMAGES, 3, 0, 0);
  GetFeature(OPENCL_C_SUBGROUPS, 3, 0, 0);
  GetFeature(OPENCL_C_WORK_GROUP_COLLECTIVE_FUNCTIONS, 3, 0, 0);

  return m_c_features;
}
