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

#ifndef BLT_MATH_H
#define BLT_MATH_H

#include "Helpers.h"
#include "RefALU.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/DerivedTypes.h"

namespace Validation {
namespace OCLBuiltins {

DEFINE_BLT_ONE_ARG(sqrt)
DEFINE_BLT_ONE_ARG(rsqrt)

DEFINE_BLT_ONE_ARG(cos)
DEFINE_BLT_ONE_ARG(cosh)
DEFINE_BLT_ONE_ARG(cospi)
DEFINE_BLT_ONE_ARG(sin)
DEFINE_BLT_ONE_ARG(sinh)
DEFINE_BLT_ONE_ARG(sinpi)
DEFINE_BLT_ONE_ARG(tan)
DEFINE_BLT_ONE_ARG(tanh)
DEFINE_BLT_ONE_ARG(tanpi)
DEFINE_BLT_ONE_ARG(acos)
DEFINE_BLT_ONE_ARG(acospi)
DEFINE_BLT_ONE_ARG(asin)
DEFINE_BLT_ONE_ARG(asinpi)
DEFINE_BLT_ONE_ARG(atan)
DEFINE_BLT_ONE_ARG(atanpi)
DEFINE_BLT_ONE_ARG(exp)
DEFINE_BLT_ONE_ARG(exp2)
DEFINE_BLT_ONE_ARG(exp10)
DEFINE_BLT_ONE_ARG(expm1)
DEFINE_BLT_ONE_ARG(log)
DEFINE_BLT_ONE_ARG(log2)
DEFINE_BLT_ONE_ARG(log10)
DEFINE_BLT_ONE_ARG(log1p)
DEFINE_BLT_ONE_ARG(logb)
DEFINE_BLT_ONE_ARG(ceil)
DEFINE_BLT_ONE_ARG(fabs)
DEFINE_BLT_ONE_ARG(floor)
DEFINE_BLT_ONE_ARG(asinh)
DEFINE_BLT_ONE_ARG(acosh)
DEFINE_BLT_ONE_ARG(atanh)
DEFINE_BLT_ONE_ARG(cbrt)

DEFINE_BLT_ONE_ARG(rint)
DEFINE_BLT_ONE_ARG(round)
DEFINE_BLT_ONE_ARG(trunc)
DEFINE_BLT_ONE_ARG(lgamma)

DEFINE_BLT_TWO_ARGS(hypot)
DEFINE_BLT_TWO_ARGS(atan2)
DEFINE_BLT_TWO_ARGS(atan2pi)
DEFINE_BLT_TWO_ARGS(pow)
DEFINE_BLT_TWO_ARGS(powr)
DEFINE_BLT_TWO_ARGS(nextafter)
DEFINE_BLT_TWO_ARGS(div)
DEFINE_BLT_TWO_ARGS(copysign)
DEFINE_BLT_TWO_ARGS(fdim)
DEFINE_BLT_TWO_ARGS(fmod)
DEFINE_BLT_TWO_ARGS(remainder)

DEFINE_BLT_TWO_ARGS(maxmag)
DEFINE_BLT_TWO_ARGS(minmag)

DEFINE_BLT_THREE_ARGS(fma)
DEFINE_BLT_THREE_ARGS(mad)

template <typename F, typename I, int32_t n>
llvm::GenericValue lle_X_nan(llvm::FunctionType *FT,
                             llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  for (int32_t i = 0; i < n; ++i) {
    getRef<F, n>(R, i) = RefALU::nan(getVal<I, n>(arg0, i));
  }
  return R;
}

template <typename T, int32_t n>
llvm::GenericValue lle_X_pown(llvm::FunctionType *FT,
                              llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (int32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) =
        RefALU::pown<T>(getVal<T, n>(arg0, i), getVal<int32_t, n>(arg1, i));
  }
  return R;
}

template <typename T, int32_t n>
llvm::GenericValue lle_X_ilogb(llvm::FunctionType *FT,
                               llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  for (int32_t i = 0; i < n; ++i) {
    getRef<int32_t, n>(R, i) =
        llvm::APInt(32, RefALU::ilogb<T>(getVal<T, n>(arg0, i)), true);
  }
  return R;
}

template <typename T, int32_t n>
llvm::GenericValue lle_X_rootn(llvm::FunctionType *FT,
                               llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (int32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) =
        RefALU::rootn<T>(getVal<T, n>(arg0, i), getVal<int32_t, n>(arg1, i));
  }
  return R;
}

