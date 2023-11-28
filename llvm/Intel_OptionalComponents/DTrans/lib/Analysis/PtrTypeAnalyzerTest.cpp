//===---------------------PtrTypeAnalyzerTest.cpp-------------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/PtrTypeAnalyzerTest.h"

#if !INTEL_PRODUCT_RELEASE

#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace dtransOP;

#define DEBUG_TYPE "dtrans-ptrtypeanalyzertest"

namespace {

// Class that executes pointer type analyzer to produce analysis traces for
// testing the behavior of the analysis.
class PtrTypeAnalyzerTest {
private:
  DTransTypeManager TM;
  TypeMetadataReader Reader;

public:
  PtrTypeAnalyzerTest(LLVMContext &Ctx) : TM(Ctx), Reader(TM) {}

  void
  runImpl(Module &M,
          std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
          WholeProgramInfo &WPInfo) {

    if (!WPInfo.isWholeProgramSafe()) {
      LLVM_DEBUG(dbgs() << "Module is not whole program safe\n");
      return;
    }
    LLVMContext &Ctx = M.getContext();
    DTransTypeManager TM(Ctx);
    TypeMetadataReader Reader(TM);
    if (!Reader.initialize(M)) {
      LLVM_DEBUG(dbgs() << "Failed to initialize type metadata reader\n");
      return;
    }
    const DataLayout &DL = M.getDataLayout();
    PtrTypeAnalyzer Analyzer(Ctx, TM, Reader, DL, GetTLI);
    Analyzer.run(M);
  }
};

} // end anonymous namespace

// Interface for new pass manager
namespace llvm {
namespace dtransOP {

PreservedAnalyses
DTransPtrTypeAnalyzerTestPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  PtrTypeAnalyzerTest Tester(M.getContext());
  Tester.runImpl(M, GetTLI, WPInfo);

  return PreservedAnalyses::all();
}

} // end namespace dtransOP
} // end namespace llvm

#endif // !INTEL_PRODUCT_RELEASE
