/*===--------------- avx512vlminmaxintrin.h - AVX512VLMINMAX intrinsics -----------------===
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
#error "Never use <avx512vlminmaxintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVX512VLMINMAXINTRIN_H
#define __AVX512VLMINMAXINTRIN_H

#define _mm_minmaxne_pbh(A, B, C)                                              \
  ((__m128bh)__builtin_ia32_vminmaxnepbf16128((__v8hi)(A), (__v8hi)(B),        \
                                              (int)(C)))

#define _mm_mask_minmaxne_pbh(W, U, A, B, C)                                   \
  ((__m128bh)__builtin_ia32_vminmaxnepbf16128_mask(                            \
      (__v8hi)(A), (__v8hi)(B), (int)(C), (__v8hi)(__m128bh)(W),               \
      (__mmask8)(U)))

#define _mm_maskz_minmaxne_pbh(U, A, B, C)                                     \
  ((__m128bh)__builtin_ia32_vminmaxnepbf16128_mask(                            \
      (__v8hi)(A), (__v8hi)(B), (int)(C),                                      \
      (__v8hi)(__m128bh)_mm_setzero_si128(), (__mmask8)(U)))

#define _mm256_minmaxne_pbh(A, B, C)                                           \
  ((__m256bh)__builtin_ia32_vminmaxnepbf16256((__v16hi)(A), (__v16hi)(B),      \
                                              (int)(C)))

#define _mm256_mask_minmaxne_pbh(W, U, A, B, C)                                \
  ((__m256bh)__builtin_ia32_vminmaxnepbf16256_mask(                            \
      (__v16hi)(A), (__v16hi)(B), (int)(C), (__v16hi)(__m256bh)(W),            \
      (__mmask16)(U)))

#define _mm256_maskz_minmaxne_pbh(U, A, B, C)                                  \
  ((__m256bh)__builtin_ia32_vminmaxnepbf16256_mask(                            \
      (__v16hi)(A), (__v16hi)(B), (int)(C),                                    \
      (__v16hi)(__m256bh)_mm256_setzero_si256(), (__mmask16)(U)))

#define _mm_minmax_pd(A, B, C)                                                 \
  ((__m128d)__builtin_ia32_vminmaxpd128((__v2df)(A), (__v2df)(B), (int)(C)))

#define _mm_mask_minmax_pd(W, U, A, B, C)                                      \
  ((__m128d)__builtin_ia32_vminmaxpd128_mask((__v2df)(A), (__v2df)(B),         \
                                             (int)(C), (__v2df)(__m128d)(W),   \
                                             (__mmask8)(U)))

#define _mm_maskz_minmax_pd(U, A, B, C)                                        \
  ((__m128d)__builtin_ia32_vminmaxpd128_mask(                                  \
      (__v2df)(A), (__v2df)(B), (int)(C), (__v2df)(__m128d)_mm_setzero_pd(),   \
      (__mmask8)(U)))

#define _mm256_minmax_pd(A, B, C)                                              \
  ((__m256d)__builtin_ia32_vminmaxpd256((__v4df)(A), (__v4df)(B), (int)(C)))

#define _mm256_mask_minmax_pd(W, U, A, B, C)                                   \
  ((__m256d)__builtin_ia32_vminmaxpd256_mask((__v4df)(A), (__v4df)(B),         \
                                             (int)(C), (__v4df)(__m256d)(W),   \
                                             (__mmask8)(U)))

#define _mm256_maskz_minmax_pd(U, A, B, C)                                     \
  ((__m256d)__builtin_ia32_vminmaxpd256_mask(                                  \
      (__v4df)(A), (__v4df)(B), (int)(C),                                      \
      (__v4df)(__m256d)_mm256_setzero_pd(), (__mmask8)(U)))

#define _mm_minmax_ph(A, B, C)                                                 \
  ((__m128h)__builtin_ia32_vminmaxph128((__v8hf)(A), (__v8hf)(B), (int)(C)))

#define _mm_mask_minmax_ph(W, U, A, B, C)                                      \
  ((__m128h)__builtin_ia32_vminmaxph128_mask((__v8hf)(A), (__v8hf)(B),         \
                                             (int)(C), (__v8hf)(__m128h)(W),   \
                                             (__mmask8)(U)))

#define _mm_maskz_minmax_ph(U, A, B, C)                                        \
  ((__m128h)__builtin_ia32_vminmaxph128_mask(                                  \
      (__v8hf)(A), (__v8hf)(B), (int)(C), (__v8hf)(__m128h)_mm_setzero_ph(),   \
      (__mmask8)(U)))

#define _mm256_minmax_ph(A, B, C)                                              \
  ((__m256h)__builtin_ia32_vminmaxph256((__v16hf)(A), (__v16hf)(B), (int)(C)))

#define _mm256_mask_minmax_ph(W, U, A, B, C)                                   \
  ((__m256h)__builtin_ia32_vminmaxph256_mask((__v16hf)(A), (__v16hf)(B),       \
                                             (int)(C), (__v16hf)(__m256h)(W),  \
                                             (__mmask16)(U)))

#define _mm256_maskz_minmax_ph(U, A, B, C)                                     \
  ((__m256h)__builtin_ia32_vminmaxph256_mask(                                  \
      (__v16hf)(A), (__v16hf)(B), (int)(C),                                    \
      (__v16hf)(__m256h)_mm256_setzero_ph(), (__mmask16)(U)))

#define _mm_minmax_ps(A, B, C)                                                 \
  ((__m128)__builtin_ia32_vminmaxps128((__v4sf)(A), (__v4sf)(B), (int)(C)))

#define _mm_mask_minmax_ps(W, U, A, B, C)                                      \
  ((__m128)__builtin_ia32_vminmaxps128_mask(                                   \
      (__v4sf)(A), (__v4sf)(B), (int)(C), (__v4sf)(__m128)(W), (__mmask8)(U)))

#define _mm_maskz_minmax_ps(U, A, B, C)                                        \
  ((__m128)__builtin_ia32_vminmaxps128_mask(                                   \
      (__v4sf)(A), (__v4sf)(B), (int)(C), (__v4sf)(__m128)_mm_setzero_ps(),    \
      (__mmask8)(U)))

#define _mm256_minmax_ps(A, B, C)                                              \
  ((__m256)__builtin_ia32_vminmaxps256((__v8sf)(A), (__v8sf)(B), (int)(C)))

#define _mm256_mask_minmax_ps(W, U, A, B, C)                                   \
  ((__m256)__builtin_ia32_vminmaxps256_mask(                                   \
      (__v8sf)(A), (__v8sf)(B), (int)(C), (__v8sf)(__m256)(W), (__mmask8)(U)))

#define _mm256_maskz_minmax_ps(U, A, B, C)                                     \
  ((__m256)__builtin_ia32_vminmaxps256_mask(                                   \
      (__v8sf)(A), (__v8sf)(B), (int)(C), (__v8sf)(__m256)_mm256_setzero_ps(), \
      (__mmask8)(U)))

#endif // __AVX512VLMINMAXINTRIN_H
