/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

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

#if !defined(_M_X64) && !defined(__LP64__)
// we have a inline assembler implementation for non windows 32 bit platforms
#define SAVE_EBX     "mov  %%ebx, %%edi\n\r"
#define RESTORE_EBX  "xchg %%edi, %%ebx\n\r"
typedef unsigned int UINT32;

extern "C" void  hw_cpuid(CPUID_PARAMS *params)
{
    UINT32 type = (UINT32)params->m_rax;
    UINT32 eax, ebx, ecx, edx;

    // Android NDK 32-bit compiler ignores clobbered registers
    // list, hence, i am preserving here the %ebx register
    // which might be used by the compiler
    __asm__ __volatile__(SAVE_EBX
			 "cpuid\n\r"
			 RESTORE_EBX
			 : "=a" (eax),
			   "=D" (ebx),
			   "=c" (ecx),
			   "=d" (edx)
                         : "a" (type));

    params->m_rax = eax;
    params->m_rbx = ebx;
    params->m_rcx = ecx;
    params->m_rdx = edx;
}
#else
    extern "C" void hw_cpuid( struct CPUID_PARAMS *);
#endif

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
            int c = 0;
            __asm__("xgetbv" : "=a"(XCRInfo[0]), "=d"(XCRInfo[1]) : "c"(c));
#else
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
                if ((viCPUInfo[1] & 0x8) == 0x8)
                    uiCPUFeatures |= CFS_BMI;
                if ((viCPUInfo[1] & 0x100) == 0x100)
                    uiCPUFeatures |= CFS_BMI2;
                // AVX-512 support
                // We use very simple procedure to check AVX-512 features
                // regardless of what Software Developer Manual Vol.1 says
                // because support of EVEX encoded short vectors will be added later.
                if ((viCPUInfo[1] & 0x10000) == 0x10000) // EBX.AVX512F[bit 16]
                {
                    // So far following simple logic - we see AVX512DQ then it is SKX, otherwise - it's KNL.
                    if ((viCPUInfo[1] & 0x20000) == 0x20000) // CPUID.(EAX=07H, ECX=0):EBX[bit 17] - AVX512DQ
                    {
                      uiCPUFeatures |= CFS_AVX512F;
                      uiCPUFeatures |= CFS_AVX512CD;
                      uiCPUFeatures |= CFS_AVX512BW;
                      uiCPUFeatures |= CFS_AVX512DQ;
                      uiCPUFeatures |= CFS_AVX512VL;
                      CPU = CPU_SKX;
                    }
                    else
                    {
#if defined ENABLE_KNL
                      uiCPUFeatures |= CFS_AVX512F;
                      uiCPUFeatures |= CFS_AVX512CD;
                      uiCPUFeatures |= CFS_AVX512ER;
                      uiCPUFeatures |= CFS_AVX512PF;
                      CPU = CPU_KNL;
#else
                      uiCPUFeatures |= CFS_AVX2;
                      CPU = CPU_HASWELL;
#endif
                    }
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
