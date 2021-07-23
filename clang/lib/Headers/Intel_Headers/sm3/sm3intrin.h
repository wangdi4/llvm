/*===-------------------- sm3intrin.h - SM3 intrinsics ---------------------===
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
#error "Never use <sm3intrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __SM3INTRIN_H
#define __SM3INTRIN_H

#define __DEFAULT_FN_ATTRS128                                                  \
  __attribute__((__always_inline__, __nodebug__, __target__("sm3"),            \
                 __min_vector_width__(128)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_sm3msg1_epi32(__m128i __A,
                                                                  __m128i __B,
                                                                  __m128i __C) {
  return (__m128i)__builtin_ia32_vsm3msg1((__v4su)__A, (__v4su)__B,
                                          (__v4su)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_sm3msg2_epi32(__m128i __A,
                                                                  __m128i __B,
                                                                  __m128i __C) {
  return (__m128i)__builtin_ia32_vsm3msg2((__v4su)__A, (__v4su)__B,
                                          (__v4su)__C);
}

#define _mm_sm3rnds2_epi32(A, B, C, D)                                         \
  (__m128i) __builtin_ia32_vsm3rnds2((__v4su)A, (__v4su)B, (__v4su)C, (int)D)

#undef __DEFAULT_FN_ATTRS128

#endif // __SM3INTRIN_H
