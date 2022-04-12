//==------- SinCosFold.cpp - Fold sin(), cos() into sincos() ----- C++ -*---==//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SinCosFold.h"
#include "NameMangleAPI.h"
#include "ParameterType.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/PassRegistry.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/RuntimeService.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-sin-cos-fold"

namespace {

/// Legacy SinCosFold pass.
class SinCosFoldLegacy : public FunctionPass {
  SinCosFoldPass Impl;

public:
  static char ID;

  SinCosFoldLegacy() : FunctionPass(ID) {
    initializeSinCosFoldLegacyPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "SinCosFoldLegacy"; }

  bool runOnFunction(Function &F) override {
    auto *RTS = getAnalysis<BuiltinLibInfoAnalysisLegacy>()
                    .getResult()
                    .getRuntimeService();
    return Impl.runImpl(F, RTS);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<BuiltinLibInfoAnalysisLegacy>();
  }
};

} // namespace

char SinCosFoldLegacy::ID = 0;
INITIALIZE_PASS_BEGIN(SinCosFoldLegacy, DEBUG_TYPE, "SinCosFoldLegacy", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfoAnalysisLegacy)
INITIALIZE_PASS_END(SinCosFoldLegacy, DEBUG_TYPE, "SinCosFoldLegacy", false,
                    false)

FunctionPass *llvm::createSinCosFoldLegacyPass() {
  return new SinCosFoldLegacy();
}

struct SinCosPairData {
  SinCosPairData(CallInst *SinCall, CallInst *CosCall, bool IsNativeCall,
                 reflection::RefParamType RefType)
      : SinCall(SinCall), CosCall(CosCall), IsNativeCall(IsNativeCall),
        RefType(RefType) {}
  bool isComplete() const { return SinCall && CosCall; }
  CallInst *getFirstCall() const {
    assert(SinCall->getParent() == CosCall->getParent() &&
           "The sin, cos pair must be in the same block!");
    return SinCall->comesBefore(CosCall) ? SinCall : CosCall;
  }

  CallInst *SinCall = nullptr;
  CallInst *CosCall = nullptr;
  bool IsNativeCall = false;

  reflection::RefParamType RefType;
};

using ArgToPairsType = MapVector<Value *, SmallVector<SinCosPairData, 1>>;

static void pushSinOrCosCall(Value *Arg, CallInst *CI, bool IsSin, bool IsCos,
                             bool IsNative, reflection::RefParamType RefType,
                             ArgToPairsType &ArgToSinCosPairs) {
  auto &SinCosPairs = ArgToSinCosPairs[Arg];
  bool Matched = false;
  assert(!(IsSin && IsCos) &&
         "A call cannot be both sin and cos at the same time!");
  // Try to match CI with an existing incomplete pair.
  for (auto &Pair : SinCosPairs) {
    if (IsNative != Pair.IsNativeCall)
      continue;
    if (IsSin && !Pair.SinCall &&
        CI->getParent() == Pair.CosCall->getParent()) {
      Matched = true;
      Pair.SinCall = CI;
      LLVM_DEBUG(dbgs() << "Paired " << *CI << " with " << *Pair.CosCall
                        << '\n');
      break;
    }
    if (IsCos && !Pair.CosCall &&
        CI->getParent() == Pair.SinCall->getParent()) {
      Matched = true;
      Pair.CosCall = CI;
      LLVM_DEBUG(dbgs() << "Paired " << *CI << " with " << *Pair.SinCall
                        << '\n');
      break;
    }
  }

  // Create a new incomplete pair if not matched.
  if (!Matched)
    SinCosPairs.emplace_back(IsSin ? CI : nullptr, IsCos ? CI : nullptr,
                             IsNative, RefType);
}

static bool isSinCosName(StringRef FuncName, bool &IsSin, bool &IsCos,
                         bool &IsNative) {
  IsNative = FuncName.consume_front("native_");
  IsSin = FuncName.equals("sin");
  IsCos = FuncName.equals("cos");
  return IsSin || IsCos;
}

