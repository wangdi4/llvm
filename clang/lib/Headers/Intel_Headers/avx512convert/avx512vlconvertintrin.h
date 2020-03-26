/*===-------- avx512vldotprodintrin.h - AVX512DOTPROD intrinsics -----------===
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
#error "Never use <avx512vlconvertintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512VLCONVERTINTRIN_H
#define __AVX512VLDOTPRODINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS256 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512convert, avx512vl"), __min_vector_width__(256)))
#define __DEFAULT_FN_ATTRS128 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512convert, avx512vl"), __min_vector_width__(128)))

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_m128_mask_vcvt2ps2ph_ph( __m128h __W, __mmask8 __U, __m128 __A, __m128 __B) {
  return (__m128h)__builtin_ia32_vcvt2ps2ph128_mask((__v4sf)__A,(__v4sf)__B,
                                                    (__v8hf)__W, (__mmask8)__U);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_m256_mask_vcvt2ps2ph_ph( __m256h __W, __mmask8 __U, __m256 __A, __m256 __B) {
  return (__m256h)__builtin_ia32_vcvt2ps2ph256_mask((__v8sf)__A,(__v8sf)__B,
                                                    (__v16hf)__W, (__mmask16)__U);
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_m128_mask_vcvtbf162ph_ph(__m128h __W, __mmask8 __U, __m128i __A) {
  return (__m128h)__builtin_ia32_vcvtbf162ph128_mask((__v8hi)__A, (__v8hf)__W, (__mmask8)__U);
}

static __inline__ __m256h __DEFAULT_FN_ATTRS256
_m256_mask_vcvtbf162ph_ph(__m256h __W, __mmask8 __U, __m256i __A) {
  return (__m256h)__builtin_ia32_vcvtbf162ph256_mask((__v16hi)__A, (__v16hf)__W, (__mmask16)__U);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_m128_mask_vcvtneph2bf16_ph(__m128i __W, __mmask16 __U, __m128h __A) {
  return (__m128i)__builtin_ia32_vcvtneph2bf16_128_mask((__v8hf)__A, (__v8hi)__W, (__mmask8)__U);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_m256_mask_vcvtneph2bf16_ph(__m256i __W, __mmask16 __U, __m256h __A) {
  return (__m256i)__builtin_ia32_vcvtneph2bf16_256_mask((__v16hf)__A, (__v16hi)__W, (__mmask16)__U);
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVX512VLCONVERTINTRIN_H
