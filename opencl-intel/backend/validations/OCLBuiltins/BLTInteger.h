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

#ifndef BLT_INTEGER_H
#define BLT_INTEGER_H

#include "Helpers.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/DerivedTypes.h"

namespace Validation {
namespace OCLBuiltins {

template <typename T> llvm::APInt ExtAPInt(llvm::APInt &a, unsigned width) {
  return a.zext(width);
}
template <> llvm::APInt ExtAPInt<int8_t>(llvm::APInt &a, unsigned width);
template <> llvm::APInt ExtAPInt<int16_t>(llvm::APInt &a, unsigned width);
template <> llvm::APInt ExtAPInt<int32_t>(llvm::APInt &a, unsigned width);
template <> llvm::APInt ExtAPInt<int64_t>(llvm::APInt &a, unsigned width);

template <typename T, int n>
llvm::GenericValue lle_X_hadd(llvm::FunctionType *FT,
                              llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (uint32_t i = 0; i < n; ++i) {
    llvm::APInt x = getRef<T, n>(arg0, i);
    if (x.getBitWidth() <= sizeof(T) * 8) {
      x = ExtAPInt<T>(x, (sizeof(T) + 1) * 8);
    }
    llvm::APInt y = getRef<T, n>(arg1, i);
    if (y.getBitWidth() <= sizeof(T) * 8) {
      y = ExtAPInt<T>(y, (sizeof(T) + 1) * 8);
    }
    llvm::APInt z = x + y;
    getRef<T, n>(R, i) = z.ashr(1).trunc(sizeof(T) * 8);
  }
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_rhadd(llvm::FunctionType *FT,
                               llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (uint32_t i = 0; i < n; ++i) {
    llvm::APInt x = getRef<T, n>(arg0, i);
    if (x.getBitWidth() <= sizeof(T) * 8) {
      x = ExtAPInt<T>(x, (sizeof(T) + 1) * 8);
    }
    llvm::APInt y = getRef<T, n>(arg1, i);
    if (y.getBitWidth() <= sizeof(T) * 8) {
      y = ExtAPInt<T>(y, (sizeof(T) + 1) * 8);
    }
    llvm::APInt z = x + y + 1;
    getRef<T, n>(R, i) = z.ashr(1).trunc(sizeof(T) * 8);
  }
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_abs(llvm::FunctionType *FT,
                             llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  for (uint32_t i = 0; i < n; ++i) {
    uint64_t x =
        (T)(getRef<T, n>(arg0, i).getLimitedValue()) <= 0
            ? (uint64_t)((T)(getRef<T, n>(arg0, i).getLimitedValue()) * -1)
            : (T)(getRef<T, n>(arg0, i).getLimitedValue());
    getRef<T, n>(R, i) = llvm::APInt(sizeof(T) * 8, x, false);
  }
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_abs_diff(llvm::FunctionType *FT,
                                  llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (uint32_t i = 0; i < n; ++i) {
    if (predLess<T>(getRef<T, n>(arg0, i), getRef<T, n>(arg1, i))) {
      getRef<T, n>(R, i) = getRef<T, n>(arg1, i) - getRef<T, n>(arg0, i);
    } else {
      getRef<T, n>(R, i) = getRef<T, n>(arg0, i) - getRef<T, n>(arg1, i);
    }
  }
  return R;
}

template <typename T, int n, int s>
llvm::GenericValue lle_X_maxi(llvm::FunctionType *FT,
                              llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) =
        std::max(getRef<T, n>(arg0, i), getRef<T, s>(arg1, i), predLess<T>);
  }
  return R;
}

template <typename T, int n, int s>
llvm::GenericValue lle_X_mini(llvm::FunctionType *FT,
                              llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) =
        std::min(getRef<T, n>(arg0, i), getRef<T, s>(arg1, i), predLess<T>);
  }
  return R;
}

template <typename T, int n, int s>
llvm::GenericValue lle_X_clampi(llvm::FunctionType *FT,
                                llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue arg2 = Args[2];
  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) = std::min(
        std::max(getRef<T, n>(arg0, i), getRef<T, s>(arg1, i), predLess<T>),
        getRef<T, s>(arg2, i), predLess<T>);
  }
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_add_sat(llvm::FunctionType *FT,
                                 llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (uint32_t i = 0; i < n; ++i) {
    llvm::APInt r = getRef<T, n>(arg0, i) + getRef<T, n>(arg1, i);
    if (predLess<T>(getRef<T, n>(arg1, i), llvm::APInt(sizeof(T) * 8, 0))) {
      if (predLess<T>(getRef<T, n>(arg0, i), r))
        r = llvm::APInt(sizeof(T) * 8, intMin<T>());
    } else {
      if (predLess<T>(r, getRef<T, n>(arg0, i)))
        r = llvm::APInt(sizeof(T) * 8, intMax<T>());
    }
    getRef<T, n>(R, i) = r;
  }
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_sub_sat(llvm::FunctionType *FT,
                                 llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (uint32_t i = 0; i < n; ++i) {
    llvm::APInt r = getRef<T, n>(arg0, i) - getRef<T, n>(arg1, i);
    if (predLess<T>(getRef<T, n>(arg1, i), llvm::APInt(sizeof(T) * 8, 0))) {
      if (predLess<T>(r, getRef<T, n>(arg0, i)))
        r = llvm::APInt(sizeof(T) * 8, intMax<T>());
    } else {
      if (predLess<T>(getRef<T, n>(arg0, i), r))
        r = llvm::APInt(sizeof(T) * 8, intMin<T>());
    }
    getRef<T, n>(R, i) = r;
  }
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_clz(llvm::FunctionType *FT,
                             llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) =
        llvm::APInt(sizeof(T) * 8, getRef<T, n>(arg0, i).countLeadingZeros(),
                    isSignedType<T>()); // clz(getVal<T,n>(arg0,i));
  }
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_ctz(llvm::FunctionType *FT,
                             llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) =
        llvm::APInt(sizeof(T) * 8, getRef<T, n>(arg0, i).countTrailingZeros(),
                    isSignedType<T>()); // ctz(getVal<T,n>(arg0,i));
  }
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_rotate(llvm::FunctionType *FT,
                                llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) = getRef<T, n>(arg0, i).rotl(
        getVal<T, n>(arg1, i) &
        (sizeof(T) * 8 - 1)); // clz(getVal<T,n>(arg0,i));
  }
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_upsample(llvm::FunctionType *FT,
                                  llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (uint32_t i = 0; i < n; ++i) {
    typedef typename superT<T>::type sT;
    typedef typename unsignedT<T>::type uT;
    sT res = getVal<uT, n>(arg1, i);
    res |= (sT)(getVal<T, n>(arg0, i)) << sizeof(T) * 8;

    getRef<T, n>(R, i) = llvm::APInt(sizeof(sT) * 8, res, isSignedType<T>());
  }
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_mul_hi(llvm::FunctionType *FT,
                                llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (uint32_t i = 0; i < n; ++i) {
    llvm::APInt x = isSignedType<T>()
                        ? getRef<T, n>(arg0, i).sext(sizeof(T) * 8 * 2)
                        : getRef<T, n>(arg0, i).zext(sizeof(T) * 8 * 2);
    llvm::APInt y = isSignedType<T>()
                        ? getRef<T, n>(arg1, i).sext(sizeof(T) * 8 * 2)
                        : getRef<T, n>(arg1, i).zext(sizeof(T) * 8 * 2);
    x *= y;
    getRef<T, n>(R, i) = x.ashr(sizeof(T) * 8).trunc(sizeof(T) * 8);
  }
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_mad_hi(llvm::FunctionType *FT,
                                llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue arg2 = Args[2];
  for (uint32_t i = 0; i < n; ++i) {
    llvm::APInt x = isSignedType<T>()
                        ? getRef<T, n>(arg0, i).sext(sizeof(T) * 8 * 2)
                        : getRef<T, n>(arg0, i).zext(sizeof(T) * 8 * 2);
    llvm::APInt y = isSignedType<T>()
                        ? getRef<T, n>(arg1, i).sext(sizeof(T) * 8 * 2)
                        : getRef<T, n>(arg1, i).zext(sizeof(T) * 8 * 2);
    x *= y;
    getRef<T, n>(R, i) =
        x.ashr(sizeof(T) * 8).trunc(sizeof(T) * 8) + getRef<T, n>(arg2, i);
  }
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_mad_sat(llvm::FunctionType *FT,
                                 llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue arg2 = Args[2];
  for (uint32_t i = 0; i < n; ++i) {
    llvm::APInt x = isSignedType<T>()
                        ? getRef<T, n>(arg0, i).sext((sizeof(T) * 8 + 1) * 2)
                        : getRef<T, n>(arg0, i).zext((sizeof(T) * 8 + 1) * 2);
    llvm::APInt y = isSignedType<T>()
                        ? getRef<T, n>(arg1, i).sext((sizeof(T) * 8 + 1) * 2)
                        : getRef<T, n>(arg1, i).zext((sizeof(T) * 8 + 1) * 2);
    llvm::APInt z = isSignedType<T>()
                        ? getRef<T, n>(arg2, i).sext((sizeof(T) * 8 + 1) * 2)
                        : getRef<T, n>(arg2, i).zext((sizeof(T) * 8 + 1) * 2);
    x = x * y + z;
    llvm::APInt clamped = std::min(
        std::max(x,
                 llvm::APInt((sizeof(T) * 8 + 1) * 2, intMin<T>(),
                             isSignedType<T>()),
                 predLess<T>),
        llvm::APInt((sizeof(T) * 8 + 1) * 2, intMax<T>(), isSignedType<T>()),
        predLess<T>);
    getRef<T, n>(R, i) = clamped.trunc(sizeof(T) * 8);
  }
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_mul24(llvm::FunctionType *FT,
                               llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (uint32_t i = 0; i < n; ++i) {
    T x = getVal<T, n>(arg0, i);
    x = (x << 8) >> 8;
    T y = getVal<T, n>(arg1, i);
    y = (y << 8) >> 8;
    T z = x * y;
    getRef<T, n>(R, i) = llvm::APInt(sizeof(T) * 8, z, isSignedType<T>());
  }
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_mad24(llvm::FunctionType *FT,
                               llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue arg2 = Args[2];
  for (uint32_t i = 0; i < n; ++i) {
    T x = getVal<T, n>(arg0, i);
    x = (x << 8) >> 8;
    T y = getVal<T, n>(arg1, i);
    y = (y << 8) >> 8;
    T z = x * y + getVal<T, n>(arg2, i);
    getRef<T, n>(R, i) = llvm::APInt(sizeof(T) * 8, z, isSignedType<T>());
  }
  return R;
}

// we allways zero extend to unsinged long, thus
static int populationCount(unsigned long long x) {
  int ret = 0;
  const unsigned long long mask = 1;
  while (x) {
    if (mask & x)
      ++ret;
    x = x >> 1;
  }
  return ret;
}

template <typename T, int n>
llvm::GenericValue lle_X_popcount(llvm::FunctionType *FT,
                                  llvm::ArrayRef<llvm::GenericValue> Args) {
  assert(1 == Args.size() && "popcount should receives exactly one argument");
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue ret;
  ret.AggregateVal.resize(n);
  for (int i = 0; i < n; i++) {
    typename Validation::OCLBuiltins::unsignedT<T>::type current;
    current =
        (typename Validation::OCLBuiltins::unsignedT<T>::type)getVal<T, n>(arg0,
                                                                           i);
    int numBits = populationCount(current);
    getRef<T, n>(ret, i) =
        llvm::APInt(sizeof(T) * 8, numBits, isSignedType<T>());
  }
  return ret;
}

} // namespace OCLBuiltins
} // namespace Validation

#endif // BLT_INTEGER_H
