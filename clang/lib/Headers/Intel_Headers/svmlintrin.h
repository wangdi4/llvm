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

/*
 * Divide packed 8-bit integers in "a" by packed elements in "b", and store the
 * truncated results in "dst".
 */
__m128i _mm_div_epi8(__m128i __a, __m128i __b);

/*
 * Divide packed unsigned 8-bit integers in "a" by packed elements in "b", and
 * store the truncated results in "dst".
 */
__m128i _mm_div_epu8(__m128i __a, __m128i __b);

/*
 * Divide packed 16-bit integers in "a" by packed elements in "b", and store the
 * truncated results in "dst".
 */
__m128i _mm_div_epi16(__m128i __a, __m128i __b);

/*
 * Divide packed unsigned 16-bit integers in "a" by packed elements in "b", and
 * store the truncated results in "dst".
 */
__m128i _mm_div_epu16(__m128i __a, __m128i __b);

/*
 * Divide packed 32-bit integers in "a" by packed elements in "b", and store the
 * truncated results in "dst".
 */
__m128i _mm_div_epi32(__m128i __a, __m128i __b);

/*
 * Divide packed unsigned 32-bit integers in "a" by packed elements in "b", and
 * store the truncated results in "dst".
 */
__m128i _mm_div_epu32(__m128i __a, __m128i __b);

/*
 * Divide packed 64-bit integers in "a" by packed elements in "b", and store the
 * truncated results in "dst".
 */
__m128i _mm_div_epi64(__m128i __a, __m128i __b);

/*
 * Divide packed unsigned 64-bit integers in "a" by packed elements in "b", and
 * store the truncated results in "dst".
 */
__m128i _mm_div_epu64(__m128i __a, __m128i __b);

/*
 * Divide packed 32-bit integers in "a" by packed elements in "b", and store the
 * truncated results in "dst".
 */
__m128i _mm_idiv_epi32(__m128i __a, __m128i __b);

/*
 * Divide packed 32-bit integers in "a" by packed elements in "b", store the
 * truncated results in "dst", and store the remainders as packed 32-bit
 * integers into memory at "mem_addr".
 */
__m128i _mm_idivrem_epi32(__m128i *__mem_addr, __m128i __a, __m128i __b);

/*
 * Divide packed 32-bit integers in "a" by packed elements in "b", and store the
 * remainders as packed 32-bit integers in "dst".
 */
__m128i _mm_irem_epi32(__m128i __a, __m128i __b);

/*
 * Divide packed 8-bit integers in "a" by packed elements in "b", and store the
 * remainders as packed 32-bit integers in "dst".
 */
__m128i _mm_rem_epi8(__m128i __a, __m128i __b);

/*
 * Divide packed unsigned 8-bit integers in "a" by packed elements in "b", and
 * store the remainders as packed unsigned 32-bit integers in "dst".
 */
__m128i _mm_rem_epu8(__m128i __a, __m128i __b);

/*
 * Divide packed 16-bit integers in "a" by packed elements in "b", and store the
 * remainders as packed 32-bit integers in "dst".
 */
__m128i _mm_rem_epi16(__m128i __a, __m128i __b);

/*
 * Divide packed unsigned 16-bit integers in "a" by packed elements in "b", and
 * store the remainders as packed unsigned 32-bit integers in "dst".
 */
__m128i _mm_rem_epu16(__m128i __a, __m128i __b);

/*
 * Divide packed 32-bit integers in "a" by packed elements in "b", and store the
 * remainders as packed 32-bit integers in "dst".
 */
__m128i _mm_rem_epi32(__m128i __a, __m128i __b);

/*
 * Divide packed unsigned 32-bit integers in "a" by packed elements in "b", and
 * store the remainders as packed unsigned 32-bit integers in "dst".
 */
__m128i _mm_rem_epu32(__m128i __a, __m128i __b);

/*
 * Divide packed 64-bit integers in "a" by packed elements in "b", and store the
 * remainders as packed 32-bit integers in "dst".
 */
__m128i _mm_rem_epi64(__m128i __a, __m128i __b);

/*
 * Divide packed unsigned 64-bit integers in "a" by packed elements in "b", and
 * store the remainders as packed unsigned 32-bit integers in "dst".
 */
__m128i _mm_rem_epu64(__m128i __a, __m128i __b);

/*
 * Divide packed unsigned 32-bit integers in "a" by packed elements in "b", and
 * store the truncated results in "dst".
 */
__m128i _mm_udiv_epi32(__m128i __a, __m128i __b);

/*
 * Divide packed unsigned 32-bit integers in "a" by packed elements in "b",
 * store the truncated results in "dst", and store the remainders as packed
 * unsigned 32-bit integers into memory at "mem_addr".
 */
__m128i _mm_udivrem_epi32(__m128i *__mem_addr, __m128i __a, __m128i __b);

/*
 * Divide packed unsigned 32-bit integers in "a" by packed elements in "b", and
 * store the remainders as packed unsigned 32-bit integers in "dst".
 */
__m128i _mm_urem_epi32(__m128i __a, __m128i __b);

/*
 * Compute the cube root of packed single-precision (32-bit) floating-point
 * elements in "a", and store the results in "dst".
 */
__m128 _mm_cbrt_ps(__m128 __a);

/*
 * Compute the cube root of packed double-precision (64-bit) floating-point
 * elements in "a", and store the results in "dst".
 */
__m128d _mm_cbrt_pd(__m128d __a);

/*
 * Compute the exponential value of "e" raised to the power of packed complex
 * single-precision (32-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m128 _mm_cexp_ps(__m128 __a);

/*
 * Compute the natural logarithm of packed complex single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128 _mm_clog_ps(__m128 __a);

/*
 * Compute the square root of packed complex single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128 _mm_csqrt_ps(__m128 __a);

/*
 * Compute the exponential value of "e" raised to the power of packed
 * single-precision (32-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m128 _mm_exp_ps(__m128 __a);

/*
 * Compute the exponential value of "e" raised to the power of packed
 * double-precision (64-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m128d _mm_exp_pd(__m128d __a);

/*
 * Compute the exponential value of 10 raised to the power of packed
 * single-precision (32-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m128 _mm_exp10_ps(__m128 __a);

/*
 * Compute the exponential value of 10 raised to the power of packed
 * double-precision (64-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m128d _mm_exp10_pd(__m128d __a);

/*
 * Compute the exponential value of 2 raised to the power of packed
 * single-precision (32-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m128 _mm_exp2_ps(__m128 __a);

/*
 * Compute the exponential value of 2 raised to the power of packed
 * double-precision (64-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m128d _mm_exp2_pd(__m128d __a);

/*
 * Compute the exponential value of "e" raised to the power of packed
 * single-precision (32-bit) floating-point elements in "a", subtract one from
 * each element, and store the results in "dst".
 */
__m128 _mm_expm1_ps(__m128 __a);

/*
 * Compute the exponential value of "e" raised to the power of packed
 * double-precision (64-bit) floating-point elements in "a", subtract one from
 * each element, and store the results in "dst".
 */
__m128d _mm_expm1_pd(__m128d __a);

/*
 * Compute the inverse cube root of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128 _mm_invcbrt_ps(__m128 __a);

/*
 * Compute the inverse cube root of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128d _mm_invcbrt_pd(__m128d __a);

/*
 * Compute the inverse square root of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128 _mm_invsqrt_ps(__m128 __a);

/*
 * Compute the inverse square root of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128d _mm_invsqrt_pd(__m128d __a);

/*
 * Compute the natural logarithm of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128 _mm_log_ps(__m128 __a);

/*
 * Compute the natural logarithm of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128d _mm_log_pd(__m128d __a);

/*
 * Compute the base-10 logarithm of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128 _mm_log10_ps(__m128 __a);

/*
 * Compute the base-10 logarithm of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128d _mm_log10_pd(__m128d __a);

/*
 * Compute the natural logarithm of one plus packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128 _mm_log1p_ps(__m128 __a);

/*
 * Compute the natural logarithm of one plus packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128d _mm_log1p_pd(__m128d __a);

/*
 * Compute the base-2 logarithm of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128 _mm_log2_ps(__m128 __a);

/*
 * Compute the base-2 logarithm of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128d _mm_log2_pd(__m128d __a);

/*
 * Convert the exponent of each packed single-precision (32-bit) floating-point
 * element in "a" to a single-precision floating-point number representing the
 * integer exponent, and store the results in "dst". This intrinsic essentially
 * calculates "floor(log2(x))" for each element.
 */
__m128 _mm_logb_ps(__m128 __a);

/*
 * Convert the exponent of each packed double-precision (64-bit) floating-point
 * element in "a" to a double-precision floating-point number representing the
 * integer exponent, and store the results in "dst". This intrinsic essentially
 * calculates "floor(log2(x))" for each element.
 */
__m128d _mm_logb_pd(__m128d __a);

/*
 * Compute the exponential value of packed single-precision (32-bit)
 * floating-point elements in "a" raised by packed elements in "b", and store
 * the results in "dst".
 */
__m128 _mm_pow_ps(__m128 __a, __m128 __b);

