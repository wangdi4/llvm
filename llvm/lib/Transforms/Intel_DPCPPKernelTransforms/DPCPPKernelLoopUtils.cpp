//===------ DPCPPKernelLoopUtils.cpp - Function definitions -*- C++ -------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelLoopUtils.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {
namespace DPCPPKernelLoopUtils {

Function *getWIFunc(Module *M, StringRef FuncName, Type *RetTy) {
  // First look in the module if it already exists
  Function *F = M->getFunction(FuncName);
  if (F)
    return F;
  // create declaration.
  std::vector<Type *> ArgTypes(1, Type::getInt32Ty(RetTy->getContext()));
  FunctionType *FType = FunctionType::get(RetTy, ArgTypes, false);
  FunctionCallee FConst = M->getOrInsertFunction(FuncName, FType);
  F = dyn_cast<Function>(FConst.getCallee());
  assert(F && "null WI function");
  return F;
}

CallInst *getWICall(Module *M, StringRef FuncName, Type *RetTy, Value *Dim,
                    BasicBlock *BB, const Twine &CallName) {
  Function *WIFunc = getWIFunc(M, FuncName, RetTy);
  CallInst *WICall = CallInst::Create(WIFunc, Dim, CallName, BB);
  return WICall;
}

CallInst *getWICall(Module *M, StringRef FuncName, Type *RetTy, Value *Dim,
                    Instruction *IP, const Twine &CallName) {
  Function *WIFunc = getWIFunc(M, FuncName, RetTy);
  CallInst *WICall = CallInst::Create(WIFunc, Dim, CallName, IP);
  return WICall;
}

CallInst *getWICall(Module *M, StringRef FuncName, Type *RetTy, unsigned Dim,
                    BasicBlock *BB, const Twine &CallName) {
  Constant *DimConst =
      ConstantInt::get(Type::getInt32Ty(RetTy->getContext()), Dim);
  return getWICall(M, FuncName, RetTy, DimConst, BB, CallName);
}

CallInst *getWICall(Module *M, StringRef FuncName, Type *RetTy, unsigned Dim,
                    Instruction *IP, const Twine &CallName) {
  Constant *DimConst =
      ConstantInt::get(Type::getInt32Ty(RetTy->getContext()), Dim);
  return getWICall(M, FuncName, RetTy, DimConst, IP, CallName);
}

Type *getIntTy(Module *M) {
  unsigned PointerSizeInBits = M->getDataLayout().getPointerSizeInBits(0);
  assert((32 == PointerSizeInBits || 64 == PointerSizeInBits) &&
         "Unsopported pointer size");
  return IntegerType::get(M->getContext(), PointerSizeInBits);
}

void getAllCallInFunc(StringRef FuncName, Function *FuncToSearch,
                      SmallVectorImpl<CallInst *> &Calls) {
  Function *F = FuncToSearch->getParent()->getFunction(FuncName);
  if (!F)
    return;

  for (auto *U : F->users()) {
    CallInst *CI = cast<CallInst>(U);
    Function *ParentFunc = CI->getParent()->getParent();
    if (ParentFunc == FuncToSearch)
      Calls.push_back(CI);
  }
}

void collectTIDCallInst(StringRef TIDName, InstVecVec &TidCalls, Function *F) {
  const unsigned MAX_OCL_NUM_DIM = 3;
  InstVec EmptyVec;
  TidCalls.assign(MAX_OCL_NUM_DIM, EmptyVec);
  SmallVector<CallInst *, 4> AllDimTIDCalls;
  getAllCallInFunc(TIDName, F, AllDimTIDCalls);

  for (CallInst *CI : AllDimTIDCalls) {
    unsigned Dim = 0;
    if (CI->getNumArgOperands() == 0) {
      // No-operand version - does not really matter what dimension we will be
      // vectorizing over. Some examples:
      //   * <something>_linear_id,
      //   * get_sub_group_local_id.
      //
      // FIXME: Separate index for such kind of functions. Currently we're
      // vectorizing over 0-dimension only.
      Dim = 0;
    } else {
      assert(CI->getNumArgOperands() == 1 && "Expected one-operand call!");
      ConstantInt *C = dyn_cast<ConstantInt>(CI->getArgOperand(0));
      // Do not expect a non-const arg in current DPCPP HOST impl.
      assert(C && "tid arg must be constant");
      // But skip it if we have it.
      if (!C)
        continue;
      Dim = C->getValue().getZExtValue();
      assert(Dim < MAX_OCL_NUM_DIM && "tid not in range");
    }
    TidCalls[Dim].push_back(CI);
  }
}

// transforms code as follows:
// prehead:
//     br head
// head:
//     indVar = phi (0 preHead, incIndVar latch)
//          :
// latch: //(can be the same as head)
//          :
//     incIndVar = add indVar, 1
//     x = Icmp eq incIndVar ,loopSize
//     br x, exit, head
// exit:
//
//  all get_local_id \ get_global_id are replaced with indVar
LoopRegion createLoop(BasicBlock *Head, BasicBlock *Latch, Value *Begin,
                      Value *Increment, Value *End, std::string &Name,
                      LLVMContext &C) {
  Type *IndTy = Begin->getType();
  assert(IndTy == Increment->getType() &&
         "Increment type does not correspond to Lower bound!");
  assert(IndTy == End->getType() &&
         "Upper bound type does not correspond to Lower Bound!");
  assert(Head->getParent() == Latch->getParent() &&
         "Head and Latch belong to different Parents!");
  // Creating Blocks to wrap the code as described above.
  Function *F = Head->getParent();
  BasicBlock *PreHead = BasicBlock::Create(C, Name + "pre_head", F, Head);
  BasicBlock *Exit = BasicBlock::Create(C, Name + "exit", F);
  Exit->moveAfter(Latch);
  BranchInst::Create(Head, PreHead);

  // Insert induction variable phi in the head entry.
  PHINode *IndVar =
      Head->empty()
          ? PHINode::Create(IndTy, 2, Name + "ind_var", Head)
          : PHINode::Create(IndTy, 2, Name + "ind_var", &*Head->begin());

  // Increment induction variable.
  BinaryOperator *IncIndVar = BinaryOperator::Create(
      Instruction::Add, IndVar, Increment, Name + "inc_ind_var", Latch);
  IncIndVar->setHasNoSignedWrap();
  IncIndVar->setHasNoUnsignedWrap();

  // Create compare and conditionally branch out from latch.
  Instruction *Compare = new ICmpInst(*Latch, CmpInst::ICMP_EQ, IncIndVar, End,
                                      Name + "cmp.to.max");
  BranchInst::Create(Exit, Head, Compare, Latch);

  // Upadte induction variable phi with the incoming values.
  IndVar->addIncoming(Begin, PreHead);
  IndVar->addIncoming(IncIndVar, Latch);
  return LoopRegion(PreHead, Exit);
}

void fillFuncUsersSet(FuncSet &Roots, FuncSet &UserFuncs) {
  FuncSet NewUsers1, NewUsers2;
  FuncSet *NewUsersPtr = &NewUsers1;
  FuncSet *RootsPtr = &NewUsers2;
  // First Get the direct users of the roots.
  fillDirectUsers(&Roots, &UserFuncs, NewUsersPtr);
  while (NewUsersPtr->size()) {
    // Iteratively swap between the new users sets, and use the current
    // as the roots for the new direct users.
    std::swap(NewUsersPtr, RootsPtr);
    NewUsersPtr->clear();
    fillDirectUsers(RootsPtr, &UserFuncs, NewUsersPtr);
  }
}

void fillDirectUsers(FuncSet *Funcs, FuncSet *UserFuncs, FuncSet *NewUsers) {
  // Go through all of the Funcs.
  SmallVector<Instruction *, 8> UserInst;
  for (Function *F : *Funcs) {
    if (!F)
      continue;

    // Get the instruction users of the function, and insert their
    // parent functions to the UserFuncs set.
    UserInst.clear();
    fillInstructionUsers(F, UserInst);
    for (Instruction *I : UserInst) {
      Function *UserFunc = I->getParent()->getParent();
      assert(UserFunc && "NULL parent function ?");
      // If the user is a call add the function contains it to UserFuncs.
      if (UserFuncs->insert(UserFunc)) {
        // If the function is new update the new user set.
        NewUsers->insert(UserFunc);
      }
    }
  }
}

void fillInstructionUsers(Function *F,
                          SmallVectorImpl<Instruction *> &UserInsts) {
  // Holds values to check.
  SmallVector<Value *, 8> WorkList(F->users());
  // Holds values that already been checked, in order to prevent
  // inifinite loops.
  SetVector<Value *> Visited;
  while (WorkList.size()) {
    Value *User = WorkList.back();
    WorkList.pop_back();
    if (!Visited.insert(User))
      continue;

    // New value if it is an Instruction add to it to usedInsts Vec,
    // otherwise check all it's users.
    Instruction *I = dyn_cast<Instruction>(User);
    if (I) {
      UserInsts.push_back(I);
    } else {
      WorkList.append(User->user_begin(), User->user_end());
    }
  }
}

} // namespace DPCPPKernelLoopUtils
} // namespace llvm
