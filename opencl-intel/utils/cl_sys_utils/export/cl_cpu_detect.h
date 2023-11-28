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

#ifndef __CL_CPU_DETECT_H__
#define __CL_CPU_DETECT_H__

#include "ICLDevBackendOptions.h"
#include "cl_types.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/ErrorHandling.h"

#include <string>
#include <vector>

namespace Intel {
namespace OpenCL {
namespace Utils {

// IsCPUSupported - Check that the current CPU is alligned with the required HW
// platform returncs CL_SUCCESS if the CPU supported returns
// CL_ERR_CPU_NOT_SUPPORTED otherwise
cl_err_code IsCPUSupported();

// CPU Features enumeration
enum ECPUFeatureSupport {
  CFS_NONE = 0x0000,
  CFS_SSE2,
  CFS_SSE3,
  CFS_SSSE3,
  CFS_SSE41,
  CFS_SSE42,
  CFS_AVX10,
  CFS_AVX20,
  CFS_FMA,
  CFS_BMI,
  CFS_BMI2,
  CFS_AVX512F,  // SKX
  CFS_AVX512CD, // SKX
  CFS_AVX512BW, // SKX
  CFS_AVX512DQ, // SKX
  CFS_AVX512VL, // SKX
  CFS_PKU,      // SKX
  CFS_CLWB,
  CFS_AVX512VNNI,      // CLX
  CFS_AVX512VBMI,      // CNL
  CFS_AVX512IFMA,      // CNL
  CFS_AVX512BITALG,    // ICL
  CFS_AVX512VBMI2,     // ICL
  CFS_AVX512VPOPCNTDQ, // ICL
  CFS_WBNOINVD,        // ICX
  CFS_PCONFIG,         // ICX
  CFS_AMXTILE,         // SPR
  CFS_AMXINT8,         // SPR
  CFS_AMXBF16,         // SPR
  CFS_AVX512FP16,      // SPR
  CFS_AVX512BF16,      // SPR
  CFS_AMXFP16,         // GNR
  CFS_PREFETCHI,       // GNR
  CFS_F16C
};

// Processor brand family
enum ECPUBrandFamily {
  BRAND_UNKNOWN,
  BRAND_INTEL_CORE,
  BRAND_INTEL_ATOM,
  BRAND_INTEL_PENTIUM,
  BRAND_INTEL_CELERON,
  BRAND_INTEL_XEON
};

#define CPU_ARCHS(modificator)                                                 \
  modificator(CPU_COREI7) modificator(CPU_SNB) modificator(CPU_HSW)            \
      modificator(CPU_SKX) modificator(CPU_CLX) modificator(CPU_ICL)           \
          modificator(CPU_ICX) modificator(CPU_SPR) modificator(CPU_GNR)

enum ECPU : unsigned {
  CPU_UNKNOWN = 0,
#define CREATE_ENUM(name) name,
  CPU_ARCHS(CREATE_ENUM)
#undef CREATE_ENUM
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CPU_DMR
  CPU_DMR = 10,
#endif // INTEL_FEATURE_CPU_DMR
#endif // INTEL_CUSTOMIZATION
  CPU_LAST_ARCH
};

enum TransposeSizeSupport { SUPPORTED, UNSUPPORTED, INVALID };

// workaground to keep backward compatibility in AOT mode
// Keep original CPUId class for KernelProperties serialization, for simplify,
// just store hasAVX1 and hasAVX2 information in m_CPUFeatures since only these
// are needed
class CPUId {
public:
  CPUId() : m_CPU(CPU_UNKNOWN), m_CPUFeatures(0), m_is64BitOS(0) {}
  CPUId(ECPU CPU, bool hasAVX1, bool hasAVX2, bool is64BitOS)
      : m_CPU(CPU), m_CPUFeatures(0), m_is64BitOS(is64BitOS ? 1 : 0) {
    if (hasAVX1)
      m_CPUFeatures |= CFS_AVX10;
    if (hasAVX2)
      m_CPUFeatures |= CFS_AVX20;
    // These variables are not used but they're kept for compatibility. To avoid
    // compilation warning ("unused private field"), add (void)var; expressions
    // for them.
    (void)m_CPU;
    (void)m_is64BitOS;
  }
  bool HasAVX1() const { return m_CPUFeatures & CFS_AVX10; }
  bool HasAVX2() const { return m_CPUFeatures & CFS_AVX20; }
  std::string str() const;

private:
  ECPU m_CPU;
  unsigned int m_CPUFeatures;
  unsigned int m_is64BitOS;
};

// CPU detection class (singleton)
class CPUDetect {
public:
  CPUDetect(const CPUDetect &) = delete;
  CPUDetect &operator=(const CPUDetect &) = delete;

