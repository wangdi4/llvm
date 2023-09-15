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

#ifndef BLT_WORK_ITEM_H
#define BLT_WORK_ITEM_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/DerivedTypes.h"
#include <map>
#include <string>

namespace Validation {
namespace OCLBuiltins {

llvm::GenericValue
lle_X_get_work_dim_impl(llvm::FunctionType *FT,
                        llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_get_global_size_impl(llvm::FunctionType *FT,
                           llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_get_global_id_impl(llvm::FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_get_local_size_impl(llvm::FunctionType *FT,
                          llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_get_local_id_impl(llvm::FunctionType *FT,
                        llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_get_num_groups_impl(llvm::FunctionType *FT,
                          llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_get_group_id_impl(llvm::FunctionType *FT,
                        llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_get_global_offset_impl(llvm::FunctionType *FT,
                             llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_get_global_linear_id_impl(llvm::FunctionType *FT,
                                llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_get_local_linear_id_impl(llvm::FunctionType *FT,
                               llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_get_enqueued_local_size_impl(llvm::FunctionType *FT,
                                   llvm::ArrayRef<llvm::GenericValue> Args);

} // namespace OCLBuiltins
} // namespace Validation
#endif // BLT_WORK_ITEM_H
