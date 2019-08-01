#ifndef _OPENCL_PLATFORM_12_H_
#define _OPENCL_PLATFORM_12_H_

#include "opencl-c-platform.h"

// OpenCL v1.1 s6.11.2, v1.2 s6.12.2, v2.0 s6.13.2 - Math functions

// opencl-c-common.h is already contains this statement, but enabled OpenCL
// extensions are not loaded from PCH

#ifdef cl_khr_fp64
#if __OPENCL_C_VERSION__ < CL_VERSION_1_2
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif
#endif

#ifdef cl_khr_fp16
#pragma OPENCL EXTENSION cl_khr_fp16 : enable
#endif

/**
 * Returns fmin(x - floor (x), 0x1.fffffep-1f ).
 * floor(x) is returned in iptr.
 */
float __ovld fract(float x, __global float *iptr);
float2 __ovld fract(float2 x, __global float2 *iptr);
float3 __ovld fract(float3 x, __global float3 *iptr);
float4 __ovld fract(float4 x, __global float4 *iptr);
float8 __ovld fract(float8 x, __global float8 *iptr);
float16 __ovld fract(float16 x, __global float16 *iptr);
float __ovld fract(float x, __local float *iptr);
float2 __ovld fract(float2 x, __local float2 *iptr);
float3 __ovld fract(float3 x, __local float3 *iptr);
float4 __ovld fract(float4 x, __local float4 *iptr);
float8 __ovld fract(float8 x, __local float8 *iptr);
float16 __ovld fract(float16 x, __local float16 *iptr);
float __ovld fract(float x, __private float *iptr);
float2 __ovld fract(float2 x, __private float2 *iptr);
float3 __ovld fract(float3 x, __private float3 *iptr);
float4 __ovld fract(float4 x, __private float4 *iptr);
float8 __ovld fract(float8 x, __private float8 *iptr);
float16 __ovld fract(float16 x, __private float16 *iptr);
#ifdef cl_khr_fp64
double __ovld fract(double x, __global double *iptr);
double2 __ovld fract(double2 x, __global double2 *iptr);
double3 __ovld fract(double3 x, __global double3 *iptr);
double4 __ovld fract(double4 x, __global double4 *iptr);
double8 __ovld fract(double8 x, __global double8 *iptr);
double16 __ovld fract(double16 x, __global double16 *iptr);
double __ovld fract(double x, __local double *iptr);
double2 __ovld fract(double2 x, __local double2 *iptr);
double3 __ovld fract(double3 x, __local double3 *iptr);
double4 __ovld fract(double4 x, __local double4 *iptr);
double8 __ovld fract(double8 x, __local double8 *iptr);
double16 __ovld fract(double16 x, __local double16 *iptr);
double __ovld fract(double x, __private double *iptr);
double2 __ovld fract(double2 x, __private double2 *iptr);
double3 __ovld fract(double3 x, __private double3 *iptr);
double4 __ovld fract(double4 x, __private double4 *iptr);
double8 __ovld fract(double8 x, __private double8 *iptr);
double16 __ovld fract(double16 x, __private double16 *iptr);
#endif //cl_khr_fp64
#ifdef cl_khr_fp16
half __ovld fract(half x, __global half *iptr);
half2 __ovld fract(half2 x, __global half2 *iptr);
half3 __ovld fract(half3 x, __global half3 *iptr);
half4 __ovld fract(half4 x, __global half4 *iptr);
half8 __ovld fract(half8 x, __global half8 *iptr);
half16 __ovld fract(half16 x, __global half16 *iptr);
half __ovld fract(half x, __local half *iptr);
half2 __ovld fract(half2 x, __local half2 *iptr);
half3 __ovld fract(half3 x, __local half3 *iptr);
half4 __ovld fract(half4 x, __local half4 *iptr);
half8 __ovld fract(half8 x, __local half8 *iptr);
half16 __ovld fract(half16 x, __local half16 *iptr);
half __ovld fract(half x, __private half *iptr);
half2 __ovld fract(half2 x, __private half2 *iptr);
half3 __ovld fract(half3 x, __private half3 *iptr);
half4 __ovld fract(half4 x, __private half4 *iptr);
half8 __ovld fract(half8 x, __private half8 *iptr);
half16 __ovld fract(half16 x, __private half16 *iptr);
#endif //cl_khr_fp16

/**
 * Extract mantissa and exponent from x. For each
 * component the mantissa returned is a float with
 * magnitude in the interval [1/2, 1) or 0. Each
 * component of x equals mantissa returned * 2^exp.
 */
float __ovld frexp(float x, __global int *exp);
float2 __ovld frexp(float2 x, __global int2 *exp);
float3 __ovld frexp(float3 x, __global int3 *exp);
float4 __ovld frexp(float4 x, __global int4 *exp);
float8 __ovld frexp(float8 x, __global int8 *exp);
float16 __ovld frexp(float16 x, __global int16 *exp);
float __ovld frexp(float x, __local int *exp);
float2 __ovld frexp(float2 x, __local int2 *exp);
float3 __ovld frexp(float3 x, __local int3 *exp);
float4 __ovld frexp(float4 x, __local int4 *exp);
float8 __ovld frexp(float8 x, __local int8 *exp);
float16 __ovld frexp(float16 x, __local int16 *exp);
float __ovld frexp(float x, __private int *exp);
float2 __ovld frexp(float2 x, __private int2 *exp);
float3 __ovld frexp(float3 x, __private int3 *exp);
float4 __ovld frexp(float4 x, __private int4 *exp);
float8 __ovld frexp(float8 x, __private int8 *exp);
float16 __ovld frexp(float16 x, __private int16 *exp);
#ifdef cl_khr_fp64
double __ovld frexp(double x, __global int *exp);
double2 __ovld frexp(double2 x, __global int2 *exp);
double3 __ovld frexp(double3 x, __global int3 *exp);
double4 __ovld frexp(double4 x, __global int4 *exp);
double8 __ovld frexp(double8 x, __global int8 *exp);
double16 __ovld frexp(double16 x, __global int16 *exp);
double __ovld frexp(double x, __local int *exp);
double2 __ovld frexp(double2 x, __local int2 *exp);
double3 __ovld frexp(double3 x, __local int3 *exp);
double4 __ovld frexp(double4 x, __local int4 *exp);
double8 __ovld frexp(double8 x, __local int8 *exp);
double16 __ovld frexp(double16 x, __local int16 *exp);
double __ovld frexp(double x, __private int *exp);
double2 __ovld frexp(double2 x, __private int2 *exp);
double3 __ovld frexp(double3 x, __private int3 *exp);
double4 __ovld frexp(double4 x, __private int4 *exp);
double8 __ovld frexp(double8 x, __private int8 *exp);
double16 __ovld frexp(double16 x, __private int16 *exp);
#endif //cl_khr_fp64
#ifdef cl_khr_fp16
half __ovld frexp(half x, __global int *exp);
half2 __ovld frexp(half2 x, __global int2 *exp);
half3 __ovld frexp(half3 x, __global int3 *exp);
half4 __ovld frexp(half4 x, __global int4 *exp);
half8 __ovld frexp(half8 x, __global int8 *exp);
half16 __ovld frexp(half16 x, __global int16 *exp);
half __ovld frexp(half x, __local int *exp);
half2 __ovld frexp(half2 x, __local int2 *exp);
half3 __ovld frexp(half3 x, __local int3 *exp);
half4 __ovld frexp(half4 x, __local int4 *exp);
half8 __ovld frexp(half8 x, __local int8 *exp);
half16 __ovld frexp(half16 x, __local int16 *exp);
half __ovld frexp(half x, __private int *exp);
half2 __ovld frexp(half2 x, __private int2 *exp);
half3 __ovld frexp(half3 x, __private int3 *exp);
half4 __ovld frexp(half4 x, __private int4 *exp);
half8 __ovld frexp(half8 x, __private int8 *exp);
half16 __ovld frexp(half16 x, __private int16 *exp);
#endif //cl_khr_fp16

/**
 * Log gamma function. Returns the natural
 * logarithm of the absolute value of the gamma
 * function. The sign of the gamma function is
 * returned in the signp argument of lgamma_r.
 */
float __ovld lgamma_r(float x, __global int *signp);
float2 __ovld lgamma_r(float2 x, __global int2 *signp);
float3 __ovld lgamma_r(float3 x, __global int3 *signp);
float4 __ovld lgamma_r(float4 x, __global int4 *signp);
float8 __ovld lgamma_r(float8 x, __global int8 *signp);
float16 __ovld lgamma_r(float16 x, __global int16 *signp);
float __ovld lgamma_r(float x, __local int *signp);
float2 __ovld lgamma_r(float2 x, __local int2 *signp);
float3 __ovld lgamma_r(float3 x, __local int3 *signp);
float4 __ovld lgamma_r(float4 x, __local int4 *signp);
float8 __ovld lgamma_r(float8 x, __local int8 *signp);
float16 __ovld lgamma_r(float16 x, __local int16 *signp);
float __ovld lgamma_r(float x, __private int *signp);
float2 __ovld lgamma_r(float2 x, __private int2 *signp);
float3 __ovld lgamma_r(float3 x, __private int3 *signp);
float4 __ovld lgamma_r(float4 x, __private int4 *signp);
float8 __ovld lgamma_r(float8 x, __private int8 *signp);
float16 __ovld lgamma_r(float16 x, __private int16 *signp);
#ifdef cl_khr_fp64
double __ovld lgamma_r(double x, __global int *signp);
double2 __ovld lgamma_r(double2 x, __global int2 *signp);
double3 __ovld lgamma_r(double3 x, __global int3 *signp);
double4 __ovld lgamma_r(double4 x, __global int4 *signp);
double8 __ovld lgamma_r(double8 x, __global int8 *signp);
double16 __ovld lgamma_r(double16 x, __global int16 *signp);
double __ovld lgamma_r(double x, __local int *signp);
double2 __ovld lgamma_r(double2 x, __local int2 *signp);
double3 __ovld lgamma_r(double3 x, __local int3 *signp);
double4 __ovld lgamma_r(double4 x, __local int4 *signp);
double8 __ovld lgamma_r(double8 x, __local int8 *signp);
double16 __ovld lgamma_r(double16 x, __local int16 *signp);
double __ovld lgamma_r(double x, __private int *signp);
double2 __ovld lgamma_r(double2 x, __private int2 *signp);
double3 __ovld lgamma_r(double3 x, __private int3 *signp);
double4 __ovld lgamma_r(double4 x, __private int4 *signp);
double8 __ovld lgamma_r(double8 x, __private int8 *signp);
double16 __ovld lgamma_r(double16 x, __private int16 *signp);
#endif //cl_khr_fp64
#ifdef cl_khr_fp16
half __ovld lgamma_r(half x, __global int *signp);
half2 __ovld lgamma_r(half2 x, __global int2 *signp);
half3 __ovld lgamma_r(half3 x, __global int3 *signp);
half4 __ovld lgamma_r(half4 x, __global int4 *signp);
half8 __ovld lgamma_r(half8 x, __global int8 *signp);
half16 __ovld lgamma_r(half16 x, __global int16 *signp);
half __ovld lgamma_r(half x, __local int *signp);
half2 __ovld lgamma_r(half2 x, __local int2 *signp);
half3 __ovld lgamma_r(half3 x, __local int3 *signp);
half4 __ovld lgamma_r(half4 x, __local int4 *signp);
half8 __ovld lgamma_r(half8 x, __local int8 *signp);
half16 __ovld lgamma_r(half16 x, __local int16 *signp);
half __ovld lgamma_r(half x, __private int *signp);
half2 __ovld lgamma_r(half2 x, __private int2 *signp);
half3 __ovld lgamma_r(half3 x, __private int3 *signp);
half4 __ovld lgamma_r(half4 x, __private int4 *signp);
half8 __ovld lgamma_r(half8 x, __private int8 *signp);
half16 __ovld lgamma_r(half16 x, __private int16 *signp);
#endif //cl_khr_fp16

