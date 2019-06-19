 #ifndef __INTRIN_PROMOTION_H
 #define __INTRIN_PROMOTION_H
typedef float __m128 __attribute__((__vector_size__(16), __aligned__(16)));
typedef double __m128d  __attribute__((__vector_size__(16), __aligned__(16)));
typedef double __v2df __attribute__ ((__vector_size__ (16)));
typedef float __v4sf __attribute__((__vector_size__(16)));

__attribute__((always_inline, target("avx512f")))
inline void fwd_decl_intrin();

__attribute__((always_inline, target("avx512f")))
inline void avx512f_intrin(){}

__attribute__((always_inline, target("bmi")))
inline void bmi_intrin(){}

__attribute__((always_inline, target("lzcnt")))
inline void lzcnt_intrin(){}

// Forces avx512f
#define _mm_cvt_roundsd_si32(A, R) \
     (int)__builtin_ia32_vcvtsd2si32((__v2df)(__m128d)(A), (int)(R))

  static __inline__ __m128 __attribute__((always_inline, target("fma4")))
_mm_macc_ps(__m128 __A, __m128 __B, __m128 __C) {
  return (__m128)__builtin_ia32_vfmaddps((__v4sf)__A, (__v4sf)__B, (__v4sf)__C);
}
 #endif /* __INTRIN_PROMOTION_H */
