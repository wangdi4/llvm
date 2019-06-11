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

/// Compute the inverse hyperbolic cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the 
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_acosh_ps(__m128 __a);

/// Compute the inverse hyperbolic cosine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_acosh_pd(__m128d __a);

/// Compute the inverse cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_acos_ps(__m128 __a);

/// Compute the inverse cosine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_acos_pd(__m128d __a);

/// Compute the inverse hyperbolic sine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_asinh_ps(__m128 __a);

/// Compute the inverse hyperbolic sine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_asinh_pd(__m128d __a);

/// Compute the inverse sine of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_asin_ps(__m128 __a);

/// Compute the inverse sine of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_asin_pd(__m128d __a);

/// Compute the inverse tangent of packed single-precision (32-bit)
///    floating-point elements in a divided by packed elements in b, and store
///    the results in dst expressed in radians.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_atan2_ps(__m128 __a, __m128 __b);

/// Compute the inverse tangent of packed double-precision (64-bit)
///    floating-point elements in a divided by packed elements in b, and store
///    the results in dst expressed in radians.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_atan2_pd(__m128d __a, __m128d __b);

/// Compute the inverse hyperbolic tangent of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_atanh_ps(__m128 __a);

/// Compute the inverse hyperbolic tangent of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_atanh_pd(__m128d __a);

/// Compute the inverse tangent of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_atan_ps(__m128 __a);

/// Compute the inverse tangent of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_atan_pd(__m128d __a);

/// Compute the cube root of packed single-precision (32-bit) floating-point
///    elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_cbrt_ps(__m128 __a);

/// Compute the cube root of packed double-precision (64-bit) floating-point
///    elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_cbrt_pd(__m128d __a);

/// Compute the hyperbolic cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_cosh_ps(__m128 __a);

/// Compute the hyperbolic cosine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_cosh_pd(__m128d __a);

/// Compute the cosine of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_cos_ps(__m128 __a);

/// Compute the cosine of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_cos_pd(__m128d __a);

/// Compute the complementary error function of packed single-precision (32-bit);
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_erfc_ps(__m128 __a);

/// Compute the complementary error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_erfc_pd(__m128d __a);

/// Compute the inverse error function of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_erfinv_ps(__m128 __a);

/// Compute the inverse error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_erfinv_pd(__m128d __a);

/// Compute the error function of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_erf_ps(__m128 __a);

/// Compute the error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_erf_pd(__m128d __a);

/// Compute the exponential value of 2 raised to the power of packed
///    single-precision (32-bit) floating-point elements in a, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_exp2_ps(__m128 __a);

/// Compute the exponential value of 2 raised to the power of packed
///    double-precision (64-bit) floating-point elements in a, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_exp2_pd(__m128d __a);

/// Compute the exponential value of e raised to the power of packed
///    single-precision (32-bit) floating-point elements in a, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_exp_ps(__m128 __a);

/// Compute the exponential value of e raised to the power of packed
///    double-precision (64-bit) floating-point elements in a, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_exp_pd(__m128d __a);

/// Compute the inverse cube root of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_invcbrt_ps(__m128 __a);

/// Compute the inverse cube root of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_invcbrt_pd(__m128d __a);

/// Compute the inverse square root of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_invsqrt_ps(__m128 __a);

/// Compute the inverse square root of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_invsqrt_pd(__m128d __a);

/// Compute the base-10 logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_log10_ps(__m128 __a);

/// Compute the base-10 logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_log10_pd(__m128d __a);

/// Compute the base-2 logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_log2_ps(__m128 __a);

/// Compute the base-2 logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_log2_pd(__m128d __a);

/// Compute the natural logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_log_ps(__m128 __a);

/// Compute the natural logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_log_pd(__m128d __a);

/// Compute the exponential value of packed single-precision (32-bit)
///    floating-point elements in a raised by packed elements in b, and store
///    the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_pow_ps(__m128 __a, __m128 __b);

/// Compute the exponential value of packed double-precision (64-bit)
///    floating-point elements in a raised by packed elements in b, and store
///    the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_pow_pd(__m128d __a, __m128d __b);

/// Compute the hyperbolic sine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_sinh_ps(__m128 __a);

