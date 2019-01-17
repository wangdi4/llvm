//===- HIRLoopReroll.cpp - Implements Loop Reroll transformation ----------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Rerolling of straight lines of code
// RF = Reroll Factor
// Suppose subscripts are as follows:
// coeff*i + K, coeff*i + dist + K,..., coeff*i + (RF - 1)*dist + K
//
// Rerolling may change subscripts and trip counts.
//
//   coeff * (i/RF) + dist * (i%RF) + K
//   TC --> TC * RF
//
// E.G
// DO i = 0, N - 1, 1
//  A[c*i + K]     =
//  A[c*i + d + K] =
//  A[c*i +2d + K] =
//  ...
//  A[c*i + (RF-1)d + K] =
// END DO
//
// Rerolling may transform the loop into
//
// DO i = 0, RF*N - 1, 1
//  A[c*(i/RF) + dist*(i%RF) + K]  =
// END DO
//
// Reroll cost-model choses to reroll only
// when explicit division and modulo operations
// can be removed.
// Namely,
//  - coeff is divisible by RF (i.e. coeff % RF == 0)
//  - coeff == dist * RF
//
//  Explanations: c stands for coeff, d stands for dist.
//   c*(i/RF) + d * (i%RF) + K
//     = d * RF * (i/RF) + d * (i % RF) + K
//     = d * (RF * (i/RF) + (i % RF)) + K
//     = d * i + K
//     = (c/RF) * i + K
//
// Other limitations and potential todos:
// If a blob IV coeff exists, we just bail out.
// Consider following example,
//    DO 0 N-1
//       A[2*b*i]     =
//       A[2*b*i + b] =
//    END DO
//  If the loop were rerolled by 2
//    DO 0 2*N-1
//       A[b*i]     =
//    END DO
//  (Is it still fine with being b < 0?)
//
//  Similarily, blob distance is not handled.
//    DO 0 N-1
//       A[2*i]     =
//       A[2*i + b] =
//    END DO
//  Hard to make sure( b == 2/2).
//
//  On the other hand,
//    DO 0 N-1
//       A[2*i]     = 2i + b
//       A[2*i + 1] = 2i + b + 1
//    END DO
//  should be unrolled to
//    DO 0 2*N-1
//       A[i]     = i + b
//    END DO

#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopReroll.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include <stack>

using namespace llvm;
using namespace llvm::loopopt;

#define OPT_SWITCH "hir-loop-reroll"
#define OPT_DESC "HIR Loop Reroll"
#define DEBUG_TYPE OPT_SWITCH

STATISTIC(LoopsRerolled, "Number of HIR loops rerolled");

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));
namespace {

class HIRLoopRerollLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRLoopRerollLegacyPass() : HIRTransformPass(ID) {
    initializeHIRLoopRerollLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override{};

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();

    AU.setPreservesAll();
  }
};

} // namespace

namespace {

// Hard to estimate the size: not using small vector.
typedef std::vector<const CanonExpr *> VecCEsTy;

class CEOpSequence {
  typedef unsigned PositionTy;
  typedef unsigned OpcodeTy;
  typedef std::pair<PositionTy, OpcodeTy> PosOpcodeTy;
  typedef std::vector<PosOpcodeTy> VecOpcodesTy;

  int NumDDRefs;
  void countRefs() { NumDDRefs++; }

public:
  CEOpSequence() : NumDDRefs(0) {}

  VecCEsTy CEList;
  VecOpcodesTy Opcodes;

  void add(const RegDDRef *Ref) {
    if (Ref->hasGEPInfo()) {
      CEList.push_back(Ref->getBaseCE());
    }
    for (const CanonExpr *CE :
         make_range(Ref->canon_begin(), Ref->canon_end())) {
      CEList.push_back(CE);
    }
    countRefs();
  }
  void add(PositionTy Pos, OpcodeTy Opcode) {
    Opcodes.emplace_back(Pos, Opcode);
  }
  unsigned size() const { return CEList.size(); }
  unsigned opSize() const { return Opcodes.size(); }
  unsigned numRefs() const { return NumDDRefs; }
};

// Hard to estimate the size: not using small vector.
typedef std::vector<const DDRef *> VecDDRefsTy;
typedef std::stack<const DDRef *, VecDDRefsTy> StackDDRefsTy;
typedef std::vector<const HLNode *> VecNodesTy;
typedef std::vector<CEOpSequence> VecCEOpSeqTy;
typedef std::vector<VecNodesTy> VecVecNodesTy;
typedef DenseMap<BlobTy, unsigned> LoopInvariantBlobTy; // SCEV, BID

namespace rerollcomparator {

bool blobIndexLess(unsigned BI1, unsigned BI2) { return BI1 < BI2; }

struct BlobDDRefLess {
  bool operator()(const BlobDDRef *B1, const BlobDDRef *B2) {
    return blobIndexLess(B1->getSingleCanonExpr()->getSingleBlobIndex(),
                         B2->getSingleCanonExpr()->getSingleBlobIndex());
  }
};

struct RegDDRefLess {
  bool operator()(const RegDDRef *R1, const RegDDRef *R2) {
    bool IsMemRef1 = R1->isMemRef();
    bool IsMemRef2 = R2->isMemRef();

    if (IsMemRef1) {
      return IsMemRef2 ? DDRefUtils::compareMemRef(R1, R2) : false;
    }

    if (IsMemRef2) {
      return true;
    }

    // Neither is MemRef just use symbase
    return R1->getSymbase() < R2->getSymbase();
  }
};

} // namespace rerollcomparator

class SequenceBuilder {

