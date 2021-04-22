//=---------------------- IndirectCallLowering.cpp -*- C++ -*----------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "IndirectCallLowering.h"
#include "CompilationUtils.h"

#include "llvm/Analysis/Intel_VectorVariant.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"

#define DEBUG_TYPE "IndirectCallLowering"

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

extern bool EnableVectorVariantPasses;

namespace intel {

char IndirectCallLowering::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(IndirectCallLowering, "indirect-call-lowering",
                          "Lowering __intel_indirect_call scalar calls", false,
                          false)
OCL_INITIALIZE_PASS_END(IndirectCallLowering, "indirect-call-lowering",
                        "Lowering __intel_indirect_call scalar calls", false,
                        false)

IndirectCallLowering::IndirectCallLowering() : ModulePass(ID) {
  initializeIndirectCallLoweringPass(*PassRegistry::getPassRegistry());
}

bool IndirectCallLowering::runOnModule(Module &M) {
  if (!EnableVectorVariantPasses)
    return false;

  bool Modified = false;

  DenseSet<CallInst *> RemoveLater;

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

      assert(Call.getAttributes().hasAttribute(AttributeList::FunctionIndex,
                                               "vector-variants") &&
             "Vector variants should always be set for this call");

      SmallVector<StringRef, 4> Variants;
      Attribute Attr =
          Call.getAttribute(AttributeList::FunctionIndex, "vector-variants");
      Attr.getValueAsString().split(Variants, ",");
      assert(!Variants.empty() &&
             "Expected non-empty vector-variants attribute");

      // Look for the first masked variant.
      unsigned Index;
      for (Index = 0; Index < Variants.size(); Index++)
        if (VectorVariant(Variants[Index]).isMasked())
          break;

      // We expect here at least one masked vector-variant.
      // In other case code generation can't be done correctly.
      if (Index >= Variants.size()) {
        // Final function's code will be incorrect, so let's mark it as
        // an invalid one.
        Fn.addFnAttr(CompilationUtils::ATTR_VECTOR_VARIANT_FAILURE,
                     CompilationUtils::ATTR_VALUE_FAILED_TO_LOWER_INDIRECT_CALL);
        continue;
      }

      VectorVariant Variant(Variants[Index]);
      unsigned VecLen = Variant.getVlen();
      ConstantInt *Zero = ConstantInt::get(M.getContext(), APInt(32, 0, true));
      ConstantInt *One = ConstantInt::get(M.getContext(), APInt(32, 1, true));
      FunctionType *FTy = Call.getFunctionType();

      IRBuilder<> Builder(&Call);

      // Preparing vector arguments. Starting from the second operand because we
      // need to skip the first function-pointer operand.
      std::vector<Type *> VecArgTy;
      std::vector<Value *> VecArgs;
      for (unsigned I = 1; I < FTy->getNumParams(); I++) {

        Type *Ty = FTy->getParamType(I);
        Value *Operand = Call.getArgOperand(I);

        if (Variant.getParameters()[I - 1].isVector()) {

          VectorType *VecTy = VectorType::get(Ty, VecLen, false);
          VecArgTy.push_back(VecTy);

          Value *VecArg =
            Builder.CreateInsertElement(UndefValue::get(VecTy), Operand, Zero);
          VecArgs.push_back(VecArg);

        } else {

          VecArgTy.push_back(Ty);
          VecArgs.push_back(Operand);
        }
      }

      // Preparing the mask.
      VectorType *MaskTy =
          VectorType::get(Type::getInt32Ty(M.getContext()), VecLen, false);
      VecArgTy.push_back(MaskTy);

      Value *MaskArg = Builder.CreateInsertElement(
          ConstantAggregateZero::get(MaskTy), One, Zero);
      VecArgs.push_back(MaskArg);

      // Preparing chosen variant call.
      Type *RetTy = FTy->getReturnType();
      VectorType *VecRetTy = VectorType::get(RetTy, VecLen, false);

      Value *GV = Call.getArgOperand(0);
      PointerType *GVTy = cast<PointerType>(GV->getType());
      unsigned AS = GVTy->getAddressSpace();

      FunctionType *VecFTy = FunctionType::get(VecRetTy, VecArgTy, false);
      PointerType *VecFPtrTy = PointerType::get(VecFTy, AS);
      PointerType *VecFPtrPtrTy = PointerType::get(VecFPtrTy, AS);

      Value *Table = Builder.CreateZExtOrBitCast(GV, VecFPtrPtrTy);
      Value *FPtrPtr = Builder.CreateGEP(
          VecFPtrTy, Table,
          {ConstantInt::get(M.getContext(), APInt(32, Index, true))});
      Value *FPtr = Builder.CreateLoad(VecFPtrTy, FPtrPtr);
      Value *VecRes = Builder.CreateCall(VecFTy, FPtr, VecArgs);
      Value *Res = Builder.CreateExtractElement(VecRes, Zero);

      Call.replaceAllUsesWith(Res);
      RemoveLater.insert(&Call);

      Modified = true;
    }
  }

  for (CallInst *Call : RemoveLater)
    Call->eraseFromParent();

  return Modified;
}

extern "C" {
ModulePass *createIndirectCallLoweringPass() {
  return new intel::IndirectCallLowering();
}
}

} // namespace intel
