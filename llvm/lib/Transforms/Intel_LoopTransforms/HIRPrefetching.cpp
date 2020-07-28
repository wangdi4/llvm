//===----------- HIRPrefetching.cpp Implements Prefetching class ---------===//
//
// Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
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
#include "llvm/Transforms/Intel_LoopTransforms/HIRPrefetchingPass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
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
    "hir-prefetching-num-memory-streams-threshold", cl::init(15), cl::Hidden,
    cl::desc("Threshold for number of memory streams"));

// Threshold for min allowed number of trip count
static cl::opt<uint64_t>
    TripCountThreshold("hir-prefetching-trip-count-threshold", cl::init(10000),
                       cl::Hidden, cl::desc("Threshold for trip count"));

static cl::opt<unsigned> ForceIterationDistance(
    "hir-prefetching-iteration-distance", cl::init(6), cl::Hidden,
    cl::desc("Iteration distance for prefetching distance computation"));

static cl::opt<unsigned>
    AssumedMemPrefetchLatency("hir-prefetching-assumed-mem-prefetch-latency",
                              cl::init(840), cl::Hidden,
                              cl::desc("Assumed Memory Prefetch Latency"));

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
  HIRLoopResource &HLR;
  const TargetTransformInfo &TTI;

public:
  HIRPrefetching(HIRFramework &HIRF, HIRLoopLocality &LA, HIRLoopResource &HLR,
                 const TargetTransformInfo &TTI)
      : HIRF(HIRF), LA(LA), HLR(HLR), TTI(TTI) {}

  bool run();

private:
  bool doPrefetching(HLLoop *Lp,
                     SmallVectorImpl<const RegDDRef *> &PrefetchCandidates);

  bool doAnalysis(HLLoop *Lp,
                  SmallVectorImpl<const RegDDRef *> &PrefetchCandidates);

  void collectPrefetchCandidates(
      RefGroupTy &RefGroup, uint64_t TripCount, uint64_t Stride, unsigned Level,
      SmallVectorImpl<const RegDDRef *> &PrefetchCandidates);
};
} // namespace

static const RegDDRef *getScalarRef(const RegDDRef *FirstRef,
                                    unsigned &VecNumElements) {
  bool HasVectorIndex = false;

  for (auto *IndexCE :
       make_range(FirstRef->canon_begin(), FirstRef->canon_end())) {
    if (IndexCE->getSrcType()->isVectorTy()) {
      HasVectorIndex = true;
      break;
    }
  }

  if (!HasVectorIndex) {
    return FirstRef;
  }

  RegDDRef *RefClone = FirstRef->clone();

  for (auto *IndexCE :
       make_range(RefClone->canon_begin(), RefClone->canon_end())) {
    BlobUtils &BU = IndexCE->getBlobUtils();

    SmallVector<unsigned, 8> BlobIdxToRemove;
    for (auto Blob : make_range(IndexCE->blob_begin(), IndexCE->blob_end())) {
      Constant *VecConst;

      if (BU.getBlob(Blob.Index)->getType()->isVectorTy()) {
        bool IsConstVec =
            BU.isConstantVectorBlob(BU.getBlob(Blob.Index), &VecConst);
        (void)IsConstVec;
        assert(IsConstVec && "The blob should be a constant vector blob");

        // Extract the first element of the vector blob and substituting it in
        // the CanonExpr
        ConstantDataVector *CV = cast<ConstantDataVector>(VecConst);
        int64_t SExtValue = CV->getElementAsAPInt(0).getSExtValue();
        VecNumElements = CV->getNumElements();
        IndexCE->addConstant(Blob.Coeff * SExtValue, false);
        BlobIdxToRemove.push_back(Blob.Index);
      }
    }

    for (unsigned BI : BlobIdxToRemove) {
      IndexCE->removeBlob(BI);
    }

    IndexCE->setSrcAndDestType(IndexCE->getSrcType()->getScalarType());
  }

  return RefClone;
}

