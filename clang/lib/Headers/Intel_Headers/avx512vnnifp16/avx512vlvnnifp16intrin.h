/*===--- avx512vlvnnifp16intrin.h - AVX512VNNIFP16 intrinsics -------=== */
/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Modifications, Copyright (C) 2022 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may
 * not use, modify, copy, publish, distribute, disclose or transmit this
 * software or the related documents without Intel's prior written permission.
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
#error "Never use <avx512vlvnnifp16intrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512VLVNNIFP16INTRIN_H
#define __AVX512VLVNNIFP16INTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS256                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512vnnifp16, avx512vl"),                       \
                 __min_vector_width__(256)))
#define __DEFAULT_FN_ATTRS128                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512vnnifp16, avx512vl"),                       \
                 __min_vector_width__(128)))

static __inline__ __m128 __DEFAULT_FN_ATTRS128 _mm_mask_dpph_ps(__m128 __W,
                                                                __mmask8 __U,
                                                                __m128h __A,
                                                                __m128h __B) {
  return (__m128)__builtin_ia32_selectps_128(
      (__mmask8)__U, (__v4sf)_mm_dpph_ps(__W, __A, __B), (__v4sf)__W);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128 _mm_maskz_dpph_ps(__mmask8 __U,
                                                                 __m128 __W,
                                                                 __m128h __A,
                                                                 __m128h __B) {
  return (__m128)__builtin_ia32_selectps_128((__mmask8)__U,
                                             (__v4sf)_mm_dpph_ps(__W, __A, __B),
                                             (__v4sf)_mm_setzero_ps());
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_mask_dpph_ps(__m256 __W, __mmask8 __U, __m256h __A, __m256h __B) {
  return (__m256)__builtin_ia32_selectps_256(
      (__mmask8)__U, (__v8sf)_mm256_dpph_ps(__W, __A, __B), (__v8sf)__W);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_maskz_dpph_ps(__mmask8 __U, __m256 __W, __m256h __A, __m256h __B) {
  return (__m256)__builtin_ia32_selectps_256(
      (__mmask8)__U, (__v8sf)_mm256_dpph_ps(__W, __A, __B),
      (__v8sf)_mm256_setzero_ps());
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVX512VLVNNIFP16INTRIN_H
