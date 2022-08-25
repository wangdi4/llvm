//=---------------------- IndirectCallLowering.cpp -*- C++ -*----------------=//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VectorVariant/IndirectCallLowering.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"

#define DEBUG_TYPE "IndirectCallLowering"

using namespace llvm;

extern bool DPCPPEnableVectorVariantPasses;

namespace llvm {

PreservedAnalyses IndirectCallLowering::run(Module &M,
                                            ModuleAnalysisManager &MAM) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

bool IndirectCallLowering::runImpl(Module &M) {
  if (!DPCPPEnableVectorVariantPasses)
    return false;

  bool Modified = false;

  SmallVector<CallInst *> RemoveLater;

  // Process all call instructions.
  for (auto &Fn : M) {
    for (auto &Inst : instructions(Fn)) {
      if (Inst.getOpcode() != Instruction::Call)
        continue;

      CallInst &Call = cast<CallInst>(Inst);
      Function *F = Call.getCalledFunction();

      // We are interested in indirect calls only.
      if (!F || !F->getName().startswith("__intel_indirect_call"))
        continue;

      assert(
          Call.getAttributes().hasFnAttr(KernelAttribute::VectorVariants) &&
          "Vector variants should always be set for this call");

      SmallVector<StringRef, 4> Variants;
      Attribute Attr =
          Call.getCallSiteOrFuncAttr(KernelAttribute::VectorVariants);
      Attr.getValueAsString().split(Variants, ",");
      assert(!Variants.empty() &&
             "Expected non-empty vector-variants attribute");

      // Look for the first masked variant.
      unsigned Index;
      for (Index = 0; Index < Variants.size(); Index++)
        if (VFABI::demangleForVFABI(Variants[Index]).isMasked())
          break;

      // We expect here at least one masked vector-variant.
      // In other case code generation can't be done correctly.
      if (Index >= Variants.size()) {
        // Final function's code will be incorrect, so let's mark it as
        // an invalid one.
        Fn.addFnAttr(KernelAttribute::VectorVariantFailure,
                     "failed to find a masked vector variant for an indirect call");
        continue;
      }

      VFInfo Variant = VFABI::demangleForVFABI(Variants[Index]);
      unsigned VecLen = Variant.getVF();
      ConstantInt *Zero = ConstantInt::get(M.getContext(), APInt(32, 0, true));
      FunctionType *FTy = Call.getFunctionType();

      IRBuilder<> Builder(&Call);

      // Preparing vector arguments. Starting from the second operand because we
      // need to skip the first function-pointer operand.
      SmallVector<Type *, 16> VecArgTy;
      SmallVector<Value *, 16> VecArgs;
      for (unsigned I = 1; I < FTy->getNumParams(); I++) {

        Type *Ty = FTy->getParamType(I);
        Value *Operand = Call.getArgOperand(I);

        if (Variant.Shape.Parameters[I - 1].isVector()) {

          VectorType *VecTy = VectorType::get(Ty, VecLen, false);
          VecArgTy.push_back(VecTy);

          Value *VecArg = Builder.CreateInsertElement(UndefValue::get(VecTy),
                                                      Operand, Zero);
          VecArgs.push_back(VecArg);

        } else {

          VecArgTy.push_back(Ty);
          VecArgs.push_back(Operand);
        }
      }

      // Preparing the mask.
      Value *Zeros = Constant::getNullValue(
          FixedVectorType::get(Type::getInt1Ty(F->getContext()), VecLen));
      Value *MaskToUse =
          Builder.CreateInsertElement(Zeros, Builder.getInt1(1), Zero);

      // Drop the first arg due to it's a function pointer.
      auto Args = llvm::drop_begin(Call.args(), 1);
      Type *CharacteristicType = llvm::calcCharacteristicType(
          Call.getType(),
          map_range(Args, [](Use &U) -> Value & { return *U.get(); }), Variant,
          Call.getParent()->getParent()->getParent()->getDataLayout());

      createVectorMaskArg(Builder, CharacteristicType, &Variant, VecArgs,
                          VecArgTy, VecLen, MaskToUse);

      // Preparing chosen variant call.
      Type *RetTy = FTy->getReturnType();
      Type *VecRetTy =
          RetTy->isVoidTy() ? RetTy : VectorType::get(RetTy, VecLen, false);

      Value *GV = Call.getArgOperand(0);
      PointerType *GVTy = cast<PointerType>(GV->getType());
      unsigned AS = GVTy->getAddressSpace();

      FunctionType *VecFTy = FunctionType::get(VecRetTy, VecArgTy, false);
      PointerType *VecFPtrTy = PointerType::get(VecFTy, AS);
      PointerType *VecFPtrPtrTy = PointerType::get(VecFPtrTy, AS);

      Value *Table = Builder.CreateZExtOrBitCast(GV, VecFPtrPtrTy);
      Value *FPtrPtr = Builder.CreateGEP(
          VecFPtrTy, Table,
          ConstantInt::get(M.getContext(), APInt(32, Index, true)));
      Value *FPtr = Builder.CreateLoad(VecFPtrTy, FPtrPtr);
      Value *VecRes = Builder.CreateCall(VecFTy, FPtr, VecArgs);

      if (!RetTy->isVoidTy()) {
        Value *Res = Builder.CreateExtractElement(VecRes, Zero);
        Call.replaceAllUsesWith(Res);
      }

      RemoveLater.push_back(&Call);

      Modified = true;
    }
  }

  for (CallInst *Call : RemoveLater)
    Call->eraseFromParent();

  return Modified;
}

} // namespace llvm

// For legacy pass manager
namespace {
class IndirectCallLoweringLegacy : public llvm::ModulePass {
public:
  static char ID;

  IndirectCallLoweringLegacy() : ModulePass(ID) {
    initializeIndirectCallLoweringLegacyPass(*PassRegistry::getPassRegistry());
  }

protected:
  bool runOnModule(llvm::Module &M) override {
    return IndirectCallLowering().runImpl(M);
  }
};
} // namespace

INITIALIZE_PASS(IndirectCallLoweringLegacy,
                "dpcpp-kernel-indirect-call-lowering",
                "Lowering __intel_indirect_call scalar calls", false, false)

char IndirectCallLoweringLegacy::ID = 0;

ModulePass *llvm::createIndirectCallLoweringLegacyPass() {
  return new IndirectCallLoweringLegacy();
}
