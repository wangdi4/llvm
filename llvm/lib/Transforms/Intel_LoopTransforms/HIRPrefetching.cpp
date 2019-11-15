//===----------- HIRPrefetching.cpp Implements Prefetching class ---------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass implements software prefetching, when the number of memrefs exceeds
// the threshold in the loop. It prefetches the memref by the intrinsic calls in
// the pre-header.
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRPrefetching.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#define OPT_SWITCH "hir-prefetching"
#define OPT_DESC "HIR Prefetching"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

typedef DDRefGrouping::RefGroupTy<const RegDDRef *> RefGroupTy;
typedef DDRefGrouping::RefGroupVecTy<const RegDDRef *> RefGroupVecTy;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

// Threshold for min allowed number of cachelines
static cl::opt<uint64_t>
    NumCachelinesThreshold("hir-prefetching-num-cachelines-threshold",
                           cl::init(8192), cl::Hidden,
                           cl::desc("Threshold for number of cachelines"));

// Threshold for min allowed number of memory streams
static cl::opt<unsigned> NumMemoryStreamsThreshold(
    "hir-prefetching-num-memory-streams-threshold", cl::init(18), cl::Hidden,
    cl::desc("Threshold for number of memory streams"));

// Threshold for min allowed number of trip count
static cl::opt<uint64_t>
    TripCountThreshold("hir-prefetching-trip-count-threshold", cl::init(100000),
                       cl::Hidden, cl::desc("Threshold for trip count"));

static cl::opt<unsigned> IterationDistance(
    "hir-prefetching-iteration-distance", cl::init(6), cl::Hidden,
    cl::desc("Iteration distance for prefetching distance computation"));

static cl::opt<bool>
    SkipNonModifiedRegions("hir-prefetching-skip-non-modified-regions",
                           cl::init(true), cl::Hidden,
                           cl::desc("Skip non-modified regions"));

static cl::opt<bool>
    SkipNumMemoryStreamsCheck("hir-prefetching-skip-num-memory-streams-check",
                              cl::init(false), cl::Hidden,
                              cl::desc("Skip number of memory streams check"));
static cl::opt<bool>
    SkipAVX2Check("hir-prefetching-skip-AVX2-check", cl::init(false),
                  cl::Hidden, cl::desc("Skip AVX2 and above processor check"));

namespace {

class HIRPrefetching {
  HIRFramework &HIRF;
  HIRLoopLocality &LA;
  const TargetTransformInfo &TTI;

public:
  HIRPrefetching(HIRFramework &HIRF, HIRLoopLocality &LA,
                 const TargetTransformInfo &TTI)
      : HIRF(HIRF), LA(LA), TTI(TTI) {}

  bool run();

private:
  bool doPrefetching(HLLoop *Lp,
                     SmallVectorImpl<const RegDDRef *> &PrefetchCandidates);

  bool doAnalysis(HLLoop *Lp,
                  SmallVectorImpl<const RegDDRef *> &PrefetchCandidates);

  void collectPrefetchCandidates(
      RefGroupTy &RefGroup, uint64_t TripCount, uint64_t Stride,
      SmallVectorImpl<const RegDDRef *> &PrefetchCandidates);
};
} // namespace

// Collect the prefetching  candidates by computing the number of Streams in the
// MemRefs
void HIRPrefetching::collectPrefetchCandidates(
    RefGroupTy &RefGroup, uint64_t TripCount, uint64_t Stride,
    SmallVectorImpl<const RegDDRef *> &PrefetchCandidates) {
  const RegDDRef *FirstRef = RefGroup.front();

  PrefetchCandidates.push_back(FirstRef);

  const RegDDRef *PrevRef = FirstRef;
  const RegDDRef *CurRef = nullptr;

  for (auto RefIt = RefGroup.begin() + 1, E = RefGroup.end(); RefIt != E;
       ++RefIt) {
    CurRef = *RefIt;

    int64_t Dist;
    DDRefUtils::getConstByteDistance(CurRef, PrevRef, &Dist);

    assert((Dist >= 0) && "Refs do not have constant non-negative distance!");

    if (Dist / Stride >= TripCount) {
      PrefetchCandidates.push_back(CurRef);
      PrevRef = CurRef;
    }
  }
  return;
}