  const HLLoop *Loop;

  void processRegDDRef(const RegDDRef *Ref, CEOpSequence &Seq,
                       StackDDRefsTy &Stack) const;
  void collectDDRefsFromAStore(const HLInst *HInst, DDGraph &DDG,
                               CEOpSequence &Seq, VecNodesTy &InstList) const;

public:
  explicit SequenceBuilder(const HLLoop *Lp) : Loop(Lp) {}

  bool areRerollSequencesBuilt(HIRDDAnalysis &DDA, VecCEOpSeqTy &VecSeq,
                               VecVecNodesTy &VecInstList,
                               VecNodesTy &VecSeeds) const;
};

void SequenceBuilder::processRegDDRef(const RegDDRef *Ref, CEOpSequence &Seq,
                                      StackDDRefsTy &Stack) const {
  if (Ref->isConstant()) {
    Seq.add(Ref);
    return;
  }

  unsigned Level = Loop->getNestingLevel();

  if (Ref->isSelfBlob()) {
    if (Ref->getSingleCanonExpr()->isLinearAtLevel(Level)) {
      // Output it to RefList and than return.
      Seq.add(Ref);
    } else if (!Loop->isLiveIn(Ref->getSymbase())) {
      // If Ref is not linear-at-level then push it to stack and return.
      Stack.push(Ref);
    }
    return;
  }

  Seq.add(Ref);

  // Push non-LinearAtLevel temps among blob ddrefs into stack.
  // Check for LiveIn is used instead of isLinearAtLevel because
  // - Usually !isLiveIn implies !isLinearAtLevel
  // - If not, e.g. "K = 5" in the innermost loop, chances are
  //   the innermost loop is not a reroll pattern.
  // - Most importantly, current algorithm for straight line codes
  //   does not handle reductions, i.e. live-in but non-loop-invariant
  //   temps.
  //
  // Sort the blobs to handle a case like
  //   %1  = B[2*i];
  //   A[2*i] = %n * %1;
  //          blob ddrefs are in the order of %n, %1.
  //   %3  = B[2*i+1];
  //   A[2*i+1] = %n * %3;
  //          blob ddrefs are in the order of %3, %n.
  // Sort blobs to make both have
  //     %n, %1
  //     %n, %3
  SmallVector<const BlobDDRef *, 8> Blobs;
  const HLLoop *Lp = Loop;
  std::copy_if(Ref->blob_begin(), Ref->blob_end(), std::back_inserter(Blobs),
               [Lp](const BlobDDRef *Blob) {
                 return !Lp->isLiveIn(Blob->getSymbase());
               });
  std::sort(Blobs.begin(), Blobs.end(), rerollcomparator::BlobDDRefLess());

  for (const DDRef *Blob : Blobs) {
    Stack.push(Blob);
  }
}

void processOpcode(const HLInst *DefInst, CEOpSequence &Seq) {
  unsigned Index = Seq.CEList.size();
  unsigned Opcode = DefInst->getLLVMInstruction()->getOpcode();
  Seq.add(Index, Opcode);
}

void sortRvals(const HLInst *DefInst,
               SmallVectorImpl<const RegDDRef *> &RvalDDRefCopy) {
  unsigned NumOps = DefInst->getNumOperands();
  unsigned CopySize = RvalDDRefCopy.size();
  assert(NumOps == (CopySize + 1));
  (void)NumOps;
  (void)CopySize;

  std::copy(DefInst->rval_op_ddref_begin(), DefInst->rval_op_ddref_end(),
            RvalDDRefCopy.begin());

  std::sort(RvalDDRefCopy.begin(), RvalDDRefCopy.end(),
            rerollcomparator::RegDDRefLess());
}

// Ref is a temp ref, either a blob ddref or a self blob
// Scan DD edges to this ref, which is a flow edge.
const HLInst *findTempDef(const DDRef *TempRef, DDGraph &DDG,
                          const HLLoop *Loop) {
  for (DDEdge *E : DDG.incoming(TempRef)) {
    if (E->isFLOWdep()) {
      DDRef *Src = E->getSrc();
      if (HLInst *DefInst = dyn_cast<HLInst>(Src->getHLDDNode())) {
        return DefInst;
      }
    }
  }
  return nullptr;
}

// Givin a Store instruction, gather all involved DDRefs. For example,
//
// %n = ...
// DO
//   %1  = B[2*i];       -- (1)
//   A[2*i] = %n * %1;   -- (2)
//
// END DO
//
// Inst (2) is a Store inst. This function collects
//  - its LVAL reference, A[2*i]
//  - its RVAL references
//     In case temp refs are present, it tracks the instruction defining
//     that temp. Then adds non-temp DDRefs to list. If another temp ref
//     is found, this process repeats. If that temp is defined outside the
//     loop interest (linear at innermost level), the repetition stop.
//     Thus, for the example code above, %n, B[2*i], are collected.
// The final result will be {A[2*i], %n * %1, B[2*i]}
// "*" in the second entry is actually separately store as (*, 1), meaning
// "*" operator and the position of the operator after i-th entry in the
// CE sequence.
//
// Output:
//  CEOpSequence - sequences of CEs involved in a seed store HLInst and
//                 sequences of operators involved.
//  InstList: a set of HLInsts from which CEs in CEOpSequence are found.
void SequenceBuilder::collectDDRefsFromAStore(const HLInst *HInst, DDGraph &DDG,
                                              CEOpSequence &Seq,
                                              VecNodesTy &InstList) const {
  StackDDRefsTy TempTracker;

  // Lval of Store is the root
  SequenceBuilder::processRegDDRef(HInst->getLvalDDRef(), Seq, TempTracker);
  InstList.push_back(HInst);

  // RHS of Store is the children to push to the stack.
  // Children ddrefs of a ddref in this context
  // are ddrefs in the right hand side of the inst which
  // defines this ddref.
  // Example:
  // %m = A[i] + %q; -- (2)
  //    = %m ..      -- (1)
  // Child of %m at (1) are A[i] and %q in (2)
  SequenceBuilder::processRegDDRef(HInst->getRvalDDRef(), Seq, TempTracker);

  while (!TempTracker.empty()) {
    const DDRef *TempRef = TempTracker.top();
    TempTracker.pop();

    assert(TempRef->isSelfBlob());

    const HLInst *DefInst = findTempDef(TempRef, DDG, Loop);
    assert(DefInst && "DefInst should exists within the"
                      "loop when a temp is not loop-invariant");
    assert(!TempRef->getSingleCanonExpr()->isLinearAtLevel(
        Loop->getNestingLevel()));
    InstList.push_back(DefInst);
    processOpcode(DefInst, Seq);

    // Push Def's RHS, no LVAL
    SmallVector<const RegDDRef *, 4> RvalDDRefCopy(DefInst->getNumOperands() -
                                                   1);
    sortRvals(DefInst, RvalDDRefCopy);
    for (const RegDDRef *ChildDDRef :
         make_range(RvalDDRefCopy.begin(), RvalDDRefCopy.end())) {
      SequenceBuilder::processRegDDRef(ChildDDRef, Seq, TempTracker);
    }
  }
}

class SequenceChecker {
  const HLLoop *Loop;
  const LoopInvariantBlobTy &LoopInvariantBlobs;

