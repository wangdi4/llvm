/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  BLTWorkGroup.h

\*****************************************************************************/
#ifndef BLT_WORK_GROUP_H
#define BLT_WORK_GROUP_H

#include <map>
#include <string>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "Helpers.h"

namespace Validation {
namespace OCLBuiltins {

llvm::GenericValue lle_X_work_group_all_impl(llvm::FunctionType *FT,
                         const std::vector<llvm::GenericValue> &Args);

llvm::GenericValue lle_X_work_group_any_impl(llvm::FunctionType *FT,
                         const std::vector<llvm::GenericValue> &Args);

llvm::GenericValue lle_X_work_group_broadcast_impl(llvm::FunctionType *FT,
                         const std::vector<llvm::GenericValue> &Args);

llvm::GenericValue lle_X_work_group_reduce_add_impl(llvm::FunctionType *FT,
                         const std::vector<llvm::GenericValue> &Args);

llvm::GenericValue lle_X_work_group_reduce_max_impl(llvm::FunctionType *FT,
                         const std::vector<llvm::GenericValue> &Args);

llvm::GenericValue lle_X_work_group_reduce_min_impl(llvm::FunctionType *FT,
                         const std::vector<llvm::GenericValue> &Args);
} // namespace OCLBuiltins
} // namespace Validation
#endif // BLT_WORK_GROUP_H
