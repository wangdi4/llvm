//===-------------------------- HIRRowWiseMV.cpp --------------------------===//
//
// Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements row-wise multiversioning for arrays where entire rows
/// may be "arithmetically convenient" ones that allow strength for strength
/// reduction (such as 0 for addition and -1, 0, and 1 for multiplication).
///
/// For example:
///
/// \code
/// DO i1
///   DO i2 = B, E, S
///     ... A[i2] * B ...
///   END DO
/// END DO
///
/// ===>
///
/// first = A[0]
/// rowcase = 0;
/// DO i1 = B + S, E, S
///   if (A[i1+S] != first)
///   {
///     goto CheckFailed;
///   }
/// END DO
///
/// if (first == 0)
/// {
///   rowcase = 1;
/// }
/// else
/// {
///   if (first == 1)
///   {
///     rowcase = 2;
///   }
/// }
///
/// CheckFailed:
/// DO i1
///   switch(rowcase)
///   {
///   case 1:
///     DO i2 = B, E, S
///        ... 0 ...
///     END DO
///   case 2:
///     DO i2 = B, E, S
///        ... B ...
///     END DO
///   default:
///     DO i2 = B, E, S
///        ... A[i2] * B ...
///     END DO
///   }
/// END DO
/// \endcode
///
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRRowWiseMV.h"

#include "Intel_DTrans/Analysis/DTransImmutableAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

#define OPT_SWITCH "hir-rowwise-mv"
#define OPT_DESC "HIR Row-Wise Multiversioning"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass{"disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass")};
static cl::opt<bool> SkipDTrans{
  OPT_SWITCH "-skip-dtrans", cl::init(false), cl::Hidden,
  cl::desc("Skip the DTrans check and multiversion for all arithmetically "
           "convenient values")};

namespace {

/// A wrapper for running HIRRowWiseMV with the old pass manager.
class HIRRowWiseMVLegacyPass : public HIRTransformPass {
public:
  static char ID;
  HIRRowWiseMVLegacyPass() : HIRTransformPass{ID} {
    initializeHIRRowWiseMVLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<DTransImmutableAnalysisWrapper>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.setPreservesAll();
  }
};

/// Implements the row-wise multiversioning transform.
class HIRRowWiseMV {
  HIRDDAnalysis &HDDA;
  HIRLoopStatistics &HLS;
  DTransImmutableInfo &DTII;

public:
  HIRRowWiseMV(HIRDDAnalysis &HDDA, HIRLoopStatistics &HLS,
               DTransImmutableInfo &DTII)
      : HDDA{HDDA}, HLS{HLS}, DTII{DTII} {}

  /// Performs row-wise multiversioning on the given loop.
  bool run(HLLoop *);
};

/// The type used for representing sets of arithmetically convenient values.
/// These will generally be sorted.
using ACVec = SmallVector<Constant *, 3>;

/// An ordering of Constants by their values.
///
/// This pass only handles floating point values for now; re-add integer values
/// if needed.
bool constantValueOrder(const Constant *A, const Constant *B) {
  const ConstantFP *AF, *BF;
  if ((AF = dyn_cast<ConstantFP>(A)) && (BF = dyn_cast<ConstantFP>(B))) {
    return AF->getValueAPF().compare(BF->getValueAPF()) == APFloat::cmpLessThan;
  }
  llvm_unreachable("Unexpected constant types");
}

/// A collection of relevant data for performing the multiversioning
/// transformation.
struct MVCandidate {

  /// The DDRef that will be multiversioned.
  const RegDDRef *Ref = nullptr;

  /// The multiversioning values that will be used.
  ACVec Values;

  /// The non-invariant outer loop level that will need a temp array, or 0 if a
  /// temp array is not needed.
  unsigned NonInvariantLevel = 0;

  /// Whether this is a valid candidate. If not, the checks failed and the
  /// queried DDRef is not eligible for multiversioning. The convention for this
  /// is that Ref should be nullptr when the candidate is invalid.
  explicit operator bool() const { return Ref; }
};

} // namespace

/// Determines whether the set of incoming edges for \p Ref in \p DDG is empty.
/// \todo Should this just be part of the DDGraph interface?
static bool incomingEmpty(const DDGraph &DDG, const DDRef *Ref) {
  return DDG.incoming_edges_begin(Ref) == DDG.incoming_edges_end(Ref);
}

