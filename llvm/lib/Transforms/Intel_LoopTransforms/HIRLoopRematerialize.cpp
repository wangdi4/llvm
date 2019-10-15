//===- HIRLoopRematerialize.cpp - Implements Loop Rematerialize  ---------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

// A pass materialize a loop from a straight line of codes
// E.g.
// From
//    %t1  = b[0] - c[0]
//    a[0] = %t1
//    %t2  = b[0] - c[0]
//    a[0] = %t2
//
// To
//    DO Loop i1 = 0 to 1
//        %t1 = b[i1] - c[i1]
//        a[i1] = %t1
//    END DO
//
// Currently, only works for the body of a region, marked as
// loop materialization candidate.
// Takes a store inst its only seed.

// TODO: Consolidate common data structures with
//       HIRLoopReroll.
//       Cares are needed since many of them
//       are slightly different.
//
//       Also, currently, only rematerializing a loop in a HLRegion
//       is implemented. In other words, no loop body is
//       rematerialized into an inner loop.
//       Some of the data structures are template
//       to be extended to handle rematerializing within an existing
//       loop body. However, instantiation with HLLoop is not implemented
//       in many places.
//
//       Loop Invariant parts are not hoisted out. If repeated calculation
//       leads to incorrectness, just bailed-out by various bailing out logics.
//
//       For general loop rematerialization following parts should be designed.
//       - SequenceBuild Logic for loop. (Where to stop temp tracking?)
//       - Update Loop livein/out info (Cares are needed not to be too
//       conservative).
//       - Probably, taking care of in-between independent insts in
//         rematerialization range.
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopRematerialize.h"

#include "HIRReroll.h"
#include "llvm/ADT/SmallSet.h"
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

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::reroll;

#define OPT_SWITCH "hir-loop-rematerialize"
#define OPT_DESC "HIR Loop Rematerialize"
#define DEBUG_TYPE OPT_SWITCH

STATISTIC(LoopsRematerialized, "Number of HIR loops rematerized");

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

// A handle for disabling rematerialization based on trip counts.
// At least this large TC is required to activate rematerialization
// Notice TC = 1 never enables rematerialization pass regardless of
// LoopRematerializeTCLowerBound.
static cl::opt<unsigned> LoopRematerializeTCLowerBound(
    OPT_SWITCH "-tc-lb", cl::init(3), cl::Hidden,
    cl::desc("Minimal Trip Count enabling " OPT_DESC " pass"));

namespace {

class HIRLoopRematerializeLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRLoopRematerializeLegacyPass() : HIRTransformPass(ID) {
    initializeHIRLoopRematerializeLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override{};

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();

    AU.setPreservesAll();
  }
};

} // namespace

namespace {

// Hard to estimate the size: not using small vector.
typedef std::set<const HLNode *> SetNodesTy;        // SmallSet allows < 32
typedef DenseMap<BlobTy, unsigned> InvariantBlobTy; // SCEV, BID

/// SequenceBuilder for Rematerialization.
/// It is for rematerialization of flat codes in a HLRegion
/// (i.e. not in loop). HLRegion is a LoopMaterializationCandidate.
/// Main logic for building sequences is in the base class.
class SequenceBuilderForRematerialze
    : public SequenceBuilder<SequenceBuilderForRematerialze, HLRegion> {

public:
  explicit SequenceBuilderForRematerialze(const HLRegion *Region, DDGraph &G,
                                          CEOpSequence &Seq, VecNodesTy &IL)
      : SequenceBuilder<SequenceBuilderForRematerialze, HLRegion>(
            nullptr, Region, G, Seq, IL) {}

  HLInst *findTempDef(const DDRef *TempRef) const {
    for (DDEdge *E : DDG.incoming(TempRef)) {
      if (E->isFlow()) {
        DDRef *Src = E->getSrc();
        if (HLInst *DefInst = dyn_cast<HLInst>(Src->getHLDDNode())) {
          return DefInst;
        }
      }
    }
    return nullptr;
  }

  bool stopTrackingTemp(const RegDDRef *Ref) const {
    return Container->isLiveIn(Ref->getSymbase());
  }
};

} // namespace

namespace {

// TODO: make it a static member of HIRLoopMaterialize
static bool isNonHandlableInst(const HLInst *HInst) {
  const Instruction *Inst = HInst->getLLVMInstruction();
  if (isa<CmpInst>(Inst) || isa<SelectInst>(Inst) || isa<AllocaInst>(Inst) ||
      HInst->isCallInst()) {
    return true;
  }
  return false;
}

template <typename ContainerTypename>
bool buildSequences(ContainerTypename *Parent, HLContainerTy::iterator Begin,
                    HLContainerTy::iterator End, HIRDDAnalysis &DDA,
                    VecCEOpSeqTy &VecSeq, VecSeedInfoTy &VecSeedInfo,
                    SetNodesTy &TrackedUpwardInsts) {

  // Map to make sure consumption of all instructions in the loop body
  auto MarkConsumedInsts = [&TrackedUpwardInsts](const VecNodesTy &InstList) {
    for (auto Node : InstList) {
      TrackedUpwardInsts.insert(Node);
    }
  };

  DDGraph DDG = DDA.getGraph(Parent);
  LLVM_DEBUG(DDG.dump());

  for (HLNode &Node : make_range(Begin, End)) {

    HLInst *HInst = dyn_cast<HLInst>(&Node);
    if (!HInst) {
      return false;
    }

    if (isNonHandlableInst(HInst)) {
      return false;
    }

    const Instruction *Inst = HInst->getLLVMInstruction();
    if (!isa<StoreInst>(Inst)) {
      continue;
    }

    // A store inst is a seed: collected all ddrefs involved
    // in the store.
    if (!buildFromStoreInst<SequenceBuilderForRematerialze, HLRegion>(
            HInst, Parent, DDG, VecSeq, VecSeedInfo)) {
      return false;
    }
    LLVM_DEBUG(dumpOprdOpSequence(HInst, VecSeq.back()));

    MarkConsumedInsts(VecSeedInfo.back().TrackedUpwardInsts);
  }
  return true;
}

// Use map not DenseMap. Don't know how large it will be.
typedef std::map<const CanonExpr *, int64_t> MapCEToDistTy;

class SequenceChecker {
private:
  /// Difference of two CELists. It is a diff vector as long as the
  /// the length of the given CEList.
  typedef std::vector<int64_t> CEListDiffTy;
  typedef std::vector<CEListDiffTy> VecCEListDiffTy;

public:
  explicit SequenceChecker(const InvariantBlobTy &Invs, MapCEToDistTy &Map)
      : InvariantBlobs(Invs), MapCEToDist(Map) {}

