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
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
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
// 0: For integer data that will be reused
// 1: For integer and floating point data that will be reused from L2 cache
// 2: For data that will be reused from L3 cache
// 3: For data that will not be reused
// However, prefetch intrinsic's locality is a temporal locality specifier
// ranging from (0) - no locality, to (3) - extremely local keep in cache. Thus,
// we need to transfer pragma prefetch hint to prefetch intrinsic's locality
// using (3 - PrefetchHint)
static cl::opt<unsigned>
    ForceHint("hir-prefetching-hint", cl::init(0), cl::Hidden,
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

// Set RW type of prefetch intrinsic is 1(write) for the store/write memory
// stream when prefetchw is enabled
static cl::opt<bool> EnablePrefetchW("hir-prefetching-prefetchw",
                                     cl::init(false), cl::Hidden,
                                     cl::desc("Enable prefetchW"));

static cl::opt<bool>
    EnableIndirectPrefetching("hir-prefetching-enable-indirect-prefetching",
                              cl::init(false), cl::Hidden,
                              cl::desc("Enable indirect prefetching"));

namespace {

struct PragmaInfo {
  unsigned BasePtrSB;
  int Hint;
  int Dist;
  bool HasSpecifiedHintOrDist;

  PragmaInfo(unsigned BasePtrSB, int Hint, int Dist,
             bool HasSpecifiedHintOrDist)
      : BasePtrSB(BasePtrSB), Hint(Hint), Dist(Dist),
        HasSpecifiedHintOrDist(HasSpecifiedHintOrDist) {}
};

struct PrefetchCandidateInfo {
  const RegDDRef *CandidateRef;
  RegDDRef *OrigIndexLoadRef; // This is only applicable for indirect refs
  int Dist;
  int Hint;
  bool IsWrite;
  bool HasSpecifiedHintOrDist;

  PrefetchCandidateInfo(const RegDDRef *CandidateRef,
                        RegDDRef *OrigIndexLoadRef, int Dist, int Hint,
                        bool IsWrite, bool HasSpecifiedHintOrDist)
      : CandidateRef(CandidateRef), OrigIndexLoadRef(OrigIndexLoadRef),
        Dist(Dist), Hint(Hint), IsWrite(IsWrite),
        HasSpecifiedHintOrDist(HasSpecifiedHintOrDist) {}
};

class HIRPrefetching {
  HIRFramework &HIRF;
  HIRLoopLocality &LA;
  HIRDDAnalysis &DDA;
  HIRLoopResource &HLR;
  const TargetTransformInfo &TTI;

public:
  HIRPrefetching(HIRFramework &HIRF, HIRLoopLocality &LA, HIRDDAnalysis &DDA,
                 HIRLoopResource &HLR, const TargetTransformInfo &TTI)
      : HIRF(HIRF), LA(LA), DDA(DDA), HLR(HLR), TTI(TTI) {}

  bool run();

private:
  bool doPrefetching(
      HLLoop *Lp, bool HasPragmaInfo, int DefaultPrefetchDist,
      const SmallVectorImpl<PrefetchCandidateInfo> &SpatialPrefetchCandidates,
      const SmallVectorImpl<PrefetchCandidateInfo> &IndirectPrefetchCandidates);

  HLInst *generatePrefetchingInst(HLLoop *Lp, RegDDRef *PrefetchRef,
                                  unsigned PrefetchHint, bool IsWrite);

  bool doAnalysis(
      HLLoop *Lp, bool &HasPragmaInfo, int &DefaultPrefetchDist,
      SmallVectorImpl<PrefetchCandidateInfo> &SpatialPrefetchCandidates,
      SmallVectorImpl<PrefetchCandidateInfo> &IndirectPrefetchCandidates);

  unsigned getPrefetchingDist(HLLoop *Lp);

  void collectPrefetchCandidates(
      RefGroupTy &RefGroup, uint64_t TripCount, uint64_t Stride, unsigned Level,
      int Distance, int Hint, bool HasSpecifiedHintOrDist,
      SmallVectorImpl<PrefetchCandidateInfo> &SpatialPrefetchCandidates);

  void collectPrefetchPragmaInfo(
      HLLoop *Lp,
      DenseMap<unsigned, std::tuple<int, int, bool>> &CandidateVarSBsDistsHints,
      int &PrefetchDist, int &PrefetchHint, bool &DefaultHasSpecifiedHintOrDist,
      bool &HasPrefetchAll);

  void collectIndirectPrefetchingCandidates(
      HLLoop *Lp, const RegDDRef *FirstRef, int Dist, int Hint,
      bool HasSpecifiedHintOrDist,
      SmallVectorImpl<PrefetchCandidateInfo> &IndirectPrefetchCandidates);

  void processIndirectPrefetching(
      HLLoop *Lp, int64_t Stride,
      const SmallVectorImpl<PrefetchCandidateInfo> &IndirectPrefetchCandidates);
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

// Collect the prefetching candidates by computing the number of Streams in the
// MemRefs
void HIRPrefetching::collectPrefetchCandidates(
    RefGroupTy &RefGroup, uint64_t TripCount, uint64_t Stride, unsigned Level,
    int Distance, int Hint, bool HasSpecifiedHintOrDist,
    SmallVectorImpl<PrefetchCandidateInfo> &SpatialPrefetchCandidates) {
  if (!Distance) {
    return;
  }

  const RegDDRef *FirstRef = RefGroup.front();
  unsigned VecNumElements = 0;

  const RegDDRef *ScalarRef = getScalarRef(FirstRef, VecNumElements);

  // Nontemporal refs should not be candidates for prefetches.
  if (ScalarRef->getMetadata(LLVMContext::MD_nontemporal))
    return;

  bool IsLval = FirstRef->isLval();
  SpatialPrefetchCandidates.emplace_back(ScalarRef, nullptr, Distance, Hint,
                                         IsLval, HasSpecifiedHintOrDist);

  unsigned ScalarRefSize = ScalarRef->getDestTypeSizeInBytes();

  // When the stride exceeds trip count, we need to create multiple scalar refs
  // for vector refs as they belong to different memory streams
  if (VecNumElements > 0 && Stride / ScalarRefSize >= TripCount) {
    for (unsigned I = 1; I < VecNumElements; ++I) {
      RegDDRef *StrideRef = ScalarRef->clone();
      StrideRef->shift(Level, I);
      SpatialPrefetchCandidates.emplace_back(StrideRef, nullptr, Distance, Hint,
                                             IsLval, HasSpecifiedHintOrDist);
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
      IsLval = CurRef->isLval();
      SpatialPrefetchCandidates.emplace_back(CurRef, nullptr, Distance, Hint,
                                             IsLval, HasSpecifiedHintOrDist);
      PrevRef = CurRef;
    } else if (CurRef->isLval() && !IsLval) {
      SpatialPrefetchCandidates.back().IsWrite = true;
    }
  }
  return;
}

static void collectLoadLvalSB(
    HLInst *HInst, SmallVectorImpl<PragmaInfo> &PragmaVarsDistsHints,
    DenseMap<unsigned, std::tuple<int, int, bool>> &CandidateVarSBsDistsHints) {
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
          std::make_tuple(It->Dist, It->Hint, It->HasSpecifiedHintOrDist);
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
    DenseMap<unsigned, std::tuple<int, int, bool>> &CandidateVarSBsDistsHints,
    int &DefaultPrefetchDist, int &DefaultPrefetchHint,
    bool &DefaultHasSpecifiedHintOrDist, bool &HasPrefetchAll) {
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
    bool HasSpecifiedHintOrDist = false;

    if (Dist == -1) {
      Dist = DefaultPrefetchDist;
    } else {
      Dist *= LpStride;
      HasSpecifiedHintOrDist = true;
    }

    // Prefetch pragma hint is ranging from 0 - 3, while
    // Prefetch intrinsic's locality is ranging from (0) - no
    // locality, to (3) - extremely local keep in cache. Thus, we need to
    // transfer prefetch pragma hint to prefetch intrinsic's locality using
    // 3 - Hint
    if (Hint == -1) {
      Hint = DefaultPrefetchHint;
    } else {
      Hint = 3 - Hint;
      HasSpecifiedHintOrDist = true;
    }

    if (Var->isNull()) {
      DefaultPrefetchDist = Dist;
      DefaultPrefetchHint = Hint;
      DefaultHasSpecifiedHintOrDist = HasSpecifiedHintOrDist;

      // In the case of #pragma no prefetch, Dist = 0
      if (Dist) {
        HasPrefetchAll = true;
      }
      continue;
    }

    PragmaVarsDistsHints.emplace_back(Var->getBasePtrSymbase(), Hint, Dist,
                                      HasSpecifiedHintOrDist);
    CandidateVarSBsDistsHints[Var->getBasePtrSymbase()] =
        std::make_tuple(Dist, Hint, HasSpecifiedHintOrDist);
  }

  if (PragmaVarsDistsHints.empty()) {
    return;
  }

  // Go through the load insts in the preheader of the loop and collect the
  // corresponding lval with the rval in the pragma vars
  for (auto II = Lp->pre_begin(), EE = Lp->pre_end(); II != EE; ++II) {
    HLInst *HInst = cast<HLInst>(&*II);
    collectLoadLvalSB(HInst, PragmaVarsDistsHints, CandidateVarSBsDistsHints);
  }

  // Go through the insts before the current loop and collect the
  // corresponding lval with the rval in the pragma vars until it hits the prev
  // loop
  HLNode *Node = Lp;
  while ((Node = Node->getPrevNode())) {
    if (isa<HLLoop>(Node)) {
      break;
    } else if (auto *Inst = dyn_cast<HLInst>(Node)) {
      collectLoadLvalSB(Inst, PragmaVarsDistsHints, CandidateVarSBsDistsHints);
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

void HIRPrefetching::collectIndirectPrefetchingCandidates(
    HLLoop *Lp, const RegDDRef *CandidateRef, int Dist, int Hint,
    bool HasSpecifiedHintOrDist,
    SmallVectorImpl<PrefetchCandidateInfo> &IndirectPrefetchCandidates) {
  const BlobDDRef *NonLinearBlobRef = CandidateRef->getSingleNonLinearBlobRef();

  if (!NonLinearBlobRef) {
    return;
  }

  DDGraph DDG = DDA.getGraph(Lp);
  unsigned Level = Lp->getNestingLevel();
  int64_t ConstStride;

  if (DDG.getNumIncomingEdges(NonLinearBlobRef) != 1) {
    return;
  }

  DDEdge *Edge = *DDG.incoming_edges_begin(NonLinearBlobRef);

  auto *Src = Edge->getSrc();
  HLInst *SrcInst = cast<HLInst>(Src->getHLDDNode());

  if (!isa<LoadInst>(SrcInst->getLLVMInstruction())) {
    return;
  }

  RegDDRef *IndexRef = SrcInst->getRvalDDRef();

  if (!IndexRef->getConstStrideAtLevel(Level, &ConstStride)) {
    return;
  }

  // Skip multiple occurences of IV case
  unsigned NumIVDims = 0;
  for (unsigned Dim = IndexRef->getNumDimensions(); Dim > 0; --Dim) {
    auto *IndexCE = IndexRef->getDimensionIndex(Dim);

    if (IndexCE->hasIV(Level)) {
      ++NumIVDims;
    }

    if (NumIVDims > 1) {
      return;
    }
  }

  if (NumIVDims != 1) {
    return;
  }

  // Record the indirect prefetch info for the transformation
  IndirectPrefetchCandidates.emplace_back(CandidateRef, IndexRef, Dist, Hint,
                                          CandidateRef->isLval(),
                                          HasSpecifiedHintOrDist);

  return;
}

bool HIRPrefetching::doAnalysis(
    HLLoop *Lp, bool &HasPragmaInfo, int &DefaultPrefetchDist,
    SmallVectorImpl<PrefetchCandidateInfo> &SpatialPrefetchCandidates,
    SmallVectorImpl<PrefetchCandidateInfo> &IndirectPrefetchCandidates) {
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

  DenseMap<unsigned, std::tuple<int, int, bool>> CandidateVarSBsDistsHints;
  DefaultPrefetchDist = getPrefetchingDist(Lp);
  int DefaultPrefetchHint = 3 - ForceHint;
  bool DefaultHasSpecifiedHintOrDist = false;
  bool HasPrefetchAll = false;
  collectPrefetchPragmaInfo(Lp, CandidateVarSBsDistsHints, DefaultPrefetchDist,
                            DefaultPrefetchHint, DefaultHasSpecifiedHintOrDist,
                            HasPrefetchAll);

  unsigned Level = Lp->getNestingLevel();
  int64_t ConstStride;
  uint64_t Stride;
  unsigned NumNonLinearStreams = 0;

  for (auto &RefGroup : SpatialGroups) {
    const RegDDRef *FirstRef = RefGroup.front();
    unsigned FirstRefBasePtrSB = FirstRef->getBasePtrSymbase();

    int Dist = DefaultPrefetchDist;
    int Hint = DefaultPrefetchHint;
    int HasSpecifiedHintOrDist = DefaultHasSpecifiedHintOrDist;

    // Prefetch candidate is either the lval in the load inst in the preheader
    // or before loop or the pragma var in the loop
    if (CandidateVarSBsDistsHints.count(FirstRefBasePtrSB)) {
      std::tie(Dist, Hint, HasSpecifiedHintOrDist) =
          CandidateVarSBsDistsHints[FirstRefBasePtrSB];
    }

    if (!FirstRef->getConstStrideAtLevel(Level, &ConstStride) ||
        ConstStride == 0) {

      // Check whether the nonlinear ref is an indirect prefetching candidate
      // if it has pragma prefetch
      if (!FirstRef->isLinearAtLevel(Level)) {
        NumNonLinearStreams++;

        if (!FirstRef->getDestType()->isVectorTy() &&
            (CandidateVarSBsDistsHints.count(FirstRefBasePtrSB) ||
             HasPrefetchAll || EnableIndirectPrefetching)) {
          collectIndirectPrefetchingCandidates(Lp, FirstRef, Dist, Hint,
                                               HasSpecifiedHintOrDist,
                                               IndirectPrefetchCandidates);
        }
      }

      continue;
    }

    Stride = std::abs(ConstStride);

    // We only handle prefetching for refs where pragma info is specified.
    // For example,
    // #pragma  prefetch A
    // #pragma  prefetch B:2:20
    // for (i=0; i< N; i++) {
    //      A[i] = B[M[i]] + C[i];
    // }
    // prefetch is generated for A and B, but not C and M
    if (!HasPragmaInfo || HasPrefetchAll ||
        CandidateVarSBsDistsHints.count(FirstRefBasePtrSB)) {

      // When Stride is a non-zero constant, we will go through the RefGroup and
      // check whether they are in the same memory streams
      // When two refs in the same RefGroup are located in the different memory
      // streams because the distance between them is larger than the product of
      // loop stride and loop trip count, we need to create more scalar refs,
      // such as A[i] and A[i + 10000]
      collectPrefetchCandidates(RefGroup, TripCount, Stride, Level, Dist, Hint,
                                HasSpecifiedHintOrDist,
                                SpatialPrefetchCandidates);
    }
  }

  if (SpatialPrefetchCandidates.empty() && IndirectPrefetchCandidates.empty()) {
    return false;
  }

  if (!HasPragmaInfo &&
      (SpatialPrefetchCandidates.size() + NumNonLinearStreams) <
          NumMemoryStreamsThreshold &&
      !SkipNumMemoryStreamsCheck) {
    return false;
  }

  return true;
}

HLInst *HIRPrefetching::generatePrefetchingInst(HLLoop *Lp,
                                                RegDDRef *PrefetchRef,
                                                unsigned PrefetchHint,
                                                bool IsWrite) {
  auto &HNU = Lp->getHLNodeUtils();
  auto &DDRU = HNU.getDDRefUtils();

  auto Int32Ty = Type::getInt32Ty(HNU.getContext());
  int64_t RW = (EnablePrefetchW && IsWrite) ? 1 : 0;

  RegDDRef *RWTy = DDRU.createConstDDRef(Int32Ty, RW);
  RegDDRef *Locality = DDRU.createConstDDRef(Int32Ty, PrefetchHint);
  RegDDRef *DataCacheTy = DDRU.createConstDDRef(Int32Ty, 1);
  HLInst *PrefetchInst =
      HNU.createPrefetch(PrefetchRef, RWTy, Locality, DataCacheTy);
  return PrefetchInst;
}

void HIRPrefetching::processIndirectPrefetching(
    HLLoop *Lp, int64_t Stride,
    const SmallVectorImpl<PrefetchCandidateInfo> &IndirectPrefetchCandidates) {
  auto &HNU = Lp->getHLNodeUtils();
  auto &DDRU = HNU.getDDRefUtils();
  unsigned Level = Lp->getNestingLevel();
  CanonExpr *LoopUpperCE = Lp->getUpperCanonExpr();
  RegDDRef *LoopUpperRef = Lp->getUpperDDRef();

  OptReportBuilder &ORBuilder =
      Lp->getHLNodeUtils().getHIRFramework().getORBuilder();

  // Record the CE that is compared with loop trip count in HLIf predicate
  CanonExpr *CompareCE = nullptr;
  HLIf *CheckIf = nullptr;
  bool HasNewIf = false;
  HLInst *FirstIndirectPrefetch = nullptr;

  for (auto &PrefCand : IndirectPrefetchCandidates) {
    const RegDDRef *CandidateRef = PrefCand.CandidateRef;
    RegDDRef *OrigIndexLoadRef = PrefCand.OrigIndexLoadRef;
    int Dist = PrefCand.Dist;
    int Hint = PrefCand.Hint;
    bool IsWrite = PrefCand.IsWrite;
    bool HasSpecifiedHintOrDist = PrefCand.HasSpecifiedHintOrDist;

    const BlobDDRef *NonLinearBlobRef =
        CandidateRef->getSingleNonLinearBlobRef();
    assert(NonLinearBlobRef && "A BlobRef is expected.");

    RegDDRef *NewIndexRef = OrigIndexLoadRef->clone();

    // TODO:Use half of the default prefetch dist for indirect profetching
    // Create a new index load inst with prefetch dist for the use of indirect
    // prefetch For example, in the case
    // Do i1
    //   %2 = (%M)[i1];
    //   %add = (@C)[0][%2];
    // Enddo
    // we create a new index load inst %Load = (%M)[i1 + 31];
    // Thus, we can prefetch @C)[0][%Load] later
    NewIndexRef->shift(Level, Dist);

    auto NewIndexLoadInst = HNU.createLoad(NewIndexRef->clone(), "Load");

    RegDDRef *IndirectRef = CandidateRef->clone();

    IndirectRef->replaceTempBlob(
        NonLinearBlobRef->getBlobIndex(),
        NewIndexLoadInst->getLvalDDRef()->getSelfBlobIndex());

    unsigned DimNum = 0;

    // We only allow one occurence of IV case and check this in the analysis
    for (unsigned Dim = NewIndexRef->getNumDimensions(); Dim > 0; --Dim) {
      auto *IndexCE = NewIndexRef->getDimensionIndex(Dim);

      if (IndexCE->hasIV(Level)) {
        DimNum = Dim;
        break;
      }
    }

    auto *NewIndexCE = NewIndexRef->getDimensionIndex(DimNum);
    auto *OriginCE = OrigIndexLoadRef->getDimensionIndex(DimNum);

    // In the indirect prefetch case
    // Do i1
    //    %2 = (%M)[i1];
    //    %add = (@C)[0][%2];
    // Enddo
    //
    // we need to create a HLIf
    // if (i1 + 31 <=u zext.i32.i64(%N) + -1)
    // {
    //    %Load = (%M)[i1 + 31];
    //    @llvm.prefetch.p0i8(&((i8*)(@C)[0][%Load]),  0,  3,  1);
    // }
    // The LHS of if predicate is the new dim index i1 + 31 in %M[i1 + 31]
    // The RHS of if predicate is the original index i1 in %M[i1] and replaced
    // iv with loop upper bound
    RegDDRef *PredicateLHS =
        DDRU.createScalarRegDDRef(GenericRvalSymbase, NewIndexCE);
    PredicateLHS->makeConsistent({}, Level);

    CanonExpr *OriginCEClone = OriginCE->clone();
    CanonExprUtils::replaceIVByCanonExpr(OriginCEClone, Level, LoopUpperCE,
                                         Lp->hasSignedIV());

    RegDDRef *PredicateRHS =
        DDRU.createScalarRegDDRef(GenericRvalSymbase, OriginCEClone);
    PredicateRHS->makeConsistent({LoopUpperRef}, Level);

    int64_t Distance = 0;

    // When the original HLIf has a smaller CE, we update the LHS and RHS of
    // the HLIf predicate using the current NewIndexCE
    if (CompareCE &&
        CanonExprUtils::getConstDistance(CompareCE, NewIndexCE, &Distance)) {
      if (Distance < 0) {
        CheckIf->setLHSPredicateOperandDDRef(PredicateLHS,
                                             CheckIf->pred_begin());
        CheckIf->setRHSPredicateOperandDDRef(PredicateRHS,
                                             CheckIf->pred_begin());

        CompareCE = NewIndexCE;
      }

      HasNewIf = false;
      // else create a new HLIf and initialize FirstIndirectPrefetch to a
      // nullptr
    } else {
      CheckIf = HNU.createHLIf(CmpInst::ICMP_ULE, PredicateLHS, PredicateRHS);
      HasNewIf = true;
      CompareCE = NewIndexCE;
      FirstIndirectPrefetch = nullptr;
    }

    // Create a prefetch intrinsic for indirect ref. For example,
    // @llvm.prefetch.p0i8(&((i8*)(@C)[0][%Load]),  0,  3,  1);
    RegDDRef *PrefetchRef = IndirectRef->clone();
    PrefetchRef->setAddressOf(true);

    // Set destination address (i8*)
    PrefetchRef->setBitCastDestVecOrElemType(
        Type::getInt8Ty(HIRF.getContext()));

    HLInst *PrefetchInst =
        generatePrefetchingInst(Lp, PrefetchRef, Hint, IsWrite);

    if (!FirstIndirectPrefetch) {
      HLNodeUtils::insertAsLastThenChild(CheckIf, NewIndexLoadInst);
      FirstIndirectPrefetch = PrefetchInst;
    } else {
      HLNodeUtils::insertBefore(FirstIndirectPrefetch, NewIndexLoadInst);
    }
    HLNodeUtils::insertAsLastThenChild(CheckIf, PrefetchInst);

    if (HasNewIf) {
      HLNodeUtils::insertAsLastChild(Lp, CheckIf);
    }

    if (HasSpecifiedHintOrDist) {
      int PragmaDist = Dist / Stride;
      int PragmaHint = 3 - Hint;

      // Using directive-based hint=%d, distance=%d for indirect memory
      // reference
      ORBuilder(*Lp).addRemark(OptReportVerbosity::Low, 25150u, PragmaHint,
                               PragmaDist);
    }
  }
}

bool HIRPrefetching::doPrefetching(
    HLLoop *Lp, bool HasPragmaInfo, int DefaultPrefetchDist,
    const SmallVectorImpl<PrefetchCandidateInfo> &SpatialPrefetchCandidates,
    const SmallVectorImpl<PrefetchCandidateInfo> &IndirectPrefetchCandidates) {
  unsigned NumIndirectPrefetches = IndirectPrefetchCandidates.size();
  unsigned NumSpatialPrefetches = SpatialPrefetchCandidates.size();
  RegDDRef *StrideRef = Lp->getStrideDDRef();
  int64_t Stride;
  StrideRef->isIntConstant(&Stride);
  DefaultPrefetchDist /= Stride;

  OptReportBuilder &ORBuilder =
      Lp->getHLNodeUtils().getHIRFramework().getORBuilder();

  if (ORBuilder.isOptReportOn()) {
    // Total number of lines prefetched=%d
    ORBuilder(*Lp).addRemark(OptReportVerbosity::Low, 25018u,
                             NumSpatialPrefetches + NumIndirectPrefetches);

    // Number of spatial prefetches=%d, default dist=%d
    ORBuilder(*Lp).addRemark(OptReportVerbosity::Low, 25019u,
                             NumSpatialPrefetches, DefaultPrefetchDist);

    if (NumIndirectPrefetches) {
      // Number of indirect prefetches=%d, default dist=%d
      ORBuilder(*Lp).addRemark(OptReportVerbosity::Low, 25033u,
                               NumIndirectPrefetches, DefaultPrefetchDist);
    }
  }

  if (!IndirectPrefetchCandidates.empty()) {
    processIndirectPrefetching(Lp, Stride, IndirectPrefetchCandidates);
  }

  for (auto It = SpatialPrefetchCandidates.begin(),
            E = SpatialPrefetchCandidates.end();
       It != E; ++It) {
    const RegDDRef *Ref = It->CandidateRef;
    int PrefetchDist = It->Dist;
    int PrefetchHint = It->Hint;
    bool IsWrite = It->IsWrite;
    bool HasSpecifiedHintOrDist = It->HasSpecifiedHintOrDist;

    RegDDRef *PrefetchRef = Ref->clone();

    PrefetchRef->setAddressOf(true);

    // Set destination address (i8*)
    PrefetchRef->setBitCastDestVecOrElemType(
        Type::getInt8Ty(HIRF.getContext()));

    unsigned Level = Lp->getNestingLevel();
    PrefetchRef->shift(Level, PrefetchDist);

    HLInst *PrefetchInst =
        generatePrefetchingInst(Lp, PrefetchRef, PrefetchHint, IsWrite);
    HLNodeUtils::insertAsLastChild(Lp, PrefetchInst);

    if (HasSpecifiedHintOrDist) {
      int PragmaHint = 3 - PrefetchHint;
      int PragmaDist = PrefetchDist / Stride;
      // Using directive-based hint=%d, distance=%d for prefetching spatial
      // memory reference
      ORBuilder(*Lp).addRemark(OptReportVerbosity::Low, 25147u, PragmaHint,
                               PragmaDist);
    }
  }

  if (HasPragmaInfo) {
    Lp->getParentRegion()->setGenCode();
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
    SmallVector<PrefetchCandidateInfo, 64> SpatialPrefetchCandidates;
    SmallVector<PrefetchCandidateInfo, 4> IndirectPrefetchCandidates;
    bool HasPragmaInfo = false;
    int DefaultPrefetchDist = 0;
    // Analyze the loop and check if it is suitable for prefetching
    if (!doAnalysis(Lp, HasPragmaInfo, DefaultPrefetchDist,
                    SpatialPrefetchCandidates, IndirectPrefetchCandidates)) {
      continue;
    }

    Result =
        doPrefetching(Lp, HasPragmaInfo, DefaultPrefetchDist,
                      SpatialPrefetchCandidates, IndirectPrefetchCandidates) ||
        Result;
  }

  return Result;
}

PreservedAnalyses HIRPrefetchingPass::runImpl(llvm::Function &F,
                                              llvm::FunctionAnalysisManager &AM,
                                              HIRFramework &HIRF) {
  HIRPrefetching(HIRF, AM.getResult<HIRLoopLocalityAnalysis>(F),
                 AM.getResult<HIRDDAnalysisPass>(F),
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
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
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
               getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
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
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopLocalityWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopResourceWrapperPass)
INITIALIZE_PASS_END(HIRPrefetchingLegacyPass, OPT_SWITCH, OPT_DESC, false,
                    false)

FunctionPass *llvm::createHIRPrefetchingPass() {
  return new HIRPrefetchingLegacyPass();
}
