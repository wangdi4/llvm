//===- HIROptVarPredicate.cpp - Optimization of predicates containing IVs -===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// The transformation splits iteration spaces of the loop, based on the
// *IF* statements.
//
// Each predicate splits the loop so the predicate could be removed.
// The split point could be non-constant and this should be taken into
// consideration.
//
// for (int i = 0, i < %UB, ++i) {
//  if (i < %b) {
//    A;
//  } else {
//    B;
//  }
// }
//                    A                 B
//     |  0 |---------------------|-----------| %UB   |
//     ^                          ^                   ^
//     %b                         %b                  %b
//
// The split point %b could be either
//  1) less than LB
//  2) between LB, UB
//  3) greater than UB
//
// for i = 0, min(%b - 1, %UB)       ztt: %b > 0
//  A
// for i = max(%b, 0), %UB           ztt: %b <= %UB
//  B
//
//===----------------------------------------------------------------------===//
//
// Equal predicate creates another case for the transformation.
//
// for i = 0, %UB
//  if (i == %b) A else B
//
//
//
// for i = 0, min(%b - 1, %UB)       ztt: %b > 0
//   B
// for i = max(%b, 0), min(%b, %UB)  ztt: max(%b, 0) = min(%b, N)
//   A
// for i = max(%b + 1, 0), %UB       ztt: %b + 1 <= N
//   B
//
//===----------------------------------------------------------------------===//
//
// TODO:
//  1) Try explicit ZTTs
//  2) Set NSW flags
//  3) Support multiple ifs statements at once
//  4) Replace known blobs with its max or min values
//  5) Support multiple predicates within single HLIf
//  6) Handle constant cases like (i + 1 < 10). In current setup IV CE should be
//     a "self-IV".
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIROptVarPredicate.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNodeMapper.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#define OPT_SWITCH "hir-opt-var-predicate"
#define OPT_DESC "HIR Var OptPredicate"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

static cl::opt<bool>
    DisableCostModel("disable-" OPT_SWITCH "-cost-model", cl::init(false),
                     cl::Hidden, cl::desc("Disable " OPT_DESC " cost model"));

static cl::list<unsigned>
    TransformNodes(OPT_SWITCH "-nodes", cl::Hidden,
                   cl::desc("List nodes to transform by " OPT_DESC));

STATISTIC(LoopsSplit, "Loops split during optimization of predicates.");

namespace {

class HIROptVarPredicate {
  HIRFramework &HIRF;
  BlobUtils &BU;

  SmallPtrSet<HLNode *, 8> NodesToInvalidate;

public:
  HIROptVarPredicate(HIRFramework &HIRF)
      : HIRF(HIRF), BU(HIRF.getBlobUtils()) {}

  bool run();

private:
  static std::unique_ptr<CanonExpr>
  findIVSolution(Type *IVType, const RegDDRef *LHSDDref, PredicateTy Pred,
                 const RegDDRef *RHSDDRef, unsigned Level,
                 bool &ShouldInvertCondition);

  void splitLoop(HLLoop *Loop, HLIf *Candidate, const RegDDRef *LHS,
                 PredicateTy Pred, const RegDDRef *RHS,
                 const CanonExpr *LowerCE, const CanonExpr *UpperCE,
                 const CanonExpr *SplitPoint, bool ShouldInvertCondition);

  bool processLoop(HLLoop *Loop);

  BlobTy castBlob(BlobTy Blob, Type *DesiredType, bool IsSigned,
                  unsigned &BlobIndex);

  void setSelfBlobDDRef(RegDDRef *Ref, BlobTy Blob, unsigned BlobIndex);

  void makeBlobsTypeConsistent(BlobTy &BlobA, BlobTy &BlobB, bool IsSigned);

  void setSelfBlobDDRef(BlobUtils &BlobUtilsObj, RegDDRef *Ref, BlobTy Blob,
                        unsigned BlobIndex);

