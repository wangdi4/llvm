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

// The following ifdef block is the standard way of creating macros which make
// exporting from a DLL simpler. All files within this DLL are compiled with the
// IMATHLIBD_EXPORTS symbol defined on the command line. This symbol should not
// be defined on any project that uses this DLL. This way any other project
// whose source files include this file see IMATHLIBD_API functions as being
// imported from a DLL, whereas this DLL sees symbols defined with this macro as
// being exported.

#if defined(_WIN32)
#define IMATHLIBD_API __declspec(dllexport)
// #define IMATHLIBD_API __declspec(dllimport)
#else
#define IMATHLIBD_API
#endif

// This class is exported from the imathLibd.dll
class IMATHLIBD_API CimathLibd {
public:
  CimathLibd(void);

  long double static imf_rint(long double x);
  long double static imf_atan(long double x);
  long double static imf_asin(long double x);
  long double static imf_tanh(long double x);
  long double static imf_sqrt(long double x);
  long double static imf_log1p(long double x);
  long double static imf_pow(long double x, long double y);
  long double static imf_nextafter(long double x, long double y);
  int static imf_ilogb(long double x);
  long double static imf_exp(long double x);
  long double static imf_exp2(long double x);
  long double static imf_expm1(long double x);
  long double static imf_ldexp(long double x, int n);
  long double static imf_frexp(long double x, int *n);
  long double static imf_fabs(long double x);
  long double static imf_floor(long double x);
  long double static imf_log2(long double x);
  long double static imf_sin(long double x);
  long double static imf_cos(long double x);
  long double static imf_tan(long double x);
  long double static imf_cosh(long double x);
};
