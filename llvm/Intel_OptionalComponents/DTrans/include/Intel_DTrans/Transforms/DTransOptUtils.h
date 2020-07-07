//===- DTransOptUtils.h - Common utility functions for DTrans transforms -===//
//
// Copyright (C) 2018-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares common utility functions that may be useful to one or
// more of the DTrans transformation passes.
//
//===----------------------------------------------------------------------===//

#if !INTEL_INCLUDE_DTRANS
#error DTransOptUtils.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_OPTIONALCOMPONENTS_INTEL_DTRANS_TRANSFORMS_DTRANSOPTUTILS_H
#define INTEL_OPTIONALCOMPONENTS_INTEL_DTRANS_TRANSFORMS_DTRANSOPTUTILS_H

#include "llvm/ADT/SetVector.h"
#include "llvm/IR/Instructions.h"

namespace llvm {
class TargetLibraryInfo;

namespace dtrans {
class CallInfo;

// Transformations may call this function to find and replace the
// input value to the specified instruction which is a multiple of the
// original operand size. This function uses the instruction type to
// determine which operand is expected to be a size operand and then
// searches the use-def chain of that operand (if necessary) to find
// a constant value which is a multiple of the alloc size of the original
// type and replaces it with the same constant multiple of the alloc size
// of the replacement type. If multiple possible values are found (such
// as in the case of a calloc instruction whose size and count arguments
// are both multiples of the original size) only one value will be
// replaced. If any value in the use-def chain between the instruction and
// the constant value that is updated has multiple uses, all instructions
// between the first instruction in the chain with multiple uses and the
// value being replaced will be cloned.
//
// Note: This function assumes that the calls involved are all processing
// the entire function. Optimizations which use this function should check
// the MemFuncPartialWrite safety condition.
void updateCallSizeOperand(llvm::Instruction *I, llvm::dtrans::CallInfo *CInfo,
                           llvm::Type *OrigTy, llvm::Type *ReplTy,
                           const llvm::TargetLibraryInfo &TLI);

// This is an overloaded version of the above function that allows the
// transformations to pass in an original structure size in the \p OrigSize
// parameter and a new structure size in the \p ReplSize parameter to use for
// replacing the size operand in the function call contained within \p CInfo.
void updateCallSizeOperand(llvm::Instruction *I, llvm::dtrans::CallInfo *CInfo,
                           uint64_t OrigSize, uint64_t ReplSize,
                           const llvm::TargetLibraryInfo &TLI);

// Given a pointer to a sub instruction that is known to subtract two
// pointers, find all users of the instruction that divide the result by
// a constant multiple of the original type and replace them with a divide
// by a constant that is the same multiple of the replacement type.
// This function requires that all uses of this instruction be either
// sdiv or udiv instructions.
void updatePtrSubDivUserSizeOperand(llvm::BinaryOperator *Sub,
                                    llvm::Type *OrigTy, llvm::Type *ReplTy,
                                    const DataLayout &DL);

// Given a pointer to a sub instruction that is known to subtract two
// pointers, find all users of the instruction that divide the result by
// a constant multiple of the \p OrigSize and replace them with a divide
// by a constant that is the same multiple of the \p ReplSize.
// This function requires that all uses of this instruction be either
// sdiv or udiv instructions.
void updatePtrSubDivUserSizeOperand(llvm::BinaryOperator *Sub,
                                    uint64_t OrigSize, uint64_t ReplSize);

// Transformations may use this function to find a constant input value,
// searching from the specified operand and following the use-def chain
// as necessary, which is a multiple of the specified size. If such a value
// is found, the function will return true and the \p UseStack vector will
// contain the stack of User-Index pairs in the use-def chain which led to
// the constant. Each entry in the stack represents an instruction and the
// index of the operand that was followed.
//
// If such a value is not found, the function will return false and the
// \p UseStack vector will not be changed.
bool findValueMultipleOfSizeInst(
    User *U, unsigned Idx, uint64_t Size,
    SmallVectorImpl<std::pair<User *, unsigned>> &UseStack);

// Returns 'true' if function, \p F, represents the program's main entry
// routine.
bool isMainFunction(Function &F);
} // namespace dtrans
} // namespace llvm

#endif // INTEL_OPTIONALCOMPONENTS_INTEL_DTRANS_TRANSFORMS_DTRANSOPTUTILS_H
