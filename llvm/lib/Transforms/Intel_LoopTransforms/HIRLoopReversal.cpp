// ===- HIRLoopReversal.cpp - Implements HIR Loop Reversal Transformation -===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// HIR Loop Reversal Example
//
// [ORIGINAL]                       [AFTER REVERSAL]
//
// for(i=0; i<=4; ++i){             for(i=0; i<=4; ++i){
//   A[100-i] = 0;                    A[96+i] = 0;
// }                                }
//
//===----------------------------------------------------------------------===//
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
// - no LiveOut Temp from loop's body;
// - Neither firstChild nor lastChild can be a nullptr;
//
// 1.Applicability: inner-most loop only, + negative constant coefficient in IV;
//   List of Nonos (within the loop's body):
//     -no call,
//     -no goto,
//     -no jump,
//     -no label;
//
// - Among the collected CEs within a loop:
//  . No symbolic part (Index != InvalidBlobCoeff)
//  . At least 1 negative constant coefficient
//
// 2.Legality:      remains legal after transformation;
//
// 3.Profitability: must have positive benefit after transformation.
//   (Use simple heuristic to model cache impact)
//
// [Transformation]
// See the example given above.
//
// TODO
// 1. Support #pragma REVERSE_LOOP
// 2. Allow simple forward gotos in the same iteration
// 3.
//
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRSafeReductionAnalysis.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopReversal.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#define DEBUG_TYPE "hir-loop-reversal"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::reversal;

// Command-line options

// Flag to disable the HIR Loop Reversal: default to OFF
static cl::opt<bool> DisableHIRLoopReversal(
    "disable-hir-loop-reversal", cl::init(false), cl::Hidden,
    cl::desc("Disable HIR Loop Reversal Transformation"));

// Statistical Counter(s):
STATISTIC(HIRLoopReversalAnalyzed, "Number of HIR Loop Analyzed for Reversal");
STATISTIC(HIRLoopReversalTriggered, "Number of HIR Loop Reversal Triggered");

// Predefined Constants, use as default values
const unsigned DefaultReadWeight = 1;         // default Weight for a Read is 1
const unsigned DefaultWriteWeight = 2;        // default Weight for a Write is 2
const unsigned DefaultShortTripThreshold = 4; // default short trip threshold

///\brief Collect all suitable MarkedCanonExprs on MemRef type of DDReg from the
/// given loop.
class MarkedCECollector final : public HLNodeVisitorBase {
private:
  SmallVectorImpl<MarkedCanonExpr> &CEVAP; // Vector of Collected MarkedCE
  unsigned LoopLevel = 0;                  // Current Loop Level
  bool AbortCollector = false; // Abort, if there is any non-linear DDRef on a
                               // CE with an IV matching on loop level.

public:
  explicit MarkedCECollector(SmallVectorImpl<MarkedCanonExpr>
                                 &InitMCER, // Vector of MarkedCanonExpr (MCE)
                             unsigned InitLevel)
      : CEVAP(InitMCER), LoopLevel(InitLevel), AbortCollector(false) {
    assert(LoopLevel <= MaxLoopNestLevel);
  }

  /// \brief Do not allow default constructor
  MarkedCECollector() = delete;

  // Getter: any prematurely abort?
  bool getCollectionAborted(void) const { return AbortCollector; }

  void visit(HLDDNode *Node) {
    for (auto I = Node->op_ddref_begin(), E = Node->op_ddref_end(); I != E;
         ++I) {
      checkAndCollectMCE(*I);
    }
  }

  // No processing needed for Goto, Label and HLNode types;
  void visit(HLGoto *Goto){};
  void visit(HLLabel *Label){};
  void visit(HLNode *Node) {
    llvm_unreachable(
        " visit(HLNode *) - Node not supported for HIR Loop Reversal.");
  }
  void postVisit(HLNode *Node) {}

  // Early BailOut: once AbortCollector is true
  bool isDone() const override { return AbortCollector; }

