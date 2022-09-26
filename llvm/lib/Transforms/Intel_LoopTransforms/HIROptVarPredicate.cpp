//===- HIROptVarPredicate.cpp - Optimization of predicates containing IVs -===//
//
// Copyright (C) 2015-2021 Intel Corporation. All rights reserved.
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
#include "llvm/Transforms/Intel_LoopTransforms/HIROptVarPredicatePass.h"

#include "llvm/ADT/Statistic.h"
#if INTEL_FEATURE_CSA
#include "llvm/ADT/Triple.h"
#endif // INTEL_FEATURE_CSA

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
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/SIMDIntrinsicChecker.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "HIROptVarPredicate.h"

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

static cl::opt<bool> BypassSIMDLoop(OPT_SWITCH "-bypass-simd", cl::init(false),
                                    cl::Hidden,
                                    cl::desc(OPT_DESC " skipps simd loops"));

static cl::opt<bool>
    SkipIVOverflowCheck(OPT_SWITCH "-relax-ov", cl::init(false), cl::Hidden,
                        cl::desc(OPT_DESC " relaxes IV overflow check"));

STATISTIC(LoopsSplit, "Loops split during optimization of predicates.");

namespace {

struct EqualCandidates : public SmallSetVector<HLIf *, 8> {
  bool HasUnsafeCall = false;
  bool BothSidesIV = false;

  EqualCandidates(HLIf *If) { insert(If); }

  bool isEqual(const HLIf *If) const {
    return HLNodeUtils::areEqualConditions(front(), If);
  }

  bool hasCandidateWithNumber(unsigned Number) const {
    return std::any_of(begin(), end(), [&](const HLIf *If) {
      return If->getNumber() == Number;
    });
  }

  bool isProfitable(const HLLoop *Lp) const {
    if (DisableCostModel) {
      return true;
    }

    return Lp->isInnermost() || HasUnsafeCall;
  }

  bool shouldGenCode(const HLLoop *Lp) const {
    if (DisableCostModel) {
      return true;
    }

    return Lp->isInnermost();
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD
  void dump() const {
    for (auto *If : *this) {
      If->dumpHeader();
      dbgs() << "\n";
    }
  }
#endif
};

class HIROptVarPredicate : public HIROptVarPredicateInterface {
  HIRFramework &HIRF;
  BlobUtils &BU;

  SmallPtrSet<HLNode *, 8> NodesToInvalidate;

public:
  HIROptVarPredicate(HIRFramework &HIRF)
      : HIRF(HIRF), BU(HIRF.getBlobUtils()) {}

  bool run();

  bool processLoop(HLLoop *Loop, bool SetRegionModified,
                   SmallVectorImpl<HLLoop *> *OutLoops) override;

  const SmallPtrSetImpl<HLNode *> &getNodesToInvalidate() const override {
    return NodesToInvalidate;
  }

  virtual ~HIROptVarPredicate() {}

private:
  static std::unique_ptr<CanonExpr>
  findIVSolution(const HLLoop *Loop, const RegDDRef *LHSDDref, PredicateTy Pred,
                 const RegDDRef *RHSDDRef, unsigned Level,
                 bool &ShouldInvertCondition);

  void splitLoop(HLLoop *Loop, const EqualCandidates &Candidates,
                 const RegDDRef *LHS, PredicateTy Pred, const RegDDRef *RHS,
                 const CanonExpr *LowerCE, const CanonExpr *UpperCE,
                 const CanonExpr *SplitPoint, bool ShouldInvertCondition,
                 SmallVectorImpl<HLLoop *> *OutLoops);

  BlobTy castBlob(BlobTy Blob, Type *DesiredType, bool IsSigned);

  void updateLoopUpperBound(HLLoop *Loop, BlobTy UpperBlob,
                            BlobTy SplitPointBlob, bool IsSigned);

  void updateLoopLowerBound(HLLoop *Loop, BlobTy LowerBlob,
                            BlobTy SplitPointBlob, bool IsSigned);

  void addVarPredicateRemark(const EqualCandidates &Candidates, HLLoop *Loop,
                             OptReportBuilder &ORBuilder, unsigned RemarkID);

  std::tuple<HLInst *, RegDDRef *, bool>
  convertOneSideToStandAloneBlob(const RegDDRef *LHS, const RegDDRef *RHS,
                                 unsigned Level);

  static void replaceIfCondWithConvertedBlob(const EqualCandidates &Candidate,
                                             HLInst *CopyInst,
                                             RegDDRef *NewBlob,
                                             bool IsLHSConverted, HLLoop *Loop);

  static void cloneSIMDDirs(ArrayRef<HLLoop *> OutLoops,
                            const SIMDIntrinsicChecker &SIC);
};
} // namespace

class IfLookup final : public HLNodeVisitorBase {
  SmallVectorImpl<EqualCandidates> &Candidates;
  unsigned Level;
  const HLNode *SkipNode;

  bool HasLabel;
  bool HasUnsafeCall;

