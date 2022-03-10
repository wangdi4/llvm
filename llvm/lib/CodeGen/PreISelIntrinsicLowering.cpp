//===- PreISelIntrinsicLowering.cpp - Pre-ISel intrinsic lowering pass ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass implements IR lowering for the llvm.load.relative and llvm.objc.*
// intrinsics.
// Also llvm.intel.subscript is lowered here. // INTEL
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/PreISelIntrinsicLowering.h"
#include "llvm/Analysis/ObjCARCInstKind.h"
#include "llvm/Analysis/ObjCARCUtil.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/User.h"
#include "llvm/InitializePasses.h"
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

  for (Use &U : llvm::make_early_inc_range(F.uses())) {
    auto CI = dyn_cast<CallInst>(U.getUser());
    if (!CI || CI->getCalledOperand() != &F)
      continue;

    IRBuilder<> B(CI);
    Value *OffsetPtr =
        B.CreateGEP(Int8Ty, CI->getArgOperand(0), CI->getArgOperand(1));
    Value *OffsetPtrI32 = B.CreateBitCast(OffsetPtr, Int32PtrTy);
    Value *OffsetI32 = B.CreateAlignedLoad(Int32Ty, OffsetPtrI32, Align(4));

    Value *ResultPtr = B.CreateGEP(Int8Ty, CI->getArgOperand(0), OffsetI32);

    CI->replaceAllUsesWith(ResultPtr);
    CI->eraseFromParent();
    Changed = true;
  }

  return Changed;
}

// ObjCARC has knowledge about whether an obj-c runtime function needs to be
// always tail-called or never tail-called.
static CallInst::TailCallKind getOverridingTailCallKind(const Function &F) {
  objcarc::ARCInstKind Kind = objcarc::GetFunctionClass(&F);
  if (objcarc::IsAlwaysTail(Kind))
    return CallInst::TCK_Tail;
  else if (objcarc::IsNeverTail(Kind))
    return CallInst::TCK_NoTail;
  return CallInst::TCK_None;
}

static bool lowerObjCCall(Function &F, const char *NewFn,
                          bool setNonLazyBind = false) {
  if (F.use_empty())
    return false;

  // If we haven't already looked up this function, check to see if the
  // program already contains a function with this name.
  Module *M = F.getParent();
  FunctionCallee FCache = M->getOrInsertFunction(NewFn, F.getFunctionType());

  if (Function *Fn = dyn_cast<Function>(FCache.getCallee())) {
    Fn->setLinkage(F.getLinkage());
    if (setNonLazyBind && !Fn->isWeakForLinker()) {
      // If we have Native ARC, set nonlazybind attribute for these APIs for
      // performance.
      Fn->addFnAttr(Attribute::NonLazyBind);
    }
  }

  CallInst::TailCallKind OverridingTCK = getOverridingTailCallKind(F);

  for (Use &U : llvm::make_early_inc_range(F.uses())) {
    auto *CB = cast<CallBase>(U.getUser());

    if (CB->getCalledFunction() != &F) {
      objcarc::ARCInstKind Kind = objcarc::getAttachedARCFunctionKind(CB);
      (void)Kind;
      assert((Kind == objcarc::ARCInstKind::RetainRV ||
              Kind == objcarc::ARCInstKind::UnsafeClaimRV) &&
             "use expected to be the argument of operand bundle "
             "\"clang.arc.attachedcall\"");
      U.set(FCache.getCallee());
      continue;
    }

    auto *CI = cast<CallInst>(CB);
    assert(CI->getCalledFunction() && "Cannot lower an indirect call!");

    IRBuilder<> Builder(CI->getParent(), CI->getIterator());
    SmallVector<Value *, 8> Args(CI->args());
    CallInst *NewCI = Builder.CreateCall(FCache, Args);
    NewCI->setName(CI->getName());

    // Try to set the most appropriate TailCallKind based on both the current
    // attributes and the ones that we could get from ObjCARC's special
    // knowledge of the runtime functions.
    //
    // std::max respects both requirements of notail and tail here:
    // * notail on either the call or from ObjCARC becomes notail
    // * tail on either side is stronger than none, but not notail
    CallInst::TailCallKind TCK = CI->getTailCallKind();
    NewCI->setTailCallKind(std::max(TCK, OverridingTCK));

    if (!CI->use_empty())
      CI->replaceAllUsesWith(NewCI);
    CI->eraseFromParent();
  }

  return true;
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
    if (!CI || CI->getCalledOperand() != &F)
      continue;

    IRBuilder<> Builder(CI);
    CI->replaceAllUsesWith(EmitSubsValue(&Builder, DL, CI));
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
    if (!CI || CI->getCalledOperand() != &F)
      continue;

    CI->replaceAllUsesWith(CI->getPointerOperand());
    salvageDebugInfo(*CI);
    CI->eraseFromParent();

    Changed = true;
  }
  return Changed;
}

