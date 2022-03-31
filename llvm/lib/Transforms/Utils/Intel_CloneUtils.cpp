//===------ Intel_CloneUtils.cpp - Utilities for Cloning -----===//
//
// Copyright (C) 2019-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file includes a set of utilities that are generally useful for
/// cloning, particularly cloning heuristics.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Utils/Intel_CloneUtils.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

#if INTEL_FEATURE_SW_ADVANCED
// Maximium period for recursive progression clone argument
static cl::opt<unsigned> IPRPCloningMaxPeriod("ip-rp-cloning-max-period",
                                              cl::init(4), cl::ReallyHidden);

// Maximium number of calls to 'F' in a recursive progression clone candidate
// 'F'. For example:
//
// static void foo(int i) {
//   int p = i % 4;
//   foo(p+1);
//   foo(p+1);
// }
//
// has 2 calls to foo() within the defintion of foo().
//
static cl::opt<unsigned> IPRPCloningMaxRecCount("ip-rp-cloning-max-rec-count",
                                                cl::init(2), cl::ReallyHidden);
//
// Return 'true' if the 'Arg' is (with high heuristic probability) a loop
// index. (This function should only be called from functions used to
// implement heuristics. We use it as a substitute for computing loops
// via dominators because it is cheaper in compile time.)
//
static bool isLoopIndexArg(Value *Arg) {
  auto TI = dyn_cast<TruncInst>(Arg);
  auto PHICand = TI ? TI->getOperand(0) : Arg;
  auto PHI = dyn_cast<PHINode>(PHICand);
  if (!PHI)
    return false;
  bool FoundLoop = false;
  for (unsigned I = 0; I < PHI->getNumIncomingValues(); I++) {
    auto IV = PHI->getIncomingValue(I);
    auto CI = dyn_cast<ConstantInt>(IV);
    if (CI)
      continue;
    auto BO = dyn_cast<BinaryOperator>(IV);
    if (!BO || BO->getOpcode() != Instruction::Add)
      return false;
    auto CII = dyn_cast<ConstantInt>(BO->getOperand(0));
    if (CII) {
      if (BO->getOperand(1) != PHI)
        return false;
    } else {
      CII = dyn_cast<ConstantInt>(BO->getOperand(1));
      if (!CII)
        return false;
      if (BO->getOperand(0) != PHI)
        return false;
      FoundLoop = true;
    }
  }
  return FoundLoop;
}

//
// Return 'true' if 'CB' has an actual argument which is (at least
// heuristically) the index of a loop.
//
static bool hasLoopIndexArg(CallBase &CB) {
  for (auto &Arg : CB.args())
    if (isLoopIndexArg(Arg.get()))
      return true;
  return false;
}

