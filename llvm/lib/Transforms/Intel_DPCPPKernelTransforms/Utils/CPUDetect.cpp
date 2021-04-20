//===- CPUDetect.h - CPU feature detection utilities ----------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "CPUDetect.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Host.h"

#include <map>

using namespace llvm;

static const std::map<int, std::string> FeatureMap = {
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

bool CPUDetect::IsFeatureSupported(ECPUFeatureSupport featureType) const {
  auto iter = FeatureMap.find(featureType);
  if (iter == FeatureMap.end())
    return false;
  return CpuFeatures.lookup(iter->second);
}

CPUDetect::CPUDetect(ECPU cpuId,
                     const SmallVectorImpl<std::string> &forcedFeatures,
                     bool is64BitOS)
    : Is64BitOS(is64BitOS), CpuArch(cpuId) {
  CPUString = GetCPUName(CpuArch);

  CpuFeatures["sse2"] = true;

  // Add standard features
  if (cpuId >= (unsigned int)GetCPUByName("core2")) {
    CpuFeatures["sse3"] = true;
    CpuFeatures["ssse3"] = true;
  }

  if (cpuId >= (unsigned int)GetCPUByName("corei7")) {
    CpuFeatures["sse4.1"] = true;
    CpuFeatures["sss4.2"] = true;
  }

  if (cpuId >= (unsigned int)GetCPUByName("corei7-avx")) {
    CpuFeatures["avx"] = true;
  }

  if (cpuId >= (unsigned int)GetCPUByName("core-avx2")) {
    CpuFeatures["avx"] = true;
    CpuFeatures["avx2"] = true;
    CpuFeatures["fma"] = true;
    CpuFeatures["bmi"] = true;
    CpuFeatures["bmi2"] = true;
  }

  // Add forced features
  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+sse4.1") !=
      forcedFeatures.end()) {
    CpuFeatures["sse4.1"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx2") !=
      forcedFeatures.end()) {
    CpuFeatures["avx"] = true;
    CpuFeatures["avx2"] = true;
    CpuFeatures["fma"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx") !=
      forcedFeatures.end()) {
    CpuFeatures["avx"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512f") !=
      forcedFeatures.end()) {
    CpuFeatures["avx512f"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512bw") !=
      forcedFeatures.end()) {
    CpuFeatures["avx512bw"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512cd") !=
      forcedFeatures.end()) {
    CpuFeatures["avx512cd"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512dq") !=
      forcedFeatures.end()) {
    CpuFeatures["avx512dq"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512er") !=
      forcedFeatures.end()) {
    CpuFeatures["avx512er"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512pf") !=
      forcedFeatures.end()) {
    CpuFeatures["avx512pf"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512vl") !=
      forcedFeatures.end()) {
    CpuFeatures["avx512vl"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512vbmi") !=
      forcedFeatures.end()) {
    CpuFeatures["avx512vbmi"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512ifma") !=
      forcedFeatures.end()) {
    CpuFeatures["avx512ifma"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(),
                "+avx512bitalg") != forcedFeatures.end()) {
    CpuFeatures["avx512bitalg"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx512vbmi2") !=
      forcedFeatures.end()) {
    CpuFeatures["avx512vbmi2"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(),
                "+avx512vpopcntdq") != forcedFeatures.end()) {
    CpuFeatures["avx512vpopcntdq"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+clwb") !=
      forcedFeatures.end()) {
    CpuFeatures["clwb"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+wbnoinvd") !=
      forcedFeatures.end()) {
    CpuFeatures["wbnoinvd"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+amx-tile") !=
      forcedFeatures.end()) {
    CpuFeatures["amx-tile"] = true;
  }
  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+amx-int8") !=
      forcedFeatures.end()) {
    CpuFeatures["amx-int8"] = true;
  }
  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+amx-bf16") !=
      forcedFeatures.end()) {
    CpuFeatures["amx-bf16"] = true;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "-sse4.1") !=
      forcedFeatures.end()) {
    CpuFeatures["sse4.1"] = false;
    CpuFeatures["sse4.2"] = false;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "-avx2") !=
      forcedFeatures.end()) {
    CpuFeatures["avx2"] = false;
    CpuFeatures["fma"] = false;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "-avx") !=
      forcedFeatures.end()) {
    CpuFeatures["avx1"] = false;
    CpuFeatures["avx2"] = false;
    CpuFeatures["fma"] = false;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "-fma") !=
      forcedFeatures.end()) {
    CpuFeatures["fma"] = false;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "-bmi") !=
      forcedFeatures.end()) {
    CpuFeatures["bmi"] = false;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "-bmi2") !=
      forcedFeatures.end()) {
    CpuFeatures["bmi2"] = false;
  }
}

CPUDetect *CPUDetect::GetInstance() {
  static CPUDetect instance;

  return &instance;
}

CPUDetect::CPUDetect(void) : Is64BitOS(sizeof(void *) == 8) {
  CPUString = llvm::sys::getHostCPUName().str();
  llvm::sys::getHostCPUFeatures(CpuFeatures);
  CpuArch = StringSwitch<ECPU>(CPUString)
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