static bool lowerSSACopy(Function &F) {
  if (F.use_empty())
    return false;

  bool Changed = false;
  for (auto I = F.use_begin(), E = F.use_end(); I != E;) {
    IntrinsicInst *CI = dyn_cast<IntrinsicInst>(I->getUser());
    ++I;
    if (!CI || CI->getIntrinsicID() != Intrinsic::ssa_copy ||
        CI->getCalledOperand() != &F)
      continue;

    CI->replaceAllUsesWith(CI->getOperand(0));
    salvageDebugInfo(*CI);
    CI->eraseFromParent();

    Changed = true;
  }
  return Changed;
}

#if INTEL_FEATURE_SW_DTRANS
static bool lowerWholeProgramSafe(Function &F) {
  if (F.use_empty())
    return false;

  bool Changed = false;

  LLVMContext &Context = F.getContext();
  ConstantInt *InitVal = ConstantInt::getFalse(Context);

  for (auto I = F.use_begin(), E = F.use_end(); I != E;) {
    CallInst *CI = dyn_cast<CallInst>(I->getUser());
    ++I;
    if (!CI || CI->getCalledOperand() != &F)
      continue;

    CI->replaceAllUsesWith(InitVal);
    salvageDebugInfo(*CI);
    CI->eraseFromParent();

    Changed = true;
  }
  return Changed;
}
#endif // INTEL_FEATURE_SW_DTRANS

static bool lowerIntelHonorFCmp(Function &F) {
  if (F.use_empty())
    return false;

  bool Changed = false;

  for (auto I = F.use_begin(), E = F.use_end(); I != E;) {
    auto *CI = dyn_cast<IntelHonorFCmpIntrinsic>(I->getUser());
    ++I;
    if (!CI || CI->getCalledOperand() != &F)
      continue;

    IRBuilder<> Builder(CI);

    // Use the fast-math flags from the intrinsic.
    Builder.setFastMathFlags(CI->getFastMathFlags());
    assert(!CI->hasNoNaNs());

    // If the operands are constant, the IRBuilder may fold this.
    Value *V = Builder.CreateFCmp(CI->getPredicate(), CI->getArgOperand(0),
                                  CI->getArgOperand(1));
    // If it didn't get folded, copy the fast-math flags and metadata.
    if (auto *FCmpI = dyn_cast<Instruction>(V))
      FCmpI->copyMetadata(*CI);
    V->takeName(CI);
    CI->replaceAllUsesWith(V);
    CI->eraseFromParent();

    Changed = true;
  }
  return Changed;
}

static bool lowerDirectiveRegionEntryExit(Function &F) {
  if (F.use_empty())
    return false;

  bool Changed = false;

  for (auto I = F.use_begin(), E = F.use_end(); I != E;) {
    auto *Intrin = dyn_cast<IntrinsicInst>(I->getUser());
    ++I;

    if (!Intrin || !Intrin->hasOperandBundles())
      continue;

    OperandBundleUse BU = Intrin->getOperandBundleAt(0);

    StringRef TagName = BU.getTagName();

    // Only handle LoopOpt related tags.
    // Others can be added, if needed.
    if (TagName.equals("DIR.PRAGMA.DISTRIBUTE_POINT") ||
        TagName.equals("DIR.PRAGMA.BLOCK_LOOP") ||
        TagName.equals("DIR.PRAGMA.PREFETCH_LOOP") ||
        TagName.equals("DIR.PRAGMA.END.DISTRIBUTE_POINT") ||
        TagName.equals("DIR.PRAGMA.END.BLOCK_LOOP") ||
        TagName.equals("DIR.PRAGMA.END.PREFETCH_LOOP")) {

      Intrin->replaceAllUsesWith(UndefValue::get(Intrin->getType()));
      Intrin->eraseFromParent();
      Changed = true;
    }
  }

  return Changed;
}
#endif // INTEL_CUSTOMIZATION

