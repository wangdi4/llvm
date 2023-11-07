// INTEL CONFIDENTIAL
//
// Copyright 2007 Intel Corporation.
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
#include "hw_utils.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/TargetParser/Host.h"
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
    {CFS_AVX512BW, "avx512bw"},
    {CFS_AVX512DQ, "avx512dq"},
    {CFS_AVX512VL, "avx512vl"},
    {CFS_PKU, "pku"},
    {CFS_CLWB, "clwb"},
    {CFS_AVX512VNNI, "avx512vnni"},
    {CFS_AVX512VBMI, "avx512vbmi"},
    {CFS_AVX512IFMA, "avx512ifma"},
    {CFS_AVX512BITALG, "avx512bitalg"},
    {CFS_AVX512VBMI2, "avx512vbmi2"},
    {CFS_AVX512VPOPCNTDQ, "avx512vpopcntdq"},
    {CFS_WBNOINVD, "wbnoinvd"},
    {CFS_PCONFIG, "pconfig"},
    {CFS_AMXTILE, "amx-tile"},
    {CFS_AMXINT8, "amx-int8"},
    {CFS_AMXBF16, "amx-bf16"},
    {CFS_AVX512FP16, "avx512fp16"},
    {CFS_AVX512BF16, "avx512bf16"},
    {CFS_F16C, "f16c"},
    {CFS_AMXFP16, "amx-fp16"},
    {CFS_PREFETCHI, "prefetchi"},
};

const std::vector<std::string> CPUDetect::CPUArchStr = {
    "CPU_UNKNOWN",
#define CREATE_STRINGS(name) #name,
    CPU_ARCHS(CREATE_STRINGS)
#undef CREATE_STRINGS
};

cl_err_code Intel::OpenCL::Utils::IsCPUSupported() {
  if (CPUDetect::GetInstance()->IsFeatureSupported(CFS_SSE41)) {
    return CL_SUCCESS;
  }
  return CL_ERR_CPU_NOT_SUPPORTED;
}

std::string CPUId::str() const {
  return ("CPU: " + llvm::Twine((int)m_CPU) +
          ", CPUFeatures: " + llvm::Twine(m_CPUFeatures) +
          ", m_is64BitOS: " + llvm::Twine(m_is64BitOS))
      .str();
}

TransposeSizeSupport
CPUDetect::isTransposeSizeSupported(ETransposeSize transposeSize) const {
  switch (transposeSize) {
  case TRANSPOSE_SIZE_NOT_SET:
  case TRANSPOSE_SIZE_AUTO:
  case TRANSPOSE_SIZE_1:
  case TRANSPOSE_SIZE_4:
  case TRANSPOSE_SIZE_8:
  case TRANSPOSE_SIZE_16:
  case TRANSPOSE_SIZE_32:
  case TRANSPOSE_SIZE_64:
    return SUPPORTED;
  case TRANSPOSE_SIZE_INVALID:
    return INVALID;
  }
  llvm_unreachable("Invalid transpose size!");
}

static bool featureQueryHelper(const llvm::StringMap<bool> &CPUFeatures,
                               ECPUFeatureSupport featureType) {
  auto iter = FeatureMap.find(featureType);
  if (iter == FeatureMap.end())
    return false;
  return CPUFeatures.lookup(iter->second);
}

// Check if given feature is supported for host CPU
bool CPUDetect::IsFeatureSupportedOnHost(ECPUFeatureSupport featureType) const {
  return featureQueryHelper(m_HostCPUFeatures, featureType);
}

// Check if given feature is supported for the CPU which can be host CPU or a
// customization CPU by config
bool CPUDetect::IsFeatureSupported(ECPUFeatureSupport featureType) const {
  return featureQueryHelper(m_CPUFeatures, featureType);
}

// This is to push disabled features in list which will be passed to code
// generator later.
void CPUDetect::GetDisabledCPUFeatures(
    llvm::SmallVector<std::string, 8> &forcedFeatures) {
  for (auto &F : m_CPUFeatures)
    if (!F.second)
      forcedFeatures.push_back("-" + F.first().str());
}

