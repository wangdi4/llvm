// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __TARGET_HEADERS_H__
#define __TARGET_HEADERS_H__

#include "ICLDevBackendOptions.h"
#include "llvm/Support/ErrorHandling.h"

#include <cassert>
#include <string>
#include <utility>

using namespace Intel::OpenCL::DeviceBackend;

namespace Intel {
// CPU enumeration
enum ECPU {
    CPU_PENTIUM       = 0,
    CPU_FIRST         = CPU_PENTIUM,
    CPU_NOCONA        = 1,
    CPU_CORE2,
    CPU_PENRYN,
    CPU_COREI7,
    CPU_LKF,
    CPU_SANDYBRIDGE,
    CPU_HASWELL,
    CPU_KNL,
    CPU_SKX,
    CPU_ICL, // Icelake client
    CPU_ICX, // Icelake server
    DEVICE_INVALID // Always last
};
// CPU Features enumeration
enum ECPUFeatureSupport {
    CFS_NONE           = 0x0000,
    CFS_SSE2           = 1,
    CFS_SSE3           = 1 << 1,
    CFS_SSSE3          = 1 << 2,
    CFS_SSE41          = 1 << 3,
    CFS_SSE42          = 1 << 4,
    CFS_AVX1           = 1 << 5,
    CFS_AVX2           = 1 << 6,
    CFS_FMA            = 1 << 7,
    CFS_BMI            = 1 << 8,
    CFS_BMI2           = 1 << 9,
    CFS_AVX512F        = 1 << 10, // KNL, SKX
    CFS_AVX512CD       = 1 << 11, // KNL, SKX
    CFS_AVX512ER       = 1 << 12, // KNL
    CFS_AVX512PF       = 1 << 13, // KNL
    CFS_AVX512BW       = 1 << 14, // SKX
    CFS_AVX512DQ       = 1 << 15, // SKX
    CFS_AVX512VL       = 1 << 16, // SKX
    CFS_AVX512VBMI     = 1 << 18, // CNL
    CFS_AVX512IFMA     = 1 << 19, // CNL
    CFS_AVX512BITALG   = 1 << 20, // ICL
    CFS_AVX512VBMI2    = 1 << 21, // ICL
    CFS_AVX512POPCNTDQ = 1 << 23, //ICL
    CFS_CLWB           = 1 << 24, // ICL
    CFS_WBNOINVD       = 1 << 25,  // ICX
    CFS_F16C           = 1 << 26
};

enum TransposeSizeSupport { SUPPORTED, UNSUPPORTED, INVALID };

class CPUId {
public:
    CPUId(): m_CPU(DEVICE_INVALID), m_CPUFeatures(0), m_is64BitOS(0) {}
    CPUId(ECPU CPU, unsigned int cpuFeatures, bool is64BitOS):
        m_CPU(CPU),
        m_CPUFeatures(cpuFeatures),
        m_is64BitOS(is64BitOS?1:0)
    {}
    bool operator==(const CPUId& RHS) const {
        return m_CPU == RHS.m_CPU && m_CPUFeatures == RHS.m_CPUFeatures;
    }
    bool operator<(const CPUId& RHS) const {
        return std::make_pair(unsigned(m_CPU),m_CPUFeatures) < std::make_pair(unsigned(RHS.m_CPU), RHS.m_CPUFeatures);
    }
    unsigned GetCPUFeatureSupport() const {
        return m_CPUFeatures;
    }

