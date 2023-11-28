//==-------- LoopUtils.h - Loop utilities -----------------------*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_LOOP_UTILS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_LOOP_UTILS_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

namespace llvm {

class RuntimeService;

/// Struct that represent loop Region in the CFG.
struct LoopRegion {
  BasicBlock *PreHeader = nullptr; // Pre header block of the loop.
  BasicBlock *Header = nullptr;    // Header of the loop.
  BasicBlock *Exit = nullptr;      // Exit block of the loop.
};

namespace LoopUtils {

/// Creates loop with loopSize iterations arround the CFG region that
///       begins in head and finishes in latch.
/// \param Head The head of the created loop.
/// \param Latch The latch block of the created loop.
/// \param Begin Lower loop bound.
/// \param Increment Loop increment (1 or VF).
/// \param End Upper loop bound.
/// \param Pred Predicate for latch compare instruction.
/// \param DimPrefix Dimension prefix string.
/// \param C LLVM context.
/// \return a pair of LoopRegion and IV.
std::pair<LoopRegion, PHINode *> createLoop(BasicBlock *Head, BasicBlock *Latch,
                                            Value *Begin, Value *Increment,
                                            Value *End, CmpInst::Predicate Pred,
                                            std::string &DimPrefix,
                                            LLVMContext &C);

/// Create WI func (get_local_size, get_global_id etc) general
///       util as they all have the same signature.
/// M - module to add function to.
/// Name - name of the function.
/// RetTy - return type of the function.
Function *getWIFunc(Module *M, StringRef Name, Type *RetTy);

/// Creates work item call instruction.
/// M - current module.
/// FuncName - name of the function.
/// RetTy - return type of the function.
/// Dim - argument of the WI call.
/// BB - basic block to put the call at it's end.
/// CallName = name of the function call.
/// Returns call to the given Work item function.
CallInst *getWICall(Module *M, StringRef FuncName, Type *RetTy, Value *Dim,
                    BasicBlock *BB, const Twine &CallName = "");
CallInst *getWICall(Module *M, StringRef FuncName, Type *RetTy, Value *Dim,
                    Instruction *IP, const Twine &CallName = "");
CallInst *getWICall(Module *M, StringRef FuncName, Type *RetTy, Value *Dim,
                    IRBuilder<> &Builder, const Twine &CallName = "");
CallInst *getWICall(Module *M, StringRef FuncName, Type *RetTy, unsigned Dim,
                    BasicBlock *BB, const Twine &CallName = "");
CallInst *getWICall(Module *M, StringRef FuncName, Type *RetTy, unsigned Dim,
                    Instruction *IP, const Twine &CallName = "");
CallInst *getWICall(Module *M, StringRef FuncName, Type *RetTy, unsigned Dim,
                    IRBuilder<> &Builder, const Twine &CallName = "");

/// Returns size_t type.
/// M - current module.
Type *getIndTy(Module *M);

/// Fill atomic builtin user functions.
void fillAtomicBuiltinUsers(Module &M, RuntimeService &RTS,
                            CompilationUtils::FuncSet &UserFunc);

/// Fills the users function through call instructions of roots
///       (also indirect users) into userFuncs.
/// Roots - function to obtain their user functions.
/// UserFuncs - set to fill with users of roots
void fillFuncUsersSet(const CompilationUtils::FuncSet &Roots,
                      CompilationUtils::FuncSet &UserFuncs);

/// Fills direct user functions through instructions of functions in
///       funcs set into userFuncs. If a function is introduced into
///       userFuncs for the first time it will be inserted into newUsers.
/// Funcs - function to obtain direct users.
/// UserFuncs - set of users functions to fills.
/// NewUsers - set of newly found users.
void fillDirectUsers(const CompilationUtils::FuncSet *Funcs,
                     CompilationUtils::FuncSet *UserFuncs,
                     CompilationUtils::FuncSet *NewUsers);

/// Fill the user instructions (including users via other values)
///        of the input Function into the input vector.
/// F function to get user instructions.
/// UserInsts vector to fill.
void fillInstructionUsers(Function *F,
                          SmallVectorImpl<Instruction *> &UserInsts);

/// Fill internal user functions.
void fillInternalFuncUsers(Module &M, CompilationUtils::FuncSet &UserFuncs);

/// Fill printf or opencl_printf user functions.
void fillPrintfs(Module &M, CompilationUtils::FuncSet &UserFuncs);

/// Fill work-item pipe builtin user functions.
void fillWorkItemPipeBuiltinUsers(Module &M,
                                  CompilationUtils::FuncSet &UserFuncs);

/// Generate the mask argument for masked vectorized kernel.
/// \param VF the vectorization factor.
/// \param LoopLen max number of active workitems.
/// \param BB entry basicblock of the masked vectorized kernel.
Value *generateRemainderMask(unsigned VF, Value *LoopLen, BasicBlock *BB);
Value *generateRemainderMask(unsigned VF, unsigned LoopLen, BasicBlock *BB);
Value *generateRemainderMask(unsigned VF, Value *LoopLen, Instruction *IP);
Value *generateRemainderMask(unsigned VF, unsigned LoopLen, Instruction *IP);
Value *generateRemainderMask(unsigned VF, Value *LoopLen, IRBuilder<> &Builder,
                             Module *M);

/// Return true if basic block \p BB is in a subloop of \p CurLoop.
bool inSubLoop(BasicBlock *BB, Loop *CurLoop, LoopInfo *LI);

/// Return true if instruction \p I is in a subloop of \p CurLoop.
bool inSubLoop(Instruction *I, Loop *CurLoop, LoopInfo *LI);

} // namespace LoopUtils
} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_LOOP_UTILS_H