/// Determines the arithmetically convenient values of type \p T for opcode
/// \p OPC.
///
/// For now, these are just 0.0 for fadd and -1.0, 0.0, and 1.0 for fmul. This
/// may be extended to other operations later as needed.
static ACVec getConvenientVals(unsigned OPC, Type *T) {
  switch (OPC) {
  case Instruction::FAdd:
    return {ConstantFP::get(T, 0.0)};
  case Instruction::FMul:
    return {
      ConstantFP::get(T, -1.0),
      ConstantFP::get(T, 0.0),
      ConstantFP::get(T, 1.0),
    };
  default:
    LLVM_DEBUG(dbgs() << "  Convenient values unknown for "
                      << Instruction::getOpcodeName(OPC) << "\n");
    return {};
  };
}

/// Computes the union of two sets of (sorted) arithmetically convenient values.
static ACVec getUnion(const ACVec &A, const ACVec &B) {
  if (A.empty())
    return B;
  if (B.empty())
    return A;
  ACVec U;
  std::set_union(std::begin(A), std::end(A), std::begin(B), std::end(B),
                 std::back_inserter(U), constantValueOrder);
  return U;
}

/// Computes the intersection of two sets of (sorted) values.
static ACVec getIntersection(const ACVec &A, const ACVec &B) {
  if (A.empty())
    return {};
  if (B.empty())
    return {};
  ACVec I;
  std::set_intersection(std::begin(A), std::end(A), std::begin(B), std::end(B),
                        std::back_inserter(I), constantValueOrder);
  return I;
}

/// Determines whether it is possible to hoist the loop header for this loop out
/// of its loop nest via cloneEmpty.
static bool canHoistLoopHeader(const HLLoop *Lp) {

  // The hoisting code can only handle do loops for now.
  // Add support for other types of loops as needed.
  if (!Lp->isDo())
    return false;

  // Make sure that the loop bounds/stride/ztt are invariant in the outer loop
  // levels.
  for (const RegDDRef *const Ref :
       make_range(Lp->ddref_begin(), Lp->ddref_end()))
    if (!Ref->isStructurallyInvariantAtLevel(1))
      return false;

  // The checks passed; we can hoist this loop header.
  return true;
}

/// Determines whether this pass is capable of hoisting a safety check for this
/// parent.
///
/// If it is not, we can't carry on with the multiversioning because we can't
/// prove that the loads hoisted to the check loop are safe to run.
static bool canHoistCheck(const HLNode *Parent) {

  // Only hoisting out of loops is currently supported.
  // Add support for ifs if needed.
  const auto *const ParentLp = dyn_cast<HLLoop>(Parent);
  if (!ParentLp) {
    LLVM_DEBUG(dbgs() << "Cannot hoist: found a non-loop parent\n");
    return false;
  }

  // Only do loops are supported.
  // Add support for other loop types if needed.
  if (!ParentLp->isDo()) {
    LLVM_DEBUG(dbgs() << "Cannot hoist: found a non-do parent loop at level "
                      << ParentLp->getNestingLevel() << "\n");
    return false;
  }

  // The ZTT refs must be invariant in all outer loop levels.
  for (const RegDDRef *const ZTTRef :
       make_range(ParentLp->ztt_ddref_begin(), ParentLp->ztt_ddref_end())) {
    if (!ZTTRef->isStructurallyInvariantAtLevel(1)) {
      LLVM_DEBUG(dbgs() << "Cannot hoist: outer loop ZTTs at level "
                        << ParentLp->getNestingLevel()
                        << " are not invariant\n");
      return false;
    }
  }

  // Everything passed; it is possible to hoist a check for this parent.
  return true;
}

