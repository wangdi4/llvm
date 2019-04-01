//===--------------- Transpose.cpp - DTransTransposePass------------------===//
//
// Copyright (C) 2019-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Transpose optimization for Fortran
// multi-dimensional arrays.
//
//===----------------------------------------------------------------------===//
#include "Intel_DTrans/Transforms/Transpose.h"
#include "Intel_DTrans/DTransCommon.h"

#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Pass.h"

using namespace llvm;

#define DEBUG_TYPE "dtrans-transpose"

namespace {

// The array stride transpose optimization for Fortran.
//
// This optimization swaps the stride values used for multi-dimensional Fortran
// arrays to improve cache utilization or enable loop unrolling by having unit
// stride memory access patterns.
//
// For example, the default memory layout for the Fortran array declared as
// "integer block(3,3)" is stored in column-major order resulting in the access
// to block(i,j) being computed as:
//     &block + j * 3 * sizeof(integer) + i * sizeof(integer)
//
// For a loop iterating along 'j', this may enable downstream optimizations if
// the strides are transposed so that iterations along 'j' will be a unit
// stride.
//
// This class will heuristically estimate the benefit and swap the stride values
// when beneficial.
class TransposeImpl {
public:
  TransposeImpl(std::function<LoopInfo &(Function &)> &GetLI) : GetLI(GetLI) {}

  bool run(Module &M) { return false; }

private:
  std::function<LoopInfo &(Function &)> &GetLI;
};

// Legacy pass manager wrapper for invoking the Transpose pass.
class DTransTransposeWrapper : public ModulePass {
private:
  dtrans::TransposePass Impl;

public:
  static char ID;
  DTransTransposeWrapper() : ModulePass(ID) {
    initializeDTransTransposeWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    dtrans::LoopInfoFuncType GetLI = [this](Function &F) -> LoopInfo & {
      return this->getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    };

    return Impl.runImpl(M, GetLI);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    // Note, this transformation is not dependent on Whole Program Analysis.
    // The only candidates that may be selected for the transformation will
    // have internal linkage, and the analysis will be verifying all uses of
    // the candidate, which will ensure that the candidate is not escaped to an
    // external routine.

    AU.addRequired<LoopInfoWrapperPass>();

    // The swapping of the stride values in the dope vectors and
    // llvm.intel.subscript intrinsic call should not invalidate any analysis.
    AU.setPreservesAll();
  }
};

} // end anonymous namespace

char DTransTransposeWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransTransposeWrapper, "dtrans-transpose",
                      "DTrans multi-dimensional array transpose for Fortran",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(DTransTransposeWrapper, "dtrans-transpose",
                    "DTrans multi-dimensional array transpose for Fortran",
                    false, false)

ModulePass *llvm::createDTransTransposeWrapperPass() {
  return new DTransTransposeWrapper();
}

namespace llvm {

namespace dtrans {

PreservedAnalyses TransposePass::run(Module &M, ModuleAnalysisManager &AM) {
  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  LoopInfoFuncType GetLI = [&FAM](Function &F) -> LoopInfo & {
    return FAM.getResult<LoopAnalysis>(F);
  };

  runImpl(M, GetLI);

  // The swapping of the stride values in the dope vectors and
  // llvm.intel.subscript intrinsic call should not invalidate any analysis.
  return PreservedAnalyses::all();
}

bool TransposePass::runImpl(Module &M,
                            std::function<LoopInfo &(Function &)> &GetLI) {
  TransposeImpl Transpose(GetLI);
  return Transpose.run(M);
}

} // end namespace dtrans
} // end namespace llvm
