/*===---- avx512dqintrin.h - AVX512DQ intrinsics -----------------------------------===
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
#ifndef __INTRIN_H__
#error "Never use <avx512dqintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512DQINTRIN_H
#define __AVX512DQINTRIN_H

static __inline__ __m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_and_ps (__m512 __A, __m512 __B) {
  return (__m512) ((__v16si) __A & (__v16si) __B);
}

static __inline__ __m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_and_ps (__m512 __W, __mmask16 __U, __m512 __A, __m512 __B) {
  return (__m512)__builtin_ia32_selectps_512((__mmask16)__U,
                                             (__v16sf)_mm512_and_ps(__A, __B),
                                             (__v16sf)__W);
}

static __inline__ __m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_maskz_and_ps (__mmask16 __U, __m512 __A, __m512 __B) {
  return (__m512)__builtin_ia32_selectps_512((__mmask16)__U,
                                             (__v16sf)_mm512_and_ps(__A, __B),
                                             (__v16sf)_mm512_setzero_ps());
}

static __inline__ __m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_or_ps (__m512 __A, __m512 __B) {
  return (__m512) ((__v16si) __A | (__v16si) __B);
}

static __inline__ __m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_or_ps (__m512 __W, __mmask16 __U, __m512 __A, __m512 __B) {
  return (__m512)__builtin_ia32_selectps_512((__mmask16)__U,
                                             (__v16sf)_mm512_or_ps(__A, __B),
                                             (__v16sf)__W);
}

static __inline__ __m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_maskz_or_ps (__mmask16 __U, __m512 __A, __m512 __B) {
  return (__m512)__builtin_ia32_selectps_512((__mmask16)__U,
                                             (__v16sf)_mm512_or_ps(__A, __B),
                                             (__v16sf)_mm512_setzero_ps());
}

static __inline__ __m512d __attribute__ ((__always_inline__, __nodebug__))
_mm512_and_pd (__m512d __A, __m512d __B) {
  return (__m512d) ((__v8di) __A & (__v8di) __B);
}

static __inline__ __m512d __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_and_pd (__m512d __W, __mmask8 __U, __m512d __A, __m512d __B) {
  return (__m512d)__builtin_ia32_selectpd_512((__mmask8)__U,
                                              (__v8df)_mm512_and_pd(__A, __B),
                                              (__v8df)__W);
}

static __inline__ __m512d __attribute__ ((__always_inline__, __nodebug__))
_mm512_maskz_and_pd (__mmask8 __U, __m512d __A, __m512d __B) {
  return (__m512d)__builtin_ia32_selectpd_512((__mmask8)__U,
                                              (__v8df)_mm512_and_pd(__A, __B),
                                              (__v8df)_mm512_setzero_pd());
}
static __inline__ __m512d __attribute__ ((__always_inline__, __nodebug__))
_mm512_or_pd (__m512d __A, __m512d __B) {
  return (__m512d) ((__v8di) __A | (__v8di) __B);
}

static __inline__ __m512d __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_or_pd (__m512d __W, __mmask8 __U, __m512d __A, __m512d __B) {
  return (__m512d)__builtin_ia32_selectpd_512((__mmask8)__U,
                                              (__v8df)_mm512_or_pd(__A, __B),
                                              (__v8df)__W);
}

static __inline__ __m512d __attribute__ ((__always_inline__, __nodebug__))
_mm512_maskz_or_pd (__mmask8 __U, __m512d __A, __m512d __B) {
  return (__m512d)__builtin_ia32_selectpd_512((__mmask8)__U,
                                              (__v8df)_mm512_or_pd(__A, __B),
                                              (__v8df)_mm512_setzero_pd());
}

#endif // __AVX512DQINTRIN_H