  void updateLoopUpperBound(HLLoop *Loop, BlobTy UpperBlob,
                            BlobTy SplitPointBlob, bool IsSigned);

  void updateLoopLowerBound(HLLoop *Loop, BlobTy LowerBlob,
                            BlobTy SplitPointBlob, bool IsSigned);

  void addVarPredicateReport(HLIf *If, HLLoop *Loop,
                             LoopOptReportBuilder &LORBuilder);
};
} // namespace

class IfLookup final : public HLNodeVisitorBase {
  SmallVectorImpl<HLIf *> &Candidates;
  unsigned Level;
  const HLNode *SkipNode;

  bool HasLabel;

public:
  IfLookup(SmallVectorImpl<HLIf *> &Candidates, unsigned Level)
      : Candidates(Candidates), Level(Level), SkipNode(nullptr),
        HasLabel(false) {}

  bool isCandidateRef(const RegDDRef *Ref, bool *HasIV) const {
    // Only handle scalar references.
    if (!Ref->isTerminalRef()) {
      return false;
    }

    const CanonExpr *CE = Ref->getSingleCanonExpr();
    if (CE->isNonLinear() || CE->getDefinedAtLevel() >= Level) {
      return false;
    }

    *HasIV = CE->hasIV(Level);
    return true;
  }

  void visit(const HLLabel *Goto) { HasLabel = true; }

  void visit(HLIf *If) {
    SkipNode = If;

    assert(If->getParentLoop() && "Parent should exist");

    IfLookup Lookup(Candidates, Level);
    HLNodeUtils::visitRange(Lookup, If->then_begin(), If->then_end());
    HLNodeUtils::visitRange(Lookup, If->else_begin(), If->else_end());

    if (Lookup.HasLabel) {
      return;
    }

    // Loop through predicates to check if they satisfy opt predicate
    // conditions.
    for (auto Iter = If->pred_begin(), E = If->pred_end(); Iter != E; ++Iter) {
      const RegDDRef *LHSRef = If->getPredicateOperandDDRef(Iter, true);
      const RegDDRef *RHSRef = If->getPredicateOperandDDRef(Iter, false);

      bool LHSIV;
      bool RHSIV;

      // Check if both DDRefs satisfy all the conditions.
      if (!isCandidateRef(LHSRef, &LHSIV) || !isCandidateRef(RHSRef, &RHSIV)) {
        return;
      }

      if ((LHSIV && RHSIV) || (!LHSIV && !RHSIV)) {
        return;
      }
    }

    Candidates.push_back(If);
  }

  void visit(const HLIf *If) { llvm_unreachable("Unexpected const HLIf."); }

  void visit(HLLoop *Loop) {
    SkipNode = Loop;
    IfLookup Lookup(Candidates, Level);
    HLNodeUtils::visitRange(Lookup, Loop->child_begin(), Loop->child_end());
  }

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

  bool skipRecursion(const HLNode *Node) const { return SkipNode == Node; }
};

static bool hasIVAndConstOnly(const CanonExpr *CE, unsigned Level) {
  bool OneIVAndConstant = ((CE->getDenominator() == 1) &&
                           (CE->numBlobs() == 0) && (CE->numIVs() == 1));

  if (!OneIVAndConstant) {
    return false;
  }

  unsigned Index;
  int64_t Coeff;
  CE->getIVCoeff(Level, &Index, &Coeff);

  return (Coeff == 1 || Coeff == -1) && (Index == InvalidBlobIndex);
}

static bool mayIVOverflowCE(const CanonExpr *CE, Type *IVType) {
  unsigned Width = IVType->getPrimitiveSizeInBits();

  if (CE->getSrcType()->getPrimitiveSizeInBits() < Width ||
      CE->getDestType()->getPrimitiveSizeInBits() < Width) {
    return true;
  }

  return false;
}

