/*===-------- avx512neconvertintrin.h - AVX512NECONVERT -------------=== */
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
    "Never use <avx512neconvertintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVX512NECONVERTINTRIN_H
#define __AVX512NECONVERTINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS512                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512neconvert"), __min_vector_width__(512)))

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_bcstnebf16_ps(const __bfloat16 *__A) {
  return (__m512)__builtin_ia32_vbcstnebf162ps512((const __bfloat16 *)__A);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_mask_bcstnebf16_ps(__m512 __W, __mmask16 __U, const __bfloat16 *__A) {
  return (__m512)__builtin_ia32_vbcstnebf162ps512_mask(
      (__v16sf)__W, (__mmask16)__U, (const __bfloat16 *)__A);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_maskz_bcstnebf16_ps(__mmask16 __U, const __bfloat16 *__A) {
  return (__m512)__builtin_ia32_vbcstnebf162ps512_maskz(
      (__mmask16)__U, (const __bfloat16 *)__A);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_bcstnesh_ps(const _Float16 *__A) {
  return (__m512)__builtin_ia32_vbcstnesh2ps512((const _Float16 *)__A);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_mask_bcstnesh_ps(__m512 __W, __mmask16 __U, const _Float16 *__A) {
  return (__m512)__builtin_ia32_vbcstnesh2ps512_mask(
      (__v16sf)__W, (__mmask16)__U, (const _Float16 *)__A);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_maskz_bcstnesh_ps(__mmask16 __U, const _Float16 *__A) {
  return (__m512)__builtin_ia32_vbcstnesh2ps512_maskz((__mmask16)__U,
                                                      (const _Float16 *)__A);
}

static __inline__ __m512h __DEFAULT_FN_ATTRS512 _mm512_cvtne2ps_ph(__m512 __A,
                                                                   __m512 __B) {
  return (__m512h)__builtin_ia32_vcvtne2ps2ph512((__v16sf)__A, (__v16sf)__B,
                                                 _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_cvtne_round2ps_ph(A, B, R)                                      \
  (__m512h) __builtin_ia32_vcvtne2ps2ph512((A), (B), (R))

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_cvtneebf16_ps(const __m512bh *__A) {
  return (__m512)__builtin_ia32_vcvtneebf162ps512((const __v32hi *)__A);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_mask_cvtneebf16_ps(__m512 __W, __mmask16 __U, const __m512bh *__A) {
  return (__m512)__builtin_ia32_vcvtneebf162ps512_mask(
      (__v16sf)__W, (__mmask16)__U, (const __v32hi *)__A);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_maskz_cvtneebf16_ps(__mmask16 __U, const __m512bh *__A) {
  return (__m512)__builtin_ia32_vcvtneebf162ps512_maskz((__mmask16)__U,
                                                        (const __v32hi *)__A);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_cvtneeph_ps(const __m512h *__A) {
  return (__m512)__builtin_ia32_vcvtneeph2ps512((const __v32hf *)__A);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_mask_cvtneeph_ps(__m512 __W, __mmask16 __U, const __m512h *__A) {
  return (__m512)__builtin_ia32_vcvtneeph2ps512_mask(
      (__v16sf)__W, (__mmask16)__U, (const __v32hf *)__A);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_maskz_cvtneeph_ps(__mmask16 __U, const __m512h *__A) {
  return (__m512)__builtin_ia32_vcvtneeph2ps512_maskz((__mmask16)__U,
                                                      (const __v32hf *)__A);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_cvtneobf16_ps(const __m512bh *__A) {
  return (__m512)__builtin_ia32_vcvtneobf162ps512((const __v32hi *)__A);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_mask_cvtneobf16_ps(__m512 __W, __mmask16 __U, const __m512bh *__A) {
  return (__m512)__builtin_ia32_vcvtneobf162ps512_mask(
      (__v16sf)__W, (__mmask16)__U, (const __v32hi *)__A);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_maskz_cvtneobf16_ps(__mmask16 __U, const __m512bh *__A) {
  return (__m512)__builtin_ia32_vcvtneobf162ps512_maskz((__mmask16)__U,
                                                        (const __v32hi *)__A);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_cvtneoph_ps(const __m512h *__A) {
  return (__m512)__builtin_ia32_vcvtneoph2ps512((const __v32hf *)__A);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_mask_cvtneoph_ps(__m512 __W, __mmask16 __U, const __m512h *__A) {
  return (__m512)__builtin_ia32_vcvtneoph2ps512_mask(
      (__v16sf)__W, (__mmask16)__U, (const __v32hf *)__A);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_maskz_cvtneoph_ps(__mmask16 __U, const __m512h *__A) {
  return (__m512)__builtin_ia32_vcvtneoph2ps512_maskz((__mmask16)__U,
                                                      (const __v32hf *)__A);
}

#undef __DEFAULT_FN_ATTRS512

#endif // __AVX512NECONVERTINTRIN_H
