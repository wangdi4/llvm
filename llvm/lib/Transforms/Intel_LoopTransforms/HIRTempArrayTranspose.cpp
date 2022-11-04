//===---------------- HIRTempArrayTranspose.cpp --------------------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// The transformation searches for non-unit stride array uses among multiple
// loopnests and replaces those with a temp array copy with unit-stride access.
// We would replace the B array in the following example:
//
// for (int j = 0; j < N; j++) {
//   sum[j] = 0;
//   for (int k = 0; k < K; k++) {
//     sum[j] += B[k * N + j];
//   }
// }
//
// for (int i = 0; i < M; i++) {
//   for (int j = 0; j < N; j++) {
//     for (int k = 0; k < K; k++) {
//       accumulator += B[k * N + j];
//     }
//   }
// }
//
// to
//
// // define new temp alloca BB
// for (int j = 0; j < N; j++) {
//   for (int k = 0; k < K; k++) {
//     BB[k + K * j] = B[k * N + j];
//   }
// }
//
// for (int j = 0; j < N; j++) {
//   sum[j] = 0;
//   for (int k = 0; k < K; k++) {
//     sum[j] += BB[k + K * j];
//   }
// }
//
// for (int i = 0; i < M; i++) {
//   for (int j = 0; j < N; j++) {
//     for (int k = 0; k < K; k++) {
//       accumulator += BB[k + K * j];
//     }
//   }
// }
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRTempArrayTranspose.h"

#include "llvm/ADT/SparseBitVector.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#define OPT_SWITCH "hir-temp-array-transpose"
#define OPT_DESC "HIR Temp-Array Transpose"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

static cl::opt<unsigned>
    TCThreshold(OPT_SWITCH "-tc",
                cl::desc("minimum profitable TC count required for enabling "
                         "HIR temp array transpose."),
                cl::Hidden, cl::init(8));

class ArrayTransposeAnalyzer {

private:
  struct UseCand {
    // The target ref which we want to transpose
    RegDDRef *UseRef;
    // The delinearlized UseRef which we know matches the loop bounds.
    // Delinearization only handled for 2D refs.
    RegDDRef *DelinearRef;
    // The original Ref's OuterLoop corresponding to a unit-stride dim
    const HLLoop *OrigOuterLoop;
    // The original Ref's InnerLoop corresponding to a non unit-stride dim
    const HLLoop *OrigInnerLoop;

    UseCand(RegDDRef *URef, RegDDRef *DRef, const HLLoop *InnerLp,
            const HLLoop *OuterLp)
        : UseRef(URef), DelinearRef(DRef), OrigOuterLoop(InnerLp),
          OrigInnerLoop(OuterLp) {}
  };

  unsigned InnerDimSize;
  unsigned OuterDimSize;
  SmallVector<UseCand, 4> Uses;
public:

  ArrayTransposeAnalyzer() :
  InnerDimSize(0), OuterDimSize(0) {}

  // TS Nums are set after we finalize all candidates. We could have pruned
  // original candidates due to DDEdges.
  std::pair<unsigned, unsigned> getOuterLoopTSNumbers() {
    unsigned MinTopSortNum = Uses.front().UseRef->getHLDDNode()->getOutermostParentLoop()->getTopSortNum();
    unsigned MaxTopSortNum = Uses.back().UseRef->getHLDDNode()->getOutermostParentLoop()->getTopSortNum();
    return std::make_pair(MinTopSortNum, MaxTopSortNum);
  }

  // Returns true if \p refs are suitable for array transposing.
  // Populates all relevant information.
  bool isValidRefGroup(SmallVector<RegDDRef *, 32> &Refs);

  // Check that loop parents and refstrides are consistent in RefGroup
  bool checkLoopLegality();

  // Remove bad candidates which have illegal DD Edges
  bool checkDDEdges(DDGraph &DDG);

  // Check heuristics to ensure we trigger when transformation is profitable
  bool isProfitable();

  // Returns true if unsafe call is detected in region
  bool hasUnsafeCalls(HIRLoopStatistics &HLS, HLRegion &Reg);

  void doTransformation();

  HLInst *createTempArrayAlloca(UseCand &UseCandidate, HLNode *InsertionPoint);

  // Create the Loopnest where we copy the temparray after \p insertionNode
  HLLoop *createArrayCopyLoop(HLNode *InsertionNode);

  // Used for creating the CE of the transposed Alloca MemRef
  void createTempArrayDims(RegDDRef *ArrayRef, RegDDRef *UseRef,
                           const HLLoop *InnerLoop, bool IVOnly);

  RegDDRef *createTempArrayCopy(UseCand &UseCandidate, HLInst *Alloca,
                                HLLoop *Loop);

  void replaceUsesWithTempArray(HLInst *Alloca);

  unsigned getBasePtrSymbase() { return Uses.front().UseRef->getBasePtrSymbase(); }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    LLVM_DUMP_METHOD void dump() {
      dbgs() << "TRANSPOSE CAND (SB:" << getBasePtrSymbase();
      dbgs() << ")\nUses:\n";
      for (auto Use : Uses) {
        Use.UseRef->dump();
        dbgs() << "\nDelinearized: ";
        Use.DelinearRef->dump();
        dbgs() << "\n";
      }
    }
#endif
};

class HIRTempArrayTranspose {
  HIRFramework &HIRF;
  HIRDDAnalysis &DDA;
  HIRLoopStatistics &HLS;

public:
  HIRTempArrayTranspose(HIRFramework &HIRF, HIRDDAnalysis &DDA,
                        HIRLoopStatistics &HLS)
      : HIRF(HIRF), DDA(DDA), HLS(HLS) {}

  bool run();

private:
  bool runOnRegion(HLRegion &Reg);
};

bool HIRTempArrayTranspose::run() {
  bool Modified = false;
  for (auto &Reg : make_range(HIRF.hir_begin(), HIRF.hir_end())) {
    Modified = runOnRegion(cast<HLRegion>(Reg)) || Modified;
  }
  return Modified;
}

