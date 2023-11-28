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

#ifndef BLT_RELATIONAL_H
#define BLT_RELATIONAL_H

#include "Helpers.h"
#include "RefALU.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/DerivedTypes.h"

namespace Validation {
namespace OCLBuiltins {

template <typename T>
llvm::GenericValue lle_X_isinf(llvm::FunctionType *FT,
                               llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  getRef<int32_t>(R) = llvm::APInt(32, RefALU::isInf<T>(getVal<T>(arg0)), true);
  return R;
}
template <typename T, int32_t n>
llvm::GenericValue lle_X_isinf(llvm::FunctionType *FT,
                               llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  for (int32_t i = 0; i < n; ++i) {
    getRef<int32_t, n>(R, i) =
        -llvm::APInt(32, RefALU::isInf<T>(getVal<T, n>(arg0, i)), true);
  }
  return R;
}

template <typename T>
llvm::GenericValue lle_X_isnormal(llvm::FunctionType *FT,
                                  llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  getRef<int32_t>(R) =
      llvm::APInt(32, RefALU::isNormal<T>(getVal<T>(arg0)), true);
  return R;
}
template <typename T, int32_t n>
llvm::GenericValue lle_X_isnormal(llvm::FunctionType *FT,
                                  llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  for (int32_t i = 0; i < n; ++i) {
    getRef<int32_t, n>(R, i) =
        -llvm::APInt(32, RefALU::isNormal<T>(getVal<T, n>(arg0, i)), true);
  }
  return R;
}

template <typename T>
llvm::GenericValue lle_X_isnan(llvm::FunctionType *FT,
                               llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  getRef<int32_t>(R) = llvm::APInt(32, RefALU::isNan<T>(getVal<T>(arg0)), true);
  return R;
}
template <typename T, int32_t n>
llvm::GenericValue lle_X_isnan(llvm::FunctionType *FT,
                               llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  for (int32_t i = 0; i < n; ++i) {
    getRef<int32_t, n>(R, i) =
        -llvm::APInt(32, RefALU::isNan<T>(getVal<T, n>(arg0, i)), true);
  }
  return R;
}

template <typename T>
llvm::GenericValue
lle_X_islessgreater(llvm::FunctionType *FT,
                    llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  getRef<int32_t>(R) = llvm::APInt(32,
                                   ((getVal<T>(arg0) < getVal<T>(arg1)) ||
                                    (getVal<T>(arg0) > getVal<T>(arg1))),
                                   true);
  return R;
}
template <typename T, int32_t n>
llvm::GenericValue
lle_X_islessgreater(llvm::FunctionType *FT,
                    llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (int32_t i = 0; i < n; ++i) {
    getRef<int32_t, n>(R, i) =
        -llvm::APInt(32,
                     ((getVal<T, n>(arg0, i) < getVal<T, n>(arg1, i)) ||
                      (getVal<T, n>(arg0, i) > getVal<T, n>(arg1, i))),
                     true);
  }
  return R;
}

template <typename T>
llvm::GenericValue lle_X_isunordered(llvm::FunctionType *FT,
                                     llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  getRef<int32_t>(R) = llvm::APInt(
      32,
      (RefALU::isNan<T>(getVal<T>(arg0)) || RefALU::isNan<T>(getVal<T>(arg1))),
      true);
  return R;
}
template <typename T, int32_t n>
llvm::GenericValue lle_X_isunordered(llvm::FunctionType *FT,
                                     llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (int32_t i = 0; i < n; ++i) {
    getRef<int32_t, n>(R, i) =
        -llvm::APInt(32,
                     (RefALU::isNan<T>(getVal<T, n>(arg0, i)) ||
                      RefALU::isNan<T>(getVal<T, n>(arg1, i))),
                     true);
  }
  return R;
}

template <typename T>
llvm::GenericValue lle_X_isordered(llvm::FunctionType *FT,
                                   llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  getRef<int32_t>(R) = llvm::APInt(32,
                                   (getVal<T>(arg0) == getVal<T>(arg0) &&
                                    getVal<T>(arg1) == getVal<T>(arg1)),
                                   true);
  return R;
}
template <typename T, int32_t n>
llvm::GenericValue lle_X_isordered(llvm::FunctionType *FT,
                                   llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (int32_t i = 0; i < n; ++i) {
    getRef<int32_t, n>(R, i) =
        -llvm::APInt(32,
                     (getVal<T, n>(arg0, i) == getVal<T, n>(arg0, i) &&
                      getVal<T, n>(arg1, i) == getVal<T, n>(arg1, i)),
                     true);
  }
  return R;
}

template <typename T>
llvm::GenericValue lle_X_isfinite(llvm::FunctionType *FT,
                                  llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  getRef<int32_t>(R) = llvm::APInt(32,
                                   (!RefALU::isInf<T>(getVal<T>(arg0)) &&
                                    !RefALU::isNan<T>(getVal<T>(arg0))),
                                   true);
  return R;
}
template <typename T, int32_t n>
llvm::GenericValue lle_X_isfinite(llvm::FunctionType *FT,
                                  llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  for (int32_t i = 0; i < n; ++i) {
    getRef<int32_t, n>(R, i) =
        llvm::APInt(32,
                    -(!RefALU::isInf<T>(getVal<T, n>(arg0, i)) &&
                      !RefALU::isNan<T>(getVal<T, n>(arg0, i))),
                    true);
  }
  return R;
}

template <typename T>
llvm::GenericValue lle_X_signbit(llvm::FunctionType *FT,
                                 llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  getRef<int32_t>(R) =
      llvm::APInt(32, RefALU::signbit<T>(getVal<T>(arg0)), true);
  return R;
}
template <typename T, int32_t n>
llvm::GenericValue lle_X_signbit(llvm::FunctionType *FT,
                                 llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  for (int32_t i = 0; i < n; ++i) {
    getRef<int32_t, n>(R, i) =
        llvm::APInt(32, (-RefALU::signbit<T>(getVal<T, n>(arg0, i))), true);
  }
  return R;
}

template <typename T>
llvm::GenericValue lle_X_any(llvm::FunctionType *FT,
                             llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  T sum = 0;
  sum |= getVal<T>(arg0);
  sum = (sum != 0) ? 1 : 0;
  getRef<int32_t>(R) = llvm::APInt(32, sum != 0, true);
  return R;
}
template <typename T, int32_t n>
llvm::GenericValue lle_X_any(llvm::FunctionType *FT,
                             llvm::ArrayRef<llvm::GenericValue> Args) {
  typedef typename signedT<T>::type sT;
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  T sum = 0;
  T mask = intMin<sT>(); // we need 1 in highest bit here
  for (int32_t i = 0; i < n; ++i) {
    sum |= getVal<T, n>(arg0, i) & mask;
  }

  getRef<int32_t>(R) = llvm::APInt(32, (sum != 0) ? 1 : 0, true);

  return R;
}

template <typename T>
llvm::GenericValue lle_X_all(llvm::FunctionType *FT,
                             llvm::ArrayRef<llvm::GenericValue> Args) {
  typedef typename signedT<T>::type sT;
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  T mask = intMin<sT>();
  T sum = mask; // we need 1 in highest bit here
  sum &= getVal<T>(arg0);
  sum = (sum != 0) ? 1 : 0;
  getRef<int32_t>(R) = llvm::APInt(32, sum != 0, true);
  return R;
}
template <typename T, int32_t n>
llvm::GenericValue lle_X_all(llvm::FunctionType *FT,
                             llvm::ArrayRef<llvm::GenericValue> Args) {
  typedef typename signedT<T>::type sT;
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  T mask = intMin<sT>(); // we need 1 in highest bit here
  T sum = mask;
  for (int32_t i = 0; i < n; ++i) {
    sum &= getVal<T, n>(arg0, i) & mask;
  }

  getRef<int32_t>(R) = llvm::APInt(32, (sum != 0) ? 1 : 0, true);

  return R;
}

template <typename T> llvm::GenericValue localBitselect(T inA, T inB, T inC) {
  llvm::GenericValue R;
  T out = (inA & ~inC) | (inB & inC);
  getRef<T>(R) = llvm::APInt(sizeof(T) * 8, out, isSignedType<T>());
  return R;
}

template <typename T>
llvm::GenericValue lle_X_bitselect(llvm::FunctionType *FT,
                                   llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue arg2 = Args[2];

  R = localBitselect<T>(getVal<T>(arg0), getVal<T>(arg1), getVal<T>(arg2));
  return R;
}
template <typename T, int32_t n>
llvm::GenericValue lle_X_bitselect(llvm::FunctionType *FT,
                                   llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue arg2 = Args[2];

  for (int32_t i = 0; i < n; ++i) {
    R.AggregateVal[i] = localBitselect<T>(
        getVal<T, n>(arg0, i), getVal<T, n>(arg1, i), getVal<T, n>(arg2, i));
  }

  return R;
}

template <typename T> llvm::GenericValue selectResult(T inC) {
  llvm::GenericValue R;
  getRef<T>(R) = llvm::APInt(sizeof(T) * 8, inC, isSignedType<T>());
  return R;
}
template <typename T, typename C>
llvm::GenericValue lle_X_select(llvm::FunctionType *FT,
                                llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue arg2 = Args[2];

  T out = getVal<C>(arg2)
              ? getVal<T>(arg1)
              : getVal<T>(arg0); // For a scalar type, result = c ? b : a.

  R = selectResult<T>(out);
  return R;
}
template <typename T, typename C, int32_t n>
llvm::GenericValue lle_X_select(llvm::FunctionType *FT,
                                llvm::ArrayRef<llvm::GenericValue> Args) {
  typedef typename signedT<C>::type sC;
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue arg2 = Args[2];

  // For each component of a vector type,
  // result[i] = if MSB of c[i] is set ? b[i] : a[i].
  for (int32_t i = 0; i < n; ++i) {
    bool cond = (getVal<C, n>(arg2, i) & intMin<sC>());
    T out = cond ? getVal<T, n>(arg1, i) : getVal<T, n>(arg0, i);
    R.AggregateVal[i] = selectResult<T>(out);
  }
  return R;
}

} // namespace OCLBuiltins
} // namespace Validation

#endif // BLT_RELATIONAL_H
