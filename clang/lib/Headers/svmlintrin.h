/*===---- svmlintrin.h - SVML intrinsics -----------------------------------===
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
#error "Never use <svmlintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __SVMLINTRIN_H
#define __SVMLINTRIN_H

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__XMMINTRIN_H) && defined(__EMMINTRIN_H)

/* Define the default attributes for the functions in this section. */
#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__, __target__("sse2"), __min_vector_width__(128)))

extern __m128 __svml_acoshf4(__m128);

/// Compute the inverse hyperbolic cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the 
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_acosh_ps(__m128 __a)
{
  return __svml_acoshf4(__a);
}

extern __m128d __svml_acosh2(__m128d);

/// Compute the inverse hyperbolic cosine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_acosh_pd(__m128d __a)
{
  return __svml_acosh2(__a);
}

extern __m128 __svml_acosf4(__m128);

/// Compute the inverse cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_acos_ps(__m128 __a)
{
  return __svml_acosf4(__a);
}

extern __m128d __svml_acos2(__m128d);

/// Compute the inverse cosine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_acos_pd(__m128d __a)
{
  return __svml_acos2(__a);
}

extern __m128 __svml_asinhf4(__m128);

/// Compute the inverse hyperbolic sine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_asinh_ps(__m128 __a)
{
  return __svml_asinhf4(__a);
}

extern __m128d __svml_asinh2(__m128d);

/// Compute the inverse hyperbolic sine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_asinh_pd(__m128d __a)
{
  return __svml_asinh2(__a);
}

extern __m128 __svml_asinf4(__m128);

/// Compute the inverse sine of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_asin_ps(__m128 __a)
{
  return __svml_asinf4(__a);
}

extern __m128d __svml_asin2(__m128d);

/// Compute the inverse sine of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_asin_pd(__m128d __a)
{
  return __svml_asin2(__a);
}

extern __m128 __svml_atan2f4(__m128, __m128);

/// Compute the inverse tangent of packed single-precision (32-bit)
///    floating-point elements in a divided by packed elements in b, and store
///    the results in dst expressed in radians.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_atan2_ps(__m128 __a, __m128 __b)
{
  return __svml_atan2f4(__a, __b);
}

extern __m128d __svml_atan22(__m128d, __m128d);

/// Compute the inverse tangent of packed double-precision (64-bit)
///    floating-point elements in a divided by packed elements in b, and store
///    the results in dst expressed in radians.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_atan2_pd(__m128d __a, __m128d __b)
{
  return __svml_atan22(__a, __b);
}

extern __m128 __svml_atanhf4(__m128);

/// Compute the inverse hyperbolic tangent of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_atanh_ps(__m128 __a)
{
  return __svml_atanhf4(__a);
}

extern __m128d __svml_atanh2(__m128d);

/// Compute the inverse hyperbolic tangent of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_atanh_pd(__m128d __a)
{
  return __svml_atanh2(__a);
}

extern __m128 __svml_atanf4(__m128);

/// Compute the inverse tangent of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_atan_ps(__m128 __a)
{
  return __svml_atanf4(__a);
}

extern __m128d __svml_atan2(__m128d);

/// Compute the inverse tangent of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_atan_pd(__m128d __a)
{
  return __svml_atan2(__a);
}

extern __m128 __svml_cbrtf4(__m128);

/// Compute the cube root of packed single-precision (32-bit) floating-point
///    elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_cbrt_ps(__m128 __a)
{
  return __svml_cbrtf4(__a);
}

extern __m128d __svml_cbrt2(__m128d);

/// Compute the cube root of packed double-precision (64-bit) floating-point
///    elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_cbrt_pd(__m128d __a)
{
  return __svml_cbrt2(__a);
}

extern __m128 __svml_coshf4(__m128);

/// Compute the hyperbolic cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_cosh_ps(__m128 __a)
{
  return __svml_coshf4(__a);
}

extern __m128d __svml_cosh2(__m128d);

/// Compute the hyperbolic cosine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_cosh_pd(__m128d __a)
{
  return __svml_cosh2(__a);
}

extern __m128 __svml_cosf4(__m128);

/// Compute the cosine of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_cos_ps(__m128 __a)
{
  return __svml_cosf4(__a);
}

extern __m128d __svml_cos2(__m128d);

/// Compute the cosine of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_cos_pd(__m128d __a)
{
  return __svml_cos2(__a);
}

extern __m128 __svml_erfcf4(__m128);

/// Compute the complementary error function of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_erfc_ps(__m128 __a)
{
  return __svml_erfcf4(__a);
}

extern __m128d __svml_erfc2(__m128d);

/// Compute the complementary error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_erfc_pd(__m128d __a)
{
  return __svml_erfc2(__a);
}

extern __m128 __svml_erfinvf4(__m128);

/// Compute the inverse error function of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_erfinv_ps(__m128 __a)
{
  return __svml_erfinvf4(__a);
}

extern __m128d __svml_erfinv2(__m128d);

/// Compute the inverse error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_erfinv_pd(__m128d __a)
{
  return __svml_erfinv2(__a);
}

extern __m128 __svml_erff4(__m128);

/// Compute the error function of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_erf_ps(__m128 __a)
{
  return __svml_erff4(__a);
}

extern __m128d __svml_erf2(__m128d);

/// Compute the error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_erf_pd(__m128d __a)
{
  return __svml_erf2(__a);
}

extern __m128 __svml_exp2f4(__m128);

/// Compute the exponential value of 2 raised to the power of packed
///    single-precision (32-bit) floating-point elements in a, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_exp2_ps(__m128 __a)
{
  return __svml_exp2f4(__a);
}

extern __m128d __svml_exp22(__m128d);

/// Compute the exponential value of 2 raised to the power of packed
///    double-precision (64-bit) floating-point elements in a, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_exp2_pd(__m128d __a)
{
  return __svml_exp22(__a);
}

extern __m128 __svml_expf4(__m128);

/// Compute the exponential value of e raised to the power of packed
///    single-precision (32-bit) floating-point elements in a, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_exp_ps(__m128 __a)
{
  return __svml_expf4(__a);
}

extern __m128d __svml_exp2(__m128d);

/// Compute the exponential value of e raised to the power of packed
///    double-precision (64-bit) floating-point elements in a, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_exp_pd(__m128d __a)
{
  return __svml_exp2(__a);
}

extern __m128 __svml_invcbrtf4(__m128);

/// Compute the inverse cube root of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_invcbrt_ps(__m128 __a)
{
  return __svml_invcbrtf4(__a);
}

extern __m128d __svml_invcbrt2(__m128d);

/// Compute the inverse cube root of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_invcbrt_pd(__m128d __a)
{
  return __svml_invcbrt2(__a);
}

/// Compute the inverse square root of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_invsqrt_ps(__m128 __a)
{
  return __builtin_ia32_rsqrtps(__a);
}

extern __m128d __svml_invsqrt2(__m128d);

/// Compute the inverse square root of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_invsqrt_pd(__m128d __a)
{
  return __svml_invsqrt2(__a);
}

extern __m128 __svml_log10f4(__m128);

/// Compute the base-10 logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_log10_ps(__m128 __a)
{
  return __svml_log10f4(__a);
}

extern __m128d __svml_log102(__m128d);

/// Compute the base-10 logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_log10_pd(__m128d __a)
{
  return __svml_log102(__a);
}

extern __m128 __svml_log2f4(__m128);

/// Compute the base-2 logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_log2_ps(__m128 __a)
{
  return __svml_log2f4(__a);
}

extern __m128d __svml_log22(__m128d);

/// Compute the base-2 logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_log2_pd(__m128d __a)
{
  return __svml_log22(__a);
}

extern __m128 __svml_logf4(__m128);

/// Compute the natural logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_log_ps(__m128 __a)
{
  return __svml_logf4(__a);
}

extern __m128d __svml_log2(__m128d);

/// Compute the natural logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_log_pd(__m128d __a)
{
  return __svml_log2(__a);
}

extern __m128 __svml_powf4(__m128, __m128);

/// Compute the exponential value of packed single-precision (32-bit)
///    floating-point elements in a raised by packed elements in b, and store
///    the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_pow_ps(__m128 __a, __m128 __b)
{
  return __svml_powf4(__a, __b);
}

extern __m128d __svml_pow2(__m128d, __m128d);

/// Compute the exponential value of packed double-precision (64-bit)
///    floating-point elements in a raised by packed elements in b, and store
///    the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_pow_pd(__m128d __a, __m128d __b)
{
  return __svml_pow2(__a, __b);
}

extern __m128 __svml_sinhf4(__m128);

/// Compute the hyperbolic sine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_sinh_ps(__m128 __a)
{
  return __svml_sinhf4(__a);
}

extern __m128d __svml_sinh2(__m128d);

/// Compute the hyperbolic sine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_sinh_pd(__m128d __a)
{
  return __svml_sinh2(__a);
}

extern __m128 __svml_sinf4(__m128);

/// Compute the sine of packed single-precision (32-bit) floating-point elements
///    in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_sin_ps(__m128 __a)
{
  return __svml_sinf4(__a);
}

extern __m128d __svml_sin2(__m128d);

/// Compute the sine of packed double-precision (64-bit) floating-point elements
///    in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_sin_pd(__m128d __a)
{
  return __svml_sin2(__a);
}

extern __m128 __svml_tanhf4(__m128);

/// Compute the hyperbolic tangent of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_tanh_ps(__m128 __a)
{
  return __svml_tanhf4(__a);
}

extern __m128d __svml_tanh2(__m128d);

/// Compute the hyperbolic tangent of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_tanh_pd(__m128d __a)
{
  return __svml_tanh2(__a);
}

extern __m128 __svml_tanf4(__m128);

/// Compute the tangent of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128 __DEFAULT_FN_ATTRS
_mm_tan_ps(__m128 __a)
{
  return __svml_tanf4(__a);
}

extern __m128d __svml_tan2(__m128d);

/// Compute the tangent of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m128d __DEFAULT_FN_ATTRS
_mm_tan_pd(__m128d __a)
{
  return __svml_tan2(__a);
}

extern __v4si __svml_idiv4(__v4si, __v4si);

/// Divide packed 32-bit integers in a by packed elements in b, and store the
/// truncated results in dst.
///
static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_idiv_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)__svml_idiv4((__v4si)__a, (__v4si)__b);
}

extern __v4si __svml_irem4(__v4si, __v4si);

/// Divide packed 32-bit integers in a by packed elements in b, and store the
///    remainders as packed 32-bit integers in dst.
///
static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_irem_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)__svml_irem4((__v4si)__a, (__v4si)__b);
}

extern __v4su __svml_udiv4(__v4su, __v4su);

/// Divide packed unsigned 32-bit integers in a by packed elements in b, and
///    store the truncated results in dst.
///
static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_udiv_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)__svml_udiv4((__v4su)__a, (__v4su)__b);
}

extern __v4su __svml_urem4(__v4su, __v4su);

/// Divide packed unsigned 32-bit integers in a by packed elements in b, and
///    store the remainders as packed unsigned 32-bit integers in dst.
///
static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_urem_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)__svml_urem4((__v4su)__a, (__v4su)__b);
}

#undef __DEFAULT_FN_ATTRS

#endif // defined(__XMMINTRIN_H) && defined(__EMMINTRIN_H)

#if defined(__SMMINTRIN_H)

/* Define the default attributes for the functions in this section. */
#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__, __target__("sse4.1"), __min_vector_width__(128)))

/// Round the packed single-precision (32-bit) floating-point elements in a up
///    to an integer value, and store the results as packed single-precision
///    floating-point elements in dst.
///
/// This intrinsic may generate the roundps/vroundps instruction.
///
#define _mm_svml_ceil_ps(X)   _mm_ceil_ps(X)

/// Round the packed single-precision (32-bit) floating-point elements in a down
///    to an integer value, and store the results as packed single-precision
///    floating-point elements in dst.
///
/// This intrinsic may generate the roundps/vroundps instruction.
///
#define _mm_svml_floor_ps(X)  _mm_floor_ps(X)

/// Round the packed single-precision (32-bit) floating-point elements in a to
///    the nearest integer value, and store the results as packed
///    single-precision floating-point elements in dst.
///
/// This intrinsic may generate the roundps/vroundps instruction.
///
#define _mm_svml_round_ps(X)  _mm_round_ps((X), _MM_FROUND_NEARBYINT)

/// Truncate the packed single-precision (32-bit) floating-point elements in a,
///    and store the results as packed single-precision floating-point elements
///    in dst.
///
/// This intrinsic may generate the roundps/vroundps instruction.
///
#define _mm_trunc_ps(X)       _mm_round_ps((X), _MM_FROUND_TRUNC)

/// Round the packed double-precision (64-bit) floating-point elements in a up
///    to an integer value, and store the results as packed double-precision
///    floating-point elements in dst.
///
/// This intrinsic may generate the roundpd/vroundpd instruction.
///
#define _mm_svml_ceil_pd(X)   _mm_ceil_pd(X)

/// Round the packed double-precision (64-bit) floating-point elements in a down
///    to an integer value, and store the results as packed double-precision
///    floating-point elements in dst.
///
/// This intrinsic may generate the roundpd/vroundpd instruction.
///
#define _mm_svml_floor_pd(X)  _mm_floor_pd(X)

/// Round the packed double-precision (64-bit) floating-point elements in a to
///    the nearest integer value, and store the results as packed
///    double-precision floating-point elements in dst.
///
/// This intrinsic may generate the roundpd/vroundpd instruction.
///
#define _mm_svml_round_pd(X)  _mm_round_pd((X), _MM_FROUND_NEARBYINT)

/// Truncate the packed double-precision (64-bit) floating-point elements in a,
///    and store the results as packed double-precision floating-point elements
///    in dst.
///
/// This intrinsic may generate the roundpd/vroundpd instruction.
///
#define _mm_trunc_pd(X)       _mm_round_pd((X), _MM_FROUND_TRUNC)

#undef __DEFAULT_FN_ATTRS

#endif // defined(__SMMINTRIN_H)

#if defined(__AVXINTRIN_H)

/* Define the default attributes for the functions in this section. */
#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__, __target__("avx"), __min_vector_width__(256)))

extern __m256 __svml_acoshf8(__m256);

/// Compute the inverse hyperbolic cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_acosh_ps(__m256 __a)
{
  return __svml_acoshf8(__a);
}

extern __m256d __svml_acosh4(__m256d);

/// Compute the inverse hyperbolic cosine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_acosh_pd(__m256d __a)
{
  return __svml_acosh4(__a);
}

extern __m256 __svml_coshf8(__m256);

/// Compute the hyperbolic cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_cosh_ps(__m256 __a)
{
  return __svml_coshf8(__a);
}

extern __m256d __svml_cosh4(__m256d);

/// Compute the hyperbolic cosine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_cosh_pd(__m256d __a)
{
  return __svml_cosh4(__a);
}

extern __m256 __svml_acosf8(__m256);

/// Compute the inverse cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_acos_ps(__m256 __a)
{
  return __svml_acosf8(__a);
}

extern __m256d __svml_acos4(__m256d);

/// Compute the inverse cosine of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_acos_pd(__m256d __a)
{
  return __svml_acos4(__a);
}

extern __m256 __svml_cosf8(__m256);

/// Compute the cosine of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_cos_ps(__m256 __a)
{
  return __svml_cosf8(__a);
}

extern __m256d __svml_cos4(__m256d);

/// Compute the cosine of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_cos_pd(__m256d __a)
{
  return __svml_cos4(__a);
}

extern __m256 __svml_asinhf8(__m256);

/// Compute the inverse hyperbolic sine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_asinh_ps(__m256 __a)
{
  return __svml_asinhf8(__a);
}

extern __m256d __svml_asinh4(__m256d);

/// Compute the inverse hyperbolic sine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_asinh_pd(__m256d __a)
{
  return __svml_asinh4(__a);
}

extern __m256 __svml_sinhf8(__m256);

/// Compute the hyperbolic sine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_sinh_ps(__m256 __a)
{
  return __svml_sinhf8(__a);
}

extern __m256d __svml_sinh4(__m256d);

/// Compute the hyperbolic sine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_sinh_pd(__m256d __a)
{
  return __svml_sinh4(__a);
}

extern __m256 __svml_asinf8(__m256);

/// Compute the inverse sine of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_asin_ps(__m256 __a)
{
  return __svml_asinf8(__a);
}

extern __m256d __svml_asin4(__m256d);

/// Compute the inverse sine of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_asin_pd(__m256d __a)
{
  return __svml_asin4(__a);
}

extern __m256 __svml_sinf8(__m256);

/// Compute the sine of packed single-precision (32-bit) floating-point elements
///    in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_sin_ps(__m256 __a)
{
  return __svml_sinf8(__a);
}

extern __m256d __svml_sin4(__m256d);

/// Compute the sine of packed double-precision (64-bit) floating-point elements
///    in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_sin_pd(__m256d __a)
{
  return __svml_sin4(__a);
}

extern __m256 __svml_atanhf8(__m256);

/// Compute the inverse hyperbolic tangent of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_atanh_ps(__m256 __a)
{
  return __svml_atanhf8(__a);
}

extern __m256d __svml_atanh4(__m256d);

/// Compute the inverse hyperbolic tangent of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_atanh_pd(__m256d __a)
{
  return __svml_atanh4(__a);
}

extern __m256 __svml_tanhf8(__m256);

/// Compute the hyperbolic tangent of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_tanh_ps(__m256 __a)
{
  return __svml_tanhf8(__a);
}

extern __m256d __svml_tanh4(__m256d);

/// Compute the hyperbolic tangent of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_tanh_pd(__m256d __a)
{
  return __svml_tanh4(__a);
}

extern __m256 __svml_atanf8(__m256);

/// Compute the inverse tangent of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_atan_ps(__m256 __a)
{
  return __svml_atanf8(__a);
}

extern __m256d __svml_atan4(__m256d);

/// Compute the inverse tangent of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_atan_pd(__m256d __a)
{
  return __svml_atan4(__a);
}

extern __m256 __svml_atan2f8(__m256, __m256);

/// Compute the inverse tangent of packed single-precision (32-bit)
///    floating-point elements in a divided by packed elements in b, and store
///    the results in dst expressed in radians.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_atan2_ps(__m256 __a, __m256 __b)
{
  return __svml_atan2f8(__a, __b);
}

extern __m256d __svml_atan24(__m256d, __m256d);

/// Compute the inverse tangent of packed double-precision (64-bit)
///    floating-point elements in a divided by packed elements in b, and store
///    the results in dst expressed in radians.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_atan2_pd(__m256d __a, __m256d __b)
{
  return __svml_atan24(__a, __b);
}

extern __m256 __svml_tanf8(__m256);

/// Compute the tangent of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_tan_ps(__m256 __a)
{
  return __svml_tanf8(__a);
}

extern __m256d __svml_tan4(__m256d);

/// Compute the tangent of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_tan_pd(__m256d __a)
{
  return __svml_tan4(__a);
}

extern __m256 __svml_expf8(__m256);

/// Compute the exponential value of e raised to the power of packed
///    single-precision (32-bit) floating-point elements in a, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_exp_ps(__m256 __a)
{
  return __svml_expf8(__a);
}

extern __m256d __svml_exp4(__m256d);

/// Compute the exponential value of e raised to the power of packed
///    double-precision (64-bit) floating-point elements in a, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_exp_pd(__m256d __a)
{
  return __svml_exp4(__a);
}

extern __m256 __svml_exp2f8(__m256);

/// Compute the exponential value of 2 raised to the power of packed
///    single-precision (32-bit) floating-point elements in a, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_exp2_ps(__m256 __a)
{
  return __svml_exp2f8(__a);
}

extern __m256d __svml_exp24(__m256d);

/// Compute the exponential value of 2 raised to the power of packed
///    double-precision (64-bit) floating-point elements in a, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_exp2_pd(__m256d __a)
{
  return __svml_exp24(__a);
}

extern __m256 __svml_powf8(__m256, __m256);

/// Compute the exponential value of packed single-precision (32-bit)
///    floating-point elements in a raised by packed elements in b, and store
///    the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_pow_ps(__m256 __a, __m256 __b)
{
  return __svml_powf8(__a, __b);
}

extern __m256d __svml_pow4(__m256d, __m256d);

/// Compute the exponential value of packed double-precision (64-bit)
///    floating-point elements in a raised by packed elements in b, and store
///    the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_pow_pd(__m256d __a, __m256d __b)
{
  return __svml_pow4(__a, __b);
}

extern __m256 __svml_logf8(__m256);

/// Compute the natural logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_log_ps(__m256 __a)
{
  return __svml_logf8(__a);
}

extern __m256d __svml_log4(__m256d);

/// Compute the natural logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_log_pd(__m256d __a)
{
  return __svml_log4(__a);
}

extern __m256 __svml_log2f8(__m256);

/// Compute the base-2 logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_log2_ps(__m256 __a)
{
  return __svml_log2f8(__a);
}

extern __m256d __svml_log24(__m256d);

/// Compute the base-2 logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_log2_pd(__m256d __a)
{
  return __svml_log24(__a);
}

extern __m256 __svml_log10f8(__m256);

/// Compute the base-10 logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_log10_ps(__m256 __a)
{
  return __svml_log10f8(__a);
}

extern __m256d __svml_log104(__m256d);

/// Compute the base-10 logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_log10_pd(__m256d __a)
{
  return __svml_log104(__a);
}

/// Compute the inverse square root of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m256 __DEFAULT_FN_ATTRS
_mm256_invsqrt_ps(__m256 __a)
{
  return __builtin_ia32_rsqrtps256(__a);
}

extern __m256d __svml_invsqrt4(__m256d);

/// Compute the inverse square root of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline__ __m256d __DEFAULT_FN_ATTRS
_mm256_invsqrt_pd(__m256d __a)
{
  return __svml_invsqrt4(__a);
}

extern __m256 __svml_cbrtf8(__m256);

/// Compute the cube root of packed single-precision (32-bit) floating-point
///    elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_cbrt_ps(__m256 __a)
{
  return __svml_cbrtf8(__a);
}

extern __m256d __svml_cbrt4(__m256d);

/// Compute the cube root of packed double-precision (64-bit) floating-point
///    elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_cbrt_pd(__m256d __a)
{
  return __svml_cbrt4(__a);
}

extern __m256 __svml_invcbrtf8(__m256);

/// Compute the inverse cube root of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_invcbrt_ps(__m256 __a)
{
  return __svml_invcbrtf8(__a);
}

extern __m256d __svml_invcbrt4(__m256d);

/// Compute the inverse cube root of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_invcbrt_pd(__m256d __a)
{
  return __svml_invcbrt4(__a);
}

extern __m256 __svml_erff8(__m256);

/// Compute the error function of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_erf_ps(__m256 __a)
{
  return __svml_erff8(__a);
}

extern __m256d __svml_erf4(__m256d);

/// Compute the error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_erf_pd(__m256d __a)
{
  return __svml_erf4(__a);
}

extern __m256 __svml_erfcf8(__m256);

/// Compute the complementary error function of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_erfc_ps(__m256 __a)
{
  return __svml_erfcf8(__a);
}

extern __m256d __svml_erfc4(__m256d);

/// Compute the complementary error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_erfc_pd(__m256d __a)
{
  return __svml_erfc4(__a);
}

extern __m256 __svml_erfinvf8(__m256);

/// Compute the inverse error function of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256 __DEFAULT_FN_ATTRS
_mm256_erfinv_ps(__m256 __a)
{
  return __svml_erfinvf8(__a);
}

extern __m256d __svml_erfinv4(__m256d);

/// Compute the inverse error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
static __inline __m256d __DEFAULT_FN_ATTRS
_mm256_erfinv_pd(__m256d __a)
{
  return __svml_erfinv4(__a);
}

/// Round the packed single-precision (32-bit) floating-point elements in a up
///    to an integer value, and store the results as packed single-precision
///    floating-point elements in dst.
///
///  This intrinsic may generate the roundps/vroundps instruction.
///
#define _mm256_svml_ceil_ps(X)   _mm256_ceil_ps(X)

/// Round the packed single-precision (32-bit) floating-point elements in a down
///    to an integer value, and store the results as packed single-precision
///    floating-point elements in dst.
///
/// This intrinsic may generate the roundps/vroundps instruction.
///
#define _mm256_svml_floor_ps(X)  _mm256_floor_ps(X)

/// Round the packed single-precision (32-bit) floating-point elements in a to
///    the nearest integer value, and store the results as packed
///    single-precision floating-point elements in dst.
///
/// This intrinsic may generate the roundps/vroundps instruction.
///
#define _mm256_svml_round_ps(X)  _mm256_round_ps((X), _MM_FROUND_NEARBYINT)

/// Truncate the packed single-precision (32-bit) floating-point elements in a,
///    and store the results as packed single-precision floating-point elements
///    in dst.
///
/// This intrinsic may generate the roundps/vroundps instruction.
///
#define _mm256_trunc_ps(X)       _mm256_round_ps((X), _MM_FROUND_TRUNC)

/// Round the packed double-precision (64-bit) floating-point elements in a up
///    to an integer value, and store the results as packed double-precision
///    floating-point elements in dst.
///
/// This intrinsic may generate the roundpd/vroundpd instruction.
///
#define _mm256_svml_ceil_pd(X)   _mm256_ceil_pd(X)

/// Round the packed double-precision (64-bit) floating-point elements in a down
///    to an integer value, and store the results as packed double-precision
///    floating-point elements in dst.
///
/// This intrinsic may generate the roundpd/vroundpd instruction.
///
#define _mm256_svml_floor_pd(X)  _mm256_floor_pd(X)

/// Round the packed double-precision (64-bit) floating-point elements in a to
///    the nearest integer value, and store the results as packed
///    double-precision floating-point elements in dst.
///
/// This intrinsic may generate the roundpd/vroundpd instruction.
///
#define _mm256_svml_round_pd(X)  _mm256_round_pd((X), _MM_FROUND_NEARBYINT)

/// Truncate the packed double-precision (64-bit) floating-point elements in a,
///    and store the results as packed double-precision floating-point elements
///    in dst.
///
/// This intrinsic may generate the roundpd/vroundpd instruction.
///
#define _mm256_trunc_pd(X)       _mm256_round_pd((X), _MM_FROUND_TRUNC)

#undef __DEFAULT_FN_ATTRS

#endif // defined(__AVXINTRIN_H)

#if defined(__AVX2INTRIN_H)

/* Define the default attributes for the functions in this section. */
#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__, __target__("avx2"), __min_vector_width__(256)))

extern __v8si __svml_idiv8(__v8si, __v8si);

/// Divide packed 32-bit integers in a by packed elements in b, and store the
///    truncated results in dst.
static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm256_div_epi32(__m256i __a, __m256i __b)
{
  return (__m256i)__svml_idiv8((__v8si)__a, (__v8si)__b);
}

extern __v8si __svml_irem8(__v8si, __v8si);

/// Divide packed 32-bit integers in a by packed elements in b, and store the
///    remainders as packed 32-bit integers in dst.
static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm256_rem_epi32(__m256i __a, __m256i __b)
{
  return (__m256i)__svml_irem8((__v8si)__a, (__v8si)__b);
}

extern __v8su __svml_udiv8(__v8su, __v8su);

/// Divide packed unsigned 32-bit integers in a by packed elements in b, and
///    store the truncated results in dst.
static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm256_div_epu32(__m256i __a, __m256i __b)
{
  return (__m256i)__svml_udiv8((__v8su)__a, (__v8su)__b);
}

extern __v8su __svml_urem8(__v8su, __v8su);

/// Divide packed unsigned 32-bit integers in a by packed elements in b, and
///    store the remainders as packed unsigned 32-bit integers in dst.
static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm256_rem_epu32(__m256i __a, __m256i __b)
{
  return (__m256i)__svml_urem8((__v8su)__a, (__v8su)__b);
}

#undef __DEFAULT_FN_ATTRS

#endif // defined(__AVX2INTRIN_H)

#if defined(__AVX512FINTRIN_H)

/* Define the default attributes for the functions in this section. */
#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__, __target__("avx512f"), __min_vector_width__(512)))

extern __m512 __svml_acoshf16(__m512);

/// Compute the inverse hyperbolic cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_acosh_ps(__m512 __a)
{
  return __svml_acoshf16(__a);
}

extern __m512d __svml_acosh8(__m512d);

/// Compute the inverse hyperbolic cosine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_acosh_pd(__m512d __a)
{
  return __svml_acosh8(__a);
}

extern __m512 __svml_coshf16(__m512);

/// Compute the hyperbolic cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_cosh_ps(__m512 __a)
{
  return __svml_coshf16(__a);
}

extern __m512d __svml_cosh8(__m512d);

/// Compute the hyperbolic cosine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_cosh_pd(__m512d __a)
{
  return __svml_cosh8(__a);
}

extern __m512 __svml_acosf16(__m512);

/// Compute the inverse cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_acos_ps(__m512 __a)
{
  return __svml_acosf16(__a);
}

extern __m512d __svml_acos8(__m512d);

/// Compute the inverse cosine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_acos_pd(__m512d __a)
{
  return __svml_acos8(__a);
}

extern __m512 __svml_cosf16(__m512);

/// Compute the cosine of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_cos_ps(__m512 __a)
{
  return __svml_cosf16(__a);
}

extern __m512d __svml_cos8(__m512d);

/// Compute the cosine of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_cos_pd(__m512d __a)
{
  return __svml_cos8(__a);
}

extern __m512 __svml_asinhf16(__m512);

/// Compute the inverse hyperbolic sine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_asinh_ps(__m512 __a)
{
  return __svml_asinhf16(__a);
}

extern __m512d __svml_asinh8(__m512d);

/// Compute the inverse hyperbolic sine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_asinh_pd(__m512d __a)
{
  return __svml_asinh8(__a);
}

extern __m512 __svml_sinhf16(__m512);

/// Compute the hyperbolic sine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_sinh_ps(__m512 __a)
{
  return __svml_sinhf16(__a);
}

extern __m512d __svml_sinh8(__m512d);