// Initial filter to find refs in region that could be candidates
static bool isTransposeCandidate(const RegDDRef *Ref, unsigned LoopLevel) {
  if (Ref->isLval() || !Ref->isMemRef() || !Ref->hasIV()) {
    return false;
  }

  // For now only 2D memrefs considered, E.g. (%4)[i3][i2]
  // If we handle more than 2D, we would need to consider the dimension
  // index of the extra dim, const/variant/iv, which would require
  // more complex analysis.
  unsigned NumDims = Ref->getNumDimensions();
  if (NumDims > 2) {
    return false;
  }

  for (unsigned Dim = 1; Dim <= NumDims; Dim++) {
    if (!Ref->getTrailingStructOffsets(Dim).empty()) {
      return false;
    }
    unsigned NumElems = Ref->getNumDimensionElements(Dim);
    if (NumElems && NumElems < TCThreshold) {
        return false;
    }
  }

  // For candidate which is single dimension, CE will have 2 IVs and
  // one of them should correspond to innerloop, which is not standalone.
  // Additional checks happen later during delinearization.
  // E.g. (%4)[i2 + %1 * i3]
  if (Ref->isSingleDimension()) {
    const auto *RefCE = Ref->getSingleCanonExpr();
    if (RefCE->numIVs() != 2) {
        return false;
    }

    unsigned Index = InvalidBlobIndex;
    int64_t Coeff = 0;
    RefCE->getIVCoeff(LoopLevel, &Index, &Coeff);
    if (Coeff != 1 && Index == InvalidBlobIndex) {
        return false;
    }
  } else {
    // Inner dimension has a single IV that is not the innerloop IV.
    const CanonExpr *InnerCE = Ref->getDimensionIndex(1);
    if (InnerCE->numIVs() != 1 || InnerCE->getFirstIVLevel() == LoopLevel) {
      return false;
    }

    // We can accept non-standalone iVs iff dimensionsize is known.
    // E.g. (%1)[i4][i3 + %2 + -1] <- Const Dim Size
    if (!Ref->getNumDimensionElements(1) && !InnerCE->isStandAloneIV()) {
      return false;
    }

    const CanonExpr *OuterCE = Ref->getDimensionIndex(2);
    if (OuterCE->numIVs() != 1 || OuterCE->getFirstIVLevel() != LoopLevel) {
      return false;
    }

    if (!Ref->getNumDimensionElements(2) && !OuterCE->isStandAloneIV()) {
      return false;
    }
  }

  return true;
}

// We want to look for potential candidates like (%20)[i2 + %11 * i3].
// i3 corresponds to theinnermost loop and can be swapped with i2 to become
// unit stride. When delinearized, the ref looks like: (%20)[i3][i2].
// Example:
//
//   + DO i1 = 0, %11 + -1, 1
//   |   + DO i2 = 0, zext.i32.i64(%18) + -1, 1
//   |   |   %34 = (%20)[i1 + %11 * i2]; <--- %20 USE
//
//   + DO i1 = 0, %10 + -1, 1
//   |   + DO i2 = 0, %11 + -1, 1
//   |   |   + DO i3 = 0, %18 + -1, 1
//   |   |   |   %45 = (%20)[i2 + ((-1 * %11 * i3];  <--- %20 USE
//
// In the above example, we see i1/i2 in the first loop correspond to i2/i3 of
// the second loop. Delinearization checks the strides of the IV blobcoeff with
// the Loop stride/tc.
bool ArrayTransposeAnalyzer::isValidRefGroup(
    SmallVector<RegDDRef *, 32> &Refs) {
  for (auto *Ref : Refs) {
    if (Ref->isLval()) {
      // TODO: Do not handle any defs/stores for now. It would be possible to
      // handle base ptr assignment, or even stores, if we transpose before
      // all uses, but profitable benchmarks do not have this pattern.
      return false;
    } else {
      if (Ref->isTerminalRef()) {
        LLVM_DEBUG(dbgs() << "[BadCand] Is Terminal:"; Ref->dump(););
        return false;
      }

      RegDDRef *DelinearlizedRef = nullptr;
      // Do delinearization for single dimension refs
      if (Ref->isSingleDimension()) {
        SmallVector<RegDDRef *, 8> DelinearizedRefs;
        if (!DDRefUtils::delinearizeRefs({Ref}, DelinearizedRefs, nullptr,
                                         true)) {
          LLVM_DEBUG(dbgs() << "[BadCand] Delinearization Failed.\n";);
          return false;
        }

        DelinearlizedRef = DelinearizedRefs.front();
      } else {
        // Ref already delinearized
        DelinearlizedRef = Ref;
      }

      unsigned NumDims = DelinearlizedRef->getNumDimensions();
      // Only handle 2D case
      if (NumDims != 2) {
        return false;
      }

      // Check that indices are valid for transposing. The most basic candidate
      // looks like A[i1][i2]. We can handle non-linear blobs like A[i1+%t][i2]
      // if the dimensions of A are known from calling getNumDimensionElements().
      HLLoop *OrigOuterLoop = nullptr;
      HLLoop *OrigInnerLoop = nullptr;
      for (unsigned Dim = 1; Dim <= NumDims; ++Dim) {
        auto *IndexCE = DelinearlizedRef->getDimensionIndex(Dim);
        if (IndexCE->getDenominator() != 1) {
          LLVM_DEBUG(dbgs() << "[BadCand] CE has bad denominator.\n";);
          return false;
        }

        unsigned IVLevel = IndexCE->getFirstIVLevel();
        if (!IVLevel || IndexCE->numIVs() != 1) {
          LLVM_DEBUG(dbgs() << "[BadCand] Index not single IV.\n";);
          return false;
        }

        unsigned IVIndex = InvalidBlobIndex;
        int64_t Coeff = 0;
        IndexCE->getIVCoeff(IVLevel, &IVIndex, &Coeff);

        if (IVIndex != InvalidBlobIndex || Coeff != 1) {
          LLVM_DEBUG(dbgs() << "[BadCand] Complex IV.\n";);
          return false;
        }

        // Check that either NumDimensionElements is known or IV is standalone
        if (!IndexCE->isStandAloneIV(IVLevel) &&
            !DelinearlizedRef->getNumDimensionElements(Dim)) {
          return false;
        }

        // Profitable candidate is non-unit stride for innermost dimension, and
        // has unit stride for outer dimension.
        auto *ParentLoopforIV =
            Ref->getHLDDNode()->getParentLoopAtLevel(IVLevel);
        if (ParentLoopforIV->isInnermost() && (Dim == 1)) {
          LLVM_DEBUG(dbgs() << "[BadCand] Strides not profitable at Dim " << Dim
                            << "\n";);
          return false;
        }

        if (Dim != 1 && !ParentLoopforIV->isInnermost()) {
          LLVM_DEBUG(dbgs() << "[BadCand] No unit stride outer dim found.\n";);
          return false;
        }

        if (!ParentLoopforIV->isNormalized() || !ParentLoopforIV->isDo()) {
          LLVM_DEBUG(dbgs() << "[BadCand] ParentLoops not normalized.\n";);
          return false;
        }

        if (Dim == 1) {
          OrigOuterLoop = ParentLoopforIV;
        } else {
          OrigInnerLoop = ParentLoopforIV;
        }
      }

      // Save this Use and relevant information
      Uses.emplace_back(Ref, DelinearlizedRef, OrigOuterLoop, OrigInnerLoop);
    }
  }

  return true;
}

