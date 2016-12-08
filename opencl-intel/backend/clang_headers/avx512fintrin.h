/*===---- avx512fintrin.h - AVX2 intrinsics --------------------------------===
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
#error "Never use <avx512fintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512FINTRIN_H
#define __AVX512FINTRIN_H

typedef double __v8df __attribute__((__vector_size__(64)));
typedef float __v16sf __attribute__((__vector_size__(64)));
#ifdef __OPENCL__
typedef long __v8di __attribute__((__vector_size__(64)));
#else
typedef long long __v8di __attribute__((__vector_size__(64)));
#endif
typedef int __v16si __attribute__((__vector_size__(64)));

typedef float __m512 __attribute__((__vector_size__(64)));
typedef double __m512d __attribute__((__vector_size__(64)));
#ifdef __OPENCL__
typedef long __m512i __attribute__((__vector_size__(64)));
#else
typedef long long __m512i __attribute__((__vector_size__(64)));
#endif


typedef unsigned char __mmask8;
typedef unsigned short __mmask16;

/* Rounding mode macros.  */
#define _MM_FROUND_TO_NEAREST_INT   0x00
#define _MM_FROUND_TO_NEG_INF       0x01
#define _MM_FROUND_TO_POS_INF       0x02
#define _MM_FROUND_TO_ZERO          0x03
#define _MM_FROUND_CUR_DIRECTION    0x04

/* Create vectors with repeated elements */

static  __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_setzero_si512(void)
{
  return (__m512i)(__v8di){ 0, 0, 0, 0, 0, 0, 0, 0 };
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_maskz_set1_epi32(__mmask16 __M, int __A)
{
  return (__m512i) __builtin_ia32_pbroadcastd512_gpr_mask (__A,
                 (__v16si)
                 _mm512_setzero_si512 (),
                 __M);
}

__m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_maskz_set1_epi64(__mmask8 __M, long long __A);

static __inline __m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_setzero_ps(void)
{
  return (__m512){ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                   0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
}
static  __inline __m512d __attribute__ ((__always_inline__, __nodebug__))
_mm512_setzero_pd(void)
{
  return (__m512d){ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
}

static __inline __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_set1_ps(float __w)
{
  return (__m512){ __w, __w, __w, __w, __w, __w, __w, __w,
                   __w, __w, __w, __w, __w, __w, __w, __w  };
}

static __inline __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_set1_pd(double __w)
{
  return (__m512d){ __w, __w, __w, __w, __w, __w, __w, __w };
}

static __inline __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_set1_epi32(int __s)
{
  return (__m512i)(__v16si){ __s, __s, __s, __s, __s, __s, __s, __s,
                             __s, __s, __s, __s, __s, __s, __s, __s };
}

static __inline __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_set1_epi64(long __d)
{
  return (__m512i)(__v8di){ __d, __d, __d, __d, __d, __d, __d, __d };
}

/* Arithmetic */

static __inline __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_add_pd(__m512d __a, __m512d __b)
{
  return __a + __b;
}

static __inline __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_add_ps(__m512 __a, __m512 __b)
{
  return __a + __b;
}

static __inline __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mul_pd(__m512d __a, __m512d __b)
{
  return __a * __b;
}

static __inline __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mul_ps(__m512 __a, __m512 __b)
{
  return __a * __b;
}

static __inline __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_sub_pd(__m512d __a, __m512d __b)
{
  return __a - __b;
}

static __inline __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_sub_ps(__m512 __a, __m512 __b)
{
  return __a - __b;
}

static  __inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_max_pd(__m512d __A, __m512d __B)
{
  return (__m512d) __builtin_ia32_maxpd512_mask ((__v8df) __A,
             (__v8df) __B,
             (__v8df)
             _mm512_setzero_pd (),
             (__mmask8) -1,
             _MM_FROUND_CUR_DIRECTION);
}

static  __inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_max_ps(__m512 __A, __m512 __B)
{
  return (__m512) __builtin_ia32_maxps512_mask ((__v16sf) __A,
            (__v16sf) __B,
            (__v16sf)
            _mm512_setzero_ps (),
            (__mmask16) -1,
            _MM_FROUND_CUR_DIRECTION);
}

static __inline __m512i
__attribute__ ((__always_inline__, __nodebug__))
_mm512_max_epi32(__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxsd512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) -1);
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_max_epu32(__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxud512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) -1);
}

