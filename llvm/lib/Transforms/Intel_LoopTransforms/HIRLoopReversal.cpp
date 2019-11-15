// ===- HIRLoopReversal.cpp - Implement HIR Loop Reversal Transformation -===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===---------------------------------------------------------------------===//
// HIR Loop Reversal Example
//
// [ORIGINAL]                       [AFTER REVERSAL]
//
// for(i=0; i<=4; ++i){             for(i=0; i<=4; ++i){
//   A[100-i] = 0;                    A[96+i] = 0;
// }                                }
//
//===---------------------------------------------------------------------===//
//
// This file implements HIR Loop Reversal Transformation.
//
// Available options:
// -hir-loop-reversal:          Perform HIR Loop Reversal
// -disable-hir-loop-reversal:  Flag to Disable/Bypass HIR Loop Reversal
//
//
// [Analysis]
// Preliminary Checks on Loop:
// - no multiple exits;
// - loop is normalized;
// - no IV in loop's UpperBound;
// - Neither firstChild nor lastChild can be a nullptr;
// - List of Nonos (within the loop's body):
//     -no call,
//     -no goto,
//     -no jump,
//     -no label;
//
// 1.Collection:
//   Collect on the given loop and check if there is any suitable CE/DDRef
//   candidates to reverse.
//
// 2.Legality:
//   Check if the loop is legal after reversal.
//
// 3.Profitability:
//   Check if the loop is profitable for reversal.
//
// [Transformation]
// See the example given above.
//
// 4. Calling Utility API to reverse a loop from any transformation.
// [Code Sample]
//  bool LoopIsReversible = HIRLoopTransformUtils::isHIRLoopReverible(
//      Lp,   // INPUT + OUTPUT: a given loop
//      HDDA, // INPUT: HIR DDAnalysis
//      HSRA, // INPUT: HIRSafeReductionAnalysis
//      HLS,  // INPUT: Existing HIRLoopStatitics
//      true  // INPUT: Control Profit Tests using Reverser's profit model
//      );
//
// if (LoopIsReversible) {
//   LLVM_DEBUG(dbgs() << "Loop is Reversible\n");
// } else {
//   LLVM_DEBUG(dbgs() << "Loop is Not Reversible\n");
// }
//
// //Revere the loop if it is reversible
// if (LoopIsReversible) {
//   HIRLoopTransformUtils::doHIRLoopReversal(
//          Lp,   // INPUT + OUTPUT: a given loop
//          HDDA, // INPUT: HIR DDAnalysis
//          HSRA, // INPUT: HIRSafeReductionAnalysis
//          HLS   // INPUT: Existing HIRLoopStatitics
//          );
// }
//
//
// TODO
// 1. Support #pragma REVERSE_LOOP
// 2. Allow simple forward gotos in the same iteration
// 3.
//
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopReversal.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "HIRLoopReversalImpl.h"

#define DEBUG_TYPE "hir-loop-reversal"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::reversal;

static cl::opt<bool> DisableHIRLoopReversal(
    "disable-hir-loop-reversal", cl::init(false), cl::Hidden,
    cl::desc("Disable HIR Loop Reversal Transformation"));

static cl::opt<bool> AssumeProfitability(
    "hir-loop-reversal-assume-profitability", cl::init(false), cl::Hidden,
    cl::desc("Assumes profitability of reversal so only legality is checked"));

STATISTIC(HIRLoopReversalTriggered, "Number of HIR Loop(s) Reversed");

const unsigned DefaultReadWeight = 1;         // default Weight for a Read is 1
const unsigned DefaultWriteWeight = 2;        // default Weight for a Write is 2
const unsigned DefaultShortTripThreshold = 4; // default short trip threshold

