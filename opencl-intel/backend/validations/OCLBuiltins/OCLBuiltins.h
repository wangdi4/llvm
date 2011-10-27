/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  OCLBuiltins.h

\*****************************************************************************/

#ifndef OCL_BUILTINS_H
#define OCL_BUILTINS_H
#include <map>
#include <string>
#include <vector>
#include <llvm/DerivedTypes.h>
#include <llvm/ExecutionEngine/GenericValue.h>

namespace Validation {
namespace OCLBuiltins {

    void FillOCLBuiltins (std::map<std::string, llvm::GenericValue (*)(const llvm::FunctionType *, const std::vector<llvm::GenericValue> &)>& funcNames);

} // namespace OCLBuiltins
} // namespace Validation

#endif // OCL_BUILTINS_H