__m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_max_epi64(__m512i __A, __m512i __B);

__m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_max_epu64(__m512i __A, __m512i __B);

static  __inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_min_pd(__m512d __A, __m512d __B)
{
  return (__m512d) __builtin_ia32_minpd512_mask ((__v8df) __A,
             (__v8df) __B,
             (__v8df)
             _mm512_setzero_pd (),
             (__mmask8) -1,
             _MM_FROUND_CUR_DIRECTION);
}

__m512 __attribute__((__always_inline__, __nodebug__))
_mm512_min_ps(__m512 __A, __m512 __B);

static __inline __m512i
__attribute__ ((__always_inline__, __nodebug__))
_mm512_min_epi32(__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminsd512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) -1);
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_min_epu32(__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminud512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) -1);
}

__m512i __attribute__((__always_inline__, __nodebug__))
_mm512_min_epi64(__m512i __A, __m512i __B);

__m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_min_epu64(__m512i __A, __m512i __B);

__m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_mul_epi32(__m512i __X, __m512i __Y);

__m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mul_epu32(__m512i __X, __m512i __Y);

__m512d __attribute__((__always_inline__, __nodebug__))
_mm512_sqrt_pd(__m512d a);

static  __inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_sqrt_ps(__m512 a)
{
  return (__m512)__builtin_ia32_sqrtps512_mask((__v16sf)a,
                                               (__v16sf) _mm512_setzero_ps (),
                                               (__mmask16) -1,
                                               _MM_FROUND_CUR_DIRECTION);
}

__m512d __attribute__((__always_inline__, __nodebug__))
_mm512_rsqrt14_pd(__m512d __A);

static  __inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_rsqrt14_ps(__m512 __A)
{
  return (__m512) __builtin_ia32_rsqrt14ps512_mask ((__v16sf) __A,
                (__v16sf)
                _mm512_setzero_ps (),
                (__mmask16) -1);
}

static  __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rsqrt14_ss(__m128 __A, __m128 __B)
{
  return (__m128) __builtin_ia32_rsqrt14ss ((__v4sf) __A,
             (__v4sf) __B,
             (__v4sf)
             _mm_setzero_ps (),
             (__mmask8) -1);
}

static  __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_rsqrt14_sd(__m128d __A, __m128d __B)
{
  return (__m128d) __builtin_ia32_rsqrt14sd ((__v2df) __A,
              (__v2df) __B,
              (__v2df)
              _mm_setzero_pd (),
              (__mmask8) -1);
}

static  __inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_rcp14_pd(__m512d __A)
{
  return (__m512d) __builtin_ia32_rcp14pd512_mask ((__v8df) __A,
               (__v8df)
               _mm512_setzero_pd (),
               (__mmask8) -1);
}

static  __inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_rcp14_ps(__m512 __A)
{
  return (__m512) __builtin_ia32_rcp14ps512_mask ((__v16sf) __A,
              (__v16sf)
              _mm512_setzero_ps (),
              (__mmask16) -1);
}
static  __inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rcp14_ss(__m128 __A, __m128 __B)
{
  return (__m128) __builtin_ia32_rcp14ss ((__v4sf) __A,
                 (__v4sf) __B,
                 (__v4sf)
                 _mm_setzero_ps (),
                 (__mmask8) -1);
}

static  __inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_rcp14_sd(__m128d __A, __m128d __B)
{
  return (__m128d) __builtin_ia32_rcp14sd ((__v2df) __A,
            (__v2df) __B,
            (__v2df)
            _mm_setzero_pd (),
            (__mmask8) -1);
}

static __inline __m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_floor_ps(__m512 __A)
{
  return (__m512) __builtin_ia32_rndscaleps_mask ((__v16sf) __A,
                                                  _MM_FROUND_FLOOR,
                                                  (__v16sf) __A, -1,
                                                  _MM_FROUND_CUR_DIRECTION);
}

static __inline __m512d __attribute__ ((__always_inline__, __nodebug__))
_mm512_floor_pd(__m512d __A)
{
  return (__m512d) __builtin_ia32_rndscalepd_mask ((__v8df) __A,
                                                   _MM_FROUND_FLOOR,
                                                   (__v8df) __A, -1,
                                                   _MM_FROUND_CUR_DIRECTION);
}

