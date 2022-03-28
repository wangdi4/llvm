/*===--------- avx512mediaxintrin.h - AVX512mediax intrinsics ---------------=== */
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
#error "Never use <avx512mediaxintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512MEDIAXINTRIN_H
#define __AVX512MEDIAXINTRIN_H

/* vmpsadbw128 */
#define _mm_mask_mpsadbw_epu8(W, U, A, B, imm) \
  (__m128i)__builtin_ia32_selectw_128((__mmask8)(U), \
                                      (__v8hi)_mm_mpsadbw_epu8((A), (B), (imm)), \
                                      (__v8hi)(__m128i)(W))

#define _mm_maskz_mpsadbw_epu8(U, A, B, imm) \
  (__m128i)__builtin_ia32_selectw_128((__mmask8)(U), \
                                      (__v8hi)_mm_mpsadbw_epu8((A), (B), (imm)), \
                                      (__v8hi)_mm_setzero_si128())

/* vmpsadbw256 */
#define _mm256_mask_mpsadbw_epu8(W, U, A, B, imm) \
  (__m256i)__builtin_ia32_selectw_256((__mmask16)(U), \
                                      (__v16hi)_mm256_mpsadbw_epu8((A), (B), (imm)), \
                                      (__v16hi)(__m256i)(W))

#define _mm256_maskz_mpsadbw_epu8(U, A, B, imm) \
  (__m256i)__builtin_ia32_selectw_256((__mmask16)(U), \
                                      (__v16hi)_mm256_mpsadbw_epu8((A), (B), (imm)), \
                                      (__v16hi)_mm256_setzero_si256())

/* vmpsadbw512 */
#define _mm512_mpsadbw_epu8(A, B, imm) \
  (__m512i)__builtin_ia32_mpsadbw512((__v64qi)(__m512i)(A), \
                                     (__v64qi)(__m512i)(B), (int)(imm))

#define _mm512_mask_mpsadbw_epu8(W, U, A, B, imm) \
  (__m512i)__builtin_ia32_selectw_512((__mmask32)(U), \
                                      (__v32hi)_mm512_mpsadbw_epu8((A), (B), (imm)), \
                                      (__v32hi)(__m512i)(W))

#define _mm512_maskz_mpsadbw_epu8(U, A, B, imm) \
  (__m512i)__builtin_ia32_selectw_512((__mmask32)(U), \
                                      (__v32hi)_mm512_mpsadbw_epu8((A), (B), (imm)), \
                                      (__v32hi)_mm512_setzero_si512())

#endif /* __AVX512MEDIAXINTRIN_H */