  /// RerollFactor and Initiation interval
  /// Return reroll factor(RF) and initiation interval(II).
  ///
  ///  RF = (total number of seeds) / II
  ///
  /// When no rerolling is possible, reroll factor 0 and initiation interval 0
  /// are returned.
  std::pair<unsigned, unsigned>
  calcRerollFactor(const VecCEOpSeqTy &VecSeq) const;

private:
  bool areEqualBlobTyForReroll(const BlobTy &Blob1, const BlobTy &Blob2) const;

  /// Different from reroll, we don't have enclosing loop.
  bool isBlobsMathchedForReroll(const CanonExpr *CE1,
                                const CanonExpr *CE2) const;
  /// Get the constant distance of each element of CEList1 and
  /// CEList2. Store the result into DiffVec.
  bool getDistance(const VecCEsTy &CEList1, const VecCEsTy &CEList2,
                   CEListDiffTy &DiffVec) const;

  /// Minimal checks to see if Seqs are by grouped by II consecutive sequences.
  /// Common logic to reroll.
  /// TODO: refactor the common part.
  bool preliminaryChecks(const unsigned II, const VecCEOpSeqTy &VecSeq) const;

  /// See if all II-sized sequence groups have matching distances.
  bool isDistanceMatched(const unsigned II, const VecCEOpSeqTy &VecSeq) const;

  /// Fill in the map from a CE to its distance
  /// We need distance for each CE. For example,
  /// a[0] = b[1];
  /// a[1] = b[4];
  /// Where II=1, RF = 2,
  /// For first  0 and 1 -->dist 1
  /// For second 1 and 4 --> dist 3
  /// Rematerialized like
  /// DO i
  ///   a[1*i] = b[3*i + 1];
  /// END DO
  /// Record the info about the first II sequences, because we are using
  /// first II sequences for loop body of a newly materialized loop.
  bool recordDists(unsigned II, const VecCEListDiffTy &VecCEListDiff,
                   const VecCEOpSeqTy &VecSeq) const;

private:
  const InvariantBlobTy &InvariantBlobs;

  /// Map from one CE in CEList to its Dist
  MapCEToDistTy &MapCEToDist;
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
    if (InvariantBlobs.find(Blob1) == InvariantBlobs.end()) {
      // Defer the comparison towards its defining instruction.
      return true;
    }
    return Blob1 == Blob2;
  }

  llvm_unreachable("Unknown Blob type!");
  return false;
}

