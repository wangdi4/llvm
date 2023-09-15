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

#ifndef __NEATDATATYPES_H__
#define __NEATDATATYPES_H__

#include "DataType.h"
#include "Exception.h"
#include "FloatOperations.h"
#include "dxfloat.h"
#include <assert.h>
#include <cstring>
#include <sstream>

namespace Validation {
///////////////////////
/// NEATValue structure
///
/// @brief defines status of vector element. Describes if it is accurate or from
/// interval or unknown.

/// TODO: write specialization for constructor getting status as argument
struct NEATValue {
public:
  ///  ACCURATE - The value described is bitwise accurate
  ///  ANY - no information about the value. Any value in the output will be
  ///  correct UNKNOWN - value cannot be presented with an interval due to ALU
  ///  design. UNWRITTEN - This value was never written to (this is somewhat
  ///  different from unknown) INTERVAL - The value is floating point (can be
  ///  NaN, inf, or between min and max).
  ///
  ///  The difference between unknown and unwritten:
  ///  unwritten means uninitialized.
  ///  unknown means arbitrary value which is a result of a previous
  ///  computation.
  enum Status { ACCURATE = 0, INTERVAL, UNKNOWN, UNWRITTEN, ANY };

  /// @brief default ctor
  NEATValue() : m_Status(UNWRITTEN) {}

  /// @brief ctor for accurate NEATValue creation
  template <typename T> explicit NEATValue(T accurateVal) {
    SetAccurateVal(accurateVal);
  }

  explicit NEATValue(Status st) {
    assert(st != ACCURATE &&
           "Use different constructor to create ACCURATE NEAT value.");
    assert(st != INTERVAL &&
           "Use different constructor to create INTERVAL NEAT value.");
    SetStatus(st);
  }

  /// @brief ctor for interval NEATValue creation
  template <typename T> explicit NEATValue(T in_lowerBound, T in_upperBound) {
    // Constraint template to work only with float, double and CFloat16.
    // Otherwise compiler will report an error
    IsFloatType<T> _notUsed;
    UNUSED_ARGUMENT(_notUsed);
    // Check if both values are NaNs
    if (Utils::IsNaN<T>(in_lowerBound) && (Utils::IsNaN<T>(in_upperBound))) {
      SetAccurateVal(in_lowerBound);
    } else if (!Utils::eq(in_lowerBound, in_upperBound))
      SetIntervalVal(in_lowerBound, in_upperBound);
    else
      SetAccurateVal(in_lowerBound);
  }

  /// assignment o-r
  NEATValue &operator=(const NEATValue &p);

  /// @brief Sets status value
  void SetStatus(Status in_Status);

  /// @return status of NEAT variable
  const Status &GetStatus() const;

  bool IsUnknown() const;
  bool IsAny() const;
  bool IsAcc() const;
  bool IsUnwritten() const;
  bool IsInterval() const {
    if (m_Status == INTERVAL) {
      return true;
    } else
      return false;
  }

  /// @return Accurate NEAT value
  /// @throws IllegalFunctionCall if status is not ACCURATE
  template <typename T> const T *GetAcc() const {
    if (m_Status != ACCURATE)
      throw Exception::IllegalFunctionCall(
          "Cannot get accurate NEAT value for inaccurate status");
    // for accurate status min and max are equal. Doesn't matter which one to
    // return
    return reinterpret_cast<const T *>(&m_min[0]);
  }

  /// @return Lower interval boundary of NEAT interval
  /// @throws IllegalFunctionCall if status is ACCURATE,UNKNOWN or UNWRITTEN
  template <typename T> const T *GetMin() const {
    if ((m_Status == UNWRITTEN) || (m_Status == UNKNOWN) || (m_Status == ANY))
      throw Exception::IllegalFunctionCall(
          "Cannot get lower interval boundary for this status");
    return reinterpret_cast<const T *>(&m_min[0]);
  }

  /// @return Upper interval boundary of NEAT interval
  /// @throws IllegalFunctionCall if status is ACCURATE,UNKNOWN or UNWRITTEN
  template <typename T> const T *GetMax() const {
    if ((m_Status == UNWRITTEN) || (m_Status == UNKNOWN) || (m_Status == ANY))
      throw Exception::IllegalFunctionCall(
          "Cannot get upper interval boundary for this status");
    // In case of accurate status min and max are equal
    return reinterpret_cast<const T *>(&m_max[0]);
  }

  /// @brief Sets accurate value and status to ACCURATE
  template <typename T> void SetAccurateVal(T in_newVal) {
    SetStatus(ACCURATE);
    memcpy(m_min, &in_newVal, sizeof(T));
    memcpy(m_max, &in_newVal, sizeof(T));
  }

  /// @brief Converts NEATValue to readable string
  template <typename T> std::string ToString() const {
    std::stringstream ss;
    switch (m_Status) {
    case ACCURATE:
      ss << "ACCURATE " << *GetAcc<T>();
      ss << " ACCURATE (hex) 0x" << std::hex
         << *(const uint64_t *)(GetAcc<T>());
      break;
    case INTERVAL:
      ss << "INTERVAL [" << *GetMin<T>() << "; " << *GetMax<T>() << "]";
      ss << " INTERVAL (hex) [0x" << std::hex
         << *(const uint64_t *)(GetMin<T>()) << "; 0x"
         << *(const uint64_t *)(GetMax<T>()) << "]";
      break;
    case UNKNOWN:
      ss << "UNKNOWN";
      break;
    case UNWRITTEN:
      ss << "UNWRITTEN";
      break;
    case ANY:
      ss << "ANY";
      break;
    }
    return ss.str();
  }

