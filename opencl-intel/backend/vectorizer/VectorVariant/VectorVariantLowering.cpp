//=--------------------- VectorVariantLowering.cpp -*- C++ -*----------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "VectorVariantLowering.h"
#include "VectorizerCommon.h"

#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/Intel_VectorVariant.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#define DEBUG_TYPE "VectorVariantLowering"

using namespace llvm;
using namespace Intel::VectorizerCommon;

extern bool EnableVectorVariantPasses;

namespace intel {

char VectorVariantLowering::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(VectorVariantLowering, "vector-variant-lowering",
                          "Lowering vector-variant attributes", false, false)
OCL_INITIALIZE_PASS_END(VectorVariantLowering, "vector-variant-lowering",
                        "Lowering vector-variant attributes", false, false)

VectorVariantLowering::VectorVariantLowering(
    const Intel::OpenCL::Utils::CPUDetect *CPUId)
    : ModulePass(ID), CPUId(CPUId) {
  initializeVectorVariantLoweringPass(*PassRegistry::getPassRegistry());
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
bool VectorVariantLowering::runOnModule(Module &M) {
  if (!EnableVectorVariantPasses)
    return false;

  bool Modified = false;

  // Process all call instructions.
  for (auto &F : M)
    for (auto &Inst : instructions(F)) {
      if (Inst.getOpcode() != Instruction::Call)
        continue;

      CallInst &Call = cast<CallInst>(Inst);
      AttributeList Attrs = Call.getAttributes();

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
          VectorVariant Variant(Variants[I]);

          if (Variant.getISA() == VectorVariant::OTHER) {
            Variant.setISA(getCPUIdISA(CPUId));
          }

          NewVariants.push_back(Variant.toString());
        }

        // Update attributes.
        Attrs = Attrs.removeAttribute(M.getContext(), Index, "vector-variants")
                    .addAttribute(M.getContext(), Index, "vector-variants",
                                  join(NewVariants, ","));
        AttrsModified = true;
      }

      // Set new attributes if it is updated.
      if (AttrsModified) {
        Call.setAttributes(Attrs);
        Modified = true;
      }
    }

  return Modified;
}

extern "C" {
ModulePass *
createVectorVariantLoweringPass(const Intel::OpenCL::Utils::CPUDetect *CPUId) {
  return new intel::VectorVariantLowering(CPUId);
}
}

} // namespace intel
