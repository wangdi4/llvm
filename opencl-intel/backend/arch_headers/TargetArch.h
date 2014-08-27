#ifndef __TARGET_HEADERS_H__
#define __TARGET_HEADERS_H__

#include <cassert>
#include <string>
#include <utility>
#include <exception>

namespace Intel {
// CPU enumeration
enum ECPU {
    CPU_PENTIUM       = 0,
    CPU_FIRST         = CPU_PENTIUM,
    CPU_NOCONA        = 1,
    CPU_CORE2,
    CPU_PENRYN,
    CPU_COREI7,
    CPU_SANDYBRIDGE,
    CPU_HASWELL,
    CPU_KNL,
    // MIC Cards must appear last
    MIC_KNC,
    MIC_FIRST = MIC_KNC,
    DEVICE_INVALID // Always last
};
// CPU Features enumeration
enum ECPUFeatureSupport {
    CFS_NONE    = 0x0000,
    CFS_SSE2    = 1,
    CFS_SSE3    = 1 << 1,
    CFS_SSSE3   = 1 << 2,
    CFS_SSE41   = 1 << 3,
    CFS_SSE42   = 1 << 4,
    CFS_AVX1    = 1 << 5,
    CFS_AVX2    = 1 << 6,
    CFS_FMA     = 1 << 7,
    CFS_BMI     = 1 << 8,
    CFS_BMI2    = 1 << 9,
    CFS_AVX512F = 1 << 10
};
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
    static ECPU GetCPUByName(const char *CPUName) {
        std::string Name(CPUName);
        if (Name == "knl") return CPU_KNL;
        if (Name == "knc") return MIC_KNC;
        if (Name == "core-avx2") return CPU_HASWELL;
        if (Name == "corei7-avx") return CPU_SANDYBRIDGE;
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
        case CPU_COREI7:
            return "corei7";
        case CPU_SANDYBRIDGE:
            return "corei7-avx";
        case CPU_HASWELL:
            return "core-avx2";
        case CPU_KNL:
            return "knl";
        case MIC_KNC:
            return "knc";
        }
    }
    const char*         GetCPUPrefix() const {
      if( Intel::CPU_SANDYBRIDGE == GetCPU() && !HasAVX1()) {
        // Use SSE4 if AVX1 is not supported
        return GetCPUPrefix(Intel::CPU_COREI7, m_is64BitOS != 0);
      } else {
        return GetCPUPrefix(m_CPU, m_is64BitOS != 0);
      }
    }
    static const char*  GetCPUPrefix(ECPU CPU, bool is64BitOS) {
        if (!is64BitOS) {
            switch(CPU) {
            default:
                return 0;
            case CPU_PENTIUM:
                return "w7";
            case CPU_NOCONA:
                return "t7";
            case CPU_CORE2:
                return "v8";
            case CPU_PENRYN:
                return "p8";
            case CPU_COREI7:
                return "n8";
            case CPU_SANDYBRIDGE:
                return "g9";
            case CPU_HASWELL:
                return "s9";
            case CPU_KNL:
                return "d3";
            case MIC_KNC:
                assert(false && "No MIC SVML lib for 32-bit OS!");
                return 0;
            }
        }
        switch(CPU) {
        default:
            return 0;
        case CPU_PENTIUM:
            return "unknown";
        case CPU_NOCONA:
            return "e7";
        case CPU_CORE2:
            return "u8";
        case CPU_PENRYN:
            return "y8";
        case CPU_COREI7:
            return "h8";
        case CPU_SANDYBRIDGE:
            return  "e9";
        case CPU_HASWELL:
            return "l9";
        case CPU_KNL:
            return "b3";
        case MIC_KNC:
            return "b2";
        }
    }
    unsigned GetLatestSupportedFeature() const {
        return 1 << m_CPU;
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
    };
    static bool HasGatherScatter(ECPU CPU) {
        return (CPU == MIC_KNC || CPU == CPU_KNL);
    };
    bool RequirePrefetch() const {
        return m_CPU == MIC_KNC;
    };
    static bool IsValidCPUName(const char* pCPUName) {
        return DEVICE_INVALID != GetCPUByName(pCPUName);
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
    static unsigned GetLatestSupportedFeature(ECPU CPU) {
        return (1 << CPU);
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
