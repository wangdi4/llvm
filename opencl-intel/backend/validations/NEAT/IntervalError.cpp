// INTEL CONFIDENTIAL
//
// Copyright 2013 Intel Corporation.
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

#include "IntervalError.h"
#include "RefALU.h"

namespace Validation {
double ComputeUlp(double ref) {
  double oneUlp = 0.;

  if (Utils::lt<double>(::fabs(ref), ::ldexp(1.0, -126))) // ref < 2**(-126)
  {
    oneUlp = ::ldexp(1.0, -149); // oneUlp = 2**(-126-23)
  } else {
    double c = ::floor(::log10(::fabs(ref)) / ::log10(2.0)) - 23.0;
    oneUlp = ::ldexp(1.0, int(c));
  }
  return oneUlp;
}

long double ComputeUlp(long double x) {

  if (sizeof(double) >= sizeof(long double)) {
    printf("This architecture needs something higher precision than double\
                    to check the precision of double.\nTest FAILED.\n");
    abort();
  }

  long double oneUlp = 0.0L;
  long double a = CimathLibd::imf_fabs(x);

  if (Utils::lt<long double>(
          a, CimathLibd::imf_ldexp(1.0L, -1022))) // x < 2**(-1022)
  {
    oneUlp = CimathLibd::imf_ldexp(1.0L, -1074); // oneUlp = 2**(-1022-52)
  } else if (Utils::IsNaN<long double>(a) || Utils::IsInf<long double>(a)) {
    // ulps for NaNs and INFs are unknown
    oneUlp = 0.0f;
  } else {
    // oneUlp = 2**(ilogb(abs(x))-52)
    int c = CimathLibd::imf_ilogb(a) - (DBL_MANT_DIG - 1);
    // oneUlp = pow(2, c);
    oneUlp = CimathLibd::imf_ldexp(1.0L, c);
  }

  return oneUlp;
}

template <>
IntervalError<float> IntervalError<float>::divFrmSetUlps(const NEATValue &a,
                                                         const NEATValue &b) {
  IntervalError<float> ulps = IntervalError<float>::fromUlpsError(2.5f, true);
  NEATValue interval1(HEX_FLT(+, 2, 0, -, 126), HEX_FLT(+, 2, 0, +, 126)),
      interval2(HEX_FLT(+, 2, 0, -, 62), HEX_FLT(+, 2, 0, +, 62));
  bool isGuaranteedUlp = false;

  // in case of special neat values don't change the ulps
  isGuaranteedUlp |= a.IsAny() || a.IsUnknown() || a.IsUnwritten();
  // check condition "is x in the domain of 2**-126 to 2**126"
  isGuaranteedUlp |=
      (b.IsInterval() && a.IsAcc() && (*a.GetAcc<float>() == float(1.0f)) &&
       interval1.Includes(*b.GetMin<float>()) &&
       interval1.Includes(*b.GetMax<float>()));
  // check condition "is x in the domain of 2**-62 to 2**62 and y in the domain
  // of 2**-62 to 2**62."
  isGuaranteedUlp |= (a.IsInterval() && b.IsInterval() &&
                      interval2.Includes(*a.GetMin<float>()) &&
                      interval2.Includes(*a.GetMax<float>()) &&
                      interval2.Includes(*b.GetMin<float>()) &&
                      interval2.Includes(*b.GetMax<float>()));

  if (!isGuaranteedUlp) {
    ulps = IntervalError<float>::fromUlpsError(+INFINITY, true);
  }

  return ulps;
}

template <>
IntervalError<float> IntervalError<float>::cosFrmSetUlps(const NEATValue &a) {
  double AbsErr = HEX_DBL(+, 2, 0, -, 13);

  NEATValue interval((float)-M_PI, (float)+M_PI);
  bool isGuaranteedUlp = false;

  // in case of special neat values don't change the ulps
  isGuaranteedUlp |= a.IsAny() || a.IsUnknown() || a.IsUnwritten();
  // check for x in the domain [-pi, pi]
  isGuaranteedUlp |=
      ((a.IsInterval() || a.IsAcc()) && interval.Includes(*a.GetMin<float>()) &&
       interval.Includes(*a.GetMax<float>()));
  if (!isGuaranteedUlp)
    AbsErr = +INFINITY;

  return IntervalError<float>::fromAbsoluteError(AbsErr, true);
}

template <>
IntervalError<float> IntervalError<float>::expFrmSetUlps(const NEATValue &a) {
  IntervalError<float> ret;
  // compute ulps for exp = 3 + floor(fabs(2 * x)) ulp
  if (a.IsAcc() || a.IsInterval()) {
    ret = IntervalError<float>::fromUlpsIntervalError(
        3.0f + RefALU::floor(RefALU::fabs(2 * (*a.GetMin<float>()))),
        3.0f + RefALU::floor(RefALU::fabs(2 * (*a.GetMax<float>()))), true);
  } else {
    ret = IntervalError<float>::fromUlpsError(0.0f, true);
  }

  return ret;
}

template <>
IntervalError<float> IntervalError<float>::logFrmSetUlps(const NEATValue &a) {
  NEATValue interval(0.5f, 2.0f);
  bool isAbsErr = false;

  // for x in the domain [0.5, 2] the maximum absolute error is <= 2**-21;
  // otherwise the maximum error is<=3 ulp for the full profile
  isAbsErr |=
      ((a.IsInterval() || a.IsAcc()) && interval.Includes(*a.GetMin<float>()) &&
       interval.Includes(*a.GetMax<float>()));

  if (isAbsErr)
    return IntervalError<float>::fromAbsoluteError(HEX_DBL(+, 2, 0, -, 21),
                                                   true);
  return IntervalError<float>::fromUlpsError(3.0f, true);
}
} // namespace Validation
