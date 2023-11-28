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

#pragma once

#include "hw_defs.h"

#ifdef _WIN32
#include <intrin.h>
#endif

namespace Intel {
namespace OpenCL {
namespace Utils {

#ifdef _WIN32
#pragma pack(push)
#endif

typedef struct {
  UINT64 m_rax;
  UINT64 m_rbx;
  UINT64 m_rcx;
  UINT64 m_rdx;
} PACKED CPUID_PARAMS;

#ifdef _WIN32
inline void cpuid(UINT32 cpuid_info[], UINT32 type, UINT32 ecxVal = 0) {
  cpuid_info[0] = type;
  cpuid_info[2] = ecxVal;
  __cpuid((int *)cpuid_info, type);
}

inline unsigned int hw_cpu_idx() {
  UINT32 cpuid_info[4];
  cpuid(cpuid_info, 1);
  return (cpuid_info[1] >> 24);
}
#else
extern "C" void ASM_FUNCTION cl_hw_cpuid(CPUID_PARAMS *);

inline void cpuid(UINT32 cpuid_info[], UINT32 type, UINT32 ecxVal = 0) {
  CPUID_PARAMS __cpuid_params;
  __cpuid_params.m_rax = type;
  __cpuid_params.m_rcx = ecxVal;
  cl_hw_cpuid(&__cpuid_params);

  cpuid_info[0] = (UINT32)__cpuid_params.m_rax;
  cpuid_info[1] = (UINT32)__cpuid_params.m_rbx;
  cpuid_info[2] = (UINT32)__cpuid_params.m_rcx;
  cpuid_info[3] = (UINT32)__cpuid_params.m_rdx;
}

inline unsigned int hw_cpu_idx() {
  CPUID_PARAMS __cpuid_params;
  __cpuid_params.m_rax = 1;
  cl_hw_cpuid(&__cpuid_params);
  return (UINT32)__cpuid_params.m_rbx >> 24;
}
#endif

inline unsigned long long RDTSC(void) {
#ifdef _WIN32
  return __rdtsc();
#else
  unsigned int a, d;
  __asm__ __volatile__("rdtsc" : "=a"(a), "=d"(d));
  return (((unsigned long long)a) | (((unsigned long long)d) << 32));
#endif
}

extern "C" void ASM_FUNCTION hw_pause();

#ifdef _WIN32
extern "C" void *ASM_FUNCTION get_next_line_address();
#else
extern "C" void *ASM_FUNCTION get_next_line_address() __attribute__((noinline));
#endif

#ifdef _WIN32
#pragma pack(pop)
#endif

} // namespace Utils
} // namespace OpenCL
} // namespace Intel