/**
 * Decompose a floating-point number. The modf
 * function breaks the argument x into integral and
 * fractional parts, each of which has the same sign as
 * the argument. It stores the integral part in the object
 * pointed to by iptr.
 */
float __ovld modf(float x, __global float *iptr);
float2 __ovld modf(float2 x, __global float2 *iptr);
float3 __ovld modf(float3 x, __global float3 *iptr);
float4 __ovld modf(float4 x, __global float4 *iptr);
float8 __ovld modf(float8 x, __global float8 *iptr);
float16 __ovld modf(float16 x, __global float16 *iptr);
float __ovld modf(float x, __local float *iptr);
float2 __ovld modf(float2 x, __local float2 *iptr);
float3 __ovld modf(float3 x, __local float3 *iptr);
float4 __ovld modf(float4 x, __local float4 *iptr);
float8 __ovld modf(float8 x, __local float8 *iptr);
float16 __ovld modf(float16 x, __local float16 *iptr);
float __ovld modf(float x, __private float *iptr);
float2 __ovld modf(float2 x, __private float2 *iptr);
float3 __ovld modf(float3 x, __private float3 *iptr);
float4 __ovld modf(float4 x, __private float4 *iptr);
float8 __ovld modf(float8 x, __private float8 *iptr);
float16 __ovld modf(float16 x, __private float16 *iptr);
#ifdef cl_khr_fp64
double __ovld modf(double x, __global double *iptr);
double2 __ovld modf(double2 x, __global double2 *iptr);
double3 __ovld modf(double3 x, __global double3 *iptr);
double4 __ovld modf(double4 x, __global double4 *iptr);
double8 __ovld modf(double8 x, __global double8 *iptr);
double16 __ovld modf(double16 x, __global double16 *iptr);
double __ovld modf(double x, __local double *iptr);
double2 __ovld modf(double2 x, __local double2 *iptr);
double3 __ovld modf(double3 x, __local double3 *iptr);
double4 __ovld modf(double4 x, __local double4 *iptr);
double8 __ovld modf(double8 x, __local double8 *iptr);
double16 __ovld modf(double16 x, __local double16 *iptr);
double __ovld modf(double x, __private double *iptr);
double2 __ovld modf(double2 x, __private double2 *iptr);
double3 __ovld modf(double3 x, __private double3 *iptr);
double4 __ovld modf(double4 x, __private double4 *iptr);
double8 __ovld modf(double8 x, __private double8 *iptr);
double16 __ovld modf(double16 x, __private double16 *iptr);
#endif //cl_khr_fp64
#ifdef cl_khr_fp16
half __ovld modf(half x, __global half *iptr);
half2 __ovld modf(half2 x, __global half2 *iptr);
half3 __ovld modf(half3 x, __global half3 *iptr);
half4 __ovld modf(half4 x, __global half4 *iptr);
half8 __ovld modf(half8 x, __global half8 *iptr);
half16 __ovld modf(half16 x, __global half16 *iptr);
half __ovld modf(half x, __local half *iptr);
half2 __ovld modf(half2 x, __local half2 *iptr);
half3 __ovld modf(half3 x, __local half3 *iptr);
half4 __ovld modf(half4 x, __local half4 *iptr);
half8 __ovld modf(half8 x, __local half8 *iptr);
half16 __ovld modf(half16 x, __local half16 *iptr);
half __ovld modf(half x, __private half *iptr);
half2 __ovld modf(half2 x, __private half2 *iptr);
half3 __ovld modf(half3 x, __private half3 *iptr);
half4 __ovld modf(half4 x, __private half4 *iptr);
half8 __ovld modf(half8 x, __private half8 *iptr);
half16 __ovld modf(half16 x, __private half16 *iptr);
#endif //cl_khr_fp16

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
float __ovld remquo(float x, float y, __global int *quo);
float2 __ovld remquo(float2 x, float2 y, __global int2 *quo);
float3 __ovld remquo(float3 x, float3 y, __global int3 *quo);
float4 __ovld remquo(float4 x, float4 y, __global int4 *quo);
float8 __ovld remquo(float8 x, float8 y, __global int8 *quo);
float16 __ovld remquo(float16 x, float16 y, __global int16 *quo);
float __ovld remquo(float x, float y, __local int *quo);
float2 __ovld remquo(float2 x, float2 y, __local int2 *quo);
float3 __ovld remquo(float3 x, float3 y, __local int3 *quo);
float4 __ovld remquo(float4 x, float4 y, __local int4 *quo);
float8 __ovld remquo(float8 x, float8 y, __local int8 *quo);
float16 __ovld remquo(float16 x, float16 y, __local int16 *quo);
float __ovld remquo(float x, float y, __private int *quo);
float2 __ovld remquo(float2 x, float2 y, __private int2 *quo);
float3 __ovld remquo(float3 x, float3 y, __private int3 *quo);
float4 __ovld remquo(float4 x, float4 y, __private int4 *quo);
float8 __ovld remquo(float8 x, float8 y, __private int8 *quo);
float16 __ovld remquo(float16 x, float16 y, __private int16 *quo);
#ifdef cl_khr_fp64
double __ovld remquo(double x, double y, __global int *quo);
double2 __ovld remquo(double2 x, double2 y, __global int2 *quo);
double3 __ovld remquo(double3 x, double3 y, __global int3 *quo);
double4 __ovld remquo(double4 x, double4 y, __global int4 *quo);
double8 __ovld remquo(double8 x, double8 y, __global int8 *quo);
double16 __ovld remquo(double16 x, double16 y, __global int16 *quo);
double __ovld remquo(double x, double y, __local int *quo);
double2 __ovld remquo(double2 x, double2 y, __local int2 *quo);
double3 __ovld remquo(double3 x, double3 y, __local int3 *quo);
double4 __ovld remquo(double4 x, double4 y, __local int4 *quo);
double8 __ovld remquo(double8 x, double8 y, __local int8 *quo);
double16 __ovld remquo(double16 x, double16 y, __local int16 *quo);
double __ovld remquo(double x, double y, __private int *quo);
double2 __ovld remquo(double2 x, double2 y, __private int2 *quo);
double3 __ovld remquo(double3 x, double3 y, __private int3 *quo);
double4 __ovld remquo(double4 x, double4 y, __private int4 *quo);
double8 __ovld remquo(double8 x, double8 y, __private int8 *quo);
double16 __ovld remquo(double16 x, double16 y, __private int16 *quo);
#endif //cl_khr_fp64
#ifdef cl_khr_fp16
half __ovld remquo(half x, half y, __global int *quo);
half2 __ovld remquo(half2 x, half2 y, __global int2 *quo);
half3 __ovld remquo(half3 x, half3 y, __global int3 *quo);
half4 __ovld remquo(half4 x, half4 y, __global int4 *quo);
half8 __ovld remquo(half8 x, half8 y, __global int8 *quo);
half16 __ovld remquo(half16 x, half16 y, __global int16 *quo);
half __ovld remquo(half x, half y, __local int *quo);
half2 __ovld remquo(half2 x, half2 y, __local int2 *quo);
half3 __ovld remquo(half3 x, half3 y, __local int3 *quo);
half4 __ovld remquo(half4 x, half4 y, __local int4 *quo);
half8 __ovld remquo(half8 x, half8 y, __local int8 *quo);
half16 __ovld remquo(half16 x, half16 y, __local int16 *quo);
half __ovld remquo(half x, half y, __private int *quo);
half2 __ovld remquo(half2 x, half2 y, __private int2 *quo);
half3 __ovld remquo(half3 x, half3 y, __private int3 *quo);
half4 __ovld remquo(half4 x, half4 y, __private int4 *quo);
half8 __ovld remquo(half8 x, half8 y, __private int8 *quo);
half16 __ovld remquo(half16 x, half16 y, __private int16 *quo);
#endif //cl_khr_fp16

/**
 * Compute sine and cosine of x. The computed sine
 * is the return value and computed cosine is returned
 * in cosval.
 */
