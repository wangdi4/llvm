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

#ifndef BLT_MISCELLANEOUSVECTOR_H
#define BLT_MISCELLANEOUSVECTOR_H

#include "Helpers.h"
#include "RefALU.h"
#include "Utils.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/DerivedTypes.h"

namespace Validation {
namespace OCLBuiltins {

template <typename T1, typename T2, int n>
llvm::GenericValue lle_X_shuffle(llvm::FunctionType *FT,
                                 llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];

  R.AggregateVal.resize(n);

  int m = (int)arg0.AggregateVal.size();
  // For shuffle, only the ilogb(2m-1) least significant bits of each mask
  // element are considered. Other bits in the mask shall be ignored.

  unsigned mask = (1 << shuffleGetNumMaskBits(m)) - 1;

  for (uint32_t i = 0; i < n; i++) {
    unsigned j = (unsigned)getVal<T2, n>(arg1, i) & mask;
    getRef<T1, n>(R, i) = getRef<T1, n>(arg0, j);
  }

  return R;
}

template <typename T1, typename T2, int n>
llvm::GenericValue lle_X_shuffle2(llvm::FunctionType *FT,
                                  llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue R;
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue arg2 = Args[2];

  R.AggregateVal.resize(n);

  int m = (int)arg0.AggregateVal.size();
  assert(m == (int)arg1.AggregateVal.size() &&
         "both arg0 and arg1 vectors should have the same size");
  // For shuffle2, only the ilogb(2m-1)+1 least significant bits of each mask
  // element are considered. Other bits in the mask shall be ignored.

  unsigned mask = (1 << (shuffleGetNumMaskBits(m) + 1)) - 1;

  for (uint32_t i = 0; i < n; i++) {
    unsigned j = (unsigned)getVal<T2, n>(arg2, i) & mask;
    if (j < arg0.AggregateVal.size()) {
      getRef<T1, n>(R, i) = getRef<T1, n>(arg0, j);
    } else {
      getRef<T1, n>(R, i) = getRef<T1, n>(arg1, j - arg0.AggregateVal.size());
    }
  }

  return R;
}

} // namespace OCLBuiltins
} // namespace Validation

#endif // BLT_MISCELLANEOUSVECTOR_H