std::unique_ptr<CanonExpr> HIROptVarPredicate::findIVSolution(
    Type *IVType, const RegDDRef *LHSDDref, PredicateTy Pred,
    const RegDDRef *RHSDDRef, unsigned Level, bool &ShouldInvertCondition) {

  assert(LHSDDref->isTerminalRef() && RHSDDRef->isTerminalRef() &&
         "Candidate If should contain only terminal references");

  const CanonExpr *LHS = LHSDDref->getSingleCanonExpr();
  const CanonExpr *RHS = RHSDDRef->getSingleCanonExpr();

  if (hasIVAndConstOnly(RHS, Level)) {
    std::swap(LHS, RHS);
    Pred = CmpInst::getSwappedPredicate(Pred);
  } else if (!hasIVAndConstOnly(LHS, Level)) {
    return nullptr;
  }

  if (CmpInst::isUnsigned(Pred) || mayIVOverflowCE(LHS, IVType)) {
    return nullptr;
  }

  // Assuming that LHS is 1*IV
  std::unique_ptr<CanonExpr> Result(RHS->clone());

  int64_t LHSConst = LHS->getConstant();
  if (LHSConst != 0) {
    int64_t RHSConst;
    if (!RHS->isIntConstant(&RHSConst)) {
      return nullptr;
    }

    Type *RHSType = RHS->getSrcType();
    Type *LHSType = LHS->getSrcType();

    if (!ConstantInt::isValueValidForType(RHSType, RHSConst) ||
        !ConstantInt::isValueValidForType(LHSType, -LHSConst)) {
      return nullptr;
    }

    bool Overflow;
    APInt RHSConstAP(RHSType->getPrimitiveSizeInBits(), RHSConst, true);
    APInt LHSConstAP(LHSType->getPrimitiveSizeInBits(), -LHSConst, true);
    (void) RHSConstAP.sadd_ov(LHSConstAP, Overflow);

    if (Overflow) {
      return nullptr;
    }

    Result->addConstant(-LHSConst, true);
  }

  int64_t Coeff = LHS->getIVConstCoeff(Level);
  if (Coeff == -1) {
    Pred = CmpInst::getSwappedPredicate(Pred);
    Result->negate();
  }

  int64_t Shift = 0;

  switch (Pred) {
  case PredicateTy::ICMP_NE: // !=
    break;
  case PredicateTy::ICMP_EQ: // ==
    ShouldInvertCondition = !ShouldInvertCondition;
    break;
  case PredicateTy::ICMP_SGE: // >=
  case PredicateTy::ICMP_UGE:
    ShouldInvertCondition = !ShouldInvertCondition;
    break;
  case PredicateTy::ICMP_SGT: // >
  case PredicateTy::ICMP_UGT:
    ShouldInvertCondition = !ShouldInvertCondition;
    Shift = 1;
    break;
  case PredicateTy::ICMP_SLE: // <=
  case PredicateTy::ICMP_ULE:
    Shift = 1;
    break;
  case PredicateTy::ICMP_SLT: // <
  case PredicateTy::ICMP_ULT:
    break;
  default:
    llvm_unreachable("Unhandled predicate");
  }

  Result->addConstant(Shift, true);

  // 1*IV < RHS + Shift

  return std::move(Result);
}

bool HIROptVarPredicate::run() {
  if (DisablePass) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "Optimization of Variant Predicates Function: "
                    << HIRF.getFunction().getName() << "\n");

  ForPostEach<HLLoop>::visitRange(HIRF.hir_begin(), HIRF.hir_end(),
                                  [this](HLLoop *Loop) { processLoop(Loop); });

  for (HLNode *Node : NodesToInvalidate) {
    if (HLLoop *Loop = dyn_cast<HLLoop>(Node)) {
      assert(Loop->isAttached() && "Every invalidated loop should be attached");
      HIRInvalidationUtils::invalidateBody(Loop);
    } else {
      HIRInvalidationUtils::invalidateNonLoopRegion(cast<HLRegion>(Node));
    }
    HLNodeUtils::removeRedundantNodes(Node, false);
    // TODO: update exits for multiexit loops
    // HLNodeUtils::updateNumLoopExits(Node);
  }

  return false;
}

