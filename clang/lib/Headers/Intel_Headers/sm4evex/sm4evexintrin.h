/*===--------------- sm4intrin.h - SM4 intrinsics -----------------===
 *
 * Copyright (C) 2021 Intel Corporation. All rights reserved.
 *
 * The information and source code contained herein is the exclusive property
 * of Intel Corporation and may not be disclosed, examined or reproduced in
 * whole or in part without explicit written authorization from the company.
 *
 *===----------------------------------------------------------------------===
 */
#ifndef __IMMINTRIN_H
#error "Never use <sm4evexintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __SM4EVEXINTRIN_H
#define __SM4EVEXINTRIN_H

#define __DEFAULT_FN_ATTRS512                                                  \
  __attribute__((__always_inline__, __nodebug__, __target__("sm4,avx512f"),    \
                 __min_vector_width__(512)))

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_sm4key4_epi32(__m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_vsm4key4512((__v16su)__A, (__v16su)__B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_sm4rnds4_epi32(__m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_vsm4rnds4512((__v16su)__A, (__v16su)__B);
}

#undef __DEFAULT_FN_ATTRS512

#endif // __SM4EVEXINTRIN_H