bool ArrayTransposeAnalyzer::isProfitable() {
  // Check tripcount estimate to not consider candidates if LoopTC are small
  unsigned NumProfitableUses = 0;
  for (auto &Use : Uses) {
    bool MeetsTCThreshold = true;
    const HLLoop *Lp = Use.OrigInnerLoop;
    unsigned Level = Lp->getNestingLevel();

    // Here we try to avoid copying arrays that have very small tripcount
    // and are unlikely to be profitable. Entire loopnest is checked even
    // though we mostly care about the IV levels mostly.
    while (Lp) {
      auto MaxTCEst = Lp->getMaxTripCountEstimate();
      if (MaxTCEst && MaxTCEst < TCThreshold) {
        MeetsTCThreshold = false;
        break;
      }

      uint64_t TC = 0;
      if (Lp->isConstTripLoop(&TC) && TC < TCThreshold) {
        MeetsTCThreshold = false;
        break;
      }

      Lp = Lp->getParentLoop();
    }

    if (MeetsTCThreshold) {
      NumProfitableUses++;
    }

    // Performance heuristic that can be removed if needed. There may be a
    // tradeoff if our array is large enough and does not have many uses, but
    // we don't know what the threshold is for that size/num uses. The following
    // pattern turned out to be unprofitable:
    // (%0))[i4][i1 + (%tmp) + -1];
    // Where the size of the dims was constant (150,150)
    if (NumProfitableUses <= 1 &&
      Use.OrigInnerLoop->getNestingLevel() - Use.OrigOuterLoop->getNestingLevel() > 2) {
      return false;
    }

    // Deem profitable if loopnest is deep and TC is large enough (general case).
    if (MeetsTCThreshold && Level > 2) {
      return true;
    }
  }

  // More uses will likely mean doing transpose will be profitable.
  return NumProfitableUses > 2;
}

static bool areEqualWithSExt(const CanonExpr *CE1, const CanonExpr *CE2) {

  if (CanonExprUtils::areEqual(CE1, CE2, true)) {
    LLVM_DEBUG(dbgs() << "[Equality] CEs proven equal with relaxed mode.\n";
               CE1->dump(); dbgs() << " | "; CE2->dump(); dbgs() << "\n";);
    return true;
  }

  if ((CE1->getConstant() != CE2->getConstant()) ||
      (CE1->getDenominator() != CE2->getDenominator()) ||
      CE1->numBlobs() != 1 || CE2->numBlobs() != 1 || CE1->hasIV() ||
      CE2->hasIV()) {
    return false;
  }

  if (CE1->getDestType() != CE2->getDestType()) {
    return false;
  }

  auto &BU = CE1->getBlobUtils();
  unsigned Index1 = BU.getUnderlyingExtBlobIndex(CE1->getSingleBlobIndex());
  unsigned Index2 = BU.getUnderlyingExtBlobIndex(CE2->getSingleBlobIndex());

  return Index1 == Index2;
}

// We want to return true for normalized do loops with the same UBCE. Note that
// this function is only invoked when CanonExprUtils::areEqual returns false.
// DO i2 = 0, zext.i32.i64(%18) + -1, 1 <LEGAL_MAX_TC = 2147483647>
// DO i3 = 0, sext.i32.i64(%18) + -1, 1 <LEGAL_MAX_TC = 2147483647>
// If we can prove that the UBCEs are non-negative and have the same legal max
// tc, then it should be safe to assume zext(%blob) == sext(%blob).
static bool areEqualLoopsWithExt(const HLLoop *Loop1, const HLLoop *Loop2) {
  if (Loop1->getLegalMaxTripCount() != Loop2->getLegalMaxTripCount()) {
    return false;
  }

  const CanonExpr *UBCE1 = Loop1->getUpperCanonExpr();
  const CanonExpr *UBCE2 = Loop2->getUpperCanonExpr();

  if (!CanonExprUtils::isTypeEqual(UBCE1, UBCE2)) {
    return false;
  }

  if ((UBCE1->getConstant() != UBCE2->getConstant()) ||
      (UBCE1->getDenominator() != UBCE2->getDenominator())) {
    return false;
  }

  // Check the number of blobs.
  if (UBCE1->numBlobs() != 1 || UBCE2->numBlobs() != 1 || UBCE1->hasIV() ||
      UBCE2->hasIV()) {
    return false;
  }

  // Check blob coeff of the single blob. Note if there is ext, we aren't
  // checking the coeff of the underlying blob.
  unsigned Index1 = UBCE1->getSingleBlobIndex();
  unsigned Index2 = UBCE2->getSingleBlobIndex();
  if (UBCE1->getBlobCoeff(Index1) != UBCE2->getBlobCoeff(Index2)) {
    return false;
  }

  if (!HLNodeUtils::isKnownNonNegative(UBCE1, Loop1) ||
      !HLNodeUtils::isKnownNonNegative(UBCE2, Loop2)) {
    return false;
  }

  auto &BU = UBCE1->getBlobUtils();
  unsigned UnderlyingIndex1 = BU.getUnderlyingExtBlobIndex(Index1);
  unsigned UnderlyingIndex2 = BU.getUnderlyingExtBlobIndex(Index2);

  return UnderlyingIndex1 == UnderlyingIndex2;
}

