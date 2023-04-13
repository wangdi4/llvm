/*===------------- avx512satcvtintrin.h - AVX512SATCVT intrinsics ----------===
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
    "Never use <avx512satcvtintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVX512SATCVTINTRIN_H
#define __AVX512SATCVTINTRIN_H

#define _mm512_cvtnebf162ibs_epi8(A)                                           \
  ((__m512i)__builtin_ia32_vcvtnebf162ibs512((__v32hu)(A)))

#define _mm512_mask_cvtnebf162ibs_epi8(S, U, A)                                \
  ((__m512i)__builtin_ia32_vcvtnebf162ibs512_mask(                             \
      (__v32hu)(S), (__mmask32)(U), (__v32hu)(A)))

#define _mm512_maskz_cvtnebf162ibs_epi8(U, A)                                  \
  ((__m512i)__builtin_ia32_vcvtnebf162ibs512_maskz((__mmask32)(U),             \
                                                   (__v32hu)(A)))

#define _mm512_cvtnebf162iubs_epi8(A)                                          \
  ((__m512i)__builtin_ia32_vcvtnebf162iubs512((__v32hu)(A)))

#define _mm512_mask_cvtnebf162iubs_epi8(S, U, A)                               \
  ((__m512i)__builtin_ia32_vcvtnebf162iubs512_mask(                            \
      (__v32hu)(S), (__mmask32)(U), (__v32hu)(A)))

#define _mm512_maskz_cvtnebf162iubs_epi8(U, A)                                 \
  ((__m512i)__builtin_ia32_vcvtnebf162iubs512_maskz((__mmask32)(U),            \
                                                    (__v32hu)(A)))

#define _mm512_cvtph2ibs_epi8(A)                                               \
  ((__m512i)__builtin_ia32_vcvtph2ibs512((__v32hf)(A)))

#define _mm512_mask_cvtph2ibs_epi8(S, U, A)                                    \
  ((__m512i)__builtin_ia32_vcvtph2ibs512_mask((__v32hu)(S), (__mmask32)(U),    \
                                              (__v32hf)(A)))

#define _mm512_maskz_cvtph2ibs_epi8(U, A)                                      \
  ((__m512i)__builtin_ia32_vcvtph2ibs512_maskz((__mmask32)(U), (__v32hf)(A)))

#define _mm512_cvtph2ibs_round_epi8(A, R)                                      \
  ((__m512i)__builtin_ia32_vcvtph2ibs512_round((__v32hf)(A), (int)(R)))

#define _mm512_mask_cvtph2ibs_round_epi8(S, U, A, R)                           \
  ((__m512i)__builtin_ia32_vcvtph2ibs512_round_mask(                           \
      (__v32hu)(S), (__mmask32)(U), (__v32hf)(A), (int)(R)))

#define _mm512_maskz_cvtph2ibs_round_epi8(U, A, R)                             \
  ((__m512i)__builtin_ia32_vcvtph2ibs512_round_maskz((__mmask32)(U),           \
                                                     (__v32hf)(A), (int)(R)))

#define _mm512_cvtph2iubs_epi8(A)                                              \
  ((__m512i)__builtin_ia32_vcvtph2iubs512((__v32hf)(A)))

#define _mm512_mask_cvtph2iubs_epi8(S, U, A)                                   \
  ((__m512i)__builtin_ia32_vcvtph2iubs512_mask((__v32hu)(S), (__mmask32)(U),   \
                                               (__v32hf)(A)))

#define _mm512_maskz_cvtph2iubs_epi8(U, A)                                     \
  ((__m512i)__builtin_ia32_vcvtph2iubs512_maskz((__mmask32)(U), (__v32hf)(A)))

#define _mm512_cvtph2iubs_round_epi8(A, R)                                     \
  ((__m512i)__builtin_ia32_vcvtph2iubs512_round((__v32hf)(A), (int)(R)))

#define _mm512_mask_cvtph2iubs_round_epi8(S, U, A, R)                          \
  ((__m512i)__builtin_ia32_vcvtph2iubs512_round_mask(                          \
      (__v32hu)(S), (__mmask32)(U), (__v32hf)(A), (int)(R)))

#define _mm512_maskz_cvtph2iubs_round_epi8(U, A, R)                            \
  ((__m512i)__builtin_ia32_vcvtph2iubs512_round_maskz((__mmask32)(U),          \
                                                      (__v32hf)(A), (int)(R)))

#define _mm512_cvtps2ibs_epi8(A)                                               \
  ((__m512i)__builtin_ia32_vcvtps2ibs512((__v16sf)(A)))

#define _mm512_mask_cvtps2ibs_epi8(S, U, A)                                    \
  ((__m512i)__builtin_ia32_vcvtps2ibs512_mask((__v16su)(S), (__mmask16)(U),    \
                                              (__v16sf)(A)))

#define _mm512_maskz_cvtps2ibs_epi8(U, A)                                      \
  ((__m512i)__builtin_ia32_vcvtps2ibs512_maskz((__mmask16)(U), (__v16sf)(A)))

#define _mm512_cvtps2ibs_round_epi8(A, R)                                      \
  ((__m512i)__builtin_ia32_vcvtps2ibs512_round((__v16sf)(A), (int)(R)))

#define _mm512_mask_cvtps2ibs_round_epi8(S, U, A, R)                           \
  ((__m512i)__builtin_ia32_vcvtps2ibs512_round_mask(                           \
      (__v16su)(S), (__mmask16)(U), (__v16sf)(A), (int)(R)))

#define _mm512_maskz_cvtps2ibs_round_epi8(U, A, R)                             \
  ((__m512i)__builtin_ia32_vcvtps2ibs512_round_maskz((__mmask16)(U),           \
                                                     (__v16sf)(A), (int)(R)))

#define _mm512_cvtps2iubs_epi8(A)                                              \
  ((__m512i)__builtin_ia32_vcvtps2iubs512((__v16sf)(A)))

#define _mm512_mask_cvtps2iubs_epi8(S, U, A)                                   \
  ((__m512i)__builtin_ia32_vcvtps2iubs512_mask((__v16su)(S), (__mmask16)(U),   \
                                               (__v16sf)(A)))

#define _mm512_maskz_cvtps2iubs_epi8(U, A)                                     \
  ((__m512i)__builtin_ia32_vcvtps2iubs512_maskz((__mmask16)(U), (__v16sf)(A)))

#define _mm512_cvtps2iubs_round_epi8(A, R)                                     \
  ((__m512i)__builtin_ia32_vcvtps2iubs512_round((__v16sf)(A), (int)(R)))

#define _mm512_mask_cvtps2iubs_round_epi8(S, U, A, R)                          \
  ((__m512i)__builtin_ia32_vcvtps2iubs512_round_mask(                          \
      (__v16su)(S), (__mmask16)(U), (__v16sf)(A), (int)(R)))

#define _mm512_maskz_cvtps2iubs_round_epi8(U, A, R)                            \
  ((__m512i)__builtin_ia32_vcvtps2iubs512_round_maskz((__mmask16)(U),          \
                                                      (__v16sf)(A), (int)(R)))

#define _mm512_cvttnebf162ibs_epi8(A)                                          \
  ((__m512i)__builtin_ia32_vcvttnebf162ibs512((__v32hu)(A)))

#define _mm512_mask_cvttnebf162ibs_epi8(S, U, A)                               \
  ((__m512i)__builtin_ia32_vcvttnebf162ibs512_mask(                            \
      (__v32hu)(S), (__mmask32)(U), (__v32hu)(A)))

#define _mm512_maskz_cvttnebf162ibs_epi8(U, A)                                 \
  ((__m512i)__builtin_ia32_vcvttnebf162ibs512_maskz((__mmask32)(U),            \
                                                    (__v32hu)(A)))

#define _mm512_cvttnebf162iubs_epi8(A)                                         \
  ((__m512i)__builtin_ia32_vcvttnebf162iubs512((__v32hu)(A)))

#define _mm512_mask_cvttnebf162iubs_epi8(S, U, A)                              \
  ((__m512i)__builtin_ia32_vcvttnebf162iubs512_mask(                           \
      (__v32hu)(S), (__mmask32)(U), (__v32hu)(A)))

#define _mm512_maskz_cvttnebf162iubs_epi8(U, A)                                \
  ((__m512i)__builtin_ia32_vcvttnebf162iubs512_maskz((__mmask32)(U),           \
                                                     (__v32hu)(A)))

#define _mm512_cvttph2ibs_epi8(A)                                              \
  ((__m512i)__builtin_ia32_vcvttph2ibs512((__v32hf)(A)))

#define _mm512_mask_cvttph2ibs_epi8(S, U, A)                                   \
  ((__m512i)__builtin_ia32_vcvttph2ibs512_mask((__v32hu)(S), (__mmask32)(U),   \
                                               (__v32hf)(A)))

#define _mm512_maskz_cvttph2ibs_epi8(U, A)                                     \
  ((__m512i)__builtin_ia32_vcvttph2ibs512_maskz((__mmask32)(U), (__v32hf)(A)))

#define _mm512_cvttph2ibs_round_epi8(A, R)                                     \
  ((__m512i)__builtin_ia32_vcvttph2ibs512_round((__v32hf)(A), (int)(R)))

#define _mm512_mask_cvttph2ibs_round_epi8(S, U, A, R)                          \
  ((__m512i)__builtin_ia32_vcvttph2ibs512_round_mask(                          \
      (__v32hu)(S), (__mmask32)(U), (__v32hf)(A), (int)(R)))

#define _mm512_maskz_cvttph2ibs_round_epi8(U, A, R)                            \
  ((__m512i)__builtin_ia32_vcvttph2ibs512_round_maskz((__mmask32)(U),          \
                                                      (__v32hf)(A), (int)(R)))

#define _mm512_cvttph2iubs_epi8(A)                                             \
  ((__m512i)__builtin_ia32_vcvttph2iubs512((__v32hf)(A)))

#define _mm512_mask_cvttph2iubs_epi8(S, U, A)                                  \
  ((__m512i)__builtin_ia32_vcvttph2iubs512_mask((__v32hu)(S), (__mmask32)(U),  \
                                                (__v32hf)(A)))

#define _mm512_maskz_cvttph2iubs_epi8(U, A)                                    \
  ((__m512i)__builtin_ia32_vcvttph2iubs512_maskz((__mmask32)(U), (__v32hf)(A)))

#define _mm512_cvttph2iubs_round_epi8(A, R)                                    \
  ((__m512i)__builtin_ia32_vcvttph2iubs512_round((__v32hf)(A), (int)(R)))

#define _mm512_mask_cvttph2iubs_round_epi8(S, U, A, R)                         \
  ((__m512i)__builtin_ia32_vcvttph2iubs512_round_mask(                         \
      (__v32hu)(S), (__mmask32)(U), (__v32hf)(A), (int)(R)))

#define _mm512_maskz_cvttph2iubs_round_epi8(U, A, R)                           \
  ((__m512i)__builtin_ia32_vcvttph2iubs512_round_maskz(                        \
      (__mmask32)(U), (__v32hf)(A), (int)(R)))

#define _mm512_cvttps2ibs_epi8(A)                                              \
  ((__m512i)__builtin_ia32_vcvttps2ibs512((__v16sf)(A)))

#define _mm512_mask_cvttps2ibs_epi8(S, U, A)                                   \
  ((__m512i)__builtin_ia32_vcvttps2ibs512_mask((__v16su)(S), (__mmask16)(U),   \
                                               (__v16sf)(A)))

#define _mm512_maskz_cvttps2ibs_epi8(U, A)                                     \
  ((__m512i)__builtin_ia32_vcvttps2ibs512_maskz((__mmask16)(U), (__v16sf)(A)))

#define _mm512_cvttps2ibs_round_epi8(A, R)                                     \
  ((__m512i)__builtin_ia32_vcvttps2ibs512_round((__v16sf)(A), (int)(R)))

#define _mm512_mask_cvttps2ibs_round_epi8(S, U, A, R)                          \
  ((__m512i)__builtin_ia32_vcvttps2ibs512_round_mask(                          \
      (__v16su)(S), (__mmask16)(U), (__v16sf)(A), (int)(R)))

#define _mm512_maskz_cvttps2ibs_round_epi8(U, A, R)                            \
  ((__m512i)__builtin_ia32_vcvttps2ibs512_round_maskz((__mmask16)(U),          \
                                                      (__v16sf)(A), (int)(R)))

#define _mm512_cvttps2iubs_epi8(A)                                             \
  ((__m512i)__builtin_ia32_vcvttps2iubs512((__v16sf)(A)))

#define _mm512_mask_cvttps2iubs_epi8(S, U, A)                                  \
  ((__m512i)__builtin_ia32_vcvttps2iubs512_mask((__v16su)(S), (__mmask16)(U),  \
                                                (__v16sf)(A)))

#define _mm512_maskz_cvttps2iubs_epi8(U, A)                                    \
  ((__m512i)__builtin_ia32_vcvttps2iubs512_maskz((__mmask16)(U), (__v16sf)(A)))

#define _mm512_cvttps2iubs_round_epi8(A, R)                                    \
  ((__m512i)__builtin_ia32_vcvttps2iubs512_round((__v16sf)(A), (int)(R)))

#define _mm512_mask_cvttps2iubs_round_epi8(S, U, A, R)                         \
  ((__m512i)__builtin_ia32_vcvttps2iubs512_round_mask(                         \
      (__v16su)(S), (__mmask16)(U), (__v16sf)(A), (int)(R)))

#define _mm512_maskz_cvttps2iubs_round_epi8(U, A, R)                           \
  ((__m512i)__builtin_ia32_vcvttps2iubs512_round_maskz(                        \
      (__mmask16)(U), (__v16sf)(A), (int)(R)))

#endif // __AVX512SATCVTINTRIN_H