///\brief Collect all suitable MarkedCanonExprs on MemRef type of DDRef
struct HIRLoopReversal::MarkedCECollector final : public HLNodeVisitorBase {
  SmallVectorImpl<MarkedCanonExpr> &CEVAP;
  const HLLoop *Lp;
  unsigned LoopLevel;
  HIRLoopReversal *HLR;
  bool AbortCollector;
  bool CheckProfitability;
  bool HasNegIVExpr;

public:
  explicit MarkedCECollector(SmallVectorImpl<MarkedCanonExpr> &InitMCEV,
                             HLLoop *InitLp, unsigned InitLevel,
                             bool CheckProfitability, HIRLoopReversal *HLR)
      : CEVAP(InitMCEV), Lp(InitLp), LoopLevel(InitLevel), HLR(HLR),
        AbortCollector(false), CheckProfitability(CheckProfitability),
        HasNegIVExpr(!CheckProfitability) {
    assert((LoopLevel <= MaxLoopNestLevel) && "LoopLevel is out of bound\n");
    assert(Lp && HLR && "none of Lp and HLR can be null\n");
  }

  bool getCollectionAborted(void) const { return AbortCollector; }

  void visit(HLDDNode *Node) {
    for (auto I = Node->op_ddref_begin(), E = Node->op_ddref_end(); I != E;
         ++I) {
      checkAndCollectMCE(*I, Node);
    }
  }

  // No processing needed for Goto, Label and HLNode types;
  void visit(HLGoto *Goto){};
  void visit(HLLabel *Label){};
  void visit(HLNode *Node) {
    llvm_unreachable(
        "visit(HLNode *) - Node not supported for HIR Loop Reversal.\n");
  }
  void postVisit(HLNode *Node) {}

  bool isDone() const { return AbortCollector; }

  bool hasNegIVExpr(void) const { return HasNegIVExpr; }

  // Since the BaseCE CAN'T have any IV (with matching loop level), we
  // don't bother to collect from the base.
  // We only collect the CE from the Subs section if it hasIV(LoopLevel).
  //
  void checkAndCollectMCE(RegDDRef *RegDD, HLDDNode *ImmedParent);
};

// Check each RegDDRef* and collect if suitable.
// During collection, we analyze the CE and
// - identify if it has a negative IV Expr
// - compute its CalculatedWeight
//     CE           CalculatedWeight
//     C * IV:      C
//     C * B * IV:  C * B (min or max value)
//
void HIRLoopReversal::MarkedCECollector::checkAndCollectMCE(
    RegDDRef *RegDD, HLDDNode *ImmedParent) {
  unsigned Dimension = 1;

#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  for (auto I = RegDD->canon_begin(), E = RegDD->canon_end(); I != E;
       ++I, ++Dimension) {
    bool HasIV = false;
    CanonExpr *CE = (*I);
    assert(CE && "checkAndCollectMCE(.) -- CanonExpr* can't be null\n");

    // See the CE we are checking
    // LLVM_DEBUG(FOS << "Checking: "; CE->print(FOS); FOS << "\n";);

    // Check if the current CE has an IV on the matching loop level
    if (CE->hasIV(LoopLevel)) {
      HasIV = true;
    }

    // Skip the rest for the current iteration:
    // if a CE has no IV, it can't be collected!
    if (!HasIV) {
      continue;
    }

    if (!CheckProfitability) {
      // Just collect the CE containing IV if we don't care about profitability.
      CEVAP.push_back(MarkedCanonExpr(CE, 0, RegDD, 0));
      continue;
    }

    // Check if the current CE has a ValidBlobIndex.
    // If so, check if we can get its CalculatedWeight.
    // E.g. C + IVConst * IVBlob * iv:
    // A CalculatedWeight = IVConst * IVBlobMinMaxVal
    //
    int64_t IVConstCoeff = 0, BlobVal = 1, CalculatedWeight = 0;
    unsigned IVIndex = 0;
    CE->getIVCoeff(LoopLevel, &IVIndex, &IVConstCoeff);

    // IV has NO blob, CalculatedWeight = IVConst
    if (IVIndex == InvalidBlobIndex) {
      CalculatedWeight = IVConstCoeff;
    }
    // If IV has a valid blob, decide IV-Blob's sign
    else if (HLNodeUtils::isKnownPositiveOrNegative(IVIndex, ImmedParent,
                                                    BlobVal)) {
      CalculatedWeight = IVConstCoeff * BlobVal;
    }
    // Error: IVExpr without known sign, abort collection!
    else {
      AbortCollector = true;
      return;
    }

    assert(CalculatedWeight && "CalculatedWeight can't be 0\n");

    // Decide HasNegIVExpr flag
    if (CalculatedWeight < 0) {
      HasNegIVExpr = true;
    }

    // If control can reach here, it is good to collect.

    // Examine the CE we are collecting
    // LLVM_DEBUG(FOS << "Collect: "; CE->print(FOS); FOS << "\n";);

    uint64_t Stride = 1;     // Stride defaults to 1
    if (RegDD->isMemRef()) { // Non-uniform Stride is only available on MemRef
                             // DDRef
      Stride = RegDD->getDimensionConstStride(Dimension);
    }

    // If the dimension stride is variable, Stride will be 0
    // TODO: Handle negative strides
    if (Stride == 0) {
      AbortCollector = true;
      return;
    }

    // Collect the MCE
    CEVAP.push_back(MarkedCanonExpr(CE, Stride, RegDD, CalculatedWeight));
  }
}

