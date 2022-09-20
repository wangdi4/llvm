/*===--------------- avx512reduction2intrin.h - AVX512REDUCTION2 intrinsics -----------------===
 *
 * Copyright (C) 2022 Intel Corporation. All rights reserved.
 *
 * The information and source code contained herein is the exclusive property
 * of Intel Corporation and may not be disclosed, examined or reproduced in
 * whole or in part without explicit written authorization from the company.
 *
 *===----------------------------------------------------------------------===
 */
#ifndef __IMMINTRIN_H
#error "Never use <avx512reduction2intrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVX512REDUCTION2INTRIN_H
#define __AVX512REDUCTION2INTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS512                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512reduction2"), __min_vector_width__(512)))

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraaddbd_epu32(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraaddbd512((__v4su)(__A), (__v64qu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraaddwd_epu32(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraaddwd512((__v4su)(__A), (__v32hu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraaddwd_epu32(__mmask32 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraaddwd512_mask((__v4su)__A, (__v32hu)__B,
                                                    (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraaddsbd_epi32(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraaddsbd512((__v4si)(__A), (__v64qi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraaddswd_epi32(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraaddswd512((__v4si)(__A), (__v32hi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraaddswd_epi32(__mmask32 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraaddswd512_mask((__v4si)__A, (__v32hi)__B,
                                                     (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraaddsbd_epi32(__mmask64 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraaddsbd512_mask((__v4si)__A, (__v64qi)__B,
                                                     (__mmask64)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraaddbd_epu32(__mmask64 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraaddbd512_mask((__v4su)__A, (__v64qu)__B,
                                                    (__mmask64)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraandb_epu8(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraandb512((__v16qu)(__A), (__v64qu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraandd_epu32(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraandd512((__v4su)(__A), (__v16su)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraandd_epu32(__mmask16 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraandd512_mask((__v4su)__A, (__v16su)__B,
                                                   (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraandq_epu64(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraandq512((__v2di)(__A), (__v8di)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraandq_epu64(__mmask8 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraandq512_mask((__v2di)__A, (__v8di)__B,
                                                   (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraandw_epu16(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraandw512((__v8hu)(__A), (__v32hu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraandw_epu16(__mmask32 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraandw512_mask((__v8hu)__A, (__v32hu)__B,
                                                   (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phramaxsb_epi8(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphramaxsb512((__v16qi)(__A), (__v64qi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phramaxsd_epi32(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphramaxsd512((__v4si)(__A), (__v16si)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phramaxsd_epi32(__mmask16 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphramaxsd512_mask((__v4si)__A, (__v16si)__B,
                                                    (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phramaxsq_epi64(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphramaxsq512((__v2di)(__A), (__v8di)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phramaxsq_epi64(__mmask8 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphramaxsq512_mask((__v2di)__A, (__v8di)__B,
                                                    (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phramaxsw_epi16(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphramaxsw512((__v8hi)(__A), (__v32hi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phramaxsw_epi16(__mmask32 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphramaxsw512_mask((__v8hi)__A, (__v32hi)__B,
                                                    (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraminb_epu8(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraminb512((__v16qu)(__A), (__v64qu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phramind_epu32(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphramind512((__v4su)(__A), (__v16su)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phramind_epu32(__mmask16 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphramind512_mask((__v4su)__A, (__v16su)__B,
                                                   (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraminq_epu64(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraminq512((__v2di)(__A), (__v8di)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraminq_epu64(__mmask8 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraminq512_mask((__v2di)__A, (__v8di)__B,
                                                   (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraminw_epu16(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraminw512((__v8hu)(__A), (__v32hu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraminw_epu16(__mmask32 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraminw512_mask((__v8hu)__A, (__v32hu)__B,
                                                   (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraminsb_epi8(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraminsb512((__v16qi)(__A), (__v64qi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraminsd_epi32(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraminsd512((__v4si)(__A), (__v16si)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraminsd_epi32(__mmask16 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraminsd512_mask((__v4si)__A, (__v16si)__B,
                                                    (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraminsq_epi64(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraminsq512((__v2di)(__A), (__v8di)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraminsq_epi64(__mmask8 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraminsq512_mask((__v2di)__A, (__v8di)__B,
                                                    (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraminsw_epi16(__m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraminsw512((__v8hi)(__A), (__v32hi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraminsw_epi16(__mmask32 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraminsw512_mask((__v8hi)__A, (__v32hi)__B,
                                                    (__mmask32)__U);
}

#if __x86_64__
static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraandb_epu8(__mmask64 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraandb512_mask((__v16qu)__A, (__v64qu)__B,
                                                   (__mmask64)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phramaxsb_epi8(__mmask64 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphramaxsb512_mask((__v16qi)__A, (__v64qi)__B,
                                                    (__mmask64)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraminb_epu8(__mmask64 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraminb512_mask((__v16qu)__A, (__v64qu)__B,
                                                   (__mmask64)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraminsb_epi8(__mmask64 __U, __m128i __A, __m512i __B) {
  return (__m128i)__builtin_ia32_vphraminsb512_mask((__v16qi)__A, (__v64qi)__B,
                                                    (__mmask64)__U);
}
#endif // __x86_64__
#undef __DEFAULT_FN_ATTRS512

#endif // __AVX512REDUCTION2INTRIN_H
