//===---------------------- Intel_ForceInline.cpp  ------------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass forces aggressive inlining by transforming all inlinehint
// attributes to alwaysinline. It's implemented as a module pass to ensure
// the consistency of the transformation and the resulting inlining.
//
//===----------------------------------------------------------------------===//
//

#include "llvm/Transforms/IPO/Intel_ForceInline.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;

#define DEBUG_TYPE "inlineforceinline"

// Enable/disable this transformation
cl::opt<bool> IntelForceInline("inline-forceinline",
                               cl::desc("Treat inline routines as forceinline"),
                               cl::ReallyHidden);

// Iterate callsites within a function and transform
// inlinehint attributes to alwaysinline
static void processCallsitesForFn(Function &F) {
  for (auto &I : instructions(F)) {
    if (auto *CB = dyn_cast<CallBase>(&I)) {
      auto Callee = CB->getCalledFunction();
      if (!Callee)
        continue;

      int CBChanged = 0;

      // promote hints to forced inlining
      if (CB->hasFnAttrOnCallsite(Attribute::InlineHint)) {
        if (CB->hasFnAttr(Attribute::NoInline))
          CB->removeFnAttr(Attribute::NoInline);
        CB->removeFnAttr(Attribute::InlineHint);
        CB->addFnAttr(Attribute::AlwaysInline);
        CBChanged = 1;
      } else if (CB->hasFnAttrOnCallsite(Attribute::InlineHintRecursive) &&
                 !CB->hasFnAttr(Attribute::NoInline)) {
        CB->removeFnAttr(Attribute::InlineHintRecursive);
        CB->addFnAttr(Attribute::AlwaysInlineRecursive);
        CBChanged = 2;
      }

      // print debug info
      LLVM_DEBUG(dbgs() << "\t\t-> call to " << Callee->getName());
      if (CBChanged == 1)
        LLVM_DEBUG(dbgs() << " [Force Inlined]");
      else if (CBChanged == 2)
        LLVM_DEBUG(dbgs() << " [Force Inlined (recursive)]");
      LLVM_DEBUG(dbgs() << "\n");
    }
  }
}

// Iterate functions and callsites in module and transform
// inlinehint attributes to alwaysinline (forceinline)
static void transformInlFnAttrs(Module &M) {
  LLVM_DEBUG(dbgs() << "Force Inlining Routines & Calls in " << M.getName()
                    << ":\n");

  // iterate functions in module
  for (Function &F : M) {

    int FnChanged = 0;

    // promote hints to forced inlining
    if (F.hasFnAttribute(Attribute::InlineHint)) {
      if (F.hasFnAttribute(Attribute::NoInline)) {
        F.removeFnAttr(Attribute::NoInline);
        if (F.hasFnAttribute(Attribute::OptimizeNone))
          F.removeFnAttr(Attribute::OptimizeNone);
      }
      F.removeFnAttr(Attribute::InlineHint);
      F.addFnAttr(Attribute::AlwaysInline);
      FnChanged = 1;
    } else if (F.hasFnAttribute(Attribute::InlineHintRecursive) &&
               !F.hasFnAttribute(Attribute::NoInline)) {
      F.removeFnAttr(Attribute::InlineHintRecursive);
      F.addFnAttr(Attribute::AlwaysInlineRecursive);
      FnChanged = 2;
    }

    // print debug info
    LLVM_DEBUG(dbgs() << "\tFunction " << F.getName());
    if (FnChanged == 1)
      LLVM_DEBUG(dbgs() << " [Force Inlined]");
    else if (FnChanged == 2)
      LLVM_DEBUG(dbgs() << " [Force Inlined (recursive)]");
    LLVM_DEBUG(dbgs() << ":\n");

    processCallsitesForFn(F);
  }
}

PreservedAnalyses InlineForceInlinePass::run(Module &M,
                                             ModuleAnalysisManager &AM) {
  if (IntelForceInline)
    transformInlFnAttrs(M);
  return PreservedAnalyses::all();
}