/// Determines whether \p Ref is a candidate for row-wise multiversioning.
static MVCandidate checkCandidateDDRef(const RegDDRef *Ref, unsigned NestLevel,
                                       const DDGraph &DDG,
                                       DTransImmutableInfo &DTII) {

  // For this to be the case, it must be a load (Rval memory reference).
  if (Ref->isLval())
    return {};
  if (!Ref->isMemRef())
    return {};

  LLVM_DEBUG({
    Ref->print(fdbgs());
    dbgs() << "\n";
  });

  // Candidate array accesses must be not invariant at the innermost loop level
  // but invariant at all higher loop levels except for at most one (which will
  // eventually be handled with a temp array). If there are fewer than three
  // loop levels, bail without trying the temp array because there won't be any
  // re-use anyway.
  if (Ref->isStructurallyInvariantAtLevel(NestLevel)) {
    LLVM_DEBUG(dbgs() << "  Invariant in inner loop\n");
    return {};
  }
  const bool RejectNonInvariantOuters = NestLevel < 3;
  unsigned NonInvariantLevel          = 0;
  for (unsigned Level = NestLevel - 1; Level >= 1; --Level) {
    if (!Ref->isStructurallyInvariantAtLevel(Level, true)) {
      if (RejectNonInvariantOuters || NonInvariantLevel) {
        LLVM_DEBUG(dbgs() << "  Not invariant at loop level " << Level << "\n");
        return {};
      }

      NonInvariantLevel = Level;
      LLVM_DEBUG(dbgs() << "  Will use array for non-invariant level "
                        << NonInvariantLevel << "\n");
    }
  }

  // The array itself should also not be modified within this region.
  // Add support for modifications in outer loops if needed.
  if (!incomingEmpty(DDG, Ref)) {
    LLVM_DEBUG({
      dbgs() << "  Array is not read-only within the region:\n";
      for (const DDEdge *const Edge : DDG.incoming(Ref)) {
        dbgs() << "    ";
        Edge->print(dbgs());
      }
    });
    return {};
  }

  // Check for uses that have arithmetically convenient values.
  // Add support for more cases as needed.
  const auto *const Inst = dyn_cast<HLInst>(Ref->getHLDDNode());
  assert(Inst && "Only DDRefs from HLInsts supported so far");

  // If this is a load, add convenient values of its users instead.
  ACVec ACVals;
  if (Inst->getLLVMInstruction()->getOpcode() == Instruction::Load) {
    for (const DDEdge *const Edge : DDG.outgoing(Inst->getLvalDDRef())) {
      const auto *const UseRef = dyn_cast<RegDDRef>(Edge->getSink());
      if (!UseRef)
        continue;
      const auto *const UseInst = dyn_cast<HLInst>(UseRef->getHLDDNode());
      if (!UseInst)
        continue;
      const ACVec CurACVals = getConvenientVals(
        UseInst->getLLVMInstruction()->getOpcode(), Ref->getDestType());
      ACVals = getUnion(ACVals, CurACVals);
    }
  } else {
    ACVals = getConvenientVals(Inst->getLLVMInstruction()->getOpcode(),
                               Ref->getDestType());
  }

  if (ACVals.empty()) {
    LLVM_DEBUG(dbgs() << "  No arithmetically convenient values found\n");
    return {};
  }

  LLVM_DEBUG({
    dbgs() << "  Arithmetically convenient values found:\n";
    for (const Constant *const ACVal : ACVals)
      dbgs() << "    " << *ACVal << "\n";
  });

  // Filter the arithmetically convenient values using DTrans unless this should
  // be skipped.
  if (!SkipDTrans) {

    // See if the base address is a pointer loaded from a struct.
    if (!Ref->getHLDDNode()->getParentRegion()->isLiveIn(
          Ref->getBasePtrSymbase())) {
      LLVM_DEBUG(dbgs() << "  Base value is not live-in to the region\n");
      return {};
    }
    const auto *const BaseLd =
      dyn_cast_or_null<LoadInst>(Ref->getTempBaseValue());
    if (!BaseLd) {
      LLVM_DEBUG(dbgs() << "  Base value is not a load\n");
      return {};
    }
    const auto *const BaseGEP =
      dyn_cast<GetElementPtrInst>(BaseLd->getPointerOperand());
    if (!BaseGEP) {
      LLVM_DEBUG(dbgs() << "  Base load does not have a GEP\n");
      return {};
    }
    auto *const BaseStructType =
      dyn_cast<StructType>(BaseGEP->getSourceElementType());
    if (!BaseStructType) {
      LLVM_DEBUG(dbgs() << "  Base GEP type is not a struct: "
                        << *BaseGEP->getSourceElementType() << "\n");
      return {};
    }

    // Figure out which field is being accessed.
    const auto *const FieldIdxC = dyn_cast<ConstantInt>(BaseGEP->getOperand(2));
    if (!FieldIdxC) {
      report_fatal_error("Non-constant index in struct access??");
    }
    const unsigned FieldIdx = FieldIdxC->getLimitedValue();

    // Retrieve the possible values.
    const auto *const LikelyValues =
      DTII.getLikelyIndirectArrayConstantValues(BaseStructType, FieldIdx);
    if (!LikelyValues) {
      LLVM_DEBUG(dbgs() << "  No likely values found for struct "
                        << *BaseStructType << " field " << FieldIdx << "\n");
      return {};
    }

    // What are the possible values?
    LLVM_DEBUG({
      dbgs() << "  Possible values found in DTrans analysis:\n";
      for (const Constant *const IAC : *LikelyValues)
        dbgs() << "    " << *IAC << "\n";
    });

    // Use the possible values to filter the arithmetically convenient values.
    ACVec PossibleVals{std::begin(*LikelyValues), std::end(*LikelyValues)};
    sort(PossibleVals, constantValueOrder);
    ACVals = getIntersection(ACVals, PossibleVals);
  }

  if (ACVals.empty()) {
    LLVM_DEBUG(dbgs() << "  No overlap with possible array values\n");
    return {};
  }

  // Using an array for non-invariant outer loops isn't supported in this
  // version yet; bail now if this DDRef would require that.
  // TODO: Remove this check when this is supported.
  if (NonInvariantLevel) {
    LLVM_DEBUG(dbgs() << "  Temp array for non-invariant outer loops NYI\n");
    return {};
  }

  // This DDRef passes all of the checks, so it is a candidate.
  LLVM_DEBUG(dbgs() << "  Overlap values found; will process\n");
  return {Ref, ACVals, NonInvariantLevel};
}

