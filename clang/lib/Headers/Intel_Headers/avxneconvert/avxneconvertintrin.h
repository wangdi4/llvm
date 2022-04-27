/*===-------------- avxneconvertintrin.h - AVXNECONVERT ---------------------=== */
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
#error                                                                         \
    "Never use <avxneconvertintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVXNECONVERTINTRIN_H
#define __AVXNECONVERTINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128                                                  \
  __attribute__((__always_inline__, __nodebug__, __target__("avxneconvert"),   \
                 __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256                                                  \
  __attribute__((__always_inline__, __nodebug__, __target__("avxneconvert"),   \
                 __min_vector_width__(256)))

/* Below intrinsics defined in avx512vlneconvertintrin.h can be used for AVXNECONVERT */
// __m128 _mm_bcstnebf16_ps(const void *__A)
// __m256 _mm256_bcstnebf16_ps(const void *__A)
// __m128 _mm_bcstnesh_ps(const void *__A)
// __m256 _mm256_bcstnesh_ps(const void *__A)
// __m128 _mm_cvtneebf16_ps(const __m128bh *__A)
// __m256 _mm256_cvtneebf16_ps(const __m256bh *__A)
// __m128 _mm_cvtneeph_ps(const __m128h *__A)
// __m256 _mm256_cvtneeph_ps(const __m256h *__A)
// __m128 _mm_cvtneobf16_ps(const __m128bh *__A)
// __m256 _mm256_cvtneobf16_ps(const __m256bh *__A)
// __m128 _mm_cvtneoph_ps(const __m128h *__A)
// __m256 _mm256_cvtneoph_ps(const __m256h *__A)
// __m128bh _mm_cvtneps_pbh(__m128 __A)
// __m128bh _mm256_cvtneps_pbh(__m256 __A)

/* Intrinsics with _avx_ prefix are for compatibility with msvc. */
static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_bcstnebf16_avx_ps(const void *__A) {
  return (__m128)__builtin_ia32_vbcstnebf162ps128((const void *)__A);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_bcstnebf16_avx_ps(const void *__A) {
  return (__m256)__builtin_ia32_vbcstnebf162ps256((const void *)__A);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_bcstnesh_avx_ps(const void *__A) {
  return (__m128)__builtin_ia32_vbcstnesh2ps128((const void *)__A);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_bcstnesh_avx_ps(const void *__A) {
  return (__m256)__builtin_ia32_vbcstnesh2ps256((const void *)__A);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_cvtneebf16_avx_ps(const __m128bh *__A) {
  return (__m128)__builtin_ia32_vcvtneebf162ps128((const __v8hi *)__A);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_cvtneebf16_avx_ps(const __m256bh *__A) {
  return (__m256)__builtin_ia32_vcvtneebf162ps256((const __v16hi *)__A);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_cvtneeph_avx_ps(const __m128h *__A) {
  return (__m128)__builtin_ia32_vcvtneeph2ps128((const __v8hf *)__A);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_cvtneeph_avx_ps(const __m256h *__A) {
  return (__m256)__builtin_ia32_vcvtneeph2ps256((const __v16hf *)__A);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_cvtneobf16_avx_ps(const __m128bh *__A) {
  return (__m128)__builtin_ia32_vcvtneobf162ps128((const __v8hi *)__A);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_cvtneobf16_avx_ps(const __m256bh *__A) {
  return (__m256)__builtin_ia32_vcvtneobf162ps256((const __v16hi *)__A);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_cvtneoph_avx_ps(const __m128h *__A) {
  return (__m128)__builtin_ia32_vcvtneoph2ps128((const __v8hf *)__A);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_cvtneoph_avx_ps(const __m256h *__A) {
  return (__m256)__builtin_ia32_vcvtneoph2ps256((const __v16hf *)__A);
}

static __inline__ __m128bh __DEFAULT_FN_ATTRS128
_mm_cvtneps_avx_pbh(__m128 __A) {
  return (__m128bh)__builtin_ia32_vcvtneps2bf16128((__v4sf)__A);
}

static __inline__ __m128bh __DEFAULT_FN_ATTRS256
_mm256_cvtneps_avx_pbh(__m256 __A) {
  return (__m128bh)__builtin_ia32_vcvtneps2bf16256((__v8sf)__A);
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVXNECONVERTINTRIN_H
