
// Copyright (c) 2010-2011(1997-2013) Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

typedef double double2 __attribute__((ext_vector_type(2)));
typedef double double3 __attribute__((ext_vector_type(3)));
typedef double double4 __attribute__((ext_vector_type(4)));
typedef double double8 __attribute__((ext_vector_type(8)));
typedef double double16 __attribute__((ext_vector_type(16)));

typedef half half2 __attribute__((ext_vector_type(2)));
typedef half half3 __attribute__((ext_vector_type(3)));
typedef half half4 __attribute__((ext_vector_type(4)));
typedef half half8 __attribute__((ext_vector_type(8)));
typedef half half16 __attribute__((ext_vector_type(16)));

int printf(__constant const char* st, ...);
double  __attribute__((const)) __attribute__((overloadable)) acos(double);
double2  __attribute__((const)) __attribute__((overloadable)) acos(double2);
double3  __attribute__((const)) __attribute__((overloadable)) acos(double3);
double4  __attribute__((const)) __attribute__((overloadable)) acos(double4);
double8  __attribute__((const)) __attribute__((overloadable)) acos(double8);
double16  __attribute__((const)) __attribute__((overloadable)) acos(double16);
//half  __attribute__((const)) __attribute__((overloadable)) acos(half);
//half2  __attribute__((const)) __attribute__((overloadable)) acos(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) acos(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) acos(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) acos(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) acos(half16);

/**
 * Inverse hyperbolic cosine.
 */
double  __attribute__((const)) __attribute__((overloadable)) acosh(double);
double2  __attribute__((const)) __attribute__((overloadable)) acosh(double2);
double3  __attribute__((const)) __attribute__((overloadable)) acosh(double3);
double4  __attribute__((const)) __attribute__((overloadable)) acosh(double4);
double8  __attribute__((const)) __attribute__((overloadable)) acosh(double8);
double16  __attribute__((const)) __attribute__((overloadable)) acosh(double16);
//half  __attribute__((const)) __attribute__((overloadable)) acosh(half);
//half2  __attribute__((const)) __attribute__((overloadable)) acosh(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) acosh(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) acosh(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) acosh(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) acosh(half16);

/**
 * Compute acos (x) / PI.
 */
double  __attribute__((const)) __attribute__((overloadable)) acospi(double x);
double2  __attribute__((const)) __attribute__((overloadable)) acospi(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) acospi(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) acospi(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) acospi(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) acospi(double16 x);
//half  __attribute__((const)) __attribute__((overloadable)) acospi(half x);
//half2  __attribute__((const)) __attribute__((overloadable)) acospi(half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) acospi(half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) acospi(half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) acospi(half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) acospi(half16 x);

/**
 * Arc sine function.
 */
double  __attribute__((const)) __attribute__((overloadable)) asin(double);
double2  __attribute__((const)) __attribute__((overloadable)) asin(double2);
double3  __attribute__((const)) __attribute__((overloadable)) asin(double3);
double4  __attribute__((const)) __attribute__((overloadable)) asin(double4);
double8  __attribute__((const)) __attribute__((overloadable)) asin(double8);
double16  __attribute__((const)) __attribute__((overloadable)) asin(double16);
//half  __attribute__((const)) __attribute__((overloadable)) asin(half);
//half2  __attribute__((const)) __attribute__((overloadable)) asin(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) asin(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) asin(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) asin(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) asin(half16);

/**
 * Inverse hyperbolic sine.
 */
double  __attribute__((const)) __attribute__((overloadable)) asinh(double);
double2  __attribute__((const)) __attribute__((overloadable)) asinh(double2);
double3  __attribute__((const)) __attribute__((overloadable)) asinh(double3);
double4  __attribute__((const)) __attribute__((overloadable)) asinh(double4);
double8  __attribute__((const)) __attribute__((overloadable)) asinh(double8);
double16  __attribute__((const)) __attribute__((overloadable)) asinh(double16);
//half  __attribute__((const)) __attribute__((overloadable)) asinh(half);
//half2  __attribute__((const)) __attribute__((overloadable)) asinh(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) asinh(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) asinh(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) asinh(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) asinh(half16);

/**
 * Compute asin (x) / PI.
 */
double  __attribute__((const)) __attribute__((overloadable)) asinpi(double x);
double2  __attribute__((const)) __attribute__((overloadable)) asinpi(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) asinpi(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) asinpi(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) asinpi(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) asinpi(double16 x);
//half  __attribute__((const)) __attribute__((overloadable)) asinpi(half x);
//half2  __attribute__((const)) __attribute__((overloadable)) asinpi(half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) asinpi(half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) asinpi(half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) asinpi(half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) asinpi(half16 x);

/**
 * Arc tangent function.
 */
double  __attribute__((const)) __attribute__((overloadable)) atan(double y_over_x);
double2  __attribute__((const)) __attribute__((overloadable)) atan(double2 y_over_x);
double3  __attribute__((const)) __attribute__((overloadable)) atan(double3 y_over_x);
double4  __attribute__((const)) __attribute__((overloadable)) atan(double4 y_over_x);
double8  __attribute__((const)) __attribute__((overloadable)) atan(double8 y_over_x);
double16  __attribute__((const)) __attribute__((overloadable)) atan(double16 y_over_x);
//half  __attribute__((const)) __attribute__((overloadable)) atan(half y_over_x);
//half2  __attribute__((const)) __attribute__((overloadable)) atan(half2 y_over_x);
//half3  __attribute__((const)) __attribute__((overloadable)) atan(half3 y_over_x);
//half4  __attribute__((const)) __attribute__((overloadable)) atan(half4 y_over_x);
//half8  __attribute__((const)) __attribute__((overloadable)) atan(half8 y_over_x);
//half16  __attribute__((const)) __attribute__((overloadable)) atan(half16 y_over_x);

/**
 * Arc tangent of y / x.
 */
double  __attribute__((const)) __attribute__((overloadable)) atan2(double y, double x);
double2  __attribute__((const)) __attribute__((overloadable)) atan2(double2 y, double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) atan2(double3 y, double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) atan2(double4 y, double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) atan2(double8 y, double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) atan2(double16 y, double16 x);
//half  __attribute__((const)) __attribute__((overloadable)) atan2(half y, half x);
//half2  __attribute__((const)) __attribute__((overloadable)) atan2(half2 y, half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) atan2(half3 y, half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) atan2(half4 y, half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) atan2(half8 y, half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) atan2(half16 y, half16 x);

/**
 * Hyperbolic arc tangent.
 */
double  __attribute__((const)) __attribute__((overloadable)) atanh(double);
double2  __attribute__((const)) __attribute__((overloadable)) atanh(double2);
double3  __attribute__((const)) __attribute__((overloadable)) atanh(double3);
double4  __attribute__((const)) __attribute__((overloadable)) atanh(double4);
double8  __attribute__((const)) __attribute__((overloadable)) atanh(double8);
double16  __attribute__((const)) __attribute__((overloadable)) atanh(double16);
//half  __attribute__((const)) __attribute__((overloadable)) atanh(half);
//half2  __attribute__((const)) __attribute__((overloadable)) atanh(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) atanh(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) atanh(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) atanh(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) atanh(half16);

/**
 * Compute atan (x) / PI.
 */
double  __attribute__((const)) __attribute__((overloadable)) atanpi(double x);
double2  __attribute__((const)) __attribute__((overloadable)) atanpi(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) atanpi(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) atanpi(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) atanpi(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) atanpi(double16 x);
//half  __attribute__((const)) __attribute__((overloadable)) atanpi(half x);
//half2  __attribute__((const)) __attribute__((overloadable)) atanpi(half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) atanpi(half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) atanpi(half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) atanpi(half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) atanpi(half16 x);

/**
 * Compute atan2 (y, x) / PI.
 */
double  __attribute__((const)) __attribute__((overloadable)) atan2pi(double y, double x);
double2  __attribute__((const)) __attribute__((overloadable)) atan2pi(double2 y, double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) atan2pi(double3 y, double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) atan2pi(double4 y, double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) atan2pi(double8 y, double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) atan2pi(double16 y, double16 x);
//half  __attribute__((const)) __attribute__((overloadable)) atan2pi(half y, half x);
//half2  __attribute__((const)) __attribute__((overloadable)) atan2pi(half2 y, half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) atan2pi(half3 y, half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) atan2pi(half4 y, half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) atan2pi(half8 y, half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) atan2pi(half16 y, half16 x);

/**
 * Compute cube-root.
 */
double  __attribute__((const)) __attribute__((overloadable)) cbrt(double);
double2  __attribute__((const)) __attribute__((overloadable)) cbrt(double2);
double3  __attribute__((const)) __attribute__((overloadable)) cbrt(double3);
double4  __attribute__((const)) __attribute__((overloadable)) cbrt(double4);
double8  __attribute__((const)) __attribute__((overloadable)) cbrt(double8);
double16  __attribute__((const)) __attribute__((overloadable)) cbrt(double16);
//half  __attribute__((const)) __attribute__((overloadable)) cbrt(half);
//half2  __attribute__((const)) __attribute__((overloadable)) cbrt(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) cbrt(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) cbrt(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) cbrt(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) cbrt(half16);

/**
 * Round to integral value using the round to +ve
 * infinity rounding mode.
 */
double  __attribute__((const)) __attribute__((overloadable)) ceil(double);
double2  __attribute__((const)) __attribute__((overloadable)) ceil(double2);
double3  __attribute__((const)) __attribute__((overloadable)) ceil(double3);
double4  __attribute__((const)) __attribute__((overloadable)) ceil(double4);
double8  __attribute__((const)) __attribute__((overloadable)) ceil(double8);
double16  __attribute__((const)) __attribute__((overloadable)) ceil(double16);
//half  __attribute__((const)) __attribute__((overloadable)) ceil(half);
//half2  __attribute__((const)) __attribute__((overloadable)) ceil(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) ceil(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) ceil(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) ceil(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) ceil(half16);

/**
 * Returns x with its sign changed to match the sign of
 * y.
 */
double  __attribute__((const)) __attribute__((overloadable)) copysign(double x, double y);
double2  __attribute__((const)) __attribute__((overloadable)) copysign(double2 x, double2 y);
double3  __attribute__((const)) __attribute__((overloadable)) copysign(double3 x, double3 y);
double4  __attribute__((const)) __attribute__((overloadable)) copysign(double4 x, double4 y);
double8  __attribute__((const)) __attribute__((overloadable)) copysign(double8 x, double8 y);
double16  __attribute__((const)) __attribute__((overloadable)) copysign(double16 x, double16 y);
//half  __attribute__((const)) __attribute__((overloadable)) copysign(half x, half y);
//half2  __attribute__((const)) __attribute__((overloadable)) copysign(half2 x, half2 y);
//half3  __attribute__((const)) __attribute__((overloadable)) copysign(half3 x, half3 y);
//half4  __attribute__((const)) __attribute__((overloadable)) copysign(half4 x, half4 y);
//half8  __attribute__((const)) __attribute__((overloadable)) copysign(half8 x, half8 y);
//half16  __attribute__((const)) __attribute__((overloadable)) copysign(half16 x, half16 y);

/**
 * Compute cosine.
 */
double  __attribute__((const)) __attribute__((overloadable)) cos(double);
double2  __attribute__((const)) __attribute__((overloadable)) cos(double2);
double3  __attribute__((const)) __attribute__((overloadable)) cos(double3);
double4  __attribute__((const)) __attribute__((overloadable)) cos(double4);
double8  __attribute__((const)) __attribute__((overloadable)) cos(double8);
double16  __attribute__((const)) __attribute__((overloadable)) cos(double16);
//half  __attribute__((const)) __attribute__((overloadable)) cos(half);
//half2  __attribute__((const)) __attribute__((overloadable)) cos(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) cos(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) cos(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) cos(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) cos(half16);

/**
 * Compute hyperbolic cosine.
 */
double  __attribute__((const)) __attribute__((overloadable)) cosh(double);
double2  __attribute__((const)) __attribute__((overloadable)) cosh(double2);
double3  __attribute__((const)) __attribute__((overloadable)) cosh(double3);
double4  __attribute__((const)) __attribute__((overloadable)) cosh(double4);
double8  __attribute__((const)) __attribute__((overloadable)) cosh(double8);
double16  __attribute__((const)) __attribute__((overloadable)) cosh(double16);
//half  __attribute__((const)) __attribute__((overloadable)) cosh(half);
//half2  __attribute__((const)) __attribute__((overloadable)) cosh(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) cosh(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) cosh(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) cosh(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) cosh(half16);

/**
 * Compute cos (PI * x).
 */
double  __attribute__((const)) __attribute__((overloadable)) cospi(double x);
double2  __attribute__((const)) __attribute__((overloadable)) cospi(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) cospi(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) cospi(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) cospi(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) cospi(double16 x);
//half  __attribute__((const)) __attribute__((overloadable)) cospi(half x);
//half2  __attribute__((const)) __attribute__((overloadable)) cospi(half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) cospi(half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) cospi(half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) cospi(half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) cospi(half16 x);

/**
 * Complementary error function.
 */
double  __attribute__((const)) __attribute__((overloadable)) erfc(double);
double2  __attribute__((const)) __attribute__((overloadable)) erfc(double2);
double3  __attribute__((const)) __attribute__((overloadable)) erfc(double3);
double4  __attribute__((const)) __attribute__((overloadable)) erfc(double4);
double8  __attribute__((const)) __attribute__((overloadable)) erfc(double8);
double16  __attribute__((const)) __attribute__((overloadable)) erfc(double16);
//half  __attribute__((const)) __attribute__((overloadable)) erfc(half);
//half2  __attribute__((const)) __attribute__((overloadable)) erfc(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) erfc(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) erfc(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) erfc(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) erfc(half16);

/**
 * Error function encountered in integrating the
 * normal distribution.
 */
double  __attribute__((const)) __attribute__((overloadable)) erf(double);
double2  __attribute__((const)) __attribute__((overloadable)) erf(double2);
double3  __attribute__((const)) __attribute__((overloadable)) erf(double3);
double4  __attribute__((const)) __attribute__((overloadable)) erf(double4);
double8  __attribute__((const)) __attribute__((overloadable)) erf(double8);
double16  __attribute__((const)) __attribute__((overloadable)) erf(double16);
//half  __attribute__((const)) __attribute__((overloadable)) erf(half);
//half2  __attribute__((const)) __attribute__((overloadable)) erf(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) erf(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) erf(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) erf(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) erf(half16);

/**
 * Compute the base- e exponential of x.
 */
double  __attribute__((const)) __attribute__((overloadable)) exp(double x);
double2  __attribute__((const)) __attribute__((overloadable)) exp(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) exp(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) exp(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) exp(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) exp(double16 x);
//half  __attribute__((const)) __attribute__((overloadable)) exp(half x);
//half2  __attribute__((const)) __attribute__((overloadable)) exp(half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) exp(half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) exp(half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) exp(half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) exp(half16 x);

/**
 * Exponential base 2 function.
 */
double  __attribute__((const)) __attribute__((overloadable)) exp2(double);
double2  __attribute__((const)) __attribute__((overloadable)) exp2(double2);
double3  __attribute__((const)) __attribute__((overloadable)) exp2(double3);
double4  __attribute__((const)) __attribute__((overloadable)) exp2(double4);
double8  __attribute__((const)) __attribute__((overloadable)) exp2(double8);
double16  __attribute__((const)) __attribute__((overloadable)) exp2(double16);
//half  __attribute__((const)) __attribute__((overloadable)) exp2(half);
//half2  __attribute__((const)) __attribute__((overloadable)) exp2(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) exp2(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) exp2(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) exp2(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) exp2(half16);

/**
 * Exponential base 10 function.
 */
double  __attribute__((const)) __attribute__((overloadable)) exp10(double);
double2  __attribute__((const)) __attribute__((overloadable)) exp10(double2);
double3  __attribute__((const)) __attribute__((overloadable)) exp10(double3);
double4  __attribute__((const)) __attribute__((overloadable)) exp10(double4);
double8  __attribute__((const)) __attribute__((overloadable)) exp10(double8);
double16  __attribute__((const)) __attribute__((overloadable)) exp10(double16);
//half  __attribute__((const)) __attribute__((overloadable)) exp10(half);
//half2  __attribute__((const)) __attribute__((overloadable)) exp10(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) exp10(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) exp10(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) exp10(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) exp10(half16);

/**
 * Compute e^x- 1.0.
 */
double  __attribute__((const)) __attribute__((overloadable)) expm1(double x);
double2  __attribute__((const)) __attribute__((overloadable)) expm1(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) expm1(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) expm1(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) expm1(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) expm1(double16 x);
//half  __attribute__((const)) __attribute__((overloadable)) expm1(half x);
//half2  __attribute__((const)) __attribute__((overloadable)) expm1(half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) expm1(half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) expm1(half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) expm1(half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) expm1(half16 x);

/**
 * Compute absolute value of a floating-point number.
 */
double  __attribute__((const)) __attribute__((overloadable)) fabs(double);
double2  __attribute__((const)) __attribute__((overloadable)) fabs(double2);
double3  __attribute__((const)) __attribute__((overloadable)) fabs(double3);
double4  __attribute__((const)) __attribute__((overloadable)) fabs(double4);
double8  __attribute__((const)) __attribute__((overloadable)) fabs(double8);
double16  __attribute__((const)) __attribute__((overloadable)) fabs(double16);
//half  __attribute__((const)) __attribute__((overloadable)) fabs(half);
//half2  __attribute__((const)) __attribute__((overloadable)) fabs(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) fabs(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) fabs(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) fabs(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) fabs(half16);

/**
 * x - y if x > y, +0 if x is less than or equal to y.
 */
double  __attribute__((const)) __attribute__((overloadable)) fdim(double x, double y);
double2  __attribute__((const)) __attribute__((overloadable)) fdim(double2 x, double2 y);
double3  __attribute__((const)) __attribute__((overloadable)) fdim(double3 x, double3 y);
double4  __attribute__((const)) __attribute__((overloadable)) fdim(double4 x, double4 y);
double8  __attribute__((const)) __attribute__((overloadable)) fdim(double8 x, double8 y);
double16  __attribute__((const)) __attribute__((overloadable)) fdim(double16 x, double16 y);
//half  __attribute__((const)) __attribute__((overloadable)) fdim(half x, half y);
//half2  __attribute__((const)) __attribute__((overloadable)) fdim(half2 x, half2 y);
//half3  __attribute__((const)) __attribute__((overloadable)) fdim(half3 x, half3 y);
//half4  __attribute__((const)) __attribute__((overloadable)) fdim(half4 x, half4 y);
//half8  __attribute__((const)) __attribute__((overloadable)) fdim(half8 x, half8 y);
//half16  __attribute__((const)) __attribute__((overloadable)) fdim(half16 x, half16 y);

/**
 * Round to integral value using the round to –ve
 * infinity rounding mode.
 */
double  __attribute__((const)) __attribute__((overloadable)) floor(double);
double2  __attribute__((const)) __attribute__((overloadable)) floor(double2);
double3  __attribute__((const)) __attribute__((overloadable)) floor(double3);
double4  __attribute__((const)) __attribute__((overloadable)) floor(double4);
double8  __attribute__((const)) __attribute__((overloadable)) floor(double8);
double16  __attribute__((const)) __attribute__((overloadable)) floor(double16);
//half  __attribute__((const)) __attribute__((overloadable)) floor(half);
//half2  __attribute__((const)) __attribute__((overloadable)) floor(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) floor(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) floor(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) floor(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) floor(half16);

/**
 * Returns the correctly rounded floating-point
 * representation of the sum of c with the infinitely
 * precise product of a and b. Rounding of
 * intermediate products shall not occur. Edge case
 * behavior is per the IEEE 754-2008 standard.
 */
double  __attribute__((const)) __attribute__((overloadable)) fma(double a, double b, double c);
double2  __attribute__((const)) __attribute__((overloadable)) fma(double2 a, double2 b, double2 c);
double3  __attribute__((const)) __attribute__((overloadable)) fma(double3 a, double3 b, double3 c);
double4  __attribute__((const)) __attribute__((overloadable)) fma(double4 a, double4 b, double4 c);
double8  __attribute__((const)) __attribute__((overloadable)) fma(double8 a, double8 b, double8 c);
double16  __attribute__((const)) __attribute__((overloadable)) fma(double16 a, double16 b, double16 c);
//half  __attribute__((const)) __attribute__((overloadable)) fma(half a, half b, half c);
//half2  __attribute__((const)) __attribute__((overloadable)) fma(half2 a, half2 b, half2 c);
//half3  __attribute__((const)) __attribute__((overloadable)) fma(half3 a, half3 b, half3 c);
//half4  __attribute__((const)) __attribute__((overloadable)) fma(half4 a, half4 b, half4 c);
//half8  __attribute__((const)) __attribute__((overloadable)) fma(half8 a, half8 b, half8 c);
//half16  __attribute__((const)) __attribute__((overloadable)) fma(half16 a, half16 b, half16 c);

/**
 * Returns y if x < y, otherwise it returns x. If one
 * argument is a NaN, fmax() returns the other
 * argument. If both arguments are NaNs, fmax()
 * returns a NaN.
 */
double  __attribute__((const)) __attribute__((overloadable)) fmax(double x, double y);
double2  __attribute__((const)) __attribute__((overloadable)) fmax(double2 x, double2 y);
double3  __attribute__((const)) __attribute__((overloadable)) fmax(double3 x, double3 y);
double4  __attribute__((const)) __attribute__((overloadable)) fmax(double4 x, double4 y);
double8  __attribute__((const)) __attribute__((overloadable)) fmax(double8 x, double8 y);
double16  __attribute__((const)) __attribute__((overloadable)) fmax(double16 x, double16 y);
double2  __attribute__((const)) __attribute__((overloadable)) fmax(double2 x, double y);
double3  __attribute__((const)) __attribute__((overloadable)) fmax(double3 x, double y);
double4  __attribute__((const)) __attribute__((overloadable)) fmax(double4 x, double y);
double8  __attribute__((const)) __attribute__((overloadable)) fmax(double8 x, double y);
double16  __attribute__((const)) __attribute__((overloadable)) fmax(double16 x, double y);
//half  __attribute__((const)) __attribute__((overloadable)) fmax(half x, half y);
//half2  __attribute__((const)) __attribute__((overloadable)) fmax(half2 x, half2 y);
//half3  __attribute__((const)) __attribute__((overloadable)) fmax(half3 x, half3 y);
//half4  __attribute__((const)) __attribute__((overloadable)) fmax(half4 x, half4 y);
//half8  __attribute__((const)) __attribute__((overloadable)) fmax(half8 x, half8 y);
//half16  __attribute__((const)) __attribute__((overloadable)) fmax(half16 x, half16 y);
//half2  __attribute__((const)) __attribute__((overloadable)) fmax(half2 x, half y);
//half3  __attribute__((const)) __attribute__((overloadable)) fmax(half3 x, half y);
//half4  __attribute__((const)) __attribute__((overloadable)) fmax(half4 x, half y);
//half8  __attribute__((const)) __attribute__((overloadable)) fmax(half8 x, half y);
//half16  __attribute__((const)) __attribute__((overloadable)) fmax(half16 x, half y);