  void mergeCandidates(const IfLookup Src) {
    for (auto SrcEqualCandidates : Src.getCandidates()) {
      const HLIf *If = SrcEqualCandidates.front();
      auto ExistingI = std::find_if(
          Candidates.begin(), Candidates.end(),
          [If](const EqualCandidates &EC) { return EC.isEqual(If); });
      if (ExistingI != Candidates.end()) {
        ExistingI->insert(SrcEqualCandidates.begin(), SrcEqualCandidates.end());
        ExistingI->HasUnsafeCall |= SrcEqualCandidates.HasUnsafeCall;
      } else {
        Candidates.push_back(SrcEqualCandidates);
      }
    }
  }

public:
  IfLookup(SmallVectorImpl<EqualCandidates> &Candidates, unsigned Level)
      : Candidates(Candidates), Level(Level), SkipNode(nullptr),
        HasLabel(false), HasUnsafeCall(false) {}

  ArrayRef<EqualCandidates> getCandidates() const { return Candidates; }

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

  void visit(const HLLabel *) { HasLabel = true; }

  void visit(HLIf *If) {
    SkipNode = If;

    assert(If->getParentLoop() && "Parent should exist");

    SmallVector<EqualCandidates, 4> CandidatesFromChildren;
    IfLookup Lookup(CandidatesFromChildren, Level);
    HLNodeUtils::visitRange(Lookup, If->then_begin(), If->then_end());
    HLNodeUtils::visitRange(Lookup, If->else_begin(), If->else_end());

    if (Lookup.HasLabel) {
      mergeCandidates(Lookup);
      return;
    }

    // Find existing candidate
    auto ExistingInChildrenI = std::find_if(
        CandidatesFromChildren.begin(), CandidatesFromChildren.end(),
        [If](const EqualCandidates &EC) { return EC.isEqual(If); });
    if (ExistingInChildrenI != CandidatesFromChildren.end()) {
      // This if can't be a candidate of OptVarPredicate since its
      // then-else-children will all removed. Merge children's candidate into
      // the global candidates and finish here.
      // An example: outer-if and inner-if are in the same equality set.
      // Eventually in transformation phase, (in splitLoop function), the
      // inner-if in the outer-if's then or else body will be removed if the
      // outer-if were a candidate. Later when the children of if candidates are
      // put back into the loop and cloned loops, no parent node (i.e. removed
      // inner-ifs) is available. This will break stability.
      //  + DO i1 = 0, 99, 1   <DO_LOOP>
      //  |   if (i1 == 20) -- if 3
      //  |   {
      //  |      if (i1 == 20) -- if 1
      //  |      {
      //  |      }
      //  |   }
      //  |   else
      //  |   {
      //  |      if (i1 == 20) -- if 2
      //  |      {
      //  |      }
      //  |   }
      //  + END LOOP
      // Here we choose not to add the outer-if as a candidate.
      // In the code above "if 3" is not a candidate, only if-1 and if-2 are.
      mergeCandidates(Lookup);
      return;
    }

    mergeCandidates(Lookup);
    // Now this if can be added to Candidates
    auto ExistingI = std::find_if(
        Candidates.begin(), Candidates.end(),
        [If](const EqualCandidates &EC) { return EC.isEqual(If); });
    if (ExistingI != Candidates.end()) {
      ExistingI->HasUnsafeCall |= Lookup.HasUnsafeCall;
      ExistingI->insert(If);
      return;
    }

    // Loop through predicates to check if they satisfy opt predicate
    // conditions.
    bool BothSidesIV = false;
    for (auto Iter = If->pred_begin(), E = If->pred_end(); Iter != E; ++Iter) {
      const RegDDRef *LHSRef = If->getLHSPredicateOperandDDRef(Iter);
      const RegDDRef *RHSRef = If->getRHSPredicateOperandDDRef(Iter);

      bool LHSIV;
      bool RHSIV;

      // Check if both DDRefs satisfy all the conditions.
      if (!isCandidateRef(LHSRef, &LHSIV) || !isCandidateRef(RHSRef, &RHSIV)) {
        return;
      }

      if ((LHSIV && RHSIV) || (!LHSIV && !RHSIV)) {
        return;
      }

      // Exactly one of LHS or RHS has IV at Level.
      // See if the other side has IV at another Level.
      if (LHSIV && RHSRef->getSingleCanonExpr()->getFirstIVLevel() &&
              RHSRef->isStructurallyInvariantAtLevel(Level) ||
          RHSIV && LHSRef->getSingleCanonExpr()->getFirstIVLevel() &&
              LHSRef->isStructurallyInvariantAtLevel(Level)) {
        BothSidesIV = true;
      }
    }

    Candidates.emplace_back(If);
    Candidates.back().HasUnsafeCall = Lookup.HasUnsafeCall;
    Candidates.back().BothSidesIV = BothSidesIV;
  }

  void visit(const HLIf *If) { llvm_unreachable("Unexpected const HLIf."); }

  void visit(HLLoop *Loop) {
    SkipNode = Loop;
    IfLookup Lookup(Candidates, Level);
    HLNodeUtils::visitRange(Lookup, Loop->child_begin(), Loop->child_end());
  }

