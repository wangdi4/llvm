// Implementation of X86 intrinsics which use clang built-ins with 'long long' parameters.
// It's moved from the header files included to OpenCL module with built-in functions where 'long long' size must be 128 bit, whereas X86 target supports only 64-bit 'long long' size.

#include <intrin.h>

#if defined (__SSE4_2__) || defined (__SSE4_1__)
__m128i __attribute__((__always_inline__, __nodebug__))
_mm_mul_epi32 (__m128i __V1, __m128i  __V2)
{
  return (__v4si) __builtin_ia32_pmuldq128 (__V1, __V2);
}

#endif

#ifdef __SSE2__
__m128i __attribute__((__always_inline__, __nodebug__))
_mm_mul_epu32(__m128i a, __m128i b)
{
  return __builtin_ia32_pmuludq128(a, b);
}
#endif

#ifdef __AVX2__
__v4di __attribute__((__always_inline__, __nodebug__))
_mm256_maskload_epi64(long long const *__X, __v4di __M)
{
  return __builtin_ia32_maskloadq256((const __v4di *) __X, __M);
}

void __attribute__((__always_inline__, __nodebug__))
_mm256_maskstore_epi64(long long *__X, __v4di __M, __v4di __Y)
{
  __builtin_ia32_maskstoreq256((__v4di *)__X, __M, __Y);
}

__m256i __attribute__((__always_inline__, __nodebug__))
_mm256_mul_epi32(__m256i a, __m256i b)
{
  return __builtin_ia32_pmuldq256(a, b);
}

__m256i __attribute__((__always_inline__, __nodebug__))
_mm256_mul_epu32(__m256i a, __m256i b)
{
  return __builtin_ia32_pmuludq256(a, b);
}
#endif

#ifdef __AVX__
__m256d __attribute__((__always_inline__, __nodebug__))
_mm256_maskload_pd(double const *p, __m256i m)
{
  return __builtin_ia32_maskloadpd256((const __v4df *)p, m);
}

void __attribute__((__always_inline__, __nodebug__))
_mm256_maskstore_pd(double *p, __m256i m, __m256d a)
{
  __builtin_ia32_maskstorepd256((__v4df *)p, m, a);
}

#endif

#ifdef __AVX512F__
__m512i __attribute__((__always_inline__, __nodebug__))
_mm512_maskz_set1_epi64(__mmask8 __M, long long __A)
{
#ifdef __x86_64__
  return (__m512i)__builtin_ia32_pbroadcastq512_gpr_mask(__A,
    (__v8di)
    _mm512_setzero_si512(),
    __M);
#else
  return (__m512i)__builtin_ia32_pbroadcastq512_mem_mask(__A,
    (__v8di)
    _mm512_setzero_si512(),
    __M);
#endif
}

__m512i __attribute__((__always_inline__, __nodebug__))
_mm512_maskz_loadu_epi64(__mmask8 __U, void const *__P)
{
  return (__m512i) __builtin_ia32_loaddqudi512_mask ((const long long *)__P,
                                                     (__v8di)
                                                     _mm512_setzero_si512 (),
                                                     (__mmask8) __U);
}

void __attribute__((__always_inline__, __nodebug__))
_mm512_mask_storeu_epi64(void *__P, __mmask8 __U, __m512i __A)
{
  __builtin_ia32_storedqudi512_mask ((long long *)__P, (__v8di) __A,
                                     (__mmask8) __U);
}

__m512i __attribute__((__always_inline__, __nodebug__))
_mm512_abs_epi32(__m512i __A)
{
  return (__m512i)__builtin_ia32_pabsd512_mask((__v16si)__A,
    (__v16si)
    _mm512_setzero_si512(),
    (__mmask16)-1);
}

__m512i __attribute__((__always_inline__, __nodebug__))
_mm512_abs_epi64(__m512i __A)
{
  return (__m512i)__builtin_ia32_pabsq512_mask((__v8di)__A,
    (__v8di)
    _mm512_setzero_si512(),
    (__mmask8)-1);
}

