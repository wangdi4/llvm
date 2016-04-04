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
// or disclosed in any way without Intel’s prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors
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

bool CPUDetect::IsMicroArchitecture(EMicroArchitecture microArchitecture)
{
    // !!! IMPORTANT NOTE !!!
    //     This whole method is wrong, CPUID's family and model numbers are unreliable
    //     Core Haswell has the same family and model numbers as Atom Cherrytrail !!!
    //     TODO: Implement a better way for detecting different micro-architectures

    if (!IsGenuineIntel())
    {
        return false;
    }

    switch(microArchitecture)
    {
    case MA_ALL:
        return true;
        break;
    case MA_YONAH:
        // TODO: is this correct ?
        if (m_ucFamily == 0x6 && m_ucModel == 0xE)
        {
            return true;
        }
        break;
    case MA_MEROM:
        if ((m_ucFamily == 0x6 && m_ucExtendedModel == 0x0F) || // Clovertown
            (m_ucFamily == 0x6 && m_ucExtendedModel == 0x16))   // Merom Conroe
        {
            return true;
        }
        break;
    case MA_PENRYN:
        if ((m_ucFamily == 0x6 && m_ucExtendedModel == 0x17) || // Yorkfield/Wolfdale
            (m_ucFamily == 0x6 && m_ucExtendedModel == 0x1D))   // Dunnington
        {
            return true;
        }
        break;
    case MA_NEHALEM:
        if ((m_ucFamily == 0x6 && m_ucExtendedModel == 0x1E) || // Clarksfield
            (m_ucFamily == 0x6 && m_ucExtendedModel == 0x1A) || // Bloomfield
            (m_ucFamily == 0x6 && m_ucExtendedModel == 0x2E))   // Nehalem-EX (Xeon)
        {
            return true;
        }
        break;
    case MA_WESTMERE:
        if ((m_ucFamily == 0x6 && m_ucExtendedModel == 0x2C) || // Arrandale/Clarksdale
            (m_ucFamily == 0x6 && m_ucExtendedModel == 0x25) || // Gulftown/Westmere-EP
            (m_ucFamily == 0x6 && m_ucExtendedModel == 0x2F))   // Westmere-EX (Xeon)
        {
            return true;
        }
        break;
    case MA_SANDYBRIDGE:
        if ((m_ucFamily == 0x6 && m_ucExtendedModel == 0x2A) || // SandyBridge
            (m_ucFamily == 0x6 && m_ucExtendedModel == 0x2D))   // SandyBridge-E
        {
            return true;
        }
        break;
    // TODO: find values for ivybridge/haswell/broadwell
    case MA_IVYBRIDGE:
        if (m_ucFamily == 0x6 && m_ucExtendedModel == 0x3A)
        {
            return true;
        }
        break;
    case MA_HASWELL:
        if ((m_ucFamily == 0x6 && m_ucExtendedModel == 0x3C) ||
            (m_ucFamily == 0x6 && m_ucModel == 0x6)) // HSW 4770R
        {
            return true;
        }
        break;
    case MA_BROADWELL:
        if (m_ucFamily == 0x6 && m_ucExtendedModel == 0x3D)
        {
            return true;
        }
        break;
    default:
        break;
    }
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
	m_ucModel(0),
	m_ucExtendedModel(0),
	m_ucFamily(0),
	m_ucType(0),
	m_szCPUString(NULL),
	m_szCPUBrandString(NULL),
	m_uiCPUFeatures(0),
	m_uiCoreCount(0),
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
    m_ucStepping = viCPUInfo[0] & 0xf;
    m_ucModel = (viCPUInfo[0] >> 4) & 0xf;
    m_ucExtendedModel = ((viCPUInfo[0] >> 12) & 0xf0) | m_ucModel;
    m_ucFamily = (viCPUInfo[0] >> 8) & 0xf;
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

