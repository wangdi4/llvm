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
#error "Never use <avx512vlfp16intrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512VLFP16INTRIN_H
#define __AVX512VLFP16INTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS256 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512fp16, avx512vl"), __min_vector_width__(256)))
#define __DEFAULT_FN_ATTRS128 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512fp16, avx512vl"), __min_vector_width__(128)))

static __inline__ _Float16 __DEFAULT_FN_ATTRS128
_mm_cvtsh_h(__m128h __a)
{
  return __a[0];
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_set_sh(_Float16 __h)
{
  return __extension__ (__m128h){ __h, 0, 0, 0, 0, 0, 0, 0 };
}

static __inline __m128h __DEFAULT_FN_ATTRS256
_mm_set1_ph(_Float16 __h)
{
  return (__m128h)(__v8hf){__h, __h, __h, __h, __h, __h, __h, __h};
}

static __inline __m256h __DEFAULT_FN_ATTRS256
_mm256_set1_ph(_Float16 __h)
{
  return (__m256h)(__v16hf){ __h, __h, __h, __h, __h, __h, __h, __h,
                             __h, __h, __h, __h, __h, __h, __h, __h};
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_add_ph(__m256h __A, __m256h __B) {
  return (__m256h)((__v16hf)__A + (__v16hf)__B);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_add_ph(__m256h __W, __mmask16 __U, __m256h __A, __m256h __B) {
  return (__m256h) __builtin_ia32_selectph_256 (__U,
            (__v16hf) _mm256_add_ph(__A, __B),
            (__v16hf) __W);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_add_ph(__mmask16 __U, __m256h __A, __m256h __B) {
  return (__m256h) __builtin_ia32_selectph_256 (__U,
            (__v16hf) _mm256_add_ph(__A, __B),
            (__v16hf) _mm256_setzero_ph ());
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_add_ph(__m128h __A, __m128h __B) {
  return (__m128h)((__v8hf)__A + (__v8hf)__B);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_add_ph(__m128h __W, __mmask8 __U, __m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_selectph_128 (__U,
            (__v8hf) _mm_add_ph(__A, __B),
            (__v8hf) __W);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_add_ph(__mmask8 __U, __m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_selectph_128 (__U,
            (__v8hf) _mm_add_ph(__A, __B),
            (__v8hf) _mm_setzero_ph ());
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_sub_ph(__m256h __A, __m256h __B) {
  return (__m256h)((__v16hf)__A - (__v16hf)__B);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_sub_ph(__m256h __W, __mmask16 __U, __m256h __A, __m256h __B) {
  return (__m256h) __builtin_ia32_selectph_256 (__U,
            (__v16hf) _mm256_sub_ph(__A, __B),
            (__v16hf) __W);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_sub_ph(__mmask16 __U, __m256h __A, __m256h __B) {
  return (__m256h) __builtin_ia32_selectph_256 (__U,
            (__v16hf) _mm256_sub_ph(__A, __B),
            (__v16hf) _mm256_setzero_ph ());
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_sub_ph(__m128h __A, __m128h __B) {
  return (__m128h)((__v8hf)__A - (__v8hf)__B);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_sub_ph(__m128h __W, __mmask8 __U, __m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_selectph_128 (__U,
            (__v8hf) _mm_sub_ph(__A, __B),
            (__v8hf) __W);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_sub_ph(__mmask8 __U, __m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_selectph_128 (__U,
            (__v8hf) _mm_sub_ph(__A, __B),
            (__v8hf) _mm_setzero_ph ());
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mul_ph(__m256h __A, __m256h __B) {
  return (__m256h)((__v16hf)__A * (__v16hf)__B);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_mul_ph(__m256h __W, __mmask16 __U, __m256h __A, __m256h __B) {
  return (__m256h) __builtin_ia32_selectph_256 (__U,
            (__v16hf) _mm256_mul_ph(__A, __B),
            (__v16hf) __W);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_mul_ph(__mmask16 __U, __m256h __A, __m256h __B) {
  return (__m256h) __builtin_ia32_selectph_256 (__U,
            (__v16hf) _mm256_mul_ph(__A, __B),
            (__v16hf) _mm256_setzero_ph ());
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mul_ph(__m128h __A, __m128h __B) {
  return (__m128h)((__v8hf)__A * (__v8hf)__B);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_mul_ph(__m128h __W, __mmask8 __U, __m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_selectph_128 (__U,
            (__v8hf) _mm_mul_ph(__A, __B),
            (__v8hf) __W);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_mul_ph(__mmask8 __U, __m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_selectph_128 (__U,
            (__v8hf) _mm_mul_ph(__A, __B),
            (__v8hf) _mm_setzero_ph ());
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_div_ph(__m256h __A, __m256h __B) {
  return (__m256h)((__v16hf)__A / (__v16hf)__B);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_div_ph(__m256h __W, __mmask16 __U, __m256h __A, __m256h __B) {
  return (__m256h) __builtin_ia32_selectph_256 (__U,
            (__v16hf) _mm256_div_ph(__A, __B),
            (__v16hf) __W);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_div_ph(__mmask16 __U, __m256h __A, __m256h __B) {
  return (__m256h) __builtin_ia32_selectph_256 (__U,
            (__v16hf) _mm256_div_ph(__A, __B),
            (__v16hf) _mm256_setzero_ph ());
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_div_ph(__m128h __A, __m128h __B) {
  return (__m128h)((__v8hf)__A / (__v8hf)__B);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_div_ph(__m128h __W, __mmask8 __U, __m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_selectph_128 (__U,
            (__v8hf) _mm_div_ph(__A, __B),
            (__v8hf) __W);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_div_ph(__mmask8 __U, __m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_selectph_128 (__U,
            (__v8hf) _mm_div_ph(__A, __B),
            (__v8hf) _mm_setzero_ph ());
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_min_ph(__m256h __A, __m256h __B) {
  return (__m256h)__builtin_ia32_minph256((__v16hf) __A,(__v16hf) __B);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_min_ph(__m256h __W, __mmask16 __U, __m256h __A, __m256h __B) {
  return (__m256h) __builtin_ia32_selectph_256 ((__mmask16) __U,
            (__v16hf) __builtin_ia32_minph256((__v16hf) __A,(__v16hf) __B),
            (__v16hf) __W);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_min_ph(__mmask16 __U, __m256h __A, __m256h __B) {
  return (__m256h) __builtin_ia32_selectph_256 ((__mmask16) __U,
            (__v16hf) __builtin_ia32_minph256((__v16hf) __A,(__v16hf) __B),
            (__v16hf) _mm256_setzero_ph ());
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_min_ph(__m128h __A, __m128h __B) {
  return (__m128h)__builtin_ia32_minph128((__v8hf) __A, (__v8hf)__B);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_min_ph(__m128h __W, __mmask8 __U, __m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_selectph_128 ((__mmask8) __U,
            (__v8hf) __builtin_ia32_minph128((__v8hf) __A, (__v8hf)__B),
            (__v8hf) __W);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_min_ph(__mmask8 __U, __m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_selectph_128 ((__mmask8) __U,
            (__v8hf) __builtin_ia32_minph128((__v8hf) __A, (__v8hf)__B),
            (__v8hf) _mm_setzero_ph ());
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_max_ph(__m256h __A, __m256h __B) {
  return (__m256h)__builtin_ia32_maxph256((__v16hf) __A,(__v16hf) __B);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_max_ph(__m256h __W, __mmask16 __U, __m256h __A, __m256h __B) {
  return (__m256h) __builtin_ia32_selectph_256 ((__mmask16) __U,
            (__v16hf) __builtin_ia32_maxph256((__v16hf) __A,(__v16hf) __B),
            (__v16hf) __W);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_max_ph(__mmask16 __U, __m256h __A, __m256h __B) {
  return (__m256h) __builtin_ia32_selectph_256 ((__mmask16) __U,
            (__v16hf) __builtin_ia32_maxph256((__v16hf) __A,(__v16hf) __B),
            (__v16hf) _mm256_setzero_ph ());
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_max_ph(__m128h __A, __m128h __B) {
  return (__m128h)__builtin_ia32_maxph128((__v8hf) __A, (__v8hf)__B);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_max_ph(__m128h __W, __mmask8 __U, __m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_selectph_128 ((__mmask8) __U,
            (__v8hf) __builtin_ia32_maxph128((__v8hf) __A, (__v8hf)__B),
            (__v8hf) __W);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_max_ph(__mmask8 __U, __m128h __A, __m128h __B) {
  return (__m128h) __builtin_ia32_selectph_128 ((__mmask8) __U,
            (__v8hf) __builtin_ia32_maxph128((__v8hf) __A, (__v8hf)__B),
            (__v8hf) _mm_setzero_ph ());
}

#define _mm256_cmp_ph_mask(a, b, p)  \
  (__mmask16)__builtin_ia32_cmpph256_mask((__v16hf)(__m256)(a), \
                                          (__v16hf)(__m256)(b), (int)(p), \
                                          (__mmask16)-1)

#define _mm256_mask_cmp_ph_mask(m, a, b, p)  \
  (__mmask16)__builtin_ia32_cmpph256_mask((__v16hf)(__m256)(a), \
                                          (__v16hf)(__m256)(b), (int)(p), \
                                          (__mmask16)(m))

#define _mm_cmp_ph_mask(a, b, p)  \
  (__mmask8)__builtin_ia32_cmpph128_mask((__v8hf)(__m128)(a), \
                                         (__v8hf)(__m128)(b), (int)(p), \
                                         (__mmask8)-1)

#define _mm_mask_cmp_ph_mask(m, a, b, p)  \
  (__mmask8)__builtin_ia32_cmpph128_mask((__v8hf)(__m128)(a), \
                                         (__v8hf)(__m128)(b), (int)(p), \
                                         (__mmask8)(m))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_fmadd_ph(__m128h __A, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_vfmaddph ((__v8hf) __A,
                                             (__v8hf) __B,
                                             (__v8hf) __C);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_fmadd_ph(__m128h __A, __mmask8 __U, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    __builtin_ia32_vfmaddph ((__v8hf) __A,
                                             (__v8hf) __B,
                                             (__v8hf) __C),
                    (__v8hf) __A);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask3_fmadd_ph(__m128h __A, __m128h __B, __m128h __C, __mmask8 __U)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    __builtin_ia32_vfmaddph ((__v8hf) __A,
                                             (__v8hf) __B,
                                             (__v8hf) __C),
                    (__v8hf) __C);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_fmadd_ph(__mmask8 __U, __m128h __A, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    __builtin_ia32_vfmaddph ((__v8hf) __A,
                                             (__v8hf) __B,
                                             (__v8hf) __C),
                    (__v8hf)_mm_setzero_ph());
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_fmsub_ph(__m128h __A, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_vfmaddph ((__v8hf) __A,
                                             (__v8hf) __B,
                                             -(__v8hf) __C);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_fmsub_ph(__m128h __A, __mmask8 __U, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    _mm_fmsub_ph ((__v8hf) __A,
                                             (__v8hf) __B,
                                             (__v8hf) __C),
                    (__v8hf) __A);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_fmsub_ph(__mmask8 __U, __m128h __A, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    _mm_fmsub_ph ((__v8hf) __A,
                                             (__v8hf) __B,
                                             (__v8hf) __C),
                    (__v8hf)_mm_setzero_ph());
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask3_fnmadd_ph(__m128h __A, __m128h __B, __m128h __C, __mmask8 __U)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    __builtin_ia32_vfmaddph (-(__v8hf) __A,
                                             (__v8hf) __B,
                                             (__v8hf) __C),
                    (__v8hf) __C);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_fnmadd_ph(__mmask8 __U, __m128h __A, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    __builtin_ia32_vfmaddph (-(__v8hf) __A,
                                             (__v8hf) __B,
                                             (__v8hf) __C),
                    (__v8hf)_mm_setzero_ph());
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_fnmsub_ph(__mmask8 __U, __m128h __A, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    __builtin_ia32_vfmaddph (-(__v8hf) __A,
                                             (__v8hf) __B,
                                             -(__v8hf) __C),
                    (__v8hf)_mm_setzero_ph());
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_fmadd_ph(__m256h __A, __m256h __B, __m256h __C)
{
  return (__m256h) __builtin_ia32_vfmaddph256 ((__v16hf) __A,
                                                (__v16hf) __B,
                                                (__v16hf) __C);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_fmadd_ph(__m256h __A, __mmask16 __U, __m256h __B, __m256h __C)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddph256 ((__v16hf) __A,
                                                (__v16hf) __B,
                                                (__v16hf) __C),
                    (__v16hf) __A);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask3_fmadd_ph(__m256h __A, __m256h __B, __m256h __C, __mmask16 __U)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddph256 ((__v16hf) __A,
                                                (__v16hf) __B,
                                                (__v16hf) __C),
                    (__v16hf) __C);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_fmadd_ph(__mmask16 __U, __m256h __A, __m256h __B, __m256h __C)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddph256 ((__v16hf) __A,
                                                (__v16hf) __B,
                                                (__v16hf) __C),
                    (__v16hf)_mm256_setzero_ph());
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_fmsub_ph(__m256h __A, __m256h __B, __m256h __C)
{
  return (__m256h) __builtin_ia32_vfmaddph256 ((__v16hf) __A,
                                                (__v16hf) __B,
                                                -(__v16hf) __C);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_fmsub_ph(__m256h __A, __mmask16 __U, __m256h __B, __m256h __C)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddph256 ((__v16hf) __A,
                                                (__v16hf) __B,
                                                -(__v16hf) __C),
                    (__v16hf) __A);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_fmsub_ph(__mmask16 __U, __m256h __A, __m256h __B, __m256h __C)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddph256 ((__v16hf) __A,
                                                (__v16hf) __B,
                                                -(__v16hf) __C),
                    (__v16hf)_mm256_setzero_ph());
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask3_fnmadd_ph(__m256h __A, __m256h __B, __m256h __C, __mmask16 __U)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddph256 (-(__v16hf) __A,
                                                (__v16hf) __B,
                                                -(__v16hf) __C),
                    (__v16hf) __C);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_fnmadd_ph(__mmask16 __U, __m256h __A, __m256h __B, __m256h __C)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddph256 (-(__v16hf) __A,
                                                (__v16hf) __B,
                                                (__v16hf) __C),
                    (__v16hf)_mm256_setzero_ph());
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_fnmsub_ph(__mmask16 __U, __m256h __A, __m256h __B, __m256h __C)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddph256 (-(__v16hf) __A,
                                                (__v16hf) __B,
                                                -(__v16hf) __C),
                    (__v16hf)_mm256_setzero_ph());
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_fmaddsub_ph(__m128h __A, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_vfmaddsubph ((__v8hf) __A,
                                                (__v8hf) __B,
                                                (__v8hf) __C);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_fmaddsub_ph(__m128h __A, __mmask8 __U, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    __builtin_ia32_vfmaddsubph ((__v8hf) __A,
                                                (__v8hf) __B,
                                                (__v8hf) __C),
                    (__v8hf) __A);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask3_fmaddsub_ph(__m128h __A, __m128h __B, __m128h __C, __mmask8 __U)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    __builtin_ia32_vfmaddsubph ((__v8hf) __A,
                                                (__v8hf) __B,
                                                (__v8hf) __C),
                    (__v8hf) __C);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_fmaddsub_ph(__mmask8 __U, __m128h __A, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    __builtin_ia32_vfmaddsubph ((__v8hf) __A,
                                                (__v8hf) __B,
                                                (__v8hf) __C),
                    (__v8hf)_mm_setzero_ph());
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_fmsubadd_ph(__m128h __A, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_vfmaddsubph ((__v8hf) __A,
                                                (__v8hf) __B,
                                                -(__v8hf) __C);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_fmsubadd_ph(__m128h __A, __mmask8 __U, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    __builtin_ia32_vfmaddsubph ((__v8hf) __A,
                                                (__v8hf) __B,
                                                -(__v8hf) __C),
                    (__v8hf) __A);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_fmsubadd_ph(__mmask8 __U, __m128h __A, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    __builtin_ia32_vfmaddsubph ((__v8hf) __A,
                                                (__v8hf) __B,
                                                -(__v8hf) __C),
                    (__v8hf)_mm_setzero_ph());
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_fmaddsub_ph(__m256h __A, __m256h __B, __m256h __C)
{
  return (__m256h) __builtin_ia32_vfmaddsubph256 ((__v16hf) __A,
                                                   (__v16hf) __B,
                                                   (__v16hf) __C);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_fmaddsub_ph(__m256h __A, __mmask16 __U, __m256h __B,
                         __m256h __C)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddsubph256 ((__v16hf) __A,
                                                   (__v16hf) __B,
                                                   (__v16hf) __C),
                    (__v16hf) __A);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask3_fmaddsub_ph(__m256h __A, __m256h __B, __m256h __C, __mmask16 __U)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddsubph256 ((__v16hf) __A,
                                                   (__v16hf) __B,
                                                   (__v16hf) __C),
                    (__v16hf) __C);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_fmaddsub_ph(__mmask16 __U, __m256h __A, __m256h __B, __m256h __C)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddsubph256 ((__v16hf) __A,
                                                   (__v16hf) __B,
                                                   (__v16hf) __C),
                    (__v16hf)_mm256_setzero_ph());
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_fmsubadd_ph(__m256h __A, __m256h __B, __m256h __C)
{
  return (__m256h) __builtin_ia32_vfmaddsubph256 ((__v16hf) __A,
                                                   (__v16hf) __B,
                                                   -(__v16hf) __C);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_fmsubadd_ph(__m256h __A, __mmask16 __U, __m256h __B, __m256h __C)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddsubph256 ((__v16hf) __A,
                                                   (__v16hf) __B,
                                                   -(__v16hf) __C),
                    (__v16hf) __A);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_fmsubadd_ph(__mmask16 __U, __m256h __A, __m256h __B, __m256h __C)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddsubph256 ((__v16hf) __A,
                                                   (__v16hf) __B,
                                                   -(__v16hf) __C),
                    (__v16hf)_mm256_setzero_ph());
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask3_fmsub_ph(__m128h __A, __m128h __B, __m128h __C, __mmask8 __U)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    __builtin_ia32_vfmaddph ((__v8hf) __A,
                                             (__v8hf) __B,
                                             -(__v8hf) __C),
                    (__v8hf) __C);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask3_fmsub_ph(__m256h __A, __m256h __B, __m256h __C, __mmask16 __U)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddph256 ((__v16hf) __A,
                                                (__v16hf) __B,
                                                -(__v16hf) __C),
                    (__v16hf) __C);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask3_fmsubadd_ph(__m128h __A, __m128h __B, __m128h __C, __mmask8 __U)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    __builtin_ia32_vfmaddsubph ((__v8hf) __A,
                                                (__v8hf) __B,
                                                -(__v8hf) __C),
                    (__v8hf) __C);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask3_fmsubadd_ph(__m256h __A, __m256h __B, __m256h __C, __mmask16 __U)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddsubph256 ((__v16hf) __A,
                                                   (__v16hf) __B,
                                                   -(__v16hf) __C),
                    (__v16hf) __C);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_fnmadd_ph(__m128h __A, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_vfmaddph ((__v8hf) __A,
                                             -(__v8hf) __B,
                                             (__v8hf) __C);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_fnmadd_ph(__m128h __A, __mmask8 __U, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    __builtin_ia32_vfmaddph ((__v8hf) __A,
                                             -(__v8hf) __B,
                                             (__v8hf) __C),
                    (__v8hf) __A);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_fnmadd_ph(__m256h __A, __m256h __B, __m256h __C)
{
  return (__m256h) __builtin_ia32_vfmaddph256 ((__v16hf) __A,
                                                -(__v16hf) __B,
                                                (__v16hf) __C);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_fnmadd_ph(__m256h __A, __mmask16 __U, __m256h __B, __m256h __C)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddph256 ((__v16hf) __A,
                                                -(__v16hf) __B,
                                                (__v16hf) __C),
                    (__v16hf) __A);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_fnmsub_ph(__m128h __A, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_vfmaddph ((__v8hf) __A,
                                             -(__v8hf) __B,
                                             -(__v8hf) __C);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_fnmsub_ph(__m128h __A, __mmask8 __U, __m128h __B, __m128h __C)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    __builtin_ia32_vfmaddph ((__v8hf) __A,
                                             -(__v8hf) __B,
                                             -(__v8hf) __C),
                    (__v8hf) __A);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask3_fnmsub_ph(__m128h __A, __m128h __B, __m128h __C, __mmask8 __U)
{
  return (__m128h) __builtin_ia32_selectph_128((__mmask8) __U,
                    __builtin_ia32_vfmaddph ((__v8hf) __A,
                                             -(__v8hf) __B,
                                             -(__v8hf) __C),
                    (__v8hf) __C);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_fnmsub_ph(__m256h __A, __m256h __B, __m256h __C)
{
  return (__m256h) __builtin_ia32_vfmaddph256 ((__v16hf) __A,
                                                -(__v16hf) __B,
                                                -(__v16hf) __C);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_fnmsub_ph(__m256h __A, __mmask16 __U, __m256h __B, __m256h __C)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddph256 ((__v16hf) __A,
                                                -(__v16hf) __B,
                                                -(__v16hf) __C),
                    (__v16hf) __A);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask3_fnmsub_ph(__m256h __A, __m256h __B, __m256h __C, __mmask16 __U)
{
  return (__m256h) __builtin_ia32_selectph_256((__mmask16) __U,
                    __builtin_ia32_vfmaddph256 ((__v16hf) __A,
                                                -(__v16hf) __B,
                                                -(__v16hf) __C),
                    (__v16hf) __C);
}
#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif

/* end INTEL_FEATURE_ISA_FP16 */
