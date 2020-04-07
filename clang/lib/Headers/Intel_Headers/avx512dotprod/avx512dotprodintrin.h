/*===-------- avx512dotprodintrin.h - AVX512DOTPROD intrinsics -----------===
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
#error "Never use <avx512dotprodintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512DOTPRODINTRIN_H
#define __AVX512DOTPRODINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS512 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512dotprod"), __min_vector_width__(512)))

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_vdpphps_ps( __m512 __W, __m512h __A, __m512h __B) {
  return (__m512)__builtin_ia32_vdpphps512_mask((__v16sf)__W,(__v16sf)__A,
                                                (__v16sf)__B, (__mmask16)-1,
                                                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_mask_vdpphps_ps( __m512 __W, __mmask16 __U, __m512h __A, __m512h __B) {
  return (__m512)__builtin_ia32_vdpphps512_mask((__v16sf)__W,(__v16sf)__A,
                                                (__v16sf)__B, (__mmask16)__U,
                                                _MM_FROUND_CUR_DIRECTION);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_maskz_vdpphps_ps( __mmask16 __U, __m512 __W, __m512h __A, __m512h __B) {
  return (__m512)__builtin_ia32_vdpphps512_maskz((__v16sf)__W,(__v16sf)__A,
                                                 (__v16sf)__B, (__mmask16)__U,
                                                 _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_vdpphps_round_ps(W, A, B, R) \
  (__m512) __builtin_ia32_vdpphps512_mask((__v16sf)(__m512h) (W), \
                                          (__v16sf)(__m512h) (A), \
                                          (__v16sf)(__m512h) (B), \
                                          (__mmask16)-1, \
                                          (int)(R))

#define _mm512_mask_vdpphps_round_ps(W, U, A, B, R) \
  (__m512) __builtin_ia32_vdpphps512_mask((__v16sf)(__m512h) (W), \
                                          (__v16sf)(__m512h) (A), \
                                          (__v16sf)(__m512h) (B), \
                                          (__mmask16) (U), \
                                          (int)(R))

#define _mm512_maskz_vdpphps_round_ps(U, W, A, B, R) \
  (__m512) __builtin_ia32_vdpphps512_maskz((__v16sf)(__m512h) (W), \
                                           (__v16sf)(__m512h) (A), \
                                           (__v16sf)(__m512h) (B), \
                                           (__mmask16) (U), \
                                           (int)(R))

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_vpdpbssd_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_vpdpbssd512((__v16si)__W, (__v16si)__A, (__v16si)__B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_vpdpbssd_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbssd_epi32(__W, __A, __B),
                  (__v16si)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_vpdpbssd_epi32(__mmask16 __U, __m512i __W,  __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbssd_epi32(__W, __A, __B),
                  (__v16si)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_vpdpbssds_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_vpdpbssds512((__v16si)__W, (__v16si)__A, (__v16si)__B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_vpdpbssds_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbssds_epi32(__W, __A, __B),
                  (__v16si)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_vpdpbssds_epi32(__mmask16 __U, __m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbssds_epi32(__W, __A, __B),
                  (__v16si)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_vpdpbsud_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_vpdpbsud512((__v16si)__W, (__v16si)__A, (__v16si)__B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_vpdpbsud_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbsud_epi32(__W, __A, __B),
                  (__v16si)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_vpdpbsud_epi32(__mmask16 __U, __m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbsud_epi32(__W, __A, __B),
                  (__v16si)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_vpdpbsuds_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_vpdpbsuds512((__v16si)__W, (__v16si)__A, (__v16si)__B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_vpdpbsuds_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbsuds_epi32(__W, __A, __B),
                  (__v16si)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_vpdpbsuds_epi32(__mmask16 __U, __m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbsuds_epi32(__W, __A, __B),
                  (__v16si)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_vpdpbuud_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_vpdpbuud512((__v16si)__W, (__v16si)__A, (__v16si)__B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_vpdpbuud_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbuud_epi32(__W, __A, __B),
                  (__v16si)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_vpdpbuud_epi32(__mmask16 __U, __m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbuud_epi32(__W, __A, __B),
                  (__v16si)_mm512_setzero_si512());
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_vpdpbuuds_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_vpdpbuuds512((__v16si)__W, (__v16si)__A, (__v16si)__B);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_mask_vpdpbuuds_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbuuds_epi32(__W, __A, __B),
                  (__v16si)__W);
}

static __inline__ __m512i __DEFAULT_FN_ATTRS512
_mm512_maskz_vpdpbuuds_epi32( __mmask16 __U, __m512i __W, __m512i __A, __m512i __B) {
  return (__m512i)__builtin_ia32_selectd_512(__U,
                  (__v16si)_mm512_vpdpbuuds_epi32(__W, __A, __B),
                  (__v16si)_mm512_setzero_si512());
}
#undef __DEFAULT_FN_ATTRS512

#endif // __AVX512DOTPRODINTRIN_H
