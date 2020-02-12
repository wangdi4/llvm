//==--- complex_wrapper.cpp - wrappers for C99 complex math functions ------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifdef __SYCL_DEVICE_ONLY__
#include "device_complex.h"
extern "C" {
SYCL_EXTERNAL
float __attribute__((weak)) cimagf(float __complex__ z) {
  return __devicelib_cimagf(z);
}

SYCL_EXTERNAL
float __attribute__((weak)) crealf(float __complex__ z) {
  return __devicelib_crealf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __attribute__((weak)) cimag(double __complex__ z) {
  return __devicelib_cimag(z);
}

SYCL_EXTERNAL
double __attribute__((weak)) creal(double __complex__ z) {
  return __devicelib_creal(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __attribute__((weak)) cargf(float __complex__ z) {
  return __devicelib_cargf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __attribute__((weak)) cabs(double __complex__ z) {
  return __devicelib_cabs(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __attribute__((weak)) cabsf(float __complex__ z) {
  return __devicelib_cabsf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __attribute__((weak)) carg(double __complex__ z) {
  return __devicelib_carg(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) cprojf(float __complex__ z) {
  return __devicelib_cprojf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __complex__ __attribute__((weak)) cproj(double __complex__ z) {
  return __devicelib_cproj(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) cexpf(float __complex__ z) {
  return __devicelib_cexpf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __complex__ __attribute__((weak)) cexp(double __complex__ z) {
  return __devicelib_cexp(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) clogf(float __complex__ z) {
  return __devicelib_clogf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __complex__ __attribute__((weak)) clog(double __complex__ z) {
  return __devicelib_clog(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) cpowf(float __complex__ x,
                                              float __complex__ y) {
  return __devicelib_cpowf(x, y);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __complex__ __attribute__((weak)) cpow(double __complex__ x,
                                              double __complex__ y) {
  return __devicelib_cpow(x, y);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) cpolarf(float rho, float theta) {
  return __devicelib_cpolarf(rho, theta);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __complex__ __attribute__((weak)) cpolar(double rho, double theta) {
  return __devicelib_cpolar(rho, theta);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) csqrtf(float __complex__ z) {
  return __devicelib_csqrtf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __complex__ __attribute__((weak)) csqrt(double __complex__ z) {
  return __devicelib_csqrt(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) csinhf(float __complex__ z) {
  return __devicelib_csinhf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __complex__ __attribute__((weak)) csinh(double __complex__ z) {
  return __devicelib_csinh(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) ccoshf(float __complex__ z) {
  return __devicelib_ccoshf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __complex__ __attribute__((weak)) ccosh(double __complex__ z) {
  return __devicelib_ccosh(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) ctanhf(float __complex__ z) {
  return __devicelib_ctanhf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __complex__ __attribute__((weak)) ctanh(double __complex__ z) {
  return __devicelib_ctanh(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) csinf(float __complex__ z) {
  return __devicelib_csinf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __complex__ __attribute__((weak)) csin(double __complex__ z) {
  return __devicelib_csin(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) ccosf(float __complex__ z) {
  return __devicelib_ccosf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __complex__ __attribute__((weak)) ccos(double __complex__ z) {
  return __devicelib_ccos(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) ctanf(float __complex__ z) {
  return __devicelib_ctanf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __complex__ __attribute__((weak)) ctan(double __complex__ z) {
  return __devicelib_ctan(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) cacosf(float __complex__ z) {
  return __devicelib_cacosf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __complex__ __attribute__((weak)) cacos(double __complex__ z) {
  return __devicelib_cacos(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) casinhf(float __complex__ z) {
  return __devicelib_casinhf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __complex__ __attribute__((weak)) casinh(double __complex__ z) {
  return __devicelib_casinh(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) casinf(float __complex__ z) {
  return __devicelib_casinf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __complex__ __attribute__((weak)) casin(double __complex__ z) {
  return __devicelib_casin(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) cacoshf(float __complex__ z) {
  return __devicelib_cacoshf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __complex__ __attribute__((weak)) cacosh(double __complex__ z) {
  return __devicelib_cacosh(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) catanhf(float __complex__ z) {
  return __devicelib_catanhf(z);
}

SYCL_EXTERNAL
<<<<<<< HEAD
double __complex__ __attribute__((weak)) catanh(double __complex__ z) {
  return __devicelib_catanh(z);
}

SYCL_EXTERNAL
=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
float __complex__ __attribute__((weak)) catanf(float __complex__ z) {
  return __devicelib_catanf(z);
}

<<<<<<< HEAD
SYCL_EXTERNAL
double __complex__ __attribute__((weak)) catan(double __complex__ z) {
  return __devicelib_catan(z);
}


=======
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
// __mulsc3
// Returns: the product of a + ib and c + id
SYCL_EXTERNAL
float __complex__ __attribute__((weak)) __mulsc3(float __a, float __b,
                                                 float __c, float __d) {
<<<<<<< HEAD
  return __devicelib_mulsc3(__a, __b, __c, __d);
}

// __muldc3
// Returns: the product of a + ib and c + id
SYCL_EXTERNAL
double __complex__ __attribute__((weak)) __muldc3(double __a, double __b,
                                                  double __c, double __d) {
  return __devicelib_muldc3(__a, __b, __c, __d);
=======
  return __devicelib___mulsc3(__a, __b, __c, __d);
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
}

// __divsc3
// Returns: the quotient of (a + ib) / (c + id)
SYCL_EXTERNAL
float __complex__ __attribute__((weak)) __divsc3(float __a, float __b,
                                                 float __c, float __d) {
<<<<<<< HEAD
  return __devicelib_divsc3(__a, __b, __c, __d);
}

// __divdc3
// Returns: the quotient of (a + ib) / (c + id)
SYCL_EXTERNAL
double __complex__ __attribute__((weak)) __divdc3(double __a, double __b,
                                                  double __c, double __d) {
  return __devicelib_divdc3(__a, __b, __c, __d);
}

=======
  return __devicelib___divsc3(__a, __b, __c, __d);
}
>>>>>>> 7abd9d503645ff252ed5ccacfd0cf0b8f86a0abf
}
#endif
