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

#include "llvm/Analysis/Intel_VectorVariant.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#include <set>

#define DEBUG_TYPE "IndirectCallLowering"

using namespace llvm;

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

  std::set<CallInst *> RemoveLater;

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
      assert(Index < Variants.size() && "Expected at least one masked variant");

      unsigned VecLen = VectorVariant(Variants[Index]).getVlen();
      ConstantInt *Zero = ConstantInt::get(M.getContext(), APInt(32, 0, true));
      ConstantInt *One = ConstantInt::get(M.getContext(), APInt(32, 1, true));
      FunctionType *FTy = Call.getFunctionType();

      // Preparing vector arguments. Starting from the second operand because we
      // need to skip the first function-pointer operand.
      std::vector<Type *> VecArgTy;
      std::vector<Value *> VecArgs;
      for (unsigned I = 1; I < FTy->getNumParams(); I++) {
        Type *Ty = FTy->getParamType(I);
        VectorType *VecTy = VectorType::get(Ty, VecLen, false);
        VecArgTy.push_back(VecTy);

        Value *VecArg = InsertElementInst::Create(
            UndefValue::get(VecTy), Call.getArgOperand(I), Zero, "", &Call);
        VecArgs.push_back(VecArg);
      }

      // Preparing the mask.
      VectorType *MaskTy =
          VectorType::get(Type::getInt32Ty(M.getContext()), VecLen, false);
      VecArgTy.push_back(MaskTy);

      Value *MaskArg = InsertElementInst::Create(
          ConstantAggregateZero::get(MaskTy), One, Zero, "", &Call);
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

      Value *Table = CastInst::CreateZExtOrBitCast(GV, VecFPtrPtrTy, "", &Call);
      Value *FPtrPtr = GetElementPtrInst::Create(
          VecFPtrTy, Table,
          {ConstantInt::get(M.getContext(), APInt(32, Index, true))}, "",
          &Call);
      Value *FPtr = new LoadInst(VecFPtrTy, FPtrPtr, "", &Call);

      Value *VecRes = CallInst::Create(VecFTy, FPtr, VecArgs, "", &Call);
      Value *Res = ExtractElementInst::Create(VecRes, Zero, "", &Call);

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
