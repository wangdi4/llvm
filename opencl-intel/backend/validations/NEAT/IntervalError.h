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

#ifndef NEAT_INTERVAL_ERROR
#define NEAT_INTERVAL_ERROR

#include "FloatOperations.h"
#include "NEATValue.h"
#include "imathLibd.h"

namespace Validation {
double ComputeUlp(double ref);
long double ComputeUlp(long double ref);

/// @brief represents error for NEAT interval
/// Template argument is a type for what the error is calculated
template <typename T> class IntervalError {
public:
  typedef typename Utils::superT<T>::type sT;
  typedef enum {
    ERROR_ABSOLUTE,
    ERROR_ULPS,
    ERROR_BOUNDARY_ULPS,
    ERROR_UNDEFINED
  } ErrorType;
  /// @brief default ctor
  IntervalError()
      : m_ErrorType(ERROR_UNDEFINED), m_isUlpValueAccurate(true),
        m_mayViolateIEEE754(false) {}

  // @brief construct from ulp value
  explicit IntervalError(float ulps, bool mayViolateIEEE754 = false) {
    m_ErrorType = ERROR_ULPS;
    m_isUlpValueAccurate = (ComputeUlp((double)ulps) < 1.0f) ? true : false;
    m_UlpError = ulps;
    m_mayViolateIEEE754 = mayViolateIEEE754;
  }

  bool operator==(const IntervalError<T> &RHS) {
    bool ret = true;
    ret &= this->m_ErrorType == RHS.m_ErrorType;
    ret &= this->m_mayViolateIEEE754 == RHS.m_mayViolateIEEE754;

    switch (m_ErrorType) {
    case ERROR_ABSOLUTE:
      ret &= this->getAbsoluteError() == RHS.getAbsoluteError();
      break;
    case ERROR_ULPS:
    case ERROR_BOUNDARY_ULPS:
      ret &= this->getUlpsForMin() == RHS.getUlpsForMin();
      ret &= this->getUlpsForMax() == RHS.getUlpsForMax();
      break;
    default:
      // return false if we are comparing unknown types
      ret &= false;
    }

    return ret;
  }
  bool operator==(const sT &RHS) {
    bool ret = true;
    switch (m_ErrorType) {
    case ERROR_ABSOLUTE:
      ret &= this->getAbsoluteError() == RHS;
      break;
    case ERROR_ULPS:
    case ERROR_BOUNDARY_ULPS:
      ret &= this->getUlpsForMin() == (float)RHS;
      ret &= this->getUlpsForMax() == (float)RHS;
      break;
    default:
      // return false if we are comparing unknown types
      ret &= false;
    }

    return ret;
  }
  bool operator>(const sT &RHS) {
    bool ret = true;

    switch (m_ErrorType) {
    case ERROR_ABSOLUTE:
      ret &= this->getAbsoluteError() > RHS;
      break;
    case ERROR_ULPS:
    case ERROR_BOUNDARY_ULPS:
      ret &= this->getUlpsForMin() > (float)RHS;
      ret &= this->getUlpsForMax() > (float)RHS;
      break;
    default:
      // return false if we are comparing unknown types
      ret &= false;
    }
    return ret;
  }
  bool operator>=(const sT &RHS) { return (*this == RHS) || (*this > RHS); }

  /// @brief construct error from Absolute Error
  /// @param ErrorValue value of the absolute error
  static IntervalError<T> fromAbsoluteError(const sT ErrorValue,
                                            bool mayViolateIEEE754 = false) {
    IntervalError<T> ret;
    ret.m_mayViolateIEEE754 = mayViolateIEEE754;
    ret.m_ErrorType = ERROR_ABSOLUTE;
    ret.m_AbsError = ErrorValue;
    return ret;
  }

  /// @brief construct error from Ulps
  /// @param ErrorValue number of Ulps
  static IntervalError<T> fromUlpsError(const float ErrorValue,
                                        bool mayViolateIEEE754 = false) {
    IntervalError<T> ret;
    ret.m_ErrorType = ERROR_ULPS;
    ret.m_mayViolateIEEE754 = mayViolateIEEE754;
    ret.m_UlpError = ErrorValue;
    ret.m_isUlpValueAccurate =
        (ComputeUlp((double)ErrorValue) < 1.0f) ? true : false;
    return ret;
  }

  /// @brief construct error from Ulps
  /// @param LHErrorValue number of Ulps in left boundary
  /// @param RHErrorValue number of Ulps in right boundary
  static IntervalError<T>
  fromUlpsIntervalError(const float LHErrorValue, const float &RHErrorValue,
                        bool mayViolateIEEE754 = false) {
    IntervalError<T> ret;
    ret.m_ErrorType = ERROR_BOUNDARY_ULPS;
    ret.m_mayViolateIEEE754 = mayViolateIEEE754;
    ret.m_UlpIntError[0] = LHErrorValue;
    ret.m_UlpIntError[1] = RHErrorValue;
    ret.m_isUlpValueAccurate =
        ((ComputeUlp((double)LHErrorValue) < 1.0f) ? true : false) &&
        ((ComputeUlp((double)RHErrorValue) < 1.0f) ? true : false);
    return ret;
  }

  /// @brief get number of ulps
  /// @note could be called only for error with ERROR_ULPS type
  float getConstantUlps() const {
    assert(m_ErrorType == ERROR_ULPS &&
           " getConstantUlps Error is not error in ulps");
    return m_UlpError;
  }

  /// @brief get type of error
  ErrorType getErrorType() const { return m_ErrorType; }

  /// @brief get number of ulps in left boundary
  /// @note could be called only for error with ERROR_ULPS or
  /// ERROR_BOUNDARY_ULPS type
  float getUlpsForMin() const {
    assert((m_ErrorType == ERROR_ULPS || m_ErrorType == ERROR_BOUNDARY_ULPS) &&
           "getUlpsForMin unsupported error type");
    return (m_ErrorType == ERROR_ULPS) ? m_UlpError : m_UlpIntError[0];
  }

  /// @brief get number of ulps in right boundary
  /// @note could be called only for error with ERROR_ULPS or
  /// ERROR_BOUNDARY_ULPS type
  float getUlpsForMax() const {
    assert((m_ErrorType == ERROR_ULPS || m_ErrorType == ERROR_BOUNDARY_ULPS) &&
           "getUlpsForMax unsupported error type");
    return (m_ErrorType == ERROR_ULPS) ? m_UlpError : m_UlpIntError[1];
  }

  /// @brief get absolute error
  sT getAbsoluteError() const {
    assert(m_ErrorType == ERROR_ABSOLUTE &&
           "getAbsoluteError unsupported error type");

    return m_AbsError;
  }

  /// @brief is finite error
  bool isInfiniteError() const {
    bool ret = false;
    switch (m_ErrorType) {
    case ERROR_ABSOLUTE:
      return Utils::IsInf<sT>(m_AbsError);
    case ERROR_ULPS:
      return Utils::IsInf<float>(m_UlpError);
    case ERROR_BOUNDARY_ULPS:
      ret |= Utils::IsInf<float>(m_UlpIntError[0]);
      ret |= Utils::IsInf<float>(m_UlpIntError[1]);
      return ret;
    default:
      throw Exception::InvalidArgument(
          "[IntervalError] Unsupported error type");
    }
    return ret;
  }

  /// @brief could ulp value be presented without losing precision
  bool isAccurateUlps() const {
    assert((m_ErrorType == ERROR_ULPS || m_ErrorType == ERROR_BOUNDARY_ULPS) &&
           "isAccurateUlps unsupported error type");
    return m_isUlpValueAccurate;
  }

  bool mayViolateIEEE754() const { return m_mayViolateIEEE754; }

  /// @brief expand interval provided by input arguments and return expanded
  /// interval through the same variables on given error value
  /// @param [input/output] minInOut left input interval boundary
  /// @param [input/output] maxInOut right input interval boundary
  /// @param error error value
  static void ExpandFPInterval(sT *minInOut, sT *maxInOut,
                               IntervalError<T> error);

  /// @brief expand interval provided by input arguments and return expanded
  /// interval through the same variables on given error value
  /// @param [input/output] minInOut left boundary
  /// @param [input/output] maxInOut right boundary
  /// @param ref4Ulps value to compute OneUlp from
  /// @param error error value
  static void ExpandFPInterval(sT *minInOut, sT *maxInOut, sT ref4Ulps,
                               IntervalError<T> error);

  /// @section construct error for specific built-in
  /// @from Table 7.2
  static IntervalError<T> divFrmSetUlps(const NEATValue &a, const NEATValue &b);
  static IntervalError<T> cosFrmSetUlps(const NEATValue &a);
  static IntervalError<T> sinFrmSetUlps(const NEATValue &a) {
    // sin error equals to cos error
    return cosFrmSetUlps(a);
  }
  static IntervalError<T> expFrmSetUlps(const NEATValue &a);
  static IntervalError<T> logFrmSetUlps(const NEATValue &a);

  /// @brief get floating point value for interval expansion
  /// @param OneUlp value of one ulp
  /// @param isMin compute value for let boundary
  sT getExpandValue(const sT OneUlp, const bool isMin) const {
    sT retValue = 0.;
    switch (m_ErrorType) {
    case ERROR_ABSOLUTE:
      retValue = m_AbsError;
      break;
    case ERROR_ULPS:
      retValue = m_UlpError * OneUlp;
      break;
    case ERROR_BOUNDARY_ULPS:
      retValue = m_UlpIntError[isMin ? 0 : 1] * OneUlp;
      break;
    default:
      throw Exception::InvalidArgument(
          "[IntervalError] Unsupported error type");
    }
    return retValue;
  }

private:
  /// Type of error
  ErrorType m_ErrorType;

  /// is m_UlpError + 1 differ from m_UlpError
  bool m_isUlpValueAccurate;

  bool m_mayViolateIEEE754;

  union {
    // @brief Represents absolute error
    // this filed is active if m_ErrorType is ERROR_ABSOLUTE
    sT m_AbsError;
    // @brief Represents constant error in ulps value
    // this filed is active if m_ErrorType is ERROR_ULPS
    float m_UlpError;
    // @brief Represents non-constant error in ulps value
    // where number of error ulps in left boundary differ from
    // number of error ulps in right boundary
    // this filed is active if m_ErrorType is ERROR_BOUNDARY_ULPS
    // [0] - error ulps in left boundary
    // [1] - error ulps in right boundary
    float m_UlpIntError[2];
  };
};

template <typename T>
void IntervalError<T>::ExpandFPInterval(typename IntervalError<T>::sT *minInOut,
                                        typename IntervalError<T>::sT *maxInOut,
                                        IntervalError<T> error) {
  typedef typename IntervalError<T>::sT sT;
  const uint64_t LONG_DOUBLE_MANTISSA_MASK = 0x7fffffffffffffff;

  assert(error >= 0 && "[IntervalError] Error value lower than zero");
  if (error == 0.0)
    return;

  int n;

  sT refMax = *maxInOut;
  sT refMin = *minInOut;
  sT ulpMax = ComputeUlp(refMax); // calc ulp for ref value
  sT ulpMin = ComputeUlp(refMin); // calc ulp for ref value

  // high limit expand
  sT result = CimathLibd::imf_frexp(refMax, &n);

  // check if refMax on the boundary of higher exponent
  // and if refMax is normal number
  if ((!(Utils::IsDenorm<sT>(refMax) || Utils::IsDenorm<T>(refMax) ||
         (T(refMax) == T(0.0) && sT(refMax) != sT(0.0)))) &&
      CimathLibd::imf_fabs(result) == (sT)0.5) {
    ulpMax /= (sT)2.;
  }

  // check if resMax have higher value of exponent. if so we can
  // possible lose a bit in precision here.
  sT resMax = refMax + error.getExpandValue(ulpMax, false); // add ulps to ref
  sT refMax_sub =
      resMax - error.getExpandValue(ulpMax, false); // add ulps to ref
  if (refMax_sub > refMax) {
    T maxD = (T)resMax; // downcast to lower precision
    sT maxS = sT(maxD); // conversion to higher precision
    if (maxS == resMax) {
      resMax -= (sT)ulpMax;
    }
  }

  // low limit expand
  result = CimathLibd::imf_frexp(refMin, &n);

  // check if refMin on the boundary of higher exponent
  // and if result is normal number
  if ((!(Utils::IsDenorm<sT>(refMin) || Utils::IsDenorm<T>(refMin) ||
         (T(refMin) == T(0.0) && sT(refMin) != sT(0.0)))) &&
      CimathLibd::imf_fabs(result) == (sT)0.5) {
    ulpMin /= (sT)2.;
  }

  // check if resMax have lower value of exponent. if so we can
  // possible lose a bit in precision here.
  sT resMin = refMin - error.getExpandValue(ulpMin, true); // add ulps to ref
  sT refMin_add =
      resMin + error.getExpandValue(ulpMin, true); // add ulps to ref
  if (refMin_add < refMin) {
    T minD = (T)resMin; // downcast to lower precision
    sT minS = sT(minD); // conversion to higher precision
    if (minS == resMin) {
      resMin += (sT)ulpMin;
    }
  }

  T maxD = (T)resMax; // downcast to lower precision

  T minD = (T)resMin; // downcast to lower precision

  sT minS = sT(minD); // conversion to higher precision

  sT maxS = sT(maxD); // conversion to higher precision

  if (maxS > resMax && fabs(ComputeUlp(maxS)) <= fabs(maxS - minS)) {
    uint64_t maxS_mant;
    memcpy(&maxS_mant, &maxS, sizeof(uint64_t));
    maxS_mant &= LONG_DOUBLE_MANTISSA_MASK;
    sT lowUlp = (maxS_mant == 0) ? ComputeUlp(refMax) : ComputeUlp(maxS);
    maxS -= lowUlp;
  }

  if (minS < resMin && fabs(ComputeUlp(minS)) <= fabs(maxS - minS)) {
    // if we are, reduce result by one ulp, calculated for
    // downcasted value
    uint64_t minS_mant;
    memcpy(&minS_mant, &minS, sizeof(uint64_t));
    minS_mant &= LONG_DOUBLE_MANTISSA_MASK;
    sT lowUlp = (minS_mant == 0) ? ComputeUlp(refMin) : ComputeUlp(minS);
    minS += lowUlp;
  }

  *maxInOut = maxS;
  *minInOut = minS;
}

template <typename T>
void IntervalError<T>::ExpandFPInterval(sT *minInOut, sT *maxInOut, sT ref4Ulps,
                                        IntervalError<T> error) {
  typedef typename IntervalError<T>::sT sT;

  assert(error >= 0 && "[IntervalError] Error value lower than zero");
  if (error == 0.0)
    return;

  int n;

  sT refMax = *maxInOut;
  sT refMin = *minInOut;

  sT ulpMax = ComputeUlp(ref4Ulps); // calc ulp for ref value
  sT ulpMin = ComputeUlp(ref4Ulps); // calc ulp for ref value

  // high limit expand
  sT result = CimathLibd::imf_frexp(refMax, &n);

  // check if refMax on the boundary of higher exponent
  if (CimathLibd::imf_fabs(result) == (sT)0.5L) {
    ulpMax /= (sT)2.0L;
  }

  sT resMax = refMax + error.getExpandValue(ulpMax, false); // add ulps to ref

  // low limit expand
  result = CimathLibd::imf_frexp(refMin, &n);

  // check if refMin on the boundary of higher exponent
  if (CimathLibd::imf_fabs(result) == (sT)0.5L) {
    ulpMin /= (sT)2.0L;
  }

  sT resMin = refMin - error.getExpandValue(ulpMin, true); // add ulps to ref

  T maxD = (T)resMax; // downcast to lower precision
  sT maxS = sT(maxD); // conversion to higher precision
  T minD = (T)resMin; // downcast to lower precision
  sT minS = sT(minD); // conversion to higher precision

  if (maxS > resMax && fabs(ComputeUlp(maxS)) <= fabs(maxS - minS)) {
    sT lowUlp = ComputeUlp(maxS);
    maxS -= lowUlp;
  }

  if (minS < resMin && fabs(ComputeUlp(minS)) <= fabs(maxS - minS)) {
    // if we are, reduce result by one ulp, calculated for
    // downcasted value
    sT lowUlp = ComputeUlp(minS);
    minS += lowUlp;
  }

  *minInOut = minS;
  *maxInOut = maxS;
}

} // namespace Validation

#endif