/*
 * Compute the exponential value of packed double-precision (64-bit)
 * floating-point elements in "a" raised by packed elements in "b", and store
 * the results in "dst".
 */
__m128d _mm_pow_pd(__m128d __a, __m128d __b);

/*
 * Compute the square root of packed single-precision (32-bit) floating-point
 * elements in "a", and store the results in "dst". Note that this intrinsic is
 * less efficient than "_mm_sqrt_ps".
 */
__m128 _mm_svml_sqrt_ps(__m128 __a);

/*
 * Compute the square root of packed double-precision (64-bit) floating-point
 * elements in "a", and store the results in "dst". Note that this intrinsic is
 * less efficient than "_mm_sqrt_pd".
 */
__m128d _mm_svml_sqrt_pd(__m128d __a);

/*
 * Compute the inverse cosine of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m128 _mm_acos_ps(__m128 __a);

/*
 * Compute the inverse cosine of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m128d _mm_acos_pd(__m128d __a);

/*
 * Compute the inverse hyperbolic cosine of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m128 _mm_acosh_ps(__m128 __a);

/*
 * Compute the inverse hyperbolic cosine of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m128d _mm_acosh_pd(__m128d __a);

/*
 * Compute the inverse sine of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m128 _mm_asin_ps(__m128 __a);

/*
 * Compute the inverse sine of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m128d _mm_asin_pd(__m128d __a);

/*
 * Compute the inverse hyperbolic sine of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m128 _mm_asinh_ps(__m128 __a);

/*
 * Compute the inverse hyperbolic sine of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m128d _mm_asinh_pd(__m128d __a);

/*
 * Compute the inverse tangent of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m128 _mm_atan_ps(__m128 __a);

/*
 * Compute the inverse tangent of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m128d _mm_atan_pd(__m128d __a);

/*
 * Compute the inverse tangent of packed single-precision (32-bit)
 * floating-point elements in "a" divided by packed elements in "b", and store
 * the results in "dst" expressed in radians.
 */
__m128 _mm_atan2_ps(__m128 __a, __m128 __b);

/*
 * Compute the inverse tangent of packed double-precision (64-bit)
 * floating-point elements in "a" divided by packed elements in "b", and store
 * the results in "dst" expressed in radians.
 */
__m128d _mm_atan2_pd(__m128d __a, __m128d __b);

/*
 * Compute the inverse hyperbolic tangent of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m128 _mm_atanh_ps(__m128 __a);

/*
 * Compute the inverse hyperbolic tangent of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m128d _mm_atanh_pd(__m128d __a);

/*
 * Compute the cosine of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m128 _mm_cos_ps(__m128 __a);

/*
 * Compute the cosine of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m128d _mm_cos_pd(__m128d __a);

/*
 * Compute the cosine of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in degrees, and store the results in "dst".
 */
__m128 _mm_cosd_ps(__m128 __a);

/*
 * Compute the cosine of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in degrees, and store the results in "dst".
 */
__m128d _mm_cosd_pd(__m128d __a);

/*
 * Compute the hyperbolic cosine of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m128 _mm_cosh_ps(__m128 __a);

/*
 * Compute the hyperbolic cosine of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m128d _mm_cosh_pd(__m128d __a);

/*
 * Compute the length of the hypotenous of a right triangle, with the lengths of
 * the other two sides of the triangle stored as packed single-precision
 * (32-bit) floating-point elements in "a" and "b", and store the results in
 * "dst".
 */
__m128 _mm_hypot_ps(__m128 __a, __m128 __b);

/*
 * Compute the length of the hypotenous of a right triangle, with the lengths of
 * the other two sides of the triangle stored as packed double-precision
 * (64-bit) floating-point elements in "a" and "b", and store the results in
 * "dst".
 */
__m128d _mm_hypot_pd(__m128d __a, __m128d __b);

/*
 * Compute the sine of packed single-precision (32-bit) floating-point elements
 * in "a" expressed in radians, and store the results in "dst".
 */
__m128 _mm_sin_ps(__m128 __a);

/*
 * Compute the sine of packed double-precision (64-bit) floating-point elements
 * in "a" expressed in radians, and store the results in "dst".
 */
__m128d _mm_sin_pd(__m128d __a);

/*
 * Compute the sine of packed single-precision (32-bit) floating-point elements
 * in "a" expressed in degrees, and store the results in "dst".
 */
__m128 _mm_sind_ps(__m128 __a);

/*
 * Compute the sine of packed double-precision (64-bit) floating-point elements
 * in "a" expressed in degrees, and store the results in "dst".
 */
__m128d _mm_sind_pd(__m128d __a);

/*
 * Compute the hyperbolic sine of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m128 _mm_sinh_ps(__m128 __a);

/*
 * Compute the hyperbolic sine of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m128d _mm_sinh_pd(__m128d __a);

/*
 * Compute the tangent of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m128 _mm_tan_ps(__m128 __a);

/*
 * Compute the tangent of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m128d _mm_tan_pd(__m128d __a);

/*
 * Compute the tangent of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in degrees, and store the results in "dst".
 */
__m128 _mm_tand_ps(__m128 __a);

/*
 * Compute the tangent of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in degrees, and store the results in "dst".
 */
__m128d _mm_tand_pd(__m128d __a);

/*
 * Compute the hyperbolic tangent of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m128 _mm_tanh_ps(__m128 __a);

/*
 * Compute the hyperbolic tangent of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m128d _mm_tanh_pd(__m128d __a);

/*
 * Compute the cumulative distribution function of packed single-precision
 * (32-bit) floating-point elements in "a" using the normal distribution, and
 * store the results in "dst".
 */
__m128 _mm_cdfnorm_ps(__m128 __a);

/*
 * Compute the cumulative distribution function of packed double-precision
 * (64-bit) floating-point elements in "a" using the normal distribution, and
 * store the results in "dst".
 */
__m128d _mm_cdfnorm_pd(__m128d __a);

/*
 * Compute the inverse cumulative distribution function of packed
 * single-precision (32-bit) floating-point elements in "a" using the normal
 * distribution, and store the results in "dst".
 */
__m128 _mm_cdfnorminv_ps(__m128 __a);

/*
 * Compute the inverse cumulative distribution function of packed
 * double-precision (64-bit) floating-point elements in "a" using the normal
 * distribution, and store the results in "dst".
 */
__m128d _mm_cdfnorminv_pd(__m128d __a);

/*
 * Compute the error function of packed single-precision (32-bit) floating-point
 * elements in "a", and store the results in "dst".
 */
__m128 _mm_erf_ps(__m128 __a);

/*
 * Compute the error function of packed double-precision (64-bit) floating-point
 * elements in "a", and store the results in "dst".
 */
__m128d _mm_erf_pd(__m128d __a);

/*
 * Compute the complementary error function of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128 _mm_erfc_ps(__m128 __a);

/*
 * Compute the complementary error function of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128d _mm_erfc_pd(__m128d __a);

/*
 * Compute the inverse complementary error function of packed single-precision
 * (32-bit) floating-point elements in "a", and store the results in "dst".
 */
__m128 _mm_erfcinv_ps(__m128 __a);

/*
 * Compute the inverse complementary error function of packed double-precision
 * (64-bit) floating-point elements in "a", and store the results in "dst".
 */
__m128d _mm_erfcinv_pd(__m128d __a);

/*
 * Compute the inverse error function of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128 _mm_erfinv_ps(__m128 __a);

/*
 * Compute the inverse error function of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m128d _mm_erfinv_pd(__m128d __a);

/*
 * Round the packed single-precision (32-bit) floating-point elements in a to
 * the nearest integer value, and store the results as packed single-precision
 * floating-point elements in dst.
 */
__m128 _mm_svml_round_ps(__m128 __a);

/*
 * Round the packed double-precision (64-bit) floating-point elements in a to
 * the nearest integer value, and store the results as packed double-precision
 * floating-point elements in dst.
 */
__m128d _mm_svml_round_pd(__m128d __a);

#if defined(__SSE4_1__)

/*
 * Round the packed single-precision (32-bit) floating-point elements in a up
 * to an integer value, and store the results as packed single-precision
 * floating-point elements in dst.
 *
 * This intrinsic may generate the roundps/vroundps instruction.
 */
#define _mm_svml_ceil_ps(X)   _mm_ceil_ps(X)

/*
 * Round the packed single-precision (32-bit) floating-point elements in a down
 * to an integer value, and store the results as packed single-precision
 * floating-point elements in dst.
 *
 * This intrinsic may generate the roundps/vroundps instruction.
 */
#define _mm_svml_floor_ps(X)  _mm_floor_ps(X)

/*
 * Truncate the packed single-precision (32-bit) floating-point elements in a,
 * and store the results as packed single-precision floating-point elements
 * in dst.
 *
 * This intrinsic may generate the roundps/vroundps instruction.
 */
#define _mm_trunc_ps(X)       _mm_round_ps((X), _MM_FROUND_TRUNC)

/*
 * Round the packed double-precision (64-bit) floating-point elements in a up
 * to an integer value, and store the results as packed double-precision
 * floating-point elements in dst.
 *
 * This intrinsic may generate the roundpd/vroundpd instruction.
 */
