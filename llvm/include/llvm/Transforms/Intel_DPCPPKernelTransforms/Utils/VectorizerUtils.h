//===-- VectorizerUtils.h - Vectorizer utilities --------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_UTILS_VECTORIZER_UTILS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_UTILS_VECTORIZER_UTILS_H

#include "llvm/IR/Instructions.h"

namespace llvm {

namespace VectorizerUtils {

/// Generate type-conversion and place in given location
///  but on debug accpets only cases size(orig) >= size(target).
/// \param Orig Source value.
/// \param TargetType Target type to convert to.
/// \param InsertPoint instruction to insert before.
Instruction *extendValToType(Value *Orig, Type *TargetType,
                             Instruction *InsertPoint);

/// Check if both types are pointer to opaque types.
/// \param X type 1
/// \param Y type 2
bool isOpaquePtrPair(Type *X, Type *Y);

/// Follow thru a function input argument, until finding the root
//   (where its type matches the "expected" type).
/// \param Arg The actual argument of the call inst.
/// \param RootTy The desired (or "real") type to find.
/// \param CI The call instruction.
/// \return The root value if found, or NULL otherwise.
Value *rootInputArgument(Value *Arg, Type *RootTy, CallInst *CI);

/// Same as above, but derive the type from the mangled call name.
/// \param Arg The actual argument of the call inst.
/// \param ParamNum The argument number in the call.
/// \param CI The call instruction.
/// \return The root value if found, or NULL otherwise.
Value *rootInputArgumentBySignature(Value *Arg, unsigned ParamNum,
                                    CallInst *CI);

/// Follow thru a function return argument, until its type matches the
/// "expected" type.
/// \param RetVal The actual returned value of the call inst.
/// \param RootTy The desired (or "real") type to find.
/// \param CI The call instruction.
/// \return The "proper" retval if found, or NULL otherwise.
Value *rootReturnValue(Value *RetVal, Type *RootTy, CallInst *CI);

} // namespace VectorizerUtils
} // namespace llvm

#endif
