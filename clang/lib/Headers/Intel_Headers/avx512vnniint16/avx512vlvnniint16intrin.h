/*===---- avx512vlvnniint16intrin.h - AVX512VLVNNIINT16 intrinsics---------===
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
    "Never use <avx512vlvnniint16intrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVX512VLVNNIINT16INTRIN_H
#define __AVX512VLVNNIINT16INTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512vl,avx512vnniint16"),                       \
                 __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512vl,avx512vnniint16"),                       \
                 __min_vector_width__(256)))

/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ISA_AVX256P */
#if defined(__AVX256P__)
#define __DEFAULT_FN_ATTRS128                                                  \
  __attribute__((__always_inline__, __nodebug__, __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256                                                  \
  __attribute__((__always_inline__, __nodebug__, __min_vector_width__(256)))
#endif
/* end INTEL_FEATURE_ISA_AVX256P */
/* end INTEL_CUSTOMIZATION */

#define _mm_dpwsud_epi32(__A, __B, __C)                                        \
  (__m128i) __builtin_ia32_vpdpwsud128((__v4si)__A, (__v4si)__B, (__v4si)__C)

#define _mm256_dpwsud_epi32(__A, __B, __C)                                     \
  (__m256i) __builtin_ia32_vpdpwsud256((__v8si)__A, (__v8si)__B, (__v8si)__C)

#define _mm_dpwsuds_epi32(__A, __B, __C)                                       \
  (__m128i) __builtin_ia32_vpdpwsuds128((__v4si)__A, (__v4si)__B, (__v4si)__C)

#define _mm256_dpwsuds_epi32(__A, __B, __C)                                    \
  (__m256i) __builtin_ia32_vpdpwsuds256((__v8si)__A, (__v8si)__B, (__v8si)__C)

#define _mm_dpwusd_epi32(__A, __B, __C)                                        \
  (__m128i) __builtin_ia32_vpdpwusd128((__v4si)__A, (__v4si)__B, (__v4si)__C)

#define _mm256_dpwusd_epi32(__A, __B, __C)                                     \
  (__m256i) __builtin_ia32_vpdpwusd256((__v8si)__A, (__v8si)__B, (__v8si)__C)

#define _mm_dpwusds_epi32(__A, __B, __C)                                       \
  (__m128i) __builtin_ia32_vpdpwusds128((__v4si)__A, (__v4si)__B, (__v4si)__C)

#define _mm256_dpwusds_epi32(__A, __B, __C)                                    \
  (__m256i) __builtin_ia32_vpdpwusds256((__v8si)__A, (__v8si)__B, (__v8si)__C)

#define _mm_dpwuud_epi32(__A, __B, __C)                                        \
  (__m128i) __builtin_ia32_vpdpwuud128((__v4si)__A, (__v4si)__B, (__v4si)__C)

#define _mm256_dpwuud_epi32(__A, __B, __C)                                     \
  (__m256i) __builtin_ia32_vpdpwuud256((__v8si)__A, (__v8si)__B, (__v8si)__C)

#define _mm_dpwuuds_epi32(__A, __B, __C)                                       \
  (__m128i) __builtin_ia32_vpdpwuuds128((__v4si)__A, (__v4si)__B, (__v4si)__C)

#define _mm256_dpwuuds_epi32(__A, __B, __C)                                    \
  (__m256i) __builtin_ia32_vpdpwuuds256((__v8si)__A, (__v8si)__B, (__v8si)__C)

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_dpwsud_epi32(__m128i __A, __mmask8 __U, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_selectd_128(
      (__mmask8)__U, (__v4si)_mm_dpwsud_epi32(__A, __B, __C), (__v4si)__A);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_dpwsud_epi32(__m128i __A, __mmask8 __U, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_selectd_128(
      (__mmask8)__U, (__v4si)_mm_dpwsud_epi32(__A, __B, __C),
      (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_dpwsud_epi32(__m256i __A, __mmask8 __U, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_selectd_256(
      (__mmask8)__U, (__v8si)_mm256_dpwsud_epi32(__A, __B, __C), (__v8si)__A);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_dpwsud_epi32(__m256i __A, __mmask8 __U, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_selectd_256(
      (__mmask8)__U, (__v8si)_mm256_dpwsud_epi32(__A, __B, __C),
      (__v8si)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_dpwsuds_epi32(__m128i __A, __mmask8 __U, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_selectd_128(
      (__mmask8)__U, (__v4si)_mm_dpwsuds_epi32(__A, __B, __C), (__v4si)__A);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_dpwsuds_epi32(__m128i __A, __mmask8 __U, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_selectd_128(
      (__mmask8)__U, (__v4si)_mm_dpwsuds_epi32(__A, __B, __C),
      (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_dpwsuds_epi32(__m256i __A, __mmask8 __U, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_selectd_256(
      (__mmask8)__U, (__v8si)_mm256_dpwsuds_epi32(__A, __B, __C), (__v8si)__A);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_dpwsuds_epi32(__m256i __A, __mmask8 __U, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_selectd_256(
      (__mmask8)__U, (__v8si)_mm256_dpwsuds_epi32(__A, __B, __C),
      (__v8si)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_dpwusd_epi32(__m128i __A, __mmask8 __U, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_selectd_128(
      (__mmask8)__U, (__v4si)_mm_dpwusd_epi32(__A, __B, __C), (__v4si)__A);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_dpwusd_epi32(__m128i __A, __mmask8 __U, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_selectd_128(
      (__mmask8)__U, (__v4si)_mm_dpwusd_epi32(__A, __B, __C),
      (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_dpwusd_epi32(__m256i __A, __mmask8 __U, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_selectd_256(
      (__mmask8)__U, (__v8si)_mm256_dpwusd_epi32(__A, __B, __C), (__v8si)__A);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_dpwusd_epi32(__m256i __A, __mmask8 __U, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_selectd_256(
      (__mmask8)__U, (__v8si)_mm256_dpwusd_epi32(__A, __B, __C),
      (__v8si)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_dpwusds_epi32(__m128i __A, __mmask8 __U, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_selectd_128(
      (__mmask8)__U, (__v4si)_mm_dpwusds_epi32(__A, __B, __C), (__v4si)__A);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_dpwusds_epi32(__m128i __A, __mmask8 __U, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_selectd_128(
      (__mmask8)__U, (__v4si)_mm_dpwusds_epi32(__A, __B, __C),
      (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_dpwusds_epi32(__m256i __A, __mmask8 __U, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_selectd_256(
      (__mmask8)__U, (__v8si)_mm256_dpwusds_epi32(__A, __B, __C), (__v8si)__A);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_dpwusds_epi32(__m256i __A, __mmask8 __U, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_selectd_256(
      (__mmask8)__U, (__v8si)_mm256_dpwusds_epi32(__A, __B, __C),
      (__v8si)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_dpwuud_epi32(__m128i __A, __mmask8 __U, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_selectd_128(
      (__mmask8)__U, (__v4si)_mm_dpwuud_epi32(__A, __B, __C), (__v4si)__A);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_dpwuud_epi32(__m128i __A, __mmask8 __U, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_selectd_128(
      (__mmask8)__U, (__v4si)_mm_dpwuud_epi32(__A, __B, __C),
      (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_dpwuud_epi32(__m256i __A, __mmask8 __U, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_selectd_256(
      (__mmask8)__U, (__v8si)_mm256_dpwuud_epi32(__A, __B, __C), (__v8si)__A);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_dpwuud_epi32(__m256i __A, __mmask8 __U, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_selectd_256(
      (__mmask8)__U, (__v8si)_mm256_dpwuud_epi32(__A, __B, __C),
      (__v8si)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_dpwuuds_epi32(__m128i __A, __mmask8 __U, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_selectd_128(
      (__mmask8)__U, (__v4si)_mm_dpwuuds_epi32(__A, __B, __C), (__v4si)__A);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_dpwuuds_epi32(__m128i __A, __mmask8 __U, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_selectd_128(
      (__mmask8)__U, (__v4si)_mm_dpwuuds_epi32(__A, __B, __C),
      (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_dpwuuds_epi32(__m256i __A, __mmask8 __U, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_selectd_256(
      (__mmask8)__U, (__v8si)_mm256_dpwuuds_epi32(__A, __B, __C), (__v8si)__A);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_dpwuuds_epi32(__m256i __A, __mmask8 __U, __m256i __B, __m256i __C) {
  return (__m256i)__builtin_ia32_selectd_256(
      (__mmask8)__U, (__v8si)_mm256_dpwuuds_epi32(__A, __B, __C),
      (__v8si)_mm256_setzero_si256());
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVX512VLVNNIINT16INTRIN_H