  bool areEqualBlobTyForReroll(const BlobTy &Blob1, const BlobTy &Blob2) const;
  bool isBlobsMathchedForReroll(const CanonExpr *CE1, const CanonExpr *CE2,
                                const HLLoop *Loop) const;
  bool isValidDistance(const VecCEsTy &CEList1, const VecCEsTy &CEList2,
                       unsigned RF) const;
  bool isSequenceMatched(const unsigned II, const VecCEOpSeqTy &VecSeq) const;

public:
  explicit SequenceChecker(const HLLoop *Lp, const LoopInvariantBlobTy &Invs)
      : Loop(Lp), LoopInvariantBlobs(Invs) {}

  std::pair<unsigned, unsigned>
  calcRerollFactor(const VecCEOpSeqTy &VecSeq) const;
};

bool SequenceChecker::areEqualBlobTyForReroll(const BlobTy &Blob1,
                                              const BlobTy &Blob2) const {
  if (Blob1 == Blob2) {
    return true;
  }

  if (Blob1->getType() != Blob2->getType()) {
    return false;
  }

  if (Blob1->getSCEVType() != Blob2->getSCEVType()) {
    return false;
  }

  if (auto ConstSCEV1 = dyn_cast<SCEVConstant>(Blob1)) {
    auto ConstSCEV2 = cast<SCEVConstant>(Blob2);
    return ConstSCEV1 == ConstSCEV2;
  }

  if (auto CastSCEV1 = dyn_cast<SCEVCastExpr>(Blob1)) {
    auto CastSCEV2 = cast<SCEVCastExpr>(Blob2);
    if (CastSCEV1->getOperand()->getType() !=
        CastSCEV2->getOperand()->getType()) {
      return false;
    }

    return areEqualBlobTyForReroll(CastSCEV1->getOperand(),
                                   CastSCEV2->getOperand());
  }

  if (auto NArySCEV1 = dyn_cast<SCEVNAryExpr>(Blob1)) {
    auto NArySCEV2 = cast<SCEVNAryExpr>(Blob2);
    unsigned NumOprds1 = NArySCEV1->getNumOperands();
    unsigned NumOprds2 = NArySCEV2->getNumOperands();
    if (NumOprds1 != NumOprds2) {
      return false;
    }

    if (isa<SCEVCommutativeExpr>(NArySCEV1) && NumOprds1 == 2) {
      // For two-operand commutative operation, a swap is tried.
      // For more than 2 oprds, we bail out.
      // No reordering of operands are tried.
      assert(NumOprds1 == 2);
      return (areEqualBlobTyForReroll(NArySCEV1->getOperand(0),
                                      NArySCEV2->getOperand(0)) &&
              areEqualBlobTyForReroll(NArySCEV1->getOperand(1),
                                      NArySCEV2->getOperand(1))) ||
             (areEqualBlobTyForReroll(NArySCEV1->getOperand(0),
                                      NArySCEV2->getOperand(1)) &&
              areEqualBlobTyForReroll(NArySCEV1->getOperand(1),
                                      NArySCEV2->getOperand(0)));
    }

    for (auto I1 = NArySCEV1->op_begin(), I2 = NArySCEV2->op_begin(),
              E1 = NArySCEV1->op_end(), E2 = NArySCEV2->op_end();
         I1 != E1 && I2 != E2; ++I1, ++I2) {
      if (!areEqualBlobTyForReroll(*I1, *I2)) {
        return false;
      }
    }
    return true;
  }

  if (auto UDivSCEV1 = dyn_cast<SCEVUDivExpr>(Blob1)) {
    auto UDivSCEV2 = cast<SCEVUDivExpr>(Blob2);

    return areEqualBlobTyForReroll(UDivSCEV1->getLHS(), UDivSCEV2->getLHS()) &&
           areEqualBlobTyForReroll(UDivSCEV1->getRHS(), UDivSCEV2->getRHS());
  }

  if (isa<SCEVUnknown>(Blob1)) {
    if (LoopInvariantBlobs.find(Blob1) == LoopInvariantBlobs.end()) {
      // Defer the comparison towards its defining instruction.
      return LoopInvariantBlobs.find(Blob2) == LoopInvariantBlobs.end();
    }
    return Blob1 == Blob2;
  }

  llvm_unreachable("Unknown Blob type!");
  return false;
}

// Compare blobs -- ignore IVs or Const if any.
// Use HIR as much as possible to compare blobs.
// If necessary, compare SCEVs for rerolloing purpose,
// by calling areEqualBlobTyForReroll().
bool SequenceChecker::isBlobsMathchedForReroll(const CanonExpr *CE1,
                                               const CanonExpr *CE2,
                                               const HLLoop *Loop) const {

  unsigned Level = Loop->getNestingLevel();
  bool AreAllInvariant1 = CE1->isLinearAtLevel(Level);
  bool AreAllInvariant2 = CE2->isLinearAtLevel(Level);
  if (AreAllInvariant1 != AreAllInvariant2) {
    return false;
  }
  if (AreAllInvariant1) {
    // No need to look further into the blobs.
    return CanonExprUtils::areEqual(CE1, CE2);
  }

  assert(CE1->hasBlob());
  assert(CE2->hasBlob());

  // Now we need to compare blobs, but not using Blob Index.
  // numBlobs and Blob coeffs are compared.

  unsigned NumBlobs = CE1->numBlobs();
  if (CE1->numBlobs() != NumBlobs) {
    return false;
  }

  typedef std::pair<int64_t, unsigned> BlobCoeffAndIndexTy;
  typedef SmallVector<BlobCoeffAndIndexTy, 4> VecBlobCoeffIndexTy;
  SmallVector<VecBlobCoeffIndexTy, 2> CoeffIDs(2);

  const CanonExpr *CEs[2] = {CE1, CE2};
  for (int I = 0; I < 2; I++) {
    for (auto Blob : make_range(CEs[I]->blob_begin(), CEs[I]->blob_end())) {
      CoeffIDs[I].push_back(std::make_pair(Blob.Coeff, Blob.Index));
    }
  }

  const BlobUtils &BU = Loop->getBlobUtils();
  const LoopInvariantBlobTy &Invs = LoopInvariantBlobs;
  auto TempCompare = [Loop, &BU, &Invs](const BlobCoeffAndIndexTy &P1,
                                        const BlobCoeffAndIndexTy &P2) {
    // Use loop invariance info roughly
    bool Inv1 = Invs.find(BU.getBlob(P1.second)) != Invs.end();
    bool Inv2 = Invs.find(BU.getBlob(P2.second)) != Invs.end();
    if (Inv1 != Inv2) {
      // Notice this check imposed strict weak ordering
      // (Inv1, Inv2) = (t, f) returns t
      // (Inv1, Inv2) = (f, t) returns f
      return Inv1;
    }
    return rerollcomparator::blobIndexLess(P1.second, P2.second);
  };

  // Sort numblobs by blobIndex.
  for (int I = 0; I < 2; I++) {
    std::sort(CoeffIDs[I].begin(), CoeffIDs[I].end(), TempCompare);
  }

  auto PredCoeff = [](const BlobCoeffAndIndexTy &CI1,
                      const BlobCoeffAndIndexTy &CI2) {
    return CI1.first == CI2.first;
  };

  auto PredBlobTy = [&BU, this](const BlobCoeffAndIndexTy &CI1,
                                const BlobCoeffAndIndexTy &CI2) {
    BlobTy B1 = BU.getBlob(CI1.second);
    BlobTy B2 = BU.getBlob(CI2.second);
    return this->areEqualBlobTyForReroll(B1, B2);
  };

  auto IsMatched = [&CoeffIDs,
                    NumBlobs](std::function<bool(const BlobCoeffAndIndexTy &,
                                                 const BlobCoeffAndIndexTy &)>
                                  Pred) {
    bool IsDone = true;
    for (auto I1 = CoeffIDs[0].begin(), I2 = CoeffIDs[1].begin(),
              E1 = CoeffIDs[0].end();
         I1 != E1; ++I1, ++I2) {
      if (!Pred(*I1, *I2)) {
        IsDone = false;
        break;
      }
    }

    if (IsDone || NumBlobs != 2)
      return IsDone;

    // Second chance for the commutative '+' and two operands.
    // For more than two, we bail out.
    // Swap the order and compare.
    return Pred(CoeffIDs[0][0], CoeffIDs[1][1]) &&
           Pred(CoeffIDs[0][1], CoeffIDs[1][0]);
  };

  return IsMatched(PredCoeff) && IsMatched(PredBlobTy);
}

// See if
// - Every IV Coeffs at Level are divisible by RF (reroll factor).
// - The quotient is the same as -Dist.
// Namely,
// Coeff == RF * -Dist
bool SequenceChecker::isValidDistance(const VecCEsTy &CEList1,
                                      const VecCEsTy &CEList2,
                                      unsigned RF) const {

  assert(CEList1.size() == CEList2.size());

  auto CheckDistance = [RF](const CanonExpr *CE1, const CanonExpr *CE2,
                            unsigned Level) {
    if (CE1->getSrcType() != CE2->getSrcType() ||
        CE1->getDestType() != CE2->getDestType()) {
      return false;
    }

    int64_t Dist = -1;
    if (!CanonExprUtils::getConstDistance(CE1, CE2, &Dist)) {
      return false;
    }

    // No blob IV coeff is guaranteed by previous checks.
    int64_t Coeff = CE1->getIVConstCoeff(Level);

    // When const only, Coeff == 0 and Dist == 0. Thus, condition holds.
    if (Coeff != static_cast<int64_t>(RF) * -Dist) {
      return false;
    }

    assert(Coeff % static_cast<int64_t>(RF) == 0);
    return true;
  };

  unsigned Level = Loop->getNestingLevel();
  for (VecCEsTy::const_iterator I1 = CEList1.begin(), I2 = CEList2.begin(),
                                EI1 = CEList1.end(), EI2 = CEList2.end();
       I1 != EI1; ++I1, ++I2) {
    const CanonExpr *CE1 = *I1;
    const CanonExpr *CE2 = *I2;

    // Cannot use CanonExprUtils::getConstDistance() right away.
    // Suppose two CEs are %n*%n*%1 and %n*%n*%2, both are
    // represented as one Blob indices.
    // getConstDistance() will just return false, with no const distance.
    // But for rerolloing purpose, serveral things should be made sure.
    // - For linear temps, (%n*%n) parts are the same. This can be handled
    //   by looking into SCEV (i.e. BlobTy).
    // - Both are in the shape of temp * temp * temp (SCEV's information)
    // - %1 and %2, non-linear blobs are A[i] and A[i+1] ...
    //   So the comparison of %1 and %2 are deferred. Actual comparison happens
    //   between A[i] and A[i+1].

    bool HasBlob = CE1->hasBlob();
    bool HasIV = CE1->hasIV(Level);

    if (HasBlob != CE2->hasBlob() || HasIV != CE2->hasIV(Level)) {
      return false;
    }

    // If no blob just call getConstDistance.
    // e.g. IV + const, IV, const
    if (!HasBlob) {
      if (!CheckDistance(CE1, CE2, Level)) {
        return false;
      }
      continue;
    }

    // IV + Blob
    // All temps involved in Blob should be linear at Level.
    // Notice that these temps are found as BlobDDRef of the RegDDRef
    // containing this CE. However, not all BlobDDRefs found in the RegDDRefs
    // are temps involved in this CE's blob.
    // Thus, we have to dig into SCEV, if the blob is not self blob,
    // to get self blobs.
    // First, simply check the distance as a preliminary check.
    // However, const distance true does not mean it is OK.
    // i + %n*%1, i + 1 + %n*%1 might gives const distance 1.
    // If %1 is non-linear, B[i], they are not the pattern.
    if (HasBlob && HasIV) {
      // Make sure all self blobs are loop-invariant.
      if (!CE1->isLinearAtLevel(Level) || !CE2->isLinearAtLevel(Level)) {
        return false;
      }

      // Allowing blob and IV togher is error prone.
      // For example, HIR is parsed like
      // t = 4i
      // B[i] = t + blob
      // The algorithms might not work.
      if (!CheckDistance(CE1, CE2, Level)) {
        return false;
      }

      continue;
    }

    if (HasBlob) {
      // Blob and/or const
      if (CE1->getConstant() != CE2->getConstant()) {
        return false;
      }

      // Look into blobs.
      // %1*n and %2*%n cannot be just compared with blob Indices.
      // If only loop-invariant temps are found in blobs,
      // using blob-indices only is safe.
      // isBlobsMathchedForReroll and its callee areEqualBlobTyForReroll
      // do the check
      if (!isBlobsMathchedForReroll(CE1, CE2, Loop)) {
        return false;
      }
    }
  }

  return true;
}

// With given II,
// check congruence among CE sequences.
bool SequenceChecker::isSequenceMatched(const unsigned II,
                                        const VecCEOpSeqTy &VecSeq) const {

  unsigned VecSize = VecSeq.size();

  // Sequence lengths should be the same.
  for (unsigned J = 0; J < II; J++) {
    // Instructions in the first initiation interval
    unsigned LeadSize = VecSeq[J].size();
    unsigned LeadNumRefs = VecSeq[J].numRefs();
    unsigned LeadOpSize = VecSeq[J].opSize();
    // Instructions in the second initiation interval and so on.
    for (unsigned K = J + II; K < VecSize; K += II) {
      // Every instance of a chunk must have the same nums of DDRefs
      if (VecSeq[K].size() != LeadSize || VecSeq[K].numRefs() != LeadNumRefs ||
          VecSeq[K].opSize() != LeadOpSize) {
        return false;
      }
    }
  }

  // Kinds of opcodes sequence should be the same.
  for (unsigned J = 0; J < II; J++) {
    for (unsigned K = J; K + II < VecSize; K += II) {
      bool IsSameOps =
          std::equal(VecSeq[K].Opcodes.begin(), VecSeq[K].Opcodes.end(),
                     VecSeq[K + II].Opcodes.begin());
      if (!IsSameOps) {
        return false;
      }
    }
  }

  // CEs in adjacent sequences should be in right distance apart.
  unsigned RF = VecSize / II;
  for (unsigned J = 0; J < II; J++) {
    for (unsigned K = J; K + II < VecSize; K += II) {
      if (!this->isValidDistance(VecSeq[K].CEList, VecSeq[K + II].CEList, RF)) {
        return false;
      }
    }
  }

  return true;
}

// RerollFactor and Initiation interval
// Return reroll factor(RF) and initiation interval(II).
//
//  RF = (total number of seeds) / II
//
// When no rerolling is possible, reroll factor 0 and initiation interval 0
// are returned.
std::pair<unsigned, unsigned>
SequenceChecker::calcRerollFactor(const VecCEOpSeqTy &VecSeq) const {
  unsigned VecSize = VecSeq.size();
  if (VecSize <= 1) {
    // VecSize == 1 means no rolling has been done.
    return std::make_pair(0, 0);
  }

  for (unsigned II = 1; II <= VecSize / 2; II++) {
    // Examine each II (Initiation interval)
    if (VecSize % II != 0) {
      continue;
    }

    if (!isSequenceMatched(II, VecSeq)) {
      continue;
    }

    return std::make_pair(VecSize / II, II);
  }

  return std::make_pair(0, 0);
}

// The class is used to preserve relavant ddrefs
// after populated.
class DDRefScavenger {
  typedef DDRefGatherer<RegDDRef, AllRefs> RefGatherer;