#define _mm_svml_ceil_pd(X)   _mm_ceil_pd(X)

/*
 * Round the packed double-precision (64-bit) floating-point elements in a down
 * to an integer value, and store the results as packed double-precision
 * floating-point elements in dst.
 *
 * This intrinsic may generate the roundpd/vroundpd instruction.
 */
#define _mm_svml_floor_pd(X)  _mm_floor_pd(X)

/*
 * Truncate the packed double-precision (64-bit) floating-point elements in a,
 * and store the results as packed double-precision floating-point elements
 * in dst.
 *
 * This intrinsic may generate the roundpd/vroundpd instruction.
 */
#define _mm_trunc_pd(X)       _mm_round_pd((X), _MM_FROUND_TRUNC)

#else // defined(__SSE4_1__)

/*
 * Round the packed single-precision (32-bit) floating-point elements in "a" up
 * to an integer value, and store the results as packed single-precision
 * floating-point elements in "dst".
 *
 * This intrinsic may generate the "roundps"/"vroundps" instruction.
 */
__m128 _mm_svml_ceil_ps(__m128 __a);

/*
 * Round the packed double-precision (64-bit) floating-point elements in "a" up
 * to an integer value, and store the results as packed double-precision
 * floating-point elements in "dst".
 *
 * This intrinsic may generate the "roundpd"/"vroundpd" instruction.
 */
__m128d _mm_svml_ceil_pd(__m128d __a);

/*
 * Round the packed single-precision (32-bit) floating-point elements in "a"
 * down to an integer value, and store the results as packed single-precision
 * floating-point elements in "dst".
 *
 * This intrinsic may generate the "roundps"/"vroundps" instruction.
 */
__m128 _mm_svml_floor_ps(__m128 __a);

/*
 * Round the packed double-precision (64-bit) floating-point elements in "a"
 * down to an integer value, and store the results as packed double-precision
 * floating-point elements in "dst".
 *
 * This intrinsic may generate the "roundpd"/"vroundpd" instruction.
 */
__m128d _mm_svml_floor_pd(__m128d __a);

/*
 * Truncate the packed single-precision (32-bit) floating-point elements in "a",
 * and store the results as packed single-precision floating-point elements in
 * "dst".
 *
 * This intrinsic may generate the "roundps"/"vroundps" instruction.
 */
__m128 _mm_trunc_ps(__m128 __a);

/*
 * Truncate the packed double-precision (64-bit) floating-point elements in "a",
 * and store the results as packed double-precision floating-point elements in
 * "dst".
 *
 * This intrinsic may generate the "roundpd"/"vroundpd" instruction.
 */
__m128d _mm_trunc_pd(__m128d __a);

#endif // defined(__SSE4_1__)

#endif // defined(__XMMINTRIN_H) && defined(__EMMINTRIN_H)

#if defined(__AVXINTRIN_H)

/*
 * Compute the cube root of packed single-precision (32-bit) floating-point
 * elements in "a", and store the results in "dst".
 */
__m256 _mm256_cbrt_ps(__m256 __a);

/*
 * Compute the cube root of packed double-precision (64-bit) floating-point
 * elements in "a", and store the results in "dst".
 */
__m256d _mm256_cbrt_pd(__m256d __a);

/*
 * Compute the exponential value of "e" raised to the power of packed complex
 * single-precision (32-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m256 _mm256_cexp_ps(__m256 __a);

/*
 * Compute the natural logarithm of packed complex single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256 _mm256_clog_ps(__m256 __a);

/*
 * Compute the square root of packed complex single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256 _mm256_csqrt_ps(__m256 __a);

/*
 * Compute the exponential value of "e" raised to the power of packed
 * single-precision (32-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m256 _mm256_exp_ps(__m256 __a);

/*
 * Compute the exponential value of "e" raised to the power of packed
 * double-precision (64-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m256d _mm256_exp_pd(__m256d __a);

/*
 * Compute the exponential value of 10 raised to the power of packed
 * single-precision (32-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m256 _mm256_exp10_ps(__m256 __a);

/*
 * Compute the exponential value of 10 raised to the power of packed
 * double-precision (64-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m256d _mm256_exp10_pd(__m256d __a);

/*
 * Compute the exponential value of 2 raised to the power of packed
 * single-precision (32-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m256 _mm256_exp2_ps(__m256 __a);

/*
 * Compute the exponential value of 2 raised to the power of packed
 * double-precision (64-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m256d _mm256_exp2_pd(__m256d __a);

/*
 * Compute the exponential value of "e" raised to the power of packed
 * single-precision (32-bit) floating-point elements in "a", subtract one from
 * each element, and store the results in "dst".
 */
__m256 _mm256_expm1_ps(__m256 __a);

/*
 * Compute the exponential value of "e" raised to the power of packed
 * double-precision (64-bit) floating-point elements in "a", subtract one from
 * each element, and store the results in "dst".
 */
__m256d _mm256_expm1_pd(__m256d __a);

/*
 * Compute the inverse cube root of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256 _mm256_invcbrt_ps(__m256 __a);

/*
 * Compute the inverse cube root of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256d _mm256_invcbrt_pd(__m256d __a);

/*
 * Compute the inverse square root of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256 _mm256_invsqrt_ps(__m256 __a);

/*
 * Compute the inverse square root of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256d _mm256_invsqrt_pd(__m256d __a);

/*
 * Compute the natural logarithm of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256 _mm256_log_ps(__m256 __a);

/*
 * Compute the natural logarithm of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256d _mm256_log_pd(__m256d __a);

/*
 * Compute the base-10 logarithm of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256 _mm256_log10_ps(__m256 __a);

/*
 * Compute the base-10 logarithm of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256d _mm256_log10_pd(__m256d __a);

/*
 * Compute the natural logarithm of one plus packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256 _mm256_log1p_ps(__m256 __a);

/*
 * Compute the natural logarithm of one plus packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256d _mm256_log1p_pd(__m256d __a);

/*
 * Compute the base-2 logarithm of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256 _mm256_log2_ps(__m256 __a);

/*
 * Compute the base-2 logarithm of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256d _mm256_log2_pd(__m256d __a);

/*
 * Convert the exponent of each packed single-precision (32-bit) floating-point
 * element in "a" to a single-precision floating-point number representing the
 * integer exponent, and store the results in "dst". This intrinsic essentially
 * calculates "floor(log2(x))" for each element.
 */
__m256 _mm256_logb_ps(__m256 __a);

/*
 * Convert the exponent of each packed double-precision (64-bit) floating-point
 * element in "a" to a double-precision floating-point number representing the
 * integer exponent, and store the results in "dst". This intrinsic essentially
 * calculates "floor(log2(x))" for each element.
 */
__m256d _mm256_logb_pd(__m256d __a);

/*
 * Compute the exponential value of packed single-precision (32-bit)
 * floating-point elements in "a" raised by packed elements in "b", and store
 * the results in "dst".
 */
__m256 _mm256_pow_ps(__m256 __a, __m256 __b);

/*
 * Compute the exponential value of packed double-precision (64-bit)
 * floating-point elements in "a" raised by packed elements in "b", and store
 * the results in "dst".
 */
__m256d _mm256_pow_pd(__m256d __a, __m256d __b);

/*
 * Compute the square root of packed single-precision (32-bit) floating-point
 * elements in "a", and store the results in "dst". Note that this intrinsic is
 * less efficient than "_mm_sqrt_ps".
 */
__m256 _mm256_svml_sqrt_ps(__m256 __a);

/*
 * Compute the square root of packed double-precision (64-bit) floating-point
 * elements in "a", and store the results in "dst". Note that this intrinsic is
 * less efficient than "_mm_sqrt_pd".
 */
__m256d _mm256_svml_sqrt_pd(__m256d __a);

/*
 * Compute the inverse cosine of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m256 _mm256_acos_ps(__m256 __a);

/*
 * Compute the inverse cosine of packed double-precision (64-bit) floating-point
 * elements in "a", and store the results in "dst".
 */
__m256d _mm256_acos_pd(__m256d __a);

/*
 * Compute the inverse hyperbolic cosine of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m256 _mm256_acosh_ps(__m256 __a);

/*
 * Compute the inverse hyperbolic cosine of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m256d _mm256_acosh_pd(__m256d __a);

/*
 * Compute the inverse sine of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m256 _mm256_asin_ps(__m256 __a);

/*
 * Compute the inverse sine of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m256d _mm256_asin_pd(__m256d __a);

/*
 * Compute the inverse hyperbolic sine of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m256 _mm256_asinh_ps(__m256 __a);

/*
 * Compute the inverse hyperbolic sine of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m256d _mm256_asinh_pd(__m256d __a);

/*
 * Compute the inverse tangent of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m256 _mm256_atan_ps(__m256 __a);

/*
 * Compute the inverse tangent of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m256d _mm256_atan_pd(__m256d __a);

/*
 * Compute the inverse tangent of packed single-precision (32-bit)
 * floating-point elements in "a" divided by packed elements in "b", and store
 * the results in "dst" expressed in radians.
 */