static void removeThenElseChildren(HLIf *If, HLContainerTy *ThenContainer,
                                   HLContainerTy *ElseContainer) {
  HLNodeUtils::remove(ThenContainer, If->then_begin(), If->then_end());
  HLNodeUtils::remove(ElseContainer, If->else_begin(), If->else_end());
}

BlobTy HIROptVarPredicate::castBlob(BlobTy Blob, Type *DesiredType,
                                    bool IsSigned, unsigned &BlobIndex) {
  if (Blob->getType() == DesiredType) {
    return Blob;
  }

  return BU.createCastBlob(Blob, IsSigned, DesiredType,
                           !isa<SCEVConstant>(Blob), &BlobIndex);
}

void HIROptVarPredicate::setSelfBlobDDRef(RegDDRef *Ref, BlobTy Blob,
                                          unsigned BlobIndex) {
  CanonExpr *CE = Ref->getSingleCanonExpr();
  CE->clear();

  int64_t Value;
  if (BlobUtils::isConstantIntBlob(Blob, &Value)) {
    CE->setConstant(Value);
    Ref->setSymbase(ConstantSymbase);
  } else {
    CE->setBlobCoeff(BlobIndex, 1);

    if (BlobUtils::isTempBlob(Blob)) {
      Ref->setSymbase(BU.findTempBlobSymbase(Blob));
    } else {
      Ref->setSymbase(GenericRvalSymbase);
    }
  }
}

void HIROptVarPredicate::makeBlobsTypeConsistent(BlobTy &BlobA, BlobTy &BlobB,
                                                 bool IsSigned) {
  Type *TypeA = BlobA->getType();
  Type *TypeB = BlobB->getType();

  if (TypeA == TypeB) {
    return;
  }

  Type *BiggerType = TypeB;

  if (TypeA->getPrimitiveSizeInBits() > TypeB->getPrimitiveSizeInBits()) {
    std::swap(BlobA, BlobB);
    BiggerType = TypeA;
  }

  unsigned BlobIndex;
  if (IsSigned) {
    BlobA = BU.createSignExtendBlob(BlobA, BiggerType, true, &BlobIndex);
  } else {
    BlobA = BU.createZeroExtendBlob(BlobA, BiggerType, true, &BlobIndex);
  }
}

void HIROptVarPredicate::updateLoopUpperBound(HLLoop *Loop, BlobTy UpperBlob,
                                              BlobTy SplitPointBlob,
                                              bool IsSigned) {
  makeBlobsTypeConsistent(UpperBlob, SplitPointBlob, IsSigned);

  unsigned MinBlobIndex;
  BlobTy MinBlob;
  bool BlobsAreConst =
      isa<SCEVConstant>(UpperBlob) && isa<SCEVConstant>(SplitPointBlob);

  if (IsSigned) {
    MinBlob = BU.createSMinBlob(SplitPointBlob, UpperBlob, !BlobsAreConst,
                                &MinBlobIndex);
  } else {
    MinBlob = BU.createUMinBlob(SplitPointBlob, UpperBlob, !BlobsAreConst,
                                &MinBlobIndex);
  }

  MinBlob = castBlob(MinBlob, Loop->getIVType(), IsSigned, MinBlobIndex);

  setSelfBlobDDRef(Loop->getUpperDDRef(), MinBlob, MinBlobIndex);
}

