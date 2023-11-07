//===- HIRIdiomRecognition.cpp - Implements Loop idiom recognition pass ---===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// The pass mostly recognizes the memory operations like memset, memcpy,
// memmove. It replaces the stores and loads with the intrinsic calls in the
// pre-header.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRIdiomRecognitionPass.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/TargetLibraryInfo.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#define OPT_SWITCH "hir-idiom"
#define OPT_DESC "HIR Loop Idiom Recognition"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

static cl::list<unsigned>
    TransformNodes(OPT_SWITCH "-nodes", cl::Hidden,
                   cl::desc("List nodes to transform by " OPT_DESC));

static cl::opt<unsigned>
    SmallTripCount(OPT_SWITCH "-small-trip-count", cl::init(12), cl::Hidden,
                   cl::desc("Generate small trip count check while " OPT_DESC));

STATISTIC(NumMemSet, "Number of memset's formed from loop stores");
STATISTIC(NumMemCpy, "Number of memcpy's formed from loop load+stores");

namespace {

struct MemOpCandidate {
  HLInst *DefInst;

  RegDDRef *StoreRef;
  RegDDRef *RHS;

  bool IsStoreNegStride;

  bool isMemset() const { return RHS->isTerminalRef(); }
  bool isMemcopy() const { return RHS->isMemRef(); }

  MemOpCandidate()
      : DefInst(nullptr), StoreRef(nullptr), RHS(nullptr),
        IsStoreNegStride(false) {}

  MemOpCandidate(RegDDRef *StoreRef)
      : StoreRef(StoreRef), IsStoreNegStride(false) {
    DefInst = cast<HLInst>(StoreRef->getHLDDNode());
    RHS = DefInst->getRvalDDRef();
  }
};

struct HoistLocation {
  bool Preheader = true;
  bool Postexit = true;
};

class HIRIdiomRecognition {
  HIRFramework &HIRF;
  HIRDDAnalysis &DDA;
  TargetLibraryInfo &TLI;

  const DataLayout &DL;
  Module &M;

  bool HasMemcopy;
  bool HasMemset;

  // Track removed nodes to ignore obsolete dependencies.
  SmallPtrSet<HLNode *, 8> RemovedNodes;

  // Checks if the candidate is legal from DDG point of view.
  template <bool IsOutgoing>
  bool isLegalDV(const DirectionVector &DV, DDEdge &Edge, unsigned Level,
                 HoistLocation &HoistLoc);

  template <bool IsOutgoing>
  bool isLegalGraph(const DDGraph &DDG, const HLLoop *Loop, const RegDDRef *Ref,
                    HoistLocation &HoistLoc);

  bool isLegalCandidate(const HLLoop *Loop, const MemOpCandidate &Candidate,
                        HoistLocation &HoistLoc);

  // Analyze and create the transformation candidate for the reference.
  bool analyzeStore(HLLoop *Loop, RegDDRef *Ref, MemOpCandidate &Candidate);

  // Transform \p Candidates into memset calls
  bool processMemset(HLLoop *Loop, bool &ExtractPreheader,
                     MemOpCandidate &Candidate, HoistLocation &HoistLoc,
                     HLNode *PostexitInsertPt);

  // Transform \p Candidates into memcpy calls
  bool processMemcpy(HLLoop *Loop, bool &ExtractPreheader,
                     MemOpCandidate &Candidate, HoistLocation &HoistLoc,
                     HLNode *PostexitInsertPt);

  // Helper method to emit memset call.
  bool genMemset(HLLoop *Loop, MemOpCandidate &Candidate, int64_t StoreSize,
                 bool IsNegStride, bool &ExtractPreheader,
                 HoistLocation &HoistLoc, HLNode *PostexitInsertPt);

  // Check if the \p Ref could be represented by a repeating i8 type.
  // Update RegDDRef if \p DoBitcast is true.
  static bool isBytewiseValue(RegDDRef *Ref, bool DoBitcast);

  // Creates a ref represented a size of the store across \p Loop iterations.
  static RegDDRef *createSizeDDRef(HLLoop *Loop, int64_t StoreSize);

  // Update \p Ref to represent a leftmost memory location for the loop-variant
  // reference.
  bool makeStartRef(RegDDRef *Ref, HLLoop *Loop, bool IsNegStride);

