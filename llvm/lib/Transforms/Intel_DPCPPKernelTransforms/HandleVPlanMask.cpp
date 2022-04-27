// ===------------------ HandleVPlanMask.cpp ------------ -*- C++ -*------=== //
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/HandleVPlanMask.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"

using namespace llvm;

HandleVPlanMask::HandleVPlanMask(const StringSet<> *VPlanMaskedFuncs)
    : VPlanMaskedFuncs(VPlanMaskedFuncs) {}

PreservedAnalyses HandleVPlanMask::run(Module &M, ModuleAnalysisManager &MAM) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

bool HandleVPlanMask::runImpl(Module &M) {

  auto hasVPlanMask = [this](const Function &F) -> bool {
    // The TableGen-generated list of function names using VPlan-fashioned
    // masks, AKA, using characteristic data type as the element type of mask
    // args. This is a workaround migrating from Volcano to VPlan, all masked
    // functions should be using VPlan-fashioned masks when we deprecate Volcano
    // completely.
    if (VPlanMaskedFuncs)
      return VPlanMaskedFuncs->count(F.getName());
    return false;
  };

  Type *Int32Type = IntegerType::get(M.getContext(), 32);
  SmallSet<Function *, 4> FuncsToRemove;
  SmallSet<Function *, 8> FuncsNeedRemovingAttrs;

  for (auto &F : M) {
    if (!F.isDeclaration() || (F.arg_size() < 1) || F.user_empty() ||
        hasVPlanMask(F))
      continue;

    // All users of built-in functions should be CallInst.
    CallInst *Call = dyn_cast<CallInst>(*F.user_begin());
    if (!Call)
      continue;

    // Check "has_vplan_mask" and "call_params_num" attr to decide
    // whether the call is masked.
    bool HasVPlanMask = Call->hasFnAttr(KernelAttribute::HasVPlanMask);
    if (!HasVPlanMask) {
      Attribute Attr = Call->getFnAttr(KernelAttribute::CallParamNum);
      if (Attr.isValid()) {
        FuncsNeedRemovingAttrs.insert(&F);
        StringRef AttrStr = Attr.getValueAsString();
        int ArgSize;
        bool Invalid = AttrStr.getAsInteger<int>(10, ArgSize);
        (void)Invalid;
        assert(!Invalid && "call-params-num attribute shouldn't be 0");
        HasVPlanMask = Call->arg_size() - 1 == (unsigned)ArgSize;
      }
    } else {
      FuncsNeedRemovingAttrs.insert(&F);
    }
    if (!HasVPlanMask)
      continue;

    // Check the type of mask parameter.
    FunctionType *FnType = F.getFunctionType();
    unsigned LastArgIdx = F.arg_size() - 1;
    auto *MaskType = cast<FixedVectorType>(FnType->getParamType(LastArgIdx));
    unsigned VF = MaskType->getNumElements();
    auto *ExpectMaskType = FixedVectorType::get(Int32Type, VF);
    auto *MaskElementType = dyn_cast<IntegerType>(MaskType->getElementType());

    // VPlan uses characteristic data type as the element type of mask arg.
    // So the element type may be non-int.
    VectorType *IntMaskType = nullptr;
    if (!MaskElementType) {
      IntMaskType = VectorType::getInteger(MaskType);
      MaskElementType = cast<IntegerType>(IntMaskType->getElementType());
    }
    unsigned ElemBitWidth = MaskElementType->getBitWidth();
    // The mask type is already <VF x i32>, skip this function.
    if (ElemBitWidth == 32 && !IntMaskType)
      continue;

    // Create a new function with expected mask type.
    std::string FnName = F.getName().str(); // Don't use StringRef
    F.setName(FnName + ".before");
    SmallVector<Type *> NewParamTypes;
    for (unsigned idx = 0; idx < LastArgIdx; ++idx)
      NewParamTypes.push_back(FnType->getParamType(idx));
    NewParamTypes.push_back(ExpectMaskType);
    auto *NewFnType = FunctionType::get(FnType->getReturnType(), NewParamTypes,
                                        FnType->isVarArg());
    auto *NewFn = Function::Create(NewFnType, F.getLinkage(), FnName, M);
    NewFn->setAttributes(F.getAttributes());
    FuncsToRemove.insert(&F);
    FuncsNeedRemovingAttrs.erase(&F);
    FuncsNeedRemovingAttrs.insert(NewFn);

    IRBuilder<> IRB(Call);

    // Generate new mask arg for all calls.
    DenseMap<CallInst *, Value *> NewMaskForCalls;
    for (auto *User : F.users()) {
      Call = cast<CallInst>(User);
      IRB.SetInsertPoint(Call);
      Value *MaskArg = Call->getArgOperand(LastArgIdx);
      // Cast the mask arg to int vector type;
      if (IntMaskType)
        MaskArg = IRB.CreateBitCast(MaskArg, IntMaskType, "mask.cast.i.");

      // For i8, i16, i64, extend or trunc the mask arg to <VF x i32>.
      if (ElemBitWidth != 32)
        MaskArg = IRB.CreateSExtOrTrunc(MaskArg, ExpectMaskType, "mask.i32.");

      NewMaskForCalls[Call] = MaskArg;
    }

    // Set called function and mask operand.
    for (auto &MaskMap : NewMaskForCalls) {
      (MaskMap.first)->setCalledFunction(NewFn);
      (MaskMap.first)->setArgOperand(LastArgIdx, MaskMap.second);
    }
  }

  for (auto *Func : FuncsNeedRemovingAttrs) {
    for (auto *User : Func->users()) {
      auto *Call = dyn_cast<CallInst>(User);
      assert(Call && "Unexpected use of OpenCL Built-ins");
      Call->removeFnAttr(KernelAttribute::HasVPlanMask);
      Call->removeFnAttr(KernelAttribute::CallParamNum);
    }
  }

  for (auto *Func : FuncsToRemove) {
    assert(Func->user_empty() && "Unexpected use of OpenCL Built-ins");
    Func->eraseFromParent();
  }
  return !(FuncsToRemove.empty() && FuncsNeedRemovingAttrs.empty());
}

// For legacy PM
namespace {
class HandleVPlanMaskLegacy : public ModulePass {
public:
  static char ID;

  HandleVPlanMaskLegacy(const StringSet<> *VPlanMaskedFuncs = nullptr)
      : ModulePass(ID), VPlanMaskedFuncs(VPlanMaskedFuncs) {
    initializeHandleVPlanMaskLegacyPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    return HandleVPlanMask(VPlanMaskedFuncs).runImpl(M);
  }

private:
  const StringSet<> *VPlanMaskedFuncs;
};
} // namespace

char HandleVPlanMaskLegacy::ID = 0;

INITIALIZE_PASS(
    HandleVPlanMaskLegacy, "dpcpp-kernel-convert-vplan-mask",
    "HandleVPlanMask pass - convert vplan style mask to volcano style", false,
    false)

ModulePass *
llvm::createHandleVPlanMaskLegacyPass(const StringSet<> *VPlanMaskedFuncs) {
  return new HandleVPlanMaskLegacy(VPlanMaskedFuncs);
}