  // ----------------------------------------------------
  // |     | HasIV   |  HasNonLinear | Collect Decision |
  // ----------------------------------------------------
  // |Base |   NA    |  Y/N          |    N             |
  // ----------------------------------------------------
  // |Subs |   Y     |  N            |    Y             |
  // ----------------------------------------------------
  //
  // Note:
  // bool CollectDecision = HasIV && !HasNonLinear;
  //
  // Since the BaseCE CAN'T have any IV (with matching loop level), we don't
  // bother to collect from the base.
  //
  // We only collect the CE from the Subs section if it hasIV(LoopLevel)
  // AND is !NonLinear.
  //
  // Collection aborts if hasIV is true and AND NonLinear is also true at any
  // moment in the collection process.
  void checkAndCollectMCE(const RegDDRef *RegDD) {
    // 0.Setup
    assert(RegDD && "RegDDRef* can't be a nullptr\n");
    unsigned Dimension = 1; // Dimension begins from 1, in sync with iterator I

    // Debug Printer
    // formatted_raw_ostream FOS(dbgs());

    // 1. Check the Subs part only (refer to the table and analysis above)
    for (auto I = RegDD->canon_begin(), E = RegDD->canon_end(); I != E; ++I) {
      bool HasIV = false, HasNonLinear = false;
      CanonExpr *CE = (*I);
      assert(CE && "checkAndCollectMCE(.) -- CanonExpr* can't be nullptr\n");

      // See what we are checking:
      // DEBUG(FOS << "Checking: "; CE->print(FOS); FOS << "\n";);

      // 2. Check if the current CE has an IV on the matching loop level
      if (CE->hasIV(LoopLevel)) {
        HasIV = true;
      }

      // 3. Check if the current CE is NonLinear
      if (CE->isNonLinear()) {
        HasNonLinear = true;
      }

      // 4. Decide whether to collect the current CE
      //    also decide on collection abortion.
      // Abort collection if HasIV AND HasNonLinear are BOTH true
      AbortCollector = HasIV && HasNonLinear;
      if (AbortCollector) {
        return;
      }

      bool CollectCE = HasIV && !HasNonLinear;

      // 5. Collect if suitable
      if (CollectCE) {
        // See what we are collecting
        // DEBUG(FOS << "Collect: "; CE->print(FOS); FOS << "\n";);

        bool IsMemRef = RegDD->isMemRef();
        bool IsWrite = RegDD->isLval();

        // This CAN'T be moved out of loop, because the Stride's value depends
        // on Dimension, and Dimension adjusts per iteration.
        uint64_t Stride = 1; // Stride defaults to 1
        if (IsMemRef) { // Non-uniform Stride is only available on MemRef DDRef
          Stride = RegDD->getDimensionStride(Dimension);
        }

        // Collect the current MCE
        CEVAP.push_back(MarkedCanonExpr(CE, IsWrite, IsMemRef, Stride,
                                        const_cast<RegDDRef *>(RegDD)));
      }
      // end_if: CollectCE

      // 6. update Dimension to match I
      Dimension++;
    }
    // end_for: I

    // Done collection
  }
};

char HIRLoopReversal::ID = 0;

// HIRLoopReversal Pass Registration
INITIALIZE_PASS_BEGIN(HIRLoopReversal, "hir-loop-reversal", "HIR Loop Reversal",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysis)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysis)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatistics)
INITIALIZE_PASS_END(HIRLoopReversal, "hir-loop-reversal", "HIR Loop Reversal",
                    false, false)

// HIRLOOPReversal Pass's Global Creator
FunctionPass *llvm::createHIRLoopReversalPass() {
  return new HIRLoopReversal();
}

// Analyze Data-Dependence for the given Loop, for Reversal
struct HIRLoopReversal::CollectDDInfo final : public HLNodeVisitorBase {
  DDGraph &DDG;
  HLLoop *Lp;           // The Loop
  HIRLoopReversal *HLR; // The HIRLoopReversal Pass*, will call its isLegal(.)
  bool HasInvalidDV;    // any invalid DV for HIRLoopReversal?
  unsigned LoopLevel;   // loop's level

  explicit CollectDDInfo(DDGraph &DDGraphRef, HLLoop *InitLoopPtr,
                         HIRLoopReversal *InitHLR, unsigned InitLoopLevel)
      : DDG(DDGraphRef), Lp(InitLoopPtr), HLR(InitHLR), HasInvalidDV(false),
        LoopLevel(InitLoopLevel) {
    assert(Lp && "CollectDDInfo(.) -- Lp can't be null\n");
    assert(HLR && "CollectDDInfo(.) -- HLR can't be null\n");
  }

  void visit(const HLNode *Node) {}

  bool isDone() const override { return HasInvalidDV; }

