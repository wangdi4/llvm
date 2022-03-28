/*===-------- avx512raointintrin.h - AVX512RAOINT -------------=== */
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
#error "Never use <avx512raointintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVX512RAOINTINTRIN_H
#define __AVX512RAOINTINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS512 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512raoint"), __min_vector_width__(512)))

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_vpaadd_epi32(__m512i *__A, __m512i __B) {
  __builtin_ia32_vpaaddd512((__v16si *)__A, (__v16si)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_mask_vpaadd_epi32(__m512i *__A, __mmask16 __U, __m512i __B) {
  __builtin_ia32_vpaaddd512_mask((__v16si *)__A, (__v16si)__B, (__mmask16)__U);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_vpaadd_epi64(__m512i *__A, __m512i __B) {
  __builtin_ia32_vpaaddq512((__v8di *)__A, (__v8di)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_mask_vpaadd_epi64(__m512i *__A, __mmask8 __U, __m512i __B) {
  __builtin_ia32_vpaaddq512_mask((__v8di *)__A, (__v8di)__B, (__mmask8)__U);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_vpaand_epi32(__m512i *__A, __m512i __B) {
  __builtin_ia32_vpaandd512((__v16si *)__A, (__v16si)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_mask_vpaand_epi32(__m512i *__A, __mmask16 __U, __m512i __B) {
  __builtin_ia32_vpaandd512_mask((__v16si *)__A, (__v16si)__B, (__mmask16)__U);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_vpaand_epi64(__m512i *__A, __m512i __B) {
  __builtin_ia32_vpaandq512((__v8di *)__A, (__v8di)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_mask_vpaand_epi64(__m512i *__A, __mmask8 __U, __m512i __B) {
  __builtin_ia32_vpaandq512_mask((__v8di *)__A, (__v8di)__B, (__mmask8)__U);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_vpaor_epi32(__m512i *__A, __m512i __B) {
  __builtin_ia32_vpaord512((__v16si *)__A, (__v16si)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_mask_vpaor_epi32(__m512i *__A, __mmask16 __U, __m512i __B) {
  __builtin_ia32_vpaord512_mask((__v16si *)__A, (__v16si)__B, (__mmask16)__U);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_vpaor_epi64(__m512i *__A, __m512i __B) {
  __builtin_ia32_vpaorq512((__v8di *)__A, (__v8di)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_mask_vpaor_epi64(__m512i *__A, __mmask8 __U, __m512i __B) {
  __builtin_ia32_vpaorq512_mask((__v8di *)__A, (__v8di)__B, (__mmask8)__U);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_vpaxor_epi32(__m512i *__A, __m512i __B) {
  __builtin_ia32_vpaxord512((__v16si *)__A, (__v16si)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_mask_vpaxor_epi32(__m512i *__A, __mmask16 __U, __m512i __B) {
  __builtin_ia32_vpaxord512_mask((__v16si *)__A, (__v16si)__B, (__mmask16)__U);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_vpaxor_epi64(__m512i *__A, __m512i __B) {
  __builtin_ia32_vpaxorq512((__v8di *)__A, (__v8di)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_mask_vpaxor_epi64(__m512i *__A, __mmask8 __U, __m512i __B) {
  __builtin_ia32_vpaxorq512_mask((__v8di *)__A, (__v8di)__B, (__mmask8)__U);
}

#undef __DEFAULT_FN_ATTRS512

#endif // __AVX512RAOINTINTRIN_H
