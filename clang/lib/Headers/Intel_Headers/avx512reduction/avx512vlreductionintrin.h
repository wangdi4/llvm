/*===--------------- avx512vlreductionintrin.h - AVX512VLREDUCTION intrinsics -----------------===
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
#error "Never use <avx512vlreductionintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVX512VLREDUCTIONINTRIN_H
#define __AVX512VLREDUCTIONINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512vl,avx512reduction"),                       \
                 __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512vl,avx512reduction"),                       \
                 __min_vector_width__(256)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraddbd_epu32(__m128i __A) {
  return (__m128i)__builtin_ia32_vphraddbd128((__v16qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraddbd_epu32(__mmask16 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphraddbd128_mask((__v16qu)__A,
                                                   (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraddbd_epu32(__m256i __A) {
  return (__m128i)__builtin_ia32_vphraddbd256((__v32qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraddbd_epu32(__mmask32 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphraddbd256_mask((__v32qu)__A,
                                                   (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phraddd_epu32(__m128i __A) {
  return (__m128i)__builtin_ia32_vphraddd128((__v4su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraddd_epu32(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphraddd128_mask((__v4su)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraddd_epu32(__m256i __A) {
  return (__m128i)__builtin_ia32_vphraddd256((__v8su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraddd_epu32(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphraddd256_mask((__v8su)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phraddq_epu64(__m128i __A) {
  return (__m128i)__builtin_ia32_vphraddq128((__v2di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraddq_epu64(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphraddq128_mask((__v2di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraddq_epu64(__m256i __A) {
  return (__m128i)__builtin_ia32_vphraddq256((__v4di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraddq_epu64(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphraddq256_mask((__v4di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraddwd_epu32(__m128i __A) {
  return (__m128i)__builtin_ia32_vphraddwd128((__v8hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraddwd_epu32(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphraddwd128_mask((__v8hu)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraddwd_epu32(__m256i __A) {
  return (__m128i)__builtin_ia32_vphraddwd256((__v16hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraddwd_epu32(__mmask16 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphraddwd256_mask((__v16hu)__A,
                                                   (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraddsbd_epi32(__m128i __A) {
  return (__m128i)__builtin_ia32_vphraddsbd128((__v16qi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraddsbd_epi32(__mmask16 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphraddsbd128_mask((__v16qi)__A,
                                                    (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraddsbd_epi32(__m256i __A) {
  return (__m128i)__builtin_ia32_vphraddsbd256((__v32qi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraddsbd_epi32(__mmask32 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphraddsbd256_mask((__v32qi)__A,
                                                    (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraddsd_epi32(__m128i __A) {
  return (__m128i)__builtin_ia32_vphraddsd128((__v4si)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraddsd_epi32(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphraddsd128_mask((__v4si)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraddsd_epi32(__m256i __A) {
  return (__m128i)__builtin_ia32_vphraddsd256((__v8si)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraddsd_epi32(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphraddsd256_mask((__v8si)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraddsq_epi64(__m128i __A) {
  return (__m128i)__builtin_ia32_vphraddsq128((__v2di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraddsq_epi64(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphraddsq128_mask((__v2di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraddsq_epi64(__m256i __A) {
  return (__m128i)__builtin_ia32_vphraddsq256((__v4di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraddsq_epi64(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphraddsq256_mask((__v4di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraddswd_epi32(__m128i __A) {
  return (__m128i)__builtin_ia32_vphraddswd128((__v8hi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraddswd_epi32(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphraddswd128_mask((__v8hi)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraddswd_epi32(__m256i __A) {
  return (__m128i)__builtin_ia32_vphraddswd256((__v16hi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraddswd_epi32(__mmask16 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphraddswd256_mask((__v16hi)__A,
                                                    (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrandb_epu8(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrandb128((__v16qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrandb_epu8(__mmask16 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrandb128_mask((__v16qu)__A, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrandb_epu8(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrandb256((__v32qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrandb_epu8(__mmask32 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrandb256_mask((__v32qu)__A, (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrandd_epu32(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrandd128((__v4su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrandd_epu32(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrandd128_mask((__v4su)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrandd_epu32(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrandd256((__v8su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrandd_epu32(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrandd256_mask((__v8su)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phranddq_epu64(__m256i __A) {
  return (__m128i)__builtin_ia32_vphranddq256((__v4di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrandq_epu64(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrandq128((__v2di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrandq_epu64(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrandq128_mask((__v2di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrandq_epu64(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrandq256((__v4di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrandq_epu64(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrandq256_mask((__v4di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrandw_epu16(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrandw128((__v8hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrandw_epu16(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrandw128_mask((__v8hu)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrandw_epu16(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrandw256((__v16hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrandw_epu16(__mmask16 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrandw256_mask((__v16hu)__A, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrmaxb_epu8(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrmaxb128((__v16qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrmaxb_epu8(__mmask16 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrmaxb128_mask((__v16qu)__A, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrmaxb_epu8(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrmaxb256((__v32qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrmaxb_epu8(__mmask32 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrmaxb256_mask((__v32qu)__A, (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrmaxd_epu32(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrmaxd128((__v4su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrmaxd_epu32(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrmaxd128_mask((__v4su)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrmaxd_epu32(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrmaxd256((__v8su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrmaxd_epu32(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrmaxd256_mask((__v8su)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrmaxq_epu64(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrmaxq128((__v2di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrmaxq_epu64(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrmaxq128_mask((__v2di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrmaxq_epu64(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrmaxq256((__v4di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrmaxq_epu64(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrmaxq256_mask((__v4di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrmaxw_epu16(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrmaxw128((__v8hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrmaxw_epu16(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrmaxw128_mask((__v8hu)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrmaxw_epu16(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrmaxw256((__v16hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrmaxw_epu16(__mmask16 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrmaxw256_mask((__v16hu)__A, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrmaxsb_epi8(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsb128((__v16qi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrmaxsb_epi8(__mmask16 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsb128_mask((__v16qi)__A,
                                                   (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrmaxsb_epi8(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsb256((__v32qi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrmaxsb_epi8(__mmask32 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsb256_mask((__v32qi)__A,
                                                   (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phrmaxsd_epi32(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsd128((__v4si)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrmaxsd_epi32(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsd128_mask((__v4si)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrmaxsd_epi32(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsd256((__v8si)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrmaxsd_epi32(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsd256_mask((__v8si)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phrmaxsq_epi64(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsq128((__v2di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrmaxsq_epi64(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsq128_mask((__v2di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrmaxsq_epi64(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsq256((__v4di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrmaxsq_epi64(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsq256_mask((__v4di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phrmaxsw_epi16(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsw128((__v8hi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrmaxsw_epi16(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsw128_mask((__v8hi)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrmaxsw_epi16(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsw256((__v16hi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrmaxsw_epi16(__mmask16 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrmaxsw256_mask((__v16hi)__A,
                                                   (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrminb_epu8(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrminb128((__v16qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrminb_epu8(__mmask16 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrminb128_mask((__v16qu)__A, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrminb_epu8(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrminb256((__v32qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrminb_epu8(__mmask32 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrminb256_mask((__v32qu)__A, (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrmind_epu32(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrmind128((__v4su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrmind_epu32(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrmind128_mask((__v4su)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrmind_epu32(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrmind256((__v8su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrmind_epu32(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrmind256_mask((__v8su)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrminq_epu64(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrminq128((__v2di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrminq_epu64(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrminq128_mask((__v2di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrminq_epu64(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrminq256((__v4di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrminq_epu64(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrminq256_mask((__v4di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrminw_epu16(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrminw128((__v8hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrminw_epu16(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrminw128_mask((__v8hu)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrminw_epu16(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrminw256((__v16hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrminw_epu16(__mmask16 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrminw256_mask((__v16hu)__A, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrminsb_epi8(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrminsb128((__v16qi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrminsb_epi8(__mmask16 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrminsb128_mask((__v16qi)__A,
                                                   (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrminsb_epi8(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrminsb256((__v32qi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrminsb_epi8(__mmask32 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrminsb256_mask((__v32qi)__A,
                                                   (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phrminsd_epi32(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrminsd128((__v4si)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrminsd_epi32(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrminsd128_mask((__v4si)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrminsd_epi32(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrminsd256((__v8si)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrminsd_epi32(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrminsd256_mask((__v8si)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phrminsq_epi64(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrminsq128((__v2di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrminsq_epi64(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrminsq128_mask((__v2di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrminsq_epi64(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrminsq256((__v4di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrminsq_epi64(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrminsq256_mask((__v4di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phrminsw_epi16(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrminsw128((__v8hi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrminsw_epi16(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrminsw128_mask((__v8hi)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrminsw_epi16(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrminsw256((__v16hi)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrminsw_epi16(__mmask16 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrminsw256_mask((__v16hi)__A,
                                                   (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrorb_epu8(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrorb128((__v16qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrorb_epu8(__mmask16 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrorb128_mask((__v16qu)__A, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrorb_epu8(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrorb256((__v32qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrorb_epu8(__mmask32 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrorb256_mask((__v32qu)__A, (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrord_epu32(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrord128((__v4su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrord_epu32(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrord128_mask((__v4su)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrord_epu32(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrord256((__v8su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrord_epu32(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrord256_mask((__v8su)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrordq_epu64(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrordq256((__v4di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrorq_epu64(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrorq128((__v2di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrorq_epu64(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrorq128_mask((__v2di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrorq_epu64(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrorq256((__v4di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrorq_epu64(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrorq256_mask((__v4di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrorw_epu16(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrorw128((__v8hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrorw_epu16(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrorw128_mask((__v8hu)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrorw_epu16(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrorw256((__v16hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrorw_epu16(__mmask16 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrorw256_mask((__v16hu)__A, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrxorb_epu8(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrxorb128((__v16qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrxorb_epu8(__mmask16 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrxorb128_mask((__v16qu)__A, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrxorb_epu8(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrxorb256((__v32qu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrxorb_epu8(__mmask32 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrxorb256_mask((__v32qu)__A, (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrxord_epu32(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrxord128((__v4su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrxord_epu32(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrxord128_mask((__v4su)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrxord_epu32(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrxord256((__v8su)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrxord_epu32(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrxord256_mask((__v8su)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrxordq_epu64(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrxordq256((__v4di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrxorq_epu64(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrxorq128((__v2di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrxorq_epu64(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrxorq128_mask((__v2di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrxorq_epu64(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrxorq256((__v4di)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrxorq_epu64(__mmask8 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrxorq256_mask((__v4di)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phrxorw_epu16(__m128i __A) {
  return (__m128i)__builtin_ia32_vphrxorw128((__v8hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phrxorw_epu16(__mmask8 __U, __m128i __A) {
  return (__m128i)__builtin_ia32_vphrxorw128_mask((__v8hu)__A, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phrxorw_epu16(__m256i __A) {
  return (__m128i)__builtin_ia32_vphrxorw256((__v16hu)(__A));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phrxorw_epu16(__mmask16 __U, __m256i __A) {
  return (__m128i)__builtin_ia32_vphrxorw256_mask((__v16hu)__A, (__mmask16)__U);
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVX512VLREDUCTIONINTRIN_H
