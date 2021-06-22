/*===----------- avxvnniint16intrin.h - AVXVNNIINT16 intrinsics-------------===
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
#error                                                                         \
    "Never use <avxvnniint16intrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVXVNNIINT16INTRIN_H
#define __AVXVNNIINT16INTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128                                                  \
  __attribute__((__always_inline__, __nodebug__, __target__("avxvnniint16"),   \
                 __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256                                                  \
  __attribute__((__always_inline__, __nodebug__, __target__("avxvnniint16"),   \
                 __min_vector_width__(256)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_dpwsud_epi32(__m128i __A,
                                                                 __m128i __B,
                                                                 __m128i __C) {
  return (__m128i)__builtin_ia32_vpdpwsud128((__v4si)__A, (__v4si)__B,
                                             (__v4si)__C);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_dpwsud_epi32(__m256i __A, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_vpdpwsud256((__v8si)__A, (__v8si)__B,
                                             (__v8si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_dpwsuds_epi32(__m128i __A,
                                                                  __m128i __B,
                                                                  __m128i __C) {
  return (__m128i)__builtin_ia32_vpdpwsuds128((__v4si)__A, (__v4si)__B,
                                              (__v4si)__C);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_dpwsuds_epi32(__m256i __A, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_vpdpwsuds256((__v8si)__A, (__v8si)__B,
                                              (__v8si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_dpwusd_epi32(__m128i __A,
                                                                 __m128i __B,
                                                                 __m128i __C) {
  return (__m128i)__builtin_ia32_vpdpwusd128((__v4si)__A, (__v4si)__B,
                                             (__v4si)__C);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_dpwusd_epi32(__m256i __A, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_vpdpwusd256((__v8si)__A, (__v8si)__B,
                                             (__v8si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_dpwusds_epi32(__m128i __A,
                                                                  __m128i __B,
                                                                  __m128i __C) {
  return (__m128i)__builtin_ia32_vpdpwusds128((__v4si)__A, (__v4si)__B,
                                              (__v4si)__C);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_dpwusds_epi32(__m256i __A, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_vpdpwusds256((__v8si)__A, (__v8si)__B,
                                              (__v8si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_dpwuud_epi32(__m128i __A,
                                                                 __m128i __B,
                                                                 __m128i __C) {
  return (__m128i)__builtin_ia32_vpdpwuud128((__v4si)__A, (__v4si)__B,
                                             (__v4si)__C);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_dpwuud_epi32(__m256i __A, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_vpdpwuud256((__v8si)__A, (__v8si)__B,
                                             (__v8si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_dpwuuds_epi32(__m128i __A,
                                                                  __m128i __B,
                                                                  __m128i __C) {
  return (__m128i)__builtin_ia32_vpdpwuuds128((__v4si)__A, (__v4si)__B,
                                              (__v4si)__C);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_dpwuuds_epi32(__m256i __A, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_vpdpwuuds256((__v8si)__A, (__v8si)__B,
                                              (__v8si)__C);
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVXVNNIINT16INTRIN_H