#if 0
// Returns the HLIf that encloses Lp1 that proves equivalency of CE1 and CE2,
// where CE1 and CE2 are the UBCEs of Lp1 and Lp2 respectively. E.g.
//
// if (-1 * ptrtoint.i8*.i64(%24) + ptrtoint.i8*.i64(%23) == %11)
//   DO i1 = 0, sext.i32.i64(%10) + -1,
//     DO i2 = 0, -1 * ptrtoint.i8*.i64(%24) + ptrtoint.i8*.i64(%23) + -1, 1
//
// We want to prove that above i2 loop is equivalent to DO i2 = 0, %11 + -1, 1
static HLIf *findEquivalencePredicate(const HLLoop *Lp1, const HLLoop *Lp2,
                                      const CanonExpr *CE1,
                                      const CanonExpr *CE2) {

  HLNode *Node = const_cast<HLLoop *>(Lp1);

  // Traverse up the parent chain to see if we can find a predicate
  // where CE1 == CE2.
  while (Node->getParent() && !isa<HLRegion>(Node->getParent())) {
    Node = Node->getParent();

    auto *IfNode = dyn_cast<HLIf>(Node);

    if (!IfNode) {
      continue;
    }

    for (auto &Pred : make_range(IfNode->pred_begin(), IfNode->pred_end())) {
      if (!CmpInst::isEquality(Pred)) {
        continue;
      }

      RegDDRef *LHS = IfNode->getLHSPredicateOperandDDRef(&Pred);
      RegDDRef *RHS = IfNode->getRHSPredicateOperandDDRef(&Pred);

      if (!LHS->isSingleDimension() || !RHS->isSingleDimension()) {
        LLVM_DEBUG(dbgs() << "[Unequal] Gave up due to dimension.\n";);
        continue;
      }

      CanonExpr *LHSCE = LHS->getSingleCanonExpr();
      CanonExpr *RHSCE = RHS->getSingleCanonExpr();

      // We want to prove the following 2 CEs are equivalent.
      // LINEAR i64 sext.i32.i64(%11) <- sign extended blob
      // LINEAR sext.i32.i64(%11) {sb:2} <- CE with different src/dst types

      if (areEqualWithSExt(LHSCE, CE1) && areEqualWithSExt(RHSCE, CE2)) {
        LLVM_DEBUG(dbgs() << "[Equal Pred] "; LHS->dump(1); dbgs() << "\t";
                   RHS->dump(1); dbgs() << "\n";);
        return IfNode;
      }

      if (areEqualWithSExt(LHSCE, CE2) && areEqualWithSExt(RHSCE, CE1)) {
        LLVM_DEBUG(dbgs() << "[Equal Pred] "; LHS->dump(1); dbgs() << "\t";
                   RHS->dump(1); dbgs() << "\n";);
        return IfNode;
      }
    }
  }

  return nullptr;
}
#endif

struct UnsafeCallsVisitor : HLNodeVisitorBase {
  HIRLoopStatistics &HLS;
  unsigned MinTSNum;
  unsigned MaxTSNum;
  const HLNode *SkipNode;
  bool HasUnsafeCall;
  bool Done;

  UnsafeCallsVisitor(HIRLoopStatistics &HLS, unsigned MinNum, unsigned MaxNum)
      : HLS(HLS), MinTSNum(MinNum), MaxTSNum(MaxNum), SkipNode(nullptr), HasUnsafeCall(false), Done(false) {}

  void visit(const HLNode *) {}
  void postVisit(const HLNode *) {}
  void visit(const HLLoop *Loop) {
    // Skip those loops before/after of our UseRefs. Ref ParentLoops are checked
    // prior to the visitor.
    if (Loop->getMinTopSortNum() < MinTSNum) {
      SkipNode = Loop;
      return;
    }

    auto LS = HLS.getTotalLoopStatistics(Loop);
    if (LS.hasCallsWithUnsafeSideEffects()) {
      HasUnsafeCall = true;
      LLVM_DEBUG(dbgs() << "[UNSAFE] "; Loop->dump(););
    }

    if (Loop->getMaxTopSortNum() >= MaxTSNum) {
      Done = true;
      return;
    }

    SkipNode = Loop;
  }

  // Traverse only top-level insts
  void visit(const HLInst *Inst) {
    if (Inst->getMinTopSortNum() < MinTSNum) {
      return;
    }

    // if (Inst->getMaxTopSortNum() > MaxTSNum) {
    //   Done = true;
    //   return;
    // }

    if (Inst->isCallInst() && Inst->isUnsafeSideEffectsCallInst()) {
      LLVM_DEBUG(dbgs() << "[UNSAFE] "; Inst->dump(););
      HasUnsafeCall = true;
    }
  }

  bool isDone() const { return Done || HasUnsafeCall; }
};

bool ArrayTransposeAnalyzer::hasUnsafeCalls(HIRLoopStatistics &HLS,
                                                 HLRegion &Reg) {
  auto MinMaxTSNums = getOuterLoopTSNumbers();

  // We check any insts/loops between the saved MaxTS and MinTS for
  // any unsafe calls. We can allow calls that occur outside the range of
  // the first and last reference to our candidate ref.
  UnsafeCallsVisitor USV(HLS, MinMaxTSNums.first, MinMaxTSNums.second);
  HLNodeUtils::visit<true /*Recursive*/, false /*RecurseInsideLoops*/>(USV, &Reg);
  return USV.HasUnsafeCall;
}

