//===-------------------------- HIRRowWiseMV.cpp --------------------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
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
/// rowtype = 0;
/// DO i1 = B + S, E, S
///   if (A[i1+S] != first)
///   {
///     goto CheckFailed;
///   }
/// END DO
///
/// if (first == 0)
/// {
///   rowtype = 1;
/// }
/// else
/// {
///   if (first == 1)
///   {
///     rowtype = 2;
///   }
/// }
///
/// CheckFailed:
/// DO i1
///   switch(rowtype)
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
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
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
    AU.setPreservesAll();
  }
};

/// Implements the row-wise multiversioning transform.
class HIRRowWiseMV {
  HIRDDAnalysis &HDDA;
  DTransImmutableInfo &DTII;

public:
  HIRRowWiseMV(HIRDDAnalysis &HDDA, DTransImmutableInfo &DTII)
      : HDDA{HDDA}, DTII{DTII} {}

  /// Performs row-wise multiversioning on the given loop.
  bool run(HLLoop *);
};

/// The type used for representing sets of arithmetically convenient values.
/// These will generally be sorted.
using ACVec = SmallVector<Constant *, 3>;

/// An ordering of Constants by their values.
///
/// Mixing of integer and FP Constants isn't supported and shouldn't happen in
/// this pass.
bool constantValueOrder(const Constant *A, const Constant *B) {
  const ConstantInt *AI, *BI;
  const ConstantFP *AF, *BF;
  if ((AI = dyn_cast<ConstantInt>(A)) && (BI = dyn_cast<ConstantInt>(B))) {
    return AI->getValue().slt(BI->getValue());
  }
  if ((AF = dyn_cast<ConstantFP>(A)) && (BF = dyn_cast<ConstantFP>(B))) {
    return AF->getValueAPF().compare(BF->getValueAPF()) == APFloat::cmpLessThan;
  }
  llvm_unreachable("Mixed integer/FP values??");
}

} // namespace

/// Determines whether the set of incoming edges for \p Ref in \p DDG is empty.
/// \todo Should this just be part of the DDGraph interface?
static bool incomingEmpty(const DDGraph &DDG, const DDRef *Ref) {
  return DDG.incoming_edges_begin(Ref) == DDG.incoming_edges_end(Ref);
}

