//==== DeleteFieldOP.cpp - Delete field with support for opaque pointers ====//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//
// This file implements the DTrans delete field optimization pass with support
// for IR using either opaque or non-opaque pointers.
//===---------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/DeleteFieldOP.h"

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOPOptBase.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

using namespace llvm;
using namespace dtransOP;

#define DEBUG_TYPE "dtrans-deletefieldop"

namespace {
class DTransDeleteFieldOPWrapper : public ModulePass {
private:
  dtransOP::DeleteFieldOPPass Impl;

public:
  static char ID;

  DTransDeleteFieldOPWrapper() : ModulePass(ID) {
    initializeDTransDeleteFieldOPWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    DTransSafetyAnalyzerWrapper &DTAnalysisWrapper =
        getAnalysis<DTransSafetyAnalyzerWrapper>();
    DTransSafetyInfo &DTInfo = DTAnalysisWrapper.getDTransSafetyInfo(M);

    auto GetTLI = [this](const Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    WholeProgramInfo &WPInfo =
        getAnalysis<WholeProgramWrapperPass>().getResult();
    bool Changed = Impl.runImpl(M, &DTInfo, WPInfo, GetTLI);
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransSafetyAnalyzerWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<TargetLibraryInfoWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

class DeleteFieldOPImpl : public DTransOPOptBase {
public:
  DeleteFieldOPImpl(LLVMContext &Ctx, DTransSafetyInfo *DTInfo,
                    StringRef DepTypePrefix, const DataLayout &DL,
                    DeleteFieldOPPass::GetTLIFn GetTLI)
      : DTransOPOptBase(Ctx, DTInfo, DepTypePrefix), DL(DL), GetTLI(GetTLI) {}

  bool prepareTypes(Module &M) override;
  void populateTypes(Module &M) override;
  GlobalVariable *createGlobalVariableReplacement(GlobalVariable *GV) override;
  void initializeGlobalVariableReplacement(GlobalVariable *OrigGV,
                                           GlobalVariable *NewGV,
                                           ValueMapper &Mapper) override;
  void postprocessGlobalVariable(GlobalVariable *OrigGV,
                                 GlobalVariable *NewGV) override;
  void processFunction(Function &F) override;
  void postprocessFunction(Function &OrigFunc, bool isCloned) override;

private:
  // TODO: Remove 'public' specifier when candidate selection is added.
  // Temporarily, declared as public to prevent build warning about unused
  // private variable.
public:
  const DataLayout &DL;

  // TODO: Remove 'public' specifier when function call size argument processing
  // code is added. Temporarily, declared as public to prevent build warning
  // about unused private variable.
public:
  DeleteFieldOPPass::GetTLIFn GetTLI;
};
} // end anonymous namespace

bool DeleteFieldOPImpl::prepareTypes(Module &M) {
  LLVM_DEBUG(dbgs() << "Delete field for opaque pointers: looking for "
                       "candidate structures.\n");

  // TODO: Create the opaque structures for the new types.
  return false;
}
void DeleteFieldOPImpl::populateTypes(Module &M) {
  // TODO: Populate the structure bodies for the new types.
}
GlobalVariable *
DeleteFieldOPImpl::createGlobalVariableReplacement(GlobalVariable *GV) {
  // TODO: Check if the global variable needs to be changed.
  return nullptr;
}
void DeleteFieldOPImpl::initializeGlobalVariableReplacement(
    GlobalVariable *OrigGV, GlobalVariable *NewGV, ValueMapper &Mapper) {
  // TODO: Set the initializer for a replaced global variable
}

void DeleteFieldOPImpl::postprocessGlobalVariable(GlobalVariable *OrigGV,
                                                  GlobalVariable *NewGV) {
    // TODO: Process all the GEPOperators that use a global variable that is
    // being changed
}

void DeleteFieldOPImpl::processFunction(Function &F) {
  // TODO: Update instructions in the function before type remapping occurs
}

void DeleteFieldOPImpl::postprocessFunction(Function &OrigFunc, bool isCloned) {
  // TODO: Update instructions in the function that need to be proceed after
  // type remapping
}

char DTransDeleteFieldOPWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransDeleteFieldOPWrapper, "dtrans-deletefieldop",
                      "DTrans delete field", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransSafetyAnalyzerWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransDeleteFieldOPWrapper, "dtrans-deletefieldop",
                    "DTrans delete field", false, false)

ModulePass *llvm::createDTransDeleteFieldOPWrapperPass() {
  return new DTransDeleteFieldOPWrapper();
}

PreservedAnalyses dtransOP::DeleteFieldOPPass::run(Module &M,
                                                   ModuleAnalysisManager &AM) {
  DTransSafetyInfo *DTInfo = &AM.getResult<DTransSafetyAnalyzer>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  bool Changed = runImpl(M, DTInfo, WPInfo, GetTLI);
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

bool dtransOP::DeleteFieldOPPass::runImpl(Module &M, DTransSafetyInfo *DTInfo,
                                          WholeProgramInfo &WPInfo,
                                          GetTLIFn GetTLI) {
  DeleteFieldOPImpl Transformer(M.getContext(), DTInfo, "__DFDT_",
                                M.getDataLayout(), GetTLI);
  return Transformer.run(M);
}
