//====-- Intel_FeatureInitCall.cpp ----------------====
//
//      Copyright (c) 2019 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
// This file defines the pass which insert libirc
// __intel_new_feature_proc_init[_n] to main function.
//

#include "X86.h"
#include "X86Subtarget.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/ConstantRange.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Intel_CPU_utils.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;

#define DEBUG_TYPE "x86-feature-proc-init"

namespace {

class X86FeatureInitPass : public FunctionPass {
  TargetMachine *TM = nullptr;

public:
  static char ID; // Pass identification, replacement for typeid..

  X86FeatureInitPass() : FunctionPass(ID) {
    initializeX86FeatureInitPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetPassConfig>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
  }

  bool getTargetAttributes(Function &F,
                           std::vector<StringRef> &TargetFeatures) {
    StringRef TF = F.getFnAttribute("target-features").getValueAsString();

    if (TF.empty())
      return false;

    SmallVector<StringRef, 20> Features;
    TF.split(Features, ",");
    for (StringRef &Feature : Features) {
      if (Feature.startswith("+"))
        TargetFeatures.push_back(Feature.substr(1));
      else if (Feature.startswith("-")) {
        StringRef S = Feature.substr(1);
        auto Itr =
            std::find_if(std::begin(TargetFeatures), std::end(TargetFeatures),
                         [&](StringRef A) { return A == S; });
        TargetFeatures.erase(Itr);
      }
    }
    return true;
  }

  bool isMainFunction(Function &F) {
    return llvm::StringSwitch<bool>(F.getName())
      .Cases("main",
             "MAIN__", true)
      .Cases("wmain",
             "WinMain",
             "wWinMain",
             TM->getTargetTriple().isOSMSVCRT())
      .Default(false);
  }

  // Finds first non alloca instruction in the entry block of a function.
  // The way clang generates code, it puts allocas at the beginning of the
  // first basic block. All code comes below that. We should follow the rule.
  static Instruction *getFirstNonAllocaInTheEntryBlock(Function &F) {
    for (Instruction &I : F.getEntryBlock())
      if (!isa<AllocaInst>(&I))
        return &I;
    llvm_unreachable("No terminator in the entry block");
  }

  // Enable Flush To Zero and Denormals Are Zero flags in MXCSR, will generate
  // the following instruction in main route:
  //     %tmp = alloca i32, align 4
  //     %0 = bitcast i32* %tmp to i8*
  //     call void @llvm.lifetime.start.p0i8(i64 4, i8* %0)
  //     call void @llvm.x86.sse.stmxcsr(i8* %0)
  //     %stmxcsr = load i32, i32* %tmp, align 4
  //     %or = or i32 %stmxcsr, 32832
  //     store i32 %or, i32* %tmp, align 4
  //     call void @llvm.x86.sse.ldmxcsr(i8* %0)
  //     call void @llvm.lifetime.end.p0i8(i64 4, i8* %0)
  bool writeMXCSRFTZBits(Function &F) {

    // stmxcsr and ldmxcsr need sse supported.
    if (!TM->getSubtarget<X86Subtarget>(F).hasSSE1())
      return false;

    auto FirstNonAlloca = getFirstNonAllocaInTheEntryBlock(F);
    IRBuilder<> IRB(FirstNonAlloca);

    // %tmp = alloca i32, align 4
    IntegerType *I32Ty = IRB.getInt32Ty();
    AllocaInst *AI = IRB.CreateAlloca(I32Ty);
    AI->setAlignment(MaybeAlign(4));

    // %0 = bitcast i32* %tmp to i8*
    PointerType *Int8PtrTy = IRB.getInt8PtrTy();
    Value *Ptr8 = IRB.CreateBitCast(AI, Int8PtrTy);

    // call void @llvm.lifetime.start.p0i8(i64 4, i8* %0)
    ConstantInt *AllocaSize = IRB.getInt64(4);
    IRB.CreateLifetimeStart(Ptr8, AllocaSize);

    // call void @llvm.x86.sse.stmxcsr(i8* %0)
    Module *Md = IRB.GetInsertBlock()->getModule();
    Intrinsic::ID IID = Intrinsic::x86_sse_stmxcsr;
    Function* FI = Intrinsic::getDeclaration(Md, IID);
    IRB.CreateCall(FI, Ptr8);

    // %stmxcsr = load i32, i32* %tmp, align 4
    LoadInst *LI = IRB.CreateAlignedLoad(AI, 4, "stmxcsr");

    // %or = or i32 %stmxcsr, 32832
    ConstantInt *CInt = IRB.getInt32(0x8040);
    Value *Or = IRB.CreateOr(LI, CInt, "ftz_daz");

    //store i32 %or, i32* %tmp1, align 4
    IRB.CreateStore(Or, AI);

    //call void @llvm.x86.sse.ldmxcsr(i8* %1)
    IID = Intrinsic::x86_sse_ldmxcsr;
    FI = Intrinsic::getDeclaration(Md, IID);
    IRB.CreateCall(FI, Ptr8);

    // call void @llvm.lifetime.end.p0i8(i64 4, i8* %0)
    IRB.CreateLifetimeEnd(Ptr8, AllocaSize);

    return true;
  }