__m256 _mm256_atan2_ps(__m256 __a, __m256 __b);

/*
 * Compute the inverse tangent of packed double-precision (64-bit)
 * floating-point elements in "a" divided by packed elements in "b", and store
 * the results in "dst" expressed in radians.
 */
__m256d _mm256_atan2_pd(__m256d __a, __m256d __b);

/*
 * Compute the inverse hyperbolic tangent of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m256 _mm256_atanh_ps(__m256 __a);

/*
 * Compute the inverse hyperbolic tangent of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m256d _mm256_atanh_pd(__m256d __a);

/*
 * Compute the cosine of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m256 _mm256_cos_ps(__m256 __a);

/*
 * Compute the cosine of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m256d _mm256_cos_pd(__m256d __a);

/*
 * Compute the cosine of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in degrees, and store the results in "dst".
 */
__m256 _mm256_cosd_ps(__m256 __a);

/*
 * Compute the cosine of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in degrees, and store the results in "dst".
 */
__m256d _mm256_cosd_pd(__m256d __a);

/*
 * Compute the hyperbolic cosine of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m256 _mm256_cosh_ps(__m256 __a);

/*
 * Compute the hyperbolic cosine of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m256d _mm256_cosh_pd(__m256d __a);

/*
 * Compute the length of the hypotenous of a right triangle, with the lengths of
 * the other two sides of the triangle stored as packed single-precision
 * (32-bit) floating-point elements in "a" and "b", and store the results in
 * "dst".
 */
__m256 _mm256_hypot_ps(__m256 __a, __m256 __b);

/*
 * Compute the length of the hypotenous of a right triangle, with the lengths of
 * the other two sides of the triangle stored as packed double-precision
 * (64-bit) floating-point elements in "a" and "b", and store the results in
 * "dst".
 */
__m256d _mm256_hypot_pd(__m256d __a, __m256d __b);

/*
 * Compute the sine of packed single-precision (32-bit) floating-point elements
 * in "a" expressed in radians, and store the results in "dst".
 */
__m256 _mm256_sin_ps(__m256 __a);

/*
 * Compute the sine of packed double-precision (64-bit) floating-point elements
 * in "a" expressed in radians, and store the results in "dst".
 */
__m256d _mm256_sin_pd(__m256d __a);

/*
 * Compute the sine of packed single-precision (32-bit) floating-point elements
 * in "a" expressed in degrees, and store the results in "dst".
 */
__m256 _mm256_sind_ps(__m256 __a);

/*
 * Compute the sine of packed double-precision (64-bit) floating-point elements
 * in "a" expressed in degrees, and store the results in "dst".
 */
__m256d _mm256_sind_pd(__m256d __a);

/*
 * Compute the hyperbolic sine of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m256 _mm256_sinh_ps(__m256 __a);

/*
 * Compute the hyperbolic sine of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m256d _mm256_sinh_pd(__m256d __a);

/*
 * Compute the tangent of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m256 _mm256_tan_ps(__m256 __a);

/*
 * Compute the tangent of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m256d _mm256_tan_pd(__m256d __a);

/*
 * Compute the tangent of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in degrees, and store the results in "dst".
 */
__m256 _mm256_tand_ps(__m256 __a);

/*
 * Compute the tangent of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in degrees, and store the results in "dst".
 */
__m256d _mm256_tand_pd(__m256d __a);

/*
 * Compute the hyperbolic tangent of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m256 _mm256_tanh_ps(__m256 __a);

/*
 * Compute the hyperbolic tangent of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m256d _mm256_tanh_pd(__m256d __a);

/*
 * Compute the cumulative distribution function of packed single-precision
 * (32-bit) floating-point elements in "a" using the normal distribution, and
 * store the results in "dst".
 */
__m256 _mm256_cdfnorm_ps(__m256 __a);

/*
 * Compute the cumulative distribution function of packed double-precision
 * (64-bit) floating-point elements in "a" using the normal distribution, and
 * store the results in "dst".
 */
__m256d _mm256_cdfnorm_pd(__m256d __a);

/*
 * Compute the inverse cumulative distribution function of packed
 * single-precision (32-bit) floating-point elements in "a" using the normal
 * distribution, and store the results in "dst".
 */
__m256 _mm256_cdfnorminv_ps(__m256 __a);

/*
 * Compute the inverse cumulative distribution function of packed
 * double-precision (64-bit) floating-point elements in "a" using the normal
 * distribution, and store the results in "dst".
 */
__m256d _mm256_cdfnorminv_pd(__m256d __a);

/*
 * Compute the error function of packed single-precision (32-bit) floating-point
 * elements in "a", and store the results in "dst".
 */
__m256 _mm256_erf_ps(__m256 __a);

/*
 * Compute the error function of packed double-precision (64-bit) floating-point
 * elements in "a", and store the results in "dst".
 */
__m256d _mm256_erf_pd(__m256d __a);

/*
 * Compute the complementary error function of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256 _mm256_erfc_ps(__m256 __a);

/*
 * Compute the complementary error function of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256d _mm256_erfc_pd(__m256d __a);

/*
 * Compute the inverse complementary error function of packed single-precision
 * (32-bit) floating-point elements in "a", and store the results in "dst".
 */
__m256 _mm256_erfcinv_ps(__m256 __a);

/*
 * Compute the inverse complementary error function of packed double-precision
 * (64-bit) floating-point elements in "a", and store the results in "dst".
 */
__m256d _mm256_erfcinv_pd(__m256d __a);

/*
 * Compute the inverse error function of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256 _mm256_erfinv_ps(__m256 __a);

/*
 * Compute the inverse error function of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m256d _mm256_erfinv_pd(__m256d __a);

/*
 * Round the packed double-precision (64-bit) floating-point elements in a to
 * the nearest integer value, and store the results as packed
 * double-precision floating-point elements in dst.
 */
__m256 _mm256_svml_round_ps(__m256 __a);

/*
 * Round the packed double-precision (64-bit) floating-point elements in a to
 * the nearest integer value, and store the results as packed
 * double-precision floating-point elements in dst.
 */
__m256d _mm256_svml_round_pd(__m256d __a);

/*
 * Round the packed single-precision (32-bit) floating-point elements in a up
 *  o an integer value, and store the results as packed single-precision
 * floating-point elements in dst.
 *
 *  This intrinsic may generate the roundps/vroundps instruction.
 */
#define _mm256_svml_ceil_ps(X)   _mm256_ceil_ps(X)

/*
 * Round the packed single-precision (32-bit) floating-point elements in a down
 * to an integer value, and store the results as packed single-precision
 * floating-point elements in dst.
 *
 * This intrinsic may generate the roundps/vroundps instruction.
 */
#define _mm256_svml_floor_ps(X)  _mm256_floor_ps(X)

/*
 * Truncate the packed single-precision (32-bit) floating-point elements in a,
 * and store the results as packed single-precision floating-point elements
 * in dst.
 *
 * This intrinsic may generate the roundps/vroundps instruction.
 */
#define _mm256_trunc_ps(X)       _mm256_round_ps((X), _MM_FROUND_TRUNC)

/*
 * Round the packed double-precision (64-bit) floating-point elements in a up
 * to an integer value, and store the results as packed double-precision
 * floating-point elements in dst.
 *
 * This intrinsic may generate the roundpd/vroundpd instruction.
 */
#define _mm256_svml_ceil_pd(X)   _mm256_ceil_pd(X)

/*
 * Round the packed double-precision (64-bit) floating-point elements in a down
 * to an integer value, and store the results as packed double-precision
 * floating-point elements in dst.
 *
 * This intrinsic may generate the roundpd/vroundpd instruction.
 */
#define _mm256_svml_floor_pd(X)  _mm256_floor_pd(X)

/*
 * Truncate the packed double-precision (64-bit) floating-point elements in a,
 * and store the results as packed double-precision floating-point elements
 * in dst.
 *
 * This intrinsic may generate the roundpd/vroundpd instruction.
 */
#define _mm256_trunc_pd(X)       _mm256_round_pd((X), _MM_FROUND_TRUNC)

#endif // defined(__AVXINTRIN_H)

#if defined(__AVX2INTRIN_H)

/*
 * Divide packed 8-bit integers in "a" by packed elements in "b", and store the
 * truncated results in "dst".
 */
__m256i _mm256_div_epi8(__m256i __a, __m256i __b);

/*
 * Divide packed unsigned 8-bit integers in "a" by packed elements in "b", and
 * store the truncated results in "dst".
 */
__m256i _mm256_div_epu8(__m256i __a, __m256i __b);

/*
 * Divide packed 16-bit integers in "a" by packed elements in "b", and store the
 * truncated results in "dst".
 */
__m256i _mm256_div_epi16(__m256i __a, __m256i __b);

/*
 * Divide packed unsigned 16-bit integers in "a" by packed elements in "b", and
 * store the truncated results in "dst".
 */
__m256i _mm256_div_epu16(__m256i __a, __m256i __b);

/*
 * Divide packed 32-bit integers in "a" by packed elements in "b", and store the
 * truncated results in "dst".
 */
