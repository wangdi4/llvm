//===- HIRIdiomRecognition.cpp - Implements Loop idiom recognition pass ---===//
//
// Copyright (C) 2016-2021 Intel Corporation. All rights reserved.
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

  MemOpCandidate() {}

  MemOpCandidate(RegDDRef *StoreRef) : StoreRef(StoreRef) {
    DefInst = cast<HLInst>(StoreRef->getHLDDNode());
    RHS = DefInst->getRvalDDRef();
  }
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
  bool isLegalEdge(const RegDDRef *Ref, const DDEdge &E,
                   const DirectionVector &DV, unsigned Level, bool IsStore);

  template <bool IsOutgoing>
  bool isLegalGraph(const DDGraph &DDG, const HLLoop *Loop, const RegDDRef *Ref,
                    bool IsStore);
  bool isLegalCandidate(const HLLoop *Loop, const MemOpCandidate &Candidate);

  // Analyze and create the transformation candidate for the reference.
  bool analyzeStore(HLLoop *Loop, RegDDRef *Ref, MemOpCandidate &Candidate);

  // Transform \p Candidates into memset calls
  bool processMemset(HLLoop *Loop, bool &ExtractPreheader,
                     MemOpCandidate &Candidate);

  // Transform \p Candidates into memcpy calls
  bool processMemcpy(HLLoop *Loop, bool &ExtractPreheader,
                     MemOpCandidate &Candidate);

  // Helper method to emit memset call.
  bool genMemset(HLLoop *Loop, MemOpCandidate &Candidate, int64_t StoreSize,
                 bool IsNegStride, bool &ExtractPreheader);

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
        M(HIRF.getModule()) {}

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

// Check that it's legal to hoist store/load to the pre-header.
bool HIRIdiomRecognition::isLegalEdge(const RegDDRef *Ref, const DDEdge &E,
                                      const DirectionVector &DV, unsigned Level,
                                      bool IsStore) {
  if (DV.isIndepFromLevel(Level)) {
    return true;
  }

  // No stores should be before the idiom DDRef.
  if (IsStore && E.isOutput()) {
    DVKind Kind = DV[Level - 1];
    return (Kind == DVKind::EQ || Kind == DVKind::LT) && E.getSrc() == Ref;
  }

  assert(!E.isInput() && "Input edges are not expected");

  // No loads should be before the idiom DDRef.
  // Legality table:
  //      | Store | Load  |
  // ANTI |   >   |   <   |
  // FLOW |   <   |   >   |
  DVKind Kind;
  if (E.isAnti() ^ !IsStore) {
    Kind = DVKind::GT;
  } else {
    Kind = DVKind::LT;
  }

  return DV[Level - 1] == Kind;
}

template <bool IsOutgoing>
bool HIRIdiomRecognition::isLegalGraph(const DDGraph &DDG, const HLLoop *Loop,
                                       const RegDDRef *Ref, bool IsStore) {
  unsigned Level = Loop->getNestingLevel();

  auto Range = IsOutgoing ? DDG.outgoing(Ref) : DDG.incoming(Ref);

  for (DDEdge *E : Range) {
    DDRef *OtherRef = IsOutgoing ? E->getSink() : E->getSrc();

    if (RemovedNodes.count(OtherRef->getHLDDNode())) {
      // No dependence if we removed the node
      continue;
    }

    const DirectionVector *DV = &E->getDV();

    // In case of the target loop is not the topmost loop we need to refine the
    // dependency to be less conservative.
    RefinedDependence RefinedDep;
    if (Level != 1) {
      const DDRef *SrcRef = Ref;
      const DDRef *DstRef = OtherRef;
      if (!IsOutgoing) {
        std::swap(SrcRef, DstRef);
      }

      RefinedDep = DDA.refineDV(SrcRef, DstRef, Level, Level, false);
      if (RefinedDep.isIndependent()) {
        continue;
      }

      if (RefinedDep.isRefined()) {
        DV = &RefinedDep.getDV();
      }
    }

    if (!isLegalEdge(Ref, *E, *DV, Level, IsStore)) {
      LLVM_DEBUG(E->dump());
      return false;
    }
  }

  return true;
}

