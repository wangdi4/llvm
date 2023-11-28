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

#ifndef BLT_COMMON_H
#define BLT_COMMON_H

#include "Helpers.h"
#include "RefALU.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/DerivedTypes.h"

namespace Validation {
namespace OCLBuiltins {

template <typename T, int n, int s>
llvm::GenericValue lle_X_max(llvm::FunctionType *FT,
                             llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) =
        RefALU::max(getVal<T, n>(arg0, i), getVal<T, s>(arg1, i));
  }
  return R;
}

template <typename T, int n, int s>
llvm::GenericValue lle_X_min(llvm::FunctionType *FT,
                             llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) =
        RefALU::min(getRef<T, n>(arg0, i), getRef<T, s>(arg1, i));
  }
  return R;
}

template <typename T, int n, int s>
llvm::GenericValue lle_X_mix(llvm::FunctionType *FT,
                             llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue arg2 = Args[2];
  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) =
        getVal<T, n>(arg0, i) +
        (getVal<T, n>(arg1, i) - getVal<T, n>(arg0, i)) * getVal<T, s>(arg2, i);
  }
  return R;
}

template <typename T, int s, int n>
llvm::GenericValue lle_X_step(llvm::FunctionType *FT,
                              llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) =
        RefALU::step(getRef<T, s>(arg0, i), getRef<T, n>(arg1, i));
  }
  return R;
}

template <typename T, int s, int n>
llvm::GenericValue lle_X_smoothstep(llvm::FunctionType *FT,
                                    llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue arg2 = Args[2];
  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) = RefALU::smoothstep(
        getRef<T, s>(arg0, i), getRef<T, s>(arg1, i), getRef<T, n>(arg2, i));
  }
  return R;
}

template <typename T, int n, int s>
llvm::GenericValue lle_X_clamp(llvm::FunctionType *FT,
                               llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue arg2 = Args[2];
  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) =
        RefALU::min(RefALU::max(getVal<T, n>(arg0, i), getVal<T, s>(arg1, i)),
                    getVal<T, s>(arg2, i));
  }
  return R;
}

DEFINE_BLT_ONE_ARG(radians)
DEFINE_BLT_ONE_ARG(degrees)
DEFINE_BLT_ONE_ARG(sign)
} // namespace OCLBuiltins
} // namespace Validation
#endif // BLT_COMMON_H
