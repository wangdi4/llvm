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

#ifndef BLT_WORK_GROUP_H
#define BLT_WORK_GROUP_H

#include "Helpers.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/DerivedTypes.h"
#include <map>
#include <string>

namespace Validation {
namespace OCLBuiltins {

llvm::GenericValue
lle_X_work_group_all_impl(llvm::FunctionType *FT,
                          llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_work_group_any_impl(llvm::FunctionType *FT,
                          llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_work_group_broadcast_impl(llvm::FunctionType *FT,
                                llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_work_group_reduce_add_impl(llvm::FunctionType *FT,
                                 llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_work_group_reduce_max_impl(llvm::FunctionType *FT,
                                 llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_work_group_reduce_min_impl(llvm::FunctionType *FT,
                                 llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_work_group_reduce_mul_impl(llvm::FunctionType *FT,
                                 llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue lle_X_work_group_reduce_bitwise_and_impl(
    llvm::FunctionType *FT, llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue lle_X_work_group_reduce_bitwise_or_impl(
    llvm::FunctionType *FT, llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue lle_X_work_group_reduce_bitwise_xor_impl(
    llvm::FunctionType *FT, llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue lle_X_work_group_reduce_logical_and_impl(
    llvm::FunctionType *FT, llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue lle_X_work_group_reduce_logical_or_impl(
    llvm::FunctionType *FT, llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue lle_X_work_group_reduce_logical_xor_impl(
    llvm::FunctionType *FT, llvm::ArrayRef<llvm::GenericValue> Args);
} // namespace OCLBuiltins
} // namespace Validation
#endif // BLT_WORK_GROUP_H