/// Replaces all refs equivalent to \p OrigRef within \p Node with \p NewConst.
static void replaceAllEquivalentRefsWithConstant(HLNode *Node,
                                                 const RegDDRef *OrigRef,
                                                 Constant *NewConst) {

  // A visitor for visiting every node within Node and replacing DDRefs with
  // constants.
  struct Visitor final : HLNodeVisitorBase {
    const RegDDRef *const OrigRef;
    Constant *const NewConst;
    DDRefUtils &DDRU;
    Visitor(const RegDDRef *OrigRef, Constant *NewConst)
        : OrigRef{OrigRef}, NewConst{NewConst}, DDRU{OrigRef->getDDRefUtils()} {
    }

    void visit(HLDDNode *Node) {

      // If this node is a load inst, it needs some special handling to replace
      // it with a copy.
      if (auto *const Load = dyn_cast<HLInst>(Node)) {
        if (Load->getLLVMInstruction()->getOpcode() == Instruction::Load) {
          if (!DDRefUtils::areEqual(Load->getRvalDDRef(), OrigRef))
            return;
          HLInst *const Copy = Node->getHLNodeUtils().createCopyInst(
            DDRU.createConstDDRef(NewConst), "", Load->getLvalDDRef()->clone());
          Node->getHLNodeUtils().replace(Load, Copy);
          return;
        }
      }

      // Otherwise, just replace any operands that are equivalent to OrigRef.
      for (unsigned OpI = 0, OpE = Node->getNumOperands(); OpI != OpE; ++OpI) {
        if (DDRefUtils::areEqual(Node->getOperandDDRef(OpI), OrigRef)) {
          Node->setOperandDDRef(DDRU.createConstDDRef(NewConst), OpI);
        }
      }
    }

    void visit(HLNode *) {}
    void postVisit(HLNode *) {}
  };

  Visitor HV{OrigRef, NewConst};
  HLNodeUtils::visit(HV, Node);
}

