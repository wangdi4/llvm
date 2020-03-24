/*===------------- avxifmaintrin.h - IFMA intrinsics ------------------===
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
#error "Never use <avxifmaintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVXIFMAINTRIN_H
#define __AVXIFMAINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128 __attribute__((__always_inline__, __nodebug__, __target__("avxifma"), __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256 __attribute__((__always_inline__, __nodebug__, __target__("avxifma"), __min_vector_width__(256)))

// must vex-encoding
static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_madd52hi_avx_epu64 (__m128i __X, __m128i __Y, __m128i __Z)
{
  return (__m128i)__builtin_ia32_vpmadd52huqvex128((__v2di) __X, (__v2di) __Y,
                                                (__v2di) __Z);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_madd52hi_avx_epu64 (__m256i __X, __m256i __Y, __m256i __Z)
{
  return (__m256i)__builtin_ia32_vpmadd52huqvex256((__v4di) __X, (__v4di) __Y,
                                                (__v4di) __Z);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_madd52lo_avx_epu64 (__m128i __X, __m128i __Y, __m128i __Z)
{
  return (__m128i)__builtin_ia32_vpmadd52luqvex128((__v2di) __X, (__v2di) __Y,
                                                (__v2di) __Z);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_madd52lo_avx_epu64 (__m256i __X, __m256i __Y, __m256i __Z)
{
  return (__m256i)__builtin_ia32_vpmadd52luqvex256((__v4di) __X, (__v4di) __Y,
                                                (__v4di) __Z);
}
#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVXIFMAINTRIN_H