//
// Return 'true' if 'Arg' is a Type I recursive progression argument of
// recursive progression function. A Type I recursive progression argument
// has the following properties:
//   (1) It is by reference.
//   (2) It is not cyclic.
//   (3) It has only one recursive call site.
// For the meaning of 'TestCountForConstant', 'Start', 'Inc', 'Count',
// and 'ArgType' and the definition of cyclic, see the description of the
// function 'isRecProgressionCloneArgument' below.
//
static bool isRecProgressionCloneArgument1(bool TestCountForConstant,
                                           Argument &Arg, unsigned &Count,
                                           int &Start, int &Inc,
                                           Type *&ArgType) {

  //
  // Given a BasicBlock 'BS' and a dominating BasicBlock 'BT', compute
  // the successors of 'BT' on some path from 'BT' to 'BS' and put them
  // in 'OutBlocks'.
  //
  auto GenOutBlocks = [](BasicBlock *BT, BasicBlock *BS,
                         SmallPtrSetImpl<BasicBlock *> &OutBlocks) {
    SmallVector<BasicBlock *, 10> Worklist;
    SmallPtrSet<BasicBlock *, 10> Visited;
    Worklist.push_back(BS);
    while (!Worklist.empty()) {
      BasicBlock *BB = Worklist.pop_back_val();
      Visited.insert(BB);
      for (auto *BP : predecessors(BB)) {
        if (BP == BT)
          OutBlocks.insert(BB);
        else if (!Visited.count(BP))
          Worklist.push_back(BP);
      }
    }
  };

  //
  // Given 'BS' and 'BT', which dominates 'BS', and 'LI', check if 'BT'
  // terminates in a test of 'LI' against a constant 'CI' which exits
  // 'BT' on a path that always avoids 'BS'. If so, return 'CI'.
  //
  auto ExitValue = [&GenOutBlocks](LoadInst *LI, BasicBlock *BT,
                                   BasicBlock *BS) -> ConstantInt * {
    SmallPtrSet<BasicBlock *, 10> OutBlocks;
    auto TI = BT->getTerminator();
    auto BI = dyn_cast<BranchInst>(TI);
    ConstantInt *ECI = nullptr;
    if (BI) {
      if (BI->isUnconditional() || BI->getNumSuccessors() != 2)
        return nullptr;
      auto CMI = dyn_cast<ICmpInst>(BI->getCondition());
      if (!CMI)
        return nullptr;
      GenOutBlocks(BT, BS, OutBlocks);
      auto P = CMI->getPredicate();
      BasicBlock *BBTrue = BI->getSuccessor(0);
      BasicBlock *BBFalse = BI->getSuccessor(1);
      if ((!OutBlocks.count(BBTrue) && OutBlocks.count(BBFalse) &&
           P == ICmpInst::ICMP_EQ) ||
          (OutBlocks.count(BBTrue) && !OutBlocks.count(BBFalse) &&
           P == ICmpInst::ICMP_NE)) {
        if (CMI->getOperand(0) == LI)
          ECI = dyn_cast<ConstantInt>(CMI->getOperand(1));
        else if (CMI->getOperand(1) == LI)
          ECI = dyn_cast<ConstantInt>(CMI->getOperand(0));
      }
    }
    auto SI = dyn_cast<SwitchInst>(TI);
    if (SI) {
      if (SI->getCondition() != LI)
        return nullptr;
      GenOutBlocks(BT, BS, OutBlocks);
      if (!OutBlocks.count(SI->getDefaultDest()))
        return nullptr;
      BasicBlock *BBExit = nullptr;
      for (auto &Case : SI->cases()) {
        BasicBlock *BBTry = Case.getCaseSuccessor();
        if (OutBlocks.count(BBTry))
          continue;
        ConstantInt *ECITry = Case.getCaseValue();
        if (BBExit)
          return nullptr;
        BBExit = BBTry;
        ECI = ECITry;
      }
    }
    return ECI;
  };

  int LocalStart = 0;
  int LocalInc = 0;
  int LocalCount = 0;
  if (!Arg.hasOneUse())
    return false;
  User *UL = *Arg.user_begin();
  auto LI = dyn_cast<LoadInst>(UL);
  if (!LI)
    return false;
  if (!LI->getType()->isIntegerTy())
    return false;
  ArgType = LI->getType();
  // Validate the form of the recursive progression.
  AllocaInst *LAV = nullptr;
  StoreInst *SI = nullptr;
  for (User *U : LI->users()) {
    auto BIT = dyn_cast<BinaryOperator>(U);
    if (!BIT || BIT->getOpcode() != Instruction::Add)
      continue;
    auto CIT = dyn_cast<ConstantInt>(BIT->getOperand(0));
    if (CIT) {
      if (BIT->getOperand(1) != LI)
        continue;
    } else {
      CIT = dyn_cast<ConstantInt>(BIT->getOperand(1));
      if (BIT->getOperand(0) != LI)
        continue;
    }
    if (!CIT)
      continue;
    for (User *US : BIT->users()) {
      auto SIT = dyn_cast<StoreInst>(US);
      if (!SIT)
        continue;
      if (SIT->getValueOperand() != BIT)
        continue;
      auto LV = SIT->getPointerOperand();
      auto LAVV = dyn_cast<AllocaInst>(LV);
      if (!LAVV)
        continue;
      if (LocalInc != 0)
        return false;
      LocalInc = CIT->getSExtValue();
      SI = SIT;
      LAV = LAVV;
    }
  }
  if (!LocalInc)
    return false;
  Function *F = Arg.getParent();
  CallInst *CLI = nullptr;
  for (User *U : LAV->users()) {
    if (U == SI)
      continue;
    if (CLI)
      return false;
    auto CB = dyn_cast<CallBase>(U);
    if (!CB || CB->getCalledFunction() != F)
      return false;
    if (Arg.getArgNo() >= CB->arg_size())
      return false;
    if (*(CB->arg_begin() + Arg.getArgNo()) != LAV)
      return false;
    CLI = cast<CallInst>(CB);
  }
  bool SawBasis = false;
  bool SawRecCall = false;
  // Check for the basis call and recursive call.
  for (User *U : F->users()) {
    auto CB = dyn_cast<CallBase>(U);
    if (!CB || CB->getCalledFunction() != F)
      return false;
    if (Arg.getArgNo() >= CB->arg_size())
      return false;
    if (CB->getFunction() == F) {
      if (SawRecCall)
        return false;
      SawRecCall = true;
    } else {
      if (SawBasis)
        return false;
      auto AI = dyn_cast<AllocaInst>(CB->arg_begin() + Arg.getArgNo());
      if (!AI)
        return false;
      bool SawStore = false;
      for (User *V : AI->users()) {
        if (CB == V)
          continue;
        if (SawStore)
          return false;
        auto SCI = dyn_cast<StoreInst>(V);
        if (!SCI)
          return false;
        if (SCI->getPointerOperand() != AI)
          return false;
        auto BCI = dyn_cast<ConstantInt>(SCI->getValueOperand());
        if (!BCI)
          return false;
        SawStore = true;
        LocalStart = BCI->getSExtValue();
      }
      if (!SawStore)
        return false;
      SawBasis = true;
    }
  }
  // Check for the termination test.
  DominatorTree DT(*(Arg.getParent()));
  ConstantInt *ECI = nullptr;
  auto BP = CLI->getParent();
  DomTreeNode *DTN = DT.getNode(BP);
  if (!DTN)
    return false;
  for (DTN = DTN->getIDom(); DTN; DTN = DTN->getIDom()) {
    BasicBlock *BT = DTN->getBlock();
    ECI = ExitValue(LI, BT, BP);
    if (ECI)
      break;
  }
  if (!ECI)
    return false;
  int LocalStop = ECI->getSExtValue();
  if (LocalInc == 0 || ((LocalStop < LocalStart) && LocalInc > 0) ||
      ((LocalStop > LocalStart) && LocalInc < 0))
    return false;
  int LocalRange = LocalStop - LocalStart + LocalInc;
  LocalCount = LocalRange / LocalInc;
  if (LocalCount * LocalInc != LocalRange)
    return false;
  assert((LocalCount > 0) && "Expecting positive range");
  // Store back the values.
  Start = LocalStart;
  Inc = LocalInc;
  Count = (unsigned) LocalCount;
  return true;
}

