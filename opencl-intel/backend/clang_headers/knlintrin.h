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

#endif // __KNLINTRIN_H