// We want to check that all UseRefs have consistent Loop Parents for the
// dimensions that we are planning to transpose. Go through and cross-
// reference all of the Inner/Outer Loops from the Uses.
bool ArrayTransposeAnalyzer::checkLoopLegality() {
  // Check that all Uses have equivalent strides and ParentLoop TCs
  UseCand &FirstUse = Uses.front();
  InnerDimSize = FirstUse.DelinearRef->getNumDimensionElements(1);
  OuterDimSize = FirstUse.DelinearRef->getNumDimensionElements(2);

  // Check that the Inner and OuterLoops are equal among all Uses
  for (auto &Use : make_range(std::next(Uses.begin()), Uses.end())) {
    if (InnerDimSize != Use.DelinearRef->getNumDimensionElements(1) ||
        OuterDimSize != Use.DelinearRef->getNumDimensionElements(2)) {
      return false;
    }

    // If dimsize is known we can skip checking of loop bounds
    if (InnerDimSize != 0 && OuterDimSize != 0) {
      LLVM_DEBUG(dbgs() << "[Legality]: DimSizes are const\n";);
      continue;
    }

    for (unsigned Dim = 1; Dim <= 2; Dim++) {
      const HLLoop *ParentLoop1 =
          Dim == 1 ? FirstUse.OrigOuterLoop : FirstUse.OrigInnerLoop;
      const HLLoop *ParentLoop2 =
          Dim == 1 ? Use.OrigOuterLoop : Use.OrigInnerLoop;

      if (!CanonExprUtils::areEqual(ParentLoop1->getUpperCanonExpr(),
                                    ParentLoop2->getUpperCanonExpr(), true) &&
          !areEqualLoopsWithExt(ParentLoop1, ParentLoop2)) {
        return false;

#if 0
        // This logic is meant to check for if predicates that ensure Loop UB
        // refs are equal. For example, if UB1 is %0 and UB2 is %1, there is an
        // if (%0 == %1), in which case, the loops would be equal. However,
        // the benchmark where we needed this logic was modified.
        CanonExpr *TC1 = ParentLoop1->getTripCountCanonExpr();
        CanonExpr *TC2 = ParentLoop2->getTripCountCanonExpr();

        LLVM_DEBUG(dbgs() << "[CandCheck] Checking CE mismatch.\n1) ";
                   TC1->dump(1); dbgs() << "\n2) "; TC2->dump(1);
                   dbgs() << "\n";);

        // CEs could unequal if there is a parent predicate proving CE
        // equivalence.
        HLIf *EqualIf =
            findEquivalencePredicate(ParentLoop1, ParentLoop2, TC1, TC2);
        if (EqualIf) {
          // If we find the predicate proving equivalence, then it must be in
          // then path.
          if (EqualIf->isThenChild(ParentLoop1)) {
            LLVM_DEBUG(dbgs() << "[Equal CEs] Found CE equivalence 1.\n";);
          } else {
            return false;
          }
        } else {
          // Try again with ParentLoop2
          EqualIf =
              findEquivalencePredicate(ParentLoop2, ParentLoop1, TC1, TC2);
          if (EqualIf) {
            if (EqualIf->isThenChild(ParentLoop2)) {
              LLVM_DEBUG(dbgs() << "[Equal CEs] Found CE equivalence 2.\n";);
            } else {
              return false;
            }
          } else {
            LLVM_DEBUG(dbgs()
                           << "[BadCand1] Could not prove CE equivalence.\n";);
            return false;
          }
        }
        TC1->getCanonExprUtils().destroy(TC1);
        TC2->getCanonExprUtils().destroy(TC2);
#endif
      }
    }
  }

  SmallPtrSet<const HLNode *, 2> IgnoreNodes;

  // Check that the UBRefs can be legally hoisted out; otherwise it is
  // not legal to create the copy loop. Skip if dimsize known.
  if (!InnerDimSize) {
    const HLLoop *InnerLp = FirstUse.OrigInnerLoop;
    const RegDDRef *InnerUBRef = InnerLp->getUpperDDRef();

    bool BadZtt =
        InnerLp->hasZtt() &&
        std::any_of(InnerLp->ztt_ddref_begin(), InnerLp->ztt_ddref_end(),
                    [&](const RegDDRef *Ref) {
                      return !Ref->isStructurallyInvariantAtLevel(1);
                    });

    if (!InnerUBRef->isStructurallyInvariantAtLevel(1) || BadZtt) {
      LLVM_DEBUG(
          dbgs() << "[Illegal] InnerLoop cannot be used for copy loop.\n";);
      return false;
    }

    const CanonExpr *InnerUBCE = InnerUBRef->getSingleCanonExpr();
    if (!InnerUBCE->canConvertToStandAloneBlobOrConstant()) {
      LLVM_DEBUG(dbgs() << "[Illegal] Invalid inner TC arithmetic!\n";);
      return false;
    }

    // Used to check for unconditional execution
    if (InnerLp->hasZtt()) {
      IgnoreNodes.insert(InnerLp);
    }
  }

  if (!OuterDimSize) {
    const HLLoop *OuterLp = FirstUse.OrigOuterLoop;
    const RegDDRef *OuterUBRef = OuterLp->getUpperDDRef();

    bool BadZtt =
        OuterLp->hasZtt() &&
        std::any_of(OuterLp->ztt_ddref_begin(), OuterLp->ztt_ddref_end(),
                    [&](const RegDDRef *Ref) {
                      return !Ref->isStructurallyInvariantAtLevel(1);
                    });

    if (!OuterUBRef->isStructurallyInvariantAtLevel(1) || BadZtt) {
      LLVM_DEBUG(
          dbgs() << "[Illegal] OuterLoop cannot be used for copy loop.\n";);
      return false;
    }

    if (OuterLp->hasZtt()) {
      IgnoreNodes.insert(OuterLp);
    }
  }

  // Check that the program must access the baseptr and/or loop bounds.
  // If normal execution could skip the accesses due to control flow,
  // we should abandon the transformation, as creating the alloca would
  // be illegal.
  if (!InnerDimSize && !OuterDimSize &&
      !FirstUse.UseRef->getHLDDNode()->isUnconditionallyExecutedinRegion(
          IgnoreNodes)) {
    LLVM_DEBUG(dbgs() << "Use may not execute.\n");
    return false;
  }

  return true;
}

bool ArrayTransposeAnalyzer::checkDDEdges(DDGraph &DDG) {
  // If we see an outgoing DDEdge from use, we prune it from our candidates.
  Uses.erase(std::remove_if(Uses.begin(), Uses.end(),
    [&](UseCand &Use) {
      if (DDG.hasIncomingOrOutgoingEdges(Use.UseRef)) {
        LLVM_DEBUG(dbgs() << "Pruning Edge.\n";);
        return true;
      }
      return false;
    }), Uses.end());

  if (Uses.empty()) {
    LLVM_DEBUG(dbgs() << "[DDEdges] No Legal Uses.\n";);
    return false;
  }

  return true;
}

// Create the Loop that copies the original contents of the non-unit stride
// memref and stores it into our temp array alloca memref.
// DO i1
//   DO i2
//     %TranspTmp = (%4)[i2][i1];
//     (%TranspTmpArr)[i1][i2] = %TranspTmp;

