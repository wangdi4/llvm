/*===----------- avx512bf16neintrin.h - AVX512-BF16-NE intrinsics ----------===
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
    "Never use <avx512bf16neintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512BF16NEINTRIN_H
#define __AVX512BF16NEINTRIN_H

/* Define the default attributes for the functions in this file. */
typedef __bf16 __v32bf __attribute__((__vector_size__(64), __aligned__(64)));
typedef __bf16 __m512bf16 __attribute__((__vector_size__(64), __aligned__(64)));
typedef __bf16 __m512bf16_u
    __attribute__((__vector_size__(64), __aligned__(1)));
typedef __bf16 __v8bf __attribute__((__vector_size__(16), __aligned__(16)));
typedef __bf16 __m128bf16 __attribute__((__vector_size__(16), __aligned__(16)));
typedef __bf16 __m128bf16_u
    __attribute__((__vector_size__(16), __aligned__(1)));
typedef __bf16 __v16bf __attribute__((__vector_size__(32), __aligned__(32)));
typedef __bf16 __m256bf16 __attribute__((__vector_size__(32), __aligned__(32)));
typedef __bf16 __m256bf16_u
    __attribute__((__vector_size__(32), __aligned__(1)));

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS512                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512bf16ne, avx512vl"),                         \
                 __min_vector_width__(512)))
#define __DEFAULT_FN_ATTRS256                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512bf16ne, avx512vl"),                         \
                 __min_vector_width__(256)))
#define __DEFAULT_FN_ATTRS128                                                  \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512bf16ne"),   \
                 __min_vector_width__(128)))

static __inline __m512bf16 __DEFAULT_FN_ATTRS512 _mm512_setzero_pbf16(void) {
  return __builtin_bit_cast(__m512bf16, _mm512_setzero_ps());
}

static __inline __m256bf16 __DEFAULT_FN_ATTRS256 _mm256_setzero_pbf16(void) {
  return __builtin_bit_cast(__m256bf16, _mm256_setzero_ps());
}

