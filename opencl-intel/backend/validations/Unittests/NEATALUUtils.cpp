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

// \brief Implementation of useful functions to share between NEATALU tests.

#include "NEATALUUtils.h"

namespace Validation {

template <> int8_t getMSB<int8_t>() { return INT8_MIN; }
template <> int16_t getMSB<int16_t>() { return INT16_MIN; }
template <> int32_t getMSB<int32_t>() { return INT32_MIN; }
template <> int64_t getMSB<int64_t>() { return INT64_MIN; }
template <> uint8_t getMSB<uint8_t>() { return INT8_MIN; }
template <> uint16_t getMSB<uint16_t>() { return INT16_MIN; }
template <> uint32_t getMSB<uint32_t>() { return INT32_MIN; }
template <> uint64_t getMSB<uint64_t>() { return INT64_MIN; }
template <> float getMSB<float>() {
  union {
    uint32_t i;
    float f;
  } a;
  a.i = INT32_MIN;
  return a.f;
}
template <> double getMSB<double>() {
  union {
    uint64_t i;
    double f;
  } a;
  a.i = INT64_MIN;
  return a.f;
}

// it is used to combine typed tests and value-parameterized tests in googletest
// framework
template <typename T, bool m> const bool ValueTypeContainer<T, m>::mode;

template <> DataTypeVal GetDataTypeVal<float>() { return F32; }
template <> DataTypeVal GetDataTypeVal<double>() { return F64; }
template <> DataTypeVal GetDataTypeVal<int8_t>() { return I8; }
template <> DataTypeVal GetDataTypeVal<uint8_t>() { return U8; }
template <> DataTypeVal GetDataTypeVal<int16_t>() { return I16; }
template <> DataTypeVal GetDataTypeVal<uint16_t>() { return U16; }
template <> DataTypeVal GetDataTypeVal<int32_t>() { return I32; }
template <> DataTypeVal GetDataTypeVal<uint32_t>() { return U32; }
template <> DataTypeVal GetDataTypeVal<int64_t>() { return I64; }
template <> DataTypeVal GetDataTypeVal<uint64_t>() { return U64; }

bool RefIsinf(double _x) {
  union {
    uint32_t u;
    float f;
  } y;
  y.u = 0x7f800000;
  return ((_x) == y.f || (_x) == -(y.f));
}

double RefCopysign(double x, double y) {
  union {
    uint64_t u;
    double f;
  } ux, uy;

  ux.f = x;
  uy.f = y;

  ux.u = (ux.u & 0x7fffffffffffffffULL) | (uy.u & 0x8000000000000000ULL);

  return ux.f;
}

int MyIlogb(double x) {
  union {
    double f;
    uint64_t u;
  } u;
  u.f = x;

  uint64_t absx = u.u & CL_LONG_MAX;
  if (absx - 0x0001000000000000ULL >=
      0x7ff0000000000000ULL - 0x0001000000000000ULL) {
    switch (absx) {
    case 0:
      return (-2147483647 - 1) /* minimum (signed) int value */;
    case 0x7ff0000000000000ULL:
      return 2147483647 /* maximum (signed) int value */;
    default:
      if (absx > 0x7ff0000000000000ULL)
        return (-2147483647 - 1) /* minimum (signed) int value */;

      // subnormal
      u.u = absx | 0x3ff0000000000000ULL;
      u.f -= 1.0;
      return (u.u >> 52) - (1023 + 1022);
    }
  }

  return (absx >> 52) - 1023;
}

int MyIlogbl(long double x) {
  union {
    long double f;
    struct {
      uint64_t m;
      uint16_t sexp;
    } u;
  } u;
  u.u.m = 0;
  u.u.sexp = 0;

  u.f = x;

  int exp = u.u.sexp & 0x7fff;
  if (0 == exp) {
    if (0 == u.u.m)
      return (-2147483647 - 1) /* minimum (signed) int value */;

    // subnormal
    u.u.sexp = 0x3fff;
    u.f -= 1.0f;
    exp = u.u.sexp & 0x7fff;

    return exp - (0x3fff + 0x3ffe);
  } else if (0x7fff == exp) {
    if (u.u.m & CL_LONG_MAX)
      return (-2147483647 - 1) /* minimum (signed) int value */;

    return 2147483647 /* maximum (signed) int value */;
  }

  return exp - 0x3fff;
}

bool TestUnknown(NEATScalarUnaryOp f) {
  return TestSpecialNEATValue<NEATValue::UNKNOWN>(f);
}

bool TestUnwritten(NEATScalarUnaryOp f) {
  return TestSpecialNEATValue<NEATValue::UNWRITTEN>(f);
}

bool TestAny(NEATScalarUnaryOp f) {
  return TestSpecialNEATValue<NEATValue::ANY>(f);
}

} // namespace Validation
