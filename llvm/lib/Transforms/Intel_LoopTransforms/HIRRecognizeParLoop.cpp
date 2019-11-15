//===----- HIRRecognizeParLoop.h - Recognizes Parallel loops --------------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// The pass recognizes parallel loops and attaches HLLoopParallelTraits to such
// loops. First, the pass tries to find parallel OpenMP directives and extract
// necessary information from them. Then, if there is no directives,
// HIRParVecAnalysis is used.
//
// When using OpenMP directives, the pass finds the following HIR patterns:
//   HLInst(
//     %x = call token @llvm.directive.region.entry()["DIR.OMP.xxx.LOOP"(),...])
//   ...
//   HLLoop
//   ...
//   HLInst(call void @llvm.directive.region.exit(%x)["DIR.OMP.END.xxx.LOOP"()])
//
// Then adds information in the region.entry intrinsic into the HLLoop and
// removes both HLInsts.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRRecognizeParLoop.h"

#include "llvm/Analysis/Directives.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRParVecAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

using namespace llvm;
using namespace llvm::loopopt;

#define OPT_SWITCH "hir-recognize-par-loop"
#define OPT_DESC "HIR Recognize OpenMP Loops"
#define DEBUG_TYPE OPT_SWITCH

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
  cl::Hidden,
  cl::desc("Disable " OPT_DESC " pass"));

namespace {

// The old-style optimization pass
class HIRRecognizeParLoop : public HIRTransformPass {

public:
  static char ID;
  HIRRecognizeParLoop() : HIRTransformPass(ID) {
    initializeHIRRecognizeParLoopPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRParVecAnalysisWrapperPass>();
    AU.setPreservesAll();
  }
};
} // namespace

char HIRRecognizeParLoop::ID = 0;
INITIALIZE_PASS_BEGIN(HIRRecognizeParLoop, OPT_SWITCH, OPT_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRParVecAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRRecognizeParLoop, OPT_SWITCH, OPT_DESC, false, false)

namespace {
/// Actual implementation shared by the old- and new-style passes
class HIRRecognizeParLoopImpl {
public:
  /// The entry point.
  /// \param HIRF
  ///     the HIR created for the current function
  bool run(HIRFramework &HIRF);

private:
  /// Attempts to transform given loop.
  /// \param Lp
  ///     the loop
  /// \param HIRF
  ///     the HIR created for the current function
  /// \return
  ///     \c true if the loop was transformed, \c false otherwise
  bool doTransform(HLLoop &Lp, HIRFramework &HIRF);

  /// Searches for region entry and exit OpenMP intrinsics associated with
  /// given loop.
  /// \param Lp
  ///     the loop
  /// \param EntryInst
  ///     [out] filled with found region entry instruction or \c nullptr
  /// \param ExitInst
  ///     [out] filled with found region exit instruction or \c nullptr
  /// \return
  ///     the type of the found region represented with an OpenMP directive
  ///     (e.g. DIR_OMP_PARALLEL_LOOP or DIR_OMP_SIMD), \c -1 if not found
  int getOmpRegion(HLLoop &Lp, HLInst **EntryInst, HLInst **ExitInst);
};
} // namespace

FunctionPass *llvm::createHIRRecognizeParLoopPass() {
  return new HIRRecognizeParLoop();
}

bool HIRRecognizeParLoop::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    LLVM_DEBUG(dbgs() << OPT_DESC << " Disabled \n");
    return false;
  }
  HIRRecognizeParLoopImpl Impl;
  return Impl.run(getAnalysis<HIRFrameworkWrapperPass>().getHIR());
}

PreservedAnalyses
HIRRecognizeParLoopPass::run(llvm::Function &F,
                             llvm::FunctionAnalysisManager &AM) {
  HIRRecognizeParLoopImpl Impl;
  Impl.run(AM.getResult<HIRFrameworkAnalysis>(F));
  return PreservedAnalyses::all();
}

void HIRRecognizeParLoop::releaseMemory() {}

// Given the source:
//   void loop1(int *ip, int n) {
//   #pragma omp target map(from : ip[n])
//     {
//   #pragma omp parallel for num_threads(3)
//       for (int i = 0; i<n; i++) {
//         ip[i] = i;
//       }
//     }
//   }
// The typical region entry at this point would be:
// %1 = @llvm.directive.region.entry(); [
//   DIR.OMP.PARALLEL.LOOP(),
//   QUAL.OMP.NUM_THREADS(3),
//   QUAL.OMP.FIRSTPRIVATE(&((%.omp.lb)[0])),
//   QUAL.OMP.NORMALIZED.IV(null),
//   QUAL.OMP.NORMALIZED.UB(null),
//   QUAL.OMP.PRIVATE(&((%i)[0])),
//   QUAL.OMP.SHARED(&((%ip.addr)[0]))
// ]
// Note that IV and UB are nulls because mem-to-reg promotion of the
// induction variable and the upperbound happened beforehands.
//
static HLLoopParallelTraits* parseOmpRegion(const HLInst *Entry) {
  OMPRegionProxy OmpP(Entry);
  assert(OmpP.isValid() && "invalid region entry HIR inst");
  int RegionKind = OmpP.getOmpDir();

  if (RegionKind != DIR_OMP_PARALLEL_LOOP)
    llvm_unreachable("HIR supports only parallel loops for now");
  HLLoopParallelTraits* PTr = new HLLoopParallelTraits();

  for (unsigned I = 0; I < OmpP.getNumOmpClauses(); ++I) {
    int ClauseID = OmpP.getOmpClauseID(I);

    switch (ClauseID) {
    case QUAL_OMP_NUM_THREADS: {
      PTr->setNumThreads(OmpP.getOmpClauseSingleOpnd(I));
      break;
    }
    case QUAL_OMP_FIRSTPRIVATE:
    case QUAL_OMP_PRIVATE:
    case QUAL_OMP_SHARED:
    case QUAL_OMP_NORMALIZED_IV:
    case QUAL_OMP_NORMALIZED_UB:
    case QUAL_OMP_COLLAPSE:
#ifndef NDEBUG
      // TODO represent these clauses in HIR
      llvm::dbgs() << "HIR ignored OpenMP clause: " <<
        OmpP.getOmpClauseName(I) << "\n";
#endif // NDEBUG
      break;
    default:
      llvm_unreachable("clause not supported yet");
    }

  }
  return PTr;
}

