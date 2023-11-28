// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#ifndef __NEATALUUTILS_H__
#define __NEATALUUTILS_H__

// \brief Useful functions to share between NEATALU tests.

#include "NEATALU.h"
#include "NEATValue.h"
#include <stdio.h>

#define isnan(x) ((x) != (x))

#ifndef LOCAL_MAX
#define LOCAL_MAX(_a, _b) ((_a) > (_b) ? (_a) : (_b))
#endif

#ifndef FLT_MANT_DIG
#define FLT_MANT_DIG 24 /* # of bits in mantissa */
#endif
#ifndef FLT_MIN_EXP
#define FLT_MIN_EXP (-125) /* min binary exponent */
#endif
#ifndef DBL_MANT_DIG
#define DBL_MANT_DIG 53 /* # of bits in mantissa */
#endif
#ifndef DBL_MIN_EXP
#define DBL_MIN_EXP (-1021) /* min binary exponent */
#endif

namespace Validation {

template <typename T> T getMSB();

// class to combine typed tests and value-parameterized tests in googletest
// framework
template <typename T, bool inMode> class ValueTypeContainer {
public:
  typedef T Type;
  static const bool mode = inMode;
};

template <typename T> DataTypeVal GetDataTypeVal();

// the limit of difference between given ulps and calculated ulps
static const float diffLimit = 2.0f;

bool RefIsinf(double _x);
double RefCopysign(double x, double y);

int MyIlogb(double x);
int MyIlogbl(long double x);

template <typename T> T FindMax(T *arr, uint32_t count) {
  T res = arr[0];
  for (uint32_t i = 1; i < count; i++)
    res = std::max(res, arr[i]);
  return res;
}

template <typename T> T FindMin(const T *const arr, uint32_t count) {
  T res = arr[0];
  for (uint32_t i = 1; i < count; i++)
    res = std::min(res, arr[i]);
  return res;
}

template <typename T> static bool TestAccValue(NEATValue testAccVal, T refVal) {
  bool passed = testAccVal.IsAcc();
  T AccVal;
  memcpy(&AccVal, testAccVal.GetAcc<T>(), sizeof(T));
  if (passed) {
    passed = Utils::eq(AccVal, (T)refVal);
  }
  return passed;
}

template <typename T>
static bool TestAccExpanded(T ref, NEATValue test,
                            IntervalError<typename downT<T>::type> error) {
  bool res = true;
  typedef typename downT<T>::type dT;

  if (test.IsAny() && error.isInfiniteError())
    return true;
  if (test.IsUnknown() || test.IsUnwritten() || test.IsAny())
    return false;

  dT min = *test.GetMin<dT>();
  dT max = *test.GetMax<dT>();

  min = RefALU::flush<dT>(min);
  max = RefALU::flush<dT>(max);

  if (error == T(0)) {
    dT refDT = RefALU::flush(NEATALU::castDown(RefALU::flush(ref)));
    if (min == refDT && max == refDT)
      return true;
    else
      return false;
  }

  if (Utils::IsNInf(ref)) {
    if (!test.IsAcc())
      return false;
    if (!Utils::IsNInf(*test.GetAcc<dT>()))
      return false;
  } else if (Utils::IsPInf(ref)) {
    if (!test.IsAcc())
      return false;
    if (!Utils::IsPInf(*test.GetAcc<dT>()))
      return false;
  } else if (Utils::IsNaN(ref)) {
    if (!test.IsNaN<dT>())
      return false;
  } else if (error.getErrorType() == IntervalError<dT>::ERROR_ABSOLUTE) {
    T diff1 = fabs(ref - min);
    T diff2 = fabs(ref - max);
    if (Utils::IsInf<dT>(min))
      res &= Utils::IsInf<dT>(min - error.getAbsoluteError());
    else
      res &= (ComputeUlp(min) < 2 * error.getAbsoluteError())
                 ? (diff1 <= error.getAbsoluteError())
                 : true;

    if (Utils::IsInf<dT>(max))
      res &= Utils::IsInf<dT>(max + error.getAbsoluteError());
    else
      res &= (ComputeUlp(max) < 2 * error.getAbsoluteError())
                 ? (diff2 <= error.getAbsoluteError())
                 : true;
  } else if (ref == 0 && RefALU::GetFTZmode() && error.isAccurateUlps()) {
    // one ulp for zero is denormal in float point precision
    // so, flushed min and max should be zero
    if (min != 0 || max != 0)
      return false;
  } else {
    dT refMin =
        NEATALU::castDown(ref - (T)error.getExpandValue(ComputeUlp(ref), true));
    dT refMax = NEATALU::castDown(
        ref + (T)error.getExpandValue(ComputeUlp(ref), false));

    double diff1 = Utils::ulpsDiff(ref, min);
    double diff2 = Utils::ulpsDiff(ref, max);

    // if refMin is INF, min should be INF
    if (Utils::IsInf<dT>(refMin)) {
      res &= Utils::IsInf<dT>(min);
    } else {
      // if min is denormal, it should be flashed to zero,
      // otherwise sub ulps
      if (refMin == 0 || Utils::IsDenorm<dT>(refMin)) {
        if (RefALU::GetFTZmode())
          res &= (min == 0);
        else {
          float diffDenorm = Utils::ulpsDiffDenormal(ref, min);
          res &= (fabs(diffDenorm) <= error.getUlpsForMin());
          res &= !error.isAccurateUlps() ||
                 (fabs(diffDenorm) >= (error.getUlpsForMin() - diffLimit));
        }
      } else {
        res &= (fabs(diff1) <= error.getUlpsForMin());
        res &= !error.isAccurateUlps() ||
               (fabs(diff1) >= (error.getUlpsForMin() - diffLimit));
      }

      // if refMax is INF, max should be INF
      if (Utils::IsInf<dT>(refMax)) {
        res &= Utils::IsInf<dT>(max);
      } else {
        // if max is denormal, it should be flashed to zero,
        // otherwise add ulps
        if (refMax == 0 || Utils::IsDenorm<dT>(refMax)) {
          if (RefALU::GetFTZmode())
            res &= (max == 0);
          else {
            float diffDenorm = Utils::ulpsDiffDenormal(ref, max);
            res &= (fabs(diffDenorm) <= error.getUlpsForMax());
            res &= !error.isAccurateUlps() ||
                   (fabs(diffDenorm) >= (error.getUlpsForMax() - diffLimit));
          }
        } else {
          res &= (fabs(diff2) <= error.getUlpsForMax());
          res &= !error.isAccurateUlps() ||
                 (fabs(diff2) >= (error.getUlpsForMax() - diffLimit));
        }
      }
    }
  }
  return res;
}
template <typename T>
static bool TestAccExpanded(T ref, NEATValue test, double ulps) {
  IntervalError<typename downT<T>::type> error(ulps);
  return TestAccExpanded<T>(ref, test, error);
}
/// Tests NEAT for accurate NEATValue input.
/// It takes NEAT input and output values, refernce output, relative error and
/// corner cases for this function It ensures that in edge case result is equal
/// to reference and accurate. If input is not an edge case it checks that
/// output interval is correct according to fiven accuracy.
/// @param neatIn   Input value for testing NEAT function
/// @param neatOut  Ouput of testing NEAT function
/// @param ref      Reference output to compare with
/// @param ulpErr   Function accuracy
/// @param cor      Pointer to buffer with corner case inputs
/// @param corSize  Size of buffer with corner cases
template <typename T>
static bool TestNeatAcc(NEATValue neatIn, NEATValue neatOut, T ref,
                        IntervalError<typename downT<T>::type> error, T *cor,
                        uint32_t corSize) {
  typedef typename downT<T>::type dT;
  bool isCorner = false;
  bool passed = true;
  if (!neatIn.IsAcc())
    return false;
  for (uint32_t i = 0; i < corSize; i++) {
    if (Utils::eq((dT)cor[i], *neatIn.GetAcc<dT>()))
      isCorner = true;
  }
  if (isCorner) {
    passed &= neatOut.IsAcc();
    passed &= (*neatOut.GetAcc<dT>() == ref);
  } else {
    passed &= TestAccExpanded<T>(ref, neatOut, error);
  }
  return passed;
}
template <typename T>
static bool TestNeatAcc(NEATValue neatIn, NEATValue neatOut, T ref,
                        double ulpErr, T *cor, uint32_t corSize) {
  IntervalError<typename downT<T>::type> error(ulpErr);
  return TestNeatAcc(neatIn, neatOut, ref, error, cor, corSize);
}
template <typename T>
static bool TestIntExpanded(T refMinIn, T refMaxIn, NEATValue test,
                            IntervalError<typename downT<T>::type> error) {
  typedef typename downT<T>::type dT;

  if (test.IsAny() && error.isInfiniteError())
    return true;

  if (test.IsUnknown() || test.IsUnwritten() || test.IsAny())
    return false;

  bool res = true;
  T refMin, refMax;

  if (refMinIn > refMaxIn) {
    refMin = refMaxIn;
    refMax = refMinIn;
  } else {
    refMin = refMinIn;
    refMax = refMaxIn;
  }

  dT min = *test.GetMin<dT>();
  dT max = *test.GetMax<dT>();

  min = RefALU::flush<dT>(min);
  max = RefALU::flush<dT>(max);

  if (error == T(0)) {
    if (min == RefALU::flush(NEATALU::castDown(RefALU::flush(refMin))) &&
        max == RefALU::flush(NEATALU::castDown(RefALU::flush(refMax))))
      return true;
    else
      return false;
  }

  if (Utils::IsNInf(refMin)) {
    if (!Utils::IsNInf(min))
      return false;
  } else if (Utils::IsPInf(refMin)) {
    if (!Utils::IsPInf(min))
      return false;
  } else if (error.getErrorType() == IntervalError<dT>::ERROR_ABSOLUTE) {
    T diff1 = fabs(refMin - T(min));
    if (Utils::IsInf<dT>(min))
      res &= Utils::IsInf<dT>(min - error.getAbsoluteError());
    else
      res &= (ComputeUlp(min) < 2 * error.getAbsoluteError())
                 ? (diff1 <= error.getAbsoluteError())
                 : true;
  } else if ((refMin == 0) && RefALU::GetFTZmode() && error.isAccurateUlps()) {
    // one ulp for zero is denormal in float point precision
    // so, flushed min and max should be zero
    if (min != 0)
      return false;
  } else {
    dT refMinUlps = NEATALU::castDown(
        refMin - error.getExpandValue(ComputeUlp(refMin), true));

    // if refMinUlps is INF, min should be INF
    if (Utils::IsInf<dT>(refMinUlps)) {
      res &= Utils::IsInf<dT>(min);
    } else if (refMinUlps == 0 || Utils::IsDenorm<dT>(refMinUlps)) {
      if (RefALU::GetFTZmode())
        res &= (min == 0);
      else {
        float diff = Utils::ulpsDiffDenormal(refMin, min);
        res &= (fabs(diff) <= error.getUlpsForMin());
        // in case isFRMFunction is enabled OneUlp of ulps.getUlpsForMin can be
        // higher then diffLimit
        res &= !error.isAccurateUlps() ||
               (fabs(diff) >= (error.getUlpsForMin() - diffLimit));
      }
    } else {
      float diff = Utils::ulpsDiff(refMin, min);
      res &= (fabs(diff) <= error.getUlpsForMin());
      // in case isFRMFunction is enabled OneUlp of ulps.getUlpsForMin can be
      // higher then diffLimit
      res &= !error.isAccurateUlps() ||
             (fabs(diff) >= (error.getUlpsForMin() - diffLimit));
    }
  }

  if (Utils::IsNInf(refMax)) {
    if (!Utils::IsNInf(max))
      return false;
  } else if (Utils::IsPInf(refMax)) {
    if (!Utils::IsPInf(max))
      return false;
  } else if (error.getErrorType() == IntervalError<dT>::ERROR_ABSOLUTE) {
    T diff2 = fabs(refMax - max);
    if (Utils::IsInf<dT>(max))
      res &= Utils::IsInf<dT>(max + error.getAbsoluteError());
    else
      res &= (ComputeUlp(max) < 2 * error.getAbsoluteError())
                 ? (diff2 <= error.getAbsoluteError())
                 : true;
  } else if ((refMax == 0) && RefALU::GetFTZmode() && error.isAccurateUlps()) {
    // one ulp for zero is denormal in float point precision
    // so, flushed max and max should be zero
    if (max != 0)
      return false;
  } else {
    dT refMaxUlps = NEATALU::castDown(
        refMax + error.getExpandValue(ComputeUlp(refMax), false));

    // if refMaxUlps is INF, max should be INF
    if (Utils::IsInf<dT>(refMaxUlps)) {
      res &= Utils::IsInf<dT>(max);
    } else if (refMaxUlps == 0 || Utils::IsDenorm<dT>(refMaxUlps)) {
      if (RefALU::GetFTZmode())
        res &= (max == 0);
      else {
        float diff = Utils::ulpsDiffDenormal(refMax, max);
        res &= (fabs(diff) <= error.getUlpsForMax());
        // in case isFRMFunction is enabled OneUlp of ulps.getUlpsForMin can be
        // higher then diffLimit
        res &= !error.isAccurateUlps() ||
               (fabs(diff) >= (error.getUlpsForMax() - diffLimit));
      }
    } else {
      float diff = Utils::ulpsDiff(refMax, max);
      res &= (fabs(diff) <= error.getUlpsForMax());
      // in case isFRMFunction is enabled OneUlp of ulps.getUlpsForMin can be
      // higher then diffLimit
      res &= !error.isAccurateUlps() ||
             (fabs(diff) >= (error.getUlpsForMax() - diffLimit));
    }
  }
  return res;
}

template <typename T>
static bool TestIntExpanded(T refMinIn, T refMaxIn, NEATValue test,
                            double ulps) {
  IntervalError<typename downT<T>::type> error(ulps);
  return TestIntExpanded(refMinIn, refMaxIn, test, error);
}
// Tests special NEATValue for function with three arguments.
// Result must be NEATValue with UNKNOWN status
template <typename T, NEATValue::Status st>
bool TestSpecialNEATValue(NEATScalarTernaryOp f) {
  bool result = true;
  NEATValue goodValue = NEATValue((T)0.1);
  NEATValue stVal = NEATValue(st);
  NEATValue res = f(stVal, goodValue, goodValue);
  result &= res.IsUnknown();
  res = f(goodValue, stVal, goodValue);
  result &= res.IsUnknown();
  res = f(goodValue, goodValue, stVal);
  result &= res.IsUnknown();
  return result;
}

template <typename T>
static bool TestAccExpandedDotMix(T ref, NEATValue test, T ref4ulps,
                                  float ulps) {
  // ref4ulps is used to calculate 1 ULP for functions mix and dot
  // it is max abs value from input values
  bool res = true;
  typedef typename downT<T>::type dT;

  if (test.IsUnknown() || test.IsUnwritten() || test.IsAny())
    return false;

  dT min = *test.GetMin<dT>();
  dT max = *test.GetMax<dT>();

  min = RefALU::flush<dT>(min);
  max = RefALU::flush<dT>(max);

  if (Utils::IsNInf(ref)) {
    if (!test.IsAcc())
      return false;
    if (!Utils::IsNInf(*test.GetAcc<dT>()))
      return false;
  } else if (Utils::IsPInf(ref)) {
    if (!test.IsAcc())
      return false;
    if (!Utils::IsPInf(*test.GetAcc<dT>()))
      return false;
  } else if ((ref4ulps == 0) && RefALU::GetFTZmode()) {
    // one ulp for zero is denormal in float point precision
    // so, flushed min and max should be zero
    if (min != 0 || max != 0)
      return false;
  } else {
    T oneUlp = ComputeUlp(ref4ulps);

    T refMinHigh = ref - T(ulps) * T(oneUlp);
    T refMaxHigh = ref + T(ulps) * T(oneUlp);
    dT refMin = NEATALU::castDown(refMinHigh);
    dT refMax = NEATALU::castDown(refMaxHigh);

    res = true;
    // if refMin is INF, min should be INF
    if (Utils::IsInf<dT>(refMin)) {
      res &= Utils::IsInf<dT>(min);
    } else {
      // if refMin is denormal and FTZ mode on, result should be zero
      // if refMin is zero and FTZ mode on, result should be zero
      if ((Utils::IsDenorm<dT>(refMin) && RefALU::GetFTZmode()) ||
          (refMin == 0 && RefALU::GetFTZmode()))
        res &= (min == 0);
      // otherwise, refMin should not be greater than result
      else {
        res &= (refMinHigh <= T(min));
      }
    }

    // if refMax is INF, max should be INF
    if (Utils::IsInf<dT>(refMax)) {
      res &= Utils::IsInf<dT>(max);
    } else {
      // if refMax is denormal and FTZ mode on, result should be zero
      // if refMax is zero and FTZ mode on, result should be zero
      if ((Utils::IsDenorm<dT>(refMax) && RefALU::GetFTZmode()) ||
          (refMax == 0 && RefALU::GetFTZmode()))
        res &= (max == 0);
      else
      // otherwise, refMax should not be less than result
      {
        res &= (refMaxHigh >= T(max));
      }
    }
  }
  return res;
}

template <typename T>
static bool TestIntExpandedDotMix(T refMinIn, T refMaxIn, NEATValue test,
                                  T ref4ulps, float ulps) {
  // ref4ulps is used to calculate 1 ULP for functions mix and dot
  // it is max abs value from input values
  typedef typename downT<T>::type dT;

  if (test.IsUnknown() || test.IsUnwritten() || test.IsAny())
    return false;

  bool res = true;
  T refMin, refMax;

  if (refMinIn > refMaxIn) {
    refMin = refMaxIn;
    refMax = refMinIn;
  } else {
    refMin = refMinIn;
    refMax = refMaxIn;
  }

  dT min = *test.GetMin<dT>();
  dT max = *test.GetMax<dT>();

  min = RefALU::flush<dT>(min);
  max = RefALU::flush<dT>(max);

  if (Utils::IsNInf(refMin)) {
    if (!Utils::IsNInf(min))
      return false;
  } else if (Utils::IsPInf(refMin)) {
    if (!Utils::IsPInf(min))
      return false;
  } else if ((ref4ulps == 0) && RefALU::GetFTZmode()) {
    // one ulp for zero is denormal in float point precision
    // so, flushed min and max should be zero
    if (min != 0)
      return false;
  } else {
    T oneUlp = ComputeUlp(ref4ulps);
    T refMinHigh = refMin - T(ulps * oneUlp);
    dT refMinUlps = NEATALU::castDown(refMinHigh);

    res = true;
    // if refMinUlps is INF, min should be INF
    if (Utils::IsInf<dT>(refMinUlps)) {
      res &= Utils::IsInf<dT>(min);
    } else if ((Utils::IsDenorm<dT>(refMinUlps) && RefALU::GetFTZmode()) ||
               (refMinUlps == 0 && RefALU::GetFTZmode()))
    // if refMin is denormal and FTZ mode on, result should be zero
    // if refMin is zero and FTZ mode on, result should be zero
    {
      res &= (min == 0);
    } else {
      // otherwise, refMin should not be greater than result
      res &= (refMinHigh <= T(min));
    }
  }

  if (Utils::IsNInf(refMax)) {
    if (!Utils::IsNInf(max))
      return false;
  } else if (Utils::IsPInf(refMax)) {
    if (!Utils::IsPInf(max))
      return false;
  } else if ((ref4ulps == 0) && RefALU::GetFTZmode()) {
    // one ulp for zero is denormal in float point precision
    // so, flushed max and max should be zero
    if (max != 0)
      return false;
  } else {
    T oneUlp = ComputeUlp(ref4ulps);
    T refMaxHigh = refMax + T(ulps * oneUlp);
    dT refMaxUlps = NEATALU::castDown(refMaxHigh);

    res = true;
    // if refMaxUlps is INF, max should be INF
    if (Utils::IsInf<dT>(refMaxUlps)) {
      res &= Utils::IsInf<dT>(max);
    } else if ((Utils::IsDenorm<dT>(refMaxUlps) && RefALU::GetFTZmode()) ||
               (refMaxUlps == 0 && RefALU::GetFTZmode()))
    // if refMax is denormal and FTZ mode on, result should be zero
    // if refMax is zero and FTZ mode on, result should be zero
    {
      res &= (max == 0);
    } else {
      // otherwise, refMax should not be less than result
      res &= (refMaxHigh >= T(max));
    }
  }
  return res;
}

template <typename T> T GetMaxValue(T vec[], size_t vecSize) {
  T max = vec[0];
  for (size_t i = 1; i < vecSize; i++)
    if (vec[i] > max)
      max = vec[i];

  return max;
}
template <typename T> T GetMinValue(T vec[], size_t vecSize) {
  T min = vec[0];
  for (size_t i = 1; i < vecSize; i++)
    if (vec[i] < min)
      min = vec[i];

  return min;
}

template <typename T> T GetMaxAbsAcc(NEATValue val0, NEATValue val1) {
  T aaa;
  if (fabs(*val0.GetAcc<T>()) > fabs(*val1.GetAcc<T>()))
    aaa = *val0.GetAcc<T>();
  else
    aaa = *val1.GetAcc<T>();
  return aaa;
}

template <typename T> T GetMaxAbsAccVec(uint32_t size, NEATVector arr) {
  T aaa = *arr[0].GetAcc<T>();
  for (uint32_t i = 1; i < size; i++) {
    if (fabs(*arr[i].GetAcc<T>()) > fabs(aaa))
      aaa = *arr[i].GetAcc<T>();
  }
  return aaa;
}

template <typename T>
T GetMaxAbsAccVec(uint32_t size, NEATVector arr0, NEATVector arr1) {
  T aaa;
  if (fabs(*arr0[0].GetAcc<T>()) > fabs(*arr1[0].GetAcc<T>()))
    aaa = *arr0[0].GetAcc<T>();
  else
    aaa = *arr1[0].GetAcc<T>();
  for (uint32_t i = 1; i < size; i++) {
    if (fabs(*arr0[i].GetAcc<T>()) > fabs(aaa))
      aaa = *arr0[i].GetAcc<T>();
    if (fabs(*arr1[i].GetAcc<T>()) > fabs(aaa))
      aaa = *arr1[i].GetAcc<T>();
  }
  return aaa;
}

template <typename T> T GetMaxAbsInt(NEATValue val0) {
  T maxAbs = 0;

  T a0 = fabs(*val0.GetMin<T>());
  T a1 = fabs(*val0.GetMax<T>());

  maxAbs = (a0 > a1 ? a0 : a1);

  return maxAbs;
}

template <typename T> T GetMaxAbsInt(NEATValue val0, NEATValue val1) {
  T maxAbs = 0;

  T a0 = fabs(*val0.GetMin<T>());
  T a1 = fabs(*val1.GetMin<T>());
  T a2 = fabs(*val0.GetMax<T>());
  T a3 = fabs(*val1.GetMax<T>());

  a0 = (a0 > a1 ? a0 : a1);
  a2 = (a2 > a3 ? a2 : a3);
  maxAbs = (a0 > a2 ? a0 : a2);

  return maxAbs;
}

template <typename T> T GetMaxAbsIntVec(uint32_t size, NEATVector arr) {
  T maxAbs = fabs(*arr[0].GetMin<T>());

  for (uint32_t i = 0; i < size; i++) {
    T a0 = fabs(*arr[i].GetMin<T>());
    T a1 = fabs(*arr[i].GetMax<T>());

    a0 = (a0 > a1 ? a0 : a1);
    if (a0 > maxAbs)
      maxAbs = a0;
  }
  return maxAbs;
}

template <typename T>
T GetMaxAbsIntVec(uint32_t size, NEATVector arr0, NEATVector arr1) {
  T maxAbs = fabs(*arr0[0].GetMin<T>());

  for (uint32_t i = 0; i < size; i++) {
    T a0 = fabs(*arr0[i].GetMin<T>());
    T a1 = fabs(*arr1[i].GetMin<T>());
    T a2 = fabs(*arr0[i].GetMax<T>());
    T a3 = fabs(*arr1[i].GetMax<T>());

    a0 = (a0 > a1 ? a0 : a1);
    a2 = (a2 > a3 ? a2 : a3);
    a0 = (a0 > a2 ? a0 : a2);

    if (a0 > maxAbs)
      maxAbs = a0;
  }

  return maxAbs;
}

// generate num (from 1 to numMax=500) random values for a, a and b or a,b and c
// in the ranges [aMin, aMax], [bMin, bMax] and [cMix,cMax] correspondly.
// All result values, calculated by reference function, should be
// between low and high limits of interval of testVal
template <typename T> class TestIntervalRandomly {
private:
  DataTypeVal dataTypeVal;
  bool test;
  static const uint32_t numMax = 500;
  T arrA[numMax], arrB[numMax], arrC[numMax];

public:
  typedef T (*RefFuncPOneArg)(const T &);
  typedef T (*RefFuncPTwoArgs)(const T &, const T &);
  typedef T (*RefFuncPThreeArgs)(const T &, const T &, const T &);

  TestIntervalRandomly(RefFuncPOneArg RefFunc, NEATValue testVal, T aMin,
                       T aMax, const uint32_t num) {
    assert(num <= numMax);
    dataTypeVal = GetDataTypeVal<T>();
    GenerateRangedVectorsAutoSeed(dataTypeVal, &arrA[0], V1, num, aMin, aMax);
    test = true;
    for (uint32_t j = 0; j < num; j++) {
      T refVal = RefFunc(RefALU::flush(arrA[j]));
      test &=
          (refVal >= *testVal.GetMin<T>() && refVal <= *testVal.GetMax<T>());
    }
  }
  TestIntervalRandomly(RefFuncPTwoArgs RefFunc, NEATValue testVal, T aMin,
                       T aMax, T bMin, T bMax, const uint32_t num) {
    assert(num <= numMax);
    dataTypeVal = GetDataTypeVal<T>();
    GenerateRangedVectorsAutoSeed(dataTypeVal, &arrA[0], V1, num, aMin, aMax);
    GenerateRangedVectorsAutoSeed(dataTypeVal, &arrB[0], V1, num, bMin, bMax);
    test = true;
    for (uint32_t j = 0; j < num; j++) {
      T refVal = RefFunc(RefALU::flush(arrA[j]), RefALU::flush(arrB[j]));
      test &=
          (refVal >= *testVal.GetMin<T>() && refVal <= *testVal.GetMax<T>());
    }
  }
  TestIntervalRandomly(RefFuncPThreeArgs RefFunc, NEATValue testVal, T aMin,
                       T aMax, T bMin, T bMax, T cMin, T cMax,
                       const uint32_t num) {
    assert(num <= numMax);
    dataTypeVal = GetDataTypeVal<T>();
    GenerateRangedVectorsAutoSeed(dataTypeVal, &arrA[0], V1, num, aMin, aMax);
    GenerateRangedVectorsAutoSeed(dataTypeVal, &arrB[0], V1, num, bMin, bMax);
    GenerateRangedVectorsAutoSeed(dataTypeVal, &arrC[0], V1, num, cMin, cMax);
    test = true;
    for (uint32_t j = 0; j < num; j++) {
      T refVal = RefFunc(RefALU::flush(arrA[j]), RefALU::flush(arrB[j]),
                         RefALU::flush(arrC[j]));
      test &=
          (refVal >= *testVal.GetMin<T>() && refVal <= *testVal.GetMax<T>());
    }
  }

  bool GetTestResult() { return test; }
};

// Tests special NEATValue for function with two arguments.
// Result must be NEATValue with UNKNOWN status.
template <typename T, NEATValue::Status st>
bool TestSpecialNEATValue(NEATScalarBinaryOp f) {
  bool result = true;
  NEATValue goodValue = NEATValue((T)0.1);
  NEATValue res = f(NEATValue(st), goodValue);
  result = result && res.IsUnknown();
  res = f(goodValue, NEATValue(st));
  result = result && res.IsUnknown();
  res = f(NEATValue(st), NEATValue(st));
  return result && res.IsUnknown();
}

// Tests special NEATValue for function with single argument.
// Result must be NEATValue with UNKNOWN status.
template <NEATValue::Status st> bool TestSpecialNEATValue(NEATScalarUnaryOp f) {
  NEATValue res = f(NEATValue(st));
  return res.IsUnknown();
}

/// Tests that for binary function f and arguments arg1 and arg2
/// NEAT returns accurate value that is equal to refOut
template <typename T>
bool TestPreciseRes(NEATScalarBinaryOp f, T arg1, T arg2, T refOut) {
  bool passed = true;
  NEATValue val1 = NEATValue(arg1);
  NEATValue val2 = NEATValue(arg2);
  NEATValue res = f(val1, val2);
  if (Utils::IsNaN<T>(refOut) && (res.IsNaN<T>()))
    return true;
  passed &= res.IsAcc();
  passed &= Utils::eq(*res.GetAcc<T>(), refOut);
  return passed;
}

/// Tests that for unary function f and argument arg
/// NEAT returns accurate value that is equal to refOut
template <typename T>
bool TestPreciseRes(NEATScalarUnaryOp f, T arg, T refOut) {
  bool passed = true;
  NEATValue val = NEATValue(arg);
  NEATValue res = f(val);
  if (Utils::IsNaN<T>(refOut) && (res.IsNaN<T>()))
    return true;
  passed &= res.IsAcc();
  passed &= Utils::eq(*res.GetAcc<T>(), refOut);
  return passed;
}

template <typename T> bool TestUnknown(NEATScalarTernaryOp f) {
  return TestSpecialNEATValue<T, NEATValue::UNKNOWN>(f);
}

template <typename T> bool TestUnknown(NEATScalarBinaryOp f) {
  return TestSpecialNEATValue<T, NEATValue::UNKNOWN>(f);
}

bool TestUnknown(NEATScalarUnaryOp f);

template <typename T> bool TestUnwritten(NEATScalarTernaryOp f) {
  return TestSpecialNEATValue<T, NEATValue::UNWRITTEN>(f);
}

template <typename T> bool TestUnwritten(NEATScalarBinaryOp f) {
  return TestSpecialNEATValue<T, NEATValue::UNWRITTEN>(f);
}

bool TestUnwritten(NEATScalarUnaryOp f);

template <typename T> bool TestAny(NEATScalarTernaryOp f) {
  return TestSpecialNEATValue<T, NEATValue::ANY>(f);
}

template <typename T> bool TestAny(NEATScalarBinaryOp f) {
  return TestSpecialNEATValue<T, NEATValue::ANY>(f);
}

bool TestAny(NEATScalarUnaryOp f);

// This method checks if 'long double' data type is larger in memory then
// 'double'. If their sizes are equal, we can't to test 'double' version of
// built-in function because reference value must be computed in higher
// precision.
template <typename T> bool SkipDoubleTest() {
  bool LongDoubleSizeEqualsDoubleSize = false;
  if (sizeof(T) == sizeof(double)) {
    LongDoubleSizeEqualsDoubleSize = sizeof(double) == sizeof(long double);
  }

  if (LongDoubleSizeEqualsDoubleSize) {
    printf("WARNING: size of long double is equal to size of double\n");
    printf("WARNING: test for double will be skipped.\n");
  }
  return LongDoubleSizeEqualsDoubleSize;
}

} // namespace Validation

#endif // #ifndef __NEATALUUTILS_H__
