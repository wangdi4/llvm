//===- StdContainerAA.cpp - Std Cotnainer Alias Alias Analysis
//---------------===//
//
// Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===---------------------------------------------------------------===//
//
// This file defines the pass of the alias analysis for std containers,
// which implements based on the metadata !std.container.ptr and 
// !std.container.ptr.iter.
// The contents of metadata !std.container.ptr is a set of unique 
// integer. Given two metadata a and b, the compiler determines 
// whether they are aliased by examining their set are overlapped
// or not. Here is one example.
//
// !10 = !{i32 0}
// !11 = !{i32 2}
//
// !10 and !11 are not aliased since the set {i32 0} are not overlapped
// with the set {i32 2}.
//
//===----------------------------------------------------------------------===//
#include "llvm/Analysis/Intel_StdContainerAA.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

static cl::opt<bool> EnableStdContainerAlias("enable-std-container-alias",
                                             cl::init(true));

AliasResult StdContainerAAResult::alias(const MemoryLocation &LocA,
                                        const MemoryLocation &LocB) {
  if (!EnableStdContainerAlias)
    return AAResultBase::alias(LocA, LocB);

  MDNode *M1, *M2;
  M1 = LocA.AATags.StdContainerPtr;
  M2 = LocB.AATags.StdContainerPtr;

  if (!mayAliasInStdContainer(M1, M2))
    return NoAlias;

  M1 = LocA.AATags.StdContainerPtrIter;
  M2 = LocB.AATags.StdContainerPtrIter;

  if (!mayAliasInStdContainer(M1, M2))
    return NoAlias;

  return AAResultBase::alias(LocA, LocB);
}

bool StdContainerAAResult::mayAliasInStdContainer(MDNode *M1, MDNode *M2) {
  if (!M1 || !M2)
    return true;
  for (unsigned I = 0; I < M1->getNumOperands(); I++)
    for (unsigned J = 0; J < M2->getNumOperands(); J++)
      if (mdconst::extract<ConstantInt>(M1->getOperand(I)) ==
          mdconst::extract<ConstantInt>(M2->getOperand(J)))
        return true;
  return false;
}

ModRefInfo StdContainerAAResult::getModRefInfo(ImmutableCallSite CS,
                                               const MemoryLocation &Loc) {
  return AAResultBase::getModRefInfo(CS, Loc);
}

ModRefInfo StdContainerAAResult::getModRefInfo(ImmutableCallSite CS1,
                                               ImmutableCallSite CS2) {
  return AAResultBase::getModRefInfo(CS1, CS2);
}

char StdContainerAA::PassID;

// Provide a definition for the static class member used to identify passes.
AnalysisKey StdContainerAA::Key;

StdContainerAAResult StdContainerAA::run(Function &F,
                                         AnalysisManager<Function> &AM) {
  return StdContainerAAResult();
}

char StdContainerAAWrapperPass::ID = 0;
INITIALIZE_PASS(StdContainerAAWrapperPass, "std-container-alias",
                "Std Container Alias Analysis", false, true)

ImmutablePass *llvm::createStdContainerAAWrapperPass() {
  return new StdContainerAAWrapperPass();
}

StdContainerAAWrapperPass::StdContainerAAWrapperPass() : ImmutablePass(ID) {
  initializeStdContainerAAWrapperPassPass(*PassRegistry::getPassRegistry());
}

bool StdContainerAAWrapperPass::doInitialization(Module &M) {
  Result.reset(new StdContainerAAResult());
  return false;
}

bool StdContainerAAWrapperPass::doFinalization(Module &M) {
  Result.reset();
  return false;
}

void StdContainerAAWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}