// The loops are either based on the original parent loops of our UseRef,
// or the constant dimsizes of the UseRef, if available.
HLLoop *ArrayTransposeAnalyzer::createArrayCopyLoop(HLNode *InsertionNode) {
  auto &DDRU = InsertionNode->getDDRefUtils();
  auto &HNU = InsertionNode->getHLNodeUtils();

  HLLoop *InnerLoop, *OuterLoop;
  Type *Ty;

  if (OuterDimSize) {
    Ty = Uses.front().OrigInnerLoop->getIVType();
    InnerLoop = HNU.createHLLoop(nullptr, DDRU.createConstDDRef(Ty, 0), DDRU.createConstDDRef(Ty, OuterDimSize - 1),
                               DDRU.createConstDDRef(Ty, 1));
  } else {
    InnerLoop = Uses.front().OrigInnerLoop->cloneEmpty();
  }

  if (InnerDimSize) {
    Ty = Uses.front().OrigOuterLoop->getIVType();
    OuterLoop = HNU.createHLLoop(nullptr, DDRU.createConstDDRef(Ty, 0), DDRU.createConstDDRef(Ty, InnerDimSize - 1),
                               DDRU.createConstDDRef(Ty, 1));
  } else {
    OuterLoop = Uses.front().OrigOuterLoop->cloneEmpty();
  }

  HLNodeUtils::insertAfter(InsertionNode, OuterLoop);
  HLNodeUtils::insertAsFirstChild(OuterLoop, InnerLoop);
  return InnerLoop;
}

// Create the alloca and insert it at \p InsertionPoint. \p UseCandidate
// is used as a reference. If we know the dimsizes of the Ref, we use those,
// otherwise we use the Loop Tripcounts.
HLInst *ArrayTransposeAnalyzer::createTempArrayAlloca(UseCand &UseCandidate,
                                                      HLNode *InsertionPoint) {
  RegDDRef *UseRef = UseCandidate.DelinearRef;
  auto &HNU = UseCandidate.UseRef->getParentLoop()->getHLNodeUtils();
  auto &CEU = UseCandidate.UseRef->getCanonExprUtils();
  unsigned ElemSizeinBytes =
      CEU.getTypeSizeInBytes(UseCandidate.UseRef->getDestType());

  HLInst *Alloca;
  RegDDRef *ArraySize;
  // Known dimsizes means we can copy exact sizes
  if (InnerDimSize && OuterDimSize) {
    auto &DDRU = UseCandidate.UseRef->getDDRefUtils();

    // ConstDDRefs must be integer type
    Type *IntType = Type::getInt32Ty(HNU.getContext());
    ArraySize = DDRU.createConstDDRef(IntType, InnerDimSize * OuterDimSize *
                                                   ElemSizeinBytes);
  } else {
    ArraySize = UseCandidate.OrigOuterLoop->getTripCountDDRef();
    assert(ArraySize->isSingleDimension() && "TCRef is not single CE!\n");
    CanonExpr *ArraySizeCE = ArraySize->getSingleCanonExpr();

    // TCCE is the IV Stride for the outer IV. This corresponds to the original
    // UseCandidate's OrigInnerLoop TripCountCE.
    CanonExpr *InnerTCCE = const_cast<CanonExpr *>(
        UseCandidate.OrigInnerLoop->getTripCountCanonExpr());
    unsigned TCIndex;
    int64_t ConstVal = 0;
    if (InnerTCCE->isIntConstant(&ConstVal)) {
    } else if (InnerTCCE->isSelfBlob()) {
      TCIndex = InnerTCCE->getSingleBlobIndex();
    } else {
      InnerTCCE->convertToStandAloneBlobOrConstant();
      TCIndex = InnerTCCE->getSingleBlobIndex();
    }

    if (ConstVal) {
      ArraySizeCE->multiplyByConstant(ConstVal);
    } else {
      ArraySizeCE->multiplyByBlob(TCIndex);
    }

    // Lastly multiply by the size of the element
    ArraySizeCE->multiplyByConstant(ElemSizeinBytes);

    SmallVector<const RegDDRef *, 2> UpperBoundRefs;
    UpperBoundRefs.push_back(UseCandidate.OrigOuterLoop->getUpperDDRef());
    UpperBoundRefs.push_back(UseCandidate.OrigInnerLoop->getUpperDDRef());
    ArraySize->makeConsistent(UpperBoundRefs, 0);
    // Note getTripCountCanonExpr() cloned the original CE, so we can cleanup.
    InnerTCCE->getCanonExprUtils().destroy(InnerTCCE);
  }

  Alloca = HNU.createAlloca(UseRef->getDestType(), ArraySize,
                            "TranspTmpArr");

  HLNodeUtils::insertBefore(InsertionPoint, Alloca);

  LLVM_DEBUG(dbgs() << "TempArrayAlloca:"; Alloca->dump(1););

  return Alloca;
}

// Computes and adds the dimensions for the TempArrayRef that will replace the
// original uses for the ref. Uses the Parent InnerLoop of the ref to compute
// the correct IVs. Example: For the original use of %4 which might be used in
// an i3 loop, we want to create (%TranspTmpArr)[i1][i2]
//    + DO i1 = 0, %1 - 1, 1
//    |   + DO i2 = 0, %2 - 1, 1
//    |   |   %T = (%4)[0:i2:4 * %1(i32*:0)][0:i1:4(i32*:0)];
//    |   |   (%TranspTmpArr)[0:i1:4 * %2(i32*:0)][0:i2:4(i32*:0)] = %T;
//
// Orig Ref: (%4)[i2][i1] has stride of 4 * %1 corresponding to i1 which is
// the inner dimsize. New Ref outer dim looks like [0:i1:4 * %2(i32*:0)]
// corresponding to the new i2 inner dimsize. InnerDim size is just typesize.
// Uses may be at a different IV level than what we want in our copy loop.
// We pass \p InnerLoop to normalize the IV to the level that we expect to
// place our AllocaRef at.
// The flag \p IVOnly is used to indicate that our dims should be standaloneIVs.
// For existing refs being replaced, the CEs can potentially have constants or
// blobs, in the case where the dimensionsizes are known.
void ArrayTransposeAnalyzer::createTempArrayDims(RegDDRef *ArrayRef,
                                                 RegDDRef *UseRef,
                                                 const HLLoop *InnerLoop,
                                                 bool IVOnly = false) {
  auto &CEU = UseRef->getCanonExprUtils();
  auto *OuterStrideCE = UseRef->getDimensionStride(1)->clone();

  // Note Inner/Outer Dimsizes correspond to original UseRef
  if (OuterDimSize) {
    OuterStrideCE->multiplyByConstant(OuterDimSize);
  } else {
    // If dimsize is not available we use LoopTC. LoopTC was checked previously
    // in checkLegality(). Set Outer dimstride as the new InnerLoopTC.
    uint64_t LoopTC = 0;
    if (InnerLoop->isConstTripLoop(&LoopTC)) {
      OuterStrideCE->multiplyByConstant(LoopTC);
    } else {
      OuterStrideCE->multiplyByBlob(
          InnerLoop->getUpperCanonExpr()->getSingleBlobIndex());
    }
  }

  unsigned InnerLevel = InnerLoop->getNestingLevel();
  CanonExpr *OuterCE;
  if (IVOnly) {
    OuterCE = CEU.createCanonExpr(InnerLoop->getParentLoop()->getIVType());
    OuterCE->addIV(InnerLevel - 1, 0, 1);
  } else {
    OuterCE = UseRef->getDimensionIndex(1)->clone();
    OuterCE->replaceIV(InnerLevel, InnerLevel - 1);
  }

  ArrayRef->addDimension(OuterCE, {}, nullptr, OuterStrideCE,
                         UseRef->getDimensionType(2),
                         UseRef->getDimensionElementType(2));

  auto *InnerStrideCE = UseRef->getDimensionStride(1)->clone();

  CanonExpr *InnerCE;
  if (IVOnly) {
    InnerCE = CEU.createCanonExpr(InnerLoop->getIVType());
    InnerCE->addIV(InnerLevel, 0, 1);
  } else {
    InnerCE = UseRef->getDimensionIndex(2)->clone();
    InnerCE->replaceIV(InnerLevel - 1, InnerLevel);
  }

  ArrayRef->addDimension(InnerCE, {}, nullptr, InnerStrideCE,
                         UseRef->getDimensionType(1),
                         UseRef->getDimensionElementType(1));
}