  // Creates fake ddref for the memset/memcpy calls
  RegDDRef *createFakeDDRef(const RegDDRef *Ref);

public:
  HIRIdiomRecognition(HIRFramework &HIRF, HIRLoopStatistics &HLS,
                      HIRDDAnalysis &DDA, TargetLibraryInfo &TLI)
      : HIRF(HIRF), DDA(DDA), TLI(TLI), DL(HIRF.getDataLayout()),
        M(HIRF.getModule()), HasMemcopy(false), HasMemset(false) {}

  bool run();
  bool runOnLoop(HLLoop *Loop);
};
} // namespace

/// If the specified terminal /p Ref can be set by repeating the same byte in
/// memory, return true. This can be done for all i8 values obviously, but is
/// also true for i32 0, i32 -1, i16 0xF0F0, double 0.0 etc. If the value can't
/// be handled with a repeated byte store (e.g. i16 0x1234), return false.
/// Also update the c0 value, Src and Dest types if /p DoBitcast is true.
bool HIRIdiomRecognition::isBytewiseValue(RegDDRef *Ref, bool DoBitcast) {
  if (Ref->getDestType()->isIntegerTy(8)) {
    // Do nothing as the destination type is already desired type.
    return true;
  }

  if (!Ref->isConstant()) {
    return false;
  }

  auto BitcastRef = [DoBitcast](APInt SplatValue, RegDDRef *Ref) -> bool {
    bool GoodToCast =
        (SplatValue.getBitWidth() % 8 == 0) && SplatValue.isSplat(8);

    if (GoodToCast && DoBitcast) {
      CanonExpr *CE = Ref->getSingleCanonExpr();
      Type *I8TY = Type::getInt8Ty(Ref->getDDRefUtils().getContext());

      CE->clear();
      CE->setSrcAndDestType(I8TY);

      if (SplatValue.getBitWidth() > 8) {
        SplatValue = SplatValue.trunc(8);
      }

      CE->setConstant(SplatValue.getSExtValue());
    }

    return GoodToCast;
  };

  if (Ref->isNull()) {
    return BitcastRef(APInt(8, 0), Ref);
  }

  ConstantFP *FPC;
  if (Ref->isFPConstant(&FPC)) {
    return BitcastRef(FPC->getValueAPF().bitcastToAPInt(), Ref);
  }

  int64_t Value;
  if (Ref->isIntConstant(&Value)) {
    APInt APValue(Ref->getDestType()->getIntegerBitWidth(), Value);
    return BitcastRef(APValue, Ref);
  }

  // TODO: add vector constant support.

  return false;
}

// Check whether it is legal to hoist candidate with DV to either preheader or
// postexit.
template <bool IsOutgoing>
bool HIRIdiomRecognition::isLegalDV(const DirectionVector &DV, DDEdge &Edge,
                                    unsigned Level, HoistLocation &HoistLoc) {
  if (DV.isIndepFromLevel(Level)) {
    return true;
  }

  if (IsOutgoing) {
    // If there is an outoing edge from the candidate ref, we can hoist to
    // preheader to maintain 'before' relationship of the edge but not to
    // postexit.
    //
    // For example-
    //
    // DO i1
    //   A[i1+1] = 0;       // preheader memcpy candidate
    //   B[i1] = A[i1] + 1;
    // END DO
    //
    // A[i1+1] -> A[i1] FLOW (<)
    //
    HoistLoc.Postexit = false;

    LLVM_DEBUG(dbgs() << "Edge prevents sinking to postexit:\n");
    LLVM_DEBUG(Edge.dump());

    // Hoisting to preheader was previously found illegal, just return false.
    if (!HoistLoc.Preheader) {
      return false;
    }

  } else {
    // If there is an incoming edge to the candidate ref, we can sink to
    // postexit to maintain 'after' relationship of the edge but not to
    // preheader.
    //
    // For example-
    //
    // DO i1
    //   A[i1] = 0;       // postexit memcpy candidate
    //   B[i1] = A[i1+1] + 1;
    // END DO
    //
    // A[i1+1] -> A[i1] ANTI (<)
    //
    HoistLoc.Preheader = false;

    LLVM_DEBUG(dbgs() << "Edge prevents hoisting to preheader:\n");
    LLVM_DEBUG(Edge.dump());

    // Hoisting to postexit was previously found illegal, just return false.
    if (!HoistLoc.Postexit) {
      return false;
    }
  }

  DVKind DVElem = DV[Level - 1];

  // '<' and '=' maintain before/after relationship.
  bool IsLegal = (DVElem == DVKind::EQ || DVElem == DVKind::LT);

  if (!IsLegal) {
    LLVM_DEBUG(dbgs() << "Edge with illegal DV:\n");
    LLVM_DEBUG(Edge.dump());
  }

  return IsLegal;
}