float __ovld sincos(float x, __global float *cosval);
float2 __ovld sincos(float2 x, __global float2 *cosval);
float3 __ovld sincos(float3 x, __global float3 *cosval);
float4 __ovld sincos(float4 x, __global float4 *cosval);
float8 __ovld sincos(float8 x, __global float8 *cosval);
float16 __ovld sincos(float16 x, __global float16 *cosval);
float __ovld sincos(float x, __local float *cosval);
float2 __ovld sincos(float2 x, __local float2 *cosval);
float3 __ovld sincos(float3 x, __local float3 *cosval);
float4 __ovld sincos(float4 x, __local float4 *cosval);
float8 __ovld sincos(float8 x, __local float8 *cosval);
float16 __ovld sincos(float16 x, __local float16 *cosval);
float __ovld sincos(float x, __private float *cosval);
float2 __ovld sincos(float2 x, __private float2 *cosval);
float3 __ovld sincos(float3 x, __private float3 *cosval);
float4 __ovld sincos(float4 x, __private float4 *cosval);
float8 __ovld sincos(float8 x, __private float8 *cosval);
float16 __ovld sincos(float16 x, __private float16 *cosval);
#ifdef cl_khr_fp64
double __ovld sincos(double x, __global double *cosval);
double2 __ovld sincos(double2 x, __global double2 *cosval);
double3 __ovld sincos(double3 x, __global double3 *cosval);
double4 __ovld sincos(double4 x, __global double4 *cosval);
double8 __ovld sincos(double8 x, __global double8 *cosval);
double16 __ovld sincos(double16 x, __global double16 *cosval);
double __ovld sincos(double x, __local double *cosval);
double2 __ovld sincos(double2 x, __local double2 *cosval);
double3 __ovld sincos(double3 x, __local double3 *cosval);
double4 __ovld sincos(double4 x, __local double4 *cosval);
double8 __ovld sincos(double8 x, __local double8 *cosval);
double16 __ovld sincos(double16 x, __local double16 *cosval);
double __ovld sincos(double x, __private double *cosval);
double2 __ovld sincos(double2 x, __private double2 *cosval);
double3 __ovld sincos(double3 x, __private double3 *cosval);
double4 __ovld sincos(double4 x, __private double4 *cosval);
double8 __ovld sincos(double8 x, __private double8 *cosval);
double16 __ovld sincos(double16 x, __private double16 *cosval);
#endif //cl_khr_fp64
#ifdef cl_khr_fp16
half __ovld sincos(half x, __global half *cosval);
half2 __ovld sincos(half2 x, __global half2 *cosval);
half3 __ovld sincos(half3 x, __global half3 *cosval);
half4 __ovld sincos(half4 x, __global half4 *cosval);
half8 __ovld sincos(half8 x, __global half8 *cosval);
half16 __ovld sincos(half16 x, __global half16 *cosval);
half __ovld sincos(half x, __local half *cosval);
half2 __ovld sincos(half2 x, __local half2 *cosval);
half3 __ovld sincos(half3 x, __local half3 *cosval);
half4 __ovld sincos(half4 x, __local half4 *cosval);
half8 __ovld sincos(half8 x, __local half8 *cosval);
half16 __ovld sincos(half16 x, __local half16 *cosval);
half __ovld sincos(half x, __private half *cosval);
half2 __ovld sincos(half2 x, __private half2 *cosval);
half3 __ovld sincos(half3 x, __private half3 *cosval);
half4 __ovld sincos(half4 x, __private half4 *cosval);
half8 __ovld sincos(half8 x, __private half8 *cosval);
half16 __ovld sincos(half16 x, __private half16 *cosval);
#endif //cl_khr_fp16

// OpenCL v1.1 s6.11.7, v1.2 s6.12.7, v2.0 s6.13.7 - Vector Data Load and Store Functions
// OpenCL extensions v1.1 s9.6.6, v1.2 s9.5.6, v2.0 s9.4.6 - Vector Data Load and Store Functions for Half Type

/**
 * Use generic type gentype to indicate the built-in data types
 * char, uchar, short, ushort, int, uint, long, ulong, float,
 * double or half.
 *
 * vloadn return sizeof (gentypen) bytes of data read from address (p + (offset * n)).
 *
 * vstoren write sizeof (gentypen) bytes given by data to address (p + (offset * n)).
 *
 * The address computed as (p + (offset * n)) must be
 * 8-bit aligned if gentype is char, uchar;
 * 16-bit aligned if gentype is short, ushort, half;
 * 32-bit aligned if gentype is int, uint, float;
 * 64-bit aligned if gentype is long, ulong, double.
 */
char2 __ovld vload2(size_t offset, const __global char *p);
uchar2 __ovld vload2(size_t offset, const __global uchar *p);
short2 __ovld vload2(size_t offset, const __global short *p);
ushort2 __ovld vload2(size_t offset, const __global ushort *p);
int2 __ovld vload2(size_t offset, const __global int *p);
uint2 __ovld vload2(size_t offset, const __global uint *p);
long2 __ovld vload2(size_t offset, const __global long *p);
ulong2 __ovld vload2(size_t offset, const __global ulong *p);
float2 __ovld vload2(size_t offset, const __global float *p);
char3 __ovld vload3(size_t offset, const __global char *p);
uchar3 __ovld vload3(size_t offset, const __global uchar *p);
short3 __ovld vload3(size_t offset, const __global short *p);
ushort3 __ovld vload3(size_t offset, const __global ushort *p);
int3 __ovld vload3(size_t offset, const __global int *p);
uint3 __ovld vload3(size_t offset, const __global uint *p);
long3 __ovld vload3(size_t offset, const __global long *p);
ulong3 __ovld vload3(size_t offset, const __global ulong *p);
float3 __ovld vload3(size_t offset, const __global float *p);
char4 __ovld vload4(size_t offset, const __global char *p);
uchar4 __ovld vload4(size_t offset, const __global uchar *p);
short4 __ovld vload4(size_t offset, const __global short *p);
ushort4 __ovld vload4(size_t offset, const __global ushort *p);
int4 __ovld vload4(size_t offset, const __global int *p);
uint4 __ovld vload4(size_t offset, const __global uint *p);
long4 __ovld vload4(size_t offset, const __global long *p);
ulong4 __ovld vload4(size_t offset, const __global ulong *p);
float4 __ovld vload4(size_t offset, const __global float *p);
char8 __ovld vload8(size_t offset, const __global char *p);
uchar8 __ovld vload8(size_t offset, const __global uchar *p);
short8 __ovld vload8(size_t offset, const __global short *p);
ushort8 __ovld vload8(size_t offset, const __global ushort *p);
int8 __ovld vload8(size_t offset, const __global int *p);
uint8 __ovld vload8(size_t offset, const __global uint *p);
long8 __ovld vload8(size_t offset, const __global long *p);
ulong8 __ovld vload8(size_t offset, const __global ulong *p);
float8 __ovld vload8(size_t offset, const __global float *p);
char16 __ovld vload16(size_t offset, const __global char *p);
uchar16 __ovld vload16(size_t offset, const __global uchar *p);
short16 __ovld vload16(size_t offset, const __global short *p);
ushort16 __ovld vload16(size_t offset, const __global ushort *p);
int16 __ovld vload16(size_t offset, const __global int *p);
uint16 __ovld vload16(size_t offset, const __global uint *p);
long16 __ovld vload16(size_t offset, const __global long *p);
ulong16 __ovld vload16(size_t offset, const __global ulong *p);
float16 __ovld vload16(size_t offset, const __global float *p);
char2 __ovld vload2(size_t offset, const __local char *p);
uchar2 __ovld vload2(size_t offset, const __local uchar *p);
short2 __ovld vload2(size_t offset, const __local short *p);
ushort2 __ovld vload2(size_t offset, const __local ushort *p);
int2 __ovld vload2(size_t offset, const __local int *p);
uint2 __ovld vload2(size_t offset, const __local uint *p);
long2 __ovld vload2(size_t offset, const __local long *p);
ulong2 __ovld vload2(size_t offset, const __local ulong *p);
float2 __ovld vload2(size_t offset, const __local float *p);
char3 __ovld vload3(size_t offset, const __local char *p);
uchar3 __ovld vload3(size_t offset, const __local uchar *p);
short3 __ovld vload3(size_t offset, const __local short *p);
ushort3 __ovld vload3(size_t offset, const __local ushort *p);
int3 __ovld vload3(size_t offset, const __local int *p);
uint3 __ovld vload3(size_t offset, const __local uint *p);
long3 __ovld vload3(size_t offset, const __local long *p);
ulong3 __ovld vload3(size_t offset, const __local ulong *p);
float3 __ovld vload3(size_t offset, const __local float *p);
char4 __ovld vload4(size_t offset, const __local char *p);
uchar4 __ovld vload4(size_t offset, const __local uchar *p);
short4 __ovld vload4(size_t offset, const __local short *p);
ushort4 __ovld vload4(size_t offset, const __local ushort *p);
int4 __ovld vload4(size_t offset, const __local int *p);
uint4 __ovld vload4(size_t offset, const __local uint *p);
long4 __ovld vload4(size_t offset, const __local long *p);
ulong4 __ovld vload4(size_t offset, const __local ulong *p);
float4 __ovld vload4(size_t offset, const __local float *p);
char8 __ovld vload8(size_t offset, const __local char *p);
uchar8 __ovld vload8(size_t offset, const __local uchar *p);
short8 __ovld vload8(size_t offset, const __local short *p);
ushort8 __ovld vload8(size_t offset, const __local ushort *p);
int8 __ovld vload8(size_t offset, const __local int *p);
uint8 __ovld vload8(size_t offset, const __local uint *p);
long8 __ovld vload8(size_t offset, const __local long *p);
ulong8 __ovld vload8(size_t offset, const __local ulong *p);
float8 __ovld vload8(size_t offset, const __local float *p);
char16 __ovld vload16(size_t offset, const __local char *p);
uchar16 __ovld vload16(size_t offset, const __local uchar *p);
short16 __ovld vload16(size_t offset, const __local short *p);
ushort16 __ovld vload16(size_t offset, const __local ushort *p);
int16 __ovld vload16(size_t offset, const __local int *p);
uint16 __ovld vload16(size_t offset, const __local uint *p);
long16 __ovld vload16(size_t offset, const __local long *p);
ulong16 __ovld vload16(size_t offset, const __local ulong *p);
float16 __ovld vload16(size_t offset, const __local float *p);
char2 __ovld vload2(size_t offset, const __private char *p);
uchar2 __ovld vload2(size_t offset, const __private uchar *p);
short2 __ovld vload2(size_t offset, const __private short *p);
ushort2 __ovld vload2(size_t offset, const __private ushort *p);
int2 __ovld vload2(size_t offset, const __private int *p);
uint2 __ovld vload2(size_t offset, const __private uint *p);
long2 __ovld vload2(size_t offset, const __private long *p);
ulong2 __ovld vload2(size_t offset, const __private ulong *p);
float2 __ovld vload2(size_t offset, const __private float *p);
char3 __ovld vload3(size_t offset, const __private char *p);
uchar3 __ovld vload3(size_t offset, const __private uchar *p);
short3 __ovld vload3(size_t offset, const __private short *p);
ushort3 __ovld vload3(size_t offset, const __private ushort *p);
int3 __ovld vload3(size_t offset, const __private int *p);
uint3 __ovld vload3(size_t offset, const __private uint *p);
long3 __ovld vload3(size_t offset, const __private long *p);
ulong3 __ovld vload3(size_t offset, const __private ulong *p);
float3 __ovld vload3(size_t offset, const __private float *p);
char4 __ovld vload4(size_t offset, const __private char *p);
uchar4 __ovld vload4(size_t offset, const __private uchar *p);
short4 __ovld vload4(size_t offset, const __private short *p);
ushort4 __ovld vload4(size_t offset, const __private ushort *p);
int4 __ovld vload4(size_t offset, const __private int *p);
uint4 __ovld vload4(size_t offset, const __private uint *p);
long4 __ovld vload4(size_t offset, const __private long *p);
ulong4 __ovld vload4(size_t offset, const __private ulong *p);
float4 __ovld vload4(size_t offset, const __private float *p);
char8 __ovld vload8(size_t offset, const __private char *p);
uchar8 __ovld vload8(size_t offset, const __private uchar *p);
short8 __ovld vload8(size_t offset, const __private short *p);
ushort8 __ovld vload8(size_t offset, const __private ushort *p);
int8 __ovld vload8(size_t offset, const __private int *p);
uint8 __ovld vload8(size_t offset, const __private uint *p);
long8 __ovld vload8(size_t offset, const __private long *p);
ulong8 __ovld vload8(size_t offset, const __private ulong *p);
float8 __ovld vload8(size_t offset, const __private float *p);
char16 __ovld vload16(size_t offset, const __private char *p);
uchar16 __ovld vload16(size_t offset, const __private uchar *p);
short16 __ovld vload16(size_t offset, const __private short *p);
ushort16 __ovld vload16(size_t offset, const __private ushort *p);
int16 __ovld vload16(size_t offset, const __private int *p);
uint16 __ovld vload16(size_t offset, const __private uint *p);
long16 __ovld vload16(size_t offset, const __private long *p);
ulong16 __ovld vload16(size_t offset, const __private ulong *p);
float16 __ovld vload16(size_t offset, const __private float *p);