  RefGatherer::VectorTy Refs;
  const HLLoop *Loop;
  unsigned NumLoopRefs;

public:
  DDRefScavenger(const HLLoop *Lp) : Loop(Lp), NumLoopRefs(0) {}

  void populateRefs() {
    // Only loop refs, no preheader/postexit or loop-body refs
    RefGatherer::gather<false>(Loop, Refs);
    NumLoopRefs = Refs.size();

    // Now loop body
    RefGatherer::gatherRange(Loop->child_begin(), Loop->child_end(), Refs);
  }

  void populateLoopInvariantBlobs(LoopInvariantBlobTy &LoopInvariantBlob) const;

  bool hasNonRerollConformantCEs() const;

  // iterators
  typedef RefGatherer::VectorTy::const_iterator ref_iterator;
  ref_iterator body_begin() const { return Refs.begin() + NumLoopRefs; }
  ref_iterator body_end() const { return Refs.end(); }
};

void DDRefScavenger::populateLoopInvariantBlobs(
    LoopInvariantBlobTy &LoopInvariantBlobs) const {

  auto AddInvariantBlob = [&LoopInvariantBlobs](const CanonExpr *CE,
                                                unsigned Level) {
    unsigned BI = CE->getSingleBlobIndex();
    const BlobUtils &BU = CE->getBlobUtils();
    if (CE->isLinearAtLevel(Level)) {
      BlobTy Blob = BU.getBlob(BI);
      LoopInvariantBlobs[Blob] = BI;
    }
  };

  unsigned Level = Loop->getNestingLevel();

  for (const RegDDRef *Ref : Refs) {
    if (Ref->isSelfBlob()) {
      AddInvariantBlob(Ref->getSingleCanonExpr(), Level);
    } else {
      for (const BlobDDRef *BRef :
           make_range(Ref->blob_begin(), Ref->blob_end())) {
        AddInvariantBlob(BRef->getSingleCanonExpr(), Level);
      }
    }
  }
}

bool DDRefScavenger::hasNonRerollConformantCEs() const {

  unsigned Level = Loop->getNestingLevel();
  for (const RegDDRef *Ref : make_range(body_begin(), body_end())) {
    for (const CanonExpr *CE :
         make_range(Ref->canon_begin(), Ref->canon_end())) {
      unsigned Index = InvalidBlobIndex;
      int64_t Coeff = 0;
      CE->getIVCoeff(Level, &Index, &Coeff);
      if (Index != InvalidBlobIndex) {
        return true;
      }
      int64_t Denom = CE->getDenominator();
      if (Denom != 1) {
        // TODO: See if Reroll algorithm needs to be refined
        //       for Denom != 1
        return true;
      }
    }
  }

  return false;
}

// Copy an empty loop and rewrite the loop's body
// with Insts in VecInstList[0] through VecInstList[II - 1].
// Replace UPPERBOUND, and IV's Coeffs  as calculated
// by Reroll Factor.
bool rewriteLoopBody(unsigned RerollFactor, unsigned II, VecNodesTy &VecSeeds) {

  HLLoop *Loop = const_cast<HLLoop *>(VecSeeds[0]->getParentLoop());
  assert(VecSeeds.size() == RerollFactor * II);

  bool IsValid = HIRTransformUtils::multiplyTripCount(Loop, RerollFactor);
  if (!IsValid) {
    // We cannot come up with a valid new TC
    return false;
  }

  // Locate the last inst of the first group
  // Inst at the (II-1) from 0
  HLNodeUtils::remove(std::next(HLContainerTy::iterator(
                          const_cast<HLNode *>(VecSeeds[II - 1]))),
                      std::next(HLContainerTy::iterator(Loop->getLastChild())));

  // Now update IV const Coeff
  unsigned Level = Loop->getNestingLevel();
  ForEach<RegDDRef>::visitRange(
      Loop->child_begin(), Loop->child_end(),
      [RerollFactor, Level](RegDDRef *Ref) {
        for (CanonExpr *CE :
             llvm::make_range(Ref->canon_begin(), Ref->canon_end())) {
          unsigned Index = 0;
          int64_t Coeff = 0;
          if (!CE->hasIV(Level)) {
            continue;
          }
          CE->getIVCoeff(Level, &Index, &Coeff);
          assert(Index == InvalidBlobIndex);

          CE->setIVCoeff(Level, Index,
                         Coeff / static_cast<int64_t>(RerollFactor));
        }
      });

  // Parent loop body doesn't have to be invalidated for DD. DD-analysis knows
  // outer loops has to be re-computed because inner loops have invalidated.
  HIRInvalidationUtils::invalidateBody(Loop);
  HIRInvalidationUtils::invalidateBounds(Loop);

  return true;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
std::string getOpcodeString(unsigned Opcode) {
  if ((Opcode == Instruction::Add) || (Opcode == Instruction::FAdd)) {
    return "  +  ";
  } else if ((Opcode == Instruction::Sub) || (Opcode == Instruction::FSub)) {
    return "  -  ";
  } else if ((Opcode == Instruction::Mul) || (Opcode == Instruction::FMul)) {
    return "  *  ";
  } else if (Opcode == Instruction::UDiv) {
    return "  /u  ";
  } else if ((Opcode == Instruction::SDiv) || (Opcode == Instruction::FDiv)) {
    return "  /  ";
  } else if (Opcode == Instruction::URem) {
    return "  %u  ";
  } else if ((Opcode == Instruction::SRem) || (Opcode == Instruction::FRem)) {
    return "  %  ";
  } else if (Opcode == Instruction::Shl) {
    return "  <<  ";
  } else if ((Opcode == Instruction::LShr) || (Opcode == Instruction::AShr)) {
    return "  >>  ";
  } else if (Opcode == Instruction::And) {
    return "  &&  ";
  } else if (Opcode == Instruction::Or) {
    return "  ||  ";
  } else if (Opcode == Instruction::Xor) {
    return "  ^  ";
  } else {
    return "Other-binary";
  }
  return "Non-binary";
}

LLVM_DUMP_METHOD void dumpOprdOpSequence(const HLInst *HInst,
                                         const CEOpSequence &Seq) {
  dbgs() << " RefList For Inst ";
  HInst->dump();

  for (auto CE : Seq.CEList) {
    CE->dump();
    dbgs() << ", ";
  }

  dbgs() << "\n";

  for (auto CE : Seq.CEList) {
    CE->getSrcType()->dump();
  }
  dbgs() << "\n";

  for (auto CE : Seq.CEList) {
    dbgs() << CE;
    dbgs() << ", ";
  }
  dbgs() << "\n";

  for (auto P : Seq.Opcodes) {
    dbgs() << P.first;
    dbgs() << ": ";
    dbgs() << getOpcodeString(P.second);
    dbgs() << ", ";
  }
  dbgs() << "\n";
}
#endif

bool SequenceBuilder::areRerollSequencesBuilt(HIRDDAnalysis &DDA,
                                              VecCEOpSeqTy &VecSeq,
                                              VecVecNodesTy &VecInstList,
                                              VecNodesTy &VecSeeds) const {

  // Map to make sure consumption of all instructions in the loop body
  DenseMap<const HLNode *, bool> InstToOccurrence(128);
  auto IsStraightCodes = [&InstToOccurrence](const HLLoop *Loop) {
    for (const HLNode &Node :
         make_range(Loop->child_begin(), Loop->child_end())) {
      const HLInst *HInst = cast<HLInst>(&Node);
      InstToOccurrence[HInst] = false;
    }
    return true;
  };

  auto MarkConsumedInsts = [&InstToOccurrence](const VecNodesTy &InstList) {
    for (auto Node : InstList) {
      InstToOccurrence[Node] = true;
    }
  };

  auto AreAllInstsConsumed = [&InstToOccurrence]() {
    for (auto I : InstToOccurrence) {
      bool IsConsumed = I.second;
      if (!IsConsumed) {
        return false;
      }
    }
    return true;
  };

  if (!IsStraightCodes(Loop)) {
    return false;
  }

  DDGraph DDG = DDA.getGraph(Loop);
  LLVM_DEBUG(DDG.dump(););

  for (const HLNode &Node :
       make_range(Loop->child_begin(), Loop->child_end())) {

    const HLInst *HInst = cast<HLInst>(&Node);
    const Instruction *Inst = HInst->getLLVMInstruction();
    // TODO: Handle Cmp and Select if needed.
    if (isa<CmpInst>(Inst) || isa<SelectInst>(Inst)) {
      return false;
    }
    if (!isa<StoreInst>(Inst)) {
      continue;
    }

    // A store inst is a seed: collected all ddrefs involved
    // in the store.
    CEOpSequence Seq;
    VecNodesTy InstList;
    VecSeeds.push_back(&Node);
    SequenceBuilder::collectDDRefsFromAStore(HInst, DDG, Seq, InstList);
    MarkConsumedInsts(InstList);
    assert(Seq.size() > 0 &&
           "collectDDRefsFromAStore is not correctly working!");
    VecSeq.push_back(Seq);
    VecInstList.push_back(InstList);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    LLVM_DEBUG(dumpOprdOpSequence(HInst, Seq));
#endif
  }

  if (!AreAllInstsConsumed()) {
    // Some insts in the loop body is not participating in the rerolling
    // pattern.
    LLVM_DEBUG(dbgs() << "Reroll: not all instructions are covered\n");
    return false;
  }

  return true;
}

bool isRerollCandidate(const HLLoop *Loop, HIRLoopStatistics &HLS,
                       DDRefScavenger &RefScavenger) {
  if (!Loop->isInnermost() || !Loop->isDo() || !Loop->isNormalized()) {
    return false;
  }

  const LoopStatistics &LS = HLS.getTotalLoopStatistics(Loop);
  if (LS.hasIfs() || LS.hasSwitches() || LS.hasForwardGotos() ||
      LS.hasCalls()) {
    return false;
  }

  // pragmas
  // We don't stop rerolling with unroll and jam pragma on the parent loop.
  if (Loop->hasUnrollEnablingPragma() || Loop->hasVectorizeEnablingPragma()) {
    return false;
  }

  // TODO: Add tests for being Vectorizable.
  //       Rerolling is mainly for coming up with unit-strided accesses.
  //       Once HIRParVecAnalysis's capability become utility calls,
  //       call those.

  RefScavenger.populateRefs();
  if (RefScavenger.hasNonRerollConformantCEs()) {
    return false;
  }

  return true;
}

// See if Loop has defined temps in [child at II, last child]
bool hasLiveOutTempsToBeRemoved(const VecNodesTy &VecSeeds, unsigned II) {
  const HLLoop *Loop = VecSeeds[0]->getParentLoop();
  for (const HLNode &Node :
       make_range(std::next(Loop->child_begin(), II), Loop->child_end())) {
    const HLInst *HInst = dyn_cast<HLInst>(&Node);

    if (!HInst) {
      continue;
    }

    const RegDDRef *Lval = HInst->getLvalDDRef();
    if (Loop->isLiveOut(Lval->getSymbase())) {
      return true;
    }
  }

  return false;
}

// Reroll given loop body.
// Loop must be the innermost loop.
// This function works with assumptions as follows:
// - loop is an innermost loop.
// - loop body is a straight line code all with HLInsts.
// Capability
// - Only a Store inst works as a seed.
// - One store for one reroll instance works.
bool rerollStraightCodes(HLLoop *Loop, HIRDDAnalysis &DDA,
                         HIRLoopStatistics &HLS) {
  DDRefScavenger RefScavenger(Loop);
  if (!isRerollCandidate(Loop, HLS, RefScavenger)) {
    return false;
  }

  VecCEOpSeqTy VecSeq;
  VecVecNodesTy VecInstList;
  VecNodesTy VecSeeds; // Separately maintained for rewriting
  SequenceBuilder SeqBuilder(Loop);
  if (!SeqBuilder.areRerollSequencesBuilt(DDA, VecSeq, VecInstList, VecSeeds)) {
    return false;
  }

  LoopInvariantBlobTy LoopInvariantBlobs;
  RefScavenger.populateLoopInvariantBlobs(LoopInvariantBlobs);

  unsigned RerollFactor = 0;
  unsigned InitInterval = 0;
  SequenceChecker SeqChecker(Loop, LoopInvariantBlobs);
  std::tie(RerollFactor, InitInterval) = SeqChecker.calcRerollFactor(VecSeq);

  if (RerollFactor <= 1) {
    return false;
  }

  if (hasLiveOutTempsToBeRemoved(VecSeeds, InitInterval)) {
    return false;
  }

  bool IsRerolled = rewriteLoopBody(RerollFactor, InitInterval, VecSeeds);
  return IsRerolled;
}

unsigned doLoopReroll(HIRFramework &HIRF, HIRDDAnalysis &DDA,
                      HIRLoopStatistics &HLS) {

  unsigned NumLoopsRerolled = 0;
  ForEach<HLLoop>::visitRange(HIRF.hir_begin(), HIRF.hir_end(),
                              [&DDA, &HLS, &NumLoopsRerolled](HLLoop *Loop) {
                                bool Result =
                                    rerollStraightCodes(Loop, DDA, HLS);
                                if (Result) {
                                  ++NumLoopsRerolled;
                                }
                              });

  return NumLoopsRerolled;
}

} // namespace

char HIRLoopRerollLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopRerollLegacyPass, OPT_SWITCH, OPT_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRLoopRerollLegacyPass, OPT_SWITCH, OPT_DESC, false, false)