// Construct the logic that copies the old array contents into our new
// \p Alloca memref. \p UseCandidate is used as a reference for
// constructing the loopnest and for makeConsistent();
// Simple example of our old ref and new alloca ref:
//          OldRef: (%4)[0:i2:4 * %1(i32*:0)][0:i1:4(i32*:0)]
//          NewRef: (%5)[0:i1:4 * %2(i32*:0)][0:i2:4(i32*:0)]
RegDDRef *ArrayTransposeAnalyzer::createTempArrayCopy(UseCand &UseCandidate,
                                                      HLInst *Alloca,
                                                      HLLoop *InnerLoop) {
  RegDDRef *UseRef = UseCandidate.DelinearRef;
  auto &HNU = InnerLoop->getHLNodeUtils();
  unsigned InnerLevel = InnerLoop->getNestingLevel();

  RegDDRef *UseClone = UseRef->clone();
  unsigned UseInnerLevel = UseCandidate.OrigInnerLoop->getNestingLevel();
  // If the Old Ref was at a different level, normalize it. For example,
  // OldRef could be (%4)[i4][i3] but our copy loop will always be i1/i2
  if (UseInnerLevel != InnerLevel) {
    CanonExpr *CEIndex;
    CEIndex = UseClone->getDimensionIndex(2);
    CEIndex->replaceIV(UseInnerLevel, InnerLevel);
    CEIndex = UseClone->getDimensionIndex(1);
    CEIndex->replaceIV(UseInnerLevel - 1, InnerLevel - 1);
  }

  if (InnerDimSize) {
    UseClone->getDimensionIndex(2)->clearBlobs();
    UseClone->getDimensionIndex(2)->setConstant(0);
  }

  if (OuterDimSize) {
    UseClone->getDimensionIndex(1)->clearBlobs();
    UseClone->getDimensionIndex(1)->setConstant(0);
  }

  RegDDRef *ArrayRef = HNU.getDDRefUtils().createMemRef(
      cast<AllocaInst>(Alloca->getLLVMInstruction())->getAllocatedType(),
      Alloca->getLvalDDRef()->getSingleCanonExpr()->getSingleBlobIndex(),
      0);

  // Add each dimension for the TempArrayCopy MemRef
  createTempArrayDims(ArrayRef, UseRef, InnerLoop, true);

  HLInst *StoreInst =
      HNU.createStore(UseClone, ".transpst", ArrayRef);
  HLNodeUtils::insertAsLastChild(InnerLoop, StoreInst);

  SmallVector<const RegDDRef *, 4> AuxRefs;
  AuxRefs.push_back(UseRef);
  AuxRefs.push_back(InnerLoop->getUpperDDRef());
  ArrayRef->makeConsistent(AuxRefs);
  UseClone->makeConsistent();

  InnerLoop->addLiveInTemp(ArrayRef);
  InnerLoop->addLiveInTemp(UseClone);
  InnerLoop->getParentLoop()->addLiveInTemp(ArrayRef);
  InnerLoop->getParentLoop()->addLiveInTemp(UseClone);

  LLVM_DEBUG(dbgs() << "[Transformation] Transpose Loop: \n";
             InnerLoop->getOutermostParentLoop()->dump(1););
  return ArrayRef;
}

// Replace live-in symbases after we replaced the old ref with the new
// alloca ref.
static void updateLiveInTemp(HLLoop *InnerLoop, unsigned OldSB,
                             unsigned NewSB) {
  HLLoop *Lp = InnerLoop;
  while (Lp) {
    if (Lp->isLiveIn(OldSB)) {
      // TODO: accurately updated livein/liveouts.
      // The transformation ignores addressof refs, which need
      // to return live-in information.
      // Lp->removeLiveInTemp(OldSB);
      Lp->addLiveInTemp(NewSB);
    }
    Lp = Lp->getParentLoop();
  }
}

// Replace all uses of the old non-unit stride ref with our new unit-
// stride alloca \p TempRef.
void ArrayTransposeAnalyzer::replaceUsesWithTempArray(HLInst *Alloca) {
  LLVM_DEBUG(dbgs() << "[Transformation] Replacing " << Uses.size()
                    << " Uses with TempArray\n";);
  for (auto &Use : Uses) {
    RegDDRef *OrigUse = Use.UseRef;

    LLVM_DEBUG(dbgs() << "[Transformation] Replacing Use:\n";
               OrigUse->getHLDDNode()->dump(););
    RegDDRef *ArrayRef = OrigUse->getDDRefUtils().createMemRef(
        cast<AllocaInst>(Alloca->getLLVMInstruction())->getAllocatedType(),
        Alloca->getLvalDDRef()->getSingleCanonExpr()->getSingleBlobIndex(),
        Alloca->getNodeLevel());
    createTempArrayDims(ArrayRef, Use.DelinearRef, Use.OrigInnerLoop);

    HIRTransformUtils::replaceOperand(OrigUse, ArrayRef);

    SmallVector<const RegDDRef *, 4> AuxRefs;
    AuxRefs.push_back(OrigUse);
    AuxRefs.push_back(Use.OrigInnerLoop->getUpperDDRef());
    ArrayRef->makeConsistent(AuxRefs);

    // New alloca should be set live-in to the loops up to the target loop.
    updateLiveInTemp(ArrayRef->getHLDDNode()->getParentLoop(),
                     OrigUse->getBasePtrSymbase(),
                     ArrayRef->getBasePtrSymbase());
  }
}

