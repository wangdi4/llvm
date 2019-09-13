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
//
//
// This pass handles certain safe reductions.
//
// - Reduction chains originated from floating-point type operations.
//
//    BEGIN REGION { }
//           + DO i1 = 0, 499, 1   <DO_LOOP>
//           |   %add = %S.013  +  (@A)[0][2 * i1];
//           |   %S.013 = %add  +  (@A)[0][2 * i1 + 1];
//           + END LOOP
//    END REGION
//
// - Self safe reduction insts originated from int-type operations
//    BEGIN REGION { }
//          + DO i1 = 0, 499, 1   <DO_LOOP>
//          |   %0 = (@A)[0][2 * i1];
//          |   %S.013 = %0 + %S.013  +  (@A)[0][2 * i1 + 1];
//          + END LOOP
//    END REGION
//
// Only ADD(FADD) reductions operations are currently covered.
// Only temp blob seeds are covered for self reduction insts.
//
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopReroll.h"

#include "HIRReroll.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeIterator.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::reroll;

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

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.addRequiredTransitive<HIRSafeReductionAnalysisWrapperPass>();

    AU.setPreservesAll();
  }
};

} // namespace

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
std::string llvm::loopopt::reroll::getOpcodeString(unsigned Opcode) {
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

LLVM_DUMP_METHOD void llvm::loopopt::reroll::dumpOprdOpSequence(
    const HLInst *HInst, const CEOpSequence &Seq, bool Detail) {
  dbgs() << " RefList For Inst ";
  HInst->dump();

  for (auto CE : Seq.CEList) {
    CE->dump();
    dbgs() << ", ";
  }

  dbgs() << "\n";

  if (Detail) {
    for (auto CE : Seq.CEList) {
      CE->getSrcType()->dump();
    }
    dbgs() << "\n";

    for (auto CE : Seq.CEList) {
      dbgs() << CE;
      dbgs() << ", ";
    }
    dbgs() << "\n";
  }

  for (auto P : Seq.Opcodes) {
    dbgs() << P.first;
    dbgs() << ": ";
    dbgs() << getOpcodeString(P.second);
    dbgs() << ", ";
  }
  dbgs() << "\n";
}
#endif

bool llvm::loopopt::reroll::rerollcomparator::blobIndexLess(unsigned BI1,
                                                            unsigned BI2) {
  return BI1 < BI2;
}

namespace {

///  Information about a seed for rerolling.
///  More pieces of information due to (Self) Safe Reductions.
///  A seed is a starting point where a tracing of CEOpSequence begins.
///  A seed is usually a DDRef, either MemRef or TempRef.
///  For example, for a store inst, A[2i] = %t,
///  LvalDDRef, A[2i], will make a seed.
///  After all CEs including temps are covered in Lval, the CEOpSequence
///  extension proceeds to the Rval of store inst.
///  When a seed is from a self safe reduction instruction,
///  it can be a SCEV. For example,in S = S + %1 + %2; %1 is a SCEV, as
///  "S + %1" forms a RValDDRef.
///  %1 and %2 can be considered as two seeds.
///  CEOpSequence extension starts from %1 and from %2, respectively.
///  And two sequences are produced.
///  RerollSeedInfo maintains pieces of information of a seed.
///  - DDRef or SCEV
///  - Set of Instructions encountered during extension of CEOpSequence
///    of a seed.
struct RerollSeedInfo : public SeedInfo {
  typedef union {
    // Seed in the form of SCEV, can be from self SR
    BlobTy SCEVSeed;

    // Seed in the form of DDRef. True for most cases
    const DDRef *DDRefSeed;
  } SeedTy;

  RerollSeedInfo(HLInst *HInst, BlobTy SSeed)
      : SeedInfo(HInst), IsSCEV(true), IsFromSelfSR(true) {
    Seed.SCEVSeed = SSeed;
  }
  RerollSeedInfo(HLInst *HInst, const DDRef *Ref, bool IsFromSelf = false)
      : SeedInfo(HInst), IsSCEV(false), IsFromSelfSR(IsFromSelf) {
    Seed.DDRefSeed = Ref;
  }

  // TODO: Following two member vars are being used only for debugging.
  // Seed is a form of SCEV due to Self SR
  bool IsSCEV;

  // Not necessarily IsSCEV == IsFromSelfSR
  // S = S + %1 + A[2i + 1]; For "A[2i + 1]" being a seed isSCEV = false
  // and IsFromSelfSR is true. The latter is needed for correct rewriting
  bool IsFromSelfSR;

  SeedTy Seed;
};

typedef VecSeedInfoTy VecRerollSeedInfoTy;
typedef DenseMap<BlobTy, unsigned> LoopInvariantBlobTy; // SCEV, BID
typedef DenseMap<BlobTy, const HLInst *> TempBlobTyToHInstTy;
typedef DenseMap<BlobTy, const DDRef *> TempToDDRefTy;

class SequenceBuilderForLoop
    : public SequenceBuilder<SequenceBuilderForLoop, HLLoop> {

public:
  explicit SequenceBuilderForLoop(const HLLoop *Lp, DDGraph &G,
                                  CEOpSequence &Seq, VecNodesTy &IL)
      : SequenceBuilder<SequenceBuilderForLoop, HLLoop>(Lp, G, Seq, IL),
        Level(Lp->getNestingLevel()) {}

  explicit SequenceBuilderForLoop(const TempStackTy *S, const HLLoop *Lp,
                                  DDGraph &G, CEOpSequence &Seq, VecNodesTy &IL)
      : SequenceBuilder<SequenceBuilderForLoop, HLLoop>(S, Lp, G, Seq, IL),
        Level(Lp->getNestingLevel()) {}

  // Ref is a temp ref, either a blob ddref or a self blob
  // Scan DD edges to this ref, which is a flow edge.
  HLInst *findTempDef(const DDRef *TempRef) const {
    for (DDEdge *E : DDG.incoming(TempRef)) {
      if (E->isFlow() && E->getDVAtLevel(Level) == DVKind::EQ) {
        DDRef *Src = E->getSrc();
        if (HLInst *DefInst = dyn_cast<HLInst>(Src->getHLDDNode())) {
          return DefInst;
        }
      }
    }
    return nullptr;
  }

  bool stopTrackingTemp(const RegDDRef *Ref) const {
    return Ref->getSingleCanonExpr()->isLinearAtLevel(Level);
  }

private:
  unsigned Level;
};

} // namespace

namespace {

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
  if (CE2->numBlobs() != NumBlobs) {
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
      // Notice this check imposes strict weak ordering
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
                                EI1 = CEList1.end();
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
  void populateMapFromSCEVToBlobDDRef(TempToDDRefTy &TempToDDRef) const;

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

void DDRefScavenger::populateMapFromSCEVToBlobDDRef(
    TempToDDRefTy &TempToDDRef) const {

  // This Map is only needed for Self SR Inst
  // Can be done per Self SR Inst to maintain a smaller map
  auto AddToMap = [&TempToDDRef](const DDRef *Ref) {
    unsigned Index = Ref->getSelfBlobIndex();
    const BlobUtils &BU = Ref->getBlobUtils();
    TempToDDRef[BU.getBlob(Index)] = Ref;
  };

  for (const RegDDRef *Ref : Refs) {
    if (Ref->isSelfBlob()) {
      AddToMap(Ref);
    } else {
      for (const BlobDDRef *BRef :
           make_range(Ref->blob_begin(), Ref->blob_end())) {
        AddToMap(BRef);
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

typedef SmallVector<BlobTy, 16> VecSCEVTy;
typedef struct SelfSRSeedsTy {
  VecSCEVTy SCEVSeeds;
  const RegDDRef *MemRef;
  SelfSRSeedsTy(VecSCEVTy &Seeds, const RegDDRef *MRef)
      : SCEVSeeds(Seeds), MemRef(MRef) {}
} SelfSRSeedsTy;
typedef std::map<const HLInst *, SelfSRSeedsTy> SelfSRToSeedsTy;

class RerollRewriterBase {
protected:
  RerollRewriterBase(unsigned RF, const VecRerollSeedInfoTy &Info,
                     HIRSafeReductionAnalysis *SA = nullptr,
                     unsigned SelfSR = 0)
      : RerollFactor(RF), VecSeedInfo(Info), SRA(SA), NumSelfSR(SelfSR) {
    Loop = (VecSeedInfo.front().ContainingInst)->getParentLoop();

    // We are handling only a normalized form already.
    assert(Loop->isNormalized());
  }

  /// Update IV const Coeff
  void updateCEs() {
    unsigned Level = Loop->getNestingLevel();
    unsigned RF = RerollFactor;

    ForEach<RegDDRef>::visitRange(
        Loop->child_begin(), Loop->child_end(), [RF, Level](RegDDRef *Ref) {
          for (CanonExpr *CE :
               llvm::make_range(Ref->canon_begin(), Ref->canon_end())) {
            unsigned Index = 0;
            int64_t Coeff = 0;
            if (!CE->hasIV(Level)) {
              continue;
            }
            CE->getIVCoeff(Level, &Index, &Coeff);
            assert(Index == InvalidBlobIndex);

            CE->setIVCoeff(Level, Index, Coeff / static_cast<int64_t>(RF));
          }
        });
  }

  ///  Update redution vars for SR chains (a.k.a non-self-SR,
  ///  a ChainSR used to denote containing more than one instructions).
  ///    S1 = SN + ..
  ///    S2 = S1 + ..
  ///    ...
  ///    SN = SN-1+ ..
  ///  S(N/2) needs to be renamed to SN.
  ///  Which of the reduction vars are live out needs to be made sure.
  void updateChainSRs();

  void invalidate() {
    // Parent loop body doesn't have to be invalidated for DD. DD-analysis knows
    // outer loops has to be re-computed because inner loops have invalidated.
    HIRInvalidationUtils::invalidateBody(Loop);
    HIRInvalidationUtils::invalidateBounds(Loop);
  }

  unsigned RerollFactor;
  const VecRerollSeedInfoTy &VecSeedInfo;
  HLLoop *Loop;
  HIRSafeReductionAnalysis *SRA;
  unsigned NumSelfSR;

private:
  /// Update Reduction Variable for a reduction chain
  void updateChainSRTemps(const SafeRedInfo *SRInfo);
};

void RerollRewriterBase::updateChainSRs() {
  if (!SRA) {
    return;
  }

  const SafeRedInfoList &RedInfoList = SRA->getSafeRedInfoList(Loop);

  for (auto &RedInfo : RedInfoList) {
    if (RedInfo.Chain.size() == 1) {
      // Skip a self SR. This routine is for update RedTemps of ChainSRs.
      continue;
    }

    updateChainSRTemps(&RedInfo);
  }
}

void RerollRewriterBase::updateChainSRTemps(const SafeRedInfo *SRInfo) {
  unsigned LenChain = SRInfo->Chain.size();
  assert(LenChain % RerollFactor == 0);

  RegDDRef *NewLastRedTemp = const_cast<RegDDRef *>(
      SRInfo->Chain[LenChain / RerollFactor - 1]->getLvalDDRef());

  assert(SRInfo->Chain.back()->getLvalDDRef()->getSymbase() == SRInfo->Symbase);

  unsigned RedTempIndex =
      (NewLastRedTemp->getBlobUtils()).findTempBlobIndex(SRInfo->Symbase);
  NewLastRedTemp->replaceSelfBlobIndex(RedTempIndex);
}

/// Rewriting by removing insts after the new last inst.
/// Work when no self SR inst exists.
class FastRerollRewriter : public RerollRewriterBase {
public:
  FastRerollRewriter(unsigned RF, const VecRerollSeedInfoTy &Info,
                     HIRSafeReductionAnalysis *SA)
      : RerollRewriterBase(RF, Info, SA, 0){};
  bool reroll();
};

bool FastRerollRewriter::reroll() {

  if (!HIRTransformUtils::multiplyTripCount(Loop, RerollFactor)) {
    return false;
  }

  // Locate the last inst of the first group
  // Inst at the (II-1) from 0
  unsigned II = VecSeedInfo.size() / RerollFactor;
  HLNodeUtils::remove(
      std::next(HLContainerTy::iterator(VecSeedInfo[II - 1].ContainingInst)),
      Loop->child_end());

  updateChainSRs();
  updateCEs();
  invalidate();

  return true;
}

/// More general reroll rewriter. Can handle self SRs.
class MoveRerollRewriter : public RerollRewriterBase {
public:
  MoveRerollRewriter(unsigned RF, const VecRerollSeedInfoTy &Info,
                     HIRSafeReductionAnalysis *HSRA, unsigned SelfSR,
                     const SelfSRToSeedsTy *SelfSRSeedInfo)
      : RerollRewriterBase(RF, Info, HSRA, SelfSR) {
    SelfSRToSeeds = SelfSRSeedInfo;
  };

  /// Use UpwardTracked Insts in VecSeedInfo[0] through VecSeedInfo[II - 1].
  /// Sort and remove duplicates from the insts.
  /// TempToDDRef is a map from a temp blob to its blob ddref.
  bool reroll(const TempToDDRefTy &TempToDDRef);

private:
  const SelfSRToSeedsTy *SelfSRToSeeds;

  /// Rewrite a self SR Inst for rerolling.
  /// Take as many seeds as (numSeeds / reroll factor) from Seeds,
  /// and attach those to reduction variable using reduction
  /// operator.
  /// numSeeds is the size of the second param passed.
  HLInst *rewriteSelfSR(HLInst *Inst, const SelfSRSeedsTy &Seeds,
                        const TempToDDRefTy &TempToDDRef) const;
};

HLInst *
MoveRerollRewriter::rewriteSelfSR(HLInst *Inst, const SelfSRSeedsTy &Seeds,
                                  const TempToDDRefTy &TempToDDRef) const {

  // Take care of only "+" for stability now.
  // TODO: replace with SelfSRRerollAnalyzer::isSupportedOp
  //       once the review is done.
  //       The class has to moved before this method.
  if (SRA->getSafeRedInfo(Inst)->OpCode != Instruction::Add) {
    llvm_unreachable("Self Safe Reductions with only add reduction operation "
                     "is taken care of by reroller.");
  }
  assert(std::distance(Inst->rval_op_ddref_begin(),
                       Inst->rval_op_ddref_end()) == 2);

  // For ADD instructions
  // Either,
  // S = S + SCEVSeed(1) + SCEVSeed(2) + ... + SCEVSeed(RF)
  // Here (S + .. SCEVSeed(RF-1)) makes the First RvalRef, and SCEVSeed(RF)
  // makes the second RvalRef of ADD instruction.
  //
  // OR
  //
  // S = S + SCEVSeed(1) + SCEVSeed(2) + ... + SCEVSeed(RF-1) + MemRef
  // Here (S + .. SCEVSeed(RF-1)) makes the First RvalRef, and MemRef
  // makes the second RvalRef of ADD instruction.
  // Each SCEVSeed is stored as a blob.

  CanonExpr *FirstCE = Inst->getLvalDDRef()->getSingleCanonExpr()->clone();
  unsigned OrigRedSymbase = Inst->getLvalDDRef()->getSymbase();

  // Add First (RF - 1) SCEVSeeds
  bool MemRefSeen = false;
  BlobUtils &BU = Inst->getBlobUtils();
  assert(Seeds.SCEVSeeds.size() % RerollFactor == 0);
  unsigned NumNewTerms = Seeds.SCEVSeeds.size() / RerollFactor;
  unsigned I = 0;
  for (; I < NumNewTerms - 1; I++) {
    if (!Seeds.SCEVSeeds[I]) {
      // MemRef should exist.
      assert(Seeds.MemRef);
      MemRefSeen = true;
      continue;
    }

    FirstCE->addBlob(BU.findOrInsertBlob(Seeds.SCEVSeeds[I]), 1);
  }

  assert(I == (NumNewTerms - 1));

  BlobTy LastSCEVTerm = Seeds.SCEVSeeds[I];
  RegDDRef *SecondRef;
  if (MemRefSeen) {
    SecondRef = const_cast<RegDDRef *>(Seeds.MemRef);

    // There must be a remaining SCEVSeed at I
    FirstCE->addBlob(BU.findOrInsertBlob(LastSCEVTerm), 1);
  } else {
    if (LastSCEVTerm == nullptr) {
      SecondRef = const_cast<RegDDRef *>(Seeds.MemRef);
    } else {
      // Last SCEVSeed will become a Second Ref
      // TODO: MappedRef is directly found here since only Temp blob seeds
      //       are covered. In order to extent to non-temp blob seeds,
      //       E.G. S = S + (%1*%2) + (%3*%4)
      //       logic should be extended.
      const DDRef *MappedRef = TempToDDRef.find(LastSCEVTerm)->second;
      SecondRef = (Inst->getDDRefUtils())
                      .createSelfBlobRef(MappedRef->getSelfBlobIndex(),
                                         MappedRef->getDefinedAtLevel());
    }
  }

  DDRefUtils &DU = Inst->getDDRefUtils();
  RegDDRef *FirstRef = DU.createScalarRegDDRef(
      NumNewTerms == 1 ? OrigRedSymbase : GenericRvalSymbase, FirstCE);

  RegDDRef *OrigRvalFirst = *(Inst->rval_op_ddref_begin());
  RegDDRef *OrigRvalSecond = *(std::next(Inst->rval_op_ddref_begin()));

  Inst->replaceOperandDDRef(OrigRvalFirst, FirstRef);
  Inst->replaceOperandDDRef(OrigRvalSecond, SecondRef);

  return Inst;
}

bool MoveRerollRewriter::reroll(const TempToDDRefTy &TempToDDRef) {

  if (!HIRTransformUtils::multiplyTripCount(Loop, RerollFactor)) {
    return false;
  }

  // II is the number of seeds to be used in the rerolled loop
  unsigned II = VecSeedInfo.size() / RerollFactor;
  VecNodesTy NewAllInsts;
  for (unsigned I = 0; I < II; I++) {
    NewAllInsts.insert(NewAllInsts.end(),
                       VecSeedInfo[I].TrackedUpwardInsts.begin(),
                       VecSeedInfo[I].TrackedUpwardInsts.end());
  }

  HLNodeUtils::sortInTopOrderAndUniq(NewAllInsts);

  // Should be called before clone. Using SafeRedInfo as Inst a key.
  updateChainSRs();

  for (auto &Node : NewAllInsts) {
    HLInst *Inst = cast<HLInst>(Node);
    SelfSRToSeedsTy::const_iterator It = SelfSRToSeeds->find(Inst);
    if (It != SelfSRToSeeds->end()) {
      rewriteSelfSR(Inst, It->second, TempToDDRef);
    }
  }

  HLNodeUtils::remove(std::next(NewAllInsts.back()->getIterator()),
                      Loop->child_end());
  updateCEs();
  invalidate();

  LLVM_DEBUG(Loop->dump(1));

  return true;
}

bool rewriteLoopBody(unsigned RerollFactor, VecRerollSeedInfoTy &VecSeedInfo,
                     HIRSafeReductionAnalysis &SRA,
                     const SelfSRToSeedsTy &SelfSRToSeeds,
                     const TempToDDRefTy &TempToDDRef) {

  assert(VecSeedInfo.size() % RerollFactor == 0);

  unsigned NumSelfSRs = SelfSRToSeeds.size();
  if (NumSelfSRs == 0) {
    return FastRerollRewriter(RerollFactor, VecSeedInfo, &SRA).reroll();
  }

  return MoveRerollRewriter(RerollFactor, VecSeedInfo, &SRA, NumSelfSRs,
                            &SelfSRToSeeds)
      .reroll(TempToDDRef);
}

class SelfSRRerollAnalyzer {
public:
  class SCEVTermsSortAndReassociater {
  public:
    SCEVTermsSortAndReassociater(VecSCEVTy &SCEVs, const HLInst *SelfInst,
                                 const TempBlobTyToHInstTy &Map,
                                 const BlobUtils &BlobUtil,
                                 const RegDDRef *MR = nullptr)
        : SCEVTerms(SCEVs), MaxTopSortNum(SelfInst->getTopSortNum()),
          MapBlobToDefInst(Map), BU(BlobUtil), MemRef(MR) {}

    // Sort given SCEVTerms using some comparator
    bool sort() {
      if (MemRef) {
        return sortWithMemRef();
      }

      return sortOnlySCEVs();
    }

    // TODO: re-associate if a need arises.
  private:
    /// Sort reduction terms by the order of top sort number
    /// of defining def insts of temp Blobs of one SCEV.
    bool sortOnlySCEVs();

    /// Sort reduction terms by matching MemRefs.
    /// Used when a Self SR Inst has a MemRef within itself.
    bool sortWithMemRef();
    unsigned getMinTopSortNum(BlobTy SCEVTerm);

    /// Input/output of the SCEVTerms.
    VecSCEVTy &SCEVTerms;
    const unsigned MaxTopSortNum;
    const TempBlobTyToHInstTy &MapBlobToDefInst;
    const BlobUtils &BU;
    const RegDDRef *MemRef;
  };

public:
  SelfSRRerollAnalyzer(const SafeRedInfo *SRI, VecSCEVTy &T)
      : SelfInst(SRI->Chain[0]), RedSymbase(SRI->Symbase), OpCode(SRI->OpCode),
        BU(SelfInst->getBlobUtils()),
        RedBlobIndex(SelfInst->getLvalDDRef()->getSelfBlobIndex()),
        RedSCEV(BU.getBlob(RedBlobIndex)), SCEVTerms(T), MemRef(nullptr) {}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print() const {
    dbgs() << "Print from SelfRedSCEVGatherer\n";
    dbgs() << "RedIndex: " << RedBlobIndex << "\n";
    dbgs() << "RedSCEV: ";
    BU.printBlob(dbgs(), RedSCEV);
    dbgs() << "\n";
    if (hasMemRef()) {
      dbgs() << "Has memRef: ", MemRef->dump();
      dbgs() << "\n";
    }

    for (auto Blob : SCEVTerms) {
      if (!Blob) {
        dbgs() << "nullptr,  ";
        continue;
      }
      BU.printBlob(dbgs(), Blob);
      dbgs() << ", ";
    }
    dbgs() << "\n";
  }
#endif

  bool hasMemRef() const { return MemRef != nullptr; }
  const RegDDRef *getMemRef() const { return MemRef; }
  bool analyze(const TempBlobTyToHInstTy &MapBlobToDefInst);

  static bool isSupportedOp(const SafeRedInfo *SRInfo) {
    return isAdd(SRInfo->OpCode);
  }

private:
  static bool isAdd(unsigned OpCode) { return (OpCode == Instruction::Add); }

  bool isAdd() const { return isAdd(OpCode); }

  bool gather(unsigned BlobIndex) {
    const BlobTy Blob = BU.getBlob(BlobIndex);

    if (hasMemRef() && !isa<SCEVUnknown>(Blob)) {
      // Bail-out if a memref present and other
      // SCEV terms are not simple temps.
      // When there is a memref, we expect
      // all other SCEV terms are temp blobs
      // that are defined by a load (i.e. store
      //  %t = A[2*i]
      //  S = S + %t + A[2*i + 1]
      //
      // Otherwise, it is hardly a reroll pattern.
      // For example,
      //  S = S + %1*%2 + A[2*i + 1]
      // is not a reroll pattern for HIR reroll purpose
      return false;
    }

    if (isAdd() || isa<SCEVUnknown>(Blob)) {
      // No further recursion.
      // If the reduction op is ADD,
      // we don't gather further recursively.
      // This function is already called for each CE's blob.

      if (Blob == RedSCEV) {
        return true;
      }

      SCEVTerms.push_back(Blob);
      return true;
    }

    if (auto ConstSCEV = dyn_cast<SCEVConstant>(Blob)) {
      // Constant should be picked up here to be
      // add to comparison of sequences.
      SCEVTerms.push_back(ConstSCEV);

      return true;
    }

    if (auto CastSCEV = dyn_cast<SCEVCastExpr>(Blob)) {
      // We do not go further into cast operator.
      // This makes casts are added to the sequence,
      // and later checked.
      // If casts are stripped off here, casts are not
      // included into the sequence comparison and
      // leading to a wrong result.
      SCEVTerms.push_back(CastSCEV);

      return true;
    }

    if (auto NArySCEV = dyn_cast<SCEVNAryExpr>(Blob)) {

      // SCEVType should match with reduction OpCode
      // Currently, only ADD reductions are covered.
      assert(NArySCEV->getSCEVType() == scAddExpr);

      for (auto ChildBlob :
           make_range(NArySCEV->op_begin(), NArySCEV->op_end())) {
        // Nary-SCEV for the first-depth only
        // If not a reduction var
        if (ChildBlob == RedSCEV) {
          continue;
        }

        SCEVTerms.push_back(ChildBlob);
      }
      return true;
    }

    if (auto UDivSCEV = dyn_cast<SCEVUDivExpr>(Blob)) {

      // SCEVType should match with reduction OpCode
      // because only the top-level ops used for
      // delimiting the scev terms.
      // For now, only ADD reductions are covered,
      // this part is not hit.
      assert(OpCode == Instruction::UDiv);

      SCEVTerms.push_back(UDivSCEV->getLHS());
      SCEVTerms.push_back(UDivSCEV->getRHS());
      return true;
    }

    return false;
  }

  bool gather(const RegDDRef *Ref) {

    assert(Ref->getNumDimensions() == 1);

    const CanonExpr *CE = Ref->getSingleCanonExpr();
    for (auto Blob : make_range(CE->blob_begin(), CE->blob_end())) {
      if (Blob.Index == RedBlobIndex) {
        continue;
      }
      if (Blob.Coeff != 1 || !gather(Blob.Index)) {
        return false;
      }
    }
    return true;
  }

  /// Return true when a self SR is not a reroll candidate
  /// trivially.
  /// For example,
  /// DO // no reroll candidate
  ///   S = S + %a
  /// END DO
  /// This routine sees how many SCEVTerms have matching
  /// def insts. If the counts is less than 2
  /// the self SR Inst is hardly a reroll candidate.
  /// DO
  ///  %a = A[2i]
  ///  %b = A[2i + 1]
  ///  S = S + %a + %b // reroll candidate with two
  ///                  // SCEVTerms %a and %b, which have
  ///                  // matching def Insts.
  /// END DO
  ///
  /// DO
  ///  %a = A[2i]
  ///  S = S + %a + A[2i + 1]  // reroll candidate
  /// END DO
  ///
  /// DO
  ///  %a = A[4i]
  ///  %b = A[4i + 1]
  ///  %c = A[4i + 2]
  ///  %d = A[4i + 3]
  ///  S = S + %a*%b + %c*%d  // no reroll candidate
  ///                         // no matching defs
  /// END DO
  /// The example above is actually a valid reroll candidate.
  /// Current implementation does not handle the case.
  /// Extensions for handling SCEVs' def@level
  /// can be added if a need arises.
  bool isTriviallyNoReroll(const TempBlobTyToHInstTy &MapBlobToDefInst) const {

    unsigned NumSCEVsWithDefs = 0;
    for (auto &Term : SCEVTerms) {
      TempBlobTyToHInstTy::const_iterator It = MapBlobToDefInst.find(Term);
      if (It != MapBlobToDefInst.end() && It->second != SelfInst) {
        NumSCEVsWithDefs++;
      }
    }

    if ((NumSCEVsWithDefs <= 1 && !hasMemRef()) ||
        (NumSCEVsWithDefs == 0 && hasMemRef())) {
      // Hardly a reroll pattern
      return true;
    }

    return SCEVTerms.size() == 0;
  }

public:
  /// gather seed SCEV
  bool gather() {

    if (!isAdd()) {
      // bail-out for now.
      // Some of the downstream logics only works for add.
      // Upstream logic already bails out in case the reduction op is not
      // add. However the upstream logic is outside of this class imple.
      // p = p * -3; "-3" comes into as a Ref
      return false;
    }

    // Preprocessing to see if there is a memref.
    for (const RegDDRef *Ref : make_range(SelfInst->rval_op_ddref_begin(),
                                          SelfInst->rval_op_ddref_end())) {
      if (Ref->isMemRef()) {
        MemRef = Ref;
        break;
      }
    }

    for (const RegDDRef *Ref : make_range(SelfInst->rval_op_ddref_begin(),
                                          SelfInst->rval_op_ddref_end())) {
      if (Ref->getSymbase() == RedSymbase || Ref->isMemRef()) {
        // Reduction variable itself is not a term of interest.
        continue;
      }

      if (!gather(Ref)) {
        return false;
      }
    }

    return true;
  }

  void sortAndReassociateTerms(const TempBlobTyToHInstTy &MapBlobToDefInst) {
    SCEVTermsSortAndReassociater(SCEVTerms, SelfInst, MapBlobToDefInst, BU,
                                 MemRef)
        .sort();
  }

private:
  const HLInst *SelfInst;
  const unsigned RedSymbase;
  const unsigned OpCode;
  const BlobUtils &BU;
  const unsigned RedBlobIndex;
  BlobTy RedSCEV;
  VecSCEVTy &SCEVTerms;
  const RegDDRef *MemRef;
};

bool SelfSRRerollAnalyzer::SCEVTermsSortAndReassociater::sortWithMemRef() {
  // This is for the case, a memref appears within the Self SR inst itself.
  // Example:
  // S = S + %a + %b + A[3i + 1]
  // Two SCEVs %a and %b must have matching MemRefs
  // for example,
  //   %a = A[3i];
  //   %b = A[3i + 3];
  // Otherwise, this self SR would not make a valid reroll candidate.

  SmallVector<const RegDDRef *, 16> MemRefs;

  // All SCEVs should be replaceable by one MemRef.
  // This is a reverse map from MemRef to SCEV.
  // A MemRef can have NULL SCEV. For example, A[3i + 1]
  // in the description above has no matching SCEV,
  // as it appears directly within a self SR inst.
  DenseMap<const RegDDRef *, BlobTy> MemRefToSCEV;

  for (auto &SCEVTerm : SCEVTerms) {
    TempBlobTyToHInstTy::const_iterator It = MapBlobToDefInst.find(SCEVTerm);
    if (It == MapBlobToDefInst.end()) {
      return false;
    }

    const HLInst *DefInst = It->second;
    const RegDDRef *RvalRef = DefInst->getRvalDDRef();
    if (!RvalRef || !RvalRef->isMemRef()) {
      // I.E. underlying LLVM inst is not StoreInst
      return false;
    }
    MemRefs.push_back(RvalRef);
    MemRefToSCEV[RvalRef] = SCEVTerm;
  }
  MemRefs.push_back(MemRef);

  // Sort MemRefs
  std::sort(MemRefs.begin(), MemRefs.end(), DDRefUtils::compareMemRef);
  SCEVTerms.clear();
  for (auto &MemRef : MemRefs) {
    // Intentionally put nullptr for MemRef without Matching SCEV
    SCEVTerms.push_back(MemRefToSCEV[MemRef]);
  }
  return true;
}

unsigned SelfSRRerollAnalyzer::SCEVTermsSortAndReassociater::getMinTopSortNum(
    BlobTy SCEVTerm) {

  VecSCEVTy TempBlobs;
  BU.collectTempBlobs(SCEVTerm, TempBlobs);

  unsigned Min = MaxTopSortNum;
  for (auto &Temp : TempBlobs) {
    TempBlobTyToHInstTy::const_iterator It = MapBlobToDefInst.find(Temp);
    if (It != MapBlobToDefInst.end()) {
      unsigned TopSortNum = It->second->getTopSortNum();
      if (TopSortNum < Min) {
        Min = TopSortNum;
      }
    }
  }
  return Min;
}

bool SelfSRRerollAnalyzer::SCEVTermsSortAndReassociater::sortOnlySCEVs() {
  // Get Min of TopSortNumber of DefInsts of a SCEVTerm
  // Use the Min values for sorting SCEVTerms
  // For example, for sorting two SCEV terms (%a*%b) and (%c*%d),
  // consider definitions of four temps.
  // %a = ..  -- TopSortNum: 1
  // %b = ..  -- TopSortNum: 2
  // %c = ..  -- TopSortNum: 3
  // %d = ..  -- TopSortNum: 4
  // Sort (%a*%b) and (%c*%d) by 1 and 3, as 1 and 3 are the mininum of
  // (1 and 2) and (3 and 4).

  std::sort(SCEVTerms.begin(), SCEVTerms.end(),
            [this](const BlobTy SCEVTerm1, const BlobTy SCEVTerm2) {
              return this->getMinTopSortNum(SCEVTerm1) <
                     this->getMinTopSortNum(SCEVTerm2);
            });

  return true;
}

bool SelfSRRerollAnalyzer::analyze(
    const TempBlobTyToHInstTy &MapBlobToDefInst) {
  if (!gather()) {
    return false;
  }

  if (isTriviallyNoReroll(MapBlobToDefInst)) {
    return false;
  }

  sortAndReassociateTerms(MapBlobToDefInst);

  LLVM_DEBUG(dbgs() << "Print after sort and re-asso:\n");
  LLVM_DEBUG(print());

  return true;
}

/// Return a RvalRef, which is NOT a reduction temp.
/// For example,
/// given S1 = S2 + A[i], this function returns A[i];
/// given S1 = S2 + %t, this function returns %t.
/// S is the reduction temp in the examples above.
/// Param Inst should be an Instruction in a reduction chain,
/// Param Chain is the reduction chain Inst belongs to,
/// and Param Index is the index of Inst in Chain.
/// For example, for a chain
///     S1 = S3 + A[i]    // (1)
///     S2 = S1 + A[i+1]  // (2)
///     S3 = S2 + A[i+2]  // (3)
/// (1), (2) and (3) are three insts in a chain, with its indices are 0, 1, and
/// 2 respectively. getNonReductionRval((1), Chain, 0) will return A[i]. This
/// function assums the reduction Chain is formed in the way a rotation-right
/// relationship between Lval reduction temp and Rval temp holds. Lval reduction
/// temps:(S1, S2, S3)
///    -- rotation-right -->
/// Rval reduction temps:(S3, S1, S2)
/// This function is for a chained safe reductions, NOT for a Self safe
/// reduction.
const RegDDRef *getNonReductionRval(const HLInst *Inst,
                                    const SafeRedChain *Chain, unsigned Index) {

  assert(std::distance(Inst->rval_op_ddref_begin(),
                       Inst->rval_op_ddref_end()) == 2);

  unsigned NumRedTemps = Chain->size();
  unsigned RedTempSymbaseAtRval =
      ((*Chain)[(Index + (NumRedTemps - 1)) % NumRedTemps])
          ->getLvalDDRef()
          ->getSymbase();

  // Inspect Inst
  for (auto RegRef :
       make_range(Inst->rval_op_ddref_begin(), Inst->rval_op_ddref_end())) {
    if (RegRef->getSymbase() != RedTempSymbaseAtRval) {
      return RegRef;
    }
  }

  return nullptr;
}

/// Return true for supported chain reduction operations.
/// Note that for a self safe reduction we have
/// separate set of supported operations.
bool isSupportedOpForChainSR(const SafeRedInfo *SRInfo) {
  return SRInfo->OpCode == Instruction::FAdd ||
         SRInfo->OpCode == Instruction::FSub;
}

bool buildFromChainSRInst(HLInst *HInst, const HLLoop *Loop,
                          const SafeRedInfo *SRInfo, DDGraph &DDG,
                          unsigned NthRedInst, VecCEOpSeqTy &VecSeq,
                          VecRerollSeedInfoTy &VecSeedInfo) {
  // For now for stability
  if (!isSupportedOpForChainSR(SRInfo)) {
    return false;
  }

  const RegDDRef *RVal =
      getNonReductionRval(HInst, &(SRInfo->Chain), NthRedInst);
  if (!RVal) {
    return false;
  }

  VecSeq.push_back(CEOpSequence());

  VecSeedInfo.push_back(RerollSeedInfo(HInst, RVal));

  if (!extendSeq<SequenceBuilderForLoop, HLLoop>(
          RVal, Loop, DDG, VecSeq.back(),
          VecSeedInfo.back().TrackedUpwardInsts)) {
    return false;
  }

  LLVM_DEBUG(dumpOprdOpSequence(HInst, VecSeq.back()));
  return true;
}

bool genTempStackAndTrack(HLInst *HInst, DDGraph DDG, BlobTy SCEVTerm,
                          const TempToDDRefTy &TempToDDRef,
                          VecCEOpSeqTy &VecSeq,
                          VecRerollSeedInfoTy &VecSeedInfo) {

  // Push blobDDRefs within the SCEVTerm into stack
  VecSCEVTy TempBlobs;
  (HInst->getBlobUtils()).collectTempBlobs(SCEVTerm, TempBlobs);
  TempStackTy TempStack;
  const HLLoop *ParentLoop = HInst->getParentLoop();
  unsigned Level = ParentLoop->getNestingLevel();

  for (BlobTy TempBlob : TempBlobs) {
    const DDRef *TempDDRef = TempToDDRef.find(TempBlob)->second;
    if (!TempDDRef) {
      LLVM_DEBUG(dbgs() << "Self SR: Temp's blob ddref is not found!\n");
      return false;
    }

    if (TempDDRef->getSingleCanonExpr()->isLinearAtLevel(Level)) {
      continue;
    }
    TempStack.push(TempDDRef);
  }

  if (TempStack.size() == 0) {
    return false;
  }

  VecSeedInfo.push_back(RerollSeedInfo(HInst, SCEVTerm));
  SequenceBuilderForLoop Builder(&TempStack, ParentLoop, DDG, VecSeq.back(),
                                 VecSeedInfo.back().TrackedUpwardInsts);
  if (!Builder.trackTemps()) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "\nPrint Seq : ");
  LLVM_DEBUG(VecSeq.back().printCEs());

  return true;
}

/// Generate seeds for a self safe reduction instruction.
/// A SCEV term of a self SR stmt makes a seed.
/// By definition of reroll, a self SR stmt may have as many as RF SCEV terms.
/// Return how many SCEV seeds are generated.
unsigned buildFromSelfSRInst(HLInst *HInst, const HLLoop *Loop,
                             const SafeRedInfo *SRInfo, DDGraph &DDG,
                             const TempToDDRefTy &TempToDDRef,
                             const TempBlobTyToHInstTy &MapBlobToDefInst,
                             VecCEOpSeqTy &VecSeq,
                             VecRerollSeedInfoTy &VecSeedInfo,
                             SelfSRToSeedsTy &SelfSRToSeeds) {

  if (!SelfSRRerollAnalyzer::isSupportedOp(SRInfo)) {
    // Bail-out for not for non ADD reduction
    return 0;
  }

  VecSCEVTy SCEVTerms;
  SelfSRRerollAnalyzer SelfTermGathers(SRInfo, SCEVTerms);
  if (!SelfTermGathers.analyze(MapBlobToDefInst)) {
    return 0;
  }

  SelfSRToSeeds.insert(std::make_pair(
      HInst, SelfSRSeedsTy(SCEVTerms, SelfTermGathers.getMemRef())));

  BlobUtils &BU = HInst->getBlobUtils();
  CanonExprUtils &CU = HInst->getCanonExprUtils();

  bool HasMemRef = SelfTermGathers.hasMemRef();
  if (!HasMemRef) {
    for (BlobTy SCEVTerm : SCEVTerms) {
      assert(SCEVTerm);

      // Gen CE out of SCEV and add it into VecSeq
      // TODO: See if the following is enough. Memory reclaim needed?
      // Preprocess: add SCEVTerm's  into CEList
      unsigned BlobIndex = BU.findOrInsertBlob(SCEVTerm);

      // TODO: may need exact level
      // Comp-fail - no-mappedRef
      // Set the defined level of SCEVTerm correctly
      CanonExpr *CE =
          CU.createCanonExpr(SCEVTerm->getType(), Loop->getNestingLevel());
      CE->addBlob(BlobIndex, 1);
      VecSeq.push_back(CEOpSequence());
      VecSeq.back().add(CE);

      // Populate TempStack and Start tracking from this seed
      if (!genTempStackAndTrack(HInst, DDG, SCEVTerm, TempToDDRef, VecSeq,
                                VecSeedInfo)) {
        return 0;
      }
    }

    return SCEVTerms.size();
  }

  // With MemRef as one seed
  for (BlobTy SCEVTerm : SCEVTerms) {
    if (SCEVTerm == nullptr) {
      // This is a MemRef seed without no SCEV
      // ie. A[i+1] in S = S + %1 + A[i+1]
      VecSeedInfo.push_back(
          RerollSeedInfo(HInst, SelfTermGathers.getMemRef(), true));
      VecSeq.push_back(CEOpSequence());

      VecSeq.back().addOpcodeToSeq(Instruction::Load);
      VecSeq.back().add(SelfTermGathers.getMemRef());

      continue;
    }

    // Do not generated CE from SCEVs
    // Populate TempStack and Start tracking from this seed
    VecSeq.push_back(CEOpSequence());
    if (!genTempStackAndTrack(HInst, DDG, SCEVTerm, TempToDDRef, VecSeq,
                              VecSeedInfo)) {
      return 0;
    }
  }

  LLVM_DEBUG(dbgs() << "=== VecSeq Size: " << VecSeq.size() << " ====\n");
  for (auto &Seq :
       make_range(std::prev(VecSeq.end(), SCEVTerms.size()), VecSeq.end())) {
    (void)Seq;
    LLVM_DEBUG(dumpOprdOpSequence(HInst, Seq));
  }

  return SCEVTerms.size();
}

bool areRerollSequencesBuilt(const HLLoop *Loop, HIRDDAnalysis &DDA,
                             HIRSafeReductionAnalysis &SRA,
                             const TempToDDRefTy &TempToDDRef,
                             VecCEOpSeqTy &VecSeq,
                             VecRerollSeedInfoTy &VecSeedInfo,
                             SelfSRToSeedsTy &SelfSRToSeeds) {

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

  unsigned NthRedInst = 0;
  TempBlobTyToHInstTy MapLvalBlobToDefInst;
  const BlobUtils &BU = Loop->getBlobUtils();
  for (const HLNode &Node :
       make_range(Loop->child_begin(), Loop->child_end())) {

    // WHERE CONST propagation ends
    HLInst *HInst = const_cast<HLInst *>(cast<HLInst>(&Node));
    // TODO: Handle Cmp and Select if needed.
    const Instruction *Inst = HInst->getLLVMInstruction();
    if (isa<CmpInst>(Inst) || isa<SelectInst>(Inst)) {
      return false;
    }
    bool IsStore = isa<StoreInst>(Inst);

    const RegDDRef *LVal = HInst->getLvalDDRef();
    if (LVal->isSelfBlob()) {
      // Build map from TempBlob to HInst map.
      // HInst's topological sort.
      // Can we use a blob util? isInstBlob? getTempBlobValue?
      MapLvalBlobToDefInst[BU.getBlob(LVal->getSelfBlobIndex())] = HInst;
    }

    // A store inst is a seed: collected all ddrefs involved
    // in the store.
    if (IsStore) {

      LLVM_DEBUG(dbgs() << "Hitting Store Inst\n");
      if (!buildFromStoreInst<SequenceBuilderForLoop, HLLoop>(
              HInst, Loop, DDG, VecSeq, VecSeedInfo)) {
        return false;
      }
      LLVM_DEBUG(dumpOprdOpSequence(HInst, VecSeq.back()));

      MarkConsumedInsts(VecSeedInfo.back().TrackedUpwardInsts);
      continue;
    }

    SRA.computeSafeReductionChains(Loop);
    const SafeRedInfo *SRInfo = SRA.getSafeRedInfo(HInst);

    if (!SRInfo) {
      continue;
    }

    bool IsSelfSR = SRInfo->Chain.size() == 1;
    if (IsSelfSR) {
      LLVM_DEBUG(dbgs() << "Hitting Self SR Inst\n");

      unsigned NumSeeds = buildFromSelfSRInst(
          HInst, Loop, SRInfo, DDG, TempToDDRef, MapLvalBlobToDefInst, VecSeq,
          VecSeedInfo, SelfSRToSeeds);

      if (NumSeeds == 0) {
        return false;
      }

      for (auto &SeedInfo : make_range(std::prev(VecSeedInfo.end(), NumSeeds),
                                       VecSeedInfo.end())) {
        MarkConsumedInsts(SeedInfo.TrackedUpwardInsts);
      }
    } else {
      LLVM_DEBUG(dbgs() << "Hitting A SR Inst\n");

      if (!buildFromChainSRInst(HInst, Loop, SRInfo, DDG, NthRedInst, VecSeq,
                                VecSeedInfo)) {
        return false;
      }
      NthRedInst++;

      MarkConsumedInsts(VecSeedInfo.back().TrackedUpwardInsts);
    }
  }

  if (!AreAllInstsConsumed()) {
    // Some insts in the loop body is not participating in the rerolling
    // pattern.
    LLVM_DEBUG(dbgs() << "Reroll: not all instructions are covered\n");
    return false;
  }

  if (VecSeq.size() <= 1) {
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

// If any non-safe-reduction-var live-out exists,
// bail-out.
// We can check this after rerolling loop if some of the
// remove lval temps are live-out. For now,
// we only take care of safe reductions. We don't need to
// delay this check after reroll.
bool hasLiveOutTempsToBeRemoved(const HLLoop *Loop,
                                HIRSafeReductionAnalysis *SRA) {
  if ((!SRA || (SRA->getSafeRedInfoList(Loop)).size() == 0) &&
      Loop->hasLiveOutTemps()) {
    return true;
  }

  for (const HLNode &Node :
       make_range(Loop->child_begin(), Loop->child_end())) {
    const HLInst *HInst = dyn_cast<HLInst>(&Node);

    if (!HInst) {
      continue;
    }

    const RegDDRef *Lval = HInst->getLvalDDRef();
    unsigned OpCode;
    if (Loop->isLiveOut(Lval->getSymbase()) &&
        !SRA->isReductionRef(Lval, OpCode)) {
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
                         HIRLoopStatistics &HLS,
                         HIRSafeReductionAnalysis &SRA) {
  DDRefScavenger RefScavenger(Loop);
  if (!isRerollCandidate(Loop, HLS, RefScavenger)) {
    return false;
  }

  // One Seq for a seed. VecSeq is a vector of Seqs from all seeds.
  VecCEOpSeqTy VecSeq;

  // One SeedInfo for a seed. VecSeedInfo is a vector of Info of all seeds.
  VecRerollSeedInfoTy VecSeedInfo;

  TempToDDRefTy TempToDDRef;
  RefScavenger.populateMapFromSCEVToBlobDDRef(TempToDDRef);
  SelfSRToSeedsTy SelfSRToSeeds;
  if (!areRerollSequencesBuilt(Loop, DDA, SRA, TempToDDRef, VecSeq, VecSeedInfo,
                               SelfSRToSeeds)) {
    return false;
  }

  LoopInvariantBlobTy LoopInvariantBlobs;
  RefScavenger.populateLoopInvariantBlobs(LoopInvariantBlobs);

  unsigned RerollFactor = 0;
  unsigned InitInterval = 0;
  SequenceChecker SeqChecker(Loop, LoopInvariantBlobs);
  std::tie(RerollFactor, InitInterval) = SeqChecker.calcRerollFactor(VecSeq);

  if (RerollFactor <= 1) {
    LLVM_DEBUG(dbgs() << "Reroll Factor is NOT calculated.\n");
    return false;
  }

  if (hasLiveOutTempsToBeRemoved(Loop, &SRA)) {
    LLVM_DEBUG(dbgs() << "Reroll Factor " << RerollFactor
                      << ". But, Has Live out to be removed. Bail out.\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "Reroll Factor: " << RerollFactor << "\n");

  if (!rewriteLoopBody(RerollFactor, VecSeedInfo, SRA, SelfSRToSeeds,
                       TempToDDRef)) {
    return false;
  }

  return true;
}

unsigned doLoopReroll(HIRFramework &HIRF, HIRDDAnalysis &DDA,
                      HIRLoopStatistics &HLS, HIRSafeReductionAnalysis &SRA) {
  unsigned NumLoopsRerolled = 0;
  for (HLRangeIterator It = HLRangeIterator(HIRF.hir_begin()),
                       EIt = HLRangeIterator(HIRF.hir_end());
       It != EIt; ++It) {
    if (HLLoop *Loop = dyn_cast<HLLoop>(*It)) {
      if (rerollStraightCodes(Loop, DDA, HLS, SRA)) {
        ++NumLoopsRerolled;
      }
    }
  }

  return NumLoopsRerolled;
}

} // namespace

char HIRLoopRerollLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopRerollLegacyPass, OPT_SWITCH, OPT_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
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
  auto &SRA = getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR();

  unsigned NumRerolled = doLoopReroll(HIRF, DDA, HLS, SRA);
  LoopsRerolled += NumRerolled;
  if (NumRerolled > 0) {
    LLVM_DEBUG(dbgs() << "Reroll happend\n");
  }

  return NumRerolled > 0;
}

PreservedAnalyses HIRLoopRerollPass::run(llvm::Function &F,
                                         llvm::FunctionAnalysisManager &AM) {
  if (DisablePass) {
    return PreservedAnalyses::all();
  }

  LLVM_DEBUG(dbgs() << OPT_DESC " for Function : " << F.getName() << "\n");

  unsigned NumRerolled = doLoopReroll(
      AM.getResult<HIRFrameworkAnalysis>(F), AM.getResult<HIRDDAnalysisPass>(F),
      AM.getResult<HIRLoopStatisticsAnalysis>(F),
      AM.getResult<HIRSafeReductionAnalysisPass>(F));
  LoopsRerolled += NumRerolled;
  if (NumRerolled > 0) {
    LLVM_DEBUG(dbgs() << "Reroll happend\n");
  }

  return PreservedAnalyses::all();
}
// TODO: alias info needs to be checked for safety? (In what cases? We don't
// change the order of execution.)
