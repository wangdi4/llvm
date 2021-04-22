//===----------- HIRPrefetching.cpp Implements Prefetching class ---------===//
//
// Copyright (C) 2019-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass implements software prefetching, when the number of memrefs exceeds
// the threshold in the loop. It prefetches the memref by the intrinsic calls at
// the end of the loop.
//
// Prefetching pragma directives can be supported in this pass.
// Pragma info for prefetching contains information for variable, hint and
// distance.
//
// Example 1:
// #pragma noprefetch
// Do I = 1..N
//    A[I] = B[I-1];
// ENDDO
//
// Example 1 issues no prefetches for all variables.
//
// Example 2:
// #pragama noprefetch C
// #pragma prefetch A:1:16
// #pragma prefetch B
// Do I = 1..N
//    A[I] = B[I] + C[I];
// ENDDO
//
// Example 2 issues no prefetch for C, issues prefetching from L1
// cache for A with a distance of 16 iterations ahead, and issues prefetching
// B. The cost model determines which cache and distance are used.
//
// Example 3:
// #pragama prefetch *:1:40
// Do I = 1..N
//    A[I] = B[I] + C[I];
// ENDDO
//
// Example 3 issues prefetching all variables with hint 1 and distance 40.
//===----------------------------------------------------------------------===//
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRPrefetchingPass.h"
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
                           cl::init(4096), cl::Hidden,
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

// Pragma prefetch hint specifies the type of prefetch. Possible values:
// 1: For integer data that will be reused
// 2: For integer and floating point data that will be reused from L2 cache
// 3: For data that will be reused from L3 cache
// 4: For data that will not be reused
// However, prefetch intrinsic's locality is a temporal locality specifier
// ranging from (0) - no locality, to (3) - extremely local keep in cache. Thus,
// we need to transfer pragma prefetch hint to prefetch intrinsic's locality
// using (4 - PrefetchHint)
static cl::opt<unsigned>
    ForceHint("hir-prefetching-hint", cl::init(1), cl::Hidden,
              cl::desc("Prefetching hint to specify the type of prefetch"));

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

struct PragmaInfo {
  unsigned BasePtrSB;
  int Hint;
  int Dist;

  PragmaInfo(unsigned BasePtrSB, int Hint, int Dist)
      : BasePtrSB(BasePtrSB), Hint(Hint), Dist(Dist) {}
};

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
  bool doPrefetching(
      HLLoop *Lp,
      SmallVectorImpl<PrefetchingPragmaInfo> &PrefetchCandidateVarsDistsHints,
      bool HasPragmaInfo);

  void generatePrefetchingInst(HLLoop *Lp, RegDDRef *PrefetchRef,
                               unsigned PrefetchHint);

  bool doAnalysis(
      HLLoop *Lp,
      SmallVectorImpl<PrefetchingPragmaInfo> &PrefetchCandidateVarsDistsHints,
      bool &HasPragmaInfo);

  unsigned getPrefetchingDist(HLLoop *Lp);

  void collectPrefetchCandidates(
      RefGroupTy &RefGroup, uint64_t TripCount, uint64_t Stride, unsigned Level,
      int Distance, int Hint,
      SmallVectorImpl<PrefetchingPragmaInfo> &PrefetchCandidateVarsDistsHints);

  void collectPrefetchPragmaInfo(
      HLLoop *Lp,
      DenseMap<unsigned, std::pair<int, int>> &CandidateVarSBsDistsHints,
      int &PrefetchDist, int &PrefetchHint);
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

unsigned HIRPrefetching::getPrefetchingDist(HLLoop *Lp) {
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

  RegDDRef *StrideRef = Lp->getStrideDDRef();
  int64_t LpStride;
  StrideRef->isIntConstant(&LpStride);

  return IterationDistance * LpStride;
}

