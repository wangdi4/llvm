//===------ DPCPPKernelLoopUtils.cpp - Function definitions -*- C++ -------===//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelLoopUtils.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/RuntimeService.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

namespace llvm {

using namespace DPCPPKernelCompilationUtils;

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

Type *getIndTy(Module *M) {
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
    Function *ParentFunc = CI->getFunction();
    if (ParentFunc == FuncToSearch)
      Calls.push_back(CI);
  }
}

void collectTIDCallInst(StringRef TIDName, InstVecVec &TidCalls, Function *F) {
  InstVec EmptyVec;
  TidCalls.assign(MAX_WORK_DIM, EmptyVec);
  SmallVector<CallInst *, 4> AllDimTIDCalls;
  getAllCallInFunc(TIDName, F, AllDimTIDCalls);

  for (CallInst *CI : AllDimTIDCalls) {
    unsigned Dim = 0;
    if (CI->arg_size() == 0) {
      // No-operand version - does not really matter what dimension we will be
      // vectorizing over. Some examples:
      //   * <something>_linear_id,
      //   * get_sub_group_local_id.
      //
      // FIXME: Separate index for such kind of functions. Currently we're
      // vectorizing over 0-dimension only.
      Dim = 0;
    } else {
      assert(CI->arg_size() == 1 && "Expected one-operand call!");
      ConstantInt *C = dyn_cast<ConstantInt>(CI->getArgOperand(0));
      // Do not expect a non-const arg in current DPCPP HOST impl.
      assert(C && "tid arg must be constant");
      // But skip it if we have it.
      if (!C)
        continue;
      Dim = C->getValue().getZExtValue();
      assert(Dim < MAX_WORK_DIM && "tid not in range");
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
std::pair<LoopRegion, PHINode *> createLoop(BasicBlock *Head, BasicBlock *Latch,
                                            Value *Begin, Value *Increment,
                                            Value *End, CmpInst::Predicate Pred,
                                            std::string &DimPrefix,
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
  BasicBlock *PreHead = BasicBlock::Create(C, DimPrefix + "pre_head", F, Head);
  BasicBlock *Exit = BasicBlock::Create(C, DimPrefix + "exit", F);
  Exit->moveAfter(Latch);
  BranchInst::Create(Head, PreHead);

  // Insert induction variable phi in the head entry.
  PHINode *IndVar =
      Head->empty()
          ? PHINode::Create(IndTy, 2, DimPrefix + "ind_var", Head)
          : PHINode::Create(IndTy, 2, DimPrefix + "ind_var", &*Head->begin());

  // Increment induction variable.
  BinaryOperator *IncIndVar = BinaryOperator::Create(
      Instruction::Add, IndVar, Increment, DimPrefix + "inc_ind_var", Latch);
  IncIndVar->setHasNoSignedWrap();
  IncIndVar->setHasNoUnsignedWrap();

  // Create compare and conditionally branch out from latch.
  Instruction *Compare =
      new ICmpInst(*Latch, Pred, IncIndVar, End, DimPrefix + "cmp.to.max");
  BranchInst::Create(Exit, Head, Compare, Latch);

  // Upadte induction variable phi with the incoming values.
  IndVar->addIncoming(Begin, PreHead);
  IndVar->addIncoming(IncIndVar, Latch);
  return {LoopRegion{PreHead, Head, Exit}, IndVar};
}

void fillAtomicBuiltinUsers(Module &M, RuntimeService *RTService,
                            FuncSet &UserFuncs) {
  FuncSet AtomicFuncs;
  for (Function &F : M)
    if (F.isDeclaration() && RTService->isAtomicBuiltin(F.getName()))
      AtomicFuncs.insert(&F);

  fillFuncUsersSet(AtomicFuncs, UserFuncs);
}

void fillFuncUsersSet(const FuncSet &Roots, FuncSet &UserFuncs) {
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

void fillDirectUsers(const FuncSet *Funcs, FuncSet *UserFuncs,
                     FuncSet *NewUsers) {
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

void fillInternalFuncUsers(Module &M, FuncSet &UserFuncs) {
  FuncSet InternalFuncs;
  for (Function &F : M)
    if (!F.isDeclaration())
      InternalFuncs.insert(&F);

  fillFuncUsersSet(InternalFuncs, UserFuncs);
}

void fillPrintfs(Module &M, FuncSet &UserFuncs) {
  FuncSet PrintfFuncs;
  for (Function &F : M) {
    StringRef FName = F.getName();
    if (F.isDeclaration() && (isPrintf(FName) || isOpenCLPrintf(FName)))
      PrintfFuncs.insert(&F);
  }
  fillFuncUsersSet(PrintfFuncs, UserFuncs);
}

void fillWorkItemPipeBuiltinUsers(Module &M, FuncSet &UserFuncs) {
  FuncSet PipeFuncs;
  for (Function &F : M)
    if (F.isDeclaration() && isWorkItemPipeBuiltin(F.getName()))
      PipeFuncs.insert(&F);

  fillFuncUsersSet(PipeFuncs, UserFuncs);
}

Value *generateRemainderMask(unsigned VF, Value *LoopLen, IRBuilder<> &Builder,
                             Module *M) {
  auto *IndTy = getIndTy(M);
  auto *MaskTy = FixedVectorType::get(Builder.getInt32Ty(), VF);

  // Generate sequential vector 0, ..., VF-1.
  SmallVector<Constant *, 16> IndVec;
  for (unsigned I = 0; I < VF; ++I)
    IndVec.push_back(ConstantInt::get(IndTy, I));
  Constant *SequenceVec = ConstantVector::get(IndVec);

  // Generate splat vector LoopLen, ..., LoopLen.
  if (LoopLen->getType() != IndTy)
    LoopLen = Builder.CreateZExtOrTrunc(LoopLen, IndTy);
  auto *SplatVec = Builder.CreateVectorSplat(VF, LoopLen);

  // Generate mask.
  auto *VecCmp = Builder.CreateICmpULT(SequenceVec, SplatVec, "mask.i1");
  auto *Mask = Builder.CreateSExt(VecCmp, MaskTy, "mask.i32");
  return Mask;
}

Value *generateRemainderMask(unsigned VF, Value *LoopLen, BasicBlock *BB) {
  IRBuilder<> Builder(BB);
  return generateRemainderMask(VF, LoopLen, Builder, BB->getModule());
}

Value *generateRemainderMask(unsigned VF, unsigned LoopLen, BasicBlock *BB) {
  auto *IndTy = getIndTy(BB->getModule());
  auto *LoopLenVal = ConstantInt::get(IndTy, LoopLen);
  return generateRemainderMask(VF, LoopLenVal, BB);
}

Value *generateRemainderMask(unsigned VF, Value *LoopLen, Instruction *IP) {
  IRBuilder<> Builder(IP);
  return generateRemainderMask(VF, LoopLen, Builder, IP->getModule());
}

Value *generateRemainderMask(unsigned VF, unsigned LoopLen, Instruction *IP) {
  auto *IndTy = getIndTy(IP->getModule());
  auto *LoopLenVal = ConstantInt::get(IndTy, LoopLen);
  return generateRemainderMask(VF, LoopLenVal, IP);
}

bool inSubLoop(BasicBlock *BB, Loop *CurLoop, LoopInfo *LI) {
  assert(CurLoop->contains(BB) && "BB not in CurLoop");
  return LI->getLoopFor(BB) != CurLoop;
}

bool inSubLoop(Instruction *I, Loop *CurLoop, LoopInfo *LI) {
  return inSubLoop(I->getParent(), CurLoop, LI);
}

} // namespace DPCPPKernelLoopUtils
} // namespace llvm
