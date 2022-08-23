//===-------------------------- HIRRowWiseMV.cpp --------------------------===//
//
// Copyright (C) 2019-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements row-wise multiversioning for arrays where entire rows
/// may be "arithmetically convenient" ones that allow for strength reduction
/// (such as 0 for addition and -1, 0, and 1 for multiplication).
///
/// For example:
///
/// \code
/// DO i = ...
///   DO j = 0, N, 1
///     ... A[j] * B ...
///   END DO
/// END DO
///
/// ===>
///
/// first = A[0]
/// rowcase = 0;
/// allclose = 1;
/// DO j = 0, N - 1, 1
///   if (fabs(A[j + 1] - first) > 0.0001)
///   {
///     allclose = 0;
///   }
/// END DO
///
/// if (allclose != 0)
/// {
///   if (first == 0)
///   {
///     rowcase = 1;
///   }
///   else
///   {
///     if (first == 1)
///     {
///       rowcase = 2;
///     }
///   }
/// }
///
/// CheckFailed:
/// DO i = ...
///   switch(rowcase)
///   {
///   case 1:
///     DO j = 0, N, 1
///        ... 0 * B ...
///     END DO
///   case 2:
///     DO j = 0, N, 1
///        ... 1 * B ...
///     END DO
///   default:
///     DO j = 0, N, 1
///        ... A[j] * B ...
///     END DO
///   }
/// END DO
/// \endcode
///
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRRowWiseMVPass.h"

#if INTEL_FEATURE_SW_DTRANS
#include "Intel_DTrans/Analysis/DTransFieldModRef.h"
#include "Intel_DTrans/Analysis/DTransImmutableAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#endif // INTEL_FEATURE_SW_DTRANS
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

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

static cl::opt<double> CheckTolerance{
  OPT_SWITCH "-tolerance", cl::init(0.0001), cl::Hidden,
  cl::desc("The tolerance used by RWMV for checking that values in a row are "
           "approximately equal")};
static cl::opt<unsigned> ApplyLimit{
    OPT_SWITCH "-per-function", cl::init(2), cl::Hidden,
    cl::desc(
        "The maximum number of times RWMV is allowed to apply per function")};

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
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
#if INTEL_FEATURE_SW_DTRANS
    AU.addRequiredTransitive<DTransImmutableAnalysisWrapper>();
    AU.addRequiredTransitive<DTransFieldModRefResultWrapper>();
#endif // INTEL_FEATURE_SW_DTRANS
    AU.setPreservesAll();
  }
};

