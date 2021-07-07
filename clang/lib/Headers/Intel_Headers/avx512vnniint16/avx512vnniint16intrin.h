/*===-- avx512vnniint16intrin.h - AVX512VNNIINT16 intrinsics---------------===
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
    "Never use <avx512vnniint16intrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVX512VNNIINT16INTRIN_H
#define __AVX512VNNIINT16INTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS512                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512vnniint16"), __min_vector_width__(512)))

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_dpwsud_epi32(__m512i __A, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_vpdpwsud512((__v16si)__A, (__v16si)__B,
                                             (__v16si)__C);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_dpwsud_epi32(__m512i __A, __mmask16 __U, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_selectd_512(
      (__mmask16)__U, (__v16si)_mm512_dpwsud_epi32(__A, __B, __C),
      (__v16si)__A);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_dpwsud_epi32(__m512i __A, __mmask16 __U, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_selectd_512(
      (__mmask16)__U, (__v16si)_mm512_dpwsud_epi32(__A, __B, __C),
      (__v16si)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_dpwsuds_epi32(__m512i __A, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_vpdpwsuds512((__v16si)__A, (__v16si)__B,
                                              (__v16si)__C);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_dpwsuds_epi32(__m512i __A, __mmask16 __U, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_selectd_512(
      (__mmask16)__U, (__v16si)_mm512_dpwsuds_epi32(__A, __B, __C),
      (__v16si)__A);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_dpwsuds_epi32(__m512i __A, __mmask16 __U, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_selectd_512(
      (__mmask16)__U, (__v16si)_mm512_dpwsuds_epi32(__A, __B, __C),
      (__v16si)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_dpwusd_epi32(__m512i __A, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_vpdpwusd512((__v16si)__A, (__v16si)__B,
                                             (__v16si)__C);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_dpwusd_epi32(__m512i __A, __mmask16 __U, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_selectd_512(
      (__mmask16)__U, (__v16si)_mm512_dpwusd_epi32(__A, __B, __C),
      (__v16si)__A);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_dpwusd_epi32(__m512i __A, __mmask16 __U, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_selectd_512(
      (__mmask16)__U, (__v16si)_mm512_dpwusd_epi32(__A, __B, __C),
      (__v16si)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_dpwusds_epi32(__m512i __A, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_vpdpwusds512((__v16si)__A, (__v16si)__B,
                                              (__v16si)__C);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_dpwusds_epi32(__m512i __A, __mmask16 __U, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_selectd_512(
      (__mmask16)__U, (__v16si)_mm512_dpwusds_epi32(__A, __B, __C),
      (__v16si)__A);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_dpwusds_epi32(__m512i __A, __mmask16 __U, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_selectd_512(
      (__mmask16)__U, (__v16si)_mm512_dpwusds_epi32(__A, __B, __C),
      (__v16si)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_dpwuud_epi32(__m512i __A, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_vpdpwuud512((__v16si)__A, (__v16si)__B,
                                             (__v16si)__C);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_dpwuud_epi32(__m512i __A, __mmask16 __U, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_selectd_512(
      (__mmask16)__U, (__v16si)_mm512_dpwuud_epi32(__A, __B, __C),
      (__v16si)__A);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_dpwuud_epi32(__m512i __A, __mmask16 __U, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_selectd_512(
      (__mmask16)__U, (__v16si)_mm512_dpwuud_epi32(__A, __B, __C),
      (__v16si)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_dpwuuds_epi32(__m512i __A, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_vpdpwuuds512((__v16si)__A, (__v16si)__B,
                                              (__v16si)__C);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_dpwuuds_epi32(__m512i __A, __mmask16 __U, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_selectd_512(
      (__mmask16)__U, (__v16si)_mm512_dpwuuds_epi32(__A, __B, __C),
      (__v16si)__A);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_dpwuuds_epi32(__m512i __A, __mmask16 __U, __m512i __B, __m512i __C) {
  return (__m512i)__builtin_ia32_selectd_512(
      (__mmask16)__U, (__v16si)_mm512_dpwuuds_epi32(__A, __B, __C),
      (__v16si)_mm512_setzero_si512());
}

#undef __DEFAULT_FN_ATTRS512

#endif // __AVX512VNNIINT16INTRIN_H
