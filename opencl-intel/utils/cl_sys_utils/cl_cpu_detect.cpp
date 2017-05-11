/////////////////////////////////////////////////////////////////////////
// cl_cpu_detect.cpp:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intels prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intels suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////

#include "cl_cpu_detect.h"
#include "cl_env.h"
#include "hw_utils.h"

#if defined( _WIN32 )
#include <windows.h>
#include <intrin.h>
#else
#include <cstdlib>
#include <cstring>
#include "hw_utils.h"
#endif

#include <assert.h>
#include <string.h>

#if defined(_M_X64) || defined(__LP64__)

#if defined(_M_X64)
  #pragma pack (1)
  #define PACKED_STRUCT
#else
  #define PACKED_STRUCT __attribute__ ((packed))
#endif

struct XGETBV_PARAMS {
    unsigned long long   m_rax;
    unsigned long long   m_rdx;
} PACKED_STRUCT;

#if defined(_M_X64)
  #pragma pack ()
#endif

extern "C" void hw_xgetbv( struct XGETBV_PARAMS *);

//------------------------------------------------------------------------------
// void ASM_FUNCTION xgetbv( int xcr_info[2]);
//------------------------------------------------------------------------------

#define xgetbv( p_xcr_info )                                                   \
{                                                                              \
    struct XGETBV_PARAMS __xgetbv_params;                                      \
    hw_xgetbv( &__xgetbv_params);                                              \
                                                                               \
    (p_xcr_info)[0] = (unsigned int)__xgetbv_params.m_rax;                     \
    (p_xcr_info)[1] = (unsigned int)__xgetbv_params.m_rdx;                     \
}

#endif

using namespace Intel::OpenCL::Utils;

static const char* CPU_STRING = "GenuineIntel";

cl_err_code Intel::OpenCL::Utils::IsCPUSupported()
{
    if( CPUDetect::GetInstance()->IsFeatureSupported(CFS_SSE41) )
    {
        return CL_SUCCESS;
     }
     return CL_ERR_CPU_NOT_SUPPORTED;
}

bool CPUDetect::IsGenuineIntel()
{
    if (m_bBypassCPUDetect)
    {
        return true;
    }

    return m_bIsGenuineIntel;
}

// INTEL CORE.
bool CPUDetect::isWestmere()
{
    if ((0x2065 == m_i16ProcessorSignature) ||  // Arrandale/Clarksdale
        (0x206C == m_i16ProcessorSignature) ||  // Gulftown/Westmere-EP
        (0x206F == m_i16ProcessorSignature))    // Westmere-EX (Xeon)
        return true;

    return false;
}

bool CPUDetect::isBroadwell()
{
    if(0x306D == m_i16ProcessorSignature || // Broadwell ULT Client.
       0x4067 == m_i16ProcessorSignature || // Broadwell Client Halo.
       0x406F == m_i16ProcessorSignature)   // Broadwell Server
        return true;

    return false;
}

bool CPUDetect::isSkylake()
{
    if(0x406E == m_i16ProcessorSignature || // Skylake ULT/ULX.
       0x506E == m_i16ProcessorSignature || // Skylake DT/HALO.
       0x5065 == m_i16ProcessorSignature)   // Skylake Server
        return true;

    return false;
}

//Check if Kabylake or Coffeelake since CFL is reusing KBL
bool CPUDetect::isKabylakeOrCoffeelake()
{
    if(0x506E == m_i16ProcessorSignature && // Early Kabylake DT/HALO.
         0x8  == m_ucStepping)
       return true;
    if(0x406E == m_i16ProcessorSignature && // Early Kabylake ULT/ULX.
         0x8  == m_ucStepping)
       return true;
    if(0x806E == m_i16ProcessorSignature || // Kabylake ULT/ULX or Coffeelake-U.
       0x906E == m_i16ProcessorSignature)   // Kabylake DT/HALO or Coffeelake-S.
        return true;

    return false;
}

bool CPUDetect::isGeminilake()
{
    if(0x706A == m_i16ProcessorSignature) // GLK Soc with Goldmont Plus CPU
        return true;

    return false;
}

bool CPUDetect::isCannonlake()
{
    if(0x6066 == m_i16ProcessorSignature || // CNL Basic SKU which includes ULT (MCP)
       0x6067 == m_i16ProcessorSignature)   // CNL Halo/DT
       return true;

    return false;
}

bool CPUDetect::isIcelake()
{
    if(0x606A == m_i16ProcessorSignature || // ICX-SP
       0x606C == m_i16ProcessorSignature || // ICX-G
       0x706D == m_i16ProcessorSignature || // ICL DT-Halo
       0x706E == m_i16ProcessorSignature)   // ICL Mobile
       return true;

    return false;
}