static __inline __m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_ceil_ps(__m512 __A)
{
  return (__m512) __builtin_ia32_rndscaleps_mask ((__v16sf) __A,
                                                  _MM_FROUND_CEIL,
                                                  (__v16sf) __A, -1,
                                                  _MM_FROUND_CUR_DIRECTION);
}

static __inline __m512d __attribute__ ((__always_inline__, __nodebug__))
_mm512_ceil_pd(__m512d __A)
{
  return (__m512d) __builtin_ia32_rndscalepd_mask ((__v8df) __A,
                                                   _MM_FROUND_CEIL,
                                                   (__v8df) __A, -1,
                                                   _MM_FROUND_CUR_DIRECTION);
}

__m512i __attribute__ (( __always_inline__, __nodebug__))
_mm512_abs_epi64(__m512i __A);

__m512i __attribute__ (( __always_inline__, __nodebug__))
_mm512_abs_epi32(__m512i __A);

#define _mm512_roundscale_ps(A, B) __extension__ ({ \
  (__m512)__builtin_ia32_rndscaleps_mask((__v16sf)(A), (B), (__v16sf)(A), \
                                         -1, _MM_FROUND_CUR_DIRECTION); })

#define _mm512_roundscale_pd(A, B) __extension__ ({ \
  (__m512d)__builtin_ia32_rndscalepd_mask((__v8df)(A), (B), (__v8df)(A), \
                                          -1, _MM_FROUND_CUR_DIRECTION); })

/* Vector permutations */

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_permutex2var_epi32(__m512i __A, __m512i __I, __m512i __B)
{
  return (__m512i)__builtin_ia32_vpermt2vard512_mask((__v16si)__I
    /* idx */,
    (__v16si)__A,
    (__v16si)__B,
    (__mmask16)-1);
}

static __inline __m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_permutex2var_ps(__m512 __A, __m512i __I, __m512 __B)
{
  return (__m512) __builtin_ia32_vpermt2varps512_mask ((__v16si) __I
                                                       /* idx */ ,
                                                       (__v16sf) __A,
                                                       (__v16sf) __B,
                                                       (__mmask16) -1);
}

/* Vector Blend */

__m512d __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_blend_pd(__mmask8 __U, __m512d __A, __m512d __W);

__m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_blend_ps(__mmask16 __U, __m512 __A, __m512 __W);

__m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_blend_epi32(__mmask16 __U, __m512i __A, __m512i __W);

/* Compare */

#define _mm512_cmp_round_ps_mask(A, B, P, R) __extension__ ({ \
  (__mmask16)__builtin_ia32_cmpps512_mask((__v16sf)(__m512)(A), \
                                          (__v16sf)(__m512)(B), (int)(P), \
                                          (__mmask16)-1, (int)(R)); })

#define _mm512_mask_cmp_round_ps_mask(U, A, B, P, R) __extension__ ({ \
  (__mmask16)__builtin_ia32_cmpps512_mask((__v16sf)(__m512)(A), \
                                          (__v16sf)(__m512)(B), (int)(P), \
                                          (__mmask16)(U), (int)(R)); })

#define _mm512_cmp_ps_mask(A, B, P) \
  _mm512_cmp_round_ps_mask((A), (B), (P), _MM_FROUND_CUR_DIRECTION)

#define _mm512_mask_cmp_ps_mask(U, A, B, P) \
  _mm512_mask_cmp_round_ps_mask((U), (A), (B), (P), _MM_FROUND_CUR_DIRECTION)

#define _mm512_cmp_round_pd_mask(A, B, P, R) __extension__ ({ \
  (__mmask8)__builtin_ia32_cmppd512_mask((__v8df)(__m512d)(A), \
                                         (__v8df)(__m512d)(B), (int)(P), \
                                         (__mmask8)-1, (int)(R)); })

#define _mm512_mask_cmp_round_pd_mask(U, A, B, P, R) __extension__ ({ \
  (__mmask8)__builtin_ia32_cmppd512_mask((__v8df)(__m512d)(A), \
                                         (__v8df)(__m512d)(B), (int)(P), \
                                         (__mmask8)(U), (int)(R)); })