/// Compute the hyperbolic sine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_sinh_pd(__m128d __a);

/// Compute the sine of packed single-precision (32-bit) floating-point elements
///    in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_sin_ps(__m128 __a);

/// Compute the sine of packed double-precision (64-bit) floating-point elements
///    in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_sin_pd(__m128d __a);

/// Compute the hyperbolic tangent of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_tanh_ps(__m128 __a);

/// Compute the hyperbolic tangent of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_tanh_pd(__m128d __a);

/// Compute the tangent of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128 _mm_tan_ps(__m128 __a);

/// Compute the tangent of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m128d _mm_tan_pd(__m128d __a);

/// Divide packed 32-bit integers in a by packed elements in b, and store the
/// truncated results in dst.
///
static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_idiv_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)__builtin_svml_idiv4((__v4si)__a, (__v4si)__b);
}

/// Divide packed 32-bit integers in a by packed elements in b, and store the
///    remainders as packed 32-bit integers in dst.
///
static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_irem_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)__builtin_svml_irem4((__v4si)__a, (__v4si)__b);
}

/// Divide packed unsigned 32-bit integers in a by packed elements in b, and
///    store the truncated results in dst.
///
static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_udiv_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)__builtin_svml_udiv4((__v4su)__a, (__v4su)__b);
}

/// Divide packed unsigned 32-bit integers in a by packed elements in b, and
///    store the remainders as packed unsigned 32-bit integers in dst.
///
static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_urem_epi32(__m128i __a, __m128i __b)
{
  return (__m128i)__builtin_svml_urem4((__v4su)__a, (__v4su)__b);
}

#undef __DEFAULT_FN_ATTRS

#endif // defined(__XMMINTRIN_H) && defined(__EMMINTRIN_H)

#if defined(__SMMINTRIN_H)

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

#endif // defined(__SMMINTRIN_H)

#if defined(__AVXINTRIN_H)

/// Compute the inverse hyperbolic cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_acosh_ps(__m256 __a);

/// Compute the inverse hyperbolic cosine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_acosh_pd(__m256d __a);

/// Compute the hyperbolic cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_cosh_ps(__m256 __a);

/// Compute the hyperbolic cosine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_cosh_pd(__m256d __a);

/// Compute the inverse cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_acos_ps(__m256 __a);

/// Compute the inverse cosine of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_acos_pd(__m256d __a);

/// Compute the cosine of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_cos_ps(__m256 __a);

/// Compute the cosine of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_cos_pd(__m256d __a);

/// Compute the inverse hyperbolic sine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_asinh_ps(__m256 __a);

/// Compute the inverse hyperbolic sine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_asinh_pd(__m256d __a);

/// Compute the hyperbolic sine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_sinh_ps(__m256 __a);

/// Compute the hyperbolic sine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_sinh_pd(__m256d __a);

/// Compute the inverse sine of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_asin_ps(__m256 __a);

/// Compute the inverse sine of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_asin_pd(__m256d __a);

/// Compute the sine of packed single-precision (32-bit) floating-point elements
///    in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_sin_ps(__m256 __a);

/// Compute the sine of packed double-precision (64-bit) floating-point elements
///    in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_sin_pd(__m256d __a);

/// Compute the inverse hyperbolic tangent of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_atanh_ps(__m256 __a);

/// Compute the inverse hyperbolic tangent of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_atanh_pd(__m256d __a);

/// Compute the hyperbolic tangent of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_tanh_ps(__m256 __a);

/// Compute the hyperbolic tangent of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_tanh_pd(__m256d __a);

/// Compute the inverse tangent of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_atan_ps(__m256 __a);

/// Compute the inverse tangent of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_atan_pd(__m256d __a);

/// Compute the inverse tangent of packed single-precision (32-bit)
///    floating-point elements in a divided by packed elements in b, and store
///    the results in dst expressed in radians.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_atan2_ps(__m256 __a, __m256 __b);

/// Compute the inverse tangent of packed double-precision (64-bit)
///    floating-point elements in a divided by packed elements in b, and store
///    the results in dst expressed in radians.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_atan2_pd(__m256d __a, __m256d __b);

