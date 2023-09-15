// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

// imathLibd.cpp : Defines the exported functions for the DLL application.
//

#include "imathLibd.h"
#include <assert.h>
#include <errno.h>

#if defined(_WIN32)
#include <mathimf.h>
#else
#include <math.h>
#endif

// This is the constructor of a class that has been exported.
// see imathLibd.h for the class definition
CimathLibd::CimathLibd() { return; }

long double CimathLibd::imf_rint(long double x) { return rintl(x); }

long double CimathLibd::imf_asin(long double x) { return asinl(x); }

long double CimathLibd::imf_atan(long double x) { return atanl(x); }

long double CimathLibd::imf_tanh(long double x) { return tanhl(x); }

long double CimathLibd::imf_sqrt(long double x) { return sqrtl(x); }

long double CimathLibd::imf_log1p(long double x) { return log1pl(x); }

long double CimathLibd::imf_pow(long double x, long double y) {
  // store old errno
  int errno_prev = errno;
  // set errno to NULL
  errno = -1;
  long double res = powl(x, y);
  // arguments were out of domain
  // Indefinite result will be returned in extended precision ( all zeros exp +
  // 62,63 bits are set) (simply NaN in single or double precision) function
  // calculated in double precision will return +-inf or nan lets compute in
  // double precision
  if (errno == EDOM) {
    res = (long double)pow((double)x, (double)y);
    assert((isnan(res) || isinf(res) || res == (long double)0.0) &&
           "imf_pow: Expected corner-case resulting value");
  }
  // restore old errno
  errno = errno_prev;
  return res;
}

long double CimathLibd::imf_nextafter(long double x, long double y) {
  return nextafterl(x, y);
}

int CimathLibd::imf_ilogb(long double x) { return ilogbl(x); }

long double CimathLibd::imf_exp(long double x) { return expl(x); }

long double CimathLibd::imf_exp2(long double x) { return exp2l(x); }

long double CimathLibd::imf_expm1(long double x) { return expm1l(x); }

long double CimathLibd::imf_ldexp(long double x, int n) { return ldexpl(x, n); }

long double CimathLibd::imf_frexp(long double x, int *n) {
  return frexpl(x, n);
}

long double CimathLibd::imf_fabs(long double x) { return fabsl(x); }

long double CimathLibd::imf_floor(long double x) { return floorl(x); }

long double CimathLibd::imf_log2(long double x) { return log2l(x); }

long double CimathLibd::imf_sin(long double x) { return sinl(x); }

long double CimathLibd::imf_cos(long double x) { return cosl(x); }

long double CimathLibd::imf_tan(long double x) { return tanl(x); }

long double CimathLibd::imf_cosh(long double x) { return coshl(x); }
