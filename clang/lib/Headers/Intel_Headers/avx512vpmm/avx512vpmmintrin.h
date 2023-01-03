/*===----------- avx512vpmmintrin.h - AVX512-VPMM intrinsics ---------------===
 *
 * INTEL CONFIDENTIAL
 *
 * Modifications, Copyright (C) 2022 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may
 * not use, modify, copy, publish, distribute, disclose or transmit this software
 * or the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
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
#error "Never use <avx512vpmmintrin.h> directly; include <immintrin.h> instead."
#endif
#ifdef __x86_64__
#ifdef __SSE2__

#ifndef __AVX512VPMMINTRIN_H
#define __AVX512VPMMINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512vpmm,avx512vl"),                            \
                 __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512vpmm,avx512vl"),                            \
                 __min_vector_width__(256)))
#define __DEFAULT_FN_ATTRS512                                                  \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512vpmm"),     \
                 __min_vector_width__(512)))

// vmmxf16ps
static __inline__ void __DEFAULT_FN_ATTRS128
_mm128_vmmxf16_ps(__m128 *__dst_low, __m128 *__dst_high, __m128h __b,
                  __m128h __c, __m128 __low, __m128 __high) {
  __builtin_ia32_vmmxf16ps_128(__dst_high, __dst_low, __b, __c, __high, __low);
}

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_vmmxf16_ps(__m256 *__dst_low, __m256 *__dst_high, __m256h __b,
                  __m256h __c, __m256 __low, __m256 __high) {
  __builtin_ia32_vmmxf16ps_256(__dst_high, __dst_low, __b, __c, __high, __low);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_vmmxf16_ps(__m512 *__dst_low, __m512 *__dst_high, __m512h __b,
                  __m512h __c, __m512 __low, __m512 __high) {
  __builtin_ia32_vmmxf16ps_512(__dst_high, __dst_low, __b, __c, __high, __low);
}

// vmmif16ps
#define _mm128_vmmif16_ps(b, c, s, i) \
  __builtin_ia32_vmmif16ps_128((__m128)(s), (__m128h)(b), (__m128h)(c), (int)i)

#define _mm256_vmmif16_ps(b, c, s, i) \
  __builtin_ia32_vmmif16ps_256((__m256)(s), (__m256h)(b), (__m256h)(c), (int)i)

#define _mm512_vmmif16_ps(b, c, s, i) \
  __builtin_ia32_vmmif16ps_512((__m512)(s), (__m512h)(b), (__m512h)(c), (int)i)

// vmmf16ps
static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm128_vmmf16_ps(__m128h __b, __m128h __c, __m128 __src) {
  return __builtin_ia32_vmmf16ps_128(__src, __b, __c);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_vmmf16_ps(__m256h __b, __m256h __c, __m256 __src) {
  return __builtin_ia32_vmmf16ps_256(__src, __b, __c);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_vmmf16_ps(__m512h __b, __m512h __c, __m512 __src) {
  return __builtin_ia32_vmmf16ps_512(__src, __b, __c);
}
#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256
#undef __DEFAULT_FN_ATTRS512

#endif
#endif
#endif