bool HIRIdiomRecognition::isLegalCandidate(const HLLoop *Loop,
                                           const MemOpCandidate &Candidate) {
  LLVM_DEBUG(dbgs() << "R: ");
  DDGraph DDG = DDA.getGraph(Loop);

  if (!isLegalGraph<true>(DDG, Loop, Candidate.StoreRef, true)) {
    return false;
  }

  if (!isLegalGraph<false>(DDG, Loop, Candidate.StoreRef, true)) {
    return false;
  }

  if (Candidate.isMemcopy()) {
    if (!isLegalGraph<true>(DDG, Loop, Candidate.RHS, false)) {
      return false;
    }

    if (!isLegalGraph<false>(DDG, Loop, Candidate.RHS, false)) {
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
                                    bool &ExtractPreheader) {
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
  HNU.insertAsLastPreheaderNode(Loop, MemsetInst);

  LLVM_DEBUG(dbgs() << "G: ");
  LLVM_DEBUG(MemsetInst->dump());
  LLVM_DEBUG(dbgs() << "\n");

  NumMemSet++;

  return true;
}

bool HIRIdiomRecognition::processMemset(HLLoop *Loop, bool &ExtractPreheader,
                                        MemOpCandidate &Candidate) {

  unsigned StoreSize = Candidate.RHS->getDestTypeSizeInBytes();

  if (genMemset(Loop, Candidate, StoreSize, Candidate.IsStoreNegStride,
                ExtractPreheader)) {
    OptReportBuilder &ORBuilder =
        Loop->getHLNodeUtils().getHIRFramework().getORBuilder();

    // Memset generated
    ORBuilder(*Loop).addRemark(OptReportVerbosity::Low, 25408u);
    return true;
  }

  LLVM_DEBUG(dbgs() << "Failed to generate memset for chain:\n");
  LLVM_DEBUG(Candidate.StoreRef->dump());
  LLVM_DEBUG(dbgs() << "\n");

  return false;
}

bool HIRIdiomRecognition::processMemcpy(HLLoop *Loop, bool &ExtractPreheader,
                                        MemOpCandidate &Candidate) {

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
  HNU.insertAsLastPreheaderNode(Loop, MemcpyInst);

  HLNodeUtils::remove(Candidate.DefInst);
  RemovedNodes.insert(Candidate.DefInst);

  LLVM_DEBUG(dbgs() << "G: ");
  LLVM_DEBUG(MemcpyInst->dump());
  LLVM_DEBUG(dbgs() << "\n");

  NumMemCpy++;

  OptReportBuilder &ORBuilder =
      Loop->getHLNodeUtils().getHIRFramework().getORBuilder();

  // Memcopy generated
  ORBuilder(*Loop).addRemark(OptReportVerbosity::Low, 25399u);
  return true;
}

static HLIf *createTripCountCheck(const HLLoop *Loop) {
  if (SmallTripCount == 0 || Loop->isConstTripLoop()) {
    // It's already checked that constant trip count loop is not small.
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
  if (SmallTripCount == 0) {
    return false;
  }

  auto MaxTCEstimate = Loop->getMaxTripCountEstimate();
  if (MaxTCEstimate != 0 && MaxTCEstimate <= SmallTripCount) {
    return true;
  }

  uint64_t ConstTC;
  if (Loop->isConstTripLoop(&ConstTC) && ConstTC <= SmallTripCount) {
    return true;
  }

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
  // HLS->getTotalLoopStatistics()

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
      OrigLoopClone->removePreheader();
      OrigLoopClone->removeZtt();
      OrigLoopClone->setPragmaBasedMaximumTripCount(SmallTripCount);
      OrigLoopClone->setMaxTripCountEstimate(SmallTripCount);
      OrigLoopClone->setLegalMaxTripCount(SmallTripCount);
    }

    LLVM_DEBUG(dbgs() << "Loop DD graph:\n");
    LLVM_DEBUG(DDA.getGraph(Loop).dump());
    LLVM_DEBUG(dbgs() << "\n");
  }

  bool Changed = false;
  for (MemOpCandidate &Candidate : Candidates) {
    LLVM_DEBUG(dbgs() << "A: ");
    LLVM_DEBUG(Candidate.DefInst->dump(true));
    LLVM_DEBUG(dbgs() << "\n");

    if (!isLegalCandidate(Loop, Candidate)) {
      continue;
    }

    if (Candidate.isMemset()) {
      Changed = processMemset(Loop, ExtractPreheader, Candidate) || Changed;
    } else if (Candidate.isMemcopy()) {
      Changed = processMemcpy(Loop, ExtractPreheader, Candidate) || Changed;
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

      // The loop has been multiversioned for the small trip count
      // Multiversioned v1
      ORBuilder(*Loop)
          .addOrigin(25474u, 1)
          .addRemark(OptReportVerbosity::Low, 25562u);

      // Multiversioned v2
      ORBuilder(*OrigLoopClone)
          .addOrigin(25474u, 2);
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
  return false;
}

PreservedAnalyses HIRIdiomRecognitionPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  HIRIdiomRecognition(HIRF, AM.getResult<HIRLoopStatisticsAnalysis>(F),
                      AM.getResult<HIRDDAnalysisPass>(F),
                      AM.getResult<TargetLibraryAnalysis>(F))
      .run();
  return PreservedAnalyses::all();
}

class HIRIdiomRecognitionLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRIdiomRecognitionLegacyPass() : HIRTransformPass(ID) {
    initializeHIRIdiomRecognitionLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<TargetLibraryInfoWrapperPass>();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRIdiomRecognition(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
               getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS(),
               getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
               getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F))
        .run();
  }
};

char HIRIdiomRecognitionLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRIdiomRecognitionLegacyPass, OPT_SWITCH, OPT_DESC,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRIdiomRecognitionLegacyPass, OPT_SWITCH, OPT_DESC, false,
                    false)

FunctionPass *llvm::createHIRIdiomRecognitionPass() {
  return new HIRIdiomRecognitionLegacyPass();
}