#ifdef cl_khr_fp64
double2 __ovld vload2(size_t offset, const __global double *p);
double3 __ovld vload3(size_t offset, const __global double *p);
double4 __ovld vload4(size_t offset, const __global double *p);
double8 __ovld vload8(size_t offset, const __global double *p);
double16 __ovld vload16(size_t offset, const __global double *p);
double2 __ovld vload2(size_t offset, const __local double *p);
double3 __ovld vload3(size_t offset, const __local double *p);
double4 __ovld vload4(size_t offset, const __local double *p);
double8 __ovld vload8(size_t offset, const __local double *p);
double16 __ovld vload16(size_t offset, const __local double *p);
double2 __ovld vload2(size_t offset, const __private double *p);
double3 __ovld vload3(size_t offset, const __private double *p);
double4 __ovld vload4(size_t offset, const __private double *p);
double8 __ovld vload8(size_t offset, const __private double *p);
double16 __ovld vload16(size_t offset, const __private double *p);
#endif //cl_khr_fp64

#ifdef cl_khr_fp16
half __ovld vload(size_t offset, const __global half *p);
half2 __ovld vload2(size_t offset, const __global half *p);
half3 __ovld vload3(size_t offset, const __global half *p);
half4 __ovld vload4(size_t offset, const __global half *p);
half8 __ovld vload8(size_t offset, const __global half *p);
half16 __ovld vload16(size_t offset, const __global half *p);
half __ovld vload(size_t offset, const __local half *p);
half2 __ovld vload2(size_t offset, const __local half *p);
half3 __ovld vload3(size_t offset, const __local half *p);
half4 __ovld vload4(size_t offset, const __local half *p);
half8 __ovld vload8(size_t offset, const __local half *p);
half16 __ovld vload16(size_t offset, const __local half *p);
half __ovld vload(size_t offset, const __private half *p);
half2 __ovld vload2(size_t offset, const __private half *p);
half3 __ovld vload3(size_t offset, const __private half *p);
half4 __ovld vload4(size_t offset, const __private half *p);
half8 __ovld vload8(size_t offset, const __private half *p);
half16 __ovld vload16(size_t offset, const __private half *p);
#endif //cl_khr_fp16