void HIROptVarPredicate::updateLoopLowerBound(HLLoop *Loop, BlobTy LowerBlob,
                                              BlobTy SplitPointBlob,
                                              bool IsSigned) {
  makeBlobsTypeConsistent(LowerBlob, SplitPointBlob, IsSigned);

  unsigned MaxBlobIndex;
  BlobTy MaxBlob;
  bool BlobsAreConst =
      isa<SCEVConstant>(LowerBlob) && isa<SCEVConstant>(SplitPointBlob);

  if (IsSigned) {
    MaxBlob = BU.createSMaxBlob(SplitPointBlob, LowerBlob, !BlobsAreConst,
                                &MaxBlobIndex);
  } else {
    MaxBlob = BU.createUMaxBlob(SplitPointBlob, LowerBlob, !BlobsAreConst,
                                &MaxBlobIndex);
  }

  MaxBlob = castBlob(MaxBlob, Loop->getIVType(), IsSigned, MaxBlobIndex);

  setSelfBlobDDRef(Loop->getLowerDDRef(), MaxBlob, MaxBlobIndex);
}

static bool isLoopRedundant(HLLoop *Loop) {
  if (!Loop->hasChildren()) {
    return true;
  }

  std::unique_ptr<CanonExpr> TripCount(Loop->getTripCountCanonExpr());

  // The following check is required to remove useless loops.
  // For example:
  // for i=0, 10   ->   for i=0, min(19, 10)
  //  if (i < 20)       for i=max(20, 0), 10  -  negative trip count loop
  int64_t ConstantTrip;
  if (TripCount->isIntConstant(&ConstantTrip)) {
    return ConstantTrip <= 0;
  }

  return false;
}

void HIROptVarPredicate::addVarPredicateReport(
    HLIf *If, HLLoop *Loop, LoopOptReportBuilder &LORBuilder) {
  bool IsReportOn = LORBuilder.isLoopOptReportOn();

  if (!IsReportOn || !Loop) {
    return;
  }

  SmallString<32> LoopNum;
  unsigned LineNum;
  raw_svector_ostream VOS(LoopNum);
  if (If->getDebugLoc()) {
    LineNum = If->getDebugLoc().getLine();
    VOS << " at line ";
    VOS << LineNum;
  }
  LORBuilder(*Loop).addRemark(OptReportVerbosity::Low,
                              "Condition%s was optimized", LoopNum);
}