  static const std::vector<std::string> CPUArchStr;
  static CPUDetect *GetInstance();
  // This is to reset CPU according to ICompilerConfig when
  // CL_CONFIG_CPU_TARGET_ARCH env is set
  void ResetCPU(ECPU CPU,
                const llvm::SmallVectorImpl<std::string> &forcedFeatures);
  // Check host CPU is westmere or not
  bool isWestmereForHost() const { return m_HostCPUString == "westmere"; }

  // Check if given feature is supported for the CPU which can be host CPU or a
  // customization CPU by config
  bool IsFeatureSupported(ECPUFeatureSupport featureType) const;
  // Check if given feature is supported for host CPU
  bool IsFeatureSupportedOnHost(ECPUFeatureSupport featureType) const;
  TransposeSizeSupport isTransposeSizeSupported(
      Intel::OpenCL::DeviceBackend::ETransposeSize transposeSize) const;

  const char *GetCPUPrefix() const {
    if (CPU_SNB == m_CPUArch && !HasAVX1()) {
      // Use SSE4 if AVX1 is not supported
      return GetCPUPrefixSSE(m_is64BitOS != 0);
    } else {
      switch (m_CPUArch) {
      default:
        llvm_unreachable("Unknown CPU!");
      case CPU_COREI7:
        return GetCPUPrefixSSE(m_is64BitOS);
      case CPU_SNB:
        return GetCPUPrefixAVX(m_is64BitOS);
      case CPU_HSW:
        return GetCPUPrefixAVX2(m_is64BitOS);
      case CPU_SKX:
      case CPU_CLX:
      case CPU_ICL:
      case CPU_ICX:
        return GetCPUPrefixAVX512(m_is64BitOS);
      case CPU_SPR:
      case CPU_GNR:
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CPU_DMR
      case CPU_DMR:
#endif // INTEL_FEATURE_CPU_DMR
#endif // INTEL_CUSTOMIZATION
        return GetCPUPrefixAMX(m_is64BitOS);
      }
    }
  }

  static ECPU GetCPUByName(const char *CPUName) {
    return llvm::StringSwitch<ECPU>(CPUName)
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CPU_DMR
        .Case("diamondrapids", CPU_DMR)
#endif // INTEL_FEATURE_CPU_DMR
#endif // INTEL_CUSTOMIZATION
        .Case("graniterapids", CPU_GNR)
        .Case("sapphirerapids", CPU_SPR)
        .Case("icelake-client", CPU_ICL)
        .Case("icelake-server", CPU_ICX)
        .Case("cascadelake", CPU_CLX)
        .Case("skx", CPU_SKX)
        .Case("core-avx2", CPU_HSW)
        .Case("corei7-avx", CPU_SNB)
        .Case("corei7", CPU_COREI7)
        .Default(CPU_UNKNOWN);
  }

  static const char *GetCPUName(ECPU CPU) {
    switch (CPU) {
    default:
      return "invalid";
    case CPU_COREI7:
      return "corei7";
    case CPU_SNB:
      return "corei7-avx";
    case CPU_HSW:
      return "core-avx2";
    case CPU_SKX:
      return "skx";
    case CPU_CLX:
      return "cascadelake";
    case CPU_ICL:
      return "icelake-client";
    case CPU_ICX:
      return "icelake-server";
    case CPU_SPR:
      return "sapphirerapids";
    case CPU_GNR:
      return "graniterapids";
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CPU_DMR
    case CPU_DMR:
      return "diamondrapids";
#endif // INTEL_FEATURE_CPU_DMR
#endif // INTEL_CUSTOMIZATION
    }
    llvm_unreachable("Unknown CPU!");
  }

  static bool IsValidCPUName(const char *pCPUName) {
    return CPU_UNKNOWN != GetCPUByName(pCPUName);
  }