void __ovld vstore2(char2 data, size_t offset, __global char *p);
void __ovld vstore2(uchar2 data, size_t offset, __global uchar *p);
void __ovld vstore2(short2 data, size_t offset, __global short *p);
void __ovld vstore2(ushort2 data, size_t offset, __global ushort *p);
void __ovld vstore2(int2 data, size_t offset, __global int *p);
void __ovld vstore2(uint2 data, size_t offset, __global uint *p);
void __ovld vstore2(long2 data, size_t offset, __global long *p);
void __ovld vstore2(ulong2 data, size_t offset, __global ulong *p);
void __ovld vstore2(float2 data, size_t offset, __global float *p);
void __ovld vstore3(char3 data, size_t offset, __global char *p);
void __ovld vstore3(uchar3 data, size_t offset, __global uchar *p);
void __ovld vstore3(short3 data, size_t offset, __global short *p);
void __ovld vstore3(ushort3 data, size_t offset, __global ushort *p);
void __ovld vstore3(int3 data, size_t offset, __global int *p);
void __ovld vstore3(uint3 data, size_t offset, __global uint *p);
void __ovld vstore3(long3 data, size_t offset, __global long *p);
void __ovld vstore3(ulong3 data, size_t offset, __global ulong *p);
void __ovld vstore3(float3 data, size_t offset, __global float *p);
void __ovld vstore4(char4 data, size_t offset, __global char *p);
void __ovld vstore4(uchar4 data, size_t offset, __global uchar *p);
void __ovld vstore4(short4 data, size_t offset, __global short *p);
void __ovld vstore4(ushort4 data, size_t offset, __global ushort *p);
void __ovld vstore4(int4 data, size_t offset, __global int *p);
void __ovld vstore4(uint4 data, size_t offset, __global uint *p);
void __ovld vstore4(long4 data, size_t offset, __global long *p);
void __ovld vstore4(ulong4 data, size_t offset, __global ulong *p);
void __ovld vstore4(float4 data, size_t offset, __global float *p);
void __ovld vstore8(char8 data, size_t offset, __global char *p);
void __ovld vstore8(uchar8 data, size_t offset, __global uchar *p);
void __ovld vstore8(short8 data, size_t offset, __global short *p);
void __ovld vstore8(ushort8 data, size_t offset, __global ushort *p);
void __ovld vstore8(int8 data, size_t offset, __global int *p);
void __ovld vstore8(uint8 data, size_t offset, __global uint *p);
void __ovld vstore8(long8 data, size_t offset, __global long *p);
void __ovld vstore8(ulong8 data, size_t offset, __global ulong *p);
void __ovld vstore8(float8 data, size_t offset, __global float *p);
void __ovld vstore16(char16 data, size_t offset, __global char *p);
void __ovld vstore16(uchar16 data, size_t offset, __global uchar *p);
void __ovld vstore16(short16 data, size_t offset, __global short *p);
void __ovld vstore16(ushort16 data, size_t offset, __global ushort *p);
void __ovld vstore16(int16 data, size_t offset, __global int *p);
void __ovld vstore16(uint16 data, size_t offset, __global uint *p);
void __ovld vstore16(long16 data, size_t offset, __global long *p);
void __ovld vstore16(ulong16 data, size_t offset, __global ulong *p);
void __ovld vstore16(float16 data, size_t offset, __global float *p);
void __ovld vstore2(char2 data, size_t offset, __local char *p);
void __ovld vstore2(uchar2 data, size_t offset, __local uchar *p);
void __ovld vstore2(short2 data, size_t offset, __local short *p);
void __ovld vstore2(ushort2 data, size_t offset, __local ushort *p);
void __ovld vstore2(int2 data, size_t offset, __local int *p);
void __ovld vstore2(uint2 data, size_t offset, __local uint *p);
void __ovld vstore2(long2 data, size_t offset, __local long *p);
void __ovld vstore2(ulong2 data, size_t offset, __local ulong *p);
void __ovld vstore2(float2 data, size_t offset, __local float *p);
void __ovld vstore3(char3 data, size_t offset, __local char *p);
void __ovld vstore3(uchar3 data, size_t offset, __local uchar *p);
void __ovld vstore3(short3 data, size_t offset, __local short *p);
void __ovld vstore3(ushort3 data, size_t offset, __local ushort *p);
void __ovld vstore3(int3 data, size_t offset, __local int *p);
void __ovld vstore3(uint3 data, size_t offset, __local uint *p);
void __ovld vstore3(long3 data, size_t offset, __local long *p);
void __ovld vstore3(ulong3 data, size_t offset, __local ulong *p);
void __ovld vstore3(float3 data, size_t offset, __local float *p);
void __ovld vstore4(char4 data, size_t offset, __local char *p);
void __ovld vstore4(uchar4 data, size_t offset, __local uchar *p);
void __ovld vstore4(short4 data, size_t offset, __local short *p);
void __ovld vstore4(ushort4 data, size_t offset, __local ushort *p);
void __ovld vstore4(int4 data, size_t offset, __local int *p);
void __ovld vstore4(uint4 data, size_t offset, __local uint *p);
void __ovld vstore4(long4 data, size_t offset, __local long *p);
void __ovld vstore4(ulong4 data, size_t offset, __local ulong *p);
void __ovld vstore4(float4 data, size_t offset, __local float *p);
void __ovld vstore8(char8 data, size_t offset, __local char *p);
void __ovld vstore8(uchar8 data, size_t offset, __local uchar *p);
void __ovld vstore8(short8 data, size_t offset, __local short *p);
void __ovld vstore8(ushort8 data, size_t offset, __local ushort *p);
void __ovld vstore8(int8 data, size_t offset, __local int *p);
void __ovld vstore8(uint8 data, size_t offset, __local uint *p);
void __ovld vstore8(long8 data, size_t offset, __local long *p);
void __ovld vstore8(ulong8 data, size_t offset, __local ulong *p);
void __ovld vstore8(float8 data, size_t offset, __local float *p);
void __ovld vstore16(char16 data, size_t offset, __local char *p);
void __ovld vstore16(uchar16 data, size_t offset, __local uchar *p);
void __ovld vstore16(short16 data, size_t offset, __local short *p);
void __ovld vstore16(ushort16 data, size_t offset, __local ushort *p);
void __ovld vstore16(int16 data, size_t offset, __local int *p);
void __ovld vstore16(uint16 data, size_t offset, __local uint *p);
void __ovld vstore16(long16 data, size_t offset, __local long *p);
void __ovld vstore16(ulong16 data, size_t offset, __local ulong *p);
void __ovld vstore16(float16 data, size_t offset, __local float *p);
void __ovld vstore2(char2 data, size_t offset, __private char *p);
void __ovld vstore2(uchar2 data, size_t offset, __private uchar *p);
void __ovld vstore2(short2 data, size_t offset, __private short *p);
void __ovld vstore2(ushort2 data, size_t offset, __private ushort *p);
void __ovld vstore2(int2 data, size_t offset, __private int *p);
void __ovld vstore2(uint2 data, size_t offset, __private uint *p);
void __ovld vstore2(long2 data, size_t offset, __private long *p);
void __ovld vstore2(ulong2 data, size_t offset, __private ulong *p);
void __ovld vstore2(float2 data, size_t offset, __private float *p);
void __ovld vstore3(char3 data, size_t offset, __private char *p);
void __ovld vstore3(uchar3 data, size_t offset, __private uchar *p);
void __ovld vstore3(short3 data, size_t offset, __private short *p);
void __ovld vstore3(ushort3 data, size_t offset, __private ushort *p);
void __ovld vstore3(int3 data, size_t offset, __private int *p);
void __ovld vstore3(uint3 data, size_t offset, __private uint *p);
void __ovld vstore3(long3 data, size_t offset, __private long *p);
void __ovld vstore3(ulong3 data, size_t offset, __private ulong *p);
void __ovld vstore3(float3 data, size_t offset, __private float *p);
void __ovld vstore4(char4 data, size_t offset, __private char *p);
void __ovld vstore4(uchar4 data, size_t offset, __private uchar *p);
void __ovld vstore4(short4 data, size_t offset, __private short *p);
void __ovld vstore4(ushort4 data, size_t offset, __private ushort *p);
void __ovld vstore4(int4 data, size_t offset, __private int *p);
void __ovld vstore4(uint4 data, size_t offset, __private uint *p);
void __ovld vstore4(long4 data, size_t offset, __private long *p);
void __ovld vstore4(ulong4 data, size_t offset, __private ulong *p);
void __ovld vstore4(float4 data, size_t offset, __private float *p);
void __ovld vstore8(char8 data, size_t offset, __private char *p);
void __ovld vstore8(uchar8 data, size_t offset, __private uchar *p);
void __ovld vstore8(short8 data, size_t offset, __private short *p);
void __ovld vstore8(ushort8 data, size_t offset, __private ushort *p);
void __ovld vstore8(int8 data, size_t offset, __private int *p);
void __ovld vstore8(uint8 data, size_t offset, __private uint *p);
void __ovld vstore8(long8 data, size_t offset, __private long *p);
void __ovld vstore8(ulong8 data, size_t offset, __private ulong *p);
void __ovld vstore8(float8 data, size_t offset, __private float *p);
void __ovld vstore16(char16 data, size_t offset, __private char *p);
void __ovld vstore16(uchar16 data, size_t offset, __private uchar *p);
void __ovld vstore16(short16 data, size_t offset, __private short *p);
void __ovld vstore16(ushort16 data, size_t offset, __private ushort *p);
void __ovld vstore16(int16 data, size_t offset, __private int *p);
void __ovld vstore16(uint16 data, size_t offset, __private uint *p);
void __ovld vstore16(long16 data, size_t offset, __private long *p);
void __ovld vstore16(ulong16 data, size_t offset, __private ulong *p);
void __ovld vstore16(float16 data, size_t offset, __private float *p);
#ifdef cl_khr_fp64
void __ovld vstore2(double2 data, size_t offset, __global double *p);
void __ovld vstore3(double3 data, size_t offset, __global double *p);
void __ovld vstore4(double4 data, size_t offset, __global double *p);
void __ovld vstore8(double8 data, size_t offset, __global double *p);
void __ovld vstore16(double16 data, size_t offset, __global double *p);
void __ovld vstore2(double2 data, size_t offset, __local double *p);
void __ovld vstore3(double3 data, size_t offset, __local double *p);
void __ovld vstore4(double4 data, size_t offset, __local double *p);
void __ovld vstore8(double8 data, size_t offset, __local double *p);
void __ovld vstore16(double16 data, size_t offset, __local double *p);
void __ovld vstore2(double2 data, size_t offset, __private double *p);
void __ovld vstore3(double3 data, size_t offset, __private double *p);
void __ovld vstore4(double4 data, size_t offset, __private double *p);
void __ovld vstore8(double8 data, size_t offset, __private double *p);
void __ovld vstore16(double16 data, size_t offset, __private double *p);
#endif //cl_khr_fp64
#ifdef cl_khr_fp16
void __ovld vstore(half data, size_t offset, __global half *p);
void __ovld vstore2(half2 data, size_t offset, __global half *p);
void __ovld vstore3(half3 data, size_t offset, __global half *p);
void __ovld vstore4(half4 data, size_t offset, __global half *p);
void __ovld vstore8(half8 data, size_t offset, __global half *p);
void __ovld vstore16(half16 data, size_t offset, __global half *p);
void __ovld vstore(half data, size_t offset, __local half *p);
void __ovld vstore2(half2 data, size_t offset, __local half *p);
void __ovld vstore3(half3 data, size_t offset, __local half *p);
void __ovld vstore4(half4 data, size_t offset, __local half *p);
void __ovld vstore8(half8 data, size_t offset, __local half *p);
void __ovld vstore16(half16 data, size_t offset, __local half *p);
void __ovld vstore(half data, size_t offset, __private half *p);
void __ovld vstore2(half2 data, size_t offset, __private half *p);
void __ovld vstore3(half3 data, size_t offset, __private half *p);
void __ovld vstore4(half4 data, size_t offset, __private half *p);
void __ovld vstore8(half8 data, size_t offset, __private half *p);
void __ovld vstore16(half16 data, size_t offset, __private half *p);
#endif //cl_khr_fp16

/**
 * Read sizeof (half) bytes of data from address
 * (p + offset). The data read is interpreted as a
 * half value. The half value is converted to a
 * float value and the float value is returned.
 * The read address computed as (p + offset)
 * must be 16-bit aligned.
 */
float __ovld vload_half(size_t offset, const __global half *p);
float __ovld vload_half(size_t offset, const __local half *p);
float __ovld vload_half(size_t offset, const __private half *p);

/**
 * Read sizeof (halfn) bytes of data from address
 * (p + (offset * n)). The data read is interpreted
 * as a halfn value. The halfn value read is
 * converted to a floatn value and the floatn
 * value is returned. The read address computed
 * as (p + (offset * n)) must be 16-bit aligned.
 */
float2 __ovld vload_half2(size_t offset, const __global half *p);
float3 __ovld vload_half3(size_t offset, const __global half *p);
float4 __ovld vload_half4(size_t offset, const __global half *p);
float8 __ovld vload_half8(size_t offset, const __global half *p);
float16 __ovld vload_half16(size_t offset, const __global half *p);
float2 __ovld vload_half2(size_t offset, const __local half *p);
float3 __ovld vload_half3(size_t offset, const __local half *p);
float4 __ovld vload_half4(size_t offset, const __local half *p);
float8 __ovld vload_half8(size_t offset, const __local half *p);
float16 __ovld vload_half16(size_t offset, const __local half *p);
float2 __ovld vload_half2(size_t offset, const __private half *p);
float3 __ovld vload_half3(size_t offset, const __private half *p);
float4 __ovld vload_half4(size_t offset, const __private half *p);
float8 __ovld vload_half8(size_t offset, const __private half *p);
float16 __ovld vload_half16(size_t offset, const __private half *p);

/**
 * The float value given by data is first
 * converted to a half value using the appropriate
 * rounding mode. The half value is then written
 * to address computed as (p + offset). The
 * address computed as (p + offset) must be 16-
 * bit aligned.
 * vstore_half use the current rounding mode.
 * The default current rounding mode is round to
 * nearest even.
 */
void __ovld vstore_half(float data, size_t offset, __global half *p);
void __ovld vstore_half_rte(float data, size_t offset, __global half *p);
void __ovld vstore_half_rtz(float data, size_t offset, __global half *p);
void __ovld vstore_half_rtp(float data, size_t offset, __global half *p);
void __ovld vstore_half_rtn(float data, size_t offset, __global half *p);
void __ovld vstore_half(float data, size_t offset, __local half *p);
void __ovld vstore_half_rte(float data, size_t offset, __local half *p);
void __ovld vstore_half_rtz(float data, size_t offset, __local half *p);
void __ovld vstore_half_rtp(float data, size_t offset, __local half *p);
void __ovld vstore_half_rtn(float data, size_t offset, __local half *p);
void __ovld vstore_half(float data, size_t offset, __private half *p);
void __ovld vstore_half_rte(float data, size_t offset, __private half *p);
void __ovld vstore_half_rtz(float data, size_t offset, __private half *p);
void __ovld vstore_half_rtp(float data, size_t offset, __private half *p);
void __ovld vstore_half_rtn(float data, size_t offset, __private half *p);
#ifdef cl_khr_fp64
void __ovld vstore_half(double data, size_t offset, __global half *p);
void __ovld vstore_half_rte(double data, size_t offset, __global half *p);
void __ovld vstore_half_rtz(double data, size_t offset, __global half *p);
void __ovld vstore_half_rtp(double data, size_t offset, __global half *p);
void __ovld vstore_half_rtn(double data, size_t offset, __global half *p);
void __ovld vstore_half(double data, size_t offset, __local half *p);
void __ovld vstore_half_rte(double data, size_t offset, __local half *p);
void __ovld vstore_half_rtz(double data, size_t offset, __local half *p);
void __ovld vstore_half_rtp(double data, size_t offset, __local half *p);
void __ovld vstore_half_rtn(double data, size_t offset, __local half *p);
void __ovld vstore_half(double data, size_t offset, __private half *p);
void __ovld vstore_half_rte(double data, size_t offset, __private half *p);
void __ovld vstore_half_rtz(double data, size_t offset, __private half *p);
void __ovld vstore_half_rtp(double data, size_t offset, __private half *p);
void __ovld vstore_half_rtn(double data, size_t offset, __private half *p);
#endif //cl_khr_fp64