  // Any premature abort?
  bool getCollectionAborted(void) const { return HasInvalidDV; }

  void visit(const HLInst *Inst) {
    if (HLR->SRA->isSafeReduction(Inst)) { // Ignore SafeReduction!
      return;
    }

    // Examine the current HLInst
    // DEBUG(Inst->dump(););

    // Iterate over each DDRef
    for (auto I = Inst->op_ddref_begin(), E = Inst->op_ddref_end(); I != E;
         ++I) {
      // Iterate over each outgoing edge:
      for (auto II = DDG.outgoing_edges_begin(*I),
                EE = DDG.outgoing_edges_end(*I);
           II != EE; ++II) {

        // Examine the DDEdge:
        const DDEdge *Edge = (*II);
        // DEBUG(Edge->print(dbgs()););
        // DEBUG(::dump(Edge, "Current Outgoing DDEdge:"););
        //(void)Edge;

        // 1.Ignore any edge if its sink is out of the current loop
        if (!(HLNodeUtils::contains(Lp, Edge->getSink()->getHLDDNode()))) {
          continue;
        }

        // 2. Check Current DV is legal to reverse
        const DirectionVector &DV = Edge->getDV();
        // DEBUG(DV.print(dbgs(), true));

        bool ValidDV = HLR->isLegal(DV, LoopLevel);
        // Abort Collection if invalid!
        if (!ValidDV) {
          HasInvalidDV = true;
          return;
        }
      }
      // end loop: out-going edge
    }
  }

  void postVisit(const HLNode *Node) {}
};

// *** HIRLoopReversal Pass's Implementation ***

/// \brief Default constructor of HIRLoopReversal
HIRLoopReversal::HIRLoopReversal(void) : HIRTransformPass(ID) {
  initializeHIRLoopReversalPass(*PassRegistry::getPassRegistry());
}

/// \brief Add all needed analysis passes.
void HIRLoopReversal::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequiredTransitive<HIRFramework>();
  AU.addRequiredTransitive<HIRDDAnalysis>();
  AU.addRequiredTransitive<HIRSafeReductionAnalysis>();
  AU.addRequiredTransitive<HIRLoopStatistics>();
  AU.setPreservesAll();
}

/// \brief Do Quick Preliminary Checks on the given loop
/// Quick checks on early bail-out conditions: even before doing any
/// LoopCollection or isApplicable() tests.
//
// The following preliminary conditions are currently checked:
// - Multiple exits;
// - Normalized Loop;
// - Loops' UpperBound CAN'T have any IV on the loop's matching level;
// - Loop has NO LiveOut Temp(s);
// - UBCE Checks: (i) getDenominator is 1, (ii) can convert to StandaloneBlob;
// - Neither firstChild nor lastChild can be a nullptr;
//
bool HIRLoopReversal::doLoopPreliminaryChecks(const HLLoop *Lp) {
  // 0.Sanity
  assert(Lp && "HIRLoopReversal::doLoopPreliminaryChecks(const HLLoop *) "
               "assert failed");

  // 1. No multiple-exit loop or unknown loops
  if (!Lp->isDo()) {
    return false;
  }

  // 2. No non-normalized loop
  // (Expect the reversal candidates to be a normalized loop)
  if (!Lp->isNormalized()) {
    return false;
  }

  // 3. Loops' UpperBound CAN'T have IV for the loop's matching level
  const CanonExpr *UBCE = Lp->getUpperCanonExpr();
  if (UBCE->hasIV(Lp->getNestingLevel())) {
    return false;
  }

  // 4. Loop has NO LiveOut Temp(s)
  if (Lp->hasLiveOutTemps()) {
    return false;
  }

  // 5. Checks on UBCE
  // 5.1 UBCE's Denominator must be 1
  if (!(UBCE->getDenominator() == 1)) {
    return false;
  }
  // TODO:
  // -Consider support loops with non-uniform denominator

  // 5.2 UBCE can be converted to StandAloneBlob
  if (!UBCE->canConvertToStandAloneBlob()) {
    return false;
  }

  // 6. Neither firstChild nor lastChild can be a nullptr
  if ((Lp->getFirstChild() == nullptr) || (Lp->getLastChild() == nullptr)) {
    return false;
  }

  // Done, return Good!
  return true;
}

