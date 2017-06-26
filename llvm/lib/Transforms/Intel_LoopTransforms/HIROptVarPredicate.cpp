//===- HIROptVarPredicate.cpp - Optimization of predicates containing IVs -===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/ForEach.h"

#define OPT_SWITCH "hir-opt-var-predicate"
#define OPT_DESC "HIR Var OptPredicate"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass(
    "disable-" OPT_SWITCH, cl::init(false), cl::Hidden,
    cl::desc("Disable " OPT_DESC " pass"));

static cl::opt<bool> DisableCostModel(
    "disable-" OPT_SWITCH "-cost-model", cl::init(false), cl::Hidden,
    cl::desc("Disable " OPT_DESC " cost model"));

static cl::list<unsigned> TransformNodes(
    OPT_SWITCH "-nodes", cl::Hidden,
    cl::desc("List nodes to transform by " OPT_DESC));

STATISTIC(LoopsSplit, "Loops split during optimization of predicates.");

namespace {

class HIROptVarPredicate : public HIRTransformPass {
  HIRFramework *HIR;
  HLNodeUtils *HLNodeUtilsObj;
  BlobUtils *BlobUtilsObj;

  SmallPtrSet<HLNode *, 8> NodesToInvalidate;

public:
  static char ID;

  HIROptVarPredicate() : HIRTransformPass(ID) {
    initializeHIROptVarPredicatePass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequiredTransitive<HIRFramework>();
    AU.setPreservesAll();
  }

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
};
}