//
// Return 'true' if 'Arg' is a Type II recursive progression argument of
// recursive progression function. A Type II recursive progression argument
// has the following properties:
//   (1) It is not by reference.
//   (2) It is cyclic.
//   (3) It has a cyclic period that is at least 2 but no more than
//       'IPRPCloningMaxPeriod'.
//   (4) It has no more than 'IPRPCloningMaxRecCount' recursive call sites.
//   (5) Its basis call is within a loop.
// For the meaning of 'TestCountForConstant', 'Start', 'Inc', and 'Count'
// and the definition of cyclic and basis call, see the description of the
// function 'isRecProgressionCloneArgument' below.
//
static bool isRecProgressionCloneArgument2(bool TestCountForConstant,
                                           Argument &Arg, unsigned &Count,
                                           int &Start, int &Inc) {
  bool SawBasis = false;
  int LocalStart = 0;
  int LocalInc = 0;
  unsigned LocalCount = 0;
  unsigned RecCount = 0;
  if (!Arg.hasOneUse())
    return false;
  // Validate the form of the recursive progression.
  User *U = *Arg.user_begin();
  auto SR = dyn_cast<Instruction>(U);
  if (!SR || SR->getOpcode() != Instruction::SRem)
    return false;
  if (TestCountForConstant) {
    auto CIR = dyn_cast<ConstantInt>(SR->getOperand(1));
    if (!CIR)
      return false;
    int64_t BigLocalCount = CIR->getSExtValue();
    if (BigLocalCount < 2 || BigLocalCount > IPRPCloningMaxPeriod)
      return false;
    LocalCount = BigLocalCount;
  }
  // Check for the basis call and recursive calls.
  Function *F = Arg.getParent();
  for (User *U : F->users()) {
    auto CB = dyn_cast<CallBase>(U);
    if (!CB || CB->getCalledFunction() != F)
      return false;
    if (Arg.getArgNo() >= CB->arg_size())
      return false;
    if (CB->getFunction() == F) {
      if (RecCount > IPRPCloningMaxRecCount)
        return false;
      auto BO = dyn_cast<BinaryOperator>(CB->arg_begin() + Arg.getArgNo());
      if (!BO)
        return false;
      if (BO->getOpcode() != Instruction::Add)
        return false;
      auto CI = dyn_cast<ConstantInt>(BO->getOperand(0));
      if (CI) {
        if (BO->getOperand(1) != SR)
          return false;
      } else {
        CI = dyn_cast<ConstantInt>(BO->getOperand(1));
        if (BO->getOperand(0) != SR)
          return false;
      }
      if (!CI)
        return false;
      int NewLocalInc = CI->getSExtValue();
      if (LocalInc != 0 && NewLocalInc != LocalInc)
        return false;
      LocalInc = NewLocalInc;
      RecCount++;
    } else {
      if (SawBasis)
        return false;
      auto BI = dyn_cast<ConstantInt>(CB->arg_begin() + Arg.getArgNo());
      if (!BI)
        return false;
      if (!hasLoopIndexArg(*CB))
        return false;
      LocalStart = BI->getSExtValue();
      SawBasis = true;
    }
  }
  // Store back the values.
  Start = LocalStart;
  Inc = LocalInc;
  Count = LocalCount;
  return true;
}