/// Compute the hyperbolic sine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_sinh_pd(__m512d __a)
{
  return __svml_sinh8(__a);
}

extern __m512 __svml_asinf16(__m512);

/// Compute the inverse sine of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_asin_ps(__m512 __a)
{
  return __svml_asinf16(__a);
}

extern __m512d __svml_asin8(__m512d);

/// Compute the inverse sine of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_asin_pd(__m512d __a)
{
  return __svml_asin8(__a);
}

extern __m512 __svml_sinf16(__m512);

/// Compute the sine of packed single-precision (32-bit) floating-point elements
///    in a expressed in radians, and store the results in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_sin_ps(__m512 __a)
{
  return __svml_sinf16(__a);
}

extern __m512d __svml_sin8(__m512d);

/// Compute the sine of packed double-precision (64-bit) floating-point elements
///    in a expressed in radians, and store the results in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_sin_pd(__m512d __a)
{
  return __svml_sin8(__a);
}

extern __m512 __svml_atanhf16(__m512);

/// Compute the inverse hyperblic tangent of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst expressed in
///    radians.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_atanh_ps(__m512 __a)
{
  return __svml_atanhf16(__a);
}

extern __m512d __svml_atanh8(__m512d);

/// Compute the inverse hyperbolic tangent of packed double-precision (64-bit)
///    floating-point elements in a and store the results in dst expressed in
///    radians.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_atanh_pd(__m512d __a)
{
  return __svml_atanh8(__a);
}

extern __m512 __svml_tanhf16(__m512);

/// Compute the hyperbolic tangent of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_tanh_ps(__m512 __a)
{
  return __svml_tanhf16(__a);
}

extern __m512d __svml_tanh8(__m512d);

/// Compute the hyperbolic tangent of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_tanh_pd(__m512d __a)
{
  return __svml_tanh8(__a);
}

extern __m512 __svml_atanf16(__m512);

/// Compute the inverse tangent of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst expressed in
///    radians.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_atan_ps(__m512 __a)
{
  return __svml_atanf16(__a);
}

extern __m512d __svml_atan8(__m512d);

/// Compute the inverse tangent of packed double-precision (64-bit)
///    floating-point elements in a and store the results in dst expressed in
///    radians.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_atan_pd(__m512d __a)
{
  return __svml_atan8(__a);
}

extern __m512 __svml_atan2f16(__m512, __m512);

/// Compute the inverse tangent of packed single-precision (32-bit)
///    floating-point elements in a divided by packed elements in b, and store
///    the results in dst expressed in radians.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_atan2_ps(__m512 __a, __m512 __b)
{
  return __svml_atan2f16(__a, __b);
}

extern __m512d __svml_atan28(__m512d, __m512d);

/// Compute the inverse tangent of packed double-precision (64-bit)
///    floating-point elements in a divided by packed elements in b, and store
///    the results in dst expressed in radians.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_atan2_pd(__m512d __a, __m512d __b)
{
  return __svml_atan28(__a, __b);
}

extern __m512 __svml_tanf16(__m512);

/// Compute the tangent of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_tan_ps(__m512 __a)
{
  return __svml_tanf16(__a);
}

extern __m512d __svml_tan8(__m512d);

/// Compute the tangent of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_tan_pd(__m512d __a)
{
  return __svml_tan8(__a);
}

extern __m512 __svml_expf16(__m512);

/// Compute the exponential value of e raised to the power of packed
///    single-precision (32-bit) floating-point elements in a, and store the
///    results in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_exp_ps(__m512 __a)
{
  return __svml_expf16(__a);
}

extern __m512d __svml_exp8(__m512d);

/// Compute the exponential value of e raised to the power of packed
///    double-precision (64-bit) floating-point elements in a, and store the
///    results in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_exp_pd(__m512d __a)
{
  return __svml_exp8(__a);
}

extern __m512 __svml_exp2f16(__m512);

/// Compute the exponential value of 2 raised to the power of packed
///    single-precision (32-bit) floating-point elements in a, and store the
///    results in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_exp2_ps(__m512 __a)
{
  return __svml_exp2f16(__a);
}

extern __m512d __svml_exp28(__m512d);

/// Compute the exponential value of 2 raised to the power of packed
///    double-precision (64-bit) floating-point elements in a, and store the
///    results in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_exp2_pd(__m512d __a)
{
  return __svml_exp28(__a);
}

extern __m512 __svml_powf16(__m512, __m512);

/// Compute the exponential value of packed single-precision (32-bit)
///    floating-point elements in a raised by packed elements in b, and store
///    the results in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_pow_ps(__m512 __a, __m512 __b)
{
  return __svml_powf16(__a, __b);
}

extern __m512d __svml_pow8(__m512d, __m512d);

/// Compute the exponential value of packed double-precision (64-bit)
///    floating-point elements in a raised by packed elements in b, and store
///    the results in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_pow_pd(__m512d __a, __m512d __b)
{
  return __svml_pow8(__a, __b);
}

extern __m512 __svml_logf16(__m512);

/// Compute the natural logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_log_ps(__m512 __a)
{
  return __svml_logf16(__a);
}

extern __m512d __svml_log8(__m512d);

