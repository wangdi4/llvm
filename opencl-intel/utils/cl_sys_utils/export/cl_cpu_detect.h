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

#include "cl_types.h"
#include "cl_env.h"

namespace Intel { namespace OpenCL { namespace Utils {

    // IsCPUSupported - Check that the current CPU is alligned with the required HW platform
    // returncs CL_SUCCESS if the cpu supported
    // returns CL_ERR_CPU_NOT_SUPPORTED otherwise
    cl_err_code IsCPUSupported();

    // CPU Features enumeration
    enum ECPUFeatureSupport
    {
        CFS_NONE     = 0x0000,
        CFS_SSE2     = 0x0001,
        CFS_SSE3     = 0x0002,
        CFS_SSSE3    = 0x0004,
        CFS_SSE41    = 0x0008,
        CFS_SSE42    = 0x0010,
        CFS_AVX10    = 0x0020,
        CFS_AVX20    = 0x0040,
        CFS_FMA      = 0x0080,
        CFS_AVX512F  = 0x0100,  // KNL, SKX
        CFS_AVX512CD = 0x0200,  // KNL, SKX
        CFS_AVX512ER = 0x0400,  // KNL
        CFS_AVX512PF = 0x0800,  // KNL
        CFS_AVX512BW = 0x1000,  // SKX
        CFS_AVX512DQ = 0x2000,  // SKX
        CFS_AVX512VL = 0x4000   // SKX
    };

    // Processor brand family
    enum ECPUBrandFamily
    {
        BRAND_UNKNOWN,
        BRAND_INTEL_CORE,
        BRAND_INTEL_ATOM,
        BRAND_INTEL_PENTIUM,
        BRAND_INTEL_CELERON,
        BRAND_INTEL_XEON
    };

    // CPU detection class (singleton)
    class CPUDetect
    {
    public:

#define CPU_ARCHS(modificator) \
      modificator(WST)         \
      modificator(WST_XEON)    \
      modificator(SNB)         \
      modificator(SNB_XEON)    \
      modificator(IVB)         \
      modificator(IVB_XEON)    \
      modificator(HSW)         \
      modificator(HSW_XEON)    \
      modificator(BDW)         \
      modificator(BDW_XEON)    \
      modificator(BXT)         \
      modificator(SKL)         \
      modificator(SKX)         \
      modificator(KBL)         \
      modificator(GLK)         \
      modificator(CNL)         \
      modificator(ICL)

    enum CPUArch {
        UNKNOWN = 0,
        #define CREATE_ENUM(name) name,
        CPU_ARCHS(CREATE_ENUM)
        #undef CREATE_ENUM
    };

   const std::vector<std::string> CPUArchStr = {
        "UNKNOWN",
        #define CREATE_STRINGS(name) #name,
        CPU_ARCHS(CREATE_STRINGS)
        #undef CREATE_STRINGS
   };

#undef CPU_ARCHS

        static CPUDetect * GetInstance();

        bool IsGenuineIntel();
        bool isWestmere();
        bool isSandyBridge();
        bool isIvyBridge();
        bool isHaswell();
        bool isBroadwell();
        bool isBroxton();
        bool isSkylake();
        bool isKabylakeOrCoffeelake();
        bool isGeminilake();
        bool isCannonlake();
        bool isIcelake();

        bool IsFeatureSupported(ECPUFeatureSupport featureType);

        unsigned char GetStepping() { return m_ucStepping; }
        unsigned char GetType() { return m_ucType; }
        const char *  GetCPUString() { return m_szCPUString; }
        const char *  GetCPUBrandString() { return m_szCPUBrandString; }
        unsigned int  GetCPUFeatureSupport() { return m_uiCPUFeatures; }
        unsigned int  GetCoreCount() { return m_uiCoreCount; }
        std::string   GetCPUArchShortName() { return CPUArchStr[m_cpuArch]; }
        ECPUBrandFamily GetCPUBrandFamily() { return m_eCPUBrand; }

    private:
        CPUDetect& operator=(const CPUDetect&);
        CPUDetect(const CPUDetect&);
        CPUDetect();
        ~CPUDetect();

        bool m_bBypassCPUDetect;
        bool m_bIsGenuineIntel;
        unsigned char m_ucStepping;
        unsigned char m_ucType;
        char * m_szCPUString;
        char * m_szCPUBrandString;
        unsigned int m_uiCPUFeatures;
        unsigned int m_uiCoreCount;
        unsigned short  m_i16ProcessorSignature;
        ECPUBrandFamily m_eCPUBrand;
        CPUArch         m_cpuArch;

        void GetCPUInfo();
        bool ShouldBypassCPUCheck();

    };

}}}
#endif //__CL_CPU_DETECT_H__
