// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

#include "hw_utils.h"

#ifdef __LP64__
#define SAVE_EBX "mov  %%rbx, %%rdi\n\r"
#define RESTORE_EBX "xchg %%rdi, %%rbx\n\r"
#else
#define SAVE_EBX "mov  %%ebx, %%edi\n\r"
#define RESTORE_EBX "xchg %%edi, %%ebx\n\r"
#endif

extern "C" void ASM_FUNCTION
Intel::OpenCL::Utils::cl_hw_cpuid(CPUID_PARAMS *params) {
  UINT32 type = (UINT32)params->m_rax;
  UINT32 eax, ebx, ecx, edx;

  // Android NDK 32-bit compiler ignores clobbered registers
  // list, hence, i am preserving here the %ebx register
  // which might be used by the compiler
  __asm__ __volatile__(SAVE_EBX "cpuid\n\r" RESTORE_EBX
                       : "=a"(eax), "=D"(ebx), "=c"(ecx), "=d"(edx)
                       : "a"(type));

  params->m_rax = eax;
  params->m_rbx = ebx;
  params->m_rcx = ecx;
  params->m_rdx = edx;
}

extern "C" void ASM_FUNCTION Intel::OpenCL::Utils::hw_pause() {
  __asm__ __volatile__("pause");
}

extern "C" void *ASM_FUNCTION get_next_line_address() {
  return __builtin_return_address(0);
}