//
// Return 'true' if 'Arg' is a recursive progression argument for a recursive
// progression clone candidate. If 'true' is returned, set 'Start', 'Inc' and
// 'Count' to the beginning, increment, and length of the recursive
// progression. Set 'IsByRef' if the recursive progression argument is
// a by reference value. Set 'ArgType' to the type of the argument if it is
// not a by reference value, and to the pointer element type of the argument
// if it is a by reference value. Set 'IsCyclic' if the recursive progression
// is cyclic.

// If 'TestForConstant' is 'true', we return 'false' if the 'Count' value is
// not a constant. In determining whether we are sure we have a recursive
// progression, we should set 'TestForConstant' to 'true'. We use this
// query function with 'TestForConstant' equal to 'false' to inhibit the
// performing of the tail call elimination optimization on functions which
// are potential recursive progression clone candidates.  This is because
// the tail call elimination optimization can occur in the 'PrepareForLTO'
// step before IP cloning and we may not know at that point the value of
// 'Count'.
//
// Here is an example of a function foo() with a cyclic recursive progression:
//   static void foo(int i) {
//     ..
//     int p = (i + 1) % 4;
//     foo(p);
//     ..
//   }
//   static void bar() {
//     ..
//     foo(0);
//     ..
//   }
// The recursive progression argument is "i", 'ArgPos' is 0, 'Start'
// is 0, 'Inc' is 1, and 'Count' is 4.  The call to foo() in bar() which
// launches the recursive progression is called the basis call. The argument
// is has the values 0, 1, 2, 3, 0, 1, 2, 3, ... on successive recursive calls
// to foo.
//
// Here is an example of a function foo() with a non-cyclic recursive
// progression:
//   static void foo(int j) {
//     ..
//     if (j != 8)
//       foo(j + 1);
//     ..
//   }
//   static void bar() {
//     ..
//     foo(1);
//     ..
//   }
// The recursive progression argument is "j", 'ArgPos' is 0, 'Start'
// is 1, 'Inc' is 1, and 'Count' is 8. The argument has the values 1, 2, 3, 4,
// 5, 6, 7, 8 on successive recursive calls to foo. The guard test around
// the recursive call ensures that the values do not cycle.
//
static bool isRecProgressionCloneArgument(bool TestCountForConstant,
                                          Argument &Arg, unsigned &Count,
                                          int &Start, int &Inc,
                                          bool &IsByRef, Type *&ArgType,
                                          bool &IsCyclic) {

   bool RV = false;
   Type *LArgType = nullptr;
   RV = isRecProgressionCloneArgument1(TestCountForConstant, Arg, Count,
                                       Start, Inc, LArgType);
   if (RV) {
     IsByRef = true;
     ArgType = LArgType;
     IsCyclic = false;
     return true;
   }
   RV = isRecProgressionCloneArgument2(TestCountForConstant, Arg, Count,
                                       Start, Inc);
   if (RV) {
     IsByRef = false;
     ArgType = Arg.getType();
     IsCyclic = true;
     return true;
   }
   return false;
}

namespace llvm {

extern bool isRecProgressionCloneCandidate(Function &F,
                                           bool TestCountForConstant,
                                           unsigned *ArgPos, unsigned *Count,
                                           int *Start, int *Inc,
                                           bool *IsByRef, Type **ArgType,
                                           bool *IsCyclic) {
  unsigned LocalCount;
  int LocalStart;
  int LocalInc;
  bool LocalIsByRef;
  Type *LocalArgType = nullptr;
  bool LocalIsCyclic;

  for (auto &Arg : F.args()) {
    if (isRecProgressionCloneArgument(TestCountForConstant, Arg, LocalCount,
                                      LocalStart, LocalInc, LocalIsByRef,
                                      LocalArgType, LocalIsCyclic)) {
      unsigned LocalArgPos = Arg.getArgNo();
      if (ArgPos)
        *ArgPos = LocalArgPos;
      if (Count)
        *Count = LocalCount;
      if (Start)
        *Start = LocalStart;
      if (Inc)
        *Inc = LocalInc;
      if (IsByRef)
        *IsByRef = LocalIsByRef;
      if (ArgType)
        *ArgType = LocalArgType;
      if (IsCyclic)
        *IsCyclic = LocalIsCyclic;
      return true;
    }
  }
  return false;
}

} // namespace llvm
#endif // INTEL_FEATURE_SW_ADVANCED

namespace llvm {

extern CallInst *uniqueCallSite(Function &F) {
  CallInst *CISave = nullptr;
  for (User *U : F.users()) {
    auto BCO = dyn_cast<BitCastOperator>(U);
    if (BCO && BCO->hasNUses(0))
      continue;
    auto CI = dyn_cast<CallInst>(U);
    if (!CI || CISave)
      return nullptr;
    CISave = CI;
  }
  return CISave;
}

} // namespace llvm