  void visit(const HLInst *Inst) {
    HasUnsafeCall = HasUnsafeCall || Inst->isUnsafeSideEffectsCallInst();
  }

  void visit(const HLNode *) {}
  void postVisit(const HLNode *) {}

  bool skipRecursion(const HLNode *Node) const { return SkipNode == Node; }
};

std::tuple<HLInst *, RegDDRef *, bool>
HIROptVarPredicate::convertOneSideToStandAloneBlob(const RegDDRef *LHS,
                                                   const RegDDRef *RHS,
                                                   unsigned Level) {
  bool hasLHSIV = LHS->hasIV(Level);
  bool hasRHSIV = RHS->hasIV(Level);
  assert(hasLHSIV != hasRHSIV);
  (void)hasLHSIV;

  bool IsLHSReplaced = false;
  if (hasRHSIV) {
    std::swap(LHS, RHS);
    IsLHSReplaced = true;
  }

  // ConvertToStandAloneBlob doesnt work when iv is present.
  // Add following copy to Blobify (c*iv + k).
  // %ivcopy = coeff * iv_anotherLevel + {const} + {blob}
  HLInst *CopyInst =
      HIRF.getHLNodeUtils().createCopyInst(RHS->clone(), "ivcopy");
  RegDDRef *BlobRHS = CopyInst->getLvalDDRef()->clone();

  // CopyInst is not inserted. BlobRHS is not yet attached either.
  // Thus, using CopyInst's Lval as AuxRef for calling makeConsistent() of
  // BlobRHS doesn't work.
  // If subsequent checks pass, CopyInst will be inserted before current Loop.
  BlobRHS->getSingleCanonExpr()->setDefinedAtLevel(Level - 1);

  return {CopyInst, BlobRHS, IsLHSReplaced};
}

void HIROptVarPredicate::replaceIfCondWithConvertedBlob(
    const EqualCandidates &Candidate, HLInst *CopyInst, RegDDRef *NewBlob,
    bool IsLHSConverted, HLLoop *Loop) {

  // Take care of the first If
  HLIf *IfCandidate = Candidate.front();
  auto PredI = IfCandidate->pred_begin();
  RegDDRef *OrigLHS = IfCandidate->getLHSPredicateOperandDDRef(PredI);
  RegDDRef *OrigRHS = IfCandidate->getRHSPredicateOperandDDRef(PredI);
  RegDDRef *OldRef = IsLHSConverted ? OrigLHS : OrigRHS;
  IfCandidate->replaceOperandDDRef(OldRef, NewBlob);

  // Insert copy using original if-cond as aux refs.
  // Notice that the copy needs to be inserted only once
  // before the enclosing loop.
  HLNodeUtils::insertBefore(Loop, CopyInst);
  CopyInst->getLvalDDRef()->makeConsistent({OrigLHS, OrigRHS});
  Loop->addLiveInTemp(CopyInst->getLvalDDRef());

  // Take care of the rest of Ifs, if any.
  for (HLIf *IfCandidate :
       make_range(std::next(Candidate.begin()), Candidate.end())) {
    auto PredI = IfCandidate->pred_begin();
    RegDDRef *OrigLHS = IfCandidate->getLHSPredicateOperandDDRef(PredI);
    RegDDRef *OrigRHS = IfCandidate->getRHSPredicateOperandDDRef(PredI);
    RegDDRef *OldRef = IsLHSConverted ? OrigLHS : OrigRHS;
    IfCandidate->replaceOperandDDRef(OldRef, NewBlob->clone());
  }
}

static inline void cloneDirEntry(HLNode *Pos, const HLInst *DirSIMD,
                                 ArrayRef<const HLInst *> AuxInsts) {
  HLInst *Entry = DirSIMD->clone();
  HLNodeUtils::insertBefore(Pos, Entry);
  for (const HLInst *Load : AuxInsts)
    HLNodeUtils::insertBefore(Pos, Load->clone());
}

static inline void cloneDirExit(HLNode *Pos, const HLInst *DirSIMDExit,
                                ArrayRef<const HLInst *> AuxInsts) {
  HLInst *Exit = DirSIMDExit->clone();
  HLNodeUtils::insertAfter(Pos, Exit);
  for (const HLInst *Store : AuxInsts)
    HLNodeUtils::insertBefore(Exit, Store->clone());
}

void HIROptVarPredicate::cloneSIMDDirs(
    ArrayRef<HLLoop *> OutLoops,
    const SIMDIntrinsicChecker &SIC) {

  unsigned NumOutLoops = OutLoops.size();
  // If there is only one output loop, no more cloning is needed,
  // so just return.
  if (NumOutLoops <= 1)
    return;

  const HLInst *DirSIMD = SIC.getSIMDEntryInst();
  const HLInst *DirSIMDExit = SIC.getSIMDExitInst();

  // NumOutLoops >= 2
  // Figure out if the there is a non-loop middle node
  HLLoop *FirstLp = OutLoops.front();
  HLLoop *LastLp = OutLoops[NumOutLoops - 1];
  HLNode *NextNode = FirstLp->getNextNode();
  if (!isa<HLLoop>(NextNode) && NextNode != LastLp) {
    // If there is a non-loop middle node, clone/insert pre/post loops.
    if (HLIf* If = dyn_cast<HLIf>(NextNode)) {
      assert(If->getNumElseChildren() == 0);
      cloneDirEntry(If->getFirstThenChild(), DirSIMD, SIC.getRedPreLoopInsts());
      cloneDirExit(If->getLastThenChild(), DirSIMDExit, SIC.getRedPostLoopInsts());
    } else {
      cloneDirEntry(NextNode, DirSIMD, SIC.getRedPreLoopInsts());
      cloneDirExit(NextNode, DirSIMDExit, SIC.getRedPostLoopInsts());
    }
  }

  // First and last loops are handled differently
  // First loop - add exit only
  cloneDirExit(FirstLp, DirSIMDExit, SIC.getRedPostLoopInsts());

  // Last - add entry only
  cloneDirEntry(LastLp, DirSIMD, SIC.getRedPreLoopInsts());

  // Middle loops, if any
  for (unsigned I = 1; I < NumOutLoops - 1; I++) {
    HLLoop *Lp = OutLoops[I];
    // Exit
    cloneDirExit(Lp, DirSIMDExit, SIC.getRedPostLoopInsts());
    // Entry
    cloneDirEntry(Lp, DirSIMD, SIC.getRedPreLoopInsts());
  }
}

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

static bool mayIVOverflowCE(const CanonExpr *CE, const HLLoop *Loop) {

  unsigned Width = Loop->getIVType()->getPrimitiveSizeInBits();

  if (CE->getSrcType()->getPrimitiveSizeInBits() >= Width &&
      CE->getDestType()->getPrimitiveSizeInBits() >= Width)
    return false;

  if (!CE->isTrunc())
    return true;

  uint64_t LegalMaxTC = Loop->getLegalMaxTripCount();
  if (!LegalMaxTC)
    return true;

  // Only takes care of wrap-around logic in the context of
  // signedness. Notice that EQ/NEQ will introduce sext and smin/max
  // through opt var predicate.
  //   + DO i32 i1 = 0, sext.i16.i32(%n) + -1, 1 <LEGAL_MAX_TC = 32767>
  //   |   if (i1 == %d)
  //   |   <RVAL-REG> LINEAR trunc.i32.i16(i1) {sb:2}
  //   |   <RVAL-REG> LINEAR i16 %d {sb:7}
  //   |   {
  //   |      (%p)[i1] = i1;
  //   |
  //   |   }
  //   |   else
  //   |   {
  //   |      (%q)[i1] = i1;
  //   |   }
  //   + END LOOP
  //
  // Will be transformed into
  //
  //  + DO i32 i1 = 0, smin(sext.i16.i32((-1 + %d)), (-1 + sext.i16.i32(%n))), 1
  //  | (%q)[i1] = i1;
  //  + END LOOP
  //
  //  if (smax(0, sext.i16.i32(%d)) < smin(sext.i16.i32(%d), (-1 +
  //  sext.i16.i32(%n))) + 1)
  //  {
  //   (%p)[smax(0, sext.i16.i32(%d))] = smax(0, sext.i16.i32(%d));
  //  }
  //
  //  + DO i32 i1 = 0, sext.i16.i32(%n) + -1 * smax(0, sext.i16.i32((1 + %d))) +
  //  -1, 1 | (%q)[i1 + smax(0, sext.i16.i32((1 + %d)))] = i1 + smax(0,
  //  sext.i16.i32((1 + %d)));
  //  + END LOOP
  //
  // Notice with sext, having bit 1, at msb of the 16-bit (leftmost of the
  // 16-bit) can lead
  // to an incorrect transformation. E.g. %d = 0xFFFE, the condition in middle
  // if-stmt is not met. smax(0, sext.i16.i32(0xfffe)) = 0,
  // smin(sext.i16.i32(0xfffe), (-1 + sext.i16.i32(%n)) + 1) = 0xfffe = -2
  // However, in the original loop before transformation the condition (i1  ==
  // %d) could have been met (e.g. i1 = 0xfffe and LEGAL_MAX_TC were 2^16 - 1).
  // To be safe, LEGAL_MAX_TC should fit in 2^15 - 1, not 2^16 - 1.
  // Cannot use mayWraparound util, which uses getMaxValue in trunc.
  // Canbe extended to use getMaxValue when contexts are zeroext, umin/max.

  APInt MaxTypeVal =
      APInt::getSignedMaxValue(CE->getDestType()->getScalarSizeInBits());

  if (MaxTypeVal.getZExtValue() >= LegalMaxTC) {
    LLVM_DEBUG(dbgs() << "Using LEGAL_MAX_TC\n");
    return false;
  }

  return true;
}

std::unique_ptr<CanonExpr> HIROptVarPredicate::findIVSolution(
    const HLLoop *Loop, const RegDDRef *LHSDDref, PredicateTy Pred,
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

  if (CmpInst::isUnsigned(Pred) ||
      !SkipIVOverflowCheck && mayIVOverflowCE(LHS, Loop)) {
    return nullptr;
  }

  // Assuming that LHS is 1*IV
  std::unique_ptr<CanonExpr> Result(RHS->clone());

  int64_t LHSConst = LHS->getConstant();
  // TODO: i2 != i1 + %207 the following if-cond is not true since
  //       we will make the RHS a standalong blob after a copy.
  //       However, revisit at some point to see the relevance.
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

    // sadd_ov can not handle different type sizes, so we bailout, e.g.
    // if (i1 + 1 == 0)
    //   <RVAL-REG> LINEAR trunc.i64.i32(i1 + 1) {sb:2}
    //   RHS's 0 is i32 type.
    // Can be extended to see if the const value in the larger bit size
    // can fit within the smaller bit size (using getSignedMin/MaxValue)
    // and then compute sadd_ov in the smaller bit size.
    if (RHSType->getPrimitiveSizeInBits() !=
        LHSType->getPrimitiveSizeInBits()) {
      return nullptr;
    }

    bool Overflow;
    APInt RHSConstAP(RHSType->getPrimitiveSizeInBits(), RHSConst, true);
    APInt LHSConstAP(LHSType->getPrimitiveSizeInBits(), -LHSConst, true);
    (void)RHSConstAP.sadd_ov(LHSConstAP, Overflow);

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

  return Result;
}

bool HIROptVarPredicate::run() {
  if (DisablePass) {
    return false;
  }

#if INTEL_FEATURE_CSA
  // When compiling for offload target = CSA:
  //
  // If the user does not specify the option -mllvm
  // disable-hir-opt-var-predicate at all on the command line, then
  // HIROptVarPredicate (i.e., this pass) will be disabled by
  // default. HIROptVarPredicate will be enabled only when the user
  // specifies -mllvm disable-hir-opt-var-predicate=false (or = 0) on
  // the command line.

  bool IsCsaTarget =
      Triple(HIRF.getFunction().getParent()->getTargetTriple()).getArch() ==
      Triple::ArchType::csa;

  if (IsCsaTarget && (DisablePass.getNumOccurrences() == 0)) {
    return false;
  }
#endif // INTEL_FEATURE_CSA

  LLVM_DEBUG(dbgs() << "Optimization of Variant Predicates Function: "
                    << HIRF.getFunction().getName() << "\n");

  ForPostEach<HLLoop>::visitRange(
      HIRF.hir_begin(), HIRF.hir_end(),
      [this](HLLoop *Loop) { processLoop(Loop, true, nullptr); });

  for (HLNode *Node : NodesToInvalidate) {
    if (HLLoop *Loop = dyn_cast<HLLoop>(Node)) {
      // Loop splitting could result in removing a loop from HIR. If one of
      // loop's children was placed in the invalidation list, the child becomes
      // detached. Skip those loops as they are not a part of HIR any more.
      if (Loop->isAttached())
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
                                    bool IsSigned) {
  if (Blob->getType() == DesiredType) {
    return Blob;
  }

  return BU.createCastBlob(Blob, IsSigned, DesiredType,
                           !isa<SCEVConstant>(Blob));
}

void HIROptVarPredicate::updateLoopUpperBound(HLLoop *Loop, BlobTy UpperBlob,
                                              BlobTy SplitPointBlob,
                                              bool IsSigned) {
  // Cast split point to an IV type, to use in loop bounds.
  SplitPointBlob = castBlob(SplitPointBlob, Loop->getIVType(), IsSigned);

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

  HIRTransformUtils::setSelfBlobDDRef(Loop->getUpperDDRef(), MinBlob,
                                      MinBlobIndex);
}

void HIROptVarPredicate::updateLoopLowerBound(HLLoop *Loop, BlobTy LowerBlob,
                                              BlobTy SplitPointBlob,
                                              bool IsSigned) {
  // Cast split point to an IV type, to use in loop bounds.
  SplitPointBlob = castBlob(SplitPointBlob, Loop->getIVType(), IsSigned);

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

  HIRTransformUtils::setSelfBlobDDRef(Loop->getLowerDDRef(), MaxBlob,
                                      MaxBlobIndex);
}

static bool isLoopRedundant(const HLLoop *Loop, const HLNode *ContextNode) {
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

  bool IsNegativeOrZeroTC =
      HLNodeUtils::isKnownNonPositive(TripCount.get(), ContextNode);

  return IsNegativeOrZeroTC;
}

static SmallString<32>
constructLineNumberString(SmallVector<unsigned, 8> LineNumbers) {
  SmallString<32> LineNumStr;
  raw_svector_ostream VOS(LineNumStr);
  auto It = LineNumbers.begin();

  if (LineNumbers.size() == 1) {
    VOS << " at line " << *It;
  } else if (LineNumbers.size() == 2) {
    VOS << " at lines ";
    VOS << *It << " and " << *std::next(It);
  } else {
    VOS << " at lines ";
    while (std::next(It) != LineNumbers.end()) {
      VOS << *It << ", ";
      ++It;
    }
    VOS << "and " << *It;
  }

  return LineNumStr;
}

void HIROptVarPredicate::addVarPredicateRemark(
    const EqualCandidates &Candidates, HLLoop *Loop,
    OptReportBuilder &ORBuilder, unsigned RemarkID) {
  if (!Loop) {
    return;
  }

  SmallVector<unsigned, 8> LineNumbers;
  for (HLIf *If : Candidates) {
    auto &DebugLoc = If->getDebugLoc();
    LineNumbers.push_back(DebugLoc ? DebugLoc.getLine() : 0);
  }

  auto LineNumStr = constructLineNumberString(LineNumbers);

  ORBuilder(*Loop).addRemark(OptReportVerbosity::Low, RemarkID, LineNumStr);
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
    HLLoop *Loop, const EqualCandidates &Candidates, const RegDDRef *LHS,
    PredicateTy Pred, const RegDDRef *RHS, const CanonExpr *LowerCE,
    const CanonExpr *UpperCE, const CanonExpr *SplitPoint,
    bool ShouldInvertCondition, SmallVectorImpl<HLLoop *> *OutLoops) {

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
  NodesToInvalidate.erase(Loop);

  Loop->extractZttPreheaderAndPostexit();

  // Process HLIfs inside the loops.

  SmallVector<HLContainerTy, 2> ThenContainers;
  SmallVector<HLContainerTy, 2> ElseContainers;
  ThenContainers.resize(Candidates.size());
  ElseContainers.resize(Candidates.size());

  for (auto CI : enumerate(Candidates)) {
    // Invalidate If's container before moving nodes around.
    HLLoop *ParentLoop = CI.value()->getParentLoop();
    HIRInvalidationUtils::invalidateBody(ParentLoop);
    NodesToInvalidate.erase(ParentLoop);

    auto &ThenContainer = ThenContainers[CI.index()];
    auto &ElseContainer = ElseContainers[CI.index()];

    if (!ShouldInvertCondition) {
      removeThenElseChildren(CI.value(), &ThenContainer, &ElseContainer);
    } else {
      removeThenElseChildren(CI.value(), &ElseContainer, &ThenContainer);
    }
  }

  unsigned Level = Loop->getNestingLevel();

  // Split loop into two loops
  auto CloneMapper =
      HLNodeLambdaMapper::mapper([&Candidates](const HLNode *Node) {
        const HLIf *If = dyn_cast<HLIf>(Node);
        return (If && Candidates.count(const_cast<HLIf *>(If))) ||
               isa<HLLabel>(Node);
      });

  HLLoop *SecondLoop = Loop->clone(&CloneMapper);

  // Replace nodes back.
  for (auto CI : enumerate(Candidates)) {
    auto &ThenContainer = ThenContainers[CI.index()];
    auto &ElseContainer = ElseContainers[CI.index()];

    HLIf *CandidateClone = CloneMapper.getMapped(CI.value());

    // Replace HIf with the statement body
    if (!ThenContainer.empty()) {
      HLNodeUtils::insertAfter(CI.value(), &ThenContainer);
    }

    if (!ElseContainer.empty()) {
      HIRTransformUtils::remapLabelsRange(CloneMapper, &ElseContainer.front(),
                                          &ElseContainer.back());
      HLNodeUtils::insertAfter(CandidateClone, &ElseContainer);
    }

    HLNodeUtils::remove(CI.value());
    HLNodeUtils::remove(CandidateClone);
  }

  HLNodeUtils::insertAfter(Loop, SecondLoop);

  // Update loop bounds and create third clone if needed.

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

    if (!isLoopRedundant(ThirdLoop, Loop)) {
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

  if (!isLoopRedundant(Loop, Loop)) {
    Loop->getUpperDDRef()->makeConsistent(Aux, Level);
    Loop->createZtt(false, true);

    FirstLoopNeeded = true;

    if (OutLoops) {
      OutLoops->push_back(Loop);
    }
  }

  if (!isLoopRedundant(SecondLoop, SecondLoop)) {
    SecondLoop->getLowerDDRef()->makeConsistent(Aux, Level);
    SecondLoop->createZtt(false, true);

    if (IsEqualCase) {
      SecondLoop->replaceByFirstIteration();
      SecondLoop = nullptr;
    } else {
      SecondLoop->normalize();
    }

    SecondLoopNeeded = true;

    if (SecondLoop && OutLoops) {
      OutLoops->push_back(SecondLoop);
    }
  } else {
    HLNodeUtils::remove(SecondLoop);

    // Remove Loop from the invalidation set as we just removed it from the HIR.
    NodesToInvalidate.erase(SecondLoop);
  }

  if (ThirdLoopNeeded && OutLoops) {
    OutLoops->push_back(ThirdLoop);
  }

  if (FirstLoopNeeded && (SecondLoopNeeded || ThirdLoopNeeded)) {
    HLNodeUtils::addCloningInducedLiveouts(Loop);
  }

  if (SecondLoop && ThirdLoopNeeded) {
    HLNodeUtils::addCloningInducedLiveouts(SecondLoop);
  }

  OptReportBuilder &ORBuilder =
      Loop->getHLNodeUtils().getHIRFramework().getORBuilder();

  if (ORBuilder.isOptReportOn()) {

    bool SecondLoopNeededAndNotNull = SecondLoopNeeded && SecondLoop;
    SmallVector<bool, 3> LoopsNeeded = { FirstLoopNeeded, SecondLoopNeededAndNotNull, ThirdLoopNeeded };
    unsigned NumLoopsNeeded = std::count(LoopsNeeded.begin(), LoopsNeeded.end(), true);
    if (NumLoopsNeeded >= 2) {
      unsigned VNum = 1;

      if (FirstLoopNeeded) {
        // Predicate Optimized v%d
        ORBuilder(*Loop).addOrigin(25476u, VNum++);
      }

      if (SecondLoopNeededAndNotNull) {
        // Predicate Optimized v%d
        ORBuilder(*SecondLoop).addOrigin(25476u, VNum++);
      }

      if (ThirdLoopNeeded) {
        // Predicate Optimized v%d
        ORBuilder(*ThirdLoop).addOrigin(25476u, VNum++);
      }
    }

    bool IsLoopPeeled = IsEqualCase && NumLoopsNeeded == 1;
    bool IsLoopOptimizedAway = NumLoopsNeeded == 0;

    HLLoop *OptReportLoop = nullptr;
    if (FirstLoopNeeded || IsLoopOptimizedAway) {
      OptReportLoop = Loop;
    } else if (SecondLoopNeededAndNotNull) {
      OptReportLoop = SecondLoop;
      ORBuilder(*Loop).moveOptReportTo(*SecondLoop);
    } else if (ThirdLoopNeeded) {
      OptReportLoop = ThirdLoop;
      ORBuilder(*Loop).moveOptReportTo(*ThirdLoop);
    }

    if (IsLoopPeeled) {
      // Loop peeled using condition%s
      addVarPredicateRemark(Candidates, OptReportLoop, ORBuilder, 25258u);
    } else if (IsLoopOptimizedAway) {
      // Loop optimized away using condition%s
      addVarPredicateRemark(Candidates, OptReportLoop, ORBuilder, 25259u);
      ORBuilder(*OptReportLoop).preserveLostOptReport();
    } else {
      // Induction variable range split using condition%s
      addVarPredicateRemark(Candidates, OptReportLoop, ORBuilder, 25580u);
    }
  }

  if (!FirstLoopNeeded) {
    HLNodeUtils::remove(Loop);
    NodesToInvalidate.erase(Loop);
  }
}

bool HIROptVarPredicate::processLoop(HLLoop *Loop, bool SetRegionModified,
                                     SmallVectorImpl<HLLoop *> *OutLoops) {
  LLVM_DEBUG(
      dbgs() << "Function "
             << Loop->getHLNodeUtils().getHIRFramework().getFunction().getName()
             << ": \n");
  LLVM_DEBUG(dbgs() << "Processing loop <" << Loop->getNumber() << ">\n");

  if (!Loop->isDo()) {
    LLVM_DEBUG(dbgs() << "Unknown/Multiexit loop skipped.\n");
    return false;
  }

  if (Loop->hasUnrollEnablingPragma() || Loop->hasVectorizeEnablingPragma()) {
    LLVM_DEBUG(dbgs() << "Loop with unroll/vector pragma skipped\n");
    return false;
  }

  const HLInst *DirSIMD = Loop->getSIMDEntryIntrinsic();
  SIMDIntrinsicChecker SIC(BypassSIMDLoop ? nullptr : DirSIMD, Loop);
  if (DirSIMD) {
    if (BypassSIMDLoop) {
      LLVM_DEBUG(dbgs() << "SIMD Loop skipped.\n");
      return false;
    }

    if (!SIC.isHandleable() || !SIC.areAllInPreAndPostLoop()) {
      LLVM_DEBUG(dbgs() << "Unhandleable SIMD Loop.\n");
      return false;
    }
  }

  SmallVector<EqualCandidates, 4> Candidates;

  std::unique_ptr<CanonExpr> LowerCE(Loop->getLowerCanonExpr()->clone());
  std::unique_ptr<CanonExpr> UpperCE(Loop->getUpperCanonExpr()->clone());

  // Blobyfy everything to make it compatible with min/max scev operations.
  // TODO: revisit this part after implementation of MIN/MAX DDRefs.
  if ((!LowerCE->isIntConstant() &&
       !LowerCE->convertToStandAloneBlobOrConstant()) ||
      (!UpperCE->isIntConstant() &&
       !UpperCE->convertToStandAloneBlobOrConstant())) {
    LLVM_DEBUG(dbgs() << "Loop bounds are not stand-alone blobs.\n");
    return false;
  }

  unsigned Level = Loop->getNestingLevel();

  IfLookup Lookup(Candidates, Level);
  HLNodeUtils::visitRange(Lookup, Loop->child_begin(), Loop->child_end());

  for (const EqualCandidates &Candidate : Candidates) {
    LLVM_DEBUG(dbgs() << "Processing Ifs:\n");
    LLVM_DEBUG(Candidate.dump());
    LLVM_DEBUG(dbgs() << "\n");

    if (!Candidate.isProfitable(Loop)) {
      LLVM_DEBUG(dbgs() << "Skipping candidate is non-profitable.\n");
      continue;
    }

    if (!TransformNodes.empty()) {
      if (std::any_of(TransformNodes.begin(), TransformNodes.end(),
                      [&](unsigned Number) {
                        return Candidate.hasCandidateWithNumber(Number);
                      })) {
        LLVM_DEBUG(dbgs() << "Skipped due to the command line option\n");
        continue;
      }
    }

    HLIf *IfCandidate = Candidate.front();

    // TODO: Skip complex HLIfs for now
    if (IfCandidate->getNumPredicates() > 1) {
      LLVM_DEBUG(dbgs() << "Complex predicate skipped\n");
      continue;
    }

    auto PredI = IfCandidate->pred_begin();
    RegDDRef *LHS = IfCandidate->getLHSPredicateOperandDDRef(PredI);
    RegDDRef *RHS = IfCandidate->getRHSPredicateOperandDDRef(PredI);

    PredicateTy Pred = *PredI;

    bool HaveBothSidesIV = Candidate.BothSidesIV;
    bool IsLHSConverted = false;
    HLInst *CopyInst = nullptr;
    RegDDRef *NewBlob = nullptr;
    if (HaveBothSidesIV) {
      // In case of BothSidesIV
      // - createCopyInst of RHS or LHS
      // - Use the Lval of the copy in place of RHS or LHS
      // Note that actual insertion and replacement of operands
      // do not happen yet.
      std::tie(CopyInst, NewBlob, IsLHSConverted) =
          convertOneSideToStandAloneBlob(LHS, RHS, Level);

      if (IsLHSConverted)
        LHS = NewBlob;
      else
        RHS = NewBlob;
    }

    // Normalize IV limitation to the form: i < SplitPoint, predicate could be:
    // <, ==, !=
    bool ShouldInvertCondition = false;
    std::unique_ptr<CanonExpr> SplitPoint(
        findIVSolution(Loop, LHS, Pred, RHS, Level, ShouldInvertCondition));

    // Can not handle this candidate
    if (!SplitPoint) {
      LLVM_DEBUG(dbgs() << "Couldn't find a solution.\n");
      continue;
    }

    LLVM_DEBUG(dbgs() << "Loop break point: ");
    LLVM_DEBUG(SplitPoint->dump());
    LLVM_DEBUG(dbgs() << "\n");

    if (!SplitPoint->isIntConstant() &&
        !SplitPoint->convertToStandAloneBlobOrConstant()) {
      // This is mostly due to IVs in the split point.
      // TODO: implement min/max ddrefs
      LLVM_DEBUG(
          dbgs() << "Could not convert split point to a stand-alone blob\n");
      continue;
    }

    if (HaveBothSidesIV) {
      // In case of BothSidesIV, prep transformations are needed
      // before splitting the loop into multiple loops.
      // - insert the copy in front of Ifs in Candidates
      // - replace Ifs in Candidates with the new condition using new L/Rval
      replaceIfCondWithConvertedBlob(Candidate, CopyInst, NewBlob,
                                     IsLHSConverted, Loop);
      LLVM_DEBUG(dbgs() << "BothSidesIV - Before splitLoop:\n"; Loop->dump());
    }

    HLLoop *ParentLoop = Loop->getParentLoop();
    HLRegion *Region = Loop->getParentRegion();

    SmallVector<HLLoop *, 4> OutLoopsVec;
    if (DirSIMD && !OutLoops)
      OutLoops = &OutLoopsVec;
    splitLoop(Loop, Candidate, LHS, Pred, RHS, LowerCE.get(), UpperCE.get(),
              SplitPoint.get(), ShouldInvertCondition, OutLoops);

    if (DirSIMD)
      cloneSIMDDirs(*OutLoops, SIC);

    if (SetRegionModified && Candidate.shouldGenCode(Loop)) {
      Region->setGenCode();
    }

    NodesToInvalidate.insert(ParentLoop ? static_cast<HLNode *>(ParentLoop)
                                        : static_cast<HLNode *>(Region));

    LLVM_DEBUG(dbgs() << "While " OPT_DESC ":\n");
    LLVM_DEBUG(Region->dump());
    LLVM_DEBUG(Region->dump(true));
    LLVM_DEBUG(dbgs() << "\n");

    LoopsSplit++;

    return true;
  }

  LLVM_DEBUG(dbgs() << "No candidates\n");
  return false;
}

PreservedAnalyses HIROptVarPredicatePass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  HIROptVarPredicate(HIRF).run();
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

std::unique_ptr<HIROptVarPredicateInterface>
HIROptVarPredicateInterface::create(HIRFramework &HIRF) {
  return std::make_unique<HIROptVarPredicate>(HIRF);
}
