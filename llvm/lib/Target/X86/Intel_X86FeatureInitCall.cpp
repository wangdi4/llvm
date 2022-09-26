//====-- Intel_FeatureInitCall.cpp ----------------====
//
//      Copyright (c) 2019-2022 Intel Corporation.
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
#include "llvm/ADT/FloatingPointMode.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/ConstantRange.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicsX86.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Intel_CPU_utils.h"
#if INTEL_FEATURE_ISA_AVX256
#include "llvm/Support/X86TargetParser.h"
#endif // INTEL_FEATURE_ISA_AVX256
#include "llvm/Target/TargetMachine.h"

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
  }

  bool getTargetAttributes(Function &F,
                           SmallVectorImpl<StringRef> &TargetFeatures) {
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
        if (Itr != std::end(TargetFeatures))
          TargetFeatures.erase(Itr);
      }
    }
    return true;
  }

  bool isMainFunction(Function &F) {
    StringRef FName = F.getName();
    if (F.hasMetadata("llvm.acd.clone"))
      FName = FName.take_front(FName.find('.'));
    return llvm::StringSwitch<bool>(FName)
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
  // the following instruction in main routine:
  //     %tmp = alloca i32, align 4
  //     %0 = bitcast i32* %tmp to i8*
  //     call void @llvm.lifetime.start.p0i8(i64 4, i8* %0)
  //     call void @llvm.x86.sse.stmxcsr(i8* %0)
  //     %stmxcsr = load i32, i32* %tmp, align 4
  //     %or = or i32 %stmxcsr, <FtzDazMask>
  //     store i32 %or, i32* %tmp, align 4
  //     call void @llvm.x86.sse.ldmxcsr(i8* %0)
  //     call void @llvm.lifetime.end.p0i8(i64 4, i8* %0)
  bool writeMXCSRFTZBits(Function &F, uint32_t FtzDaz) {

    // stmxcsr and ldmxcsr need sse supported.
    if (!TM->getSubtarget<X86Subtarget>(F).hasSSE1())
      return false;

    auto FirstNonAlloca = getFirstNonAllocaInTheEntryBlock(F);
    IRBuilder<> IRB(FirstNonAlloca);

    // These instructions should be emitted as part of the prolog and marked
    // frame-setup, but they are added here instead. To prevent debuggers
    // from stopping too early inside the prolog, remove the source correlation
    // and hope these get folded into the correlation for the prolog sequence.
    IRB.SetCurrentDebugLocation(DebugLoc());

    // %tmp = alloca i32, align 4
    IntegerType *I32Ty = IRB.getInt32Ty();
    AllocaInst *AI = IRB.CreateAlloca(I32Ty);
    AI->setAlignment(Align(4));

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
    LoadInst *LI = IRB.CreateAlignedLoad(I32Ty, AI, Align(4), "stmxcsr");

    // To share code, we're using the ftz_daz argument that
    // __intel_new_feature_proc_init expects, but it must be
    // expanded here to get the proper MXCSR mask.
    uint32_t FtzDazMask = 0;
    if (FtzDaz & 0b10)
      FtzDazMask |= 0x0040; // DAZ
    if (FtzDaz & 0b01)
      FtzDazMask |= 0x8000; // FTZ

    // %or = or i32 %stmxcsr, <FtzDazMask>
    ConstantInt *CInt = IRB.getInt32(FtzDazMask);
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

  // This function initializes X87's PC bits in FPCW, it will generate the
  // following instruction in main routine:
  //   %1 = alloca i16, align 2
  //   store i16 Imm, i16* %1, align 2
  //   call void asm sideeffect "fldcw ${0:w}",
  //                            "*m,~{dirflag},~{fpsr},~{flags}"(i16* %1)
  bool setX87Precision(Function &F, int X87Precision) {
    if (!TM->getSubtarget<X86Subtarget>(F).hasX87())
      return false;

    short Imm = 0;
    switch (X87Precision) {
    default: llvm_unreachable("Bad X87 precision value");
    case 32: Imm = 0x107f; break; // PC = 00 (SP)
    case 64: Imm = 0x127f; break; // PC = 10 (DP)
    case 80: Imm = 0x137f; break; // PC = 11 (DEP)
    }

    auto FirstNonAlloca = getFirstNonAllocaInTheEntryBlock(F);
    const DataLayout &DL = FirstNonAlloca->getModule()->getDataLayout();
    IRBuilder<> IRB(FirstNonAlloca);

    // These instructions should be emitted as part of the prolog and marked
    // frame-setup, but they are added here instead. To prevent debuggers
    // from stopping too early inside the prolog, remove the source correlation
    // and hope these get folded into the correlation for the prolog sequence.
    IRB.SetCurrentDebugLocation(DebugLoc());

    // %1 = alloca i16, align 2
    IntegerType *I16Ty = IRB.getInt16Ty();
    AllocaInst *AI = IRB.CreateAlloca(I16Ty);
    AI->setAlignment(DL.getPrefTypeAlign(I16Ty));

    // %2 = bitcast i16* %1 to i8*
    PointerType *Int8PtrTy = IRB.getInt8PtrTy();
    Value *Ptr8 = IRB.CreateBitCast(AI, Int8PtrTy);

    // call void @llvm.lifetime.start.p0i8(i64 2, i8* %2)
    ConstantInt *AllocaSize = IRB.getInt64(DL.getTypeStoreSize(I16Ty));
    IRB.CreateLifetimeStart(Ptr8, AllocaSize);

    // store i16 Imm, i16* %1, align 2
    ConstantInt *CInt = IRB.getInt16(Imm);
    IRB.CreateStore(CInt, AI);

    // call void asm sideeffect ...
    InlineAsm *Asm = InlineAsm::get(
        FunctionType::get(IRB.getVoidTy(), {Ptr8->getType()}, false),
        "fldcw ${0:w}", "*m,~{dirflag},~{fpsr},~{flags}", true);
    CallBase *CallInst = IRB.CreateCall(Asm, Ptr8);
    // Add elementtype attribute for indirect constraints.
    auto Attr = Attribute::get(F.getContext(), llvm::Attribute::ElementType,
                               IRB.getInt8Ty());
    CallInst->addParamAttr(0, Attr);

    // call void @llvm.lifetime.end.p0i8(i64 2, i8* %2)
    IRB.CreateLifetimeEnd(Ptr8, AllocaSize);

    return true;
  }

  uint32_t getFtzDaz(Function &F) const {
    Attribute Attr = F.getFnAttribute("denormal-fp-math");
    StringRef Val = Attr.getValueAsString();
    if (Val.empty())
      return 0;
    DenormalMode Mode = parseDenormalFPAttribute(Val);
    uint32_t FtzDaz = 0;
    if (Mode.Input == DenormalMode::PreserveSign)
      FtzDaz |= 0b10; // DAZ
    if (Mode.Output == DenormalMode::PreserveSign)
      FtzDaz |= 0b01; // FTZ
    return FtzDaz;
 }

  bool insertProcInitCall(Function &F) {
    // To Be Done
    // Maybe better to follow icc's behavior, which is to call the base version
    // of "__intel_new_feature_proc_init" even when Options.IntelAdvancedOptim
    // are not enabled but Options.IntelLibIRCAllowed are enabled.
    if (!TM->Options.IntelLibIRCAllowed || !TM->Options.IntelAdvancedOptim)
      return false;

    // Collect target feature mask
    SmallVector<StringRef> TargetFeatures;
#if INTEL_FEATURE_ISA_AVX256
    // FIXME: We only check SKX features for AVX256+ for better usability.
    if (F.getFnAttribute("target-cpu").getValueAsString() == "common-avx256")
      X86::getFeaturesForCPU("skylake-avx512", TargetFeatures);
    else
#endif // INTEL_FEATURE_ISA_AVX256
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

    // These instructions should be emitted as part of the prolog and marked
    // frame-setup, but they are added here instead. To prevent debuggers
    // from stopping too early inside the prolog, remove the source correlation
    // and hope these get folded into the correlation for the prolog sequence.
    IRB.SetCurrentDebugLocation(DebugLoc());

    uint32_t FtzDaz = getFtzDaz(F);
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

    // The following code works for CMPLRLLVM-9854.
    // ICC supports options -pc80/-pc64/-pc32, which sets x87 FPU to
    // 64/53/24-bit precision (FP80/FP64/FP32).
    // We add the same support in Xmain.
    bool X87PrecisionInit = false;
    int X87Precision = TM->Options.X87Precision;

    if (X87Precision == 0)
      F.getFnAttribute("x87-precision")
          .getValueAsString()
          .getAsInteger(10, X87Precision);

    if (X87Precision)
      X87PrecisionInit = setX87Precision(F, X87Precision);

    // This pass only works for X87 precision control when OptLevel is -O0.
    if (skipFunction(F) || TM->getOptLevel() == CodeGenOpt::None)
      return X87PrecisionInit;

    bool ProcInit = insertProcInitCall(F);
    bool FTZ = false;
    uint32_t FtzDaz = getFtzDaz(F);

    // If FTZ + DAZ are set by libirc call __intel_new_feature_proc_init
    // in insertProcInitCall, we should not set them again.
    if (FtzDaz && !ProcInit)
      FTZ = writeMXCSRFTZBits(F, FtzDaz);

    return ProcInit || FTZ || X87PrecisionInit;
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
