/*===----------- avx512vpmmintrin.h - AVX512-VPMM intrinsics ---------------===
 *
 * INTEL CONFIDENTIAL
 *
 * Modifications, Copyright (C) 2022 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may
 * not use, modify, copy, publish, distribute, disclose or transmit this
 *software or the related documents without Intel's prior written permission.
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
#define __DEFAULT_FN_ATTRS512                                                  \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512vpmm"),     \
                 __min_vector_width__(512)))

static __inline__ __m512 __DEFAULT_FN_ATTRS512 _mm512_vmmf16_ps(__m512h __b,
                                                                __m512h __c,
                                                                __m512 __src) {
  return __builtin_ia32_vmmf16ps_512(__src, __b, __c);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512 _mm512_vmmbf16_ps(__m512bh __B,
                                                                 __m512bh __C,
                                                                 __m512 __A) {
  return (__m512)__builtin_ia32_vmmbf16ps_512((__v16sf)(__A), (__v32bf)(__B),
                                              (__v32bf)(__C));
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512 _mm512_vmmbf8_ps(__m512i __B,
                                                                __m512i __C,
                                                                __m512 __A) {
  return (__m512)__builtin_ia32_vmmbf8ps_512((__v16sf)(__A), (__v64qi)(__B),
                                             (__v64qi)(__C));
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512 _mm512_vmmbhf8_ps(__m512i __B,
                                                                 __m512i __C,
                                                                 __m512 __A) {
  return (__m512)__builtin_ia32_vmmbhf8ps_512((__v16sf)(__A), (__v64qi)(__B),
                                              (__v64qi)(__C));
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512 _mm512_vmmhbf8_ps(__m512i __B,
                                                                 __m512i __C,
                                                                 __m512 __A) {
  return (__m512)__builtin_ia32_vmmhbf8ps_512((__v16sf)(__A), (__v64qi)(__B),
                                              (__v64qi)(__C));
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512 _mm512_vmmhf8_ps(__m512i __B,
                                                                __m512i __C,
                                                                __m512 __A) {
  return (__m512)__builtin_ia32_vmmhf8ps_512((__v16sf)(__A), (__v64qi)(__B),
                                             (__v64qi)(__C));
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512 _mm512_vmmtf32_ps(__m512i __B,
                                                                 __m512i __C,
                                                                 __m512 __A) {
  return (__m512)__builtin_ia32_vmmtf32ps_512((__v16sf)(__A), (__v16si)(__B),
                                              (__v16si)(__C));
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_vpmmssbd_epi32(__m512i __B, __m512i __C, __m512i __A) {
  return (__m512i)__builtin_ia32_vpmmssbd_512((__v16si)(__A), (__v64qi)(__B),
                                              (__v64qi)(__C));
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_vpmmsubd_epi32(__m512i __B, __m512i __C, __m512i __A) {
  return (__m512i)__builtin_ia32_vpmmsubd_512((__v16si)(__A), (__v64qi)(__B),
                                              (__v64qu)(__C));
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_vpmmusbd_epi32(__m512i __B, __m512i __C, __m512i __A) {
  return (__m512i)__builtin_ia32_vpmmusbd_512((__v16si)(__A), (__v64qu)(__B),
                                              (__v64qi)(__C));
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_vpmmuubd_epi32(__m512i __B, __m512i __C, __m512i __A) {
  return (__m512i)__builtin_ia32_vpmmuubd_512((__v16si)(__A), (__v64qu)(__B),
                                              (__v64qu)(__C));
}

#undef __DEFAULT_FN_ATTRS512

#endif // __AVX512VPMMINTRIN_H
#endif // __SSE2__
#endif // __x86_64__