#define _mm512_cmp_pd_mask(A, B, P) \
  _mm512_cmp_round_pd_mask((A), (B), (P), _MM_FROUND_CUR_DIRECTION)

#define _mm512_mask_cmp_pd_mask(U, A, B, P) \
  _mm512_mask_cmp_round_pd_mask((U), (A), (B), (P), _MM_FROUND_CUR_DIRECTION)

/* Conversion */

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_cvttps_epu32(__m512 __A)
{
  return (__m512i) __builtin_ia32_cvttps2udq512_mask ((__v16sf) __A,
                  (__v16si)
                  _mm512_setzero_si512 (),
                  (__mmask16) -1,
                  _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_cvt_roundepi32_ps(A, R) __extension__ ({ \
  (__m512)__builtin_ia32_cvtdq2ps512_mask((__v16si)(A), \
                                          (__v16sf)_mm512_setzero_ps(), \
                                          (__mmask16)-1, (R)); })

#define _mm512_cvt_roundepu32_ps(A, R) __extension__ ({ \
  (__m512)__builtin_ia32_cvtudq2ps512_mask((__v16si)(A), \
                                           (__v16sf)_mm512_setzero_ps(), \
                                           (__mmask16)-1, (R)); })

static __inline __m512d __attribute__ (( __always_inline__, __nodebug__))
_mm512_cvtepi32_pd(__m256i __A)
{
  return (__m512d) __builtin_ia32_cvtdq2pd512_mask ((__v8si) __A,
                (__v8df)
                _mm512_setzero_pd (),
                (__mmask8) -1);
}

static __inline __m512d __attribute__ (( __always_inline__, __nodebug__))
_mm512_cvtepu32_pd(__m256i __A)
{
  return (__m512d) __builtin_ia32_cvtudq2pd512_mask ((__v8si) __A,
                (__v8df)
                _mm512_setzero_pd (),
                (__mmask8) -1);
}
#define _mm512_cvt_roundpd_ps(A, R) __extension__ ({ \
  (__m256)__builtin_ia32_cvtpd2ps512_mask((__v8df)(A), \
                                          (__v8sf)_mm256_setzero_ps(), \
                                          (__mmask8)-1, (R)); })

#define _mm512_cvtps_ph(A, I) __extension__ ({ \
  (__m256i)__builtin_ia32_vcvtps2ph512_mask((__v16sf)(A), (I), \
                                            (__v16hi)_mm256_setzero_si256(), \
                                            -1); })

static  __inline __m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_cvtph_ps(__m256i __A)
{
  return (__m512) __builtin_ia32_vcvtph2ps512_mask ((__v16hi) __A,
                (__v16sf)
                _mm512_setzero_ps (),
                (__mmask16) -1,
                _MM_FROUND_CUR_DIRECTION);
}

static __inline __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_cvttps_epi32(__m512 a)
{
  return (__m512i)
    __builtin_ia32_cvttps2dq512_mask((__v16sf) a,
                                     (__v16si) _mm512_setzero_si512 (),
                                     (__mmask16) -1, _MM_FROUND_CUR_DIRECTION);
}

#define _mm512_cvtt_roundps_epi32(A, R) __extension__ ({ \
  (__m512i)__builtin_ia32_cvttps2dq512_mask((__v16sf)(A), \
                                            (__v16si)_mm512_setzero_si512(), \
                                            (__mmask16)-1, (R)); })

#define _mm512_cvt_roundps_epi32(A, R) __extension__ ({ \
  (__m512i)__builtin_ia32_cvtps2dq512_mask((__v16sf)(A), \
                                           (__v16si)_mm512_setzero_si512(), \
                                           (__mmask16)-1, (R)); })

#define _mm512_cvt_roundpd_epi32(A, R) __extension__ ({ \
  (__m256i)__builtin_ia32_cvtpd2dq512_mask((__v8df)(A), \
                                           (__v8si)_mm256_setzero_si256(), \
                                           (__mmask8)-1, (R)); })

#define _mm512_cvt_roundps_epu32(A, R) __extension__ ({ \
  (__m512i)__builtin_ia32_cvtps2udq512_mask((__v16sf)(A), \
                                            (__v16si)_mm512_setzero_si512(), \
                                            (__mmask16)-1, (R)); })

