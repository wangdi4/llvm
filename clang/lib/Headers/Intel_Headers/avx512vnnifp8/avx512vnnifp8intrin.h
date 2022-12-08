/*===--------- avx512vnnifp8intrin.h - AVX512VNNIFP8 intrinsics ---------=== */
/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Modifications, Copyright (C) 2022 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may
 * not use, modify, copy, publish, distribute, disclose or transmit this
 * software or the related documents without Intel's prior written permission.
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
    "Never use <avx512vnnifp8intrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVX512VNNIFP8INTRIN_H
#define __AVX512VNNIFP8INTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS512                                                  \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512vnnifp8"),  \
                 __min_vector_width__(512)))

static __inline__ __m512 __DEFAULT_FN_ATTRS512 _mm512_dpbf8_ps(__m512 __A,
                                                               __m512i __B,
                                                               __m512i __C) {
  return (__m512)__builtin_ia32_vdpbf8ps512((__v16sf)(__A), (__v16sf)(__B),
                                            (__v16sf)(__C));
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_mask_dpbf8_ps(__m512 __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return (__m512)__builtin_ia32_selectps_512(
      (__mmask16)__U, (__v16sf)_mm512_dpbf8_ps(__W, __A, __B), (__v16sf)__W);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_maskz_dpbf8_ps(__mmask16 __U, __m512 __A, __m512i __B, __m512i __C) {
  return (__m512)__builtin_ia32_selectps_512(
      (__mmask16)__U, (__v16sf)_mm512_dpbf8_ps(__A, __B, __C),
      (__v16sf)_mm512_setzero_ps());
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512 _mm512_dpbhf8_ps(__m512 __A,
                                                                __m512i __B,
                                                                __m512i __C) {
  return (__m512)__builtin_ia32_vdpbhf8ps512((__v16sf)(__A), (__v16sf)(__B),
                                             (__v16sf)(__C));
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_mask_dpbhf8_ps(__m512 __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return (__m512)__builtin_ia32_selectps_512(
      (__mmask16)__U, (__v16sf)_mm512_dpbhf8_ps(__W, __A, __B), (__v16sf)__W);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_maskz_dpbhf8_ps(__mmask16 __U, __m512 __A, __m512i __B, __m512i __C) {
  return (__m512)__builtin_ia32_selectps_512(
      (__mmask16)__U, (__v16sf)_mm512_dpbhf8_ps(__A, __B, __C),
      (__v16sf)_mm512_setzero_ps());
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512 _mm512_dphbf8_ps(__m512 __A,
                                                                __m512i __B,
                                                                __m512i __C) {
  return (__m512)__builtin_ia32_vdphbf8ps512((__v16sf)(__A), (__v16sf)(__B),
                                             (__v16sf)(__C));
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_mask_dphbf8_ps(__m512 __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return (__m512)__builtin_ia32_selectps_512(
      (__mmask16)__U, (__v16sf)_mm512_dphbf8_ps(__W, __A, __B), (__v16sf)__W);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_maskz_dphbf8_ps(__mmask16 __U, __m512 __A, __m512i __B, __m512i __C) {
  return (__m512)__builtin_ia32_selectps_512(
      (__mmask16)__U, (__v16sf)_mm512_dphbf8_ps(__A, __B, __C),
      (__v16sf)_mm512_setzero_ps());
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512 _mm512_dphf8_ps(__m512 __A,
                                                               __m512i __B,
                                                               __m512i __C) {
  return (__m512)__builtin_ia32_vdphf8ps512((__v16sf)(__A), (__v16sf)(__B),
                                            (__v16sf)(__C));
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_mask_dphf8_ps(__m512 __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return (__m512)__builtin_ia32_selectps_512(
      (__mmask16)__U, (__v16sf)_mm512_dphf8_ps(__W, __A, __B), (__v16sf)__W);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_maskz_dphf8_ps(__mmask16 __U, __m512 __A, __m512i __B, __m512i __C) {
  return (__m512)__builtin_ia32_selectps_512(
      (__mmask16)__U, (__v16sf)_mm512_dphf8_ps(__A, __B, __C),
      (__v16sf)_mm512_setzero_ps());
}

#undef __DEFAULT_FN_ATTRS512

#endif // __AVX512VNNIFP8INTRIN_H