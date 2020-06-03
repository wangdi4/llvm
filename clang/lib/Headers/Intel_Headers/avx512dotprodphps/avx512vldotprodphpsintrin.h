/*===--- avx512vldotprodphpsintrin.h - AVX512DOTPRODPHPS intrinsics -------===
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
#error "Never use <avx512vldotprodphpsintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512VLDOTPRODPHPSINTRIN_H
#define __AVX512VLDOTPRODPHPSINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS256 \
  __attribute__((__always_inline__, __nodebug__, \
  __target__("avx512dotprodphps, avx512vl"), __min_vector_width__(256)))
#define __DEFAULT_FN_ATTRS128 \
  __attribute__((__always_inline__, __nodebug__,\
  __target__("avx512dotprodphps, avx512vl"), __min_vector_width__(128)))

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_mask_vdpphps_ps( __m128 __W, __mmask8 __U, __m128h __A, __m128h __B) {
  return (__m128)__builtin_ia32_vdpphps128_mask((__v4sf)__W, (__v4sf)__A,
                                                (__v4sf)__B, (__mmask8)__U);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_maskz_vdpphps_ps( __mmask8 __U, __m128 __W, __m128h __A, __m128h __B) {
  return (__m128)__builtin_ia32_vdpphps128_maskz((__v4sf)__W, (__v4sf)__A,
                                                 (__v4sf)__B, (__mmask8)__U);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_mask_vdpphps_ps(__m256 __W, __mmask8 __U, __m256h __A, __m256h __B) {
  return (__m256)__builtin_ia32_vdpphps256_mask((__v8sf)__W, (__v8sf)__A,
                                                (__v8sf)__B, (__mmask8)__U);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_maskz_vdpphps_ps(__mmask8 __U, __m256 __W, __m256h __A, __m256h __B) {
  return (__m256)__builtin_ia32_vdpphps256_maskz((__v8sf)__W, (__v8sf)__A,
                                                 (__v8sf)__B, (__mmask8)__U);
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVX512VLDOTPRODPHPSINTRIN_H