/// \brief Collect all suitable MarkedCanonExprs from the given Loop
bool HIRLoopReversal::doLoopCollection(HLLoop *Lp) {
  // 0.Sanity
  assert(Lp && "HIRLoopReversal::doLoopCollection(HLLoop *) assert fired\n");

  // 1.Collect Loop-related CanonExpr and DDRefs into CEAV vector
  MarkedCECollector MCEC(CEAV, LoopLevel);
  HLNodeUtils::visitRange(MCEC, Lp->getFirstChild(), Lp->getLastChild());

  // 2.Check if Collection aborts prematurely
  bool CollectionAborted = MCEC.getCollectionAborted();
  if (CollectionAborted) {
    return false;
  }

  // Debug: see all MCEs we have collected
  // DEBUG(::dump(CEAV, StringRef("All Collected MCEs:")););

  // 3.Collection Good if not aborted
  return true;
}

///\brief A dedicated routine to handle all command-line arguments.
bool HIRLoopReversal::handleCmdlineArgs(Function &F) {
  // 1. Skip the Pass if DisableHIRLoopReversal flag is enabled
  // or
  // support opt-bisect via skipFunction() call
  if (DisableHIRLoopReversal || skipFunction(F)) {
    DEBUG(dbgs() << "HIR Loop Reversal Transformation Disabled through "
                    "DisableHIRLoopReversal flag\n");
    return false;
  }

  // 2. Done
  return true;
}

// main entry of the HIRLoopReversal FunctionPass
bool HIRLoopReversal::runOnFunction(Function &F) {
  // 0. Sanity+Setup

  // process cmdline argument(s)
  bool CmdLineOptions = handleCmdlineArgs(F);
  if (!CmdLineOptions) {
    return false;
  }

  DEBUG(dbgs() << "HIR LoopReversal on Function : " << F.getName() << "\n");

  // Gather ALL Innermost Loops as Candidates, use 64 increment
  SmallVector<HLLoop *, 64> CandidateLoops;
  HLNodeUtils::gatherInnermostLoops(CandidateLoops);
  // DEBUG(dbgs() << " # Innermost Loops: " << CandidateLoops.size() << "\n");
  if (CandidateLoops.empty()) {
    return false;
  }

  // Obtain Analysis Result(s)
  DDA = &getAnalysis<HIRDDAnalysis>();
  SRA = &getAnalysis<HIRSafeReductionAnalysis>();
  HLS = &getAnalysis<HIRLoopStatistics>();

  // TODO:
  // Re-Build DDA on demand if needed

  // 1.Iterate Over Each Candidate Loop
  for (auto &Lp : CandidateLoops) {
    // 2. Check the loop's suitability for reversal
    bool SuitableLoop = isReversible(Lp);

    // Update HIRLoopReversalAnalyzed Statistical Counter
    HIRLoopReversalAnalyzed++;

    // 3. Do HIR Loop Reversal Transformation if suitable
    if (SuitableLoop) {
      bool DoReversal = doHIRReversalTransform(Lp);

      // Update Loops-Reversal-Performed Counter
      if (DoReversal) {
        HIRLoopReversalTriggered++;

        // Debug dump after a Loop Reversal
        // DEBUG(dbgs() << "\n Reversed Loop: \n"; Lp->dump());
      }
    }
  }
  // end: for loop on I

  // 4.Cleanup and Return
  CandidateLoops.clear();
  clearWorkingSetMemory();
  return false;
}

