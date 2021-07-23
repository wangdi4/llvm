/*===-------- avx512vlneconvertintrin.h - AVX512VLNECONVERT -------------===
 *
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
    "Never use <avx512vlneconvertintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVX512VLNECONVERTINTRIN_H
#define __AVX512VLNECONVERTINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512vl,avx512neconvert"),                       \
                 __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256                                                  \
  __attribute__((__always_inline__, __nodebug__,                               \
                 __target__("avx512vl,avx512neconvert"),                       \
                 __min_vector_width__(256)))

#define _mm_bcstnebf16_ps(A) (__m128)__builtin_ia32_vbcstnebf162ps128(A)

#define _mm256_bcstnebf16_ps(A) (__m256)__builtin_ia32_vbcstnebf162ps256(A)

#define _mm_bcstnesh_ps(A) (__m128)__builtin_ia32_vbcstnesh2ps128(A)

#define _mm256_bcstnesh_ps(A) (__m256)__builtin_ia32_vbcstnesh2ps256(A);

#define _mm_cvtneebf16_ps(A) (__m128)__builtin_ia32_vcvtneebf162ps128(A)

#define _mm256_cvtneebf16_ps(A) (__m256)__builtin_ia32_vcvtneebf162ps256(A)

#define _mm_cvtneeph_ps(A) (__m128)__builtin_ia32_vcvtneeph2ps128(A);

#define _mm256_cvtneeph_ps(A) (__m256)__builtin_ia32_vcvtneeph2ps256(A);

#define _mm_cvtneobf16_ps(A) (__m128)__builtin_ia32_vcvtneobf162ps128(A);

#define _mm256_cvtneobf16_ps(A) (__m256)__builtin_ia32_vcvtneobf162ps256(A);

#define _mm_cvtneoph_ps(A) (__m128)__builtin_ia32_vcvtneoph2ps128(A);

#define _mm256_cvtneoph_ps(A) (__m256)__builtin_ia32_vcvtneoph2ps256(A);

#ifdef __AVX512NECONVERTINTRIN_H
static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_mask_bcstnebf16_ps(__mmask8 __U, const void *__A) {
  return (__m128)__builtin_ia32_vbcstnebf162ps128_mask((__mmask8)__U,
                                                       (const void *)__A);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_maskz_bcstnebf16_ps(__mmask8 __U, const void *__A) {
  return (__m128)__builtin_ia32_vbcstnebf162ps128_maskz((__mmask8)__U,
                                                        (const void *)__A);
}


static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_mask_bcstnebf16_ps(__mmask8 __U, const void *__A) {
  return (__m256)__builtin_ia32_vbcstnebf162ps256_mask((__mmask8)__U,
                                                       (const void *)__A);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_maskz_bcstnebf16_ps(__mmask8 __U, const void *__A) {
  return (__m256)__builtin_ia32_vbcstnebf162ps256_maskz((__mmask8)__U,
                                                        (const void *)__A);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_mask_bcstnesh_ps(__mmask8 __U, const void *__A) {
  return (__m128)__builtin_ia32_vbcstnesh2ps128_mask((__mmask8)__U,
                                                     (const void *)__A);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_maskz_bcstnesh_ps(__mmask8 __U, const void *__A) {
  return (__m128)__builtin_ia32_vbcstnesh2ps128_maskz((__mmask8)__U,
                                                      (const void *)__A);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_mask_bcstnesh_ps(__mmask8 __U, const void *__A) {
  return (__m256)__builtin_ia32_vbcstnesh2ps256_mask((__mmask8)__U,
                                                     (const void *)__A);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_maskz_bcstnesh_ps(__mmask8 __U, const void *__A) {
  return (__m256)__builtin_ia32_vbcstnesh2ps256_maskz((__mmask8)__U,
                                                      (const void *)__A);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128 _mm_cvtne2ps_ph(__m128 __A,
                                                                __m128 __B) {
  return (__m128h)__builtin_ia32_vcvtne2ps2ph128((__v4sf)__A, (__v4sf)__B);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_mask_cvtne2ps_ph(__m128h __W, __mmask8 __U, __m128 __A, __m128 __B) {
  return (__m128h)__builtin_ia32_vcvtne2ps2ph128_mask(
      (__v4sf)__A, (__v4sf)__B, (__v8hf)__W, (__mmask8)__U);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_maskz_cvtne2ps_ph(__m128h __W, __mmask8 __U, __m128 __A, __m128 __B) {
  return (__m128h)__builtin_ia32_vcvtne2ps2ph128_maskz(
      (__v4sf)__A, (__v4sf)__B, (__v8hf)__W, (__mmask8)__U);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256 _mm256_cvtne2ps_ph(__m256 __A,
                                                                   __m256 __B) {
  return (__m256h)__builtin_ia32_vcvtne2ps2ph256((__v8sf)__A, (__v8sf)__B);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_mask_cvtne2ps_ph(__m256h __W, __mmask16 __U, __m256 __A, __m256 __B) {
  return (__m256h)__builtin_ia32_vcvtne2ps2ph256_mask(
      (__v8sf)__A, (__v8sf)__B, (__v16hf)__W, (__mmask16)__U);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtne2ps_ph(__m256h __W, __mmask16 __U, __m256 __A, __m256 __B) {
  return (__m256h)__builtin_ia32_vcvtne2ps2ph256_maskz(
      (__v8sf)__A, (__v8sf)__B, (__v16hf)__W, (__mmask16)__U);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_mask_cvtneebf16_ps(__mmask8 __U, const __m128bh *__A) {
  return (__m128)__builtin_ia32_vcvtneebf162ps128_mask((__mmask8)__U,
                                                       (const __v8hi *)__A);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_maskz_cvtneebf16_ps(__mmask8 __U, const __m128bh *__A) {
  return (__m128)__builtin_ia32_vcvtneebf162ps128_maskz((__mmask8)__U,
                                                        (const __v8hi *)__A);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_mask_cvtneebf16_ps(__mmask8 __U, const __m256bh *__A) {
  return (__m256)__builtin_ia32_vcvtneebf162ps256_mask((__mmask8)__U,
                                                       (const __v16hi *)__A);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtneebf16_ps(__mmask8 __U, const __m256bh *__A) {
  return (__m256)__builtin_ia32_vcvtneebf162ps256_maskz((__mmask8)__U,
                                                        (const __v16hi *)__A);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_mask_cvtneeph_ps(__mmask8 __U, const __m128h *__A) {
  return (__m128)__builtin_ia32_vcvtneeph2ps128_mask((__mmask8)__U,
                                                     (const __v8hf *)__A);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_maskz_cvtneeph_ps(__mmask8 __U, const __m128h *__A) {
  return (__m128)__builtin_ia32_vcvtneeph2ps128_maskz((__mmask8)__U,
                                                      (const __v8hf *)__A);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_mask_cvtneeph_ps(__mmask8 __U, const __m256h *__A) {
  return (__m256)__builtin_ia32_vcvtneeph2ps256_mask((__mmask8)__U,
                                                     (const __v16hf *)__A);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtneeph_ps(__mmask8 __U, const __m256h *__A) {
  return (__m256)__builtin_ia32_vcvtneeph2ps256_maskz((__mmask8)__U,
                                                      (const __v16hf *)__A);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_mask_cvtneobf16_ps(__mmask8 __U, const __m128bh *__A) {
  return (__m128)__builtin_ia32_vcvtneobf162ps128_mask((__mmask8)__U,
                                                       (const __v8hi *)__A);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_maskz_cvtneobf16_ps(__mmask8 __U, const __m128bh *__A) {
  return (__m128)__builtin_ia32_vcvtneobf162ps128_maskz((__mmask8)__U,
                                                        (const __v8hi *)__A);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_mask_cvtneobf16_ps(__mmask8 __U, const __m256bh *__A) {
  return (__m256)__builtin_ia32_vcvtneobf162ps256_mask((__mmask8)__U,
                                                       (const __v16hi *)__A);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtneobf16_ps(__mmask8 __U, const __m256bh *__A) {
  return (__m256)__builtin_ia32_vcvtneobf162ps256_maskz((__mmask8)__U,
                                                        (const __v16hi *)__A);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_mask_cvtneoph_ps(__mmask8 __U, const __m128h *__A) {
  return (__m128)__builtin_ia32_vcvtneoph2ps128_mask((__mmask8)__U,
                                                     (const __v8hf *)__A);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_maskz_cvtneoph_ps(__mmask8 __U, const __m128h *__A) {
  return (__m128)__builtin_ia32_vcvtneoph2ps128_maskz((__mmask8)__U,
                                                      (const __v8hf *)__A);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_mask_cvtneoph_ps(__mmask8 __U, const __m256h *__A) {
  return (__m256)__builtin_ia32_vcvtneoph2ps256_mask((__mmask8)__U,
                                                     (const __v16hf *)__A);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_maskz_cvtneoph_ps(__mmask8 __U, const __m256h *__A) {
  return (__m256)__builtin_ia32_vcvtneoph2ps256_maskz((__mmask8)__U,
                                                      (const __v16hf *)__A);
}
#endif /* __AVX512NECONVERTINTRIN_H */

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVX512VLNECONVERTINTRIN_H
