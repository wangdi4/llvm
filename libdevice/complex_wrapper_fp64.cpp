//==--- complex_wrapper_fp64.cpp - wrappers for double precision C99 complex
// math functions ------==//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
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
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "device_complex.h"

#ifdef __SPIR__
#if INTEL_COLLAB
#if OMP_LIBDEVICE
#pragma omp declare target
#endif // OMP_LIBDEVICE
#endif // INTEL_COLLAB

DEVICE_EXTERN_C_INLINE
double cimag(double __complex__ z) { return __devicelib_cimag(z); }

DEVICE_EXTERN_C_INLINE
double creal(double __complex__ z) { return __devicelib_creal(z); }

DEVICE_EXTERN_C_INLINE
double cabs(double __complex__ z) { return __devicelib_cabs(z); }

DEVICE_EXTERN_C_INLINE
double carg(double __complex__ z) { return __devicelib_carg(z); }

DEVICE_EXTERN_C_INLINE
double __complex__ cproj(double __complex__ z) { return __devicelib_cproj(z); }

DEVICE_EXTERN_C_INLINE
double __complex__ cexp(double __complex__ z) { return __devicelib_cexp(z); }

DEVICE_EXTERN_C_INLINE
double __complex__ clog(double __complex__ z) { return __devicelib_clog(z); }

DEVICE_EXTERN_C_INLINE
double __complex__ cpow(double __complex__ x, double __complex__ y) {
  return __devicelib_cpow(x, y);
}

DEVICE_EXTERN_C_INLINE
double __complex__ cpolar(double rho, double theta) {
  return __devicelib_cpolar(rho, theta);
}

DEVICE_EXTERN_C_INLINE
double __complex__ csqrt(double __complex__ z) { return __devicelib_csqrt(z); }

DEVICE_EXTERN_C_INLINE
double __complex__ csinh(double __complex__ z) { return __devicelib_csinh(z); }

DEVICE_EXTERN_C_INLINE
double __complex__ ccosh(double __complex__ z) { return __devicelib_ccosh(z); }

DEVICE_EXTERN_C_INLINE
double __complex__ ctanh(double __complex__ z) { return __devicelib_ctanh(z); }

DEVICE_EXTERN_C_INLINE
double __complex__ csin(double __complex__ z) { return __devicelib_csin(z); }

DEVICE_EXTERN_C_INLINE
double __complex__ ccos(double __complex__ z) { return __devicelib_ccos(z); }

DEVICE_EXTERN_C_INLINE
double __complex__ ctan(double __complex__ z) { return __devicelib_ctan(z); }

DEVICE_EXTERN_C_INLINE
double __complex__ cacos(double __complex__ z) { return __devicelib_cacos(z); }

DEVICE_EXTERN_C_INLINE
double __complex__ casinh(double __complex__ z) {
  return __devicelib_casinh(z);
}

DEVICE_EXTERN_C_INLINE
double __complex__ casin(double __complex__ z) { return __devicelib_casin(z); }

DEVICE_EXTERN_C_INLINE
double __complex__ cacosh(double __complex__ z) {
  return __devicelib_cacosh(z);
}

DEVICE_EXTERN_C_INLINE
double __complex__ catanh(double __complex__ z) {
  return __devicelib_catanh(z);
}

DEVICE_EXTERN_C_INLINE
double __complex__ catan(double __complex__ z) { return __devicelib_catan(z); }

// __muldc3
// Returns: the product of a + ib and c + id
DEVICE_EXTERN_C_INLINE
double __complex__ __muldc3(double __a, double __b, double __c, double __d) {
  return __devicelib___muldc3(__a, __b, __c, __d);
}

// __divdc3
// Returns: the quotient of (a + ib) / (c + id)
DEVICE_EXTERN_C_INLINE
double __complex__ __divdc3(double __a, double __b, double __c, double __d) {
  return __devicelib___divdc3(__a, __b, __c, __d);
}

#ifdef INTEL_CUSTOMIZATION
// cexp10 is not standard C99 complex API, it is only required by omp libdevice.
DEVICE_EXTERN_C_INLINE
double __complex__ cexp10(double __complex__ z) {
  return __devicelib_cexp10(z);
}

DEVICE_EXTERN_C_INLINE
double __complex__ clog10(double __complex__ z) {
  return __devicelib_clog10(z);
}
#endif
#if INTEL_COLLAB
#if OMP_LIBDEVICE
#pragma omp end declare target
#endif // OMP_LIBDEVICE
#endif // INTEL_COLLAB
#endif // __SPIR__