/// Implements the row-wise multiversioning transform.
class HIRRowWiseMV {
  HIRDDAnalysis &HDDA;
#if INTEL_FEATURE_SW_DTRANS
  DTransImmutableInfo *DTII;
  FieldModRefResult *FieldModRef;
#endif // INTEL_FEATURE_SW_DTRANS
  HIRLoopStatistics &HLS;

public:
  HIRRowWiseMV(HIRDDAnalysis &HDDA,
#if INTEL_FEATURE_SW_DTRANS
               DTransImmutableInfo *DTII, FieldModRefResult *FieldModRef,
#endif // INTEL_FEATURE_SW_DTRANS
                HIRLoopStatistics &HLS)
      : HDDA{HDDA},
#if INTEL_FEATURE_SW_DTRANS
        DTII{DTII}, FieldModRef{FieldModRef},
#endif // INTEL_FEATURE_SW_DTRANS
        HLS{HLS} {}

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

/// Determines whether \p Inst has enough fast math flags to allow the
/// approximate checks used by this pass. None of the individual fast-math flags
/// are really a good fit for this, but the "unsafe-fp-math" function attribute
/// (which we also require for this transform) is broad enough to cover this.
/// The clang driver sets this when all of afn, arecip, ressoc, and nsz are
/// enabled, so these four flags seem like a reasonable set to check here.
static bool allowsApproxChecks(const HLInst *Inst) {
  const auto *const FPOp = dyn_cast<FPMathOperator>(Inst->getLLVMInstruction());
  if (!FPOp)
    return false;
  return FPOp->hasApproxFunc() && FPOp->hasAllowReciprocal() &&
         FPOp->hasAllowReassoc() && FPOp->hasNoSignedZeros();
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

  // The loop also has to be normalized: this is required to be able to use
  // peelFirstIteration on the check loop, and for the temp array loop it is
  // required because support for non-normalized loops hasn't been implemented
  // by this pass yet.
  if (!Lp->isNormalized())
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

/// Compiles a list of all uses of a load \p Load according to \p DDG as long as
/// each of those uses is replaceable during multiversioning by HIRRowWiseMV. If
/// any are not, this function returns an empty vector.
static SmallVector<RegDDRef *, 3> getLoadUses(const HLInst *Load,
                                              const DDGraph &DDG) {
  assert(Load->getLLVMInstruction()->getOpcode() == Instruction::Load);

  // Avoid this load if it is live out of its containing loop.
  if (Load->getParentLoop()->isLiveOut(Load->getLvalDDRef()->getSymbase()))
    return {};

  SmallVector<RegDDRef *, 3> Uses;
  for (const DDEdge *const Edge : DDG.outgoing(Load->getLvalDDRef())) {

    // Edges that don't lead to RegDDRefs in HLInsts are not supported.
    auto *const UseRef = dyn_cast<RegDDRef>(Edge->getSink());
    if (!UseRef)
      return {};
    const auto *const UseInst = dyn_cast<HLInst>(UseRef->getHLDDNode());
    if (!UseInst)
      return {};

    // Give up if there are any other lvalue refs.
    if (UseRef->isLval())
      return {};

    // Also give up if there are instructions not dominated by the load.
    if (!HLNodeUtils::dominates(Load, UseInst))
      return {};

    // All of the checks passed; this should be a safe use.
    Uses.push_back(UseRef);
  }

  return Uses;
}

/// Determines whether \p Ref is a candidate for row-wise multiversioning.
static MVCandidate checkCandidateDDRef(const RegDDRef *Ref, const HLLoop *Lp,
                                       const HLLoop *SafeCheckLevelParent,
                                       HIRDDAnalysis &HDDA,
#if INTEL_FEATURE_SW_DTRANS
                                       DTransImmutableInfo *DTII,
                                       FieldModRefResult *FieldModRef,
#endif // INTEL_FEATURE_SW_DTRANS
                                       HIRLoopStatistics &HLS) {

  // For this to be the case, it must be a load (Rval memory reference).
  if (Ref->isLval())
    return {};
  if (!Ref->isMemRef())
    return {};

  LLVM_DEBUG({
    Ref->print(fdbgs());
    dbgs() << "\n";
  });

  // For now, the ref is required to have a single CanonExpr.
  // Update the transformation to handle multi-dimensional refs if needed.
  if (!Ref->isSingleDimension()) {
    LLVM_DEBUG(dbgs() << "  Ref has multile CanonExprs\n");
    return {};
  }

  // Candidate array accesses must be linear at the loop level they'll be
  // hoisted to because this pass does not yet hoist blobs.
  unsigned SafeCheckLevel = SafeCheckLevelParent->getNestingLevel();
  if (!Ref->isLinearAtLevel(SafeCheckLevel)) {
    LLVM_DEBUG(dbgs() << "  Not linear at level " << SafeCheckLevel << "\n");
    return {};
  }

  // Candidate array accesses must be not invariant at the innermost loop level
  // but invariant at all higher loop levels except for at most one (which will
  // eventually be handled with a temp array). If there are fewer than three
  // loop levels, bail without trying the temp array because there won't be any
  // re-use anyway. These checks only concern IVs because blobs always required
  // to be invariant and that is checked below as part of the isLoopInvariant
  // call.
  const unsigned NestLevel = Lp->getNestingLevel();
  if (!Ref->hasIV(NestLevel)) {
    LLVM_DEBUG(dbgs() << "  Invariant in inner loop\n");
    return {};
  }
  const bool RejectNonInvariantOuters = NestLevel < 3;
  unsigned NonInvariantLevel          = 0;
  for (unsigned Level = NestLevel - 1; Level >= 1; --Level) {
    if (Ref->hasIV(Level)) {
      if (RejectNonInvariantOuters || NonInvariantLevel) {
        LLVM_DEBUG(dbgs() << "  Not invariant at loop level " << Level << "\n");
        return {};
      }

      NonInvariantLevel = Level;
      LLVM_DEBUG(dbgs() << "  Will use array for non-invariant level "
                        << NonInvariantLevel << "\n");
    }
  }

  // Carry out extra checks on the non-invariant outer loop if one was found.
  if (NonInvariantLevel) {
    const HLLoop *const NonInvariantLoop =
      Ref->getParentLoop()->getParentLoopAtLevel(NonInvariantLevel);

    // If the non-invariant outer loop can't actually be hoisted, we can't use
    // the temp array for it.
    if (!canHoistLoopHeader(NonInvariantLoop)) {
      LLVM_DEBUG(dbgs() << "  Unable to use array: parent loop at level "
                        << NonInvariantLevel << " can't be hoisted \n");
      return {};
    }
  }

  // The array itself should also not be modified within the loop nest.
  // Add support for modifications in outer loops if needed.
  const HLLoop *const OutermostParent =
    Ref->getParentLoop()->getOutermostParentLoop();
  if (!HIRTransformUtils::isLoopInvariant(Ref, OutermostParent, HDDA, HLS,
#if INTEL_FEATURE_SW_DTRANS
                                          FieldModRef,
#endif // INTEL_FEATURE_SW_DTRANS
                                          true)) {
    LLVM_DEBUG({
      dbgs() << "  Array is not read-only within the loop nest:\n";
      for (const DDEdge *const Edge :
           HDDA.getGraph(OutermostParent).incoming(Ref)) {
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
    for (const RegDDRef *const UseRef : getLoadUses(Inst, HDDA.getGraph(Lp))) {
      const HLInst *const UseInst = cast<HLInst>(UseRef->getHLDDNode());
      if (!allowsApproxChecks(UseInst)) {
        LLVM_DEBUG({
          dbgs() << "  Used by instruction that does not allow approximate "
                    "values:\n    ";
          UseInst->print(fdbgs(), 0);
        });
        return {};
      }
      const ACVec CurACVals = getConvenientVals(
        UseInst->getLLVMInstruction()->getOpcode(), Ref->getDestType());
      ACVals = getUnion(ACVals, CurACVals);
    }
  } else {
    if (!allowsApproxChecks(Inst)) {
      LLVM_DEBUG({
        dbgs() << "  Used by instruction that does not allow approximate "
                  "values:\n    ";
        Inst->print(fdbgs(), 0);
      });
      return {};
    }
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

#if INTEL_FEATURE_SW_DTRANS
  // Filter the arithmetically convenient values using DTrans unless this should
  // be skipped.
  if (!SkipDTrans && DTII) {

    // See if the base address is a pointer loaded from a struct.
    const RegDDRef *const BaseLoadRef = DDUtils::getSingleBasePtrLoadRef(
        HDDA.getGraph(OutermostParent->getParentRegion()), Ref);
    if (!BaseLoadRef) {
      LLVM_DEBUG(dbgs() << "  Could not find base load\n");
      return {};
    }
    bool IsPrecise = false;
    const auto *const BaseGEP =
      dyn_cast<GetElementPtrInst>(BaseLoadRef->getLocationPtr(IsPrecise));
    if (!BaseGEP) {
      LLVM_DEBUG(dbgs() << "  Base load does not have a GEP\n");
      return {};
    }
    if (!IsPrecise) {
      LLVM_DEBUG(dbgs() << "  Base load location was not precise\n");
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
        DTII->getLikelyIndirectArrayConstantValues(BaseStructType, FieldIdx);
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
#endif // INTEL_FEATURE_SW_DTRANS

  if (ACVals.empty()) {
    LLVM_DEBUG(dbgs() << "  No overlap with possible array values\n");
    return {};
  }

  // This DDRef passes all of the checks, so it is a candidate.
  LLVM_DEBUG(dbgs() << "  Overlap values found; will process\n");
  return {Ref, ACVals, NonInvariantLevel};
}

/// Replaces all refs equivalent to \p OrigRef within \p Node with \p NewConst.
static void replaceAllEquivalentRefsWithConstant(HLNode *Node,
                                                 const RegDDRef *OrigRef,
                                                 Constant *NewConst,
                                                 const DDGraph &DDG) {

  // A visitor for visiting every node within Node and replacing DDRefs with
  // constants.
  struct Visitor final : HLNodeVisitorBase {
    const RegDDRef *const OrigRef;
    Constant *const NewConst;
    DDRefUtils &DDRU;
    const DDGraph &DDG;
    Visitor(const RegDDRef *OrigRef, Constant *NewConst, const DDGraph &DDG)
        : OrigRef{OrigRef}, NewConst{NewConst}, DDRU{OrigRef->getDDRefUtils()},
          DDG{DDG} {}

    void visit(HLDDNode *Node) {

      // If this node is a relevant load inst, replace its uses and delete the
      // original load.
      if (auto *const Load = dyn_cast<HLInst>(Node)) {
        if (Load->getLLVMInstruction()->getOpcode() == Instruction::Load) {
          if (!DDRefUtils::areEqual(Load->getRvalDDRef(), OrigRef))
            return;
          bool UsesReplaced = false;
          for (RegDDRef *const UseRef : getLoadUses(Load, DDG)) {
            UseRef->getHLDDNode()->replaceOperandDDRef(
              UseRef, DDRU.createConstDDRef(NewConst));
            UsesReplaced = true;
          }
          if (UsesReplaced)
            HLNodeUtils::remove(Load);
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

  Visitor HV{OrigRef, NewConst, DDG};
  HLNodeUtils::visit(HV, Node);
}

// Replace following pattern:
//
//  %t = - (%B)[i1 + 32 * i3];
//  %sum = %sum  +  %t;
//
//  with >>>
//
//  %sum = %sum - (%B)[i1 + 32 * i3];
//
// Removal %t iff no DDedges in loop and is not live in/out
//
// This is the HIR equivalent of the "(-X) + Y --> Y - X" peephole in
// InstCombinerImpl::visitFAdd; as in InstCombine, no fast math flags are needed
// to perform this peephole.

static void applyPeepHole(HLLoop *Loop, HIRDDAnalysis &DDA) {
  DenseMap<unsigned, HLInst *> FNegCandidates;
  SmallVector<std::pair<HLInst *, HLInst *>, 4> CandidatePairs;

  for (auto Iter = Loop->child_begin(), End = Loop->child_end(); Iter != End;
       ++Iter) {
    HLInst *Inst = dyn_cast<HLInst>(Iter);
    if (!Inst) {
      continue;
    }

    const Instruction *LLVMInst = Inst->getLLVMInstruction();
    const RegDDRef *LvalRef = Inst->getLvalDDRef();

    // Find Fneg inst: t = - (%B)[i1];
    if (LLVMInst->getOpcode() == Instruction::FNeg) {
      unsigned SB = LvalRef->getSymbase();
      bool OutsideUses = Loop->isLiveOut(SB) || Loop->isLiveIn(SB);

      if (!OutsideUses && LvalRef->isSelfBlob()) {
        FNegCandidates[LvalRef->getSelfBlobIndex()] = Inst;
        continue;
      }
    }

    // Find Fadd inst: %sum = %sum  +  %t;
    if (LLVMInst->getOpcode() == Instruction::FAdd && !FNegCandidates.empty()) {
      unsigned TempOpNum = 0;
      if (DDRefUtils::areEqual(LvalRef, Inst->getOperandDDRef(1))) {
        TempOpNum = 2;
      } else if (DDRefUtils::areEqual(LvalRef, Inst->getOperandDDRef(2))) {
        TempOpNum = 1;
      } else {
        continue;
      }

      auto *AddTemp = Inst->getOperandDDRef(TempOpNum);
      if (!AddTemp->isSelfBlob()) {
        continue;
      }

      unsigned TempIndex = AddTemp->getSelfBlobIndex();
      auto It = FNegCandidates.find(TempIndex);
      if (It == FNegCandidates.end()) {
        continue;
      }

      auto *FNegInst = It->second;
      CandidatePairs.push_back(std::make_pair(FNegInst, Inst));
    }
  }

  if (CandidatePairs.empty()) {
    return;
  }

  HIRInvalidationUtils::invalidateBody(Loop);
  DDGraph DDG = DDA.getGraph(Loop);

  // Check for DDEdge between Inst Pair and do replacement
  for (auto Pair : CandidatePairs) {
    HLInst *FNeg = Pair.first;
    HLInst *FAdd = Pair.second;

    const RegDDRef *FNegLval = FNeg->getLvalDDRef();
    if (DDG.getNumOutgoingEdges(FNegLval) != 1) {
      continue;
    }

    // Bailout if expected edge doesn't point to FAdd
    DDEdge *Edge = *DDG.outgoing_edges_begin(FNegLval);
    if (!Edge->isFlow() || Edge->getSink()->getHLDDNode() != FAdd) {
      continue;
    }

    auto *NegRef = FNeg->removeRvalDDRef();
    unsigned OpNum =
        DDRefUtils::areEqual(FAdd->getLvalDDRef(), FAdd->getOperandDDRef(1))
            ? 1
            : 2;

    FastMathFlags SubFastMathFlags = FAdd->getFastMathFlags();
    SubFastMathFlags &= FNeg->getFastMathFlags();
    HLInst *const NewFSub = FAdd->getHLNodeUtils().createFPMathBinOp(
        Instruction::FSub, FAdd->removeOperandDDRef(OpNum), NegRef,
        SubFastMathFlags, "", FAdd->removeLvalDDRef());

    HLNodeUtils::replace(FAdd, NewFSub);
    HLNodeUtils::remove(FNeg);
  }
}

/// Performs the multiversioning transform on \p Lp for the multiversioning
/// candidate \p MVCand, putting the check loop just outside of \p
/// SafeCheckLevelParent.
static void multiversionLoop(HLLoop *Lp, const MVCandidate &MVCand,
                             HLLoop *SafeCheckLevelParent,
#if INTEL_FEATURE_SW_DTRANS
                             DTransImmutableInfo *DTII,
#endif // INTEL_FEATURE_SW_DTRANS
                             HIRDDAnalysis &HDDA) {
  assert(MVCand);
  const RegDDRef *const MVRef   = MVCand.Ref;
  const ACVec &MVVals           = MVCand.Values;
  const unsigned MVNonInvariantLevel = MVCand.NonInvariantLevel;
  HLNodeUtils &HNU              = Lp->getHLNodeUtils();
  DDRefUtils &DDRU              = Lp->getDDRefUtils();
  CanonExprUtils &CEU                = Lp->getCanonExprUtils();
  HLLoop *const OutermostParent = Lp->getOutermostParentLoop();
#if INTEL_INTERNAL_BUILD
  OptReportBuilder &ORBuilder = HNU.getHIRFramework().getORBuilder();
#endif
  assert(OutermostParent && "Lp should not be the outermost loop");

  LLVM_DEBUG({
    dbgs() << "Multiversioning on ";
    MVRef->print(fdbgs());
    dbgs() << "\n";
  });

  // If it isn't safe to hoist the check loop all of the way out of the loop
  // nest, construct an if at the right place inside the loop nest and set up
  // NeedCheck to ensure that the check loop is run only once per entry into the
  // loop nest:
  //
  //    %rwmv.needcheck = 1;
  // + DO i1 = 0, %N1 + -1, 1   <DO_LOOP>
  // |   if (%M1 > 0)
  // |   {
  // |      if (%rwmv.needcheck != 0)
  // |      {
  // |         %rwmv.needcheck = 0;
  // |      }
  // |
  // |      + DO i2 = 0, %M1 + -1, 1   <DO_LOOP>
  // |      |   + DO i3 = 0, %K + -1, 1   <DO_LOOP>
  // |      |   |   + DO i4 = 0, %N2 + -1, 1   <DO_LOOP>
  // |      |   |   |   + DO i5 = 0, %M2 + -1, 1   <DO_LOOP>
  // |      |   |   |   |   %AijBkl = (%A)[%M1 * i1 + i2] * (%B)[%M2 * i4 + i5];
  // |      |   |   |   |   %sum = %sum  +  %AijBkl;
  // |      |   |   |   + END LOOP
  // |      |   |   + END LOOP
  // |      |   + END LOOP
  // |      + END LOOP
  // |   }
  // + END LOOP
  HLIf *SafeCheckIf = nullptr;
  Type *const BoolType  = IntegerType::getInt1Ty(HNU.getContext());
  RegDDRef *const True  = DDRU.createConstDDRef(BoolType, 1);
  RegDDRef *const False = DDRU.createConstDDRef(BoolType, 0);
  if (SafeCheckLevelParent != OutermostParent) {
    HLInst *const NeedCheckInit =
      HNU.createCopyInst(True->clone(), "rwmv.needcheck");
    HLNodeUtils::insertAsLastPreheaderNode(OutermostParent, NeedCheckInit);
    const RegDDRef *const NeedCheck = NeedCheckInit->getLvalDDRef();
    SafeCheckIf =
      HNU.createHLIf(PredicateTy::ICMP_NE, NeedCheck->clone(), False->clone());
    SafeCheckLevelParent->extractZtt();
    SafeCheckLevelParent->extractPreheader();
    HLNodeUtils::insertBefore(SafeCheckLevelParent, SafeCheckIf);
    HLInst *const NeedCheckReset =
      HNU.createCopyInst(False->clone(), "", NeedCheck->clone());
    HLNodeUtils::insertAsLastThenChild(SafeCheckIf, NeedCheckReset);
    for (HLLoop *Parent            = SafeCheckLevelParent->getParentLoop();
         Parent != nullptr; Parent = Parent->getParentLoop()) {
      Parent->addLiveInTemp(NeedCheck->getSymbase());
    }
  }

  // Create an empty clone of the non-invariant loop if needed:
  //
  //    %rwmv.needcheck = 1;
  // + DO i1 = 0, %N1 + -1, 1   <DO_LOOP>
  // |   if (%M1 > 0)
  // |   {
  // |      if (%rwmv.needcheck != 0)
  // |      {
  // |         + DO i2 = 0, %N2 + -1, 1   <DO_LOOP>
  // |         + END LOOP
  // |
  // |         %rwmv.needcheck = 0;
  // |      }
  // |
  // |      + DO i2 = 0, %M1 + -1, 1   <DO_LOOP>
  // |      |   + DO i3 = 0, %K + -1, 1   <DO_LOOP>
  // |      |   |   + DO i4 = 0, %N2 + -1, 1   <DO_LOOP>
  // |      |   |   |   + DO i5 = 0, %M2 + -1, 1   <DO_LOOP>
  // |      |   |   |   |   %AijBkl = (%A)[%M1 * i1 + i2] * (%B)[%M2 * i4 + i5];
  // |      |   |   |   |   %sum = %sum  +  %AijBkl;
  // |      |   |   |   + END LOOP
  // |      |   |   + END LOOP
  // |      |   + END LOOP
  // |      + END LOOP
  // |   }
  // + END LOOP
  const HLLoop *const NonInvariantLoop =
    MVNonInvariantLevel ? Lp->getParentLoopAtLevel(MVNonInvariantLevel)
                        : nullptr;
  HLLoop *NonInvariantCheckLoop = nullptr;
  if (MVNonInvariantLevel) {
    NonInvariantCheckLoop = NonInvariantLoop->cloneEmpty();
    if (SafeCheckIf)
      HLNodeUtils::insertAsFirstThenChild(SafeCheckIf, NonInvariantCheckLoop);
    else {
      OutermostParent->extractZtt();
      OutermostParent->extractPreheader();
      HLNodeUtils::insertBefore(OutermostParent, NonInvariantCheckLoop);
    }
  }

  // Create an empty clone of the inner loop to start the check loop:
  //
  //    %rwmv.needcheck = 1;
  // + DO i1 = 0, %N1 + -1, 1   <DO_LOOP>
  // |   if (%M1 > 0)
  // |   {
  // |      if (%rwmv.needcheck != 0)
  // |      {
  // |         + DO i2 = 0, %N2 + -1, 1   <DO_LOOP>
  // |         |   + DO i3 = 0, %M2 + -1, 1   <DO_LOOP>
  // |         |   + END LOOP
  // |         + END LOOP
  // |
  // |         %rwmv.needcheck = 0;
  // |      }
  // |
  // |      + DO i2 = 0, %M1 + -1, 1   <DO_LOOP>
  // |      |   + DO i3 = 0, %K + -1, 1   <DO_LOOP>
  // |      |   |   + DO i4 = 0, %N2 + -1, 1   <DO_LOOP>
  // |      |   |   |   + DO i5 = 0, %M2 + -1, 1   <DO_LOOP>
  // |      |   |   |   |   %AijBkl = (%A)[%M1 * i1 + i2] * (%B)[%M2 * i4 + i5];
  // |      |   |   |   |   %sum = %sum  +  %AijBkl;
  // |      |   |   |   + END LOOP
  // |      |   |   + END LOOP
  // |      |   + END LOOP
  // |      + END LOOP
  // |   }
  // + END LOOP
  HLLoop *const CheckLoop = Lp->cloneEmpty();
  if (NonInvariantCheckLoop) {
    HLNodeUtils::insertAsLastChild(NonInvariantCheckLoop, CheckLoop);
  } else if (SafeCheckIf) {
    HLNodeUtils::insertAsFirstThenChild(SafeCheckIf, CheckLoop);
  } else {
    OutermostParent->extractZtt();
    OutermostParent->extractPreheader();
    HLNodeUtils::insertBefore(OutermostParent, CheckLoop);
  }

#if INTEL_INTERNAL_BUILD
  ORBuilder(*CheckLoop).addOrigin("Probe loop for row-wise multiversioning");
#endif

  // Add ZTTs from the outer loops to make sure the accesses are safe. This
  // should be done on an outer if so that ZTTs aren't added directly to loops
  // with unrelated bounds:
  //
  //    %rwmv.needcheck = 1;
  // + DO i1 = 0, %N1 + -1, 1   <DO_LOOP>
  // |   if (%M1 > 0)
  // |   {
  // |      if (%rwmv.needcheck != 0)
  // |      {
  // |         if (%N2 > 0 && %K > 0)
  // |         {
  // |            + DO i2 = 0, %N2 + -1, 1   <DO_LOOP>
  // |            |   + DO i3 = 0, %M2 + -1, 1   <DO_LOOP>
  // |            |   + END LOOP
  // |            + END LOOP
  // |         }
  // |
  // |         %rwmv.needcheck = 0;
  // |      }
  // |
  // |      + DO i2 = 0, %M1 + -1, 1   <DO_LOOP>
  // |      |   + DO i3 = 0, %K + -1, 1   <DO_LOOP>
  // |      |   |   + DO i4 = 0, %N2 + -1, 1   <DO_LOOP>
  // |      |   |   |   + DO i5 = 0, %M2 + -1, 1   <DO_LOOP>
  // |      |   |   |   |   %AijBkl = (%A)[%M1 * i1 + i2] * (%B)[%M2 * i4 + i5];
  // |      |   |   |   |   %sum = %sum  +  %AijBkl;
  // |      |   |   |   + END LOOP
  // |      |   |   + END LOOP
  // |      |   + END LOOP
  // |      + END LOOP
  // |   }
  // + END LOOP
  const HLNode *const PreZTTParent = CheckLoop->getParent();
  HLLoop *const ZTTLoop =
    NonInvariantCheckLoop ? NonInvariantCheckLoop : CheckLoop;
  HLIf *ZTTIf = nullptr;
  for (const HLLoop *ParentLp                     = Lp->getParentLoop();
       ParentLp != SafeCheckLevelParent; ParentLp = ParentLp->getParentLoop()) {
    assert(ParentLp && "Lp should not be the outermost loop");
    if (!ParentLp->hasZtt())
      continue;
    if (ParentLp == NonInvariantLoop)
      continue;
    for (auto ZTTIter = ParentLp->ztt_pred_begin(),
              ZTTEnd  = ParentLp->ztt_pred_end();
         ZTTIter != ZTTEnd; ++ZTTIter) {
      const RegDDRef *const Left =
          ParentLp->getLHSZttPredicateOperandDDRef(ZTTIter);
      const RegDDRef *const Right =
          ParentLp->getRHSZttPredicateOperandDDRef(ZTTIter);
      if (!ZTTIf) {
        if (ZTTLoop->hasZtt()) {
          ZTTLoop->extractZtt();
          ZTTIf = dyn_cast<HLIf>(ZTTLoop->getParent());
          assert(ZTTIf);
        } else {
          ZTTIf = HNU.createHLIf(*ZTTIter, Left->clone(), Right->clone());
          HNU.insertBefore(ZTTLoop, ZTTIf);
          HNU.moveAsFirstThenChild(ZTTIf, ZTTLoop);
          continue;
        }
      }
      ZTTIf->addPredicate(*ZTTIter, Left->clone(), Right->clone());
    }
  }

  // Add a load of the DDRef chosen for multiversioning:
  //
  // + DO i2 = 0, %N2 + -1, 1   <DO_LOOP>
  // |   + DO i3 = 0, %M2 + -1, 1   <DO_LOOP>
  // |   |   %rwmv.next = (%B)[%M2 * i2 + i3];
  // |   + END LOOP
  // + END LOOP
  RegDDRef *const CheckRef = MVRef->clone();
  unsigned SafeCheckLevel  = SafeCheckLevelParent->getNestingLevel();
  if (MVNonInvariantLevel) {
    CheckRef->getSingleCanonExpr()->replaceIV(MVNonInvariantLevel,
                                              SafeCheckLevel);
    CheckRef->getSingleCanonExpr()->replaceIV(Lp->getNestingLevel(),
                                              SafeCheckLevel + 1);
  } else {
    CheckRef->getSingleCanonExpr()->replaceIV(Lp->getNestingLevel(),
                                              SafeCheckLevel);
  }
  HLInst *const NextLoad = HNU.createLoad(CheckRef, "rwmv.next");
  HLNodeUtils::insertAsLastChild(CheckLoop, NextLoad);
  const RegDDRef *const Next = NextLoad->getLvalDDRef();

  // Peel the first iteration of the loop to load the first value of the array:
  //
  // + DO i2 = 0, %N2 + -1, 1   <DO_LOOP>
  // |   %rwmv.first = (%B)[%M2 * i2];
  // |
  // |   + DO i3 = 0, %M2 + -2, 1   <DO_LOOP>
  // |   |   %rwmv.next = (%B)[%M2 * i2 + i3 + 1];
  // |   + END LOOP
  // + END LOOP
  HLLoop *const PeelLoop            = CheckLoop->peelFirstIteration();
  auto *const FirstLoad             = cast<HLInst>(PeelLoop->getFirstChild());
  HNU.createAndReplaceTemp(FirstLoad->getLvalDDRef(), "rwmv.first");
  const RegDDRef *const First = FirstLoad->getLvalDDRef();
  PeelLoop->replaceByFirstIteration();

  // Sometimes the process of peeling or adding outer ZTTs to the loop
  // introduces a new parent if for the check loop. If this happens, keep track
  // of that if to add things outside of it if needed.
  HLIf *OuterIf = nullptr;
  if (CheckLoop->getParent() != PreZTTParent) {
    OuterIf = dyn_cast<HLIf>(CheckLoop->getParent());
    assert(OuterIf);
    assert(OuterIf->getParent() == PreZTTParent);
  }

  // Add the comparison to check that each value in the array is close to the
  // first:
  //
  // + DO i2 = 0, %N2 + -1, 1   <DO_LOOP>
  // |   %rwmv.first = (%B)[%M2 * i2];
  // |
  // |   + DO i3 = 0, %M2 + -2, 1   <DO_LOOP>
  // |   |   %rwmv.next = (%B)[%M2 * i2 + i3 + 1];
  // |   |   %rwmv.diff = %rwmv.next  -  %rwmv.first;
  // |   |   %rwmv.absdiff = @llvm.fabs.f64(%rwmv.diff);
  // |   |   if (%rwmv.absdiff >u 1.000000e-04)
  // |   |   {
  // |   |   }
  // |   + END LOOP
  // + END LOOP
  HLInst *const Diff =
    HNU.createFSub(Next->clone(), First->clone(), "rwmv.diff");
  HLNodeUtils::insertAsLastChild(CheckLoop, Diff);
  Function *const FAbs = Intrinsic::getDeclaration(
    &HNU.getModule(), Intrinsic::fabs, Next->getDestType());
  HLInst *const AbsDiff =
    HNU.createCall(FAbs, Diff->getLvalDDRef()->clone(), "rwmv.absdiff");
  HLNodeUtils::insertAsLastChild(CheckLoop, AbsDiff);
  RegDDRef *const Tolerance =
    DDRU.createConstDDRef(ConstantFP::get(Next->getDestType(), CheckTolerance));
  HLIf *const CheckIf = HNU.createHLIf(
    CmpInst::FCMP_UGT, AbsDiff->getLvalDDRef()->clone(), Tolerance);
  HLNodeUtils::insertAsLastChild(CheckLoop, CheckIf);
  CheckLoop->addLiveInTemp(First->getSymbase());

  // Add a variable to keep track of whether the check fails:
  //
  // + DO i2 = 0, %N2 + -1, 1   <DO_LOOP>
  // |   %rwmv.first = (%B)[%M2 * i2];
  // |   %rwmv.allclose = 1;
  // |
  // |   + DO i3 = 0, %M2 + -2, 1   <DO_LOOP>
  // |   |   %rwmv.next = (%B)[%M2 * i2 + i3 + 1];
  // |   |   %rwmv.diff = %rwmv.next  -  %rwmv.first;
  // |   |   %rwmv.absdiff = @llvm.fabs.f64(%rwmv.diff);
  // |   |   if (%rwmv.absdiff >u 1.000000e-04)
  // |   |   {
  // |   |      %rwmv.allclose = 0;
  // |   |   }
  // |   + END LOOP
  // + END LOOP
  HLInst *const AllCloseInit = HNU.createCopyInst(True, "rwmv.allclose");
  HLNodeUtils::insertBefore(CheckLoop, AllCloseInit);
  const RegDDRef *const AllClose = AllCloseInit->getLvalDDRef();
  HLInst *const AllCloseReset =
    HNU.createCopyInst(False->clone(), "", AllClose->clone());
  HLNodeUtils::insertAsLastThenChild(CheckIf, AllCloseReset);
  CheckLoop->addLiveOutTemp(AllClose->getSymbase());

  // Initialize the multiversioning case to 0 (no special value):
  //
  // + DO i2 = 0, %N2 + -1, 1   <DO_LOOP>
  // |   %rwmv.first = (%B)[%M2 * i2];
  // |   %rwmv.allclose = 1;
  // |
  // |   + DO i3 = 0, %M2 + -2, 1   <DO_LOOP>
  // |   |   %rwmv.next = (%B)[%M2 * i2 + i3 + 1];
  // |   |   %rwmv.diff = %rwmv.next  -  %rwmv.first;
  // |   |   %rwmv.absdiff = @llvm.fabs.f64(%rwmv.diff);
  // |   |   if (%rwmv.absdiff >u 1.000000e-04)
  // |   |   {
  // |   |      %rwmv.allclose = 0;
  // |   |   }
  // |   + END LOOP
  // |
  // |   %rwmv.rowcase = 0;
  // + END LOOP
  Type *const RowCaseTy = IntegerType::getInt8Ty(HNU.getContext());
  HLInst *const RowCaseInit =
    HNU.createCopyInst(DDRU.createConstDDRef(RowCaseTy, 0), "rwmv.rowcase");
  if (SafeCheckIf && !NonInvariantCheckLoop)
    HLNodeUtils::insertAsLastPreheaderNode(OutermostParent, RowCaseInit);
  else if (OuterIf)
    HLNodeUtils::insertBefore(OuterIf, RowCaseInit);
  else
    HLNodeUtils::insertAfter(CheckLoop, RowCaseInit);
  const RegDDRef *const RowCase = RowCaseInit->getLvalDDRef();
  if (!NonInvariantCheckLoop) {
    for (HLLoop *Parent = Lp->getParentLoop(); Parent != nullptr;
         Parent         = Parent->getParentLoop()) {
      Parent->addLiveInTemp(RowCase->getSymbase());
    }
  }

  // Check for identified multiversion values:
  //
  // + DO i2 = 0, %N2 + -1, 1   <DO_LOOP>
  // |   %rwmv.first = (%B)[%M2 * i2];
  // |   %rwmv.allclose = 1;
  // |
  // |   + DO i3 = 0, %M2 + -2, 1   <DO_LOOP>
  // |   |   %rwmv.next = (%B)[%M2 * i2 + i3 + 1];
  // |   |   %rwmv.diff = %rwmv.next  -  %rwmv.first;
  // |   |   %rwmv.absdiff = @llvm.fabs.f64(%rwmv.diff);
  // |   |   if (%rwmv.absdiff >u 1.000000e-04)
  // |   |   {
  // |   |      %rwmv.allclose = 0;
  // |   |   }
  // |   + END LOOP
  // |
  // |   %rwmv.rowcase = 0;
  // |   if (%rwmv.allclose != 0)
  // |   {
  // |      if (%rwmv.first == -1.0)
  // |      {
  // |         %rwmv.rowcase = 1;
  // |      }
  // |      else
  // |      {
  // |         if (%rwmv.first == 0.0)
  // |         {
  // |            %rwmv.rowcase = 2;
  // |         }
  // |      }
  // |   }
  // + END LOOP
  HLIf *const CloseIf =
    HNU.createHLIf(CmpInst::ICMP_NE, AllClose->clone(), False);
  if ((SafeCheckIf && !NonInvariantCheckLoop) || OuterIf)
    HLNodeUtils::insertAfter(CheckLoop, CloseIf);
  else
    HLNodeUtils::insertAfter(RowCaseInit, CloseIf);
  HLIf *PrevIf = nullptr;
  for (size_t MVInd = 0, MVEnd = MVVals.size(); MVInd != MVEnd; ++MVInd) {
    HLIf *const CurIf = HNU.createHLIf(CmpInst::FCMP_OEQ, First->clone(),
                                       DDRU.createConstDDRef(MVVals[MVInd]));
    if (PrevIf)
      HLNodeUtils::insertAsLastElseChild(PrevIf, CurIf);
    else
      HLNodeUtils::insertAsFirstThenChild(CloseIf, CurIf);
    HLInst *const RowCaseUpdate = HNU.createCopyInst(
      DDRU.createConstDDRef(RowCaseTy, MVInd + 1), "", RowCase->clone());
    HLNodeUtils::insertAsLastThenChild(CurIf, RowCaseUpdate);
    PrevIf = CurIf;
  }

  // If needed, collect the row types in a temp array:
  //
  //    %rwmv.needcheck = 1;
  //    %rwmv.rowcases = alloca %N2
  // + DO i1 = 0, %N1 + -1, 1   <DO_LOOP>
  // |   if (%M1 > 0)
  // |   {
  // |      if (%rwmv.needcheck != 0)
  // |      {
  // |         if (%N2 > 0 && %K > 0)
  // |         {
  // |            + DO i2 = 0, %N2 + -1, 1   <DO_LOOP>
  // |            |   %rwmv.first = (%B)[%M2 * i2];
  // |            |   %rwmv.allclose = 1;
  // |            |
  // |            |   + DO i3 = 0, %M2 + -2, 1   <DO_LOOP>
  // |            |   |   %rwmv.next = (%B)[%M2 * i2 + i3 + 1];
  // |            |   |   %rwmv.diff = %rwmv.next  -  %rwmv.first;
  // |            |   |   %rwmv.absdiff = @llvm.fabs.f64(%rwmv.diff);
  // |            |   |   if (%rwmv.absdiff >u 1.000000e-04)
  // |            |   |   {
  // |            |   |      %rwmv.allclose = 0;
  // |            |   |   }
  // |            |   + END LOOP
  // |            |
  // |            |   %rwmv.rowcase = 0;
  // |            |   if (%rwmv.allclose != 0)
  // |            |   {
  // |            |      if (%rwmv.first == -1.0)
  // |            |      {
  // |            |         %rwmv.rowcase = 1;
  // |            |      }
  // |            |      else
  // |            |      {
  // |            |         if (%rwmv.first == 0.0)
  // |            |         {
  // |            |            %rwmv.rowcase = 2;
  // |            |         }
  // |            |      }
  // |            |   }
  // |            |   (rwmv.rowcases)[i2] = %rwmv.rowcase;
  // |            + END LOOP
  // |         }
  // |
  // |         %rwmv.needcheck = 0;
  // |      }
  // |
  // |      + DO i2 = 0, %M1 + -1, 1   <DO_LOOP>
  // |      |   + DO i3 = 0, %K + -1, 1   <DO_LOOP>
  // |      |   |   + DO i4 = 0, %N2 + -1, 1   <DO_LOOP>
  // |      |   |   |   + DO i5 = 0, %M2 + -1, 1   <DO_LOOP>
  // |      |   |   |   |   %AijBkl = (%A)[%M1 * i1 + i2] * (%B)[%M2 * i4 + i5];
  // |      |   |   |   |   %sum = %sum  +  %AijBkl;
  // |      |   |   |   + END LOOP
  // |      |   |   + END LOOP
  // |      |   + END LOOP
  // |      + END LOOP
  // |   }
  // + END LOOP
  RegDDRef *RowCases    = nullptr;
  RegDDRef *RowCasesRef = nullptr;
  if (NonInvariantCheckLoop) {
    RegDDRef *const TripCount = NonInvariantCheckLoop->getTripCountDDRef();
    assert(TripCount && "Normalized loops should have known trip counts");
    HLInst *const RowCaseAlloca =
      HNU.createAlloca(RowCaseTy, TripCount, "rwmv.rowcases");
    if (SafeCheckIf)
      HLNodeUtils::insertAsLastPreheaderNode(OutermostParent, RowCaseAlloca);
    else
      HLNodeUtils::insertBefore(NonInvariantCheckLoop, RowCaseAlloca);
    RowCases = RowCaseAlloca->getLvalDDRef();

    RowCasesRef =
      DDRU.createMemRef(cast<AllocaInst>(RowCaseAlloca->getLLVMInstruction())->getAllocatedType(), RowCases->getSingleCanonExpr()->getSingleBlobIndex());
    CanonExpr *const RowCasesRefDim =
      CEU.createCanonExpr(NonInvariantCheckLoop->getIVType());
    RowCasesRefDim->addIV(SafeCheckLevel, InvalidBlobIndex, 1);
    RowCasesRef->addDimension(RowCasesRefDim);
    HLInst *const RowCaseStore =
      HNU.createStore(RowCase->clone(), "", RowCasesRef);
    HLNodeUtils::insertAsLastChild(NonInvariantCheckLoop, RowCaseStore);

    NonInvariantCheckLoop->addLiveInTemp(RowCases->getSymbase());
    for (HLLoop *Parent = Lp->getParentLoop(); Parent != nullptr;
         Parent         = Parent->getParentLoop()) {
      Parent->addLiveInTemp(RowCases->getSymbase());
    }
  }

  // We need to be careful that the temp array is initialized in every case
  // where it will be used. This is the case for this transformation because
  // we're requiring that the ZTTs are loop-invariant and are copying the ZTTs
  // from the loops using the temp array so they're going to be present on both
  // loops.

  // Construct the switch for the multiversioning:
  //
  // + DO i4 = 0, %N2 + -1, 1   <DO_LOOP>
  // |   + DO i5 = 0, %M2 + -1, 1   <DO_LOOP>
  // |   |   %AijBkl = (%A)[%M1 * i1 + i2] * (%B)[%M2 * i4 + i5];
  // |   |   %sum = %sum  +  %AijBkl;
  // |   + END LOOP
  // |
  // |   switch((%rwmv.rowcases)[i4])
  // |   {
  // |   }
  // + END LOOP
  RegDDRef *SwitchRef = nullptr;
  if (RowCases) {
    SwitchRef = RowCasesRef->clone();
    SwitchRef->getSingleCanonExpr()->replaceIV(SafeCheckLevel,
                                               MVNonInvariantLevel);
  } else {
    SwitchRef = RowCase->clone();
  }
  HLSwitch *const MVSwitch = HNU.createHLSwitch(SwitchRef);
  HLNodeUtils::insertAfter(Lp, MVSwitch);

  // The unmodified loop will be the default case:
  //
  // + DO i4 = 0, %N2 + -1, 1   <DO_LOOP>
  // |   switch((%rwmv.rowcases)[i4])
  // |   {
  // |   default:
  // |      + DO i5 = 0, %M2 + -1, 1   <DO_LOOP>
  // |      |   %AijBkl = (%A)[%M1 * i1 + i2] * (%B)[%M2 * i4 + i5];
  // |      |   %sum = %sum  +  %AijBkl;
  // |      + END LOOP
  // |      break;
  // |   }
  // + END LOOP
  HLNodeUtils::moveAsLastDefaultChild(MVSwitch, Lp);

  // Clone the loop for each other case:
  //
  // + DO i4 = 0, %N2 + -1, 1   <DO_LOOP>
  // |   switch((%rwmv.rowcases)[i4])
  // |   {
  // |   case 1:
  // |      + DO i5 = 0, %M2 + -1, 1   <DO_LOOP>
  // |      |   %AijBkl = (%A)[%M1 * i1 + i2] * -1.0;
  // |      |   %sum = %sum  +  %AijBkl;
  // |      + END LOOP
  // |      break;
  // |   case 2:
  // |      + DO i5 = 0, %M2 + -1, 1   <DO_LOOP>
  // |      |   %AijBkl = (%A)[%M1 * i1 + i2] * 0.0;
  // |      |   %sum = %sum  +  %AijBkl;
  // |      + END LOOP
  // |      break;
  // |   default:
  // |      + DO i5 = 0, %M2 + -1, 1   <DO_LOOP>
  // |      |   %AijBkl = (%A)[%M1 * i1 + i2] * (%B)[%M2 * i4 + i5];
  // |      |   %sum = %sum  +  %AijBkl;
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

    // Opt reports don't support %f yet; construct the message as a string
    // instead.
#if INTEL_INTERNAL_BUILD
    if (ORBuilder.isOptReportOn()) {
      std::string Message;
      raw_string_ostream MessageStream{Message};
      MessageStream
        << "Row-wise multiversioned loop for value "
        << cast<ConstantFP>(MVVals[MVInd])->getValueAPF().convertToDouble();
      ORBuilder(*MVLoop).addOrigin(Message);
    }
#endif

    // Replace all uses of the loaded value within the loop with the constant.
    replaceAllEquivalentRefsWithConstant(MVLoop, MVRef, MVVals[MVInd],
                                         HDDA.getGraph(MVLoop));

#if INTEL_FEATURE_SW_DTRANS
    if (HIRTransformUtils::doConstantPropagation(MVLoop, DTII)) {
#else // INTEL_FEATURE_SW_DTRANS
    if (HIRTransformUtils::doConstantPropagation(MVLoop)) {
#endif // INTEL_FEATURE_SW_DTRANS
      applyPeepHole(MVLoop, HDDA);
      HLNodeUtils::removeRedundantNodes(MVLoop);
    }
  }

#if INTEL_INTERNAL_BUILD
  ORBuilder(*Lp).addRemark(OptReportVerbosity::Low, 25581u);
#endif
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(SafeCheckLevelParent);
  if (SafeCheckIf)
    HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(OutermostParent);
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

  // Determine how many of the parents it is possible to hoist checks for.
  const HLLoop *const OutermostParent = Lp->getOutermostParentLoop();
  HLLoop *SafeCheckLevelParent        = Lp;
  for (HLNode *Parent            = Lp->getParent(),
              *ParentE           = OutermostParent->getParent();
       Parent != ParentE; Parent = Parent->getParent()) {
    if (!canHoistCheck(Parent))
      break;
    if (auto *const ParentLp = dyn_cast<HLLoop>(Parent))
      SafeCheckLevelParent = ParentLp;
  }
  if (SafeCheckLevelParent == Lp)
    return false;
  unsigned SafeCheckLevel = SafeCheckLevelParent->getNestingLevel();
  if (SafeCheckLevel != 1) {
    LLVM_DEBUG(
      dbgs() << "Falling back to an in-nest check loop outside loop level "
             << SafeCheckLevel << "\n");
  }

  // Also check that there are no forward gotos in the hoisted-from portion of
  // the loop nest.
  //
  // Really, only the gotos that jump over an inner loop that will be hoisted
  // are a problem, but it's easier to be more conservative and avoid having any
  // forward gotos at all. Fancier checking for only the problematic gotos can
  // be added later if needed, and should probably be done by checking that each
  // child HLNode post-dominates the corresponding branch of its parent.
  if (HLS.getTotalLoopStatistics(SafeCheckLevelParent).hasForwardGotos()) {
    LLVM_DEBUG(dbgs() << "Avoiding this loop because there are forward gotos "
                         "in the loop nest\n");
    return false;
  }

  // Avoid loops containing HLIfs because they're less likely to be hot loops.
  if (HLS.getSelfLoopStatistics(Lp).hasIfs()) {
    LLVM_DEBUG(dbgs() << "Avoiding this loop because there are internal ifs\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "Loads:\n");

  // Take a look through the enclosed instructions and collect candidate DDRefs.
  // Add support for ifs/switches if needed.
  SmallVector<MVCandidate, 1> MVCands;
  ACVec ACVals;
  for (const HLNode &C : make_range(Lp->child_begin(), Lp->child_end()))
    if (const auto *const CI = dyn_cast<HLInst>(&C))
      for (const RegDDRef *const Ref :
           make_range(CI->ddref_begin(), CI->ddref_end()))
        if (const MVCandidate MVCand = checkCandidateDDRef(
                Ref, Lp, SafeCheckLevelParent, HDDA,
#if INTEL_FEATURE_SW_DTRANS
                DTII, FieldModRef,
#endif // INTEL_FEATURE_SW_DTRANS
                HLS))
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

  multiversionLoop(Lp, ChosenCand, SafeCheckLevelParent,
#if INTEL_FEATURE_SW_DTRANS
                   DTII,
#endif // INTEL_FEATURE_SW_DTRANS
                   HDDA);

  return true;
}

/// Performs row-wise multiversioning using the given analysis results.
static bool runRowWiseMV(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
#if INTEL_FEATURE_SW_DTRANS
                         DTransImmutableInfo *DTII,
                         FieldModRefResult *FieldModRef,
#endif // INTEL_FEATURE_SW_DTRANS
                         HIRLoopStatistics &HLS) {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << OPT_DESC " Disabled\n");
    return false;
  }

  // This pass adds approximate checks, so if this function is not marked with
  // unsafe-fp-math leave it alone.
  const Attribute UnsafeFpMath =
    HIRF.getFunction().getFnAttribute("unsafe-fp-math");
  if (!UnsafeFpMath.isStringAttribute() ||
      UnsafeFpMath.getValueAsString() != "true") {
    LLVM_DEBUG(dbgs() << "Function is not marked with unsafe-fp-math; skipping "
                         "row-wise multiversioning\n");
    return false;
  }

  HIRRowWiseMV RWMV{HDDA,
#if INTEL_FEATURE_SW_DTRANS
                    DTII, FieldModRef,
#endif // INTEL_FEATURE_SW_DTRANS
                    HLS};

  // Attempt row-wise multiversioning on innermost loops.
  SmallVector<HLLoop *, 16> CandLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(CandLoops);
  bool Changed = false;
  unsigned ApplyCount = 0;
  for (HLLoop *const Lp : CandLoops) {
    if (RWMV.run(Lp)) {
      Changed = true;
      ++ApplyCount;

      // Because of how much code is generated by this transform, it is limited
      // by default to a small number of applications per function (which can be
      // increased with the -hir-rowwise-mv-per-function option). More
      // sophisticated ways to pick which loops are transformed may be possible
      // in the future.
      if (ApplyCount == ApplyLimit)
        return true;
    }
  }

  return Changed;
}

char HIRRowWiseMVLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRRowWiseMVLegacyPass, OPT_SWITCH, OPT_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
#if INTEL_FEATURE_SW_DTRANS
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(DTransFieldModRefResultWrapper)
#endif // INTEL_FEATURE_SW_DTRANS
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
#if INTEL_FEATURE_SW_DTRANS
      &getAnalysis<DTransImmutableAnalysisWrapper>().getResult(),
      &getAnalysis<DTransFieldModRefResultWrapper>().getResult(),
#endif // INTEL_FEATURE_SW_DTRANS
      getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS());
}

PreservedAnalyses HIRRowWiseMVPass::runImpl(Function &F,
                                            llvm::FunctionAnalysisManager &AM,
                                            HIRFramework &HIRF) {
#if INTEL_FEATURE_SW_DTRANS
  auto &MAMProxy = AM.getResult<ModuleAnalysisManagerFunctionProxy>(F);
#endif // INTEL_FEATURE_SW_DTRANS

  runRowWiseMV(
      HIRF, AM.getResult<HIRDDAnalysisPass>(F),
#if INTEL_FEATURE_SW_DTRANS
      MAMProxy.getCachedResult<DTransImmutableAnalysis>(*F.getParent()),
      MAMProxy.getCachedResult<DTransFieldModRefResult>(*F.getParent()),
#endif // INTEL_FEATURE_SW_DTRANS
      AM.getResult<HIRLoopStatisticsAnalysis>(F));
  return PreservedAnalyses::all();
}
