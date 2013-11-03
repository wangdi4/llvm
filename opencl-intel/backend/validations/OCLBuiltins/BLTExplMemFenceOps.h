/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  BLTExplMemFenceOps.h

\*****************************************************************************/
#ifndef BLT_EXPL_MEM_FENCE_H
#define BLT_EXPL_MEM_FENCE_H

#include <map>
#include <string>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/ExecutionEngine/GenericValue.h>

namespace Validation {
namespace OCLBuiltins {

llvm::GenericValue lle_X_mem_fence_impl(llvm::FunctionType *FT,
                         const std::vector<llvm::GenericValue> &Args);

llvm::GenericValue lle_X_read_mem_fence_impl(llvm::FunctionType *FT,
                         const std::vector<llvm::GenericValue> &Args);

llvm::GenericValue lle_X_write_mem_fence_impl(llvm::FunctionType *FT,
                         const std::vector<llvm::GenericValue> &Args);


} // namespace OCLBuiltins
} // namespace Validation
#endif // BLT_EXPL_MEM_FENCE_H
