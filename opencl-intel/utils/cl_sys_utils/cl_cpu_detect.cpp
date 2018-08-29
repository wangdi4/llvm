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
    return m_cpuArch == WST || m_cpuArch == WST_XEON;
}

bool CPUDetect::isSandyBridge()
{
    return m_cpuArch == SNB || m_cpuArch == SNB_XEON;
}

bool CPUDetect::isIvyBridge()
{
    return m_cpuArch == IVB || m_cpuArch == IVB_XEON;
}

bool CPUDetect::isHaswell()
{
    return m_cpuArch == HSW || m_cpuArch == HSW_XEON;
}


bool CPUDetect::isBroadwell()
{
    return m_cpuArch == BDW || m_cpuArch == BDW_XEON;
}

bool CPUDetect::isSkylake()
{
    return m_cpuArch == SKL || m_cpuArch == SKX;
}

//Check if Kabylake or Coffeelake since CFL is reusing KBL
bool CPUDetect::isKabylakeOrCoffeelake()
{
    return m_cpuArch == KBL;
}

bool CPUDetect::isGeminilake()
{
    return m_cpuArch == GLK;
}

bool CPUDetect::isCannonlake()
{
    return m_cpuArch == CNL;
}

bool CPUDetect::isIcelake()
{
    return m_cpuArch == ICL;
}

// INTEL ATOM.
bool CPUDetect::isBroxton()
{
    return m_cpuArch == BXT;
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
    m_szCPUString(nullptr),
    m_szCPUBrandString(nullptr),
    m_uiCPUFeatures(0),
    m_uiCoreCount(0),
    m_i16ProcessorSignature(0),
    m_eCPUBrand(BRAND_UNKNOWN)
{
    m_bBypassCPUDetect = ShouldBypassCPUCheck();
    GetCPUInfo();

    switch(m_i16ProcessorSignature)
    {
        case 0x2065: // Arrandale/Clarksdale
        case 0x206C: // Gulftown/Westmere-EP
            m_cpuArch = WST;
            break;
        case 0x206F: // Westmere-EX (Xeon)
            m_cpuArch = WST_XEON;
            break;
        case 0x206A: // SandyBridge DT
            m_cpuArch = SNB;
            break;
        case 0x206D: // SandyBridge Server
            m_cpuArch = SNB_XEON;
            break;
        case 0x306A:// IvyBridge DT
            m_cpuArch = IVB;
            break;
        case 0x306E:  // IvyBridge Server
            m_cpuArch = IVB_XEON;
            break;
        case 0x306C:// Haswell DT
        case 0x4065:// Haswell U
            m_cpuArch = HSW;
            break;
        case 0x306F:// Haswell Server
            m_cpuArch = HSW_XEON;
            break;
        case 0x306D:// Broadwell ULT Client.
        case 0x4067:// Broadwell Client Halo.
            m_cpuArch = BDW;
            break;
        case 0x406F:// Broadwell Server
            m_cpuArch = BDW_XEON;
            break;
        case 0x406E:// Skylake ULT/ULX.
        case 0x506E:// Skylake DT/HALO.
            if(0x8 == m_ucStepping) //Early Kabylake
            {
                m_cpuArch = KBL;
                break;
            }
            m_cpuArch = SKL;
            break;
        case 0x5065:// SkylakeX
            m_cpuArch = SKX;
            break;
        case 0x806E:// Kabylake ULT/ULX or Coffeelake-U.
        case 0x906E:// Kabylake DT/HALO or Coffeelake-S.
            m_cpuArch = KBL;
            break;
        case 0x706A:// GLK Soc with Goldmont Plus CPU
            m_cpuArch = GLK;
            break;
        case 0x6066:// CNL Basic SKU which includes ULT (MCP)
        case 0x6067:// CNL Halo/DT
            m_cpuArch = CNL;
            break;
        case 0x606A:// ICX-SP
        case 0x606C:// ICX-G
        case 0x706D:// ICL DT-Halo
        case 0x706E:// ICL Mobile
            m_cpuArch = ICL;
            break;
        case 0x506C:// BXT A stepping?
            m_cpuArch = BXT;
            break;
        default:
            m_cpuArch = UNKNOWN;
    }
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

    if (iValidExIDs >= 0x80000004)
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
    if (nullptr != m_szCPUBrandString)
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
