//===- Intel_X86PreISelIntrinsicLowering - X86 Pre-ISel intrinsic lowering ===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
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
#include "X86TargetMachine.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/IntrinsicsX86.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

using namespace llvm;

#define DEBUG_TYPE "x86-pre-isel-intrinsic-lowering"

// Belowing macro MUST be same with counterpart in libirc/fast_cpu_core_type.c.

// Specify max number of logic processores each cpu chip has. It must be a
// multiple of 2 for simplicity.
#define MAX_NUM_PROCESSORS_PC 256

// Specify max number of numa nodes the system has.
#define MAX_NUM_NUMA_NODES 4

#define MAX_NUM_CPU (MAX_NUM_NUMA_NODES * MAX_NUM_PROCESSORS_PC)
#define CPU_MASK (MAX_NUM_CPU - 1)

// Indicate whether core type detection is supported.
#define UNSUPPORT_CTD 0xfe
#define SUPPORT_CTD 0xff

namespace {

class X86PreISelIntrinsicLowering : public ModulePass {
public:
  static char ID;

  X86PreISelIntrinsicLowering() : ModulePass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetPassConfig>();
  }

  bool lowerIntrinsics(Module &M);
  bool runOnModule(Module &M) override { return lowerIntrinsics(M); }
};

} // end anonymous namespace

char X86PreISelIntrinsicLowering::ID;

