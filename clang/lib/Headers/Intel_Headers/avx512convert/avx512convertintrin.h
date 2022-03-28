/*===-------- avx512convertintrin.h - AVX512CONVERT intrinsics -----------=== */
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
#error "Never use <avx512convertintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512CONVERTINTRIN_H
#define __AVX512CONVERTINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS512 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512convert"), __min_vector_width__(512)))

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_m512_mask_vcvt2ps2ph_ph( __m512h __W, __mmask32 __U, __m512 __A, __m512 __B) {
  return (__m512h)__builtin_ia32_vcvt2ps2ph512_mask((__v16sf)__A,
                                                    (__v16sf)__B,
                                                    (__v32hf)__W,
                                                    (__mmask32)__U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mask_vcvt2ps2ph_round_ph(W, U, A, B, R) \
  (__m512h) __builtin_ia32_vcvt2ps2ph512_mask((__v16sf)(__m512h) (A), \
                                              (__v16sf)(__m512h) (B), \
                                              (__v32hf)(__m512h) (W), \
                                              (__mmask32) (U), \
                                              (int)(R))


static __inline__ __m512h __DEFAULT_FN_ATTRS512
_m512_maskz_vcvt2ps2ph_ph(__mmask32 __U, __m512 __A, __m512 __B) {
  return (__m512h)__builtin_ia32_vcvt2ps2ph512_mask((__v16sf)__A,
                                                    (__v16sf)__B,
                                                    (__v32hf)_mm512_setzero_ph(),
                                                    (__mmask32)__U,
                                                    _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_maskz_vcvt2ps2ph_round_ph(U, A, B, R) \
  (__m512h) __builtin_ia32_vcvt2ps2ph512_mask((__v16sf)(__m512h) (A), \
                                              (__v16sf)(__m512h) (B), \
                                              (__v32hf)(__v32hf) _mm512_setzero_ph(), \
                                              (__mmask32) (U), \
                                              (int)(R))



static __inline__ __m512h __DEFAULT_FN_ATTRS512
_m512_mask_vcvtbf162ph_ph(__m512h __W, __mmask32 __U, __m512i __A) {
  return (__m512h)__builtin_ia32_vcvtbf162ph512_mask((__v32hi)__A,
                                                     (__v32hf)__W,
                                                     (__mmask32)__U,
                                                     _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_mask_vcvtbf162ph_round_ph(W, U, A, R) \
  (__m512h) __builtin_ia32_vcvtbf162ph512_mask((__v32hi)(__m512i) (A), \
                                               (__v32hf)(__m512h) (W), \
                                               (__mmask32) (U), \
                                               (int)(R))

static __inline__ __m512h __DEFAULT_FN_ATTRS512
_m512_maskz_vcvtbf162ph_ph(__mmask32 __U, __m512i __A) {
  return (__m512h)__builtin_ia32_vcvtbf162ph512_mask((__v32hi)__A,
                                                     (__v32hf)_mm512_setzero_ph(),
                                                     (__mmask32)__U,
                                                     _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_maskz_vcvtbf162ph_round_ph(U, A, R) \
  (__m512h) __builtin_ia32_vcvtbf162ph512_mask((__v32hi)(__m512i) (A), \
                                               (__v32hf)_mm512_setzero_ph(), \
                                               (__mmask32) (U), \
                                               (int)(R))

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_m512_mask_vcvtneph2bf16_ph(__m512i __W, __mmask32 __U, __m512h __A) {
  return (__m512i)__builtin_ia32_vcvtneph2bf16_512_mask((__v32hf)__A,
                                                        (__v32hi)__W,
                                                        (__mmask32)__U);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_m512_maskz_vcvtneph2bf16_ph(__mmask32 __U, __m512h __A) {
  return (__m512i)__builtin_ia32_vcvtneph2bf16_512_mask((__v32hf)__A,
                                                        (__v32hi)_mm512_setzero_si512(),
                                                        (__mmask32)__U);
}
#undef __DEFAULT_FN_ATTRS512

#endif // __AVX512CONVERTINTRIN_H
