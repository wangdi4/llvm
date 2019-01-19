//===------ Intel_CloneUtils.cpp - Utilities for Cloning -----===//
//
// Copyright (C) 2019-2019 Intel Corporation. All rights reserved.
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
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Constants.h"

using namespace llvm;

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
// Return 'true' if 'CS' has an actual argument which is (at least
// heuristically) the index of a loop.
//
static bool hasLoopIndexArg(CallSite &CS) {
  for (auto &Arg : CS.args())
    if (isLoopIndexArg(Arg.get()))
      return true;
  return false;
}

//
// Return 'true' if 'F' is a recursive progression clone candidate. If
// 'true' is returned, set 'ArgPos' to the position of the argument through
// which the recursive progression occurs, and set 'Start', 'Inc' and
// 'Count' to the beginning, increment, and length of the recursive
// progression.
//
// As an example, consider a function foo() with the form:
//   static void foo(int i) {
//     ..
//     int p = (i + 1) % 4;
//     foo(p);
//     ..
//   }
//
//   static void bar() {
//     ..
//     foo(1);
//     ..
//   }
// Here, the recursive progressive argument is "i", 'ArgPos' is 0, 'Start'
// is 1, 'Inc' is 1, and 'Count' is 4.  The call to foo() in bar() which
// launches the recursive progression is called the basis call.
//
static bool isRecProgressionCloneArgument(bool TestCountForConstant,
                                          Argument &Arg, unsigned &Count,
                                          int &Start, int &Inc) {
  bool SawBasis = false;
  int LocalStart = 0;
  int LocalInc = 0;
  unsigned LocalCount = 0;
  unsigned RecCount = 0;
  if (!Arg.hasOneUse())
    return false;
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
  Function *F = Arg.getParent();
  for (User *U : F->users()) {
    CallSite CS(U);
    if (!CS || CS.getCalledFunction() != F)
      return false;
    if (Arg.getArgNo() >= CS.arg_size())
      return false;
    auto CSI = CS.getInstruction();
    if (CSI->getFunction() == F) {
      if (RecCount > IPRPCloningMaxRecCount)
        return false;
      auto BO = dyn_cast<BinaryOperator>(CS.arg_begin() + Arg.getArgNo());
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
      auto BI = dyn_cast<ConstantInt>(CS.arg_begin() + Arg.getArgNo());
      if (!BI)
        return false;
      if (!hasLoopIndexArg(CS))
        return false;
      LocalStart = BI->getSExtValue();
      SawBasis = true;
    }
  }
  Start = LocalStart;
  Inc = LocalInc;
  Count = LocalCount;
  return true;
}

namespace llvm {

extern bool isRecProgressionCloneCandidate(Function &F,
                                           bool TestCountForConstant,
                                           unsigned *ArgPos, unsigned *Count,
                                           int *Start, int *Inc) {
  unsigned LocalCount;
  int LocalStart;
  int LocalInc;
  for (auto &Arg : F.args()) {
    if (isRecProgressionCloneArgument(TestCountForConstant, Arg, LocalCount,
                                      LocalStart, LocalInc)) {
      unsigned LocalArgPos = Arg.getArgNo();
      if (ArgPos)
        *ArgPos = LocalArgPos;
      if (Count)
        *Count = LocalCount;
      if (Start)
        *Start = LocalStart;
      if (Inc)
        *Inc = LocalInc;
      return true;
    }
  }
  return false;
}

} // namespace llvm