// This function lowers
//      %core_type = call i8 llvm.x86.intel.fast.cpuid.coretype;
// to (pseudo C code) if target has no TuningFastCoreType feature
//     // Run once before main.
//     __init_cpu_core_type();
//
//     // Run below code on each call site.
//     uint8_t core_type = 0;
//     if (__cpu_core_type[MAX_NUM_CPU] == SUPPORT_CTD)
//       if (__builtin_expect(!(core_type = __cpu_core_type[_rdpid_u32() &
//                                                          CPU_MASK]),
//                            0))
//         core_type = __detect_cpu_core_type();
//
// to (pseudo C code) if target has TuningFastCoreType feature
//     // Run below code on each call site.
//     uint8_t core_type = 0;
//     if (__builtin_expect(!(core_type = __cpu_core_type[_rdpid_u32() &
//         CPU_MASK]), 0))
//       core_type = __detect_cpu_core_type();
//
// __cpu_core_type cached core type for each cpu indexed by IA32_TSC_AUX &
// CPU_MASK. __init_cpu_core_type will check whether this cpu supports cpu core
// type detection. if it is, __cpu_core_type[MAX_NUM_CPU] will be set to
// SUPPORT_CTD. Otherwise, this intrinsic will always return 0.
// __detect_cpu_core_type detect core type for cpu we are runing and update
// __cpu_core_type array. Note that __detect_cpu_core_type may return 0 (not
// likely) if it can't bullet proof pid and core type it read was executed on
// same core. Read comments in libirc/cpu_core_type.c for the detail explanation
// about IA32_TSC_AUX.
static bool lowerFastCpuIdCoreType(Function &F, const X86Subtarget *ST) {
  bool HasFastCoreType = ST->hasFastCoreType();
  Module *M = F.getParent();
  IRBuilder<> IRB(F.getContext());

  // Need to run init function first if we can't gurantee target support fast
  // core type detection.
  if (!HasFastCoreType) {
    // Append __init_cpu_core_type to llvm.global_ctors.
    FunctionCallee InitCpuCoreType = M->getOrInsertFunction(
        "__init_cpu_core_type",
        FunctionType::get(IRB.getVoidTy(), /*isVarArg=*/false));
    appendToGlobalCtors(*M, cast<Function>(InitCpuCoreType.getCallee()),
                        UINT16_MAX);
  }

  MDBuilder MDB(F.getContext());
  Type *Int8Ty = IRB.getInt8Ty();
  ArrayType *CpuCTTy = ArrayType::get(Int8Ty, MAX_NUM_CPU + 1);
  GlobalVariable *CpuCoreType =
      cast<GlobalVariable>(M->getOrInsertGlobal("__cpu_core_type", CpuCTTy));
  CpuCoreType->setUnnamedAddr(GlobalValue::UnnamedAddr::Local);
  FunctionCallee DetectCpuCoreType = M->getOrInsertFunction(
      "__detect_cpu_core_type", FunctionType::get(Int8Ty, /*isVarArg=*/false));
  for (auto I = F.use_begin(), E = F.use_end(); I != E;) {
    IntrinsicInst *Intrin = cast<IntrinsicInst>((I++)->getUser());
    BasicBlock *UpperBB = Intrin->getParent();
    BasicBlock *LowerBB = UpperBB->splitBasicBlock(Intrin);
    BasicBlock *FastDetectBB = UpperBB;
    BasicBlock *DetectBB =
        BasicBlock::Create(M->getContext(), "", Intrin->getFunction());

    // Check whether host cpu supports core type detection.
    UpperBB->getTerminator()->eraseFromParent();
    IRB.SetInsertPoint(UpperBB);
    if (!HasFastCoreType) {
      Value *IndicatorAddr = IRB.CreateGEP(
          CpuCTTy, CpuCoreType, {IRB.getInt64(0), IRB.getInt64(MAX_NUM_CPU)});
      Value *Indicator = IRB.CreateLoad(Int8Ty, IndicatorAddr);
      Value *IsCpuSupported =
          IRB.CreateICmpEQ(Indicator, IRB.getInt8(SUPPORT_CTD));
      FastDetectBB = BasicBlock::Create(M->getContext(), "",
                                        Intrin->getFunction(), LowerBB);
      IRB.CreateCondBr(IsCpuSupported, FastDetectBB, LowerBB);
    }

    // Use InlineAsm instead of x86_rdpid to get rid of rdpid predicates so
    // that we can emit rdpid instruction no matter if target supports this.
    // Since rdpid result may change over each execution, it should be marked as
    // hasSideEffects to prevent loop optimization like MachineLICM.
    IRB.SetInsertPoint(FastDetectBB);
    InlineAsm *RdpidAsm =
        InlineAsm::get(FunctionType::get(IRB.getInt32Ty(), /*isVarArg=*/false),
                       "rdpid ${0:q}", "=r", /*hasSideEffects=*/true);
    CallInst *RawPid = IRB.CreateCall(RdpidAsm, std::nullopt);
    Value *Pid = IRB.CreateAnd(RawPid, CPU_MASK);
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
    if (!HasFastCoreType)
      PHI->addIncoming(IRB.getInt8(0), UpperBB);
    PHI->addIncoming(CoreType0, FastDetectBB);
    PHI->addIncoming(CoreType1, DetectBB);
    Intrin->replaceAllUsesWith(PHI);
    Intrin->eraseFromParent();
  }
  return F.getNumUses();
}

bool X86PreISelIntrinsicLowering::lowerIntrinsics(Module &M) {
  bool Changed = false;
  for (Function &F : M) {
    switch (F.getIntrinsicID()) {
    default:
      break;
    case Intrinsic::x86_intel_fast_cpuid_coretype:
      const X86TargetMachine &TM =
          getAnalysis<TargetPassConfig>().getTM<X86TargetMachine>();
      assert(TM.Options.IntelLibIRCAllowed &&
             "x86_intel_fast_cpuid_coretype requires libirc");
      Changed |= lowerFastCpuIdCoreType(F, TM.getSubtargetImpl(F));
      break;
    }
  }
  return Changed;
}

INITIALIZE_PASS_BEGIN(X86PreISelIntrinsicLowering, DEBUG_TYPE,
                      "X86 Pre-ISel Intrinsic Lowering", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_END(X86PreISelIntrinsicLowering, DEBUG_TYPE,
                    "X86 Pre-ISel Intrinsic Lowering", false, false)

ModulePass *llvm::createX86PreISelIntrinsicLoweringPass() {
  return new X86PreISelIntrinsicLowering;
}