// Collect the prefetching  candidates by computing the number of Streams in the
// MemRefs
void HIRPrefetching::collectPrefetchCandidates(
    RefGroupTy &RefGroup, uint64_t TripCount, uint64_t Stride, unsigned Level,
    int Distance, int Hint,
    SmallVectorImpl<PrefetchingPragmaInfo> &PrefetchCandidateVarsDistsHints) {
  if (!Distance) {
    return;
  }

  const RegDDRef *FirstRef = RefGroup.front();
  unsigned VecNumElements = 0;

  const RegDDRef *ScalarRef = getScalarRef(FirstRef, VecNumElements);
  // Nontemporal refs should not be candidates for prefetches.
  if (ScalarRef->getMetadata(LLVMContext::MD_nontemporal))
    return;
  PrefetchCandidateVarsDistsHints.emplace_back(ScalarRef, Hint, Distance);

  unsigned ScalarRefSize = ScalarRef->getDestTypeSizeInBytes();

  // When the stride exceeds trip count, we need to create multiple scalar refs
  // for vector refs as they belong to different memory streams
  if (VecNumElements > 0 && Stride / ScalarRefSize >= TripCount) {
    for (unsigned I = 1; I < VecNumElements; ++I) {
      RegDDRef *StrideRef = ScalarRef->clone();
      StrideRef->shift(Level, I);
      PrefetchCandidateVarsDistsHints.emplace_back(StrideRef, Hint, Distance);
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
      PrefetchCandidateVarsDistsHints.emplace_back(CurRef, Hint, Distance);
      PrevRef = CurRef;
    }
  }
  return;
}

static void collectLoadLvalSB(
    HLInst *HInst, int PrefetchDist, int PrefetchHint,
    SmallVectorImpl<PragmaInfo> &PragmaVarsDistsHints,
    DenseMap<unsigned, std::pair<int, int>> &CandidateVarSBsDistsHints) {
  assert(HInst && "Expect HLInst* only\n");

  if (!isa<LoadInst>(HInst->getLLVMInstruction())) {
    return;
  }

  unsigned RvalBasePtrSB = HInst->getRvalDDRef()->getBasePtrSymbase();

  for (auto It = PragmaVarsDistsHints.begin(), End = PragmaVarsDistsHints.end();
       It != End; ++It) {
    // BlobIndex are equal
    if (It->BasePtrSB == RvalBasePtrSB) {
      CandidateVarSBsDistsHints[HInst->getLvalDDRef()->getSymbase()] =
          std::make_pair(It->Dist, It->Hint);
      return;
    }
  }

  return;
}

// Collect Vars' SBs and corresponding prefetch distance and prefetch hint
// into a lookup map. Vars can also be the lval of the load insts with pragma
// vars in the preheader or before the loop. \p DefaultPrefetchDist and \p
// DefaultPrefetchHint can be used to record the dist and hint when prefetching
// all vars
void HIRPrefetching::collectPrefetchPragmaInfo(
    HLLoop *Lp,
    DenseMap<unsigned, std::pair<int, int>> &CandidateVarSBsDistsHints,
    int &DefaultPrefetchDist, int &DefaultPrefetchHint) {
  auto Info = Lp->getPrefetchingPragmaInfo();
  unsigned Size = Info.size();
  SmallVector<PragmaInfo, 16> PragmaVarsDistsHints;
  RegDDRef *StrideRef = Lp->getStrideDDRef();
  int64_t LpStride;
  StrideRef->isIntConstant(&LpStride);

  for (unsigned I = 0; I < Size; ++I) {
    const RegDDRef *Var = Info[I].Var;
    int Dist = Info[I].Dist;
    int Hint = Info[I].Hint;

    if (Dist == -1) {
      Dist = DefaultPrefetchDist;
    } else {
      Dist *= LpStride;
    }

    // Prefetch pragma hint is ranging from 1 - 4, while
    // Prefetch intrinsic's locality is ranging from (0) - no
    // locality, to (3) - extremely local keep in cache. Thus, we need to
    // transfer prefetch pragma hint to prefetch intrinsic's locality using
    // 4 - Hint
    if (Hint == -1) {
      Hint = DefaultPrefetchHint;
    } else {
      Hint = 4 - Hint;
    }

    if (Var->isNull()) {
      DefaultPrefetchDist = Dist;
      DefaultPrefetchHint = Hint;
      continue;
    }

    PragmaVarsDistsHints.emplace_back(Var->getBasePtrSymbase(), Hint, Dist);
    CandidateVarSBsDistsHints[Var->getBasePtrSymbase()] =
        std::make_pair(Dist, Hint);
  }

  if (PragmaVarsDistsHints.empty()) {
    return;
  }

  // Go through the load insts in the preheader of the loop and collect the
  // corresponding lval with the rval in the pragma vars
  for (auto II = Lp->pre_begin(), EE = Lp->pre_end(); II != EE; ++II) {
    HLInst *HInst = cast<HLInst>(&*II);
    collectLoadLvalSB(HInst, DefaultPrefetchDist, DefaultPrefetchHint,
                      PragmaVarsDistsHints, CandidateVarSBsDistsHints);
  }

  // Go through the insts before the current loop and collect the
  // corresponding lval with the rval in the pragma vars until it hits the prev
  // loop
  HLNode *Node = Lp;
  while ((Node = Node->getPrevNode())) {
    if (isa<HLLoop>(Node)) {
      break;
    } else if (auto *Inst = dyn_cast<HLInst>(Node)) {
      collectLoadLvalSB(Inst, DefaultPrefetchDist, DefaultPrefetchHint,
                        PragmaVarsDistsHints, CandidateVarSBsDistsHints);
    } else {
      break;
    }
  }

  return;
}

