//==--- cmath_wrapper.cpp - wrappers for C math library functions ----------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifdef __SYCL_DEVICE_ONLY__
#include "device_math.h"
extern "C" {
// All exported functions in math and complex device libraries are weak
// reference. If users provide their own math or complex functions(with
// the prototype), functions in device libraries will be ignored and
// overrided by users' version.
SYCL_EXTERNAL
float __attribute__((weak)) scalbnf(float x, int n) {
  return __devicelib_scalbnf(x, n);
}

SYCL_EXTERNAL
float __attribute__((weak)) logf(float x) {
  return __devicelib_logf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) log(double x) {
  return __devicelib_log(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) expf(float x) {
  return __devicelib_expf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) exp(double x) {
  return __devicelib_exp(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) frexpf(float x, int *exp) {
  return __devicelib_frexpf(x, exp);
}

SYCL_EXTERNAL
double __attribute__((weak)) frexp(double x, int *exp) {
  return __devicelib_frexp(x, exp);
}

SYCL_EXTERNAL
float __attribute__((weak)) ldexpf(float x, int exp) {
  return __devicelib_ldexpf(x, exp);
}

SYCL_EXTERNAL
double __attribute__((weak)) ldexp(double x, int exp) {
  return __devicelib_ldexp(x, exp);
}

SYCL_EXTERNAL
float __attribute__((weak)) log10f(float x) {
  return __devicelib_log10f(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) log10(double x) {
  return __devicelib_log10(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) modff(float x, float *intpart) {
  return __devicelib_modff(x, intpart);
}

SYCL_EXTERNAL
double __attribute__((weak)) modf(double x, double *intpart) {
  return __devicelib_modf(x, intpart);
}

SYCL_EXTERNAL
float __attribute__((weak)) exp2f(float x) {
  return __devicelib_exp2f(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) exp2(double x) {
  return __devicelib_exp2(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) expm1f(float x) {
  return __devicelib_expm1f(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) expm1(double x) {
  return __devicelib_expm1(x);
}

SYCL_EXTERNAL
int __attribute__((weak)) ilogbf(float x) {
  return __devicelib_ilogbf(x);
}

SYCL_EXTERNAL
int __attribute__((weak)) ilogb(double x) {
  return __devicelib_ilogb(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) log1pf(float x) {
  return __devicelib_log1pf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) log1p(double x) {
  return __devicelib_log1p(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) log2f(float x) {
  return __devicelib_log2f(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) log2(double x) {
  return __devicelib_log2(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) logbf(float x) {
  return __devicelib_logbf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) logb(double x) {
  return __devicelib_logb(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) sqrtf(float x) {
  return __devicelib_sqrtf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) sqrt(double x) {
  return __devicelib_sqrt(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) cbrtf(float x) {
  return __devicelib_cbrtf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) cbrt(double x) {
  return __devicelib_cbrt(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) hypotf(float x, float y) {
  return __devicelib_hypotf(x, y);
}

SYCL_EXTERNAL
double __attribute__((weak)) hypot(double x, double y) {
  return __devicelib_hypot(x, y);
}

SYCL_EXTERNAL
float __attribute__((weak)) erff(float x) {
  return __devicelib_erff(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) erf(double x) {
  return __devicelib_erff(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) erfcf(float x) {
  return __devicelib_erfcf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) erfc(double x) {
  return __devicelib_erfc(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) tgammaf(float x) {
  return __devicelib_tgammaf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) tgamma(double x) {
  return __devicelib_tgamma(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) lgammaf(float x) {
  return __devicelib_lgammaf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) lgamma(double x) {
  return __devicelib_lgamma(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) fmodf(float x, float y) {
  return __devicelib_fmodf(x, y);
}

SYCL_EXTERNAL
double __attribute__((weak)) fmod(double x, double y) {
  return __devicelib_fmod(x, y);
}

SYCL_EXTERNAL
float __attribute__((weak)) remainderf(float x, float y) {
  return __devicelib_remainderf(x, y);
}

SYCL_EXTERNAL
double __attribute__((weak)) remainder(double x, double y) {
  return __devicelib_remainder(x, y);
}

SYCL_EXTERNAL
float __attribute__((weak)) remquof(float x, float y, int *q) {
  return __devicelib_remquof(x, y, q);
}

SYCL_EXTERNAL
double __attribute__((weak)) remquo(double x, double y, int *q) {
  return __devicelib_remquo(x, y, q);
}

SYCL_EXTERNAL
float __attribute__((weak)) nextafterf(float x, float y) {
  return __devicelib_nextafterf(x, y);
}

SYCL_EXTERNAL
double __attribute__((weak)) nextafter(double x, double y) {
  return __devicelib_nextafter(x, y);
}

SYCL_EXTERNAL
float __attribute__((weak)) fdimf(float x, float y) {
  return __devicelib_fdimf(x, y);
}

SYCL_EXTERNAL
double __attribute__((weak)) fdim(double x, double y) {
  return __devicelib_fdim(x, y);
}

SYCL_EXTERNAL
float __attribute__((weak)) fmaf(float x, float y, float z) {
  return __devicelib_fmaf(x, y, z);
}

SYCL_EXTERNAL
double __attribute__((weak)) fma(double x, double y, double z) {
  return __devicelib_fma(x, y, z);
}

SYCL_EXTERNAL
float __attribute__((weak)) sinf(float x) {
  return __devicelib_sinf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) sin(double x) {
  return __devicelib_sin(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) cosf(float x) {
  return __devicelib_cosf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) cos(double x) {
  return __devicelib_cos(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) tanf(float x) {
  return __devicelib_tanf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) tan(double x) {
  return __devicelib_tan(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) pow(double x, double y) {
  return __devicelib_pow(x, y);
}

SYCL_EXTERNAL
float __attribute__((weak)) powf(float x, float y) {
  return __devicelib_powf(x, y);
}

SYCL_EXTERNAL
float __attribute__ ((weak)) acosf(float x) {
  return __devicelib_acosf(x);
}

SYCL_EXTERNAL
double __attribute__ ((weak)) acos(double x) {
  return __devicelib_acos(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) asinf(float x) {
  return __devicelib_asinf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) asin(double x) {
  return __devicelib_asin(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) atanf(float x) {
  return __devicelib_atanf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) atan(double x) {
  return __devicelib_atan(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) atan2f(float x, float y) {
  return __devicelib_atan2f(x, y);
}

SYCL_EXTERNAL
double __attribute__((weak)) atan2(double x, double y) {
  return __devicelib_atan2(x, y);
}

SYCL_EXTERNAL
float __attribute__((weak)) coshf(float x) {
  return __devicelib_coshf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) cosh(double x) {
  return __devicelib_cosh(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) sinhf(float x) {
  return __devicelib_sinhf(x);
}

SYCL_EXTERNAL
double  __attribute__((weak)) sinh(double x) {
  return __devicelib_sinh(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) tanhf(float x) {
  return __devicelib_tanhf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) tanh(double x) {
  return __devicelib_tanh(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) acoshf(float x) {
  return __devicelib_acoshf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) acosh(double x) {
  return __devicelib_acosh(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) asinhf(float x) {
  return __devicelib_asinhf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) asinh(double x) {
  return __devicelib_asinh(x);
}

SYCL_EXTERNAL
float __attribute__((weak)) atanhf(float x) {
  return __devicelib_atanhf(x);
}

SYCL_EXTERNAL
double __attribute__((weak)) atanh(double x) {
  return __devicelib_atanh(x);
}
}
#endif
