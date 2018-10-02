//===- PreISelIntrinsicLowering.cpp - Pre-ISel intrinsic lowering pass ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass implements IR lowering for the llvm.load.relative intrinsic.
// Also llvm.intel.subscript is lowered here. // INTEL
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/PreISelIntrinsicLowering.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/User.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Transforms/Utils/Local.h" // INTEL

using namespace llvm;

static bool lowerLoadRelative(Function &F) {
  if (F.use_empty())
    return false;

  bool Changed = false;
  Type *Int32Ty = Type::getInt32Ty(F.getContext());
  Type *Int32PtrTy = Int32Ty->getPointerTo();
  Type *Int8Ty = Type::getInt8Ty(F.getContext());

  for (auto I = F.use_begin(), E = F.use_end(); I != E;) {
    auto CI = dyn_cast<CallInst>(I->getUser());
    ++I;
    if (!CI || CI->getCalledValue() != &F)
      continue;

    IRBuilder<> B(CI);
    Value *OffsetPtr =
        B.CreateGEP(Int8Ty, CI->getArgOperand(0), CI->getArgOperand(1));
    Value *OffsetPtrI32 = B.CreateBitCast(OffsetPtr, Int32PtrTy);
    Value *OffsetI32 = B.CreateAlignedLoad(OffsetPtrI32, 4);

    Value *ResultPtr = B.CreateGEP(Int8Ty, CI->getArgOperand(0), OffsetI32);

    CI->replaceAllUsesWith(ResultPtr);
    CI->eraseFromParent();
    Changed = true;
  }

  return Changed;
}

#if INTEL_CUSTOMIZATION
// Reference code is in Intel_LowerSubscriptIntrinsic.cpp:lowerIntrinsics().
// Duplicated to avoid build dependencies on Scalar library.
static bool lowerSubscript(Function &F) {
  if (F.use_empty())
    return false;

  bool Changed = false;
  const DataLayout &DL = F.getParent()->getDataLayout();
  for (auto I = F.use_begin(), E = F.use_end(); I != E;) {
    SubscriptInst *CI = dyn_cast<SubscriptInst>(I->getUser());
    ++I;
    if (!CI || CI->getCalledValue() != &F)
      continue;

    IRBuilder<> Builder(CI);
    Value *Offset[] = {EmitSubsOffset(&Builder, DL, CI)};
    CI->replaceAllUsesWith(
        Builder.CreateInBoundsGEP(CI->getPointerOperand(), Offset));
    salvageDebugInfo(*CI);
    CI->eraseFromParent();

    Changed = true;
  }
  return Changed;
}

static bool lowerFakeload(Function &F) {
  if (F.use_empty())
    return false;

  bool Changed = false;
  for (auto I = F.use_begin(), E = F.use_end(); I != E;) {
    FakeloadInst *CI = dyn_cast<FakeloadInst>(I->getUser());
    ++I;
    if (!CI || CI->getCalledValue() != &F)
      continue;

    CI->replaceAllUsesWith(CI->getPointerOperand());
    salvageDebugInfo(*CI);
    CI->eraseFromParent();

    Changed = true;
  }
  return Changed;
}

static bool lowerWholeProgramSafe(Function &F) {
  if (F.use_empty())
    return false;

  bool Changed = false;

  LLVMContext &Context = F.getContext();
  ConstantInt *InitVal = ConstantInt::getFalse(Context);

  for (auto I = F.use_begin(), E = F.use_end(); I != E;) {
    CallInst *CI = dyn_cast<CallInst>(I->getUser());
    ++I;
    if (!CI || CI->getCalledValue() != &F)
      continue;

    CI->replaceAllUsesWith(InitVal);
    salvageDebugInfo(*CI);
    CI->eraseFromParent();

    Changed = true;
  }
  return Changed;
}
#endif // INTEL_CUSTOMIZATION

static bool lowerIntrinsics(Module &M) {
  bool Changed = false;
  for (Function &F : M) {
    if (F.getName().startswith("llvm.load.relative."))
      Changed |= lowerLoadRelative(F);

    if (F.getIntrinsicID() == Intrinsic::intel_subscript) // INTEL
      Changed |= lowerSubscript(F);                       // INTEL

    if (F.getIntrinsicID() == Intrinsic::intel_fakeload)  // INTEL
      Changed |= lowerFakeload(F);                        // INTEL

    if (F.getIntrinsicID() == Intrinsic::intel_wholeprogramsafe)  // INTEL
      Changed |= lowerWholeProgramSafe(F);                        // INTEL
  }
  return Changed;
}

namespace {

class PreISelIntrinsicLoweringLegacyPass : public ModulePass {
public:
  static char ID;

  PreISelIntrinsicLoweringLegacyPass() : ModulePass(ID) {}

  bool runOnModule(Module &M) override { return lowerIntrinsics(M); }
};

} // end anonymous namespace

char PreISelIntrinsicLoweringLegacyPass::ID;

INITIALIZE_PASS(PreISelIntrinsicLoweringLegacyPass,
                "pre-isel-intrinsic-lowering", "Pre-ISel Intrinsic Lowering",
                false, false)

ModulePass *llvm::createPreISelIntrinsicLoweringPass() {
  return new PreISelIntrinsicLoweringLegacyPass;
}

PreservedAnalyses PreISelIntrinsicLoweringPass::run(Module &M,
                                                    ModuleAnalysisManager &AM) {
  if (!lowerIntrinsics(M))
    return PreservedAnalyses::all();
  else
    return PreservedAnalyses::none();
}