/// Performs the multiversioning transform on \p Lp for the multiversioning
/// candidate \p MVCand.
static void multiversionLoop(HLLoop *Lp, const MVCandidate &MVCand) {
  assert(MVCand);
  const RegDDRef *const MVRef   = MVCand.Ref;
  const ACVec &MVVals           = MVCand.Values;
  HLNodeUtils &HNU              = Lp->getHLNodeUtils();
  DDRefUtils &DDRU              = Lp->getDDRefUtils();
  HLLoop *const OutermostParent = Lp->getOutermostParentLoop();

  LLVM_DEBUG({
    dbgs() << "Multiversioning on ";
    MVRef->print(fdbgs());
    dbgs() << "\n";
  });

  // Create an empty clone of the inner loop outside of the loop nest to start
  // the check loop:
  //
  // + DO i1 = 0, %M + -1, 1    <DO_LOOP>
  // + END LOOP
  //
  // + DO i1 = 0, %N + -1, 1   <DO_LOOP>
  // |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
  // |   |   %Aijbj = (%A)[%M * i1 + i2]  *  (%b)[i2];
  // |   |   %sum = %sum  +  %Aijbj;
  // |   + END LOOP
  // + END LOOP
  HLLoop *const CheckLoop = Lp->cloneEmpty();
  HLNodeUtils::insertBefore(OutermostParent, CheckLoop);

  // Add ZTTs from the outer loops to make sure the accesses are safe:
  //
  // + Ztt: if (%M > 0 && %N > 0)
  // + DO i1 = 0, %M + -1, 1    <DO_LOOP>
  // + END LOOP
  //
  // + DO i1 = 0, %N + -1, 1   <DO_LOOP>
  // |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
  // |   |   %Aijbj = (%A)[%M * i1 + i2]  *  (%b)[i2];
  // |   |   %sum = %sum  +  %Aijbj;
  // |   + END LOOP
  // + END LOOP
  const HLLoop *ParentLp = Lp;
  while (ParentLp = ParentLp->getParentLoop()) {
    if (!ParentLp->hasZtt())
      continue;
    for (auto ZTTIter = ParentLp->ztt_pred_begin(),
              ZTTEnd  = ParentLp->ztt_pred_end();
         ZTTIter != ZTTEnd; ++ZTTIter) {
      const RegDDRef *const Left =
        ParentLp->getZttPredicateOperandDDRef(ZTTIter, true);
      const RegDDRef *const Right =
        ParentLp->getZttPredicateOperandDDRef(ZTTIter, false);
      CheckLoop->addZttPredicate(*ZTTIter, Left->clone(), Right->clone());
    }
  }

  // Add a load of the DDRef chosen for multiversioning:
  //
  // + DO i1 = 0, %M + -1, 1    <DO_LOOP>
  // |   %rwmv.next = (%b)[i1];
  // + END LOOP
  RegDDRef *const CheckRef = MVRef->clone();
  CheckRef->getSingleCanonExpr()->replaceIV(Lp->getNestingLevel(), 1);
  HLInst *const NextLoad = HNU.createLoad(CheckRef, "rwmv.next");
  HLNodeUtils::insertAsLastChild(CheckLoop, NextLoad);
  const RegDDRef *const Next = NextLoad->getLvalDDRef();

  // Peel the first iteration of the loop to load the first value of the array:
  //
  // if (%M > 0 && %N > 0)
  // {
  //    %rwmv.first = (%b)[0];
  //
  //    + DO i1 = 0, %M + -2, 1    <DO_LOOP>
  //    |   %rwmv.next = (%b)[i1 + 1];
  //    + END LOOP
  // }
  const HLNode *const PrePeelParent = CheckLoop->getParent();
  HLLoop *const PeelLoop            = CheckLoop->peelFirstIteration();
  auto *const FirstLoad             = cast<HLInst>(PeelLoop->getFirstChild());
  HNU.createAndReplaceTemp(FirstLoad->getLvalDDRef(), "rwmv.first");
  const RegDDRef *const First = FirstLoad->getLvalDDRef();
  PeelLoop->replaceByFirstIteration();

  // Sometimes the process of peeling the loop introduces a new parent if for
  // the check loop. If this happens, keep track of that if to add things
  // outside of it if needed.
  HLIf *OuterIf = nullptr;
  if (CheckLoop->getParent() != PrePeelParent) {
    OuterIf = dyn_cast<HLIf>(CheckLoop->getParent());
    assert(OuterIf);
    assert(OuterIf->getParent() == PrePeelParent);
  }

  // Add the comparison to check that each value in the array is equal to the
  // first:
  //
  // if (%M > 0 && %N > 0)
  // {
  //    %rwmv.first = (%b)[0];
  //
  //    + DO i1 = 0, %M + -2, 1    <DO_LOOP>
  //    |   %rwmv.next = (%b)[i1 + 1];
  //    |   if (%rwmv.next !=u %rwmv.first)
  //    |   {
  //    |   }
  //    + END LOOP
  // }
  HLIf *const CheckIf =
    HNU.createHLIf(CmpInst::FCMP_UNE, Next->clone(), First->clone());
  HLNodeUtils::insertAsLastChild(CheckLoop, CheckIf);
  CheckLoop->addLiveInTemp(First->getSymbase());

  // Add a goto to exit the loop when the check fails:
  //
  // if (%M > 0 && %N > 0)
  // {
  //    %rwmv.first = (%b)[0];
  //
  //    + DO i1 = 0, %M + -2, 1    <DO_MULTI_EXIT_LOOP>
  //    |   %rwmv.next = (%b)[i1 + 1];
  //    |   if (%rwmv.next !=u %rwmv.first)
  //    |   {
  //    |      goto RWMVCheckFailed;
  //    |   }
  //    + END LOOP
  //
  //    RWMVCheckFailed:
  // }
  HLLabel *const FailedLabel = HNU.createHLLabel("RWMVCheckFailed");
  HLNodeUtils::insertAfter(CheckLoop, FailedLabel);
  HLGoto *const FailedGoto = HNU.createHLGoto(FailedLabel);
  HLNodeUtils::insertAsLastThenChild(CheckIf, FailedGoto);
  CheckLoop->setNumExits(2);

  // Initialize the multiversioning case to 0 (no special value):
  //
  // %rwmv.rowcase = 0;
  // if (%M > 0 && %N > 0)
  // {
  //    %rwmv.first = (%b)[0];
  //
  //    + DO i1 = 0, %M + -2, 1    <DO_MULTI_EXIT_LOOP>
  //    |   %rwmv.next = (%b)[i1 + 1];
  //    |   if (%rwmv.next !=u %rwmv.first)
  //    |   {
  //    |      goto RWMVCheckFailed;
  //    |   }
  //    + END LOOP
  //
  //    RWMVCheckFailed:
  // }
  Type *const RowCaseTy = IntegerType::getInt8Ty(HNU.getContext());
  HLInst *const RowCaseInit =
    HNU.createCopyInst(DDRU.createConstDDRef(RowCaseTy, 0), "rwmv.rowcase");
  if (OuterIf)
    HLNodeUtils::insertBefore(OuterIf, RowCaseInit);
  else
    HLNodeUtils::insertBefore(CheckLoop, RowCaseInit);
  const RegDDRef *const RowCase = RowCaseInit->getLvalDDRef();
  for (HLLoop *Parent = Lp->getParentLoop(); Parent != nullptr;
       Parent         = Parent->getParentLoop()) {
    Parent->addLiveInTemp(RowCase->getSymbase());
  }

  // Check for identified multiversion values:
  //
  // %rwmv.rowcase = 0;
  // if (%M > 0 && %N > 0)
  // {
  //    %rwmv.first = (%b)[0];
  //
  //    + DO i1 = 0, %M + -2, 1    <DO_MULTI_EXIT_LOOP>
  //    |   %rwmv.next = (%b)[i1 + 1];
  //    |   if (%rwmv.next !=u %rwmv.first)
  //    |   {
  //    |      goto RWMVCheckFailed;
  //    |   }
  //    + END LOOP
  //
  //    if (%rwmv.first == -1.0)
  //    {
  //       %rwmv.rowcase = 1;
  //    }
  //    else
  //    {
  //       if (%rwmv.first == 0.0)
  //       {
  //          %rwmv.rowcase = 2;
  //       }
  //    }
  //    RWMVCheckFailed:
  // }
  HLIf *PrevIf = nullptr;
  for (size_t MVInd = 0, MVEnd = MVVals.size(); MVInd != MVEnd; ++MVInd) {
    HLIf *const CurIf = HNU.createHLIf(CmpInst::FCMP_OEQ, First->clone(),
                                       DDRU.createConstDDRef(MVVals[MVInd]));
    if (PrevIf)
      HLNodeUtils::insertAsLastElseChild(PrevIf, CurIf);
    else
      HLNodeUtils::insertAfter(CheckLoop, CurIf);
    HLInst *const RowCaseUpdate = HNU.createCopyInst(
      DDRU.createConstDDRef(RowCaseTy, MVInd + 1), "", RowCase->clone());
    HLNodeUtils::insertAsLastThenChild(CurIf, RowCaseUpdate);
    PrevIf = CurIf;
  }

  // Construct the switch for the multiversioning:
  //
  // + DO i1 = 0, %N + -1, 1   <DO_LOOP>
  // |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
  // |   |   %Aijbj = (%A)[%M * i1 + i2]  *  (%b)[i2];
  // |   |   %sum = %sum  +  %Aijbj;
  // |   + END LOOP
  // |   switch(%rwmv.rowcase)
  // |   {
  // |   }
  // + END LOOP
  HLSwitch *const MVSwitch = HNU.createHLSwitch(RowCase->clone());
  HLNodeUtils::insertAfter(Lp, MVSwitch);

  // The unmodified loop will be the default case:
  //
  // + DO i1 = 0, %N + -1, 1   <DO_LOOP>
  // |   switch(%rwmv.rowcase)
  // |   {
  // |   default:
  // |      + DO i2 = 0, %M + -1, 1   <DO_LOOP>
  // |      |   %Aijbj = (%A)[%M * i1 + i2]  *  (%b)[i2];
  // |      |   %sum = %sum  +  %Aijbj;
  // |      + END LOOP
  // |      break;
  // |   }
  // + END LOOP
  HLNodeUtils::moveAsLastDefaultChild(MVSwitch, Lp);

  // Clone the loop for each other case:
  //
  // + DO i1 = 0, %N + -1, 1   <DO_LOOP>
  // |   switch(%rwmv.rowcase)
  // |   {
  // |   case 1:
  // |      + DO i2 = 0, %M + -1, 1   <DO_LOOP>
  // |      |   %Aijbj = (%A)[%M * i1 + i2]  *  -1.0;
  // |      |   %sum = %sum  +  %Aijbj;
  // |      + END LOOP
  // |      break;
  // |   case 2:
  // |      + DO i2 = 0, %M + -1, 1   <DO_LOOP>
  // |      |   %Aijbj = (%A)[%M * i1 + i2]  *  0.0;
  // |      |   %sum = %sum  +  %Aijbj;
  // |      + END LOOP
  // |      break;
  // |   default:
  // |      + DO i2 = 0, %M + -1, 1   <DO_LOOP>
  // |      |   %Aijbj = (%A)[%M * i1 + i2]  *  (%b)[i2];
  // |      |   %sum = %sum  +  %Aijbj;
  // |      + END LOOP
  // |      break;
  // |   }
  // + END LOOP
  for (size_t MVInd = 0, MVEnd = MVVals.size(); MVInd != MVEnd; ++MVInd) {
    HLLoop *const MVLoop = Lp->clone();
    MVSwitch->addCase(DDRU.createConstDDRef(RowCaseTy, MVInd + 1));
    const unsigned CaseNum = MVSwitch->getNumCases();
    assert(MVSwitch->getConstCaseValue(CaseNum) == int64_t(MVInd + 1));
    HLNodeUtils::insertAsLastChild(MVSwitch, MVLoop, CaseNum);

    // Replace all uses of the loaded value within the loop with the constant.
    replaceAllEquivalentRefsWithConstant(MVLoop, MVRef, MVVals[MVInd]);
  }

  HIRInvalidationUtils::invalidateBody(Lp->getParentLoop());
}