/**
 * The floatn value given by data is converted to
 * a halfn value using the appropriate rounding
 * mode. The halfn value is then written to
 * address computed as (p + (offset * n)). The
 * address computed as (p + (offset * n)) must be
 * 16-bit aligned.
 * vstore_halfn uses the current rounding mode.
 * The default current rounding mode is round to
 * nearest even.
 */
void __ovld vstore_half2(float2 data, size_t offset, __global half *p);
void __ovld vstore_half3(float3 data, size_t offset, __global half *p);
void __ovld vstore_half4(float4 data, size_t offset, __global half *p);
void __ovld vstore_half8(float8 data, size_t offset, __global half *p);
void __ovld vstore_half16(float16 data, size_t offset, __global half *p);
void __ovld vstore_half2_rte(float2 data, size_t offset, __global half *p);
void __ovld vstore_half3_rte(float3 data, size_t offset, __global half *p);
void __ovld vstore_half4_rte(float4 data, size_t offset, __global half *p);
void __ovld vstore_half8_rte(float8 data, size_t offset, __global half *p);
void __ovld vstore_half16_rte(float16 data, size_t offset, __global half *p);
void __ovld vstore_half2_rtz(float2 data, size_t offset, __global half *p);
void __ovld vstore_half3_rtz(float3 data, size_t offset, __global half *p);
void __ovld vstore_half4_rtz(float4 data, size_t offset, __global half *p);
void __ovld vstore_half8_rtz(float8 data, size_t offset, __global half *p);
void __ovld vstore_half16_rtz(float16 data, size_t offset, __global half *p);
void __ovld vstore_half2_rtp(float2 data, size_t offset, __global half *p);
void __ovld vstore_half3_rtp(float3 data, size_t offset, __global half *p);
void __ovld vstore_half4_rtp(float4 data, size_t offset, __global half *p);
void __ovld vstore_half8_rtp(float8 data, size_t offset, __global half *p);
void __ovld vstore_half16_rtp(float16 data, size_t offset, __global half *p);
void __ovld vstore_half2_rtn(float2 data, size_t offset, __global half *p);
void __ovld vstore_half3_rtn(float3 data, size_t offset, __global half *p);
void __ovld vstore_half4_rtn(float4 data, size_t offset, __global half *p);
void __ovld vstore_half8_rtn(float8 data, size_t offset, __global half *p);
void __ovld vstore_half16_rtn(float16 data, size_t offset, __global half *p);
void __ovld vstore_half2(float2 data, size_t offset, __local half *p);
void __ovld vstore_half3(float3 data, size_t offset, __local half *p);
void __ovld vstore_half4(float4 data, size_t offset, __local half *p);
void __ovld vstore_half8(float8 data, size_t offset, __local half *p);
void __ovld vstore_half16(float16 data, size_t offset, __local half *p);
void __ovld vstore_half2_rte(float2 data, size_t offset, __local half *p);
void __ovld vstore_half3_rte(float3 data, size_t offset, __local half *p);
void __ovld vstore_half4_rte(float4 data, size_t offset, __local half *p);
void __ovld vstore_half8_rte(float8 data, size_t offset, __local half *p);
void __ovld vstore_half16_rte(float16 data, size_t offset, __local half *p);
void __ovld vstore_half2_rtz(float2 data, size_t offset, __local half *p);
void __ovld vstore_half3_rtz(float3 data, size_t offset, __local half *p);
void __ovld vstore_half4_rtz(float4 data, size_t offset, __local half *p);
void __ovld vstore_half8_rtz(float8 data, size_t offset, __local half *p);
void __ovld vstore_half16_rtz(float16 data, size_t offset, __local half *p);
void __ovld vstore_half2_rtp(float2 data, size_t offset, __local half *p);
void __ovld vstore_half3_rtp(float3 data, size_t offset, __local half *p);
void __ovld vstore_half4_rtp(float4 data, size_t offset, __local half *p);
void __ovld vstore_half8_rtp(float8 data, size_t offset, __local half *p);
void __ovld vstore_half16_rtp(float16 data, size_t offset, __local half *p);
void __ovld vstore_half2_rtn(float2 data, size_t offset, __local half *p);
void __ovld vstore_half3_rtn(float3 data, size_t offset, __local half *p);
void __ovld vstore_half4_rtn(float4 data, size_t offset, __local half *p);
void __ovld vstore_half8_rtn(float8 data, size_t offset, __local half *p);
void __ovld vstore_half16_rtn(float16 data, size_t offset, __local half *p);
void __ovld vstore_half2(float2 data, size_t offset, __private half *p);
void __ovld vstore_half3(float3 data, size_t offset, __private half *p);
void __ovld vstore_half4(float4 data, size_t offset, __private half *p);
void __ovld vstore_half8(float8 data, size_t offset, __private half *p);
void __ovld vstore_half16(float16 data, size_t offset, __private half *p);
void __ovld vstore_half2_rte(float2 data, size_t offset, __private half *p);
void __ovld vstore_half3_rte(float3 data, size_t offset, __private half *p);
void __ovld vstore_half4_rte(float4 data, size_t offset, __private half *p);
void __ovld vstore_half8_rte(float8 data, size_t offset, __private half *p);
void __ovld vstore_half16_rte(float16 data, size_t offset, __private half *p);
void __ovld vstore_half2_rtz(float2 data, size_t offset, __private half *p);
void __ovld vstore_half3_rtz(float3 data, size_t offset, __private half *p);
void __ovld vstore_half4_rtz(float4 data, size_t offset, __private half *p);
void __ovld vstore_half8_rtz(float8 data, size_t offset, __private half *p);
void __ovld vstore_half16_rtz(float16 data, size_t offset, __private half *p);
void __ovld vstore_half2_rtp(float2 data, size_t offset, __private half *p);
void __ovld vstore_half3_rtp(float3 data, size_t offset, __private half *p);
void __ovld vstore_half4_rtp(float4 data, size_t offset, __private half *p);
void __ovld vstore_half8_rtp(float8 data, size_t offset, __private half *p);
void __ovld vstore_half16_rtp(float16 data, size_t offset, __private half *p);
void __ovld vstore_half2_rtn(float2 data, size_t offset, __private half *p);
void __ovld vstore_half3_rtn(float3 data, size_t offset, __private half *p);
void __ovld vstore_half4_rtn(float4 data, size_t offset, __private half *p);
void __ovld vstore_half8_rtn(float8 data, size_t offset, __private half *p);
void __ovld vstore_half16_rtn(float16 data, size_t offset, __private half *p);
#ifdef cl_khr_fp64
void __ovld vstore_half2(double2 data, size_t offset, __global half *p);
void __ovld vstore_half3(double3 data, size_t offset, __global half *p);
void __ovld vstore_half4(double4 data, size_t offset, __global half *p);
void __ovld vstore_half8(double8 data, size_t offset, __global half *p);
void __ovld vstore_half16(double16 data, size_t offset, __global half *p);
void __ovld vstore_half2_rte(double2 data, size_t offset, __global half *p);
void __ovld vstore_half3_rte(double3 data, size_t offset, __global half *p);
void __ovld vstore_half4_rte(double4 data, size_t offset, __global half *p);
void __ovld vstore_half8_rte(double8 data, size_t offset, __global half *p);
void __ovld vstore_half16_rte(double16 data, size_t offset, __global half *p);
void __ovld vstore_half2_rtz(double2 data, size_t offset, __global half *p);
void __ovld vstore_half3_rtz(double3 data, size_t offset, __global half *p);
void __ovld vstore_half4_rtz(double4 data, size_t offset, __global half *p);
void __ovld vstore_half8_rtz(double8 data, size_t offset, __global half *p);
void __ovld vstore_half16_rtz(double16 data, size_t offset, __global half *p);
void __ovld vstore_half2_rtp(double2 data, size_t offset, __global half *p);
void __ovld vstore_half3_rtp(double3 data, size_t offset, __global half *p);
void __ovld vstore_half4_rtp(double4 data, size_t offset, __global half *p);
void __ovld vstore_half8_rtp(double8 data, size_t offset, __global half *p);
void __ovld vstore_half16_rtp(double16 data, size_t offset, __global half *p);
void __ovld vstore_half2_rtn(double2 data, size_t offset, __global half *p);
void __ovld vstore_half3_rtn(double3 data, size_t offset, __global half *p);
void __ovld vstore_half4_rtn(double4 data, size_t offset, __global half *p);
void __ovld vstore_half8_rtn(double8 data, size_t offset, __global half *p);
void __ovld vstore_half16_rtn(double16 data, size_t offset, __global half *p);
void __ovld vstore_half2(double2 data, size_t offset, __local half *p);
void __ovld vstore_half3(double3 data, size_t offset, __local half *p);
void __ovld vstore_half4(double4 data, size_t offset, __local half *p);
void __ovld vstore_half8(double8 data, size_t offset, __local half *p);
void __ovld vstore_half16(double16 data, size_t offset, __local half *p);
void __ovld vstore_half2_rte(double2 data, size_t offset, __local half *p);
void __ovld vstore_half3_rte(double3 data, size_t offset, __local half *p);
void __ovld vstore_half4_rte(double4 data, size_t offset, __local half *p);
void __ovld vstore_half8_rte(double8 data, size_t offset, __local half *p);
void __ovld vstore_half16_rte(double16 data, size_t offset, __local half *p);
void __ovld vstore_half2_rtz(double2 data, size_t offset, __local half *p);
void __ovld vstore_half3_rtz(double3 data, size_t offset, __local half *p);
void __ovld vstore_half4_rtz(double4 data, size_t offset, __local half *p);
void __ovld vstore_half8_rtz(double8 data, size_t offset, __local half *p);
void __ovld vstore_half16_rtz(double16 data, size_t offset, __local half *p);
void __ovld vstore_half2_rtp(double2 data, size_t offset, __local half *p);
void __ovld vstore_half3_rtp(double3 data, size_t offset, __local half *p);
void __ovld vstore_half4_rtp(double4 data, size_t offset, __local half *p);
void __ovld vstore_half8_rtp(double8 data, size_t offset, __local half *p);
void __ovld vstore_half16_rtp(double16 data, size_t offset, __local half *p);
void __ovld vstore_half2_rtn(double2 data, size_t offset, __local half *p);
void __ovld vstore_half3_rtn(double3 data, size_t offset, __local half *p);
void __ovld vstore_half4_rtn(double4 data, size_t offset, __local half *p);
void __ovld vstore_half8_rtn(double8 data, size_t offset, __local half *p);
void __ovld vstore_half16_rtn(double16 data, size_t offset, __local half *p);
void __ovld vstore_half2(double2 data, size_t offset, __private half *p);
void __ovld vstore_half3(double3 data, size_t offset, __private half *p);
void __ovld vstore_half4(double4 data, size_t offset, __private half *p);
void __ovld vstore_half8(double8 data, size_t offset, __private half *p);
void __ovld vstore_half16(double16 data, size_t offset, __private half *p);
void __ovld vstore_half2_rte(double2 data, size_t offset, __private half *p);
void __ovld vstore_half3_rte(double3 data, size_t offset, __private half *p);
void __ovld vstore_half4_rte(double4 data, size_t offset, __private half *p);
void __ovld vstore_half8_rte(double8 data, size_t offset, __private half *p);
void __ovld vstore_half16_rte(double16 data, size_t offset, __private half *p);
void __ovld vstore_half2_rtz(double2 data, size_t offset, __private half *p);
void __ovld vstore_half3_rtz(double3 data, size_t offset, __private half *p);
void __ovld vstore_half4_rtz(double4 data, size_t offset, __private half *p);
void __ovld vstore_half8_rtz(double8 data, size_t offset, __private half *p);
void __ovld vstore_half16_rtz(double16 data, size_t offset, __private half *p);
void __ovld vstore_half2_rtp(double2 data, size_t offset, __private half *p);
void __ovld vstore_half3_rtp(double3 data, size_t offset, __private half *p);
void __ovld vstore_half4_rtp(double4 data, size_t offset, __private half *p);
void __ovld vstore_half8_rtp(double8 data, size_t offset, __private half *p);
void __ovld vstore_half16_rtp(double16 data, size_t offset, __private half *p);
void __ovld vstore_half2_rtn(double2 data, size_t offset, __private half *p);
void __ovld vstore_half3_rtn(double3 data, size_t offset, __private half *p);
void __ovld vstore_half4_rtn(double4 data, size_t offset, __private half *p);
void __ovld vstore_half8_rtn(double8 data, size_t offset, __private half *p);
void __ovld vstore_half16_rtn(double16 data, size_t offset, __private half *p);
#endif //cl_khr_fp64

