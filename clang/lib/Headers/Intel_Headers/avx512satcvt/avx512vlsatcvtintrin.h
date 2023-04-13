/*===---------- avx512vlsatcvtintrin.h - AVX512VLSATCVT intrinsics ---------===
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
#error                                                                         \
    "Never use <avx512vlsatcvtintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVX512VLSATCVTINTRIN_H
#define __AVX512VLSATCVTINTRIN_H

#define _mm_cvtnebf162ibs_epi8(A)                                              \
  ((__m128i)__builtin_ia32_vcvtnebf162ibs128((__v8hu)(A)))

#define _mm_mask_cvtnebf162ibs_epi8(S, U, A)                                   \
  ((__m128i)__builtin_ia32_vcvtnebf162ibs128_mask((__v8hu)(S), (__mmask8)(U),  \
                                                  (__v8hu)(A)))

#define _mm_maskz_cvtnebf162ibs_epi8(U, A)                                     \
  ((__m128i)__builtin_ia32_vcvtnebf162ibs128_maskz((__mmask8)(U), (__v8hu)(A)))

#define _mm256_cvtnebf162ibs_epi8(A)                                           \
  ((__m256i)__builtin_ia32_vcvtnebf162ibs256((__v16hu)(A)))

#define _mm256_mask_cvtnebf162ibs_epi8(S, U, A)                                \
  ((__m256i)__builtin_ia32_vcvtnebf162ibs256_mask(                             \
      (__v16hu)(S), (__mmask16)(U), (__v16hu)(A)))

#define _mm256_maskz_cvtnebf162ibs_epi8(U, A)                                  \
  ((__m256i)__builtin_ia32_vcvtnebf162ibs256_maskz((__mmask16)(U),             \
                                                   (__v16hu)(A)))

#define _mm_cvtnebf162iubs_epi8(A)                                             \
  ((__m128i)__builtin_ia32_vcvtnebf162iubs128((__v8hu)(A)))

#define _mm_mask_cvtnebf162iubs_epi8(S, U, A)                                  \
  ((__m128i)__builtin_ia32_vcvtnebf162iubs128_mask((__v8hu)(S), (__mmask8)(U), \
                                                   (__v8hu)(A)))

#define _mm_maskz_cvtnebf162iubs_epi8(U, A)                                    \
  ((__m128i)__builtin_ia32_vcvtnebf162iubs128_maskz((__mmask8)(U), (__v8hu)(A)))

#define _mm256_cvtnebf162iubs_epi8(A)                                          \
  ((__m256i)__builtin_ia32_vcvtnebf162iubs256((__v16hu)(A)))

#define _mm256_mask_cvtnebf162iubs_epi8(S, U, A)                               \
  ((__m256i)__builtin_ia32_vcvtnebf162iubs256_mask(                            \
      (__v16hu)(S), (__mmask16)(U), (__v16hu)(A)))

#define _mm256_maskz_cvtnebf162iubs_epi8(U, A)                                 \
  ((__m256i)__builtin_ia32_vcvtnebf162iubs256_maskz((__mmask16)(U),            \
                                                    (__v16hu)(A)))

#define _mm_cvtph2ibs_epi8(A)                                                  \
  ((__m128i)__builtin_ia32_vcvtph2ibs128((__v8hf)(A)))

#define _mm_mask_cvtph2ibs_epi8(S, U, A)                                       \
  ((__m128i)__builtin_ia32_vcvtph2ibs128_mask((__v8hu)(S), (__mmask8)(U),      \
                                              (__v8hf)(A)))

#define _mm_maskz_cvtph2ibs_epi8(U, A)                                         \
  ((__m128i)__builtin_ia32_vcvtph2ibs128_maskz((__mmask8)(U), (__v8hf)(A)))

#define _mm256_cvtph2ibs_epi8(A)                                               \
  ((__m256i)__builtin_ia32_vcvtph2ibs256((__v16hf)(A)))

#define _mm256_mask_cvtph2ibs_epi8(S, U, A)                                    \
  ((__m256i)__builtin_ia32_vcvtph2ibs256_mask((__v16hu)(S), (__mmask16)(U),    \
                                              (__v16hf)(A)))

#define _mm256_maskz_cvtph2ibs_epi8(U, A)                                      \
  ((__m256i)__builtin_ia32_vcvtph2ibs256_maskz((__mmask16)(U), (__v16hf)(A)))

#define _mm_cvtph2iubs_epi8(A)                                                 \
  ((__m128i)__builtin_ia32_vcvtph2iubs128((__v8hf)(A)))

#define _mm_mask_cvtph2iubs_epi8(S, U, A)                                      \
  ((__m128i)__builtin_ia32_vcvtph2iubs128_mask((__v8hu)(S), (__mmask8)(U),     \
                                               (__v8hf)(A)))

#define _mm_maskz_cvtph2iubs_epi8(U, A)                                        \
  ((__m128i)__builtin_ia32_vcvtph2iubs128_maskz((__mmask8)(U), (__v8hf)(A)))

#define _mm256_cvtph2iubs_epi8(A)                                              \
  ((__m256i)__builtin_ia32_vcvtph2iubs256((__v16hf)(A)))

#define _mm256_mask_cvtph2iubs_epi8(S, U, A)                                   \
  ((__m256i)__builtin_ia32_vcvtph2iubs256_mask((__v16hu)(S), (__mmask16)(U),   \
                                               (__v16hf)(A)))

#define _mm256_maskz_cvtph2iubs_epi8(U, A)                                     \
  ((__m256i)__builtin_ia32_vcvtph2iubs256_maskz((__mmask16)(U), (__v16hf)(A)))

#define _mm_cvtps2ibs_epi8(A)                                                  \
  ((__m128i)__builtin_ia32_vcvtps2ibs128((__v4sf)(A)))

#define _mm_mask_cvtps2ibs_epi8(S, U, A)                                       \
  ((__m128i)__builtin_ia32_vcvtps2ibs128_mask((__v4su)(S), (__mmask8)(U),      \
                                              (__v4sf)(A)))

#define _mm_maskz_cvtps2ibs_epi8(U, A)                                         \
  ((__m128i)__builtin_ia32_vcvtps2ibs128_maskz((__mmask8)(U), (__v4sf)(A)))

#define _mm256_cvtps2ibs_epi8(A)                                               \
  ((__m256i)__builtin_ia32_vcvtps2ibs256((__v8sf)(A)))

#define _mm256_mask_cvtps2ibs_epi8(S, U, A)                                    \
  ((__m256i)__builtin_ia32_vcvtps2ibs256_mask((__v8su)(S), (__mmask8)(U),      \
                                              (__v8sf)(A)))

#define _mm256_maskz_cvtps2ibs_epi8(U, A)                                      \
  ((__m256i)__builtin_ia32_vcvtps2ibs256_maskz((__mmask8)(U), (__v8sf)(A)))

#define _mm_cvtps2iubs_epi8(A)                                                 \
  ((__m128i)__builtin_ia32_vcvtps2iubs128((__v4sf)(A)))

#define _mm_mask_cvtps2iubs_epi8(S, U, A)                                      \
  ((__m128i)__builtin_ia32_vcvtps2iubs128_mask((__v4su)(S), (__mmask8)(U),     \
                                               (__v4sf)(A)))

#define _mm_maskz_cvtps2iubs_epi8(U, A)                                        \
  ((__m128i)__builtin_ia32_vcvtps2iubs128_maskz((__mmask8)(U), (__v4sf)(A)))

#define _mm256_cvtps2iubs_epi8(A)                                              \
  ((__m256i)__builtin_ia32_vcvtps2iubs256((__v8sf)(A)))

#define _mm256_mask_cvtps2iubs_epi8(S, U, A)                                   \
  ((__m256i)__builtin_ia32_vcvtps2iubs256_mask((__v8su)(S), (__mmask8)(U),     \
                                               (__v8sf)(A)))

#define _mm256_maskz_cvtps2iubs_epi8(U, A)                                     \
  ((__m256i)__builtin_ia32_vcvtps2iubs256_maskz((__mmask8)(U), (__v8sf)(A)))

#define _mm_cvttnebf162ibs_epi8(A)                                             \
  ((__m128i)__builtin_ia32_vcvttnebf162ibs128((__v8hu)(A)))

#define _mm_mask_cvttnebf162ibs_epi8(S, U, A)                                  \
  ((__m128i)__builtin_ia32_vcvttnebf162ibs128_mask((__v8hu)(S), (__mmask8)(U), \
                                                   (__v8hu)(A)))

#define _mm_maskz_cvttnebf162ibs_epi8(U, A)                                    \
  ((__m128i)__builtin_ia32_vcvttnebf162ibs128_maskz((__mmask8)(U), (__v8hu)(A)))

#define _mm256_cvttnebf162ibs_epi8(A)                                          \
  ((__m256i)__builtin_ia32_vcvttnebf162ibs256((__v16hu)(A)))

#define _mm256_mask_cvttnebf162ibs_epi8(S, U, A)                               \
  ((__m256i)__builtin_ia32_vcvttnebf162ibs256_mask(                            \
      (__v16hu)(S), (__mmask16)(U), (__v16hu)(A)))

#define _mm256_maskz_cvttnebf162ibs_epi8(U, A)                                 \
  ((__m256i)__builtin_ia32_vcvttnebf162ibs256_maskz((__mmask16)(U),            \
                                                    (__v16hu)(A)))

#define _mm_cvttnebf162iubs_epi8(A)                                            \
  ((__m128i)__builtin_ia32_vcvttnebf162iubs128((__v8hu)(A)))

#define _mm_mask_cvttnebf162iubs_epi8(S, U, A)                                 \
  ((__m128i)__builtin_ia32_vcvttnebf162iubs128_mask(                           \
      (__v8hu)(S), (__mmask8)(U), (__v8hu)(A)))

#define _mm_maskz_cvttnebf162iubs_epi8(U, A)                                   \
  ((__m128i)__builtin_ia32_vcvttnebf162iubs128_maskz((__mmask8)(U),            \
                                                     (__v8hu)(A)))

#define _mm256_cvttnebf162iubs_epi8(A)                                         \
  ((__m256i)__builtin_ia32_vcvttnebf162iubs256((__v16hu)(A)))

#define _mm256_mask_cvttnebf162iubs_epi8(S, U, A)                              \
  ((__m256i)__builtin_ia32_vcvttnebf162iubs256_mask(                           \
      (__v16hu)(S), (__mmask16)(U), (__v16hu)(A)))

#define _mm256_maskz_cvttnebf162iubs_epi8(U, A)                                \
  ((__m256i)__builtin_ia32_vcvttnebf162iubs256_maskz((__mmask16)(U),           \
                                                     (__v16hu)(A)))

#define _mm_cvttph2ibs_epi8(A)                                                 \
  ((__m128i)__builtin_ia32_vcvttph2ibs128((__v8hf)(A)))

#define _mm_mask_cvttph2ibs_epi8(S, U, A)                                      \
  ((__m128i)__builtin_ia32_vcvttph2ibs128_mask((__v8hu)(S), (__mmask8)(U),     \
                                               (__v8hf)(A)))

#define _mm_maskz_cvttph2ibs_epi8(U, A)                                        \
  ((__m128i)__builtin_ia32_vcvttph2ibs128_maskz((__mmask8)(U), (__v8hf)(A)))

#define _mm256_cvttph2ibs_epi8(A)                                              \
  ((__m256i)__builtin_ia32_vcvttph2ibs256((__v16hf)(A)))

#define _mm256_mask_cvttph2ibs_epi8(S, U, A)                                   \
  ((__m256i)__builtin_ia32_vcvttph2ibs256_mask((__v16hu)(S), (__mmask16)(U),   \
                                               (__v16hf)(A)))

#define _mm256_maskz_cvttph2ibs_epi8(U, A)                                     \
  ((__m256i)__builtin_ia32_vcvttph2ibs256_maskz((__mmask16)(U), (__v16hf)(A)))

#define _mm_cvttph2iubs_epi8(A)                                                \
  ((__m128i)__builtin_ia32_vcvttph2iubs128((__v8hf)(A)))

#define _mm_mask_cvttph2iubs_epi8(S, U, A)                                     \
  ((__m128i)__builtin_ia32_vcvttph2iubs128_mask((__v8hu)(S), (__mmask8)(U),    \
                                                (__v8hf)(A)))

#define _mm_maskz_cvttph2iubs_epi8(U, A)                                       \
  ((__m128i)__builtin_ia32_vcvttph2iubs128_maskz((__mmask8)(U), (__v8hf)(A)))

#define _mm256_cvttph2iubs_epi8(A)                                             \
  ((__m256i)__builtin_ia32_vcvttph2iubs256((__v16hf)(A)))

#define _mm256_mask_cvttph2iubs_epi8(S, U, A)                                  \
  ((__m256i)__builtin_ia32_vcvttph2iubs256_mask((__v16hu)(S), (__mmask16)(U),  \
                                                (__v16hf)(A)))

#define _mm256_maskz_cvttph2iubs_epi8(U, A)                                    \
  ((__m256i)__builtin_ia32_vcvttph2iubs256_maskz((__mmask16)(U), (__v16hf)(A)))

#define _mm_cvttps2ibs_epi8(A)                                                 \
  ((__m128i)__builtin_ia32_vcvttps2ibs128((__v4sf)(A)))

#define _mm_mask_cvttps2ibs_epi8(S, U, A)                                      \
  ((__m128i)__builtin_ia32_vcvttps2ibs128_mask((__v4su)(S), (__mmask8)(U),     \
                                               (__v4sf)(A)))

#define _mm_maskz_cvttps2ibs_epi8(U, A)                                        \
  ((__m128i)__builtin_ia32_vcvttps2ibs128_maskz((__mmask8)(U), (__v4sf)(A)))

#define _mm256_cvttps2ibs_epi8(A)                                              \
  ((__m256i)__builtin_ia32_vcvttps2ibs256((__v8sf)(A)))

#define _mm256_mask_cvttps2ibs_epi8(S, U, A)                                   \
  ((__m256i)__builtin_ia32_vcvttps2ibs256_mask((__v8su)(S), (__mmask8)(U),     \
                                               (__v8sf)(A)))

#define _mm256_maskz_cvttps2ibs_epi8(U, A)                                     \
  ((__m256i)__builtin_ia32_vcvttps2ibs256_maskz((__mmask8)(U), (__v8sf)(A)))

#define _mm_cvttps2iubs_epi8(A)                                                \
  ((__m128i)__builtin_ia32_vcvttps2iubs128((__v4sf)(A)))

#define _mm_mask_cvttps2iubs_epi8(S, U, A)                                     \
  ((__m128i)__builtin_ia32_vcvttps2iubs128_mask((__v4su)(S), (__mmask8)(U),    \
                                                (__v4sf)(A)))

#define _mm_maskz_cvttps2iubs_epi8(U, A)                                       \
  ((__m128i)__builtin_ia32_vcvttps2iubs128_maskz((__mmask8)(U), (__v4sf)(A)))

#define _mm256_cvttps2iubs_epi8(A)                                             \
  ((__m256i)__builtin_ia32_vcvttps2iubs256((__v8sf)(A)))

#define _mm256_mask_cvttps2iubs_epi8(S, U, A)                                  \
  ((__m256i)__builtin_ia32_vcvttps2iubs256_mask((__v8su)(S), (__mmask8)(U),    \
                                                (__v8sf)(A)))

#define _mm256_maskz_cvttps2iubs_epi8(U, A)                                    \
  ((__m256i)__builtin_ia32_vcvttps2iubs256_maskz((__mmask8)(U), (__v8sf)(A)))

#endif // __AVX512VLSATCVTINTRIN_H
