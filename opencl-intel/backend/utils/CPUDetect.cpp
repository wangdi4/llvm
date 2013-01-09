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


CPUDetect::CPUDetect(void)
{
    int viCPUInfo[4] = {-1};
    int XCRInfo[2] = {0};

    __cpuid(viCPUInfo, 1);

    unsigned int uiCPUFeatures = 0;
    ECPU CPU = DEVICE_INVALID;
    if (viCPUInfo[3] & 0x04000000)
    {
        uiCPUFeatures |= CFS_SSE2;
        CPU = CPU_PENTIUM;
    }

    if (viCPUInfo[2] & 0x00000001)
    {
        uiCPUFeatures |= CFS_SSE3;
        CPU = CPU_NOCONA;
    }

    if (viCPUInfo[2] & 0x00000200)
    {
        uiCPUFeatures |= CFS_SSSE3;
        CPU = CPU_CORE2;
    }

    if (viCPUInfo[2] & 0x00080000)
    {
        uiCPUFeatures |= CFS_SSE41;
        CPU = CPU_PENRYN;
    }

    if (viCPUInfo[2] & 0x00100000)
    {
        CPU = CPU_COREI7;
        uiCPUFeatures |= CFS_SSE42;
    }

    if (viCPUInfo[2] & 0x10000000)
    {
        CPU = CPU_SANDYBRIDGE;

        // Check if XSAVE enabled by OS
        if (viCPUInfo[2] & 0x08000000)
        {

#if defined(_WIN32)
#if defined(_M_X64)
          xgetbv( XCRInfo )
#else
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
#endif
#elif defined(__APPLE__)
          xgetbv( XCRInfo )
#endif
                if ((XCRInfo[0] & 0x00000006) == 0x00000006)
                {
                    uiCPUFeatures |= CFS_AVX1;

                    if ((viCPUInfo[2] & 0x1000) == 0x1000) // Check bit 12 for FMA
                    {
                        uiCPUFeatures |= CFS_FMA;
                    }
                    // AVX2 support
                    viCPUInfo[0] = viCPUInfo[1] = viCPUInfo[2] = viCPUInfo[3] =-1;
                    __cpuidex(viCPUInfo, 7, 0); //eax=7, ecx=0
                    if ((viCPUInfo[1] & 0x20) == 0x20) // EBX.AVX2[bit 5]
                    {
                        uiCPUFeatures |= CFS_AVX2;
                        CPU = CPU_HASWELL;
                    }
                }
        }
    }

    assert(CPU!=DEVICE_INVALID && "Unknown CPU");
    m_CPUId = CPUId(CPU, uiCPUFeatures, sizeof(void*)==8);
}

CPUDetect::~CPUDetect(void) 
{
}

CPUDetect* CPUDetect::m_Instance = 0;
