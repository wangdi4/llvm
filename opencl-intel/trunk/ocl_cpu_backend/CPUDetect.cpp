/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  CPUDetect.cpp

\*****************************************************************************/

#include "CPUDetect.h"
#if defined(_WIN32)
#include <intrin.h>
#endif

#include <assert.h>
#include <string.h>

#if !defined(_WIN32)
struct CPUID_PARAMS {
    unsigned long long   m_rax;
    unsigned long long   m_rbx;
    unsigned long long   m_rcx;
    unsigned long long   m_rdx;
} __attribute__ ((packed));
extern "C" void hw_cpuid( struct CPUID_PARAMS *);

//------------------------------------------------------------------------------
// void ASM_FUNCTION cpuid( int cpuid_info[4], UINT32 type);
//------------------------------------------------------------------------------

#define __cpuid( p_cpuid_info, type )                                          \
{                                                                              \
    struct CPUID_PARAMS __cpuid_params;                                        \
    __cpuid_params.m_rax = type;                                               \
    hw_cpuid( &__cpuid_params);                                                \
                                                                               \
    (p_cpuid_info)[0] = (unsigned int)__cpuid_params.m_rax;                    \
    (p_cpuid_info)[1] = (unsigned int)__cpuid_params.m_rbx;                    \
    (p_cpuid_info)[2] = (unsigned int)__cpuid_params.m_rcx;                    \
    (p_cpuid_info)[3] = (unsigned int)__cpuid_params.m_rdx;                    \
}
#define __cpuidex( p_cpuid_info, type, rcxVal )                                \
{                                                                              \
    struct CPUID_PARAMS __cpuid_params;                                        \
    __cpuid_params.m_rax = type;                                               \
    __cpuid_params.m_rcx = rcxVal;                                               \
    hw_cpuid( &__cpuid_params);                                                \
                                                                               \
    (p_cpuid_info)[0] = (unsigned int)__cpuid_params.m_rax;                    \
    (p_cpuid_info)[1] = (unsigned int)__cpuid_params.m_rbx;                    \
    (p_cpuid_info)[2] = (unsigned int)__cpuid_params.m_rcx;                    \
    (p_cpuid_info)[3] = (unsigned int)__cpuid_params.m_rdx;                    \
}
#endif

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

using namespace Intel::OpenCL::DeviceBackend::Utils;


CPUDetect::CPUDetect(void) : m_uiCPUFeatures(0)
{
	int viCPUInfo[4] = {-1};
	int XCRInfo[2] = {0};

	__cpuid(viCPUInfo, 1);

	m_uiCPUFeatures = 0;
	m_CPU = CPU_LAST;
	if (viCPUInfo[3] & 0x04000000)
	{
		m_uiCPUFeatures |= CFS_SSE2;
		m_CPU = CPU_PENTIUM;		
	}

	if (viCPUInfo[2] & 0x00000001)
	{
		m_uiCPUFeatures |= CFS_SSE3;
		m_CPU = CPU_NOCONA;
	}

	if (viCPUInfo[2] & 0x00000200)
	{
		m_uiCPUFeatures |= CFS_SSSE3;
		m_CPU = CPU_CORE2;
	}

	if (viCPUInfo[2] & 0x00080000)
	{
		m_uiCPUFeatures |= CFS_SSE41;				
		m_CPU = CPU_PENRYN;
	}

	if (viCPUInfo[2] & 0x00100000)
	{
		m_CPU = CPU_COREI7;
		m_uiCPUFeatures |= CFS_SSE42;
	}

	if (viCPUInfo[2] & 0x10000000)
	{
		m_CPU = CPU_SANDYBRIDGE;

		// Check if XSAVE enabled by OS
		if (viCPUInfo[2] & 0x08000000)
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
				m_uiCPUFeatures |= CFS_AVX1;
        /*
        if ((viCPUInfo[2] & 0x1000) == 0x1000) // Check bit 12 for FMA
        {
          m_uiCPUFeatures |= CFS_FMA;
        }*/
        // AVX2 support
        viCPUInfo[0] = viCPUInfo[1] = viCPUInfo[2] = viCPUInfo[3] =-1;
        __cpuidex(viCPUInfo, 7, 0); //eax=7, ecx=0
        if ((viCPUInfo[1] & 0x20) == 0x20) // EBX.AVX2[bit 5]
        {
          m_uiCPUFeatures |= CFS_AVX2;
        }
      }
		}
	}

	assert(m_CPU!=CPU_LAST && "Unknown CPU");

	m_CPUNames[CPU_PENTIUM]     = "pentium";
	m_CPUNames[CPU_NOCONA]      = "nicona";
	m_CPUNames[CPU_CORE2]       = "core2";
	m_CPUNames[CPU_PENRYN]      = "penryn";
	m_CPUNames[CPU_COREI7]      = "corei7";
	m_CPUNames[CPU_SANDYBRIDGE] = "sandybridge";
	m_CPUNames[CPU_HASWELL]     = "haswell";
    m_CPUNames[MIC_KNIGHTSFERRY] = "knf";

#if !defined(_M_X64) && !defined(__LP64__)
	m_CPUPrefixes[CPU_PENTIUM] = "w7";
	m_CPUPrefixes[CPU_NOCONA] = "t7";
	m_CPUPrefixes[CPU_CORE2] = "v8";
	m_CPUPrefixes[CPU_PENRYN] = "p8";
	m_CPUPrefixes[CPU_COREI7] = "n8";
	m_CPUPrefixes[CPU_SANDYBRIDGE] = "g9";
	m_CPUPrefixes[CPU_HASWELL] = "g9";
    m_CPUPrefixes[MIC_KNIGHTSFERRY] = "knf";
#else
	m_CPUPrefixes[CPU_PENTIUM] = "unknown";
	m_CPUPrefixes[CPU_NOCONA] = "e7";
	m_CPUPrefixes[CPU_CORE2] = "u8";
	m_CPUPrefixes[CPU_PENRYN] = "y8";
	m_CPUPrefixes[CPU_COREI7] = "h8";
	m_CPUPrefixes[CPU_SANDYBRIDGE] = "e9";
	m_CPUPrefixes[CPU_HASWELL] = "e9";
    m_CPUPrefixes[MIC_KNIGHTSFERRY] = "knf";
#endif
}

Intel::ECPU CPUDetect::GetCPUByName(const char *CPUName) const
{
	for (unsigned i=0; i<CPU_LAST; ++i)
		if (!strcmp(m_CPUNames[i], CPUName))
			return (ECPU)i;
	assert(false && "Unknown CPU Name");
	return CPU_LAST;
}

bool CPUDetect::IsMICCPU(Intel::ECPU cpuId)
{
    return (cpuId >= MIC_KNIGHTSFERRY);
}

CPUDetect::~CPUDetect(void) 
{
}

CPUDetect CPUDetect::m_Instance;