// Collect the prefetching  candidates by computing the number of Streams in the
// MemRefs
void HIRPrefetching::collectPrefetchCandidates(
    RefGroupTy &RefGroup, uint64_t TripCount, uint64_t Stride, unsigned Level,
    SmallVectorImpl<const RegDDRef *> &PrefetchCandidates) {
  const RegDDRef *FirstRef = RefGroup.front();
  unsigned VecNumElements = 0;

  const RegDDRef *ScalarRef = getScalarRef(FirstRef, VecNumElements);
  // Nontemporal refs should not be candidates for prefetches.
  if (ScalarRef->getMetadata(LLVMContext::MD_nontemporal))
    return;

  PrefetchCandidates.push_back(ScalarRef);

  unsigned ScalarRefSize = ScalarRef->getDestTypeSizeInBytes();

  // When the stride exceeds trip count, we need to create multiple scalar refs
  // for vector refs as they belong to different memory streams
  if (VecNumElements > 0 && Stride / ScalarRefSize >= TripCount) {
    for (unsigned I = 1; I < VecNumElements; ++I) {
      RegDDRef *StrideRef = ScalarRef->clone();
      StrideRef->shift(Level, I);
      PrefetchCandidates.push_back(StrideRef);
    }
  }

  // TODO: Compare the stride with the other vector refs in the ref group
  if (VecNumElements > 0) {
    return;
  }

  // Rest of the code is precessing scalar refs case
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

  if (!Lp->isDo()) {
    return false;
  }

  uint64_t TripCount = 0;

  bool IsConstTC = Lp->isConstTripLoop(&TripCount);

  if (!IsConstTC) {
    TripCount = Lp->getMaxTripCountEstimate();
    if (TripCount == 0) {
      TripCount = TripCountThreshold;
    }
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
  unsigned NumNonLinearStreams = 0;

  for (auto &RefGroup : SpatialGroups) {
    const RegDDRef *FirstRef = RefGroup.front();

    if (!FirstRef->getConstStrideAtLevel(Level, &ConstStride) ||
        ConstStride == 0) {

      if (!FirstRef->isLinearAtLevel(Level)) {
        NumNonLinearStreams++;
      }

      continue;
    }

    Stride = std::abs(ConstStride);

    collectPrefetchCandidates(RefGroup, TripCount, Stride, Level,
                              PrefetchCandidates);
  }

  if (PrefetchCandidates.empty()) {
    return false;
  }

  if ((PrefetchCandidates.size() + NumNonLinearStreams) <
          NumMemoryStreamsThreshold &&
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

  unsigned IterationDistance = 0;

  if (ForceIterationDistance.getNumOccurrences() > 0) {
    IterationDistance = ForceIterationDistance;
  } else {
    // Dynamically compute the IterationDistance by considering the total cost
    // in loop resource as the loop latency. The IterationDistance and the cost
    // have an inverse ratio.
    unsigned Cost = HLR.getTotalLoopResource(Lp).getTotalCost();
    IterationDistance = AssumedMemPrefetchLatency / Cost;

    if (IterationDistance == 0) {
      IterationDistance = ForceIterationDistance;
    }
  }

  unsigned Distance = IterationDistance * Stride;

  unsigned Level = Lp->getNestingLevel();
  unsigned NumSpatialPrefetches = PrefetchCandidates.size();

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

  LoopOptReportBuilder &LORBuilder = HNU.getHIRFramework().getLORBuilder();

  LORBuilder(*Lp).addRemark(OptReportVerbosity::Low,
                            "Number of spatial prefetches=%d, dist=%d",
                            NumSpatialPrefetches, IterationDistance);

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
                 AM.getResult<HIRLoopResourceAnalysis>(F),
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
    AU.addRequiredTransitive<HIRLoopResourceWrapperPass>();
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
               getAnalysis<HIRLoopResourceWrapperPass>().getHLR(),
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
INITIALIZE_PASS_DEPENDENCY(HIRLoopResourceWrapperPass)
INITIALIZE_PASS_END(HIRPrefetchingLegacyPass, OPT_SWITCH, OPT_DESC, false,
                    false)

FunctionPass *llvm::createHIRPrefetchingPass() {
  return new HIRPrefetchingLegacyPass();
}