/**
 * Returns y if y < x, otherwise it returns x. If one
 * argument is a NaN, fmin() returns the other
 * argument. If both arguments are NaNs, fmin()
 * returns a NaN.
 */
double2  __attribute__((const)) __attribute__((overloadable)) fmin(double2 x, double y);
double3  __attribute__((const)) __attribute__((overloadable)) fmin(double3 x, double y);
double4  __attribute__((const)) __attribute__((overloadable)) fmin(double4 x, double y);
double8  __attribute__((const)) __attribute__((overloadable)) fmin(double8 x, double y);
double16  __attribute__((const)) __attribute__((overloadable)) fmin(double16 x, double y);
double __attribute__((const)) __attribute__((overloadable)) fmin(double x, double y);
double2 __attribute__((const)) __attribute__((overloadable)) fmin(double2 x, double2 y);
double3 __attribute__((const)) __attribute__((overloadable)) fmin(double3 x, double3 y);
double4 __attribute__((const)) __attribute__((overloadable)) fmin(double4 x, double4 y);
double8 __attribute__((const)) __attribute__((overloadable)) fmin(double8 x, double8 y);
double16 __attribute__((const)) __attribute__((overloadable)) fmin(double16 x, double16 y);
//half const_func __attribute__((overloadable)) fmin(half x, half y);
//half2 const_func __attribute__((overloadable)) fmin(half2 x, half2 y);
//half3 const_func __attribute__((overloadable)) fmin(half3 x, half3 y);
//half4 const_func __attribute__((overloadable)) fmin(half4 x, half4 y);
//half8 const_func __attribute__((overloadable)) fmin(half8 x, half8 y);
//half16 const_func __attribute__((overloadable)) fmin(half16 x, half16 y);
//half2 const_func __attribute__((overloadable)) fmin(half2 x, half y);
//half3 const_func __attribute__((overloadable)) fmin(half3 x, half y);
//half4 const_func __attribute__((overloadable)) fmin(half4 x, half y);
//half8 const_func __attribute__((overloadable)) fmin(half8 x, half y);
//half16 const_func __attribute__((overloadable)) fmin(half16 x, half y);


/**
 * Modulus. Returns x – y * trunc (x/y).
 */
double  __attribute__((const)) __attribute__((overloadable)) fmod(double x, double y);
double2  __attribute__((const)) __attribute__((overloadable)) fmod(double2 x, double2 y);
double3  __attribute__((const)) __attribute__((overloadable)) fmod(double3 x, double3 y);
double4  __attribute__((const)) __attribute__((overloadable)) fmod(double4 x, double4 y);
double8  __attribute__((const)) __attribute__((overloadable)) fmod(double8 x, double8 y);
double16  __attribute__((const)) __attribute__((overloadable)) fmod(double16 x, double16 y);
//half  __attribute__((const)) __attribute__((overloadable)) fmod(half x, half y);
//half2  __attribute__((const)) __attribute__((overloadable)) fmod(half2 x, half2 y);
//half3  __attribute__((const)) __attribute__((overloadable)) fmod(half3 x, half3 y);
//half4  __attribute__((const)) __attribute__((overloadable)) fmod(half4 x, half4 y);
//half8  __attribute__((const)) __attribute__((overloadable)) fmod(half8 x, half8 y);
//half16  __attribute__((const)) __attribute__((overloadable)) fmod(half16 x, half16 y);

/**
 * Returns fmin( x – floor (x), 0x1.fffffep-1f ).
 * floor(x) is returned in iptr.
 */

/**
 * Compute the value of the square root of x^2+ y^2
 * without undue overflow or underflow.
 */
double  __attribute__((const)) __attribute__((overloadable)) hypot(double x, double y);
double2  __attribute__((const)) __attribute__((overloadable)) hypot(double2 x, double2 y);
double3  __attribute__((const)) __attribute__((overloadable)) hypot(double3 x, double3 y);
double4  __attribute__((const)) __attribute__((overloadable)) hypot(double4 x, double4 y);
double8  __attribute__((const)) __attribute__((overloadable)) hypot(double8 x, double8 y);
double16  __attribute__((const)) __attribute__((overloadable)) hypot(double16 x, double16 y);
//half  __attribute__((const)) __attribute__((overloadable)) hypot(half x, half y);
//half2  __attribute__((const)) __attribute__((overloadable)) hypot(half2 x, half2 y);
//half3  __attribute__((const)) __attribute__((overloadable)) hypot(half3 x, half3 y);
//half4  __attribute__((const)) __attribute__((overloadable)) hypot(half4 x, half4 y);
//half8  __attribute__((const)) __attribute__((overloadable)) hypot(half8 x, half8 y);
//half16  __attribute__((const)) __attribute__((overloadable)) hypot(half16 x, half16 y);

/**
 * Return the exponent as an integer value.
 */
int  __attribute__((const)) __attribute__((overloadable)) ilogb(double x);
int2  __attribute__((const)) __attribute__((overloadable)) ilogb(double2 x);
int3  __attribute__((const)) __attribute__((overloadable)) ilogb(double3 x);
int4  __attribute__((const)) __attribute__((overloadable)) ilogb(double4 x);
int8  __attribute__((const)) __attribute__((overloadable)) ilogb(double8 x);
int16  __attribute__((const)) __attribute__((overloadable)) ilogb(double16 x);
//int  __attribute__((const)) __attribute__((overloadable)) ilogb(half x);
//int2  __attribute__((const)) __attribute__((overloadable)) ilogb(half2 x);
//int3  __attribute__((const)) __attribute__((overloadable)) ilogb(half3 x);
//int4  __attribute__((const)) __attribute__((overloadable)) ilogb(half4 x);
//int8  __attribute__((const)) __attribute__((overloadable)) ilogb(half8 x);
//int16  __attribute__((const)) __attribute__((overloadable)) ilogb(half16 x);

/**
 * Multiply x by 2 to the power n.
 */
double  __attribute__((const)) __attribute__((overloadable)) ldexp(double x, int n);
double2  __attribute__((const)) __attribute__((overloadable)) ldexp(double2 x, int2 n);
double3  __attribute__((const)) __attribute__((overloadable)) ldexp(double3 x, int3 n);
double4  __attribute__((const)) __attribute__((overloadable)) ldexp(double4 x, int4 n);
double8  __attribute__((const)) __attribute__((overloadable)) ldexp(double8 x, int8 n);
double16  __attribute__((const)) __attribute__((overloadable)) ldexp(double16 x, int16 n);
double2  __attribute__((const)) __attribute__((overloadable)) ldexp(double2 x, int n);
double3  __attribute__((const)) __attribute__((overloadable)) ldexp(double3 x, int n);
double4  __attribute__((const)) __attribute__((overloadable)) ldexp(double4 x, int n);
double8  __attribute__((const)) __attribute__((overloadable)) ldexp(double8 x, int n);
double16  __attribute__((const)) __attribute__((overloadable)) ldexp(double16 x, int n);
//half  __attribute__((const)) __attribute__((overloadable)) ldexp(half x, int n);
//half2  __attribute__((const)) __attribute__((overloadable)) ldexp(half2 x, int2 n);
//half3  __attribute__((const)) __attribute__((overloadable)) ldexp(half3 x, int3 n);
//half4  __attribute__((const)) __attribute__((overloadable)) ldexp(half4 x, int4 n);
//half8  __attribute__((const)) __attribute__((overloadable)) ldexp(half8 x, int8 n);
//half16  __attribute__((const)) __attribute__((overloadable)) ldexp(half16 x, int16 n);
//half  __attribute__((const)) __attribute__((overloadable)) ldexp(half x, int n);
//half2  __attribute__((const)) __attribute__((overloadable)) ldexp(half2 x, int n);
//half3  __attribute__((const)) __attribute__((overloadable)) ldexp(half3 x, int n);
//half4  __attribute__((const)) __attribute__((overloadable)) ldexp(half4 x, int n);
//half8  __attribute__((const)) __attribute__((overloadable)) ldexp(half8 x, int n);
//half16  __attribute__((const)) __attribute__((overloadable)) ldexp(half16 x, int n);

/**
 * Log gamma function. Returns the natural
 * logarithm of the absolute value of the gamma
 * function. The sign of the gamma function is
 * returned in the signp argument of lgamma_r.
 */
double  __attribute__((const)) __attribute__((overloadable)) lgamma(double x);
double2  __attribute__((const)) __attribute__((overloadable)) lgamma(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) lgamma(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) lgamma(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) lgamma(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) lgamma(double16 x);

//half  __attribute__((const)) __attribute__((overloadable)) lgamma(half x);
//half2  __attribute__((const)) __attribute__((overloadable)) lgamma(half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) lgamma(half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) lgamma(half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) lgamma(half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) lgamma(half16 x);
//half __attribute__((const)) __attribute__((overloadable)) lgamma_r(half x, __global int *signp);
//half2 __attribute__((const)) __attribute__((overloadable)) lgamma_r(half2 x, __global int2 *signp);
//half3 __attribute__((const)) __attribute__((overloadable)) lgamma_r(half3 x, __global int3 *signp);
//half4 __attribute__((const)) __attribute__((overloadable)) lgamma_r(half4 x, __global int4 *signp);
//half8 __attribute__((const)) __attribute__((overloadable)) lgamma_r(half8 x, __global int8 *signp);
//half16 __attribute__((const)) __attribute__((overloadable)) lgamma_r(half16 x, __global int16 *signp);
//half __attribute__((const)) __attribute__((overloadable)) lgamma_r(half x, __local int *signp);
//half2 __attribute__((const)) __attribute__((overloadable)) lgamma_r(half2 x, __local int2 *signp);
//half3 __attribute__((const)) __attribute__((overloadable)) lgamma_r(half3 x, __local int3 *signp);
//half4 __attribute__((const)) __attribute__((overloadable)) lgamma_r(half4 x, __local int4 *signp);
//half8 __attribute__((const)) __attribute__((overloadable)) lgamma_r(half8 x, __local int8 *signp);
//half16 __attribute__((const)) __attribute__((overloadable)) lgamma_r(half16 x, __local int16 *signp);
//half __attribute__((const)) __attribute__((overloadable)) lgamma_r(half x, __private int *signp);
//half2 __attribute__((const)) __attribute__((overloadable)) lgamma_r(half2 x, __private int2 *signp);
//half3 __attribute__((const)) __attribute__((overloadable)) lgamma_r(half3 x, __private int3 *signp);
//half4 __attribute__((const)) __attribute__((overloadable)) lgamma_r(half4 x, __private int4 *signp);
//half8 __attribute__((const)) __attribute__((overloadable)) lgamma_r(half8 x, __private int8 *signp);
//half16 __attribute__((const)) __attribute__((overloadable)) lgamma_r(half16 x, __private int16 *signp);

/**
 * Compute natural logarithm.
 */
double  __attribute__((const)) __attribute__((overloadable)) log(double);
double2  __attribute__((const)) __attribute__((overloadable)) log(double2);
double3  __attribute__((const)) __attribute__((overloadable)) log(double3);
double4  __attribute__((const)) __attribute__((overloadable)) log(double4);
double8  __attribute__((const)) __attribute__((overloadable)) log(double8);
double16  __attribute__((const)) __attribute__((overloadable)) log(double16);
//half  __attribute__((const)) __attribute__((overloadable)) log(half);
//half2  __attribute__((const)) __attribute__((overloadable)) log(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) log(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) log(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) log(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) log(half16);

/**
 * Compute a base 2 logarithm.
 */
double  __attribute__((const)) __attribute__((overloadable)) log2(double);
double2  __attribute__((const)) __attribute__((overloadable)) log2(double2);
double3  __attribute__((const)) __attribute__((overloadable)) log2(double3);
double4  __attribute__((const)) __attribute__((overloadable)) log2(double4);
double8  __attribute__((const)) __attribute__((overloadable)) log2(double8);
double16  __attribute__((const)) __attribute__((overloadable)) log2(double16);
//half  __attribute__((const)) __attribute__((overloadable)) log2(half);
//half2  __attribute__((const)) __attribute__((overloadable)) log2(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) log2(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) log2(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) log2(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) log2(half16);

/**
 * Compute a base 10 logarithm.
 */
double  __attribute__((const)) __attribute__((overloadable)) log10(double);
double2  __attribute__((const)) __attribute__((overloadable)) log10(double2);
double3  __attribute__((const)) __attribute__((overloadable)) log10(double3);
double4  __attribute__((const)) __attribute__((overloadable)) log10(double4);
double8  __attribute__((const)) __attribute__((overloadable)) log10(double8);
double16  __attribute__((const)) __attribute__((overloadable)) log10(double16);
//half  __attribute__((const)) __attribute__((overloadable)) log10(half);
//half2  __attribute__((const)) __attribute__((overloadable)) log10(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) log10(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) log10(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) log10(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) log10(half16);

/**
 * Compute a base e logarithm of (1.0 + x).
 */
float16  __attribute__((const)) __attribute__((overloadable)) log1p(float16 x);
double  __attribute__((const)) __attribute__((overloadable)) log1p(double x);
double2  __attribute__((const)) __attribute__((overloadable)) log1p(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) log1p(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) log1p(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) log1p(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) log1p(double16 x);
//half  __attribute__((const)) __attribute__((overloadable)) log1p(half x);
//half2  __attribute__((const)) __attribute__((overloadable)) log1p(half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) log1p(half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) log1p(half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) log1p(half8 x);
//half16  __attribute__((overloadable)) log1p(half16 x);

/**
 * Compute the exponent of x, which is the integral
 * part of logr | x |.
 */
double  __attribute__((const)) __attribute__((overloadable)) logb(double x);
double2  __attribute__((const)) __attribute__((overloadable)) logb(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) logb(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) logb(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) logb(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) logb(double16 x);
//half  __attribute__((const)) __attribute__((overloadable)) logb(half x);
//half2  __attribute__((const)) __attribute__((overloadable)) logb(half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) logb(half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) logb(half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) logb(half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) logb(half16 x);

/**
 * mad approximates a * b + c. Whether or how the
 * product of a * b is rounded and how supernormal or
 * subnormal intermediate products are handled is not
 * defined. mad is intended to be used where speed is
 * preferred over accuracy.
 */
double  __attribute__((const)) __attribute__((overloadable)) mad(double a, double b, double c);
double2  __attribute__((const)) __attribute__((overloadable)) mad(double2 a, double2 b, double2 c);
double3  __attribute__((const)) __attribute__((overloadable)) mad(double3 a, double3 b, double3 c);
double4  __attribute__((const)) __attribute__((overloadable)) mad(double4 a, double4 b, double4 c);
double8  __attribute__((const)) __attribute__((overloadable)) mad(double8 a, double8 b, double8 c);
double16  __attribute__((const)) __attribute__((overloadable)) mad(double16 a, double16 b, double16 c);
//half  __attribute__((const)) __attribute__((overloadable)) mad(half a, half b, half c);
//half2  __attribute__((const)) __attribute__((overloadable)) mad(half2 a, half2 b, half2 c);
//half3  __attribute__((const)) __attribute__((overloadable)) mad(half3 a, half3 b, half3 c);
//half4  __attribute__((const)) __attribute__((overloadable)) mad(half4 a, half4 b, half4 c);
//half8  __attribute__((const)) __attribute__((overloadable)) mad(half8 a, half8 b, half8 c);
//half16  __attribute__((const)) __attribute__((overloadable)) mad(half16 a, half16 b, half16 c);

/**
 * Returns x if | x | > | y |, y if | y | > | x |, otherwise
 * fmax(x, y).
 */
double  __attribute__((const)) __attribute__((overloadable)) maxmag(double x, double y);
double2  __attribute__((const)) __attribute__((overloadable)) maxmag(double2 x, double2 y);
double3  __attribute__((const)) __attribute__((overloadable)) maxmag(double3 x, double3 y);
double4  __attribute__((const)) __attribute__((overloadable)) maxmag(double4 x, double4 y);
double8  __attribute__((const)) __attribute__((overloadable)) maxmag(double8 x, double8 y);
double16  __attribute__((const)) __attribute__((overloadable)) maxmag(double16 x, double16 y);
//half  __attribute__((const)) __attribute__((overloadable)) maxmag(half x, half y);
//half2  __attribute__((const)) __attribute__((overloadable)) maxmag(half2 x, half2 y);
//half3  __attribute__((const)) __attribute__((overloadable)) maxmag(half3 x, half3 y);
//half4  __attribute__((const)) __attribute__((overloadable)) maxmag(half4 x, half4 y);
//half8  __attribute__((const)) __attribute__((overloadable)) maxmag(half8 x, half8 y);
//half16  __attribute__((const)) __attribute__((overloadable)) maxmag(half16 x, half16 y);

/**
 * Returns x if | x | < | y |, y if | y | < | x |, otherwise
 * fmin(x, y).
 */
double  __attribute__((const)) __attribute__((overloadable)) minmag(double x, double y);
double2  __attribute__((const)) __attribute__((overloadable)) minmag(double2 x, double2 y);
double3  __attribute__((const)) __attribute__((overloadable)) minmag(double3 x, double3 y);
double4  __attribute__((const)) __attribute__((overloadable)) minmag(double4 x, double4 y);
double8  __attribute__((const)) __attribute__((overloadable)) minmag(double8 x, double8 y);
double16  __attribute__((const)) __attribute__((overloadable)) minmag(double16 x, double16 y);
//half  __attribute__((const)) __attribute__((overloadable)) minmag(half x, half y);
//half2  __attribute__((const)) __attribute__((overloadable)) minmag(half2 x, half2 y);
//half3  __attribute__((const)) __attribute__((overloadable)) minmag(half3 x, half3 y);
//half4  __attribute__((const)) __attribute__((overloadable)) minmag(half4 x, half4 y);
//half8  __attribute__((const)) __attribute__((overloadable)) minmag(half8 x, half8 y);
//half16  __attribute__((const)) __attribute__((overloadable)) minmag(half16 x, half16 y);

/**
 * Decompose a floating-point number. The modf
 * function breaks the argument x into integral and
 * fractional parts, each of which has the same sign as
 * the argument. It stores the integral part in the object
 * pointed to by iptr.
 */

/**
 * Returns a quiet NaN. The nancode may be placed
 * in the significand of the resulting NaN.
 */
double  __attribute__((const)) __attribute__((overloadable)) nan(ulong nancode);
double2  __attribute__((const)) __attribute__((overloadable)) nan(ulong2 nancode);
double3  __attribute__((const)) __attribute__((overloadable)) nan(ulong3 nancode);
double4  __attribute__((const)) __attribute__((overloadable)) nan(ulong4 nancode);
double8  __attribute__((const)) __attribute__((overloadable)) nan(ulong8 nancode);
double16  __attribute__((const)) __attribute__((overloadable)) nan(ulong16 nancode);
//half  __attribute__((const)) __attribute__((overloadable)) nan(ushort nancode);
//half2  __attribute__((const)) __attribute__((overloadable)) nan(ushort2 nancode);
//half3  __attribute__((const)) __attribute__((overloadable)) nan(ushort3 nancode);
//half4  __attribute__((const)) __attribute__((overloadable)) nan(ushort4 nancode);
//half8  __attribute__((const)) __attribute__((overloadable)) nan(ushort8 nancode);
//half16  __attribute__((const)) __attribute__((overloadable)) nan(ushort16 nancode);

/**
 * Computes the next representable single-precision
 * floating-point value following x in the direction of
 * y. Thus, if y is less than x, nextafter() returns the
 * largest representable floating-point number less
 * than x.
 */
double  __attribute__((const)) __attribute__((overloadable)) nextafter(double x, double y);
double2  __attribute__((const)) __attribute__((overloadable)) nextafter(double2 x, double2 y);
double3  __attribute__((const)) __attribute__((overloadable)) nextafter(double3 x, double3 y);
double4  __attribute__((const)) __attribute__((overloadable)) nextafter(double4 x, double4 y);
double8  __attribute__((const)) __attribute__((overloadable)) nextafter(double8 x, double8 y);
double16  __attribute__((const)) __attribute__((overloadable)) nextafter(double16 x, double16 y);
//half  __attribute__((const)) __attribute__((overloadable)) nextafter(half x, half y);
//half2  __attribute__((const)) __attribute__((overloadable)) nextafter(half2 x, half2 y);
//half3  __attribute__((const)) __attribute__((overloadable)) nextafter(half3 x, half3 y);
//half4  __attribute__((const)) __attribute__((overloadable)) nextafter(half4 x, half4 y);
//half8  __attribute__((const)) __attribute__((overloadable)) nextafter(half8 x, half8 y);
//half16  __attribute__((const)) __attribute__((overloadable)) nextafter(half16 x, half16 y);

/**
 * Compute x to the power y.
 */
double  __attribute__((const)) __attribute__((overloadable)) pow(double x, double y);
double2  __attribute__((const)) __attribute__((overloadable)) pow(double2 x, double2 y);
double3  __attribute__((const)) __attribute__((overloadable)) pow(double3 x, double3 y);
double4  __attribute__((const)) __attribute__((overloadable)) pow(double4 x, double4 y);
double8  __attribute__((const)) __attribute__((overloadable)) pow(double8 x, double8 y);
double16  __attribute__((const)) __attribute__((overloadable)) pow(double16 x, double16 y);
//half  __attribute__((const)) __attribute__((overloadable)) pow(half x, half y);
//half2  __attribute__((const)) __attribute__((overloadable)) pow(half2 x, half2 y);
//half3  __attribute__((const)) __attribute__((overloadable)) pow(half3 x, half3 y);
//half4  __attribute__((const)) __attribute__((overloadable)) pow(half4 x, half4 y);
//half8  __attribute__((const)) __attribute__((overloadable)) pow(half8 x, half8 y);
//half16  __attribute__((const)) __attribute__((overloadable)) pow(half16 x, half16 y);

/**
 * Compute x to the power y, where y is an integer.
 */
double  __attribute__((const)) __attribute__((overloadable)) pown(double x, int y);
double2  __attribute__((const)) __attribute__((overloadable)) pown(double2 x, int2 y);
double3  __attribute__((const)) __attribute__((overloadable)) pown(double3 x, int3 y);
double4  __attribute__((const)) __attribute__((overloadable)) pown(double4 x, int4 y);
double8  __attribute__((const)) __attribute__((overloadable)) pown(double8 x, int8 y);
double16  __attribute__((const)) __attribute__((overloadable)) pown(double16 x, int16 y);
//half  __attribute__((const)) __attribute__((overloadable)) pown(half x, int y);
//half2  __attribute__((const)) __attribute__((overloadable)) pown(half2 x, int2 y);
//half3  __attribute__((const)) __attribute__((overloadable)) pown(half3 x, int3 y);
//half4  __attribute__((const)) __attribute__((overloadable)) pown(half4 x, int4 y);
//half8  __attribute__((const)) __attribute__((overloadable)) pown(half8 x, int8 y);
//half16  __attribute__((const)) __attribute__((overloadable)) pown(half16 x, int16 y);