static void collectSinCosPairs(Function &F, ArgToPairsType &ArgToSinCosPairs) {
  for (auto &BB : F) {
    for (auto &I : BB) {
      if (!isa<CallInst>(I))
        continue;
      auto &CI = cast<CallInst>(I);
      Function *Callee = CI.getCalledFunction();
      if (!Callee)
        continue;

      auto FDesc = NameMangleAPI::demangle(Callee->getName());
      bool IsSin, IsCos, IsNative;
      if (!isSinCosName(FDesc.Name, IsSin, IsCos, IsNative))
        continue;
      pushSinOrCosCall(CI.getArgOperand(0), &CI, IsSin, IsCos, IsNative,
                       FDesc.Parameters[0], ArgToSinCosPairs);
    }
  }
}

static bool foldSinCosPairs(ArgToPairsType &ArgToSinCosPairs,
                            BasicBlock &EntryBlock, Module *M,
                            RuntimeService *RTS) {
  bool Changed = false;
  for (auto &Item : ArgToSinCosPairs) {
    Value *Arg = Item.first;
    Type *DataType = Arg->getType();
    for (SinCosPairData &Pair : Item.second) {
      if (!Pair.isComplete())
        continue;
      auto *CosAlloca = new AllocaInst(DataType, 0, "cos.ptr",
                                       &*EntryBlock.getFirstInsertionPt());

      // Get sincos mangled name.
      reflection::FunctionDescriptor FDesc;
      FDesc.Name = Pair.IsNativeCall ? "native_sincos" : "sincos";
      reflection::RefParamType CosPtrRefTy(new reflection::PointerType(
          Pair.RefType, {reflection::ATTR_PRIVATE}));
      FDesc.Parameters.push_back(Pair.RefType);
      FDesc.Parameters.push_back(CosPtrRefTy);
      std::string MangledSincosFuncName = NameMangleAPI::mangle(FDesc);

      Function *SincosF =
          RTS->findFunctionInBuiltinModules(MangledSincosFuncName);
      assert(SincosF && "sincos function not found in builtin modules!");

      SincosF = DPCPPKernelCompilationUtils::importFunctionDecl(M, SincosF);
      assert(SincosF && "Failed to import sincos into current module!");

      SmallVector<Value *, 2> Args{Arg, CosAlloca};
      CallInst *SincosCall =
          CallInst::Create(SincosF, Args, "sin.val", Pair.getFirstCall());
      SincosCall->setDebugLoc(Pair.SinCall->getDebugLoc());
      LoadInst *CosVal =
          new LoadInst(DataType, CosAlloca, "cos.val", Pair.getFirstCall());
      CosVal->setDebugLoc(Pair.CosCall->getDebugLoc());

      Pair.SinCall->replaceAllUsesWith(SincosCall);
      Pair.SinCall->eraseFromParent();
      Pair.CosCall->replaceAllUsesWith(CosVal);
      Pair.CosCall->eraseFromParent();

      Changed = true;
    }
  }
  return Changed;
}

bool SinCosFoldPass::runImpl(Function &F, RuntimeService *RTS) {
  ArgToPairsType ArgToSinCosPairs;
  collectSinCosPairs(F, ArgToSinCosPairs);
  return foldSinCosPairs(ArgToSinCosPairs, F.getEntryBlock(), F.getParent(),
                         RTS);
}

PreservedAnalyses SinCosFoldPass::run(Function &F,
                                      FunctionAnalysisManager &FAM) {
  auto *RTS = FAM.getResult<ModuleAnalysisManagerFunctionProxy>(F)
                  .getCachedResult<BuiltinLibInfoAnalysis>(*F.getParent())
                  ->getRuntimeService();
  assert(RTS && "Runtime service is not valid");
  if (!runImpl(F, RTS))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}