// *** Applicability Check: is the Loop applicable for HIR Loop Reversal ***
// From the generic CanonExpr form:
// (C1*B1*i1 + C2*B2*i2 + ... + BC1*b1 + BC2*b2 + ... + C0) / D
//
// List of NoNos in Loop's Body (3)
// - No function call
// - No label
// - No goto
//
// Loop Body Tests (2)
// For all collected MarkedCEs:
// - No symbolic part
// - At least 1 negative constant coefficient
//
bool HIRLoopReversal::isApplicable(HLLoop *Lp) {
  assert(Lp && "HIRLoopReversal::isApplicableLoop(HLLoop *) assert failed");

  // 1. Do LS Collection
  // CHECK1: List of Nonos (Need to collect Loop Statistics once)
  // - No function call
  // - No label
  // - No goto
  const LoopStatistics &LS = HLS->getSelfLoopStatistics(Lp);

  // DEBUG(LS.dump(););
  if (LS.hasCalls() || LS.hasGotos() || LS.hasLabels()) {
    return false;
  }

  // CHECK2: Loop Body Tests (2)
  // for all MCEs connected:
  // - 1. No symbolic part (Index != InvalidBlobCoeff)
  // AND
  // - 2. At least 1 negative constant coefficient
  bool HasNonZeroBlobCoeff = false;
  bool HasNegativeIVConst = false;

  // 2. Iterate over each MCE inside the CEAV collection
  for (auto &MCE : CEAV) {
    unsigned Index = 0;
    int64_t Coeff = 0;
    CanonExpr *CE = MCE.getCE();
    CE->getIVCoeff(LoopLevel, &Index, &Coeff);

    // 2.1 Check if the IVBlobCoeff is valid
    if (Index != InvalidBlobIndex) {
      HasNonZeroBlobCoeff = true;
      break; // prematurely abort the loop, for performance
    }

    // 2.2 Check if the IVConst is negative
    if (Coeff < 0) {
      HasNegativeIVConst = true;
    }
  }
  // end_for: CEAV

  // DEBUG(dbgs() << "any NonZeroBlobCoeff? : " << HasNonZeroBlobCoeff <<
  // "\n");
  if (HasNonZeroBlobCoeff) { // Bail out if any BlobCoeff (Symbolic) is non 0
    return false;
  }

  // DEBUG(dbgs() << "Any NegativeIVConst? : " << HasNegativeIVConst << "\n");
  if (!HasNegativeIVConst) { // Bail out if not any NegativeIVConst is found
    return false;
  }

  // Done: return true finally!
  return true;
}

// Profitability Analysis for HIR Loop Reversal
//
// *** ALGORITHM SCRATCH: using Heuristic ***
// For all DDR MemRef using IV in the matching loop level,
// 1. Count # of negative IV CEs
// 2. Count # of positive IV CEs
// 3. Decision: return (CountNegIVs > CountPosIVs);
//
// Note:
// 1. If the IV's constant coeff is not 1, use its reciprocal;
// 2. Even if the IV's blob is not 0, ignore its blob value;
// 3. Use different weights to differentiate between a write and a read:
//    heuristic: W (2) vs. R (1)
// 4. Consider: skip LOW trip-count loops, or add potential overhead of
//    reversal into the cost metric;
//
bool HIRLoopReversal::isProfitable(const HLLoop *Lp) {
  // Sanity Checks and Setup;
  assert(Lp && "HIRLoopReversal::isProfitable(.) assert: Loop can't be NULL\n");

  // 1. Filter off any loop with constant trip count below the given threshold
  uint64_t TripCount = 0;
  if (Lp->isConstTripLoop(&TripCount)) {
    if (TripCount < DefaultShortTripThreshold) {
      return false;
    }
  }

  // 2. Accumulate weights of negative IV CEs and positive IV CEs
  unsigned AccumuWeightNegIVs = 0, AccumuWeightPosIVs = 0, Weight = 0;
  for (auto &MCE : CEAV) {
    // 2.1 Get related IV info on LoopLevel: IVCoeff, IVBlob,
    // DEBUG(MCE.dump());
    unsigned IVIndex = 0;
    int64_t IVCoeff = 0;
    MCE.getCE()->getIVCoeff(LoopLevel, &IVIndex, &IVCoeff);
    bool IsWrite = MCE.isWrite();
    bool IsMemRef = MCE.isMemRef();
    uint64_t Stride = MCE.getStride();

    // Allow ONLY a MemRef type of MarkedCanonExpr to participate in profit
    // analysis.
    if (IsMemRef) {
      // 2.2 Use a different weight, differentiating between a Write and Read
      IsWrite ? (Weight = DefaultWriteWeight) : (Weight = DefaultReadWeight);

      // 2.3 Accumulate Weight on Positive IVs
      // Use scaled integer math to replace precise floating-point math
      // and reduce cost.
      if (IVCoeff > 0) {
        AccumuWeightPosIVs += (1000 / (IVCoeff * Stride)) * Weight;
      }
      // 2.4 Accumulate Weight on Negative IVs
      else { // flip the sign when calculating
        AccumuWeightNegIVs += (1000 / ((-IVCoeff) * Stride)) * Weight;
      }
    }
  }

  // Examine the values of AccumuWeightNegIVs and AccumuWeightPosIVs:
  DEBUG(dbgs() << "  AccumuWeightNegIVs :" << AccumuWeightNegIVs << "  "
               << "  AccumuWeightPosIVs :" << AccumuWeightPosIVs << "  "
               << "\n");

  // 3. Decision: true if negative IVs have higher weight
  return (AccumuWeightNegIVs > AccumuWeightPosIVs);
}

