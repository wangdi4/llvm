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
// platform returncs CL_SUCCESS if the cpu supported returns
// CL_ERR_CPU_NOT_SUPPORTED otherwise
cl_err_code IsCPUSupported();

// CPU Features enumeration
enum ECPUFeatureSupport {
  CFS_NONE = 0x0000,
  CFS_SSE2 = 1,
  CFS_SSE3 = 1 << 1,
  CFS_SSSE3 = 1 << 2,
  CFS_SSE41 = 1 << 3,
  CFS_SSE42 = 1 << 4,
  CFS_AVX10 = 1 << 5,
  CFS_AVX20 = 1 << 6,
  CFS_FMA = 1 << 7,
  CFS_BMI = 1 << 8,
  CFS_BMI2 = 1 << 9,
  CFS_AVX512F = 1 << 10,
  CFS_AVX512CD = 1 << 11,       // KNL, SKX
  CFS_AVX512ER = 1 << 12,       // KNL
  CFS_AVX512PF = 1 << 13,       // KNL
  CFS_AVX512BW = 1 << 14,       // SKX
  CFS_AVX512DQ = 1 << 15,       // SKX
  CFS_AVX512VL = 1 << 16,       // SKX
  CFS_AVX512VBMI = 1 << 17,     // CNL
  CFS_AVX512IFMA = 1 << 18,     // CNL
  CFS_AVX512BITALG = 1 << 19,   // ICL
  CFS_AVX512VBMI2 = 1 << 20,    // ICL
  CFS_AVX512POPCNTDQ = 1 << 21, // ICL
  CFS_CLWB = 1 << 22,           // ICL
  CFS_WBNOINVD = 1 << 23,       // ICX
  CFS_AMXTILE = 1 << 26,        // AMX
  CFS_AMXINT8 = 1 << 27,        // AMX
  CFS_AMXBF16 = 1 << 28,        // AMX
  CFS_F16C = 1 << 29

};

#define CPU_ARCHS(modificator)                                                 \
  modificator(CPU_PENTIUM) modificator(CPU_NOCONA) modificator(CPU_CORE2)      \
      modificator(CPU_WST) modificator(CPU_PENRYN) modificator(CPU_COREI7)     \
          modificator(CPU_SNB) modificator(CPU_HSW) modificator(CPU_KNL)       \
              modificator(CPU_IVB) modificator(CPU_BDW) modificator(CPU_SKL)   \
                  modificator(CPU_SKX) modificator(CPU_GLK)                    \
                      modificator(CPU_CNL) modificator(CPU_ICL)                \
                          modificator(CPU_ICX) modificator(CPU_SPR)            \
                              modificator(CPU_RKL) modificator(CPU_TGL)        \
                                  modificator(CPU_ADL)

enum ECPU {
  CPU_UNKNOWN = 0,
#define CREATE_ENUM(name) name,
  CPU_ARCHS(CREATE_ENUM)
#undef CREATE_ENUM
};

enum TransposeSizeSupport { SUPPORTED, UNSUPPORTED, INVALID };

// CPU detection class (singleton)
class CPUDetect {
public:
  const std::vector<std::string> CPUArchStr = {"CPU_UNKNOWN",
#define CREATE_STRINGS(name) #name,
                                               CPU_ARCHS(CREATE_STRINGS)
#undef CREATE_STRINGS
  };
#undef CPU_ARCHS

  static CPUDetect *GetInstance();
  CPUDetect(ECPU CPU, const llvm::SmallVectorImpl<std::string> &forcedFeatures,
            bool is64BitOS);

  bool isWestmere() const { return m_cpuArch == CPU_WST; }

  bool isSandyBridge() const { return m_cpuArch == CPU_SNB; }

  bool isIvyBridge() const { return m_cpuArch == CPU_IVB; }

  bool isHaswell() const { return m_cpuArch == CPU_HSW; }

  bool isBroadwell() const { return m_cpuArch == CPU_BDW; }

  bool isSkylake() const {
    return m_cpuArch == CPU_SKL || m_cpuArch == CPU_SKX;
  }

  bool isGeminilake() const { return m_cpuArch == CPU_GLK; }

  bool isCannonlake() const { return m_cpuArch == CPU_CNL; }

  bool isIcelake() const {
    return m_cpuArch == CPU_ICL || m_cpuArch == CPU_ICX;
  }

  bool isRocketlake() const { return m_cpuArch == CPU_RKL; }
  bool isTigerlake() const { return m_cpuArch == CPU_TGL; }
  bool isAlderlake() const { return m_cpuArch == CPU_ADL; }