struct HIRLoopReversal::AnalyzeDDInfo final : public HLNodeVisitorBase {
  DDGraph &DDG;
  const HLLoop *Lp;
  HIRLoopReversal &HLR;
  bool AbortCollector;
  unsigned LoopLevel;
  SmallSet<unsigned, 4> LvalSBSet; // Lval Symbase set from SafeReduction Chain

  explicit AnalyzeDDInfo(DDGraph &DDG, const HLLoop *Lp, HIRLoopReversal &HLR,
                         unsigned LoopLevel)
      : DDG(DDG), Lp(Lp), HLR(HLR), AbortCollector(false),
        LoopLevel(LoopLevel) {
    HLR.HSRA.computeSafeReductionChains(Lp);
    collectLvalSymbase(Lp);
  }

  void visit(const HLNode *Node) {}

  bool isDone() const { return AbortCollector; }

  // Any premature abort?
  bool getCollectionAborted(void) const { return AbortCollector; }

  void visit(const HLDDNode *DDNode);

  void postVisit(const HLNode *Node) {}

  // collect all Lval's symbase(s) from all inst(s) on Lp's SafeReduction Chain
  void collectLvalSymbase(const HLLoop *Lp);

  bool findSymbase(unsigned Item) const { return LvalSBSet.count(Item); }
};

void HIRLoopReversal::AnalyzeDDInfo::collectLvalSymbase(const HLLoop *Lp) {
  const SafeRedInfoList &SRCL = HLR.HSRA.getSafeRedInfoList(Lp);

  // Walk SafeReductionChain, collect each Inst's Lval Symbase into LvalSBSet
  for (auto &SafeRedInfo : SRCL) {
    for (auto &Inst : SafeRedInfo.Chain) {
      LvalSBSet.insert(Inst->getLvalDDRef()->getSymbase());
    }
  }
}

// Do Legal Test on HLInst*:
//
// If Inst is NOT a SafeReduction:
//   if (Lval is TempLiveOut){
//     return false;
//   }
//
// - Ignore any operand whose Symbase is in LvalSBSet
// - Do legal test on any remaining operand
//
void HIRLoopReversal::AnalyzeDDInfo::visit(const HLDDNode *DDNode) {
  bool IsSafeReduction = false;

  // Examine the current HLInst
  // LLVM_DEBUG(DDNode->dump(););

  // Support for HLInst*
  if (const HLInst *Inst = dyn_cast<HLInst>(DDNode)) {
    if (!(IsSafeReduction = HLR.HSRA.isSafeReduction(Inst))) {
      // For any non-Safe-Reduction HLInst:
      // do LiveOut Temp test only if Lval exists
      const RegDDRef *LRef = Inst->getLvalDDRef();
      if (LRef && Lp->isLiveOut(LRef->getSymbase())) {
        AbortCollector = true;
        return;
      }
    }
  }

  // Iterate over each DDRef
  for (auto It = DDNode->ddref_begin(), ItE = DDNode->ddref_end(); It != ItE;
       ++It) {

    // Selectively skip operand(s) from a SafeReduction Instruction
    const RegDDRef *Ref = (*It);
    if (IsSafeReduction && findSymbase(Ref->getSymbase())) {
      continue;
    }

    // Iterate over each outgoing edge:
    for (auto II = DDG.outgoing_edges_begin(*It),
              EE = DDG.outgoing_edges_end(*It);
         II != EE; ++II) {

      // Examine the DDEdge:
      const DDEdge *Edge = (*II);
      // LLVM_DEBUG(Edge->print(dbgs()););

      // Check Current DV is legal to reverse
      const DirectionVector &DV = Edge->getDV();
      // LLVM_DEBUG(DV.print(dbgs(), true));

      // Abort Collection if invalid!
      if (!HIRLoopReversal::isLegal(DV, LoopLevel)) {
        AbortCollector = true;
        return;
      }
    }
    // end loop: out-going edge
  }
}