// INTEL ATOM.
bool CPUDetect::isBroxton()
{
    // TODO. There shoud be more signatures.
    if (0x506C == m_i16ProcessorSignature)// BXT A stepping?
        return true;

    return false;
}

bool CPUDetect::IsFeatureSupported(ECPUFeatureSupport featureType)
{
    if (m_bBypassCPUDetect)
    {
        return true;
    }

        return (0 != (m_uiCPUFeatures & (unsigned int)featureType));
}
CPUDetect * CPUDetect::GetInstance()
{
    static CPUDetect instance;

    return &instance;
}

bool CPUDetect::ShouldBypassCPUCheck()
{
#ifndef NDEBUG
    string strVal;
    cl_err_code err = GetEnvVar(strVal, "OCL_CFG_BYPASS_CPU_DETECT");
    if (CL_SUCCEEDED(err))
    {
      return true;
    }
#endif
    return false;
}

CPUDetect::CPUDetect(void) :
    m_bBypassCPUDetect(false),
    m_bIsGenuineIntel (false),
    m_ucStepping(0),
    m_ucType(0),
    m_szCPUString(NULL),
    m_szCPUBrandString(NULL),
    m_uiCPUFeatures(0),
    m_uiCoreCount(0),
    m_i16ProcessorSignature(0),
    m_eCPUBrand(BRAND_UNKNOWN)
{
    m_bBypassCPUDetect = ShouldBypassCPUCheck();
    GetCPUInfo();
}

CPUDetect::~CPUDetect(void)
{
    if (m_szCPUString)
    {
        free(m_szCPUString);
    }
    if (m_szCPUBrandString)
    {
        free(m_szCPUBrandString);
    }
}