template <bool IsOutgoing>
bool HIRIdiomRecognition::isLegalGraph(const DDGraph &DDG, const HLLoop *Loop,
                                       const RegDDRef *Ref,
                                       HoistLocation &HoistLoc) {
  unsigned Level = Loop->getNestingLevel();

  auto Range = IsOutgoing ? DDG.outgoing(Ref) : DDG.incoming(Ref);

  for (DDEdge *E : Range) {
    DDRef *OtherRef = IsOutgoing ? E->getSink() : E->getSrc();
    const DirectionVector *DV = &E->getDV();

    // In case of the target loop is not the topmost loop we need to refine the
    // dependency to be less conservative.
    RefinedDependence RefinedDep;
    // RefinedDV fails for removed nodes so we conservatively only check
    // available DV.
    if (Level != 1 && !RemovedNodes.count(OtherRef->getHLDDNode())) {
      RefinedDep = DDA.refineDV(E, Level, Level, false);
      if (RefinedDep.isIndependent()) {
        LLVM_DEBUG(dbgs() << "Edge was refined as independant:\n");
        LLVM_DEBUG(E->dump());
        continue;
      }

      if (RefinedDep.isRefined()) {
        DV = &RefinedDep.getDV();
      }
    }

    if (!isLegalDV<IsOutgoing>(*DV, *E, Level, HoistLoc)) {
      return false;
    }
  }

  return true;
}

bool HIRIdiomRecognition::isLegalCandidate(const HLLoop *Loop,
                                           const MemOpCandidate &Candidate,
                                           HoistLocation &HoistLoc) {
  DDGraph DDG = DDA.getGraph(Loop);

  if (!isLegalGraph<true>(DDG, Loop, Candidate.StoreRef, HoistLoc)) {
    return false;
  }

  if (!isLegalGraph<false>(DDG, Loop, Candidate.StoreRef, HoistLoc)) {
    return false;
  }

  if (Candidate.isMemcopy()) {
    if (!isLegalGraph<true>(DDG, Loop, Candidate.RHS, HoistLoc)) {
      return false;
    }

    if (!isLegalGraph<false>(DDG, Loop, Candidate.RHS, HoistLoc)) {
      return false;
    }
  }

  LLVM_DEBUG(dbgs() << "OK\n");

  return true;
}

bool HIRIdiomRecognition::analyzeStore(HLLoop *Loop, RegDDRef *Ref,
                                       MemOpCandidate &Candidate) {
  Candidate = MemOpCandidate(Ref);

  bool ForMemset = false;
  bool ForMemcpy = false;

  assert(Ref->isMemRef() && Ref->isLval() &&
         "Only lval memory references are expected here");

  unsigned LoopLevel = Loop->getNestingLevel();

  // Check that store stride is constant.
  if (!Ref->isUnitStride(LoopLevel, Candidate.IsStoreNegStride)) {
    return false;
  }

  RegDDRef *RHS = Candidate.RHS;

  if (RHS->isTerminalRef()) {
    const CanonExpr *CE = RHS->getSingleCanonExpr();
    if (CE->isNonLinear()) {
      // Could be legal for memcopy
      ForMemcpy = HasMemcopy;
    } else {
      // Could be legal for memset
      ForMemset = HasMemset;
    }
  } else {
    // Could be legal for memcopy
    ForMemcpy = HasMemcopy;
  }

  if (ForMemset) {
    // Check if the RHS could be represented as a i8 value.
    if (!isBytewiseValue(RHS, false)) {
      return false;
    }

    const CanonExpr *CE = RHS->getSingleCanonExpr();
    if (!CE->isInvariantAtLevel(LoopLevel)) {
      return false;
    }

    return true;
  }

  if (ForMemcpy) {
    if (!RHS->isMemRef()) {
      LLVM_DEBUG(
          dbgs() << "WARNING: Something like a[i] = %0 is found, this pass "
                    "depends on the TempCleanup pass. Check if the input HIR "
                    "is already clean.\n");
      return false;
    }

    bool IsNegStride;
    if (!RHS->isUnitStride(LoopLevel, IsNegStride)) {
      return false;
    }

    if (IsNegStride != Candidate.IsStoreNegStride) {
      return false;
    }

    return true;
  }

  LLVM_DEBUG(dbgs() << "Unsupported candidate\n");
  return false;
}