bool HIRRowWiseMV::run(HLLoop *Lp) {

  // Ignore any loops with no outer loops: this transformation is not likely to
  // be profitable for those.
  if (Lp->getNestingLevel() == 1)
    return false;

  LLVM_DEBUG({
    dbgs() << "Examining loop:\n";
    Lp->dump();
  });

  // We need to be able to hoist the header of this loop for the check loop.
  if (!canHoistLoopHeader(Lp)) {
    LLVM_DEBUG(dbgs() << "Cannot hoist loop header for check loop\n");
    return false;
  }

  // TODO: Add something to evaluate whether the peeling trick might be a good
  // idea here. Until then, use the following legality checks.

  // And also it needs to be safe to hoist checks for all of the parents within
  // the loop.
  const HLLoop *const OutermostParent = Lp->getOutermostParentLoop();
  for (const HLNode *Parent      = Lp->getParent(),
                    *ParentE     = OutermostParent->getParent();
       Parent != ParentE; Parent = Parent->getParent())
    if (!canHoistCheck(Parent))
      return false;

  // Also check that there are no forward gotos in the loop nest.
  //
  // Really, only the gotos that jump over an inner loop that will be hoisted
  // are a problem, but it's easier to be more conservative and avoid having any
  // forward gotos at all. Fancier checking for only the problematic gotos can
  // be added later if needed, and should probably be done by checking that each
  // child HLNode post-dominates the corresponding branch of its parent.
  if (HLS.getTotalLoopStatistics(OutermostParent).hasForwardGotos()) {
    LLVM_DEBUG(dbgs() << "Avoiding this loop because there are forward gotos "
                         "in the loop nest\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "Loads:\n");

  const DDGraph DDG = HDDA.getGraph(Lp->getParentRegion());

  // Take a look through the enclosed instructions and collect candidate DDRefs.
  // Add support for ifs/switches if needed.
  SmallVector<MVCandidate, 1> MVCands;
  ACVec ACVals;
  for (const HLNode &C : make_range(Lp->child_begin(), Lp->child_end()))
    if (const auto *const CI = dyn_cast<HLInst>(&C))
      for (const RegDDRef *const Ref :
           make_range(CI->ddref_begin(), CI->ddref_end()))
        if (const MVCandidate MVCand =
              checkCandidateDDRef(Ref, Lp->getNestingLevel(), DDG, DTII))
          MVCands.push_back(MVCand);

  LLVM_DEBUG(dbgs() << "\n");

  if (MVCands.empty()) {
    LLVM_DEBUG(
      dbgs() << "No candidate array accesses identified for multiversioning\n");
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "Identified these candidates for multiversioning:\n";
    for (const MVCandidate &MVCand : MVCands) {
      dbgs() << "  ";
      MVCand.Ref->print(fdbgs());
      dbgs() << ":\n";
      for (const Constant *const Val : MVCand.Values)
        dbgs() << "    " << *Val << "\n";
      dbgs() << "\n";
    }
  });

  // We may have found multiple candidates, but doing a combinatorial
  // multiversioning here for all of them is probably not helpful. For now, just
  // do the first one. Later, we may want to have fancier logic to choose one.
  const MVCandidate &ChosenCand = MVCands.front();

  multiversionLoop(Lp, ChosenCand);

  return true;
}

/// Performs row-wise multiversioning using the given analysis results.
static bool runRowWiseMV(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
                         HIRLoopStatistics &HLS, DTransImmutableInfo &DTII) {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << OPT_DESC " Disabled\n");
    return false;
  }

  HIRRowWiseMV RWMV{HDDA, HLS, DTII};

  // Attempt row-wise multiversioning on innermost loops.
  SmallVector<HLLoop *, 16> CandLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(CandLoops);
  bool Changed = false;
  for (HLLoop *const Lp : CandLoops) {
    if (RWMV.run(Lp))
      Changed = true;
  }

  return Changed;
}

char HIRRowWiseMVLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRRowWiseMVLegacyPass, OPT_SWITCH, OPT_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_END(HIRRowWiseMVLegacyPass, OPT_SWITCH, OPT_DESC, false, false)

FunctionPass *llvm::createHIRRowWiseMVPass() {
  return new HIRRowWiseMVLegacyPass{};
}

bool HIRRowWiseMVLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    return false;
  }

  return runRowWiseMV(
    getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
    getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
    getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS(),
    getAnalysis<DTransImmutableAnalysisWrapper>().getResult());
}

PreservedAnalyses HIRRowWiseMVPass::run(Function &F,
                                        llvm::FunctionAnalysisManager &AM) {
  runRowWiseMV(AM.getResult<HIRFrameworkAnalysis>(F),
               AM.getResult<HIRDDAnalysisPass>(F),
               AM.getResult<HIRLoopStatisticsAnalysis>(F),
               AM.getResult<DTransImmutableAnalysis>(F));
  return PreservedAnalyses::all();
}
