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

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"

namespace llvm {

class RuntimeService;

namespace VectorizerUtils {

class CanVectorize {
public:
  /// Checks whether we can (as opposed to should) vectorize this function
  /// for VPO. \param F Function to check. \param UnsupportedFuncs
  /// Unsupported function obtained from getNonInlineUnsupportedFunctions.
  /// \param RTService Runtime service.
  /// \param EnableDirectCallVectorization Whether to enable direct function
  /// call vectorization.
  /// \param EnableSGDirectCallVectorization Whether to enable direct
  /// subgroup function call vectorization. \returns true if the function
  /// can be vectorized.
  static bool
  canVectorizeForVPO(Function &F, RuntimeService *RTService,
                     DPCPPKernelCompilationUtils::FuncSet &UnsupportedFuncs,
                     bool EnableDirectCallVectorization = false,
                     bool EnableSGDirectCallVectorization = false);

  // Check if the function has variable access to get_global/loval_id(X)
  static bool hasVariableGetTIDAccess(Function &F, RuntimeService *RTService);

  // Get unsupported function in a module.
  // An unsupported function is function that contains
  // barrier/get_local_id/get_global_id or a call to unsupported function.
  // Vectorize of kernel that calls non-inline function is done today by
  // calling the scalar version of called function VecWidth times.
  // This means that the implict arguments sent to the vectorized kernel
  // that is respone for calculating the barrier/get_local_id/get_global_id
  // cannot passed as is to the scalar function, what make it too difficult
  // to support these cases.
  static DPCPPKernelCompilationUtils::FuncSet
  getNonInlineUnsupportedFunctions(Module &M);
};

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

/// @brief Returns true if the llvm intrinsic is safe to ignore
inline bool isSafeIntrinsic(Intrinsic::ID IntrinsicID) {
  switch (IntrinsicID) {
  case Intrinsic::lifetime_start:
  case Intrinsic::lifetime_end:
  case Intrinsic::var_annotation:
  case Intrinsic::ptr_annotation:
  case Intrinsic::invariant_start:
  case Intrinsic::invariant_end:
  case Intrinsic::dbg_addr:
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