bool HIRIdiomRecognition::makeStartRef(RegDDRef *Ref, HLLoop *Loop,
                                       bool IsNegStride) {
  bool BlobUpdateNeeded = false;

  // Update IV to zero or UB.
  for (CanonExpr *CE : llvm::make_range(Ref->canon_begin(), Ref->canon_end())) {
    unsigned Level = Loop->getNestingLevel();

    if (IsNegStride) {
      const CanonExpr *OrigUpperCE = Loop->getUpperCanonExpr();

      // Try to merge upper bound with CE
      if (!CanonExprUtils::replaceIVByCanonExpr(CE, Level, OrigUpperCE,
                                                Loop->hasSignedIV(), true)) {
        LLVM_DEBUG(dbgs() << "Unable to replace i" << Level << " with UB.");
        return false;
      }

      BlobUpdateNeeded = true;
    } else {
      // Positive stride, loop normalized.
      CE->replaceIVByConstant(Level, 0);
    }
  }

  unsigned PreheaderLevel = Loop->getNestingLevel() - 1;

  if (BlobUpdateNeeded) {
    // Make blobs consistent because of upper bound substitution.
    Ref->makeConsistent(Loop->getUpperDDRef(), PreheaderLevel);
  } else {
    // Update of blob definition level is still required as we move statements
    // to the pre-header.
    Ref->updateDefLevel(PreheaderLevel);
  }

  Ref->setAddressOf(true);

  // Set destination address (i8*)
  Ref->setBitCastDestVecOrElemType(Type::getInt8Ty(HIRF.getContext()));

  return true;
}

RegDDRef *HIRIdiomRecognition::createFakeDDRef(const RegDDRef *Ref) {
  RegDDRef *FakeRef = Ref->clone();
  FakeRef->setAddressOf(false);

  return FakeRef;
}

RegDDRef *HIRIdiomRecognition::createSizeDDRef(HLLoop *Loop,
                                               int64_t StoreSize) {
  unsigned PreheaderLevel = Loop->getNestingLevel() - 1;

  std::unique_ptr<RegDDRef> Size(Loop->getTripCountDDRef(PreheaderLevel));
  CanonExpr *SizeCE = Size->getSingleCanonExpr();
  if (!SizeCE->multiplyByConstant(StoreSize)) {
    return nullptr;
  }

  // Multiplication could convert self blob to a regular blob.
  Size->makeConsistent(Loop->getUpperDDRef(), PreheaderLevel);

  return Size.release();
}

bool HIRIdiomRecognition::genMemset(HLLoop *Loop, MemOpCandidate &Candidate,
                                    int64_t StoreSize, bool IsNegStride,
                                    bool &ExtractPreheader,
                                    HoistLocation &HoistLoc,
                                    HLNode *PostexitInsertPt) {
  HLNodeUtils &HNU = HIRF.getHLNodeUtils();

  std::unique_ptr<RegDDRef> Ref(Candidate.StoreRef->clone());
  if (!makeStartRef(Ref.get(), Loop, IsNegStride)) {
    return false;
  }

  RegDDRef *Size = createSizeDDRef(Loop, StoreSize);

  if (!Size) {
    return false;
  }

  // Remove store
  RemovedNodes.insert(Candidate.DefInst);
  HLNodeUtils::remove(Candidate.DefInst);

  RegDDRef *RHS = Candidate.DefInst->removeRvalDDRef();
  assert(Candidate.RHS == RHS && "DefInst Rval DDRef is a Candidate RHS");

  bool Bitcast = isBytewiseValue(RHS, true);
  assert(Bitcast && "Non-regular candidate, all of them should be filtered");
  (void)Bitcast;

  // The i8 blob could be non linear at the pre-header level.
  RHS->updateDefLevel(Loop->getNestingLevel() - 1);

  HLInst *MemsetInst = HNU.createMemset(Ref.get(), RHS, Size);
  MemsetInst->addFakeLvalDDRef(createFakeDDRef(Ref.release()));
  if (ExtractPreheader) {
    Loop->extractPreheader();
    ExtractPreheader = false;
  }

  if (HoistLoc.Preheader) {
    HLNodeUtils::insertAsLastPreheaderNode(Loop, MemsetInst);

  } else {
    assert(HoistLoc.Postexit &&
           "Legal candidate should be hoistable to preheader or postexit!");

    if (PostexitInsertPt) {
      HLNodeUtils::insertBefore(PostexitInsertPt, MemsetInst);
    } else {
      HLNodeUtils::insertAsLastPostexitNode(Loop, MemsetInst);
    }
  }

  LLVM_DEBUG(dbgs() << "Generated memset:\n");
  LLVM_DEBUG(MemsetInst->dump());
  LLVM_DEBUG(dbgs() << "\n");

  NumMemSet++;

  return true;
}