/**
 * Compute x to the power y, where x is >= 0.
 */
double  __attribute__((const)) __attribute__((overloadable)) powr(double x, double y);
double2  __attribute__((const)) __attribute__((overloadable)) powr(double2 x, double2 y);
double3  __attribute__((const)) __attribute__((overloadable)) powr(double3 x, double3 y);
double4  __attribute__((const)) __attribute__((overloadable)) powr(double4 x, double4 y);
double8  __attribute__((const)) __attribute__((overloadable)) powr(double8 x, double8 y);
double16  __attribute__((const)) __attribute__((overloadable)) powr(double16 x, double16 y);
//half  __attribute__((const)) __attribute__((overloadable)) powr(half x, half y);
//half2  __attribute__((const)) __attribute__((overloadable)) powr(half2 x, half2 y);
//half3  __attribute__((const)) __attribute__((overloadable)) powr(half3 x, half3 y);
//half4  __attribute__((const)) __attribute__((overloadable)) powr(half4 x, half4 y);
//half8  __attribute__((const)) __attribute__((overloadable)) powr(half8 x, half8 y);
//half16  __attribute__((const)) __attribute__((overloadable)) powr(half16 x, half16 y);

/**
 * Compute the value r such that r = x - n*y, where n
 * is the integer nearest the exact value of x/y. If there
 * are two integers closest to x/y, n shall be the even
 * one. If r is zero, it is given the same sign as x.
 */
double  __attribute__((const)) __attribute__((overloadable)) remainder(double x, double y);
double2  __attribute__((const)) __attribute__((overloadable)) remainder(double2 x, double2 y);
double3  __attribute__((const)) __attribute__((overloadable)) remainder(double3 x, double3 y);
double4  __attribute__((const)) __attribute__((overloadable)) remainder(double4 x, double4 y);
double8  __attribute__((const)) __attribute__((overloadable)) remainder(double8 x, double8 y);
double16  __attribute__((const)) __attribute__((overloadable)) remainder(double16 x, double16 y);
//half  __attribute__((const)) __attribute__((overloadable)) remainder(half x, half y);
//half2  __attribute__((const)) __attribute__((overloadable)) remainder(half2 x, half2 y);
//half3  __attribute__((const)) __attribute__((overloadable)) remainder(half3 x, half3 y);
//half4  __attribute__((const)) __attribute__((overloadable)) remainder(half4 x, half4 y);
//half8  __attribute__((const)) __attribute__((overloadable)) remainder(half8 x, half8 y);
//half16  __attribute__((const)) __attribute__((overloadable)) remainder(half16 x, half16 y);

/**
 * The remquo function computes the value r such
 * that r = x - n*y, where n is the integer nearest the
 * exact value of x/y. If there are two integers closest
 * to x/y, n shall be the even one. If r is zero, it is
 * given the same sign as x. This is the same value
 * that is returned by the remainder function.
 * remquo also calculates the lower seven bits of the
 * integral quotient x/y, and gives that value the same
 * sign as x/y. It stores this signed value in the object
 * pointed to by quo.
 */

/**
 * Round to integral value (using round to nearest
 * even rounding mode) in floating-point format.
 * Refer to section 7.1 for description of rounding
 * modes.
 */
double  __attribute__((const)) __attribute__((overloadable)) rint(double);
double2  __attribute__((const)) __attribute__((overloadable)) rint(double2);
double3  __attribute__((const)) __attribute__((overloadable)) rint(double3);
double4  __attribute__((const)) __attribute__((overloadable)) rint(double4);
double8  __attribute__((const)) __attribute__((overloadable)) rint(double8);
double16  __attribute__((const)) __attribute__((overloadable)) rint(double16);
//half  __attribute__((const)) __attribute__((overloadable)) rint(half);
//half2  __attribute__((const)) __attribute__((overloadable)) rint(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) rint(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) rint(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) rint(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) rint(half16);

/**
 * Compute x to the power 1/y.
 */
double  __attribute__((const)) __attribute__((overloadable)) rootn(double x, int y);
double2  __attribute__((const)) __attribute__((overloadable)) rootn(double2 x, int2 y);
double3  __attribute__((const)) __attribute__((overloadable)) rootn(double3 x, int3 y);
double4  __attribute__((const)) __attribute__((overloadable)) rootn(double4 x, int4 y);
double8  __attribute__((const)) __attribute__((overloadable)) rootn(double8 x, int8 y);
double16  __attribute__((const)) __attribute__((overloadable)) rootn(double16 x, int16 y);
//half  __attribute__((const)) __attribute__((overloadable)) rootn(half x, int y);
//half2  __attribute__((const)) __attribute__((overloadable)) rootn(half2 x, int2 y);
//half3  __attribute__((const)) __attribute__((overloadable)) rootn(half3 x, int3 y);
//half4  __attribute__((const)) __attribute__((overloadable)) rootn(half4 x, int4 y);
//half8  __attribute__((const)) __attribute__((overloadable)) rootn(half8 x, int8 y);
//half16  __attribute__((const)) __attribute__((overloadable)) rootn(half16 x, int16 y);

/**
 * Return the integral value nearest to x rounding
 * halfway cases away from zero, regardless of the
 * current rounding direction.
 */
double  __attribute__((const)) __attribute__((overloadable)) round(double x);
double2  __attribute__((const)) __attribute__((overloadable)) round(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) round(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) round(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) round(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) round(double16 x);
//half  __attribute__((const)) __attribute__((overloadable)) round(half x);
//half2  __attribute__((const)) __attribute__((overloadable)) round(half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) round(half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) round(half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) round(half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) round(half16 x);

/**
 * Compute inverse square root.
 */
double  __attribute__((const)) __attribute__((overloadable)) rsqrt(double);
double2  __attribute__((const)) __attribute__((overloadable)) rsqrt(double2);
double3  __attribute__((const)) __attribute__((overloadable)) rsqrt(double3);
double4  __attribute__((const)) __attribute__((overloadable)) rsqrt(double4);
double8  __attribute__((const)) __attribute__((overloadable)) rsqrt(double8);
double16  __attribute__((const)) __attribute__((overloadable)) rsqrt(double16);
//half  __attribute__((const)) __attribute__((overloadable)) rsqrt(half);
//half2  __attribute__((const)) __attribute__((overloadable)) rsqrt(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) rsqrt(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) rsqrt(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) rsqrt(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) rsqrt(half16);

/**
 * Compute sine.
 */
double  __attribute__((const)) __attribute__((overloadable)) sin(double);
double2  __attribute__((const)) __attribute__((overloadable)) sin(double2);
double3  __attribute__((const)) __attribute__((overloadable)) sin(double3);
double4  __attribute__((const)) __attribute__((overloadable)) sin(double4);
double8  __attribute__((const)) __attribute__((overloadable)) sin(double8);
double16  __attribute__((const)) __attribute__((overloadable)) sin(double16);
//half  __attribute__((const)) __attribute__((overloadable)) sin(half);
//half2  __attribute__((const)) __attribute__((overloadable)) sin(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) sin(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) sin(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) sin(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) sin(half16);


/**
 * Compute hyperbolic sine.
 */
double  __attribute__((const)) __attribute__((overloadable)) sinh(double);
double2  __attribute__((const)) __attribute__((overloadable)) sinh(double2);
double3  __attribute__((const)) __attribute__((overloadable)) sinh(double3);
double4  __attribute__((const)) __attribute__((overloadable)) sinh(double4);
double8  __attribute__((const)) __attribute__((overloadable)) sinh(double8);
double16  __attribute__((const)) __attribute__((overloadable)) sinh(double16);
//half  __attribute__((const)) __attribute__((overloadable)) sinh(half);
//half2  __attribute__((const)) __attribute__((overloadable)) sinh(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) sinh(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) sinh(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) sinh(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) sinh(half16);

/**
 * Compute sin (PI * x).
 */
double  __attribute__((const)) __attribute__((overloadable)) sinpi(double x);
double2  __attribute__((const)) __attribute__((overloadable)) sinpi(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) sinpi(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) sinpi(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) sinpi(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) sinpi(double16 x);
//half  __attribute__((const)) __attribute__((overloadable)) sinpi(half x);
//half2  __attribute__((const)) __attribute__((overloadable)) sinpi(half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) sinpi(half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) sinpi(half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) sinpi(half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) sinpi(half16 x);

/**
 * Compute square root.
 */
double  __attribute__((const)) __attribute__((overloadable)) sqrt(double);
double2  __attribute__((const)) __attribute__((overloadable)) sqrt(double2);
double3  __attribute__((const)) __attribute__((overloadable)) sqrt(double3);
double4  __attribute__((const)) __attribute__((overloadable)) sqrt(double4);
double8  __attribute__((const)) __attribute__((overloadable)) sqrt(double8);
double16  __attribute__((const)) __attribute__((overloadable)) sqrt(double16);
//half  __attribute__((const)) __attribute__((overloadable)) sqrt(half);
//half2  __attribute__((const)) __attribute__((overloadable)) sqrt(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) sqrt(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) sqrt(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) sqrt(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) sqrt(half16);

/**
 * Compute tangent.
 */
double  __attribute__((const)) __attribute__((overloadable)) tan(double);
double2  __attribute__((const)) __attribute__((overloadable)) tan(double2);
double3  __attribute__((const)) __attribute__((overloadable)) tan(double3);
double4  __attribute__((const)) __attribute__((overloadable)) tan(double4);
double8  __attribute__((const)) __attribute__((overloadable)) tan(double8);
double16  __attribute__((const)) __attribute__((overloadable)) tan(double16);
//half  __attribute__((const)) __attribute__((overloadable)) tan(half);
//half2  __attribute__((const)) __attribute__((overloadable)) tan(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) tan(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) tan(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) tan(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) tan(half16);

/**
 * Compute hyperbolic tangent.
 */
double  __attribute__((const)) __attribute__((overloadable)) tanh(double);
double2  __attribute__((const)) __attribute__((overloadable)) tanh(double2);
double3  __attribute__((const)) __attribute__((overloadable)) tanh(double3);
double4  __attribute__((const)) __attribute__((overloadable)) tanh(double4);
double8  __attribute__((const)) __attribute__((overloadable)) tanh(double8);
double16  __attribute__((const)) __attribute__((overloadable)) tanh(double16);
//half  __attribute__((const)) __attribute__((overloadable)) tanh(half);
//half2  __attribute__((const)) __attribute__((overloadable)) tanh(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) tanh(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) tanh(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) tanh(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) tanh(half16);

/**
 * Compute tan (PI * x).
 */
double  __attribute__((const)) __attribute__((overloadable)) tanpi(double x);
double2  __attribute__((const)) __attribute__((overloadable)) tanpi(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) tanpi(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) tanpi(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) tanpi(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) tanpi(double16 x);
//half  __attribute__((const)) __attribute__((overloadable)) tanpi(half x);
//half2  __attribute__((const)) __attribute__((overloadable)) tanpi(half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) tanpi(half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) tanpi(half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) tanpi(half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) tanpi(half16 x);

/**
 * Compute the gamma function.
 */
double  __attribute__((const)) __attribute__((overloadable)) tgamma(double);
double2  __attribute__((const)) __attribute__((overloadable)) tgamma(double2);
double3  __attribute__((const)) __attribute__((overloadable)) tgamma(double3);
double4  __attribute__((const)) __attribute__((overloadable)) tgamma(double4);
double8  __attribute__((const)) __attribute__((overloadable)) tgamma(double8);
double16  __attribute__((const)) __attribute__((overloadable)) tgamma(double16);
//half  __attribute__((const)) __attribute__((overloadable)) tgamma(half);
//half2  __attribute__((const)) __attribute__((overloadable)) tgamma(half2);
//half3  __attribute__((const)) __attribute__((overloadable)) tgamma(half3);
//half4  __attribute__((const)) __attribute__((overloadable)) tgamma(half4);
//half8  __attribute__((const)) __attribute__((overloadable)) tgamma(half8);
//half16  __attribute__((const)) __attribute__((overloadable)) tgamma(half16);

/**
 * Round to integral value using the round to zero
 * rounding mode.
 */
double  __attribute__((const)) __attribute__((overloadable)) trunc(double);
double2  __attribute__((const)) __attribute__((overloadable)) trunc(double2);
double3  __attribute__((const)) __attribute__((overloadable)) trunc(double3);
double4  __attribute__((const)) __attribute__((overloadable)) trunc(double4);
double8  __attribute__((const)) __attribute__((overloadable)) trunc(double8);
double16  __attribute__((const)) __attribute__((overloadable)) trunc(double16);

// EXTENSTION: native double
double  __attribute__((const)) __attribute__((overloadable)) native_cos(double x);
double2  __attribute__((const)) __attribute__((overloadable)) native_cos(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) native_cos(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) native_cos(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) native_cos(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) native_cos(double16 x);

/**
 * Compute x / y over an implementation-defined range.
 * The maximum error is implementation-defined.
 */

// EXTENSTION: native double
double  __attribute__((const)) __attribute__((overloadable)) native_divide(double x, double y);
double2  __attribute__((const)) __attribute__((overloadable)) native_divide(double2 x, double2 y);
double3  __attribute__((const)) __attribute__((overloadable)) native_divide(double3 x, double3 y);
double4  __attribute__((const)) __attribute__((overloadable)) native_divide(double4 x, double4 y);
double8  __attribute__((const)) __attribute__((overloadable)) native_divide(double8 x, double8 y);
double16  __attribute__((const)) __attribute__((overloadable)) native_divide(double16 x, double16 y);

/**
 * Compute the base- e exponential of x over an
 * implementation-defined range. The maximum error is
 * implementation-defined.
 */

// EXTENSTION: native double
double  __attribute__((const)) __attribute__((overloadable)) native_exp(double x);
double2  __attribute__((const)) __attribute__((overloadable)) native_exp(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) native_exp(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) native_exp(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) native_exp(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) native_exp(double16 x);

/**
 * Compute the base- 2 exponential of x over an
 * implementation-defined range. The maximum error is
 * implementation-defined.
 */

// EXTENSTION: native double
double  __attribute__((const)) __attribute__((overloadable)) native_exp2(double x);
double2  __attribute__((const)) __attribute__((overloadable)) native_exp2(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) native_exp2(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) native_exp2(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) native_exp2(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) native_exp2(double16 x);


/**
 * Compute the base- 10 exponential of x over an
 * implementation-defined range. The maximum error is
 * implementation-defined.
 */

// EXTENSTION: native double
double  __attribute__((const)) __attribute__((overloadable)) native_exp10(double x);
double2  __attribute__((const)) __attribute__((overloadable)) native_exp10(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) native_exp10(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) native_exp10(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) native_exp10(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) native_exp10(double16 x);


/**
 * Compute natural logarithm over an implementationdefined
 * range. The maximum error is implementation
 * defined.
 */

// EXTENSTION: native double
double  __attribute__((const)) __attribute__((overloadable)) native_log(double x);
double2  __attribute__((const)) __attribute__((overloadable)) native_log(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) native_log(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) native_log(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) native_log(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) native_log(double16 x);


/**
 * Compute a base 2 logarithm over an implementationdefined
 * range. The maximum error is implementationdefined.
 */

// EXTENSTION: native double
double  __attribute__((const)) __attribute__((overloadable)) native_log2(double x);
double2  __attribute__((const)) __attribute__((overloadable)) native_log2(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) native_log2(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) native_log2(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) native_log2(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) native_log2(double16 x);


/**
 * Compute a base 10 logarithm over an implementationdefined
 * range. The maximum error is implementationdefined.
 */

// EXTENSTION: native double
double  __attribute__((const)) __attribute__((overloadable)) native_log10(double x);
double2  __attribute__((const)) __attribute__((overloadable)) native_log10(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) native_log10(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) native_log10(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) native_log10(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) native_log10(double16 x);


/**
 * Compute x to the power y, where x is >= 0. The range of
 * x and y are implementation-defined. The maximum error
 * is implementation-defined.
 */

// EXTENSTION: native double
double  __attribute__((const)) __attribute__((overloadable)) native_powr(double x, double y);
double2  __attribute__((const)) __attribute__((overloadable)) native_powr(double2 x, double2 y);
double3  __attribute__((const)) __attribute__((overloadable)) native_powr(double3 x, double3 y);
double4  __attribute__((const)) __attribute__((overloadable)) native_powr(double4 x, double4 y);
double8  __attribute__((const)) __attribute__((overloadable)) native_powr(double8 x, double8 y);
double16  __attribute__((const)) __attribute__((overloadable)) native_powr(double16 x, double16 y);


/**
 * Compute reciprocal over an implementation-defined
 * range. The maximum error is implementation-defined.
 */

// EXTENSTION: native double
double  __attribute__((const)) __attribute__((overloadable)) native_recip(double x);
double2  __attribute__((const)) __attribute__((overloadable)) native_recip(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) native_recip(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) native_recip(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) native_recip(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) native_recip(double16 x);


/**
 * Compute inverse square root over an implementationdefined
 * range. The maximum error is implementationdefined.
 */

// EXTENSTION: native double
double  __attribute__((const)) __attribute__((overloadable)) native_rsqrt(double x);
double2  __attribute__((const)) __attribute__((overloadable)) native_rsqrt(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) native_rsqrt(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) native_rsqrt(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) native_rsqrt(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) native_rsqrt(double16 x);


/**
 * Compute sine over an implementation-defined range.
 * The maximum error is implementation-defined.
 */

// EXTENSTION: native double
double  __attribute__((const)) __attribute__((overloadable)) native_sin(double x);
double2  __attribute__((const)) __attribute__((overloadable)) native_sin(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) native_sin(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) native_sin(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) native_sin(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) native_sin(double16 x);


/**
 * Compute square root over an implementation-defined
 * range. The maximum error is implementation-defined.
 */

// EXTENSTION: native double
double  __attribute__((const)) __attribute__((overloadable)) native_sqrt(double x);
double2  __attribute__((const)) __attribute__((overloadable)) native_sqrt(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) native_sqrt(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) native_sqrt(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) native_sqrt(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) native_sqrt(double16 x);


/**
 * Compute tangent over an implementation-defined range.
 * The maximum error is implementation-defined.
 */

// EXTENSTION: native double
double  __attribute__((const)) __attribute__((overloadable)) native_tan(double x);
double2  __attribute__((const)) __attribute__((overloadable)) native_tan(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) native_tan(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) native_tan(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) native_tan(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) native_tan(double16 x);


// Integer functions:

/**
 * Returns | x |.
 */
double  __attribute__((const)) __attribute__((overloadable)) clamp(double x, double minval, double maxval);
double2  __attribute__((const)) __attribute__((overloadable)) clamp(double2 x, double2 minval, double2 maxval);
double3  __attribute__((const)) __attribute__((overloadable)) clamp(double3 x, double3 minval, double3 maxval);
double4  __attribute__((const)) __attribute__((overloadable)) clamp(double4 x, double4 minval, double4 maxval);
double8  __attribute__((const)) __attribute__((overloadable)) clamp(double8 x, double8 minval, double8 maxval);
double16  __attribute__((const)) __attribute__((overloadable)) clamp(double16 x, double16 minval, double16 maxval);
double2  __attribute__((const)) __attribute__((overloadable)) clamp(double2 x, double minval, double maxval);
double3  __attribute__((const)) __attribute__((overloadable)) clamp(double3 x, double minval, double maxval);
double4  __attribute__((const)) __attribute__((overloadable)) clamp(double4 x, double minval, double maxval);
double8  __attribute__((const)) __attribute__((overloadable)) clamp(double8 x, double minval, double maxval);
double16  __attribute__((const)) __attribute__((overloadable)) clamp(double16 x, double minval, double maxval);
//half  __attribute__((const)) __attribute__((overloadable)) clamp(half x, half minval, half maxval);
//half2  __attribute__((const)) __attribute__((overloadable)) clamp(half2 x, half2 minval, half2 maxval);
//half3  __attribute__((const)) __attribute__((overloadable)) clamp(half3 x, half3 minval, half3 maxval);
//half4  __attribute__((const)) __attribute__((overloadable)) clamp(half4 x, half4 minval, half4 maxval);
//half8  __attribute__((const)) __attribute__((overloadable)) clamp(half8 x, half8 minval, half8 maxval);
//half16  __attribute__((const)) __attribute__((overloadable)) clamp(half16 x, half16 minval, half16 maxval);
//half2  __attribute__((const)) __attribute__((overloadable)) clamp(half2 x, half minval, half maxval);
//half3  __attribute__((const)) __attribute__((overloadable)) clamp(half3 x, half minval, half maxval);
//half4  __attribute__((const)) __attribute__((overloadable)) clamp(half4 x, half minval, half maxval);
//half8  __attribute__((const)) __attribute__((overloadable)) clamp(half8 x, half minval, half maxval);
//half16  __attribute__((const)) __attribute__((overloadable)) clamp(half16 x, half minval, half maxval);

/**
 * Returns the number of leading 0-bits in x, starting
 * at the most significant bit position.
 */

/*
 * popcount(x): returns the number of set bit in x
 */