/// Determines the arithmetically convenient values of type \p T for opcode
/// \p OPC.
///
/// For now, these are just 0 for add and -1, 0, and 1 for mul. This may be
/// extended to other operations later as needed.
static ACVec getConvenientVals(unsigned OPC, Type *T) {
  switch (OPC) {
  case Instruction::Add:
    return {ConstantInt::get(T, 0, true)};
  case Instruction::Mul:
    return {
      ConstantInt::get(T, int64_t(-1), true),
      ConstantInt::get(T, 0, true),
      ConstantInt::get(T, 1, true),
    };
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

/// Determines whether \p Ref is a candidate for row-wise multiversioning. When
/// it is, the arithmetically convenient values to multiversion are are returned
/// in \p ACVals.
static bool isCandidateDDRef(const RegDDRef *Ref, unsigned NestLevel,
                             const DDGraph &DDG, DTransImmutableInfo &DTII,
                             ACVec &ACVals) {

  // For this to be the case, it must be a load (Rval memory reference).
  if (Ref->isLval())
    return false;
  if (!Ref->isMemRef())
    return false;

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
    return false;
  }
  const bool RejectNonInvariantOuters = NestLevel < 3;
  unsigned NonInvariantLevel          = 0;
  for (unsigned Level = NestLevel - 1; Level >= 1; --Level) {
    if (!Ref->isStructurallyInvariantAtLevel(Level, true)) {
      if (RejectNonInvariantOuters || NonInvariantLevel) {
        LLVM_DEBUG(dbgs() << "  Not invariant at loop level " << Level << "\n");
        return false;
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
    return false;
  }

  // Check for uses that have arithmetically convenient values.
  // Add support for more cases as needed.
  const auto *const Inst = dyn_cast<HLInst>(Ref->getHLDDNode());
  assert(Inst && "Only DDRefs from HLInsts supported so far");

  // If this is a load, add convenient values of its users instead.
  if (Inst->getLLVMInstruction()->getOpcode() == Instruction::Load) {
    ACVals.clear();
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
    return false;
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
      return false;
    }
    const auto *const BaseLd =
      dyn_cast_or_null<LoadInst>(Ref->getTempBaseValue());
    if (!BaseLd) {
      LLVM_DEBUG(dbgs() << "  Base value is not a load\n");
      return false;
    }
    const auto *const BaseGEP =
      dyn_cast<GetElementPtrInst>(BaseLd->getPointerOperand());
    if (!BaseGEP) {
      LLVM_DEBUG(dbgs() << "  Base load does not have a GEP\n");
      return false;
    }
    auto *const BaseStructType =
      dyn_cast<StructType>(BaseGEP->getSourceElementType());
    if (!BaseStructType) {
      LLVM_DEBUG(dbgs() << "  Base GEP type is not a struct: "
                        << *BaseGEP->getSourceElementType() << "\n");
      return false;
    }

    // Figure out which field is being accessed.
    const auto *const FieldIdxC = dyn_cast<ConstantInt>(BaseGEP->getOperand(2));
    if (!FieldIdxC) {
      report_fatal_error("Non-constant index in struct access??");
    }
    const unsigned FieldIdx = FieldIdxC->getLimitedValue();

    // Retrieve the possible values.
    const auto &LikelyValues =
      *DTII.getLikelyIndirectArrayConstantValues(BaseStructType, FieldIdx);

    // What are the possible values?
    LLVM_DEBUG({
      dbgs() << "  Possible values found in DTrans analysis:\n";
      for (const Constant *const IAC : LikelyValues)
        dbgs() << "    " << *IAC << "\n";
    });

    // Use the possible values to filter the arithmetically convenient values.
    ACVec PossibleVals{std::begin(LikelyValues), std::end(LikelyValues)};
    sort(PossibleVals, constantValueOrder);
    ACVals = getIntersection(ACVals, PossibleVals);
  }

  if (ACVals.empty()) {
    LLVM_DEBUG(dbgs() << "  No overlap with possible array values\n");
    return false;
  }

  // This DDRef passes all of the checks, so it is a candidate.
  LLVM_DEBUG(dbgs() << "  Overlap values found; will process\n");
  return true;
}

bool HIRRowWiseMV::run(HLLoop *Lp) {

  // Only handle do loops for now.
  // Add support for other types of loops if needed.
  if (!Lp->isDo())
    return false;

  // Ignore any loops with no outer loops: this transformation is not likely to
  // be profitable for those.
  if (Lp->getNestingLevel() == 1)
    return false;

  // The loop bounds/stride also need to be invariant in the outer loop levels.
  for (const RegDDRef *const Ref :
       make_range(Lp->ddref_begin(), Lp->ddref_end()))
    for (unsigned Level = Lp->getNestingLevel() - 1; Level >= 1; --Level)
      if (!Ref->isStructurallyInvariantAtLevel(Level))
        return false;

  LLVM_DEBUG({
    dbgs() << "Examining loop:\n";
    Lp->dump();
    dbgs() << "Loads:\n";
  });

  const DDGraph DDG = HDDA.getGraph(Lp->getParentRegion());

  // Take a look through the enclosed instructions and collect candidate DDRefs.
  // Add support for ifs/switches if needed.
  SmallVector<std::pair<const RegDDRef *, ACVec>, 1> CandRefs;
  ACVec ACVals;
  for (const HLNode &C : make_range(Lp->child_begin(), Lp->child_end()))
    if (const auto *const CI = dyn_cast<HLInst>(&C))
      for (const RegDDRef *const Ref :
           make_range(CI->ddref_begin(), CI->ddref_end()))
        if (isCandidateDDRef(Ref, Lp->getNestingLevel(), DDG, DTII, ACVals))
          CandRefs.push_back({Ref, ACVals});

  LLVM_DEBUG(dbgs() << "\n");

  if (CandRefs.empty()) {
    LLVM_DEBUG(
      dbgs() << "No candidate array accesses identified for multiversioning\n");
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "Identified these candidates for multiversioning:\n";
    for (const auto &CandRef : CandRefs) {
      dbgs() << "  ";
      CandRef.first->print(fdbgs());
      dbgs() << ":\n";
      for (const Constant *const ACVal : CandRef.second)
        dbgs() << "    " << *ACVal << "\n";
      dbgs() << "\n";
    }
  });

  return false;
}

/// Performs row-wise multiversioning using the given analysis results.
static bool runRowWiseMV(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
                         DTransImmutableInfo &DTII) {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << OPT_DESC " Disabled\n");
    return false;
  }

  HIRRowWiseMV RWMV{HDDA, DTII};

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
    getAnalysis<DTransImmutableAnalysisWrapper>().getResult());
}

PreservedAnalyses HIRRowWiseMVPass::run(Function &F,
                                        llvm::FunctionAnalysisManager &AM) {
  runRowWiseMV(AM.getResult<HIRFrameworkAnalysis>(F),
               AM.getResult<HIRDDAnalysisPass>(F),
               AM.getResult<DTransImmutableAnalysis>(F));
  return PreservedAnalyses::all();
}