/**
 * For n = 1, 2, 4, 8 and 16 read sizeof (halfn)
 * bytes of data from address (p + (offset * n)).
 * The data read is interpreted as a halfn value.
 * The halfn value read is converted to a floatn
 * value and the floatn value is returned.
 * The address computed as (p + (offset * n))
 * must be aligned to sizeof (halfn) bytes.
 * For n = 3, vloada_half3 reads a half3 from
 * address (p + (offset * 4)) and returns a float3.
 * The address computed as (p + (offset * 4))
 * must be aligned to sizeof (half) * 4 bytes.
 */
float __ovld vloada_half(size_t offset, const __global half *p);
float2 __ovld vloada_half2(size_t offset, const __global half *p);
float3 __ovld vloada_half3(size_t offset, const __global half *p);
float4 __ovld vloada_half4(size_t offset, const __global half *p);
float8 __ovld vloada_half8(size_t offset, const __global half *p);
float16 __ovld vloada_half16(size_t offset, const __global half *p);
float __ovld vloada_half(size_t offset, const __local half *p);
float2 __ovld vloada_half2(size_t offset, const __local half *p);
float3 __ovld vloada_half3(size_t offset, const __local half *p);
float4 __ovld vloada_half4(size_t offset, const __local half *p);
float8 __ovld vloada_half8(size_t offset, const __local half *p);
float16 __ovld vloada_half16(size_t offset, const __local half *p);
float __ovld vloada_half(size_t offset, const __private half *p);
float2 __ovld vloada_half2(size_t offset, const __private half *p);
float3 __ovld vloada_half3(size_t offset, const __private half *p);
float4 __ovld vloada_half4(size_t offset, const __private half *p);
float8 __ovld vloada_half8(size_t offset, const __private half *p);
float16 __ovld vloada_half16(size_t offset, const __private half *p);

/**
 * The floatn value given by data is converted to
 * a halfn value using the appropriate rounding
 * mode.
 * For n = 1, 2, 4, 8 and 16, the halfn value is
 * written to the address computed as (p + (offset
 * * n)). The address computed as (p + (offset *
 * n)) must be aligned to sizeof (halfn) bytes.
 * For n = 3, the half3 value is written to the
 * address computed as (p + (offset * 4)). The
 * address computed as (p + (offset * 4)) must be
 * aligned to sizeof (half) * 4 bytes.
 * vstorea_halfn uses the current rounding
 * mode. The default current rounding mode is
 * round to nearest even.
 */
void __ovld vstorea_half(float data, size_t offset, __global half *p);
void __ovld vstorea_half2(float2 data, size_t offset, __global half *p);
void __ovld vstorea_half3(float3 data, size_t offset, __global half *p);
void __ovld vstorea_half4(float4 data, size_t offset, __global half *p);
void __ovld vstorea_half8(float8 data, size_t offset, __global half *p);
void __ovld vstorea_half16(float16 data, size_t offset, __global half *p);

void __ovld vstorea_half_rte(float data, size_t offset, __global half *p);
void __ovld vstorea_half2_rte(float2 data, size_t offset, __global half *p);
void __ovld vstorea_half3_rte(float3 data, size_t offset, __global half *p);
void __ovld vstorea_half4_rte(float4 data, size_t offset, __global half *p);
void __ovld vstorea_half8_rte(float8 data, size_t offset, __global half *p);
void __ovld vstorea_half16_rte(float16 data, size_t offset, __global half *p);

void __ovld vstorea_half_rtz(float data, size_t offset, __global half *p);
void __ovld vstorea_half2_rtz(float2 data, size_t offset, __global half *p);
void __ovld vstorea_half3_rtz(float3 data, size_t offset, __global half *p);
void __ovld vstorea_half4_rtz(float4 data, size_t offset, __global half *p);
void __ovld vstorea_half8_rtz(float8 data, size_t offset, __global half *p);
void __ovld vstorea_half16_rtz(float16 data, size_t offset, __global half *p);

void __ovld vstorea_half_rtp(float data, size_t offset, __global half *p);
void __ovld vstorea_half2_rtp(float2 data, size_t offset, __global half *p);
void __ovld vstorea_half3_rtp(float3 data, size_t offset, __global half *p);
void __ovld vstorea_half4_rtp(float4 data, size_t offset, __global half *p);
void __ovld vstorea_half8_rtp(float8 data, size_t offset, __global half *p);
void __ovld vstorea_half16_rtp(float16 data, size_t offset, __global half *p);

void __ovld vstorea_half_rtn(float data, size_t offset, __global half *p);
void __ovld vstorea_half2_rtn(float2 data, size_t offset, __global half *p);
void __ovld vstorea_half3_rtn(float3 data, size_t offset, __global half *p);
void __ovld vstorea_half4_rtn(float4 data, size_t offset, __global half *p);
void __ovld vstorea_half8_rtn(float8 data, size_t offset, __global half *p);
void __ovld vstorea_half16_rtn(float16 data, size_t offset, __global half *p);

void __ovld vstorea_half(float data, size_t offset, __local half *p);
void __ovld vstorea_half2(float2 data, size_t offset, __local half *p);
void __ovld vstorea_half3(float3 data, size_t offset, __local half *p);
void __ovld vstorea_half4(float4 data, size_t offset, __local half *p);
void __ovld vstorea_half8(float8 data, size_t offset, __local half *p);
void __ovld vstorea_half16(float16 data, size_t offset, __local half *p);

void __ovld vstorea_half_rte(float data, size_t offset, __local half *p);
void __ovld vstorea_half2_rte(float2 data, size_t offset, __local half *p);
void __ovld vstorea_half3_rte(float3 data, size_t offset, __local half *p);
void __ovld vstorea_half4_rte(float4 data, size_t offset, __local half *p);
void __ovld vstorea_half8_rte(float8 data, size_t offset, __local half *p);
void __ovld vstorea_half16_rte(float16 data, size_t offset, __local half *p);

void __ovld vstorea_half_rtz(float data, size_t offset, __local half *p);
void __ovld vstorea_half2_rtz(float2 data, size_t offset, __local half *p);
void __ovld vstorea_half3_rtz(float3 data, size_t offset, __local half *p);
void __ovld vstorea_half4_rtz(float4 data, size_t offset, __local half *p);
void __ovld vstorea_half8_rtz(float8 data, size_t offset, __local half *p);
void __ovld vstorea_half16_rtz(float16 data, size_t offset, __local half *p);

void __ovld vstorea_half_rtp(float data, size_t offset, __local half *p);
void __ovld vstorea_half2_rtp(float2 data, size_t offset, __local half *p);
void __ovld vstorea_half3_rtp(float3 data, size_t offset, __local half *p);
void __ovld vstorea_half4_rtp(float4 data, size_t offset, __local half *p);
void __ovld vstorea_half8_rtp(float8 data, size_t offset, __local half *p);
void __ovld vstorea_half16_rtp(float16 data, size_t offset, __local half *p);

void __ovld vstorea_half_rtn(float data, size_t offset, __local half *p);
void __ovld vstorea_half2_rtn(float2 data, size_t offset, __local half *p);
void __ovld vstorea_half3_rtn(float3 data, size_t offset, __local half *p);
void __ovld vstorea_half4_rtn(float4 data, size_t offset, __local half *p);
void __ovld vstorea_half8_rtn(float8 data, size_t offset, __local half *p);
void __ovld vstorea_half16_rtn(float16 data, size_t offset, __local half *p);