__m256i _mm256_div_epi32(__m256i __a, __m256i __b);

/*
 * Divide packed unsigned 32-bit integers in "a" by packed elements in "b", and
 * store the truncated results in "dst".
 */
__m256i _mm256_div_epu32(__m256i __a, __m256i __b);

/*
 * Divide packed 64-bit integers in "a" by packed elements in "b", and store the
 * truncated results in "dst".
 */
__m256i _mm256_div_epi64(__m256i __a, __m256i __b);

/*
 * Divide packed unsigned 64-bit integers in "a" by packed elements in "b", and
 * store the truncated results in "dst".
 */
__m256i _mm256_div_epu64(__m256i __a, __m256i __b);

/*
 * Divide packed 32-bit integers in "a" by packed elements in "b", and store the
 * truncated results in "dst".
 */
__m256i _mm256_idiv_epi32(__m256i __a, __m256i __b);

/*
 * Divide packed 32-bit integers in "a" by packed elements in "b", store the
 * truncated results in "dst", and store the remainders as packed 32-bit
 * integers into memory at "mem_addr".
 */
__m256i _mm256_idivrem_epi32(__m256i *__mem_addr, __m256i __a, __m256i __b);

/*
 * Divide packed 32-bit integers in "a" by packed elements in "b", and store the
 * remainders as packed 32-bit integers in "dst".
 */
__m256i _mm256_irem_epi32(__m256i __a, __m256i __b);

/*
 * Divide packed 8-bit integers in "a" by packed elements in "b", and store the
 * remainders as packed 32-bit integers in "dst".
 */
__m256i _mm256_rem_epi8(__m256i __a, __m256i __b);

/*
 * Divide packed unsigned 8-bit integers in "a" by packed elements in "b", and
 * store the remainders as packed unsigned 32-bit integers in "dst".
 */
__m256i _mm256_rem_epu8(__m256i __a, __m256i __b);

/*
 * Divide packed 16-bit integers in "a" by packed elements in "b", and store the
 * remainders as packed 32-bit integers in "dst".
 */
__m256i _mm256_rem_epi16(__m256i __a, __m256i __b);

/*
 * Divide packed unsigned 16-bit integers in "a" by packed elements in "b", and
 * store the remainders as packed unsigned 32-bit integers in "dst".
 */
__m256i _mm256_rem_epu16(__m256i __a, __m256i __b);

/*
 * Divide packed 32-bit integers in "a" by packed elements in "b", and store the
 * remainders as packed 32-bit integers in "dst".
 */
__m256i _mm256_rem_epi32(__m256i __a, __m256i __b);

/*
 * Divide packed unsigned 32-bit integers in "a" by packed elements in "b", and
 * store the remainders as packed unsigned 32-bit integers in "dst".
 */
__m256i _mm256_rem_epu32(__m256i __a, __m256i __b);

/*
 * Divide packed 64-bit integers in "a" by packed elements in "b", and store the
 * remainders as packed 32-bit integers in "dst".
 */
__m256i _mm256_rem_epi64(__m256i __a, __m256i __b);

/*
 * Divide packed unsigned 64-bit integers in "a" by packed elements in "b", and
 * store the remainders as packed unsigned 32-bit integers in "dst".
 */
__m256i _mm256_rem_epu64(__m256i __a, __m256i __b);

/*
 * Divide packed unsigned 32-bit integers in "a" by packed elements in "b", and
 * store the truncated results in "dst".
 */
__m256i _mm256_udiv_epi32(__m256i __a, __m256i __b);

/*
 * Divide packed unsigned 32-bit integers in "a" by packed elements in "b",
 * store the truncated results in "dst", and store the remainders as packed
 * unsigned 32-bit integers into memory at "mem_addr".
 */
__m256i _mm256_udivrem_epi32(__m256i *__mem_addr, __m256i __a, __m256i __b);

/*
 * Divide packed unsigned 32-bit integers in "a" by packed elements in "b", and
 * store the remainders as packed unsigned 32-bit integers in "dst".
 */
__m256i _mm256_urem_epi32(__m256i __a, __m256i __b);

#endif // defined(__AVX2INTRIN_H)

#if defined(__AVX512FINTRIN_H)

/* Define the default attributes for the functions in this section. */
#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__, __target__("avx512f"), __min_vector_width__(512)))

/*
 * Divide packed 8-bit integers in "a" by packed elements in "b", and store the
 * truncated results in "dst".
 */
__m512i _mm512_div_epi8(__m512i __a, __m512i __b);

/*
 * Divide packed unsigned 8-bit integers in "a" by packed elements in "b", and
 * store the truncated results in "dst".
 */
__m512i _mm512_div_epu8(__m512i __a, __m512i __b);

/*
 * Divide packed 16-bit integers in "a" by packed elements in "b", and store the
 * truncated results in "dst".
 */
__m512i _mm512_div_epi16(__m512i __a, __m512i __b);

/*
 * Divide packed unsigned 16-bit integers in "a" by packed elements in "b", and
 * store the truncated results in "dst".
 */
__m512i _mm512_div_epu16(__m512i __a, __m512i __b);

/*
 * Divide packed 32-bit integers in "a" by packed elements in "b", and store the
 * truncated results in "dst".
 */
__m512i _mm512_div_epi32(__m512i __a, __m512i __b);

/*
 * Divide packed unsigned 32-bit integers in "a" by packed elements in "b", and
 * store the truncated results in "dst".
 */
__m512i _mm512_div_epu32(__m512i __a, __m512i __b);

/*
 * Divide packed 32-bit integers in "a" by packed elements in "b", and store the
 * truncated results in "dst" using writemask "k" (elements are copied from
 * "src" when the corresponding mask bit is not set).
 */
__m512i _mm512_mask_div_epi32(__m512i __src, __mmask16 __k, __m512i __a,
                              __m512i __b);

/*
 * Divide packed unsigned 32-bit integers in "a" by packed elements in "b", and
 * store the truncated results in "dst" using writemask "k" (elements are copied
 * from "src" when the corresponding mask bit is not set).
 */
__m512i _mm512_mask_div_epu32(__m512i __src, __mmask16 __k, __m512i __a,
                              __m512i __b);

/*
 * Divide packed 64-bit integers in "a" by packed elements in "b", and store the
 * truncated results in "dst".
 */
__m512i _mm512_div_epi64(__m512i __a, __m512i __b);

/*
 * Divide packed unsigned 64-bit integers in "a" by packed elements in "b", and
 * store the truncated results in "dst".
 */
__m512i _mm512_div_epu64(__m512i __a, __m512i __b);

/*
 * Divide packed 8-bit integers in "a" by packed elements in "b", and store the
 * remainders as packed 32-bit integers in "dst".
 */
__m512i _mm512_rem_epi8(__m512i __a, __m512i __b);

/*
 * Divide packed unsigned 8-bit integers in "a" by packed elements in "b", and
 * store the remainders as packed unsigned 32-bit integers in "dst".
 */
__m512i _mm512_rem_epu8(__m512i __a, __m512i __b);

/*
 * Divide packed 16-bit integers in "a" by packed elements in "b", and store the
 * remainders as packed 32-bit integers in "dst".
 */
__m512i _mm512_rem_epi16(__m512i __a, __m512i __b);

/*
 * Divide packed unsigned 16-bit integers in "a" by packed elements in "b", and
 * store the remainders as packed unsigned 32-bit integers in "dst".
 */
__m512i _mm512_rem_epu16(__m512i __a, __m512i __b);

/*
 * Divide packed 32-bit integers in "a" by packed elements in "b", and store the
 * remainders as packed 32-bit integers in "dst".
 */
__m512i _mm512_rem_epi32(__m512i __a, __m512i __b);

/*
 * Divide packed unsigned 32-bit integers in "a" by packed elements in "b", and
 * store the remainders as packed unsigned 32-bit integers in "dst".
 */
__m512i _mm512_rem_epu32(__m512i __a, __m512i __b);

/*
 * Divide packed 32-bit integers in "a" by packed elements in "b", and store the
 * remainders as packed 32-bit integers in "dst" using writemask "k" (elements
 * are copied from "src" when the corresponding mask bit is not set).
 */
__m512i _mm512_mask_rem_epi32(__m512i __src, __mmask16 __k, __m512i __a,
                              __m512i __b);

/*
 * Divide packed unsigned 32-bit integers in "a" by packed elements in "b", and
 * store the remainders as packed unsigned 32-bit integers in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512i _mm512_mask_rem_epu32(__m512i __src, __mmask16 __k, __m512i __a,
                              __m512i __b);

/*
 * Divide packed 64-bit integers in "a" by packed elements in "b", and store the
 * remainders as packed 32-bit integers in "dst".
 */
__m512i _mm512_rem_epi64(__m512i __a, __m512i __b);

/*
 * Divide packed unsigned 64-bit integers in "a" by packed elements in "b", and
 * store the remainders as packed unsigned 32-bit integers in "dst".
 */
__m512i _mm512_rem_epu64(__m512i __a, __m512i __b);