static __inline __m128bf16 __DEFAULT_FN_ATTRS128 _mm_setzero_pbf16(void) {
  return __builtin_bit_cast(__m128bf16, _mm_setzero_ps());
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_undefined_pbf16(void) {
  return (__m512bf16)__builtin_ia32_undef512();
}

static __inline __m512bf16 __DEFAULT_FN_ATTRS512 _mm512_set1_pbf16(__bf16 bf) {
  return (__m512bf16)(__v32bf){bf, bf, bf, bf, bf, bf, bf, bf, bf, bf, bf,
                               bf, bf, bf, bf, bf, bf, bf, bf, bf, bf, bf,
                               bf, bf, bf, bf, bf, bf, bf, bf, bf, bf};
}

static __inline __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_set_pbf16(__bf16 bf1, __bf16 bf2, __bf16 bf3, __bf16 bf4, __bf16 bf5,
                 __bf16 bf6, __bf16 bf7, __bf16 bf8, __bf16 bf9, __bf16 bf10,
                 __bf16 bf11, __bf16 bf12, __bf16 bf13, __bf16 bf14,
                 __bf16 bf15, __bf16 bf16, __bf16 bf17, __bf16 bf18,
                 __bf16 bf19, __bf16 bf20, __bf16 bf21, __bf16 bf22,
                 __bf16 bf23, __bf16 bf24, __bf16 bf25, __bf16 bf26,
                 __bf16 bf27, __bf16 bf28, __bf16 bf29, __bf16 bf30,
                 __bf16 bf31, __bf16 bf32) {
  return (__m512bf16)(__v32bf){bf32, bf31, bf30, bf29, bf28, bf27, bf26, bf25,
                               bf24, bf23, bf22, bf21, bf20, bf19, bf18, bf17,
                               bf16, bf15, bf14, bf13, bf12, bf11, bf10, bf9,
                               bf8,  bf7,  bf6,  bf5,  bf4,  bf3,  bf2,  bf1};
}

#define _mm512_setr_pbf16(bf1, bf2, bf3, bf4, bf5, bf6, bf7, bf8, bf9, bf10,   \
                          bf11, bf12, bf13, bf14, bf15, bf16, bf17, bf18,      \
                          bf19, bf20, bf21, bf22, bf23, bf24, bf25, bf26,      \
                          bf27, bf28, bf29, bf30, bf31, bf32)                  \
  _mm512_set_pbf16((bf32), (bf31), (bf30), (bf29), (bf28), (bf27), (bf26),     \
                   (bf25), (bf24), (bf23), (bf22), (bf21), (bf20), (bf19),     \
                   (bf18), (bf17), (bf16), (bf15), (bf14), (bf13), (bf12),     \
                   (bf11), (bf10), (bf9), (bf8), (bf7), (bf6), (bf5), (bf4),   \
                   (bf3), (bf2), (bf1))

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_castpbf16_ps(__m128bf16 __a) {
  return (__m128)__a;
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_castpbf16_ps(__m256bf16 __a) {
  return (__m256)__a;
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_castpbf16_ps(__m512bf16 __a) {
  return (__m512)__a;
}

static __inline__ __m128d __DEFAULT_FN_ATTRS128
_mm_castpbf16_pd(__m128bf16 __a) {
  return (__m128d)__a;
}

static __inline__ __m256d __DEFAULT_FN_ATTRS256
_mm256_castpbf16_pd(__m256bf16 __a) {
  return (__m256d)__a;
}

static __inline__ __m512d __DEFAULT_FN_ATTRS512
_mm512_castpbf16_pd(__m512bf16 __a) {
  return (__m512d)__a;
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_castpbf16_si128(__m128bf16 __a) {
  return (__m128i)__a;
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_castpbf16_si256(__m256bf16 __a) {
  return (__m256i)__a;
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_castpbf16_si512(__m512bf16 __a) {
  return (__m512i)__a;
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_castps_pbf16(__m128 __a) {
  return (__m128bf16)__a;
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_castps_pbf16(__m256 __a) {
  return (__m256bf16)__a;
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_castps_pbf16(__m512 __a) {
  return (__m512bf16)__a;
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_castpd_pbf16(__m128d __a) {
  return (__m128bf16)__a;
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_castpd_pbf16(__m256d __a) {
  return (__m256bf16)__a;
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_castpd_pbf16(__m512d __a) {
  return (__m512bf16)__a;
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_castsi128_pbf16(__m128i __a) {
  return (__m128bf16)__a;
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_castsi256_pbf16(__m256i __a) {
  return (__m256bf16)__a;
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_castsi512_pbf16(__m512i __a) {
  return (__m512bf16)__a;
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS256
_mm256_castpbf16256_pbf16128(__m256bf16 __a) {
  return __builtin_shufflevector(__a, __a, 0, 1, 2, 3, 4, 5, 6, 7);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS512
_mm512_castpbf16512_pbf16128(__m512bf16 __a) {
  return __builtin_shufflevector(__a, __a, 0, 1, 2, 3, 4, 5, 6, 7);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS512
_mm512_castpbf16512_pbf16256(__m512bf16 __a) {
  return __builtin_shufflevector(__a, __a, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
                                 12, 13, 14, 15);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_castpbf16128_pbf16256(__m128bf16 __a) {
  return __builtin_shufflevector(__a, __a, 0, 1, 2, 3, 4, 5, 6, 7, -1, -1, -1,
                                 -1, -1, -1, -1, -1);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_castpbf16128_pbf16512(__m128bf16 __a) {
  return __builtin_shufflevector(__a, __a, 0, 1, 2, 3, 4, 5, 6, 7, -1, -1, -1,
                                 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                 -1, -1, -1, -1, -1, -1, -1, -1, -1);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_castpbf16256_pbf16512(__m256bf16 __a) {
  return __builtin_shufflevector(__a, __a, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
                                 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1,
                                 -1, -1, -1, -1, -1, -1, -1, -1);
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_zextpbf16128_pbf16256(__m128bf16 __a) {
  return __builtin_shufflevector(__a, (__v8bf)_mm_setzero_pbf16(), 0, 1, 2, 3,
                                 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_zextpbf16128_pbf16512(__m128bf16 __a) {
  return __builtin_shufflevector(__a, (__v8bf)_mm_setzero_pbf16(), 0, 1, 2, 3,
                                 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 8, 9,
                                 10, 11, 12, 13, 14, 15, 8, 9, 10, 11, 12, 13,
                                 14, 15);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_zextpbf16256_pbf16512(__m256bf16 __a) {
  return __builtin_shufflevector(__a, (__v16bf)_mm256_setzero_pbf16(), 0, 1, 2,
                                 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                                 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
                                 28, 29, 30, 31);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_abs_pbf16(__m512bf16 __A) {
  return (__m512bf16)_mm512_and_epi32(_mm512_set1_epi32(0x7FFF7FFF),
                                      (__m512i)__A);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_load_sbf16(void const *__dp) {
  __m128bf16 src = (__v8bf)_mm_setzero_pbf16();
  return (__m128bf16)__builtin_ia32_loadsbf16128_mask((__v8bf *)__dp, src, 1);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_load_sbf16(__m128bf16 __W, __mmask8 __U, const void *__A) {
  __m128bf16 src = (__v8bf)__builtin_shufflevector(
      (__v8bf)__W, (__v8bf)_mm_setzero_pbf16(), 0, 8, 8, 8, 8, 8, 8, 8);

  return (__m128bf16)__builtin_ia32_loadsbf16128_mask((__v8bf *)__A, src,
                                                      __U & 1);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maskz_load_sbf16(__mmask8 __U, const void *__A) {
  return (__m128bf16)__builtin_ia32_loadsbf16128_mask(
      (__v8bf *)__A, (__v8bf)_mm_setzero_pbf16(), __U & 1);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_load_pbf16(void const *__p) {
  return *(const __m512bf16 *)__p;
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_load_pbf16(void const *__p) {
  return *(const __m256bf16 *)__p;
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_load_pbf16(void const *__p) {
  return *(const __m128bf16 *)__p;
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_loadu_pbf16(void const *__p) {
  struct __loadu_pbf16 {
    __m512bf16_u __v;
  } __attribute__((__packed__, __may_alias__));
  return ((const struct __loadu_pbf16 *)__p)->__v;
}

static __inline__ __m256bf16 __DEFAULT_FN_ATTRS256
_mm256_loadu_pbf16(void const *__p) {
  struct __loadu_pbf16 {
    __m256bf16_u __v;
  } __attribute__((__packed__, __may_alias__));
  return ((const struct __loadu_pbf16 *)__p)->__v;
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_loadu_pbf16(void const *__p) {
  struct __loadu_pbf16 {
    __m128bf16_u __v;
  } __attribute__((__packed__, __may_alias__));
  return ((const struct __loadu_pbf16 *)__p)->__v;
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_store_sbf16(void *__dp, __m128bf16 __a) {
  struct __mm_store_sbf16_struct {
    __bf16 __u;
  } __attribute__((__packed__, __may_alias__));
  ((struct __mm_store_sbf16_struct *)__dp)->__u = __a[0];
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_mask_store_sbf16(void *__W, __mmask8 __U, __m128bf16 __A) {
  __builtin_ia32_storesbf16128_mask((__v8bf *)__W, __A, __U & 1);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_store_pbf16(void *__P, __m512bf16 __A) {
  *(__m512bf16 *)__P = __A;
}

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_store_pbf16(void *__P, __m256bf16 __A) {
  *(__m256bf16 *)__P = __A;
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_store_pbf16(void *__P, __m128bf16 __A) {
  *(__m128bf16 *)__P = __A;
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_storeu_pbf16(void *__P, __m512bf16 __A) {
  struct __storeu_pbf16 {
    __m512bf16_u __v;
  } __attribute__((__packed__, __may_alias__));
  ((struct __storeu_pbf16 *)__P)->__v = __A;
}

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_storeu_pbf16(void *__P, __m256bf16 __A) {
  struct __storeu_pbf16 {
    __m256bf16_u __v;
  } __attribute__((__packed__, __may_alias__));
  ((struct __storeu_pbf16 *)__P)->__v = __A;
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_storeu_pbf16(void *__P, __m128bf16 __A) {
  struct __storeu_pbf16 {
    __m128bf16_u __v;
  } __attribute__((__packed__, __may_alias__));
  ((struct __storeu_pbf16 *)__P)->__v = __A;
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_move_sbf16(__m128bf16 __a, __m128bf16 __b) {
  __a[0] = __b[0];
  return __a;
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_mask_move_sbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A,
                    __m128bf16 __B) {
  return __builtin_ia32_selectsbf_128(__U, _mm_move_sbf16(__A, __B), __W);
}

static __inline__ __m128bf16 __DEFAULT_FN_ATTRS128
_mm_maskz_move_sbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B) {
  return __builtin_ia32_selectsbf_128(__U, _mm_move_sbf16(__A, __B),
                                        _mm_setzero_pbf16());
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask_blend_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __W) {
  return (__m512bf16)__builtin_ia32_selectpbf_512((__mmask32)__U,
                                                    (__v32bf)__W, (__v32bf)__A);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_permutex2var_pbf16(__m512bf16 __A, __m512i __I, __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_vpermi2varhi512((__v32hi)__A, (__v32hi)__I,
                                                    (__v32hi)__B);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_permutexvar_pbf16(__m512i __A, __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_permvarhi512((__v32hi)__B, (__v32hi)__A);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_addne_pbf16(__m512bf16 __A, __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_vaddnepbf16512((__v32bf)__A, (__v32bf)__B);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask_addne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A,
                        __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, (__v32bf)_mm512_addne_pbf16(__A, __B), (__v32bf)__W);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_maskz_addne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, (__v32bf)_mm512_addne_pbf16(__A, __B),
      (__v32bf)_mm512_setzero_pbf16());
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_subne_pbf16(__m512bf16 __A, __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_vsubnepbf16512((__v32bf)__A, (__v32bf)__B);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask_subne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A,
                        __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, (__v32bf)_mm512_subne_pbf16(__A, __B), (__v32bf)__W);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_maskz_subne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, (__v32bf)_mm512_subne_pbf16(__A, __B),
      (__v32bf)_mm512_setzero_pbf16());
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mulne_pbf16(__m512bf16 __A, __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_vmulnepbf16512((__v32bf)__A, (__v32bf)__B);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask_mulne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A,
                        __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, (__v32bf)_mm512_mulne_pbf16(__A, __B), (__v32bf)__W);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_maskz_mulne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, (__v32bf)_mm512_mulne_pbf16(__A, __B),
      (__v32bf)_mm512_setzero_pbf16());
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_divne_pbf16(__m512bf16 __A, __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_vdivnepbf16512((__v32bf)__A, (__v32bf)__B);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask_divne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A,
                        __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, (__v32bf)_mm512_divne_pbf16(__A, __B), (__v32bf)__W);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_maskz_divne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, (__v32bf)_mm512_divne_pbf16(__A, __B),
      (__v32bf)_mm512_setzero_pbf16());
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_maxne_pbf16(__m512bf16 __A, __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_vmaxnepbf16512((__v32bf)__A, (__v32bf)__B);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask_maxne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A,
                        __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, (__v32bf)_mm512_maxne_pbf16(__A, __B), (__v32bf)__W);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_maskz_maxne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, (__v32bf)_mm512_maxne_pbf16(__A, __B),
      (__v32bf)_mm512_setzero_pbf16());
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_minne_pbf16(__m512bf16 __A, __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_vminnepbf16512((__v32bf)__A, (__v32bf)__B);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask_minne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A,
                        __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, (__v32bf)_mm512_minne_pbf16(__A, __B), (__v32bf)__W);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_maskz_minne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, (__v32bf)_mm512_minne_pbf16(__A, __B),
      (__v32bf)_mm512_setzero_pbf16());
}

static __inline__ int __DEFAULT_FN_ATTRS128
_mm_comeqne_sbf16(__m128bf16 A, __m128bf16 B) {
  return __builtin_ia32_vcomnesbf16eq((__v8bf)A, (__v8bf)B);
}

static __inline__ int __DEFAULT_FN_ATTRS128
_mm_comltne_sbf16(__m128bf16 A, __m128bf16 B) {
  return __builtin_ia32_vcomnesbf16lt((__v8bf)A, (__v8bf)B);
}

static __inline__ int __DEFAULT_FN_ATTRS128
_mm_comlene_sbf16(__m128bf16 A, __m128bf16 B) {
  return __builtin_ia32_vcomnesbf16le((__v8bf)A, (__v8bf)B);
}

static __inline__ int __DEFAULT_FN_ATTRS128
_mm_comgtne_sbf16(__m128bf16 A, __m128bf16 B) {
  return __builtin_ia32_vcomnesbf16gt((__v8bf)A, (__v8bf)B);
}

static __inline__ int __DEFAULT_FN_ATTRS128
_mm_comgene_sbf16(__m128bf16 A, __m128bf16 B) {
  return __builtin_ia32_vcomnesbf16ge((__v8bf)A, (__v8bf)B);
}

static __inline__ int __DEFAULT_FN_ATTRS128
_mm_comneqne_sbf16(__m128bf16 A, __m128bf16 B) {
  return __builtin_ia32_vcomnesbf16neq((__v8bf)A, (__v8bf)B);
}

#define _mm512_cmpne_pbf16_mask(A, B, P)                                       \
  ((__mmask32)__builtin_ia32_vcmpnepbf16512_mask((__v32bf)(__m512bf16)(A),     \
                                                 (__v32bf)(__m512bf16)(B),     \
                                                 (int)(P), (__mmask32)-1))

#define _mm512_mask_cmpne_pbf16_mask(U, A, B, P)                               \
  ((__mmask32)__builtin_ia32_vcmpnepbf16512_mask((__v32bf)(__m512bf16)(A),     \
                                                 (__v32bf)(__m512bf16)(B),     \
                                                 (int)(P), (__mmask32)(U)))

#define _mm512_mask_fpclassne_pbf16_mask(U, A, imm)                            \
  ((__mmask32)__builtin_ia32_vfpclassnepbf16512_mask(                          \
      (__v32bf)(__m512bf16)(A), (int)(imm), (__mmask32)(U)))

#define _mm512_fpclassne_pbf16_mask(A, imm)                                    \
  ((__mmask32)__builtin_ia32_vfpclassnepbf16512_mask(                          \
      (__v32bf)(__m512bf16)(A), (int)(imm), (__mmask32)-1))

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_scalefne_pbf16(__m512bf16 __A, __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_vscalefnepbf16512_mask(
      (__v32bf)__A, (__v32bf)__B, (__v32bf)_mm512_undefined_pbf16(),
      (__mmask32)-1);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask_scalefne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A,
                           __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_vscalefnepbf16512_mask(
      (__v32bf)__A, (__v32bf)__B, (__v32bf)__W, (__mmask32)__U);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_maskz_scalefne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  return (__m512bf16)__builtin_ia32_vscalefnepbf16512_mask(
      (__v32bf)__A, (__v32bf)__B, (__v32bf)_mm512_setzero_pbf16(),
      (__mmask32)__U);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_rcpne_pbf16(__m512bf16 __A) {
  return (__m512bf16)__builtin_ia32_vrcpnepbf16512_mask(
      (__v32bf)__A, (__v32bf)_mm512_undefined_pbf16(), (__mmask32)-1);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask_rcpne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A) {
  return (__m512bf16)__builtin_ia32_vrcpnepbf16512_mask(
      (__v32bf)__A, (__v32bf)__W, (__mmask32)__U);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_maskz_rcpne_pbf16(__mmask32 __U, __m512bf16 __A) {
  return (__m512bf16)__builtin_ia32_vrcpnepbf16512_mask(
      (__v32bf)__A, (__v32bf)_mm512_setzero_pbf16(), (__mmask32)__U);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_getexpne_pbf16(__m512bf16 __A) {
  return (__m512bf16)__builtin_ia32_vgetexpnepbf16512_mask(
      (__v32bf)__A, (__v32bf)_mm512_undefined_pbf16(), (__mmask32)-1);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask_getexpne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A) {
  return (__m512bf16)__builtin_ia32_vgetexpnepbf16512_mask(
      (__v32bf)__A, (__v32bf)__W, (__mmask32)__U);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_maskz_getexpne_pbf16(__mmask32 __U, __m512bf16 __A) {
  return (__m512bf16)__builtin_ia32_vgetexpnepbf16512_mask(
      (__v32bf)__A, (__v32bf)_mm512_setzero_pbf16(), (__mmask32)__U);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_rsqrtne_pbf16(__m512bf16 __A) {
  return (__m512bf16)__builtin_ia32_vrsqrtnepbf16512_mask(
      (__v32bf)__A, (__v32bf)_mm512_undefined_pbf16(), (__mmask32)-1);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask_rsqrtne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A) {
  return (__m512bf16)__builtin_ia32_vrsqrtnepbf16512_mask(
      (__v32bf)__A, (__v32bf)__W, (__mmask32)__U);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_maskz_rsqrtne_pbf16(__mmask32 __U, __m512bf16 __A) {
  return (__m512bf16)__builtin_ia32_vrsqrtnepbf16512_mask(
      (__v32bf)__A, (__v32bf)_mm512_setzero_pbf16(), (__mmask32)__U);
}

#define _mm512_reducene_pbf16(A, imm)                                          \
  ((__m512bf16)__builtin_ia32_vreducenepbf16512_mask(                          \
      (__v32bf)(__m512bf16)(A), (int)(imm), (__v32bf)_mm512_undefined_pbf16(), \
      (__mmask32)-1))

#define _mm512_mask_reducene_pbf16(W, U, A, imm)                               \
  ((__m512bf16)__builtin_ia32_vreducenepbf16512_mask(                          \
      (__v32bf)(__m512bf16)(A), (int)(imm), (__v32bf)(__m512bf16)(W),          \
      (__mmask32)(U)))

#define _mm512_maskz_reducene_pbf16(U, A, imm)                                 \
  ((__m512bf16)__builtin_ia32_vreducenepbf16512_mask(                          \
      (__v32bf)(__m512bf16)(A), (int)(imm), (__v32bf)_mm512_setzero_pbf16(),   \
      (__mmask32)(U)))

#define _mm512_roundscalene_pbf16(A, B)                                        \
  ((__m512bf16)__builtin_ia32_vrndscalenepbf16_mask(                           \
      (__v32bf)(__m512bf16)(A), (int)(B), (__v32bf)(__m512bf16)(A),            \
      (__mmask32)-1))

#define _mm512_mask_roundscalene_pbf16(A, B, C, imm)                           \
  ((__m512bf16)__builtin_ia32_vrndscalenepbf16_mask(                           \
      (__v32bf)(__m512bf16)(C), (int)(imm), (__v32bf)(__m512bf16)(A),          \
      (__mmask32)(B)))

#define _mm512_maskz_roundscalene_pbf16(A, B, imm)                             \
  ((__m512bf16)__builtin_ia32_vrndscalenepbf16_mask(                           \
      (__v32bf)(__m512bf16)(B), (int)(imm), (__v32bf)_mm512_setzero_pbf16(),   \
      (__mmask32)(A)))

#define _mm512_getmantne_pbf16(A, B, C)                                        \
  ((__m512bf16)__builtin_ia32_vgetmantnepbf16512_mask(                         \
      (__v32bf)(__m512bf16)(A), (int)(((C) << 2) | (B)),                       \
      (__v32bf)_mm512_undefined_pbf16(), (__mmask32)-1))

#define _mm512_mask_getmantne_pbf16(W, U, A, B, C)                             \
  ((__m512bf16)__builtin_ia32_vgetmantnepbf16512_mask(                         \
      (__v32bf)(__m512bf16)(A), (int)(((C) << 2) | (B)),                       \
      (__v32bf)(__m512bf16)(W), (__mmask32)(U)))

#define _mm512_maskz_getmantne_pbf16(U, A, B, C)                               \
  ((__m512bf16)__builtin_ia32_vgetmantnepbf16512_mask(                         \
      (__v32bf)(__m512bf16)(A), (int)(((C) << 2) | (B)),                       \
      (__v32bf)_mm512_setzero_pbf16(), (__mmask32)(U)))

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_sqrtne_pbf16(__m512bf16 __A) {
  return (__m512bf16)__builtin_ia32_vsqrtnepbf16512_mask(
      (__v32bf)__A, (__v32bf)_mm512_undefined_pbf16(), (__mmask32)-1);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask_sqrtne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A) {
  return (__m512bf16)__builtin_ia32_vsqrtnepbf16512_mask(
      (__v32bf)__A, (__v32bf)__W, (__mmask32)__U);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_maskz_sqrtne_pbf16(__mmask32 __U, __m512bf16 __A) {
  return (__m512bf16)__builtin_ia32_vsqrtnepbf16512_mask(
      (__v32bf)__A, (__v32bf)_mm512_setzero_pbf16(), (__mmask32)__U);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_fmaddne_pbf16(__m512bf16 __A, __m512bf16 __B, __m512bf16 __C) {
  return (__m512bf16)__builtin_ia32_vfmadd213nepbf16512(
      (__v32bf)__A, (__v32bf)__B, (__v32bf)__C);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask_fmaddne_pbf16(__m512bf16 __A, __mmask32 __U, __m512bf16 __B,
                          __m512bf16 __C) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, __builtin_ia32_vfmadd132nepbf16512(
                          (__v32bf)__A, (__v32bf)__C, (__v32bf)__B),
      (__v32bf)__A);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask3_fmaddne_pbf16(__m512bf16 __A, __m512bf16 __B, __m512bf16 __C,
                           __mmask32 __U) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, __builtin_ia32_vfmadd231nepbf16512(
                          (__v32bf)__C, (__v32bf)__A, (__v32bf)__B),
      (__v32bf)__A);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_maskz_fmaddne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B,
                           __m512bf16 __C) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, __builtin_ia32_vfmadd213nepbf16512(
                          (__v32bf)__A, (__v32bf)__B, (__v32bf)__C),
      (__v32bf)_mm512_setzero_pbf16());
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_fmsubne_pbf16(__m512bf16 __A, __m512bf16 __B, __m512bf16 __C) {
  return (__m512bf16)__builtin_ia32_vfmsub213nepbf16512(
      (__v32bf)__A, (__v32bf)__B, (__v32bf)__C);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask_fmsubne_pbf16(__m512bf16 __A, __mmask32 __U, __m512bf16 __B,
                          __m512bf16 __C) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, __builtin_ia32_vfmsub132nepbf16512(
                          (__v32bf)__A, (__v32bf)__C, (__v32bf)__B),
      (__v32bf)__A);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask3_fmsubne_pbf16(__m512bf16 __A, __m512bf16 __B, __m512bf16 __C,
                           __mmask32 __U) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, __builtin_ia32_vfmsub231nepbf16512(
                          (__v32bf)__C, (__v32bf)__A, (__v32bf)__B),
      (__v32bf)__A);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_maskz_fmsubne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B,
                           __m512bf16 __C) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, __builtin_ia32_vfmsub213nepbf16512(
                          (__v32bf)__A, (__v32bf)__B, (__v32bf)__C),
      (__v32bf)_mm512_setzero_pbf16());
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_fnmaddne_pbf16(__m512bf16 __A, __m512bf16 __B, __m512bf16 __C) {
  return (__m512bf16)__builtin_ia32_vfnmadd213nepbf16512(
      (__v32bf)__A, (__v32bf)__B, (__v32bf)__C);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask_fnmaddne_pbf16(__m512bf16 __A, __mmask32 __U, __m512bf16 __B,
                           __m512bf16 __C) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, __builtin_ia32_vfnmadd132nepbf16512(
                          (__v32bf)__A, (__v32bf)__C, (__v32bf)__B),
      (__v32bf)__A);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask3_fnmaddne_pbf16(__m512bf16 __A, __m512bf16 __B, __m512bf16 __C,
                            __mmask32 __U) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, __builtin_ia32_vfnmadd231nepbf16512(
                          (__v32bf)__C, (__v32bf)__A, (__v32bf)__B),
      (__v32bf)__A);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_maskz_fnmaddne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B,
                            __m512bf16 __C) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, __builtin_ia32_vfnmadd213nepbf16512(
                          (__v32bf)__A, (__v32bf)__B, (__v32bf)__C),
      (__v32bf)_mm512_setzero_pbf16());
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_fnmsubne_pbf16(__m512bf16 __A, __m512bf16 __B, __m512bf16 __C) {
  return (__m512bf16)__builtin_ia32_vfnmsub213nepbf16512(
      (__v32bf)__A, (__v32bf)__B, (__v32bf)__C);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask_fnmsubne_pbf16(__m512bf16 __A, __mmask32 __U, __m512bf16 __B,
                           __m512bf16 __C) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, __builtin_ia32_vfnmsub132nepbf16512(
                          (__v32bf)__A, (__v32bf)__C, (__v32bf)__B),
      (__v32bf)__A);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_mask3_fnmsubne_pbf16(__m512bf16 __A, __m512bf16 __B, __m512bf16 __C,
                            __mmask32 __U) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, __builtin_ia32_vfnmsub231nepbf16512(
                          (__v32bf)__C, (__v32bf)__A, (__v32bf)__B),
      (__v32bf)__A);
}

static __inline__ __m512bf16 __DEFAULT_FN_ATTRS512
_mm512_maskz_fnmsubne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B,
                            __m512bf16 __C) {
  return (__m512bf16)__builtin_ia32_selectpbf_512(
      (__mmask32)__U, __builtin_ia32_vfnmsub213nepbf16512(
                          (__v32bf)__A, (__v32bf)__B, (__v32bf)__C),
      (__v32bf)_mm512_setzero_pbf16());
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256
#undef __DEFAULT_FN_ATTRS512

#endif