bool HIRIdiomRecognition::processMemset(HLLoop *Loop, bool &ExtractPreheader,
                                        MemOpCandidate &Candidate,
                                        HoistLocation &HoistLoc,
                                        HLNode *PostexitInsertPt) {

  unsigned StoreSize = Candidate.RHS->getDestTypeSizeInBytes();

  if (genMemset(Loop, Candidate, StoreSize, Candidate.IsStoreNegStride,
                ExtractPreheader, HoistLoc, PostexitInsertPt)) {
    OptReportBuilder &ORBuilder =
        Loop->getHLNodeUtils().getHIRFramework().getORBuilder();

    // Memset generated
    ORBuilder(*Loop).addRemark(OptReportVerbosity::Low,
                               OptRemarkID::MemsetGenerated);
    return true;
  }

  LLVM_DEBUG(dbgs() << "Failed to generate memset for chain:\n");
  LLVM_DEBUG(Candidate.StoreRef->dump());
  LLVM_DEBUG(dbgs() << "\n");

  return false;
}

bool HIRIdiomRecognition::processMemcpy(HLLoop *Loop, bool &ExtractPreheader,
                                        MemOpCandidate &Candidate,
                                        HoistLocation &HoistLoc,
                                        HLNode *PostexitInsertPt) {

  HLNodeUtils &HNU = HIRF.getHLNodeUtils();

  std::unique_ptr<RegDDRef> StoreRef(Candidate.StoreRef->clone());
  std::unique_ptr<RegDDRef> LoadRef(Candidate.RHS->clone());

  unsigned StoreSize = StoreRef.get()->getDestTypeSizeInBytes();

  if (!makeStartRef(StoreRef.get(), Loop, Candidate.IsStoreNegStride)) {
    return false;
  }

  if (!makeStartRef(LoadRef.get(), Loop, Candidate.IsStoreNegStride)) {
    return false;
  }

  RegDDRef *Size = createSizeDDRef(Loop, StoreSize);

  if (!Size) {
    return false;
  }

  HLInst *MemcpyInst =
      HNU.createMemcpy(StoreRef.get(), LoadRef.get(), Size);
  MemcpyInst->addFakeLvalDDRef(createFakeDDRef(StoreRef.release()));
  MemcpyInst->addFakeRvalDDRef(createFakeDDRef(LoadRef.release()));

  if (ExtractPreheader) {
    Loop->extractPreheader();
    ExtractPreheader = false;
  }

  if (HoistLoc.Preheader) {
    HLNodeUtils::insertAsLastPreheaderNode(Loop, MemcpyInst);

  } else {
    assert(HoistLoc.Postexit &&
           "Legal candidate should be hoistable to preheader or postexit!");

    if (PostexitInsertPt) {
      HLNodeUtils::insertBefore(PostexitInsertPt, MemcpyInst);
    } else {
      HLNodeUtils::insertAsLastPostexitNode(Loop, MemcpyInst);
    }
  }

  HLNodeUtils::remove(Candidate.DefInst);
  RemovedNodes.insert(Candidate.DefInst);

  LLVM_DEBUG(dbgs() << "Generated memcpy:\n");
  LLVM_DEBUG(MemcpyInst->dump());
  LLVM_DEBUG(dbgs() << "\n");

  NumMemCpy++;

  OptReportBuilder &ORBuilder =
      Loop->getHLNodeUtils().getHIRFramework().getORBuilder();

  // Memcopy generated
  ORBuilder(*Loop).addRemark(OptReportVerbosity::Low,
                             OptRemarkID::MemcopyGenerated);
  return true;
}