bool HIRRecognizeParLoopImpl::doTransform(HLLoop &Lp, HIRFramework &HIRF) {
  HLInst *Entry = nullptr, *Exit = nullptr;
  HLLoopParallelTraits *CTr = nullptr;

  // First, try to parse OpenMP pragmas.
  if (getOmpRegion(Lp, &Entry, &Exit) >= 0) {
    CTr = parseOmpRegion(Entry);

    // Remove the consumed region entry/exit intrinsics.
    HLNodeUtils::remove(Entry);
    HLNodeUtils::remove(Exit);
  }

  // Then, try to refine the information based on the analysis.
  if (!CTr) {
    auto *HPVA = HIRF.getHIRAnalysisProvider().get<HIRParVecAnalysis>();
    const ParVecInfo *Info =
        HPVA ? HPVA->getInfo(ParVecInfo::Parallel, &Lp) : nullptr;
    if (Info && Info->getParType() == ParVecInfo::ParOkay)
      CTr = new HLLoopParallelTraits;
  }

  // Early return if the loop is not parallel.
  if (!CTr)
    return false;

  Lp.setParallelTraits(CTr);
  Lp.getParentRegion()->setGenCode();
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(&Lp);

  return true;
}

bool HIRRecognizeParLoopImpl::run(HIRFramework &HIRF) {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << OPT_DESC << " Disabled \n");
    return false;
  }
  SmallVector<HLLoop *, 64> CandidateLoops;
  HIRF.getHLNodeUtils().gatherAllLoops(CandidateLoops);

  if (CandidateLoops.empty()) {
    LLVM_DEBUG(dbgs() << HIRF.getFunction().getName() << "() has no loops\n ");
    return false;
  }
  bool Result = false;

  for (auto Lp : CandidateLoops) {
    Result = doTransform(*Lp, HIRF) || Result;
  }
  return Result;
}

static int getOmpRegionImpl(HLNode &Node, HLInst **EntryInst, HLInst **ExitInst) {
  *EntryInst = nullptr;
  *ExitInst = nullptr;
  int RegionKind = -1;

  // search for region entry
  for (auto Prev = Node.getPrevNode(); Prev; Prev = Prev->getPrevNode()) {
    HLInst *Inst = dyn_cast<HLInst>(Prev);

    if (!Inst)
      break; // Loop, IF, Switch, etc. - terminate the search

    // check if it is region entry ...
    int Kind = OMPRegionProxy::getOmpRegionEntryDir(Inst);

    if (Kind >= 0) {
      *EntryInst = Inst;
      assert(RegionKind == -1 && "multiple omp regions around a loop");
      RegionKind = Kind;
#ifdef NDEBUG
      break; // ... found - stop the search
#endif // NDEBUG
    }
  }
  if (!*EntryInst)
    return -1;

  // search for region exit
  for (auto Next = Node.getNextNode(); Next; Next = Next->getNextNode()) {
    HLInst *Inst = dyn_cast<HLInst>(Next);

    if (!Inst)
      llvm_unreachable("matching region exit not found");

    // check if it is region exit ...
    if (OMPRegionProxy::getOmpRegionExitDir(Inst, *EntryInst) >= 0) {
      assert(!*ExitInst && "orphan region exit?");
      *ExitInst = Inst;
#ifdef NDEBUG
      break; // ... found - stop the search
#endif
    }
  }
  assert(*ExitInst && "matching region exit not found (1)");
  return RegionKind;
}

// Assumptions:
// - there can be only one region entry marker per loop.
int HIRRecognizeParLoopImpl::getOmpRegion(HLLoop &Lp, HLInst **EntryInst,
                                          HLInst **ExitInst) {
  int Res = getOmpRegionImpl(Lp, EntryInst, ExitInst);

  // TODO FIXME Lp.hasZtt() returns false bacause loop formation did not set it
  // because ScalarEvolution::isLoopZtt test failed for this loop (apparenly
  // because of the OpenMP stuff); this needs to be fixed and the code below
  // must be updated to make sure Lp.getParent() is the ZTT.
  if (Res < 0)
    if (HLIf *Ztt = dyn_cast_or_null<HLIf>(Lp.getParent()))
      Res = getOmpRegionImpl(*Ztt, EntryInst, ExitInst);
  return Res;
}