static bool hasPrefetchingPragma(HLLoop *Lp) {
  auto Info = Lp->getPrefetchingPragmaInfo();
  return !Info.empty();
}

bool HIRPrefetching::doAnalysis(
    HLLoop *Lp,
    SmallVectorImpl<PrefetchingPragmaInfo> &PrefetchCandidateVarsDistsHints,
    bool &HasPragmaInfo) {
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

  HasPragmaInfo = hasPrefetchingPragma(Lp);

  if (!HasPragmaInfo) {
    if (!SkipAVX2Check &&
        !TTI.isAdvancedOptEnabled(
            TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2)) {
      return false;
    }

    if (SkipNonModifiedRegions && !Lp->getParentRegion()->shouldGenCode()) {
      return false;
    }

    if (TripCount < TripCountThreshold) {
      return false;
    }
  }

  HIRLoopLocality::RefGroupVecTy SpatialGroups;

  uint64_t NumCachelines = LA.getNumCacheLines(Lp, &SpatialGroups);

  if (!HasPragmaInfo && NumCachelines < NumCachelinesThreshold) {
    return false;
  }

  DenseMap<unsigned, std::pair<int, int>> CandidateVarSBsDistsHints;
  int DefaultPrefetchDist = getPrefetchingDist(Lp);
  int DefaultPrefetchHint = 4 - ForceHint;

  collectPrefetchPragmaInfo(Lp, CandidateVarSBsDistsHints, DefaultPrefetchDist,
                            DefaultPrefetchHint);

  unsigned Level = Lp->getNestingLevel();
  int64_t ConstStride;
  uint64_t Stride;
  unsigned NumNonLinearStreams = 0;

  for (auto &RefGroup : SpatialGroups) {
    const RegDDRef *FirstRef = RefGroup.front();
    unsigned FirstRefBasePtrSB = FirstRef->getBasePtrSymbase();

    if (!FirstRef->getConstStrideAtLevel(Level, &ConstStride) ||
        ConstStride == 0) {

      if (!FirstRef->isLinearAtLevel(Level)) {
        NumNonLinearStreams++;
      }

      continue;
    }

    Stride = std::abs(ConstStride);
    int Dist = DefaultPrefetchDist;
    int Hint = DefaultPrefetchHint;

    // Prefetch candidate is either the lval in the load inst in the preheader
    // or before loop or the pragma var in the loop
    if (CandidateVarSBsDistsHints.count(FirstRefBasePtrSB)) {
      std::tie(Dist, Hint) = CandidateVarSBsDistsHints[FirstRefBasePtrSB];
    }

    // When Stride is a non-zero constant, we will go through the RefGroup and
    // check whether they are in the same memory streams
    // When two refs in the same RefGroup are located in the different memory
    // streams because the distance between them is larger than the product of
    // loop stride and loop trip count, we need to create more scalar refs,
    // such as A[i] and A[i + 10000]
    collectPrefetchCandidates(RefGroup, TripCount, Stride, Level, Dist, Hint,
                              PrefetchCandidateVarsDistsHints);
  }

  if (PrefetchCandidateVarsDistsHints.empty()) {
    return false;
  }

  if (!HasPragmaInfo &&
      (PrefetchCandidateVarsDistsHints.size() + NumNonLinearStreams) <
          NumMemoryStreamsThreshold &&
      !SkipNumMemoryStreamsCheck) {
    return false;
  }

  return true;
}

