/*===---------------- avxraofpintrin.h - AVXRAOFP ---------------------------=== */
/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Modifications, Copyright (C) 2021 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may not
 * use, modify, copy, publish, distribute, disclose or transmit this software or
 * the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/* end INTEL_CUSTOMIZATION */
/*
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *===-----------------------------------------------------------------------===
 */
#ifndef __IMMINTRIN_H
#error "Never use <avxraofpintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVXRAOFPINTRIN_H
#define __AVXRAOFPINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128 \
  __attribute__((__always_inline__, __nodebug__, __target__("avxraofp"), __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256 \
  __attribute__((__always_inline__, __nodebug__, __target__("avxraofp"), __min_vector_width__(256)))

/* Below intrinsics defined in avx512vlraofpintrin.h can be used for AVX_RAO_FP */
/// \fn void _mm_vaadd_pbh(__m128bh *__B, __m128bh __C)
/// \fn void _mm256_vaadd_pbh(__m256bh *__B, __m256bh __C)
/// \fn void _mm_vaadd_pd(__m128d *__B, __m128d __C)
/// \fn void _mm256_vaadd_pd(__m256d *__B, __m256d __C)
/// \fn void _mm_vaadd_ph(__m128h *__B, __m128h __C)
/// \fn void _mm256_vaadd_ph(__m256h *__B, __m256h __C)
/// \fn void _mm_vaadd_ps(__m128 *__B, __m128 __C)
/// \fn void _mm256_vaadd_ps(__m256 *__B, __m256 __C)
//
/* Below intrinsics defined in avx512raofpintrin.h can be used for AVX_RAO_FP */
/// \fn void _mm_vaadd_sbh(__m128bh *__B, __m128bh __C)
/// \fn void _mm_vaadd_sd(__m128d *__B, __m128d __C)
/// \fn void _mm_vaadd_sh(__m128h *__B, __m128h __C)
/// \fn void _mm_vaadd_ss(__m128 *__B, __m128 __C)



static __inline__ void __DEFAULT_FN_ATTRS128
_mm_vaadd_avx_pbh(__m128bh *__A, __m128bh __B) {
  __builtin_ia32_vaaddpbf16128((__v8hi *)__A, (__v8hi)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_vaadd_avx_pbh(__m256bh *__A, __m256bh __B) {
  __builtin_ia32_vaaddpbf16256((__v16hi *)__A, (__v16hi)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_vaadd_avx_pd(__m128d *__A, __m128d __B) {
  __builtin_ia32_vaaddpd128((__v2df *)__A, (__v2df)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_vaadd_avx_pd(__m256d *__A, __m256d __B) {
  __builtin_ia32_vaaddpd256((__v4df *)__A, (__v4df)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_vaadd_avx_ph(__m128h *__A, __m128h __B) {
  __builtin_ia32_vaaddph128((__v8hf *)__A, (__v8hf)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_vaadd_avx_ph(__m256h *__A, __m256h __B) {
  __builtin_ia32_vaaddph256((__v16hf *)__A, (__v16hf)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_vaadd_avx_ps(__m128 *__A, __m128 __B) {
  __builtin_ia32_vaaddps128((__v4sf *)__A, (__v4sf)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_vaadd_avx_ps(__m256 *__A, __m256 __B) {
  __builtin_ia32_vaaddps256((__v8sf *)__A, (__v8sf)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_vaadd_avx_sbh(void *__A, __m128bh __B) {
  __builtin_ia32_vaaddsbf16((void *)__A, (__v8hi)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_vaadd_avx_sd(void *__A, __m128d __B) {
  __builtin_ia32_vaaddsd((void *)__A, (__v2df)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_vaadd_avx_sh(void *__A, __m128h __B) {
  __builtin_ia32_vaaddsh((void *)__A, (__v8hf)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_vaadd_avx_ss(void *__A, __m128 __B) {
  __builtin_ia32_vaaddss((void *)__A, (__v4sf)__B);
}


#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVXRAOFPINTRIN_H