bool HIRLoopReversal::run() {
  if (DisableHIRLoopReversal) {
    LLVM_DEBUG(
        dbgs() << "HIR Loop Reversal Transformation Disabled or Skipped\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIR LoopReversal on Function : "
                    << HIRF.getFunction().getName() << "\n");

  // Gather ALL Innermost Loops as Candidates, use 64 increment
  SmallVector<HLLoop *, 64> CandidateLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(CandidateLoops);
  // LLVM_DEBUG(dbgs() << " # Innermost Loops: " << CandidateLoops.size()
  // <<"\n");
  if (CandidateLoops.empty()) {
    return false;
  }

  // TODO:
  // Re-Build DDA on demand if needed

  LoopOptReportBuilder &LORBuilder = HIRF.getLORBuilder();

  // Iterate Over Each Candidate Loop
  for (auto &Lp : CandidateLoops) {

    // Check the loop's suitability for reversal
    bool SuitableLoop =
        isReversible(Lp,
                     true, // always do profit test when running as a pass
                     true, // always do legal test when running as a pass
                     false // don't skip loop bound checks
        );

    // *** Do HIR Loop Reversal Transformation if suitable ***
    // Skip the loop if it is not suitable
    if (!SuitableLoop) {
      continue;
    }

    // Reverse the loop
    bool LoopIsReversed = doHIRReversalTransform(Lp);
    LORBuilder(*Lp).addRemark(OptReportVerbosity::Low, "Loop was reversed");

    // Update Loops-Reversal-Triggered Counter
    if (LoopIsReversed) {
      HIRLoopReversalTriggered++;
    }
  }
  // end: for loop on I

  CandidateLoops.clear();
  return false;
}

/// \brief Do Quick Preliminary Checks on the given loop and decide whether
/// there is a chance for successful reversal.
//
// The following preliminary conditions are currently checked:
// - Multiple exits;
// - Normalized Loop;
// - UBCE Related Checks:
//  . getDenominator is 1;
//  . can convert to StandaloneBlob;
//  . filter off any loop with very small trip count;
// - Neither firstChild nor lastChild can be null;
// - Run a statistics on Loop and check it contains no: goto/jmp/lablel;
// - Filter off any loop with very small trip count;
//
bool HIRLoopReversal::doLoopPreliminaryChecks(const HLLoop *Lp,
                                              bool CheckProfitability,
                                              bool SkipLoopBoundChecks) {
  // No multiple-exit loop or unknown loops
  if (!Lp->isDo()) {
    return false;
  }

  if (!SkipLoopBoundChecks) {
    // No non-normalized loop
    // (Expect the reversal candidates to be a normalized loop)
    if (!Lp->isNormalized()) {
      return false;
    }

    // Checks on UBCE and related
    const CanonExpr *UBCE = Lp->getUpperCanonExpr();

    // UBCE's Denominator must be 1
    if (UBCE->getDenominator() != 1) {
      return false;
    }

    // UBCE can be converted to StandAloneBlob
    if (!UBCE->canConvertToStandAloneBlob()) {
      return false;
    }
  }

  // Filter off any loop with constant trip count below the given threshold
  uint64_t TripCount = 0;
  if (CheckProfitability && Lp->isConstTripLoop(&TripCount)) {
    if (TripCount < DefaultShortTripThreshold) {
      return false;
    }
  }

  // Do a LoopStatistics Collection and test for
  // - No function call
  // - No label
  // - No goto
  const LoopStatistics &LS = HLS.getSelfLoopStatistics(Lp);

  // LLVM_DEBUG(LS.dump(););
  if (LS.hasCallsWithUnsafeSideEffects() || LS.hasForwardGotos()) {
    return false;
  }

  return true;
}

bool HIRLoopReversal::doCollection(HLLoop *Lp, bool CheckProfitability) {
  // Do a loop collection on MCEs
  MarkedCECollector MCEC(MCEAV, Lp, LoopLevel, CheckProfitability, this);
  Lp->getHLNodeUtils().visitRange(MCEC, Lp->getFirstChild(),
                                  Lp->getLastChild());

  // Check if Collection aborts prematurely
  bool MCECollectionAbortion = MCEC.getCollectionAborted();
  if (MCECollectionAbortion) {
    LLVM_DEBUG(dbgs() << "Reversal: Loop MCE collection failed\n";);
    return false;
  }

  return MCEC.hasNegIVExpr();
}

// Profitability Analysis for HIR Loop Reversal
//
// *** ALGORITHM SCRATCH ***
// For each collected MCE:
// - Accumulate Weights for PositiveIVs into PositiveWeight
// - Accumulate Weights for NegativeIVs into NegativeWeight
// - Decision: return (AccumulatedNegIVs > AccumulatedPosIVs)
//
bool HIRLoopReversal::isProfitable(const HLLoop *Lp) {

  // Accumulate weights over each Neg-IV CEs and Pos-IV CE
  unsigned AccumuWeightNegIVs = 0, AccumuWeightPosIVs = 0, Weight = 0;
  for (auto &MCE : MCEAV) {
    // Skip non-MemRef MCEs in profit analysis
    if (!MCE.isMemRef()) {
      continue;
    }

    // LLVM_DEBUG(MCE.dump());

    bool IsWrite = MCE.isWrite();
    uint64_t Stride = MCE.getStride();

    // Use a different weight, differentiate between a Write and Read
    IsWrite ? (Weight = DefaultWriteWeight) : (Weight = DefaultReadWeight);

    // Check Simulated Weight if IV-Expr's sign is known:
    int64_t CalculatedWeight = MCE.getCalculatedWeight();

    // Accumulate Weight on Positive or Negative IVs
    // Use scaled integer math to replace expensive floating-point math
    if (CalculatedWeight > 0) {
      AccumuWeightPosIVs += (1000 / (CalculatedWeight * Stride)) * Weight;
    } else {
      AccumuWeightNegIVs += (1000 / ((-CalculatedWeight) * Stride)) * Weight;
    }
  }
  // end_for: MCE

  // Examine the values of AccumuWeightNegIVs and AccumuWeightPosIVs:
  LLVM_DEBUG(dbgs() << "  AccumuWeightNegIVs :" << AccumuWeightNegIVs << "  "
                    << "  AccumuWeightPosIVs :" << AccumuWeightPosIVs << "  "
                    << "\n");

  // Decide: true if negative IVs have higher weight
  return (AccumuWeightNegIVs > AccumuWeightPosIVs);
}

/* ------------------------------------------------------------------- */
// Legality Tests for HIR Loop Reversal: is it legal to do Loop Reversal?
// Launch legal test for each DV in the loop.
/* ------------------------------------------------------------------- */
bool HIRLoopReversal::isLegal(const HLLoop *Lp) {
  DDGraph DDG = HDDA.getGraph(Lp);
  // LLVM_DEBUG(dbgs() << "Dump the Full DDGraph:\n"; DDG.dump(););

  AnalyzeDDInfo ADDI(DDG, Lp, *this, LoopLevel);
  Lp->getHLNodeUtils().visitRange(ADDI, Lp->child_begin(), Lp->child_end());

  // Check DDInfoAnalysis's early abortion
  bool CollectionAborted = ADDI.getCollectionAborted();
  if (CollectionAborted) {
    // Legality Test fails if there is at least 1 invalid DV!
    return false;
  }

  return true;
}

/* ------------------------------------------------------------------- */
// Legality Tests for HIR Loop Reversal: is it legal to do Loop Reversal?
//
// For each DV in the Candidate Loop:
// (1) legal if the DV on last level is (=);
// Otherwise;
//
// (2) Scan each level: left -> right
// if DV on current level is (<) or (>), Return true;
// if DV on current level is (=), continue scan;
//
/* ------------------------------------------------------------------- */
bool HIRLoopReversal::isLegal(const DirectionVector &DV, unsigned Level) {
  // LLVM_DEBUG(DV.print(dbgs(), true););

  // Case1: true if the DV at the given level is DVKind::EQ
  if (DV[Level - 1] == DVKind::EQ) {
    return true;
  }

  // Case2: true if the DV at any previous level is DVKind::LT or DVKind::GT
  if (DV.isIndepFromLevel(Level)) {
    return true;
  }

  return false;
}

/// \brief Conduct necessary HIR-Loop-Reversal Tests and decide whether the
/// given loop is suitable for reversal.
//
// Note:
// 1. Please do NOT change the order of the tests in isReversible().
//    The tests are arranged in the order of increasing expensiveness.
//
// 2. This function has been extended (with flags) to support both called by a
//    normal pass and called through utility APIs.
//
bool HIRLoopReversal::isReversible(HLLoop *Lp, bool DoProfitTest,
                                   bool DoLegalTest, bool SkipLoopBoundChecks) {
  // LLVM_DEBUG(dbgs() << "Current Loop: \n"; Lp->dump(););
  clearWorkingSetMemory();

  // Show The Current Loop
  // LLVM_DEBUG(Lp->dump(););
  LoopLevel = Lp->getNestingLevel();
  bool CheckProfitability = (DoProfitTest && !AssumeProfitability);

  if (DoLegalTest) {
    // Do Preliminary Check on a loop, aim for early/rapid bail out
    if (!doLoopPreliminaryChecks(Lp, CheckProfitability, SkipLoopBoundChecks)) {
      LLVM_DEBUG(dbgs() << "Reversal: Loop Preliminary Checks failed\n";);
      return false;
    }
  } else {
    // Assert legality (including loop bound checks) when the caller skips it.
    // This is for the utility path when the client is actually trying to
    // perform reversal.
    assert(doLoopPreliminaryChecks(Lp, false, false) &&
           "Loop Reversal Preliminary Test failed\n");
  }

  // Do collection and check result
  //(must do, can't control by a parameter)
  if (!doCollection(Lp, CheckProfitability)) {
    LLVM_DEBUG(dbgs() << "Reversal: collection failed\n");
    return false;
  }

  // Check Profitability
  if (CheckProfitability && !isProfitable(Lp)) {
    LLVM_DEBUG(dbgs() << "Reversal: Loop Profitability Test failed\n");
    return false;
  }

  if (DoLegalTest) {
    if (!isLegal(Lp)) {
      LLVM_DEBUG(dbgs() << "Reversal: Loop Legality Test failed\n");
      return false;
    }
  } else {
    // Assert legality when the caller skips it.
    assert(isLegal(Lp) && "Loop Reversal Legal Test failed\n");
  }

  return true;
}

// HIR Loop Reversal Transformation
// === Update Each MarkedCanonExpr ===
//
// [FROM]
// IV in CE: Ci*Bi*IV + Blob + C0
// IV' = UB-IV  (LB = 0)
//
// [TO]
// IV' in CE: Ci*Bi*(IV') + Blob + C0
//
// E.g.
// CE: 100 + (-1)*iv;
// IV': 4-iv
// After: 100 + (-1) *(4-iv)
//    =  100 + (-4 + iv)
//    =   96 + iv
//
// ** Detailed Steps: For-each MCE in Collection ***
//
// 1. Create CE' = -IV;
// 2. Update CE' = UB - IV;
// 3. CanonExprUtil::replaceIVWithCE(CE');
// 4. Call to RegDD.makeConsistent();
// 5. Mark the loop has changed, request HIR CodeGen support
//
bool HIRLoopReversal::doHIRReversalTransform(HLLoop *Lp) {
  // Get Loop's UpperBound (UB)
  CanonExpr *UBCE = Lp->getUpperCanonExpr();
  // LLVM_DEBUG(::dump(UBCE, "Loop's UpperBound (UB) CE:"););
  // LLVM_DEBUG(dbgs() << "UBCEDenom: " << UBCE->getDenominator() << "\n");

  //===  Do Loop Reversal Transformation for each MarkedCE  ===
  // For-each MCE in Collection:
  //  - Goal is to replace (IV) -> (UB - IV).
  //    (C * B * i1) -> (C * B * (UB - IV)) -> -C*B*i1 + C*B*UB
  //
  //  To do it:
  //  1. Replace IV with UB;
  //  2. Add IV with -C and B as a coeff and a blob index.
  //  3. RegDD.makeConsistent();
  //=== ---------------------------------------------------- ===

  // For each MCE in the MCEAV collection
  for (auto &MCE : MCEAV) {
    // LLVM_DEBUG(dbgs() << " MCE: "; MCE.dump());

    CanonExpr *CE = MCE.getCE();

    unsigned BlobIndex;
    int64_t Coeff;
    CE->getIVCoeff(LoopLevel, &BlobIndex, &Coeff);

    bool ReplaceIVByCE = CanonExprUtils::replaceIVByCanonExpr(
        CE, LoopLevel, UBCE, Lp->isNSW(), true);
    (void)ReplaceIVByCE;
    assert(ReplaceIVByCE && "replaceIVByCanonExpr(.) failed\n");

    CE->setIVCoeff(LoopLevel, BlobIndex, -Coeff);

    // Make the corresponding RegDDRef consistent with the new CE
    const SmallVector<const RegDDRef *, 3> AuxRefs = {Lp->getUpperDDRef()};
    RegDDRef *MemRef = MCE.getDDRef();
    MemRef->makeConsistent(AuxRefs, Lp->getNestingLevel());
  }
  // end_loop: MCE

  // Mark the loop has been changed, request CodeGen support
  assert(Lp->getParentRegion() && " Loop does not have a parent region\n");
  Lp->getParentRegion()->setGenCode();
  // Invalidate the loop's body
  // (with HIRLoopStatistics and HIRSafeReduction analysis passes preserved)
  HIRInvalidationUtils::invalidateBody<HIRLoopStatistics,
                                       HIRSafeReductionAnalysis>(Lp);

  return true;
}

void HIRLoopReversal::clearWorkingSetMemory(void) {
  MCEAV.clear();
}

PreservedAnalyses HIRLoopReversalPass::run(llvm::Function &F,
                                           llvm::FunctionAnalysisManager &AM) {
  HIRLoopReversal(AM.getResult<HIRFrameworkAnalysis>(F),
                  AM.getResult<HIRDDAnalysisPass>(F),
                  AM.getResult<HIRLoopStatisticsAnalysis>(F),
                  AM.getResult<HIRSafeReductionAnalysisPass>(F))
      .run();

  return PreservedAnalyses::all();
}

class HIRLoopReversalLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRLoopReversalLegacyPass() : HIRTransformPass(ID) {
    initializeHIRLoopReversalLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRSafeReductionAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRLoopReversal(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
               getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
               getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS(),
               getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR())
        .run();
  }
};

char HIRLoopReversalLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopReversalLegacyPass, "hir-loop-reversal",
                      "HIR Loop Reversal", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRLoopReversalLegacyPass, "hir-loop-reversal",
                    "HIR Loop Reversal", false, false)

FunctionPass *llvm::createHIRLoopReversalPass() {
  return new HIRLoopReversalLegacyPass();
}
