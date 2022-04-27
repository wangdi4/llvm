/*===-------- avxconvertintrin.h - AVXCONVERT intrinsics -----------=== */
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
#error "Never use <avxconvertintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVXCONVERTINTRIN_H
#define __AVXCONVERTINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS256 \
  __attribute__((__always_inline__, __nodebug__, __target__("avxconvert"), __min_vector_width__(256)))
#define __DEFAULT_FN_ATTRS128 \
  __attribute__((__always_inline__, __nodebug__, __target__("avxconvert"), __min_vector_width__(128)))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_m128_vcvt2ps2ph_ph(__m128 __A, __m128 __B) {
  return (__m128h)__builtin_ia32_vcvt2ps2ph128((__v4sf)__A,(__v4sf)__B);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_m256_vcvt2ps2ph_ph(__m256 __A, __m256 __B) {
  return (__m256h)__builtin_ia32_vcvt2ps2ph256((__v8sf)__A,(__v8sf)__B);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_m128_vcvtbf162ph_ph(__m128i __A) {
  return (__m128h)__builtin_ia32_vcvtbf162ph128((__v8hi)__A);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_m256_vcvtbf162ph_ph(__m256i __A) {
  return (__m256h)__builtin_ia32_vcvtbf162ph256((__v16hi)__A);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_m128_vcvtneph2bf16_ph(__m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf16_128((__v8hf)__A);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_m256_vcvtneph2bf16_ph(__m256h __A) {
  return (__m256i)__builtin_ia32_vcvtneph2bf16_256((__v16hf)__A);
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVX512VLCONVERTINTRIN_H
