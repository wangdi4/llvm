// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
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

#include "cl_cpu_detect.h"
#include "cl_env.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Host.h"

#include <map>

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::DeviceBackend;

const std::map<int, std::string> FeatureMap = {
    {CFS_NONE, "none"},
    {CFS_SSE2, "sse2"},
    {CFS_SSE3, "sse3"},
    {CFS_SSSE3, "ssse3"},
    {CFS_SSE41, "sse4.1"},
    {CFS_SSE42, "sse4.2"},
    {CFS_AVX10, "avx"},
    {CFS_AVX20, "avx2"},
    {CFS_FMA, "fma"},
    {CFS_BMI, "bmi"},
    {CFS_BMI2, "bmi2"},
    {CFS_AVX512F, "avx512f"},
    {CFS_AVX512CD, "avx512cd"},
    {CFS_AVX512ER, "avx512er"},
    {CFS_AVX512PF, "avx512pf"},
    {CFS_AVX512BW, "avx512bw"},
    {CFS_AVX512DQ, "avx512dq"},
    {CFS_AVX512VL, "avx512vl"},
    {CFS_AVX512VBMI, "avx512vbmi"},
    {CFS_AVX512IFMA, "avx512ifma"},
    {CFS_AVX512BITALG, "avx512bitalg"},
    {CFS_AVX512VBMI2, "avx512vbmi2"},
    {CFS_AVX512POPCNTDQ, "avx512vpopcntdq"},
    {CFS_CLWB, "clwb"},
    {CFS_WBNOINVD, "wbnoinvd"},
    {CFS_AMXTILE, "amx-tile"},
    {CFS_AMXINT8, "amx-int8"},
    {CFS_AMXBF16, "amx-bf16"},
    {CFS_F16C, "f16c"},
};

cl_err_code Intel::OpenCL::Utils::IsCPUSupported() {
  if (CPUDetect::GetInstance()->IsFeatureSupported(CFS_SSE41)) {
    return CL_SUCCESS;
  }
  return CL_ERR_CPU_NOT_SUPPORTED;
}

TransposeSizeSupport
CPUDetect::isTransposeSizeSupported(ETransposeSize transposeSize) const {
  switch (transposeSize) {
  default:
    return INVALID;

  case TRANSPOSE_SIZE_NOT_SET:
  case TRANSPOSE_SIZE_AUTO:
  case TRANSPOSE_SIZE_1:
    return SUPPORTED;

  case TRANSPOSE_SIZE_4:
    return HasSSE41() ? SUPPORTED : UNSUPPORTED;

  case TRANSPOSE_SIZE_8:
    return HasAVX1() ? SUPPORTED : UNSUPPORTED;

  case TRANSPOSE_SIZE_16:
    return HasGatherScatter() ? SUPPORTED : UNSUPPORTED;
  }
}

bool CPUDetect::IsFeatureSupported(ECPUFeatureSupport featureType) const {
  if (m_bBypassCPUDetect) {
    return true;
  }
  auto iter = FeatureMap.find(featureType);
  if (iter == FeatureMap.end())
    return false;
  return m_cpuFeatures.lookup(iter->second);
}

bool CPUDetect::ShouldBypassCPUCheck() const {
#ifndef NDEBUG
  std::string strVal;
  cl_err_code err = GetEnvVar(strVal, "OCL_CFG_BYPASS_CPU_DETECT");
  if (CL_SUCCEEDED(err)) {
    return true;
  }
#endif
  return false;
}

CPUDetect::CPUDetect(ECPU cpuId,
                     const llvm::SmallVectorImpl<std::string> &forcedFeatures,
                     bool is64BitOS)
    : m_is64BitOS(is64BitOS), m_cpuArch(cpuId) {
  m_bBypassCPUDetect = ShouldBypassCPUCheck();
  m_CPUString = GetCPUName(m_cpuArch);

  m_cpuFeatures["sse2"] = true;

  // Add standard features
  if (cpuId >=
      (unsigned int)Intel::OpenCL::Utils::CPUDetect::GetCPUByName("core2")) {
    m_cpuFeatures["sse3"] = true;
    m_cpuFeatures["ssse3"] = true;
  }

  if (cpuId >=
      (unsigned int)Intel::OpenCL::Utils::CPUDetect::GetCPUByName("corei7")) {
    m_cpuFeatures["sse4.1"] = true;
    m_cpuFeatures["sss4.2"] = true;
  }

  if (cpuId >= (unsigned int)Intel::OpenCL::Utils::CPUDetect::GetCPUByName(
                   "corei7-avx")) {
    m_cpuFeatures["avx"] = true;
  }

  if (cpuId >= (unsigned int)Intel::OpenCL::Utils::CPUDetect::GetCPUByName(
                   "core-avx2")) {
    m_cpuFeatures["avx"] = true;
    m_cpuFeatures["avx2"] = true;
    m_cpuFeatures["fma"] = true;
    m_cpuFeatures["bmi"] = true;
    m_cpuFeatures["bmi2"] = true;
  }
  // Add forced features
  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+sse4.1") !=
      forcedFeatures.end()) {
    m_cpuFeatures["sse4.1"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx2") !=
      forcedFeatures.end()) {
    m_cpuFeatures["avx"] = true;
    m_cpuFeatures["avx2"] = true;
    m_cpuFeatures["fma"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx") !=
      forcedFeatures.end()) {
    m_cpuFeatures["avx"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512f") !=
      forcedFeatures.end()) {
    m_cpuFeatures["avx512f"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512bw") !=
      forcedFeatures.end()) {
    m_cpuFeatures["avx512bw"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512cd") !=
      forcedFeatures.end()) {
    m_cpuFeatures["avx512cd"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512dq") !=
      forcedFeatures.end()) {
    m_cpuFeatures["avx512dq"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512er") !=
      forcedFeatures.end()) {
    m_cpuFeatures["avx512er"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512pf") !=
      forcedFeatures.end()) {
    m_cpuFeatures["avx512pf"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512vl") !=
      forcedFeatures.end()) {
    m_cpuFeatures["avx512vl"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512vbmi") !=
      forcedFeatures.end()) {
    m_cpuFeatures["avx512vbmi"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512ifma") !=
      forcedFeatures.end()) {
    m_cpuFeatures["avx512ifma"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(),
                "+avx512bitalg") != forcedFeatures.end()) {
    m_cpuFeatures["avx512bitalg"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512vbmi2") !=
      forcedFeatures.end()) {
    m_cpuFeatures["avx512vbmi2"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(),
                "+avx512vpopcntdq") != forcedFeatures.end()) {
    m_cpuFeatures["avx512vpopcntdq"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+clwb") !=
      forcedFeatures.end()) {
    m_cpuFeatures["clwb"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+wbnoinvd") !=
      forcedFeatures.end()) {
    m_cpuFeatures["wbnoinvd"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+amx-tile") !=
      forcedFeatures.end()) {
    m_cpuFeatures["amx-tile"] = true;
  }
  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+amx-int8") !=
      forcedFeatures.end()) {
    m_cpuFeatures["amx-int8"] = true;
  }
  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+amx-bf16") !=
      forcedFeatures.end()) {
    m_cpuFeatures["amx-bf16"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "-sse4.1") !=
      forcedFeatures.end()) {
    m_cpuFeatures["sse4.1"] = false;
    m_cpuFeatures["sse4.2"] = false;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "-avx2") !=
      forcedFeatures.end()) {
    m_cpuFeatures["avx2"] = false;
    m_cpuFeatures["fma"] = false;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "-avx") !=
      forcedFeatures.end()) {
    m_cpuFeatures["avx1"] = false;
    m_cpuFeatures["avx2"] = false;
    m_cpuFeatures["fma"] = false;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "-fma") !=
      forcedFeatures.end()) {
    m_cpuFeatures["fma"] = false;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "-bmi") !=
      forcedFeatures.end()) {
    m_cpuFeatures["bmi"] = false;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "-bmi2") !=
      forcedFeatures.end()) {
    m_cpuFeatures["bmi2"] = false;
  }
}

CPUDetect *CPUDetect::GetInstance() {
  static CPUDetect instance;

  return &instance;
}

CPUDetect::CPUDetect(void) : m_is64BitOS(sizeof(void *) == 8) {
  m_bBypassCPUDetect = ShouldBypassCPUCheck();
  m_CPUString = llvm::sys::getHostCPUName().str();
  llvm::sys::getHostCPUFeatures(m_cpuFeatures);
  m_cpuArch = llvm::StringSwitch<ECPU>(m_CPUString)
                  .Case("westmere", CPU_WST)
                  .Case("sandybridge", CPU_SNB)
                  .Case("pentium", CPU_PENTIUM)
                  .Case("nocona", CPU_NOCONA)
                  .Case("core2", CPU_CORE2)
                  .Case("penryn", CPU_PENRYN)
                  .Case("knl", CPU_KNL)
                  .Case("ivybridge", CPU_IVB)
                  .Case("haswell", CPU_HSW)
                  .Case("broadwell", CPU_BDW)
                  .Case("goldmont", CPU_GLK)
                  .Case("rocketlake", CPU_RKL)
                  .Case("skylake", CPU_SKL)
                  .Cases("cooperlake", "cascadelake", "skylake-avx512", CPU_SKX)
                  .Case("cannonlake", CPU_CNL)
                  .Case("icelake-client", CPU_ICL)
                  .Case("icelake-server", CPU_ICX)
                  .Case("alderlake", CPU_ADL)
                  .Case("tigerlake", CPU_TGL)
                  .Case("sapphirerapids", CPU_SPR)
                  .Default(CPU_UNKNOWN);
}