void __ovld vstorea_half(float data, size_t offset, __private half *p);
void __ovld vstorea_half2(float2 data, size_t offset, __private half *p);
void __ovld vstorea_half3(float3 data, size_t offset, __private half *p);
void __ovld vstorea_half4(float4 data, size_t offset, __private half *p);
void __ovld vstorea_half8(float8 data, size_t offset, __private half *p);
void __ovld vstorea_half16(float16 data, size_t offset, __private half *p);

void __ovld vstorea_half_rte(float data, size_t offset, __private half *p);
void __ovld vstorea_half2_rte(float2 data, size_t offset, __private half *p);
void __ovld vstorea_half3_rte(float3 data, size_t offset, __private half *p);
void __ovld vstorea_half4_rte(float4 data, size_t offset, __private half *p);
void __ovld vstorea_half8_rte(float8 data, size_t offset, __private half *p);
void __ovld vstorea_half16_rte(float16 data, size_t offset, __private half *p);

void __ovld vstorea_half_rtz(float data, size_t offset, __private half *p);
void __ovld vstorea_half2_rtz(float2 data, size_t offset, __private half *p);
void __ovld vstorea_half3_rtz(float3 data, size_t offset, __private half *p);
void __ovld vstorea_half4_rtz(float4 data, size_t offset, __private half *p);
void __ovld vstorea_half8_rtz(float8 data, size_t offset, __private half *p);
void __ovld vstorea_half16_rtz(float16 data, size_t offset, __private half *p);

void __ovld vstorea_half_rtp(float data, size_t offset, __private half *p);
void __ovld vstorea_half2_rtp(float2 data, size_t offset, __private half *p);
void __ovld vstorea_half3_rtp(float3 data, size_t offset, __private half *p);
void __ovld vstorea_half4_rtp(float4 data, size_t offset, __private half *p);
void __ovld vstorea_half8_rtp(float8 data, size_t offset, __private half *p);
void __ovld vstorea_half16_rtp(float16 data, size_t offset, __private half *p);

void __ovld vstorea_half_rtn(float data, size_t offset, __private half *p);
void __ovld vstorea_half2_rtn(float2 data, size_t offset, __private half *p);
void __ovld vstorea_half3_rtn(float3 data, size_t offset, __private half *p);
void __ovld vstorea_half4_rtn(float4 data, size_t offset, __private half *p);
void __ovld vstorea_half8_rtn(float8 data, size_t offset, __private half *p);
void __ovld vstorea_half16_rtn(float16 data, size_t offset, __private half *p);

#ifdef cl_khr_fp64
void __ovld vstorea_half(double data, size_t offset, __global half *p);
void __ovld vstorea_half2(double2 data, size_t offset, __global half *p);
void __ovld vstorea_half3(double3 data, size_t offset, __global half *p);
void __ovld vstorea_half4(double4 data, size_t offset, __global half *p);
void __ovld vstorea_half8(double8 data, size_t offset, __global half *p);
void __ovld vstorea_half16(double16 data, size_t offset, __global half *p);

void __ovld vstorea_half_rte(double data, size_t offset, __global half *p);
void __ovld vstorea_half2_rte(double2 data, size_t offset, __global half *p);
void __ovld vstorea_half3_rte(double3 data, size_t offset, __global half *p);
void __ovld vstorea_half4_rte(double4 data, size_t offset, __global half *p);
void __ovld vstorea_half8_rte(double8 data, size_t offset, __global half *p);
void __ovld vstorea_half16_rte(double16 data, size_t offset, __global half *p);

void __ovld vstorea_half_rtz(double data, size_t offset, __global half *p);
void __ovld vstorea_half2_rtz(double2 data, size_t offset, __global half *p);
void __ovld vstorea_half3_rtz(double3 data, size_t offset, __global half *p);
void __ovld vstorea_half4_rtz(double4 data, size_t offset, __global half *p);
void __ovld vstorea_half8_rtz(double8 data, size_t offset, __global half *p);
void __ovld vstorea_half16_rtz(double16 data, size_t offset, __global half *p);

void __ovld vstorea_half_rtp(double data, size_t offset, __global half *p);
void __ovld vstorea_half2_rtp(double2 data, size_t offset, __global half *p);
void __ovld vstorea_half3_rtp(double3 data, size_t offset, __global half *p);
void __ovld vstorea_half4_rtp(double4 data, size_t offset, __global half *p);
void __ovld vstorea_half8_rtp(double8 data, size_t offset, __global half *p);
void __ovld vstorea_half16_rtp(double16 data, size_t offset, __global half *p);

void __ovld vstorea_half_rtn(double data, size_t offset, __global half *p);
void __ovld vstorea_half2_rtn(double2 data, size_t offset, __global half *p);
void __ovld vstorea_half3_rtn(double3 data, size_t offset, __global half *p);
void __ovld vstorea_half4_rtn(double4 data, size_t offset, __global half *p);
void __ovld vstorea_half8_rtn(double8 data, size_t offset, __global half *p);
void __ovld vstorea_half16_rtn(double16 data, size_t offset, __global half *p);

void __ovld vstorea_half(double data, size_t offset, __local half *p);
void __ovld vstorea_half2(double2 data, size_t offset, __local half *p);
void __ovld vstorea_half3(double3 data, size_t offset, __local half *p);
void __ovld vstorea_half4(double4 data, size_t offset, __local half *p);
void __ovld vstorea_half8(double8 data, size_t offset, __local half *p);
void __ovld vstorea_half16(double16 data, size_t offset, __local half *p);

void __ovld vstorea_half_rte(double data, size_t offset, __local half *p);
void __ovld vstorea_half2_rte(double2 data, size_t offset, __local half *p);
void __ovld vstorea_half3_rte(double3 data, size_t offset, __local half *p);
void __ovld vstorea_half4_rte(double4 data, size_t offset, __local half *p);
void __ovld vstorea_half8_rte(double8 data, size_t offset, __local half *p);
void __ovld vstorea_half16_rte(double16 data, size_t offset, __local half *p);

void __ovld vstorea_half_rtz(double data, size_t offset, __local half *p);
void __ovld vstorea_half2_rtz(double2 data, size_t offset, __local half *p);
void __ovld vstorea_half3_rtz(double3 data, size_t offset, __local half *p);
void __ovld vstorea_half4_rtz(double4 data, size_t offset, __local half *p);
void __ovld vstorea_half8_rtz(double8 data, size_t offset, __local half *p);
void __ovld vstorea_half16_rtz(double16 data, size_t offset, __local half *p);

void __ovld vstorea_half_rtp(double data, size_t offset, __local half *p);
void __ovld vstorea_half2_rtp(double2 data, size_t offset, __local half *p);
void __ovld vstorea_half3_rtp(double3 data, size_t offset, __local half *p);
void __ovld vstorea_half4_rtp(double4 data, size_t offset, __local half *p);
void __ovld vstorea_half8_rtp(double8 data, size_t offset, __local half *p);
void __ovld vstorea_half16_rtp(double16 data, size_t offset, __local half *p);

void __ovld vstorea_half_rtn(double data, size_t offset, __local half *p);
void __ovld vstorea_half2_rtn(double2 data, size_t offset, __local half *p);
void __ovld vstorea_half3_rtn(double3 data, size_t offset, __local half *p);
void __ovld vstorea_half4_rtn(double4 data, size_t offset, __local half *p);
void __ovld vstorea_half8_rtn(double8 data, size_t offset, __local half *p);
void __ovld vstorea_half16_rtn(double16 data, size_t offset, __local half *p);

void __ovld vstorea_half(double data, size_t offset, __private half *p);
void __ovld vstorea_half2(double2 data, size_t offset, __private half *p);
void __ovld vstorea_half3(double3 data, size_t offset, __private half *p);
void __ovld vstorea_half4(double4 data, size_t offset, __private half *p);
void __ovld vstorea_half8(double8 data, size_t offset, __private half *p);
void __ovld vstorea_half16(double16 data, size_t offset, __private half *p);

void __ovld vstorea_half_rte(double data, size_t offset, __private half *p);
void __ovld vstorea_half2_rte(double2 data, size_t offset, __private half *p);
void __ovld vstorea_half3_rte(double3 data, size_t offset, __private half *p);
void __ovld vstorea_half4_rte(double4 data, size_t offset, __private half *p);
void __ovld vstorea_half8_rte(double8 data, size_t offset, __private half *p);
void __ovld vstorea_half16_rte(double16 data, size_t offset, __private half *p);

void __ovld vstorea_half_rtz(double data, size_t offset, __private half *p);
void __ovld vstorea_half2_rtz(double2 data, size_t offset, __private half *p);
void __ovld vstorea_half3_rtz(double3 data, size_t offset, __private half *p);
void __ovld vstorea_half4_rtz(double4 data, size_t offset, __private half *p);
void __ovld vstorea_half8_rtz(double8 data, size_t offset, __private half *p);
void __ovld vstorea_half16_rtz(double16 data, size_t offset, __private half *p);

void __ovld vstorea_half_rtp(double data, size_t offset, __private half *p);
void __ovld vstorea_half2_rtp(double2 data, size_t offset, __private half *p);
void __ovld vstorea_half3_rtp(double3 data, size_t offset, __private half *p);
void __ovld vstorea_half4_rtp(double4 data, size_t offset, __private half *p);
void __ovld vstorea_half8_rtp(double8 data, size_t offset, __private half *p);
void __ovld vstorea_half16_rtp(double16 data, size_t offset, __private half *p);

void __ovld vstorea_half_rtn(double data, size_t offset, __private half *p);
void __ovld vstorea_half2_rtn(double2 data,size_t offset, __private half *p);
void __ovld vstorea_half3_rtn(double3 data,size_t offset, __private half *p);
void __ovld vstorea_half4_rtn(double4 data,size_t offset, __private half *p);
void __ovld vstorea_half8_rtn(double8 data,size_t offset, __private half *p);
void __ovld vstorea_half16_rtn(double16 data,size_t offset, __private half *p);
#endif //cl_khr_fp64

#endif //_OPENCL_PLATFORM_12_H_
