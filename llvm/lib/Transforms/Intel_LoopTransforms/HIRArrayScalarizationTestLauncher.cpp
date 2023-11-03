//===- HIRArrayScalarizationTestLauncher.cpp -                             ===//
// Implements HIR Array-Scalarization Test Launcher Pass.
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRArrayScalarizationTestLauncherPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "HIRArrayScalarization.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

#define OPT_SWITCH "hir-arrayscalarization-test-launcher"
#define OPT_DESC "HIR Array Scalarization Test Launcher"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::arrayscalarization;

typedef DDRefGrouping::RefGroupVecTy<const RegDDRef *> RefGroupVecTy;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(true),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

static cl::list<unsigned>
    ArrayScalarizationSymbases(OPT_SWITCH "-array-scalarization-symbases",
                               cl::CommaSeparated,
                               cl::desc("Symbases for Array Scalarization"));

namespace {
class HIRArrayScalarizationTestLauncher {

public:
  bool run(HLRegion &Reg);
};

} // namespace

bool HIRArrayScalarizationTestLauncher::run(HLRegion &Reg) {
  // Check: a non-empty list of unsigned integers as vector of symbase:
  if (ArrayScalarizationSymbases.empty()) {
    LLVM_DEBUG(dbgs() << "Empty Symbase\n";);
    return false;
  }
  SmallSet<unsigned, 8> SymbaseSet;
  for (auto SB : ArrayScalarizationSymbases) {
    SymbaseSet.insert(SB);
  }

  LLVM_DEBUG({
    dbgs() << "set of symbase(s): ";
    std::for_each(SymbaseSet.begin(), SymbaseSet.end(),
                  [&](unsigned SB) { dbgs() << SB << ","; });
    dbgs() << "\n";
  });

  // Collect loop(s): expect a single loop from test input
  SmallVector<HLLoop *, 64> CandidateLoops;
  HLNodeUtils &HNU = Reg.getHLNodeUtils();
  HNU.gatherInnermostLoops(CandidateLoops);
  if (CandidateLoops.size() != 1) {
    LLVM_DEBUG(dbgs() << "Expect 1 loop(nest) in test input\n";);
    return false;
  }
  HLLoop *Lp = CandidateLoops.front();
  assert(Lp && "Expect a valid Loop");

  return HIRTransformUtils::doArrayScalarization(Lp, SymbaseSet);
}

static bool runHIRArrayScalarizationTestLauncher(HIRFramework &HIRF) {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << OPT_DESC " Disabled \n");
    return false;
  }

  bool Result = false;
  HIRArrayScalarizationTestLauncher HASTL;

  for (auto &Reg : make_range(HIRF.hir_begin(), HIRF.hir_end())) {
    Result = HASTL.run(cast<HLRegion>(Reg)) || Result;
  }

  return Result;
}

PreservedAnalyses HIRArrayScalarizationTestLauncherPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  runHIRArrayScalarizationTestLauncher(HIRF);
  return PreservedAnalyses::all();
}
