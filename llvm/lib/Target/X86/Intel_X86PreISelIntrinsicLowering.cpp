//===- Intel_X86PreISelIntrinsicLowering - X86 Pre-ISel intrinsic lowering ===//
//
// Copyright (C) 2019-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass implements IR lowering for X86 intrinsics right before ISel.
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicsX86.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/Pass.h"

using namespace llvm;

#define DEBUG_TYPE "x86-pre-isel-intrinsic-lowering"

// For alderlake target, this function lowers
//      %core_type = call i8 llvm.x86.intel.fast.cpuid.coretype;
// to (pseudo C code)
//      char core_type;
//      if (__builtin_expect(
//              !(core_type = __cpu_core_type[_rdpid_u32() & 0xff])),
//          0)
//        core_type = __detect_cpu_core_type_1n();
// __cpu_core_type cached core type for each cpu indexed by IA32_TSC_AUX & 0xFF.
// __detect_cpu_core_type_1n detect core type for cpu we are runing and update
// __cpu_core_type array. Note that __detect_cpu_core_type_1n may return 0 (not
// likely) if it can't bullet proof pid and core type it read was executed on
// same core. Read comments in libirc/cpu_core_type.c for the detail explanation
// about IA32_TSC_AUX.
//
// For other target, this function lowers
//      %core_type = call i8 llvm.x86.intel.fast.cpuid.coretype
// to
//      %info = call {i32, i32, i32, i32} @llvm.x86.cpuid(i32 26, i32 0)
//      %eax = extractvalue {i32, i32, i32, i32} %info, 0
//      %tmp = lshr i32 %eax, 24
//      %core_type = trunc i32 %tmp to i8
//
static bool lowerFastCpuIdCoreType(Function &F) {
  Module *M = F.getParent();
  IRBuilder<> IRB(F.getContext());
  MDBuilder MDB(F.getContext());
  Type *Int8Ty = IRB.getInt8Ty();
  StringRef CPU = F.getFnAttribute("target-cpu").getValueAsString();
  GlobalVariable *CpuCoreType;
  FunctionCallee DetectCpuCoreType;
  bool IsCpuCoreTypeDeclared = false;

  for (auto I = F.use_begin(), E = F.use_end(); I != E;) {
    IntrinsicInst *Intrin = cast<IntrinsicInst>((I++)->getUser());
    Value *CoreType;
    if (CPU == "alderlake") {
      ArrayType *CpuCTTy = ArrayType::get(Int8Ty, 0);
      if (!IsCpuCoreTypeDeclared) {
        IsCpuCoreTypeDeclared = true;
        CpuCoreType = cast<GlobalVariable>(
            M->getOrInsertGlobal("__cpu_core_type", CpuCTTy));
        CpuCoreType->setUnnamedAddr(GlobalValue::UnnamedAddr::Local);
        DetectCpuCoreType = M->getOrInsertFunction(
            "__detect_cpu_core_type_1n",
            FunctionType::get(Int8Ty, /*isVarArg=*/false));
      }

      BasicBlock *UpperBB = Intrin->getParent();
      BasicBlock *LowerBB = UpperBB->splitBasicBlock(Intrin);
      BasicBlock *DetectBB =
          BasicBlock::Create(M->getContext(), "", Intrin->getFunction());

      UpperBB->getTerminator()->eraseFromParent();
      IRB.SetInsertPoint(UpperBB);
      CallInst *RawPid =
          IRB.CreateIntrinsic(Intrinsic::x86_rdpid, std::nullopt, std::nullopt);

      // Mask must be same with CPU_MASK in libirc/fast_cpu_core_type.c.
      Value *Pid = IRB.CreateAnd(RawPid, 0xff);
      Value *CoreTypeAddr =
          IRB.CreateGEP(CpuCTTy, CpuCoreType, {IRB.getInt64(0), Pid});
      Value *CoreType0 = IRB.CreateLoad(Int8Ty, CoreTypeAddr);
      Value *IsVaild = IRB.CreateICmpNE(CoreType0, IRB.getInt8(0));
      BranchInst *BI = IRB.CreateCondBr(IsVaild, LowerBB, DetectBB);

      // Not likely to go DetectBB since it is called once for each logical cpu.
      BI->setMetadata(LLVMContext::MD_prof, MDB.createBranchWeights(2000, 1));

      IRB.SetInsertPoint(DetectBB);
      Value *CoreType1 = IRB.CreateCall(DetectCpuCoreType.getFunctionType(),
                                        DetectCpuCoreType.getCallee());
      IRB.CreateBr(LowerBB);

      IRB.SetInsertPoint(LowerBB, LowerBB->getFirstInsertionPt());
      PHINode *PHI = IRB.CreatePHI(Int8Ty, 2);
      PHI->addIncoming(CoreType0, UpperBB);
      PHI->addIncoming(CoreType1, DetectBB);
      CoreType = PHI;
    } else {
      IRB.SetInsertPoint(Intrin);
      CallInst *CpuId =
          IRB.CreateIntrinsic(Intrinsic::x86_cpuid, std::nullopt,
                              {IRB.getInt32(0x1a), IRB.getInt32(0)});
      Value *EAX = IRB.CreateExtractValue(CpuId, 0, "eax");
      CoreType = IRB.CreateLShr(EAX, IRB.getInt32(24));
      CoreType = IRB.CreateTrunc(CoreType, Int8Ty);
    }
    Intrin->replaceAllUsesWith(CoreType);
    Intrin->eraseFromParent();
  }
  return F.getNumUses();
}

static bool lowerIntrinsics(Module &M) {
  bool Changed = false;
  for (Function &F : M) {
    switch (F.getIntrinsicID()) {
    default:
      break;
    case Intrinsic::x86_intel_fast_cpuid_coretype:
      Changed |= lowerFastCpuIdCoreType(F);
      break;
    }
  }
  return Changed;
}

namespace {

class X86PreISelIntrinsicLowering : public ModulePass {
public:
  static char ID;

  X86PreISelIntrinsicLowering() : ModulePass(ID) {}

  bool runOnModule(Module &M) override { return lowerIntrinsics(M); }
};

} // end anonymous namespace

char X86PreISelIntrinsicLowering::ID;

INITIALIZE_PASS(X86PreISelIntrinsicLowering, DEBUG_TYPE,
                "X86 Pre-ISel Intrinsic Lowering", false, false)

ModulePass *llvm::createX86PreISelIntrinsicLoweringPass() {
  return new X86PreISelIntrinsicLowering;
}