static bool lowerIntrinsics(Module &M) {
  bool Changed = false;
  for (Function &F : M) {
    if (F.getName().startswith("llvm.load.relative.")) {
      Changed |= lowerLoadRelative(F);
      continue;
    }

#if INTEL_CUSTOMIZATION
    // TODO: These checks could be placed under the switch case once they are
    // fully tested in xmain.
    if (F.getIntrinsicID() == Intrinsic::intel_subscript ||
        F.getIntrinsicID() == Intrinsic::intel_subscript_nonexact)
      Changed |= lowerSubscript(F);

    if (F.getIntrinsicID() == Intrinsic::intel_fakeload)
      Changed |= lowerFakeload(F);

#if INTEL_FEATURE_SW_DTRANS
    if (F.getIntrinsicID() == Intrinsic::intel_wholeprogramsafe)
      Changed |= lowerWholeProgramSafe(F);
#endif // INTEL_FEATURE_SW_DTRANS

    if (F.getIntrinsicID() == Intrinsic::ssa_copy)
      Changed |= lowerSSACopy(F);

    if (F.getIntrinsicID() == Intrinsic::intel_honor_fcmp)
      Changed |= lowerIntelHonorFCmp(F);

    if (F.getIntrinsicID() == Intrinsic::directive_region_entry ||
        F.getIntrinsicID() == Intrinsic::directive_region_exit)
      Changed |= lowerDirectiveRegionEntryExit(F);
#endif // INTEL_CUSTOMIZATION

    switch (F.getIntrinsicID()) {
    default:
      break;
    case Intrinsic::objc_autorelease:
      Changed |= lowerObjCCall(F, "objc_autorelease");
      break;
    case Intrinsic::objc_autoreleasePoolPop:
      Changed |= lowerObjCCall(F, "objc_autoreleasePoolPop");
      break;
    case Intrinsic::objc_autoreleasePoolPush:
      Changed |= lowerObjCCall(F, "objc_autoreleasePoolPush");
      break;
    case Intrinsic::objc_autoreleaseReturnValue:
      Changed |= lowerObjCCall(F, "objc_autoreleaseReturnValue");
      break;
    case Intrinsic::objc_copyWeak:
      Changed |= lowerObjCCall(F, "objc_copyWeak");
      break;
    case Intrinsic::objc_destroyWeak:
      Changed |= lowerObjCCall(F, "objc_destroyWeak");
      break;
    case Intrinsic::objc_initWeak:
      Changed |= lowerObjCCall(F, "objc_initWeak");
      break;
    case Intrinsic::objc_loadWeak:
      Changed |= lowerObjCCall(F, "objc_loadWeak");
      break;
    case Intrinsic::objc_loadWeakRetained:
      Changed |= lowerObjCCall(F, "objc_loadWeakRetained");
      break;
    case Intrinsic::objc_moveWeak:
      Changed |= lowerObjCCall(F, "objc_moveWeak");
      break;
    case Intrinsic::objc_release:
      Changed |= lowerObjCCall(F, "objc_release", true);
      break;
    case Intrinsic::objc_retain:
      Changed |= lowerObjCCall(F, "objc_retain", true);
      break;
    case Intrinsic::objc_retainAutorelease:
      Changed |= lowerObjCCall(F, "objc_retainAutorelease");
      break;
    case Intrinsic::objc_retainAutoreleaseReturnValue:
      Changed |= lowerObjCCall(F, "objc_retainAutoreleaseReturnValue");
      break;
    case Intrinsic::objc_retainAutoreleasedReturnValue:
      Changed |= lowerObjCCall(F, "objc_retainAutoreleasedReturnValue");
      break;
    case Intrinsic::objc_retainBlock:
      Changed |= lowerObjCCall(F, "objc_retainBlock");
      break;
    case Intrinsic::objc_storeStrong:
      Changed |= lowerObjCCall(F, "objc_storeStrong");
      break;
    case Intrinsic::objc_storeWeak:
      Changed |= lowerObjCCall(F, "objc_storeWeak");
      break;
    case Intrinsic::objc_unsafeClaimAutoreleasedReturnValue:
      Changed |= lowerObjCCall(F, "objc_unsafeClaimAutoreleasedReturnValue");
      break;
    case Intrinsic::objc_retainedObject:
      Changed |= lowerObjCCall(F, "objc_retainedObject");
      break;
    case Intrinsic::objc_unretainedObject:
      Changed |= lowerObjCCall(F, "objc_unretainedObject");
      break;
    case Intrinsic::objc_unretainedPointer:
      Changed |= lowerObjCCall(F, "objc_unretainedPointer");
      break;
    case Intrinsic::objc_retain_autorelease:
      Changed |= lowerObjCCall(F, "objc_retain_autorelease");
      break;
    case Intrinsic::objc_sync_enter:
      Changed |= lowerObjCCall(F, "objc_sync_enter");
      break;
    case Intrinsic::objc_sync_exit:
      Changed |= lowerObjCCall(F, "objc_sync_exit");
      break;
    }
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
