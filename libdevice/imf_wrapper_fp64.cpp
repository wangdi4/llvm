//==----- imf_wrapper_fp64.cpp - wrappers for double precision intel math
// library functions ------==//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "device_imf.hpp"

#ifdef __LIBDEVICE_IMF_ENABLED__

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_double2float_rd(double);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_double2float_rn(double);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_double2float_ru(double);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_double2float_rz(double);

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_double2hiint(double);

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_double2int_rd(double);

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_double2int_rn(double);

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_double2int_ru(double);

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_double2int_rz(double);

DEVICE_EXTERN_C_INLINE
unsigned int __devicelib_imf_double2uint_rd(double);

DEVICE_EXTERN_C_INLINE
unsigned int __devicelib_imf_double2uint_rn(double);

DEVICE_EXTERN_C_INLINE
unsigned int __devicelib_imf_double2uint_ru(double);

DEVICE_EXTERN_C_INLINE
unsigned int __devicelib_imf_double2uint_rz(double);

DEVICE_EXTERN_C_INLINE
long long int __devicelib_imf_double2ll_rd(double);

DEVICE_EXTERN_C_INLINE
long long int __devicelib_imf_double2ll_rn(double);

DEVICE_EXTERN_C_INLINE
long long int __devicelib_imf_double2ll_ru(double);

DEVICE_EXTERN_C_INLINE
long long int __devicelib_imf_double2ll_rz(double);

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_double2loint(double);

DEVICE_EXTERN_C_INLINE
unsigned long long int __devicelib_imf_double2ull_rd(double);

DEVICE_EXTERN_C_INLINE
unsigned long long int __devicelib_imf_double2ull_rn(double);

DEVICE_EXTERN_C_INLINE
unsigned long long int __devicelib_imf_double2ull_ru(double);

DEVICE_EXTERN_C_INLINE
unsigned long long int __devicelib_imf_double2ull_rz(double);

DEVICE_EXTERN_C_INLINE
long long int __devicelib_imf_double_as_longlong(double);

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_hiloint2double(int, int);

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_int2double_rn(int);

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_ll2double_rd(long long int);

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_ll2double_rn(long long int);

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_ll2double_ru(long long int);

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_ll2double_rz(long long int);

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_longlong_as_double(long long int);

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_uint2double_rn(unsigned int);

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_ull2double_rd(unsigned long long int);

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_ull2double_rn(unsigned long long int);

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_ull2double_ru(unsigned long long int);

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_ull2double_rz(unsigned long long int);

DEVICE_EXTERN_C_INLINE
float __imf_double2float_rd(double x) {
  return __devicelib_imf_double2float_rd(x);
}

DEVICE_EXTERN_C_INLINE
float __imf_double2float_rn(double x) {
  return __devicelib_imf_double2float_rn(x);
}

DEVICE_EXTERN_C_INLINE
float __imf_double2float_ru(double x) {
  return __devicelib_imf_double2float_ru(x);
}

DEVICE_EXTERN_C_INLINE
float __imf_double2float_rz(double x) {
  return __devicelib_imf_double2float_rz(x);
}

DEVICE_EXTERN_C_INLINE
int __imf_double2int_rd(double x) { return __devicelib_imf_double2int_rd(x); }

DEVICE_EXTERN_C_INLINE
int __imf_double2int_rn(double x) { return __devicelib_imf_double2int_rn(x); }

DEVICE_EXTERN_C_INLINE
int __imf_double2int_ru(double x) { return __devicelib_imf_double2int_ru(x); }

DEVICE_EXTERN_C_INLINE
int __imf_double2int_rz(double x) { return __devicelib_imf_double2int_rz(x); }

// TODO: For __imf_double2hiint and __imf_double2loint, we assume underlying
// device is little-endian. We need to check if it is necessary to provide an
// endian independent implementation.
DEVICE_EXTERN_C_INLINE
int __imf_double2hiint(double x) { return __devicelib_imf_double2hiint(x); }

DEVICE_EXTERN_C_INLINE
int __imf_double2loint(double x) { return __devicelib_imf_double2loint(x); }

DEVICE_EXTERN_C_INLINE
unsigned int __imf_double2uint_rd(double x) {
  return __devicelib_imf_double2uint_rd(x);
}

