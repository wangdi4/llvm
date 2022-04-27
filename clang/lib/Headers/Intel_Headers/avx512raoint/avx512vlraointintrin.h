/*===-------- avx512vlraointintrin.h - AVX512VLRAOINT -------------=== */
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
#error "Never use <avx512vlraointintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVX512VLRAOINTINTRIN_H
#define __AVX512VLRAOINTINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512vl,avx512raoint"), __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512vl,avx512raoint"), __min_vector_width__(256)))

#define _mm_vpaadd_epi32(A, B) \
  __builtin_ia32_vpaaddd128((__v4si *)A, (__v4si)B)

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_mask_vpaadd_epi32(__m128i *__A, __mmask8 __U, __m128i __B) {
  __builtin_ia32_vpaaddd128_mask((__v4si *)__A, (__v4si)__B, (__mmask8)__U);
}

#define _mm256_vpaadd_epi32(A, B) \
  __builtin_ia32_vpaaddd256((__v8si *)A, (__v8si)B)

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_mask_vpaadd_epi32(__m256i *__A, __mmask8 __U, __m256i __B) {
  __builtin_ia32_vpaaddd256_mask((__v8si *)__A, (__v8si)__B, (__mmask8)__U);
}

#define _mm_vpaadd_epi64(A, B) \
  __builtin_ia32_vpaaddq128((__v2di *)A, (__v2di)B)

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_mask_vpaadd_epi64(__m128i *__A, __mmask8 __U, __m128i __B) {
  __builtin_ia32_vpaaddq128_mask((__v2di *)__A, (__v2di)__B, (__mmask8)__U);
}

#define _mm256_vpaadd_epi64(A, B) \
  __builtin_ia32_vpaaddq256((__v4di *)A, (__v4di)B)

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_mask_vpaadd_epi64(__m256i *__A, __mmask8 __U, __m256i __B) {
  __builtin_ia32_vpaaddq256_mask((__v4di *)__A, (__v4di)__B, (__mmask8)__U);
}

#define _mm_vpaand_epi32(A, B) \
  __builtin_ia32_vpaandd128((__v4si *)A, (__v4si)B)

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_mask_vpaand_epi32(__m128i *__A, __mmask8 __U, __m128i __B) {
  __builtin_ia32_vpaandd128_mask((__v4si *)__A, (__v4si)__B, (__mmask8)__U);
}

#define _mm256_vpaand_epi32(A, B) \
  __builtin_ia32_vpaandd256((__v8si *)A, (__v8si)B)

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_mask_vpaand_epi32(__m256i *__A, __mmask8 __U, __m256i __B) {
  __builtin_ia32_vpaandd256_mask((__v8si *)__A, (__v8si)__B, (__mmask8)__U);
}

#define _mm_vpaand_epi64(A, B) \
  __builtin_ia32_vpaandq128((__v2di *)A, (__v2di)B)

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_mask_vpaand_epi64(__m128i *__A, __mmask8 __U, __m128i __B) {
  __builtin_ia32_vpaandq128_mask((__v2di *)__A, (__v2di)__B, (__mmask8)__U);
}

#define _mm256_vpaand_epi64(A, B) \
  __builtin_ia32_vpaandq256((__v4di *)A, (__v4di)B)

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_mask_vpaand_epi64(__m256i *__A, __mmask8 __U, __m256i __B) {
  __builtin_ia32_vpaandq256_mask((__v4di *)__A, (__v4di)__B, (__mmask8)__U);
}

#define _mm_vpaor_epi32(A, B) \
  __builtin_ia32_vpaord128((__v4si *)A, (__v4si)B)

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_mask_vpaor_epi32(__m128i *__A, __mmask8 __U, __m128i __B) {
  __builtin_ia32_vpaord128_mask((__v4si *)__A, (__v4si)__B, (__mmask8)__U);
}

#define _mm256_vpaor_epi32(A, B) \
  __builtin_ia32_vpaord256((__v8si *)A, (__v8si)B)

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_mask_vpaor_epi32(__m256i *__A, __mmask8 __U, __m256i __B) {
  __builtin_ia32_vpaord256_mask((__v8si *)__A, (__v8si)__B, (__mmask8)__U);
}

#define _mm_vpaor_epi64(A, B) \
  __builtin_ia32_vpaorq128((__v2di *)A, (__v2di)B)

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_mask_vpaor_epi64(__m128i *__A, __mmask8 __U, __m128i __B) {
  __builtin_ia32_vpaorq128_mask((__v2di *)__A, (__v2di)__B, (__mmask8)__U);
}

#define _mm256_vpaor_epi64(A, B) \
  __builtin_ia32_vpaorq256((__v4di *)A, (__v4di)B)

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_mask_vpaor_epi64(__m256i *__A, __mmask8 __U, __m256i __B) {
  __builtin_ia32_vpaorq256_mask((__v4di *)__A, (__v4di)__B, (__mmask8)__U);
}

#define _mm_vpaxor_epi32(A, B) \
  __builtin_ia32_vpaxord128((__v4si *)A, (__v4si)B)

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_mask_vpaxor_epi32(__m128i *__A, __mmask8 __U, __m128i __B) {
  __builtin_ia32_vpaxord128_mask((__v4si *)__A, (__v4si)__B, (__mmask8)__U);
}

#define _mm256_vpaxor_epi32(A, B) \
  __builtin_ia32_vpaxord256((__v8si *)A, (__v8si)B)

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_mask_vpaxor_epi32(__m256i *__A, __mmask8 __U, __m256i __B) {
  __builtin_ia32_vpaxord256_mask((__v8si *)__A, (__v8si)__B, (__mmask8)__U);
}

#define _mm_vpaxor_epi64(A, B) \
  __builtin_ia32_vpaxorq128((__v2di *)A, (__v2di)B)

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_mask_vpaxor_epi64(__m128i *__A, __mmask8 __U, __m128i __B) {
  __builtin_ia32_vpaxorq128_mask((__v2di *)__A, (__v2di)__B, (__mmask8)__U);
}

#define _mm256_vpaxor_epi64(A, B) \
  __builtin_ia32_vpaxorq256((__v4di *)A, (__v4di)B)

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_mask_vpaxor_epi64(__m256i *__A, __mmask8 __U, __m256i __B) {
  __builtin_ia32_vpaxorq256_mask((__v4di *)__A, (__v4di)__B, (__mmask8)__U);
}


#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVX512VLRAOINTINTRIN_H