  /// @brief Sets interval value and status according to type in template
  /// parameter If lower bound equals to upper bound then status is accurate.
  /// Interval status is set otherwise.
  template <typename T> void SetIntervalVal(T in_lowerBound, T in_upperBound) {
    // TODO: Test low > up to catch NaNs also.
    // Currently tests are failing, so comment it out
    // if(!(in_lowerBound < in_upperBound))
    if (Utils::gt(in_lowerBound, in_upperBound)) {
      throw Exception::InvalidArgument(
          "Invalid interval specified.Lower interval boundary exceeds upper "
          "boundary.");
    }
    // Constraint template to work only with float, double and CFloat16.
    // Otherwise compiler will report an error
    IsFloatType<T> _notUsed;
    UNUSED_ARGUMENT(_notUsed);
    if (Utils::eq(in_lowerBound, in_upperBound)) {
      SetStatus(ACCURATE);
    } else {
      SetStatus(INTERVAL);
    }
    memcpy(m_min, &in_lowerBound, sizeof(T));
    memcpy(m_max, &in_upperBound, sizeof(T));
  }

  template <typename T> bool IsFinite() const {
    if ((m_Status == UNWRITTEN) || (m_Status == UNKNOWN) || (m_Status == ANY))
      throw Exception::IllegalFunctionCall(
          "IsFinite was called for illegal status");
    T min = *GetMin<T>();
    T max = *GetMax<T>();
    if (Utils::IsNaN(min) || Utils::IsNaN(max) || Utils::IsPInf(min) ||
        Utils::IsPInf(max) || Utils::IsNInf(min) || Utils::IsNInf(max)) {
      return false;
    }
    return true;
  }

  template <typename T> bool Includes(T val) const {
    switch (m_Status) {
    case ACCURATE:
      return val == *GetAcc<T>();
    case INTERVAL:
      if (Utils::le(*GetMin<T>(), val) && Utils::le(val, *GetMax<T>()))
        return true;
      return false;
    case ANY:
      // variable can have ANY value (including NaN and infinities)
      return true;
    default:
      return false;
    }
  }

  template <typename T> bool IsNaN() const {
    bool res = false;
    // suppose that NaN is accurate value
    if (GetStatus() == ACCURATE) {
      T accVal;
      memcpy(&accVal, GetAcc<T>(), sizeof(T));
      res = Utils::IsNaN<T>(accVal);
    }
    return res;
  }
  /// support for serialization
  friend std::istream &operator>>(std::istream &, NEATValue &);
  friend std::ostream &operator<<(std::ostream &, const NEATValue &);

  template <typename T> static NEATValue NaN() {
    NEATValue val(Utils::GetNaN<T>());
    return val;
  }

  /// compare two NEATValues for equality, used for test purpose
  template <typename T>
  static bool areEqual(const NEATValue &val_1, const NEATValue &val_2) {
    NEATValue val1 = val_1;
    NEATValue val2 = val_2;

    if (val1.IsNaN<T>() && val2.IsNaN<T>())
      return true;

    NEATValue::Status status = val1.GetStatus();
    if (val2.GetStatus() != status)
      return false;

    switch (status) {
    case ACCURATE:
      T acc1, acc2;
      memcpy(&acc1, val1.GetAcc<T>(), sizeof(T));
      memcpy(&acc2, val1.GetAcc<T>(), sizeof(T));
      if (acc1 == acc2)
        return true;
      else
        return false;
    case UNKNOWN:
    case UNWRITTEN:
    case ANY:
      return true;
    case INTERVAL: {
      bool res = true;
      T min1, min2, max1, max2;
      memcpy(&min1, val1.GetMin<T>(), sizeof(T));
      memcpy(&max1, val1.GetMax<T>(), sizeof(T));
      memcpy(&min2, val2.GetMin<T>(), sizeof(T));
      memcpy(&max2, val2.GetMax<T>(), sizeof(T));

      if (Utils::IsNaN(min1)) {
        res &= (Utils::IsNaN(min1) && Utils::IsNaN(min2));
      } else {
        res &= (double(min1) == min2);
      }

      if (Utils::IsNaN(max1)) {
        res &= (Utils::IsNaN(max1) && Utils::IsNaN(max2));
      } else {
        res &= (double(max1) == max2);
      }
      return res;
    }
    }
    assert(false && "Invalid status!");
  }

  /// @return true if NEATValue status is valid i.e. one of expected values.
  bool IsValid() const {
    return IsAny() || IsInterval() || IsAcc() || IsUnknown() || IsUnwritten();
  }

private:
// TODO: delete MaxBytes = sizeof(long double);
#ifdef _WIN32
  static const uint32_t MaxBytes = sizeof(double);
#else
  static const uint32_t MaxBytes = sizeof(long double);
#endif
  Status m_Status;
  // DataTypeValWrapper m_Type;
  uint8_t m_min[MaxBytes];
  uint8_t m_max[MaxBytes];
};

// support for serialization
std::istream &operator>>(std::istream &, NEATValue &);
std::ostream &operator<<(std::ostream &s, const NEATValue &);
} // namespace Validation

#endif // __NEATDATATYPES_H__
