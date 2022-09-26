/*===--------------- avx512minmaxintrin.h - AVX512MINMAX intrinsics -----------------===
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
#error "Never use <avx512minmaxintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVX512MINMAXINTRIN_H
#define __AVX512MINMAXINTRIN_H

#define _mm512_minmaxne_pbh(A, B, C)                                           \
  ((__m512bh)__builtin_ia32_vminmaxnepbf16512((__v32hi)(A), (__v32hi)(B),      \
                                              (int)(C)))

#define _mm512_mask_minmaxne_pbh(W, U, A, B, C)                                \
  ((__m512bh)__builtin_ia32_vminmaxnepbf16512_mask(                            \
      (__v32hi)(A), (__v32hi)(B), (int)(C), (__v32hi)(__m512bh)(W),            \
      (__mmask32)(U)))

#define _mm512_maskz_minmaxne_pbh(U, A, B, C)                                  \
  ((__m512bh)__builtin_ia32_vminmaxnepbf16512_mask(                            \
      (__v32hi)(A), (__v32hi)(B), (int)(C),                                    \
      (__v32hi)(__m512bh)_mm512_setzero_si512(), (__mmask32)(U)))

#define _mm512_minmax_pd(A, B, C)                                              \
  ((__m512d)__builtin_ia32_vminmaxpd_round_mask(                               \
      (__v8df)(A), (__v8df)(B), (C), (__v8df)_mm512_undefined_pd(),            \
      (__mmask8)-1, _MM_FROUND_CUR_DIRECTION))

#define _mm512_mask_minmax_pd(W, U, A, B, C)                                   \
  ((__m512d)__builtin_ia32_vminmaxpd_round_mask(                               \
      (__v8df)(A), (__v8df)(B), (C), (__v8df)(__m512d)(W), (__mmask8)(U),      \
      _MM_FROUND_CUR_DIRECTION))

#define _mm512_maskz_minmax_pd(U, A, B, C)                                     \
  ((__m512d)__builtin_ia32_vminmaxpd_round_mask(                               \
      (__v8df)(A), (__v8df)(B), (C), (__v8df)(__m512d)_mm512_setzero_pd(),     \
      (__mmask8)(U), _MM_FROUND_CUR_DIRECTION))

#define _mm512_minmaxpd_round_pd(A, B, C, R)                                   \
  ((__m512d)__builtin_ia32_vminmaxpd_round_mask((__v8df)(A), (__v8df)(B), (C), \
                                                (__v8df)_mm512_undefined_pd(), \
                                                (__mmask8)-1, (int)(R)))

#define _mm512_mask_minmaxpd_round_pd(W, U, A, B, C, R)                        \
  ((__m512d)__builtin_ia32_vminmaxpd_round_mask((__v8df)(A), (__v8df)(B), (C), \
                                                (__v8df)(__m512d)(W),          \
                                                (__mmask8)(U), (int)(R)))

#define _mm512_maskz_minmaxpd_round_pd(U, A, B, C, R)                          \
  ((__m512d)__builtin_ia32_vminmaxpd_round_mask(                               \
      (__v8df)(A), (__v8df)(B), (C), (__v8df)(__m512d)_mm512_setzero_pd(),     \
      (__mmask8)(U), (int)(R)))

#define _mm512_minmax_ph(A, B, C)                                              \
  ((__m512h)__builtin_ia32_vminmaxph_round_mask(                               \
      (__v32hf)(A), (__v32hf)(B), (C), (__v32hf)_mm512_undefined_ph(),         \
      (__mmask32)-1, _MM_FROUND_CUR_DIRECTION))

#define _mm512_mask_minmax_ph(W, U, A, B, C)                                   \
  ((__m512h)__builtin_ia32_vminmaxph_round_mask(                               \
      (__v32hf)(A), (__v32hf)(B), (C), (__v32hf)(__m512h)(W), (__mmask32)(U),  \
      _MM_FROUND_CUR_DIRECTION))

#define _mm512_maskz_minmax_ph(U, A, B, C)                                     \
  ((__m512h)__builtin_ia32_vminmaxph_round_mask(                               \
      (__v32hf)(A), (__v32hf)(B), (C), (__v32hf)(__m512h)_mm512_setzero_ph(),  \
      (__mmask32)(U), _MM_FROUND_CUR_DIRECTION))

#define _mm512_minmaxph_round_ph(A, B, C, R)                                   \
  ((__m512h)__builtin_ia32_vminmaxph_round_mask(                               \
      (__v32hf)(A), (__v32hf)(B), (C), (__v32hf)_mm512_undefined_ph(),         \
      (__mmask32)-1, (int)(R)))

#define _mm512_mask_minmaxph_round_ph(W, U, A, B, C, R)                        \
  ((__m512h)__builtin_ia32_vminmaxph_round_mask((__v32hf)(A), (__v32hf)(B),    \
                                                (C), (__v32hf)(__m512h)(W),    \
                                                (__mmask32)(U), (int)(R)))

#define _mm512_maskz_minmaxph_round_ph(U, A, B, C, R)                          \
  ((__m512h)__builtin_ia32_vminmaxph_round_mask(                               \
      (__v32hf)(A), (__v32hf)(B), (C), (__v32hf)(__m512h)_mm512_setzero_ph(),  \
      (__mmask32)(U), (int)(R)))

#define _mm512_minmax_ps(A, B, C)                                              \
  ((__m512)__builtin_ia32_vminmaxps_round_mask(                                \
      (__v16sf)(A), (__v16sf)(B), (C), (__v16sf)_mm512_undefined_ps(),         \
      (__mmask16)-1, _MM_FROUND_CUR_DIRECTION))

#define _mm512_mask_minmax_ps(W, U, A, B, C)                                   \
  ((__m512)__builtin_ia32_vminmaxps_round_mask(                                \
      (__v16sf)(A), (__v16sf)(B), (C), (__v16sf)(__m512)(W), (__mmask16)(U),   \
      _MM_FROUND_CUR_DIRECTION))

#define _mm512_maskz_minmax_ps(U, A, B, C)                                     \
  ((__m512)__builtin_ia32_vminmaxps_round_mask(                                \
      (__v16sf)(A), (__v16sf)(B), (C), (__v16sf)(__m512)_mm512_setzero_ps(),   \
      (__mmask16)(U), _MM_FROUND_CUR_DIRECTION))

#define _mm512_minmaxps_round_ps(A, B, C, R)                                   \
  ((__m512)__builtin_ia32_vminmaxps_round_mask(                                \
      (__v16sf)(A), (__v16sf)(B), (C), (__v16sf)_mm512_undefined_ps(),         \
      (__mmask16)-1, (int)(R)))

#define _mm512_mask_minmaxps_round_ps(W, U, A, B, C, R)                        \
  ((__m512)__builtin_ia32_vminmaxps_round_mask((__v16sf)(A), (__v16sf)(B),     \
                                               (C), (__v16sf)(__m512)(W),      \
                                               (__mmask16)(U), (int)(R)))

#define _mm512_maskz_minmaxps_round_ps(U, A, B, C, R)                          \
  ((__m512)__builtin_ia32_vminmaxps_round_mask(                                \
      (__v16sf)(A), (__v16sf)(B), (C), (__v16sf)(__m512)_mm512_setzero_ps(),   \
      (__mmask16)(U), (int)(R)))

#define _mm_minmaxsd_sd(A, B, C)                                               \
  ((__m128d)__builtin_ia32_vminmaxsd_round_mask(                               \
      (__v2df)(A), (__v2df)(B), (C), (__v2df)_mm_undefined_pd(), (__mmask8)-1, \
      _MM_FROUND_CUR_DIRECTION))

#define _mm_mask_minmaxsd_sd(W, U, A, B, C)                                    \
  ((__m128d)__builtin_ia32_vminmaxsd_round_mask(                               \
      (__v2df)(A), (__v2df)(B), (C), (__v2df)(__m128d)(W), (__mmask8)(U),      \
      _MM_FROUND_CUR_DIRECTION))

#define _mm_maskz_minmaxsd_sd(U, A, B, C)                                      \
  ((__m128d)__builtin_ia32_vminmaxsd_round_mask(                               \
      (__v2df)(A), (__v2df)(B), (C), (__v2df)(__m128d)_mm_setzero_pd(),        \
      (__mmask8)(U), _MM_FROUND_CUR_DIRECTION))

#define _mm_minmaxsd_round_sd(A, B, C, R)                                      \
  ((__m128d)__builtin_ia32_vminmaxsd_round_mask((__v2df)(A), (__v2df)(B), (C), \
                                                (__v2df)_mm_undefined_pd(),    \
                                                (__mmask8)-1, (int)(R)))

#define _mm_mask_minmaxsd_round_sd(W, U, A, B, C, R)                           \
  ((__m128d)__builtin_ia32_vminmaxsd_round_mask((__v2df)(A), (__v2df)(B), (C), \
                                                (__v2df)(__m128d)(W),          \
                                                (__mmask8)(U), (int)(R)))

#define _mm_maskz_minmaxsd_round_sd(U, A, B, C, R)                             \
  ((__m128d)__builtin_ia32_vminmaxsd_round_mask(                               \
      (__v2df)(A), (__v2df)(B), (C), (__v2df)(__m128d)_mm_setzero_pd(),        \
      (__mmask8)(U), (int)(R)))

#define _mm_minmaxsh_sh(A, B, C)                                               \
  ((__m128h)__builtin_ia32_vminmaxsh_round_mask(                               \
      (__v8hf)(A), (__v8hf)(B), (C), (__v8hf)_mm_undefined_ph(), (__mmask8)-1, \
      _MM_FROUND_CUR_DIRECTION))

#define _mm_mask_minmaxsh_sh(W, U, A, B, C)                                    \
  ((__m128h)__builtin_ia32_vminmaxsh_round_mask(                               \
      (__v8hf)(A), (__v8hf)(B), (C), (__v8hf)(__m128h)(W), (__mmask8)(U),      \
      _MM_FROUND_CUR_DIRECTION))

#define _mm_maskz_minmaxsh_sh(U, A, B, C)                                      \
  ((__m128h)__builtin_ia32_vminmaxsh_round_mask(                               \
      (__v8hf)(A), (__v8hf)(B), (C), (__v8hf)(__m128h)_mm_setzero_ph(),        \
      (__mmask8)(U), _MM_FROUND_CUR_DIRECTION))

#define _mm_minmaxsh_round_sh(A, B, C, R)                                      \
  ((__m128h)__builtin_ia32_vminmaxsh_round_mask((__v8hf)(A), (__v8hf)(B), (C), \
                                                (__v8hf)_mm_undefined_ph(),    \
                                                (__mmask8)-1, (int)(R)))

#define _mm_mask_minmaxsh_round_sh(W, U, A, B, C, R)                           \
  ((__m128h)__builtin_ia32_vminmaxsh_round_mask((__v8hf)(A), (__v8hf)(B), (C), \
                                                (__v8hf)(__m128h)(W),          \
                                                (__mmask8)(U), (int)(R)))

#define _mm_maskz_minmaxsh_round_sh(U, A, B, C, R)                             \
  ((__m128h)__builtin_ia32_vminmaxsh_round_mask(                               \
      (__v8hf)(A), (__v8hf)(B), (C), (__v8hf)(__m128h)_mm_setzero_ph(),        \
      (__mmask8)(U), (int)(R)))

#define _mm_minmaxss_ss(A, B, C)                                               \
  ((__m128)__builtin_ia32_vminmaxss_round_mask(                                \
      (__v4sf)(A), (__v4sf)(B), (C), (__v4sf)_mm_undefined_ps(), (__mmask8)-1, \
      _MM_FROUND_CUR_DIRECTION))

#define _mm_mask_minmaxss_ss(W, U, A, B, C)                                    \
  ((__m128)__builtin_ia32_vminmaxss_round_mask(                                \
      (__v4sf)(A), (__v4sf)(B), (C), (__v4sf)(__m128)(W), (__mmask8)(U),       \
      _MM_FROUND_CUR_DIRECTION))

#define _mm_maskz_minmaxss_ss(U, A, B, C)                                      \
  ((__m128)__builtin_ia32_vminmaxss_round_mask(                                \
      (__v4sf)(A), (__v4sf)(B), (C), (__v4sf)(__m128)_mm_setzero_ps(),         \
      (__mmask8)(U), _MM_FROUND_CUR_DIRECTION))

#define _mm_minmaxss_round_ss(A, B, C, R)                                      \
  ((__m128)__builtin_ia32_vminmaxss_round_mask((__v4sf)(A), (__v4sf)(B), (C),  \
                                               (__v4sf)_mm_undefined_ps(),     \
                                               (__mmask8)-1, (int)(R)))

#define _mm_mask_minmaxss_round_ss(W, U, A, B, C, R)                           \
  ((__m128)__builtin_ia32_vminmaxss_round_mask((__v4sf)(A), (__v4sf)(B), (C),  \
                                               (__v4sf)(__m128)(W),            \
                                               (__mmask8)(U), (int)(R)))

#define _mm_maskz_minmaxss_round_ss(U, A, B, C, R)                             \
  ((__m128)__builtin_ia32_vminmaxss_round_mask(                                \
      (__v4sf)(A), (__v4sf)(B), (C), (__v4sf)(__m128)_mm_setzero_ps(),         \
      (__mmask8)(U), (int)(R)))

#endif // __AVX512MINMAXINTRIN_H
