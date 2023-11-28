/*===---------- avx512vlvpmmintrin.h - AVX512VL-VPMM intrinsics ------------===
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
#error                                                                         \
    "Never use <avx512vlvpmmintrin.h> directly; include <immintrin.h> instead."
#endif
#ifdef __x86_64__
#ifdef __SSE2__

#ifndef __AVX512VLVPMMINTRIN_H
#define __AVX512VLVPMMINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512vpmm,avx512vl"),                            \
                 __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512vpmm,avx512vl"),                            \
                 __min_vector_width__(256)))

static __inline__ __m128 __DEFAULT_FN_ATTRS128 _mm_vmmf16_ps(__m128h __b,
                                                             __m128h __c,
                                                             __m128 __src) {
  return __builtin_ia32_vmmf16ps_128(__src, __b, __c);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256 _mm256_vmmf16_ps(__m256h __b,
                                                                __m256h __c,
                                                                __m256 __src) {
  return __builtin_ia32_vmmf16ps_256(__src, __b, __c);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256 _mm256_vmmbf16_ps(__m256bh __B,
                                                                 __m256bh __C,
                                                                 __m256 __A) {
  return (__m256)__builtin_ia32_vmmbf16ps_256((__v8sf)(__A), (__v16bf)(__B),
                                              (__v16bf)(__C));
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256 _mm256_vmmbf8_ps(__m256i __B,
                                                                __m256i __C,
                                                                __m256 __A) {
  return (__m256)__builtin_ia32_vmmbf8ps_256((__v8sf)(__A), (__v32qi)(__B),
                                             (__v32qi)(__C));
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256 _mm256_vmmbhf8_ps(__m256i __B,
                                                                 __m256i __C,
                                                                 __m256 __A) {
  return (__m256)__builtin_ia32_vmmbhf8ps_256((__v8sf)(__A), (__v32qi)(__B),
                                              (__v32qi)(__C));
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256 _mm256_vmmhbf8_ps(__m256i __B,
                                                                 __m256i __C,
                                                                 __m256 __A) {
  return (__m256)__builtin_ia32_vmmhbf8ps_256((__v8sf)(__A), (__v32qi)(__B),
                                              (__v32qi)(__C));
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256 _mm256_vmmhf8_ps(__m256i __B,
                                                                __m256i __C,
                                                                __m256 __A) {
  return (__m256)__builtin_ia32_vmmhf8ps_256((__v8sf)(__A), (__v32qi)(__B),
                                             (__v32qi)(__C));
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256 _mm256_vmmtf32_ps(__m256i __B,
                                                                 __m256i __C,
                                                                 __m256 __A) {
  return (__m256)__builtin_ia32_vmmtf32ps_256((__v8sf)(__A), (__v8si)(__B),
                                              (__v8si)(__C));
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_vpmmssbd_epi32(__m256i __B, __m256i __C, __m256i __A) {
  return (__m256i)__builtin_ia32_vpmmssbd_256((__v8si)(__A), (__v32qi)(__B),
                                              (__v32qi)(__C));
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_vpmmsubd_epi32(__m256i __B, __m256i __C, __m256i __A) {
  return (__m256i)__builtin_ia32_vpmmsubd_256((__v8si)(__A), (__v32qi)(__B),
                                              (__v32qu)(__C));
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_vpmmusbd_epi32(__m256i __B, __m256i __C, __m256i __A) {
  return (__m256i)__builtin_ia32_vpmmusbd_256((__v8si)(__A), (__v32qu)(__B),
                                              (__v32qi)(__C));
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_vpmmuubd_epi32(__m256i __B, __m256i __C, __m256i __A) {
  return (__m256i)__builtin_ia32_vpmmuubd_256((__v8si)(__A), (__v32qu)(__B),
                                              (__v32qu)(__C));
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128 _mm_vmmbf16_ps(__m128bh __B,
                                                              __m128bh __C,
                                                              __m128 __A) {
  return (__m128)__builtin_ia32_vmmbf16ps_128((__v4sf)(__A), (__v8bf)(__B),
                                              (__v8bf)(__C));
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128 _mm_vmmbf8_ps(__m128i __B,
                                                             __m128i __C,
                                                             __m128 __A) {
  return (__m128)__builtin_ia32_vmmbf8ps_128((__v4sf)(__A), (__v16qi)(__B),
                                             (__v16qi)(__C));
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128 _mm_vmmbhf8_ps(__m128i __B,
                                                              __m128i __C,
                                                              __m128 __A) {
  return (__m128)__builtin_ia32_vmmbhf8ps_128((__v4sf)(__A), (__v16qi)(__B),
                                              (__v16qi)(__C));
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128 _mm_vmmhbf8_ps(__m128i __B,
                                                              __m128i __C,
                                                              __m128 __A) {
  return (__m128)__builtin_ia32_vmmhbf8ps_128((__v4sf)(__A), (__v16qi)(__B),
                                              (__v16qi)(__C));
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128 _mm_vmmhf8_ps(__m128i __B,
                                                             __m128i __C,
                                                             __m128 __A) {
  return (__m128)__builtin_ia32_vmmhf8ps_128((__v4sf)(__A), (__v16qi)(__B),
                                             (__v16qi)(__C));
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128 _mm_vmmtf32_ps(__m128i __B,
                                                              __m128i __C,
                                                              __m128 __A) {
  return (__m128)__builtin_ia32_vmmtf32ps_128((__v4sf)(__A), (__v4si)(__B),
                                              (__v4si)(__C));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_vpmmssbd_epi32(__m128i __B, __m128i __C, __m128i __A) {
  return (__m128i)__builtin_ia32_vpmmssbd_128((__v4si)(__A), (__v16qi)(__B),
                                              (__v16qi)(__C));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_vpmmsubd_epi32(__m128i __B, __m128i __C, __m128i __A) {
  return (__m128i)__builtin_ia32_vpmmsubd_128((__v4si)(__A), (__v16qi)(__B),
                                              (__v16qu)(__C));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_vpmmusbd_epi32(__m128i __B, __m128i __C, __m128i __A) {
  return (__m128i)__builtin_ia32_vpmmusbd_128((__v4si)(__A), (__v16qu)(__B),
                                              (__v16qi)(__C));
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_vpmmuubd_epi32(__m128i __B, __m128i __C, __m128i __A) {
  return (__m128i)__builtin_ia32_vpmmuubd_128((__v4si)(__A), (__v16qu)(__B),
                                              (__v16qu)(__C));
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVX512VLVPMMINTRIN_H
#endif // __SSE2__
#endif // __x86_64__