/// Compute the natural logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_log_pd(__m512d __a)
{
  return __svml_log8(__a);
}

extern __m512 __svml_log2f16(__m512);

/// Compute the base-2 logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_log2_ps(__m512 __a)
{
  return __svml_log2f16(__a);
}

extern __m512d __svml_log28(__m512d);

/// Compute the base-2 logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_log2_pd(__m512d __a)
{
  return __svml_log28(__a);
}

extern __m512 __svml_log10f16(__m512);

/// Compute the base-10 logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_log10_ps(__m512 __a)
{
  return __svml_log10f16(__a);
}

extern __m512d __svml_log108(__m512d);

/// Compute the base-10 logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_log10_pd(__m512d __a)
{
  return __svml_log108(__a);
}

extern __m512 __svml_invsqrtf16(__m512);

/// Compute the inverse square root of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_invsqrt_ps(__m512 __a)
{
  return __svml_invsqrtf16(__a);
}

extern __m512d __svml_invsqrt8(__m512d);

/// Compute the inverse square root of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_invsqrt_pd(__m512d __a)
{
  return __svml_invsqrt8(__a);
}

extern __m512 __svml_cbrtf16(__m512);

/// Compute the cube root of packed single-precision (32-bit) floating-point
///    elements in a, and store the results in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_cbrt_ps(__m512 __a)
{
  return __svml_cbrtf16(__a);
}

extern __m512d __svml_cbrt8(__m512d);

/// Compute the cube root of packed double-precision (64-bit) floating-point
///    elements in a, and store the results in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_cbrt_pd(__m512d __a)
{
  return __svml_cbrt8(__a);
}

extern __m512 __svml_invcbrtf16(__m512);

/// Compute the inverse cube root of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_invcbrt_ps(__m512 __a)
{
  return __svml_invcbrtf16(__a);
}

extern __m512d __svml_invcbrt8(__m512d);

/// Compute the inverse cube root of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_invcbrt_pd(__m512d __a)
{
  return __svml_invcbrt8(__a);
}

extern __m512 __svml_erff16(__m512);

/// Compute the error function of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_erf_ps(__m512 __a)
{
  return __svml_erff16(__a);
}

extern __m512d __svml_erf8(__m512d);

/// Compute the error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_erf_pd(__m512d __a)
{
  return __svml_erf8(__a);
}

extern __m512 __svml_erfcf16(__m512);

/// Compute the complementary error function of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_erfc_ps(__m512 __a)
{
  return __svml_erfcf16(__a);
}

extern __m512d __svml_erfc8(__m512d);

/// Compute the complementary error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_erfc_pd(__m512d __a)
{
  return __svml_erfc8(__a);
}

extern __m512 __svml_erfinvf16(__m512);

/// Compute the inverse error function of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_erfinv_ps(__m512 __a)
{
  return __svml_erfinvf16(__a);
}

extern __m512d __svml_erfinv8(__m512d);

/// Compute the inverse error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_erfinv_pd(__m512d __a)
{
  return __svml_erfinv8(__a);
}

extern __m512 __svml_truncf16(__m512);

/// Truncate the packed single-precision (32-bit) floating-point elements in a,
///    and store the results as packed single-precision floating-point elements
///    in dst.
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_trunc_ps(__m512 __a)
{
  return __svml_truncf16(__a);
}

extern __m512d __svml_trunc8(__m512d);

/// Truncate the packed double-precision (64-bit) floating-point elements in a,
///    and store the results as packed single-precision floating-point elements
///    in dst.
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_trunc_pd(__m512d __a)
{
  return __svml_trunc8(__a);
}

extern __v16si __svml_idiv16(__v16si, __v16si);

/// Divide packed 32-bit integers in a by packed elements in b, and store the
///    truncated results in dst.
static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_div_epi32(__m512i __a, __m512i __b)
{
  return (__m512i)__svml_idiv16((__v16si)__a, (__v16si)__b);
}

extern __v16si __svml_irem16(__v16si, __v16si);

/// Divide packed 32-bit integers in a by packed elements in b, and store the
///    remainders as packed 32-bit integers in dst.
static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_rem_epi32(__m512i __a, __m512i __b)
{
  return (__m512i)__svml_irem16((__v16si)__a, (__v16si)__b);
}

extern __v16su __svml_udiv16(__v16su, __v16su);

/// Divide packed unsigned 32-bit integers in a by packed elements in b, and
///    store the truncated results in dst.
static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_div_epu32(__m512i __a, __m512i __b)
{
  return (__m512i)__svml_udiv16((__v16su)__a, (__v16su)__b);
}

extern __v16su __svml_urem16(__v16su, __v16su);

/// Divide packed unsigned 32-bit integers in a by packed elements in b, and
///    store the remainders as packed unsigned 32-bit integers in dst.
static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_rem_epu32(__m512i __a, __m512i __b)
{
  return (__m512i)__svml_urem16((__v16su)__a, (__v16su)__b);
}

#undef __DEFAULT_FN_ATTRS

#endif // defined(__AVX512FINTRIN_H)

#if defined(__cplusplus)
} // extern "C"
#endif

#endif /* __SVMLINTRIN_H */
