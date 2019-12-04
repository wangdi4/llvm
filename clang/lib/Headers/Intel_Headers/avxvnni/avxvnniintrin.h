/*===------------- avx512vlvnniintrin.h - VNNI intrinsics ------------------===
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
#error "Never use <avxvnniintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVXVNNIINTRIN_H
#define __AVXVNNIINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128 __attribute__((__always_inline__, __nodebug__, __target__("avxvnni"), __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256 __attribute__((__always_inline__, __nodebug__, __target__("avxvnni"), __min_vector_width__(256)))

// must vex-encoding
static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_dpbusd_avx_epi32(__m256i __S, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_vpdpbusdvex256((__v8si)__S, (__v8si)__A,
                                             (__v8si)__B);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_dpbusds_avx_epi32(__m256i __S, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_vpdpbusdsvex256((__v8si)__S, (__v8si)__A,
                                              (__v8si)__B);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_dpwssd_avx_epi32(__m256i __S, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_vpdpwssdvex256((__v8si)__S, (__v8si)__A,
                                             (__v8si)__B);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_dpwssds_avx_epi32(__m256i __S, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_vpdpwssdsvex256((__v8si)__S, (__v8si)__A,
                                              (__v8si)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dpbusd_avx_epi32(__m128i __S, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_vpdpbusdvex128((__v4si)__S, (__v4si)__A,
                                             (__v4si)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dpbusds_avx_epi32(__m128i __S, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_vpdpbusdsvex128((__v4si)__S, (__v4si)__A,
                                              (__v4si)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dpwssd_avx_epi32(__m128i __S, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_vpdpwssdvex128((__v4si)__S, (__v4si)__A,
                                             (__v4si)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dpwssds_avx_epi32(__m128i __S, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_vpdpwssdsvex128((__v4si)__S, (__v4si)__A,
                                              (__v4si)__B);
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVXVNNIINTRIN_H