CPUId CPUDetect::GetCPUIdForKernelPropertiesSerialize() const {
  bool hasAVX1 = IsFeatureSupported(CFS_AVX10);
  bool hasAVX2 = IsFeatureSupported(CFS_AVX20);
  return CPUId(m_CPUArch, hasAVX1, hasAVX2, m_is64BitOS);
}

// This is to reset CPU according to ICompilerConfig when
// CL_CONFIG_CPU_TARGET_ARCH env is set
void CPUDetect::ResetCPU(
    ECPU cpuId, const llvm::SmallVectorImpl<std::string> &forcedFeatures) {
  // Set CPU arch according to config
  m_CPUArch = cpuId;
  m_CPUString = GetCPUName(m_CPUArch);
  // Clear feature map firstly
  m_CPUFeatures.clear();
  m_CPUFeatures["sse2"] = true;

  // Add standard features
  if (cpuId >= Intel::OpenCL::Utils::CPUDetect::GetCPUByName("core2")) {
    m_CPUFeatures["sse3"] = true;
    m_CPUFeatures["ssse3"] = true;
  }

  if (cpuId >= Intel::OpenCL::Utils::CPUDetect::GetCPUByName("corei7")) {
    m_CPUFeatures["sse4.1"] = true;
    m_CPUFeatures["sse4.2"] = true;
  }

  if (cpuId >= Intel::OpenCL::Utils::CPUDetect::GetCPUByName("corei7-avx"))
    m_CPUFeatures["avx"] = true;

  if (cpuId >= Intel::OpenCL::Utils::CPUDetect::GetCPUByName("core-avx2")) {
    m_CPUFeatures["avx"] = true;
    m_CPUFeatures["avx2"] = true;
    m_CPUFeatures["fma"] = true;
    m_CPUFeatures["bmi"] = true;
    m_CPUFeatures["bmi2"] = true;
  }

  // Add forced features
  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "+avx2") !=
      forcedFeatures.end()) {
    m_CPUFeatures["avx"] = true;
    m_CPUFeatures["avx2"] = true;
    m_CPUFeatures["fma"] = true;
  }

  for (auto &feature : forcedFeatures) {
    assert(feature[0] == '+' ||
           feature[0] == '-' && "feature doesn't have valid prefix");
    m_CPUFeatures[feature.substr(1)] = (feature[0] == '+');
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "-sse4.1") !=
      forcedFeatures.end()) {
    m_CPUFeatures["sse4.1"] = false;
    m_CPUFeatures["sse4.2"] = false;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "-avx2") !=
      forcedFeatures.end()) {
    m_CPUFeatures["avx2"] = false;
    m_CPUFeatures["fma"] = false;
  }

  if (std::find(forcedFeatures.begin(), forcedFeatures.end(), "-avx") !=
      forcedFeatures.end()) {
    m_CPUFeatures["avx1"] = false;
    m_CPUFeatures["avx2"] = false;
    m_CPUFeatures["fma"] = false;
  }
}

CPUDetect *CPUDetect::Instance = nullptr;
CPUDetect::Deleter CPUDetect::InstanceDeleter{};

CPUDetect *CPUDetect::GetInstance() {
  static CPUDetect *S = [] {
    Instance = new CPUDetect;
    return Instance;
  }();

  return S;
}