#define _mm512_cvt_roundpd_epu32(A, R) __extension__ ({ \
  (__m256i)__builtin_ia32_cvtpd2udq512_mask((__v8df)(A), \
                                            (__v8si)_mm256_setzero_si256(), \
                                            (__mmask8) -1, (R)); })

/* Bit Test */

static __inline __mmask16 __attribute__ ((__always_inline__, __nodebug__))
_mm512_test_epi32_mask(__m512i __A, __m512i __B)
{
  return (__mmask16) __builtin_ia32_ptestmd512 ((__v16si) __A,
            (__v16si) __B,
            (__mmask16) -1);
}

__mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_test_epi64_mask(__m512i __A, __m512i __B);

/* SIMD load ops */

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_maskz_loadu_epi32(__mmask16 __U, void const *__P)
{
  return (__m512i) __builtin_ia32_loaddqusi512_mask ((__private const __v16si *)(const __v16si *)__P,
                                                     (__v16si)
                                                     _mm512_setzero_si512 (),
                                                     (__mmask16) __U);
}

__m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_maskz_loadu_epi64(__mmask8 __U, void const *__P);

static __inline __m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_maskz_loadu_ps(__mmask16 __U, void const *__P)
{
  return (__m512) __builtin_ia32_loadups512_mask ((__private const __v16sf *)(const __v16sf *)__P,
                                                  (__v16sf)
                                                  _mm512_setzero_ps (),
                                                  (__mmask16) __U);
}

static __inline __m512d __attribute__ ((__always_inline__, __nodebug__))
_mm512_maskz_loadu_pd(__mmask8 __U, void const *__P)
{
  return (__m512d) __builtin_ia32_loadupd512_mask ((__private const __v8df *)(const __v8df *)__P,
                                                   (__v8df)
                                                   _mm512_setzero_pd (),
                                                   (__mmask8) __U);
}

static __inline __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_loadu_pd(double const *__p)
{
  struct __loadu_pd {
    __m512d __v;
  } __attribute__((packed, may_alias));
  return ((struct __loadu_pd*)__p)->__v;
}

static __inline __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_loadu_ps(float const *__p)
{
  struct __loadu_ps {
    __m512 __v;
  } __attribute__((packed, may_alias));
  return ((struct __loadu_ps*)__p)->__v;
}

/* SIMD store ops */

void __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_storeu_epi64(void *__P, __mmask8 __U, __m512i __A);

static __inline void __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_storeu_epi32(void *__P, __mmask16 __U, __m512i __A)
{
  __builtin_ia32_storedqusi512_mask ((__private __v16si *)(__v16si *)__P, (__v16si) __A,
                                     (__mmask16) __U);
}

static __inline void __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_storeu_pd(void *__P, __mmask8 __U, __m512d __A)
{
  __builtin_ia32_storeupd512_mask ((__private __v8df *)(__v8df *)__P, (__v8df) __A, (__mmask8) __U);
}

static __inline void __attribute__ ((__always_inline__, __nodebug__))
_mm512_storeu_pd(void *__P, __m512d __A)
{
  __builtin_ia32_storeupd512_mask((__private __v8df *)(__v8df *)__P, (__v8df)__A, (__mmask8)-1);
}

static __inline void __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_storeu_ps(void *__P, __mmask16 __U, __m512 __A)
{
  __builtin_ia32_storeups512_mask ((__private __v16sf *)(__v16sf *)__P, (__v16sf) __A,
                                   (__mmask16) __U);
}

static __inline void __attribute__ ((__always_inline__, __nodebug__))
_mm512_storeu_ps(void *__P, __m512 __A)
{
  __builtin_ia32_storeups512_mask((__private __v16sf *)(__v16sf *)__P, (__v16sf)__A, (__mmask16)-1);
}

static __inline void __attribute__ ((__always_inline__, __nodebug__))
_mm512_store_ps(void *__P, __m512 __A)
{
  *(__m512*)__P = __A;
}

static __inline void __attribute__ ((__always_inline__, __nodebug__))
_mm512_store_pd(void *__P, __m512d __A)
{
  *(__m512d*)__P = __A;
}

#endif // __AVX512FINTRIN_H
