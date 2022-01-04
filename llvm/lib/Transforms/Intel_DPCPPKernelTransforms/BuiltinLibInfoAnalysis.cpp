//===- BuiltinLibInfoAnalysis.cpp - Builtin RTL info analysis ---*- C++ -*-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-builtin-info-analysis"

INITIALIZE_PASS(BuiltinLibInfoAnalysisLegacy, DEBUG_TYPE,
                "Builtin runtime library info", false, true)

char BuiltinLibInfoAnalysisLegacy::ID = 0;

BuiltinLibInfoAnalysisLegacy::BuiltinLibInfoAnalysisLegacy() : ModulePass(ID) {
  initializeBuiltinLibInfoAnalysisLegacyPass(*PassRegistry::getPassRegistry());
}

ModulePass *llvm::createBuiltinLibInfoAnalysisLegacyPass() {
  return new BuiltinLibInfoAnalysisLegacy();
}

void BuiltinLibInfo::loadBuiltinModules(LLVMContext &Ctx) {
  BuiltinModules =
      DPCPPKernelCompilationUtils::loadBuiltinModulesFromCommandLine(Ctx);
  transform(BuiltinModules, std::back_inserter(BuiltinModuleRawPtrs),
            [](auto &BM) { return BM.get(); });
}

bool BuiltinLibInfo::invalidate(Module &, const PreservedAnalyses &PA,
                                ModuleAnalysisManager::Invalidator &) {
  // Check whether the analysis has been explicitly invalidated. Otherwise, it's
  // stateless and remains preserved.
  auto PAC = PA.getChecker<BuiltinLibInfoAnalysis>();
  return !PAC.preservedWhenStateless();
}

AnalysisKey BuiltinLibInfoAnalysis::Key;

BuiltinLibInfo BuiltinLibInfoAnalysis::run(Module &M, ModuleAnalysisManager &) {
  BuiltinLibInfo BLInfo;
  BLInfo.loadBuiltinModules(M.getContext());
  return BLInfo;
}

void BuiltinLibInfo::print(raw_ostream &OS) const {
  OS << "BuiltinLibInfo: number of builtin runtime libraries is "
     << BuiltinModules.size() << "\n";
}

PreservedAnalyses
BuiltinLibInfoAnalysisPrinter::run(Module &M, ModuleAnalysisManager &AM) {
  AM.getResult<BuiltinLibInfoAnalysis>(M).print(OS);
  return PreservedAnalyses::all();
}
