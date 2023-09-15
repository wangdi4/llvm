// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#ifndef BLT_ATOMIC_H
#define BLT_ATOMIC_H

#include "Helpers.h"
#include "RefALU.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/DerivedTypes.h"

namespace Validation {
namespace OCLBuiltins {

template <typename T>
llvm::GenericValue lle_X_atomic_add(llvm::FunctionType *FT,
                                    llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  T *p = static_cast<T *>(arg0.PointerVal);

  T old = *p;
  *p = old + getVal<T>(arg1);

  getRef<T>(R) = llvm::APInt(sizeof(T) * 8, old);

  return R;
}

template <typename T>
llvm::GenericValue lle_X_atomic_sub(llvm::FunctionType *FT,
                                    llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  T *p = static_cast<T *>(arg0.PointerVal);

  T old = *p;
  *p = old - getVal<T>(arg1);

  getRef<T>(R) = llvm::APInt(sizeof(T) * 8, old);

  return R;
}

template <typename T>
llvm::GenericValue lle_X_atomic_xchg(llvm::FunctionType *FT,
                                     llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  T *p = static_cast<T *>(arg0.PointerVal);

  T old = *p;
  *p = getVal<T>(arg1);

  getRef<T>(R) = llvm::APInt(sizeof(T) * 8, old);

  return R;
}

template <typename T>
llvm::GenericValue lle_X_atomic_inc(llvm::FunctionType *FT,
                                    llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  T *p = static_cast<T *>(arg0.PointerVal);

  T old = *p;
  *p = *p + 1;

  getRef<T>(R) = llvm::APInt(sizeof(T) * 8, old);

  return R;
}

template <typename T>
llvm::GenericValue lle_X_atomic_dec(llvm::FunctionType *FT,
                                    llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  T *p = static_cast<T *>(arg0.PointerVal);

  T old = *p;
  *p = *p - 1;

  getRef<T>(R) = llvm::APInt(sizeof(T) * 8, old);

  return R;
}

template <typename T>
llvm::GenericValue
lle_X_atomic_cmpxchg(llvm::FunctionType *FT,
                     llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue arg2 = Args[2];
  T *p = static_cast<T *>(arg0.PointerVal);

  T old = *p;
  *p = (old == getVal<T>(arg1)) ? getVal<T>(arg2) : old;

  getRef<T>(R) = llvm::APInt(sizeof(T) * 8, old);

  return R;
}

template <typename T>
llvm::GenericValue lle_X_atomic_min(llvm::FunctionType *FT,
                                    llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  T *p = static_cast<T *>(arg0.PointerVal);

  T old = *p;
  *p = std::min(old, getVal<T>(arg1));

  getRef<T>(R) = llvm::APInt(sizeof(T) * 8, old);

  return R;
}

template <typename T>
llvm::GenericValue lle_X_atomic_max(llvm::FunctionType *FT,
                                    llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  T *p = static_cast<T *>(arg0.PointerVal);

  T old = *p;
  *p = std::max(old, getVal<T>(arg1));

  getRef<T>(R) = llvm::APInt(sizeof(T) * 8, old);

  return R;
}

template <typename T>
llvm::GenericValue lle_X_atomic_and(llvm::FunctionType *FT,
                                    llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  T *p = static_cast<T *>(arg0.PointerVal);

  T old = *p;
  *p = old & getVal<T>(arg1);

  getRef<T>(R) = llvm::APInt(sizeof(T) * 8, old);

  return R;
}

template <typename T>
llvm::GenericValue lle_X_atomic_or(llvm::FunctionType *FT,
                                   llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  T *p = static_cast<T *>(arg0.PointerVal);

  T old = *p;
  *p = old | getVal<T>(arg1);

  getRef<T>(R) = llvm::APInt(sizeof(T) * 8, old);

  return R;
}

template <typename T>
llvm::GenericValue lle_X_atomic_xor(llvm::FunctionType *FT,
                                    llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  T *p = static_cast<T *>(arg0.PointerVal);

  T old = *p;
  *p = old ^ getVal<T>(arg1);

  getRef<T>(R) = llvm::APInt(sizeof(T) * 8, old);

  return R;
}

} // namespace OCLBuiltins
} // namespace Validation

#endif // BLT_ATOMIC_H