/* -----------------------------------------------------------------------*/
// Legality Tests for HIR Loop Reversal: is it legal to do Loop Reversal?
// ALGORITHM Scratch:
//
// For the Candidate Loop:
// 1. Collect All DVs
// 2. For each collected DV
//    - it must be LEGAL for reversal
//
// How is a DV Legal for Reversal?
// (1) if on last level and still legal so far: use existing last-level
// knowledge;
//
// (2) if not (YET) on last level:
//   . if DV on current level is < or >, Return true on the current DV;
//   . if current one is =, advance to next level;
//
/* ----------------------------------------------------------------------- */
bool HIRLoopReversal::isLegal(const HLLoop *Lp) {
  // 0. Sanity Check/Setup;
  assert(Lp &&
         "HIRLoopReversal::isLegal(.) assert fired: Loop can't be NULL\n");
  // DEBUG(dbgs() << "Current Loop: \n"; Lp->dump(););
  HLLoop *Lp2 = const_cast<HLLoop *>(Lp); // cast-away the constant qualifier

  // Get DDGraph
  DDGraph DDG = DDA->getGraph(Lp2, false);
  SRA->computeSafeReductionChains(Lp2);
  // Check the DDG:
  // DEBUG(dbgs() << "Dump the Full DDGraph:\n"; DDG.dump(););

  // 1. Collect All DVs from the Loop;
  // Note: legality test is inside CollectDDInfo, no DVs are ever returned!
  CollectDDInfo CDDI(DDG, Lp2, this, LoopLevel);
  HLNodeUtils::visit(CDDI, Lp2);

  // 2. Check collection abortion
  bool CollectionAborted = CDDI.getCollectionAborted();
  if (CollectionAborted) {
    // Legality Test fails if there is at least 1 invalid DV!
    return false;
  }

  // 3.Done, all good!
  return true;
}

/// \brief Legality check for a given DirectionVector with a given loop level
bool HIRLoopReversal::isLegal(const DirectionVector &DV, unsigned Level) {
  // 0.Setup;
  // DEBUG(DV.print(dbgs(), true););

  // 1. Case1: true if the DV at the given level is DVKind::EQ
  if (DV[Level - 1] == DVKind::EQ) {
    return true;
  }

  // 2. Case2: true if the DV at any previous level is DVKind::LT or
  // DVKind::GT
  // DVKind::LT:  001
  // DVKind::GT : 100
  // LT | GT :    101 <=> Either LT or GT <=> DVKind::NE
  // DVKind::EQ : 010
  for (unsigned Lvl = 1; Lvl <= Level - 1; ++Lvl) {
    if ((DV[Lvl - 1] & DVKind::EQ) == DVKind::NONE) {
      return true;
    }
  }

  // 3.Default Case: fail-through as false
  return false;
}

