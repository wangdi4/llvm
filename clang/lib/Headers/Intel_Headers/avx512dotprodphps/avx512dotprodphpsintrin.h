/*===----- avx512dotprodphpsintrin.h - AVX512DOTPRODPHPS intrinsics --------=== */
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
#error "Never use <avx512dotprodphpsintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512DOTPRODPHPSINTRIN_H
#define __AVX512DOTPRODPHPSINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS512 \
  __attribute__((__always_inline__, __nodebug__, \
  __target__("avx512dotprodphps"), __min_vector_width__(512)))

/* FIXME: For now the operand order is inconsistent with the instruction.
 * We need to be consistent with gcc when upstream. */

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_vdpphps_ps( __m512 __W, __m512h __A, __m512h __B) {
  return (__m512)__builtin_ia32_vdpphps512_mask((__v16sf)__W,(__v16sf)__A,
                                                (__v16sf)__B, (__mmask16)-1,
                                                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_mask_vdpphps_ps( __m512 __W, __mmask16 __U, __m512h __A, __m512h __B) {
  return (__m512)__builtin_ia32_vdpphps512_mask((__v16sf)__A,(__v16sf)__B,
                                                (__v16sf)__W, (__mmask16)__U,
                                                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_maskz_vdpphps_ps( __mmask16 __U, __m512 __W, __m512h __A, __m512h __B) {
  return (__m512)__builtin_ia32_vdpphps512_maskz((__v16sf)__A,(__v16sf)__B,
                                                 (__v16sf)__W, (__mmask16)__U,
                                                 _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_vdpphps_round_ps(W, A, B, R) \
  (__m512) __builtin_ia32_vdpphps512_mask((__v16sf)(__m512h) (A), \
                                          (__v16sf)(__m512h) (B), \
                                          (__v16sf)(__m512h) (W), \
                                          (__mmask16)-1, \
                                          (int)(R))

#define _mm512_mask_vdpphps_round_ps(W, U, A, B, R) \
  (__m512) __builtin_ia32_vdpphps512_mask((__v16sf)(__m512h) (A), \
                                          (__v16sf)(__m512h) (B), \
                                          (__v16sf)(__m512h) (W), \
                                          (__mmask16) (U), \
                                          (int)(R))

#define _mm512_maskz_vdpphps_round_ps(U, W, A, B, R) \
  (__m512) __builtin_ia32_vdpphps512_maskz((__v16sf)(__m512h) (A), \
                                           (__v16sf)(__m512h) (B), \
                                           (__v16sf)(__m512h) (W), \
                                           (__mmask16) (U), \
                                           (int)(R))

#undef __DEFAULT_FN_ATTRS512

#endif // __AVX512DOTPRODPHPSINTRIN_H
