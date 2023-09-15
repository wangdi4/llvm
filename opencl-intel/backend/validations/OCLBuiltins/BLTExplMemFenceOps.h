// INTEL CONFIDENTIAL
//
// Copyright 2013 Intel Corporation.
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

#ifndef BLT_EXPL_MEM_FENCE_H
#define BLT_EXPL_MEM_FENCE_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/DerivedTypes.h"
#include <map>
#include <string>

namespace Validation {
namespace OCLBuiltins {

llvm::GenericValue
lle_X_mem_fence_impl(llvm::FunctionType *FT,
                     llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_read_mem_fence_impl(llvm::FunctionType *FT,
                          llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_write_mem_fence_impl(llvm::FunctionType *FT,
                           llvm::ArrayRef<llvm::GenericValue> Args);

} // namespace OCLBuiltins
} // namespace Validation
#endif // BLT_EXPL_MEM_FENCE_H