// The loop could be split into two loops:
// for i = 0, min(%b - 1, %UB) ztt: %b > 0            <-- Loop
// for i = max(%b, 0), %UB     ztt: %b <= %UB         <-- LoopClone
//
// Predicates like == and != create special case:
// for i = 0, min(%b - 1, %UB) ztt: %b > 0            <-- Loop
// for i = %b, %b              ztt: 0 < %b <= %UB     <-- LoopClone
// for i = max(%b + 1, 0), %UB ztt: %b + 1 <= %UB     <-- LoopRest
void HIROptVarPredicate::splitLoop(
    HLLoop *Loop, HLIf *Candidate, const RegDDRef *LHS, PredicateTy Pred,
    const RegDDRef *RHS, const CanonExpr *LowerCE, const CanonExpr *UpperCE,
    const CanonExpr *SplitPoint, bool ShouldInvertCondition) {

  assert((LowerCE->isIntConstant() || LowerCE->isStandAloneBlob(false)) &&
         "LowerCE should be a constant or stand-alone blob");
  assert((UpperCE->isIntConstant() || UpperCE->isStandAloneBlob(false)) &&
         "UpperCE should be a constant or stand-alone blob");
  assert((SplitPoint->isIntConstant() || SplitPoint->isStandAloneBlob(false)) &&
         "SplitPoint should be a constant or stand-alone blob");

  bool IsSigned = CmpInst::isSigned(Pred) || Pred == PredicateTy::ICMP_EQ ||
                  Pred == PredicateTy::ICMP_NE;
  bool FirstLoopNeeded = false;
  bool SecondLoopNeeded = false;
  bool ThirdLoopNeeded = false;

  // Invalidate the loop before it would be split.
  HIRInvalidationUtils::invalidateBounds(Loop);
  HIRInvalidationUtils::invalidateBody(Loop);

  Loop->extractZtt();
  Loop->extractPreheaderAndPostexit();

  HLContainerTy ThenContainer;
  HLContainerTy ElseContainer;

  if (!ShouldInvertCondition) {
    removeThenElseChildren(Candidate, &ThenContainer, &ElseContainer);
  } else {
    removeThenElseChildren(Candidate, &ElseContainer, &ThenContainer);
  }

  unsigned Level = Loop->getNestingLevel();

  // Split loop into two loops
  auto CloneMapper =
      HLNodeLambdaMapper::mapper([Candidate](const HLNode *Node) {
        return Node == Candidate || isa<HLLabel>(Node);
      });

  HLLoop *SecondLoop = Loop->clone(&CloneMapper);
  HLIf *CandidateClone = CloneMapper.getMapped(Candidate);

  HLNodeUtils::insertAfter(Loop, SecondLoop);

  // Replace HIf with the statement body
  if (!ThenContainer.empty()) {
    HLNodeUtils::insertAfter(Candidate, &ThenContainer);
  }

  if (!ElseContainer.empty()) {
    HIRTransformUtils::remapLabelsRange(CloneMapper, &ElseContainer.front(),
                                        &ElseContainer.back());
    HLNodeUtils::insertAfter(CandidateClone, &ElseContainer);
  }

  HLNodeUtils::remove(Candidate);
  HLNodeUtils::remove(CandidateClone);

  int64_t Val;
  bool SplitPtIsConst = SplitPoint->isIntConstant(&Val);
  // %b
  BlobTy SplitPointBlob = SplitPtIsConst
                              ? BU.createBlob(Val, SplitPoint->getDestType())
                              : BU.getBlob(SplitPoint->getSingleBlobIndex());
  // %UB
  BlobTy UpperBlob = UpperCE->isIntConstant(&Val)
                         ? BU.createBlob(Val, UpperCE->getDestType())
                         : BU.getBlob(UpperCE->getSingleBlobIndex());
  // %LB
  BlobTy LowerBlob = LowerCE->isIntConstant(&Val)
                         ? BU.createBlob(Val, LowerCE->getDestType())
                         : BU.getBlob(LowerCE->getSingleBlobIndex());

  // Clone is required as we will be updating *Loop* upper ref and will be using
  // original ref to make it consistent.
  std::unique_ptr<RegDDRef> LoopUpperDDRef(Loop->getUpperDDRef()->clone());
  SmallVector<const RegDDRef *, 4> Aux{LHS, RHS, Loop->getLowerDDRef(),
                                       LoopUpperDDRef.get()};

  // Special case ==, != predicates..
  HLLoop *ThirdLoop = nullptr;
  bool IsEqualCase =
      (Pred == PredicateTy::ICMP_EQ || Pred == PredicateTy::ICMP_NE);
  if (IsEqualCase) {
    ThirdLoop = Loop->clone();

    updateLoopUpperBound(SecondLoop, UpperBlob, SplitPointBlob, IsSigned);
    SecondLoop->getUpperDDRef()->makeConsistent(Aux, Level);

    // %b + 1
    BlobTy SplitPointPlusBlob = BU.createAddBlob(
        SplitPointBlob, BU.createBlob(1, SplitPointBlob->getType()),
        !SplitPtIsConst);

    updateLoopLowerBound(ThirdLoop, LowerBlob, SplitPointPlusBlob, IsSigned);

    if (!isLoopRedundant(ThirdLoop)) {
      HLNodeUtils::insertAfter(SecondLoop, ThirdLoop);
      ThirdLoop->getLowerDDRef()->makeConsistent(Aux, Level);

      ThirdLoop->createZtt(false, true);
      ThirdLoop->normalize();

      ThirdLoopNeeded = true;
    }
  }

  // %b - 1
  BlobTy SplitPointMinusBlob = BU.createMinusBlob(
      SplitPointBlob, BU.createBlob(1, SplitPointBlob->getType()),
      !SplitPtIsConst);

  updateLoopUpperBound(Loop, UpperBlob, SplitPointMinusBlob, IsSigned);
  updateLoopLowerBound(SecondLoop, LowerBlob, SplitPointBlob, IsSigned);

  if (!isLoopRedundant(Loop)) {
    Loop->getUpperDDRef()->makeConsistent(Aux, Level);
    Loop->createZtt(false, true);

    FirstLoopNeeded = true;
  } else {
    HLNodeUtils::remove(Loop);

    // Remove Loop from the invalidation set as we just removed it from the HIR.
    NodesToInvalidate.erase(Loop);
  }

  if (!isLoopRedundant(SecondLoop)) {
    SecondLoop->getLowerDDRef()->makeConsistent(Aux, Level);
    SecondLoop->createZtt(false, true);

    if (IsEqualCase) {
      SecondLoop->replaceByFirstIteration();
      SecondLoop = nullptr;
    } else {
      SecondLoop->normalize();
    }

    SecondLoopNeeded = true;
  } else {
    HLNodeUtils::remove(SecondLoop);
  }

  if (FirstLoopNeeded && (SecondLoopNeeded || ThirdLoopNeeded)) {
    HIRTransformUtils::addCloningInducedLiveouts(Loop);
  }

  if (SecondLoop && ThirdLoopNeeded) {
    HIRTransformUtils::addCloningInducedLiveouts(SecondLoop);
  }

  LoopOptReportBuilder &LORBuilder =
      Loop->getHLNodeUtils().getHIRFramework().getLORBuilder();

  HLLoop *OptReportLoop = nullptr;
  unsigned VNum = 1;

  if (FirstLoopNeeded) {
    OptReportLoop = Loop;
    LORBuilder(*Loop).addOrigin("Predicate Optimized v%d", VNum++);
  }

  if (SecondLoopNeeded && SecondLoop) {
    if (!OptReportLoop) {
      OptReportLoop = SecondLoop;
    }
    LORBuilder(*SecondLoop).addOrigin("Predicate Optimized v%d", VNum++);
  }

  if (ThirdLoopNeeded) {
    if (!OptReportLoop) {
      OptReportLoop = ThirdLoop;
    }
    LORBuilder(*ThirdLoop).addOrigin("Predicate Optimized v%d", VNum++);
  }

  addVarPredicateReport(Candidate, OptReportLoop, LORBuilder);
}

