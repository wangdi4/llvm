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

#ifndef BLT_ASYNC_COPIES_AND_PREFETCH_H
#define BLT_ASYNC_COPIES_AND_PREFETCH_H

#include "Helpers.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/DerivedTypes.h"
#include <RefALU.h>

namespace Validation {
namespace OCLBuiltins {

llvm::GenericValue lle_X_prefetch(llvm::FunctionType *FT,
                                  llvm::ArrayRef<llvm::GenericValue> Args);
llvm::GenericValue
lle_X_wait_group_events(llvm::FunctionType *FT,
                        llvm::ArrayRef<llvm::GenericValue> Args);

template <typename T, int n>
llvm::GenericValue
lle_X_async_work_group_copy(llvm::FunctionType *FT,
                            llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue arg2 = Args[2];
  llvm::GenericValue arg3 = Args[3];
  size_t num_gentypes = getVal<size_t>(arg2);
  size_t eventValue = getVal<size_t>(arg3);
  T *dst = static_cast<T *>(arg0.PointerVal);
  T *src = static_cast<T *>(arg1.PointerVal);
  std::copy(src, src + n * num_gentypes, dst);
  if (eventValue)
    R.IntVal = llvm::APInt(sizeof(size_t) * 8, eventValue);
  else
    R.IntVal = llvm::APInt(sizeof(size_t) * 8, 1);
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_async_work_group_strided_copy_l2g(
    llvm::FunctionType *FT, llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue arg2 = Args[2];
  llvm::GenericValue arg3 = Args[3];
  llvm::GenericValue arg4 = Args[4];
  size_t num_gentypes = getVal<size_t>(arg2);
  size_t dst_stride = getVal<size_t>(arg3);
  size_t eventValue = getVal<size_t>(arg4);
  T *dst = static_cast<T *>(arg0.PointerVal);
  T *src = static_cast<T *>(arg1.PointerVal);
  for (size_t i = 0; i < num_gentypes; ++i, dst += n * dst_stride, src += n) {
    std::copy(src, src + n, dst);
  }
  if (eventValue)
    R.IntVal = llvm::APInt(sizeof(size_t) * 8, eventValue);
  else
    R.IntVal = llvm::APInt(sizeof(size_t) * 8, 1);
  return R;
}

template <typename T, int n>
llvm::GenericValue lle_X_async_work_group_strided_copy_g2l(
    llvm::FunctionType *FT, llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue arg2 = Args[2];
  llvm::GenericValue arg3 = Args[3];
  llvm::GenericValue arg4 = Args[4];
  size_t num_gentypes = getVal<size_t>(arg2);
  size_t src_stride = getVal<size_t>(arg3);
  size_t eventValue = getVal<size_t>(arg4);
  T *dst = static_cast<T *>(arg0.PointerVal);
  T *src = static_cast<T *>(arg1.PointerVal);
  for (size_t i = 0; i < num_gentypes; ++i, src += n * src_stride, dst += n) {
    std::copy(src, src + n, dst);
  }
  if (eventValue)
    R.IntVal = llvm::APInt(sizeof(size_t) * 8, eventValue);
  else
    R.IntVal = llvm::APInt(sizeof(size_t) * 8, 1);
  return R;
}

} // namespace OCLBuiltins
} // namespace Validation
#endif // BLT_ASYNC_COPIES_AND_PREFETCH_H