  bool insertProcInitCall(Function &F) {
    // To Be Done
    // Maybe better to follow icc's behavior, which is to call the base version
    // of "__intel_new_feature_proc_init" even when Options.IntelAdvancedOptim
    // are not enabled but Options.IntelLibIRCAllowed are enabled.
    if (!TM->Options.IntelLibIRCAllowed || !TM->Options.IntelAdvancedOptim)
      return false;

    // Collect target feature mask
    std::vector<StringRef> TargetFeatures;
    if (getTargetAttributes(F, TargetFeatures) == false)
      report_fatal_error(
          "Advanced optimizations are enabled, but no target features");

    std::array<X86::ISAVecElementTy, 2>
        CpuBitMap = X86::getCpuFeatureBitmap(TargetFeatures,
                                             /*OnlyAutogenerated*/true);

    LLVM_DEBUG(dbgs() << "[Feature bitmap] : " << CpuBitMap[0]
               << " - " << CpuBitMap[1] << "\n");

    auto FirstNonAlloca = getFirstNonAllocaInTheEntryBlock(F);
    IRBuilder<> IRB(FirstNonAlloca);
    uint32_t FtzDaz = TM->Options.IntelFtzDaz ? 0x11 : 0x0;
    Value *Args[] = {
        ConstantInt::get(IRB.getInt32Ty(), FtzDaz),
        ConstantInt::get(IRB.getInt64Ty(), CpuBitMap[0]),
    };
    FunctionCallee FeatureInit = F.getParent()->getOrInsertFunction(
        "__intel_new_feature_proc_init", IRB.getVoidTy(), IRB.getInt32Ty(),
        IRB.getInt64Ty());
    IRB.CreateCall(FeatureInit, Args);

    for (unsigned int Index = 1; Index < CpuBitMap.size(); Index++) {
      if (CpuBitMap[Index]) {
        Value *ArgsN[] = {
            ConstantInt::get(IRB.getInt32Ty(), Index),
            ConstantInt::get(IRB.getInt64Ty(), CpuBitMap[Index]),
        };
        FunctionCallee FeatureInitN = F.getParent()->getOrInsertFunction(
            "__intel_new_feature_proc_init_n", IRB.getVoidTy(),
            IRB.getInt32Ty(), IRB.getInt64Ty());
        IRB.CreateCall(FeatureInitN, ArgsN);
      }
    }

    return true;
  }

  bool runOnFunction(Function &F) override {
    TM = &getAnalysis<TargetPassConfig>().getTM<TargetMachine>();
    if (isMainFunction(F) == false)
      return false;

    bool ProcInit = insertProcInitCall(F);
    bool FTZ = false;

    // If FTZ + DAZ are set by libirc call __intel_new_feature_proc_init
    // in insertProcInitCall, we should not set them again.
    if (TM->Options.IntelFtzDaz && !ProcInit)
      FTZ = writeMXCSRFTZBits(F);

    return ProcInit || FTZ;
  }
};

} // end anonymous namespace

char X86FeatureInitPass::ID = 0;

INITIALIZE_PASS_BEGIN(X86FeatureInitPass, DEBUG_TYPE,
                      "X86 runtime feature initialization pass", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_END(X86FeatureInitPass, DEBUG_TYPE,
                    "X86 runtime feature initialization pass", false, false)

FunctionPass *llvm::createFeatureInitPass() { return new X86FeatureInitPass(); }
