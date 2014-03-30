#ifndef __KNLINTRIN_H
#define __KNLINTRIN_H

#if !defined(__AVX512F__)
#error "KNL is not enabled!"
#endif

typedef double __v8df __attribute__((__vector_size__(64)));
typedef float __v16sf __attribute__((__vector_size__(64)));
typedef long long __v8di __attribute__((__vector_size__(64)));
typedef int __v16si __attribute__((__vector_size__(64)));

typedef float __m512 __attribute__((__vector_size__(64)));
typedef double __m512d __attribute__((__vector_size__(64)));
typedef long long __m512i __attribute__((__vector_size__(64)));

typedef unsigned char __mmask8;
typedef unsigned short __mmask16;

/* Rounding mode macros.  */
#define _MM_FROUND_TO_NEAREST_INT   0x00
#define _MM_FROUND_TO_NEG_INF       0x01
#define _MM_FROUND_TO_POS_INF       0x02
#define _MM_FROUND_TO_ZERO          0x03
#define _MM_FROUND_CUR_DIRECTION    0x04

 __inline __m512 __attribute__ ((__always_inline__, __nodebug__))
 _mm512_setzero_ps (void)
{
  return (__m512){ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                   0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
}
__inline __m512d __attribute__ ((__always_inline__, __nodebug__))
_mm512_setzero_pd (void)
{
  return (__m512d){ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
}
__inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_setzero_si512 (void)
{
  return (__m512i)(__v8di){ 0, 0, 0, 0, 0, 0, 0, 0 };
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_sqrt_pd(__m512d a)
{
  return (__m512d)__builtin_ia32_sqrtpd512((__v8df)a);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_sqrt_ps(__m512 a)
{
  return (__m512)__builtin_ia32_sqrtps512((__v16sf)a);
}

// rsqrt14
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_rsqrt14_pd(__m512d __A)
{
  return (__m512d) __builtin_ia32_rsqrt14pd512_mask ((__v8df) __A,
                 (__v8df)
                 _mm512_setzero_pd (),
                 (__mmask8) -1);}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_rsqrt14_ps(__m512 __A)
{
  return (__m512) __builtin_ia32_rsqrt14ps512_mask ((__v16sf) __A,
                (__v16sf)
                _mm512_setzero_ps (),
                (__mmask16) -1);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rsqrt14_ss(__m128 __A, __m128 __B)
{
  return (__m128) __builtin_ia32_rsqrt14ss_mask ((__v4sf) __A,
             (__v4sf) __B,
             (__v4sf)
             _mm_setzero_ps (),
             (__mmask8) -1);
}

__inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_rsqrt14_sd(__m128d __A, __m128d __B)
{
  return (__m128d) __builtin_ia32_rsqrt14sd_mask ((__v2df) __A,
              (__v2df) __B,
              (__v2df)
              _mm_setzero_pd (),
              (__mmask8) -1);
}

// rsqrt28
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_rsqrt28_round_pd (__m512d __A, int __R)
{
  return (__m512d)__builtin_ia32_rsqrt28pd_mask ((__v8df)__A,
                                                 (__v8df)_mm512_setzero_pd(),
                                                 (__mmask8)-1,
                                                 __R);
}
__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_rsqrt28_round_ps(__m512 __A, int __R)
{
  return (__m512)__builtin_ia32_rsqrt28ps_mask ((__v16sf)__A,
                                                (__v16sf)_mm512_setzero_ps(),
                                                (__mmask16)-1,
                                                __R);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rsqrt28_round_ss(__m128 __A, __m128 __B, int __R)
{
  return (__m128) __builtin_ia32_rsqrt28ss_mask ((__v4sf) __A,
             (__v4sf) __B,
             (__v4sf)
             _mm_setzero_ps (),
             (__mmask8) -1,
             __R);
}

__inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_rsqrt28_round_sd (__m128d __A, __m128d __B, int __R)
{
  return (__m128d) __builtin_ia32_rsqrt28sd_mask ((__v2df) __A,
              (__v2df) __B,
              (__v2df)
              _mm_setzero_pd (),
              (__mmask8) -1,
             __R);
}

// rcp14
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_rcp14_pd(__m512d __A)
{
  return (__m512d) __builtin_ia32_rcp14pd512_mask ((__v8df) __A,
               (__v8df)
               _mm512_setzero_pd (),
               (__mmask8) -1);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_rcp14_ps(__m512 __A)
{
  return (__m512) __builtin_ia32_rcp14ps512_mask ((__v16sf) __A,
              (__v16sf)
              _mm512_setzero_ps (),
              (__mmask16) -1);
}
__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rcp14_ss (__m128 __A, __m128 __B)
{
  return (__m128) __builtin_ia32_rcp14ss_mask ((__v4sf) __A,
                 (__v4sf) __B,
                 (__v4sf)
                 _mm_setzero_ps (),
                 (__mmask8) -1);
}

__inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_rcp14_sd (__m128d __A, __m128d __B)
{
  return (__m128d) __builtin_ia32_rcp14sd_mask ((__v2df) __A,
            (__v2df) __B,
            (__v2df)
            _mm_setzero_pd (),
            (__mmask8) -1);
}

// rcp28
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_rcp28_round_pd (__m512d __A, int __R)
{
  return (__m512d)__builtin_ia32_rcp28pd_mask ((__v8df)__A,
                                               (__v8df)_mm512_setzero_pd(),
                                               (__mmask8)-1,
                                               __R);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_rcp28_round_ps (__m512 __A, int __R)
{
  return (__m512)__builtin_ia32_rcp28ps_mask ((__v16sf)__A,
                                              (__v16sf)_mm512_setzero_ps (),
                                              (__mmask16)-1,
                                              __R);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rcp28_round_ss (__m128 __A, __m128 __B, int __R)
{
  return (__m128) __builtin_ia32_rcp28ss_mask ((__v4sf) __A,
             (__v4sf) __B,
             (__v4sf)
             _mm_setzero_ps (),
             (__mmask8) -1,
             __R);
}
__inline__ __m128d __attribute__((__always_inline__, __nodebug__))
_mm_rcp28_round_sd (__m128d __A, __m128d __B, int __R)
{
  return (__m128d) __builtin_ia32_rcp28sd_mask ((__v2df) __A,
              (__v2df) __B,
              (__v2df)
              _mm_setzero_pd (),
              (__mmask8) -1,
             __R);
}

// min/max
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_max_pd(__m512d __A, __m512d __B)
{
  return (__m512d) __builtin_ia32_maxpd512_mask ((__v8df) __A,
             (__v8df) __B,
             (__v8df)
             _mm512_setzero_pd (),
             (__mmask8) -1,
             _MM_FROUND_CUR_DIRECTION);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_max_ps(__m512 __A, __m512 __B)
{
  return (__m512) __builtin_ia32_maxps512_mask ((__v16sf) __A,
            (__v16sf) __B,
            (__v16sf)
            _mm512_setzero_ps (),
            (__mmask16) -1,
            _MM_FROUND_CUR_DIRECTION);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_min_pd(__m512d __A, __m512d __B)
{
  return (__m512d) __builtin_ia32_minpd512_mask ((__v8df) __A,
             (__v8df) __B,
             (__v8df)
             _mm512_setzero_pd (),
             (__mmask8) -1,
             _MM_FROUND_CUR_DIRECTION);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_min_ps(__m512 __A, __m512 __B)
{
  return (__m512) __builtin_ia32_minps512_mask ((__v16sf) __A,
            (__v16sf) __B,
            (__v16sf)
            _mm512_setzero_ps (),
            (__mmask16) -1,
            _MM_FROUND_CUR_DIRECTION);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm512_cvtps_ph (__m512 __A, const int __I)
{
  return (__m256i) __builtin_ia32_vcvtps2ph512_mask ((__v16sf) __A,
                 __I,
                 (__v16hi)
                 _mm256_setzero_si256 (),
                 -1);
}

__inline __m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_cvtph_ps (__m256i __A)
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

static __inline __m256i __attribute__((__always_inline__, __nodebug__))
_mm512_cvttpd_epi32(__m512d a)
{
  return (__m256i)__builtin_ia32_cvttpd2dq512_mask((__v8df) a,
                                                   (__v8si)_mm256_setzero_si256(),
                                                   (__mmask8) -1,
                                                    _MM_FROUND_CUR_DIRECTION);
}

static __inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm512_cvtt_roundpd_epi32 (__m512d __A, const int __R)
{
  return (__m256i) __builtin_ia32_cvttpd2dq512_mask ((__v8df) __A,
                 (__v8si)
                 _mm256_setzero_si256 (),
                 (__mmask8) -1,
                 __R);
}
static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_cvtt_roundps_epi32 (__m512 __A, const int __R)
{
  return (__m512i) __builtin_ia32_cvttps2dq512_mask ((__v16sf) __A,
                 (__v16si)
                 _mm512_setzero_si512 (),
                 (__mmask16) -1,
                 __R);
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_cvt_roundps_epi32 (__m512 __A, const int __R)
{
  return (__m512i) __builtin_ia32_cvtps2dq512_mask ((__v16sf) __A,
                (__v16si)
                _mm512_setzero_si512 (),
                (__mmask16) -1,
                __R);
}
static __inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm512_cvt_roundpd_epi32 (__m512d __A, const int __R)
{
  return (__m256i) __builtin_ia32_cvtpd2dq512_mask ((__v8df) __A,
                (__v8si)
                _mm256_setzero_si256 (),
                (__mmask8) -1,
                __R);
}
static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_cvt_roundps_epu32 (__m512 __A, const int __R)
{
  return (__m512i) __builtin_ia32_cvtps2udq512_mask ((__v16sf) __A,
                (__v16si)
                _mm512_setzero_si512 (),
                (__mmask16) -1,
                __R);
}
static __inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm512_cvt_roundpd_epu32 (__m512d __A, const int __R)
{
  return (__m256i) __builtin_ia32_cvtpd2udq512_mask ((__v8df) __A,
                (__v8si)
                _mm256_setzero_si256 (),
                (__mmask8) -1,
                __R);
}

static __inline __m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_roundscale_ps (__m512 __A, const int __imm)
{
  return (__m512) __builtin_ia32_rndscaleps_mask ((__v16sf) __A, __imm,
              (__v16sf) _mm512_setzero_ps (), -1,
              _MM_FROUND_CUR_DIRECTION);
}
static __inline __m512d __attribute__ ((__always_inline__, __nodebug__))
_mm512_roundscale_pd (__m512d __A, const int __imm)
{
  return (__m512d) __builtin_ia32_rndscalepd_mask ((__v8df) __A, __imm,
               (__v8df)
               _mm512_setzero_pd (), -1,
               _MM_FROUND_CUR_DIRECTION);
}

static __inline __mmask16 __attribute__ ((__always_inline__, __nodebug__))
_mm512_cmp_ps_mask (__m512 a, __m512 b, const int p)
{
  return (__mmask16) __builtin_ia32_cmpps512_mask ((__v16sf) a,
               (__v16sf) b, p, (__mmask16) -1,
               _MM_FROUND_CUR_DIRECTION);
}

static __inline __mmask8 __attribute__ ((__always_inline__, __nodebug__)) 
_mm512_cmp_pd_mask (__m512d __X, __m512d __Y, const int __P)
{
  return (__mmask8) __builtin_ia32_cmppd512_mask ((__v8df) __X,
              (__v8df) __Y, __P,
              (__mmask8) -1,
              _MM_FROUND_CUR_DIRECTION);
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_cvttps_epu32 (__m512 __A)
{
  return (__m512i) __builtin_ia32_cvttps2udq512_mask ((__v16sf) __A,
                  (__v16si)
                  _mm512_setzero_si512 (),
                  (__mmask16) -1,
                  _MM_FROUND_CUR_DIRECTION);
}

static __inline __m512 __attribute__ (( __always_inline__, __nodebug__))
_mm512_cvt_roundepi32_ps (__m512i __A, const int __R)
{
  return (__m512) __builtin_ia32_cvtdq2ps512_mask ((__v16si) __A,
               (__v16sf)
               _mm512_setzero_ps (),
               (__mmask16) -1,
               __R);
}

static __inline __m512 __attribute__ (( __always_inline__, __nodebug__))
_mm512_cvt_roundepu32_ps (__m512i __A, const int __R)
{
  return (__m512) __builtin_ia32_cvtudq2ps512_mask ((__v16si) __A,
               (__v16sf)
               _mm512_setzero_ps (),
               (__mmask16) -1,
               __R);
}

static __inline __m512d __attribute__ (( __always_inline__, __nodebug__))
_mm512_cvtepi32_pd (__m256i __A)
{
  return (__m512d) __builtin_ia32_cvtdq2pd512_mask ((__v8si) __A,
                (__v8df)
                _mm512_setzero_pd (),
                (__mmask8) -1);
}

static __inline __m512d __attribute__ (( __always_inline__, __nodebug__))
_mm512_cvtepu32_pd (__m256i __A)
{
  return (__m512d) __builtin_ia32_cvtudq2pd512_mask ((__v8si) __A,
                (__v8df)
                _mm512_setzero_pd (),
                (__mmask8) -1);
}
static __inline __m256 __attribute__ (( __always_inline__, __nodebug__))
_mm512_cvt_roundpd_ps (__m512d __A, const int __R)
{
  return (__m256) __builtin_ia32_cvtpd2ps512_mask ((__v8df) __A,
               (__v8sf)
               _mm256_setzero_ps (),
               (__mmask8) -1,
               __R);
}

static __inline __m512i __attribute__ (( __always_inline__, __nodebug__))
_mm512_abs_epi64 (__m512i __A)
{
  return (__m512i) __builtin_ia32_pabsq512_mask ((__v8di) __A,
             (__v8di)
             _mm512_setzero_si512 (),
             (__mmask8) -1);
}

static __inline __m512i __attribute__ (( __always_inline__, __nodebug__))
_mm512_abs_epi32 (__m512i __A)
{
  return (__m512i) __builtin_ia32_pabsd512_mask ((__v16si) __A,
             (__v16si)
             _mm512_setzero_si512 (),
             (__mmask16) -1);
}

static __inline __m512i
__attribute__ ((__always_inline__, __nodebug__))
_mm512_max_epi32 (__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxsd512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) -1);
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_max_epu32 (__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxud512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) -1);
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_max_epi64 (__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxsq512_mask ((__v8di) __A,
              (__v8di) __B,
              (__v8di)
              _mm512_setzero_si512 (),
              (__mmask8) -1);
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_max_epu64 (__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pmaxuq512_mask ((__v8di) __A,
              (__v8di) __B,
              (__v8di)
              _mm512_setzero_si512 (),
              (__mmask8) -1);
}
static __inline __m512i
__attribute__ ((__always_inline__, __nodebug__))
_mm512_min_epi32 (__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminsd512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) -1);
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_min_epu32 (__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminud512_mask ((__v16si) __A,
              (__v16si) __B,
              (__v16si)
              _mm512_setzero_si512 (),
              (__mmask16) -1);
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_min_epi64 (__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminsq512_mask ((__v8di) __A,
              (__v8di) __B,
              (__v8di)
              _mm512_setzero_si512 (),
              (__mmask8) -1);
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_min_epu64 (__m512i __A, __m512i __B)
{
  return (__m512i) __builtin_ia32_pminuq512_mask ((__v8di) __A,
              (__v8di) __B,
              (__v8di)
              _mm512_setzero_si512 (),
              (__mmask8) -1);
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_mul_epi32 (__m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_pmuldq512_mask ((__v16si) __X,
              (__v16si) __Y,
              (__v8di)
              _mm512_setzero_si512 (),
              (__mmask8) -1);
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_mul_epu32 (__m512i __X, __m512i __Y)
{
  return (__m512i) __builtin_ia32_pmuludq512_mask ((__v16si) __X,
               (__v16si) __Y,
               (__v8di)
               _mm512_setzero_si512 (),
               (__mmask8) -1);
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_blend_epi64 (__mmask8 __U, __m512i __A, __m512i __W)
{
  return (__m512i) __builtin_ia32_blendmq_512_mask ((__v8di) __A,
                (__v8di) __W,
                (__mmask8) __U);
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_blend_epi32 (__mmask16 __U, __m512i __A, __m512i __W)
{
  return (__m512i) __builtin_ia32_blendmd_512_mask ((__v16si) __A,
                (__v16si) __W,
                (__mmask16) __U);
}

static __inline __mmask16 __attribute__ ((__always_inline__, __nodebug__))
_mm512_test_epi32_mask (__m512i __A, __m512i __B)
{
  return (__mmask16) __builtin_ia32_ptestmd512 ((__v16si) __A,
            (__v16si) __B,
            (__mmask16) -1);
}

static __inline __mmask8 __attribute__ ((__always_inline__, __nodebug__))
_mm512_test_epi64_mask (__m512i __A, __m512i __B)
{
  return (__mmask8) __builtin_ia32_ptestmq512 ((__v8di) __A,
                 (__v8di) __B,
                 (__mmask8) -1);
}

static __inline __m512d __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_blend_pd (__mmask8 __U, __m512d __A, __m512d __W)
{
  return (__m512d) __builtin_ia32_blendmpd_512_mask ((__v8df) __A,
                 (__v8df) __W,
                 (__mmask8) __U);
}

static __inline __m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_blend_ps (__mmask16 __U, __m512 __A, __m512 __W)
{
  return (__m512) __builtin_ia32_blendmps_512_mask ((__v16sf) __A,
                (__v16sf) __W,
                (__mmask16) __U);
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_maskz_set1_epi32 (__mmask16 __M, int __A)
{
  return (__m512i) __builtin_ia32_pbroadcastd512_gpr_mask (__A,
                 (__v16si)
                 _mm512_setzero_si512 (),
                 __M);
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_maskz_set1_epi64 (__mmask8 __M, long long __A)
{
#ifdef __x86_64__
  return (__m512i) __builtin_ia32_pbroadcastq512_gpr_mask (__A,
                 (__v8di)
                 _mm512_setzero_si512 (),
                 __M);
#else
  return (__m512i) __builtin_ia32_pbroadcastq512_mem_mask (__A,
                 (__v8di)
                 _mm512_setzero_si512 (),
                 __M);
#endif
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_maskz_loadu_epi32 (__mmask16 __U, void const *__P)
{
  return (__m512i) __builtin_ia32_loaddqusi512_mask ((const __v16si *)__P,
                                                     (__v16si)
                                                     _mm512_setzero_si512 (),
                                                     (__mmask16) __U);
}

static __inline __m512i __attribute__ ((__always_inline__, __nodebug__))
_mm512_maskz_loadu_epi64 (__mmask8 __U, void const *__P)
{
  return (__m512i) __builtin_ia32_loaddqudi512_mask ((const __v8di *)__P,
                                                     (__v8di)
                                                     _mm512_setzero_si512 (),
                                                     (__mmask8) __U);
}

static __inline __m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_maskz_loadu_ps (__mmask16 __U, void const *__P)
{
  return (__m512) __builtin_ia32_loadups512_mask ((const __v16sf *)__P,
                                                  (__v16sf)
                                                  _mm512_setzero_ps (),
                                                  (__mmask16) __U);
}

static __inline __m512d __attribute__ ((__always_inline__, __nodebug__))
_mm512_maskz_loadu_pd (__mmask8 __U, void const *__P)
{
  return (__m512d) __builtin_ia32_loadupd512_mask ((const __v8df *)__P,
                                                   (__v8df)
                                                   _mm512_setzero_pd (),
                                                   (__mmask8) __U);
}

static __inline void __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_storeu_epi64 (void *__P, __mmask8 __U, __m512i __A)
{
  __builtin_ia32_storedqudi512_mask ((__v8di *)__P, (__v8di) __A,
                                     (__mmask8) __U);
}

static __inline void __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_storeu_epi32 (void *__P, __mmask16 __U, __m512i __A)
{
  __builtin_ia32_storedqusi512_mask ((__v16si *)__P, (__v16si) __A,
                                     (__mmask16) __U);
}

static __inline void __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_storeu_pd (void *__P, __mmask8 __U, __m512d __A)
{
  __builtin_ia32_storeupd512_mask ((__v8df *)__P, (__v8df) __A, (__mmask8) __U);
}

static __inline void __attribute__ ((__always_inline__, __nodebug__))
_mm512_mask_storeu_ps (void *__P, __mmask16 __U, __m512 __A)
{
  __builtin_ia32_storeups512_mask ((__v16sf *)__P, (__v16sf) __A,
                                   (__mmask16) __U);
}



#endif // __KNLINTRIN_H
