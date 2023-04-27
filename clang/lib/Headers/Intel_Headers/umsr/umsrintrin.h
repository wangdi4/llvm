/*===--------------- umsrintrin.h - UMSR intrinsics -----------------===
 *
 * INTEL CONFIDENTIAL
 *
 * Modifications, Copyright (C) 2023 Intel Corporation. All rights reserved.
 *
 * The information and source code contained herein is the exclusive property
 * of Intel Corporation and may not be disclosed, examined or reproduced in
 * whole or in part without explicit written authorization from the company.
 *
 *===----------------------------------------------------------------------===
 */
#ifndef __IMMINTRIN_H
#error "Never use <umsrintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __UMSRINTRIN_H
#define __UMSRINTRIN_H
#ifdef __x86_64__

static __inline__ unsigned long long
    __attribute__((__always_inline__, __nodebug__, __target__("umsr")))
    _urdmsr(unsigned long long __A) {
  return __builtin_ia32_urdmsr(__A);
}

static __inline__ void
    __attribute__((__always_inline__, __nodebug__, __target__("umsr")))
    _uwrmsr(unsigned long long __A, unsigned long long __B) {
  return __builtin_ia32_uwrmsr(__A, __B);
}

#endif // __x86_64__
#endif // __UMSRINTRIN_H
