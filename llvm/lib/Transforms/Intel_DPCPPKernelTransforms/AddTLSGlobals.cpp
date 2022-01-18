//===--- AddTLSGlobals.cpp - AddTLSGlobals pass - C++ -* ------------------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/AddTLSGlobals.h"
#include "ImplicitArgsUtils.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"

using namespace llvm;

// Command line option for other passes to use TLS mode
bool EnableTLSGlobals;
static cl::opt<bool, true> OptEnableTLSGlobals(
    "dpcpp-kernel-enable-tls-globals", cl::desc("Enable TLS globals"),
    cl::location(EnableTLSGlobals), cl::init(false), cl::Hidden);

namespace {
/// @brief  AddTLSGlobals class adds TLS global variables to the module
class AddTLSGlobalsLegacy : public ModulePass {

public:
  static char ID;

  AddTLSGlobalsLegacy();

  StringRef getPassName() const override { return "AddTLSGlobalsLegacy"; }

  bool runOnModule(Module &M) override;

  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    // Depends on LocalBuffAnalysis for finding all local buffers each function
    // uses directly
    AU.addRequired<LocalBufferAnalysisLegacy>();
    AU.addRequired<ImplicitArgsAnalysisLegacy>();
    AU.addPreserved<ImplicitArgsAnalysisLegacy>();
  }

private:
  AddTLSGlobalsPass Impl;
};

} // namespace

char AddTLSGlobalsLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(AddTLSGlobalsLegacy, "dpcpp-kernel-add-tls-globals",
                      "Adds TLS global variables to the module", false, false)
INITIALIZE_PASS_DEPENDENCY(LocalBufferAnalysisLegacy)
INITIALIZE_PASS_DEPENDENCY(ImplicitArgsAnalysisLegacy)
INITIALIZE_PASS_END(AddTLSGlobalsLegacy, "dpcpp-kernel-add-tls-globals",
                    "Adds TLS global variables to the module", false, false)

AddTLSGlobalsLegacy::AddTLSGlobalsLegacy() : ModulePass(ID) {
  initializeAddTLSGlobalsLegacyPass(*llvm::PassRegistry::getPassRegistry());
}

bool AddTLSGlobalsLegacy::runOnModule(Module &M) {
  LocalBufferInfo *LBInfo =
      &getAnalysis<LocalBufferAnalysisLegacy>().getResult();
  ImplicitArgsAnalysisLegacy *IAA = &getAnalysis<ImplicitArgsAnalysisLegacy>();
  ImplicitArgsInfo *IAInfo = &(IAA->getResult());
  return Impl.runImpl(M, LBInfo, IAInfo);
}

PreservedAnalyses AddTLSGlobalsPass::run(Module &M, ModuleAnalysisManager &AM) {
  LocalBufferInfo *LBInfo = &AM.getResult<LocalBufferAnalysis>(M);
  ImplicitArgsInfo *IAInfo = &AM.getResult<ImplicitArgsAnalysis>(M);
  if (!runImpl(M, LBInfo, IAInfo))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<ImplicitArgsAnalysis>();
  return PA;
}

bool AddTLSGlobalsPass::runImpl(Module &M, LocalBufferInfo *LBInfo,
                                ImplicitArgsInfo *IAInfo) {
  this->LBInfo = LBInfo;
  this->M = &M;
  Ctx = &M.getContext();

  // Create TLS globals
  for (unsigned I = 0; I < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++I) {
    // TODO handle name conflicts
    assert(!M.getGlobalVariable(ImplicitArgsUtils::getArgName(I)));
    Type *ArgType = IAInfo->getArgType(I);
    GlobalVariable *GV = new GlobalVariable(
        M, ArgType, false, GlobalValue::LinkOnceODRLinkage,
        UndefValue::get(ArgType), ImplicitArgsUtils::getArgName(I), nullptr,
        GlobalValue::GeneralDynamicTLSModel);
    GV->setAlignment(M.getDataLayout().getPreferredAlign(GV));
    if (I == ImplicitArgsUtils::IA_SLM_BUFFER)
      LocalMemBase = GV;
  }

  // Collect all module functions that are not declarations for handling
  SmallVector<Function *> FunctionsToHandle;
  for (Function &Func : M) {
    if (Func.isDeclaration()) {
      // Function is not defined inside module
      continue;
    }
    // No need to handle global ctors/dtors
    if (DPCPPKernelCompilationUtils::isGlobalCtorDtorOrCPPFunc(&Func))
      continue;

    FunctionsToHandle.push_back(&Func);
  }

  // Run on all collected functions for handling and handle them
  for (Function *Func : FunctionsToHandle)
    runOnFunction(Func);

  return true;
}

void AddTLSGlobalsPass::runOnFunction(Function *Func) {
  llvm::DataLayout DL(M);

  // Calculate pointer to the local memory buffer
  unsigned int DirectLocalSize =
      (unsigned int)LBInfo->getDirectLocalsSize(Func);

  // Go through function instructions and search for calls
  for (BasicBlock &BB : *Func) {
    for (Instruction &Inst : BB) {
      if (auto *Call = dyn_cast<CallInst>(&Inst)) {
        // Check call for not inlined module function
        // TODO Updating LocalMemBase in this manner renders debugger unable to
        // call functions that use local buffers properly, they should be
        // handled differently
        Function *Callee = Call->getCalledFunction();
        if (nullptr != Callee && !Callee->isDeclaration()) {
          IRBuilder<> B(Call);
          Value *Load =
              B.CreateLoad(LocalMemBase->getValueType(), LocalMemBase);
          std::string ValName("pLocalMem_");
          ValName += Callee->getName();
          Value *NewLocalMem = B.CreateGEP(
              Load->getType()->getScalarType()->getPointerElementType(), Load,
              ConstantInt::get(IntegerType::get(*Ctx, 32), DirectLocalSize),
              ValName);

          // Now that the local memory buffer is rebased, the memory location of
          // directly used local values should be passed to the callee so that
          // local values can be loaded/stored correctly by callee. In this way,
          // the pointers to local values are pushed to new local memory buffer
          // base, and then callee can access local values via the pointers.
          auto &CalleeLocalSet = LBInfo->getDirectLocals(Callee);
          Type *Ty =
              NewLocalMem->getType()->getScalarType()->getPointerElementType();
          unsigned int CurrLocalOffset = 0;
          for (auto *Local : CalleeLocalSet) {
            GlobalVariable *GV = cast<GlobalVariable>(Local);
            size_t ArraySize = DL.getTypeAllocSize(GV->getValueType());
            assert(0 != ArraySize && "zero array size!");
            // Get the position of Local in NewLocalMem.
            auto *Idx =
                ConstantInt::get(IntegerType::get(*Ctx, 32), CurrLocalOffset);
            auto *LocalPtr = B.CreateGEP(Ty, NewLocalMem, Idx);
            // Cast to pointer type of Local.
            auto *Cast =
                B.CreatePointerCast(LocalPtr, GV->getType()->getPointerTo());
            // Store Local to NewLocalMem.
            B.CreateStore(GV, Cast);
            // Calculate the offset of next direct used local value.
            CurrLocalOffset += ADJUST_SIZE_TO_MAXIMUM_ALIGN(ArraySize);
          }

          B.CreateStore(NewLocalMem, LocalMemBase);
          B.SetInsertPoint(Call->getNextNode());
          B.SetCurrentDebugLocation(Call->getDebugLoc());
          B.CreateStore(Load, LocalMemBase);
        }
      }
    }
  }
}

ModulePass *llvm::createAddTLSGlobalsLegacyPass() {
  return new AddTLSGlobalsLegacy();
}