static HLIf *createTripCountCheck(const HLLoop *Loop) {
  // Don't create trip count check if the loop has constant trip count, the
  // trip count is small, or the loop is not the innermost. We skip
  // multiversioning for outer loops since they aren't profitable for
  // vectorization.
  if (SmallTripCount == 0 || Loop->isConstTripLoop() || !Loop->isInnermost()) {
    return nullptr;
  }

  RegDDRef *TCRef = Loop->getTripCountDDRef();
  assert(TCRef && "Only Unknown loops may not have a trip count");

  return Loop->getHLNodeUtils().createHLIf(
      CmpInst::ICMP_UGT, TCRef,
      Loop->getDDRefUtils().createConstDDRef(TCRef->getDestType(),
                                             SmallTripCount));
}

static bool isSmallCountLoop(const HLLoop *Loop) {
  if (SmallTripCount == 0)
    return false;

  if (Loop->hasLikelySmallTripCount(SmallTripCount))
    return true;

  return false;
}

bool HIRIdiomRecognition::runOnLoop(HLLoop *Loop) {
  LLVM_DEBUG(dbgs() << "\nProcessing Loop: <" << Loop->getNumber() << ">\n");

  if (!Loop->isDo() || !Loop->isNormalized() || Loop->isSIMD() ||
      Loop->hasUnrollEnablingPragma() || Loop->hasVectorizeEnablingPragma()) {
    LLVM_DEBUG(
        dbgs() << "Skipping - non-DO-Loop / non-Normalized / SIMD / unroll "
                  "pragma loop\n");
    return false;
  }

  if (isSmallCountLoop(Loop)) {
    LLVM_DEBUG(dbgs() << "Skipping small trip count loops\n");
    return false;
  }

  // Check for EH context
  // HLS->getTotalStatistics()

  SmallVector<MemOpCandidate, 16> Candidates;

  for (HLNode &Node :
       llvm::make_range(Loop->child_begin(), Loop->child_end())) {
    HLInst *Inst = dyn_cast<HLInst>(&Node);
    if (!Inst) {
      continue;
    }

    RegDDRef *Ref = Inst->getLvalDDRef();

    if (!Ref || !Ref->isMemRef() ||
        !isa<StoreInst>(Inst->getLLVMInstruction())) {
      continue;
    }

    // Check if executes on each iteration
    if (!HLNodeUtils::postDominates(Inst, Loop->getFirstChild())) {
      continue;
    }

    MemOpCandidate NewCandidate;

    if (!analyzeStore(Loop, Ref, NewCandidate)) {
      continue;
    }

    Candidates.push_back(NewCandidate);
  }

  HLLoop *OrigLoopClone = nullptr;
  HLIf *SmallTripCountCheck = nullptr;
  bool ExtractPreheader = false;

  if (!Candidates.empty()) {
    if ((SmallTripCountCheck = createTripCountCheck(Loop))) {
      ExtractPreheader = true;
      OrigLoopClone = Loop->clone();
    }

    LLVM_DEBUG(dbgs() << "Loop DD graph:\n");
    LLVM_DEBUG(DDA.getGraph(Loop).dump());
    LLVM_DEBUG(dbgs() << "\n");
  }

  bool Changed = false;
  // Postexit candidates will be inserted before the first postexit node, if it
  // exists.
  HLNode *PostexitInsertPt = Loop->getFirstPostexitNode();

  for (MemOpCandidate &Candidate : Candidates) {
    LLVM_DEBUG(dbgs() << "Analyzing candidate:\n");
    LLVM_DEBUG(Candidate.DefInst->dump(true));
    LLVM_DEBUG(dbgs() << "\n");

    HoistLocation HoistLoc;
    if (!isLegalCandidate(Loop, Candidate, HoistLoc)) {
      continue;
    }

    if (Candidate.isMemset()) {
      Changed = processMemset(Loop, ExtractPreheader, Candidate, HoistLoc,
                              PostexitInsertPt) ||
                Changed;
    } else if (Candidate.isMemcopy()) {
      Changed = processMemcpy(Loop, ExtractPreheader, Candidate, HoistLoc,
                              PostexitInsertPt) ||
                Changed;
    } else {
      llvm_unreachable("Unknown memopt kind");
    }
  }

  if (Changed) {
    // TODO: maybe do not multiversion the loop if it has other instructions
    // apart from memset/memcpy calls.
    if (SmallTripCountCheck) {
      OptReportBuilder &ORBuilder =
          Loop->getHLNodeUtils().getHIRFramework().getORBuilder();

      Loop->extractZtt();
      HLNodeUtils::replace(Loop, SmallTripCountCheck);
      HLNodeUtils::insertAsFirstThenChild(SmallTripCountCheck, Loop);
      HLNodeUtils::insertAsFirstElseChild(SmallTripCountCheck, OrigLoopClone);

      // Ztt and Preheader nodes are shared by both the loops.
      OrigLoopClone->removePreheader();
      OrigLoopClone->removeZtt();

      // Update max trip count info of known small trip count loop.
      OrigLoopClone->setPragmaBasedMaximumTripCount(SmallTripCount);
      OrigLoopClone->setMaxTripCountEstimate(SmallTripCount);
      OrigLoopClone->setLegalMaxTripCount(SmallTripCount);

      // The loop has been multiversioned for the small trip count
      // Multiversioned v1
      ORBuilder(*Loop)
          .addOrigin(OptRemarkID::LoopMultiversioned, 1)
          .addRemark(OptReportVerbosity::Low,
                     OptRemarkID::LoopMultiversionedSmallTripCount);

      // Multiversioned v2
      ORBuilder(*OrigLoopClone).addOrigin(OptRemarkID::LoopMultiversioned, 2);
    }

    HLNodeUtils::removeEmptyNodes(Loop, false);
  }

  return Changed;
}

