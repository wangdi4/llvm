//===----- HIRRecognizeOmpLoop.h - Recognizes OpenMP loops ----------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Finds HIR patterns:
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

#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/Intel_Directives.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRRecognizeOmpLoop.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

using namespace llvm;
using namespace llvm::loopopt;

#define OPT_SWITCH "hir-rec-omp-loop"
#define OPT_DESC "HIR Recognize OpenMP Loops"
#define DEBUG_TYPE OPT_SWITCH

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
  cl::Hidden,
  cl::desc("Disable " OPT_DESC " pass"));

namespace {

// The old-style optimization pass
class HIRRecognizeOmpLoop : public HIRTransformPass {

public:
  static char ID;
  HIRRecognizeOmpLoop() : HIRTransformPass(ID) {
    initializeHIRRecognizeOmpLoopPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.setPreservesAll();
  }
};
} // namespace

char HIRRecognizeOmpLoop::ID = 0;
INITIALIZE_PASS_BEGIN(HIRRecognizeOmpLoop, OPT_SWITCH, OPT_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_END(HIRRecognizeOmpLoop, OPT_SWITCH, OPT_DESC, false, false)

namespace {
/// Actual implementation shared by the old- and new-style passes
class HIRRecognizeOmpLoopImpl {
public:
  /// The entry point.
  /// \param HIRF
  ///     the HIR created for the current function
  bool run(HIRFramework &HIRF);

private:
  /// Attempts to transform given loop.
  /// \param Lp
  ///     the loop
  /// \return
  ///     \c true if the loop was transformed, \c false otherwise
  bool doTransform(HLLoop &Lp);

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

FunctionPass *llvm::createHIRRecognizeOmpLoopPass() {
  return new HIRRecognizeOmpLoop();
}

bool HIRRecognizeOmpLoop::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    LLVM_DEBUG(dbgs() << OPT_DESC << " Disabled \n");
    return false;
  }
  HIRRecognizeOmpLoopImpl Impl;
  return Impl.run(getAnalysis<HIRFrameworkWrapperPass>().getHIR());
}

PreservedAnalyses
HIRRecognizeOmpLoopPass::run(llvm::Function &F,
  llvm::FunctionAnalysisManager &AM) {
  HIRRecognizeOmpLoopImpl Impl;
  Impl.run(AM.getResult<HIRFrameworkAnalysis>(F));
  return PreservedAnalyses::all();
}

void HIRRecognizeOmpLoop::releaseMemory() {}

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

bool HIRRecognizeOmpLoopImpl::doTransform(HLLoop &Lp) {
  HLInst *Entry = nullptr, *Exit = nullptr;

  if (getOmpRegion(Lp, &Entry, &Exit) < 0)
    return false;
  HLLoopParallelTraits *CTr = parseOmpRegion(Entry);
  Lp.setParallelTraits(CTr);

  // now remove the consumed region entry/exit intrinsics
  HLNodeUtils::remove(Entry);
  HLNodeUtils::remove(Exit);
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(&Lp);
  return true;
}

bool HIRRecognizeOmpLoopImpl::run(HIRFramework &HIRF) {
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
    Result = doTransform(*Lp) || Result;
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
int HIRRecognizeOmpLoopImpl::getOmpRegion(HLLoop &Lp, HLInst **EntryInst,
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