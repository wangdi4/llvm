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

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
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

static cl::opt<bool> RunArrayScalarizationMemRefs(
    "run-" OPT_SWITCH "-array-scalarization-memrefs", cl::init(false),
    cl::Hidden, cl::desc("Run " OPT_DESC " Array Scalarization with MemRefs"));

static cl::opt<bool> RunArrayScalarizationSymbases(
    "run-" OPT_SWITCH "-array-scalarization-symbases", cl::init(false),
    cl::Hidden, cl::desc("Run " OPT_DESC " Array Scalarization with Symbases"));

static cl::list<unsigned>
    ArrayScalarizationSymbases(OPT_SWITCH "-array-scalarization-symbases",
                               cl::CommaSeparated,
                               cl::desc("Symbases for Array Scalarization"));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static void printRefVector(SmallVectorImpl<const RegDDRef *> &RefVec,
                           std::string Msg, bool PrintPointer = false) {
  formatted_raw_ostream FOS(dbgs());
  if (Msg.size())
    FOS << Msg << ": " << RefVec.size() << "\n";
  unsigned Count = 0;
  for (const RegDDRef *Ref : RefVec) {
    FOS << Count++ << "\t";
    if (PrintPointer) {
      FOS << Ref << "\t";
    }
    Ref->print(FOS, true);
    FOS << " : ";
    (Ref->isLval()) ? FOS << "W" : FOS << "R";
    FOS << "\n";
  }
}
#endif

namespace {
class HIRArrayScalarizationTestLauncherLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRArrayScalarizationTestLauncherLegacyPass() : HIRTransformPass(ID) {
    initializeHIRArrayScalarizationTestLauncherLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.setPreservesAll();
  }
};

class HIRArrayScalarizationTestLauncher {
  HIRFramework &HIRF;
  HIRDDAnalysis &HDDA;
  HLNodeUtils &HNU;

  // Run array scalarization on a given vector of MemRefs.
  bool runArrayScalarizationMemRefs(HLRegion &Region);

  // Run array scalarization on a given set of symbases.
  bool runArrayScalarizationSymbases(HLRegion &Region);

public:
  HIRArrayScalarizationTestLauncher(HIRFramework &HIRF, HIRDDAnalysis &HDDA)
      : HIRF(HIRF), HDDA(HDDA), HNU(HIRF.getHLNodeUtils()) {}

  bool run(HLRegion &Reg);
};

} // namespace

char HIRArrayScalarizationTestLauncherLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRArrayScalarizationTestLauncherLegacyPass, OPT_SWITCH,
                      OPT_DESC, false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRArrayScalarizationTestLauncherLegacyPass, OPT_SWITCH,
                    OPT_DESC, false, false)

FunctionPass *llvm::createHIRArrayScalarizationTestLauncherPass() {
  return new HIRArrayScalarizationTestLauncherLegacyPass();
}

bool HIRArrayScalarizationTestLauncher::runArrayScalarizationMemRefs(
    HLRegion &Region) {
  bool Result = false;

  // Collect loop(s):
  SmallVector<HLLoop *, 64> CandidateLoops;
  HNU.gatherInnermostLoops(CandidateLoops);
  if (CandidateLoops.empty()) {
    LLVM_DEBUG(dbgs() << "No loop collected\n";);
    return false;
  }

  for (HLLoop *Lp : CandidateLoops) {
    // Collect equal memref(s) group(s) in Lp:
    HIRLoopLocality::RefGroupVecTy EqualityGroups;
    SmallSet<unsigned, 8> UniqueGroupSymbases;

    HIRLoopLocality::populateEqualityGroups(Lp->child_begin(), Lp->child_end(),
                                            EqualityGroups,
                                            &UniqueGroupSymbases);
    if (EqualityGroups.empty()) {
      LLVM_DEBUG(dbgs() << "No memref(s) in Lp: " << Lp << "\n";);
      continue;
    }

    HIRArrayScalarization HAS(HIRF, HDDA);

    for (auto &RefVec : EqualityGroups) {
      // Sort the group in DESC TOPO order:
      llvm::sort(RefVec.begin(), RefVec.end(),
                 [](const RegDDRef *Ref0, const RegDDRef *Ref1) {
                   return Ref0->getHLDDNode()->getTopSortNum() <
                          Ref1->getHLDDNode()->getTopSortNum();
                 });
      LLVM_DEBUG(printRefVector(RefVec, "RefVec: "););

      // cast off the const qualifier:
      SmallVector<RegDDRef *, 4> NCRefVec;
      for (auto &Ref : RefVec) {
        NCRefVec.push_back(const_cast<RegDDRef *>(Ref));
      }

      Result = HAS.doScalarization(Lp, NCRefVec) || Result;
    }

    LLVM_DEBUG(Lp->dump(););
  }

  return Result;
}

bool HIRArrayScalarizationTestLauncher::runArrayScalarizationSymbases(
    HLRegion &Region) {
  LLVM_DEBUG(Region.dump(true););

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
  HNU.gatherInnermostLoops(CandidateLoops);
  if (CandidateLoops.size() != 1) {
    LLVM_DEBUG(dbgs() << "Expect 1 loop(nest) in test input\n";);
    return false;
  }
  HLLoop *Lp = CandidateLoops.front();
  assert(Lp && "Expect a valid Loop");

  return HIRTransformUtils::doScalarization(HIRF, HDDA, Lp, SymbaseSet);
}

bool HIRArrayScalarizationTestLauncher::run(HLRegion &Reg) {
  bool Result = false;

  // Use an if-then-else structure to work around the possibility that both
  // RunArrayScalarizationMemRefs and RunArrayScalarizationSymbases flags are
  // true accidentally.
  if (RunArrayScalarizationMemRefs) {
    Result = runArrayScalarizationMemRefs(Reg) || Result;
  } else if (RunArrayScalarizationSymbases) {
    Result = runArrayScalarizationSymbases(Reg) || Result;
  }

  return Result;
}

static bool runHIRArrayScalarizationTestLauncher(HIRFramework &HIRF,
                                                 HIRDDAnalysis &HDDA) {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << OPT_DESC " Disabled \n");
    return false;
  }

  bool Result = false;
  HIRArrayScalarizationTestLauncher HASTL(HIRF, HDDA);

  for (auto &Reg : make_range(HIRF.hir_begin(), HIRF.hir_end())) {
    Result = HASTL.run(cast<HLRegion>(Reg)) || Result;
  }

  return Result;
}

bool HIRArrayScalarizationTestLauncherLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    LLVM_DEBUG(dbgs() << OPT_DESC " Disabled \n");
    return false;
  }

  return runHIRArrayScalarizationTestLauncher(
      getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
      getAnalysis<HIRDDAnalysisWrapperPass>().getDDA());
}

PreservedAnalyses HIRArrayScalarizationTestLauncherPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  runHIRArrayScalarizationTestLauncher(HIRF,
                                       AM.getResult<HIRDDAnalysisPass>(F));
  return PreservedAnalyses::all();
}
