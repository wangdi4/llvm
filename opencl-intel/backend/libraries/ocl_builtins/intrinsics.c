// Implementation of X86 intrinsics which use clang built-ins with 'long long' parameters.
// It's moved from the header files included to OpenCL module with built-in functions where 'long long' size must be 128 bit, whereas X86 target supports only 64-bit 'long long' size.
typedef int __v4si __attribute__((__vector_size__(16)));

#if defined (__SSE4_2__) || defined (__SSE4_1__)
__v4si __attribute__((__always_inline__, __nodebug__))
_mm_mul_epi32 (__v4si __V1, __v4si __V2)
{
  return (__v4si) __builtin_ia32_pmuldq128 (__V1, __V2);
}

#endif

#ifdef __SSE2__
__v4si __attribute__((__always_inline__, __nodebug__))
_mm_mul_epu32(__v4si a, __v4si b)
{
  return __builtin_ia32_pmuludq128(a, b);
}
#endif

typedef double __v4df __attribute__((__vector_size__(32)));
typedef long long __v4di __attribute__((__vector_size__(32)));
typedef int __v8si __attribute__((__vector_size__(32)));

#ifdef __AVX2__
// AVX2
__v4di __attribute__((__always_inline__, __nodebug__))
_mm256_maskload_epi64(__v4di const *__X, __v4di __M)
{
  return __builtin_ia32_maskloadq256(__X, __M);
}

void __attribute__((__always_inline__, __nodebug__))
_mm256_maskstore_epi64(__v4di *__X, __v4di __M, __v4di __Y)
{
  __builtin_ia32_maskstoreq256(__X, __M, __Y);
}

__v8si __attribute__((__always_inline__, __nodebug__))
_mm256_mul_epi32(__v8si a, __v8si b)
{
  return __builtin_ia32_pmuldq256(a, b);
}

__v8si __attribute__((__always_inline__, __nodebug__))
_mm256_mul_epu32(__v8si a, __v8si b)
{
  return __builtin_ia32_pmuludq256(a, b);
}
#endif

#ifdef __AVX__
// AVX
__v4df __attribute__((__always_inline__, __nodebug__))
_mm256_maskload_pd(__v4df const *p, __v4di m)
{
  return __builtin_ia32_maskloadpd256(p, m);
}

void __attribute__((__always_inline__, __nodebug__))
_mm256_maskstore_pd(__v4df *p, __v4di m, __v4di a)
{
  __builtin_ia32_maskstorepd256(p, m, a);
}
#endif