char  __attribute__((const)) __attribute__((overloadable)) popcount(char x);
uchar  __attribute__((const)) __attribute__((overloadable)) popcount(uchar x);
char2  __attribute__((const)) __attribute__((overloadable)) popcount(char2 x);
uchar2  __attribute__((const)) __attribute__((overloadable)) popcount(uchar2 x);
char3  __attribute__((const)) __attribute__((overloadable)) popcount(char3 x);
uchar3  __attribute__((const)) __attribute__((overloadable)) popcount(uchar3 x);
char4  __attribute__((const)) __attribute__((overloadable)) popcount(char4 x);
uchar4  __attribute__((const)) __attribute__((overloadable)) popcount(uchar4 x);
char8  __attribute__((const)) __attribute__((overloadable)) popcount(char8 x);
uchar8  __attribute__((const)) __attribute__((overloadable)) popcount(uchar8 x);
char16  __attribute__((const)) __attribute__((overloadable)) popcount(char16 x);
uchar16  __attribute__((const)) __attribute__((overloadable)) popcount(uchar16 x);
short  __attribute__((const)) __attribute__((overloadable)) popcount(short x);
ushort  __attribute__((const)) __attribute__((overloadable)) popcount(ushort x);
short2  __attribute__((const)) __attribute__((overloadable)) popcount(short2 x);
ushort2  __attribute__((const)) __attribute__((overloadable)) popcount(ushort2 x);
short3  __attribute__((const)) __attribute__((overloadable)) popcount(short3 x);
ushort3  __attribute__((const)) __attribute__((overloadable)) popcount(ushort3 x);
short4  __attribute__((const)) __attribute__((overloadable)) popcount(short4 x);
ushort4  __attribute__((const)) __attribute__((overloadable)) popcount(ushort4 x);
short8  __attribute__((const)) __attribute__((overloadable)) popcount(short8 x);
ushort8  __attribute__((const)) __attribute__((overloadable)) popcount(ushort8 x);
short16  __attribute__((const)) __attribute__((overloadable)) popcount(short16 x);
ushort16  __attribute__((const)) __attribute__((overloadable)) popcount(ushort16 x);
int  __attribute__((const)) __attribute__((overloadable)) popcount(int x);
uint  __attribute__((const)) __attribute__((overloadable)) popcount(uint x);
int2  __attribute__((const)) __attribute__((overloadable)) popcount(int2 x);
uint2  __attribute__((const)) __attribute__((overloadable)) popcount(uint2 x);
int3  __attribute__((const)) __attribute__((overloadable)) popcount(int3 x);
uint3  __attribute__((const)) __attribute__((overloadable)) popcount(uint3 x);
int4  __attribute__((const)) __attribute__((overloadable)) popcount(int4 x);
uint4  __attribute__((const)) __attribute__((overloadable)) popcount(uint4 x);
int8  __attribute__((const)) __attribute__((overloadable)) popcount(int8 x);
uint8  __attribute__((const)) __attribute__((overloadable)) popcount(uint8 x);
int16  __attribute__((const)) __attribute__((overloadable)) popcount(int16 x);
uint16  __attribute__((const)) __attribute__((overloadable)) popcount(uint16 x);
long  __attribute__((const)) __attribute__((overloadable)) popcount(long x);
ulong  __attribute__((const)) __attribute__((overloadable)) popcount(ulong x);
long2  __attribute__((const)) __attribute__((overloadable)) popcount(long2 x);
ulong2  __attribute__((const)) __attribute__((overloadable)) popcount(ulong2 x);
long3  __attribute__((const)) __attribute__((overloadable)) popcount(long3 x);
ulong3  __attribute__((const)) __attribute__((overloadable)) popcount(ulong3 x);
long4  __attribute__((const)) __attribute__((overloadable)) popcount(long4 x);
ulong4  __attribute__((const)) __attribute__((overloadable)) popcount(ulong4 x);
long8  __attribute__((const)) __attribute__((overloadable)) popcount(long8 x);
ulong8  __attribute__((const)) __attribute__((overloadable)) popcount(ulong8 x);
long16  __attribute__((const)) __attribute__((overloadable)) popcount(long16 x);
ulong16  __attribute__((const)) __attribute__((overloadable)) popcount(ulong16 x);

/**
 * Computes x * y and returns the high half of the
 * product of x and y.
 */
double  __attribute__((const)) __attribute__((overloadable)) degrees(double radians);
double2  __attribute__((const)) __attribute__((overloadable)) degrees(double2 radians);
double3  __attribute__((const)) __attribute__((overloadable)) degrees(double3 radians);
double4  __attribute__((const)) __attribute__((overloadable)) degrees(double4 radians);
double8  __attribute__((const)) __attribute__((overloadable)) degrees(double8 radians);
double16  __attribute__((const)) __attribute__((overloadable)) degrees(double16 radians);
//half  __attribute__((const)) __attribute__((overloadable)) degrees(half radians);
//half2  __attribute__((const)) __attribute__((overloadable)) degrees(half2 radians);
//half3  __attribute__((const)) __attribute__((overloadable)) degrees(half3 radians);
//half4  __attribute__((const)) __attribute__((overloadable)) degrees(half4 radians);
//half8  __attribute__((const)) __attribute__((overloadable)) degrees(half8 radians);
//half16  __attribute__((const)) __attribute__((overloadable)) degrees(half16 radians);

/**
 * Returns y if x < y, otherwise it returns x. If x and y
 * are infinite or NaN, the return values are undefined.
 */
double  __attribute__((const)) __attribute__((overloadable)) max(double x, double y);
double2  __attribute__((const)) __attribute__((overloadable)) max(double2 x, double2 y);
double3  __attribute__((const)) __attribute__((overloadable)) max(double3 x, double3 y);
double4  __attribute__((const)) __attribute__((overloadable)) max(double4 x, double4 y);
double8  __attribute__((const)) __attribute__((overloadable)) max(double8 x, double8 y);
double16  __attribute__((const)) __attribute__((overloadable)) max(double16 x, double16 y);
double2  __attribute__((const)) __attribute__((overloadable)) max(double2 x, double y);
double3  __attribute__((const)) __attribute__((overloadable)) max(double3 x, double y);
double4  __attribute__((const)) __attribute__((overloadable)) max(double4 x, double y);
double8  __attribute__((const)) __attribute__((overloadable)) max(double8 x, double y);
double16  __attribute__((const)) __attribute__((overloadable)) max(double16 x, double y);
//half  __attribute__((const)) __attribute__((overloadable)) max(half x, half y);
//half2  __attribute__((const)) __attribute__((overloadable)) max(half2 x, half2 y);
//half3  __attribute__((const)) __attribute__((overloadable)) max(half3 x, half3 y);
//half4  __attribute__((const)) __attribute__((overloadable)) max(half4 x, half4 y);
//half8  __attribute__((const)) __attribute__((overloadable)) max(half8 x, half8 y);
//half16  __attribute__((const)) __attribute__((overloadable)) max(half16 x, half16 y);
//half2  __attribute__((const)) __attribute__((overloadable)) max(half2 x, half y);
//half3  __attribute__((const)) __attribute__((overloadable)) max(half3 x, half y);
//half4  __attribute__((const)) __attribute__((overloadable)) max(half4 x, half y);
//half8  __attribute__((const)) __attribute__((overloadable)) max(half8 x, half y);
//half16  __attribute__((const)) __attribute__((overloadable)) max(half16 x, half y);

/**
 * Returns y if y < x, otherwise it returns x. If x and y
 * are infinite or NaN, the return values are undefined.
 */
double  __attribute__((const)) __attribute__((overloadable)) min(double x, double y);
double2  __attribute__((const)) __attribute__((overloadable)) min(double2 x, double2 y);
double3  __attribute__((const)) __attribute__((overloadable)) min(double3 x, double3 y);
double4  __attribute__((const)) __attribute__((overloadable)) min(double4 x, double4 y);
double8  __attribute__((const)) __attribute__((overloadable)) min(double8 x, double8 y);
double16  __attribute__((const)) __attribute__((overloadable)) min(double16 x, double16 y);
double2  __attribute__((const)) __attribute__((overloadable)) min(double2 x, double y);
double3  __attribute__((const)) __attribute__((overloadable)) min(double3 x, double y);
double4  __attribute__((const)) __attribute__((overloadable)) min(double4 x, double y);
double8  __attribute__((const)) __attribute__((overloadable)) min(double8 x, double y);
double16  __attribute__((const)) __attribute__((overloadable)) min(double16 x, double y);
//half  __attribute__((const)) __attribute__((overloadable)) min(half x, half y);
//half2  __attribute__((const)) __attribute__((overloadable)) min(half2 x, half2 y);
//half3  __attribute__((const)) __attribute__((overloadable)) min(half3 x, half3 y);
//half4  __attribute__((const)) __attribute__((overloadable)) min(half4 x, half4 y);
//half8  __attribute__((const)) __attribute__((overloadable)) min(half8 x, half8 y);
//half16  __attribute__((const)) __attribute__((overloadable)) min(half16 x, half16 y);
//half2  __attribute__((const)) __attribute__((overloadable)) min(half2 x, half y);
//half3  __attribute__((const)) __attribute__((overloadable)) min(half3 x, half y);
//half4  __attribute__((const)) __attribute__((overloadable)) min(half4 x, half y);
//half8  __attribute__((const)) __attribute__((overloadable)) min(half8 x, half y);
//half16  __attribute__((const)) __attribute__((overloadable)) min(half16 x, half y);

/**
 * Returns the linear blend of x & y implemented as:
 * x + (y – x) * a
 * a must be a value in the range 0.0 … 1.0. If a is not
 * in the range 0.0 … 1.0, the return values are
 * undefined.
 */
double  __attribute__((const)) __attribute__((overloadable)) mix(double x, double y, double a);
double2  __attribute__((const)) __attribute__((overloadable)) mix(double2 x, double2 y, double2 a);
double3  __attribute__((const)) __attribute__((overloadable)) mix(double3 x, double3 y, double3 a);
double4  __attribute__((const)) __attribute__((overloadable)) mix(double4 x, double4 y, double4 a);
double8  __attribute__((const)) __attribute__((overloadable)) mix(double8 x, double8 y, double8 a);
double16  __attribute__((const)) __attribute__((overloadable)) mix(double16 x, double16 y, double16 a);
double2  __attribute__((const)) __attribute__((overloadable)) mix(double2 x, double2 y, double a);
double3  __attribute__((const)) __attribute__((overloadable)) mix(double3 x, double3 y, double a);
double4  __attribute__((const)) __attribute__((overloadable)) mix(double4 x, double4 y, double a);
double8  __attribute__((const)) __attribute__((overloadable)) mix(double8 x, double8 y, double a);
double16  __attribute__((const)) __attribute__((overloadable)) mix(double16 x, double16 y, double a);
//half  __attribute__((const)) __attribute__((overloadable)) mix(half x, half y, half a);
//half2  __attribute__((const)) __attribute__((overloadable)) mix(half2 x, half2 y, half2 a);
//half3  __attribute__((const)) __attribute__((overloadable)) mix(half3 x, half3 y, half3 a);
//half4  __attribute__((const)) __attribute__((overloadable)) mix(half4 x, half4 y, half4 a);
//half8  __attribute__((const)) __attribute__((overloadable)) mix(half8 x, half8 y, half8 a);
//half16  __attribute__((const)) __attribute__((overloadable)) mix(half16 x, half16 y, half16 a);
//half2  __attribute__((const)) __attribute__((overloadable)) mix(half2 x, half2 y, half a);
//half3  __attribute__((const)) __attribute__((overloadable)) mix(half3 x, half3 y, half a);
//half4  __attribute__((const)) __attribute__((overloadable)) mix(half4 x, half4 y, half a);
//half8  __attribute__((const)) __attribute__((overloadable)) mix(half8 x, half8 y, half a);
//half16  __attribute__((const)) __attribute__((overloadable)) mix(half16 x, half16 y, half a);

/**
 * Converts degrees to radians, i.e. (PI / 180) *
 * degrees.
 */
double  __attribute__((const)) __attribute__((overloadable)) radians(double degrees);
double2  __attribute__((const)) __attribute__((overloadable)) radians(double2 degrees);
double3  __attribute__((const)) __attribute__((overloadable)) radians(double3 degrees);
double4  __attribute__((const)) __attribute__((overloadable)) radians(double4 degrees);
double8  __attribute__((const)) __attribute__((overloadable)) radians(double8 degrees);
double16  __attribute__((const)) __attribute__((overloadable)) radians(double16 degrees);
//half  __attribute__((const)) __attribute__((overloadable)) radians(half degrees);
//half2  __attribute__((const)) __attribute__((overloadable)) radians(half2 degrees);
//half3  __attribute__((const)) __attribute__((overloadable)) radians(half3 degrees);
//half4  __attribute__((const)) __attribute__((overloadable)) radians(half4 degrees);
//half8  __attribute__((const)) __attribute__((overloadable)) radians(half8 degrees);
//half16  __attribute__((const)) __attribute__((overloadable)) radians(half16 degrees);

/**
 * Returns 0.0 if x < edge, otherwise it returns 1.0.
 */
double  __attribute__((const)) __attribute__((overloadable)) step(double edge, double x);
double2  __attribute__((const)) __attribute__((overloadable)) step(double2 edge, double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) step(double3 edge, double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) step(double4 edge, double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) step(double8 edge, double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) step(double16 edge, double16 x);
double  __attribute__((const)) __attribute__((overloadable)) step(double edge, double x);
double2  __attribute__((const)) __attribute__((overloadable)) step(double edge, double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) step(double edge, double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) step(double edge, double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) step(double edge, double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) step(double edge, double16 x);
//half  __attribute__((const)) __attribute__((overloadable)) step(half edge, half x);
//half2  __attribute__((const)) __attribute__((overloadable)) step(half2 edge, half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) step(half3 edge, half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) step(half4 edge, half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) step(half8 edge, half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) step(half16 edge, half16 x);
//half  __attribute__((const)) __attribute__((overloadable)) step(half edge, half x);
//half2  __attribute__((const)) __attribute__((overloadable)) step(half edge, half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) step(half edge, half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) step(half edge, half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) step(half edge, half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) step(half edge, half16 x);

/**
 * Returns 0.0 if x <= edge0 and 1.0 if x >= edge1 and
 * performs smooth Hermite interpolation between 0
 * and 1when edge0 < x < edge1. This is useful in
 * cases where you would want a threshold function
 * with a smooth transition.
 * This is equivalent to:
 * gentype t;
 * t = clamp ((x – edge0) / (edge1 – edge0), 0, 1);
 * return t * t * (3 – 2 * t);
 * Results are undefined if edge0 >= edge1 or if x,
 * edge0 or edge1 is a NaN.
 */
double  __attribute__((const)) __attribute__((overloadable)) smoothstep(double edge0, double edge1, double x);
double2  __attribute__((const)) __attribute__((overloadable)) smoothstep(double edge0, double edge1, double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) smoothstep(double edge0, double edge1, double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) smoothstep(double edge0, double edge1, double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) smoothstep(double edge0, double edge1, double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) smoothstep(double edge0, double edge1, double16 x);
double  __attribute__((const)) __attribute__((overloadable)) smoothstep(double edge0, double edge1, double x);
double2  __attribute__((const)) __attribute__((overloadable)) smoothstep(double2 edge0, double2 edge1, double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) smoothstep(double3 edge0, double3 edge1, double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) smoothstep(double4 edge0, double4 edge1, double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) smoothstep(double8 edge0, double8 edge1, double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) smoothstep(double16 edge0, double16 edge1, double16 x);
//half  __attribute__((const)) __attribute__((overloadable)) smoothstep(half edge0, half edge1, half x);
//half2  __attribute__((const)) __attribute__((overloadable)) smoothstep(half2 edge0, half2 edge1, half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) smoothstep(half3 edge0, half3 edge1, half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) smoothstep(half4 edge0, half4 edge1, half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) smoothstep(half8 edge0, half8 edge1, half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) smoothstep(half16 edge0, half16 edge1, half16 x);
//half  __attribute__((const)) __attribute__((overloadable)) smoothstep(half edge0, half edge1, half x);
//half2  __attribute__((const)) __attribute__((overloadable)) smoothstep(half edge0, half edge1, half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) smoothstep(half edge0, half edge1, half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) smoothstep(half edge0, half edge1, half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) smoothstep(half edge0, half edge1, half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) smoothstep(half edge0, half edge1, half16 x);

/**
 * Returns 1.0 if x > 0, -0.0 if x = -0.0, +0.0 if x =
 * +0.0, or –1.0 if x < 0. Returns 0.0 if x is a NaN.
 */
double  __attribute__((const)) __attribute__((overloadable)) sign(double x);
double2  __attribute__((const)) __attribute__((overloadable)) sign(double2 x);
double3  __attribute__((const)) __attribute__((overloadable)) sign(double3 x);
double4  __attribute__((const)) __attribute__((overloadable)) sign(double4 x);
double8  __attribute__((const)) __attribute__((overloadable)) sign(double8 x);
double16  __attribute__((const)) __attribute__((overloadable)) sign(double16 x);
//half  __attribute__((const)) __attribute__((overloadable)) sign(half x);
//half2  __attribute__((const)) __attribute__((overloadable)) sign(half2 x);
//half3  __attribute__((const)) __attribute__((overloadable)) sign(half3 x);
//half4  __attribute__((const)) __attribute__((overloadable)) sign(half4 x);
//half8  __attribute__((const)) __attribute__((overloadable)) sign(half8 x);
//half16  __attribute__((const)) __attribute__((overloadable)) sign(half16 x);

// Geometric functions:

/**
 * Returns the cross product of p0.xyz and p1.xyz. The
 * w component of float4 result returned will be 0.0.
 */
double4  __attribute__((const)) __attribute__((overloadable)) cross(double4 p0, double4 p1);
double3  __attribute__((const)) __attribute__((overloadable)) cross(double3 p0, double3 p1);
//half4  __attribute__((const)) __attribute__((overloadable)) cross(half4 p0, half4 p1);
//half3  __attribute__((const)) __attribute__((overloadable)) cross(half3 p0, half3 p1);

/**
 * Compute dot product.
 */
double  __attribute__((const)) __attribute__((overloadable)) dot(double p0, double p1);
double  __attribute__((const)) __attribute__((overloadable)) dot(double2 p0, double2 p1);
double  __attribute__((const)) __attribute__((overloadable)) dot(double3 p0, double3 p1);
double  __attribute__((const)) __attribute__((overloadable)) dot(double4 p0, double4 p1);
//half  __attribute__((const)) __attribute__((overloadable)) dot(half p0, half p1);
//half  __attribute__((const)) __attribute__((overloadable)) dot(half2 p0, half2 p1);
//half  __attribute__((const)) __attribute__((overloadable)) dot(half3 p0, half3 p1);
//half  __attribute__((const)) __attribute__((overloadable)) dot(half4 p0, half4 p1);

/**
 * Returns the distance between p0 and p1. This is
 * calculated as length(p0 – p1).
 */
double  __attribute__((const)) __attribute__((overloadable)) distance(double p0, double p1);
double  __attribute__((const)) __attribute__((overloadable)) distance(double2 p0, double2 p1);
double  __attribute__((const)) __attribute__((overloadable)) distance(double3 p0, double3 p1);
double  __attribute__((const)) __attribute__((overloadable)) distance(double4 p0, double4 p1);
//half  __attribute__((const)) __attribute__((overloadable)) distance(half p0, half p1);
//half  __attribute__((const)) __attribute__((overloadable)) distance(half2 p0, half2 p1);
//half  __attribute__((const)) __attribute__((overloadable)) distance(half3 p0, half3 p1);
//half  __attribute__((const)) __attribute__((overloadable)) distance(half4 p0, half4 p1);

/**
 * Return the length of vector p, i.e.,
 * sqrt(p.x2 + p.y 2 + ...)
 */
double  __attribute__((const)) __attribute__((overloadable)) length(double p);
double  __attribute__((const)) __attribute__((overloadable)) length(double2 p);
double  __attribute__((const)) __attribute__((overloadable)) length(double3 p);
double  __attribute__((const)) __attribute__((overloadable)) length(double4 p);
//half  __attribute__((const)) __attribute__((overloadable)) length(half p);
//half  __attribute__((const)) __attribute__((overloadable)) length(half2 p);
//half  __attribute__((const)) __attribute__((overloadable)) length(half3 p);
//half  __attribute__((const)) __attribute__((overloadable)) length(half4 p);

/**
 * Returns a vector in the same direction as p but with a
 * length of 1.
 */
double  __attribute__((const)) __attribute__((overloadable)) normalize(double p);
double2  __attribute__((const)) __attribute__((overloadable)) normalize(double2 p);
double3  __attribute__((const)) __attribute__((overloadable)) normalize(double3 p);
double4  __attribute__((const)) __attribute__((overloadable)) normalize(double4 p);
//half  __attribute__((const)) __attribute__((overloadable)) normalize(half p);
//half2  __attribute__((const)) __attribute__((overloadable)) normalize(half2 p);
//half3  __attribute__((const)) __attribute__((overloadable)) normalize(half3 p);
//half4  __attribute__((const)) __attribute__((overloadable)) normalize(half4 p);

/**
 * Returns the component-wise compare of
 * (x < y) || (x > y) .
 */
int  __attribute__((const)) __attribute__((overloadable)) islessgreater(double x, double y);
long2  __attribute__((const)) __attribute__((overloadable)) islessgreater(double2 x, double2 y);
long3  __attribute__((const)) __attribute__((overloadable)) islessgreater(double3 x, double3 y);
long4  __attribute__((const)) __attribute__((overloadable)) islessgreater(double4 x, double4 y);
long8  __attribute__((const)) __attribute__((overloadable)) islessgreater(double8 x, double8 y);
long16  __attribute__((const)) __attribute__((overloadable)) islessgreater(double16 x, double16 y);
//int  __attribute__((const)) __attribute__((overloadable)) islessgreater(half x, half y);
//short2  __attribute__((const)) __attribute__((overloadable)) islessgreater(half2 x, half2 y);
//short3  __attribute__((const)) __attribute__((overloadable)) islessgreater(half3 x, half3 y);
//short4  __attribute__((const)) __attribute__((overloadable)) islessgreater(half4 x, half4 y);
//short8  __attribute__((const)) __attribute__((overloadable)) islessgreater(half8 x, half8 y);
//short16  __attribute__((const)) __attribute__((overloadable)) islessgreater(half16 x, half16 y);

/**
 * Test for finite value.
 */
int  __attribute__((const)) __attribute__((overloadable)) isfinite(double);
long2  __attribute__((const)) __attribute__((overloadable)) isfinite(double2);
long3  __attribute__((const)) __attribute__((overloadable)) isfinite(double3);
long4  __attribute__((const)) __attribute__((overloadable)) isfinite(double4);
long8  __attribute__((const)) __attribute__((overloadable)) isfinite(double8);
long16  __attribute__((const)) __attribute__((overloadable)) isfinite(double16);
//int  __attribute__((const)) __attribute__((overloadable)) isfinite(half);
//short2  __attribute__((const)) __attribute__((overloadable)) isfinite(half2);
//short3  __attribute__((const)) __attribute__((overloadable)) isfinite(half3);
//short4  __attribute__((const)) __attribute__((overloadable)) isfinite(half4);
//short8  __attribute__((const)) __attribute__((overloadable)) isfinite(half8);
//short16  __attribute__((const)) __attribute__((overloadable)) isfinite(half16);

/**
 * Test for infinity value (+ve or –ve) .
 */
int  __attribute__((const)) __attribute__((overloadable)) isinf(double);
long2  __attribute__((const)) __attribute__((overloadable)) isinf(double2);
long3  __attribute__((const)) __attribute__((overloadable)) isinf(double3);
long4  __attribute__((const)) __attribute__((overloadable)) isinf(double4);
long8  __attribute__((const)) __attribute__((overloadable)) isinf(double8);
long16  __attribute__((const)) __attribute__((overloadable)) isinf(double16);
//int  __attribute__((const)) __attribute__((overloadable)) isinf(half);
//short2  __attribute__((const)) __attribute__((overloadable)) isinf(half2);
//short3  __attribute__((const)) __attribute__((overloadable)) isinf(half3);
//short4  __attribute__((const)) __attribute__((overloadable)) isinf(half4);
//short8  __attribute__((const)) __attribute__((overloadable)) isinf(half8);
//short16  __attribute__((const)) __attribute__((overloadable)) isinf(half16);

/**
 * Test for a NaN.
 */
