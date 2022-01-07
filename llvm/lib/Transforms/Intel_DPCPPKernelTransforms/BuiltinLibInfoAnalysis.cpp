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
#include "llvm/IRReader/IRReader.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-builtin-info-analysis"

static cl::list<std::string>
    OptBuiltinModuleFiles(cl::CommaSeparated, "dpcpp-kernel-builtin-lib",
                          cl::desc("Builtin declarations (bitcode) libraries"),
                          cl::value_desc("filename1,filename2"));

INITIALIZE_PASS(BuiltinLibInfoAnalysisLegacy, DEBUG_TYPE,
                "Builtin runtime library info", false, true)

char BuiltinLibInfoAnalysisLegacy::ID = 0;

BuiltinLibInfoAnalysisLegacy::BuiltinLibInfoAnalysisLegacy(
    ArrayRef<Module *> BuiltinModules)
    : ImmutablePass(ID), BLInfo(BuiltinModules) {
  initializeBuiltinLibInfoAnalysisLegacyPass(*PassRegistry::getPassRegistry());
}

ImmutablePass *llvm::createBuiltinLibInfoAnalysisLegacyPass(
    ArrayRef<Module *> BuiltinModules) {
  return new BuiltinLibInfoAnalysisLegacy(BuiltinModules);
}

void BuiltinLibInfo::loadBuiltinModules(Module &M) {
  if (!BuiltinModuleRawPtrs.empty())
    return;

  auto &Ctx = M.getContext();
  for (auto &ModuleFile : OptBuiltinModuleFiles) {
    if (ModuleFile.empty()) {
      BuiltinModules.push_back(std::make_unique<Module>("empty", Ctx));
    } else {
      SMDiagnostic Err;
      std::unique_ptr<Module> BuiltinModule =
          getLazyIRFileModule(ModuleFile, Err, Ctx);
      assert(BuiltinModule && "failed to load builtin lib from file");
      BuiltinModules.push_back(std::move(BuiltinModule));
    }
  }
  transform(BuiltinModules, std::back_inserter(BuiltinModuleRawPtrs),
            [](auto &M) { return M.get(); });

  // Builtin module may contain platform independent byte code, so we set triple
  // and data layout in order to avoid warnings from linker.
  for (auto &BM : BuiltinModules) {
    BM->setTargetTriple(M.getTargetTriple());
    BM->setDataLayout(M.getDataLayout());
  }

  RTService.reset(new RuntimeService());
}

bool BuiltinLibInfo::invalidate(Module &, const PreservedAnalyses &PA,
                                ModuleAnalysisManager::Invalidator &) {
  // Check whether the analysis has been explicitly invalidated. Otherwise, it's
  // stateless and remains preserved.
  auto PAC = PA.getChecker<BuiltinLibInfoAnalysis>();
  return !PAC.preservedWhenStateless();
}

AnalysisKey BuiltinLibInfoAnalysis::Key;

void BuiltinLibInfo::print(raw_ostream &OS) const {
  OS << "BuiltinLibInfo: number of builtin runtime libraries is "
     << BuiltinModuleRawPtrs.size() << "\n";
}

BuiltinLibInfo BuiltinLibInfoAnalysis::run(Module &M, ModuleAnalysisManager &) {
  BuiltinLibInfo BLInfo(BuiltinModules);
  BLInfo.loadBuiltinModules(M);
  return BLInfo;
}

PreservedAnalyses
BuiltinLibInfoAnalysisPrinter::run(Module &M, ModuleAnalysisManager &AM) {
  AM.getResult<BuiltinLibInfoAnalysis>(M).print(OS);
  return PreservedAnalyses::all();
}
