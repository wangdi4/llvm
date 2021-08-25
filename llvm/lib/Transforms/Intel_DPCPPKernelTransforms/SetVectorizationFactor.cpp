//==-- SetVectorizationFactor.cpp - Set vectorization factor ------ C++ -*-==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===-------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SetVectorizationFactor.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/Intel_VectorVariant.h"
#include "llvm/InitializePasses.h"
#include "llvm/PassRegistry.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-set-vf"

namespace {

/// Legacy SetVectorizationFactor pass.
class SetVectorizationFactorLegacy : public FunctionPass {
  SetVectorizationFactorPass Impl;

public:
  static char ID;

  SetVectorizationFactorLegacy(VectorVariant::ISAClass ISA = VectorVariant::XMM)
      : FunctionPass(ID), Impl(ISA) {
    initializeSetVectorizationFactorLegacyPass(
        *PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "SetVectorizationFactorLegacy";
  }

  bool runOnFunction(Function &F) override { return Impl.runImpl(F); }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<CallGraphWrapperPass>();
    AU.setPreservesCFG();
  }
};
} // namespace

char SetVectorizationFactorLegacy::ID = 0;

INITIALIZE_PASS(SetVectorizationFactorLegacy, DEBUG_TYPE,
                "Set VF metadata for each function", false, false)

FunctionPass *
llvm::createSetVectorizationFactorLegacyPass(VectorVariant::ISAClass ISA) {
  return new SetVectorizationFactorLegacy(ISA);
}

extern cl::opt<VectorVariant::ISAClass> IsaEncodingOverride;
SetVectorizationFactorPass::SetVectorizationFactorPass(
    VectorVariant::ISAClass ISA)
    : ISA(ISA) {
  if (IsaEncodingOverride.getNumOccurrences())
    this->ISA = IsaEncodingOverride.getValue();
}

/// Get preferred vec width according to ISA.
/// TODO: Future heuristics could be implemented here.
static unsigned getPreferredVectorizationWidth(VectorVariant::ISAClass ISA) {
  switch (ISA) {
  case VectorVariant::XMM:
  case VectorVariant::YMM1:
    return 4;
  case VectorVariant::YMM2:
    return 8;
  case VectorVariant::ZMM:
    return 16;
  default:
    llvm_unreachable("unexpected ISA");
  }
}

bool SetVectorizationFactorPass::runImpl(Function &F) {
  DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(&F);
  if (KIMD.RecommendedVL.hasValue())
    return false;
  // Set "recommended_vector_length" metadata.
  unsigned VF = getPreferredVectorizationWidth(ISA);
  KIMD.RecommendedVL.set(VF);
  LLVM_DEBUG(dbgs() << "Set VF=" << VF << " for function " << F.getName()
                    << '\n');
  return true;
}

PreservedAnalyses SetVectorizationFactorPass::run(Function &F,
                                                  FunctionAnalysisManager &) {
  if (!runImpl(F))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  PA.preserve<CallGraphAnalysis>();
  return PA;
}
