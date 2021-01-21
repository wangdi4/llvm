//=-------------------- SGSizeCollectorIndirect.cpp -*- C++ -*---------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "SGSizeCollectorIndirect.h"

#include "llvm/Analysis/Intel_VectorVariant.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#define DEBUG_TYPE "SGSizeCollectorIndirect"

using namespace llvm;

bool EnableVectorVariantPasses = true;
static cl::opt<bool, true> EnableVectorVariantPassesOpt(
  "enable-vector-variant-passes", cl::location(EnableVectorVariantPasses),
  cl::Hidden,
  cl::desc(
    "Enable vector-variant/vector_function_ptrs attributes processing."));

namespace intel {

char SGSizeCollectorIndirect::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(
    SGSizeCollectorIndirect, "sg-size-collector-indirect",
    "Collecting subgroup size information for indirect calls", false, false)
OCL_INITIALIZE_PASS_END(
    SGSizeCollectorIndirect, "sg-size-collector-indirect",
    "Collecting subgroup size information for indirect calls", false, false)

SGSizeCollectorIndirect::SGSizeCollectorIndirect(
    const Intel::OpenCL::Utils::CPUDetect *CPUId)
    : ModulePass(ID), Impl(CPUId) {
  initializeSGSizeCollectorIndirectPass(*PassRegistry::getPassRegistry());
}

bool SGSizeCollectorIndirect::runOnModule(Module &M) { return Impl.runImpl(M); }

SGSizeCollectorIndirectImpl::SGSizeCollectorIndirectImpl(
    const Intel::OpenCL::Utils::CPUDetect *CPUId)
    : SGSizeCollectorImpl(CPUId) {}

// This pass collects vector lengths from all existing functions and then
// updates vector_function_ptrs attributes and adds vector-variants attribute
// for indirect call instructions.
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
bool SGSizeCollectorIndirectImpl::runImpl(Module &M) {
  if (!EnableVectorVariantPasses)
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

  VectorVariant::ISAClass ISAClass = getCPUIdISA();
  auto GenerateVectorVariants =
      [&VecLengths, ISAClass](const std::string &FuncName, int NumParams) {
        std::vector<VectorKind> Parameters(NumParams, VectorKind::vector());
        SmallVector<std::string, 4> Variants;
        for (int VecLength : VecLengths) {
          VectorVariant VariantMasked(ISAClass, true, VecLength, Parameters,
                                      FuncName, "");
          VectorVariant VariantUnmasked(ISAClass, false, VecLength, Parameters,
                                        FuncName, "");
          Variants.push_back(VariantMasked.toString());
          Variants.push_back(VariantUnmasked.toString());
        }
        return join(Variants, ",");
      };

  for (auto &Fn : M) {
    if (Fn.hasFnAttribute("vector_function_ptrs")) {
      Attribute Attr = Fn.getFnAttribute("vector_function_ptrs");

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
        Fn.removeFnAttr("vector_function_ptrs");
        Fn.addFnAttr("vector_function_ptrs", join(Strs, ","));
        Fn.addFnAttr("vector-variants", join(VectorVariants, ","));
        Modified = true;
      }
    }

    // Process all call instructions.
    for (auto &Inst : instructions(Fn)) {
      if (Inst.getOpcode() != Instruction::Call)
        continue;

      CallInst &Call = cast<CallInst>(Inst);
      Function *F = Call.getCalledFunction();

      // We are interested in indirect calls only.
      if (!F || !F->getName().startswith("__intel_indirect_call"))
        continue;

      AttributeList Attrs = Call.getAttributes();
      if (Attrs.hasAttribute(AttributeList::FunctionIndex, "vector-variants"))
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
      Attrs = Attrs.addAttribute(
          M.getContext(), AttributeList::FunctionIndex, "vector-variants",
          GenerateVectorVariants("__intel_indirect_call_XXX",
                                 FuncTy->getNumParams()));
      Call.setAttributes(Attrs);

      Modified = true;
    }
  }

  return Modified;
}

extern "C" {
ModulePass *createSGSizeCollectorIndirectPass(
    const Intel::OpenCL::Utils::CPUDetect *CPUId) {
  return new intel::SGSizeCollectorIndirect(CPUId);
}
}

} // namespace intel
