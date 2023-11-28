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

#ifndef BLT_CONVERSION_H
#define BLT_CONVERSION_H

#include "Helpers.h"
#include "RefALU.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/DerivedTypes.h"

namespace Validation {
namespace OCLBuiltins {

#define SATURATE true
#define DONT_SATURATE !SATURATE
// template func to convert from type T to either APInt in case of integers of
// convert to float and double in case of floating point
template <typename T> typename retType<T>::type inline AsT(const T &R) {
  llvm::APInt ret(sizeof(T) * 8, R, isSignedType<T>());
  return ret;
}

template <> inline float AsT(const float &R) { return R; }

template <> inline double AsT(const double &R) { return R; }

// RMode - rounding mode
// 0 - default
// 1 - rte(nearest even)
// 2 - rtz(towards zero)
// 3 - rtp(towards +inf)
// 4 - rtn(towards -inf)
typedef enum { I2I = 0, I2F, F2I, F2F } convType;

const int RModeDef = 0;
const int rte = 1;
const int rtz = 2;
const int rtp = 3;
const int rtn = 4;

template <typename T> struct IsFloat;
template <> struct IsFloat<float> {
  static bool const is = true;
};
template <> struct IsFloat<double> {
  static bool const is = true;
};
template <> struct IsFloat<int8_t> {
  static bool const is = false;
};
template <> struct IsFloat<uint8_t> {
  static bool const is = false;
};
template <> struct IsFloat<int16_t> {
  static bool const is = false;
};
template <> struct IsFloat<uint16_t> {
  static bool const is = false;
};
template <> struct IsFloat<int32_t> {
  static bool const is = false;
};
template <> struct IsFloat<uint32_t> {
  static bool const is = false;
};
template <> struct IsFloat<int64_t> {
  static bool const is = false;
};
template <> struct IsFloat<uint64_t> {
  static bool const is = false;
};

template <typename TDst, typename TSrc, bool fDst = IsFloat<TDst>::is,
          bool fSrc = IsFloat<TSrc>::is>
struct types2ConvType;

template <typename TDst, typename TSrc>
struct types2ConvType<TDst, TSrc, true, true> {
  static convType const Ty = F2F;
};
template <typename TDst, typename TSrc>
struct types2ConvType<TDst, TSrc, true, false> {
  static convType const Ty = I2F;
};
template <typename TDst, typename TSrc>
struct types2ConvType<TDst, TSrc, false, true> {
  static convType const Ty = F2I;
};
template <typename TDst, typename TSrc>
struct types2ConvType<TDst, TSrc, false, false> {
  static convType const Ty = I2I;
};

template <typename TDst, typename TSrc,
          convType cT = types2ConvType<TDst, TSrc>::Ty>
struct Saturate;

template <typename TDst, typename TSrc> struct Saturate<TDst, TSrc, F2I> {
  static TSrc sat(const TSrc &val) {
    IsFloatType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // Float to Integer
    IsIntegerType<TDst> _y;
    UNUSED_ARGUMENT(_y);
    TSrc R;
    if (val != val) // NaN case
      R = 0;
    R = std::max(std::min((TSrc)std::numeric_limits<TDst>::max(), val),
                 (TSrc)std::numeric_limits<TDst>::min());
    return R;
  }
};
template <typename TDst, typename TSrc> struct Saturate<TDst, TSrc, F2F> {
  static TSrc sat(const TSrc &val) {
    IsFloatType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // Float to Float
    IsFloatType<TDst> _y;
    UNUSED_ARGUMENT(_y);
    return val;
  }
};
template <typename TDst, typename TSrc> struct Saturate<TDst, TSrc, I2F> {
  static TSrc sat(const TSrc &val) {
    IsIntegerType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // Int to Float/Int
    IsFloatType<TDst> _y;
    UNUSED_ARGUMENT(_y);
    TSrc R;
    R = std::max(std::min((TSrc)std::numeric_limits<TDst>::max(), val),
                 (TSrc)std::numeric_limits<TDst>::min());
    return R;
  }
};
template <typename TDst, typename TSrc> struct Saturate<TDst, TSrc, I2I> {
  static TSrc sat(const TSrc &val) {
    IsIntegerType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // Int to Float/Int
    IsIntegerType<TDst> _y;
    UNUSED_ARGUMENT(_y);
    TSrc R;
    R = std::max(std::min((TSrc)std::numeric_limits<TDst>::max(), val),
                 (TSrc)std::numeric_limits<TDst>::min());
    return R;
  }
};

// call_rte_rounding
template <typename TDst, typename TSrc,
          convType cT = types2ConvType<TDst, TSrc>::Ty>
struct call_rte_rounding;

template <typename TDst, typename TSrc>
struct call_rte_rounding<TDst, TSrc, F2I> {
  static TDst round(const TSrc &val) // Round to nearest even
  {
    IsFloatType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // float to int case
    IsIntegerType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    TSrc fIntPart;
    TSrc fract = std::modf(val, &fIntPart);
    TDst intPart = (TDst)fIntPart;
    if (fract == (TSrc)0.5f) // half case
    {
      if (intPart % 2 == 1)                                // is odd
        intPart += intPart <= 0 ? (TDst)(-1) : (TDst)(+1); // sign
    } else if ((fract > (TSrc)0.5f) &&
               (fract != (TSrc)0.0f)) // round to nearest
    {
      intPart += intPart <= 0 ? (TDst)(-1) : (TDst)(1);
    }
    return intPart;
  }
};

template <typename TDst, typename TSrc>
struct call_rte_rounding<TDst, TSrc, F2F> {
  static TDst round(const TSrc &val) // Round to nearest even
  {
    IsFloatType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // float to float case
    IsFloatType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    return (TDst)val;
  }
};

template <typename TDst, typename TSrc>
struct call_rte_rounding<TDst, TSrc, I2F> {
  static TDst round(const TSrc &val) // Round to nearest even
  {
    IsIntegerType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // int to float case
    IsFloatType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    return val;
  }
};

template <typename TDst, typename TSrc>
struct call_rte_rounding<TDst, TSrc, I2I> {
  static TDst round(const TSrc &val) // Round to nearest even
  {
    IsIntegerType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // int to int case
    IsIntegerType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    return val;
  }
};

// call_rtz_rounding
template <typename TDst, typename TSrc,
          convType cT = types2ConvType<TDst, TSrc>::Ty>
struct call_rtz_rounding;

template <typename TDst, typename TSrc>
struct call_rtz_rounding<TDst, TSrc, F2I> {
  static TDst round(const TSrc &val) // Round toward zero
  {
    IsFloatType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // float to int case
    IsIntegerType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    return (TDst)val;
  }
};

template <typename TDst, typename TSrc>
struct call_rtz_rounding<TDst, TSrc, F2F> {
  static TDst round(const TSrc &val) // Round toward zero
  {
    IsFloatType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // float to float case
    IsFloatType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    return (TDst)val;
  }
};

template <typename TDst, typename TSrc>
struct call_rtz_rounding<TDst, TSrc, I2F> {
  static TDst round(const TSrc &val) // Round toward zero
  {
    IsIntegerType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // int to float case
    IsFloatType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    return (TDst)val;
  }
};

template <typename TDst, typename TSrc>
struct call_rtz_rounding<TDst, TSrc, I2I> {
  static TDst round(const TSrc &val) // Round toward zero
  {
    IsIntegerType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // int to int case
    IsIntegerType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    return (TDst)val;
  }
};

// call_rtp_rounding
template <typename TDst, typename TSrc,
          convType cT = types2ConvType<TDst, TSrc>::Ty>
struct call_rtp_rounding;

template <typename TDst, typename TSrc>
struct call_rtp_rounding<TDst, TSrc, F2F> {
  static TDst round(const TSrc &val) // Round toward +inf
  {
    IsFloatType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // float to float case
    IsFloatType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    return (TDst)val;
  }
};

template <typename TDst, typename TSrc>
struct call_rtp_rounding<TDst, TSrc, I2I> {
  static TDst round(const TSrc &val) // Round toward +inf
  {
    IsIntegerType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // int to int case
    IsIntegerType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    return (TDst)val;
  }
};

template <typename TDst, typename TSrc>
struct call_rtp_rounding<TDst, TSrc, I2F> {
  static TDst round(const TSrc &val) // Round toward +inf
  {
    IsIntegerType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // int to float case
    IsFloatType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    return (TDst)val;
  }
};

template <typename TDst, typename TSrc>
struct call_rtp_rounding<TDst, TSrc, F2I> {
  static TDst round(const TSrc &val) // Round toward +inf
  {
    IsFloatType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // float to int case
    IsIntegerType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    TSrc fIntPart;
    TSrc fract = std::modf(val, &fIntPart);
    TDst intPart = (TDst)fIntPart;

    if (fract != (TSrc)0.0f) // round to nearest
    {
      intPart += (TDst)1;
    }
    return intPart;
  }
};

// call_rtn_rounding
template <typename TDst, typename TSrc,
          convType cT = types2ConvType<TDst, TSrc>::Ty>
struct call_rtn_rounding; // Round toward -inf

template <typename TDst, typename TSrc>
struct call_rtn_rounding<TDst, TSrc, F2F> {
  static TDst round(const TSrc &val) // Round toward -inf
  {
    IsFloatType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // float to float case
    IsFloatType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    return (TDst)val;
  }
};

template <typename TDst, typename TSrc>
struct call_rtn_rounding<TDst, TSrc, I2I> {
  static TDst round(const TSrc &val) // Round toward -inf
  {
    IsIntegerType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // int to int case
    IsIntegerType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    return (TDst)val;
  }
};

template <typename TDst, typename TSrc>
struct call_rtn_rounding<TDst, TSrc, I2F> {
  static TDst round(const TSrc &val) // Round toward -inf
  {
    IsIntegerType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // int to float case
    IsFloatType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    return (TDst)val;
  }
};

template <typename TDst, typename TSrc>
struct call_rtn_rounding<TDst, TSrc, F2I> {
  static TDst round(const TSrc &val) // Round toward -inf
  {
    IsFloatType<TSrc> _x;
    UNUSED_ARGUMENT(_x); // float to int case
    IsIntegerType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    TSrc fIntPart;
    TSrc fract = std::modf(val, &fIntPart);
    TDst intPart = (TDst)fIntPart;

    if (fract != (TSrc)0.0f) // round to nearest
    {
      intPart -= (TDst)1;
    }
    return intPart;
  }
};

// call_def_rounding
template <typename TDst, typename TSrc,
          convType cT = types2ConvType<TDst, TSrc>::Ty>
struct call_def_rounding; // def rounding

template <typename TDst, typename TSrc>
struct call_def_rounding<TDst, TSrc, I2I> {
  static TDst round(const TSrc &val) // def rounding
  {
    // to int case
    IsIntegerType<TSrc> _x;
    UNUSED_ARGUMENT(_x);
    IsIntegerType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    return call_rtz_rounding<TDst, TSrc>::round(val);
  }
};

template <typename TDst, typename TSrc>
struct call_def_rounding<TDst, TSrc, F2I> {
  static TDst round(const TSrc &val) // def rounding
  {
    // to int case
    IsFloatType<TSrc> _x;
    UNUSED_ARGUMENT(_x);
    IsIntegerType<TDst> _y;
    UNUSED_ARGUMENT(_y);

    return call_rtz_rounding<TDst, TSrc>::round(val);
  }
};

template <typename TDst, typename TSrc>
struct call_def_rounding<TDst, TSrc, F2F> {
  static TDst round(const TSrc &val) // def rounding
  {
    // to float case
    IsFloatType<TDst> _x;
    UNUSED_ARGUMENT(_x);
    IsFloatType<TSrc> _y;
    UNUSED_ARGUMENT(_y);

    return call_rte_rounding<TDst, TSrc>::round(val);
  }
};

template <typename TDst, typename TSrc>
struct call_def_rounding<TDst, TSrc, I2F> {
  static TDst round(const TSrc &val) // def rounding
  {
    // to float case
    IsFloatType<TDst> _x;
    UNUSED_ARGUMENT(_x);
    IsIntegerType<TSrc> _y;
    UNUSED_ARGUMENT(_y);

    return call_rte_rounding<TDst, TSrc>::round(val);
  }
};

template <typename TDst, typename TSrc, int n, bool saturate, int RMode>
llvm::GenericValue lle_X_convert(llvm::FunctionType *FT,
                                 llvm::ArrayRef<llvm::GenericValue> Args) {
  IsScalarType<TDst> _x;
  UNUSED_ARGUMENT(_x);
  IsScalarType<TSrc> _y;
  UNUSED_ARGUMENT(_y);
  llvm::GenericValue R;

  // if it is vector use AggregateVal
  if (n > 1)
    R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  for (uint32_t i = 0; i < n; ++i) {
    TSrc val = (TSrc)getVal<TSrc, n>(arg0, i);

    if (saturate)
      val = Saturate<TDst, TSrc>::sat(val);
    switch (RMode) {
    case 0:
      getRef<TDst, n>(R, i) =
          AsT<TDst>(call_def_rounding<TDst, TSrc>::round(val));
      break;
    case 1:
      getRef<TDst, n>(R, i) =
          AsT<TDst>(call_rte_rounding<TDst, TSrc>::round(val));
      break;
    case 2:
      getRef<TDst, n>(R, i) =
          AsT<TDst>(call_rtz_rounding<TDst, TSrc>::round(val));
      break;
    case 3:
      getRef<TDst, n>(R, i) =
          AsT<TDst>(call_rtp_rounding<TDst, TSrc>::round(val));
      break;
    case 4:
      getRef<TDst, n>(R, i) =
          AsT<TDst>(call_rtn_rounding<TDst, TSrc>::round(val));
      break;
    }
  }
  return R;
}

} // namespace OCLBuiltins
} // namespace Validation

#endif // BLT_CONVERSION_H