    TransposeSizeSupport
    isTransposeSizeSupported(ETransposeSize transposeSize) const {
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

      static ECPU GetCPUByName(const char *CPUName) {
        std::string Name(CPUName);
        if (Name == "icelake-client") return CPU_ICL;
        if (Name == "icelake-server") return CPU_ICX;
        if (Name == "knl") return CPU_KNL;
        if (Name == "skx") return CPU_SKX;
        if (Name == "core-avx2") return CPU_HASWELL;
        if (Name == "corei7-avx") return CPU_SANDYBRIDGE;
        if (Name == "lkf") return CPU_LKF;
        if (Name == "corei7") return CPU_COREI7;
        if (Name == "penryn") return CPU_PENRYN;
        if (Name == "core2") return CPU_CORE2;
        if (Name == "nicona") return CPU_NOCONA;
        if (Name == "pentium") return CPU_PENTIUM;
        return DEVICE_INVALID;
    }
    const char* GetCPUName() const {
        return GetCPUName(m_CPU);
    }
    static const char* GetCPUName(ECPU CPU) {
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
        case CPU_LKF:
            return "lkf";
        case CPU_COREI7:
            return "corei7";
        case CPU_SANDYBRIDGE:
            return "corei7-avx";
        case CPU_HASWELL:
            return "core-avx2";
        case CPU_KNL:
            return "knl";
        case CPU_SKX:
            return "skx";
        case CPU_ICL:
            return "icelake-client";
        case CPU_ICX:
            return "icelake-server";
        }
        llvm_unreachable("Unknown CPU!");
    }
    const char* GetCPUPrefix() const {
      if( Intel::CPU_SANDYBRIDGE == GetCPU() && !HasAVX1()) {
        // Use SSE4 if AVX1 is not supported
        return GetCPUPrefix(Intel::CPU_COREI7, m_is64BitOS != 0);
      } else {
        return GetCPUPrefix(m_CPU, m_is64BitOS != 0);
      }
    }
    static const char* GetCPUPrefixSSE(bool is64BitOS) {
        return is64BitOS ? "h8" : "n8";
    }
    static const char* GetCPUPrefixAVX(bool is64BitOS) {
        return is64BitOS ? "e9" : "g9";
    }
    static const char* GetCPUPrefixAVX2(bool is64BitOS) {
        return is64BitOS ? "l9" : "s9";
    }
    static const char* GetCPUPrefixAVX512(bool is64BitOS) {
        return is64BitOS ? "z0" : "x0";
    }
    static const char* GetCPUPrefix(ECPU CPU, bool is64BitOS) {
        switch(CPU) {
        default:
            return 0;
        case CPU_PENTIUM:
            return is64BitOS ? "unknown" : "w7";
        case CPU_NOCONA:
            return is64BitOS ? "e7" : "t7";
        case CPU_CORE2:
            return is64BitOS ? "u8" : "v8";
        case CPU_PENRYN:
            return is64BitOS ? "y8" : "p8";
        case CPU_LKF: // fallthrough
        case CPU_COREI7:
            return GetCPUPrefixSSE(is64BitOS);
        case CPU_SANDYBRIDGE:
            return GetCPUPrefixAVX(is64BitOS);
        case CPU_HASWELL:
            return GetCPUPrefixAVX2(is64BitOS);
        case CPU_KNL:
            return is64BitOS ? "b3" : "d3";
        case CPU_SKX: // fallthrough
        case CPU_ICL: // fallthrough
        case CPU_ICX:
            return GetCPUPrefixAVX512(is64BitOS);
        }
    }
    ECPU GetCPU() const {
        return m_CPU;
    }
    void SetCPU(ECPU CPU) {
        m_CPU = CPU;
    }
    bool IsFeatureOn(unsigned int Feature) const {
        return (m_CPUFeatures & Feature) != 0;
    }
    bool HasGatherScatter() const {
        return HasGatherScatter(m_CPU);
    }
    static bool HasGatherScatter(ECPU CPU) {
        return (CPU == CPU_KNL || CPU == CPU_SKX
             || CPU == CPU_ICL || CPU == CPU_ICX);
    }
    static bool HasGatherScatterPrefetch(ECPU CPU) {
        return (CPU == CPU_KNL);
    }
    bool HasGatherScatterPrefetch() const {
        return HasGatherScatterPrefetch(m_CPU);
    }
    static bool IsValidCPUName(const char* pCPUName) {
        return DEVICE_INVALID != GetCPUByName(pCPUName);
    }
    bool RequirePrefetch() const {
        // There are no architectures that use prefetch by now
        return false;
    }
    bool HasAVX1() const {
        return IsFeatureOn(CFS_AVX1);
    }
    bool HasAVX2() const {
        return IsFeatureOn(CFS_AVX2);
    }
    bool HasSSE2() const {
        return IsFeatureOn(CFS_SSE2);
    }
    bool HasSSE41() const {
        return IsFeatureOn(CFS_SSE41);
    }
    bool HasSSE42() const {
        return IsFeatureOn(CFS_SSE42);
    }
    bool HasAVX512Core() const {
      return IsFeatureOn(CFS_AVX512F);
    }
    bool HasAVX512SKX() const {
      return IsFeatureOn(CFS_AVX512BW) && IsFeatureOn(CFS_AVX512DQ) &&
             IsFeatureOn(CFS_AVX512VL);
    }
    bool HasAVX512ICL() const {
      return IsFeatureOn(CFS_AVX512BITALG) && IsFeatureOn(CFS_AVX512VBMI2) &&
             IsFeatureOn(CFS_AVX512POPCNTDQ);
    }
    bool Is64BitOS() const {
        return m_is64BitOS > 0;
    }
    void ToggleFeatureOn(unsigned int FeatureBits) {
        m_CPUFeatures |= FeatureBits;
    };
private:
    ECPU m_CPU;
    unsigned int m_CPUFeatures;
    unsigned int m_is64BitOS;
};
}
#endif
