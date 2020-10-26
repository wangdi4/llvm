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

#include "CPUDetect.h"
#if defined(_WIN32)
#include <intrin.h>
#else
#include "cpuid.h"
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

using namespace Intel::OpenCL::DeviceBackend::Utils;

CPUDetect::CPUDetect(void)
{
    int viCPUInfo[4] = {-1, -1, -1, -1};
    int XCRInfo[2] = {0, 0};

#ifdef _WIN32
    __cpuid(viCPUInfo, 1);
#else
    __get_cpuid(1, (unsigned*)&viCPUInfo[0], (unsigned*)&viCPUInfo[1],
                (unsigned*)&viCPUInfo[2], (unsigned*)&viCPUInfo[3]);
#endif

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
    if (viCPUInfo[2] & 0x20000000)
    {
        uiCPUFeatures |= CFS_F16C;
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
#ifdef _WIN32
                __cpuidex(viCPUInfo, 7, 0); //eax=7, ecx=0
#else
                __cpuid_count(7, 0, viCPUInfo[0], viCPUInfo[1], viCPUInfo[2],
                              viCPUInfo[3]);
#endif
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
                      // Ok, we have AVX512. Let's check IFMA for CNL level.
                      if ((viCPUInfo[1] & 0x200000) == 0x200000) // CPUID.(EAX=07H, ECX=0):EBX[bit 21] - AVX512IFMA
                      {
                        uiCPUFeatures |= CFS_AVX512VBMI;
                        uiCPUFeatures |= CFS_AVX512IFMA;
                        // Check for Icelake-client level VBMI2.
                        if ((viCPUInfo[2] & 0x40) == 0x40) // CPUID.(EAX=07H, ECX=0):ECX[bit 06] - AVX512VBMI2
                        {
                          uiCPUFeatures |= CFS_AVX512VBMI2;
                          uiCPUFeatures |= CFS_AVX512BITALG;
                          if ((viCPUInfo[3] & 0x40000) == 0x40000) // CPUID.(EAX=07H, ECX=0):EDX[bit 18] - PCONFIG
                            CPU = CPU_ICX;
                          else
                            CPU = CPU_ICL;
                        }
                        else
                          llvm_unreachable("Do not expect that CPU");
                      }
                      else
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
