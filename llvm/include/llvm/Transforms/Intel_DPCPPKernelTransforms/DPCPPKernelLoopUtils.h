//==-------- DPCPPKernelLoopUtils.h - Function declarations -*- C++---------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef __DPCPP_KERNEL_LOOP_UTILS_H__
#define __DPCPP_KERNEL_LOOP_UTILS_H__

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"

using namespace llvm;

namespace llvm {

/// Helpful shortcuts for structures.
using ValueVec = SmallVector<Value *, 4>;
using InstVec = SmallVector<Instruction *, 4>;
using InstVecVec = SmallVector<InstVec, 4>;
using FuncSet = DPCPPKernelCompilationUtils::FuncSet;

/// Struct that represent loop Region in the CFG.
struct LoopRegion {
  BasicBlock *PreHeader; // Pre header block of the loop.
  BasicBlock *Exit;      // Exit block of the loop.

  /// C'tor.
  LoopRegion(BasicBlock *PreHeader, BasicBlock *Exit)
      : PreHeader(PreHeader), Exit(Exit) {}

  LoopRegion() : PreHeader(nullptr), Exit(nullptr) {}
};

namespace DPCPPKernelLoopUtils {

/// Creates loop with loopSize iterations arround the CFG region that
///       begins in head and finishes in latch.
/// Head - The head of the created loop.
/// Latch - The latch block of the created loop.
/// Begin - Lower loop bound.
/// Increment - Loop increment (1 or PacketSize).
/// End - Upper loop bound.
/// Returns struct with pre header and exit block fot the created loop.
LoopRegion createLoop(BasicBlock *Head, BasicBlock *Latch, Value *Begin,
                      Value *Increment, Value *End, std::string &Name,
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
CallInst *getWICall(Module *M, StringRef FuncName, Type *RetTy, unsigned Dim,
                    BasicBlock *BB, const Twine &CallName = "");
CallInst *getWICall(Module *M, StringRef FuncName, Type *RetTy, unsigned Dim,
                    Instruction *IP, const Twine &CallName = "");

/// Fills call vector with all calls to function named func name in
///       funcToSearch
/// FuncName - name of functions to obtain its calls.
/// FuncToSearch - Function to look call instructions in.
/// Calls - vector to fill.
void getAllCallInFunc(StringRef FuncName, Function *FuncToSearch,
                      SmallVectorImpl<CallInst *> &Calls);

/// Returns size_t type.
/// M - current module.
Type *getIntTy(Module *M);

/// Collect the get_***_id() in F.
/// TIDName - name of the tid generator get_global_id\ get_local_id.
/// TidCalls - array of get_***_id call to fill.
/// F - kernel to collect information for.
void collectTIDCallInst(StringRef TIDName, InstVecVec &TidCalls, Function *F);

/// Fills the users function through call instructions of roots
///       (also indirect users) into userFuncs.
/// Roots - function to obtain their user functions.
/// UserFuncs - set to fill with users of roots
void fillFuncUsersSet(FuncSet &Roots, FuncSet &UserFuncs);

/// Fills direct user functions through instructions of functions in
///       funcs set into userFuncs. If a function is introduced into
///       userFuncs for the first time it will be inserted into newUsers.
/// Funcs - function to obtain direct users.
/// UserFuncs - set of users functions to fills.
/// NewUsers - set of newly found users.
void fillDirectUsers(FuncSet *Funcs, FuncSet *UserFuncs, FuncSet *NewUsers);

/// Fill the user instructions (including users via other values)
///        of the input Function into the input vector.
/// F function to get user instructions.
/// UserInsts vector to fill.
void fillInstructionUsers(Function *F,
                          SmallVectorImpl<Instruction *> &UserInsts);

} // namespace DPCPPKernelLoopUtils
} // namespace llvm

#endif //__DPCPP_KERNEL_LOOP_UTILS_H__
