/* INTEL_FEATURE_ISA_FP16 */
/*===--------------- avx512fp16intrin.h - FP16 intrinsics -----------------===
 *
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 *
 * The information and source code contained herein is the exclusive property
 * of Intel Corporation and may not be disclosed, examined or reproduced in
 * whole or in part without explicit written authorization from the company.
 *
 *===----------------------------------------------------------------------===
 */
#ifndef __IMMINTRIN_H
#error "Never use <avx512fp16intrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512FP16INTRIN_H
#define __AVX512FP16INTRIN_H

/* Define the default attributes for the functions in this file. */
typedef _Float16 __v32hf __attribute__((__vector_size__(64), __aligned__(64)));
typedef _Float16 __m512h __attribute__((__vector_size__(64), __aligned__(64)));
typedef _Float16 __v8hf __attribute__((__vector_size__(16), __aligned__(16)));
typedef _Float16 __m128h __attribute__((__vector_size__(16), __aligned__(16)));
typedef _Float16 __v16hf __attribute__((__vector_size__(32), __aligned__(32)));
typedef _Float16 __m256h __attribute__((__vector_size__(32), __aligned__(32)));

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS512 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512fp16"), __min_vector_width__(512)))
#define __DEFAULT_FN_ATTRS256 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512fp16"), __min_vector_width__(256)))
#define __DEFAULT_FN_ATTRS128 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512fp16"), __min_vector_width__(128)))