bool SequenceChecker::isBlobsMathchedForReroll(const CanonExpr *CE1,
                                               const CanonExpr *CE2) const {
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

  const BlobUtils &BU = CE1->getBlobUtils();
  const InvariantBlobTy &Invs = InvariantBlobs;
  auto TempCompare = [&BU, &Invs](const BlobCoeffAndIndexTy &P1,
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

bool SequenceChecker::getDistance(const VecCEsTy &CEList1,
                                  const VecCEsTy &CEList2,
                                  CEListDiffTy &DiffVec) const {

  for (VecCEsTy::const_iterator I1 = CEList1.begin(), I2 = CEList2.begin(),
                                EI1 = CEList1.end();
       I1 != EI1; ++I1, ++I2) {
    const CanonExpr *CE1 = *I1;
    const CanonExpr *CE2 = *I2;

    if (CE1->getDestType() != CE2->getDestType() ||
        CE1->getSrcType() != CE2->getSrcType()) {
      return false;
    }

    int64_t Dist;
    if (CanonExprUtils::getConstDistance(CE1, CE2, &Dist)) {
      DiffVec.push_back(Dist);
      continue;
    }

    if (CE1->isConstant() || CE2->isConstant()) {
      // Purely different constants
      return false;
    }

    // They are not pure constants but may have constant parts
    // e.g. %a + 1 or %b - 1
    if (CE1->getConstant() != CE2->getConstant()) {
      return false;
    }

    // If there were differences in IVs, return false
    for (auto Level :
         make_range(AllLoopLevelRange::begin(), AllLoopLevelRange::end())) {
      const CanonExpr *CEs[2] = {CE1, CE2};
      unsigned Indexes[2] = {InvalidBlobIndex, InvalidBlobIndex};
      int64_t Coeffs[2] = {0, 0};
      CEs[0]->getIVCoeff(Level, &Indexes[0], &Coeffs[0]);
      CEs[1]->getIVCoeff(Level, &Indexes[1], &Coeffs[1]);

      if (Indexes[0] != Indexes[1] || Coeffs[0] != Coeffs[1]) {

        return false;
      }
    }

    // No blob difference
    if (!isBlobsMathchedForReroll(CE1, CE2)) {
      return false;
    }

    // These blobs passed isBlobMatchedForReroll.
    // Record the distance as 0.
    // It is all right because later part of the sequence
    // will tell actual distance between the blobs.
    DiffVec.push_back(0);
  }

  return true;
}

bool isSameTrailingOffsets(const VecRefsTy &MemRefs1,
                           const VecRefsTy &MemRefs2) {

  unsigned Size = MemRefs1.size();
  if (Size != MemRefs2.size()) {
    return false;
  }

  // Skip inspecting baseCEs, since will be done later.
  // Inspect trailing offsets.
  for (VecRefsTy::const_iterator I1 = MemRefs1.begin(), I2 = MemRefs2.begin(),
                                 E1 = MemRefs1.end();
       I1 != E1; ++I1, ++I2) {
    unsigned NumDims = (*I1)->getNumDimensions();
    if (NumDims != (*I2)->getNumDimensions()) {
      return false;
    }

    for (unsigned J = 1; J <= NumDims; J++) {
      ArrayRef<unsigned> Offsets1 = (*I1)->getTrailingStructOffsets(J);
      ArrayRef<unsigned> Offsets2 = (*I2)->getTrailingStructOffsets(J);
      if (Offsets1.size() != Offsets2.size() ||
          !std::equal(Offsets1.begin(), Offsets1.end(), Offsets2.begin())) {
        return false;
      }
    }
  }

  return true;
}

bool SequenceChecker::preliminaryChecks(const unsigned II,
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

  for (unsigned J = 0; J < II; J++) {
    for (unsigned K = J; K + II < VecSize; K += II) {
      if (!isSameTrailingOffsets(VecSeq[K].MemRefs, VecSeq[K + II].MemRefs)) {
        return false;
      }
    }
  }

  return true;
}

bool SequenceChecker::isDistanceMatched(const unsigned II,
                                        const VecCEOpSeqTy &VecSeq) const {

  unsigned VecSize = VecSeq.size();

  // CEs in adjacent sequences should be in right distance apart.
  // J loop scans through first II CELists
  // Vector of CEListDiffs. We need II of those.
  VecCEListDiffTy VecCEListDiff;
  for (unsigned J = 0; J < II; J++) {

    CEListDiffTy LeadDiffVec;
    if (J + II < VecSize) {
      // Initial case
      if (!getDistance(VecSeq[J].CEList, VecSeq[J + II].CEList, LeadDiffVec)) {
        return false;
      }
    } else {
      return false;
    }
    VecCEListDiff.push_back(LeadDiffVec);

    for (unsigned K = J + II; K + II < VecSize; K += II) {
      CEListDiffTy DiffVec;
      if (!getDistance(VecSeq[K].CEList, VecSeq[K + II].CEList, DiffVec)) {
        return false;
      }

      assert(DiffVec.size() == LeadDiffVec.size());

      if (!std::equal(LeadDiffVec.begin(), LeadDiffVec.end(),
                      DiffVec.begin())) {
        return false;
      }
    }
  }

  return recordDists(II, VecCEListDiff, VecSeq);
}

bool SequenceChecker::recordDists(unsigned II,
                                  const VecCEListDiffTy &VecCEListDiff,
                                  const VecCEOpSeqTy &VecSeq) const {

  assert(VecCEListDiff.size() == II);
  MapCEToDist.clear();

  for (unsigned J = 0; J < II; J++) {
    unsigned CEListLen = VecSeq[J].CEList.size();
    assert(CEListLen == VecCEListDiff[J].size());
    for (unsigned K = 0; K < CEListLen; K++) {
      const CanonExpr *CE = VecSeq[J].CEList[K];
      MapCEToDistTy::iterator It = MapCEToDist.find(CE);

      // If this CE already has recorded CE, this means this CE
      // used multiple times to caculate RF.
      // Our rematerialization algorithm assumes
      // only one-time use of CE in caculating diff in the first II sequences.
      // See test "math-vectors.ll" for example.
      if (It != MapCEToDist.end() && It->second != VecCEListDiff[J][K]) {
        return false;
      }
      MapCEToDist[CE] = VecCEListDiff[J][K];
    }
  }

  return true;
}

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

    if (!preliminaryChecks(II, VecSeq)) {
      continue;
    }
    if (!isDistanceMatched(II, VecSeq)) {
      continue;
    }

    return std::make_pair(VecSize / II, II);
  }

  return std::make_pair(0, 0);
}

class HIRLoopRematerialize : public HLNodeVisitorBase {
public:
  typedef std::pair<HLContainerTy::iterator, HLContainerTy::iterator>
      NodeRangeTy;

public:
  explicit HIRLoopRematerialize(HIRFramework &F, HIRDDAnalysis &A)
      : HIRF(F), DDA(A){};

  bool run();

  /// Materialized a Loop in [Begin, End) of the given HLRegion.
  /// This function assumes the given HLRegion's is one basic block,
  /// i.e. a set of straight-line codes.
  bool tryRematerialize(HLRegion *Parent, HLContainerTy::iterator Begin,
                        HLContainerTy::iterator End);

  void visit(HLRegion *);
  void visit(HLLoop *){}; // TODO

  void visit(HLNode *) {}
  void postVisit(HLNode *) {}

private:
  // void updateCEs(HLLoop *Loop, const MapCEToDistTy &MapCEToDist);

