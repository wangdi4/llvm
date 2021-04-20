//===- CPUDetect.h - CPU feature detection utilities ----------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DPCPP_KERNEL_TRANSFORMS_CPU_DETECT_H
#define INTEL_DPCPP_KERNEL_TRANSFORMS_CPU_DETECT_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/ErrorHandling.h"

#include <string>
#include <vector>

namespace llvm {

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

/// CPU detection utility class.
class CPUDetect {
public:
  const std::vector<std::string> CPUArchStr = {"CPU_UNKNOWN",
#define CREATE_STRINGS(name) #name,
                                               CPU_ARCHS(CREATE_STRINGS)
#undef CREATE_STRINGS
  };
#undef CPU_ARCHS

  /// Get singleton instance.
  static CPUDetect *GetInstance();

  CPUDetect(ECPU CPU, const SmallVectorImpl<std::string> &forcedFeatures,
            bool is64BitOS);

  bool isWestmere() const { return CpuArch == CPU_WST; }

  bool isSandyBridge() const { return CpuArch == CPU_SNB; }

  bool isIvyBridge() const { return CpuArch == CPU_IVB; }

  bool isHaswell() const { return CpuArch == CPU_HSW; }

  bool isBroadwell() const { return CpuArch == CPU_BDW; }

  bool isSkylake() const { return CpuArch == CPU_SKL || CpuArch == CPU_SKX; }

  bool isGeminilake() const { return CpuArch == CPU_GLK; }

  bool isCannonlake() const { return CpuArch == CPU_CNL; }

  bool isIcelake() const { return CpuArch == CPU_ICL || CpuArch == CPU_ICX; }

  bool isRocketlake() const { return CpuArch == CPU_RKL; }
  bool isTigerlake() const { return CpuArch == CPU_TGL; }
  bool isAlderlake() const { return CpuArch == CPU_ADL; }

  bool IsFeatureSupported(ECPUFeatureSupport featureType) const;

  const char *GetCPUPrefix() const {
    if (CPU_SNB == CpuArch && !HasAVX1()) {
      // Use SSE4 if AVX1 is not supported
      return GetCPUPrefixSSE(Is64BitOS);
    } else {
      switch (CpuArch) {
      default:
        break;
      case CPU_PENTIUM:
        return Is64BitOS ? "unknown" : "w7";
      case CPU_NOCONA:
        return Is64BitOS ? "e7" : "t7";
      case CPU_CORE2:
        return Is64BitOS ? "u8" : "v8";
      case CPU_PENRYN:
        return Is64BitOS ? "y8" : "p8";
      case CPU_COREI7:
        return GetCPUPrefixSSE(Is64BitOS);
      case CPU_SNB:
        return GetCPUPrefixAVX(Is64BitOS);
      case CPU_HSW:
        return GetCPUPrefixAVX2(Is64BitOS);
      case CPU_KNL:
        return Is64BitOS ? "b3" : "d3";
      case CPU_SKX:
      case CPU_ICL:
      case CPU_ICX:
        return GetCPUPrefixAVX512(Is64BitOS);
      case CPU_SPR:
        return GetCPUPrefixAMX(Is64BitOS);
      }
      if (HasAVX512Core())
        return GetCPUPrefixAVX512(Is64BitOS);
      if (HasAVX2())
        return GetCPUPrefixAVX2(Is64BitOS);
      if (HasAVX1())
        return GetCPUPrefixAVX(Is64BitOS);
      if (HasSSE41())
        return Is64BitOS ? "y8" : "p8";
      if (HasSSE3())
        return Is64BitOS ? "e7" : "t7";
      if (HasSSE2())
        return Is64BitOS ? "ex" : "a6";

      llvm_unreachable("Unknown CPU!");
    }
  }

  static ECPU GetCPUByName(const char *CPUName) {
    return StringSwitch<ECPU>(CPUName)
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

  const std::string &GetCPUName() const { return CPUString; }
  std::string GetCPUArchShortName() const { return CPUArchStr.at(CpuArch); }
  ECPU GetCPU() const { return CpuArch; }

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

  bool HasGatherScatter() const { return HasGatherScatter(CpuArch); }

  static bool HasGatherScatter(ECPU CPU) {
    return (CPU == CPU_KNL || CPU == CPU_SKX || CPU == CPU_ICL ||
            CPU == CPU_ICX);
  }
  static bool HasGatherScatterPrefetch(ECPU CPU) { return (CPU == CPU_KNL); }
  bool HasGatherScatterPrefetch() const {
    return HasGatherScatterPrefetch(CpuArch);
  }

  bool RequirePrefetch() const {
    // There are no architectures that use prefetch by now.
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
  bool Is64Bit() const { return Is64BitOS; }
  StringMap<bool> GetCPUFeatures() const { return CpuFeatures; }

private:
  CPUDetect &operator=(const CPUDetect &);
  CPUDetect(const CPUDetect &);
  CPUDetect();
  ~CPUDetect(){};

  bool Is64BitOS;
  ECPU CpuArch;
  std::string CPUString;
  StringMap<bool> CpuFeatures;
};

} // namespace llvm

#endif