static  __inline __m128h __DEFAULT_FN_ATTRS128
_mm_setzero_ph(void)
{
  return (__m128h){ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
}

static  __inline __m256h __DEFAULT_FN_ATTRS256
_mm256_setzero_ph(void)
{
  return (__m256h){0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                   0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_undefined_ph()
{
  return (__m256h)__builtin_ia32_undef256();
}

static  __inline __m512h __DEFAULT_FN_ATTRS512
_mm512_setzero_ph(void)
{
  return (__m512h){0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                   0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                   0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_undefined_ph()
{
  return (__m128h)__builtin_ia32_undef128();
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_undefined_ph()
{
  return (__m512h)__builtin_ia32_undef512();
}

static __inline __m512h __DEFAULT_FN_ATTRS512
_mm512_set1_ph(_Float16 __h)
{
  return (__m512h)(__v32hf){ __h, __h, __h, __h, __h, __h, __h, __h,
                             __h, __h, __h, __h, __h, __h, __h, __h,
                             __h, __h, __h, __h, __h, __h, __h, __h,
                             __h, __h, __h, __h, __h, __h, __h, __h };
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_castph_ps(__m128h __a)
{
  return (__m128)__a;
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_castph_ps(__m256h __a)
{
  return (__m256)__a;
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_castph_ps(__m512h __a)
{
  return (__m512)__a;
}

static __inline__ __m128d __DEFAULT_FN_ATTRS128
_mm_castph_pd(__m128h __a)
{
  return (__m128d)__a;
}

static __inline__ __m256d __DEFAULT_FN_ATTRS256
_mm256_castph_pd(__m256h __a)
{
  return (__m256d)__a;
}

static __inline__ __m512d __DEFAULT_FN_ATTRS512
_mm512_castph_pd(__m512h __a)
{
  return (__m512d)__a;
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_castph_si128(__m128h __a)
{
  return (__m128i)__a;
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_castph_si256(__m256h __a)
{
  return (__m256i)__a;
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_castph_si512(__m512h __a)
{
  return (__m512i)__a;
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_castps_ph(__m128 __a)
{
  return (__m128h)__a;
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_castps_ph(__m256 __a)
{
  return (__m256h)__a;
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_castps_ph(__m512 __a)
{
  return (__m512h)__a;
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_castpd_ph(__m128d __a)
{
  return (__m128h)__a;
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_castpd_ph(__m256d __a)
{
  return (__m256h)__a;
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_castpd_ph(__m512d __a)
{
  return (__m512h)__a;
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_castsi128_ph(__m128i __a)
{
  return (__m128h)__a;
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_castsi256_ph(__m256i __a)
{
  return (__m256h)__a;
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_castsi512_ph(__m512i __a)
{
  return (__m512h)__a;
}

static __inline__ __m128h __DEFAULT_FN_ATTRS256
_mm256_castph256_ph128(__m256h __a)
{
  return __builtin_shufflevector(__a, __a, 0, 1, 2, 3, 4, 5, 6, 7);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS512
_mm512_castph512_ph128(__m512h __a)
{
  return __builtin_shufflevector(__a, __a, 0, 1, 2, 3, 4, 5, 6, 7);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS512
_mm512_castph512_ph256(__m512h __a)
{
  return __builtin_shufflevector(__a, __a, 0,  1,  2,  3,  4,  5,  6,  7,
                                           8,  9, 10, 11, 12, 13, 14, 15);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_castph128_ph256(__m128h __a)
{
  return __builtin_shufflevector(__a, __a, 0,  1,  2,  3,  4,  5,  6,  7,
                                          -1, -1, -1, -1, -1, -1, -1, -1);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_castph128_ph512(__m128h __a)
{
  return __builtin_shufflevector(__a, __a, 0,  1,  2,  3,  4,  5,  6,  7,
                                          -1, -1, -1, -1, -1, -1, -1, -1,
                                          -1, -1, -1, -1, -1, -1, -1, -1,
                                          -1, -1, -1, -1, -1, -1, -1, -1);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_castph256_ph512(__m256h __a)
{
  return __builtin_shufflevector(__a, __a, 0,  1,  2,  3,  4,  5,  6,  7,
                                           8,  9, 10, 11, 12, 13, 14, 15,
                                          -1, -1, -1, -1, -1, -1, -1, -1,
                                          -1, -1, -1, -1, -1, -1, -1, -1);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_zextph128_ph256(__m128h __a)
{
  return __builtin_shufflevector(__a, (__v8hf)_mm_setzero_ph(),
                                 0,  1,  2,  3,  4,  5,  6,  7,
                                 8,  9, 10, 11, 12, 13, 14, 15);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_zextph128_ph512(__m128h __a)
{
  return __builtin_shufflevector(__a, (__v8hf)_mm_setzero_ph(),
                                 0,  1,  2,  3,  4,  5,  6,  7,
                                 8,  9, 10, 11, 12, 13, 14, 15,
                                 8,  9, 10, 11, 12, 13, 14, 15,
                                 8,  9, 10, 11, 12, 13, 14, 15);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_zextph256_ph512(__m256h __a)
{
  return __builtin_shufflevector(__a, (__v16hf)_mm256_setzero_ph(),
                                  0,  1,  2,  3,  4,  5,  6,  7,
                                  8,  9, 10, 11, 12, 13, 14, 15,
                                 16, 17, 18, 19, 20, 21, 22, 23,
                                 24, 25, 26, 27, 28, 29, 30, 31);
}

#define _mm_comi_round_sh(A, B, P, R) \
  __builtin_ia32_vcomish((__v8hf)A, (__v8hf)B, (int)(P), (int)(R))

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_add_ph(__m512h __A, __m512h __B) {
  return (__m512h)((__v32hf)__A + (__v32hf)__B);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_add_ph(__m512h __W, __mmask32 __U, __m512h __A, __m512h __B) {
  return (__m512h)__builtin_ia32_selectph_512((__mmask32)__U,
                                              (__v32hf)_mm512_add_ph(__A, __B),
                                              (__v32hf)__W);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_add_ph(__mmask32 __U, __m512h __A, __m512h __B) {
  return (__m512h)__builtin_ia32_selectph_512((__mmask32)__U,
                                              (__v32hf)_mm512_add_ph(__A, __B),
                                              (__v32hf)_mm512_setzero_ph());
}

#define _mm512_add_round_ph(A, B, R) \
  (__m512h)__builtin_ia32_addph512((__v32hf)(__m512h)(A), \
                                   (__v32hf)(__m512h)(B), (int)(R))

#define _mm512_mask_add_round_ph(W, U, A, B, R) \
  (__m512h)__builtin_ia32_selectph_512((__mmask32)(U), \
                                   (__v32hf)_mm512_add_round_ph((A), (B), (R)), \
                                   (__v32hf)(__m512h)(W));

#define _mm512_maskz_add_round_ph(U, A, B, R) \
  (__m512h)__builtin_ia32_selectph_512((__mmask32)(U), \
                                   (__v32hf)_mm512_add_round_ph((A), (B), (R)), \
                                   (__v32hf)_mm512_setzero_ph());


static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_sub_ph(__m512h __A, __m512h __B) {
  return (__m512h)((__v32hf)__A - (__v32hf)__B);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_sub_ph(__m512h __W, __mmask32 __U, __m512h __A, __m512h __B) {
  return (__m512h)__builtin_ia32_selectph_512((__mmask32)__U,
                                              (__v32hf)_mm512_sub_ph(__A, __B),
                                              (__v32hf)__W);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_sub_ph(__mmask32 __U, __m512h __A, __m512h __B) {
  return (__m512h)__builtin_ia32_selectph_512((__mmask32)__U,
                                              (__v32hf)_mm512_sub_ph(__A, __B),
                                              (__v32hf)_mm512_setzero_ph());
}

#define _mm512_sub_round_ph(A, B, R) \
  (__m512h)__builtin_ia32_subph512((__v32hf)(__m512h)(A), \
                                   (__v32hf)(__m512h)(B), (int)(R))

#define _mm512_mask_sub_round_ph(W, U, A, B, R) \
  (__m512h)__builtin_ia32_selectph_512((__mmask32)(U), \
                                   (__v32hf)_mm512_sub_round_ph((A), (B), (R)), \
                                   (__v32hf)(__m512h)(W));

#define _mm512_maskz_sub_round_ph(U, A, B, R) \
  (__m512h)__builtin_ia32_selectph_512((__mmask32)(U), \
                                   (__v32hf)_mm512_sub_round_ph((A), (B), (R)), \
                                   (__v32hf)_mm512_setzero_ph());

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mul_ph(__m512h __A, __m512h __B) {
  return (__m512h)((__v32hf)__A * (__v32hf)__B);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_mul_ph(__m512h __W, __mmask32 __U, __m512h __A, __m512h __B) {
  return (__m512h)__builtin_ia32_selectph_512((__mmask32)__U,
                                              (__v32hf)_mm512_mul_ph(__A, __B),
                                              (__v32hf)__W);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_mul_ph(__mmask32 __U, __m512h __A, __m512h __B) {
  return (__m512h)__builtin_ia32_selectph_512((__mmask32)__U,
                                              (__v32hf)_mm512_mul_ph(__A, __B),
                                              (__v32hf)_mm512_setzero_ph());
}

#define _mm512_mul_round_ph(A, B, R) \
  (__m512h)__builtin_ia32_mulph512((__v32hf)(__m512h)(A), \
                                   (__v32hf)(__m512h)(B), (int)(R))

#define _mm512_mask_mul_round_ph(W, U, A, B, R) \
  (__m512h)__builtin_ia32_selectph_512((__mmask32)(U), \
                                   (__v32hf)_mm512_mul_round_ph((A), (B), (R)), \
                                   (__v32hf)(__m512h)(W));

#define _mm512_maskz_mul_round_ph(U, A, B, R) \
  (__m512h)__builtin_ia32_selectph_512((__mmask32)(U), \
                                   (__v32hf)_mm512_mul_round_ph((A), (B), (R)), \
                                   (__v32hf)_mm512_setzero_ph());

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_div_ph(__m512h __A, __m512h __B) {
  return (__m512h)((__v32hf)__A / (__v32hf)__B);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_div_ph(__m512h __W, __mmask32 __U, __m512h __A, __m512h __B) {
  return (__m512h)__builtin_ia32_selectph_512((__mmask32)__U,
                                              (__v32hf)_mm512_div_ph(__A, __B),
                                              (__v32hf)__W);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_div_ph(__mmask32 __U, __m512h __A, __m512h __B) {
  return (__m512h)__builtin_ia32_selectph_512((__mmask32)__U,
                                              (__v32hf)_mm512_div_ph(__A, __B),
                                              (__v32hf)_mm512_setzero_ph());
}

#define _mm512_div_round_ph(A, B, R) \
  (__m512h)__builtin_ia32_divph512((__v32hf)(__m512h)(A), \
                                   (__v32hf)(__m512h)(B), (int)(R))

#define _mm512_mask_div_round_ph(W, U, A, B, R) \
  (__m512h)__builtin_ia32_selectph_512((__mmask32)(U), \
                                   (__v32hf)_mm512_div_round_ph((A), (B), (R)), \
                                   (__v32hf)(__m512h)(W));

#define _mm512_maskz_div_round_ph(U, A, B, R) \
  (__m512h)__builtin_ia32_selectph_512((__mmask32)(U), \
                                   (__v32hf)_mm512_div_round_ph((A), (B), (R)), \
                                   (__v32hf)_mm512_setzero_ph());

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_min_ph(__m512h __A, __m512h __B) {
  return (__m512h) __builtin_ia32_minph512((__v32hf) __A, (__v32hf) __B,
                                           _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_min_ph(__m512h __W, __mmask32 __U, __m512h __A, __m512h __B) {
  return (__m512h)__builtin_ia32_selectph_512((__mmask32)__U,
                                              (__v32hf)_mm512_min_ph(__A, __B),
                                              (__v32hf)__W);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_min_ph(__mmask32 __U, __m512h __A, __m512h __B) {
  return (__m512h)__builtin_ia32_selectph_512((__mmask32)__U,
                                              (__v32hf)_mm512_min_ph(__A, __B),
                                              (__v32hf)_mm512_setzero_ph());
}

#define _mm512_min_round_ph(A, B, R) \
  (__m512h)__builtin_ia32_minph512((__v32hf)(__m512h)(A), \
                                   (__v32hf)(__m512h)(B), (int)(R))

#define _mm512_mask_min_round_ph(W, U, A, B, R) \
  (__m512h)__builtin_ia32_selectph_512((__mmask32)(U), \
                                   (__v32hf)_mm512_min_round_ph((A), (B), (R)), \
                                   (__v32hf)(__m512h)(W));

#define _mm512_maskz_min_round_ph(U, A, B, R) \
  (__m512h)__builtin_ia32_selectph_512((__mmask32)(U), \
                                   (__v32hf)_mm512_min_round_ph((A), (B), (R)), \
                                   (__v32hf)_mm512_setzero_ph());

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_max_ph(__m512h __A, __m512h __B) {
  return (__m512h) __builtin_ia32_maxph512((__v32hf) __A, (__v32hf) __B,
                                           _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_max_ph(__m512h __W, __mmask32 __U, __m512h __A, __m512h __B) {
  return (__m512h)__builtin_ia32_selectph_512((__mmask32)__U,
                                              (__v32hf)_mm512_max_ph(__A, __B),
                                              (__v32hf)__W);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_max_ph(__mmask32 __U, __m512h __A, __m512h __B) {
  return (__m512h)__builtin_ia32_selectph_512((__mmask32)__U,
                                              (__v32hf)_mm512_max_ph(__A, __B),
                                              (__v32hf)_mm512_setzero_ph());
}

#define _mm512_max_round_ph(A, B, R) \
  (__m512h)__builtin_ia32_maxph512((__v32hf)(__m512h)(A), \
                                   (__v32hf)(__m512h)(B), (int)(R))

#define _mm512_mask_max_round_ph(W, U, A, B, R) \
  (__m512h)__builtin_ia32_selectph_512((__mmask32)(U), \
                                   (__v32hf)_mm512_max_round_ph((A), (B), (R)), \
                                   (__v32hf)(__m512h)(W));

#define _mm512_maskz_max_round_ph(U, A, B, R) \
  (__m512h)__builtin_ia32_selectph_512((__mmask32)(U), \
                                   (__v32hf)_mm512_max_round_ph((A), (B), (R)), \
                                   (__v32hf)_mm512_setzero_ph());

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_add_sh(__m128h __A, __m128h __B) {
  __A[0] += __B[0];
  return __A;
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_add_sh(__m128h __W, __mmask8 __U,__m128h __A, __m128h __B) {
  __A = _mm_add_sh(__A, __B);
  return __builtin_ia32_selectsh_128(__U, __A, __W);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_add_sh(__mmask8 __U,__m128h __A, __m128h __B) {
  __A = _mm_add_sh(__A, __B);
  return __builtin_ia32_selectsh_128(__U, __A, _mm_setzero_ph());
}

#define _mm_add_round_sh(A, B, R) \
  (__m128h)__builtin_ia32_addsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)_mm_setzero_ph(), \
                                           (__mmask8)-1, (int)(R))

#define _mm_mask_add_round_sh(W, U, A, B, R) \
  (__m128h)__builtin_ia32_addsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)(__m128h)(W), \
                                           (__mmask8)(U), (int)(R))

#define _mm_maskz_add_round_sh(U, A, B, R) \
  (__m128h)__builtin_ia32_addsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)_mm_setzero_ph(), \
                                           (__mmask8)(U), (int)(R))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_sub_sh(__m128h __A, __m128h __B) {
  __A[0] -= __B[0];
  return __A;
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_sub_sh(__m128h __W, __mmask8 __U,__m128h __A, __m128h __B) {
  __A = _mm_sub_sh(__A, __B);
  return __builtin_ia32_selectsh_128(__U, __A, __W);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_sub_sh(__mmask8 __U,__m128h __A, __m128h __B) {
  __A = _mm_sub_sh(__A, __B);
  return __builtin_ia32_selectsh_128(__U, __A, _mm_setzero_ph());
}

#define _mm_sub_round_sh(A, B, R) \
  (__m128h)__builtin_ia32_subsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)_mm_setzero_ph(), \
                                           (__mmask8)-1, (int)(R))

#define _mm_mask_sub_round_sh(W, U, A, B, R) \
  (__m128h)__builtin_ia32_subsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)(__m128h)(W), \
                                           (__mmask8)(U), (int)(R))

#define _mm_maskz_sub_round_sh(U, A, B, R) \
  (__m128h)__builtin_ia32_subsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)_mm_setzero_ph(), \
                                           (__mmask8)(U), (int)(R))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mul_sh(__m128h __A, __m128h __B) {
  __A[0] *= __B[0];
  return __A;
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_mul_sh(__m128h __W, __mmask8 __U,__m128h __A, __m128h __B) {
  __A = _mm_mul_sh(__A, __B);
  return __builtin_ia32_selectsh_128(__U, __A, __W);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_mul_sh(__mmask8 __U,__m128h __A, __m128h __B) {
  __A = _mm_mul_sh(__A, __B);
  return __builtin_ia32_selectsh_128(__U, __A, _mm_setzero_ph());
}

#define _mm_mul_round_sh(A, B, R) \
  (__m128h)__builtin_ia32_mulsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)_mm_setzero_ph(), \
                                           (__mmask8)-1, (int)(R))

#define _mm_mask_mul_round_sh(W, U, A, B, R) \
  (__m128h)__builtin_ia32_mulsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)(__m128h)(W), \
                                           (__mmask8)(U), (int)(R))

#define _mm_maskz_mul_round_sh(U, A, B, R) \
  (__m128h)__builtin_ia32_mulsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)_mm_setzero_ph(), \
                                           (__mmask8)(U), (int)(R))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_div_sh(__m128h __A, __m128h __B) {
  __A[0] /= __B[0];
  return __A;
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_div_sh(__m128h __W, __mmask8 __U,__m128h __A, __m128h __B) {
  __A = _mm_div_sh(__A, __B);
  return __builtin_ia32_selectsh_128(__U, __A, __W);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_div_sh(__mmask8 __U,__m128h __A, __m128h __B) {
  __A = _mm_div_sh(__A, __B);
  return __builtin_ia32_selectsh_128(__U, __A, _mm_setzero_ph());
}

#define _mm_div_round_sh(A, B, R) \
  (__m128h)__builtin_ia32_divsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)_mm_setzero_ph(), \
                                           (__mmask8)-1, (int)(R))

#define _mm_mask_div_round_sh(W, U, A, B, R) \
  (__m128h)__builtin_ia32_divsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)(__m128h)(W), \
                                           (__mmask8)(U), (int)(R))

#define _mm_maskz_div_round_sh(U, A, B, R) \
  (__m128h)__builtin_ia32_divsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)_mm_setzero_ph(), \
                                           (__mmask8)(U), (int)(R))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_min_sh(__m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_minsh_round_mask ((__v8hf) __A,
                (__v8hf) __B,
                (__v8hf) _mm_setzero_ph(),
                (__mmask8) -1,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_min_sh(__m128h __W, __mmask8 __U,__m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_minsh_round_mask ((__v8hf) __A,
                (__v8hf) __B,
                (__v8hf) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_min_sh(__mmask8 __U,__m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_minsh_round_mask ((__v8hf) __A,
                (__v8hf) __B,
                (__v8hf)  _mm_setzero_ph (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_min_round_sh(A, B, R) \
  (__m128h)__builtin_ia32_minsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)_mm_setzero_ph(), \
                                           (__mmask8)-1, (int)(R))

#define _mm_mask_min_round_sh(W, U, A, B, R) \
  (__m128h)__builtin_ia32_minsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)(__m128h)(W), \
                                           (__mmask8)(U), (int)(R))

#define _mm_maskz_min_round_sh(U, A, B, R) \
  (__m128h)__builtin_ia32_minsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)_mm_setzero_ph(), \
                                           (__mmask8)(U), (int)(R))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_max_sh(__m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_maxsh_round_mask ((__v8hf) __A,
                (__v8hf) __B,
                (__v8hf) _mm_setzero_ph(),
                (__mmask8) -1,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_max_sh(__m128h __W, __mmask8 __U,__m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_maxsh_round_mask ((__v8hf) __A,
                (__v8hf) __B,
                (__v8hf) __W,
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_max_sh(__mmask8 __U,__m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_maxsh_round_mask ((__v8hf) __A,
                (__v8hf) __B,
                (__v8hf)  _mm_setzero_ph (),
                (__mmask8) __U,
                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_max_round_sh(A, B, R) \
  (__m128h)__builtin_ia32_maxsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)_mm_setzero_ph(), \
                                           (__mmask8)-1, (int)(R))

#define _mm_mask_max_round_sh(W, U, A, B, R) \
  (__m128h)__builtin_ia32_maxsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)(__m128h)(W), \
                                           (__mmask8)(U), (int)(R))

#define _mm_maskz_max_round_sh(U, A, B, R) \
  (__m128h)__builtin_ia32_maxsh_round_mask((__v8hf)(__m128h)(A), \
                                           (__v8hf)(__m128h)(B), \
                                           (__v8hf)_mm_setzero_ph(), \
                                           (__mmask8)(U), (int)(R))

#define _mm512_cmp_round_ph_mask(A, B, P, R) \
  (__mmask32)__builtin_ia32_cmpph512_mask((__v32hf)(__m512)(A), \
                                          (__v32hf)(__m512)(B), (int)(P), \
                                          (__mmask32)-1, (int)(R))

#define _mm512_mask_cmp_round_ph_mask(U, A, B, P, R) \
  (__mmask32)__builtin_ia32_cmpph512_mask((__v32hf)(__m512)(A), \
                                          (__v32hf)(__m512)(B), (int)(P), \
                                          (__mmask32)(U), (int)(R))

#define _mm512_cmp_ph_mask(A, B, P) \
  _mm512_cmp_round_ph_mask((A), (B), (P), _MM_FROUND_CUR_DIRECTION)

#define _mm512_mask_cmp_ph_mask(U, A, B, P) \
  _mm512_mask_cmp_round_ph_mask((U), (A), (B), (P), _MM_FROUND_CUR_DIRECTION)

#define _mm_cmp_round_sh_mask(X, Y, P, R) \
  (__mmask8)__builtin_ia32_cmpsh_mask((__v8hf)(__m128)(X), \
                                      (__v8hf)(__m128)(Y), (int)(P), \
                                      (__mmask8)-1, (int)(R))

#define _mm_mask_cmp_round_sh_mask(M, X, Y, P, R) \
  (__mmask8)__builtin_ia32_cmpsh_mask((__v8hf)(__m128)(X), \
                                      (__v8hf)(__m128)(Y), (int)(P), \
                                      (__mmask8)(M), (int)(R))

#define _mm_cmp_sh_mask(X, Y, P) \
  (__mmask8)__builtin_ia32_cmpsh_mask((__v8hf)(__m128)(X), \
                                      (__v8hf)(__m128)(Y), (int)(P), \
                                      (__mmask8)-1, \
                                      _MM_FROUND_CUR_DIRECTION)

#define _mm_mask_cmp_sh_mask(M, X, Y, P) \
  (__mmask8)__builtin_ia32_cmpsh_mask((__v8hf)(__m128)(X), \
                                      (__v8hf)(__m128)(Y), (int)(P), \
                                      (__mmask8)(M), \
                                      _MM_FROUND_CUR_DIRECTION)
// loads with vmovsh:
static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_load_sh(void const *__dp)
{
  struct __mm_load_sh_struct {
    _Float16 __u;
  } __attribute__((__packed__, __may_alias__));
  _Float16 __u = ((struct __mm_load_sh_struct*)__dp)->__u;
  return (__m128h){ __u, 0, 0, 0, 0, 0, 0, 0 };
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_load_sh (__m128h __W, __mmask8 __U, const void* __A)
{
  __m128h src = (__v8hf) __builtin_shufflevector((__v8hf) __W,
                                                 (__v8hf)_mm_setzero_ph(),
                                                 0, 8, 8, 8, 8, 8, 8, 8);

  return (__m128h) __builtin_ia32_loadsh128_mask ((__v8hf *) __A, src, __U & 1);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_load_sh (__mmask8 __U, const void* __A)
{
  return (__m128h) __builtin_ia32_loadsh128_mask ((__v8hf *) __A,
                                                  (__v8hf) _mm_setzero_ph(),
                                                  __U & 1);
}

// stores with vmovsh:
static __inline__ void __DEFAULT_FN_ATTRS128
_mm_store_sh(void *__dp, __m128h __a)
{
  struct __mm_store_sh_struct {
    _Float16 __u;
  } __attribute__((__packed__, __may_alias__));
  ((struct __mm_store_sh_struct*)__dp)->__u = __a[0];
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_mask_store_sh (void * __W, __mmask8 __U, __m128h __A)
{
  __builtin_ia32_storesh128_mask ((__v8hf *)__W, __A, __U & 1);
}

// moves with vmovsh:
static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_move_sh(__m128h __a, __m128h __b)
{
  __a[0] = __b[0];
  return __a;
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_move_sh (__m128h __W, __mmask8 __U, __m128h __A, __m128h __B)
{
  return __builtin_ia32_selectsh_128(__U, _mm_move_sh(__A, __B), __W);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_move_sh (__mmask8 __U, __m128h __A, __m128h __B)
{
  return __builtin_ia32_selectsh_128(__U, _mm_move_sh(__A, __B),
                                     _mm_setzero_ph());
}

// vmovw:
static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtsi16_si128(short __a)
{
  return (__m128i)(__v8hi){ __a, 0, 0, 0, 0, 0, 0, 0 };
}

static __inline__ short __DEFAULT_FN_ATTRS128
_mm_cvtsi128_si16(__m128i __a)
{
  __v8hi __b = (__v8hi)__a;
  return __b[0];
}

#define _mm512_fmadd_round_ph(A, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddph512_mask((__v32hf)(__m512h)(A), \
                                          (__v32hf)(__m512h)(B), \
                                          (__v32hf)(__m512h)(C), \
                                          (__mmask32)-1, (int)(R))

#define _mm512_mask_fmadd_round_ph(A, U, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddph512_mask((__v32hf)(__m512h)(A), \
                                          (__v32hf)(__m512h)(B), \
                                          (__v32hf)(__m512h)(C), \
                                          (__mmask32)(U), (int)(R))

#define _mm512_mask3_fmadd_round_ph(A, B, C, U, R) \
  (__m512h)__builtin_ia32_vfmaddph512_mask3((__v32hf)(__m512h)(A), \
                                           (__v32hf)(__m512h)(B), \
                                           (__v32hf)(__m512h)(C), \
                                           (__mmask32)(U), (int)(R))

#define _mm512_maskz_fmadd_round_ph(U, A, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddph512_maskz((__v32hf)(__m512h)(A), \
                                           (__v32hf)(__m512h)(B), \
                                           (__v32hf)(__m512h)(C), \
                                           (__mmask32)(U), (int)(R))

#define _mm512_fmsub_round_ph(A, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddph512_mask((__v32hf)(__m512h)(A), \
                                          (__v32hf)(__m512h)(B), \
                                          -(__v32hf)(__m512h)(C), \
                                          (__mmask32)-1, (int)(R))

#define _mm512_mask_fmsub_round_ph(A, U, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddph512_mask((__v32hf)(__m512h)(A), \
                                          (__v32hf)(__m512h)(B), \
                                          -(__v32hf)(__m512h)(C), \
                                          (__mmask32)(U), (int)(R))

#define _mm512_maskz_fmsub_round_ph(U, A, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddph512_maskz((__v32hf)(__m512h)(A), \
                                           (__v32hf)(__m512h)(B), \
                                           -(__v32hf)(__m512h)(C), \
                                           (__mmask32)(U), (int)(R))

#define _mm512_fnmadd_round_ph(A, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddph512_mask((__v32hf)(__m512h)(A), \
                                          -(__v32hf)(__m512h)(B), \
                                          (__v32hf)(__m512h)(C), \
                                          (__mmask32)-1, (int)(R))

#define _mm512_mask3_fnmadd_round_ph(A, B, C, U, R) \
  (__m512h)__builtin_ia32_vfmaddph512_mask3(-(__v32hf)(__m512h)(A), \
                                           (__v32hf)(__m512h)(B), \
                                           (__v32hf)(__m512h)(C), \
                                           (__mmask32)(U), (int)(R))

#define _mm512_maskz_fnmadd_round_ph(U, A, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddph512_maskz(-(__v32hf)(__m512h)(A), \
                                           (__v32hf)(__m512h)(B), \
                                           (__v32hf)(__m512h)(C), \
                                           (__mmask32)(U), (int)(R))

#define _mm512_fnmsub_round_ph(A, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddph512_mask((__v32hf)(__m512h)(A), \
                                          -(__v32hf)(__m512h)(B), \
                                          -(__v32hf)(__m512h)(C), \
                                          (__mmask32)-1, (int)(R))

#define _mm512_maskz_fnmsub_round_ph(U, A, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddph512_maskz(-(__v32hf)(__m512h)(A), \
                                           (__v32hf)(__m512h)(B), \
                                           -(__v32hf)(__m512h)(C), \
                                           (__mmask32)(U), (int)(R))

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_fmadd_ph(__m512h __A, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddph512_mask ((__v32hf) __A,
                                                   (__v32hf) __B,
                                                   (__v32hf) __C,
                                                   (__mmask32) -1,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_fmadd_ph(__m512h __A, __mmask32 __U, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddph512_mask ((__v32hf) __A,
                                                   (__v32hf) __B,
                                                   (__v32hf) __C,
                                                   (__mmask32) __U,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask3_fmadd_ph(__m512h __A, __m512h __B, __m512h __C, __mmask32 __U)
{
  return (__m512h) __builtin_ia32_vfmaddph512_mask3 ((__v32hf) __A,
                                                    (__v32hf) __B,
                                                    (__v32hf) __C,
                                                    (__mmask32) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_fmadd_ph(__mmask32 __U, __m512h __A, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddph512_maskz ((__v32hf) __A,
                                                    (__v32hf) __B,
                                                    (__v32hf) __C,
                                                    (__mmask32) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_fmsub_ph(__m512h __A, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddph512_mask ((__v32hf) __A,
                                                   (__v32hf) __B,
                                                   -(__v32hf) __C,
                                                   (__mmask32) -1,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_fmsub_ph(__m512h __A, __mmask32 __U, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddph512_mask ((__v32hf) __A,
                                                   (__v32hf) __B,
                                                   -(__v32hf) __C,
                                                   (__mmask32) __U,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_fmsub_ph(__mmask32 __U, __m512h __A, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddph512_maskz ((__v32hf) __A,
                                                    (__v32hf) __B,
                                                    -(__v32hf) __C,
                                                    (__mmask32) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_fnmadd_ph(__m512h __A, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddph512_mask ((__v32hf) __A,
                                                   -(__v32hf) __B,
                                                   (__v32hf) __C,
                                                   (__mmask32) -1,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask3_fnmadd_ph(__m512h __A, __m512h __B, __m512h __C, __mmask32 __U)
{
  return (__m512h) __builtin_ia32_vfmaddph512_mask3 (-(__v32hf) __A,
                                                    (__v32hf) __B,
                                                    (__v32hf) __C,
                                                    (__mmask32) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_fnmadd_ph(__mmask32 __U, __m512h __A, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddph512_maskz (-(__v32hf) __A,
                                                    (__v32hf) __B,
                                                    (__v32hf) __C,
                                                    (__mmask32) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_fnmsub_ph(__m512h __A, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddph512_mask ((__v32hf) __A,
                                                   -(__v32hf) __B,
                                                   -(__v32hf) __C,
                                                   (__mmask32) -1,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_fnmsub_ph(__mmask32 __U, __m512h __A, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddph512_maskz (-(__v32hf) __A,
                                                    (__v32hf) __B,
                                                    -(__v32hf) __C,
                                                    (__mmask32) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_fmaddsub_round_ph(A, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddsubph512_mask((__v32hf)(__m512h)(A), \
                                             (__v32hf)(__m512h)(B), \
                                             (__v32hf)(__m512h)(C), \
                                             (__mmask32)-1, (int)(R))

#define _mm512_mask_fmaddsub_round_ph(A, U, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddsubph512_mask((__v32hf)(__m512h)(A), \
                                             (__v32hf)(__m512h)(B), \
                                             (__v32hf)(__m512h)(C), \
                                             (__mmask32)(U), (int)(R))


#define _mm512_mask3_fmaddsub_round_ph(A, B, C, U, R) \
  (__m512h)__builtin_ia32_vfmaddsubph512_mask3((__v32hf)(__m512h)(A), \
                                              (__v32hf)(__m512h)(B), \
                                              (__v32hf)(__m512h)(C), \
                                              (__mmask32)(U), (int)(R))

#define _mm512_maskz_fmaddsub_round_ph(U, A, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddsubph512_maskz((__v32hf)(__m512h)(A), \
                                              (__v32hf)(__m512h)(B), \
                                              (__v32hf)(__m512h)(C), \
                                              (__mmask32)(U), (int)(R))

#define _mm512_fmsubadd_round_ph(A, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddsubph512_mask((__v32hf)(__m512h)(A), \
                                             (__v32hf)(__m512h)(B), \
                                             -(__v32hf)(__m512h)(C), \
                                             (__mmask32)-1, (int)(R))

#define _mm512_mask_fmsubadd_round_ph(A, U, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddsubph512_mask((__v32hf)(__m512h)(A), \
                                             (__v32hf)(__m512h)(B), \
                                             -(__v32hf)(__m512h)(C), \
                                             (__mmask32)(U), (int)(R))


#define _mm512_maskz_fmsubadd_round_ph(U, A, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddsubph512_maskz((__v32hf)(__m512h)(A), \
                                              (__v32hf)(__m512h)(B), \
                                              -(__v32hf)(__m512h)(C), \
                                              (__mmask32)(U), (int)(R))

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_fmaddsub_ph(__m512h __A, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddsubph512_mask ((__v32hf) __A,
                                                      (__v32hf) __B,
                                                      (__v32hf) __C,
                                                      (__mmask32) -1,
                                                      _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_fmaddsub_ph(__m512h __A, __mmask32 __U, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddsubph512_mask ((__v32hf) __A,
                                                      (__v32hf) __B,
                                                      (__v32hf) __C,
                                                      (__mmask32) __U,
                                                      _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask3_fmaddsub_ph(__m512h __A, __m512h __B, __m512h __C, __mmask32 __U)
{
  return (__m512h) __builtin_ia32_vfmaddsubph512_mask3 ((__v32hf) __A,
                                                       (__v32hf) __B,
                                                       (__v32hf) __C,
                                                       (__mmask32) __U,
                                                       _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_fmaddsub_ph(__mmask32 __U, __m512h __A, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddsubph512_maskz ((__v32hf) __A,
                                                       (__v32hf) __B,
                                                       (__v32hf) __C,
                                                       (__mmask32) __U,
                                                       _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_fmsubadd_ph(__m512h __A, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddsubph512_mask ((__v32hf) __A,
                                                      (__v32hf) __B,
                                                      -(__v32hf) __C,
                                                      (__mmask32) -1,
                                                      _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_fmsubadd_ph(__m512h __A, __mmask32 __U, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddsubph512_mask ((__v32hf) __A,
                                                      (__v32hf) __B,
                                                      -(__v32hf) __C,
                                                      (__mmask32) __U,
                                                      _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_fmsubadd_ph(__mmask32 __U, __m512h __A, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddsubph512_maskz ((__v32hf) __A,
                                                       (__v32hf) __B,
                                                       -(__v32hf) __C,
                                                       (__mmask32) __U,
                                                       _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mask3_fmsub_round_ph(A, B, C, U, R) \
  (__m512h)__builtin_ia32_vfmsubph512_mask3((__v32hf)(__m512h)(A), \
                                           (__v32hf)(__m512h)(B), \
                                           (__v32hf)(__m512h)(C), \
                                           (__mmask32)(U), (int)(R))

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask3_fmsub_ph(__m512h __A, __m512h __B, __m512h __C, __mmask32 __U)
{
  return (__m512h)__builtin_ia32_vfmsubph512_mask3 ((__v32hf) __A,
                                                   (__v32hf) __B,
                                                   (__v32hf) __C,
                                                   (__mmask32) __U,
                                                   _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mask3_fmsubadd_round_ph(A, B, C, U, R) \
  (__m512h)__builtin_ia32_vfmsubaddph512_mask3((__v32hf)(__m512h)(A), \
                                              (__v32hf)(__m512h)(B), \
                                              (__v32hf)(__m512h)(C), \
                                              (__mmask32)(U), (int)(R))

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask3_fmsubadd_ph(__m512h __A, __m512h __B, __m512h __C, __mmask32 __U)
{
  return (__m512h)__builtin_ia32_vfmsubaddph512_mask3 ((__v32hf) __A,
                                                      (__v32hf) __B,
                                                      (__v32hf) __C,
                                                      (__mmask32) __U,
                                                      _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mask_fnmadd_round_ph(A, U, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddph512_mask((__v32hf)(__m512h)(A), \
                                          -(__v32hf)(__m512h)(B), \
                                          (__v32hf)(__m512h)(C), \
                                          (__mmask32)(U), (int)(R))

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_fnmadd_ph(__m512h __A, __mmask32 __U, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddph512_mask ((__v32hf) __A,
                                                   -(__v32hf) __B,
                                                   (__v32hf) __C,
                                                   (__mmask32) __U,
                                                   _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mask_fnmsub_round_ph(A, U, B, C, R) \
  (__m512h)__builtin_ia32_vfmaddph512_mask((__v32hf)(__m512h)(A), \
                                          -(__v32hf)(__m512h)(B), \
                                          -(__v32hf)(__m512h)(C), \
                                          (__mmask32)(U), (int)(R))


#define _mm512_mask3_fnmsub_round_ph(A, B, C, U, R) \
  (__m512h)__builtin_ia32_vfmsubph512_mask3(-(__v32hf)(__m512h)(A), \
                                           (__v32hf)(__m512h)(B), \
                                           (__v32hf)(__m512h)(C), \
                                           (__mmask32)(U), (int)(R))

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_fnmsub_ph(__m512h __A, __mmask32 __U, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddph512_mask ((__v32hf) __A,
                                                   -(__v32hf) __B,
                                                   -(__v32hf) __C,
                                                   (__mmask32) __U,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask3_fnmsub_ph(__m512h __A, __m512h __B, __m512h __C, __mmask32 __U)
{
  return (__m512h) __builtin_ia32_vfmsubph512_mask3 (-(__v32hf) __A,
                                                    (__v32hf) __B,
                                                    (__v32hf) __C,
                                                    (__mmask32) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_fmadd_sh (__m128h __W, __m128h __A, __m128h __B)
{
  return __builtin_ia32_vfmaddsh3_mask((__v8hf)__W,
                                       (__v8hf)__A,
                                       (__v8hf)__B,
                                       (__mmask8)-1,
                                       _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_fmadd_sh (__m128h __W, __mmask8 __U, __m128h __A, __m128h __B)
{
  return __builtin_ia32_vfmaddsh3_mask((__v8hf)__W,
                                       (__v8hf)__A,
                                       (__v8hf)__B,
                                       (__mmask8)__U,
                                       _MM_FROUND_CUR_DIRECTION);
}

#define _mm_fmadd_round_sh(A, B, C, R) \
  (__m128h)__builtin_ia32_vfmaddsh3_mask((__v8hf)(__m128h)(A), \
                                        (__v8hf)(__m128h)(B), \
                                        (__v8hf)(__m128h)(C), (__mmask8)-1, \
                                        (int)(R))

#define _mm_mask_fmadd_round_sh(W, U, A, B, R) \
  (__m128h)__builtin_ia32_vfmaddsh3_mask((__v8hf)(__m128h)(W), \
                                        (__v8hf)(__m128h)(A), \
                                        (__v8hf)(__m128h)(B), (__mmask8)(U), \
                                        (int)(R))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_fmadd_sh (__mmask8 __U, __m128h __A, __m128h __B, __m128h __C)
{
  return __builtin_ia32_vfmaddsh3_maskz((__v8hf)__A,
                                        (__v8hf)__B,
                                        (__v8hf)__C,
                                        (__mmask8)__U,
                                        _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_fmadd_round_sh(U, A, B, C, R) \
  (__m128h)__builtin_ia32_vfmaddsh3_maskz((__v8hf)(__m128h)(A), \
                                         (__v8hf)(__m128h)(B), \
                                         (__v8hf)(__m128h)(C), (__mmask8)(U), \
                                         (int)(R))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask3_fmadd_sh (__m128h __W, __m128h __X, __m128h __Y, __mmask8 __U)
{
  return __builtin_ia32_vfmaddsh3_mask3((__v8hf)__W,
                                        (__v8hf)__X,
                                        (__v8hf)__Y,
                                        (__mmask8)__U,
                                        _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask3_fmadd_round_sh(W, X, Y, U, R) \
  (__m128h)__builtin_ia32_vfmaddsh3_mask3((__v8hf)(__m128h)(W), \
                                         (__v8hf)(__m128h)(X), \
                                         (__v8hf)(__m128h)(Y), (__mmask8)(U), \
                                         (int)(R))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_fmsub_sh (__m128h __W, __m128h __A, __m128h __B)
{
  return (__m128h)__builtin_ia32_vfmaddsh3_mask((__v8hf)__W,
           (__v8hf)__A,
           -(__v8hf)__B,
           (__mmask8) -1,
           _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_fmsub_sh (__m128h __W, __mmask8 __U, __m128h __A, __m128h __B)
{
 return (__m128h) __builtin_ia32_vfmaddsh3_mask ((__v8hf) __W,
          (__v8hf) __A,
          -(__v8hf) __B,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_fmsub_round_sh(A, B, C, R) \
  (__m128h)__builtin_ia32_vfmaddsh3_mask((__v8hf)(__m128h)(A), \
                                        (__v8hf)(__m128h)(B), \
                                        -(__v8hf)(__m128h)(C), (__mmask8)-1, \
                                        (int)(R));

#define _mm_mask_fmsub_round_sh(W, U, A, B, R) \
  (__m128h)__builtin_ia32_vfmaddsh3_mask((__v8hf)(__m128h)(W), \
                                        (__v8hf)(__m128h)(A), \
                                        -(__v8hf)(__m128h)(B), (__mmask8)(U), \
                                        (int)(R));

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_fmsub_sh (__mmask8 __U, __m128h __A, __m128h __B, __m128h __C)
{
 return (__m128h) __builtin_ia32_vfmaddsh3_maskz ((__v8hf) __A,
          (__v8hf) __B,
          -(__v8hf) __C,
          (__mmask8) __U,
          _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_fmsub_round_sh(U, A, B, C, R) \
  (__m128h)__builtin_ia32_vfmaddsh3_maskz((__v8hf)(__m128h)(A), \
                                         (__v8hf)(__m128h)(B), \
                                         -(__v8hf)(__m128h)(C), (__mmask8)(U), \
                                         (int)R);

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask3_fmsub_sh (__m128h __W, __m128h __X, __m128h __Y, __mmask8 __U)
{
  return __builtin_ia32_vfmsubsh3_mask3((__v8hf)__W,
                                        (__v8hf)__X,
                                        (__v8hf)__Y,
                                        (__mmask8)__U,
                                        _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask3_fmsub_round_sh(W, X, Y, U, R) \
  (__m128h)__builtin_ia32_vfmsubsh3_mask3((__v8hf)(__m128h)(W), \
                                         (__v8hf)(__m128h)(X), \
                                         (__v8hf)(__m128h)(Y), (__mmask8)(U), \
                                         (int)(R))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_fnmadd_sh (__m128h __W, __m128h __A, __m128h __B)
{
  return __builtin_ia32_vfmaddsh3_mask((__v8hf)__W,
                                       -(__v8hf)__A,
                                       (__v8hf)__B,
                                       (__mmask8)-1,
                                       _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_fnmadd_sh (__m128h __W, __mmask8 __U, __m128h __A, __m128h __B)
{
  return __builtin_ia32_vfmaddsh3_mask((__v8hf)__W,
                                       -(__v8hf)__A,
                                       (__v8hf)__B,
                                       (__mmask8)__U,
                                       _MM_FROUND_CUR_DIRECTION);
}

#define _mm_fnmadd_round_sh(A, B, C, R) \
  (__m128h)__builtin_ia32_vfmaddsh3_mask((__v8hf)(__m128h)(A), \
                                        -(__v8hf)(__m128h)(B), \
                                        (__v8hf)(__m128h)(C), (__mmask8)-1, \
                                        (int)(R))

#define _mm_mask_fnmadd_round_sh(W, U, A, B, R) \
  (__m128h)__builtin_ia32_vfmaddsh3_mask((__v8hf)(__m128h)(W), \
                                        -(__v8hf)(__m128h)(A), \
                                        (__v8hf)(__m128h)(B), (__mmask8)(U), \
                                        (int)(R))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_fnmadd_sh (__mmask8 __U, __m128h __A, __m128h __B, __m128h __C)
{
  return __builtin_ia32_vfmaddsh3_maskz((__v8hf)__A,
                                        -(__v8hf)__B,
                                        (__v8hf)__C,
                                        (__mmask8)__U,
                                        _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_fnmadd_round_sh(U, A, B, C, R) \
  (__m128h)__builtin_ia32_vfmaddsh3_maskz((__v8hf)(__m128h)(A), \
                                         -(__v8hf)(__m128h)(B), \
                                         (__v8hf)(__m128h)(C), (__mmask8)(U), \
                                         (int)(R))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask3_fnmadd_sh (__m128h __W, __m128h __X, __m128h __Y, __mmask8 __U)
{
  return __builtin_ia32_vfmaddsh3_mask3((__v8hf)__W,
                                        -(__v8hf)__X,
                                        (__v8hf)__Y,
                                        (__mmask8)__U,
                                        _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask3_fnmadd_round_sh(W, X, Y, U, R) \
  (__m128h)__builtin_ia32_vfmaddsh3_mask3((__v8hf)(__m128h)(W), \
                                         -(__v8hf)(__m128h)(X), \
                                         (__v8hf)(__m128h)(Y), (__mmask8)(U), \
                                         (int)(R))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_fnmsub_sh (__m128h __W, __m128h __A, __m128h __B)
{
  return __builtin_ia32_vfmaddsh3_mask((__v8hf)__W,
                                       -(__v8hf)__A,
                                       -(__v8hf)__B,
                                       (__mmask8)-1,
                                       _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_fnmsub_sh (__m128h __W, __mmask8 __U, __m128h __A, __m128h __B)
{
  return __builtin_ia32_vfmaddsh3_mask((__v8hf)__W,
                                       -(__v8hf)__A,
                                       -(__v8hf)__B,
                                       (__mmask8)__U,
                                       _MM_FROUND_CUR_DIRECTION);
}

#define _mm_fnmsub_round_sh(A, B, C, R) \
  (__m128h)__builtin_ia32_vfmaddsh3_mask((__v8hf)(__m128h)(A), \
                                        -(__v8hf)(__m128h)(B), \
                                        -(__v8hf)(__m128h)(C), (__mmask8)-1, \
                                        (int)(R))

#define _mm_mask_fnmsub_round_sh(W, U, A, B, R) \
  (__m128h)__builtin_ia32_vfmaddsh3_mask((__v8hf)(__m128h)(W), \
                                        -(__v8hf)(__m128h)(A), \
                                        -(__v8hf)(__m128h)(B), (__mmask8)(U), \
                                        (int)(R))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_fnmsub_sh (__mmask8 __U, __m128h __A, __m128h __B, __m128h __C)
{
  return __builtin_ia32_vfmaddsh3_maskz((__v8hf)__A,
                                        -(__v8hf)__B,
                                        -(__v8hf)__C,
                                        (__mmask8)__U,
                                        _MM_FROUND_CUR_DIRECTION);
}

#define _mm_maskz_fnmsub_round_sh(U, A, B, C, R) \
  (__m128h)__builtin_ia32_vfmaddsh3_maskz((__v8hf)(__m128h)(A), \
                                         -(__v8hf)(__m128h)(B), \
                                         -(__v8hf)(__m128h)(C), (__mmask8)(U), \
                                         (int)(R))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask3_fnmsub_sh (__m128h __W, __m128h __X, __m128h __Y, __mmask8 __U)
{
  return __builtin_ia32_vfmsubsh3_mask3((__v8hf)__W,
                                        -(__v8hf)__X,
                                        (__v8hf)__Y,
                                        (__mmask8)__U,
                                        _MM_FROUND_CUR_DIRECTION);
}

#define _mm_mask3_fnmsub_round_sh(W, X, Y, U, R) \
  (__m128h)__builtin_ia32_vfmsubsh3_mask3((__v8hf)(__m128h)(W), \
                                         -(__v8hf)(__m128h)(X), \
                                         (__v8hf)(__m128h)(Y), (__mmask8)(U), \
                                         (int)(R))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_fcmadd_sch(__m128h __A, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_vfcmaddcsh_mask ((__v4sf) __A,
                                                  (__v4sf) __B,
                                                  (__v4sf) __C,
                                                  (__mmask8) -1,
                                                  _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_fcmadd_sch(__m128h __A, __mmask8 __U, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_vfcmaddcsh_mask ((__v4sf) __A,
                                                  (__v4sf) __B,
                                                  (__v4sf) __C,
                                                  (__mmask8) __U,
                                                  _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_fcmadd_sch(__mmask8 __U, __m128h __A, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_vfcmaddcsh_maskz ((__v4sf) __A,
                                                  (__v4sf) __B,
                                                  (__v4sf) __C,
                                                  (__mmask8) __U,
                                                  _MM_FROUND_CUR_DIRECTION);
}

#define _mm_fcmadd_round_sch(A, B, C, R) \
  (__m128h) __builtin_ia32_vfcmaddcsh_mask((__v4sf)(__m128h)(A), \
                                          (__v4sf)(__m128h)(B), \
                                          (__v4sf)(__m128h)(C), \
                                          (__mmask8) -1, \
                                          (int)(R))


#define _mm_mask_fcmadd_round_sch(A, U, B, C, R) \
  (__m128h) __builtin_ia32_vfcmaddcsh_mask((__v4sf)(__m128h)(A), \
                                          (__v4sf)(__m128h)(B), \
                                          (__v4sf)(__m128h)(C), \
                                          (__mmask8)(U), \
                                          (int)(R))


#define _mm_maskz_fcmadd_round_sch(U, A, B, C, R) \
  (__m128h) __builtin_ia32_vfcmaddcsh_maskz ((__v4sf)(__m128h)(A), \
                                           (__v4sf)(__m128h)(B), \
                                           (__v4sf) (__m128h)(C), \
                                           (__mmask8)(U), \
                                           (int)(R))


static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_fmadd_sch(__m128h __A, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_vfmaddcsh_mask ((__v4sf) __A,
                                                 (__v4sf) __B,
                                                 (__v4sf) __C,
                                                 (__mmask8) -1,
                                                 _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_fmadd_sch(__m128h __A, __mmask8 __U, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_vfmaddcsh_mask ((__v4sf) __A,
                                                 (__v4sf) __B,
                                                 (__v4sf) __C,
                                                 (__mmask8) __U,
                                                 _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_fmadd_sch(__mmask8 __U, __m128h __A, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_vfmaddcsh_maskz ((__v4sf) __A,
                                                 (__v4sf) __B,
                                                 (__v4sf) __C,
                                                 (__mmask8) __U,
                                                 _MM_FROUND_CUR_DIRECTION);
}

#define _mm_fmadd_round_sch(A, B, C, R) \
  (__m128h) __builtin_ia32_vfmaddcsh_mask ((__v4sf)(__m128h) (A), \
                                          (__v4sf)(__m128h) (B), \
                                          (__v4sf)(__m128h) (C), \
                                          (__mmask8) -1, \
                                          (int)(R))

#define _mm_mask_fmadd_round_sch(A, U, B, C, R) \
  (__m128h) __builtin_ia32_vfmaddcsh_mask ((__v4sf)(__m128h) (A), \
                                          (__v4sf)(__m128h) (B), \
                                          (__v4sf)(__m128h) (C), \
                                          (__mmask8) U, \
                                          (int)(R))

#define _mm_maskz_fmadd_round_sch(U, A, B, C, R) \
  (__m128h) __builtin_ia32_vfmaddcsh_maskz ((__v4sf)(__m128h) (A), \
                                           (__v4sf)(__m128h) (B), \
                                           (__v4sf) (__m128h) (C), \
                                           (__mmask8) (U), \
                                           (int)(R))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_fcmul_sch(__m128h __A, __m128h __B)
{
  return (__m128h) __builtin_ia32_vfcmulcsh_mask ((__v4sf) __A,
                                                 (__v4sf) __B,
                                                 (__v4sf) _mm_undefined_ph (),
                                                 (__mmask8) -1,
                                                 _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_fcmul_sch(__m128h __W, __mmask8 __U, __m128h __A, __m128h __B)
{
  return (__m128h) __builtin_ia32_vfcmulcsh_mask ((__v4sf) __A,
                                                 (__v4sf) __B,
                                                 (__v4sf) __W,
                                                 (__mmask8) __U,
                                                 _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_fcmul_sch(__mmask8 __U, __m128h __A, __m128h __B)
{
  return (__m128h) __builtin_ia32_vfcmulcsh_mask ((__v4sf) __A,
                                                 (__v4sf) __B,
                                                 (__v4sf) _mm_setzero_ph (),
                                                 (__mmask8) __U,
                                                 _MM_FROUND_CUR_DIRECTION);
}

#define _mm_fcmul_round_sch(A, B, R) \
  (__m128h) __builtin_ia32_vfcmulcsh_mask ((__v4sf)(__m128h) (A), \
                                          (__v4sf)(__m128h) (B), \
                                          (__v4sf)(__m128h) _mm_undefined_ph(), \
                                          (__mmask8) -1, \
                                          (int)(R))

#define _mm_mask_fcmul_round_sch(W, U, A, B, R) \
  (__m128h) __builtin_ia32_vfcmulcsh_mask ((__v4sf)(__m128h) (A), \
                                          (__v4sf)(__m128h) (B), \
                                          (__v4sf)(__m128h) (W), \
                                          (__mmask8) (U), \
                                          (int)(R))

#define _mm_maskz_fcmul_round_sch(U, A, B, R) \
  (__m128h) __builtin_ia32_vfcmulcsh_mask ((__v4sf)(__m128h) (A), \
                                          (__v4sf)(__m128h) (B), \
                                          (__v4sf)(__m128h) _mm_setzero_ph (), \
                                          (__mmask8) (U), \
                                          (int)(R))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_fmul_sch(__m128h __A, __m128h __B)
{
  return (__m128h) __builtin_ia32_vfmulcsh_mask ((__v4sf) __A,
                                                (__v4sf) __B,
                                                (__v4sf) _mm_undefined_ph (),
                                                (__mmask8) -1,
                                                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_fmul_sch(__m128h __W, __mmask8 __U, __m128h __A, __m128h __B)
{
  return (__m128h) __builtin_ia32_vfmulcsh_mask ((__v4sf) __A,
                                                (__v4sf) __B,
                                                (__v4sf) __W,
                                                (__mmask8) __U,
                                                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_fmul_sch(__mmask8 __U, __m128h __A, __m128h __B)
{
  return (__m128h) __builtin_ia32_vfmulcsh_mask ((__v4sf) __A,
                                                (__v4sf) __B,
                                                (__v4sf) _mm_setzero_ph (),
                                                (__mmask8) __U,
                                                _MM_FROUND_CUR_DIRECTION);
}

#define _mm_fmul_round_sch(A, B, R) \
  (__m128h) __builtin_ia32_vfmulcsh_mask ((__v4sf)(__m128h) (A), \
                                         (__v4sf)(__m128h) (B), \
                                         (__v4sf)(__m128h) _mm_undefined_ph(), \
                                         (__mmask8) -1, \
                                         (int)(R))

#define _mm_mask_fmul_round_sch(W, U, A, B, R) \
  (__m128h) __builtin_ia32_vfmulcsh_mask ((__v4sf)(__m128h) (A), \
                                         (__v4sf)(__m128h) (B), \
                                         (__v4sf)(__m128h) (W), \
                                         (__mmask8) (U), \
                                         (int)(R))

#define _mm_maskz_fmul_round_sch(U, A, B, R) \
  (__m128h) __builtin_ia32_vfmulcsh_mask ((__v4sf)(__m128h) (A), \
                                         (__v4sf)(__m128h) (B), \
                                         (__v4sf)(__m128h) _mm_setzero_ph (), \
                                         (__mmask8) (U), \
                                         (int)(R))

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_fcmul_pch(__m512h __A, __m512h __B)
{
  return (__m512h) __builtin_ia32_vfcmulcph512_mask ((__v16sf) __A,
                                                    (__v16sf) __B,
                                                    (__v16sf) _mm512_undefined_ph (),
                                                    (__mmask16) -1,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_fcmul_pch(__m512h __W, __mmask16 __U, __m512h __A, __m512h __B)
{
  return (__m512h) __builtin_ia32_vfcmulcph512_mask ((__v16sf) __A,
                                                    (__v16sf) __B,
                                                    (__v16sf) __W,
                                                    (__mmask16) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_fcmul_pch(__mmask16 __U, __m512h __A, __m512h __B)
{
  return (__m512h) __builtin_ia32_vfcmulcph512_mask ((__v16sf) __A,
                                                    (__v16sf) __B,
                                                    (__v16sf) _mm512_setzero_ph (),
                                                    (__mmask16) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_fcmul_round_pch(A, B, R) \
  (__m512h) __builtin_ia32_vfcmulcph512_mask ((__v16sf)(__m512h) (A), \
                                             (__v16sf)(__m512h) (B), \
                                             (__v16sf)(__m512h) _mm512_undefined_ph(), \
                                             (__mmask16) -1, \
                                             (int)(R))

#define _mm512_mask_fcmul_round_pch(W, U, A, B, R) \
  (__m512h) __builtin_ia32_vfcmulcph512_mask ((__v16sf)(__m512h) (A), \
                                             (__v16sf)(__m512h) (B), \
                                             (__v16sf)(__m512h) (W), \
                                             (__mmask16) (U), \
                                             (int)(R))

#define _mm512_maskz_fcmul_round_pch(U, A, B, R) \
  (__m512h) __builtin_ia32_vfcmulcph512_mask ((__v16sf)(__m512h) (A), \
                                             (__v16sf)(__m512h) (B), \
                                             (__v16sf)(__m512h) _mm512_setzero_ph (), \
                                             (__mmask16) (U), \
                                             (int)(R))

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_fmul_pch(__m512h __A, __m512h __B)
{
  return (__m512h) __builtin_ia32_vfmulcph512_mask ((__v16sf) __A,
                                                   (__v16sf) __B,
                                                   (__v16sf) _mm512_undefined_ph (),
                                                   (__mmask16) -1,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_fmul_pch(__m512h __W, __mmask16 __U, __m512h __A, __m512h __B)
{
  return (__m512h) __builtin_ia32_vfmulcph512_mask ((__v16sf) __A,
                                                   (__v16sf) __B,
                                                   (__v16sf) __W,
                                                   (__mmask16) __U,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_fmul_pch(__mmask16 __U, __m512h __A, __m512h __B)
{
  return (__m512h) __builtin_ia32_vfmulcph512_mask ((__v16sf) __A,
                                                   (__v16sf) __B,
                                                   (__v16sf) _mm512_setzero_ph (),
                                                   (__mmask16) __U,
                                                   _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_fmul_round_pch(A, B, R) \
  (__m512h) __builtin_ia32_vfmulcph512_mask ((__v16sf)(__m512h) (A), \
                                             (__v16sf)(__m512h) (B), \
                                             (__v16sf)(__m512h) _mm512_undefined_ph(), \
                                             (__mmask16) -1, \
                                             (int)(R))

#define _mm512_mask_fmul_round_pch(W, U, A, B, R)  \
  (__m512h) __builtin_ia32_vfmulcph512_mask ((__v16sf)(__m512h) (A), \
                                             (__v16sf)(__m512h) (B), \
                                             (__v16sf)(__m512h) (W), \
                                             (__mmask16) (U), \
                                             (int)(R))

#define _mm512_maskz_fmul_round_pch(U, A, B, R)  \
  (__m512h) __builtin_ia32_vfmulcph512_mask ((__v16sf)(__m512h) (A), \
                                            (__v16sf)(__m512h) (B), \
                                            (__v16sf)(__m512h) _mm512_setzero_ph(), \
                                            (__mmask16) (U), \
                                            (int)(R))

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_fcmadd_pch(__m512h __A, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfcmaddcph512_mask ((__v16sf) __A,
                                                    (__v16sf) __B,
                                                    (__v16sf) __C,
                                                    (__mmask16) -1,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_fcmadd_pch(__m512h __A, __mmask16 __U, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfcmaddcph512_mask ((__v16sf) __A,
                                                    (__v16sf) __B,
                                                    (__v16sf) __C,
                                                    (__mmask16) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_fcmadd_pch(__mmask16 __U, __m512h __A, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfcmaddcph512_maskz ((__v16sf) __A,
                                                    (__v16sf) __B,
                                                    (__v16sf) __C,
                                                    (__mmask16) __U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_fcmadd_round_pch(A, B, C, R)  \
  (__m512h) __builtin_ia32_vfcmaddcph512_mask ((__v16sf)(__m512h) (A), \
                                             (__v16sf)(__m512h) (B), \
                                             (__v16sf)(__m512h) (C), \
                                             (__mmask16) -1, \
                                             (int)(R))

#define _mm512_mask_fcmadd_round_pch(A, U, B, C, R)  \
  (__m512h) __builtin_ia32_vfcmaddcph512_mask ((__v16sf)(__m512h) (A), \
                                             (__v16sf)(__m512h) (B), \
                                             (__v16sf)(__m512h) (C), \
                                             (__mmask16) (U), \
                                             (int)(R))

#define _mm512_maskz_fcmadd_round_pch(U, A, B, C, R)  \
  (__m512h) __builtin_ia32_vfcmaddcph512_maskz ((__v16sf)(__m512h) (A), \
                                             (__v16sf)(__m512h) (B), \
                                             (__v16sf)(__m512h) (C), \
                                             (__mmask16) (U), \
                                             (int)(R))

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_fmadd_pch(__m512h __A, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddcph512_mask ((__v16sf) __A,
                                                   (__v16sf) __B,
                                                   (__v16sf) __C,
                                                   (__mmask16) -1,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_fmadd_pch(__m512h __A, __mmask16 __U, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddcph512_mask ((__v16sf) __A,
                                                   (__v16sf) __B,
                                                   (__v16sf) __C,
                                                   (__mmask16) __U,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_fmadd_pch(__mmask16 __U, __m512h __A, __m512h __B, __m512h __C)
{
  return (__m512h) __builtin_ia32_vfmaddcph512_maskz ((__v16sf) __A,
                                                   (__v16sf) __B,
                                                   (__v16sf) __C,
                                                   (__mmask16) __U,
                                                   _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_fmadd_round_pch(A, B, C, R)  \
  (__m512h) __builtin_ia32_vfmaddcph512_mask ((__v16sf)(__m512h) (A), \
                                             (__v16sf)(__m512h) (B), \
                                             (__v16sf)(__m512h) (C), \
                                             (__mmask16) -1, \
                                             (int)(R))

#define _mm512_mask_fmadd_round_pch(A, U, B, C, R)  \
  (__m512h) __builtin_ia32_vfmaddcph512_mask ((__v16sf)(__m512h) (A), \
                                             (__v16sf)(__m512h) (B), \
                                             (__v16sf)(__m512h) (C), \
                                             (__mmask16) (U), \
                                             (int)(R))

#define _mm512_maskz_fmadd_round_pch(U, A, B, C, R)  \
  (__m512h) __builtin_ia32_vfmaddcph512_maskz ((__v16sf)(__m512h) (A), \
                                            (__v16sf)(__m512h) (B), \
                                            (__v16sf)(__m512h) (C), \
                                            (__mmask16) (U), \
                                            (int)(R))

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256
#undef __DEFAULT_FN_ATTRS512

#endif
/* end INTEL_FEATURE_ISA_FP16 */