void HIRPrefetching::generatePrefetchingInst(HLLoop *Lp, RegDDRef *PrefetchRef,
                                             unsigned PrefetchHint) {
  auto &HNU = Lp->getHLNodeUtils();
  auto &DDRU = HNU.getDDRefUtils();

  auto Int32Ty = Type::getInt32Ty(HNU.getContext());

  RegDDRef *ReadTy = DDRU.createConstDDRef(Int32Ty, 0);
  RegDDRef *Locality = DDRU.createConstDDRef(Int32Ty, PrefetchHint);
  RegDDRef *DataCacheTy = DDRU.createConstDDRef(Int32Ty, 1);
  HLInst *PrefetchInst =
      HNU.createPrefetch(PrefetchRef, ReadTy, Locality, DataCacheTy);
  HLNodeUtils::insertAsLastChild(Lp, PrefetchInst);
  return;
}

bool HIRPrefetching::doPrefetching(
    HLLoop *Lp,
    SmallVectorImpl<PrefetchingPragmaInfo> &PrefetchCandidateVarsDistsHints,
    bool HasPragmaInfo) {
  unsigned NumSpatialPrefetches = 0;
  int PrefetchDist = 0;

  for (auto It = PrefetchCandidateVarsDistsHints.begin(),
            E = PrefetchCandidateVarsDistsHints.end();
       It != E; ++It) {
    const RegDDRef *Ref = It->Var;
    PrefetchDist = It->Dist;
    int PrefetchHint = It->Hint;

    RegDDRef *PrefetchRef = Ref->clone();

    PrefetchRef->setAddressOf(true);

    // Set destination address (i8*)
    PrefetchRef->setBitCastDestType(Type::getInt8PtrTy(
        HIRF.getContext(), PrefetchRef->getPointerAddressSpace()));

    unsigned Level = Lp->getNestingLevel();
    PrefetchRef->shift(Level, PrefetchDist);

    generatePrefetchingInst(Lp, PrefetchRef, PrefetchHint);
    ++NumSpatialPrefetches;
  }

  LoopOptReportBuilder &LORBuilder =
      Lp->getHLNodeUtils().getHIRFramework().getLORBuilder();

  LORBuilder(*Lp).addRemark(OptReportVerbosity::Low,
                            "Number of spatial prefetches=%d",
                            NumSpatialPrefetches);
  if (HasPragmaInfo) {
    Lp->getParentRegion()->setGenCode();
    LORBuilder(*Lp).addRemark(OptReportVerbosity::Low,
                              "Number of spatial prefetches=%d",
                              NumSpatialPrefetches);
  } else {
    RegDDRef *StrideRef = Lp->getStrideDDRef();
    int64_t Stride;
    StrideRef->isIntConstant(&Stride);
    int Distance = PrefetchDist / Stride;

    LORBuilder(*Lp).addRemark(OptReportVerbosity::Low,
                              "Number of spatial prefetches=%d, dist=%d",
                              NumSpatialPrefetches, Distance);
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

  // Gather all inner-most loops as Candidates
  SmallVector<HLLoop *, 64> CandidateLoops;

  HLNodeUtils &HNU = HIRF.getHLNodeUtils();

  // TODO:Process prefetch directives for outer loops
  HNU.gatherInnermostLoops(CandidateLoops);

  if (CandidateLoops.empty()) {
    LLVM_DEBUG(dbgs() << HIRF.getFunction().getName()
                      << "() has no inner-most loop\n ");
    return false;
  }

  bool Result = false;

  for (auto &Lp : CandidateLoops) {
    SmallVector<PrefetchingPragmaInfo, 64> PrefetchCandidateVarsDistsHints;
    bool HasPragmaInfo = false;

    // Analyze the loop and check if it is suitable for prefetching
    if (!doAnalysis(Lp, PrefetchCandidateVarsDistsHints, HasPragmaInfo)) {
      continue;
    }

    Result =
        doPrefetching(Lp, PrefetchCandidateVarsDistsHints, HasPragmaInfo) ||
        Result;
  }

  return Result;
}

PreservedAnalyses HIRPrefetchingPass::runImpl(llvm::Function &F,
                                              llvm::FunctionAnalysisManager &AM,
                                              HIRFramework &HIRF) {
  HIRPrefetching(HIRF, AM.getResult<HIRLoopLocalityAnalysis>(F),
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
