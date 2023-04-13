/*===-------- avx512vlneconvertfp8intrin.h - AVX512NECONVERTFP8 ----------=== */
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
#error "Never use <avx512vlneconvertfp8intrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifdef __SSE2__

#ifndef __AVX512VLNECONVERTFP8INTRIN_H
#define __AVX512VLNECONVERTFP8INTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512vl,avx512neconvertfp8"), __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512vl,avx512neconvertfp8"), __min_vector_width__(256)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtbias2ph_pbf8(__m128i __A, __m128h __B, __m128h __C) {
  return (__m128i)__builtin_ia32_vcvtbias2ph2bf8_128((__v16qi)(__A), (__v8hf)(__B), (__v8hf)(__C));
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_cvtbias2ph_pbf8(__m256i __A, __m256h __B, __m256h __C) {
  return (__m256i)__builtin_ia32_vcvtbias2ph2bf8_256((__v32qi)(__A), (__v16hf)(__B), (__v16hf)(__C));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtbias2ph2bf8s_pbf8(__m128i __A, __m128h __B, __m128h __C) {
  return (__m128i)__builtin_ia32_vcvtbias2ph2bf8s_128((__v16qi)(__A), (__v8hf)(__B), (__v8hf)(__C));
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_cvtbias2ph2bf8s_pbf8(__m256i __A, __m256h __B, __m256h __C) {
  return (__m256i)__builtin_ia32_vcvtbias2ph2bf8s_256((__v32qi)(__A), (__v16hf)(__B), (__v16hf)(__C));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtbias2ph_phf8(__m128i __A, __m128h __B, __m128h __C) {
  return (__m128i)__builtin_ia32_vcvtbias2ph2hf8_128((__v16qi)(__A), (__v8hf)(__B), (__v8hf)(__C));
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_cvtbias2ph_phf8(__m256i __A, __m256h __B, __m256h __C) {
  return (__m256i)__builtin_ia32_vcvtbias2ph2hf8_256((__v32qi)(__A), (__v16hf)(__B), (__v16hf)(__C));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtbias2ph2hf8s_phf8(__m128i __A, __m128h __B, __m128h __C) {
  return (__m128i)__builtin_ia32_vcvtbias2ph2hf8s_128((__v16qi)(__A), (__v8hf)(__B), (__v8hf)(__C));
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_cvtbias2ph2hf8s_phf8(__m256i __A, __m256h __B, __m256h __C) {
  return (__m256i)__builtin_ia32_vcvtbias2ph2hf8s_256((__v32qi)(__A), (__v16hf)(__B), (__v16hf)(__C));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtbiasph_pbf8(__m128h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8_128_mask((__v8hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask8)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtbiasph_pbf8(__m128i __W, __mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8_128_mask((__v8hf)__A, (__v16qi)(__m128i)__W, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtbiasph_pbf8(__mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8_128_mask((__v8hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_cvtbiasph_pbf8(__m256h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8_256_mask((__v16hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask16)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_cvtbiasph_pbf8(__m128i __W, __mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8_256_mask((__v16hf)__A, (__v16qi)(__m128i)__W, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtbiasph_pbf8(__mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8_256_mask((__v16hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtbiasph2bf8s_pbf8(__m128h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8s_128_mask((__v8hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask8)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtbiasph2bf8s_pbf8(__m128i __W, __mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8s_128_mask((__v8hf)__A, (__v16qi)(__m128i)__W, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtbiasph2bf8s_pbf8(__mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8s_128_mask((__v8hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_cvtbiasph2bf8s_pbf8(__m256h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8s_256_mask((__v16hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask16)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_cvtbiasph2bf8s_pbf8(__m128i __W, __mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8s_256_mask((__v16hf)__A, (__v16qi)(__m128i)__W, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtbiasph2bf8s_pbf8(__mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2bf8s_256_mask((__v16hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtbiasph_phf8(__m128h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8_128_mask((__v8hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask8)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtbiasph_phf8(__m128i __W, __mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8_128_mask((__v8hf)__A, (__v16qi)(__m128i)__W, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtbiasph_phf8(__mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8_128_mask((__v8hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_cvtbiasph_phf8(__m256h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8_256_mask((__v16hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask16)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_cvtbiasph_phf8(__m128i __W, __mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8_256_mask((__v16hf)__A, (__v16qi)(__m128i)__W, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtbiasph_phf8(__mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8_256_mask((__v16hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtbiasph2hf8s_phf8(__m128h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8s_128_mask((__v8hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask8)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtbiasph2hf8s_phf8(__m128i __W, __mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8s_128_mask((__v8hf)__A, (__v16qi)(__m128i)__W, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtbiasph2hf8s_phf8(__mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8s_128_mask((__v8hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_cvtbiasph2hf8s_phf8(__m256h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8s_256_mask((__v16hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask16)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_cvtbiasph2hf8s_phf8(__m128i __W, __mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8s_256_mask((__v16hf)__A, (__v16qi)(__m128i)__W, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtbiasph2hf8s_phf8(__mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtbiasph2hf8s_256_mask((__v16hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtne2ph_pbf8(__m128h __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtne2ph2bf8_128((__v8hf)(__A), (__v8hf)(__B));
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_cvtne2ph_pbf8(__m256h __A, __m256h __B) {
  return (__m256i)__builtin_ia32_vcvtne2ph2bf8_256((__v16hf)(__A), (__v16hf)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtne2ph2bf8s_pbf8(__m128h __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtne2ph2bf8s_128((__v8hf)(__A), (__v8hf)(__B));
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_cvtne2ph2bf8s_pbf8(__m256h __A, __m256h __B) {
  return (__m256i)__builtin_ia32_vcvtne2ph2bf8s_256((__v16hf)(__A), (__v16hf)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtne2ph_phf8(__m128h __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtne2ph2hf8_128((__v8hf)(__A), (__v8hf)(__B));
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_cvtne2ph_phf8(__m256h __A, __m256h __B) {
  return (__m256i)__builtin_ia32_vcvtne2ph2hf8_256((__v16hf)(__A), (__v16hf)(__B));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtne2ph2hf8s_phf8(__m128h __A, __m128h __B) {
  return (__m128i)__builtin_ia32_vcvtne2ph2hf8s_128((__v8hf)(__A), (__v8hf)(__B));
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_cvtne2ph2hf8s_phf8(__m256h __A, __m256h __B) {
  return (__m256i)__builtin_ia32_vcvtne2ph2hf8s_256((__v16hf)(__A), (__v16hf)(__B));
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_cvtnebf8_ph(__m128i __A) {
  return (__m128h)__builtin_ia32_vcvtnebf8_2ph128_mask((__v16qi)__A, (__v8hf)(__m128h)_mm_undefined_ph(), (__mmask8)-1);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_cvtnebf8_ph(__m128h __W, __mmask8 __U, __m128i __A) {
  return (__m128h)__builtin_ia32_vcvtnebf8_2ph128_mask((__v16qi)__A, (__v8hf)(__m128h)__W, (__mmask8)__U);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_cvtnebf8_ph(__mmask8 __U, __m128i __A) {
  return (__m128h)__builtin_ia32_vcvtnebf8_2ph128_mask((__v16qi)__A, (__v8hf)(__m128h)_mm_setzero_ph(), (__mmask8)__U);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_cvtnebf8_ph(__m128i __A) {
  return (__m256h)__builtin_ia32_vcvtnebf8_2ph256_mask((__v16qi)__A, (__v16hf)(__m256h)_mm256_undefined_ph(), (__mmask16)-1);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_cvtnebf8_ph(__m256h __W, __mmask16 __U, __m128i __A) {
  return (__m256h)__builtin_ia32_vcvtnebf8_2ph256_mask((__v16qi)__A, (__v16hf)(__m256h)__W, (__mmask16)__U);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtnebf8_ph(__mmask16 __U, __m128i __A) {
  return (__m256h)__builtin_ia32_vcvtnebf8_2ph256_mask((__v16qi)__A, (__v16hf)(__m256h)_mm256_setzero_ph(), (__mmask16)__U);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_cvtnehf8_ph(__m128i __A) {
  return (__m128h)__builtin_ia32_vcvtnehf8_2ph128_mask((__v16qi)__A, (__v8hf)(__m128h)_mm_undefined_ph(), (__mmask8)-1);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_cvtnehf8_ph(__m128h __W, __mmask8 __U, __m128i __A) {
  return (__m128h)__builtin_ia32_vcvtnehf8_2ph128_mask((__v16qi)__A, (__v8hf)(__m128h)__W, (__mmask8)__U);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_cvtnehf8_ph(__mmask8 __U, __m128i __A) {
  return (__m128h)__builtin_ia32_vcvtnehf8_2ph128_mask((__v16qi)__A, (__v8hf)(__m128h)_mm_setzero_ph(), (__mmask8)__U);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_cvtnehf8_ph(__m128i __A) {
  return (__m256h)__builtin_ia32_vcvtnehf8_2ph256_mask((__v16qi)__A, (__v16hf)(__m256h)_mm256_undefined_ph(), (__mmask16)-1);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_cvtnehf8_ph(__m256h __W, __mmask16 __U, __m128i __A) {
  return (__m256h)__builtin_ia32_vcvtnehf8_2ph256_mask((__v16qi)__A, (__v16hf)(__m256h)__W, (__mmask16)__U);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtnehf8_ph(__mmask16 __U, __m128i __A) {
  return (__m256h)__builtin_ia32_vcvtnehf8_2ph256_mask((__v16qi)__A, (__v16hf)(__m256h)_mm256_setzero_ph(), (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtneph_pbf8(__m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8_128_mask((__v8hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask8)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtneph_pbf8(__m128i __W, __mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8_128_mask((__v8hf)__A, (__v16qi)(__m128i)__W, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtneph_pbf8(__mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8_128_mask((__v8hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_cvtneph_pbf8(__m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8_256_mask((__v16hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask16)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_cvtneph_pbf8(__m128i __W, __mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8_256_mask((__v16hf)__A, (__v16qi)(__m128i)__W, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtneph_pbf8(__mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8_256_mask((__v16hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtneph2bf8s_pbf8(__m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8s_128_mask((__v8hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask8)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtneph2bf8s_pbf8(__m128i __W, __mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8s_128_mask((__v8hf)__A, (__v16qi)(__m128i)__W, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtneph2bf8s_pbf8(__mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8s_128_mask((__v8hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_cvtneph2bf8s_pbf8(__m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8s_256_mask((__v16hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask16)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_cvtneph2bf8s_pbf8(__m128i __W, __mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8s_256_mask((__v16hf)__A, (__v16qi)(__m128i)__W, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtneph2bf8s_pbf8(__mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf8s_256_mask((__v16hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtneph_phf8(__m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8_128_mask((__v8hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask8)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtneph_phf8(__m128i __W, __mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8_128_mask((__v8hf)__A, (__v16qi)(__m128i)__W, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtneph_phf8(__mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8_128_mask((__v8hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_cvtneph_phf8(__m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8_256_mask((__v16hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask16)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_cvtneph_phf8(__m128i __W, __mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8_256_mask((__v16hf)__A, (__v16qi)(__m128i)__W, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtneph_phf8(__mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8_256_mask((__v16hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_cvtneph2hf8s_phf8(__m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8s_128_mask((__v8hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask8)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_cvtneph2hf8s_phf8(__m128i __W, __mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8s_128_mask((__v8hf)__A, (__v16qi)(__m128i)__W, (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_cvtneph2hf8s_phf8(__mmask8 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8s_128_mask((__v8hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask8)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_cvtneph2hf8s_phf8(__m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8s_256_mask((__v16hf)__A, (__v16qi)(__m128i)_mm_undefined_si128(), (__mmask16)-1);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_mask_cvtneph2hf8s_phf8(__m128i __W, __mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8s_256_mask((__v16hf)__A, (__v16qi)(__m128i)__W, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtneph2hf8s_phf8(__mmask16 __U, __m256h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2hf8s_256_mask((__v16hf)__A, (__v16qi)(__m128i)_mm_setzero_si128(), (__mmask16)__U);
}


#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVX512VLNECONVERTFP8INTRIN_H
#endif // __SSE2__