/// Compute the tangent of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_tan_ps(__m256 __a);

/// Compute the tangent of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_tan_pd(__m256d __a);

/// Compute the exponential value of e raised to the power of packed
///    single-precision (32-bit) floating-point elements in a, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_exp_ps(__m256 __a);

/// Compute the exponential value of e raised to the power of packed
///    double-precision (64-bit) floating-point elements in a, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_exp_pd(__m256d __a);

/// Compute the exponential value of 2 raised to the power of packed
///    single-precision (32-bit) floating-point elements in a, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_exp2_ps(__m256 __a);

/// Compute the exponential value of 2 raised to the power of packed
///    double-precision (64-bit) floating-point elements in a, and store the
///    results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_exp2_pd(__m256d __a);

/// Compute the exponential value of packed single-precision (32-bit)
///    floating-point elements in a raised by packed elements in b, and store
///    the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_pow_ps(__m256 __a, __m256 __b);

/// Compute the exponential value of packed double-precision (64-bit)
///    floating-point elements in a raised by packed elements in b, and store
///    the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_pow_pd(__m256d __a, __m256d __b);

/// Compute the natural logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_log_ps(__m256 __a);

/// Compute the natural logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_log_pd(__m256d __a);

/// Compute the base-2 logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_log2_ps(__m256 __a);

/// Compute the base-2 logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_log2_pd(__m256d __a);

/// Compute the base-10 logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_log10_ps(__m256 __a);

/// Compute the base-10 logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_log10_pd(__m256d __a);

/// Compute the inverse square root of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_invsqrt_ps(__m256 __a);

/// Compute the inverse square root of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_invsqrt_pd(__m256d __a);

/// Compute the cube root of packed single-precision (32-bit) floating-point
///    elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_cbrt_ps(__m256 __a);

/// Compute the cube root of packed double-precision (64-bit) floating-point
///    elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_cbrt_pd(__m256d __a);

/// Compute the inverse cube root of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_invcbrt_ps(__m256 __a);

/// Compute the inverse cube root of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_invcbrt_pd(__m256d __a);

/// Compute the error function of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_erf_ps(__m256 __a);

/// Compute the error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_erf_pd(__m256d __a);

/// Compute the complementary error function of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_erfc_ps(__m256 __a);

/// Compute the complementary error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_erfc_pd(__m256d __a);

/// Compute the inverse error function of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256 _mm256_erfinv_ps(__m256 __a);

/// Compute the inverse error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
///
/// This intrinsic has no corresponding instruction.
///
__m256d _mm256_erfinv_pd(__m256d __a);

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

#endif // defined(__AVXINTRIN_H)

#if defined(__AVX2INTRIN_H)

/* Define the default attributes for the functions in this section. */
#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__, __target__("avx2"), __min_vector_width__(256)))

/// Divide packed 32-bit integers in a by packed elements in b, and store the
///    truncated results in dst.
static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm256_div_epi32(__m256i __a, __m256i __b)
{
  return (__m256i)__builtin_svml_idiv8((__v8si)__a, (__v8si)__b);
}

/// Divide packed 32-bit integers in a by packed elements in b, and store the
///    remainders as packed 32-bit integers in dst.
static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm256_rem_epi32(__m256i __a, __m256i __b)
{
  return (__m256i)__builtin_svml_irem8((__v8si)__a, (__v8si)__b);
}

/// Divide packed unsigned 32-bit integers in a by packed elements in b, and
///    store the truncated results in dst.
static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm256_div_epu32(__m256i __a, __m256i __b)
{
  return (__m256i)__builtin_svml_udiv8((__v8su)__a, (__v8su)__b);
}

/// Divide packed unsigned 32-bit integers in a by packed elements in b, and
///    store the remainders as packed unsigned 32-bit integers in dst.
static __inline__ __m256i __DEFAULT_FN_ATTRS
_mm256_rem_epu32(__m256i __a, __m256i __b)
{
  return (__m256i)__builtin_svml_urem8((__v8su)__a, (__v8su)__b);
}

#undef __DEFAULT_FN_ATTRS

#endif // defined(__AVX2INTRIN_H)

#if defined(__AVX512FINTRIN_H)

