//=--------------------- VectorVariantLowering.cpp -*- C++ -*----------------=//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VectorVariant/VectorVariantLowering.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"

#define DEBUG_TYPE "VectorVariantLowering"

using namespace llvm;

extern bool DPCPPEnableVectorVariantPasses;

PreservedAnalyses VectorVariantLowering::run(Module &M,
                                             ModuleAnalysisManager &MAM) {
  CallGraph &CG = MAM.getResult<CallGraphAnalysis>(M);
  if (!runImpl(M, CG))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<CallGraphAnalysis>();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

// This pass updates vector-variants attributes accordingly to the current ISA.
//
// Example:
//
//   define void @foo(i32 %i, float %f) {
//   entry:
//     call void @bar(i32 %i, float %f) #0
//     ret void
//   }
//   attributes #0 = { "vector-variants"="_ZGVxN0lu_XXX,_ZGVxM0vv_XXX" }
//
// After the pass:
//
//   attributes #0 = { "vector-variants"="_ZGVdN0lu_XXX,_ZGVdM0vv_XXX" }
//
bool VectorVariantLowering::runImpl(Module &M, CallGraph &CG) {
  if (!DPCPPEnableVectorVariantPasses)
    return false;

  bool Modified = false;

  // Process all call instructions.
  for (auto &F : M) {
    if (F.hasOptNone() || F.isDeclaration())
      continue;
    for (auto &N : *CG[&F]) {
      auto Call = cast<CallInst>(*N.first);
      AttributeList Attrs = Call->getAttributes();

      unsigned Index = AttributeList::FunctionIndex;
      AttributeSet Set = Attrs.getAttributes(Index);

      // Look for vector-variants attribute.
      bool AttrsModified = false;
      for (auto &Node : Set) {
        if (!Node.isStringAttribute() ||
            Node.getKindAsString() != "vector-variants")
          continue;

        SmallVector<StringRef, 4> Variants;
        SmallVector<std::string, 4> NewVariants;
        Node.getValueAsString().split(Variants, ",");

        // Fix ISA for vector-variants attribute.
        for (unsigned I = 0; I < Variants.size(); I++) {
          VFInfo Variant = VFABI::demangleForVFABI(Variants[I]);

          if (Variant.getISA() == VFISAKind::Unknown) {
            Variant.setISA(ISA);
          }

          NewVariants.push_back(std::move(Variant.VectorName));
        }

        // Update attributes.
        Attrs = Attrs.removeAttribute(M.getContext(), Index, "vector-variants")
                    .addAttributeAtIndex(M.getContext(), Index, "vector-variants",
                                         join(NewVariants, ","));
        AttrsModified = true;
      }

      // Set new attributes if it is updated.
      if (AttrsModified) {
        Call->setAttributes(Attrs);
        Modified = true;
      }
    }
  }
  return Modified;
}

// For legacy pass manager
namespace {
class VectorVariantLoweringLegacy : public ModulePass {
public:
  static char ID;

  VectorVariantLoweringLegacy(VFISAKind ISA = VFISAKind::SSE);

  llvm::StringRef getPassName() const override {
    return "VectorVariantLoweringLegacy";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<CallGraphWrapperPass>();
    AU.addPreserved<CallGraphWrapperPass>();
    AU.setPreservesCFG();
  }

protected:
  bool runOnModule(llvm::Module &M) override;

private:
  VFISAKind ISA;
};

char VectorVariantLoweringLegacy::ID = 0;

VectorVariantLoweringLegacy::VectorVariantLoweringLegacy(
    VFISAKind ISA)
    : ModulePass(ID), ISA(ISA) {
  initializeVectorVariantLoweringLegacyPass(*PassRegistry::getPassRegistry());
}

bool VectorVariantLoweringLegacy::runOnModule(Module &M) {
  CallGraph &CG = getAnalysis<CallGraphWrapperPass>().getCallGraph();
  return VectorVariantLowering(ISA).runImpl(M, CG);
}

} // namespace

INITIALIZE_PASS_BEGIN(VectorVariantLoweringLegacy,
                "dpcpp-kernel-vector-variant-lowering",
                "Lowering vector-variant attributes", false, false)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_END(VectorVariantLoweringLegacy,
                "dpcpp-kernel-vector-variant-lowering",
                "Lowering vector-variant attributes", false, false)

ModulePass *
llvm::createVectorVariantLoweringLegacyPass(VFISAKind ISA) {
  return new VectorVariantLoweringLegacy(ISA);
}