void ArrayTransposeAnalyzer::doTransformation() {
  // Insert our alloca and transpose loop at the Region level before the first use.
  UseCand *Candidate = &Uses.front();
  HLRegion *Region = Candidate->OrigInnerLoop->getParentRegion();
  assert(Region && "Region must be defined!");
  HLNode *InsertNode = HLNodeUtils::getImmediateChildContainingNode(Region, Candidate->UseRef->getHLDDNode());

  assert(InsertNode && "Insertion Node is null!\n");
  LLVM_DEBUG(
      dbgs() << "[Transformation] Insertion point for TempArray Transpose: "
             << InsertNode->getNumber() << "\n";);

  auto &HNU = InsertNode->getHLNodeUtils();
  HLInst *StacksaveCall = HNU.createStacksave(InsertNode->getDebugLoc());
  HLNodeUtils::insertBefore(InsertNode, StacksaveCall);

  // Create alloca and insert it before the first UseLoop
  HLInst *AllocaInst = createTempArrayAlloca(*Candidate, InsertNode);

  // Create 2 level loopnest where we will copy the original array into the
  // allocated temp array. Then populate the loop with the load/store insts.
  // We insert the loop after the \p AllocaInst.
  HLLoop *InnerLoop = createArrayCopyLoop(AllocaInst);
  createTempArrayCopy(*Candidate, AllocaInst, InnerLoop);

  // Get last use before we do replacement
  UseCand *LastCand = &Uses.back();
  HLNode *LastNode = HLNodeUtils::getImmediateChildContainingNode(Region, LastCand->UseRef->getHLDDNode());

  // Replace the uses of the bad array with our unit stride temp array
  replaceUsesWithTempArray(AllocaInst);

  auto &CEU = InnerLoop->getCanonExprUtils();
  auto &DDRU = InnerLoop->getDDRefUtils();
  auto Int8Ty = Type::getInt8Ty(CEU.getContext());

  RegDDRef *StackSaveOp = StacksaveCall->getOperandDDRef(0);
  RegDDRef *StackAddrRef = DDRU.createAddressOfRef(
      Int8Ty, StackSaveOp->getSelfBlobIndex(), StackSaveOp->getDefinedAtLevel(),
      StackSaveOp->getSymbase(), true);
  StackAddrRef->addDimension(CEU.createCanonExpr(Int8Ty, APInt(8, 0)));

  HLInst *StackrestoreCall = HNU.createStackrestore(StackAddrRef);
  HLNodeUtils::insertAfter(LastNode, StackrestoreCall);
}

bool HIRTempArrayTranspose::runOnRegion(HLRegion &Reg) {
  SmallVector<HLLoop *, 12> InnermostLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(InnermostLoops, &Reg);

  SmallVector<RegDDRef *, 32> TransposeCandidates;
  bool Modified = false;
  SparseBitVector<> CandidateIndices;

  // Gather all transpose candidates from innerloops here. This is mainly to
  // reduce compile time since profitable candidates must be in innerloopnests.
  for (auto *Loop : InnermostLoops) {
    unsigned LoopLevel = Loop->getNestingLevel();
    // Heuristic: Seems like most profitable loops are not large.
    // Cut down on compile time (DDG) by not considering large loops.
    if (!Loop->isDo() || !Loop->isNormalized() || LoopLevel < 3 ||
        Loop->getNumChildren() > 10) {
      continue;
    }

    DDRefGathererLambda<RegDDRef>::gather(Loop, TransposeCandidates,
                                          std::bind(isTransposeCandidate, std::placeholders::_1, LoopLevel));

    for (auto *Ref : TransposeCandidates) {
      assert(Ref->hasGEPInfo() && "Candidate missing GEPInfo.\n");
      CandidateIndices.set(Ref->getBasePtrSymbase());
    }
  }

  for (auto SB : CandidateIndices) {
    LLVM_DEBUG(dbgs() << "Cand SB: " << SB << "\n";);
    SmallVector<RegDDRef *, 32> Refs;

    // Scan Region for TransposeCandidate using Symbase or BasePtrSymbase
    DDRefGathererLambda<RegDDRef>::gather(&Reg, Refs, [&](const RegDDRef *R) {
      if (R->hasGEPInfo()) {
        if (R->isAddressOf()) {
          return false;
        }

        return (R->getBasePtrSymbase() == SB);
      } else {
        return (R->getSymbase() == SB);
      }
    });

    assert(!Refs.empty() && "No refs with Candidate Symbase!");

    ArrayTransposeAnalyzer ATA;
    if (!ATA.isValidRefGroup(Refs)) {
      continue;
    }

    DDGraph DDG = DDA.getGraph(&Reg);
    if (!ATA.checkDDEdges(DDG)) {
      continue;
    }

    if (!ATA.isProfitable()) {
      LLVM_DEBUG(dbgs() << "[BadCand] Not Profitable.\n");
      continue;
    }

    if (ATA.hasUnsafeCalls(HLS, Reg)) {
      LLVM_DEBUG(dbgs() << "[Illegal] Found Unsafe Call in Region.\n");
      continue;
    }

    if (!ATA.checkLoopLegality()) {
      continue;
    }

    LLVM_DEBUG(ATA.dump(););
    LLVM_DEBUG(dbgs() << "Doing Transformation.\n");

    ATA.doTransformation();

    Modified = true;
    LLVM_DEBUG(dbgs() << "Region After Transpose:\n"; Reg.dump(););
  }

  // Invalidate region at the end as we modified the DD for only transposed
  // arrays which are cross loop
  if (Modified) {
    HIRInvalidationUtils::invalidateNonLoopRegion(&Reg);
    Reg.setGenCode();
  }

  return Modified;
}

PreservedAnalyses
HIRTempArrayTransposePass::runImpl(Function &F, FunctionAnalysisManager &AM,
                                   HIRFramework &HIRF) {
  HIRTempArrayTranspose(HIRF, AM.getResult<HIRDDAnalysisPass>(F),
                        AM.getResult<HIRLoopStatisticsAnalysis>(F))
      .run();
  return PreservedAnalyses::all();
}