/* Define the default attributes for the functions in this section. */
#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__, __target__("avx512f"), __min_vector_width__(512)))

/// Compute the inverse hyperbolic cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
__m512 _mm512_acosh_ps(__m512 __a);

/// Compute the inverse hyperbolic cosine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
__m512d _mm512_acosh_pd(__m512d __a);

/// Compute the hyperbolic cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
__m512 _mm512_cosh_ps(__m512 __a);

/// Compute the hyperbolic cosine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
__m512d _mm512_cosh_pd(__m512d __a);

/// Compute the inverse cosine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
__m512 _mm512_acos_ps(__m512 __a);

/// Compute the inverse cosine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
__m512d _mm512_acos_pd(__m512d __a);

/// Compute the cosine of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
__m512 _mm512_cos_ps(__m512 __a);

/// Compute the cosine of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
__m512d _mm512_cos_pd(__m512d __a);

/// Compute the inverse hyperbolic sine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
__m512 _mm512_asinh_ps(__m512 __a);

/// Compute the inverse hyperbolic sine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
__m512d _mm512_asinh_pd(__m512d __a);

/// Compute the hyperbolic sine of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
__m512 _mm512_sinh_ps(__m512 __a);

/// Compute the hyperbolic sine of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
__m512d _mm512_sinh_pd(__m512d __a);

/// Compute the inverse sine of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
__m512 _mm512_asin_ps(__m512 __a);

/// Compute the inverse sine of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
__m512d _mm512_asin_pd(__m512d __a);

/// Compute the sine of packed single-precision (32-bit) floating-point elements
///    in a expressed in radians, and store the results in dst.
__m512 _mm512_sin_ps(__m512 __a);

/// Compute the sine of packed double-precision (64-bit) floating-point elements
///    in a expressed in radians, and store the results in dst.
__m512d _mm512_sin_pd(__m512d __a);

/// Compute the inverse hyperblic tangent of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst expressed in
///    radians.
__m512 _mm512_atanh_ps(__m512 __a);

/// Compute the inverse hyperbolic tangent of packed double-precision (64-bit)
///    floating-point elements in a and store the results in dst expressed in
///    radians.
__m512d _mm512_atanh_pd(__m512d __a);

/// Compute the hyperbolic tangent of packed single-precision (32-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
__m512 _mm512_tanh_ps(__m512 __a);

/// Compute the hyperbolic tangent of packed double-precision (64-bit)
///    floating-point elements in a expressed in radians, and store the results
///    in dst.
__m512d _mm512_tanh_pd(__m512d __a);

/// Compute the inverse tangent of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst expressed in
///    radians.
__m512 _mm512_atan_ps(__m512 __a);

/// Compute the inverse tangent of packed double-precision (64-bit)
///    floating-point elements in a and store the results in dst expressed in
///    radians.
__m512d _mm512_atan_pd(__m512d __a);

/// Compute the inverse tangent of packed single-precision (32-bit)
///    floating-point elements in a divided by packed elements in b, and store
///    the results in dst expressed in radians.
__m512 _mm512_atan2_ps(__m512 __a, __m512 __b);

/// Compute the inverse tangent of packed double-precision (64-bit)
///    floating-point elements in a divided by packed elements in b, and store
///    the results in dst expressed in radians.
__m512d _mm512_atan2_pd(__m512d __a, __m512d __b);

/// Compute the tangent of packed single-precision (32-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
__m512 _mm512_tan_ps(__m512 __a);

/// Compute the tangent of packed double-precision (64-bit) floating-point
///    elements in a expressed in radians, and store the results in dst.
__m512d _mm512_tan_pd(__m512d __a);

/// Compute the exponential value of e raised to the power of packed
///    single-precision (32-bit) floating-point elements in a, and store the
///    results in dst.
__m512 _mm512_exp_ps(__m512 __a);

/// Compute the exponential value of e raised to the power of packed
///    double-precision (64-bit) floating-point elements in a, and store the
///    results in dst.
__m512d _mm512_exp_pd(__m512d __a);

/// Compute the exponential value of 2 raised to the power of packed
///    single-precision (32-bit) floating-point elements in a, and store the
///    results in dst.
__m512 _mm512_exp2_ps(__m512 __a);