  /// Rewrite a range to materialize a loop.
  /// It is assumed that range
  /// [Parent->child_begin(), Parent->child_end()) has three portions.
  ///  [(1) A sequence of untracked insts,
  ///   (2) Tracked Insts for rematerialze pattern matching,
  ///   (3) A sequence of untracked insts]
  /// In other words, any instruction in (2) is
  /// either rewritten into the a newly materialized loop body OR
  /// should be removed.  (1) and (3) should remain unchanged.
  /// Lengths of (1) and (3) can be zero.
  /// Some of the instructions in (2) can be reordered.
  /// TODO: Some parts of this function is equally applicable
  ///       when an inner loop is materialized into existing Loop.
  ///       In that case, first argument's type can be HLLoop*
  bool materializeALoop(HLRegion *Parent, const VecSeedInfoTy &VecSeedInfo,
                        const SetNodesTy &TrackedUpwardInsts,
                        unsigned RerollFactor, unsigned II,
                        const MapCEToDistTy &MapCEToDist);

  /// Write a loop of TC being RerollFactor
  /// Loop body becomes insts in NewLoopInsts.
  /// Remove surplus insts.
  /// Insert the new loop where the old insts were.
  HLLoop *replaceInstsWithLoop(unsigned RerollFactor,
                               HLContainerTy::iterator OrigBegin,
                               HLContainerTy::iterator OrigEnd,
                               VecNodesTy &NewLoopInsts,
                               const MapCEToDistTy &MapCEToDist);

