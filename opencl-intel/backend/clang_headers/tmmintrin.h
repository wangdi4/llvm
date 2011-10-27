/*===---- tmmintrin.h - SSSE3 intrinsics -----------------------------------===
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
 
#ifndef __TMMINTRIN_H
#define __TMMINTRIN_H

#ifndef __SSSE3__
#error "SSSE3 instruction set not enabled"
#else

#include <pmmintrin.h>

__inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_abs_epi8(__m128i a)
{
    return (__m128i)__builtin_ia32_pabsb128((__v16qi)a);
}

__inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_abs_epi16(__m128i a)
{
    return (__m128i)__builtin_ia32_pabsw128((__v8hi)a);
}

__inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_abs_epi32(__m128i a)
{
    return (__m128i)__builtin_ia32_pabsd128((__v4si)a);
}

#define _mm_alignr_epi8(a, b, n) (__builtin_ia32_palignr128((a), (b), (n)))
#define _mm_alignr_pi8(a, b, n) (__builtin_ia32_palignr((a), (b), (n)))

__inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hadd_epi16(__m128i a, __m128i b)
{
    return (__m128i)__builtin_ia32_phaddw128((__v8hi)a, (__v8hi)b);
}

__inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hadd_epi32(__m128i a, __m128i b)
{
    return (__m128i)__builtin_ia32_phaddd128((__v4si)a, (__v4si)b);
}

__inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hadds_epi16(__m128i a, __m128i b)
{
    return (__m128i)__builtin_ia32_phaddsw128((__v8hi)a, (__v8hi)b);
}

__inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hsub_epi16(__m128i a, __m128i b)
{
    return (__m128i)__builtin_ia32_phsubw128((__v8hi)a, (__v8hi)b);
}

__inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hsub_epi32(__m128i a, __m128i b)
{
    return (__m128i)__builtin_ia32_phsubd128((__v4si)a, (__v4si)b);
}

__inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_hsubs_epi16(__m128i a, __m128i b)
{
    return (__m128i)__builtin_ia32_phsubsw128((__v8hi)a, (__v8hi)b);
}

__inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_maddubs_epi16(__m128i a, __m128i b)
{
    return (__m128i)__builtin_ia32_pmaddubsw128((__v16qi)a, (__v16qi)b);
}

__inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_mulhrs_epi16(__m128i a, __m128i b)
{
    return (__m128i)__builtin_ia32_pmulhrsw128((__v8hi)a, (__v8hi)b);
}

__inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_shuffle_epi8(__m128i a, __m128i b)
{
    return (__m128i)__builtin_ia32_pshufb128((__v16qi)a, (__v16qi)b);
}

__inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sign_epi8(__m128i a, __m128i b)
{
    return (__m128i)__builtin_ia32_psignb128((__v16qi)a, (__v16qi)b);
}

__inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sign_epi16(__m128i a, __m128i b)
{
    return (__m128i)__builtin_ia32_psignw128((__v8hi)a, (__v8hi)b);
}

__inline__ __m128i __attribute__((__always_inline__, __nodebug__))
_mm_sign_epi32(__m128i a, __m128i b)
{
    return (__m128i)__builtin_ia32_psignd128((__v4si)a, (__v4si)b);
}

#endif /* __SSSE3__ */

#endif /* __TMMINTRIN_H */