CPUDetect::CPUDetect(void) : m_is64BitOS(sizeof(void *) == 8) {
  m_HostCPUString = llvm::sys::getHostCPUName().str();
#ifdef INTEL_PRODUCT_RELEASE
  if (m_HostCPUString == "generic")
    reportWarning("Unknown host CPU.");
#endif // INTEL_PRODUCT_RELEASE
  assert(m_HostCPUString != "generic" && "Unknown host CPU.");
  llvm::sys::getHostCPUFeatures(m_HostCPUFeatures);
  GetHostCPUBrandInfo();
  // If CL_CONFIG_CPU_TARGET_ARCH is not used, m_CPUFeatures and
  // m_HostCPUFeatures should be same
  m_CPUFeatures = m_HostCPUFeatures;
  m_CPUString = m_HostCPUString;
  // Detect CPU enum according to cpu features
  m_CPUArch = CPU_UNKNOWN;
  if (HasSSE42())
    m_CPUArch = CPU_COREI7;

  if (HasAVX1())
    m_CPUArch = CPU_SNB;

  if (HasAVX2())
    m_CPUArch = CPU_HSW;

  if (HasAVX512SKX())
    m_CPUArch = CPU_SKX;

  if (HasAVX512CLX())
    m_CPUArch = CPU_CLX;

  if (HasAVX512ICL())
    m_CPUArch = CPU_ICL;

  if (HasAVX512ICX())
    m_CPUArch = CPU_ICX;

  if (HasSPR())
    m_CPUArch = CPU_SPR;

  if (HasGNR())
    m_CPUArch = CPU_GNR;

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CPU_DMR
  if (HasDMR())
    m_CPUArch = CPU_DMR;
#endif // INTEL_FEATURE_CPU_DMR
#endif // INTEL_CUSTOMIZATION

  if (m_CPUArch == CPU_UNKNOWN) {
    std::string errMessage = m_CPUString + ": Unsupported CPU!";
    llvm_unreachable(errMessage.data());
  }
}

// Get Host CPU brand info, this function is used on cpu_device, cpu_config and
// framework
void CPUDetect::GetHostCPUBrandInfo() {
  char vcCPUBrandString[0x40] = {0};
  unsigned int viCPUInfo[4] = {(unsigned int)-1};

  CPUID(viCPUInfo, 0);
  CPUID(viCPUInfo, 1);
  CPUID(viCPUInfo, 0x80000000);
  unsigned int iValidExIDs = viCPUInfo[0];

  if (iValidExIDs >= 0x80000004) {
    for (unsigned int i = 0x80000000; i <= iValidExIDs; ++i) {
      CPUID(viCPUInfo, i);

      // Interpret CPU brand string.
      if (i == 0x80000002) {
        MEMCPY_S(vcCPUBrandString, sizeof(vcCPUBrandString), viCPUInfo,
                 sizeof(viCPUInfo));
      } else if (i == 0x80000003) {
        MEMCPY_S(vcCPUBrandString + 16, sizeof(vcCPUBrandString) - 16,
                 viCPUInfo, sizeof(viCPUInfo));
      } else if (i == 0x80000004) {
        MEMCPY_S(vcCPUBrandString + 32, sizeof(vcCPUBrandString) - 32,
                 viCPUInfo, sizeof(viCPUInfo));
      }
    }
    m_HostCPUBrandString = vcCPUBrandString;
  }

  // detect CPU brand
  if (!m_HostCPUBrandString.empty()) {
    if (m_HostCPUBrandString.find("Intel(R) Core(TM)") !=
        m_HostCPUBrandString.npos) {
      m_HostCPUBrand = BRAND_INTEL_CORE;
    } else if (m_HostCPUBrandString.find("Intel(R) Atom(TM)") !=
               m_HostCPUBrandString.npos) {
      m_HostCPUBrand = BRAND_INTEL_ATOM;
    } else if (m_HostCPUBrandString.find("Intel(R) Pentium(R)") !=
               m_HostCPUBrandString.npos) {
      m_HostCPUBrand = BRAND_INTEL_PENTIUM;
    } else if (m_HostCPUBrandString.find("Intel(R) Celeron(R)") !=
               m_HostCPUBrandString.npos) {
      m_HostCPUBrand = BRAND_INTEL_CELERON;
    } else if (m_HostCPUBrandString.find("Intel(R) Xeon(R)") !=
               m_HostCPUBrandString.npos) {
      m_HostCPUBrand = BRAND_INTEL_XEON;
    } else {
      // uknown brand name
      m_HostCPUBrand = BRAND_UNKNOWN;
    }
  }
}
