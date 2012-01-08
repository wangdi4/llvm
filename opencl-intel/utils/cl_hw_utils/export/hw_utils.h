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

namespace Intel { namespace OpenCL { namespace Utils {

#pragma PACK_ON

typedef struct {
    UINT64   m_rax;
    UINT64   m_rbx;
    UINT64   m_rcx;
    UINT64   m_rdx;
} PACKED CPUID_PARAMS;

extern "C" void ASM_FUNCTION  hw_cpuid(CPUID_PARAMS *);

//------------------------------------------------------------------------------
// void ASM_FUNCTION cpuid( int cpuid_info[4], UINT32 type);
//------------------------------------------------------------------------------

#define cpuid( p_cpuid_info, type )                                            \
{                                                                              \
    CPUID_PARAMS __cpuid_params;                                               \
    __cpuid_params.m_rax = type;                                               \
    hw_cpuid(&__cpuid_params);                                                 \
                                                                               \
    (p_cpuid_info)[0] = (UINT32)__cpuid_params.m_rax;                          \
    (p_cpuid_info)[1] = (UINT32)__cpuid_params.m_rbx;                          \
    (p_cpuid_info)[2] = (UINT32)__cpuid_params.m_rcx;                          \
    (p_cpuid_info)[3] = (UINT32)__cpuid_params.m_rdx;                          \
}

extern "C" void ASM_FUNCTION  hw_pause();

extern "C" void* ASM_FUNCTION  get_next_line_address();

#pragma PACK_OFF

}}}
