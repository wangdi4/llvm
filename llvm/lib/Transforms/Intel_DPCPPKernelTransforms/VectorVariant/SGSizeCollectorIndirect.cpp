//=-------------------- SGSizeCollectorIndirect.cpp -*- C++ -*---------------=//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VectorVariant/SGSizeCollectorIndirect.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace llvm;
using namespace llvm::DPCPPKernelMetadataAPI;

extern bool DPCPPEnableVectorVariantPasses;

static bool hasVecLength(Function *F, int &VecLength) {
  auto KIMD = KernelInternalMetadataAPI(F);
  if (KIMD.RecommendedVL.hasValue()) {
    VecLength = KIMD.RecommendedVL.get();
    return VecLength > 1;
  }
  return false;
}

// This pass collects vector lengths from all existing functions and then
// updates vector_function_ptrs attributes and adds vector-variants attribute
// for indirect call instructions.
//
// clang-format off
//
// Example:
//
//   define dso_local i32 @foo() !intel_reqd_sub_group_size !0 {
//   entry:
//     call i32 (i32 (i32, float)**, ...) @__intel_indirect_call_i32_p0p0f_i32i32f32f(i32 (i32, float)** %call.i, i32 5, float 2.000000e+00)
//   }
//   !0 = !{i32 8}
//
// After the pass:
//
//   call i32 (i32 (i32, float)**, ...) @__intel_indirect_call_i32_p0p0f_i32i32f32f(i32 (i32, float)** %call.i, i32 5, float 2.000000e+00) #0
//   attributes #0 = { "vector-variants"="_ZGVbM8vv_XXX,_ZGVbN8vv_XXX" }
//
// clang-format on
//
bool SGSizeCollectorIndirectPass::runImpl(Module &M, CallGraph &CG) {
  if (!DPCPPEnableVectorVariantPasses)
    return false;

  bool Modified = false;

  // Collect all possible vector lengths.
  DenseSet<int> VecLengths;
  for (auto &Fn : M) {
    int VecLength;
    if (!hasVecLength(&Fn, VecLength))
      continue;
    VecLengths.insert(VecLength);
  }

  auto GenerateVectorVariants = [&VecLengths, this](const std::string &FuncName,
                                                    int NumParams) {
    std::vector<VFParamKind> ParamKinds(NumParams, VFParamKind::Vector);
    SmallVector<std::string, 4> Variants;
    for (int VecLength : VecLengths) {
      auto VariantMasked = VFInfo::get(ISA, true, VecLength, ParamKinds, FuncName);
      auto VariantUnmasked = VFInfo::get(ISA, false, VecLength, ParamKinds, FuncName);
      Variants.push_back(VariantMasked.VectorName);
      Variants.push_back(VariantUnmasked.VectorName);
    }
    return join(Variants, ",");
  };

  for (auto &Fn : M) {
    Attribute Attr = Fn.getFnAttribute(Attribute::VectorFunctionPtrsStrAttr);
    if (Attr.isValid()) {
      SmallVector<StringRef, 4> Ptrs;
      Attr.getValueAsString().split(Ptrs, ",");

      SmallVector<std::string, 4> VectorVariants;
      SmallVector<std::string, 4> Strs;
      for (StringRef Str : Ptrs)
        Strs.push_back(Str.str());

      bool PtrsModified = false;
      for (size_t I = 0; I < Strs.size(); I++) {
        StringRef Var = Strs[I];

        size_t PosBegin = Var.find('(');
        size_t PosEnd = Var.find(')');
        assert(PosBegin != StringRef::npos && PosEnd != StringRef::npos &&
               "Expecting brackets in vector_function_ptrs attribute");
        assert(PosEnd == PosBegin + 1 &&
               "Expecting empty brackets in vector_function_ptrs attribute");

        Var = Var.slice(0, PosBegin);
        (void)PosEnd;

        // Update the attribute with the default vector attributes set.
        GlobalVariable *GV = M.getGlobalVariable(Var);
        if (GV && GV->hasInitializer()) {
          Constant *Initializer = GV->getInitializer();
          Constant *El = Initializer->getAggregateElement(0u);
          Function *F = cast<Function>(El);

          std::string Variants = GenerateVectorVariants(
              F->getName().str(), F->getFunctionType()->getNumParams());

          Strs[I] = (Var + "(" + Variants + ")").str();
          VectorVariants.push_back(Variants);

          PtrsModified = true;
        }
      }

      if (PtrsModified) {
        Fn.removeFnAttr(Attribute::VectorFunctionPtrsStrAttr);
        Fn.addFnAttr(Attribute::VectorFunctionPtrsStrAttr, join(Strs, ","));
        Fn.addFnAttr("vector-variants", join(VectorVariants, ","));
        Modified = true;
      }
    }

    // Process all call instructions.
    if (Fn.isDeclaration())
      continue;
    for (auto &N : *CG[&Fn]) {
      auto &Call = *cast<CallInst>(*N.first);
      Function *F = Call.getCalledFunction();

      // We are interested in indirect calls only.
      if (!F || !F->getName().startswith("__intel_indirect_call"))
        continue;

      AttributeList Attrs = Call.getAttributes();
      if (Attrs.hasFnAttr("vector-variants"))
        continue;

      // Add vector-variants attribute.
      FunctionType *FuncTy = Call.getFunctionType();
      assert(FuncTy->getNumParams() > 0 &&
             "Expected at least one function argument");

      Type *ParamTy = FuncTy->getParamType(0);
      PointerType *Pointer = cast<PointerType>(ParamTy);
      Pointer = cast<PointerType>(Pointer->getElementType());
      FuncTy = cast<FunctionType>(Pointer->getElementType());

      // Update attributes.
      Attrs = Attrs.addFnAttribute(
          M.getContext(), "vector-variants",
          GenerateVectorVariants("__intel_indirect_call_XXX",
                                 FuncTy->getNumParams()));
      Call.setAttributes(Attrs);

      Modified = true;
    }
  }

  return Modified;
}

