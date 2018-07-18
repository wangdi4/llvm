//===--------------- ResolveTypes.cpp - DTransResolveTypesPass ------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans ResolveTypes pass.
//
// This pass looks for duplicate types that result from the same type being
// defined in multiple modules. When this happens the IR will contain two or
// more types with the same base name but different numeric suffixes that
// have the same layout, possibly distinguished by other duplicate types.
// The types may appear in bitcasts, including function bitcasts, where
// cross-module calls were made.
//
// For example:
//
//   %struct.A = type { i32, i32 }
//   %struct.B = type { %struct.A*, %struct.A* }
//   %struct.A.123 = type { i32, i32 }
//   %struct.B.456 = type { %struct.A.123*, %struct.A.123* }
//
//   define void @f(%struct.A* %a, %struct.B* %b) {
//     < function body >
//   }
//
//   define void @g(%struct.A.123* %a, %struct.B.456* %b) {
//     call void bitcast (void (%struct.A*, %struct.B*)* @f
//                          to void (%struct.A.123*, %struct.B.456*)*) (%a, %b)
//     ret void
//   }
//
// This pass will combine the duplicate types above and remove the bitcast at
// the callsite.
//
// This pass should be run before DTransAnalysis in order to prevent bitcast
// safety conditions resulting from the duplicate types.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/ResolveTypes.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOptBase.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
using namespace llvm;

#define DEBUG_TYPE "dtrans-resolvetypes"

namespace {

class DTransResolveTypesWrapper : public ModulePass {
private:
  dtrans::ResolveTypesPass Impl;

public:
  static char ID;

  DTransResolveTypesWrapper() : ModulePass(ID) {
    initializeDTransResolveTypesWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    const TargetLibraryInfo &TLI =
        getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
    WholeProgramInfo &WPInfo =
        getAnalysis<WholeProgramWrapperPass>().getResult();
    return Impl.runImpl(M, TLI, WPInfo);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    // TODO: Mark the actual required and preserved analyses.
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // end anonymous namespace

char DTransResolveTypesWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransResolveTypesWrapper, "dtrans-resolvetypes",
                      "DTrans resolve types", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransResolveTypesWrapper, "dtrans-resolvetypes",
                    "DTrans resolve types", false, false)

ModulePass *llvm::createDTransResolveTypesWrapperPass() {
  return new DTransResolveTypesWrapper();
}

bool dtrans::ResolveTypesPass::runImpl(Module &M, const TargetLibraryInfo &TLI,
                                      WholeProgramInfo &WPInfo) {
  if (!WPInfo.isWholeProgramSafe())
    return false;

  // TODO: Implement the transformation.

  return false;
}

PreservedAnalyses dtrans::ResolveTypesPass::run(Module &M,
                                                ModuleAnalysisManager &AM) {
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  if (!runImpl(M, TLI, WPInfo))
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}