char HIROptVarPredicate::ID = 0;
INITIALIZE_PASS_BEGIN(HIROptVarPredicate, OPT_SWITCH, OPT_DESC, false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_END(HIROptVarPredicate, OPT_SWITCH, OPT_DESC, false, false)

FunctionPass *llvm::createHIROptVarPredicatePass() {
  return new HIROptVarPredicate();
}

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

  void visit(const HLLabel *Goto) {
    HasLabel = true;
  }

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

  void visit(const HLIf *If) {
    llvm_unreachable("Unexpected const HLIf.");
  }

  void visit(HLLoop *Loop) {
    SkipNode = Loop;
    IfLookup Lookup(Candidates, Level);
    HLNodeUtils::visitRange(Lookup, Loop->child_begin(), Loop->child_end());
  }

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

  bool skipRecursion(const HLNode *Node) const override {
    return SkipNode == Node;
  }
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
    RHSConstAP.sadd_ov(LHSConstAP, Overflow);

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

bool HIROptVarPredicate::runOnFunction(Function &F) {
  if (skipFunction(F) || DisablePass) {
    return false;
  }

  DEBUG(dbgs() << "Optimization of Variant Predicates Function: " << F.getName()
               << "\n");

  HIR = &getAnalysis<HIRFramework>();
  HLNodeUtilsObj = &HIR->getHLNodeUtils();
  BlobUtilsObj = &HIR->getBlobUtils();

  ForPostEach<HLLoop>::visitRange(HIR->hir_begin(), HIR->hir_end(),
                                  [this](HLLoop *Loop) {
    // Opt on non-innermost loops is likely to cause degradations.
    if (!DisableCostModel && !Loop->isInnermost()) {
      return;
    }

    processLoop(Loop);
  });

  for (HLNode *Node : NodesToInvalidate) {
    if (HLLoop *Loop = dyn_cast<HLLoop>(Node)) {
      assert(Loop->isAttached() && "Every invalidated loop should be attached");
      HIRInvalidationUtils::invalidateBody(Loop);
    } else {
      HIRInvalidationUtils::invalidateNonLoopRegion(cast<HLRegion>(Node));
    }
    HLNodeUtils::removeEmptyNodes(Node, false);
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

  return BlobUtilsObj->createCastBlob(Blob, IsSigned, DesiredType, true,
                                      &BlobIndex);
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
      Ref->setSymbase(BlobUtilsObj->findTempBlobSymbase(Blob));
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
    BlobA =
        BlobUtilsObj->createSignExtendBlob(BlobA, BiggerType, true, &BlobIndex);
  } else {
    BlobA =
        BlobUtilsObj->createZeroExtendBlob(BlobA, BiggerType, true, &BlobIndex);
  }
}

void HIROptVarPredicate::updateLoopUpperBound(HLLoop *Loop, BlobTy UpperBlob,
                                              BlobTy SplitPointBlob,
                                              bool IsSigned) {
  makeBlobsTypeConsistent(UpperBlob, SplitPointBlob, IsSigned);

  unsigned MinBlobIndex;
  BlobTy MinBlob;

  if (IsSigned) {
    MinBlob = BlobUtilsObj->createSMinBlob(SplitPointBlob, UpperBlob, true,
                                           &MinBlobIndex);
  } else {
    MinBlob = BlobUtilsObj->createUMinBlob(SplitPointBlob, UpperBlob, true,
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

  if (IsSigned) {
    MaxBlob = BlobUtilsObj->createSMaxBlob(SplitPointBlob, LowerBlob, true,
                                           &MaxBlobIndex);
  } else {
    MaxBlob = BlobUtilsObj->createUMaxBlob(SplitPointBlob, LowerBlob, true,
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

  assert(LowerCE->isStandAloneBlob(false) &&
         "LowerCE should be a stand-alone blob");
  assert(UpperCE->isStandAloneBlob(false) &&
         "UpperCE should be a stand-alone blob");
  assert(SplitPoint->isStandAloneBlob(false) &&
         "SplitPoint should be a stand-alone blob");

  bool IsSigned = CmpInst::isSigned(Pred) || Pred == PredicateTy::ICMP_EQ ||
                  Pred == PredicateTy::ICMP_NE;

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
  auto CloneMapper = HLNodeLambdaMapper::mapper([Candidate](
      const HLNode *Node) { return Node == Candidate || isa<HLLabel>(Node); });

  HLLoop *SecondLoop = Loop->clone(&CloneMapper);
  HLIf *CandidateClone = CloneMapper.getMapped(Candidate);

  HLNodeUtilsObj->insertAfter(Loop, SecondLoop);

  // Replace HIf with the statement body
  if (!ThenContainer.empty()) {
    HLNodeUtilsObj->insertAfter(Candidate, &ThenContainer);
  }

  if (!ElseContainer.empty()) {
    HIRTransformUtils::remapLabelsRange(CloneMapper, &ElseContainer.front(),
                                        &ElseContainer.back());
    HLNodeUtilsObj->insertAfter(CandidateClone, &ElseContainer);
  }

  HLNodeUtils::remove(Candidate);
  HLNodeUtils::remove(CandidateClone);

  // %b
  BlobTy SplitPointBlob =
      BlobUtilsObj->getBlob(SplitPoint->getSingleBlobIndex());
  // %UB
  BlobTy UpperBlob = BlobUtilsObj->getBlob(UpperCE->getSingleBlobIndex());
  // %LB
  BlobTy LowerBlob = BlobUtilsObj->getBlob(LowerCE->getSingleBlobIndex());

  SmallVector<const RegDDRef *, 2> Aux {LHS, RHS, Loop->getLowerDDRef(),
                                        Loop->getUpperDDRef()};

  // Special case ==, != predicates..
  if (Pred == PredicateTy::ICMP_EQ || Pred == PredicateTy::ICMP_NE) {
    HLLoop *ThirdLoop = Loop->clone();

    updateLoopUpperBound(SecondLoop, UpperBlob, SplitPointBlob, IsSigned);
    SecondLoop->getUpperDDRef()->makeConsistent(&Aux, Level);

    // %b + 1
    BlobTy SplitPointPlusBlob = BlobUtilsObj->createAddBlob(
        SplitPointBlob,
        BlobUtilsObj->createBlob(1, SplitPointBlob->getType()));

    updateLoopLowerBound(ThirdLoop, LowerBlob, SplitPointPlusBlob, IsSigned);

    if (!isLoopRedundant(ThirdLoop)) {
      HLNodeUtilsObj->insertAfter(SecondLoop, ThirdLoop);
      ThirdLoop->getLowerDDRef()->makeConsistent(&Aux, Level);

      ThirdLoop->createZtt(false, true);
      ThirdLoop->normalize();
    }
  }

  // %b - 1
  BlobTy SplitPointMinusBlob = BlobUtilsObj->createMinusBlob(
      SplitPointBlob, BlobUtilsObj->createBlob(1, SplitPointBlob->getType()));

  updateLoopUpperBound(Loop, UpperBlob, SplitPointMinusBlob, IsSigned);
  updateLoopLowerBound(SecondLoop, LowerBlob, SplitPointBlob, IsSigned);

  if (!isLoopRedundant(Loop)) {
    Loop->getUpperDDRef()->makeConsistent(&Aux, Level);
    Loop->createZtt(false, true);

    HIRInvalidationUtils::invalidateBounds(Loop);
    HIRInvalidationUtils::invalidateBody(Loop);
  } else {
    HLNodeUtils::remove(Loop);

    // Remove Loop from the invalidation set as we just removed it from the HIR.
    NodesToInvalidate.erase(Loop);
  }

  if (!isLoopRedundant(SecondLoop)) {
    SecondLoop->getLowerDDRef()->makeConsistent(&Aux, Level);
    SecondLoop->createZtt(false, true);
    SecondLoop->normalize();
  } else {
    HLNodeUtils::remove(SecondLoop);
  }
}

bool HIROptVarPredicate::processLoop(HLLoop *Loop) {
  DEBUG(dbgs() << "Processing loop #" << Loop->getNumber() << "\n");

  if (!Loop->isDo()) {
    DEBUG(dbgs() << "Non-DO loop found\n");
    return false;
  }

  SmallVector<HLIf *, 4> Candidates;

  std::unique_ptr<CanonExpr> LowerCE(Loop->getLowerCanonExpr()->clone());
  std::unique_ptr<CanonExpr> UpperCE(Loop->getUpperCanonExpr()->clone());

  // Blobyfy everything to make it compatible with min/max scev operations.
  // TODO: revisit this part after implementation of MIN/MAX DDRefs.
  if (!LowerCE->convertToStandAloneBlob() ||
      !UpperCE->convertToStandAloneBlob()) {
    return false;
  }

  unsigned Level = Loop->getNestingLevel();

  IfLookup Lookup(Candidates, Level);
  HLNodeUtils::visitRange(Lookup, Loop->child_begin(), Loop->child_end());

  for (HLIf *Candidate :
       llvm::make_range(Candidates.begin(), Candidates.end())) {
    DEBUG(dbgs() << "Processing: ");
    DEBUG(Candidate->dumpHeader());
    DEBUG(dbgs() << "\n");

    if (!TransformNodes.empty()) {
      if (std::find(TransformNodes.begin(), TransformNodes.end(),
                    Candidate->getNumber()) == TransformNodes.end()) {
        DEBUG(dbgs() << "Skipped due to the command line option\n");
        continue;
      }
    }

    // TODO: Skip complex HLIfs for now
    if (Candidate->getNumPredicates() > 1) {
      DEBUG(dbgs() << "Complex predicate skipped\n");
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
      DEBUG(dbgs() << "Couldn't find a solution.\n");
      continue;
    }

    DEBUG(dbgs() << "Loop break point: ");
    DEBUG(SplitPoint->dump());
    DEBUG(dbgs() << "\n");

    if (!SplitPoint->convertToStandAloneBlob()) {
      // This is mostly due to IVs in the split point.
      // TODO: implement min/max ddrefs
      DEBUG(dbgs() << "Could not convert split point to a stand-alone blob\n");
      continue;
    }

    HLLoop *ParentLoop = Loop->getParentLoop();
    HLRegion *Region = Loop->getParentRegion();

    splitLoop(Loop, Candidate, LHS, Pred, RHS, LowerCE.get(), UpperCE.get(),
              SplitPoint.get(), ShouldInvertCondition);

    Region->setGenCode();

    NodesToInvalidate.insert(ParentLoop ? static_cast<HLNode *>(ParentLoop)
                                        : static_cast<HLNode *>(Region));

    DEBUG(dbgs() << "While " OPT_DESC ":\n");
    DEBUG(Region->dump(true));
    DEBUG(dbgs() << "\n");

    LoopsSplit++;

    return true;
  }

  DEBUG(dbgs() << "No candidates\n");
  return false;
}

void HIROptVarPredicate::releaseMemory() {
  NodesToInvalidate.clear();
}
