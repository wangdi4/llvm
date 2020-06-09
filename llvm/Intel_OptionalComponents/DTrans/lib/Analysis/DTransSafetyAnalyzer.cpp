//===----------------------DTransSafetyAnalyzer.cpp-----------------------===//
//
// Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"

#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "Intel_DTrans/DTransCommon.h"

#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "dtrans-safetyanalyzer"

using namespace llvm;
using namespace dtrans;

DTransSafetyInfo::DTransSafetyInfo(DTransSafetyInfo &&Other)
    : TM(std::move(Other.TM)), MDReader(std::move(Other.MDReader)),
      PtrAnalyzer(std::move(Other.PtrAnalyzer)),
      DTransSafetyAnalysisRan(Other.DTransSafetyAnalysisRan) {}

DTransSafetyInfo::~DTransSafetyInfo() { reset(); }

DTransSafetyInfo &DTransSafetyInfo::operator=(DTransSafetyInfo &&Other) {
  reset();
  TM = std::move(Other.TM);
  MDReader = std::move(Other.MDReader);
  PtrAnalyzer = std::move(Other.PtrAnalyzer);
  DTransSafetyAnalysisRan = Other.DTransSafetyAnalysisRan;
  return *this;
}

void DTransSafetyInfo::analyzeModule(
    Module &M,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
    WholeProgramInfo &WPInfo,
    function_ref<BlockFrequencyInfo &(Function &)> GetBFI) {

  LLVM_DEBUG(dbgs() << "DTransSafetyInfo::analyzeModule running\n");
  if (!WPInfo.isWholeProgramSafe()) {
    LLVM_DEBUG(dbgs() << "  DTransSafetyInfo: Not Whole Program Safe\n");
    return;
  }

  LLVMContext &Ctx = M.getContext();
  const DataLayout &DL = M.getDataLayout();
  TM = std::make_unique<DTransTypeManager>(Ctx);
  MDReader = std::make_unique<TypeMetadataReader>(*TM);
  if (!MDReader->initialize(M)) {
    LLVM_DEBUG(dbgs() << "DTransSafetyInfo: Type metadata reader did not find "
                         "structure type metadata\n");
    return;
  }

  PtrAnalyzer =
      std::make_unique<PtrTypeAnalyzer>(Ctx, *TM, *MDReader, DL, GetTLI);
  PtrAnalyzer->run(M);
  LLVM_DEBUG(dbgs() << "DTransSafetyInfo: PtrTypeAnalyzer complete\n");

  // TODO: Run safety checks on the IR
}

void DTransSafetyInfo::reset() {
  TM.reset();
  MDReader.reset();
  PtrAnalyzer.reset();
  DTransSafetyAnalysisRan = false;
}

bool DTransSafetyInfo::useDTransSafetyAnalysis() const {
  return DTransSafetyAnalysisRan;
}

// Provide a definition for the static class member used to identify passes.
AnalysisKey DTransSafetyAnalyzer::Key;

DTransSafetyInfo DTransSafetyAnalyzer::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetBFI = [&FAM](Function &F) -> BlockFrequencyInfo & {
    return FAM.getResult<BlockFrequencyAnalysis>(F);
  };
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  WholeProgramInfo &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  DTransSafetyInfo DTResult;
  DTResult.analyzeModule(M, GetTLI, WPInfo, GetBFI);
  return DTResult;
}

namespace {
class DTransSafetyAnalyzerWrapper : public ModulePass {

public:
  static char ID;

  DTransSafetyAnalyzerWrapper() : ModulePass(ID) {
    initializeDTransSafetyAnalyzerWrapperPass(*PassRegistry::getPassRegistry());
  }

  DTransSafetyInfo &getDTransSafetyInfo(Module &M) { return Result; }

  bool runOnModule(Module &M) override {
    auto GetBFI = [this](Function &F) -> BlockFrequencyInfo & {
      return this->getAnalysis<BlockFrequencyInfoWrapperPass>(F).getBFI();
    };
    auto GetTLI = [this](const Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    WholeProgramInfo &WPInfo =
        getAnalysis<WholeProgramWrapperPass>().getResult();
    Result.analyzeModule(M, GetTLI, WPInfo, GetBFI);
    return false;
  }

  bool doFinalization(Module &M) override {
    Result.reset();
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<BlockFrequencyInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
  }

private:
  DTransSafetyInfo Result;
};
} // end anonymous namespace

INITIALIZE_PASS_BEGIN(DTransSafetyAnalyzerWrapper, "dtrans-safetyanalyzer",
                      "Data transformation safety analyzer", false, true)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(BlockFrequencyInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransSafetyAnalyzerWrapper, "dtrans-safetyanalyzer",
                    "Data transformation safety analyzer", false, true)

char DTransSafetyAnalyzerWrapper::ID = 0;

ModulePass *llvm::createDTransSafetyAnalyzerTestWrapperPass() {
  return new DTransAnalysisWrapper();
}
