//=--------------------- VectorVariantLowering.cpp -*- C++ -*----------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/SYCLTransforms/Intel_VectorVariant/VectorVariantLowering.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"

#define DEBUG_TYPE "sycl-kernel-vector-variant-lowering"

using namespace llvm;

extern bool SYCLEnableVectorVariantPasses;

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
//   attributes #0 = \
//   { "vector-variants"="_ZGV_unknown_N0lu_XXX,_ZGV_unknown_M0vv_XXX" }
//
// After the pass:
//
//   attributes #0 = { "vector-variants"="_ZGVdN0lu_XXX,_ZGVdM0vv_XXX" }
//
bool VectorVariantLowering::runImpl(Module &M, CallGraph &CG) {
  if (!SYCLEnableVectorVariantPasses)
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
            Node.getKindAsString() != VectorUtils::VectorVariantsAttrName)
          continue;

        SmallVector<StringRef, 4> Variants;
        SmallVector<std::string, 4> NewVariants;
        Node.getValueAsString().split(Variants, ",");

        // Fix ISA for vector-variants attribute.
        bool UnknownFound = false;
        for (unsigned I = 0; I < Variants.size(); I++) {
          VFInfo Variant = VFABI::demangleForVFABI(Variants[I]);

          if (Variant.getISA() == VFISAKind::Unknown) {
            Variant.setISA(ISA);
            UnknownFound = true;
          }

          NewVariants.push_back(std::move(Variant.VectorName));
        }
        if (!UnknownFound)
          continue;

        // Update attributes.
        Attrs = Attrs
                    .removeAttribute(M.getContext(), Index,
                                     VectorUtils::VectorVariantsAttrName)
                    .addAttributeAtIndex(M.getContext(), Index,
                                         VectorUtils::VectorVariantsAttrName,
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