  const std::string &GetCPUName() const { return m_CPUString; }
  std::string GetHostCPUBrandString() const { return m_HostCPUBrandString; }
  std::string GetCPUArchShortName() const { return CPUArchStr.at(m_CPUArch); }
  ECPUBrandFamily GetHostCPUBrandFamily() { return m_HostCPUBrand; }
  ECPU GetCPU() const { return m_CPUArch; }

  static const char *GetCPUPrefixSSE(bool is64BitOS) {
    return is64BitOS ? "h8" : "n8";
  }
  static const char *GetCPUPrefixAVX(bool is64BitOS) {
    return is64BitOS ? "e9" : "g9";
  }
  static const char *GetCPUPrefixAVX2(bool is64BitOS) {
    return is64BitOS ? "l9" : "s9";
  }
  static const char *GetCPUPrefixAVX512(bool is64BitOS) {
    return is64BitOS ? "z0" : "x0";
  }
  static const char *GetCPUPrefixAMX(bool is64BitOS) {
    return is64BitOS ? "z1" : "x1";
  }

  bool HasAVX1() const { return IsFeatureSupported(CFS_AVX10); }
  bool HasAVX2() const { return IsFeatureSupported(CFS_AVX20); }
  bool HasSSE41() const { return IsFeatureSupported(CFS_SSE41); }
  bool HasSSE42() const { return IsFeatureSupported(CFS_SSE42); }
  bool HasAVX512Core() const { return IsFeatureSupported(CFS_AVX512F); }
  bool HasAVX512SKX() const {
    return IsFeatureSupported(CFS_AVX512BW) &&
           IsFeatureSupported(CFS_AVX512DQ) && IsFeatureSupported(CFS_AVX512VL);
  }
  bool HasAVX512CLX() const {
    return HasAVX512SKX() && IsFeatureSupported(CFS_AVX512VNNI);
  }
  bool HasAVX512ICL() const {
    return IsFeatureSupported(CFS_AVX512BITALG) &&
           IsFeatureSupported(CFS_AVX512VBMI2) &&
           IsFeatureSupported(CFS_AVX512VPOPCNTDQ);
  }
  bool HasAVX512ICX() const {
    return HasAVX512ICL() && IsFeatureSupported(CFS_WBNOINVD) &&
           IsFeatureSupported(CFS_CLWB) && IsFeatureSupported(CFS_PCONFIG);
  }
  bool HasSPR() const {
    return IsFeatureSupported(CFS_AMXTILE) && IsFeatureSupported(CFS_AMXINT8) &&
           IsFeatureSupported(CFS_AMXBF16);
  }
  bool HasGNR() const {
    return HasSPR() && IsFeatureSupported(CFS_AMXFP16) &&
           IsFeatureSupported(CFS_PREFETCHI);
  }
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CPU_DMR
  bool HasDMR() const {
    // FIXME: Add diamondrapids features check when they are disclosed
    return HasGNR() && m_CPUString == "diamondrapids";
  }
#endif // INTEL_FEATURE_CPU_DMR
#endif // INTEL_CUSTOMIZATION

  bool Is64BitOS() const { return m_is64BitOS; }

  // This is to push disabled CPU features in list which is passed to code
  // generator.
  void
  GetDisabledCPUFeatures(llvm::SmallVector<std::string, 8> &forcedFeatures);

  CPUId GetCPUIdForKernelPropertiesSerialize() const;

private:
  CPUDetect();
  ~CPUDetect(){};
  void GetHostCPUBrandInfo();

  bool m_is64BitOS;
  ECPU m_CPUArch;
  std::string m_CPUString;
  llvm::StringMap<bool> m_CPUFeatures;
  ECPUBrandFamily m_HostCPUBrand; // host CPU brand
  std::string m_HostCPUString;
  std::string m_HostCPUBrandString;
  llvm::StringMap<bool> m_HostCPUFeatures;
  static CPUDetect *Instance;
  static struct Deleter {
    ~Deleter() {
      if (Instance)
        delete Instance;
    }
  } InstanceDeleter;

  bool ShouldBypassCPUCheck() const;
};

} // namespace Utils
} // namespace OpenCL
} // namespace Intel
#endif //__CL_CPU_DETECT_H__