PreservedAnalyses SGSizeCollectorIndirectPass::run(Module &M,
                                                   ModuleAnalysisManager &MAM) {
  CallGraph &CG = MAM.getResult<CallGraphAnalysis>(M);
  if (!runImpl(M, CG))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<CallGraphAnalysis>();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

// For legacy pass manager
namespace {
class SGSizeCollectorIndirectLegacy : public ModulePass {
public:
  static char ID;

  SGSizeCollectorIndirectLegacy(VFISAKind ISA = VFISAKind::SSE);

  StringRef getPassName() const override {
    return "SGSizeCollectorIndirectLegacy";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<CallGraphWrapperPass>();
    AU.addPreserved<CallGraphWrapperPass>();
    AU.setPreservesCFG();
  }

protected:
  bool runOnModule(Module &M) override;

private:
  VFISAKind ISA;
};
} // namespace

char SGSizeCollectorIndirectLegacy::ID = 0;

bool SGSizeCollectorIndirectLegacy::runOnModule(Module &M) {
  CallGraph &CG = getAnalysis<CallGraphWrapperPass>().getCallGraph();
  return SGSizeCollectorIndirectPass(ISA).runImpl(M, CG);
}

SGSizeCollectorIndirectLegacy::SGSizeCollectorIndirectLegacy(
    VFISAKind ISA)
    : ModulePass(ID), ISA(ISA) {
  initializeSGSizeCollectorIndirectLegacyPass(*PassRegistry::getPassRegistry());
}

INITIALIZE_PASS_BEGIN(SGSizeCollectorIndirectLegacy,
                      "dpcpp-kernel-sg-size-collector-indirect",
                      "Collecting subgroup size information for indirect calls",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_END(SGSizeCollectorIndirectLegacy,
                    "dpcpp-kernel-sg-size-collector-indirect",
                    "Collecting subgroup size information for indirect calls",
                    false, false)

ModulePass *
llvm::createSGSizeCollectorIndirectLegacyPass(VFISAKind ISA) {
  return new SGSizeCollectorIndirectLegacy(ISA);
}