int  __attribute__((const)) __attribute__((overloadable)) isnan(double);
long2  __attribute__((const)) __attribute__((overloadable)) isnan(double2);
long3  __attribute__((const)) __attribute__((overloadable)) isnan(double3);
long4  __attribute__((const)) __attribute__((overloadable)) isnan(double4);
long8  __attribute__((const)) __attribute__((overloadable)) isnan(double8);
long16  __attribute__((const)) __attribute__((overloadable)) isnan(double16);
//int  __attribute__((const)) __attribute__((overloadable)) isnan(half);
//short2  __attribute__((const)) __attribute__((overloadable)) isnan(half2);
//short3  __attribute__((const)) __attribute__((overloadable)) isnan(half3);
//short4  __attribute__((const)) __attribute__((overloadable)) isnan(half4);
//short8  __attribute__((const)) __attribute__((overloadable)) isnan(half8);
//short16  __attribute__((const)) __attribute__((overloadable)) isnan(half16);

/**
 * Test for a normal value.
 */
int  __attribute__((const)) __attribute__((overloadable)) isnormal(double);
long2  __attribute__((const)) __attribute__((overloadable)) isnormal(double2);
long3  __attribute__((const)) __attribute__((overloadable)) isnormal(double3);
long4  __attribute__((const)) __attribute__((overloadable)) isnormal(double4);
long8  __attribute__((const)) __attribute__((overloadable)) isnormal(double8);
long16  __attribute__((const)) __attribute__((overloadable)) isnormal(double16);
//int  __attribute__((const)) __attribute__((overloadable)) isnormal(half);
//short2  __attribute__((const)) __attribute__((overloadable)) isnormal(half2);
//short3  __attribute__((const)) __attribute__((overloadable)) isnormal(half3);
//short4  __attribute__((const)) __attribute__((overloadable)) isnormal(half4);
//short8  __attribute__((const)) __attribute__((overloadable)) isnormal(half8);
//short16  __attribute__((const)) __attribute__((overloadable)) isnormal(half16);

/**
 * Test if arguments are ordered. isordered() takes
 * arguments x and y, and returns the result
 * isequal(x, x) && isequal(y, y).
 */
int  __attribute__((const)) __attribute__((overloadable)) isordered(double x, double y);
long2  __attribute__((const)) __attribute__((overloadable)) isordered(double2 x, double2 y);
long3  __attribute__((const)) __attribute__((overloadable)) isordered(double3 x, double3 y);
long4  __attribute__((const)) __attribute__((overloadable)) isordered(double4 x, double4 y);
long8  __attribute__((const)) __attribute__((overloadable)) isordered(double8 x, double8 y);
long16  __attribute__((const)) __attribute__((overloadable)) isordered(double16 x, double16 y);
//int  __attribute__((const)) __attribute__((overloadable)) isordered(half x, half y);
//short2  __attribute__((const)) __attribute__((overloadable)) isordered(half2 x, half2 y);
//short3  __attribute__((const)) __attribute__((overloadable)) isordered(half3 x, half3 y);
//short4  __attribute__((const)) __attribute__((overloadable)) isordered(half4 x, half4 y);
//short8  __attribute__((const)) __attribute__((overloadable)) isordered(half8 x, half8 y);
//short16  __attribute__((const)) __attribute__((overloadable)) isordered(half16 x, half16 y);

/**
 * Test if arguments are unordered. isunordered()
 * takes arguments x and y, returning non-zero if x or y
 * is NaN, and zero otherwise.
 */
int  __attribute__((const)) __attribute__((overloadable)) isunordered(double x, double y);
long2  __attribute__((const)) __attribute__((overloadable)) isunordered(double2 x, double2 y);
long3  __attribute__((const)) __attribute__((overloadable)) isunordered(double3 x, double3 y);
long4  __attribute__((const)) __attribute__((overloadable)) isunordered(double4 x, double4 y);
long8  __attribute__((const)) __attribute__((overloadable)) isunordered(double8 x, double8 y);
long16  __attribute__((const)) __attribute__((overloadable)) isunordered(double16 x, double16 y);
//int  __attribute__((const)) __attribute__((overloadable)) isunordered(half x, half y);
//short2  __attribute__((const)) __attribute__((overloadable)) isunordered(half2 x, half2 y);
//short3  __attribute__((const)) __attribute__((overloadable)) isunordered(half3 x, half3 y);
//short4  __attribute__((const)) __attribute__((overloadable)) isunordered(half4 x, half4 y);
//short8  __attribute__((const)) __attribute__((overloadable)) isunordered(half8 x, half8 y);
//short16  __attribute__((const)) __attribute__((overloadable)) isunordered(half16 x, half16 y);

/**
 * Test for sign bit. The scalar version of the function
 * returns a 1 if the sign bit in the float is set else returns
 * 0. The vector version of the function returns the
 * following for each component in floatn: a -1 if the
 * sign bit in the float is set else returns 0.
 */
int  __attribute__((const)) __attribute__((overloadable)) signbit(double);
long2  __attribute__((const)) __attribute__((overloadable)) signbit(double2);
long3  __attribute__((const)) __attribute__((overloadable)) signbit(double3);
long4  __attribute__((const)) __attribute__((overloadable)) signbit(double4);
long8  __attribute__((const)) __attribute__((overloadable)) signbit(double8);
long16  __attribute__((const)) __attribute__((overloadable)) signbit(double16);
//int  __attribute__((const)) __attribute__((overloadable)) signbit(half);
//short2  __attribute__((const)) __attribute__((overloadable)) signbit(half2);
//short3  __attribute__((const)) __attribute__((overloadable)) signbit(half3);
//short4  __attribute__((const)) __attribute__((overloadable)) signbit(half4);
//short8  __attribute__((const)) __attribute__((overloadable)) signbit(half8);
//short16  __attribute__((const)) __attribute__((overloadable)) signbit(half16);

/**
 * Returns 1 if the most significant bit in any component
 * of x is set; otherwise returns 0.
 */
double  __attribute__((const)) __attribute__((overloadable)) bitselect(double a, double b, double c);
double2  __attribute__((const)) __attribute__((overloadable)) bitselect(double2 a, double2 b, double2 c);
double3  __attribute__((const)) __attribute__((overloadable)) bitselect(double3 a, double3 b, double3 c);
double4  __attribute__((const)) __attribute__((overloadable)) bitselect(double4 a, double4 b, double4 c);
double8  __attribute__((const)) __attribute__((overloadable)) bitselect(double8 a, double8 b, double8 c);
double16  __attribute__((const)) __attribute__((overloadable)) bitselect(double16 a, double16 b, double16 c);
//half  __attribute__((const)) __attribute__((overloadable)) bitselect(half a, half b, half c);
//half2  __attribute__((const)) __attribute__((overloadable)) bitselect(half2 a, half2 b, half2 c);
//half3  __attribute__((const)) __attribute__((overloadable)) bitselect(half3 a, half3 b, half3 c);
//half4  __attribute__((const)) __attribute__((overloadable)) bitselect(half4 a, half4 b, half4 c);
//half8  __attribute__((const)) __attribute__((overloadable)) bitselect(half8 a, half8 b, half8 c);
//half16  __attribute__((const)) __attribute__((overloadable)) bitselect(half16 a, half16 b, half16 c);

/**
 * For each component of a vector type,
 * result[i] = if MSB of c[i] is set ? b[i] : a[i].
 * For a scalar type, result = c ? b : a.
 */
double  __attribute__((const)) __attribute__((overloadable)) select(double a, double b, long c);
double2  __attribute__((const)) __attribute__((overloadable)) select(double2 a, double2 b, long2 c);
double3  __attribute__((const)) __attribute__((overloadable)) select(double3 a, double3 b, long3 c);
double4  __attribute__((const)) __attribute__((overloadable)) select(double4 a, double4 b, long4 c);
double8  __attribute__((const)) __attribute__((overloadable)) select(double8 a, double8 b, long8 c);
double16  __attribute__((const)) __attribute__((overloadable)) select(double16 a, double16 b, long16 c);
double  __attribute__((const)) __attribute__((overloadable)) select(double a, double b, ulong c);
double2  __attribute__((const)) __attribute__((overloadable)) select(double2 a, double2 b, ulong2 c);
double3  __attribute__((const)) __attribute__((overloadable)) select(double3 a, double3 b, ulong3 c);
double4  __attribute__((const)) __attribute__((overloadable)) select(double4 a, double4 b, ulong4 c);
double8  __attribute__((const)) __attribute__((overloadable)) select(double8 a, double8 b, ulong8 c);
double16  __attribute__((const)) __attribute__((overloadable)) select(double16 a, double16 b, ulong16 c);
//half  __attribute__((const)) __attribute__((overloadable)) select(half a, half b, short c);
//half2  __attribute__((const)) __attribute__((overloadable)) select(half2 a, half2 b, short2 c);
//half3  __attribute__((const)) __attribute__((overloadable)) select(half3 a, half3 b, short3 c);
//half4  __attribute__((const)) __attribute__((overloadable)) select(half4 a, half4 b, short4 c);
//half8  __attribute__((const)) __attribute__((overloadable)) select(half8 a, half8 b, short8 c);
//half16  __attribute__((const)) __attribute__((overloadable)) select(half16 a, half16 b, short16 c);
//half  __attribute__((const)) __attribute__((overloadable)) select(half a, half b, ushort c);
//half2  __attribute__((const)) __attribute__((overloadable)) select(half2 a, half2 b, ushort2 c);
//half3  __attribute__((const)) __attribute__((overloadable)) select(half3 a, half3 b, ushort3 c);
//half4  __attribute__((const)) __attribute__((overloadable)) select(half4 a, half4 b, ushort4 c);
//half8  __attribute__((const)) __attribute__((overloadable)) select(half8 a, half8 b, ushort8 c);
//half16  __attribute__((const)) __attribute__((overloadable)) select(half16 a, half16 b, ushort16 c);

// Vector data load and store functions

// Atomic functions

/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * (old + val) and store result at location
 * pointed by p. The function returns old.
 */
int __attribute__((overloadable)) atom_add(__global int *p, int val);
unsigned int __attribute__((overloadable)) atom_add(__global unsigned int *p, unsigned int val);
int __attribute__((overloadable)) atom_add(__local int *p, int val);
unsigned int __attribute__((overloadable)) atom_add(__local unsigned int *p, unsigned int val);

/**
 * unsigned int atomic_sub (
 * volatile __global unsigned int *p,
 * unsigned int val)
 * int atomic_sub (volatile __local int *p, int val)
 * unsigned int atomic_sub (
 * volatile __local unsigned int *p,
 * unsigned int val)
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * (old - val) and store result at location
 * pointed by p. The function returns old.
 */
int __attribute__((overloadable)) atom_sub(__global int *p, int val);
unsigned int __attribute__((overloadable)) atom_sub(__global unsigned int *p, unsigned int val);
int __attribute__((overloadable)) atom_sub(__local int *p, int val);
unsigned int __attribute__((overloadable)) atom_sub(__local unsigned int *p, unsigned int val);


/**
 * Swaps the old value stored at location p
 * with new value given by val. Returns old
 * value.
 */


int __attribute__((overloadable)) atom_xchg(volatile __global int *p, int val);
unsigned int __attribute__((overloadable)) atom_xchg(volatile __global unsigned int *p, unsigned int val);
int __attribute__((overloadable)) atom_xchg(volatile __local int *p, int val);
unsigned int __attribute__((overloadable)) atom_xchg(volatile __local unsigned int *p, unsigned int val);

/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * (old + 1) and store result at location
 * pointed by p. The function returns old.
 */

int __attribute__((overloadable)) atom_inc(__global int *p);
unsigned int __attribute__((overloadable)) atom_inc(__global unsigned int *p);
int __attribute__((overloadable)) atom_inc(__local int *p);
unsigned int __attribute__((overloadable)) atom_inc(__local unsigned int *p);

/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * (old - 1) and store result at location
 * pointed by p. The function returns old.
 */
int __attribute__((overloadable)) atom_dec(__global int *p);
unsigned int __attribute__((overloadable)) atom_dec(__global unsigned int *p);
int __attribute__((overloadable)) atom_dec(__local int *p);
unsigned int __attribute__((overloadable)) atom_dec(__local unsigned int *p);


/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * (old == cmp) ? val : old and store result at
 * location pointed by p. The function
 * returns old.
 */

int __attribute__((overloadable)) atom_cmpxchg(__global int *p, int cmp, int val);
unsigned int __attribute__((overloadable)) atom_cmpxchg(__global unsigned int *p, unsigned int cmp, unsigned int val);
int __attribute__((overloadable)) atom_cmpxchg(__local int *p, int cmp, int val);
unsigned int __attribute__((overloadable)) atom_cmpxchg(__local unsigned int *p, unsigned int cmp, unsigned int val);


/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * min(old, val) and store minimum value at
 * location pointed by p. The function
 * returns old.
 */

int __attribute__((overloadable)) atom_min(__global int *p, int val);
unsigned int __attribute__((overloadable)) atom_min(__global unsigned int *p, unsigned int val);
int __attribute__((overloadable)) atom_min(__local int *p, int val);
unsigned int __attribute__((overloadable)) atom_min(__local unsigned int *p, unsigned int val);


/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * max(old, val) and store maximum value at
 * location pointed by p. The function
 * returns old.
 */

int __attribute__((overloadable)) atom_max(__global int *p, int val);
unsigned int __attribute__((overloadable)) atom_max(__global unsigned int *p, unsigned int val);
int __attribute__((overloadable)) atom_max(__local int *p, int val);
unsigned int __attribute__((overloadable)) atom_max(__local unsigned int *p, unsigned int val);

/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * (old & val) and store result at location
 * pointed by p. The function returns old.
 */

int __attribute__((overloadable)) atom_and(__global int *p, int val);
unsigned int __attribute__((overloadable)) atom_and(__global unsigned int *p, unsigned int val);
int __attribute__((overloadable)) atom_and(__local int *p, int val);
unsigned int __attribute__((overloadable)) atom_and(__local unsigned int *p, unsigned int val);


/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * (old | val) and store result at location
 * pointed by p. The function returns old.
 */

int __attribute__((overloadable)) atom_or(__global int *p, int val);
unsigned int __attribute__((overloadable)) atom_or(__global unsigned int *p, unsigned int val);
int __attribute__((overloadable)) atom_or(__local int *p, int val);
unsigned int __attribute__((overloadable)) atom_or(__local unsigned int *p, unsigned int val);


/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * (old ^ val) and store result at location
 * pointed by p. The function returns old.
 */

int __attribute__((overloadable)) atom_xor(__global int *p, int val);
unsigned int __attribute__((overloadable)) atom_xor(__global unsigned int *p, unsigned int val);
int __attribute__((overloadable)) atom_xor(__local int *p, int val);
unsigned int __attribute__((overloadable)) atom_xor(__local unsigned int *p, unsigned int val);

// Miscellaneous vector functions

/**
 * The shuffle and shuffle2 built-in functions construct
 * a permutation of elements from one or two input
 * vectors respectively that are of the same type,
 * returning a vector with the same element type as the
 * input and length that is the same as the shuffle mask.
 * The size of each element in the mask must match the
 * size of each element in the result. For shuffle, only
 * the ilogb(2m-1) least significant bits of each mask
 * element are considered. For shuffle2, only the
 * ilogb(2m-1)+1 least significant bits of each mask
 * element are considered. Other bits in the mask shall
 * be ignored.
 * The elements of the input vectors are numbered from
 * left to right across one or both of the vectors. For this
 * purpose, the number of elements in a vector is given
 * by vec_step(gentypem). The shuffle mask operand
 * specifies, for each element of the result vector, which
 * element of the one or two input vectors the result
 * element gets.
 * Examples:
 * uint4 mask = (uint4)(3, 2,
 * 1, 0);
 * float4 a;
 * float4 r = shuffle(a, mask);
 * // r.s0123 = a.wzyx
 * uint8 mask = (uint8)(0, 1, 2, 3,
 * 4, 5, 6, 7);
 * float4 a, b;
 * float8 r = shuffle2(a, b, mask);
 * // r.s0123 = a.xyzw
 * // r.s4567 = b.xyzw
 * uint4 mask;
 * float8 a;
 * float4 b;
 * b = shuffle(a, mask);
 * Examples that are not valid are:
 * uint8 mask;
 * short16 a;
 * short8 b;
 * b = shuffle(a, mask); <- not valid
 */

//half2  __attribute__((const)) __attribute__((overloadable)) shuffle(half2 x, ushort2 mask);
//half2  __attribute__((const)) __attribute__((overloadable)) shuffle(half4 x, ushort2 mask);
//half2  __attribute__((const)) __attribute__((overloadable)) shuffle(half8 x, ushort2 mask);
//half2  __attribute__((const)) __attribute__((overloadable)) shuffle(half16 x, ushort2 mask);

double2  __attribute__((const)) __attribute__((overloadable)) shuffle(double2 x, ulong2 mask);
double2  __attribute__((const)) __attribute__((overloadable)) shuffle(double4 x, ulong2 mask);
double2  __attribute__((const)) __attribute__((overloadable)) shuffle(double8 x, ulong2 mask);
double2  __attribute__((const)) __attribute__((overloadable)) shuffle(double16 x, ulong2 mask);

//half4  __attribute__((const)) __attribute__((overloadable)) shuffle(half2 x, ushort4 mask);
//half4  __attribute__((const)) __attribute__((overloadable)) shuffle(half4 x, ushort4 mask);
//half4  __attribute__((const)) __attribute__((overloadable)) shuffle(half8 x, ushort4 mask);
//half4  __attribute__((const)) __attribute__((overloadable)) shuffle(half16 x, ushort4 mask);


double4  __attribute__((const)) __attribute__((overloadable)) shuffle(double2 x, ulong4 mask);
double4  __attribute__((const)) __attribute__((overloadable)) shuffle(double4 x, ulong4 mask);
double4  __attribute__((const)) __attribute__((overloadable)) shuffle(double8 x, ulong4 mask);
double4  __attribute__((const)) __attribute__((overloadable)) shuffle(double16 x, ulong4 mask);


//half8  __attribute__((const)) __attribute__((overloadable)) shuffle(half2 x, ushort8 mask);
//half8  __attribute__((const)) __attribute__((overloadable)) shuffle(half4 x, ushort8 mask);
//half8  __attribute__((const)) __attribute__((overloadable)) shuffle(half8 x, ushort8 mask);
//half8  __attribute__((const)) __attribute__((overloadable)) shuffle(half16 x, ushort8 mask);


double8  __attribute__((const)) __attribute__((overloadable)) shuffle(double2 x, ulong8 mask);
double8  __attribute__((const)) __attribute__((overloadable)) shuffle(double4 x, ulong8 mask);
double8  __attribute__((const)) __attribute__((overloadable)) shuffle(double8 x, ulong8 mask);
double8  __attribute__((const)) __attribute__((overloadable)) shuffle(double16 x, ulong8 mask);

//half16  __attribute__((const)) __attribute__((overloadable)) shuffle(half2 x, ushort16 mask);
//half16  __attribute__((const)) __attribute__((overloadable)) shuffle(half4 x, ushort16 mask);
//half16  __attribute__((const)) __attribute__((overloadable)) shuffle(half8 x, ushort16 mask);
//half16  __attribute__((const)) __attribute__((overloadable)) shuffle(half16 x, ushort16 mask);


double16 __attribute__((const)) __attribute__((overloadable)) shuffle(double2 x, ulong16 mask);
double16 __attribute__((const)) __attribute__((overloadable)) shuffle(double4 x, ulong16 mask);
double16 __attribute__((const)) __attribute__((overloadable)) shuffle(double8 x, ulong16 mask);
double16 __attribute__((const)) __attribute__((overloadable)) shuffle(double16 x, ulong16 mask);



//half2  __attribute__((const)) __attribute__((overloadable)) shuffle2(half2 x, half2 y, ushort2 mask);
//half2  __attribute__((const)) __attribute__((overloadable)) shuffle2(half4 x, half4 y, ushort2 mask);
//half2  __attribute__((const)) __attribute__((overloadable)) shuffle2(half8 x, half8 y, ushort2 mask);
//half2  __attribute__((const)) __attribute__((overloadable)) shuffle2(half16 x, half16 y, ushort2 mask);


double2  __attribute__((const)) __attribute__((overloadable)) shuffle2(double2 x, double2 y, ulong2 mask);
double2  __attribute__((const)) __attribute__((overloadable)) shuffle2(double4 x, double4 y, ulong2 mask);
double2  __attribute__((const)) __attribute__((overloadable)) shuffle2(double8 x, double8 y, ulong2 mask);
double2  __attribute__((const)) __attribute__((overloadable)) shuffle2(double16 x, double16 y, ulong2 mask);


//half4  __attribute__((const)) __attribute__((overloadable)) shuffle2(half2 x, half2 y, ushort4 mask);
//half4  __attribute__((const)) __attribute__((overloadable)) shuffle2(half4 x, half4 y, ushort4 mask);
//half4  __attribute__((const)) __attribute__((overloadable)) shuffle2(half8 x, half8 y, ushort4 mask);
//half4  __attribute__((const)) __attribute__((overloadable)) shuffle2(half16 x, half16 y, ushort4 mask);


double4  __attribute__((const)) __attribute__((overloadable)) shuffle2(double2 x, double2 y, ulong4 mask);
double4  __attribute__((const)) __attribute__((overloadable)) shuffle2(double4 x, double4 y, ulong4 mask);
double4  __attribute__((const)) __attribute__((overloadable)) shuffle2(double8 x, double8 y, ulong4 mask);
double4  __attribute__((const)) __attribute__((overloadable)) shuffle2(double16 x, double16 y, ulong4 mask);


//half8  __attribute__((const)) __attribute__((overloadable)) shuffle2(half2 x, half2 y, ushort8 mask);
//half8  __attribute__((const)) __attribute__((overloadable)) shuffle2(half4 x, half4 y, ushort8 mask);
//half8  __attribute__((const)) __attribute__((overloadable)) shuffle2(half8 x, half8 y, ushort8 mask);
//half8  __attribute__((const)) __attribute__((overloadable)) shuffle2(half16 x, half16 y, ushort8 mask);


double8  __attribute__((const)) __attribute__((overloadable)) shuffle2(double2 x, double2 y, ulong8 mask);
double8  __attribute__((const)) __attribute__((overloadable)) shuffle2(double4 x, double4 y, ulong8 mask);
double8  __attribute__((const)) __attribute__((overloadable)) shuffle2(double8 x, double8 y, ulong8 mask);
double8  __attribute__((const)) __attribute__((overloadable)) shuffle2(double16 x, double16 y, ulong8 mask);


//half16  __attribute__((const)) __attribute__((overloadable)) shuffle2(half2 x, half2 y, ushort16 mask);
//half16  __attribute__((const)) __attribute__((overloadable)) shuffle2(half4 x, half4 y, ushort16 mask);
//half16  __attribute__((const)) __attribute__((overloadable)) shuffle2(half8 x, half8 y, ushort16 mask);
//half16  __attribute__((const)) __attribute__((overloadable)) shuffle2(half16 x, half16 y, ushort16 mask);