bool HIROptVarPredicate::processLoop(HLLoop *Loop) {
  LLVM_DEBUG(dbgs() << "Processing loop <" << Loop->getNumber() << ">\n");

  // Opt on non-innermost loops is likely to cause degradations.
  if (!DisableCostModel && !Loop->isInnermost()) {
    LLVM_DEBUG(
        dbgs()
        << "Non-innermost loop skipped due to profitability assumptions\n");
    return false;
  }

  if (!Loop->isDo()) {
    LLVM_DEBUG(dbgs() << "Unknown/Multiexit loop skipped.\n");
    return false;
  }

  if (Loop->hasUnrollEnablingPragma() || Loop->hasVectorizeEnablingPragma()) {
    LLVM_DEBUG(dbgs() << "Loop with unroll/vector pragma skipped\n");
    return false;
  }

  if (Loop->isSIMD()) {
    LLVM_DEBUG(dbgs() << "SIMD Loop skipped\n");
    return false;
  }

  SmallVector<HLIf *, 4> Candidates;

  std::unique_ptr<CanonExpr> LowerCE(Loop->getLowerCanonExpr()->clone());
  std::unique_ptr<CanonExpr> UpperCE(Loop->getUpperCanonExpr()->clone());

  // Blobyfy everything to make it compatible with min/max scev operations.
  // TODO: revisit this part after implementation of MIN/MAX DDRefs.
  if ((!LowerCE->isIntConstant() && !LowerCE->convertToStandAloneBlob()) ||
      (!UpperCE->isIntConstant() && !UpperCE->convertToStandAloneBlob())) {
    return false;
  }

  unsigned Level = Loop->getNestingLevel();

  IfLookup Lookup(Candidates, Level);
  HLNodeUtils::visitRange(Lookup, Loop->child_begin(), Loop->child_end());

  for (HLIf *Candidate :
       llvm::make_range(Candidates.begin(), Candidates.end())) {
    LLVM_DEBUG(dbgs() << "Processing: ");
    LLVM_DEBUG(Candidate->dumpHeader());
    LLVM_DEBUG(dbgs() << "\n");

    if (!TransformNodes.empty()) {
      if (std::find(TransformNodes.begin(), TransformNodes.end(),
                    Candidate->getNumber()) == TransformNodes.end()) {
        LLVM_DEBUG(dbgs() << "Skipped due to the command line option\n");
        continue;
      }
    }

    // TODO: Skip complex HLIfs for now
    if (Candidate->getNumPredicates() > 1) {
      LLVM_DEBUG(dbgs() << "Complex predicate skipped\n");
      continue;
    }

    auto PredI = Candidate->pred_begin();
    RegDDRef *LHS = Candidate->getPredicateOperandDDRef(PredI, true);
    RegDDRef *RHS = Candidate->getPredicateOperandDDRef(PredI, false);

    PredicateTy Pred = *PredI;

    // Normalize IV limitation to the form: i < SplitPoint, predicate could be:
    // <, ==, !=
    bool ShouldInvertCondition = false;
    std::unique_ptr<CanonExpr> SplitPoint(findIVSolution(
        Loop->getIVType(), LHS, Pred, RHS, Level, ShouldInvertCondition));

    // Can not handle this candidate
    if (!SplitPoint) {
      LLVM_DEBUG(dbgs() << "Couldn't find a solution.\n");
      continue;
    }

    LLVM_DEBUG(dbgs() << "Loop break point: ");
    LLVM_DEBUG(SplitPoint->dump());
    LLVM_DEBUG(dbgs() << "\n");

    if (!SplitPoint->isIntConstant() &&
        !SplitPoint->convertToStandAloneBlob()) {
      // This is mostly due to IVs in the split point.
      // TODO: implement min/max ddrefs
      LLVM_DEBUG(
          dbgs() << "Could not convert split point to a stand-alone blob\n");
      continue;
    }

    HLLoop *ParentLoop = Loop->getParentLoop();
    HLRegion *Region = Loop->getParentRegion();

    splitLoop(Loop, Candidate, LHS, Pred, RHS, LowerCE.get(), UpperCE.get(),
              SplitPoint.get(), ShouldInvertCondition);

    Region->setGenCode();

    NodesToInvalidate.insert(ParentLoop ? static_cast<HLNode *>(ParentLoop)
                                        : static_cast<HLNode *>(Region));

    LLVM_DEBUG(dbgs() << "While " OPT_DESC ":\n");
    LLVM_DEBUG(Region->dump(true));
    LLVM_DEBUG(dbgs() << "\n");

    LoopsSplit++;

    return true;
  }

  LLVM_DEBUG(dbgs() << "No candidates\n");
  return false;
}

PreservedAnalyses
HIROptVarPredicatePass::run(llvm::Function &F,
                            llvm::FunctionAnalysisManager &AM) {
  HIROptVarPredicate(AM.getResult<HIRFrameworkAnalysis>(F)).run();
  return PreservedAnalyses::all();
}

class HIROptVarPredicateLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIROptVarPredicateLegacyPass() : HIRTransformPass(ID) {
    initializeHIROptVarPredicateLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIROptVarPredicate(getAnalysis<HIRFrameworkWrapperPass>().getHIR())
        .run();
  }
};

char HIROptVarPredicateLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIROptVarPredicateLegacyPass, OPT_SWITCH, OPT_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_END(HIROptVarPredicateLegacyPass, OPT_SWITCH, OPT_DESC, false,
                    false)

FunctionPass *llvm::createHIROptVarPredicatePass() {
  return new HIROptVarPredicateLegacyPass();
}