  HIRFramework &HIRF;
  HIRDDAnalysis &DDA;
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void dumpMap(const MapCEToDistTy &MapCEToDist) {
  for (auto I : MapCEToDist) {
    (I.first)->dump(1);
    dbgs() << " = dist => ";
    dbgs() << I.second << "\n";
  }
}
#endif

/// Update CEs of newly materialized loop.
/// Update a CE with CE + dist*IV
/// CE is usually a constant.
/// Notice this update logic works since
/// we use the group of instructions corresponding to the
/// first iteration of a new loop.
/// E.G.
/// A[1] = B[1]
/// A[2] = B[2]
///
/// We use A[1] = B[1] as loop body.
/// The instruction is updated A[i+1] = B[i+1]
void updateCEs(HLLoop *Loop, const MapCEToDistTy &MapCEToDist) {

  LLVM_DEBUG(dumpMap(MapCEToDist));

  unsigned Level = Loop->getNestingLevel();
  ForEach<RegDDRef>::visitRange(
      Loop->child_begin(), Loop->child_end(),
      [MapCEToDist, Level](RegDDRef *Ref) {
        bool IsUpdated = false;
        for (CanonExpr *CE : make_range(Ref->canon_begin(), Ref->canon_end())) {
          MapCEToDistTy::const_iterator I = MapCEToDist.find(CE);
          if (I == MapCEToDist.end()) {
            continue;
          }

          assert(!CE->hasIV(Level));
          int64_t Dist = I->second;
          if (Dist == 0) {
            // Supposedly should be the same for the all instances
            // E.G. same invariant blob, same constant.
            continue;
          }
          CE->setIVCoeff(Level, InvalidBlobIndex, 0 - Dist);
          IsUpdated = true;
        }
        if (IsUpdated) {
          Ref->makeConsistent();
        }
      });
}

typedef SmallSet<unsigned, 8> SymbaseTy;

inline void markDefSymbase(const HLDDNode *Inst, SymbaseTy &DefSymbase) {
  if (Inst->hasLval() && Inst->getLvalDDRef()->isTerminalRef()) {
    DefSymbase.insert(Inst->getLvalDDRef()->getSymbase());
  }
}

template <class BidirIt>
void fillDefSymbase(BidirIt Begin, BidirIt End, SymbaseTy &DefSymbase) {

  for (auto It = Begin, EIt = End; It != EIt; ++It) {
    const HLDDNode *Inst = cast<HLDDNode>(*It);
    markDefSymbase(Inst, DefSymbase);
  }
}

template <>
void fillDefSymbase<HLContainerTy::iterator>(HLContainerTy::iterator Begin,
                                             HLContainerTy::iterator End,
                                             SymbaseTy &DefSymbase) {

  for (auto It = Begin, EIt = End; It != EIt; ++It) {
    const HLDDNode *Inst = cast<HLDDNode>(It);
    markDefSymbase(Inst, DefSymbase);
  }
}

template <>
void fillDefSymbase<HLContainerTy::const_iterator>(
    HLContainerTy::const_iterator Begin, HLContainerTy::const_iterator End,
    SymbaseTy &DefSymbase) {

  for (auto It = Begin, EIt = End; It != EIt; ++It) {
    const HLDDNode *Inst = cast<HLDDNode>(It);
    markDefSymbase(Inst, DefSymbase);
  }
}

/// We bail-out in case loop-live-out information
/// may be set conservatively.
/// New DO Loop
///    t1 = ...
/// END DO Loop
/// ...(1)
/// t1 = ...
///
/// t1 might be or might be not new loop's live-out
/// depending on t1 is used or not in the region marked with (1).
/// We don't currently track this fine-grain liveness info.
///
bool willHaveReDefInPostLoop(const VecNodesTy &NewLoopInsts,
                             HLContainerTy::iterator PostLoopBegin,
                             HLContainerTy::iterator PostLoopEnd) {

  if (PostLoopBegin == PostLoopEnd) {
    return false;
  }

  SymbaseTy DefSymbase;
  fillDefSymbase(NewLoopInsts.begin(), NewLoopInsts.end(), DefSymbase);

  // Allow non-straight line ranges.
  bool HasSeen = false;
  ForEach<HLInst>::visitRange(
      PostLoopBegin, PostLoopEnd, [&DefSymbase, &HasSeen](HLInst *Inst) {
        if (HasSeen || !Inst->hasLval() ||
            !Inst->getLvalDDRef()->isTerminalRef()) {
          return;
        }
        if (DefSymbase.count(Inst->getLvalDDRef()->getSymbase())) {
          HasSeen = true;
        }
      });

  return HasSeen;
}

/// Is a def that has a use is removed by rematerialization?
/// If so return true.
bool hasUsefulDefToBeRemoved(HLContainerTy::iterator OrigBegin,
                             HLContainerTy::iterator OrigEnd,
                             HLContainerTy::iterator PostLoopBegin,
                             HLContainerTy::iterator PostLoopEnd,
                             const HLNode *Parent) {

  SymbaseTy DefSymbase;
  fillDefSymbase(OrigBegin, OrigEnd, DefSymbase);

  // Region's LiveOut shouldn't be removed.
  if (const HLRegion *PRegion = dyn_cast<HLRegion>(Parent)) {
    for (auto It = PRegion->live_out_begin(), Eit = PRegion->live_out_end();
         It != Eit; ++It) {

      unsigned Symbase = It->first;
      if (DefSymbase.count(Symbase)) {
        return true;
      }
    }
  }

  bool HasSeen = false;
  ForEach<HLInst>::visitRange(
      PostLoopBegin, PostLoopEnd, [&DefSymbase, &HasSeen](const HLInst *Inst) {
        if (HasSeen) {
          return;
        }

        const DDRef *LvalRef = Inst->getLvalDDRef();
        for (const DDRef *Ref :
             make_range(Inst->all_dd_begin(), Inst->all_dd_end())) {
          if (!Ref->isTerminalRef()) {
            continue;
          }

          if (Ref == LvalRef) {
            // Skip defined temp e.g. %t = ...
            // This temp is not used symbase.
            // On the contrary, %a[i] = 's %a is used symbase.
            continue;
          }

          if (DefSymbase.count(Ref->getSymbase())) {
            HasSeen = true;
            break;
          }
        }
      });

  return HasSeen;
}

/// If UseRef is found in DefSymbase,
/// add UseRef to NewLoops LiveIn/Out temp list.
template <bool IsLiveIn = true>
void AddLiveTemp(const DDRef *UseRef, SymbaseTy &DefSymbase, HLLoop *NewLoop) {

  if (!UseRef->isTerminalRef()) {
    return;
  }

  if (DefSymbase.count(UseRef->getSymbase())) {
    if (IsLiveIn) {
      NewLoop->addLiveInTemp(UseRef->getSymbase());
    } else {
      NewLoop->addLiveOutTemp(UseRef->getSymbase());
    }
  }
}

/// This routine assumes a temp is not redefined within
/// the newly materialized loop.
void updateLoopLiveIn(HLLoop *NewLoop, const HLNode *Parent,
                      HLContainerTy::const_iterator PreLoopBegin,
                      HLContainerTy::const_iterator PreLoopEnd) {
  SymbaseTy DefSymbase;
  if (isa<HLRegion>(Parent)) {
    const HLRegion *PRegion = cast<HLRegion>(Parent);
    for (auto It = PRegion->live_in_begin(), EIt = PRegion->live_in_end();
         It != EIt; It++) {
      unsigned Symbase = It->first;
      DefSymbase.insert(Symbase);
    }
  }

  // If Parent were a HLLoop the following would be conservative
  // as [PreLoopBegin, PreLoopEnd) can contain any control-flows.
  // There could be output dependencies between defs in the range,
  // and sink can be postdominate the src. However, fillDefSymbase
  // simply scans all the defs in the range without any such consideration.
  fillDefSymbase(PreLoopBegin, PreLoopEnd, DefSymbase);

  ForEach<HLInst>::visitRange(
      NewLoop->child_begin(), NewLoop->child_end(),
      [&DefSymbase, NewLoop](HLInst *Inst) {
        // Scan all "used" symbase
        // Go throug Rval ddref and Rval blob ddref
        // Also scan blob ddrefs of Lval if any.
        for (const DDRef *UseRef :
             make_range(Inst->all_dd_begin(), Inst->all_dd_end())) {
          AddLiveTemp(UseRef, DefSymbase, NewLoop);
        }
      });
}

void updateLoopLiveOut(HLLoop *NewLoop, const HLNode *Parent,
                       HLContainerTy::const_iterator PostLoopBegin,
                       HLContainerTy::const_iterator PostLoopEnd) {
  SymbaseTy DefSymbase;

  fillDefSymbase(NewLoop->child_begin(), NewLoop->child_end(), DefSymbase);

  // Region's live-out needs to be loop live out
  // if the def folds into the loop.
  if (isa<HLRegion>(Parent)) {
    const HLRegion *PRegion = cast<HLRegion>(Parent);
    for (auto It = PRegion->live_out_begin(), Eit = PRegion->live_out_end();
         It != Eit; ++It) {

      unsigned Symbase = It->first;

      if (DefSymbase.count(Symbase)) {
        NewLoop->addLiveOutTemp(Symbase);
      }
    }
  }

  ForEach<const HLInst>::visitRange(
      PostLoopBegin, PostLoopEnd, [&DefSymbase, NewLoop](const HLInst *Inst) {
        // Scan all "used" symbase
        // Go throug Rval ddref and Rval blob ddref
        // Also scan blob ddrefs of Lval if any.
        for (const DDRef *UseRef :
             make_range(Inst->all_dd_begin(), Inst->all_dd_end())) {
          AddLiveTemp<false>(UseRef, DefSymbase, NewLoop);
        }
      });
}

// Make ddrefs in [Begin, end) consistent using given AuxRefs.
// If a ddref is a ref found in AuxRefs, make its def at level
// as the same as AuxRef's level. All refs contained in AuxRefs
// are non-linear.
// TODO: might be removed if generic makeConsistent() are
//       used in the current caller upateDefAtLevel.
void makeConsistent(HLDDNode::ddref_iterator Begin,
                    HLDDNode::ddref_iterator End,
                    const SmallVectorImpl<const RegDDRef *> &AuxRefs) {
  auto Update = [&AuxRefs](DDRef *Ref) {
    unsigned Symbase = Ref->getSymbase();
    for (auto Aux : AuxRefs) {
      if (Aux->getSymbase() == Symbase) {
        Ref->getSingleCanonExpr()->setNonLinear();
        return;
      }
    }
  };

  for (auto Ref : make_range(Begin, End)) {
    if (Ref->isSelfBlob()) {
      Update(Ref);
    } else {
      bool Updated = false;
      for (auto BRef : make_range(Ref->blob_begin(), Ref->blob_end())) {
        Update(BRef);
        Updated = true;
      }
      if (Updated && isa<RegDDRef>(Ref)) {
        // Its blob ddref is changed into NonLinear.
        cast<RegDDRef>(Ref)->updateDefLevel(NonLinearLevel);
      }
    }
  }
}

/// Update defined at level of DDRefs pushed into a loop.
/// e.g.
/// from
///        %mul = %a[0];  // %mul's def@level was 0
///        .. = %mul;
/// to
///     DO i = ..
///        %mul = %a[i]   // now %muls becomes non-linear
///        .. = %mul
///        ...
///     END DO
/// This routine mark lval temps to non-linear first and
/// its uses as non-linear accordingly.
/// TODO: Replace custom makeConsistent with common makeConsistent
/// API with stablity. Currently, directly replacing custom makeConsistent
/// incurs stability issue (e.g. verification failure at nullC
///  alias_20@opt_speed)
///
void updateDefAtLevel(HLLoop *NewLoop) {

  SmallVector<const RegDDRef *, 4> AuxRefs;

  for (HLNode &Node :
       make_range(NewLoop->child_begin(), NewLoop->child_end())) {
    HLDDNode *DDNode = cast<HLDDNode>(&Node);

    if (DDNode->hasLval() && DDNode->getLvalDDRef()->isTerminalRef()) {

      DDNode->getLvalDDRef()->getSingleCanonExpr()->setNonLinear();
      AuxRefs.push_back(DDNode->getLvalDDRef());
      makeConsistent(DDNode->rval_op_ddref_begin(), DDNode->rval_op_ddref_end(),
                     AuxRefs);
    } else {
      makeConsistent(DDNode->ddref_begin(), DDNode->ddref_end(), AuxRefs);
    }
  }
}

/// Check if there is a non-tracked inst in-between tracked insts.
/// If so, bail-out.
bool untrackedInBetweenInsts(HLContainerTy::const_iterator NewLoopBodyBegin,
                             HLContainerTy::const_iterator NewLoopBodyEnd,
                             const SetNodesTy &TrackedUpwardInsts) {
  for (auto &Node : make_range(NewLoopBodyBegin, NewLoopBodyEnd)) {
    if (!TrackedUpwardInsts.count(&Node)) {
      // Untracked Inst is in the range.
      return true;
    }
  }

  return false;
}

/// See if rematerialization-inhibiting DD edges exist.
/// If any, return false.
/// TODO: Will be removed if common parts of all iterations are
///       kept outside the new loop.
bool dependencyCheck(DDGraph G, const VecSeedInfoTy &VecSeedInfo, unsigned II) {
  // See if tracked instructions from the first II seeds,
  // have an outgoing ANTIdep DDEdge to any other instructions tracked from
  // [II + 1, End).
  // If so, by rematerializing a loop, where first iteration has
  // tracked intructions of first II seeds,
  // and the second iteration has tracked instructions
  // of the next II seeds, incorrect WAR (i.e. not exisiting in the original
  // code) dedpendencies are added.

  auto IsInLaterIterations = [VecSeedInfo, II](const HLNode *Node) {
    for (auto &Info :
         make_range(std::next(VecSeedInfo.begin(), II), VecSeedInfo.end())) {
      for (auto &NodeInLater : Info.TrackedUpwardInsts) {
        if (NodeInLater == Node) {
          return true;
        }
      }
    }

    return false;
  };

  LLVM_DEBUG(dbgs() << "See Edges\n");
  for (auto &Info :
       make_range(VecSeedInfo.begin(), std::next(VecSeedInfo.begin(), II))) {
    for (auto Node : Info.TrackedUpwardInsts) {
      const DDRef *Lval = cast<HLDDNode>(Node)->getLvalDDRef();
      for (const DDRef *Ref : make_range(cast<HLDDNode>(Node)->all_dd_begin(),
                                         cast<HLDDNode>(Node)->all_dd_end())) {
        if (Ref == Lval) {
          continue;
        }

        for (auto Edge : G.outgoing(Ref)) {
          if (!Edge->isAnti()) {
            continue;
          }

          LLVM_DEBUG(Edge->dump());

          const HLNode *SinkNode = Edge->getSink()->getHLDDNode();
          // See if this SinkNode is in a later iteration
          // than the first iteration.
          if (IsInLaterIterations(SinkNode)) {
            LLVM_DEBUG(dbgs() << "Rematerialization Inhibiting edge:\n");
            LLVM_DEBUG(Edge->dump());
            return false;
          }
        }
      }
    }
  }
  return true;
}

/// From the range [Begin, End), remove all insts
/// that are NOT found in NewInsts.
/// NewInsts are only the insts to be used for the newly
/// rematerialized loop after updating.
void removeUnusedInsts(HLContainerTy::iterator Begin,
                       HLContainerTy::iterator End,
                       const VecNodesTy &NewInsts) {
  VecNodesTy ToBeRemoved;
  // Zip-through two vectors of insts.
  // Dest are to be used in the newly rematerialized loop body.
  // Src range is the original range.
  // Remove from Src to be the same as Dest.
  // # nodes in Dest <= # nodes in src, by the definition of rematerialization.
  HLContainerTy::iterator ISrc = Begin;
  HLContainerTy::iterator ISrcEnd = End;
  VecNodesTy::const_iterator IDest, IDestEnd;
  for (IDest = NewInsts.begin(), IDestEnd = NewInsts.end();
       IDest != IDestEnd && ISrc != ISrcEnd; ++ISrc) {
    HLNode *Src = &(*ISrc);
    const HLNode *Dest = *IDest;
    if (Dest == Src) {
      ++IDest;
      continue;
    }

    ToBeRemoved.push_back(Src);
  }

  assert(IDest == IDestEnd);

  // Remove remaining from [Begin, end)
  for (HLContainerTy::iterator It = ISrc; It != ISrcEnd; ++It) {
    ToBeRemoved.push_back(&(*It));
  }

  for (HLNode *Node : ToBeRemoved) {
    HLNodeUtils::remove(Node);
  }
}

/// This function collects tracked instructions for first II seeds.
/// Using first instructions eases update of CEs.
/// TODO: use tracked instructions for last II seeds.
///       Then definitions inside the loop will have the value that
///       can be used outside the loop.
void collectInstsForNewLoopBody(const VecSeedInfoTy &VecSeedInfo, unsigned II,
                                VecNodesTy &NewLoopInsts) {
  // Use II seeds' tracked insts.
  // These NewLoopInsts will be the body of a newly materialized loop.
  for (auto Info :
       make_range(VecSeedInfo.begin(), std::next(VecSeedInfo.begin(), II))) {
    NewLoopInsts.insert(NewLoopInsts.end(), Info.TrackedUpwardInsts.begin(),
                        Info.TrackedUpwardInsts.end());
  }
}

HLLoop *HIRLoopRematerialize::replaceInstsWithLoop(
    unsigned RerollFactor, HLContainerTy::iterator OrigBegin,
    HLContainerTy::iterator OrigEnd, VecNodesTy &NewLoopInsts,
    const MapCEToDistTy &MapCEToDist) {

  removeUnusedInsts(OrigBegin, OrigEnd, NewLoopInsts);

  // Our RF is unsigned type, which always fits in 32-bit.
  // Unsigned information will be realized by NSW flag, which is default.
  Type *Ty = Type::getInt32Ty(HIRF.getContext());

  DDRefUtils &DDRU = HIRF.getDDRefUtils();
  HLLoop *NewLoop = HIRF.getHLNodeUtils().createHLLoop(
      nullptr, DDRU.createConstDDRef(Ty, 0),
      DDRU.createConstDDRef(Ty, RerollFactor - 1),
      DDRU.createConstDDRef(Ty, 1));
  HLNodeUtils::insertBefore(NewLoopInsts.front(), NewLoop);
  HLNodeUtils::moveAsLastChildren(
      NewLoop, (NewLoopInsts.front())->getIterator(),
      std::next((NewLoopInsts.back())->getIterator()));

  NewLoop->setBranchDebugLoc(NewLoop->getFirstChild()->getDebugLoc());

  updateCEs(NewLoop, MapCEToDist);

  return NewLoop;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void printRegionDetail(const HLRegion *Parent) {
  if (!Parent) {
    return;
  }
  formatted_raw_ostream FOS(dbgs());
  Parent->print(FOS, 0, true, true);
}
#endif

bool HIRLoopRematerialize::materializeALoop(
    HLRegion *Parent, const VecSeedInfoTy &VecSeedInfo,
    const SetNodesTy &TrackedUpwardInsts, unsigned RerollFactor, unsigned II,
    const MapCEToDistTy &MapCEToDist) {

  // Dependency check
  // TODO: too conservative. Remove when loop invariant codes are hoisted.
  DDGraph G = DDA.getGraph(Parent);
  if (!dependencyCheck(G, VecSeedInfo, II)) {
    return false;
  }

  // Gather the potential loop body
  VecNodesTy NewLoopInsts;
  collectInstsForNewLoopBody(VecSeedInfo, II, NewLoopInsts);

  // Calc potential PostLoop within Parent Region
  // Need to update LiveOut info of a new Loop.
  HLContainerTy::iterator PostLoopBegin =
      std::next((VecSeedInfo.back().ContainingInst)->getIterator());
  HLContainerTy::iterator PostLoopEnd = Parent->child_end();

  if (willHaveReDefInPostLoop(NewLoopInsts, PostLoopBegin, PostLoopEnd)) {
    return false;
  }

  HLNodeUtils::sortInTopOrderAndUniq(NewLoopInsts);

  HLNode *FirstNodeOfFirstLoopIter = NewLoopInsts.front();
  HLNode *LastNodeOfLastLoopIter = VecSeedInfo.back().ContainingInst;
  if (untrackedInBetweenInsts(FirstNodeOfFirstLoopIter->getIterator(),
                              std::next(LastNodeOfLastLoopIter->getIterator()),
                              TrackedUpwardInsts)) {
    return false;
  }

  if (hasUsefulDefToBeRemoved(FirstNodeOfFirstLoopIter->getIterator(),
                              std::next(LastNodeOfLastLoopIter->getIterator()),
                              PostLoopBegin, PostLoopEnd, Parent)) {

    return false;
  }

  // Finally, materialize a loop around NewLoopInsts after removing
  // insts in ToBeRemovedInsts
  HLLoop *NewLoop = replaceInstsWithLoop(
      RerollFactor, FirstNodeOfFirstLoopIter->getIterator(),
      std::next(LastNodeOfLastLoopIter->getIterator()), NewLoopInsts,
      MapCEToDist);

  // Update Loop LiveIn/Out info
  HLContainerTy::iterator PreLoopBegin = Parent->child_begin();
  HLContainerTy::const_iterator PreLoopEnd = NewLoop->getIterator();
  updateLoopLiveIn(NewLoop, Parent, PreLoopBegin, PreLoopEnd);
  updateLoopLiveOut(NewLoop, Parent, PostLoopBegin, PostLoopEnd);
  updateDefAtLevel(NewLoop);

  LLVM_DEBUG(dbgs() << "After all: \n");
  LLVM_DEBUG(NewLoop->getParent()->dump());
  LLVM_DEBUG(printRegionDetail(NewLoop->getParentRegion()));

  return true;
}

/// Regards HIR Region Live ins as invariant throughout region.
/// This is true for current loop-materialization-candidate-regions.
/// For general rematerializaiton materializng a inner loop in a loop body
/// all the live-in to the rematerialization "range" needs to be scanned.
/// Basically reaching definitions to the begining of the range,
/// and probably set conservatively in our HIR environment.
/// We alreayd came out of SSA representation.
void collectInvariantBlobs(const HLRegion *Region,
                           InvariantBlobTy &InvariantBlobs) {

  const BlobUtils &BU = Region->getBlobUtils();

  for (auto It = Region->live_in_begin(), EIt = Region->live_in_end();
       It != EIt; It++) {
    unsigned Symbase = It->first;
    unsigned Index = BU.findTempBlobIndex(Symbase);
    InvariantBlobs[BU.getBlob(Index)] = Index;
  }
}

bool HIRLoopRematerialize::tryRematerialize(HLRegion *Parent,
                                            HLContainerTy::iterator Begin,
                                            HLContainerTy::iterator End) {
  if (Begin == End) {
    // Empty range
    return false;
  }

  VecCEOpSeqTy VecSeq;
  VecSeedInfoTy VecSeedInfo; // Separately maintained for rewriting
  SetNodesTy TrackedUpwardInsts;
  if (!buildSequences<HLRegion>(Parent, Begin, End, DDA, VecSeq, VecSeedInfo,
                                TrackedUpwardInsts)) {
    return false;
  }

  MapCEToDistTy MapCEToDist;
  InvariantBlobTy InvariantBlobs;
  collectInvariantBlobs(Parent, InvariantBlobs);

  unsigned RerollFactor, II;
  std::tie(RerollFactor, II) =
      SequenceChecker(InvariantBlobs, MapCEToDist).calcRerollFactor(VecSeq);

  if (RerollFactor < std::max<unsigned>(2, LoopRematerializeTCLowerBound)) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "Reroll Factor: " << RerollFactor << ", "
                    << "II : " << II << "\n");

  if (!materializeALoop(Parent, VecSeedInfo, TrackedUpwardInsts, RerollFactor,
                        II, MapCEToDist)) {
    return false;
  }

  return true;
}

void HIRLoopRematerialize::visit(HLRegion *HRegion) {
  if (!HRegion->isLoopMaterializationCandidate()) {
    return;
  }

  // Currently, we only care marked regions.
  // Marked regions are guaranteed to have straight lines of code.
  // So just inspect the whole range [child_begin(), child_end()).
  if (tryRematerialize(HRegion, HRegion->child_begin(), HRegion->child_end())) {
    LoopsRematerialized++;
  }
}

bool HIRLoopRematerialize::run() {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << OPT_SWITCH << " is disabled.\n");
    return false;
  }

  // Scan HLRegions
  unsigned PreStat = LoopsRematerialized;
  for (HLNode &Node : make_range(HIRF.hir_begin(), HIRF.hir_end())) {
    HLNodeUtils::visit(*this, &Node);
  }

  return (LoopsRematerialized - PreStat);
}

} // namespace

char HIRLoopRematerializeLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopRematerializeLegacyPass, OPT_SWITCH, OPT_DESC,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRLoopRematerializeLegacyPass, OPT_SWITCH, OPT_DESC, false,
                    false)

FunctionPass *llvm::createHIRLoopRematerializePass() {
  return new HIRLoopRematerializeLegacyPass();
}

bool HIRLoopRematerializeLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    return false;
  }

  LLVM_DEBUG(dbgs() << OPT_DESC " for Function : " << F.getName() << "\n");

  return HIRLoopRematerialize(getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
                              getAnalysis<HIRDDAnalysisWrapperPass>().getDDA())
      .run();
}

PreservedAnalyses
HIRLoopRematerializePass::run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &AM) {
  if (DisablePass) {
    return PreservedAnalyses::all();
  }

  LLVM_DEBUG(dbgs() << OPT_DESC " for Function : " << F.getName() << "\n");

  HIRLoopRematerialize(AM.getResult<HIRFrameworkAnalysis>(F),
                       AM.getResult<HIRDDAnalysisPass>(F))
      .run();

  return PreservedAnalyses::all();
}