DEVICE_EXTERN_C_INLINE
unsigned int __imf_double2uint_rn(double x) {
  return __devicelib_imf_double2uint_rn(x);
}

DEVICE_EXTERN_C_INLINE
unsigned int __imf_double2uint_ru(double x) {
  return __devicelib_imf_double2uint_ru(x);
}

DEVICE_EXTERN_C_INLINE
unsigned int __imf_double2uint_rz(double x) {
  return __devicelib_imf_double2uint_rz(x);
}

DEVICE_EXTERN_C_INLINE
long long int __imf_double2ll_rd(double x) {
  return __devicelib_imf_double2ll_rd(x);
}

DEVICE_EXTERN_C_INLINE
long long int __imf_double2ll_rn(double x) {
  return __devicelib_imf_double2ll_rn(x);
}

DEVICE_EXTERN_C_INLINE
long long int __imf_double2ll_ru(double x) {
  return __devicelib_imf_double2ll_ru(x);
}

DEVICE_EXTERN_C_INLINE
long long int __imf_double2ll_rz(double x) {
  return __devicelib_imf_double2ll_rz(x);
}

DEVICE_EXTERN_C_INLINE
unsigned long long int __imf_double2ull_rd(double x) {
  return __devicelib_imf_double2ull_rd(x);
}

DEVICE_EXTERN_C_INLINE
unsigned long long int __imf_double2ull_rn(double x) {
  return __devicelib_imf_double2ull_rn(x);
}

DEVICE_EXTERN_C_INLINE
unsigned long long int __imf_double2ull_ru(double x) {
  return __devicelib_imf_double2ull_ru(x);
}

DEVICE_EXTERN_C_INLINE
unsigned long long int __imf_double2ull_rz(double x) {
  return __devicelib_imf_double2ull_rz(x);
}

DEVICE_EXTERN_C_INLINE
long long int __imf_double_as_longlong(double x) {
  return __devicelib_imf_double_as_longlong(x);
}

DEVICE_EXTERN_C_INLINE
double __imf_hiloint2double(int hi, int lo) {
  return __devicelib_imf_hiloint2double(hi, lo);
}

DEVICE_EXTERN_C_INLINE
double __imf_int2double_rn(int x) { return __devicelib_imf_int2double_rn(x); }

DEVICE_EXTERN_C_INLINE
double __imf_ll2double_rd(long long int x) {
  return __devicelib_imf_ll2double_rd(x);
}

DEVICE_EXTERN_C_INLINE
double __imf_ll2double_rn(long long int x) {
  return __devicelib_imf_ll2double_rn(x);
}

DEVICE_EXTERN_C_INLINE
double __imf_ll2double_ru(long long int x) {
  return __devicelib_imf_ll2double_ru(x);
}

DEVICE_EXTERN_C_INLINE
double __imf_ll2double_rz(long long int x) {
  return __devicelib_imf_ll2double_rz(x);
}

DEVICE_EXTERN_C_INLINE
double __imf_longlong_as_double(long long int x) {
  return __devicelib_imf_longlong_as_double(x);
}

DEVICE_EXTERN_C_INLINE
double __imf_uint2double_rn(unsigned int x) {
  return __devicelib_imf_uint2double_rn(x);
}

DEVICE_EXTERN_C_INLINE
double __imf_ull2double_rd(unsigned long long int x) {
  return __devicelib_imf_ull2double_rd(x);
}

DEVICE_EXTERN_C_INLINE
double __imf_ull2double_rn(unsigned long long int x) {
  return __devicelib_imf_ull2double_rn(x);
}

DEVICE_EXTERN_C_INLINE
double __imf_ull2double_ru(unsigned long long int x) {
  return __devicelib_imf_ull2double_ru(x);
}

DEVICE_EXTERN_C_INLINE
double __imf_ull2double_rz(unsigned long long int x) {
  return __devicelib_imf_ull2double_rz(x);
}

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_fma(double, double, double);

DEVICE_EXTERN_C_INLINE
double __imf_fma(double x, double y, double z) {
  return __devicelib_imf_fma(x, y, z);
}

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_floor(double);

DEVICE_EXTERN_C_INLINE
double __imf_floor(double x) { return __devicelib_imf_floor(x); }

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_ceil(double);

DEVICE_EXTERN_C_INLINE
double __imf_ceil(double x) { return __devicelib_imf_ceil(x); }

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_trunc(double);

DEVICE_EXTERN_C_INLINE
double __imf_trunc(double x) { return __devicelib_imf_trunc(x); }

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_rint(double);

DEVICE_EXTERN_C_INLINE
double __imf_rint(double x) { return __devicelib_imf_rint(x); }

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_nearbyint(double);

DEVICE_EXTERN_C_INLINE
double __imf_nearbyint(double x) { return __devicelib_imf_nearbyint(x); }

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_sqrt(double);

DEVICE_EXTERN_C_INLINE
double __imf_sqrt(double x) { return __devicelib_imf_sqrt(x); }

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_rsqrt(double);

DEVICE_EXTERN_C_INLINE
double __imf_rsqrt(double x) { return __devicelib_imf_rsqrt(x); }

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_inv(double);

DEVICE_EXTERN_C_INLINE
double __imf_inv(double x) { return __devicelib_imf_inv(x); }

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_fabs(double);

DEVICE_EXTERN_C_INLINE
double __imf_fabs(double x) { return __devicelib_imf_fabs(x); }

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_fmax(double, double);

DEVICE_EXTERN_C_INLINE
double __imf_fmax(double x, double y) { return __devicelib_imf_fmax(x, y); }

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_fmin(double, double);

DEVICE_EXTERN_C_INLINE
double __imf_fmin(double x, double y) { return __devicelib_imf_fmin(x, y); }

DEVICE_EXTERN_C_INLINE
double __devicelib_imf_copysign(double, double);

DEVICE_EXTERN_C_INLINE
double __imf_copysign(double x, double y) {
  return __devicelib_imf_copysign(x, y);
}