void CPUDetect::GetCPUInfo()
{
    char vcCPUString[0x20] = {0};
    char vcCPUBrandString[0x40] = {0};
    unsigned int viCPUInfo[4] = {(unsigned int)-1};
    int XCRInfo[2] = {0};

    // get the CPU string and the number of valid of valid IDs
    CPUID(viCPUInfo, 0);
    int iValidIDs = viCPUInfo[0];
    MEMCPY_S( vcCPUString, sizeof(vcCPUString), viCPUInfo + 1, sizeof(unsigned int));
    MEMCPY_S( vcCPUString + 4, sizeof(vcCPUString) - 4, viCPUInfo + 3, sizeof(unsigned int));
    MEMCPY_S( vcCPUString + 8, sizeof(vcCPUString) - 8, viCPUInfo + 2, sizeof(unsigned int));

    m_szCPUString = STRDUP(vcCPUString);
    if(!m_szCPUString)
    {
        m_bIsGenuineIntel = false;
    }
    else
    {
        if (strcmp(m_szCPUString, CPU_STRING) == 0)
        {
            m_bIsGenuineIntel = true;
        }
        else
        {
            m_bIsGenuineIntel = false;
        }
    }

    if (iValidIDs == 1)
    {
        return;
    }

    CPUID(viCPUInfo, 1);

    m_i16ProcessorSignature = (short)(viCPUInfo[0] >> 4);
    m_ucStepping = viCPUInfo[0] & 0xf;
    m_ucType = (viCPUInfo[0] >> 12) & 0x3;
    m_uiCoreCount = (viCPUInfo[1] >> 16) & 0xff;

    m_uiCPUFeatures = 0;
    if (viCPUInfo[3] & 0x04000000)
    {
        m_uiCPUFeatures |= CFS_SSE2;
    }

    if (viCPUInfo[2] & 0x00000001)
    {
        m_uiCPUFeatures |= CFS_SSE3;
    }

    if (viCPUInfo[2] & 0x00000200)
    {
        m_uiCPUFeatures |= CFS_SSSE3;
    }

    if (viCPUInfo[2] & 0x00080000)
    {
        m_uiCPUFeatures |= CFS_SSE41;
    }

    if (viCPUInfo[2] & 0x00100000)
    {
        m_uiCPUFeatures |= CFS_SSE42;
    }

    if (viCPUInfo[2] & 0x18000000)
    {
#if defined(_WIN32) && !defined(_M_X64)
        // Use this inline asm in Win32 only
        __asm
        {
            // specify 0 for XFEATURE_ENABLED_MASK register
            mov ecx, 0
            // XGETBV result in EDX:EAX
            xgetbv
            mov XCRInfo[0], eax
            mov XCRInfo[1], edx
        }
#elif defined(__ANDROID__)
        // No support for AVX on android
        XCRInfo[0] = XCRInfo[0] & ~0x00000006;
#else
        xgetbv( XCRInfo )
#endif
        if ((XCRInfo[0] & 0x00000006) == 0x00000006)
        {
            m_uiCPUFeatures |= CFS_AVX10;
            if ((viCPUInfo[2] & 0x1000) == 0x1000) // Check bit 12 for FMA
            {
                m_uiCPUFeatures |= CFS_FMA;
            }
            // AVX2 support
            viCPUInfo[0] = viCPUInfo[1] = viCPUInfo[2] = viCPUInfo[3] =-1;
            cpuid(viCPUInfo, 7, 0); //eax=7, ecx=0
            if ((viCPUInfo[1] & 0x20) == 0x20) // EBX.AVX2[bit 5]
            {
                m_uiCPUFeatures |= CFS_AVX20;
            }
            // AVX-512 support
            // We use very simple procedure to check AVX-512 features
            // regardless of what Software Developer Manual Vol.1 says
            // because support of EVEX encoded short vectors will be added later.
            if ((viCPUInfo[1] & 0x10000) == 0x10000) // EBX.AVX512F[bit 16]
            {
                // So far following simple logic - we see AVX512DQ then it is SKX, otherwise - it's KNL.
                if ((viCPUInfo[1] & 0x20000) == 0x20000) // CPUID.(EAX=07H, ECX=0):EBX[bit 17] - AVX512DQ
                {
                    m_uiCPUFeatures |= CFS_AVX512F;
                    m_uiCPUFeatures |= CFS_AVX512CD;
                    m_uiCPUFeatures |= CFS_AVX512BW;
                    m_uiCPUFeatures |= CFS_AVX512DQ;
                    m_uiCPUFeatures |= CFS_AVX512VL;
                }
                else
                {
#if defined ENABLE_KNL
                    m_uiCPUFeatures |= CFS_AVX512F;
                    m_uiCPUFeatures |= CFS_AVX512CD;
                    m_uiCPUFeatures |= CFS_AVX512ER;
                    m_uiCPUFeatures |= CFS_AVX512PF;
#else
                    m_uiCPUFeatures |= CFS_AVX20;
#endif
                }
            }
        }
    }

    CPUID(viCPUInfo, 0x80000000);
    unsigned int iValidExIDs = viCPUInfo[0];

    if (iValidExIDs < 0x80000004)
    {
#if defined(__ANDROID__)
        // Android is not supporting Brand String query
        m_szCPUBrandString = STRDUP("Intel(R) Atom(TM)");
#endif
    }
    else
    {
        for (unsigned int i=0x80000000; i <= iValidExIDs; ++i)
        {
            CPUID(viCPUInfo, i);

            // Interpret CPU brand string.
            if (i == 0x80000002)
            {
                MEMCPY_S(vcCPUBrandString, sizeof(vcCPUBrandString), viCPUInfo, sizeof(viCPUInfo));
            }
            else if (i == 0x80000003)
            {
                MEMCPY_S(vcCPUBrandString + 16, sizeof(vcCPUBrandString) - 16, viCPUInfo, sizeof(viCPUInfo));
            }
            else if (i == 0x80000004)
            {
                MEMCPY_S(vcCPUBrandString + 32, sizeof(vcCPUBrandString) - 32, viCPUInfo, sizeof(viCPUInfo));
            }
        }
        m_szCPUBrandString = STRDUP(vcCPUBrandString);
    }

    // detect CPU brand
    if (NULL != m_szCPUBrandString)
    {
        if (m_szCPUBrandString == strstr(m_szCPUBrandString, "Intel(R) Core(TM)"))
        {
            m_eCPUBrand = BRAND_INTEL_CORE;
        }
        else if (m_szCPUBrandString == strstr(m_szCPUBrandString, "Intel(R) Atom(TM)"))
        {
            m_eCPUBrand = BRAND_INTEL_ATOM;
        }
        else if (m_szCPUBrandString == strstr(m_szCPUBrandString, "Intel(R) Pentium(R)"))
        {
            m_eCPUBrand = BRAND_INTEL_PENTIUM;
        }
        else if (m_szCPUBrandString == strstr(m_szCPUBrandString, "Intel(R) Celeron(R)"))
        {
            m_eCPUBrand = BRAND_INTEL_CELERON;
        }
        else if (m_szCPUBrandString == strstr(m_szCPUBrandString, "Intel(R) Xeon(R)"))
        {
            m_eCPUBrand = BRAND_INTEL_XEON;
        }
        else
        {
            // uknown brand name
            m_eCPUBrand = BRAND_UNKNOWN;
        }
    }
}