/// \brief Conduct ALL HIR-Loop-Reversal Tests to decide whether the given
/// loop
/// is suitable for reversal.
bool HIRLoopReversal::isReversible(HLLoop *Lp) {
  // 0. Sanity Check/Setup;
  assert(Lp && "HIRLoopReversal::isReversible(.) assert fired: Loop can't be "
               "nullptr\n");
  // DEBUG(dbgs() << "Current Loop: \n"; Lp->dump(););

  // 1. Clean the plate before any work begins
  clearWorkingSetMemory();

  // Show The Current Loop
  // DEBUG(Lp->dump(););
  LoopLevel = Lp->getNestingLevel();

  // 2. Do Preliminary Check on the given loop, aim for early/rapid bail out
  if (!doLoopPreliminaryChecks(Lp)) {
    DEBUG(dbgs() << "Reversal: failed Loop Preliminary Checks\n";);
    return false;
  }

  // 3. Collect qualified MCEs, and check any failure for early bail out
  if (!doLoopCollection(Lp)) {
    DEBUG(dbgs() << "Reversal: failed Loop Collection due to NonLinear flag "
                    "present\n";);
    return false;
  }

  // 4. Check Applicability
  if (!isApplicable(Lp)) {
    DEBUG(dbgs() << "Reversal: failed Loop Applicability Test\n");
    return false;
  }

  // 5. Check Profitability
  if (!isProfitable(Lp)) {
    DEBUG(dbgs() << "Reversal: failed Loop Profitability Test\n");
    return false;
  }

  // 6. Check Legality
  if (!isLegal(Lp)) {
    DEBUG(dbgs() << "Reversal: failed Loop Legality Test\n");
    return false;
  }

  // If ALL tests pass, the loop is suitable to reverse.
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
// ** Detailed Steps **
// 0. Setup
// For-each MCE in Collection:
//  1. Create CE' = -IV;
//  2. Update CE' = UB - IV;
//  3. CanonExprUtil::replaceIVWithCE(CE');
//  4. Call to RegDD.makeConsistent();
// 5. Mark the loop has changed, request HIR CodeGen support
//
bool HIRLoopReversal::doHIRReversalTransform(HLLoop *Lp) {
  // Sanity Checks and Setup;
  assert(Lp && "HIRLoopReversal::doHIRReversalTransform(.) assert "
               "fired: Loop cannot be NULL\n");

  // 1.Setup
  // Note: Since the loop is normalized, LB is always 0. No need for LB.

  // 1.1 Get Loop's UpperBound (UB)
  CanonExpr *UBCE = Lp->getUpperCanonExpr();
  // DEBUG(::dump(UBCE, "Loop's UpperBound (UB) CE:"););
  // DEBUG(dbgs() << "UBCEDenom: " << UBCE->getDenominator() << "\n");

  //=== 2 Do Loop Reversal Transformation for each MarkedCE  ===
  // For-each MCE in Collection:
  //  1. Create CE' = -IV;
  //  2. Update CE' = UB - IV;
  //  3. CanonExprUtil::replaceIVWithCE(CE');
  //  4. Call to RegDD.makeConsistent();
  //=== ---------------------------------------------------- ===

  // For each MCE in the CEAV collection
  for (auto &MCE : CEAV) {
    // DEBUG(dbgs() << " MCE: "; MCE.dump());

    // 2.0 Setup
    CanonExpr *CE = MCE.getCE();

    // 2.1 Build CE' = -IV on the matching LoopLevel
    CanonExpr *CEPrime = CanonExprUtils::createExtCanonExpr(
        CE->getSrcType(), CE->getDestType(), CE->isSExt());
    CEPrime->setIVCoeff(LoopLevel, InvalidBlobIndex, -1);

    // 2.2 Check if CE is merge-able with UBCE
    bool MergeableCase = CanonExprUtils::mergeable(CE, UBCE, true);

    // Handle merge-able case: Merge directly
    if (MergeableCase) {
      // 2.3 Update: CE' = UB - IV; (LB is always 0)
      CEPrime = CanonExprUtils::add(CEPrime, UBCE, true);
      assert(CEPrime && "CanonExprUtils::add(.) failed on UBCE\n ");
      // DEBUG(::dump(CEPrime, "CEPrime, Expect: CE' = UB - IV"));

      // 2.4 Replace original IV with CE' = UB - IV;
      // DEBUG(::dump(CE, "CE [BEFORE replaceIVByCanonExpr(.)]\n"););
      bool ReplaceIVByCE =
          CanonExprUtils::replaceIVByCanonExpr(CE, LoopLevel, CEPrime, true);
      (void)ReplaceIVByCE;
      assert(ReplaceIVByCE && "replaceIVByCanonExpr(.) failed\n");
      // DEBUG(::dump(CE, "CE [AFTER replaceIVByCanonExpr(.)]\n"););
    }
    // handle StandaloneBlob Case (not merge-able case)
    else {
      // 2.5 Cast UBCE for StandaloneBlog form;
      DEBUG(auto T0 = CEPrime->getSrcType(); auto T1 = UBCE->getSrcType();
            dbgs() << "CEPrime->getSrcType(): "; T0->print(dbgs());
            dbgs() << "  "
                   << "UBCE->getSrcType(): ";
            T1->print(dbgs()); dbgs() << "\n";);

      // Make a UBCE clone, work ONLY with the clone for the rest of this
      // section!
      CanonExpr *UBCEClone = UBCE->clone();
      // DEBUG(::dump(UBCEClone, "UBCEClone [BEFORE]: "));

      bool CastToStandaloneBlob = false;
      if (CEPrime->getSrcType() != UBCE->getSrcType()) {
        CastToStandaloneBlob =
            UBCEClone->castStandAloneBlob(CE->getSrcType(), false);
      } else {
        // assert(0 && "Expect different Src Types\n");
        // Bring the next line back if the above assert fires!
        CastToStandaloneBlob = UBCEClone->convertToStandAloneBlob();
      }

      // DEBUG(::dump(UBCEClone, "UBCEClone [AFTER]: "));
      // Note: the CastToStandaloneBlob is may NOT necessarily return true,
      // and
      // it is not an error if not!
      // assert(CastToStandaloneBlob &&
      //      "Expect castToStandAloneBlob() to be always succeed\n");
      (void)CastToStandaloneBlob;

      // 2.6 Build: CE' = UBCEClone - iv;
      CEPrime = CanonExprUtils::add(CEPrime, UBCEClone, true);
      assert(CEPrime && "CanonExprUtils::add(.) failed on UBCE\n ");
      // DEBUG(::dump(CEPrime, "Expect: CE' = UB - iv"));

      // 2.7 Replace CE's original IV with the CE' = UB - IV;
      bool ReplaceIVByCE =
          CanonExprUtils::replaceIVByCanonExpr(CE, LoopLevel, CEPrime, true);
      assert(ReplaceIVByCE && "replaceIVByCanonExpr(.) failed\n");
      (void)ReplaceIVByCE;
      // DEBUG(::dump(CE, "CE After replaceIVByCanonExpr(.)\n"););

      // 2.8 Cleanup: remove UBCEClone
      CanonExprUtils::destroy(UBCEClone);
    }

    // 3. Make the corresponding RegDDRef consistent with the new CE
    const SmallVector<const RegDDRef *, 3> AuxRefs = {Lp->getUpperDDRef()};
    RegDDRef *MemRef = MCE.getDDRef();
    MemRef->makeConsistent(&AuxRefs, Lp->getNestingLevel() - 1);
  }
  // end_loop: MCE

  // 4. Mark the loop has been changed, request CodeGen support
  assert(Lp->getParentRegion() && " Loop does not have a parent region\n");
  Lp->getParentRegion()->setGenCode();
  // Invalidate the loop's body
  HIRInvalidationUtils::invalidateBody(Lp);

  // 5.Return Loop Reversal Result
  //-true:  successfully reversed
  //-false: fail to reverse
  return true;
}

// \brief Clear per-iteration working-set memory
void HIRLoopReversal::clearWorkingSetMemory(void) { CEAV.clear(); }

// Free any memory allocated while HIRLoopReversal is running
void HIRLoopReversal::releaseMemory(void) { clearWorkingSetMemory(); }

/// \brief Run the HIRLoopReversal pass on a HLLoop * as an external utility
/// function.
//
// Actions:
// 1. Detect whether the loop is suitable for reversal;
// 2. If suitable and the DoReverse flag is true, reverse it;
// 3. Return: bool
//  - true:  if the loop is suitable for reversal
//  - false: otherwise
//
bool HIRLoopReversal::runOnLoop(
    HLLoop *Lp,          // INPUT + OUTPUT PARAM: a given loop
    bool DoReverse,      // INPUT PARAM: true to reverse if the loop is suitable
    HIRDDAnalysis &HDDA, // INPUT PARAM: an existing HIRDDAnalysis
    HIRSafeReductionAnalysis &HSRA, HIRLoopStatistics &LS,
    bool &LoopReversed // OUTPUT PARAM: true if the loop is successfully
                       // reversed
    ) {
  // 0.Sanity
  assert(Lp &&
         "HIRLoopReversal.runOnLoop(.) assert -- Loop can't be nullptr\n");

  // Obtain DDA Analysis Result from Parameter
  DDA = &HDDA;
  SRA = &HSRA;
  HLS = &LS;

  // 1. Check if the loop is suitable for reversal
  bool ReversibleLoop = isReversible(Lp);
  if (!ReversibleLoop) {
    LoopReversed = false;
    return false;
  }

  // 2. Reverse if suitable and DoReverse is true
  if (ReversibleLoop && DoReverse) {
    LoopReversed = doHIRReversalTransform(Lp);
  }

  // 3. Return whether the loop is Reversible
  return ReversibleLoop;
}