#ifdef INTEL_CUSTOMIZATION
// float64 devicelib API's
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_erfinv(double);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_acos (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_acosh (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_asin (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_asinh (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_atan2 (double x, double y);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_atan (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_atanh (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_cbrt (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_cdfnorm (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_cdfnorminv (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_cos (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_cosh (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_cospi (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_erfc (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_erfcinv (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_erfcx (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_erf (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_exp10 (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_exp2 (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_exp (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_expm1 (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_fdim (double x, double y);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_fmod (double x, double y);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_frexp (double x, int* z);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_hypot (double x, double y);
DEVICE_EXTERN_C_INLINE
int __devicelib_imf_ilogb (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_rcbrt (double x);
DEVICE_EXTERN_C_INLINE
int __devicelib_imf_isfinite (double x);
DEVICE_EXTERN_C_INLINE
int __devicelib_imf_isinf (double x);
DEVICE_EXTERN_C_INLINE
int __devicelib_imf_isnan (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_ldexp (double x, int y);
DEVICE_EXTERN_C_INLINE
long long int __devicelib_imf_llrint (double x);
DEVICE_EXTERN_C_INLINE
long long int __devicelib_imf_llround (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_log (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_log10 (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_log1p (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_log2 (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_logb (double x);
DEVICE_EXTERN_C_INLINE
int64_t __devicelib_imf_lrint (double x);
DEVICE_EXTERN_C_INLINE
int64_t __devicelib_imf_lround (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_modf (double x, double* z);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_nan (const char* x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_nextafter (double x, double y);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_norm3d (double x, double y, double z);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_norm4d (double x, double y, double z, double t);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_pow (double x, double y);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_remainder (double x, double y);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_remquo (double x, double y, int* z);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_rhypot (double x, double y);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_rnorm3d (double x, double y, double z);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_rnorm4d (double x, double y, double z, double t);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_round (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_scalbn (double x, int y);
DEVICE_EXTERN_C_INLINE
int __devicelib_imf_signbit (double x);
DEVICE_EXTERN_C_INLINE
void __devicelib_imf_sincos (double x, double* y, double* z);
DEVICE_EXTERN_C_INLINE
void __devicelib_imf_sincospi (double x, double* y, double* z);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_sin (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_sinh (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_sinpi (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_tan (double x);
DEVICE_EXTERN_C_INLINE
double __devicelib_imf_tanh (double x);

// float64 imf wrappers
DEVICE_EXTERN_C_INLINE
double __imf_erfinv(double x) { return __devicelib_imf_erfinv(x); }
DEVICE_EXTERN_C_INLINE
double __imf_acos (double x) { return __devicelib_imf_acos (x); }
DEVICE_EXTERN_C_INLINE
double __imf_acosh (double x) { return __devicelib_imf_acosh (x); }
DEVICE_EXTERN_C_INLINE
double __imf_asin (double x) { return __devicelib_imf_asin (x); }
DEVICE_EXTERN_C_INLINE
double __imf_asinh (double x) { return __devicelib_imf_asinh (x); }
DEVICE_EXTERN_C_INLINE
double __imf_atan2 (double x, double y) { return __devicelib_imf_atan2 (x, y); }
DEVICE_EXTERN_C_INLINE
double __imf_atan (double x) { return __devicelib_imf_atan (x); }
DEVICE_EXTERN_C_INLINE
double __imf_atanh (double x) { return __devicelib_imf_atanh (x); }
DEVICE_EXTERN_C_INLINE
double __imf_cbrt (double x) { return __devicelib_imf_cbrt (x); }
DEVICE_EXTERN_C_INLINE
double __imf_cdfnorm (double x) { return __devicelib_imf_cdfnorm (x); }
DEVICE_EXTERN_C_INLINE
double __imf_cdfnorminv (double x) { return __devicelib_imf_cdfnorminv (x); }
DEVICE_EXTERN_C_INLINE
double __imf_cos (double x) { return __devicelib_imf_cos (x); }
DEVICE_EXTERN_C_INLINE
double __imf_cosh (double x) { return __devicelib_imf_cosh (x); }
DEVICE_EXTERN_C_INLINE
double __imf_cospi (double x) { return __devicelib_imf_cospi (x); }
DEVICE_EXTERN_C_INLINE
double __imf_erfc (double x) { return __devicelib_imf_erfc (x); }
DEVICE_EXTERN_C_INLINE
double __imf_erfcinv (double x) { return __devicelib_imf_erfcinv (x); }
DEVICE_EXTERN_C_INLINE
double __imf_erfcx (double x) { return __devicelib_imf_erfcx (x); }
DEVICE_EXTERN_C_INLINE
double __imf_erf (double x) { return __devicelib_imf_erf (x); }
DEVICE_EXTERN_C_INLINE
double __imf_exp10 (double x) { return __devicelib_imf_exp10 (x); }
DEVICE_EXTERN_C_INLINE
double __imf_exp2 (double x) { return __devicelib_imf_exp2 (x); }
DEVICE_EXTERN_C_INLINE
double __imf_exp (double x) { return __devicelib_imf_exp (x); }
DEVICE_EXTERN_C_INLINE
double __imf_expm1 (double x) { return __devicelib_imf_expm1 (x); }
DEVICE_EXTERN_C_INLINE
double __imf_fdim (double x, double y) { return __devicelib_imf_fdim (x, y); }
DEVICE_EXTERN_C_INLINE
double __imf_fmod (double x, double y) { return __devicelib_imf_fmod (x, y); }
DEVICE_EXTERN_C_INLINE
double __imf_frexp (double x, int* z) { return __devicelib_imf_frexp (x, z); }
DEVICE_EXTERN_C_INLINE
double __imf_hypot (double x, double y) { return __devicelib_imf_hypot (x, y); }
DEVICE_EXTERN_C_INLINE
int __imf_ilogb (double x) { return __devicelib_imf_ilogb (x); }
DEVICE_EXTERN_C_INLINE
double __imf_rcbrt (double x) { return __devicelib_imf_rcbrt (x); }
DEVICE_EXTERN_C_INLINE
int __imf_isfinite (double x) { return __devicelib_imf_isfinite (x); }
DEVICE_EXTERN_C_INLINE
int __imf_isinf (double x) { return __devicelib_imf_isinf (x); }
DEVICE_EXTERN_C_INLINE
int __imf_isnan (double x) { return __devicelib_imf_isnan (x); }
DEVICE_EXTERN_C_INLINE
double __imf_ldexp (double x, int y) { return __devicelib_imf_ldexp (x, y); }
DEVICE_EXTERN_C_INLINE
long long int __imf_llrint (double x) { return __devicelib_imf_llrint (x); }
DEVICE_EXTERN_C_INLINE
long long int __imf_llround (double x) { return __devicelib_imf_llround (x); }
DEVICE_EXTERN_C_INLINE
double __imf_log (double x) { return __devicelib_imf_log (x); }
DEVICE_EXTERN_C_INLINE
double __imf_log10 (double x) { return __devicelib_imf_log10 (x); }
DEVICE_EXTERN_C_INLINE
double __imf_log1p (double x) { return __devicelib_imf_log1p (x); }
DEVICE_EXTERN_C_INLINE
double __imf_log2 (double x) { return __devicelib_imf_log2 (x); }
DEVICE_EXTERN_C_INLINE
double __imf_logb (double x) { return __devicelib_imf_logb (x); }
DEVICE_EXTERN_C_INLINE
int64_t __imf_lrint (double x) { return __devicelib_imf_lrint (x); }
DEVICE_EXTERN_C_INLINE
int64_t __imf_lround (double x) { return __devicelib_imf_lround (x); }
DEVICE_EXTERN_C_INLINE
double __imf_modf (double x, double* z) { return __devicelib_imf_modf (x, z); }
DEVICE_EXTERN_C_INLINE
double __imf_nan (const char* x) { return __devicelib_imf_nan (x); }
DEVICE_EXTERN_C_INLINE
double __imf_nextafter (double x, double y) { return __devicelib_imf_nextafter (x, y); }
DEVICE_EXTERN_C_INLINE
double __imf_norm3d (double x, double y, double z) { return __devicelib_imf_norm3d (x, y, z); }
DEVICE_EXTERN_C_INLINE
double __imf_norm4d (double x, double y, double z, double t) { return __devicelib_imf_norm4d (x, y, z, t); }
DEVICE_EXTERN_C_INLINE
double __imf_pow (double x, double y) { return __devicelib_imf_pow (x, y); }
DEVICE_EXTERN_C_INLINE
double __imf_remainder (double x, double y) { return __devicelib_imf_remainder (x, y); }
DEVICE_EXTERN_C_INLINE
double __imf_remquo (double x, double y, int* z) { return __devicelib_imf_remquo (x, y, z); }
DEVICE_EXTERN_C_INLINE
double __imf_rhypot (double x, double y) { return __devicelib_imf_rhypot (x, y); }
DEVICE_EXTERN_C_INLINE
double __imf_rnorm3d (double x, double y, double z) { return __devicelib_imf_rnorm3d (x, y, z); }
DEVICE_EXTERN_C_INLINE
double __imf_rnorm4d (double x, double y, double z, double t) { return __devicelib_imf_rnorm4d (x, y, z, t); }
DEVICE_EXTERN_C_INLINE
double __imf_round (double x) { return __devicelib_imf_round (x); }
DEVICE_EXTERN_C_INLINE
double __imf_scalbn (double x, int y) { return __devicelib_imf_scalbn (x, y); }
DEVICE_EXTERN_C_INLINE
int __imf_signbit (double x) { return __devicelib_imf_signbit (x); }
DEVICE_EXTERN_C_INLINE
void __imf_sincos (double x, double* y, double* z) { __devicelib_imf_sincos (x, y, z); return; }
DEVICE_EXTERN_C_INLINE
void __imf_sincospi (double x, double* y, double* z) { __devicelib_imf_sincospi (x, y, z); return; }
DEVICE_EXTERN_C_INLINE
double __imf_sin (double x) { return __devicelib_imf_sin (x); }
DEVICE_EXTERN_C_INLINE
double __imf_sinh (double x) { return __devicelib_imf_sinh (x); }
DEVICE_EXTERN_C_INLINE
double __imf_sinpi (double x) { return __devicelib_imf_sinpi (x); }
DEVICE_EXTERN_C_INLINE
double __imf_tan (double x) { return __devicelib_imf_tan (x); }
DEVICE_EXTERN_C_INLINE
double __imf_tanh (double x) { return __devicelib_imf_tanh (x); }
#endif
#endif // __LIBDEVICE_IMF_ENABLED__
