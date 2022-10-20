/*===---------- avx512vlbf16neintrin.h - AVX512-BF16-NE intrinsics ---------===
/* INTEL_CUSTOMIZATION */
/*
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
    "Never use <avx512vlbf16neintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512VLBF16NEINTRIN_H
#define __AVX512VLBF16NEINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS256                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512bf16ne, avx512vl"),                         \
                 __min_vector_width__(256)))
#define __DEFAULT_FN_ATTRS128                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512bf16ne, avx512vl"),                         \
                 __min_vector_width__(128)))

static __inline__ __bf16 __DEFAULT_FN_ATTRS128
_mm_cvtsbf16_bf16(__m128bf16 __a) {
  return __a[0];
}

static __inline__ __bf16 __DEFAULT_FN_ATTRS256
_mm256_cvtsbf16_bf16(__m256bf16 __a) {
  return __a[0];
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_undefined_pbf16(void) {
  return (__m256bf16)__builtin_ia32_undef256();
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128 _mm_undefined_pbf16(void) {
  return (__m128bf16)__builtin_ia32_undef128();
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128 _mm_set_sbf16(__bf16 bf) {
  return (__v8bf)__builtin_shufflevector(
      (__v8bf){bf, bf, bf, bf, bf, bf, bf, bf}, (__v8bf)_mm_setzero_pbf16(), 0,
      8, 8, 8, 8, 8, 8, 8);
}

static __inline __m128bf16 __DEFAULT_FN_ATTRS128 _mm_set1_pbf16(__bf16 bf) {
  return (__m128bf16)(__v8bf){bf, bf, bf, bf, bf, bf, bf, bf};
}

static __inline __m256bf16 __DEFAULT_FN_ATTRS256 _mm256_set1_pbf16(__bf16 bf) {
  return (__m256bf16)(__v16bf){bf, bf, bf, bf, bf, bf, bf, bf,
                               bf, bf, bf, bf, bf, bf, bf, bf};
}

static __inline __m128bf16 __DEFAULT_FN_ATTRS128
_mm_set_pbf16(__bf16 bf1, __bf16 bf2, __bf16 bf3, __bf16 bf4, __bf16 bf5,
              __bf16 bf6, __bf16 bf7, __bf16 bf8) {
  return (__m128bf16)(__v8bf){bf1, bf2, bf3, bf4, bf5, bf6, bf7, bf8};
}

static __inline __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_set_pbf16(__bf16 bf1, __bf16 bf2, __bf16 bf3, __bf16 bf4, __bf16 bf5,
                 __bf16 bf6, __bf16 bf7, __bf16 bf8, __bf16 bf9, __bf16 bf10,
                 __bf16 bf11, __bf16 bf12, __bf16 bf13, __bf16 bf14,
                 __bf16 bf15, __bf16 bf16) {
  return (__m256bf16)(__v16bf){bf1, bf2,  bf3,  bf4,  bf5,  bf6,  bf7,  bf8,
                               bf9, bf10, bf11, bf12, bf13, bf14, bf15, bf16};
}

#define _mm_setr_pbf16(bf1, bf2, bf3, bf4, bf5, bf6, bf7, bf8)                 \
  _mm_set_pbf16((bf8), (bf7), (bf6), (bf5), (bf4), (bf3), (bf2), (bf1))

#define _mm256_setr_pbf16(bf1, bf2, bf3, bf4, bf5, bf6, bf7, bf8, bf9, bf10,   \
                          bf11, bf12, bf13, bf14, bf15, bf16)                  \
  _mm256_set_pbf16((bf16), (bf15), (bf14), (bf13), (bf12), (bf11), (bf10),     \
                   (bf9), (bf8), (bf7), (bf6), (bf5), (bf4), (bf3), (bf2),     \
                   (bf1))

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_abs_pbf16(__m256bf16 __A) {
  return (__m256bf16)_mm256_and_epi32(_mm256_set1_epi32(0x7FFF7FFF),
                                      (__m256i)__A);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_abs_pbf16(__m128bf16 __A) {
  return (__m128bf16)_mm_and_epi32(_mm_set1_epi32(0x7FFF7FFF), (__m128i)__A);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_blend_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __W) {
  return (__m128bf16)__builtin_ia32_selectpbf_128((__mmask8)__U, (__v8bf)__W,
                                                    (__v8bf)__A);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask_blend_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __W) {
  return (__m256bf16)__builtin_ia32_selectpbf_256((__mmask16)__U,
                                                    (__v16bf)__W, (__v16bf)__A);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_permutex2var_pbf16(__m128bf16 __A, __m128i __I, __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_vpermi2varhi128((__v8hi)__A, (__v8hi)__I,
                                                    (__v8hi)__B);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_permutex2var_pbf16(__m256bf16 __A, __m256i __I, __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_vpermi2varhi256((__v16hi)__A, (__v16hi)__I,
                                                    (__v16hi)__B);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_permutexvar_pbf16(__m128i __A, __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_permvarhi128((__v8hi)__B, (__v8hi)__A);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_permutexvar_pbf16(__m256i __A, __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_permvarhi256((__v16hi)__B, (__v16hi)__A);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_addne_pbf16(__m256bf16 __A, __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_vaddnepbf16256((__v16bf)__A, (__v16bf)__B);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask_addne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A,
                        __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, (__v16bf)_mm256_addne_pbf16(__A, __B), (__v16bf)__W);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_maskz_addne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, (__v16bf)_mm256_addne_pbf16(__A, __B),
      (__v16bf)_mm256_setzero_pbf16());
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_addne_pbf16(__m128bf16 __A, __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_vaddnepbf16128((__v8bf)__A, (__v8bf)__B);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_addne_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A,
                     __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, (__v8bf)_mm_addne_pbf16(__A, __B), (__v8bf)__W);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maskz_addne_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, (__v8bf)_mm_addne_pbf16(__A, __B),
      (__v8bf)_mm_setzero_pbf16());
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_subne_pbf16(__m256bf16 __A, __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_vsubnepbf16256((__v16bf)__A, (__v16bf)__B);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask_subne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A,
                        __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, (__v16bf)_mm256_subne_pbf16(__A, __B), (__v16bf)__W);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_maskz_subne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, (__v16bf)_mm256_subne_pbf16(__A, __B),
      (__v16bf)_mm256_setzero_pbf16());
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_subne_pbf16(__m128bf16 __A, __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_vsubnepbf16128((__v8bf)__A, (__v8bf)__B);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_subne_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A,
                     __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, (__v8bf)_mm_subne_pbf16(__A, __B), (__v8bf)__W);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maskz_subne_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, (__v8bf)_mm_subne_pbf16(__A, __B),
      (__v8bf)_mm_setzero_pbf16());
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mulne_pbf16(__m256bf16 __A, __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_vmulnepbf16256((__v16bf)__A, (__v16bf)__B);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask_mulne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A,
                        __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, (__v16bf)_mm256_mulne_pbf16(__A, __B), (__v16bf)__W);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_maskz_mulne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, (__v16bf)_mm256_mulne_pbf16(__A, __B),
      (__v16bf)_mm256_setzero_pbf16());
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mulne_pbf16(__m128bf16 __A, __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_vmulnepbf16128((__v8bf)__A, (__v8bf)__B);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_mulne_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A,
                     __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, (__v8bf)_mm_mulne_pbf16(__A, __B), (__v8bf)__W);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maskz_mulne_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, (__v8bf)_mm_mulne_pbf16(__A, __B),
      (__v8bf)_mm_setzero_pbf16());
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_divne_pbf16(__m256bf16 __A, __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_vdivnepbf16256((__v16bf)__A, (__v16bf)__B);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask_divne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A,
                        __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, (__v16bf)_mm256_divne_pbf16(__A, __B), (__v16bf)__W);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_maskz_divne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, (__v16bf)_mm256_divne_pbf16(__A, __B),
      (__v16bf)_mm256_setzero_pbf16());
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_divne_pbf16(__m128bf16 __A, __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_vdivnepbf16128((__v8bf)__A, (__v8bf)__B);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_divne_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A,
                     __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, (__v8bf)_mm_divne_pbf16(__A, __B), (__v8bf)__W);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maskz_divne_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, (__v8bf)_mm_divne_pbf16(__A, __B),
      (__v8bf)_mm_setzero_pbf16());
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_maxne_pbf16(__m256bf16 __A, __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_vmaxnepbf16256((__v16bf)__A, (__v16bf)__B);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask_maxne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A,
                        __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, (__v16bf)_mm256_maxne_pbf16(__A, __B), (__v16bf)__W);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_maskz_maxne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, (__v16bf)_mm256_maxne_pbf16(__A, __B),
      (__v16bf)_mm256_setzero_pbf16());
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maxne_pbf16(__m128bf16 __A, __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_vmaxnepbf16128((__v8bf)__A, (__v8bf)__B);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_maxne_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A,
                     __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, (__v8bf)_mm_maxne_pbf16(__A, __B), (__v8bf)__W);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maskz_maxne_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, (__v8bf)_mm_maxne_pbf16(__A, __B),
      (__v8bf)_mm_setzero_pbf16());
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_minne_pbf16(__m256bf16 __A, __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_vminnepbf16256((__v16bf)__A, (__v16bf)__B);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask_minne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A,
                        __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, (__v16bf)_mm256_minne_pbf16(__A, __B), (__v16bf)__W);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_maskz_minne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, (__v16bf)_mm256_minne_pbf16(__A, __B),
      (__v16bf)_mm256_setzero_pbf16());
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_minne_pbf16(__m128bf16 __A, __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_vminnepbf16128((__v8bf)__A, (__v8bf)__B);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_minne_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A,
                     __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, (__v8bf)_mm_minne_pbf16(__A, __B), (__v8bf)__W);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maskz_minne_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, (__v8bf)_mm_minne_pbf16(__A, __B),
      (__v8bf)_mm_setzero_pbf16());
}

#define _mm256_cmpne_pbf16_mask(A, B, P)                                       \
  ((__mmask16)__builtin_ia32_vcmpnepbf16256_mask((__v16bf)(__m256bf16)(A),     \
                                                 (__v16bf)(__m256bf16)(B),     \
                                                 (int)(P), (__mmask16)-1))

#define _mm256_mask_cmpne_pbf16_mask(U, A, B, P)                               \
  ((__mmask16)__builtin_ia32_vcmpnepbf16256_mask((__v16bf)(__m256bf16)(A),     \
                                                 (__v16bf)(__m256bf16)(B),     \
                                                 (int)(P), (__mmask16)(U)))

#define _mm_cmpne_pbf16_mask(A, B, P)                                          \
  ((__mmask8)__builtin_ia32_vcmpnepbf16128_mask((__v8bf)(__m128bf16)(A),       \
                                                (__v8bf)(__m128bf16)(B),       \
                                                (int)(P), (__mmask8)-1))

#define _mm_mask_cmpne_pbf16_mask(U, A, B, P)                                  \
  ((__mmask8)__builtin_ia32_vcmpnepbf16128_mask((__v8bf)(__m128bf16)(A),       \
                                                (__v8bf)(__m128bf16)(B),       \
                                                (int)(P), (__mmask8)(U)))

#define _mm256_mask_fpclassne_pbf16_mask(U, A, imm)                            \
  ((__mmask16)__builtin_ia32_vfpclassnepbf16256_mask(                          \
      (__v16bf)(__m256bf16)(A), (int)(imm), (__mmask16)(U)))

#define _mm256_fpclassne_pbf16_mask(A, imm)                                    \
  ((__mmask16)__builtin_ia32_vfpclassnepbf16256_mask(                          \
      (__v16bf)(__m256bf16)(A), (int)(imm), (__mmask16)-1))

#define _mm_mask_fpclassne_pbf16_mask(U, A, imm)                               \
  ((__mmask8)__builtin_ia32_vfpclassnepbf16128_mask(                           \
      (__v8bf)(__m128bf16)(A), (int)(imm), (__mmask8)(U)))

#define _mm_fpclassne_pbf16_mask(A, imm)                                       \
  ((__mmask8)__builtin_ia32_vfpclassnepbf16128_mask((__v8bf)(__m128bf16)(A),   \
                                                    (int)(imm), (__mmask8)-1))

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_scalefne_pbf16(__m256bf16 __A, __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_vscalefnepbf16256_mask(
      (__v16bf)__A, (__v16bf)__B, (__v16bf)_mm256_undefined_pbf16(),
      (__mmask16)-1);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask_scalefne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A,
                           __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_vscalefnepbf16256_mask(
      (__v16bf)__A, (__v16bf)__B, (__v16bf)__W, (__mmask16)__U);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_maskz_scalefne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  return (__m256bf16)__builtin_ia32_vscalefnepbf16256_mask(
      (__v16bf)__A, (__v16bf)__B, (__v16bf)_mm256_setzero_pbf16(),
      (__mmask16)__U);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_scalefne_pbf16(__m128bf16 __A, __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_vscalefnepbf16128_mask(
      (__v8bf)__A, (__v8bf)__B, (__v8bf)_mm_undefined_pbf16(), (__mmask8)-1);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_scalefne_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A,
                        __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_vscalefnepbf16128_mask(
      (__v8bf)__A, (__v8bf)__B, (__v8bf)__W, (__mmask8)__U);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maskz_scalefne_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B) {
  return (__m128bf16)__builtin_ia32_vscalefnepbf16128_mask(
      (__v8bf)__A, (__v8bf)__B, (__v8bf)_mm_setzero_pbf16(), (__mmask8)__U);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_rcpne_pbf16(__m256bf16 __A) {
  return (__m256bf16)__builtin_ia32_vrcpnepbf16256_mask(
      (__v16bf)__A, (__v16bf)_mm256_undefined_pbf16(), (__mmask16)-1);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask_rcpne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A) {
  return (__m256bf16)__builtin_ia32_vrcpnepbf16256_mask(
      (__v16bf)__A, (__v16bf)__W, (__mmask16)__U);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_maskz_rcpne_pbf16(__mmask16 __U, __m256bf16 __A) {
  return (__m256bf16)__builtin_ia32_vrcpnepbf16256_mask(
      (__v16bf)__A, (__v16bf)_mm256_setzero_pbf16(), (__mmask16)__U);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_rcpne_pbf16(__m128bf16 __A) {
  return (__m128bf16)__builtin_ia32_vrcpnepbf16128_mask(
      (__v8bf)__A, (__v8bf)_mm_undefined_pbf16(), (__mmask8)-1);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_rcpne_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A) {
  return (__m128bf16)__builtin_ia32_vrcpnepbf16128_mask(
      (__v8bf)__A, (__v8bf)__W, (__mmask8)__U);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maskz_rcpne_pbf16(__mmask8 __U, __m128bf16 __A) {
  return (__m128bf16)__builtin_ia32_vrcpnepbf16128_mask(
      (__v8bf)__A, (__v8bf)_mm_setzero_pbf16(), (__mmask8)__U);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_getexpne_pbf16(__m256bf16 __A) {
  return (__m256bf16)__builtin_ia32_vgetexpnepbf16256_mask(
      (__v16bf)__A, (__v16bf)_mm256_undefined_pbf16(), (__mmask16)-1);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask_getexpne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A) {
  return (__m256bf16)__builtin_ia32_vgetexpnepbf16256_mask(
      (__v16bf)__A, (__v16bf)__W, (__mmask16)__U);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_maskz_getexpne_pbf16(__mmask16 __U, __m256bf16 __A) {
  return (__m256bf16)__builtin_ia32_vgetexpnepbf16256_mask(
      (__v16bf)__A, (__v16bf)_mm256_setzero_pbf16(), (__mmask16)__U);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_getexpne_pbf16(__m128bf16 __A) {
  return (__m128bf16)__builtin_ia32_vgetexpnepbf16128_mask(
      (__v8bf)__A, (__v8bf)_mm_undefined_pbf16(), (__mmask8)-1);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_getexpne_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A) {
  return (__m128bf16)__builtin_ia32_vgetexpnepbf16128_mask(
      (__v8bf)__A, (__v8bf)__W, (__mmask8)__U);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maskz_getexpne_pbf16(__mmask8 __U, __m128bf16 __A) {
  return (__m128bf16)__builtin_ia32_vgetexpnepbf16128_mask(
      (__v8bf)__A, (__v8bf)_mm_setzero_pbf16(), (__mmask8)__U);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_rsqrtne_pbf16(__m256bf16 __A) {
  return (__m256bf16)__builtin_ia32_vrsqrtnepbf16256_mask(
      (__v16bf)__A, (__v16bf)_mm256_undefined_pbf16(), (__mmask16)-1);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask_rsqrtne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A) {
  return (__m256bf16)__builtin_ia32_vrsqrtnepbf16256_mask(
      (__v16bf)__A, (__v16bf)__W, (__mmask16)__U);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_maskz_rsqrtne_pbf16(__mmask16 __U, __m256bf16 __A) {
  return (__m256bf16)__builtin_ia32_vrsqrtnepbf16256_mask(
      (__v16bf)__A, (__v16bf)_mm256_setzero_pbf16(), (__mmask16)__U);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_rsqrtne_pbf16(__m128bf16 __A) {
  return (__m128bf16)__builtin_ia32_vrsqrtnepbf16128_mask(
      (__v8bf)__A, (__v8bf)_mm_undefined_pbf16(), (__mmask8)-1);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_rsqrtne_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A) {
  return (__m128bf16)__builtin_ia32_vrsqrtnepbf16128_mask(
      (__v8bf)__A, (__v8bf)__W, (__mmask8)__U);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maskz_rsqrtne_pbf16(__mmask8 __U, __m128bf16 __A) {
  return (__m128bf16)__builtin_ia32_vrsqrtnepbf16128_mask(
      (__v8bf)__A, (__v8bf)_mm_setzero_pbf16(), (__mmask8)__U);
}

#define _mm256_reducene_pbf16(A, imm)                                          \
  ((__m256bf16)__builtin_ia32_vreducenepbf16256_mask(                          \
      (__v16bf)(__m256bf16)(A), (int)(imm), (__v16bf)_mm256_undefined_pbf16(), \
      (__mmask16)-1))

#define _mm256_mask_reducene_pbf16(W, U, A, imm)                               \
  ((__m256bf16)__builtin_ia32_vreducenepbf16256_mask(                          \
      (__v16bf)(__m256bf16)(A), (int)(imm), (__v16bf)(__m256bf16)(W),          \
      (__mmask16)(U)))

#define _mm256_maskz_reducene_pbf16(U, A, imm)                                 \
  ((__m256bf16)__builtin_ia32_vreducenepbf16256_mask(                          \
      (__v16bf)(__m256bf16)(A), (int)(imm), (__v16bf)_mm256_setzero_pbf16(),   \
      (__mmask16)(U)))

#define _mm_reducene_pbf16(A, imm)                                             \
  ((__m128bf16)__builtin_ia32_vreducenepbf16128_mask(                          \
      (__v8bf)(__m128bf16)(A), (int)(imm), (__v8bf)_mm_undefined_pbf16(),      \
      (__mmask8)-1))

#define _mm_mask_reducene_pbf16(W, U, A, imm)                                  \
  ((__m128bf16)__builtin_ia32_vreducenepbf16128_mask(                          \
      (__v8bf)(__m128bf16)(A), (int)(imm), (__v8bf)(__m128bf16)(W),            \
      (__mmask8)(U)))

#define _mm_maskz_reducene_pbf16(U, A, imm)                                    \
  ((__m128bf16)__builtin_ia32_vreducenepbf16128_mask(                          \
      (__v8bf)(__m128bf16)(A), (int)(imm), (__v8bf)_mm_setzero_pbf16(),        \
      (__mmask8)(U)))

#define _mm256_roundscalene_pbf16(A, B)                                        \
  ((__m256bf16)__builtin_ia32_vrndscalenepbf16_256_mask(                       \
      (__v16bf)(__m256bf16)(A), (int)(B), (__v16bf)(__m256bf16)(A),            \
      (__mmask16)-1))

#define _mm256_mask_roundscalene_pbf16(A, B, C, imm)                           \
  ((__m256bf16)__builtin_ia32_vrndscalenepbf16_256_mask(                       \
      (__v16bf)(__m256bf16)(C), (int)(imm), (__v16bf)(__m256bf16)(A),          \
      (__mmask16)(B)))

#define _mm256_maskz_roundscalene_pbf16(A, B, imm)                             \
  ((__m256bf16)__builtin_ia32_vrndscalenepbf16_256_mask(                       \
      (__v16bf)(__m256bf16)(B), (int)(imm), (__v16bf)_mm256_setzero_pbf16(),   \
      (__mmask16)(A)))

#define _mm_roundscalene_pbf16(A, B)                                           \
  ((__m128bf16)__builtin_ia32_vrndscalenepbf16_128_mask(                       \
      (__v8bf)(__m128bf16)(A), (int)(B), (__v8bf)(__m128bf16)(A),              \
      (__mmask8)-1))

#define _mm_mask_roundscalene_pbf16(A, B, C, imm)                              \
  ((__m128bf16)__builtin_ia32_vrndscalenepbf16_128_mask(                       \
      (__v8bf)(__m128bf16)(C), (int)(imm), (__v8bf)(__m128bf16)(A),            \
      (__mmask8)(B)))

#define _mm_maskz_roundscalene_pbf16(A, B, imm)                                \
  ((__m128bf16)__builtin_ia32_vrndscalenepbf16_128_mask(                       \
      (__v8bf)(__m128bf16)(B), (int)(imm), (__v8bf)_mm_setzero_pbf16(),        \
      (__mmask8)(A)))

#define _mm256_getmantne_pbf16(A, B, C)                                        \
  ((__m256bf16)__builtin_ia32_vgetmantnepbf16256_mask(                         \
      (__v16bf)(__m256bf16)(A), (int)(((C) << 2) | (B)),                       \
      (__v16bf)_mm256_undefined_pbf16(), (__mmask16)-1))

#define _mm256_mask_getmantne_pbf16(W, U, A, B, C)                             \
  ((__m256bf16)__builtin_ia32_vgetmantnepbf16256_mask(                         \
      (__v16bf)(__m256bf16)(A), (int)(((C) << 2) | (B)),                       \
      (__v16bf)(__m256bf16)(W), (__mmask16)(U)))

#define _mm256_maskz_getmantne_pbf16(U, A, B, C)                               \
  ((__m256bf16)__builtin_ia32_vgetmantnepbf16256_mask(                         \
      (__v16bf)(__m256bf16)(A), (int)(((C) << 2) | (B)),                       \
      (__v16bf)_mm256_setzero_pbf16(), (__mmask16)(U)))

#define _mm_getmantne_pbf16(A, B, C)                                           \
  ((__m128bf16)__builtin_ia32_vgetmantnepbf16128_mask(                         \
      (__v8bf)(__m128bf16)(A), (int)(((C) << 2) | (B)),                        \
      (__v8bf)_mm_undefined_pbf16(), (__mmask8)-1))

#define _mm_mask_getmantne_pbf16(W, U, A, B, C)                                \
  ((__m128bf16)__builtin_ia32_vgetmantnepbf16128_mask(                         \
      (__v8bf)(__m128bf16)(A), (int)(((C) << 2) | (B)),                        \
      (__v8bf)(__m128bf16)(W), (__mmask8)(U)))

#define _mm_maskz_getmantne_pbf16(U, A, B, C)                                  \
  ((__m128bf16)__builtin_ia32_vgetmantnepbf16128_mask(                         \
      (__v8bf)(__m128bf16)(A), (int)(((C) << 2) | (B)),                        \
      (__v8bf)_mm_setzero_pbf16(), (__mmask8)(U)))

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_sqrtne_pbf16(__m256bf16 __A) {
  return (__m256bf16)__builtin_ia32_vsqrtnepbf16256_mask(
      (__v16bf)__A, (__v16bf)_mm256_undefined_pbf16(), (__mmask16)-1);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask_sqrtne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A) {
  return (__m256bf16)__builtin_ia32_vsqrtnepbf16256_mask(
      (__v16bf)__A, (__v16bf)__W, (__mmask16)__U);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_maskz_sqrtne_pbf16(__mmask16 __U, __m256bf16 __A) {
  return (__m256bf16)__builtin_ia32_vsqrtnepbf16256_mask(
      (__v16bf)__A, (__v16bf)_mm256_setzero_pbf16(), (__mmask16)__U);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_sqrtne_pbf16(__m128bf16 __A) {
  return (__m128bf16)__builtin_ia32_vsqrtnepbf16128_mask(
      (__v8bf)__A, (__v8bf)_mm_undefined_pbf16(), (__mmask8)-1);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_sqrtne_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A) {
  return (__m128bf16)__builtin_ia32_vsqrtnepbf16128_mask(
      (__v8bf)__A, (__v8bf)__W, (__mmask8)__U);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maskz_sqrtne_pbf16(__mmask8 __U, __m128bf16 __A) {
  return (__m128bf16)__builtin_ia32_vsqrtnepbf16128_mask(
      (__v8bf)__A, (__v8bf)_mm_setzero_pbf16(), (__mmask8)__U);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_fmaddne_pbf16(__m256bf16 __A, __m256bf16 __B, __m256bf16 __C) {
  return (__m256bf16)__builtin_ia32_vfmadd213nepbf16256(
      (__v16bf)__A, (__v16bf)__B, (__v16bf)__C);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask_fmaddne_pbf16(__m256bf16 __A, __mmask16 __U, __m256bf16 __B,
                          __m256bf16 __C) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, __builtin_ia32_vfmadd132nepbf16256(
                          (__v16bf)__A, (__v16bf)__C, (__v16bf)__B),
      (__v16bf)__A);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask3_fmaddne_pbf16(__m256bf16 __A, __m256bf16 __B, __m256bf16 __C,
                           __mmask16 __U) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, __builtin_ia32_vfmadd231nepbf16256(
                          (__v16bf)__C, (__v16bf)__A, (__v16bf)__B),
      (__v16bf)__A);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_maskz_fmaddne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B,
                           __m256bf16 __C) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, __builtin_ia32_vfmadd213nepbf16256(
                          (__v16bf)__A, (__v16bf)__B, (__v16bf)__C),
      (__v16bf)_mm256_setzero_pbf16());
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_fmsubne_pbf16(__m256bf16 __A, __m256bf16 __B, __m256bf16 __C) {
  return (__m256bf16)__builtin_ia32_vfmsub213nepbf16256(
      (__v16bf)__A, (__v16bf)__B, (__v16bf)__C);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask_fmsubne_pbf16(__m256bf16 __A, __mmask16 __U, __m256bf16 __B,
                          __m256bf16 __C) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, __builtin_ia32_vfmsub132nepbf16256(
                          (__v16bf)__A, (__v16bf)__C, (__v16bf)__B),
      (__v16bf)__A);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask3_fmsubne_pbf16(__m256bf16 __A, __m256bf16 __B, __m256bf16 __C,
                           __mmask16 __U) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, __builtin_ia32_vfmsub231nepbf16256(
                          (__v16bf)__C, (__v16bf)__A, (__v16bf)__B),
      (__v16bf)__A);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_maskz_fmsubne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B,
                           __m256bf16 __C) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, __builtin_ia32_vfmsub213nepbf16256(
                          (__v16bf)__A, (__v16bf)__B, (__v16bf)__C),
      (__v16bf)_mm256_setzero_pbf16());
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_fnmaddne_pbf16(__m256bf16 __A, __m256bf16 __B, __m256bf16 __C) {
  return (__m256bf16)__builtin_ia32_vfnmadd213nepbf16256(
      (__v16bf)__A, (__v16bf)__B, (__v16bf)__C);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask_fnmaddne_pbf16(__m256bf16 __A, __mmask16 __U, __m256bf16 __B,
                           __m256bf16 __C) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, __builtin_ia32_vfnmadd132nepbf16256(
                          (__v16bf)__A, (__v16bf)__C, (__v16bf)__B),
      (__v16bf)__A);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask3_fnmaddne_pbf16(__m256bf16 __A, __m256bf16 __B, __m256bf16 __C,
                            __mmask16 __U) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, __builtin_ia32_vfnmadd231nepbf16256(
                          (__v16bf)__C, (__v16bf)__A, (__v16bf)__B),
      (__v16bf)__A);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_maskz_fnmaddne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B,
                            __m256bf16 __C) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, __builtin_ia32_vfnmadd213nepbf16256(
                          (__v16bf)__A, (__v16bf)__B, (__v16bf)__C),
      (__v16bf)_mm256_setzero_pbf16());
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_fnmsubne_pbf16(__m256bf16 __A, __m256bf16 __B, __m256bf16 __C) {
  return (__m256bf16)__builtin_ia32_vfnmsub213nepbf16256(
      (__v16bf)__A, (__v16bf)__B, (__v16bf)__C);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask_fnmsubne_pbf16(__m256bf16 __A, __mmask16 __U, __m256bf16 __B,
                           __m256bf16 __C) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, __builtin_ia32_vfnmsub132nepbf16256(
                          (__v16bf)__A, (__v16bf)__C, (__v16bf)__B),
      (__v16bf)__A);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_mask3_fnmsubne_pbf16(__m256bf16 __A, __m256bf16 __B, __m256bf16 __C,
                            __mmask16 __U) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, __builtin_ia32_vfnmsub231nepbf16256(
                          (__v16bf)__C, (__v16bf)__A, (__v16bf)__B),
      (__v16bf)__A);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_maskz_fnmsubne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B,
                            __m256bf16 __C) {
  return (__m256bf16)__builtin_ia32_selectpbf_256(
      (__mmask16)__U, __builtin_ia32_vfnmsub213nepbf16256(
                          (__v16bf)__A, (__v16bf)__B, (__v16bf)__C),
      (__v16bf)_mm256_setzero_pbf16());
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_fmaddne_pbf16(__m128bf16 __A, __m128bf16 __B, __m128bf16 __C) {
  return (__m128bf16)__builtin_ia32_vfmadd213nepbf16128(
      (__v8bf)__A, (__v8bf)__B, (__v8bf)__C);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_fmaddne_pbf16(__m128bf16 __A, __mmask8 __U, __m128bf16 __B,
                       __m128bf16 __C) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U,
      __builtin_ia32_vfmadd132nepbf16128((__v8bf)__A, (__v8bf)__C, (__v8bf)__B),
      (__v8bf)__A);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask3_fmaddne_pbf16(__m128bf16 __A, __m128bf16 __B, __m128bf16 __C,
                        __mmask8 __U) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U,
      __builtin_ia32_vfmadd231nepbf16128((__v8bf)__C, (__v8bf)__A, (__v8bf)__B),
      (__v8bf)__A);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maskz_fmaddne_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B,
                        __m128bf16 __C) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U,
      __builtin_ia32_vfmadd213nepbf16128((__v8bf)__A, (__v8bf)__B, (__v8bf)__C),
      (__v8bf)_mm_setzero_pbf16());
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_fmsubne_pbf16(__m128bf16 __A, __m128bf16 __B, __m128bf16 __C) {
  return (__m128bf16)__builtin_ia32_vfmsub213nepbf16128(
      (__v8bf)__A, (__v8bf)__B, (__v8bf)__C);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_fmsubne_pbf16(__m128bf16 __A, __mmask8 __U, __m128bf16 __B,
                       __m128bf16 __C) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U,
      __builtin_ia32_vfmsub132nepbf16128((__v8bf)__A, (__v8bf)__C, (__v8bf)__B),
      (__v8bf)__A);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask3_fmsubne_pbf16(__m128bf16 __A, __m128bf16 __B, __m128bf16 __C,
                        __mmask8 __U) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U,
      __builtin_ia32_vfmsub231nepbf16128((__v8bf)__C, (__v8bf)__A, (__v8bf)__B),
      (__v8bf)__A);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maskz_fmsubne_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B,
                        __m128bf16 __C) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U,
      __builtin_ia32_vfmsub213nepbf16128((__v8bf)__A, (__v8bf)__B, (__v8bf)__C),
      (__v8bf)_mm_setzero_pbf16());
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_fnmaddne_pbf16(__m128bf16 __A, __m128bf16 __B, __m128bf16 __C) {
  return (__m128bf16)__builtin_ia32_vfnmadd213nepbf16128(
      (__v8bf)__A, (__v8bf)__B, (__v8bf)__C);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_fnmaddne_pbf16(__m128bf16 __A, __mmask8 __U, __m128bf16 __B,
                        __m128bf16 __C) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, __builtin_ia32_vfnmadd132nepbf16128(
                         (__v8bf)__A, (__v8bf)__C, (__v8bf)__B),
      (__v8bf)__A);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask3_fnmaddne_pbf16(__m128bf16 __A, __m128bf16 __B, __m128bf16 __C,
                         __mmask8 __U) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, __builtin_ia32_vfnmadd231nepbf16128(
                         (__v8bf)__C, (__v8bf)__A, (__v8bf)__B),
      (__v8bf)__A);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maskz_fnmaddne_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B,
                         __m128bf16 __C) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, __builtin_ia32_vfnmadd213nepbf16128(
                         (__v8bf)__A, (__v8bf)__B, (__v8bf)__C),
      (__v8bf)_mm_setzero_pbf16());
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_fnmsubne_pbf16(__m128bf16 __A, __m128bf16 __B, __m128bf16 __C) {
  return (__m128bf16)__builtin_ia32_vfnmsub213nepbf16128(
      (__v8bf)__A, (__v8bf)__B, (__v8bf)__C);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_fnmsubne_pbf16(__m128bf16 __A, __mmask8 __U, __m128bf16 __B,
                        __m128bf16 __C) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, __builtin_ia32_vfnmsub132nepbf16128(
                         (__v8bf)__A, (__v8bf)__C, (__v8bf)__B),
      (__v8bf)__A);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask3_fnmsubne_pbf16(__m128bf16 __A, __m128bf16 __B, __m128bf16 __C,
                         __mmask8 __U) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, __builtin_ia32_vfnmsub231nepbf16128(
                         (__v8bf)__C, (__v8bf)__A, (__v8bf)__B),
      (__v8bf)__A);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maskz_fnmsubne_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B,
                         __m128bf16 __C) {
  return (__m128bf16)__builtin_ia32_selectpbf_128(
      (__mmask8)__U, __builtin_ia32_vfnmsub213nepbf16128(
                         (__v8bf)__A, (__v8bf)__B, (__v8bf)__C),
      (__v8bf)_mm_setzero_pbf16());
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif
