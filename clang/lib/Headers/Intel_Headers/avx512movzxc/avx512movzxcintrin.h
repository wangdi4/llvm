/*===--------------- avx512movzxcintrin.h - AVX512MOVZXC intrinsics -----------------===
 *
 * Copyright (C) 2023 Intel Corporation. All rights reserved.
 *
 * The information and source code contained herein is the exclusive property
 * of Intel Corporation and may not be disclosed, examined or reproduced in
 * whole or in part without explicit written authorization from the company.
 *
 *===----------------------------------------------------------------------===
 */
#ifndef __IMMINTRIN_H
#error "Never use <avx512movzxcintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVX512MOVZXCINTRIN_H
#define __AVX512MOVZXCINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512movzxc"), __min_vector_width__(128)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_move_epi32(__m128i __A) {
  return (__m128i)__builtin_shufflevector((__v4si)__A, (__v4si)_mm_setzero_si128(), 0, 4, 4, 4);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_move_epi16(__m128i __A) {
  return (__m128i)__builtin_shufflevector((__v8hi)__A, (__v8hi)_mm_setzero_si128(), 0, 8, 8, 8, 8, 8, 8, 8);
}

#undef __DEFAULT_FN_ATTRS128

#endif // __AVX512MOVZXCINTRIN_H