  bool IsFeatureSupported(ECPUFeatureSupport featureType) const;
  TransposeSizeSupport isTransposeSizeSupported(
      Intel::OpenCL::DeviceBackend::ETransposeSize transposeSize) const;
  const char *GetCPUPrefix() const {
    if (CPU_SNB == m_cpuArch && !HasAVX1()) {
      // Use SSE4 if AVX1 is not supported
      return GetCPUPrefixSSE(m_is64BitOS);
    } else {

      switch (m_cpuArch) {
      default:
        break;
      case CPU_PENTIUM:
        return m_is64BitOS ? "unknown" : "w7";
      case CPU_NOCONA:
        return m_is64BitOS ? "e7" : "t7";
      case CPU_CORE2:
        return m_is64BitOS ? "u8" : "v8";
      case CPU_PENRYN:
        return m_is64BitOS ? "y8" : "p8";
      case CPU_COREI7:
        return GetCPUPrefixSSE(m_is64BitOS);
      case CPU_SNB:
        return GetCPUPrefixAVX(m_is64BitOS);
      case CPU_HSW:
        return GetCPUPrefixAVX2(m_is64BitOS);
      case CPU_KNL:
        return m_is64BitOS ? "b3" : "d3";
      case CPU_SKX:
      case CPU_ICL:
      case CPU_ICX:
        return GetCPUPrefixAVX512(m_is64BitOS);
      case CPU_SPR:
        return GetCPUPrefixAMX(m_is64BitOS);
      }
      if (HasAVX512Core())
        return GetCPUPrefixAVX512(m_is64BitOS);
      if (HasAVX2())
        return GetCPUPrefixAVX2(m_is64BitOS);
      if (HasAVX1())
        return GetCPUPrefixAVX(m_is64BitOS);
      if (HasSSE41())
        return m_is64BitOS ? "y8" : "p8";
      if (HasSSE3())
        return m_is64BitOS ? "e7" : "t7";
      if (HasSSE2())
        return m_is64BitOS ? "ex" : "a6";

      llvm_unreachable("Unknown CPU!");
    }
  }

  static ECPU GetCPUByName(const char *CPUName) {
    return llvm::StringSwitch<ECPU>(CPUName)
        .Case("sapphirerapids", CPU_SPR)
        .Case("icelake-client", CPU_ICL)
        .Case("icelake-server", CPU_ICX)
        .Case("knl", CPU_KNL)
        .Case("skx", CPU_SKX)
        .Case("core-avx2", CPU_HSW)
        .Case("corei7-avx", CPU_SNB)
        .Case("corei7", CPU_COREI7)
        .Case("penryn", CPU_PENRYN)
        .Case("core2", CPU_CORE2)
        .Case("nicona", CPU_NOCONA)
        .Case("pentium", CPU_PENTIUM)
        .Default(CPU_UNKNOWN);
  }

  static const char *GetCPUName(ECPU CPU) {
    switch (CPU) {
    default:
      return "invalid";
    case CPU_PENTIUM:
      return "pentium";
    case CPU_NOCONA:
      return "nicona";
    case CPU_CORE2:
      return "core2";
    case CPU_PENRYN:
      return "penryn";
    case CPU_COREI7:
      return "corei7";
    case CPU_SNB:
      return "corei7-avx";
    case CPU_HSW:
      return "core-avx2";
    case CPU_KNL:
      return "knl";
    case CPU_SKX:
      return "skx";
    case CPU_ICL:
      return "icelake-client";
    case CPU_ICX:
      return "icelake-server";
    case CPU_SPR:
      return "sapphirerapids";
    }
    llvm_unreachable("Unknown CPU!");
  }

  static bool IsValidCPUName(const char *pCPUName) {
    return CPU_UNKNOWN != GetCPUByName(pCPUName);
  }

  const std::string &GetCPUName() const { return m_CPUString; }
  std::string GetCPUArchShortName() const { return CPUArchStr.at(m_cpuArch); }
  ECPU GetCPU() const { return m_cpuArch; }

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

  bool HasGatherScatter() const { return HasGatherScatter(m_cpuArch); }

  static bool HasGatherScatter(ECPU CPU) {
    return (CPU == CPU_KNL || CPU == CPU_SKX || CPU == CPU_ICL ||
            CPU == CPU_ICX);
  }
  static bool HasGatherScatterPrefetch(ECPU CPU) { return (CPU == CPU_KNL); }
  bool HasGatherScatterPrefetch() const {
    return HasGatherScatterPrefetch(m_cpuArch);
  }

  bool RequirePrefetch() const {
    // There are no architectures that use prefetch by now
    return false;
  }
  bool HasAVX1() const { return IsFeatureSupported(CFS_AVX10); }
  bool HasAVX2() const { return IsFeatureSupported(CFS_AVX20); }
  bool HasSSE2() const { return IsFeatureSupported(CFS_SSE2); }
  bool HasSSE3() const { return IsFeatureSupported(CFS_SSE3); }
  bool HasSSE41() const { return IsFeatureSupported(CFS_SSE41); }
  bool HasSSE42() const { return IsFeatureSupported(CFS_SSE42); }
  bool HasAVX512Core() const { return IsFeatureSupported(CFS_AVX512F); }
  bool HasAVX512SKX() const {
    return IsFeatureSupported(CFS_AVX512BW) &&
           IsFeatureSupported(CFS_AVX512DQ) && IsFeatureSupported(CFS_AVX512VL);
  }
  bool HasAVX512ICL() const {
    return IsFeatureSupported(CFS_AVX512BITALG) &&
           IsFeatureSupported(CFS_AVX512VBMI2) &&
           IsFeatureSupported(CFS_AVX512POPCNTDQ);
  }
  bool Is64BitOS() const { return m_is64BitOS; }
  llvm::StringMap<bool> GetCPUFeatures() const { return m_cpuFeatures; }

private:
  CPUDetect &operator=(const CPUDetect &);
  CPUDetect(const CPUDetect &);
  CPUDetect();
  ~CPUDetect(){};

  bool m_is64BitOS;
  bool m_bBypassCPUDetect;
  ECPU m_cpuArch;
  std::string m_CPUString;
  llvm::StringMap<bool> m_cpuFeatures;

  bool ShouldBypassCPUCheck() const;
};

} // namespace Utils
} // namespace OpenCL
} // namespace Intel
#endif //__CL_CPU_DETECT_H__
