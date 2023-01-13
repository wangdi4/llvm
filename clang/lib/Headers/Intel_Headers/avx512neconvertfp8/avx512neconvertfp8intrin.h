/*===------- avx512neconvertfp8intrin.h - AVX512NECONVERTFP8 -------------=== */
/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Modifications, Copyright (C) 2022 Intel Corporation
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
#error "Never use <avx512neconvertfp8intrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifdef __SSE2__

#ifndef __AVX512NECONVERTFP8INTRIN_H
#define __AVX512NECONVERTFP8INTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS512 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512neconvertfp8"), __min_vector_width__(512)))

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_cvtbias2ph_pbf8(__m512i __A, __m512h __B, __m512h __C) {
  return (__m512i)__builtin_ia32_vcvtbias2ph2bf8_512((__v64qi)(__A), (__v32hf)(__B), (__v32hf)(__C));
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_cvtbias2ph2bf8s_pbf8(__m512i __A, __m512h __B, __m512h __C) {
  return (__m512i)__builtin_ia32_vcvtbias2ph2bf8s_512((__v64qi)(__A), (__v32hf)(__B), (__v32hf)(__C));
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_cvtbias2ph_phf8(__m512i __A, __m512h __B, __m512h __C) {
  return (__m512i)__builtin_ia32_vcvtbias2ph2hf8_512((__v64qi)(__A), (__v32hf)(__B), (__v32hf)(__C));
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_cvtbias2ph2hf8s_phf8(__m512i __A, __m512h __B, __m512h __C) {
  return (__m512i)__builtin_ia32_vcvtbias2ph2hf8s_512((__v64qi)(__A), (__v32hf)(__B), (__v32hf)(__C));
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_cvtbiasph_pbf8(__m512h __A) {
  return (__m256i)__builtin_ia32_vcvtbiasph2bf8_512_mask((__v32hf)__A, (__v32qi)(__m256i)_mm256_undefined_si256(), (__mmask32)-1);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_mask_cvtbiasph_pbf8(__m256i __W, __mmask32 __U, __m512h __A) {
  return (__m256i)__builtin_ia32_vcvtbiasph2bf8_512_mask((__v32hf)__A, (__v32qi)(__m256i)__W, (__mmask32)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_maskz_cvtbiasph_pbf8(__mmask32 __U, __m512h __A) {
  return (__m256i)__builtin_ia32_vcvtbiasph2bf8_512_mask((__v32hf)__A, (__v32qi)(__m256i)_mm256_setzero_si256(), (__mmask32)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_cvtbiasph2bf8s_pbf8(__m512h __A) {
  return (__m256i)__builtin_ia32_vcvtbiasph2bf8s_512_mask((__v32hf)__A, (__v32qi)(__m256i)_mm256_undefined_si256(), (__mmask32)-1);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_mask_cvtbiasph2bf8s_pbf8(__m256i __W, __mmask32 __U, __m512h __A) {
  return (__m256i)__builtin_ia32_vcvtbiasph2bf8s_512_mask((__v32hf)__A, (__v32qi)(__m256i)__W, (__mmask32)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_maskz_cvtbiasph2bf8s_pbf8(__mmask32 __U, __m512h __A) {
  return (__m256i)__builtin_ia32_vcvtbiasph2bf8s_512_mask((__v32hf)__A, (__v32qi)(__m256i)_mm256_setzero_si256(), (__mmask32)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_cvtbiasph_phf8(__m512h __A) {
  return (__m256i)__builtin_ia32_vcvtbiasph2hf8_512_mask((__v32hf)__A, (__v32qi)(__m256i)_mm256_undefined_si256(), (__mmask32)-1);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_mask_cvtbiasph_phf8(__m256i __W, __mmask32 __U, __m512h __A) {
  return (__m256i)__builtin_ia32_vcvtbiasph2hf8_512_mask((__v32hf)__A, (__v32qi)(__m256i)__W, (__mmask32)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_maskz_cvtbiasph_phf8(__mmask32 __U, __m512h __A) {
  return (__m256i)__builtin_ia32_vcvtbiasph2hf8_512_mask((__v32hf)__A, (__v32qi)(__m256i)_mm256_setzero_si256(), (__mmask32)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_cvtbiasph2hf8s_phf8(__m512h __A) {
  return (__m256i)__builtin_ia32_vcvtbiasph2hf8s_512_mask((__v32hf)__A, (__v32qi)(__m256i)_mm256_undefined_si256(), (__mmask32)-1);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_mask_cvtbiasph2hf8s_phf8(__m256i __W, __mmask32 __U, __m512h __A) {
  return (__m256i)__builtin_ia32_vcvtbiasph2hf8s_512_mask((__v32hf)__A, (__v32qi)(__m256i)__W, (__mmask32)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_maskz_cvtbiasph2hf8s_phf8(__mmask32 __U, __m512h __A) {
  return (__m256i)__builtin_ia32_vcvtbiasph2hf8s_512_mask((__v32hf)__A, (__v32qi)(__m256i)_mm256_setzero_si256(), (__mmask32)__U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_cvtne2ph_pbf8(__m512h __A, __m512h __B) {
  return (__m512i)__builtin_ia32_vcvtne2ph2bf8_512((__v32hf)(__A), (__v32hf)(__B));
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_cvtne2ph2bf8s_pbf8(__m512h __A, __m512h __B) {
  return (__m512i)__builtin_ia32_vcvtne2ph2bf8s_512((__v32hf)(__A), (__v32hf)(__B));
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_cvtne2ph_phf8(__m512h __A, __m512h __B) {
  return (__m512i)__builtin_ia32_vcvtne2ph2hf8_512((__v32hf)(__A), (__v32hf)(__B));
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_cvtne2ph2hf8s_phf8(__m512h __A, __m512h __B) {
  return (__m512i)__builtin_ia32_vcvtne2ph2hf8s_512((__v32hf)(__A), (__v32hf)(__B));
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_cvtnebf8_ph(__m256i __A) {
  return (__m512h)__builtin_ia32_vcvtnebf8_2ph512_mask((__v32qi)__A, (__v32hf)(__m512h)_mm512_undefined_ph(), (__mmask32)-1);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_cvtnebf8_ph(__m512h __W, __mmask32 __U, __m256i __A) {
  return (__m512h)__builtin_ia32_vcvtnebf8_2ph512_mask((__v32qi)__A, (__v32hf)(__m512h)__W, (__mmask32)__U);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_cvtnebf8_ph(__mmask32 __U, __m256i __A) {
  return (__m512h)__builtin_ia32_vcvtnebf8_2ph512_mask((__v32qi)__A, (__v32hf)(__m512h)_mm512_setzero_ph(), (__mmask32)__U);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_cvtnehf8_ph(__m256i __A) {
  return (__m512h)__builtin_ia32_vcvtnehf8_2ph512_mask((__v32qi)__A, (__v32hf)(__m512h)_mm512_undefined_ph(), (__mmask32)-1);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_mask_cvtnehf8_ph(__m512h __W, __mmask32 __U, __m256i __A) {
  return (__m512h)__builtin_ia32_vcvtnehf8_2ph512_mask((__v32qi)__A, (__v32hf)(__m512h)__W, (__mmask32)__U);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_mm512_maskz_cvtnehf8_ph(__mmask32 __U, __m256i __A) {
  return (__m512h)__builtin_ia32_vcvtnehf8_2ph512_mask((__v32qi)__A, (__v32hf)(__m512h)_mm512_setzero_ph(), (__mmask32)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_cvtneph_pbf8(__m512h __A) {
  return (__m256i)__builtin_ia32_vcvtneph2bf8_512_mask((__v32hf)__A, (__v32qi)(__m256i)_mm256_undefined_si256(), (__mmask32)-1);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_mask_cvtneph_pbf8(__m256i __W, __mmask32 __U, __m512h __A) {
  return (__m256i)__builtin_ia32_vcvtneph2bf8_512_mask((__v32hf)__A, (__v32qi)(__m256i)__W, (__mmask32)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_maskz_cvtneph_pbf8(__mmask32 __U, __m512h __A) {
  return (__m256i)__builtin_ia32_vcvtneph2bf8_512_mask((__v32hf)__A, (__v32qi)(__m256i)_mm256_setzero_si256(), (__mmask32)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_cvtneph2bf8s_pbf8(__m512h __A) {
  return (__m256i)__builtin_ia32_vcvtneph2bf8s_512_mask((__v32hf)__A, (__v32qi)(__m256i)_mm256_undefined_si256(), (__mmask32)-1);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_mask_cvtneph2bf8s_pbf8(__m256i __W, __mmask32 __U, __m512h __A) {
  return (__m256i)__builtin_ia32_vcvtneph2bf8s_512_mask((__v32hf)__A, (__v32qi)(__m256i)__W, (__mmask32)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_maskz_cvtneph2bf8s_pbf8(__mmask32 __U, __m512h __A) {
  return (__m256i)__builtin_ia32_vcvtneph2bf8s_512_mask((__v32hf)__A, (__v32qi)(__m256i)_mm256_setzero_si256(), (__mmask32)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_cvtneph_phf8(__m512h __A) {
  return (__m256i)__builtin_ia32_vcvtneph2hf8_512_mask((__v32hf)__A, (__v32qi)(__m256i)_mm256_undefined_si256(), (__mmask32)-1);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_mask_cvtneph_phf8(__m256i __W, __mmask32 __U, __m512h __A) {
  return (__m256i)__builtin_ia32_vcvtneph2hf8_512_mask((__v32hf)__A, (__v32qi)(__m256i)__W, (__mmask32)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_maskz_cvtneph_phf8(__mmask32 __U, __m512h __A) {
  return (__m256i)__builtin_ia32_vcvtneph2hf8_512_mask((__v32hf)__A, (__v32qi)(__m256i)_mm256_setzero_si256(), (__mmask32)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_cvtneph2hf8s_phf8(__m512h __A) {
  return (__m256i)__builtin_ia32_vcvtneph2hf8s_512_mask((__v32hf)__A, (__v32qi)(__m256i)_mm256_undefined_si256(), (__mmask32)-1);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_mask_cvtneph2hf8s_phf8(__m256i __W, __mmask32 __U, __m512h __A) {
  return (__m256i)__builtin_ia32_vcvtneph2hf8s_512_mask((__v32hf)__A, (__v32qi)(__m256i)__W, (__mmask32)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS512
_mm512_maskz_cvtneph2hf8s_phf8(__mmask32 __U, __m512h __A) {
  return (__m256i)__builtin_ia32_vcvtneph2hf8s_512_mask((__v32hf)__A, (__v32qi)(__m256i)_mm256_setzero_si256(), (__mmask32)__U);
}

#undef __DEFAULT_FN_ATTRS512

#endif // __AVX512NECONVERTFP8INTRIN_H
#endif // __SSE2__