__m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mul_epi32(__m512i __X, __m512i __Y)
{
  return (__m512i)__builtin_ia32_pmuldq512((__v16si)__X, (__v16si) __Y);
}

__m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mul_epu32(__m512i __X, __m512i __Y)
{
  return (__m512i)__builtin_ia32_pmuludq512((__v16si)__X, (__v16si)__Y);
}

__m512i __attribute__((__always_inline__, __nodebug__))
_mm512_max_epi64(__m512i __A, __m512i __B)
{
  return (__m512i)__builtin_ia32_pmaxsq512_mask((__v8di)__A,
    (__v8di)__B,
    (__v8di)
    _mm512_setzero_si512(),
    (__mmask8)-1);
}

__m512i __attribute__((__always_inline__, __nodebug__))
_mm512_max_epu64(__m512i __A, __m512i __B)
{
  return (__m512i)__builtin_ia32_pmaxuq512_mask((__v8di)__A,
    (__v8di)__B,
    (__v8di)
    _mm512_setzero_si512(),
    (__mmask8)-1);
}

__m512i __attribute__((__always_inline__, __nodebug__))
_mm512_min_epi64(__m512i __A, __m512i __B)
{
  return (__m512i)__builtin_ia32_pminsq512_mask((__v8di)__A,
    (__v8di)__B,
    (__v8di)
    _mm512_setzero_si512(),
    (__mmask8)-1);
}

__m512i __attribute__((__always_inline__, __nodebug__))
_mm512_min_epu64(__m512i __A, __m512i __B)
{
  return (__m512i)__builtin_ia32_pminuq512_mask((__v8di)__A,
    (__v8di)__B,
    (__v8di)
    _mm512_setzero_si512(),
    (__mmask8)-1);
}

__m512 __attribute__((__always_inline__, __nodebug__))
_mm512_min_ps(__m512 __A, __m512 __B)
{
  return (__m512)__builtin_ia32_minps512_mask((__v16sf)__A,
    (__v16sf)__B,
    (__v16sf)
    _mm512_setzero_ps(),
    (__mmask16)-1,
    _MM_FROUND_CUR_DIRECTION);
}

__m512d __attribute__((__always_inline__, __nodebug__))
_mm512_rsqrt14_pd(__m512d __A)
{
  return (__m512d)__builtin_ia32_rsqrt14pd512_mask((__v8df)__A,
    (__v8df)
    _mm512_setzero_pd(),
    (__mmask8)-1);
}

__m512d __attribute__((__always_inline__, __nodebug__))
_mm512_sqrt_pd(__m512d a)
{
  return (__m512d)__builtin_ia32_sqrtpd512_mask((__v8df)a,
    (__v8df)_mm512_setzero_pd(),
    (__mmask8)-1,
    _MM_FROUND_CUR_DIRECTION);
}

/* Vector Blend */

__m512d __attribute__((__always_inline__, __nodebug__))
_mm512_mask_blend_pd(__mmask8 __U, __m512d __A, __m512d __W)
{
  return (__m512d) __builtin_ia32_selectpd_512 ((__mmask8) __U,
                 (__v8df) __W,
                 (__v8df) __A);
}

__m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_blend_epi32(__mmask16 __U, __m512i __A, __m512i __W)
{
  return (__m512i) __builtin_ia32_selectd_512 ((__mmask16) __U,
                (__v16si) __W,
                (__v16si) __A);
}

__m512 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_blend_ps(__mmask16 __U, __m512 __A, __m512 __W)
{
  return (__m512) __builtin_ia32_selectps_512 ((__mmask16) __U,
                (__v16sf) __W,
                (__v16sf) __A);
}

/* Bit Test */

__mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_test_epi64_mask(__m512i __A, __m512i __B)
{
  return (__mmask8)__builtin_ia32_ptestmq512((__v8di)__A,
    (__v8di)__B,
    (__mmask8)-1);
}

#endif
