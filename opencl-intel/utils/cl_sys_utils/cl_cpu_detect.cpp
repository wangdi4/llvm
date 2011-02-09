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

#if defined( _WIN32 )
#include <windows.h>
#else
#include <cstdlib>
#include <cstring>
#include "hw_utils.h"
#endif

#if defined( _WIN32 )
	#define STRDUP(X) (_strdup(X))
	#define CPUID(cpu_info, type) __cpuid((int*)(cpu_info), type)
#else
	#define STRDUP(X) (strdup(X))
#if defined (__INTEL_COMPILER)
	#define CPUID(cpu_info, type) __cpuid(cpu_info, type)
#else
	#define CPUID(cpu_info, type) cpuid(cpu_info, type)
#endif
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

bool CPUDetect::IsProcessorType(EProcessorType processorType)
{
    if (!IsGenuineIntel())
    {
        return false;
    }

    switch(processorType)
    {
    case PT_ALL:
        return true;
    	break;
    case PT_YONAH:
        if (m_ucFamily == 0x6 && m_ucModel == 0xE)
        {
            return true;
        }
        break;
    case PT_MEROM:
        if (m_ucFamily == 0x6 && m_ucModel == 0xF)
        {
            return true;
        }
        break;
	case PT_PENRYN:
        if (m_ucFamily == 0xF && m_ucModel == 0x6)
        {
            return true;
        }
        break;
	case PT_NEHALEM:
        if (m_ucFamily == 0x6 && m_ucModel == 0xA)
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
	string strVal;
	cl_err_code err = GetEnvVar(strVal, "OCL_CFG_BYPASS_CPU_DETECT");
	if (CL_SUCCEEDED(err))
	{
		return true;
	}
	return false;
}

CPUDetect::CPUDetect(void) :
	m_bBypassCPUDetect(false),
	m_bIsGenuineIntel (false),
	m_ucStepping(0),
	m_ucModel(0),
	m_ucFamily(0),
	m_ucType(0),
	m_szCPUString(NULL),
	m_szCPUBrandString(NULL),
	m_uiCPUFeatures(0),
	m_uiCoreCount(0)
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

    // get the CPU string and the number of valid of valid IDs
    CPUID(viCPUInfo, 0);
    int iValidIDs = viCPUInfo[0];
    memcpy( vcCPUString,     viCPUInfo + 1, sizeof(unsigned int));
    memcpy( vcCPUString + 4, viCPUInfo + 3, sizeof(unsigned int));
    memcpy( vcCPUString + 8, viCPUInfo + 2, sizeof(unsigned int));

    m_szCPUString = STRDUP(vcCPUString);
	if(!m_szCPUString)
		m_bIsGenuineIntel = false;
    if (strcmp(m_szCPUString, CPU_STRING) == 0)
    {
        m_bIsGenuineIntel = true;
    }
    else
    {
        m_bIsGenuineIntel = false;
    }

    if (iValidIDs == 1)
    {
        return;
    }

    CPUID(viCPUInfo, 1);
    m_ucStepping = viCPUInfo[0] & 0xf;
    m_ucModel = (viCPUInfo[0] >> 4) & 0xf;
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

    CPUID(viCPUInfo, 0x80000000);
    unsigned int iValidExIDs = viCPUInfo[0];

    for (unsigned int i=0x80000000; i <= iValidExIDs; ++i)
    {
        CPUID(viCPUInfo, i);

        // Interpret CPU brand string.
        if (i == 0x80000002)
        {
            memcpy(vcCPUBrandString, viCPUInfo, sizeof(viCPUInfo));
        }
        else if (i == 0x80000003)
        {
            memcpy(vcCPUBrandString + 16, viCPUInfo, sizeof(viCPUInfo));
        }
        else if (i == 0x80000004)
        {
            memcpy(vcCPUBrandString + 32, viCPUInfo, sizeof(viCPUInfo));
        }
    }

    m_szCPUBrandString = STRDUP(vcCPUBrandString);
}

