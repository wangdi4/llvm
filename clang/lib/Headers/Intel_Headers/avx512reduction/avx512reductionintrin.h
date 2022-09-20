/*===--------------- avx512reductionintrin.h - AVX512REDUCTION intrinsics -----------------===
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
#error "Never use <avx512reductionintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVX512REDUCTIONINTRIN_H
#define __AVX512REDUCTIONINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS512                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512reduction"), __min_vector_width__(512)))

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraddbd_epu32(__m512i __A) {
  return (__m128i)__builtin_ia32_vphraddbd512((__v64qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraddd_epu32(__m512i __A) {
  return (__m128i)__builtin_ia32_vphraddd512((__v16su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraddd_epu32(__mmask16 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphraddd512_mask((__v16su)__A, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraddq_epu64(__m512i __A) {
  return (__m128i)__builtin_ia32_vphraddq512((__v8di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraddq_epu64(__mmask8 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphraddq512_mask((__v8di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraddwd_epu32(__m512i __A) {
  return (__m128i)__builtin_ia32_vphraddwd512((__v32hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraddwd_epu32(__mmask32 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphraddwd512_mask((__v32hu)__A,
                                                   (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraddsbd_epi32(__m512i __A) {
  return (__m128i)__builtin_ia32_vphraddsbd512((__v64qi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraddsd_epi32(__m512i __A) {
  return (__m128i)__builtin_ia32_vphraddsd512((__v16si)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraddsd_epi32(__mmask16 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphraddsd512_mask((__v16si)__A,
                                                   (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraddsq_epi64(__m512i __A) {
  return (__m128i)__builtin_ia32_vphraddsq512((__v8di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraddsq_epi64(__mmask8 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphraddsq512_mask((__v8di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phraddswd_epi32(__m512i __A) {
  return (__m128i)__builtin_ia32_vphraddswd512((__v32hi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraddswd_epi32(__mmask32 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphraddswd512_mask((__v32hi)__A,
                                                    (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrandb_epu8(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrandb512((__v64qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrandd_epu32(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrandd512((__v16su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrandd_epu32(__mmask16 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrandd512_mask((__v16su)__A, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phranddq_epu64(__m512i __A) {
  return (__m128i)__builtin_ia32_vphranddq512((__v8di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrandq_epu64(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrandq512((__v8di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrandq_epu64(__mmask8 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrandq512_mask((__v8di)__A, (__mmask8)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_phrandqq_epu64(__m512i __A) {
  return (__m256i)__builtin_ia32_vphrandqq512((__v8di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrandw_epu16(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrandw512((__v32hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrandw_epu16(__mmask32 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrandw512_mask((__v32hu)__A, (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrmaxb_epu8(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrmaxb512((__v64qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrmaxd_epu32(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrmaxd512((__v16su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrmaxd_epu32(__mmask16 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrmaxd512_mask((__v16su)__A, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrmaxq_epu64(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrmaxq512((__v8di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrmaxq_epu64(__mmask8 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrmaxq512_mask((__v8di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrmaxw_epu16(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrmaxw512((__v32hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrmaxw_epu16(__mmask32 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrmaxw512_mask((__v32hu)__A, (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrmaxsb_epi8(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsb512((__v64qi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrmaxsd_epi32(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsd512((__v16si)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrmaxsd_epi32(__mmask16 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsd512_mask((__v16si)__A,
                                                   (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrmaxsq_epi64(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsq512((__v8di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrmaxsq_epi64(__mmask8 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsq512_mask((__v8di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrmaxsw_epi16(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsw512((__v32hi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrmaxsw_epi16(__mmask32 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsw512_mask((__v32hi)__A,
                                                   (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrminb_epu8(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrminb512((__v64qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrmind_epu32(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrmind512((__v16su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrmind_epu32(__mmask16 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrmind512_mask((__v16su)__A, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrminq_epu64(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrminq512((__v8di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrminq_epu64(__mmask8 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrminq512_mask((__v8di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrminw_epu16(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrminw512((__v32hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrminw_epu16(__mmask32 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrminw512_mask((__v32hu)__A, (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrminsb_epi8(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrminsb512((__v64qi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrminsd_epi32(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrminsd512((__v16si)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrminsd_epi32(__mmask16 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrminsd512_mask((__v16si)__A,
                                                   (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrminsq_epi64(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrminsq512((__v8di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrminsq_epi64(__mmask8 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrminsq512_mask((__v8di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrminsw_epi16(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrminsw512((__v32hi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrminsw_epi16(__mmask32 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrminsw512_mask((__v32hi)__A,
                                                   (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrorb_epu8(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrorb512((__v64qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrord_epu32(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrord512((__v16su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrord_epu32(__mmask16 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrord512_mask((__v16su)__A, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrordq_epu64(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrordq512((__v8di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrorq_epu64(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrorq512((__v8di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrorq_epu64(__mmask8 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrorq512_mask((__v8di)__A, (__mmask8)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_phrorqq_epu64(__m512i __A) {
  return (__m256i)__builtin_ia32_vphrorqq512((__v8di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrorw_epu16(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrorw512((__v32hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrorw_epu16(__mmask32 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrorw512_mask((__v32hu)__A, (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrxorb_epu8(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrxorb512((__v64qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrxord_epu32(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrxord512((__v16su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrxord_epu32(__mmask16 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrxord512_mask((__v16su)__A, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrxordq_epu64(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrxordq512((__v8di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrxorq_epu64(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrxorq512((__v8di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrxorq_epu64(__mmask8 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrxorq512_mask((__v8di)__A, (__mmask8)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_phrxorqq_epu64(__m512i __A) {
  return (__m256i)__builtin_ia32_vphrxorqq512((__v8di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_phrxorw_epu16(__m512i __A) {
  return (__m128i)__builtin_ia32_vphrxorw512((__v32hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrxorw_epu16(__mmask32 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrxorw512_mask((__v32hu)__A, (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraddbd_epu32(__mmask64 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphraddbd512_mask((__v64qu)__A,
                                                   (__mmask64)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phraddsbd_epi32(__mmask64 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphraddsbd512_mask((__v64qi)__A,
                                                    (__mmask64)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrandb_epu8(__mmask64 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrandb512_mask((__v64qu)__A, (__mmask64)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrmaxb_epu8(__mmask64 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrmaxb512_mask((__v64qu)__A, (__mmask64)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrmaxsb_epi8(__mmask64 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsb512_mask((__v64qi)__A,
                                                   (__mmask64)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrminb_epu8(__mmask64 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrminb512_mask((__v64qu)__A, (__mmask64)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrminsb_epi8(__mmask64 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrminsb512_mask((__v64qi)__A,
                                                   (__mmask64)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrorb_epu8(__mmask64 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrorb512_mask((__v64qu)__A, (__mmask64)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS512
_mm512_mask_phrxorb_epu8(__mmask64 __U, __m512i __A) {
  return (__m128i)__builtin_ia32_vphrxorb512_mask((__v64qu)__A, (__mmask64)__U);
}

#undef __DEFAULT_FN_ATTRS512

#endif // __AVX512REDUCTIONINTRIN_H
