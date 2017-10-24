// Copyright (c) 2006-2013 Intel Corporation
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

#include "hw_utils.h"

#ifdef __LP64__
#define SAVE_EBX     "mov  %%rbx, %%rdi\n\r"
#define RESTORE_EBX  "xchg %%rdi, %%rbx\n\r"
#else
#define SAVE_EBX     "mov  %%ebx, %%edi\n\r"
#define RESTORE_EBX  "xchg %%edi, %%ebx\n\r"
#endif

extern "C" void ASM_FUNCTION  Intel::OpenCL::Utils::cl_hw_cpuid(CPUID_PARAMS *params)
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

extern "C" void ASM_FUNCTION  Intel::OpenCL::Utils::hw_pause()
{
    __asm__ __volatile__ ("pause");
}

extern "C" void* ASM_FUNCTION  get_next_line_address()
{
    return __builtin_return_address(0);
}