double16  __attribute__((const)) __attribute__((overloadable)) shuffle2(double2 x, double2 y, ulong16 mask);
double16  __attribute__((const)) __attribute__((overloadable)) shuffle2(double4 x, double4 y, ulong16 mask);
double16  __attribute__((const)) __attribute__((overloadable)) shuffle2(double8 x, double8 y, ulong16 mask);
double16  __attribute__((const)) __attribute__((overloadable)) shuffle2(double16 x, double16 y, ulong16 mask);

// Built-in image functions
/**
 * Use the coordinate (x, y) to do an element lookup in
 * the 2D image object specified by image.
 * read_imagef returns floating-point values in the
 * range [0.0 … 1.0] for image objects created with
 * image_channel_data_type set to one of the predefined
 * packed formats or CL_UNORM_INT8, or
 * CL_UNORM_INT16.
 * read_imagef returns floating-point values in the
 * range [-1.0 … 1.0] for image objects created with
 * image_channel_data_type set to CL_SNORM_INT8,
 * or CL_SNORM_INT16.
 * read_imagef returns floating-point values for image
 * objects created with image_channel_data_type set to
 * CL_HALF_FLOAT or CL_FLOAT.
 * The read_imagef calls that take integer coordinates
 * must use a sampler with filter mode set to
 * CLK_FILTER_NEAREST, normalized coordinates set
 * to CLK_NORMALIZED_COORDS_FALSE and
 * addressing mode set to
 * CLK_ADDRESS_CLAMP_TO_EDGE,
 * CLK_ADDRESS_CLAMP or CLK_ADDRESS_NONE;
 * otherwise the values returned are undefined.
 * Values returned by read_imagef for image objects
 * with image_channel_data_type values not specified
 * in the description above are undefined.
 */
//
// Addressing Mode.
//

//half4 __attribute__((overloadable))  read_imageh(__read_only image2d_t image, sampler_t sampler, int2 coord);
//half4 __attribute__((overloadable))  read_imageh(__read_only image2d_t image, sampler_t sampler, float2 coord);

// Since OpenCL 2.0 image2d_depth_t and image2d_array_depth_t are mandatory

//void __attribute__((overloadable)) write_imageh(__write_only image2d_t image, int2 coord, half4 color);

// IMAGE 1.2 built-ins
// with samplers and samplerless
float4  __attribute__((const)) __attribute__((overloadable))  read_imagef(__read_only image2d_array_t image, sampler_t sampler, int4 coord);
float4  __attribute__((const)) __attribute__((overloadable))  read_imagef(__read_only image2d_array_t image, sampler_t sampler, float4 coord);
int4  __attribute__((const)) __attribute__((overloadable))  read_imagei(__read_only image2d_array_t image, sampler_t sampler, int4 coord);
int4  __attribute__((const)) __attribute__((overloadable))  read_imagei(__read_only image2d_array_t image, sampler_t sampler, float4 coord);
uint4  __attribute__((const)) __attribute__((overloadable))  read_imageui(__read_only image2d_array_t image, sampler_t sampler, int4 coord);
uint4  __attribute__((const)) __attribute__((overloadable))  read_imageui(__read_only image2d_array_t image, sampler_t sampler, float4 coord);
float4 __attribute__((const)) __attribute__((overloadable))  read_imagef(__read_only image1d_t image, sampler_t sampler, int coord);
float4 __attribute__((const)) __attribute__((overloadable))  read_imagef(__read_only image1d_t image, sampler_t sampler, float coord);
int4 __attribute__((const)) __attribute__((overloadable))  read_imagei(__read_only image1d_t image, sampler_t sampler, int coord);
int4 __attribute__((const)) __attribute__((overloadable))  read_imagei(__read_only image1d_t image, sampler_t sampler, float coord);
uint4 __attribute__((const)) __attribute__((overloadable))  read_imageui(__read_only image1d_t image, sampler_t sampler, int coord);
uint4 __attribute__((const)) __attribute__((overloadable))  read_imageui(__read_only image1d_t image, sampler_t sampler, float coord);
float4 __attribute__((const)) __attribute__((overloadable))  read_imagef(__read_only image1d_array_t image, sampler_t sampler, int2 coord);
float4 __attribute__((const)) __attribute__((overloadable))  read_imagef(__read_only image1d_array_t image, sampler_t sampler, float2 coord);
int4 __attribute__((const)) __attribute__((overloadable))  read_imagei(__read_only image1d_array_t image, sampler_t sampler, int2 coord);
int4 __attribute__((const)) __attribute__((overloadable))  read_imagei(__read_only image1d_array_t image, sampler_t sampler, float2 coord);
uint4 __attribute__((const)) __attribute__((overloadable))  read_imageui(__read_only image1d_array_t image, sampler_t sampler, int2 coord);
uint4 __attribute__((const)) __attribute__((overloadable))  read_imageui(__read_only image1d_array_t image, sampler_t sampler, float2 coord);
float4 __attribute__((const)) __attribute__((overloadable))  read_imagef (__read_only image2d_t image, int2 coord);
int4 __attribute__((const)) __attribute__((overloadable))  read_imagei (__read_only image2d_t image, int2 coord);
uint4 __attribute__((const)) __attribute__((overloadable))  read_imageui (__read_only image2d_t image, int2 coord);
float4 __attribute__((const)) __attribute__((overloadable))  read_imagef (__read_only image3d_t image, int4 coord);
int4 __attribute__((const)) __attribute__((overloadable))  read_imagei (__read_only image3d_t image, int4 coord);
uint4 __attribute__((const)) __attribute__((overloadable))  read_imageui (__read_only image3d_t image, int4 coord);
float4 __attribute__((const)) __attribute__((overloadable))  read_imagef (__read_only image2d_array_t image, int4 coord);
int4 __attribute__((const)) __attribute__((overloadable))  read_imagei (__read_only image2d_array_t image, int4 coord);
uint4 __attribute__((const)) __attribute__((overloadable))  read_imageui (__read_only image2d_array_t image, int4 coord);
float4 __attribute__((const)) __attribute__((overloadable))  read_imagef (__read_only image1d_t image, int coord);
float4 __attribute__((const)) __attribute__((overloadable))  read_imagef (__read_only image1d_buffer_t image, int coord);
int4 __attribute__((const)) __attribute__((overloadable))  read_imagei(__read_only image1d_t image, int coord);
uint4 __attribute__((const)) __attribute__((overloadable))  read_imageui(__read_only image1d_t image, int coord);
int4 __attribute__((const)) __attribute__((overloadable))  read_imagei(__read_only image1d_buffer_t image, int coord);
uint4 __attribute__((const)) __attribute__((overloadable))  read_imageui(__read_only image1d_buffer_t image, int coord);
float4 __attribute__((const)) __attribute__((overloadable))  read_imagef(__read_only image1d_array_t image, int2 coord);
int4 __attribute__((const)) __attribute__((overloadable))  read_imagei(__read_only image1d_array_t image, int2 coord);
uint4 __attribute__((const)) __attribute__((overloadable))  read_imageui(__read_only image1d_array_t image, int2 coord);
void __attribute__((overloadable)) write_imagef (__write_only image2d_array_t image, int4 coord, float4 color);
void __attribute__((overloadable)) write_imagei (__write_only image2d_array_t image, int4 coord, int4 color);
void __attribute__((overloadable)) write_imageui (__write_only image2d_array_t image, int4 coord, uint4 color);
void __attribute__((overloadable)) write_imagef (__write_only image1d_t image, int coord, float4 color);
void __attribute__((overloadable)) write_imagei (__write_only image1d_t image, int coord, int4 color);
void __attribute__((overloadable)) write_imageui (__write_only image1d_t image, int coord, uint4 color);
void __attribute__((overloadable)) write_imagef (__write_only image1d_buffer_t image, int coord, float4 color);
void __attribute__((overloadable)) write_imagei (__write_only image1d_buffer_t image, int coord, int4 color);
void __attribute__((overloadable)) write_imageui (__write_only image1d_buffer_t image, int coord, uint4 color);
void __attribute__((overloadable)) write_imagef (__write_only image1d_array_t image, int2 coord, float4 color);
void __attribute__((overloadable)) write_imagei (__write_only image1d_array_t image, int2 coord, int4 color);
void __attribute__((overloadable)) write_imageui (__write_only image1d_array_t image, int2 coord, uint4 color);



/**
 * Return the image width in pixels.
 */
int __attribute__((const)) __attribute__((overloadable)) get_image_width(image1d_t image);
int __attribute__((const)) __attribute__((overloadable)) get_image_width(image1d_buffer_t image);
int __attribute__((const)) __attribute__((overloadable)) get_image_width(image1d_array_t image);
int __attribute__((const)) __attribute__((overloadable)) get_image_width(image2d_array_t image);

/**
 * Return the image height in pixels.
 */
int __attribute__((const)) __attribute__((overloadable)) get_image_height(image2d_array_t image);


/**
 * Return the channel data type. Valid values are:
 * CLK_SNORM_INT8
 * CLK_SNORM_INT16
 * CLK_UNORM_INT8
 * CLK_UNORM_INT16
 * CLK_UNORM_SHORT_565
 * CLK_UNORM_SHORT_555
 * CLK_UNORM_SHORT_101010
 * CLK_SIGNED_INT8
 * CLK_SIGNED_INT16
 * CLK_SIGNED_INT32
 * CLK_UNSIGNED_INT8
 * CLK_UNSIGNED_INT16
 * CLK_UNSIGNED_INT32
 * CLK_HALF_FLOAT
 * CLK_FLOAT
 */

// OpenCL2.0 image formats beyond SPIR 1.2 spec


int __attribute__((const)) __attribute__((overloadable)) get_image_channel_data_type(image1d_t image);
int __attribute__((const)) __attribute__((overloadable)) get_image_channel_data_type(image1d_buffer_t image);
int __attribute__((const)) __attribute__((overloadable)) get_image_channel_data_type(image1d_array_t image);
int __attribute__((const)) __attribute__((overloadable)) get_image_channel_data_type(image2d_array_t image);

/**
 * Return the image channel order. Valid values are:
 * CLK_A
 * CLK_R
 * CLK_Rx
 * CLK_RG
 * CLK_RGx
 * CLK_RA
 * CLK_RGB
 * CLK_RGBx
 * CLK_RGBA
 * CLK_ARGB
 * CLK_BGRA
 * CLK_INTENSITY
 * CLK_LUMINANCE
 */
int __attribute__((const)) __attribute__((overloadable)) get_image_channel_order(image1d_t image);
int __attribute__((const)) __attribute__((overloadable)) get_image_channel_order(image1d_buffer_t image);
int __attribute__((const)) __attribute__((overloadable)) get_image_channel_order(image1d_array_t image);
int __attribute__((const)) __attribute__((overloadable)) get_image_channel_order(image2d_array_t image);

/**
 * Return the 2D image width and height as an int2
 * type. The width is returned in the x component, and
 * the height in the y component.
 */
int2 __attribute__((const)) __attribute__((overloadable)) get_image_dim(image2d_array_t image);


/**
 *  Use coord.xy to do an element lookup in the 2D image layer identified by index coord.z in the 2D image array.
 */
float4  __attribute__((const)) __attribute__((overloadable)) read_imagef(__read_only image2d_array_t image_array, sampler_t sampler, int4 coord);

/**
 * Use coord.xy to do an element lookup in the 2D image layer identified by index coord.z in the 2D image array.
 */
float4  __attribute__((const)) __attribute__((overloadable)) read_imagef(__read_only image2d_array_t image_array, sampler_t sampler, float4 coord);

/**
 * Use coord.xy to do an element lookup in the 2D image layer identified by index coord.z in the 2D image array.
 */
int4  __attribute__((const)) __attribute__((overloadable)) read_imagei(__read_only image2d_array_t image_array, sampler_t sampler, int4 coord);

/**
 * Use coord.xy to do an element lookup in the 2D image layer identified by index coord.z in the 2D image array.
 */
int4  __attribute__((const)) __attribute__((overloadable)) read_imagei(__read_only image2d_array_t image_array, sampler_t sampler, float4 coord);

/**
 * Use coord.xy to do an element lookup in the 2D image layer identified by index coord.z in the 2D image array.
 */
uint4  __attribute__((const)) __attribute__((overloadable)) read_imageui(__read_only image2d_array_t image_array, sampler_t sampler, int4 coord);

/**
 * Use coord.xy to do an element lookup in the 2D image layer identified by index coord.z in the 2D image array.
 */
uint4  __attribute__((const)) __attribute__((overloadable)) read_imageui(__read_only image2d_array_t image_array, sampler_t sampler, float4 coord);

/**
 * Write color value to location specified by coord.xy in the 2D image layer identified by index coord.z in the 2D image array.
 */
void __attribute__((overloadable)) write_imagef(__write_only image2d_array_t image_array, int4 coord, float4 color);

/**
 * Write color value to location specified by coord.xy in the 2D image layer identified by index coord.z in the 2D image array.
 */
void __attribute__((overloadable)) write_imagei(__write_only image2d_array_t image_array, int4 coord, int4 color);

/**
 * Write color value to location specified by coord.xy in the 2D image layer identified by index coord.z in the 2D image array.
 */
void __attribute__((overloadable)) write_imageui(__write_only image2d_array_t image_array, int4 coord, uint4 color);

/**
 * OpenCL as_typen operators
 * Reinterprets a data type as another data type of the same size
 */
char  __attribute__((const)) __attribute__((overloadable)) convert_char_rte(double);
char  __attribute__((const)) __attribute__((overloadable)) convert_char_sat_rte(double);
char  __attribute__((const)) __attribute__((overloadable)) convert_char_rtz(double);
char  __attribute__((const)) __attribute__((overloadable)) convert_char_sat_rtz(double);
char  __attribute__((const)) __attribute__((overloadable)) convert_char_rtp(double);
char  __attribute__((const)) __attribute__((overloadable)) convert_char_sat_rtp(double);
char  __attribute__((const)) __attribute__((overloadable)) convert_char_rtn(double);
char  __attribute__((const)) __attribute__((overloadable)) convert_char_sat_rtn(double);
char  __attribute__((const)) __attribute__((overloadable)) convert_char(double);
char  __attribute__((const)) __attribute__((overloadable)) convert_char_sat(double);
uchar  __attribute__((const)) __attribute__((overloadable)) convert_uchar_rte(double);
uchar  __attribute__((const)) __attribute__((overloadable)) convert_uchar_sat_rte(double);
uchar  __attribute__((const)) __attribute__((overloadable)) convert_uchar_rtz(double);
uchar  __attribute__((const)) __attribute__((overloadable)) convert_uchar_sat_rtz(double);
uchar  __attribute__((const)) __attribute__((overloadable)) convert_uchar_rtp(double);
uchar  __attribute__((const)) __attribute__((overloadable)) convert_uchar_sat_rtp(double);
uchar  __attribute__((const)) __attribute__((overloadable)) convert_uchar_rtn(double);
uchar  __attribute__((const)) __attribute__((overloadable)) convert_uchar_sat_rtn(double);
uchar  __attribute__((const)) __attribute__((overloadable)) convert_uchar(double);
uchar  __attribute__((const)) __attribute__((overloadable)) convert_uchar_sat(double);

short  __attribute__((const)) __attribute__((overloadable)) convert_short_rte(double);
short  __attribute__((const)) __attribute__((overloadable)) convert_short_sat_rte(double);
short  __attribute__((const)) __attribute__((overloadable)) convert_short_rtz(double);
short  __attribute__((const)) __attribute__((overloadable)) convert_short_sat_rtz(double);
short  __attribute__((const)) __attribute__((overloadable)) convert_short_rtp(double);
short  __attribute__((const)) __attribute__((overloadable)) convert_short_sat_rtp(double);
short  __attribute__((const)) __attribute__((overloadable)) convert_short_rtn(double);
short  __attribute__((const)) __attribute__((overloadable)) convert_short_sat_rtn(double);
short  __attribute__((const)) __attribute__((overloadable)) convert_short(double);
short  __attribute__((const)) __attribute__((overloadable)) convert_short_sat(double);

ushort  __attribute__((const)) __attribute__((overloadable)) convert_ushort_rte(double);
ushort  __attribute__((const)) __attribute__((overloadable)) convert_ushort_sat_rte(double);
ushort  __attribute__((const)) __attribute__((overloadable)) convert_ushort_rtz(double);
ushort  __attribute__((const)) __attribute__((overloadable)) convert_ushort_sat_rtz(double);
ushort  __attribute__((const)) __attribute__((overloadable)) convert_ushort_rtp(double);
ushort  __attribute__((const)) __attribute__((overloadable)) convert_ushort_sat_rtp(double);
ushort  __attribute__((const)) __attribute__((overloadable)) convert_ushort_rtn(double);
ushort  __attribute__((const)) __attribute__((overloadable)) convert_ushort_sat_rtn(double);
ushort  __attribute__((const)) __attribute__((overloadable)) convert_ushort(double);
ushort  __attribute__((const)) __attribute__((overloadable)) convert_ushort_sat(double);

int  __attribute__((const)) __attribute__((overloadable)) convert_int_rte(double);
int  __attribute__((const)) __attribute__((overloadable)) convert_int_sat_rte(double);
int  __attribute__((const)) __attribute__((overloadable)) convert_int_rtz(double);
int  __attribute__((const)) __attribute__((overloadable)) convert_int_sat_rtz(double);
int  __attribute__((const)) __attribute__((overloadable)) convert_int_rtp(double);
int  __attribute__((const)) __attribute__((overloadable)) convert_int_sat_rtp(double);
int  __attribute__((const)) __attribute__((overloadable)) convert_int_rtn(double);
int  __attribute__((const)) __attribute__((overloadable)) convert_int_sat_rtn(double);
int  __attribute__((const)) __attribute__((overloadable)) convert_int(double);
int  __attribute__((const)) __attribute__((overloadable)) convert_int_sat(double);

uint  __attribute__((const)) __attribute__((overloadable)) convert_uint_rte(double);
uint  __attribute__((const)) __attribute__((overloadable)) convert_uint_sat_rte(double);
uint  __attribute__((const)) __attribute__((overloadable)) convert_uint_rtz(double);
uint  __attribute__((const)) __attribute__((overloadable)) convert_uint_sat_rtz(double);
uint  __attribute__((const)) __attribute__((overloadable)) convert_uint_rtp(double);
uint  __attribute__((const)) __attribute__((overloadable)) convert_uint_sat_rtp(double);
uint  __attribute__((const)) __attribute__((overloadable)) convert_uint_rtn(double);
uint  __attribute__((const)) __attribute__((overloadable)) convert_uint_sat_rtn(double);
uint  __attribute__((const)) __attribute__((overloadable)) convert_uint(double);
uint  __attribute__((const)) __attribute__((overloadable)) convert_uint_sat(double);
long  __attribute__((const)) __attribute__((overloadable)) convert_long_rte(double);
long  __attribute__((const)) __attribute__((overloadable)) convert_long_sat_rte(double);
long  __attribute__((const)) __attribute__((overloadable)) convert_long_rtz(double);
long  __attribute__((const)) __attribute__((overloadable)) convert_long_sat_rtz(double);
long  __attribute__((const)) __attribute__((overloadable)) convert_long_rtp(double);
long  __attribute__((const)) __attribute__((overloadable)) convert_long_sat_rtp(double);
long  __attribute__((const)) __attribute__((overloadable)) convert_long_rtn(double);
long  __attribute__((const)) __attribute__((overloadable)) convert_long_sat_rtn(double);
long  __attribute__((const)) __attribute__((overloadable)) convert_long(double);
long  __attribute__((const)) __attribute__((overloadable)) convert_long_sat(double);

ulong  __attribute__((const)) __attribute__((overloadable)) convert_ulong_rte(double);
ulong  __attribute__((const)) __attribute__((overloadable)) convert_ulong_sat_rte(double);
ulong  __attribute__((const)) __attribute__((overloadable)) convert_ulong_rtz(double);
ulong  __attribute__((const)) __attribute__((overloadable)) convert_ulong_sat_rtz(double);
ulong  __attribute__((const)) __attribute__((overloadable)) convert_ulong_rtp(double);
ulong  __attribute__((const)) __attribute__((overloadable)) convert_ulong_sat_rtp(double);
ulong  __attribute__((const)) __attribute__((overloadable)) convert_ulong_rtn(double);
ulong  __attribute__((const)) __attribute__((overloadable)) convert_ulong_sat_rtn(double);
ulong  __attribute__((const)) __attribute__((overloadable)) convert_ulong(double);
ulong  __attribute__((const)) __attribute__((overloadable)) convert_ulong_sat(double);

float  __attribute__((const)) __attribute__((overloadable)) convert_float_rte(double);
float  __attribute__((const)) __attribute__((overloadable)) convert_float_rtz(double);
float  __attribute__((const)) __attribute__((overloadable)) convert_float_rtp(double);
float  __attribute__((const)) __attribute__((overloadable)) convert_float_rtn(double);
float  __attribute__((const)) __attribute__((overloadable)) convert_float(double);

double  __attribute__((const)) __attribute__((overloadable)) convert_double_rte(char);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtz(char);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtp(char);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtn(char);
double  __attribute__((const)) __attribute__((overloadable)) convert_double(char);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rte(uchar);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtz(uchar);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtp(uchar);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtn(uchar);
double  __attribute__((const)) __attribute__((overloadable)) convert_double(uchar);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rte(short);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtz(short);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtp(short);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtn(short);
double  __attribute__((const)) __attribute__((overloadable)) convert_double(short);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rte(ushort);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtz(ushort);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtp(ushort);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtn(ushort);
double  __attribute__((const)) __attribute__((overloadable)) convert_double(ushort);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rte(int);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtz(int);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtp(int);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtn(int);
double  __attribute__((const)) __attribute__((overloadable)) convert_double(int);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rte(uint);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtz(uint);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtp(uint);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtn(uint);
double  __attribute__((const)) __attribute__((overloadable)) convert_double(uint);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rte(long);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtz(long);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtp(long);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtn(long);
double  __attribute__((const)) __attribute__((overloadable)) convert_double(long);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rte(ulong);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtz(ulong);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtp(ulong);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtn(ulong);
double  __attribute__((const)) __attribute__((overloadable)) convert_double(ulong);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rte(float);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtz(float);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtp(float);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtn(float);
double  __attribute__((const)) __attribute__((overloadable)) convert_double(float);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rte(double);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtz(double);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtp(double);
double  __attribute__((const)) __attribute__((overloadable)) convert_double_rtn(double);
double  __attribute__((const)) __attribute__((overloadable)) convert_double(double);

