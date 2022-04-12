/*===--- avx512vnniint8intrin.h - AVX512VNNIINT8 intrinsics --------=== */
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
#error "Never use <avx512vnniint8intrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512VNNIINT8INTRIN_H
#define __AVX512VNNIINT8INTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS512 \
  __attribute__((__always_inline__, __nodebug__,\
  __target__("avx512vnniint8"), __min_vector_width__(512)))

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_vpdpbssd_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_vpdpbssd512((__v16si)__W,
                                             (__v16si)__A, (__v16si)__B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_vpdpbssd_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbssd_epi32(__W, __A, __B),
                  (__v16si)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_vpdpbssd_epi32(__mmask16 __U, __m512i __W,  __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbssd_epi32(__W, __A, __B),
                  (__v16si)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_vpdpbssds_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_vpdpbssds512((__v16si)__W,
                                              (__v16si)__A, (__v16si)__B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_vpdpbssds_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbssds_epi32(__W, __A, __B),
                  (__v16si)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_vpdpbssds_epi32(__mmask16 __U, __m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbssds_epi32(__W, __A, __B),
                  (__v16si)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_vpdpbsud_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_vpdpbsud512((__v16si)__W,
                                             (__v16si)__A, (__v16si)__B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_vpdpbsud_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbsud_epi32(__W, __A, __B),
                  (__v16si)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_vpdpbsud_epi32(__mmask16 __U, __m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbsud_epi32(__W, __A, __B),
                  (__v16si)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_vpdpbsuds_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_vpdpbsuds512((__v16si)__W,
                                              (__v16si)__A, (__v16si)__B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_vpdpbsuds_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbsuds_epi32(__W, __A, __B),
                  (__v16si)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_vpdpbsuds_epi32(__mmask16 __U, __m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbsuds_epi32(__W, __A, __B),
                  (__v16si)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_vpdpbuud_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_vpdpbuud512((__v16si)__W, (__v16si)__A, (__v16si)__B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_vpdpbuud_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbuud_epi32(__W, __A, __B),
                  (__v16si)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_vpdpbuud_epi32(__mmask16 __U, __m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbuud_epi32(__W, __A, __B),
                  (__v16si)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_vpdpbuuds_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_vpdpbuuds512((__v16si)__W,
                                              (__v16si)__A, (__v16si)__B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_vpdpbuuds_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbuuds_epi32(__W, __A, __B),
                  (__v16si)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_vpdpbuuds_epi32( __mmask16 __U, __m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbuuds_epi32(__W, __A, __B),
                  (__v16si)_mm512_setzero_si512());
}
#undef __DEFAULT_FN_ATTRS512

#endif // __AVX512VNNIINT8INTRIN_H