/// Compute the exponential value of 2 raised to the power of packed
///    double-precision (64-bit) floating-point elements in a, and store the
///    results in dst.
__m512d _mm512_exp2_pd(__m512d __a);

/// Compute the exponential value of packed single-precision (32-bit)
///    floating-point elements in a raised by packed elements in b, and store
///    the results in dst.
__m512 _mm512_pow_ps(__m512 __a, __m512 __b);

/// Compute the exponential value of packed double-precision (64-bit)
///    floating-point elements in a raised by packed elements in b, and store
///    the results in dst.
__m512d _mm512_pow_pd(__m512d __a, __m512d __b);

/// Compute the natural logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
__m512 _mm512_log_ps(__m512 __a);

/// Compute the natural logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
__m512d _mm512_log_pd(__m512d __a);

/// Compute the base-2 logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
__m512 _mm512_log2_ps(__m512 __a);

/// Compute the base-2 logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
__m512d _mm512_log2_pd(__m512d __a);

/// Compute the base-10 logarithm of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
__inline __m512
_mm512_log10_ps(__m512 __a);

/// Compute the base-10 logarithm of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
__m512d _mm512_log10_pd(__m512d __a);

/// Compute the inverse square root of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
__m512 _mm512_invsqrt_ps(__m512 __a);

/// Compute the inverse square root of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
__m512d _mm512_invsqrt_pd(__m512d __a);

/// Compute the cube root of packed single-precision (32-bit) floating-point
///    elements in a, and store the results in dst.
__m512 _mm512_cbrt_ps(__m512 __a);

/// Compute the cube root of packed double-precision (64-bit) floating-point
///    elements in a, and store the results in dst.
__m512d _mm512_cbrt_pd(__m512d __a);

/// Compute the inverse cube root of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
__m512 _mm512_invcbrt_ps(__m512 __a);

/// Compute the inverse cube root of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
__m512d _mm512_invcbrt_pd(__m512d __a);

/// Compute the error function of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
__m512 _mm512_erf_ps(__m512 __a);

/// Compute the error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
__m512d _mm512_erf_pd(__m512d __a);

/// Compute the complementary error function of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
__m512 _mm512_erfc_ps(__m512 __a);

/// Compute the complementary error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
__m512d _mm512_erfc_pd(__m512d __a);

/// Compute the inverse error function of packed single-precision (32-bit)
///    floating-point elements in a, and store the results in dst.
__m512 _mm512_erfinv_ps(__m512 __a);

/// Compute the inverse error function of packed double-precision (64-bit)
///    floating-point elements in a, and store the results in dst.
__m512d _mm512_erfinv_pd(__m512d __a);

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

/// Divide packed 32-bit integers in a by packed elements in b, and store the
///    truncated results in dst.
static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_div_epi32(__m512i __a, __m512i __b)
{
  return (__m512i)__builtin_svml_idiv16((__v16si)__a, (__v16si)__b);
}

/// Divide packed 32-bit integers in a by packed elements in b, and store the
///    remainders as packed 32-bit integers in dst.
static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_rem_epi32(__m512i __a, __m512i __b)
{
  return (__m512i)__builtin_svml_irem16((__v16si)__a, (__v16si)__b);
}

/// Divide packed unsigned 32-bit integers in a by packed elements in b, and
///    store the truncated results in dst.
static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_div_epu32(__m512i __a, __m512i __b)
{
  return (__m512i)__builtin_svml_udiv16((__v16su)__a, (__v16su)__b);
}

/// Divide packed unsigned 32-bit integers in a by packed elements in b, and
///    store the remainders as packed unsigned 32-bit integers in dst.
static __inline__ __m512i __DEFAULT_FN_ATTRS
_mm512_rem_epu32(__m512i __a, __m512i __b)
{
  return (__m512i)__builtin_svml_urem16((__v16su)__a, (__v16su)__b);
}

#undef __DEFAULT_FN_ATTRS

#endif // defined(__AVX512FINTRIN_H)

#if defined(__cplusplus)
} // extern "C"
#endif

#endif /* __SVMLINTRIN_H */