char2  __attribute__((const)) __attribute__((overloadable)) convert_char2_rte(double2);
char2  __attribute__((const)) __attribute__((overloadable)) convert_char2_sat_rte(double2);
char2  __attribute__((const)) __attribute__((overloadable)) convert_char2_rtz(double2);
char2  __attribute__((const)) __attribute__((overloadable)) convert_char2_sat_rtz(double2);
char2  __attribute__((const)) __attribute__((overloadable)) convert_char2_rtp(double2);
char2  __attribute__((const)) __attribute__((overloadable)) convert_char2_sat_rtp(double2);
char2  __attribute__((const)) __attribute__((overloadable)) convert_char2_rtn(double2);
char2  __attribute__((const)) __attribute__((overloadable)) convert_char2_sat_rtn(double2);
char2  __attribute__((const)) __attribute__((overloadable)) convert_char2(double2);
char2  __attribute__((const)) __attribute__((overloadable)) convert_char2_sat(double2);

uchar2  __attribute__((const)) __attribute__((overloadable)) convert_uchar2_rte(double2);
uchar2  __attribute__((const)) __attribute__((overloadable)) convert_uchar2_sat_rte(double2);
uchar2  __attribute__((const)) __attribute__((overloadable)) convert_uchar2_rtz(double2);
uchar2  __attribute__((const)) __attribute__((overloadable)) convert_uchar2_sat_rtz(double2);
uchar2  __attribute__((const)) __attribute__((overloadable)) convert_uchar2_rtp(double2);
uchar2  __attribute__((const)) __attribute__((overloadable)) convert_uchar2_sat_rtp(double2);
uchar2  __attribute__((const)) __attribute__((overloadable)) convert_uchar2_rtn(double2);
uchar2  __attribute__((const)) __attribute__((overloadable)) convert_uchar2_sat_rtn(double2);
uchar2  __attribute__((const)) __attribute__((overloadable)) convert_uchar2(double2);
uchar2  __attribute__((const)) __attribute__((overloadable)) convert_uchar2_sat(double2);

short2  __attribute__((const)) __attribute__((overloadable)) convert_short2_rte(double2);
short2  __attribute__((const)) __attribute__((overloadable)) convert_short2_sat_rte(double2);
short2  __attribute__((const)) __attribute__((overloadable)) convert_short2_rtz(double2);
short2  __attribute__((const)) __attribute__((overloadable)) convert_short2_sat_rtz(double2);
short2  __attribute__((const)) __attribute__((overloadable)) convert_short2_rtp(double2);
short2  __attribute__((const)) __attribute__((overloadable)) convert_short2_sat_rtp(double2);
short2  __attribute__((const)) __attribute__((overloadable)) convert_short2_rtn(double2);
short2  __attribute__((const)) __attribute__((overloadable)) convert_short2_sat_rtn(double2);
short2  __attribute__((const)) __attribute__((overloadable)) convert_short2(double2);
short2  __attribute__((const)) __attribute__((overloadable)) convert_short2_sat(double2);

ushort2  __attribute__((const)) __attribute__((overloadable)) convert_ushort2_rte(double2);
ushort2  __attribute__((const)) __attribute__((overloadable)) convert_ushort2_sat_rte(double2);
ushort2  __attribute__((const)) __attribute__((overloadable)) convert_ushort2_rtz(double2);
ushort2  __attribute__((const)) __attribute__((overloadable)) convert_ushort2_sat_rtz(double2);
ushort2  __attribute__((const)) __attribute__((overloadable)) convert_ushort2_rtp(double2);
ushort2  __attribute__((const)) __attribute__((overloadable)) convert_ushort2_sat_rtp(double2);
ushort2  __attribute__((const)) __attribute__((overloadable)) convert_ushort2_rtn(double2);
ushort2  __attribute__((const)) __attribute__((overloadable)) convert_ushort2_sat_rtn(double2);
ushort2  __attribute__((const)) __attribute__((overloadable)) convert_ushort2(double2);
ushort2  __attribute__((const)) __attribute__((overloadable)) convert_ushort2_sat(double2);

int2  __attribute__((const)) __attribute__((overloadable)) convert_int2_rte(double2);
int2  __attribute__((const)) __attribute__((overloadable)) convert_int2_sat_rte(double2);
int2  __attribute__((const)) __attribute__((overloadable)) convert_int2_rtz(double2);
int2  __attribute__((const)) __attribute__((overloadable)) convert_int2_sat_rtz(double2);
int2  __attribute__((const)) __attribute__((overloadable)) convert_int2_rtp(double2);
int2  __attribute__((const)) __attribute__((overloadable)) convert_int2_sat_rtp(double2);
int2  __attribute__((const)) __attribute__((overloadable)) convert_int2_rtn(double2);
int2  __attribute__((const)) __attribute__((overloadable)) convert_int2_sat_rtn(double2);
int2  __attribute__((const)) __attribute__((overloadable)) convert_int2(double2);
int2  __attribute__((const)) __attribute__((overloadable)) convert_int2_sat(double2);

uint2  __attribute__((const)) __attribute__((overloadable)) convert_uint2_rte(double2);
uint2  __attribute__((const)) __attribute__((overloadable)) convert_uint2_sat_rte(double2);
uint2  __attribute__((const)) __attribute__((overloadable)) convert_uint2_rtz(double2);
uint2  __attribute__((const)) __attribute__((overloadable)) convert_uint2_sat_rtz(double2);
uint2  __attribute__((const)) __attribute__((overloadable)) convert_uint2_rtp(double2);
uint2  __attribute__((const)) __attribute__((overloadable)) convert_uint2_sat_rtp(double2);
uint2  __attribute__((const)) __attribute__((overloadable)) convert_uint2_rtn(double2);
uint2  __attribute__((const)) __attribute__((overloadable)) convert_uint2_sat_rtn(double2);
uint2  __attribute__((const)) __attribute__((overloadable)) convert_uint2(double2);
uint2  __attribute__((const)) __attribute__((overloadable)) convert_uint2_sat(double2);
long2  __attribute__((const)) __attribute__((overloadable)) convert_long2_rte(double2);
long2  __attribute__((const)) __attribute__((overloadable)) convert_long2_sat_rte(double2);
long2  __attribute__((const)) __attribute__((overloadable)) convert_long2_rtz(double2);
long2  __attribute__((const)) __attribute__((overloadable)) convert_long2_sat_rtz(double2);
long2  __attribute__((const)) __attribute__((overloadable)) convert_long2_rtp(double2);
long2  __attribute__((const)) __attribute__((overloadable)) convert_long2_sat_rtp(double2);
long2  __attribute__((const)) __attribute__((overloadable)) convert_long2_rtn(double2);
long2  __attribute__((const)) __attribute__((overloadable)) convert_long2_sat_rtn(double2);
long2  __attribute__((const)) __attribute__((overloadable)) convert_long2(double2);
long2  __attribute__((const)) __attribute__((overloadable)) convert_long2_sat(double2);

ulong2  __attribute__((const)) __attribute__((overloadable)) convert_ulong2_rte(double2);
ulong2  __attribute__((const)) __attribute__((overloadable)) convert_ulong2_sat_rte(double2);
ulong2  __attribute__((const)) __attribute__((overloadable)) convert_ulong2_rtz(double2);
ulong2  __attribute__((const)) __attribute__((overloadable)) convert_ulong2_sat_rtz(double2);
ulong2  __attribute__((const)) __attribute__((overloadable)) convert_ulong2_rtp(double2);
ulong2  __attribute__((const)) __attribute__((overloadable)) convert_ulong2_sat_rtp(double2);
ulong2  __attribute__((const)) __attribute__((overloadable)) convert_ulong2_rtn(double2);
ulong2  __attribute__((const)) __attribute__((overloadable)) convert_ulong2_sat_rtn(double2);
ulong2  __attribute__((const)) __attribute__((overloadable)) convert_ulong2(double2);
ulong2  __attribute__((const)) __attribute__((overloadable)) convert_ulong2_sat(double2);

float2  __attribute__((const)) __attribute__((overloadable)) convert_float2_rte(double2);
float2  __attribute__((const)) __attribute__((overloadable)) convert_float2_rtz(double2);
float2  __attribute__((const)) __attribute__((overloadable)) convert_float2_rtp(double2);
float2  __attribute__((const)) __attribute__((overloadable)) convert_float2_rtn(double2);
float2  __attribute__((const)) __attribute__((overloadable)) convert_float2(double2);

