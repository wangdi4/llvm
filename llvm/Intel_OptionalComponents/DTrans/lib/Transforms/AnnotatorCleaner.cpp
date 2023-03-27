//===-----------AnnotatorCleaner.cpp - AnnotatorCleanerPass----------------===//
//
// Copyright (C) 2018-2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Annotation Cleanup (metadata/annotation
// intrinsics) pass.
//
// Some of the DTrans Transformation pass may insert metadata or annotation
// intrinsics to communicate structure type information from one transformation
// to another. This pass is used to clean information that is not needed
// following DTrans.

#include "Intel_DTrans/Transforms/AnnotatorCleaner.h"
#include "Intel_DTrans/Analysis/DTransAnnotator.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"

using namespace llvm;

#define DEBUG_TYPE "dtrans-annotatorcleaner"

PreservedAnalyses dtrans::AnnotatorCleanerPass::run(Module &M,
                                                    ModuleAnalysisManager &AM) {
  if (!runImpl(M, AM.getResult<WholeProgramAnalysis>(M)))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

bool dtrans::AnnotatorCleanerPass::runImpl(Module &M, WholeProgramInfo &WPInfo) {
  // Whole program is not a strict criteria for the correctness of this
  // transformation, but is checked as a compile time optimization. It is
  // known that none of the DTrans optimizations that could produce an
  // annotation will be run unless it is whole program safe, so this pass can
  // avoid walking the IR in that case. TODO: In the future, it may be good to
  // replace this by having the DTransAnnotator insert a named metadata item
  // that indicates the presence of DTrans annotations.
  if (!WPInfo.isWholeProgramSafe())
    return false;

  bool Changed = false;
  for (auto &F : M)
    Changed |= cleanFunction(F);

  return Changed;
}

bool dtrans::AnnotatorCleanerPass::cleanFunction(Function &F) {
  bool Changed = false;

  // Identify/remove the annotations. Metadata can be directly removed while
  // iterating, annotation intrinsic calls are collected for deletion after
  // iterating all the instructions.
  Changed |= DTransAnnotator::removeDTransSOAToAOSTypeAnnotation(F);

  SmallVector<Instruction *, 16> InstToDelete;
  for (auto &I : instructions(&F)) {
    if (DTransAnnotator::isDTransPtrAnnotation(I)) {
      InstToDelete.push_back(&I);
      continue;
    }

    Changed |= DTransAnnotator::removeDTransTypeAnnotation(I);
  }

  if (InstToDelete.empty())
    return Changed;

  Changed = true;
  for (auto *I : InstToDelete) {
    // The llvm.ptr.annotation result is the same as the first argument.
    // If there are uses of the annotation, they can be replaced
    // with the ptr being annotated.
    I->replaceAllUsesWith(I->getOperand(0));
    I->eraseFromParent();
  }

  return Changed;
}
