/*===--------------- avx512vlreduction2intrin.h - AVX512VLREDUCTION2 intrinsics -----------------===
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
#error "Never use <avx512vlreduction2intrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVX512VLREDUCTION2INTRIN_H
#define __AVX512VLREDUCTION2INTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512vl,avx512reduction2"),                      \
                 __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512vl,avx512reduction2"),                      \
                 __min_vector_width__(256)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraaddbd_epu32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraaddbd128((__v4su)(__A), (__v16qu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraaddbd_epu32(__mmask16 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraaddbd128_mask((__v4su)__A, (__v16qu)__B,
                                                    (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraaddbd_epu32(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraaddbd256((__v4su)(__A), (__v32qu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraaddbd_epu32(__mmask32 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraaddbd256_mask((__v4su)__A, (__v32qu)__B,
                                                    (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraaddwd_epu32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraaddwd128((__v4su)(__A), (__v8hu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraaddwd_epu32(__mmask8 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraaddwd128_mask((__v4su)__A, (__v8hu)__B,
                                                    (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraaddwd_epu32(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraaddwd256((__v4su)(__A), (__v16hu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraaddwd_epu32(__mmask16 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraaddwd256_mask((__v4su)__A, (__v16hu)__B,
                                                    (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraaddsbd_epi32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraaddsbd128((__v4si)(__A), (__v16qi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraaddsbd_epi32(__mmask16 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraaddsbd128_mask((__v4si)__A, (__v16qi)__B,
                                                     (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraaddsbd_epi32(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraaddsbd256((__v4si)(__A), (__v32qi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraaddsbd_epi32(__mmask32 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraaddsbd256_mask((__v4si)__A, (__v32qi)__B,
                                                     (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraaddswd_epi32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraaddswd128((__v4si)(__A), (__v8hi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraaddswd_epi32(__mmask8 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraaddswd128_mask((__v4si)__A, (__v8hi)__B,
                                                     (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraaddswd_epi32(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraaddswd256((__v4si)(__A), (__v16hi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraaddswd_epi32(__mmask16 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraaddswd256_mask((__v4si)__A, (__v16hi)__B,
                                                     (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phraandb_epu8(__m128i __A,
                                                                  __m128i __B) {
  return (__m128i)__builtin_ia32_vphraandb128((__v16qu)(__A), (__v16qu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraandb_epu8(__mmask16 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraandb128_mask((__v16qu)__A, (__v16qu)__B,
                                                   (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraandb_epu8(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraandb256((__v16qu)(__A), (__v32qu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraandb_epu8(__mmask32 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraandb256_mask((__v16qu)__A, (__v32qu)__B,
                                                   (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraandd_epu32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraandd128((__v4su)(__A), (__v4su)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraandd_epu32(__mmask8 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraandd128_mask((__v4su)__A, (__v4su)__B,
                                                   (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraandd_epu32(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraandd256((__v4su)(__A), (__v8su)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraandd_epu32(__mmask8 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraandd256_mask((__v4su)__A, (__v8su)__B,
                                                   (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraandq_epu64(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraandq128((__v2di)(__A), (__v2di)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraandq_epu64(__mmask8 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraandq128_mask((__v2di)__A, (__v2di)__B,
                                                   (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraandq_epu64(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraandq256((__v2di)(__A), (__v4di)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraandq_epu64(__mmask8 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraandq256_mask((__v2di)__A, (__v4di)__B,
                                                   (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraandw_epu16(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraandw128((__v8hu)(__A), (__v8hu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraandw_epu16(__mmask8 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraandw128_mask((__v8hu)__A, (__v8hu)__B,
                                                   (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraandw_epu16(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraandw256((__v8hu)(__A), (__v16hu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraandw_epu16(__mmask16 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraandw256_mask((__v8hu)__A, (__v16hu)__B,
                                                   (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phramaxsb_epi8(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphramaxsb128((__v16qi)(__A), (__v16qi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phramaxsb_epi8(__mmask16 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphramaxsb128_mask((__v16qi)__A, (__v16qi)__B,
                                                    (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phramaxsb_epi8(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphramaxsb256((__v16qi)(__A), (__v32qi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phramaxsb_epi8(__mmask32 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphramaxsb256_mask((__v16qi)__A, (__v32qi)__B,
                                                    (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phramaxsd_epi32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphramaxsd128((__v4si)(__A), (__v4si)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phramaxsd_epi32(__mmask8 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphramaxsd128_mask((__v4si)__A, (__v4si)__B,
                                                    (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phramaxsd_epi32(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphramaxsd256((__v4si)(__A), (__v8si)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phramaxsd_epi32(__mmask8 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphramaxsd256_mask((__v4si)__A, (__v8si)__B,
                                                    (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phramaxsq_epi64(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphramaxsq128((__v2di)(__A), (__v2di)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phramaxsq_epi64(__mmask8 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphramaxsq128_mask((__v2di)__A, (__v2di)__B,
                                                    (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phramaxsq_epi64(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphramaxsq256((__v2di)(__A), (__v4di)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phramaxsq_epi64(__mmask8 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphramaxsq256_mask((__v2di)__A, (__v4di)__B,
                                                    (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phramaxsw_epi16(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphramaxsw128((__v8hi)(__A), (__v8hi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phramaxsw_epi16(__mmask8 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphramaxsw128_mask((__v8hi)__A, (__v8hi)__B,
                                                    (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phramaxsw_epi16(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphramaxsw256((__v8hi)(__A), (__v16hi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phramaxsw_epi16(__mmask16 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphramaxsw256_mask((__v8hi)__A, (__v16hi)__B,
                                                    (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128 _mm_phraminb_epu8(__m128i __A,
                                                                  __m128i __B) {
  return (__m128i)__builtin_ia32_vphraminb128((__v16qu)(__A), (__v16qu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraminb_epu8(__mmask16 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraminb128_mask((__v16qu)__A, (__v16qu)__B,
                                                   (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraminb_epu8(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraminb256((__v16qu)(__A), (__v32qu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraminb_epu8(__mmask32 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraminb256_mask((__v16qu)__A, (__v32qu)__B,
                                                   (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phramind_epu32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphramind128((__v4su)(__A), (__v4su)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phramind_epu32(__mmask8 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphramind128_mask((__v4su)__A, (__v4su)__B,
                                                   (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phramind_epu32(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphramind256((__v4su)(__A), (__v8su)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phramind_epu32(__mmask8 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphramind256_mask((__v4su)__A, (__v8su)__B,
                                                   (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraminq_epu64(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraminq128((__v2di)(__A), (__v2di)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraminq_epu64(__mmask8 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraminq128_mask((__v2di)__A, (__v2di)__B,
                                                   (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraminq_epu64(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraminq256((__v2di)(__A), (__v4di)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraminq_epu64(__mmask8 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraminq256_mask((__v2di)__A, (__v4di)__B,
                                                   (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraminw_epu16(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraminw128((__v8hu)(__A), (__v8hu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraminw_epu16(__mmask8 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraminw128_mask((__v8hu)__A, (__v8hu)__B,
                                                   (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraminw_epu16(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraminw256((__v8hu)(__A), (__v16hu)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraminw_epu16(__mmask16 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraminw256_mask((__v8hu)__A, (__v16hu)__B,
                                                   (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraminsb_epi8(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraminsb128((__v16qi)(__A), (__v16qi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraminsb_epi8(__mmask16 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraminsb128_mask((__v16qi)__A, (__v16qi)__B,
                                                    (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraminsb_epi8(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraminsb256((__v16qi)(__A), (__v32qi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraminsb_epi8(__mmask32 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraminsb256_mask((__v16qi)__A, (__v32qi)__B,
                                                    (__mmask32)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraminsd_epi32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraminsd128((__v4si)(__A), (__v4si)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraminsd_epi32(__mmask8 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraminsd128_mask((__v4si)__A, (__v4si)__B,
                                                    (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraminsd_epi32(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraminsd256((__v4si)(__A), (__v8si)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraminsd_epi32(__mmask8 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraminsd256_mask((__v4si)__A, (__v8si)__B,
                                                    (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraminsq_epi64(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraminsq128((__v2di)(__A), (__v2di)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraminsq_epi64(__mmask8 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraminsq128_mask((__v2di)__A, (__v2di)__B,
                                                    (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraminsq_epi64(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraminsq256((__v2di)(__A), (__v4di)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraminsq_epi64(__mmask8 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraminsq256_mask((__v2di)__A, (__v4di)__B,
                                                    (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_phraminsw_epi16(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraminsw128((__v8hi)(__A), (__v8hi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_phraminsw_epi16(__mmask8 __U, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vphraminsw128_mask((__v8hi)__A, (__v8hi)__B,
                                                    (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_phraminsw_epi16(__m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraminsw256((__v8hi)(__A), (__v16hi)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_phraminsw_epi16(__mmask16 __U, __m128i __A, __m256i __B) {
  return (__m128i)__builtin_ia32_vphraminsw256_mask((__v8hi)__A, (__v16hi)__B,
                                                    (__mmask16)__U);
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVX512VLREDUCTION2INTRIN_H