double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rte(char2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtz(char2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtp(char2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtn(char2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2(char2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rte(uchar2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtz(uchar2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtp(uchar2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtn(uchar2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2(uchar2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rte(short2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtz(short2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtp(short2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtn(short2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2(short2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rte(ushort2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtz(ushort2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtp(ushort2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtn(ushort2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2(ushort2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rte(int2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtz(int2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtp(int2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtn(int2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2(int2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rte(uint2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtz(uint2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtp(uint2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtn(uint2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2(uint2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rte(long2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtz(long2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtp(long2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtn(long2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2(long2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rte(ulong2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtz(ulong2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtp(ulong2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtn(ulong2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2(ulong2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rte(float2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtz(float2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtp(float2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtn(float2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2(float2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rte(double2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtz(double2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtp(double2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2_rtn(double2);
double2  __attribute__((const)) __attribute__((overloadable)) convert_double2(double2);

char3  __attribute__((const)) __attribute__((overloadable)) convert_char3_rte(double3);
char3  __attribute__((const)) __attribute__((overloadable)) convert_char3_sat_rte(double3);
char3  __attribute__((const)) __attribute__((overloadable)) convert_char3_rtz(double3);
char3  __attribute__((const)) __attribute__((overloadable)) convert_char3_sat_rtz(double3);
char3  __attribute__((const)) __attribute__((overloadable)) convert_char3_rtp(double3);
char3  __attribute__((const)) __attribute__((overloadable)) convert_char3_sat_rtp(double3);
char3  __attribute__((const)) __attribute__((overloadable)) convert_char3_rtn(double3);
char3  __attribute__((const)) __attribute__((overloadable)) convert_char3_sat_rtn(double3);
char3  __attribute__((const)) __attribute__((overloadable)) convert_char3(double3);
char3  __attribute__((const)) __attribute__((overloadable)) convert_char3_sat(double3);

uchar3  __attribute__((const)) __attribute__((overloadable)) convert_uchar3_rte(double3);
uchar3  __attribute__((const)) __attribute__((overloadable)) convert_uchar3_sat_rte(double3);
uchar3  __attribute__((const)) __attribute__((overloadable)) convert_uchar3_rtz(double3);
uchar3  __attribute__((const)) __attribute__((overloadable)) convert_uchar3_sat_rtz(double3);
uchar3  __attribute__((const)) __attribute__((overloadable)) convert_uchar3_rtp(double3);
uchar3  __attribute__((const)) __attribute__((overloadable)) convert_uchar3_sat_rtp(double3);
uchar3  __attribute__((const)) __attribute__((overloadable)) convert_uchar3_rtn(double3);
uchar3  __attribute__((const)) __attribute__((overloadable)) convert_uchar3_sat_rtn(double3);
uchar3  __attribute__((const)) __attribute__((overloadable)) convert_uchar3(double3);
uchar3  __attribute__((const)) __attribute__((overloadable)) convert_uchar3_sat(double3);

short3  __attribute__((const)) __attribute__((overloadable)) convert_short3_rte(double3);
short3  __attribute__((const)) __attribute__((overloadable)) convert_short3_sat_rte(double3);
short3  __attribute__((const)) __attribute__((overloadable)) convert_short3_rtz(double3);
short3  __attribute__((const)) __attribute__((overloadable)) convert_short3_sat_rtz(double3);
short3  __attribute__((const)) __attribute__((overloadable)) convert_short3_rtp(double3);
short3  __attribute__((const)) __attribute__((overloadable)) convert_short3_sat_rtp(double3);
short3  __attribute__((const)) __attribute__((overloadable)) convert_short3_rtn(double3);
short3  __attribute__((const)) __attribute__((overloadable)) convert_short3_sat_rtn(double3);
short3  __attribute__((const)) __attribute__((overloadable)) convert_short3(double3);
short3  __attribute__((const)) __attribute__((overloadable)) convert_short3_sat(double3);

ushort3  __attribute__((const)) __attribute__((overloadable)) convert_ushort3_rte(double3);
ushort3  __attribute__((const)) __attribute__((overloadable)) convert_ushort3_sat_rte(double3);
ushort3  __attribute__((const)) __attribute__((overloadable)) convert_ushort3_rtz(double3);
ushort3  __attribute__((const)) __attribute__((overloadable)) convert_ushort3_sat_rtz(double3);
ushort3  __attribute__((const)) __attribute__((overloadable)) convert_ushort3_rtp(double3);
ushort3  __attribute__((const)) __attribute__((overloadable)) convert_ushort3_sat_rtp(double3);
ushort3  __attribute__((const)) __attribute__((overloadable)) convert_ushort3_rtn(double3);
ushort3  __attribute__((const)) __attribute__((overloadable)) convert_ushort3_sat_rtn(double3);
ushort3  __attribute__((const)) __attribute__((overloadable)) convert_ushort3(double3);
ushort3  __attribute__((const)) __attribute__((overloadable)) convert_ushort3_sat(double3);

int3  __attribute__((const)) __attribute__((overloadable)) convert_int3_rte(double3);
int3  __attribute__((const)) __attribute__((overloadable)) convert_int3_sat_rte(double3);
int3  __attribute__((const)) __attribute__((overloadable)) convert_int3_rtz(double3);
int3  __attribute__((const)) __attribute__((overloadable)) convert_int3_sat_rtz(double3);
int3  __attribute__((const)) __attribute__((overloadable)) convert_int3_rtp(double3);
int3  __attribute__((const)) __attribute__((overloadable)) convert_int3_sat_rtp(double3);
int3  __attribute__((const)) __attribute__((overloadable)) convert_int3_rtn(double3);
int3  __attribute__((const)) __attribute__((overloadable)) convert_int3_sat_rtn(double3);
int3  __attribute__((const)) __attribute__((overloadable)) convert_int3(double3);
int3  __attribute__((const)) __attribute__((overloadable)) convert_int3_sat(double3);

uint3  __attribute__((const)) __attribute__((overloadable)) convert_uint3_rte(double3);
uint3  __attribute__((const)) __attribute__((overloadable)) convert_uint3_sat_rte(double3);
uint3  __attribute__((const)) __attribute__((overloadable)) convert_uint3_rtz(double3);
uint3  __attribute__((const)) __attribute__((overloadable)) convert_uint3_sat_rtz(double3);
uint3  __attribute__((const)) __attribute__((overloadable)) convert_uint3_rtp(double3);
uint3  __attribute__((const)) __attribute__((overloadable)) convert_uint3_sat_rtp(double3);
uint3  __attribute__((const)) __attribute__((overloadable)) convert_uint3_rtn(double3);
uint3  __attribute__((const)) __attribute__((overloadable)) convert_uint3_sat_rtn(double3);
uint3  __attribute__((const)) __attribute__((overloadable)) convert_uint3(double3);
uint3  __attribute__((const)) __attribute__((overloadable)) convert_uint3_sat(double3);
long3  __attribute__((const)) __attribute__((overloadable)) convert_long3_rte(double3);
long3  __attribute__((const)) __attribute__((overloadable)) convert_long3_sat_rte(double3);
long3  __attribute__((const)) __attribute__((overloadable)) convert_long3_rtz(double3);
long3  __attribute__((const)) __attribute__((overloadable)) convert_long3_sat_rtz(double3);
long3  __attribute__((const)) __attribute__((overloadable)) convert_long3_rtp(double3);
long3  __attribute__((const)) __attribute__((overloadable)) convert_long3_sat_rtp(double3);
long3  __attribute__((const)) __attribute__((overloadable)) convert_long3_rtn(double3);
long3  __attribute__((const)) __attribute__((overloadable)) convert_long3_sat_rtn(double3);
long3  __attribute__((const)) __attribute__((overloadable)) convert_long3(double3);
long3  __attribute__((const)) __attribute__((overloadable)) convert_long3_sat(double3);
ulong3  __attribute__((const)) __attribute__((overloadable)) convert_ulong3_rte(double3);
ulong3  __attribute__((const)) __attribute__((overloadable)) convert_ulong3_sat_rte(double3);
ulong3  __attribute__((const)) __attribute__((overloadable)) convert_ulong3_rtz(double3);
ulong3  __attribute__((const)) __attribute__((overloadable)) convert_ulong3_sat_rtz(double3);
ulong3  __attribute__((const)) __attribute__((overloadable)) convert_ulong3_rtp(double3);
ulong3  __attribute__((const)) __attribute__((overloadable)) convert_ulong3_sat_rtp(double3);
ulong3  __attribute__((const)) __attribute__((overloadable)) convert_ulong3_rtn(double3);
ulong3  __attribute__((const)) __attribute__((overloadable)) convert_ulong3_sat_rtn(double3);
ulong3  __attribute__((const)) __attribute__((overloadable)) convert_ulong3(double3);
ulong3  __attribute__((const)) __attribute__((overloadable)) convert_ulong3_sat(double3);
float3  __attribute__((const)) __attribute__((overloadable)) convert_float3_rte(double3);
float3  __attribute__((const)) __attribute__((overloadable)) convert_float3_rtz(double3);
float3  __attribute__((const)) __attribute__((overloadable)) convert_float3_rtp(double3);
float3  __attribute__((const)) __attribute__((overloadable)) convert_float3_rtn(double3);
float3  __attribute__((const)) __attribute__((overloadable)) convert_float3(double3);
ulong3  __attribute__((const)) __attribute__((overloadable)) convert_ulong3_sat_rtz(double3);

double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rte(char3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtz(char3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtp(char3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtn(char3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3(char3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rte(uchar3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtz(uchar3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtp(uchar3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtn(uchar3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3(uchar3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rte(short3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtz(short3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtp(short3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtn(short3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3(short3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rte(ushort3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtz(ushort3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtp(ushort3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtn(ushort3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3(ushort3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rte(int3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtz(int3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtp(int3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtn(int3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3(int3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rte(uint3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtz(uint3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtp(uint3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtn(uint3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3(uint3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rte(long3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtz(long3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtp(long3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtn(long3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3(long3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rte(ulong3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtz(ulong3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtp(ulong3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtn(ulong3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3(ulong3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rte(float3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtz(float3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtp(float3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtn(float3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3(float3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rte(double3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtz(double3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtp(double3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3_rtn(double3);
double3  __attribute__((const)) __attribute__((overloadable)) convert_double3(double3);

char4  __attribute__((const)) __attribute__((overloadable)) convert_char4_rte(double4);
char4  __attribute__((const)) __attribute__((overloadable)) convert_char4_sat_rte(double4);
char4  __attribute__((const)) __attribute__((overloadable)) convert_char4_rtz(double4);
char4  __attribute__((const)) __attribute__((overloadable)) convert_char4_sat_rtz(double4);
char4  __attribute__((const)) __attribute__((overloadable)) convert_char4_rtp(double4);
char4  __attribute__((const)) __attribute__((overloadable)) convert_char4_sat_rtp(double4);
char4  __attribute__((const)) __attribute__((overloadable)) convert_char4_rtn(double4);
char4  __attribute__((const)) __attribute__((overloadable)) convert_char4_sat_rtn(double4);
char4  __attribute__((const)) __attribute__((overloadable)) convert_char4(double4);
char4  __attribute__((const)) __attribute__((overloadable)) convert_char4_sat(double4);

uchar4  __attribute__((const)) __attribute__((overloadable)) convert_uchar4_rte(double4);
uchar4  __attribute__((const)) __attribute__((overloadable)) convert_uchar4_sat_rte(double4);
uchar4  __attribute__((const)) __attribute__((overloadable)) convert_uchar4_rtz(double4);
uchar4  __attribute__((const)) __attribute__((overloadable)) convert_uchar4_sat_rtz(double4);
uchar4  __attribute__((const)) __attribute__((overloadable)) convert_uchar4_rtp(double4);
uchar4  __attribute__((const)) __attribute__((overloadable)) convert_uchar4_sat_rtp(double4);
uchar4  __attribute__((const)) __attribute__((overloadable)) convert_uchar4_rtn(double4);
uchar4  __attribute__((const)) __attribute__((overloadable)) convert_uchar4_sat_rtn(double4);
uchar4  __attribute__((const)) __attribute__((overloadable)) convert_uchar4(double4);
uchar4  __attribute__((const)) __attribute__((overloadable)) convert_uchar4_sat(double4);

short4  __attribute__((const)) __attribute__((overloadable)) convert_short4_rte(double4);
short4  __attribute__((const)) __attribute__((overloadable)) convert_short4_sat_rte(double4);
short4  __attribute__((const)) __attribute__((overloadable)) convert_short4_rtz(double4);
short4  __attribute__((const)) __attribute__((overloadable)) convert_short4_sat_rtz(double4);
short4  __attribute__((const)) __attribute__((overloadable)) convert_short4_rtp(double4);
short4  __attribute__((const)) __attribute__((overloadable)) convert_short4_sat_rtp(double4);
short4  __attribute__((const)) __attribute__((overloadable)) convert_short4_rtn(double4);
short4  __attribute__((const)) __attribute__((overloadable)) convert_short4_sat_rtn(double4);
short4  __attribute__((const)) __attribute__((overloadable)) convert_short4(double4);
short4  __attribute__((const)) __attribute__((overloadable)) convert_short4_sat(double4);

ushort4  __attribute__((const)) __attribute__((overloadable)) convert_ushort4_rte(double4);
ushort4  __attribute__((const)) __attribute__((overloadable)) convert_ushort4_sat_rte(double4);
ushort4  __attribute__((const)) __attribute__((overloadable)) convert_ushort4_rtz(double4);
ushort4  __attribute__((const)) __attribute__((overloadable)) convert_ushort4_sat_rtz(double4);
ushort4  __attribute__((const)) __attribute__((overloadable)) convert_ushort4_rtp(double4);
ushort4  __attribute__((const)) __attribute__((overloadable)) convert_ushort4_sat_rtp(double4);
ushort4  __attribute__((const)) __attribute__((overloadable)) convert_ushort4_rtn(double4);
ushort4  __attribute__((const)) __attribute__((overloadable)) convert_ushort4_sat_rtn(double4);
ushort4  __attribute__((const)) __attribute__((overloadable)) convert_ushort4(double4);
ushort4  __attribute__((const)) __attribute__((overloadable)) convert_ushort4_sat(double4);

int4  __attribute__((const)) __attribute__((overloadable)) convert_int4_rte(double4);
int4  __attribute__((const)) __attribute__((overloadable)) convert_int4_sat_rte(double4);
int4  __attribute__((const)) __attribute__((overloadable)) convert_int4_rtz(double4);
int4  __attribute__((const)) __attribute__((overloadable)) convert_int4_sat_rtz(double4);
int4  __attribute__((const)) __attribute__((overloadable)) convert_int4_rtp(double4);
int4  __attribute__((const)) __attribute__((overloadable)) convert_int4_sat_rtp(double4);
int4  __attribute__((const)) __attribute__((overloadable)) convert_int4_rtn(double4);
int4  __attribute__((const)) __attribute__((overloadable)) convert_int4_sat_rtn(double4);
int4  __attribute__((const)) __attribute__((overloadable)) convert_int4(double4);
int4  __attribute__((const)) __attribute__((overloadable)) convert_int4_sat(double4);

uint4  __attribute__((const)) __attribute__((overloadable)) convert_uint4_rte(double4);
uint4  __attribute__((const)) __attribute__((overloadable)) convert_uint4_sat_rte(double4);
uint4  __attribute__((const)) __attribute__((overloadable)) convert_uint4_rtz(double4);
uint4  __attribute__((const)) __attribute__((overloadable)) convert_uint4_sat_rtz(double4);
uint4  __attribute__((const)) __attribute__((overloadable)) convert_uint4_rtp(double4);
uint4  __attribute__((const)) __attribute__((overloadable)) convert_uint4_sat_rtp(double4);
uint4  __attribute__((const)) __attribute__((overloadable)) convert_uint4_rtn(double4);
uint4  __attribute__((const)) __attribute__((overloadable)) convert_uint4_sat_rtn(double4);
uint4  __attribute__((const)) __attribute__((overloadable)) convert_uint4(double4);
uint4  __attribute__((const)) __attribute__((overloadable)) convert_uint4_sat(double4);
long4  __attribute__((const)) __attribute__((overloadable)) convert_long4_rte(double4);
long4  __attribute__((const)) __attribute__((overloadable)) convert_long4_sat_rte(double4);
long4  __attribute__((const)) __attribute__((overloadable)) convert_long4_rtz(double4);
long4  __attribute__((const)) __attribute__((overloadable)) convert_long4_sat_rtz(double4);
long4  __attribute__((const)) __attribute__((overloadable)) convert_long4_rtp(double4);
long4  __attribute__((const)) __attribute__((overloadable)) convert_long4_sat_rtp(double4);
long4  __attribute__((const)) __attribute__((overloadable)) convert_long4_rtn(double4);
long4  __attribute__((const)) __attribute__((overloadable)) convert_long4_sat_rtn(double4);
long4  __attribute__((const)) __attribute__((overloadable)) convert_long4(double4);
long4  __attribute__((const)) __attribute__((overloadable)) convert_long4_sat(double4);

ulong4  __attribute__((const)) __attribute__((overloadable)) convert_ulong4_rte(double4);
ulong4  __attribute__((const)) __attribute__((overloadable)) convert_ulong4_sat_rte(double4);
ulong4  __attribute__((const)) __attribute__((overloadable)) convert_ulong4_rtz(double4);
ulong4  __attribute__((const)) __attribute__((overloadable)) convert_ulong4_sat_rtz(double4);
ulong4  __attribute__((const)) __attribute__((overloadable)) convert_ulong4_rtp(double4);
ulong4  __attribute__((const)) __attribute__((overloadable)) convert_ulong4_sat_rtp(double4);
ulong4  __attribute__((const)) __attribute__((overloadable)) convert_ulong4_rtn(double4);
ulong4  __attribute__((const)) __attribute__((overloadable)) convert_ulong4_sat_rtn(double4);
ulong4  __attribute__((const)) __attribute__((overloadable)) convert_ulong4(double4);
ulong4  __attribute__((const)) __attribute__((overloadable)) convert_ulong4_sat(double4);

float4  __attribute__((const)) __attribute__((overloadable)) convert_float4_rte(double4);
float4  __attribute__((const)) __attribute__((overloadable)) convert_float4_rtz(double4);
float4  __attribute__((const)) __attribute__((overloadable)) convert_float4_rtp(double4);
float4  __attribute__((const)) __attribute__((overloadable)) convert_float4_rtn(double4);
float4  __attribute__((const)) __attribute__((overloadable)) convert_float4(double4);

double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rte(char4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtz(char4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtp(char4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtn(char4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4(char4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rte(uchar4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtz(uchar4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtp(uchar4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtn(uchar4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4(uchar4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rte(short4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtz(short4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtp(short4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtn(short4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4(short4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rte(ushort4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtz(ushort4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtp(ushort4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtn(ushort4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4(ushort4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rte(int4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtz(int4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtp(int4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtn(int4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4(int4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rte(uint4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtz(uint4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtp(uint4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtn(uint4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4(uint4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rte(long4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtz(long4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtp(long4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtn(long4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4(long4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rte(ulong4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtz(ulong4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtp(ulong4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtn(ulong4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4(ulong4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rte(float4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtz(float4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtp(float4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtn(float4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4(float4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rte(double4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtz(double4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtp(double4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4_rtn(double4);
double4  __attribute__((const)) __attribute__((overloadable)) convert_double4(double4);

char8  __attribute__((const)) __attribute__((overloadable)) convert_char8_rte(double8);
char8  __attribute__((const)) __attribute__((overloadable)) convert_char8_sat_rte(double8);
char8  __attribute__((const)) __attribute__((overloadable)) convert_char8_rtz(double8);
char8  __attribute__((const)) __attribute__((overloadable)) convert_char8_sat_rtz(double8);
char8  __attribute__((const)) __attribute__((overloadable)) convert_char8_rtp(double8);
char8  __attribute__((const)) __attribute__((overloadable)) convert_char8_sat_rtp(double8);
char8  __attribute__((const)) __attribute__((overloadable)) convert_char8_rtn(double8);
char8  __attribute__((const)) __attribute__((overloadable)) convert_char8_sat_rtn(double8);
char8  __attribute__((const)) __attribute__((overloadable)) convert_char8(double8);
char8  __attribute__((const)) __attribute__((overloadable)) convert_char8_sat(double8);

uchar8  __attribute__((const)) __attribute__((overloadable)) convert_uchar8_rte(double8);
uchar8  __attribute__((const)) __attribute__((overloadable)) convert_uchar8_sat_rte(double8);
uchar8  __attribute__((const)) __attribute__((overloadable)) convert_uchar8_rtz(double8);
uchar8  __attribute__((const)) __attribute__((overloadable)) convert_uchar8_sat_rtz(double8);
uchar8  __attribute__((const)) __attribute__((overloadable)) convert_uchar8_rtp(double8);
uchar8  __attribute__((const)) __attribute__((overloadable)) convert_uchar8_sat_rtp(double8);
uchar8  __attribute__((const)) __attribute__((overloadable)) convert_uchar8_rtn(double8);
uchar8  __attribute__((const)) __attribute__((overloadable)) convert_uchar8_sat_rtn(double8);
uchar8  __attribute__((const)) __attribute__((overloadable)) convert_uchar8(double8);
uchar8  __attribute__((const)) __attribute__((overloadable)) convert_uchar8_sat(double8);

short8  __attribute__((const)) __attribute__((overloadable)) convert_short8_rte(double8);
short8  __attribute__((const)) __attribute__((overloadable)) convert_short8_sat_rte(double8);
short8  __attribute__((const)) __attribute__((overloadable)) convert_short8_rtz(double8);
short8  __attribute__((const)) __attribute__((overloadable)) convert_short8_sat_rtz(double8);
short8  __attribute__((const)) __attribute__((overloadable)) convert_short8_rtp(double8);
short8  __attribute__((const)) __attribute__((overloadable)) convert_short8_sat_rtp(double8);
short8  __attribute__((const)) __attribute__((overloadable)) convert_short8_rtn(double8);
short8  __attribute__((const)) __attribute__((overloadable)) convert_short8_sat_rtn(double8);
short8  __attribute__((const)) __attribute__((overloadable)) convert_short8(double8);
short8  __attribute__((const)) __attribute__((overloadable)) convert_short8_sat(double8);

ushort8  __attribute__((const)) __attribute__((overloadable)) convert_ushort8_rte(double8);
ushort8  __attribute__((const)) __attribute__((overloadable)) convert_ushort8_sat_rte(double8);
ushort8  __attribute__((const)) __attribute__((overloadable)) convert_ushort8_rtz(double8);
ushort8  __attribute__((const)) __attribute__((overloadable)) convert_ushort8_sat_rtz(double8);
ushort8  __attribute__((const)) __attribute__((overloadable)) convert_ushort8_rtp(double8);
ushort8  __attribute__((const)) __attribute__((overloadable)) convert_ushort8_sat_rtp(double8);
ushort8  __attribute__((const)) __attribute__((overloadable)) convert_ushort8_rtn(double8);
ushort8  __attribute__((const)) __attribute__((overloadable)) convert_ushort8_sat_rtn(double8);
ushort8  __attribute__((const)) __attribute__((overloadable)) convert_ushort8(double8);
ushort8  __attribute__((const)) __attribute__((overloadable)) convert_ushort8_sat(double8);

int8  __attribute__((const)) __attribute__((overloadable)) convert_int8_rte(double8);
int8  __attribute__((const)) __attribute__((overloadable)) convert_int8_sat_rte(double8);
int8  __attribute__((const)) __attribute__((overloadable)) convert_int8_rtz(double8);
int8  __attribute__((const)) __attribute__((overloadable)) convert_int8_sat_rtz(double8);
int8  __attribute__((const)) __attribute__((overloadable)) convert_int8_rtp(double8);
int8  __attribute__((const)) __attribute__((overloadable)) convert_int8_sat_rtp(double8);
int8  __attribute__((const)) __attribute__((overloadable)) convert_int8_rtn(double8);
int8  __attribute__((const)) __attribute__((overloadable)) convert_int8_sat_rtn(double8);
int8  __attribute__((const)) __attribute__((overloadable)) convert_int8(double8);
int8  __attribute__((const)) __attribute__((overloadable)) convert_int8_sat(double8);

uint8  __attribute__((const)) __attribute__((overloadable)) convert_uint8_rte(double8);
uint8  __attribute__((const)) __attribute__((overloadable)) convert_uint8_sat_rte(double8);
uint8  __attribute__((const)) __attribute__((overloadable)) convert_uint8_rtz(double8);
uint8  __attribute__((const)) __attribute__((overloadable)) convert_uint8_sat_rtz(double8);
uint8  __attribute__((const)) __attribute__((overloadable)) convert_uint8_rtp(double8);
uint8  __attribute__((const)) __attribute__((overloadable)) convert_uint8_sat_rtp(double8);
uint8  __attribute__((const)) __attribute__((overloadable)) convert_uint8_rtn(double8);
uint8  __attribute__((const)) __attribute__((overloadable)) convert_uint8_sat_rtn(double8);
uint8  __attribute__((const)) __attribute__((overloadable)) convert_uint8(double8);
uint8  __attribute__((const)) __attribute__((overloadable)) convert_uint8_sat(double8);
long8  __attribute__((const)) __attribute__((overloadable)) convert_long8_rte(double8);
long8  __attribute__((const)) __attribute__((overloadable)) convert_long8_sat_rte(double8);
long8  __attribute__((const)) __attribute__((overloadable)) convert_long8_rtz(double8);
long8  __attribute__((const)) __attribute__((overloadable)) convert_long8_sat_rtz(double8);
long8  __attribute__((const)) __attribute__((overloadable)) convert_long8_rtp(double8);
long8  __attribute__((const)) __attribute__((overloadable)) convert_long8_sat_rtp(double8);
long8  __attribute__((const)) __attribute__((overloadable)) convert_long8_rtn(double8);
long8  __attribute__((const)) __attribute__((overloadable)) convert_long8_sat_rtn(double8);
long8  __attribute__((const)) __attribute__((overloadable)) convert_long8(double8);
long8  __attribute__((const)) __attribute__((overloadable)) convert_long8_sat(double8);

ulong8  __attribute__((const)) __attribute__((overloadable)) convert_ulong8_rte(double8);
ulong8  __attribute__((const)) __attribute__((overloadable)) convert_ulong8_sat_rte(double8);
ulong8  __attribute__((const)) __attribute__((overloadable)) convert_ulong8_rtz(double8);
ulong8  __attribute__((const)) __attribute__((overloadable)) convert_ulong8_sat_rtz(double8);
ulong8  __attribute__((const)) __attribute__((overloadable)) convert_ulong8_rtp(double8);
ulong8  __attribute__((const)) __attribute__((overloadable)) convert_ulong8_sat_rtp(double8);
ulong8  __attribute__((const)) __attribute__((overloadable)) convert_ulong8_rtn(double8);
ulong8  __attribute__((const)) __attribute__((overloadable)) convert_ulong8_sat_rtn(double8);
ulong8  __attribute__((const)) __attribute__((overloadable)) convert_ulong8(double8);
ulong8  __attribute__((const)) __attribute__((overloadable)) convert_ulong8_sat(double8);

float8  __attribute__((const)) __attribute__((overloadable)) convert_float8_rte(double8);
float8  __attribute__((const)) __attribute__((overloadable)) convert_float8_rtz(double8);
float8  __attribute__((const)) __attribute__((overloadable)) convert_float8_rtp(double8);
float8  __attribute__((const)) __attribute__((overloadable)) convert_float8_rtn(double8);
float8  __attribute__((const)) __attribute__((overloadable)) convert_float8(double8);

double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rte(char8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtz(char8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtp(char8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtn(char8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8(char8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rte(uchar8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtz(uchar8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtp(uchar8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtn(uchar8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8(uchar8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rte(short8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtz(short8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtp(short8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtn(short8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8(short8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rte(ushort8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtz(ushort8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtp(ushort8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtn(ushort8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8(ushort8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rte(int8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtz(int8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtp(int8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtn(int8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8(int8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rte(uint8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtz(uint8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtp(uint8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtn(uint8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8(uint8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rte(long8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtz(long8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtp(long8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtn(long8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8(long8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rte(ulong8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtz(ulong8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtp(ulong8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtn(ulong8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8(ulong8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rte(float8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtz(float8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtp(float8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtn(float8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8(float8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rte(double8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtz(double8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtp(double8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8_rtn(double8);
double8  __attribute__((const)) __attribute__((overloadable)) convert_double8(double8);
char16  __attribute__((const)) __attribute__((overloadable)) convert_char16_rte(double16);
char16  __attribute__((const)) __attribute__((overloadable)) convert_char16_sat_rte(double16);
char16  __attribute__((const)) __attribute__((overloadable)) convert_char16_rtz(double16);
char16  __attribute__((const)) __attribute__((overloadable)) convert_char16_sat_rtz(double16);
char16  __attribute__((const)) __attribute__((overloadable)) convert_char16_rtp(double16);
char16  __attribute__((const)) __attribute__((overloadable)) convert_char16_sat_rtp(double16);
char16  __attribute__((const)) __attribute__((overloadable)) convert_char16_rtn(double16);
char16  __attribute__((const)) __attribute__((overloadable)) convert_char16_sat_rtn(double16);
char16  __attribute__((const)) __attribute__((overloadable)) convert_char16(double16);
char16  __attribute__((const)) __attribute__((overloadable)) convert_char16_sat(double16);

uchar16  __attribute__((const)) __attribute__((overloadable)) convert_uchar16_rte(double16);
uchar16  __attribute__((const)) __attribute__((overloadable)) convert_uchar16_sat_rte(double16);
uchar16  __attribute__((const)) __attribute__((overloadable)) convert_uchar16_rtz(double16);
uchar16  __attribute__((const)) __attribute__((overloadable)) convert_uchar16_sat_rtz(double16);
uchar16  __attribute__((const)) __attribute__((overloadable)) convert_uchar16_rtp(double16);
uchar16  __attribute__((const)) __attribute__((overloadable)) convert_uchar16_sat_rtp(double16);
uchar16  __attribute__((const)) __attribute__((overloadable)) convert_uchar16_rtn(double16);
uchar16  __attribute__((const)) __attribute__((overloadable)) convert_uchar16_sat_rtn(double16);
uchar16  __attribute__((const)) __attribute__((overloadable)) convert_uchar16(double16);
uchar16  __attribute__((const)) __attribute__((overloadable)) convert_uchar16_sat(double16);

short16  __attribute__((const)) __attribute__((overloadable)) convert_short16_rte(double16);
short16  __attribute__((const)) __attribute__((overloadable)) convert_short16_sat_rte(double16);
short16  __attribute__((const)) __attribute__((overloadable)) convert_short16_rtz(double16);
short16  __attribute__((const)) __attribute__((overloadable)) convert_short16_sat_rtz(double16);
short16  __attribute__((const)) __attribute__((overloadable)) convert_short16_rtp(double16);
short16  __attribute__((const)) __attribute__((overloadable)) convert_short16_sat_rtp(double16);
short16  __attribute__((const)) __attribute__((overloadable)) convert_short16_rtn(double16);
short16  __attribute__((const)) __attribute__((overloadable)) convert_short16_sat_rtn(double16);
short16  __attribute__((const)) __attribute__((overloadable)) convert_short16(double16);
short16  __attribute__((const)) __attribute__((overloadable)) convert_short16_sat(double16);

ushort16  __attribute__((const)) __attribute__((overloadable)) convert_ushort16_rte(double16);
ushort16  __attribute__((const)) __attribute__((overloadable)) convert_ushort16_sat_rte(double16);
ushort16  __attribute__((const)) __attribute__((overloadable)) convert_ushort16_rtz(double16);
ushort16  __attribute__((const)) __attribute__((overloadable)) convert_ushort16_sat_rtz(double16);
ushort16  __attribute__((const)) __attribute__((overloadable)) convert_ushort16_rtp(double16);
ushort16  __attribute__((const)) __attribute__((overloadable)) convert_ushort16_sat_rtp(double16);
ushort16  __attribute__((const)) __attribute__((overloadable)) convert_ushort16_rtn(double16);
ushort16  __attribute__((const)) __attribute__((overloadable)) convert_ushort16_sat_rtn(double16);
ushort16  __attribute__((const)) __attribute__((overloadable)) convert_ushort16(double16);
ushort16  __attribute__((const)) __attribute__((overloadable)) convert_ushort16_sat(double16);

int16  __attribute__((const)) __attribute__((overloadable)) convert_int16_rte(double16);
int16  __attribute__((const)) __attribute__((overloadable)) convert_int16_sat_rte(double16);
int16  __attribute__((const)) __attribute__((overloadable)) convert_int16_rtz(double16);
int16  __attribute__((const)) __attribute__((overloadable)) convert_int16_sat_rtz(double16);
int16  __attribute__((const)) __attribute__((overloadable)) convert_int16_rtp(double16);
int16  __attribute__((const)) __attribute__((overloadable)) convert_int16_sat_rtp(double16);
int16  __attribute__((const)) __attribute__((overloadable)) convert_int16_rtn(double16);
int16  __attribute__((const)) __attribute__((overloadable)) convert_int16_sat_rtn(double16);
int16  __attribute__((const)) __attribute__((overloadable)) convert_int16(double16);
int16  __attribute__((const)) __attribute__((overloadable)) convert_int16_sat(double16);

uint16  __attribute__((const)) __attribute__((overloadable)) convert_uint16_rte(double16);
uint16  __attribute__((const)) __attribute__((overloadable)) convert_uint16_sat_rte(double16);
uint16  __attribute__((const)) __attribute__((overloadable)) convert_uint16_rtz(double16);
uint16  __attribute__((const)) __attribute__((overloadable)) convert_uint16_sat_rtz(double16);
uint16  __attribute__((const)) __attribute__((overloadable)) convert_uint16_rtp(double16);
uint16  __attribute__((const)) __attribute__((overloadable)) convert_uint16_sat_rtp(double16);
uint16  __attribute__((const)) __attribute__((overloadable)) convert_uint16_rtn(double16);
uint16  __attribute__((const)) __attribute__((overloadable)) convert_uint16_sat_rtn(double16);
uint16  __attribute__((const)) __attribute__((overloadable)) convert_uint16(double16);
uint16  __attribute__((const)) __attribute__((overloadable)) convert_uint16_sat(double16);
long16  __attribute__((const)) __attribute__((overloadable)) convert_long16_rte(double16);
long16  __attribute__((const)) __attribute__((overloadable)) convert_long16_sat_rte(double16);
long16  __attribute__((const)) __attribute__((overloadable)) convert_long16_rtz(double16);
long16  __attribute__((const)) __attribute__((overloadable)) convert_long16_sat_rtz(double16);
long16  __attribute__((const)) __attribute__((overloadable)) convert_long16_rtp(double16);
long16  __attribute__((const)) __attribute__((overloadable)) convert_long16_sat_rtp(double16);
long16  __attribute__((const)) __attribute__((overloadable)) convert_long16_rtn(double16);
long16  __attribute__((const)) __attribute__((overloadable)) convert_long16_sat_rtn(double16);
long16  __attribute__((const)) __attribute__((overloadable)) convert_long16(double16);
long16  __attribute__((const)) __attribute__((overloadable)) convert_long16_sat(double16);

ulong16  __attribute__((const)) __attribute__((overloadable)) convert_ulong16_rte(double16);
ulong16  __attribute__((const)) __attribute__((overloadable)) convert_ulong16_sat_rte(double16);
ulong16  __attribute__((const)) __attribute__((overloadable)) convert_ulong16_rtz(double16);
ulong16  __attribute__((const)) __attribute__((overloadable)) convert_ulong16_sat_rtz(double16);
ulong16  __attribute__((const)) __attribute__((overloadable)) convert_ulong16_rtp(double16);
ulong16  __attribute__((const)) __attribute__((overloadable)) convert_ulong16_sat_rtp(double16);
ulong16  __attribute__((const)) __attribute__((overloadable)) convert_ulong16_rtn(double16);
ulong16  __attribute__((const)) __attribute__((overloadable)) convert_ulong16_sat_rtn(double16);
ulong16  __attribute__((const)) __attribute__((overloadable)) convert_ulong16(double16);
ulong16  __attribute__((const)) __attribute__((overloadable)) convert_ulong16_sat(double16);

float16  __attribute__((const)) __attribute__((overloadable)) convert_float16_rte(double16);
float16  __attribute__((const)) __attribute__((overloadable)) convert_float16_rtz(double16);
float16  __attribute__((const)) __attribute__((overloadable)) convert_float16_rtp(double16);
float16  __attribute__((const)) __attribute__((overloadable)) convert_float16_rtn(double16);
float16  __attribute__((const)) __attribute__((overloadable)) convert_float16(double16);

double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rte(char16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtz(char16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtp(char16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtn(char16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16(char16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rte(uchar16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtz(uchar16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtp(uchar16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtn(uchar16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16(uchar16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rte(short16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtz(short16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtp(short16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtn(short16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16(short16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rte(ushort16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtz(ushort16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtp(ushort16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtn(ushort16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16(ushort16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rte(int16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtz(int16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtp(int16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtn(int16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16(int16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rte(uint16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtz(uint16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtp(uint16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtn(uint16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16(uint16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rte(long16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtz(long16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtp(long16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtn(long16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16(long16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rte(ulong16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtz(ulong16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtp(ulong16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtn(ulong16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16(ulong16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rte(float16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtz(float16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtp(float16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtn(float16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16(float16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rte(double16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtz(double16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtp(double16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16_rtn(double16);
double16  __attribute__((const)) __attribute__((overloadable)) convert_double16(double16);