bool HIRIdiomRecognition::run() {
  if (DisablePass) {
    return false;
  }

  LLVM_DEBUG(dbgs() << OPT_DESC " for Function: "
                    << HIRF.getFunction().getName() << "\n");

  HasMemcopy = TLI.has(LibFunc_memcpy);
  HasMemset = TLI.has(LibFunc_memset);

  if (!HasMemcopy && !HasMemset) {
    LLVM_DEBUG(dbgs() << "Memcpy and Memset calls are not available\n");
    return false;
  }

  SmallPtrSet<HLNode *, 8> NodesToInvalidate;

  ForPostEach<HLLoop>::visitRange(
      HIRF.hir_begin(), HIRF.hir_end(),
      [&NodesToInvalidate, this](HLLoop *Loop) {
        if (!TransformNodes.empty()) {
          if (std::find(TransformNodes.begin(), TransformNodes.end(),
                        Loop->getNumber()) == TransformNodes.end()) {
            LLVM_DEBUG(dbgs() << "Skipped due to the command line option\n");
            return;
          }
        }

        HLRegion *ParentRegion = Loop->getParentRegion();
        HLLoop *ParentLoop = Loop->getParentLoop();

        if (this->runOnLoop(Loop)) {
          ParentRegion->setGenCode();

          // Check if Loop was removed during
          // transformation
          if (Loop->isAttached()) {
            NodesToInvalidate.insert(Loop);
          }

          if (ParentLoop) {
            NodesToInvalidate.insert(ParentLoop);
          } else {
            NodesToInvalidate.insert(ParentRegion);
          }
        }
      });

  for (HLNode *Node : NodesToInvalidate) {
    if (HLRegion *Region = dyn_cast<HLRegion>(Node)) {
      HIRInvalidationUtils::invalidateNonLoopRegion(Region);
    } else {
      HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(
          cast<HLLoop>(Node));
    }
  }

  LLVM_DEBUG(dbgs() << "\n");
  return !NodesToInvalidate.empty();
}

PreservedAnalyses HIRIdiomRecognitionPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  ModifiedHIR =
      HIRIdiomRecognition(HIRF, AM.getResult<HIRLoopStatisticsAnalysis>(F),
                          AM.getResult<HIRDDAnalysisPass>(F),
                          AM.getResult<TargetLibraryAnalysis>(F))
          .run();
  return PreservedAnalyses::all();
}