// TODO: to check this functions with n != nj after the issue is resolved (OCL
// backend can't build program with external function
// "gentype ldexp (gentype x, int n)")
template <typename T, int32_t n, int32_t nj>
llvm::GenericValue lle_X_ldexp(llvm::FunctionType *FT,
                               llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (int32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) =
        RefALU::ldexp<T>(getVal<T, n>(arg0, i), getVal<int32_t, nj>(arg1, i));
  }
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_modf(llvm::FunctionType *FT,
                              llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  T *p = static_cast<T *>(arg1.PointerVal);
  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) = RefALU::modf<T>(getVal<T, n>(arg0, i), p);
    p++;
  }

  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_frexp(llvm::FunctionType *FT,
                               llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  int32_t *p = static_cast<int32_t *>(arg1.PointerVal);

  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) = RefALU::frexp<T>(getVal<T, n>(arg0, i), p);
    p++;
  }

  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_sincos(llvm::FunctionType *FT,
                                llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  T *p = static_cast<T *>(arg1.PointerVal);

  for (uint32_t i = 0; i < n; ++i, ++p) {
    getRef<T, n>(R, i) = RefALU::sin<T>(getVal<T, n>(arg0, i));
    *p = RefALU::cos<T>(getVal<T, n>(arg0, i));
  }

  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_recip(llvm::FunctionType *FT,
                               llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];

  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) = RefALU::div<T>(T(1), getVal<T, n>(arg0, i));
  }

  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_fract(llvm::FunctionType *FT,
                               llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  T *p = static_cast<T *>(arg1.PointerVal);

  for (uint32_t i = 0; i < n; ++i, ++p) {
    T fl = RefALU::floor<T>(getVal<T, n>(arg0, i));
    getRef<T, n>(R, i) =
        RefALU::min<T>(getVal<T, n>(arg0, i) - fl, getOneMinus1ULP<T>());
    *p = fl;
  }

  return R;
}

template <typename T, int32_t n, int32_t nj>
llvm::GenericValue lle_X_fmax(llvm::FunctionType *FT,
                              llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (int32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) =
        RefALU::fmax<T>(getVal<T, n>(arg0, i), getVal<T, nj>(arg1, i));
  }
  return R;
}

template <typename T, int32_t n, int32_t nj>
llvm::GenericValue lle_X_fmin(llvm::FunctionType *FT,
                              llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (int32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) =
        RefALU::fmin<T>(getVal<T, n>(arg0, i), getVal<T, nj>(arg1, i));
  }
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_lgamma_r(llvm::FunctionType *FT,
                                  llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  int32_t *p = static_cast<int32_t *>(arg1.PointerVal);

  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) = RefALU::lgamma_r<T>(getVal<T, n>(arg0, i), p);
    p++;
  }

  return R;
}

template <typename T, int32_t n>
llvm::GenericValue lle_X_remquo(llvm::FunctionType *FT,
                                llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue arg2 = Args[2];
  int32_t *p = static_cast<int32_t *>(arg2.PointerVal);

  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) =
        RefALU::remquo<T>(getVal<T, n>(arg0, i), getVal<T, n>(arg1, i), p);
    p++;
  }

  return R;
}

} // namespace OCLBuiltins
} // namespace Validation

#endif // BLT_MATH_H