/*
 * Compute the cube root of packed single-precision (32-bit) floating-point
 * elements in "a", and store the results in "dst".
 */
__m512 _mm512_cbrt_ps(__m512 __a);

/*
 * Compute the cube root of packed single-precision (32-bit) floating-point
 * elements in "a", and store the results in "dst" using writemask "k" (elements
 * are copied from "src" when the corresponding mask bit is not set).
 */
__m512 _mm512_mask_cbrt_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the cube root of packed double-precision (64-bit) floating-point
 * elements in "a", and store the results in "dst".
 */
__m512d _mm512_cbrt_pd(__m512d __a);

/*
 * Compute the cube root of packed double-precision (64-bit) floating-point
 * elements in "a", and store the results in "dst" using writemask "k" (elements
 * are copied from "src" when the corresponding mask bit is not set).
 */
__m512d _mm512_mask_cbrt_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the exponential value of "e" raised to the power of packed
 * single-precision (32-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m512 _mm512_exp_ps(__m512 __a);

/*
 * Compute the exponential value of "e" raised to the power of packed
 * single-precision (32-bit) floating-point elements in "a", and store the
 * results in "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512 _mm512_mask_exp_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the exponential value of "e" raised to the power of packed
 * double-precision (64-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m512d _mm512_exp_pd(__m512d __a);

/*
 * Compute the exponential value of "e" raised to the power of packed
 * double-precision (64-bit) floating-point elements in "a", and store the
 * results in "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512d _mm512_mask_exp_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the exponential value of 10 raised to the power of packed
 * single-precision (32-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m512 _mm512_exp10_ps(__m512 __a);

/*
 * Compute the exponential value of 10 raised to the power of packed
 * single-precision (32-bit) floating-point elements in "a", and store the
 * results in "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512 _mm512_mask_exp10_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the exponential value of 10 raised to the power of packed
 * double-precision (64-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m512d _mm512_exp10_pd(__m512d __a);

/*
 * Compute the exponential value of 10 raised to the power of packed
 * double-precision (64-bit) floating-point elements in "a", and store the
 * results in "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512d _mm512_mask_exp10_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the exponential value of 2 raised to the power of packed
 * single-precision (32-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m512 _mm512_exp2_ps(__m512 __a);

/*
 * Compute the exponential value of 2 raised to the power of packed
 * single-precision (32-bit) floating-point elements in "a", and store the
 * results in "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512 _mm512_mask_exp2_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the exponential value of 2 raised to the power of packed
 * double-precision (64-bit) floating-point elements in "a", and store the
 * results in "dst".
 */
__m512d _mm512_exp2_pd(__m512d __a);

/*
 * Compute the exponential value of 2 raised to the power of packed
 * double-precision (64-bit) floating-point elements in "a", and store the
 * results in "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512d _mm512_mask_exp2_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the exponential value of "e" raised to the power of packed
 * single-precision (32-bit) floating-point elements in "a", subtract one from
 * each element, and store the results in "dst".
 */
__m512 _mm512_expm1_ps(__m512 __a);

/*
 * Compute the exponential value of "e" raised to the power of packed
 * single-precision (32-bit) floating-point elements in "a", subtract one from
 * each element, and store the results in "dst" using writemask "k" (elements
 * are copied from "src" when the corresponding mask bit is not set).
 */
__m512 _mm512_mask_expm1_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the exponential value of "e" raised to the power of packed
 * double-precision (64-bit) floating-point elements in "a", subtract one from
 * each element, and store the results in "dst".
 */
__m512d _mm512_expm1_pd(__m512d __a);

/*
 * Compute the exponential value of "e" raised to the power of packed
 * double-precision (64-bit) floating-point elements in "a", subtract one from
 * each element, and store the results in "dst" using writemask "k" (elements
 * are copied from "src" when the corresponding mask bit is not set).
 */
__m512d _mm512_mask_expm1_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the length of the hypotenous of a right triangle, with the lengths of
 * the other two sides of the triangle stored as packed single-precision
 * (32-bit) floating-point elements in "a" and "b", and store the results in
 * "dst".
 */
__m512 _mm512_hypot_ps(__m512 __a, __m512 __b);

/*
 * Compute the length of the hypotenous of a right triangle, with the lengths of
 * the other two sides of the triangle stored as packed single-precision
 * (32-bit) floating-point elements in "a" and "b", and store the results in
 * "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512 _mm512_mask_hypot_ps(__m512 __src, __mmask16 __k, __m512 __a,
                            __m512 __b);

/*
 * Compute the length of the hypotenous of a right triangle, with the lengths of
 * the other two sides of the triangle stored as packed double-precision
 * (64-bit) floating-point elements in "a" and "b", and store the results in
 * "dst".
 */
__m512d _mm512_hypot_pd(__m512d __a, __m512d __b);

/*
 * Compute the length of the hypotenous of a right triangle, with the lengths of
 * the other two sides of the triangle stored as packed double-precision
 * (64-bit) floating-point elements in "a" and "b", and store the results in
 * "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512d _mm512_mask_hypot_pd(__m512d __src, __mmask8 __k, __m512d __a,
                             __m512d __b);

/*
 * Compute the inverse square root of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m512 _mm512_invsqrt_ps(__m512 __a);

/*
 * Compute the inverse square root of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512 _mm512_mask_invsqrt_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the inverse square root of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m512d _mm512_invsqrt_pd(__m512d __a);

/*
 * Compute the inverse square root of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512d _mm512_mask_invsqrt_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the natural logarithm of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m512 _mm512_log_ps(__m512 __a);

/*
 * Compute the natural logarithm of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512 _mm512_mask_log_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the natural logarithm of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m512d _mm512_log_pd(__m512d __a);

/*
 * Compute the natural logarithm of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512d _mm512_mask_log_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the base-10 logarithm of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m512 _mm512_log10_ps(__m512 __a);

/*
 * Compute the base-10 logarithm of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512 _mm512_mask_log10_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the base-10 logarithm of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m512d _mm512_log10_pd(__m512d __a);

/*
 * Compute the base-10 logarithm of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512d _mm512_mask_log10_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the natural logarithm of one plus packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m512 _mm512_log1p_ps(__m512 __a);

/*
 * Compute the natural logarithm of one plus packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512 _mm512_mask_log1p_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the natural logarithm of one plus packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m512d _mm512_log1p_pd(__m512d __a);

/*
 * Compute the natural logarithm of one plus packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512d _mm512_mask_log1p_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the base-2 logarithm of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m512 _mm512_log2_ps(__m512 __a);

/*
 * Compute the base-2 logarithm of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512 _mm512_mask_log2_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the base-2 logarithm of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m512d _mm512_log2_pd(__m512d __a);

/*
 * Compute the base-2 logarithm of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512d _mm512_mask_log2_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Convert the exponent of each packed single-precision (32-bit) floating-point
 * element in "a" to a single-precision floating-point number representing the
 * integer exponent, and store the results in "dst". This intrinsic essentially
 * calculates "floor(log2(x))" for each element.
 */
__m512 _mm512_logb_ps(__m512 __a);

/*
 * Convert the exponent of each packed single-precision (32-bit) floating-point
 * element in "a" to a single-precision floating-point number representing the
 * integer exponent, and store the results in "dst" using writemask "k"
 * (elements are copied from "src" when the corresponding mask bit is not set).
 * This intrinsic essentially calculates "floor(log2(x))" for each element.
 */
__m512 _mm512_mask_logb_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Convert the exponent of each packed double-precision (64-bit) floating-point
 * element in "a" to a double-precision floating-point number representing the
 * integer exponent, and store the results in "dst". This intrinsic essentially
 * calculates "floor(log2(x))" for each element.
 */
__m512d _mm512_logb_pd(__m512d __a);

/*
 * Convert the exponent of each packed double-precision (64-bit) floating-point
 * element in "a" to a double-precision floating-point number representing the
 * integer exponent, and store the results in "dst" using writemask "k"
 * (elements are copied from "src" when the corresponding mask bit is not set).
 * This intrinsic essentially calculates "floor(log2(x))" for each element.
 */
__m512d _mm512_mask_logb_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the exponential value of packed single-precision (32-bit)
 * floating-point elements in "a" raised by packed elements in "b", and store
 * the results in "dst".
 */
__m512 _mm512_pow_ps(__m512 __a, __m512 __b);

/*
 * Compute the exponential value of packed single-precision (32-bit)
 * floating-point elements in "a" raised by packed elements in "b", and store
 * the results in "dst" using writemask "k" (elements are copied from "src" when
 * the corresponding mask bit is not set).
 */
__m512 _mm512_mask_pow_ps(__m512 __src, __mmask16 __k, __m512 __a, __m512 __b);

/*
 * Compute the exponential value of packed double-precision (64-bit)
 * floating-point elements in "a" raised by packed elements in "b", and store
 * the results in "dst".
 */
__m512d _mm512_pow_pd(__m512d __a, __m512d __b);

/*
 * Compute the exponential value of packed double-precision (64-bit)
 * floating-point elements in "a" raised by packed elements in "b", and store
 * the results in "dst" using writemask "k" (elements are copied from "src" when
 * the corresponding mask bit is not set).
 */
__m512d _mm512_mask_pow_pd(__m512d __src, __mmask8 __k, __m512d __a,
                           __m512d __b);

/*
 * Computes the reciprocal of packed single-precision (32-bit) floating-point
 * elements in "a", storing the results in "dst".
 */
__m512 _mm512_recip_ps(__m512 __a);

/*
 * Computes the reciprocal of packed single-precision (32-bit) floating-point
 * elements in "a", storing the results in "dst" using writemask "k" (elements
 * are copied from "src" when the corresponding mask bit is not set).
 */
__m512 _mm512_mask_recip_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Computes the reciprocal of packed double-precision (64-bit) floating-point
 * elements in "a", storing the results in "dst".
 */
__m512d _mm512_recip_pd(__m512d __a);

/*
 * Computes the reciprocal of packed double-precision (64-bit) floating-point
 * elements in "a", storing the results in "dst" using writemask "k" (elements
 * are copied from "src" when the corresponding mask bit is not set).
 */
__m512d _mm512_mask_recip_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the inverse cosine of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m512 _mm512_acos_ps(__m512 __a);

/*
 * Compute the inverse cosine of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512 _mm512_mask_acos_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the inverse cosine of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m512d _mm512_acos_pd(__m512d __a);

/*
 * Compute the inverse cosine of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512d _mm512_mask_acos_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the inverse hyperbolic cosine of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m512 _mm512_acosh_ps(__m512 __a);

/*
 * Compute the inverse hyperbolic cosine of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512 _mm512_mask_acosh_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the inverse hyperbolic cosine of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m512d _mm512_acosh_pd(__m512d __a);

/*
 * Compute the inverse hyperbolic cosine of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512d _mm512_mask_acosh_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the inverse sine of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m512 _mm512_asin_ps(__m512 __a);

/*
 * Compute the inverse sine of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512 _mm512_mask_asin_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the inverse sine of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m512d _mm512_asin_pd(__m512d __a);

/*
 * Compute the inverse sine of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512d _mm512_mask_asin_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the inverse hyperbolic sine of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m512 _mm512_asinh_ps(__m512 __a);

/*
 * Compute the inverse hyperbolic sine of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512 _mm512_mask_asinh_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the inverse hyperbolic sine of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m512d _mm512_asinh_pd(__m512d __a);

/*
 * Compute the inverse hyperbolic sine of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512d _mm512_mask_asinh_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the inverse tangent of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst" expressed in
 * radians.
 */
__m512 _mm512_atan_ps(__m512 __a);

/*
 * Compute the inverse tangent of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512 _mm512_mask_atan_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the inverse tangent of packed double-precision (64-bit)
 * floating-point elements in "a" and store the results in "dst" expressed in
 * radians.
 */
__m512d _mm512_atan_pd(__m512d __a);

/*
 * Compute the inverse tangent of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst" expressed in
 * radians using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512d _mm512_mask_atan_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the inverse tangent of packed single-precision (32-bit)
 * floating-point elements in "a" divided by packed elements in "b", and store
 * the results in "dst" expressed in radians.
 */
__m512 _mm512_atan2_ps(__m512 __a, __m512 __b);

/*
 * Compute the inverse tangent of packed single-precision (32-bit)
 * floating-point elements in "a" divided by packed elements in "b", and store
 * the results in "dst" expressed in radians using writemask "k" (elements are
 * copied from "src" when the corresponding mask bit is not set).
 */
__m512 _mm512_mask_atan2_ps(__m512 __src, __mmask16 __k, __m512 __a,
                            __m512 __b);

/*
 * Compute the inverse tangent of packed double-precision (64-bit)
 * floating-point elements in "a" divided by packed elements in "b", and store
 * the results in "dst" expressed in radians.
 */
__m512d _mm512_atan2_pd(__m512d __a, __m512d __b);

/*
 * Compute the inverse tangent of packed double-precision (64-bit)
 * floating-point elements in "a" divided by packed elements in "b", and store
 * the results in "dst" expressed in radians using writemask "k" (elements are
 * copied from "src" when the corresponding mask bit is not set).
 */
__m512d _mm512_mask_atan2_pd(__m512d __src, __mmask8 __k, __m512d __a,
                             __m512d __b);

/*
 * Compute the inverse hyperblic tangent of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst" expressed in
 * radians.
 */
__m512 _mm512_atanh_ps(__m512 __a);

/*
 * Compute the inverse hyperbolic tangent of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512 _mm512_mask_atanh_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the inverse hyperbolic tangent of packed double-precision (64-bit)
 * floating-point elements in "a" and store the results in "dst" expressed in
 * radians.
 */
__m512d _mm512_atanh_pd(__m512d __a);

/*
 * Compute the inverse hyperbolic tangent of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst" expressed in
 * radians using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512d _mm512_mask_atanh_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the cosine of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m512 _mm512_cos_ps(__m512 __a);

/*
 * Compute the cosine of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512 _mm512_mask_cos_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the cosine of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m512d _mm512_cos_pd(__m512d __a);

/*
 * Compute the cosine of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512d _mm512_mask_cos_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the cosine of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in degrees, and store the results in "dst".
 */
__m512 _mm512_cosd_ps(__m512 __a);

/*
 * Compute the cosine of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in degrees, and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512 _mm512_mask_cosd_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the cosine of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in degrees, and store the results in "dst".
 */
__m512d _mm512_cosd_pd(__m512d __a);

/*
 * Compute the cosine of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in degrees, and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512d _mm512_mask_cosd_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the hyperbolic cosine of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m512 _mm512_cosh_ps(__m512 __a);

/*
 * Compute the hyperbolic cosine of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512 _mm512_mask_cosh_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the hyperbolic cosine of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m512d _mm512_cosh_pd(__m512d __a);

/*
 * Compute the hyperbolic cosine of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512d _mm512_mask_cosh_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the sine of packed single-precision (32-bit) floating-point elements
 * in "a" expressed in radians, and store the results in "dst".
 */
__m512 _mm512_sin_ps(__m512 __a);

/*
 * Compute the sine of packed single-precision (32-bit) floating-point elements
 * in "a" expressed in radians, and store the results in "dst" using writemask
 * "k" (elements are copied from "src" when the corresponding mask bit is not
 * set).
 */
__m512 _mm512_mask_sin_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the sine of packed double-precision (64-bit) floating-point elements
 * in "a" expressed in radians, and store the results in "dst".
 */
__m512d _mm512_sin_pd(__m512d __a);

/*
 * Compute the sine of packed double-precision (64-bit) floating-point elements
 * in "a" expressed in radians, and store the results in "dst" using writemask
 * "k" (elements are copied from "src" when the corresponding mask bit is not
 * set).
 */
__m512d _mm512_mask_sin_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the sine of packed single-precision (32-bit) floating-point elements
 * in "a" expressed in degrees, and store the results in "dst".
 */
__m512 _mm512_sind_ps(__m512 __a);

/*
 * Compute the sine of packed single-precision (32-bit) floating-point elements
 * in "a" expressed in degrees, and store the results in "dst" using writemask
 * "k" (elements are copied from "src" when the corresponding mask bit is not
 * set).
 */
__m512 _mm512_mask_sind_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the sine of packed double-precision (64-bit) floating-point elements
 * in "a" expressed in degrees, and store the results in "dst".
 */
__m512d _mm512_sind_pd(__m512d __a);

/*
 * Compute the sine of packed double-precision (64-bit) floating-point elements
 * in "a" expressed in degrees, and store the results in "dst" using writemask
 * "k" (elements are copied from "src" when the corresponding mask bit is not
 * set).
 */
__m512d _mm512_mask_sind_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the hyperbolic sine of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m512 _mm512_sinh_ps(__m512 __a);

/*
 * Compute the hyperbolic sine of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512 _mm512_mask_sinh_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the hyperbolic sine of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m512d _mm512_sinh_pd(__m512d __a);

/*
 * Compute the hyperbolic sine of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512d _mm512_mask_sinh_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the tangent of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m512 _mm512_tan_ps(__m512 __a);

/*
 * Compute the tangent of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512 _mm512_mask_tan_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the tangent of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst".
 */
__m512d _mm512_tan_pd(__m512d __a);

/*
 * Compute the tangent of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in radians, and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512d _mm512_mask_tan_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the tangent of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in degrees, and store the results in "dst".
 */
__m512 _mm512_tand_ps(__m512 __a);

/*
 * Compute the tangent of packed single-precision (32-bit) floating-point
 * elements in "a" expressed in degrees, and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512 _mm512_mask_tand_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the tangent of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in degrees, and store the results in "dst".
 */
__m512d _mm512_tand_pd(__m512d __a);

/*
 * Compute the tangent of packed double-precision (64-bit) floating-point
 * elements in "a" expressed in degrees, and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512d _mm512_mask_tand_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the hyperbolic tangent of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m512 _mm512_tanh_ps(__m512 __a);

/*
 * Compute the hyperbolic tangent of packed single-precision (32-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512 _mm512_mask_tanh_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the hyperbolic tangent of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst".
 */
__m512d _mm512_tanh_pd(__m512d __a);

/*
 * Compute the hyperbolic tangent of packed double-precision (64-bit)
 * floating-point elements in "a" expressed in radians, and store the results in
 * "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
__m512d _mm512_mask_tanh_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the cumulative distribution function of packed single-precision
 * (32-bit) floating-point elements in "a" using the normal distribution, and
 * store the results in "dst".
 */
__m512 _mm512_cdfnorm_ps(__m512 __a);

/*
 * Compute the cumulative distribution function of packed single-precision
 * (32-bit) floating-point elements in "a" using the normal distribution, and
 * store the results in "dst" using writemask "k" (elements are copied from
 * "src" when the corresponding mask bit is not set).
 */
__m512 _mm512_mask_cdfnorm_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the cumulative distribution function of packed double-precision
 * (64-bit) floating-point elements in "a" using the normal distribution, and
 * store the results in "dst".
 */
__m512d _mm512_cdfnorm_pd(__m512d __a);

/*
 * Compute the cumulative distribution function of packed double-precision
 * (64-bit) floating-point elements in "a" using the normal distribution, and
 * store the results in "dst" using writemask "k" (elements are copied from
 * "src" when the corresponding mask bit is not set).
 */
__m512d _mm512_mask_cdfnorm_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the inverse cumulative distribution function of packed
 * single-precision (32-bit) floating-point elements in "a" using the normal
 * distribution, and store the results in "dst".
 */
__m512 _mm512_cdfnorminv_ps(__m512 __a);

/*
 * Compute the inverse cumulative distribution function of packed
 * single-precision (32-bit) floating-point elements in "a" using the normal
 * distribution, and store the results in "dst" using writemask "k" (elements
 * are copied from "src" when the corresponding mask bit is not set).
 */
__m512 _mm512_mask_cdfnorminv_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the inverse cumulative distribution function of packed
 * double-precision (64-bit) floating-point elements in "a" using the normal
 * distribution, and store the results in "dst".
 */
__m512d _mm512_cdfnorminv_pd(__m512d __a);

/*
 * Compute the inverse cumulative distribution function of packed
 * double-precision (64-bit) floating-point elements in "a" using the normal
 * distribution, and store the results in "dst" using writemask "k" (elements
 * are copied from "src" when the corresponding mask bit is not set).
 */
__m512d _mm512_mask_cdfnorminv_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the error function of packed single-precision (32-bit) floating-point
 * elements in "a", and store the results in "dst".
 */
__m512 _mm512_erf_ps(__m512 __a);

/*
 * Compute the error function of packed single-precision (32-bit) floating-point
 * elements in "a", and store the results in "dst" using writemask "k" (elements
 * are copied from "src" when the corresponding mask bit is not set).
 */
__m512 _mm512_mask_erf_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the error function of packed double-precision (64-bit) floating-point
 * elements in "a", and store the results in "dst".
 */
__m512d _mm512_erf_pd(__m512d __a);

/*
 * Compute the error function of packed double-precision (64-bit) floating-point
 * elements in "a", and store the results in "dst" using writemask "k" (elements
 * are copied from "src" when the corresponding mask bit is not set).
 */
__m512d _mm512_mask_erf_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the complementary error function of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m512 _mm512_erfc_ps(__m512 __a);

/*
 * Compute the complementary error function of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512 _mm512_mask_erfc_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the complementary error function of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m512d _mm512_erfc_pd(__m512d __a);

/*
 * Compute the complementary error function of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512d _mm512_mask_erfc_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the inverse complementary error function of packed single-precision
 * (32-bit) floating-point elements in "a", and store the results in "dst".
 */
__m512 _mm512_erfcinv_ps(__m512 __a);

/*
 * Compute the inverse complementary error function of packed single-precision
 * (32-bit) floating-point elements in "a", and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512 _mm512_mask_erfcinv_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the inverse complementary error function of packed double-precision
 * (64-bit) floating-point elements in "a", and store the results in "dst".
 */
__m512d _mm512_erfcinv_pd(__m512d __a);

/*
 * Compute the inverse complementary error function of packed double-precision
 * (64-bit) floating-point elements in "a", and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512d _mm512_mask_erfcinv_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Compute the inverse error function of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m512 _mm512_erfinv_ps(__m512 __a);

/*
 * Compute the inverse error function of packed single-precision (32-bit)
 * floating-point elements in "a", and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512 _mm512_mask_erfinv_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Compute the inverse error function of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst".
 */
__m512d _mm512_erfinv_pd(__m512d __a);

/*
 * Compute the inverse error function of packed double-precision (64-bit)
 * floating-point elements in "a", and store the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512d _mm512_mask_erfinv_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Rounds each packed single-precision (32-bit) floating-point element in "a" to
 * the nearest integer value and stores the results as packed double-precision
 * floating-point elements in "dst".
 */
__m512 _mm512_nearbyint_ps(__m512 __a);

/*
 * Rounds each packed single-precision (32-bit) floating-point element in "a" to
 * the nearest integer value and stores the results as packed double-precision
 * floating-point elements in "dst" using writemask "k" (elements are copied
 * from "src" when the corresponding mask bit is not set).
 */
__m512 _mm512_mask_nearbyint_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Rounds each packed double-precision (64-bit) floating-point element in "a" to
 * the nearest integer value and stores the results as packed double-precision
 * floating-point elements in "dst".
 */
__m512d _mm512_nearbyint_pd(__m512d __a);

/*
 * Rounds each packed double-precision (64-bit) floating-point element in "a" to
 * the nearest integer value and stores the results as packed double-precision
 * floating-point elements in "dst" using writemask "k" (elements are copied
 * from "src" when the corresponding mask bit is not set).
 */
__m512d _mm512_mask_nearbyint_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Rounds the packed single-precision (32-bit) floating-point elements in "a" to
 * the nearest even integer value and stores the results in "dst".
 */
__m512 _mm512_rint_ps(__m512 __a);

/*
 * Rounds the packed single-precision (32-bit) floating-point elements in "a" to
 * the nearest even integer value and stores the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512 _mm512_mask_rint_ps(__m512 __src, __mmask16 __k, __m512 __a);

/*
 * Rounds the packed double-precision (64-bit) floating-point elements in "a" to
 * the nearest even integer value and stores the results in "dst".
 */
__m512d _mm512_rint_pd(__m512d __a);

/*
 * Rounds the packed double-precision (64-bit) floating-point elements in "a" to
 * the nearest even integer value and stores the results in "dst" using
 * writemask "k" (elements are copied from "src" when the corresponding mask bit
 * is not set).
 */
__m512d _mm512_mask_rint_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Round the packed double-precision (64-bit) floating-point elements in "a" to
 * the nearest integer value, and store the results as packed double-precision
 * floating-point elements in "dst".
 */
__m512d _mm512_svml_round_pd(__m512d __a);

/*
 * Round the packed double-precision (64-bit) floating-point elements in "a" to
 * the nearest integer value, and store the results as packed double-precision
 * floating-point elements in "dst" using writemask "k" (elements are copied
 * from "src" when the corresponding mask bit is not set).
 */
__m512d _mm512_mask_svml_round_pd(__m512d __src, __mmask8 __k, __m512d __a);

/*
 * Truncate the packed single-precision (32-bit) floating-point elements in "a",
 * and store the results as packed single-precision floating-point elements in
 * "dst".
 */
static __inline __m512 __DEFAULT_FN_ATTRS
_mm512_trunc_ps(__m512 __a)
{
  return _mm512_roundscale_ps(__a, _MM_FROUND_TRUNC);
}

/*
 * Truncate the packed single-precision (32-bit) floating-point elements in "a",
 * and store the results as packed single-precision floating-point elements in
 * "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
static __inline__ __m512 __DEFAULT_FN_ATTRS
_mm512_mask_trunc_ps (__m512 __src, __mmask16 __k, __m512 __a)
{
  return _mm512_mask_roundscale_ps(__src, __k, __a, _MM_FROUND_TRUNC);
}

/*
 * Truncate the packed double-precision (64-bit) floating-point elements in "a",
 * and store the results as packed double-precision floating-point elements in
 * "dst".
 */
static __inline __m512d __DEFAULT_FN_ATTRS
_mm512_trunc_pd(__m512d __a)
{
  return _mm512_roundscale_pd(__a, _MM_FROUND_TRUNC);
}

/*
 * Truncate the packed double-precision (64-bit) floating-point elements in "a",
 * and store the results as packed double-precision floating-point elements in
 * "dst" using writemask "k" (elements are copied from "src" when the
 * corresponding mask bit is not set).
 */
static __inline__ __m512d __DEFAULT_FN_ATTRS
_mm512_mask_trunc_pd (__m512d __src, __mmask8 __k, __m512d __a)
{
  return _mm512_mask_roundscale_pd(__src, __k, __a, _MM_FROUND_TRUNC);
}

#undef __DEFAULT_FN_ATTRS

#endif // defined(__AVX512FINTRIN_H)

#if defined(__cplusplus)
} // extern "C"
#endif

#endif /* __SVMLINTRIN_H */