bool HIRPrefetching::doAnalysis(
    HLLoop *Lp, SmallVectorImpl<const RegDDRef *> &PrefetchCandidates) {
  if (SkipNonModifiedRegions && !Lp->getParentRegion()->shouldGenCode()) {
    return false;
  }

  uint64_t TripCount = 0;

  if (!Lp->isConstTripLoop(&TripCount)) {
    return false;
  }

  if (TripCount < TripCountThreshold) {
    return false;
  }

  HIRLoopLocality::RefGroupVecTy SpatialGroups;

  uint64_t NumCachelines = LA.getNumCacheLines(Lp, &SpatialGroups);

  if (NumCachelines < NumCachelinesThreshold) {
    return false;
  }

  unsigned Level = Lp->getNestingLevel();
  int64_t ConstStride;
  uint64_t Stride;

  for (auto &RefGroup : SpatialGroups) {
    const RegDDRef *FirstRef = RefGroup.front();

    if (!FirstRef->getConstStrideAtLevel(Level, &ConstStride) ||
        ConstStride == 0) {
      continue;
    }

    Stride = std::abs(ConstStride);

    collectPrefetchCandidates(RefGroup, TripCount, Stride, PrefetchCandidates);
  }

  if (PrefetchCandidates.size() < NumMemoryStreamsThreshold &&
      !SkipNumMemoryStreamsCheck) {
    return false;
  }

  return true;
}

bool HIRPrefetching::doPrefetching(
    HLLoop *Lp, SmallVectorImpl<const RegDDRef *> &PrefetchCandidates) {
  auto &HNU = Lp->getHLNodeUtils();
  auto &DDRU = HNU.getDDRefUtils();

  auto Int32Ty = Type::getInt32Ty(HNU.getContext());

  RegDDRef *StrideRef = Lp->getStrideDDRef();
  int64_t Stride;

  StrideRef->isIntConstant(&Stride);

  unsigned Distance = IterationDistance * Stride;
  unsigned Level = Lp->getNestingLevel();

  for (auto RefIt = PrefetchCandidates.begin(), E = PrefetchCandidates.end();
       RefIt != E; ++RefIt) {
    const RegDDRef *Ref = *RefIt;
    RegDDRef *PrefetchRef = Ref->clone();

    PrefetchRef->setAddressOf(true);

    // Set destination address (i8*)
    PrefetchRef->setBitCastDestType(Type::getInt8PtrTy(
        HIRF.getContext(), PrefetchRef->getPointerAddressSpace()));

    PrefetchRef->shift(Level, Distance);

    RegDDRef *ReadTy = DDRU.createConstDDRef(Int32Ty, 0);
    RegDDRef *Locality = DDRU.createConstDDRef(Int32Ty, 3);
    RegDDRef *DataCacheTy = DDRU.createConstDDRef(Int32Ty, 1);
    HLInst *PrefetchInst =
        HNU.createPrefetch(PrefetchRef, ReadTy, Locality, DataCacheTy);
    HLNodeUtils::insertAsLastChild(Lp, PrefetchInst);
  }

  HIRInvalidationUtils::invalidateBody(Lp);

  return true;
}

bool HIRPrefetching::run() {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << "HIR Prefetching Disabled \n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIR Prefetching on Function : "
                    << HIRF.getFunction().getName() << "\n");

  if (!SkipAVX2Check &&
      !TTI.isAdvancedOptEnabled(
          TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2)) {
    return false;
  }

  // Gather all inner-most loops as Candidates
  SmallVector<HLLoop *, 64> CandidateLoops;

  HLNodeUtils &HNU = HIRF.getHLNodeUtils();
  HNU.gatherInnermostLoops(CandidateLoops);

  if (CandidateLoops.empty()) {
    LLVM_DEBUG(dbgs() << HIRF.getFunction().getName()
                      << "() has no inner-most loop\n ");
    return false;
  }

  bool Result = false;

  for (auto &Lp : CandidateLoops) {

    // Analyze the loop and check if it is suitable for prefetching
    SmallVector<const RegDDRef *, 64> PrefetchCandidates;

    if (!doAnalysis(Lp, PrefetchCandidates)) {
      continue;
    }

    Result = doPrefetching(Lp, PrefetchCandidates) || Result;
  }

  return Result;
}

PreservedAnalyses HIRPrefetchingPass::run(llvm::Function &F,
                                          llvm::FunctionAnalysisManager &AM) {
  HIRPrefetching(AM.getResult<HIRFrameworkAnalysis>(F),
                 AM.getResult<HIRLoopLocalityAnalysis>(F),
                 AM.getResult<TargetIRAnalysis>(F))
      .run();
  return PreservedAnalyses::all();
}

class HIRPrefetchingLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRPrefetchingLegacyPass() : HIRTransformPass(ID) {
    initializeHIRPrefetchingLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRLoopLocalityWrapperPass>();
    AU.addRequiredTransitive<TargetTransformInfoWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRPrefetching(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
               getAnalysis<HIRLoopLocalityWrapperPass>().getHLL(),
               getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F))
        .run();
  }
};

char HIRPrefetchingLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRPrefetchingLegacyPass, OPT_SWITCH, OPT_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopLocalityWrapperPass)
INITIALIZE_PASS_END(HIRPrefetchingLegacyPass, OPT_SWITCH, OPT_DESC, false,
                    false)

FunctionPass *llvm::createHIRPrefetchingPass() {
  return new HIRPrefetchingLegacyPass();
}