FunctionPass *llvm::createHIRLoopRerollPass() {
  return new HIRLoopRerollLegacyPass();
}

bool HIRLoopRerollLegacyPass::runOnFunction(Function &F) {
  if (DisablePass || skipFunction(F)) {
    return false;
  }

  LLVM_DEBUG(dbgs() << OPT_DESC " for Function : " << F.getName() << "\n");

  auto &HIRF = getAnalysis<HIRFrameworkWrapperPass>().getHIR();
  auto &DDA = getAnalysis<HIRDDAnalysisWrapperPass>().getDDA();
  auto &HLS = getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS();

  LoopsRerolled = doLoopReroll(HIRF, DDA, HLS);
  if (LoopsRerolled > 0) {
    LLVM_DEBUG(dbgs() << "Reroll happend\n");
  }
  return LoopsRerolled > 0;
}

PreservedAnalyses HIRLoopRerollPass::run(llvm::Function &F,
                                         llvm::FunctionAnalysisManager &AM) {
  if (DisablePass) {
    return PreservedAnalyses::all();
  }

  LLVM_DEBUG(dbgs() << OPT_DESC " for Function : " << F.getName() << "\n");

  bool IsRerolled = doLoopReroll(AM.getResult<HIRFrameworkAnalysis>(F),
                                 AM.getResult<HIRDDAnalysisPass>(F),
                                 AM.getResult<HIRLoopStatisticsAnalysis>(F));
  if (IsRerolled) {
    LLVM_DEBUG(dbgs() << "Reroll happend\n");
  }

  return PreservedAnalyses::all();
}
// TODO: alias info needs to be checked for safety? (In what cases? We don't
// change
//       the order of execution.)
