// Copyright (c) 2006-2012 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#pragma once

#include "hw_defs.h"

#ifdef WIN32
    #include "intrin.h"
#endif

namespace Intel { namespace OpenCL { namespace Utils {

#ifdef WIN32
    #pragma PACK_ON
#endif

typedef struct {
    UINT64   m_rax;
    UINT64   m_rbx;
    UINT64   m_rcx;
    UINT64   m_rdx;
} PACKED CPUID_PARAMS;

#ifdef WIN32
	inline void cpuid( UINT32 cpuid_info[], UINT32 type, UINT32 ecxVal = 0 )
	{
		cpuid_info[0] = type;
		cpuid_info[2] = ecxVal;
		__cpuid( (int*)cpuid_info, type );
	}

	inline unsigned int hw_cpu_idx()
	{
		UINT32 cpuid_info[4];
		cpuid( cpuid_info, 1 );
		return (cpuid_info[1] >> 24);
	}
#else
	extern "C" void ASM_FUNCTION  hw_cpuid(CPUID_PARAMS *);

	inline void cpuid( UINT32 cpuid_info[], UINT32 type, UINT32 ecxVal = 0 )
	{
		CPUID_PARAMS __cpuid_params;
		__cpuid_params.m_rax = type;
		__cpuid_params.m_rcx = ecxVal;
		hw_cpuid(&__cpuid_params);

		cpuid_info[0] = (UINT32)__cpuid_params.m_rax;
		cpuid_info[1] = (UINT32)__cpuid_params.m_rbx;
		cpuid_info[2] = (UINT32)__cpuid_params.m_rcx;
		cpuid_info[3] = (UINT32)__cpuid_params.m_rdx;
	}

	inline unsigned int hw_cpu_idx()
	{
		CPUID_PARAMS __cpuid_params;
		__cpuid_params.m_rax = 1;
		hw_cpuid(&__cpuid_params);
		return (UINT32)__cpuid_params.m_rbx >> 24;
	}
#endif

inline unsigned long long RDTSC(void)
{
#ifdef WIN32
    return __rdtsc();
#else
     unsigned int a, d;
     __asm__ __volatile__("rdtsc" : "=a" (a), "=d" (d));
     return (((unsigned long long)a) | (((unsigned long long)d) << 32));
#endif
}

extern "C" void ASM_FUNCTION  hw_pause();

#ifdef WIN32
    extern "C" void* ASM_FUNCTION  get_next_line_address();
#else
    extern "C" void* ASM_FUNCTION  get_next_line_address() __attribute__((noinline));
#endif

#ifdef WIN32
    #pragma PACK_OFF
#endif

}}}
