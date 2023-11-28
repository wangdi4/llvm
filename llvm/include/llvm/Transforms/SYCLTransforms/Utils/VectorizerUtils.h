//===-- VectorizerUtils.h - Vectorizer utilities --------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_VECTORIZER_UTILS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_VECTORIZER_UTILS_H

#include "llvm/Analysis/VectorUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

namespace llvm {

class RuntimeService;

namespace VectorizerUtils {

/// Get the vector length of the vector variant.
/// \param V VFInfo to access
/// \return vector length of input vector variant.
unsigned getVFLength(const VFInfo &V);

/// Check whether it's vector.
/// \Param VFParam VFParameter* to check
/// \return bool whether it's vector
bool VFParamIsVector(const VFParameter &VFParam);

/// Check whether it's Uniform.
/// \Param VFParam VFParameter* to check
/// \return bool whether it's uniform
bool VFParamIsUniform(const VFParameter &VFParam);

/// Check whether it's masked
/// \param V VFInfo to access
/// \return bool whether it's masked
bool VFIsMasked(const VFInfo &V);

/// Create a broadcast sequence (insertelement + shufflevector).
/// \param V value to broadcast.
/// \param VectorWidth width of generated vector.
/// \param InsertBefore instruction to insert new vector before.
/// \return new broadcast vector.
Instruction *createBroadcast(Value *V, unsigned VectorWidth,
                             Instruction *InsertBefore);

/// Generate type-conversion and place in given location
///  but on debug accpets only cases size(orig) >= size(target).
/// \param Orig Source value.
/// \param TargetType Target type to convert to.
/// \param InsertPoint instruction to insert before.
Instruction *extendValToType(Value *Orig, Type *TargetType,
                             Instruction *InsertPoint);

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

/// @brief Returns true if the llvm intrinsic is safe to ignore
inline bool isSafeIntrinsic(Intrinsic::ID IntrinsicID) {
  switch (IntrinsicID) {
  case Intrinsic::lifetime_start:
  case Intrinsic::lifetime_end:
  case Intrinsic::var_annotation:
  case Intrinsic::ptr_annotation:
  case Intrinsic::invariant_start:
  case Intrinsic::invariant_end:
  case Intrinsic::dbg_label:
  case Intrinsic::dbg_declare:
  case Intrinsic::dbg_value:
  case Intrinsic::annotation:
  case Intrinsic::assume:
    return true;
  default:
    return false;
  }
}
} // namespace VectorizerUtils
} // namespace llvm

#endif
