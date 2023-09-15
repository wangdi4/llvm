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

#ifndef BLT_GEOMETRIC_H
#define BLT_GEOMETRIC_H

#include "Helpers.h"
#include "RefALU.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/DerivedTypes.h"

namespace Validation {
namespace OCLBuiltins {

template <typename T, int n>
llvm::GenericValue lle_X_dot(llvm::FunctionType *FT,
                             llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];

  T a[n], b[n];
  for (uint32_t i = 0; i < n; ++i) {
    a[i] = getVal<T, n>(arg0, i);
    b[i] = getVal<T, n>(arg1, i);
  }

  T dot = RefALU::dot<T>(a, b, n);

  getRef<T>(R) = dot;
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_normalize(llvm::FunctionType *FT,
                                   llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  T a[n], out[n];
  for (uint32_t i = 0; i < n; ++i)
    a[i] = getVal<T, n>(arg0, i);

  RefALU::normalize(a, out, n);

  for (uint32_t i = 0; i < n; ++i)
    getRef<T, n>(R, i) = out[i];

  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_length(llvm::FunctionType *FT,
                                llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  T a[n];
  for (uint32_t i = 0; i < n; ++i)
    a[i] = getVal<T, n>(arg0, i);

  getRef<T>(R) = RefALU::length(a, n);
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_distance(llvm::FunctionType *FT,
                                  llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  T a[n], b[n];
  for (uint32_t i = 0; i < n; ++i) {
    a[i] = getVal<T, n>(arg0, i);
    b[i] = getVal<T, n>(arg1, i);
  }
  getRef<T>(R) = RefALU::distance(a, b, n);
  return R;
}

template <typename T, uint32_t n>
llvm::GenericValue lle_X_cross(llvm::FunctionType *FT,
                               llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  R.AggregateVal.resize(n);
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];

  // outVector[ 0 ] = ( vecA[ 1 ] * vecB[ 2 ] ) - ( vecA[ 2 ] * vecB[ 1 ] );
  // outVector[ 1 ] = ( vecA[ 2 ] * vecB[ 0 ] ) - ( vecA[ 0 ] * vecB[ 2 ] );
  // outVector[ 2 ] = ( vecA[ 0 ] * vecB[ 1 ] ) - ( vecA[ 1 ] * vecB[ 0 ] );
  T a[n], b[n], out[n];

  for (uint32_t i = 0; i < n; ++i) {
    a[i] = getVal<T, n>(arg0, i);
    b[i] = getVal<T, n>(arg1, i);
  }

  RefALU::cross(a, b, out, n);

  for (uint32_t i = 0; i < n; ++i) {
    getRef<T, n>(R, i) = out[i];
  }

  return R;
}

} // namespace OCLBuiltins
} // namespace Validation

#endif // BLT_GEOMETRIC_H
