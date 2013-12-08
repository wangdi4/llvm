#ifndef __KNLINTRIN_H
#define __KNLINTRIN_H

#if !defined(__AVX512F__)
#error "KNL is not enabled!"
#endif

typedef double __v8df __attribute__((__vector_size__(64)));
typedef float __v16sf __attribute__((__vector_size__(64)));
typedef long __v8di __attribute__((__vector_size__(64)));
typedef int __v16si __attribute__((__vector_size__(64)));

typedef float __m512 __attribute__((__vector_size__(64)));
typedef double __m512d __attribute__((__vector_size__(64)));
typedef long __m512i __attribute__((__vector_size__(64)));

typedef unsigned char __mmask8;
typedef unsigned short __mmask16;

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

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_rsqrt14_pd(__m512d a)
{
  return (__m512d)__builtin_ia32_rsqrt14pd512((__v8df)a);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_rsqrt14_ps(__m512 a)
{
  return (__m512)__builtin_ia32_rsqrt14ps512((__v16sf)a);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_rsqrt28_pd(__m512d a)
{
  return (__m512d)__builtin_ia32_rsqrt28pd512((__v8df)a);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_rsqrt28_ps(__m512 a)
{
  return (__m512)__builtin_ia32_rsqrt28ps512((__v16sf)a);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rsqrt14_ss(__m128 a)
{
  return (__m128)__builtin_ia32_rsqrt14ss((__v4sf)a);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rsqrt14_sd(__m128d a)
{
  return (__m128)__builtin_ia32_rsqrt14sd((__v2df)a);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rsqrt28_ss(__m128 a)
{
  return (__m128)__builtin_ia32_rsqrt28ss((__v4sf)a);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rsqrt28_sd(__m128d a)
{
  return (__m128)__builtin_ia32_rsqrt28sd((__v2df)a);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_rcp14_pd(__m512d a)
{
  return (__m512d)__builtin_ia32_rcp14pd512((__v8df)a);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_rcp14_ps(__m512 a)
{
  return (__m512)__builtin_ia32_rcp14ps512((__v16sf)a);
}

__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_rcp28_pd(__m512d a)
{
  return (__m512d)__builtin_ia32_rcp28pd512((__v8df)a);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_rcp28_ps(__m512 a)
{
  return (__m512)__builtin_ia32_rcp28ps512((__v16sf)a);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rcp14_ss(__m128 a)
{
  return (__m128)__builtin_ia32_rcp14ss((__v4sf)a);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rcp14_sd(__m128d a)
{
  return (__m128)__builtin_ia32_rcp14sd((__v2df)a);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rcp28_ss(__m128 a)
{
  return (__m128)__builtin_ia32_rcp28ss((__v4sf)a);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rcp28_sd(__m128d a)
{
  return (__m128)__builtin_ia32_rcp28sd((__v2df)a);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_max_pd(__m512d a, __m512d b)
{
  return (__m512d)__builtin_ia32_maxpd512((__v8df)a, (__v8df)b);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_max_ps(__m512 a, __m512 b)
{
  return (__m512)__builtin_ia32_maxps512((__v16sf)a, (__v16sf)b);
}
__inline__ __m512d __attribute__((__always_inline__, __nodebug__))
_mm512_min_pd(__m512d a, __m512d b)
{
  return (__m512d)__builtin_ia32_minpd512((__v8df)a, (__v8df)b);
}

__inline__ __m512 __attribute__((__always_inline__, __nodebug__))
_mm512_min_ps(__m512 a, __m512 b)
{
  return (__m512)__builtin_ia32_minps512((__v16sf)a, (__v16sf)b);
}

__inline __m512 __attribute__ ((__always_inline__, __nodebug__))
_mm512_cvtph_ps (__m256i x)
{
  return (__m512) __builtin_ia32_vcvtph2ps512 ((__v16hi)x);
}

#define _mm512_cvtps_ph(x, imm) ((__m256i) __builtin_ia32_vcvtps2ph512 ((__v16sf)x, imm))


#endif // __KNLINTRIN_H
