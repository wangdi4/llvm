//==--- HIRSymbolicTripCountCompleteUnroll.cpp -----------------*- C++-*---===//
// Implements HIR Loop Early Pattern Match Pass.
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a special HIR Loop Pattern Matching Pass for Symbolic
// TripCount 2-level loop nest.
//
// Available options:
// -hir-pm-symbolic-tripcount-completeunroll: Perform HIR Symbolic TripCount
// Complete Unroll Pattern-Matching Pass
//
// -disable-hir-pm-symbolic-tripcount-completeunroll: Disable
// HIR Symbolic TripCount CompleteUnroll Pattern-Matching pass
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRSymbolicTripCountCompleteUnrollPass.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/TargetTransformInfo.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"

#include "HIRSymbolicTripCountCompleteUnrollImpl.h"

#define DEBUG_TYPE "hir-pm-symbolic-tripcount-completeunroll"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::unrollsymtc;

const std::string TempName = "mv";

// Flag to disable the HIR Loop Pattern Match Early Optimization
static cl::opt<bool> DisableHIRSymbolicTripCountCompleteUnroll(
    "disable-hir-pm-symbolic-tripcount-completeunroll", cl::init(false),
    cl::Hidden,
    cl::desc(
        "Disable HIR Symbolic TripCount Complete-Unroll Pattern Match Pass"));

STATISTIC(NumHIRSymbolicTripCountCompleteUnroll,
          "Number of HIR Symbolic TripCount CompleteUnroll Pattern(s) Matched");

// *** BEGIN: StructuralCollector ***
//
// Collect the following Node(s) from the loop nest:
// - OuterLpNodeVec: all HLNode(s) on OuterLp level;
// - InnerLpNodeVec: all HLNode(s) on InnerLp level;
// - HLIf* :         on any level;
// - RegDDRef*: OuterLp level only, on Non-Local MemRef only
//
// The collector also populates the OuterLpNodeVec into
// - OuterLpInstVec
// - OuterLpLabelVec
// - OuterLpGotoVec
//
struct HIRSymbolicTripCountCompleteUnroll::StructuralCollector final
    : public HLNodeVisitorBase {
  HIRSymbolicTripCountCompleteUnroll *HSTCCU = nullptr;
  SmallVectorImpl<HLNode *> &OuterLpNodeVec;
  SmallVectorImpl<HLNode *> &InnerLpNodeVec;
  SmallVectorImpl<HLIf *> &HLIfVec;
  SmallVectorImpl<RegDDRef *> &NonLocalRefVec;

  SmallVectorImpl<HLInst *> &OuterLpInstVec;
  SmallVectorImpl<HLLabel *> &OuterLpLabelVec;
  SmallVectorImpl<HLGoto *> &OuterLpGotoVec;

public:
  explicit StructuralCollector(HIRSymbolicTripCountCompleteUnroll *HSTCCU,
                               SmallVectorImpl<HLIf *> &HLIfVec)
      : HSTCCU(HSTCCU), OuterLpNodeVec(HSTCCU->OuterLpNodeVec),
        InnerLpNodeVec(HSTCCU->InnerLpNodeVec), HLIfVec(HLIfVec),
        NonLocalRefVec(HSTCCU->NonLocalRefVec),
        OuterLpInstVec(HSTCCU->OuterLpInstVec),
        OuterLpLabelVec(HSTCCU->OuterLpLabelVec),
        OuterLpGotoVec(HSTCCU->OuterLpGotoVec) {}

  void visit(HLNode *Node);

  void postVisit(HLNode *Node) {}

#ifndef NDEBUG
  void print(bool PrintOuterLpNode = true, bool PrintOuterLpInstNode = true,
             bool PrintOuterLpLabelNode = true,
             bool PrintOuterLpGotoNode = true, bool PrintInnerLpNode = true,
             bool PrintHLIf = true, bool PrintNonLocalDDRef = true,
             bool PrintIndividualNewLine = true) const;
#endif
};

void HIRSymbolicTripCountCompleteUnroll::StructuralCollector::visit(
    HLNode *Node) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  LLVM_DEBUG(Node->dump(); FOS << "\n";);

  // Skip any HLLoop* or HLSwitch* type:
  if (dyn_cast<HLLoop>(Node) || dyn_cast<HLSwitch>(Node)) {
    return;
  }

  // Collect any HLIf:
  if (dyn_cast<HLIf>(Node)) {
    HLIfVec.push_back(dyn_cast<HLIf>(Node));
    return;
  }

  bool IsLabel = isa<HLLabel>(Node);
  bool IsGoto = isa<HLGoto>(Node);
  bool IsInst = isa<HLInst>(Node);
  bool IsInOuterLp =
      (dyn_cast<HLLoop>(Node->getParentLoop()) == HSTCCU->OuterLp);
  bool IsInInnerLp =
      (dyn_cast<HLLoop>(Node->getParentLoop()) == HSTCCU->InnerLp);

  // Collect from OuterLp:
  if (IsInOuterLp) {
    OuterLpNodeVec.push_back(Node);

    // Handle: HLInst, HLLabel, and HLGoto
    if (IsInst) {
      OuterLpInstVec.push_back(dyn_cast<HLInst>(Node));
    } else if (IsGoto) {
      OuterLpGotoVec.push_back(dyn_cast<HLGoto>(Node));
    } else if (IsLabel) {
      OuterLpLabelVec.push_back(dyn_cast<HLLabel>(Node));
    } else {
      llvm_unreachable("visit(HLNode *) - Not expect control to reach here: "
                       "Collect OuterLp Node(s).\n");
    }
  }
  // Collect from InnerLp:
  else if (IsInInnerLp) {
    InnerLpNodeVec.push_back(Node);
  }
  // Otherwise: error case
  else {
    llvm_unreachable("visit(HLNode *) - Not expect control to reach here.\n");
  }

  if (!IsInst) {
    return;
  }

  // Collect any Non-LOCAL MemRef from the HLInst Node:
  HLInst *HInst = cast<HLInst>(Node);
  for (auto I = HInst->op_ddref_begin(), E = HInst->op_ddref_end(); I != E;
       ++I) {
    RegDDRef *Ref = (*I);

    // LLVM_DEBUG(Ref->dump(); FOS << "\n";);
    if (HIRSymbolicTripCountCompleteUnroll::isNonLocalMemRef(Ref)) {
      NonLocalRefVec.push_back(Ref);
    }
  }
}

#ifndef NDEBUG
void HIRSymbolicTripCountCompleteUnroll::StructuralCollector::print(
    bool PrintOuterLpNode, bool PrintOuterLpInstNode,
    bool PrintOuterLpLabelNode, bool PrintOuterLpGotoNode,
    bool PrintInnerLpNode, bool PrintHLIf, bool PrintNonLocalDDRef,
    bool PrintIndividualNewLine) const {

  formatted_raw_ostream FOS(dbgs());

  // Print the OuterLpNodeVec:
  if (PrintOuterLpNode) {
    FOS << "Nodes From OuterLp: " << OuterLpNodeVec.size() << "\n";
    for (auto I = OuterLpNodeVec.begin(), E = OuterLpNodeVec.end(); I != E;
         ++I) {
      // Print a HLDDNode:
      HLDDNode *DDNode = dyn_cast<HLDDNode>(*I);
      if (DDNode) {
        DDNode->dump();
      }

      // Print a HLLabel:
      HLLabel *Label = dyn_cast<HLLabel>(*I);
      if (Label) {
        Label->dump();
      }

      // Print a HLGoto:
      HLGoto *Goto = dyn_cast<HLGoto>(*I);
      if (Goto) {
        Goto->dump();
      }
    }
    FOS << "\n";
  }

  // Print the OuterLpInstVec:
  if (PrintOuterLpInstNode) {
    FOS << "HLInst(s) From OuterLp: " << OuterLpInstVec.size() << "\n";
    for (auto I = OuterLpInstVec.begin(), E = OuterLpInstVec.end(); I != E;
         ++I) {
      (*I)->dump();
    }
    FOS << "\n";
  }

  // Print the OuterLpLabelVec:
  if (PrintOuterLpLabelNode) {
    FOS << "HLLabel(s) From OuterLp: " << OuterLpLabelVec.size() << "\n";
    for (auto I = OuterLpLabelVec.begin(), E = OuterLpLabelVec.end(); I != E;
         ++I) {
      (*I)->dump();
    }
    FOS << "\n";
  }

  // Print the OuterLpGotoVec:
  if (PrintOuterLpGotoNode) {
    FOS << "HLGoto(s) From OuterLp: " << OuterLpGotoVec.size() << "\n";
    for (auto I = OuterLpGotoVec.begin(), E = OuterLpGotoVec.end(); I != E;
         ++I) {
      (*I)->dump();
    }
    FOS << "\n";
  }

  // Print the InnerLpNodeVec:
  if (PrintInnerLpNode) {
    FOS << "Nodes from InnerLp: " << InnerLpNodeVec.size() << "\n";
    for (auto I = InnerLpNodeVec.begin(), E = InnerLpNodeVec.end(); I != E;
         ++I) {
      // Print a HLDDNode:
      HLDDNode *DDNode = dyn_cast<HLDDNode>(*I);
      if (DDNode) {
        DDNode->dump();
      }

      // Print a HLLabel:
      HLLabel *Label = dyn_cast<HLLabel>(*I);
      if (Label) {
        Label->dump();
      }

      // Print a HLGoto:
      HLGoto *Goto = dyn_cast<HLGoto>(*I);
      if (Goto) {
        Goto->dump();
      }
    }
  }
  FOS << "\n";

  // Print the HLIfVec:
  if (PrintHLIf) {
    FOS << "HLIf(s): " << HLIfVec.size() << "\n";
    unsigned Count = 0;

    for (auto If : HLIfVec) {
      FOS << "HLIF" << Count++ << ":\n";
      If->dump();
      FOS << "\n";
    }
  }
  FOS << "\n";

  // Print the NonLocalRefVec:
  if (PrintNonLocalDDRef) {
    FOS << "NonLocalDDRef(s): " << NonLocalRefVec.size() << "\n";
    for (auto Ref : NonLocalRefVec) {
      FOS << " ";
      Ref->dump();
      FOS << "\n";
    }
  }
  FOS << "\n";
}
#endif
// ### END: StructuralCollector ###

bool HIRSymbolicTripCountCompleteUnroll::run() {
  if (DisableHIRSymbolicTripCountCompleteUnroll) {
    LLVM_DEBUG(dbgs() << "HIR Loop Pattern Match Early Disabled\n");
    return false;
  }

  if (!TTI.isAdvancedOptEnabled(
          TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2)) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIRSymbolicTripCountCompleteUnroll on Function : "
                    << HIRF.getFunction().getName() << "()\n");

  // Gather all innermost Loop Candidates:
  SmallVector<HLLoop *, 64> InnermostLoops;
  HNU.gatherInnermostLoops(InnermostLoops);

  if (InnermostLoops.empty()) {
    LLVM_DEBUG(dbgs() << HIRF.getFunction().getName()
                      << "() has no Innermost loop\n ");
    return false;
  }
  LLVM_DEBUG(dbgs() << " # Innermost Loops: " << InnermostLoops.size() << "\n");

  bool Result = false;

  // Try to match pattern starting from each Innermost Loop:
  for (HLLoop *InnerLoop : InnermostLoops) {
    Result = doLoopPatternMatch(InnerLoop) || Result;
  }

  return Result;
}

bool HIRSymbolicTripCountCompleteUnroll::doLoopPatternMatch(HLLoop *InnerLp) {
  clearWorkingSetMemory();

  // Analyze the LoopNest for trying to match a given pattern
  if (!doAnalysis(InnerLp)) {
    return false;
  }

  // Do Loop PatterMatch Early Transformation on the matched LoopNest
  doTransform(OuterLp);

  return true;
}

bool HIRSymbolicTripCountCompleteUnroll::doAnalysis(HLLoop *InnerLp) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  // Do preliminary tests, filtering out unsuitable loops as early as possible
  if (!doPreliminaryChecks(InnerLp)) {
    LLVM_DEBUG(FOS << "HIRSymbolicTripCountCompleteUnroll: failed Preliminary "
                      "Checks And Collection\n");
    return false;
  }
  LLVM_DEBUG(dbgs() << "The entire Loop Nest: \n"; OuterLp->dump(););

  // Collect relevant data, populate relevant containers

  if (!doCollection()) {
    LLVM_DEBUG(
        FOS << "HIRSymbolicTripCountCompleteUnroll: failed Collection\n");
    return false;
  }

  // Check pattern:
  if (!isPattern()) {
    LLVM_DEBUG(
        FOS << "HIRSymbolicTripCountCompleteUnroll: failed Pattern Tests\n");
    return false;
  }

  // Do legal tests:
  if (!isLegal()) {
    LLVM_DEBUG(
        FOS << "HIRSymbolicTripCountCompleteUnroll: failed Legal Tests\n");
    return false;
  }

  return true;
}

// Preliminary Tests: Check LoopNest
//  - Test LoopNest:
//    . 2-levels of loop nest;
//    . both OuterLp and InnerLp are normalized;
//
//  - Test InnerLp:
//    . InnerLp has ZTT;
//    . Has 1 HLIF: inside InnerLp;
//    . Has side exit;
//
// Collection:
//    . run StructuralCollector, populate its default data fields;
//    . assign: HLIF0, HLIF1;
//
bool HIRSymbolicTripCountCompleteUnroll::doPreliminaryChecks(
    HLLoop *InnerLoop) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  // Expect the InnerLoop to be on level 2 or 3
  if ((InnerLoop->getNestingLevel() != 2) &&
      (InnerLoop->getNestingLevel() != 3)) {
    return false;
  }

  InnerLp = InnerLoop;
  OuterLp = InnerLp->getParentLoop();
  LLVM_DEBUG(FOS << "OuterLp: "; OuterLp->dump(); FOS << "\n";);

  // Check: expect both OuterLp and InnerLp are normalized
  if (!InnerLp->isNormalized() || !OuterLp->isNormalized()) {
    return false;
  }

  // Check OuterLp:
  if (!doOuterLpTest()) {
    LLVM_DEBUG(
        FOS << "HIRSymbolicTripCountCompleteUnroll: failed OuterLp Test\n");
    return false;
  }

  // Check InnerLp:
  if (!doInnerLpTest()) {
    LLVM_DEBUG(
        FOS << "HIRSymbolicTripCountCompleteUnroll: failed InnerLp Test\n");
    return false;
  }

  return true;
}

// Test OuterLp:
//  - UB: 3 (constant)
//  - TripCount : 4 (precise)
//  - NO ZTT;
//  - NO Preheader, NO Postexit;
//  - NO MultiExits;
//
bool HIRSymbolicTripCountCompleteUnroll::doOuterLpTest(void) {

  // Check: UB is Integer Constant 3
  int64_t UBConst = 0;
  if (!OuterLp->getUpperDDRef()->isIntConstant(&UBConst)) {
    return false;
  }
  if (UBConst != 3) {
    return false;
  }

  // Check: NOT MultiExit
  if (!OuterLp->isDo()) {
    return false;
  }

  // Check: NO Ztt
  if (OuterLp->hasZtt()) {
    return false;
  }

  // Check: NO preheader, NO postexit
  if (OuterLp->hasPreheader() || OuterLp->hasPostexit()) {
    return false;
  }

  return true;
}

// Test InnerLp:
//  - UB: unknown, in a temp non-local to its current level
//  - TripCount : 4 (estimated)
//  - Has 1 HLIF inside it;
//  - No ZTT;
//  - has MultiExits;
//
bool HIRSymbolicTripCountCompleteUnroll::doInnerLpTest(void) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  // Check: UB is unknown (in a temp linear to the loop)
  // RegDDRef *UBRef = InnerLp->getUpperDDRef();
  CanonExpr *UBCE = InnerLp->getUpperDDRef()->getSingleCanonExpr();
  LLVM_DEBUG(FOS << "UBCE: "; UBCE->dump(); FOS << "\n";);

  // Check: NO IV in UBCE
  if (UBCE->hasIV()) {
    return false;
  }

  // Check: Blob of the UBCE has only 1 item
  if (UBCE->numBlobs() != 1) {
    return false;
  }

  // Check: Estimated TripCount is 4
  if (InnerLp->getMaxTripCountEstimate() != 4) {
    return false;
  }

  // Check: NO ZTT
  if (InnerLp->hasZtt()) {
    return false;
  }

  // Check: HAS MultiExits
  if (!InnerLp->isDoMultiExit()) {
    return false;
  }

  return true;
}

bool HIRSymbolicTripCountCompleteUnroll::doCollection(void) {
  // run StructuralCollector over OuterLp:
  SmallVector<HLIf *, 2> HLIfVec;
  StructuralCollector Collector(this, HLIfVec);

  HLNodeUtils::visitRange(Collector, OuterLp->getFirstChild(),
                          OuterLp->getLastChild());
  LLVM_DEBUG(Collector.print(););

  // *** Check the HLIfs ***
  // Expect 2 HLIfs:
  if (HLIfVec.size() != 2) {
    return false;
  }

  // Save into class member variables: HLIF0 and HLIF1
  HLIF0 = HLIfVec[0];
  HLIF1 = HLIfVec[1];

  return true;
}

// Test HLIf0: if (%t38 > 0)
//  .Predicate: Has 1 predicate only
//    -Operand0: a function local variable
//    -Operand1: 0
//    -Compare operator: >
//  .ThenBlock: not empty;
//  .ElesBlock: empty;
//
// Note:
// - Simplified logic to detect EXACT (rather than vague) pattern on HLIF0
//
bool HIRSymbolicTripCountCompleteUnroll::doHLIF0Test(void) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif
  LLVM_DEBUG(FOS << "HLIF0: "; HLIF0->dump(); FOS << "\n";);

  // On Predicate: Has 1 predicate only
  if (HLIF0->getNumPredicates() != 1) {
    return false;
  }

  auto PredI = HLIF0->pred_begin();
  PredicateTy Pred = *PredI;
  bool PredCond =
      (Pred == PredicateTy::ICMP_SGT || Pred == PredicateTy::ICMP_UGT);
  if (!PredCond) {
    return false;
  }

  // Check the predicate's operands are:
  // Operand0: a (function) local variable
  // Operand1: 0 (an integer constant)
  RegDDRef *LHSRef = HLIF0->getPredicateOperandDDRef(PredI, true);
  RegDDRef *RHSRef = HLIF0->getPredicateOperandDDRef(PredI, false);
  // Examine both operands:
  LLVM_DEBUG(FOS << "LHSRef: "; LHSRef->dump(); FOS << "\n";);
  LLVM_DEBUG(FOS << "RHSRef: "; RHSRef->dump(); FOS << "\n";);

  // Check: LHSRef is a temp
  if (!LHSRef->isSelfBlob()) {
    return false;
  }

  // Check: RHSRef is 0
  int64_t IntConst = 0;
  bool IsRHSZero = RHSRef->isIntConstant(&IntConst) && (IntConst == 0);
  if (!IsRHSZero) {
    return false;
  }

  // Check: ThenBlock is not empty
  if (!HLIF0->hasThenChildren()) {
    return false;
  }

  // Check: ElseBlock is empty
  if (HLIF0->hasElseChildren()) {
    return false;
  }

  // Check: InnerLp is inside HLIF0
  if (InnerLp->getParent() != HLIF0) {
    return false;
  }

  return true;
}

// Test HLIf1: if ((%t4)[0].0[i2] == %t48)
// -Inside InnerLp;
// -Overall:
//  .Has ThenBlock, NO ElseBlock;
//
// -Predicate:
//  . operand0: a local memref;
//  . operand1: a temp;
//  . operator: ==
//
// -ThenBlock:
//  . single HLGoto instruction;
//  . Target label outside HLIF1 (inside HLIF0);
//  . NOT empty;
//
// -ElseBlock:
//  . empty;
//
bool HIRSymbolicTripCountCompleteUnroll::doHLIF1Test(void) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif
  LLVM_DEBUG(FOS << "HLIF1: "; HLIF1->dump(); FOS << "\n";);

  // -Predicate:
  //  . operand0: a local memref;
  //  . operand1: a temp;
  //  . operator: ==
  if (HLIF1->getNumPredicates() != 1) {
    return false;
  }

  auto PredI = HLIF1->pred_begin();
  PredicateTy Pred = *PredI;
  if (Pred != PredicateTy::ICMP_EQ) {
    return false;
  }

  RegDDRef *LHSRef = HLIF1->getPredicateOperandDDRef(PredI, true);
  RegDDRef *RHSRef = HLIF1->getPredicateOperandDDRef(PredI, false);
  // Examine both operands:
  LLVM_DEBUG(FOS << "LHSRef: "; LHSRef->dump(); FOS << "\n";);
  LLVM_DEBUG(FOS << "RHSRef: "; RHSRef->dump(); FOS << "\n";);

  // Check: LHSRef is a local memref
  if (!HIRSymbolicTripCountCompleteUnroll::isLocalMemRef(LHSRef)) {
    return false;
  }

  // Check: RHSRef is a temp
  if (!RHSRef->isTerminalRef()) {
    return false;
  }

  // Check: ThenBlock is NOT empty
  if (!HLIF1->hasThenChildren()) {
    return false;
  }

  // Check: ElseBlock is empty
  if (HLIF1->hasElseChildren()) {
    return false;
  }

  // Check: HLIF1 is inside InnerLp
  if (HLIF1->getParent() != InnerLp) {
    return false;
  }

  return true;
}

// Do per-instruction deep pattern match on OuterLp-level instructions,
// including label(s) and goto(s).
//
// Following is the expected complete list of OuterLp HLNode*: 14
// 0.  %t38.out = %t38;
// 1. %t40 = (%0)[0].12.0[i1];
// 2.  %t44 = (%0)[0].10.0[%t40 + %2];
// 3.  (%0)[0].10.0[%t40 + %2] = %t44 + %t35;
// 4.  %t48 = (%0)[0].7.0[%t40 + %2];
// 5.  goto t61;
// 6.  t68:
// 7.  goto t69;
// 8.  t61:
// 9.  %t64 = (%0)[0].8.0[%t48];
// 10. (%0)[0].8.0[%t48] = %t64 + -1;
// 11. %t38 = %t38  +  1;
// 12. (%t4)[0].0[%t38.out] = %t48;
// 13. t69:
//
bool HIRSymbolicTripCountCompleteUnroll::doDeepPatternTestOuterLp(void) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif
  // LLVM_DEBUG(FOS << "OuterLpNodeVec: \n"; print(OuterLpNodeVec););

  // Expect a total of 14 HLNode* in OuterLp-only level:
  if (OuterLpNodeVec.size() != 14) {
    return false;
  }

  // 0. %t38.out = %t38;
  // Check: CopyInst, Lval is a temp, Rval is a temp;
  HLInst *HInst = dyn_cast<HLInst>(OuterLpNodeVec[0]);
  if (!HInst || !HInst->isCopyInst() ||
      !HInst->getLvalDDRef()->isTerminalRef() ||
      !HInst->getRvalDDRef()->isTerminalRef()) {
    return false;
  }

  int64_t IntConst = 0;
  RegDDRef *LvalRef = nullptr, *RvalRef = nullptr;

  // 1. %t40 = (%0)[0].12.0[i1];
  // Check:
  // - LoadInst
  // - Lval is a temp
  // - Rval is a 2-Dimensional non-local memref, Dim1 has IV, Dim2 is a
  // const 0
  HInst = dyn_cast<HLInst>(OuterLpNodeVec[1]);
  if (!HInst) {
    return false;
  }
  RvalRef = HInst->getRvalDDRef();
  if (!isa<LoadInst>(HInst->getLLVMInstruction()) ||
      !HInst->getLvalDDRef()->isTerminalRef() ||
      !HIRSymbolicTripCountCompleteUnroll::isNonLocalMemRef(RvalRef)) {
    return false;
  }
  if (RvalRef->getNumDimensions() != 2) {
    return false;
  }

  CanonExpr *CE = RvalRef->getDimensionIndex(1);
  if (!CE || !CE->isStandAloneIV(false)) {
    return false;
  }

  if (!isCanonExprConstVal(RvalRef->getDimensionIndex(2), 0)) {
    return false;
  }

  // 2. %t44 = (%0)[0].10.0[%t40 + %2];
  // Check:
  // - LoadInst
  // - Lval is a temp
  // - Rval is a 2-Dimensional non-local memref, Dim1 has NO IV, Dim2 is a
  //   const 0
  HInst = dyn_cast<HLInst>(OuterLpNodeVec[2]);
  if (!HInst) {
    return false;
  }
  RvalRef = HInst->getRvalDDRef();
  if (!isa<LoadInst>(HInst->getLLVMInstruction()) ||
      !HInst->getLvalDDRef()->isTerminalRef() ||
      !HIRSymbolicTripCountCompleteUnroll::isNonLocalMemRef(RvalRef)) {
    return false;
  }

  if (RvalRef->getNumDimensions() != 2) {
    return false;
  }

  CE = RvalRef->getDimensionIndex(1);
  if (!CE || CE->isStandAloneIV(false)) {
    return false;
  }

  if (!isCanonExprConstVal(RvalRef->getDimensionIndex(2), 0)) {
    return false;
  }

  // 3. (%0)[0].10.0[%t40 + %2] = %t44 + %t35;
  // Check:
  // - StoreInst
  // - Lval is a 2-Dimensional non-local memref, Dim1 has IV, Dim2 is a const 0
  //  . [note]: Lval here is the same as Rval in previous instruction
  // - Rval is a terminalRef
  HInst = dyn_cast<HLInst>(OuterLpNodeVec[3]);
  if (!HInst) {
    return false;
  }
  LvalRef = HInst->getLvalDDRef();
  if (!isa<StoreInst>(HInst->getLLVMInstruction()) ||
      !HIRSymbolicTripCountCompleteUnroll::isNonLocalMemRef(LvalRef)) {
    return false;
  }
  if (LvalRef->getNumDimensions() != 2) {
    return false;
  }

  CE = LvalRef->getDimensionIndex(1);
  if (!CE || CE->isStandAloneIV(false)) {
    return false;
  }

  if (!isCanonExprConstVal(LvalRef->getDimensionIndex(2), 0)) {
    return false;
  }
  //  Special check: Lval here is the same as Rval in the previous instruction
  if (!DDRefUtils::areEqual(LvalRef, RvalRef)) {
    return false;
  }

  RvalRef = HInst->getRvalDDRef();
  if (!RvalRef->isTerminalRef()) {
    return false;
  }

  if (RvalRef->hasIV(OuterLp->getNestingLevel())) {
    return false;
  }

  // 4. %t48 = (%0)[0].7.0[%t40 + %2];
  // Check:
  // - LoadInst
  // - Lval is a temp
  // - Rval is a 2-Dimensional non-local memref, Dim1 has NO IV, Dim2 is a
  //   const 0
  HInst = dyn_cast<HLInst>(OuterLpNodeVec[4]);
  if (!HInst) {
    return false;
  }
  RvalRef = HInst->getRvalDDRef();
  if (!isa<LoadInst>(HInst->getLLVMInstruction()) ||
      !HInst->getLvalDDRef()->isTerminalRef() ||
      !HIRSymbolicTripCountCompleteUnroll::isNonLocalMemRef(RvalRef)) {
    return false;
  }

  if (RvalRef->getNumDimensions() != 2) {
    return false;
  }

  CE = RvalRef->getDimensionIndex(1);
  if (!CE || RvalRef->getDimensionIndex(1)->hasIV()) {
    return false;
  }

  if (!isCanonExprConstVal(RvalRef->getDimensionIndex(2), 0)) {
    return false;
  }

  // 5. goto t61;
  HLGoto *Goto = dyn_cast<HLGoto>(OuterLpNodeVec[5]);
  if (!Goto) {
    return false;
  }
  HLLabel *Label = Goto->getTargetLabel();
  if (!Label) {
    return false;
  }
  // if.then18's parent is inside OuterLp
  if (Label->getParent() != OuterLp) {
    return false;
  }

  // 6. t68:
  Label = dyn_cast<HLLabel>(OuterLpNodeVec[6]);
  if (!Label) {
    return false;
  }

  // 7. goto t69;
  Goto = dyn_cast<HLGoto>(OuterLpNodeVec[7]);
  if (!Goto) {
    return false;
  }
  Label = Goto->getTargetLabel();
  if (!Label) {
    return false;
  }
  // if.end31's parent is inside OuterLp
  if (Label->getParent() != OuterLp) {
    return false;
  }

  // 8. t61:
  Label = dyn_cast<HLLabel>(OuterLpNodeVec[8]);
  if (!Label) {
    return false;
  }

  // 9. %t64 = (%0)[0].8.0[%t48];
  // Check:
  // - LoadInst
  // - Lval is a temp
  // - Rval is a 2-Dimensional non-local memref, Dim1 has NO IV, Dim2 is a
  //   const 0
  HInst = dyn_cast<HLInst>(OuterLpNodeVec[9]);
  if (!HInst) {
    return false;
  }
  RvalRef = HInst->getRvalDDRef();
  if (!isa<LoadInst>(HInst->getLLVMInstruction()) ||
      !HInst->getLvalDDRef()->isTerminalRef() ||
      !HIRSymbolicTripCountCompleteUnroll::isNonLocalMemRef(RvalRef)) {
    return false;
  }

  if (RvalRef->getNumDimensions() != 2) {
    return false;
  }

  CE = RvalRef->getDimensionIndex(1);
  if (CE->hasIV()) {
    return false;
  }
  if (!isCanonExprConstVal(RvalRef->getDimensionIndex(2), 0)) {
    return false;
  }

  // 10. (%0)[0].8.0[%t48] = %t64 + -1;
  // Check:
  // - StoreInst
  // - Lval is a 2-Dimensional non-local memref, Dim1 has NO IV, Dim2 is a
  //   const 0
  // - Rval is a CanonExpr* without IV
  HInst = dyn_cast<HLInst>(OuterLpNodeVec[10]);
  if (!HInst) {
    return false;
  }
  LvalRef = HInst->getLvalDDRef();
  if (!isa<StoreInst>(HInst->getLLVMInstruction()) ||
      !HIRSymbolicTripCountCompleteUnroll::isNonLocalMemRef(LvalRef)) {
    return false;
  }

  if (LvalRef->getNumDimensions() != 2) {
    return false;
  }

  CE = LvalRef->getDimensionIndex(1);
  if (!CE || CE->hasIV()) {
    return false;
  }
  if (!isCanonExprConstVal(LvalRef->getDimensionIndex(2), 0)) {
    return false;
  }

  RvalRef = HInst->getRvalDDRef();
  if (!RvalRef || RvalRef->getNumDimensions() != 1) {
    return false;
  }

  for (auto I = RvalRef->canon_begin(), E = RvalRef->canon_end(); I != E; ++I) {
    CE = (*I);
    if (CE->hasIV()) {
      return false;
    }
  }

  // 11. %t38 = %t38  +  1;
  // Check:
  // - BinaryOp (+) instruction:
  // - Operand0: a temp;
  // - Operand1: the same temp;
  // - Operand2: a constant integer (1)
  HInst = dyn_cast<HLInst>(OuterLpNodeVec[11]);
  if (!HInst || !isa<BinaryOperator>(HInst->getLLVMInstruction()) ||
      !HInst->getOperandDDRef(0)->isTerminalRef()) {
    return false;
  }

  auto BOp = dyn_cast<BinaryOperator>(HInst->getLLVMInstruction());
  if (!BOp || BOp->getOpcode() != Instruction::Add) {
    return false;
  }
  if (HInst->getNumOperands() != 3) {
    return false;
  }

  // Check: operand0
  RegDDRef *Op0 = HInst->getOperandDDRef(0);
  if (!Op0->isTerminalRef()) {
    return false;
  }

  // Check: operand1
  RegDDRef *Op1 = HInst->getOperandDDRef(1);
  if (!Op1->isTerminalRef()) {
    return false;
  }
  if (!DDRefUtils::areEqual(Op0, Op1)) {
    return false;
  }

  // Check: operand2
  RegDDRef *Op2 = HInst->getOperandDDRef(2);
  if (!Op2->isIntConstant(&IntConst) || (IntConst != 1)) {
    return false;
  }

  // 12. (%t4)[0].0[%t38.out] = %t48;
  // Check:
  // - StoreInst
  // - Lval is a 2-Dimensional local memref, Dim1 has NO IV, Dim2 is a
  //   const 0
  // - Rval is a temp
  HInst = dyn_cast<HLInst>(OuterLpNodeVec[12]);
  if (!HInst) {
    return false;
  }
  LvalRef = HInst->getLvalDDRef();
  if (!isa<StoreInst>(HInst->getLLVMInstruction()) ||
      !HIRSymbolicTripCountCompleteUnroll::isLocalMemRef(LvalRef) ||
      !HInst->getRvalDDRef()->isTerminalRef()) {
    return false;
  }
  if (LvalRef->getNumDimensions() != 2) {
    return false;
  }

  CE = LvalRef->getDimensionIndex(1);
  if (!CE || !CE->isStandAloneBlob(true) || CE->hasIV()) {
    return false;
  }

  if (!isCanonExprConstVal(LvalRef->getDimensionIndex(2), 0)) {
    return false;
  }

  // 13. t69:
  Label = dyn_cast<HLLabel>(OuterLpNodeVec[13]);
  if (!Label) {
    return false;
  }

  // Check: does OuterLp have any LiveOut temp?
  // Expect: OuterLp has no LiveOut
  if (OuterLp->hasLiveOutTemps()) {
    LLVM_DEBUG(FOS << "Failed OuterLp's LiveOutTemp test\n";);
    return false;
  }

  return true;
}

// List of instructions in the InnerLp:
// 0. goto t68;
//
bool HIRSymbolicTripCountCompleteUnroll::doDeepPatternTestInnerLp(void) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif
  LLVM_DEBUG(print(InnerLpNodeVec));

  // Expect a total of 1 instruction in InnerLp-only level:
  if (InnerLpNodeVec.size() != 1) {
    return false;
  }
  // 0. goto t68;
  HLGoto *Goto = dyn_cast<HLGoto>(InnerLpNodeVec[0]);
  if (!Goto) {
    assert(0 && "Expect a valid entry on InnerLpNodeVec[0]\n");
    return false;
  }
  // LLVM_DEBUG(Goto->dump(); FOS << "\n";);

  // Check: the Goto's target is inside HLIF0;
  HLLabel *Label = Goto->getTargetLabel();
  if (!Label) {
    assert(0 && "Expect a valid target Label\n");
    return false;
  }

  if (Label->getParent() != HLIF0) {
    return false;
  }

  return true;
}

// Note:
//- this function happens after the collection, since it uses HLIF0 and HLIF1.
bool HIRSymbolicTripCountCompleteUnroll::isPattern(void) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  if (!doHLIF0Test()) {
    LLVM_DEBUG(
        FOS << "HIRSymbolicTripCountCompleteUnroll: failed HLIf0 Test\n");
    return false;
  }

  if (!doHLIF1Test()) {
    LLVM_DEBUG(
        FOS << "HIRSymbolicTripCountCompleteUnroll: failed HLIf1 Test\n");
    return false;
  }

  if (!doDeepPatternTestOuterLp()) {
    LLVM_DEBUG(FOS << "HIRSymbolicTripCountCompleteUnroll: failed "
                      "DeepPatternTestOuterLp\n");
    return false;
  }

  if (!doDeepPatternTestInnerLp()) {
    LLVM_DEBUG(FOS << "HIRSymbolicTripCountCompleteUnroll: failed "
                      "DeepPatternTestInnerLp\n");
    return false;
  }

  return true;
}

bool HIRSymbolicTripCountCompleteUnroll::isLegal(void) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  // Check: m_parent[ai] is READONLY (each ref appears as a Rval)
  if (!isMParentReadOnly()) {
    LLVM_DEBUG(FOS << "Failed MParent READ-ONLY test\n";);
    return false;
  }

  // Check: m_parent[.] and m_libs[.] refs are not aliased to each other
  if (!checkMParentAndMLibs()) {
    LLVM_DEBUG(FOS << "Failed MParentAndMLibs test\n";);
    return false;
  }

  return true;
}
// Check: all m_parent[.] ref is READONLY (Rval)
bool HIRSymbolicTripCountCompleteUnroll::isMParentReadOnly(void) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  LLVM_DEBUG(print(NonLocalRefVec););
  // 0. (%0)[0].12.0[i1]
  // 1. (%0)[0].10.0[%t40 + %2]
  // 2. (%0)[0].10.0[%t40 + %2]
  // 3. (%0)[0].7.0[%t40 + %2]
  // 4. (%0)[0].8.0[%t48]
  // 5. (%0)[0].8.0[%t48]
  // Note:
  // m_parent    <-> (%0)[0].07.0
  // The 3rd index Ref* in NonLocalRefVec[] is a reference to m_parent[.]
  // unsigned Size = NonLocalRefVec.size();
  RegDDRef *MParentRef = NonLocalRefVec[3];
  LLVM_DEBUG(MParentRef->dump(); FOS << "\n";);

  // Collect all m_parent[.] Refs
  for (auto Ref : NonLocalRefVec) {
    if (DDRefUtils::areEqual(Ref, MParentRef)) {
      MParentRefVec.push_back(Ref);
    }
  }

  LLVM_DEBUG(print(MParentRefVec););
  // MParentRefVec:2
  // 3: (%0)[0].7.0[%t40 + %2]

  // Check: each m_parent[.] ref is Rval.
  for (auto Ref : MParentRefVec) {
    if (Ref->isLval()) {
      return false;
    }
  }

  return true;
}

// Check: m_parent[] and m_libs[] refs are NOT aliased to each other.
//
// LLVM IR Mapping:
// m_libs      <-> (%0)[0].08.0
// m_parent    <-> (%0)[0].07.0
// - there should not be any FLOW or ANTI edges between m_parent[.] and
// m_libs[.]
//
bool HIRSymbolicTripCountCompleteUnroll::checkMParentAndMLibs(void) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  LLVM_DEBUG(print(NonLocalRefVec););
  // NonLocalRefVec<7>:
  // 0. (%0)[0].12.0[i1]
  // 1. (%0)[0].10.0[%t40 + %2]
  // 2. (%0)[0].10.0[%t40 + %2]
  // 3. (%0)[0].7.0[%t40 + %2]
  // 4. (%0)[0].8.0[%t48]
  // 5. (%0)[0].8.0[%t48]

  // m_libs[%t48]    <-> (%0)[0].8.0[%t48]
  // The last-2 index Ref* in NonLocalRefVec is a reference to m_libs[.]
  unsigned Size = NonLocalRefVec.size();
  RegDDRef *MLibsRef = NonLocalRefVec[Size - 2];
  LLVM_DEBUG(MLibsRef->dump(); FOS << "\n";);

  // Collect: all m_libs[.] Refs
  for (auto Ref : NonLocalRefVec) {
    if (DDRefUtils::areEqual(Ref, MLibsRef)) {
      MLibsRefVec.push_back(Ref);
    }
  }

  LLVM_DEBUG(print(MParentRefVec););
  // MParentRefVec:2
  // 3: (%0)[0].7.0[%40 + %2]

  LLVM_DEBUG(print(MLibsRefVec););
  // MLibs RefVec:2
  // 4: (%0)[0].8.0[%48]
  // 5: (%0)[0].8.0[%48]

  DDGraph DDG = HDDA.getGraph(OuterLp);
  // LLVM_DEBUG(DDG.dump(););

  // Check: each Ref in MParentRefVec vs. MLibsRefV
  for (auto Ref : MParentRefVec) {
    if (!checkExclusiveEdge(Ref, MLibsRefVec, DDG)) {
      return false;
    }
  }

  // Check: each Ref in MLibsRefVec vs. MParentRefV
  for (auto Ref : MLibsRefVec) {
    if (!checkExclusiveEdge(Ref, MParentRefVec, DDG)) {
      return false;
    }
  }

  return true;
}

// Check:
// any edge with one end in Ref, is the other end (OtherRef) in the RefV?
bool HIRSymbolicTripCountCompleteUnroll::checkExclusiveEdge(
    RegDDRef *Ref, SmallVectorImpl<RegDDRef *> &RefV, DDGraph &DDG) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  bool IsLoad = Ref->isRval();
  DDRef *OtherRef = nullptr;

  LLVM_DEBUG(FOS << "Ref: "; Ref->dump(); FOS << "\n";);
  LLVM_DEBUG(print(RefV););

  // Iterate over each relevant DDEdge
  for (const DDEdge *Edge : (IsLoad ? DDG.incoming(Ref) : DDG.outgoing(Ref))) {
    LLVM_DEBUG(Edge->print(dbgs()););

    // Get OtherRef:
    if (IsLoad) {
      OtherRef = Edge->getSrc();
    } else {
      OtherRef = Edge->getSink();
    }

    // Is OtherRef in RefV?
    auto It = std::find(RefV.begin(), RefV.end(), OtherRef);
    if (It != RefV.end()) {
      return false;
    }
  }

  return true;
}

bool HIRSymbolicTripCountCompleteUnroll::doTransform(HLLoop *OuterLp) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  HLRegion *Region = OuterLp->getParentRegion();
  assert(Region && "Region can't be a nullptr\n");
  LLVM_DEBUG(FOS << "BEFORE SymbolicTripCountCompleteUnroll Pattern Match "
                    "CodeGen:\n";
             Region->dump(); FOS << "\n";);

  // Remove HLIF0:
  HLNodeUtils::remove(HLIF0);

  // Clean OuterLp's body, by:
  // -remove any HLLabel/HLGoto;
  // -remove any instruction with load from/store to local data;
  // -remove ONE dead load on non-local data;
  cleanOuterLpBody();

  // 3 transformation actions:
  // - Do complete unroll of the OuterLp;
  // - Do temporary variable renaming, for each temp defined within;
  // - Do partial Code Scheduling: Sink the last Store, together;
  doUnrollActions();

  ++NumHIRSymbolicTripCountCompleteUnroll;

  // Mark CodeGen for parent region:
  Region->setGenCode();
  HIRInvalidationUtils::invalidateNonLoopRegion(Region);

  LLVM_DEBUG(FOS << "AFTER SymbolicTripCountCompleteUnroll Pattern Match:\n";
             Region->dump(); FOS << "\n";);

  return true;
}

// Inside OuterLp's body:
// 1. Remove any HLLabel/HLGoto;
// 2. Remove any instruction that load from/store to local data;
// 3. Cleanup an extra Dead Load on a non-local MemRef array that has NO use;
//
// [BEFORE cleanOuterLpBody()]
// <145>     + DO i1 = 0, 3, 1   <DO_LOOP>
// <2>       |   %t38.out = %t38;
// <4>       |   %t40 = (%0)[0].12.0[i1];
// <8>       |   %t44 = (%0)[0].10.0[%t40 + %2];
// <10>      |   (%0)[0].10.0[%t40 + %2] = %t44 + %t35;
// <13>      |   %t48 = (%0)[0].7.0[%t40 + %2];
// <40>      |   t61:
// <43>      |   %t64 = (%0)[0].8.0[%t48];
// <45>      |   (%0)[0].8.0[%t48] = %t64 + -1;
// <46>      |   %t38 = %t38  +  1;
// <48>      |   (%t4)[0].0[%t38.out] = %t48;
// <50>      |   t69:
// <145>     + END LOOP
//
// [AFTER removal of Load/Store on local data in OuterLp]:
// <145>     + DO i1 = 0, 3, 1   <DO_LOOP>
// <4>       |   %t40 = (%0)[0].12.0[i1];
// <8>       |   %t44 = (%0)[0].10.0[%t40 + %2];
// <10>      |   (%0)[0].10.0[%t40 + %2] = %t44 + %t35;
// <13>      |   %t48 = (%0)[0].7.0[%t40 + %2];
// <43>      |   %t64 = (%0)[0].8.0[%t48];
// <45>      |   (%0)[0].8.0[%t48] = %t64 + -1;
// <145>     + END LOOP
//
void HIRSymbolicTripCountCompleteUnroll::cleanOuterLpBody(void) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  LLVM_DEBUG(FOS << "BEFORE cleanOuterLpBody:\n"; OuterLp->dump();
             FOS << "\n";);

  // ** Remove any HLLabel* and/or HLGoto* in OuterLp **
  for (auto *Label : OuterLpLabelVec) {
    HLNodeUtils::remove(Label);
  }

  for (auto *Goto : OuterLpGotoVec) {
    HLNodeUtils::remove(Goto);
  }

  LLVM_DEBUG(FOS << "AFTER removal of any Label/Goto in OuterLp:\n";
             OuterLp->dump(); FOS << "\n";);

  // ** Remove any HLInst* that has load from/store to local data **
  for (HLInst *Inst : OuterLpInstVec) {
    if (HIRSymbolicTripCountCompleteUnroll::hasLocalLoadOrStore(Inst)) {
      HLNodeUtils::remove(Inst);
    }
  }

  LLVM_DEBUG(FOS << "AFTER removal of Load/Store on local data in OuterLp:\n";
             OuterLp->dump(); FOS << "\n";);
}

void HIRSymbolicTripCountCompleteUnroll::fixLoopIvToConst(HLContainerTy &V,
                                                          unsigned LoopLevel,
                                                          unsigned IVConst) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif
  // LLVM_DEBUG(print(V););

  for (auto I = V.begin(), E = V.end(); I != E; ++I) {
    HLInst *HInst = dyn_cast<HLInst>(I);
    assert(HInst && "Expect HLInst* only\n");
    // LLVM_DEBUG(HInst->dump(); FOS << "\n";);

    // Replace each loop-level IV with the given integer IVConst:
    for (auto I = HInst->ddref_begin(), E = HInst->ddref_end(); I != E; ++I) {
      RegDDRef *Ref = (*I);
      Ref->replaceIVByConstant(LoopLevel, IVConst);
      Ref->makeConsistent({}, LoopLevel - 1);
    }
  }
}

// E.g. [Input code]
//
// 1. %t40 = (%0)[0].12.0[i1];
// 2. %t44 = (%0)[0].10.0[%t40 + %2];
// 3. (%0)[0].10.0[%t40 + %2] = %t44 + %t35;
// 4. %t48 = (%0)[0].7.0[%t40 + %2];
// 5. %t64 = (%0)[0].8.0[%t48];
// 6. (%0)[0].8.0[%t48] = %t64 + -1;
//
// Given the above code, all explicit def-use chain(s) on TempDDRefs are:
//      def    use(s)
//%t40  <1>    <2,3,4>
//%t44  <2>    <3>
//%t48  <4>    <5,6>
//%t64  <5>    <6>
//
// All TempDefs are: {%t40, %t44, %t48, %t64}
//
void HIRSymbolicTripCountCompleteUnroll::collectTempDefition(
    HLContainerTy &V, SmallVectorImpl<RegDDRef *> &DefVec) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif
  // LLVM_DEBUG(print(V););

  for (auto I = V.begin(), E = V.end(); I != E; ++I) {
    HLInst *HInst = cast<HLInst>(I);
    // LLVM_DEBUG(HInst->dump(); FOS << "\n";);

    // Collect all Temp Definitions on Lval:
    RegDDRef *Lval = HInst->getOperandDDRef(0);
    if (Lval->isSelfBlob()) {
      DefVec.push_back(Lval);
    }
  }

  // LLVM_DEBUG(dbgs() << "DefVec: "; print(DefVec););
}

//[Original code]
// 1. %t40 = (%0)[0].12.0[i1];
// 2. %t44 = (%0)[0].10.0[%t40 + %2];
// 3. (%0)[0].10.0[%t40 + %2] = %t44 + %t35;
// 4. %t48 = (%0)[0].7.0[%t40 + %2];
// 5. %t64 = (%0)[0].8.0[%t48];
// 6. (%0)[0].8.0[%t48] = %t64 + -1;
//
//[All explicit Non-overwrite definitions, with the mapping generated]
// ----------------------
// |OrigDef  | MappedDef |
// ----------------------|
// | %t40    |  %mv      |
// | %t44    |  %mv4     |
// | %t48    |  %mv5     |
// | %t64    |  %mv6     |
// ----------------------
//
//[The definition-mapped code for 2nd iteration]
// 0: %mv = (%0)[0].12.0[1];
// 1: %mv4 = (%0)[0].10.0[%2 + %mv];
// 2: (%0)[0].10.0[%2 + %mv] = %t35 + %mv4;
// 3: %mv5 = (%0)[0].7.0[%2 + %mv];
// 4: %mv6 = (%0)[0].8.0[%mv5];
// 5: (%0)[0].8.0[%mv5] = %mv6 + -1;
//
void HIRSymbolicTripCountCompleteUnroll::buildTempDefMap(
    SmallVectorImpl<RegDDRef *> &DefVec,
    DenseMap<RegDDRef *, RegDDRef *> &DefMap) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif
  // LLVM_DEBUG(print(V););

  // Build a Map between OLD TempDef and NEW TempDef:
  // (Note: use explicit clone on OrigDef)
  for (auto Def : DefVec) {
    RegDDRef *OldDefClone = Def->clone();
    RegDDRef *NewDef = HNU.createTemp(Def->getDestType(), TempName);
    DefMap[OldDefClone] = NewDef;
  }

  // Examine the map we just created:
  // LLVM_DEBUG(dbgs() << " DefMap: "; print(DefMap););
}

void HIRSymbolicTripCountCompleteUnroll::updateTempUse(
    HLContainerTy &V, SmallVectorImpl<RegDDRef *> &DefVec,
    DenseMap<RegDDRef *, RegDDRef *> &DefMap) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif
  // LLVM_DEBUG(print(V););

  for (auto I = V.begin(), E = V.end(); I != E; ++I) {
    HLInst *HInst = cast<HLInst>(I);
    // LLVM_DEBUG(HInst->dump(); FOS << "\n";);

    for (auto Item : DefMap) {
      RegDDRef *OldDef = Item.first;
      RegDDRef *NewDef = Item.second;
      unsigned OldIndex = OldDef->getSelfBlobIndex();
      unsigned NewIndex = NewDef->getSelfBlobIndex();

      LLVM_DEBUG(FOS << "OldDef: "; OldDef->dump(); FOS << "\t";
                 FOS << "OldIndex: " << OldIndex << "\t"; FOS << "NewDef: ";
                 NewDef->dump(); FOS << "\t"; FOS << "NewIndex: " << NewIndex;
                 FOS << "\n";);

      // For each operand in HLInst*: Replace OldRef with NewRef using Index
      for (unsigned I = 0, E = HInst->getNumOperands(); I < E; ++I) {
        RegDDRef *Ref = HInst->getOperandDDRef(I);
        Ref->replaceTempBlob(OldIndex, NewIndex);
      }
    }
  }
}

// *** DoUnrollActions: 3 STEPS ***
// 1. Completely unroll the OuterLp;
// 2. Do Temp's Definition Renaming;
// 3. Code Scheduling by Sinking the Last Store from each unrolled iteration
//    to right BEFORE the LastStoreMarker;
//
//[BEFORE complete Unroll and Scheduled for OuterLp]
//<61>  + DO i1 = 0, 3, 1   <DO_LOOP>
//<4>   |   %t40 = (%0)[0].12.0[i1]; //load: ai = m_dirs[k]
//<8>   |   %t44 = (%0)[0].10.0[%t40 + %2]; //load: m_neighbour[ai+i]
//<12>  |   (%0)[0].10.0[%t40 + %2] = %t44 + %35;
//                                   //store: m_neighbour[ai] += %t35
//<15>  |   %t48 = (%0)[0].7.0[%t40 + %2];//load: m_parent[ai+i]
//<45>  |   %t64 = (%0)[0].8.0[%t48];     //load: m_libs[m_parent[ai+i]]
//<47>  |   (%0)[0].8.0[%48] = %64 + -1;//store: m_libs[m_parent[ai+i]] += -1
//<61>  + END LOOP
//
// Piggy-back to C source level:
// for(int k = 0; k<4; ++k){
//  ai = m_dirs[k];                 <61>
//  m_neighbour[ai] += SHL + 65280; <4> <8><12>
//  int val =m_libs[m_parent[ai+i]]+ -1;
//  m_libs[m_parent[ai+i]] = val;   <15><45><47>
//}
//
//[AFTER complete Unroll of OuterLp]
//(k=0):
// ai0 = m_dirs[0];
// m_neighbour[ai0] += SHL + 65280;
// val0 =m_libs[m_parent[ai0+i]]+ -1;
// m_libs[m_parent[ai0+i]] = val0;
//
//(k=1):
// ai0 = m_dirs[1];
// m_neighbour[ai0] += SHL + 65280;
// val0 =m_libs[m_parent[ai0+i]]+ -1;
// m_libs[m_parent[ai0+i]] = val0;
//
//(k=2):
// ai0 = m_dirs[2];
// m_neighbour[ai0] += SHL + 65280;
// val0 =m_libs[m_parent[ai0+i]]+ -1;
// m_libs[m_parent[ai0+i]] = val0;
//
//(k=3):
// ai0 = m_dirs[3];
// m_neighbour[ai0] += SHL + 65280;
// val0 =m_libs[m_parent[ai0+i]]+ -1;
// m_libs[m_parent[ai0+i]] = val0;
//
//
//[AFTER complete Unroll of OuterLp + Temp Renaming]:
//
//(k=0):
// ai0 = m_dirs[0];
// m_neighbour[ai0] += SHL + 65280;
// val0 =m_libs[m_parent[ai0+i]]+ -1;
// m_libs[m_parent[ai0+i]] = val0;
//
//(k=1):
// ai1 = m_dirs[1];
// m_neighbour[ai1] += SHL + 65280;
// val1 =m_libs[m_parent[ai1+i]]+ -1;
// m_libs[m_parent[ai1+i]] = val1;
//
//(k=2):
// ai2 = m_dirs[2];
// m_neighbour[ai2] += SHL + 65280;
// val2 =m_libs[m_parent[ai2+i]]+ -1;
// m_libs[m_parent[ai2+i]] = val2;
//
//(k=3):
// ai3 = m_dirs[3];
// m_neighbour[ai3] += SHL + 65280;
// val3 =m_libs[m_parent[ai3+i]]+ -1;
// m_libs[m_parent[ai3+i]] = val3;
//
//
//[AFTER complete Unroll of the OuterLp + Temp renaming, + Sink the
// -per-iteration last-store instructions together]:
//
//(k=0):
// ai0 = m_dirs[0];
// m_neighbour[ai0] += SHL + 65280;
// val0 =m_libs[m_parent[ai0+i]]+ -1;
//
//(k=1):
// ai1 = m_dirs[1];
// m_neighbour[ai1] += SHL + 65280;
// val1 =m_libs[m_parent[ai1+i]]+ -1;
//
//(k=2):
// ai2 = m_dirs[2];
// m_neighbour[ai2] += SHL + 65280;
// val2 =m_libs[m_parent[ai2+i]]+ -1;
//
//(k=3):
// ai3 = m_dirs[3];
// m_neighbour[ai3] += SHL + 65280;
// val3 =m_libs[m_parent[ai3+i]]+ -1;
//
//(Store-Sink section):
// m_libs[m_parent[ai0+i]] = val0;
// m_libs[m_parent[ai1+i]] = val1;
// m_libs[m_parent[ai2+i]] = val2;
// m_libs[m_parent[ai3+i]] = val3;
//
void HIRSymbolicTripCountCompleteUnroll::doUnrollActions(void) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif
  HLRegion *Region = OuterLp->getParentRegion();
  LLVM_DEBUG(FOS << "BEFORE doUnrollActions():\n"; Region->dump();
             FOS << "\n";);

  const unsigned OuterLpLevel = OuterLp->getNestingLevel();
  HLContainerTy LoopBody; // Container for cloning loop body per iteration
  HLNodeUtils &HNU = OuterLp->getHLNodeUtils();

  auto OrigFirstChild = OuterLp->getFirstChild(),
       OrigLastChild = OuterLp->getLastChild();
  bool IsFirstIter = false, IsLastIter = false;

  // Prepare for manual unrolling:
  // OuterLp->extractPreheaderAndPostexit();
  HLNode *Marker = HNU.getOrCreateMarkerNode();

  // Replace OuterLp with marker node:
  HLNodeUtils::replace(OuterLp, Marker);

  // Vector of only the Last Instruction per unrolled body:
  SmallVector<HLDDNode *, 4> LastInstVec;
  HLDDNode *LastInstMarker = nullptr;

  // Completely unroll OuterLp's body, fix iv within [0..3] (4 times):
  for (int64_t I = 0; I <= 3; ++I) {
    IsFirstIter = (I == 0), IsLastIter = (I == 3);

    // Do complete unroll for current iteration, fix iv to I

    // Clone iteration:
    HLNodeUtils::cloneSequence(&LoopBody, OrigFirstChild, OrigLastChild);
    LLVM_DEBUG(FOS << "LoopBody without IV fixed:\n"; print(LoopBody););

    fixLoopIvToConst(LoopBody, OuterLpLevel, I);

    LLVM_DEBUG(FOS << "LoopBody with IV fixed:\n"; print(LoopBody););

    // For any non-last Iteration unroll: collect the last Instruction node
    // (for the final Store-Sink step)
    HLInst *LastInst = dyn_cast<HLInst>(--LoopBody.end());
    assert(LastInst && "Expect HLInst* only\n");
    LLVM_DEBUG(FOS << "LastInst:"; LastInst->dump(););
    if (!IsLastIter) {
      LastInstVec.push_back(LastInst);
    } else {
      // VERY LAST instruction of the last unrolled body
      // (will Sink Stores BEFORE it)
      LastInstMarker = LastInst;
    }

    // Do Temp-Definition Renaming over unrolled body for any non-1st
    // iteration
    //
    // Note:
    // No need to rename temp definition(s) for the 1st iteration, if all
    // other iterations are renamed.
    //
    if (!IsFirstIter) {
      SmallVector<RegDDRef *, 8> TmpDefVec;    // Temp's Definitions
      DenseMap<RegDDRef *, RegDDRef *> DefMap; // Mapping: OLDDef->NEWDef

      // Collect all Tmp's definitions: into TmpDefVec
      collectTempDefition(LoopBody, TmpDefVec);
      LLVM_DEBUG(FOS << "TmpDefVec:\n"; print(TmpDefVec););

      // Build Mapping between OldDef and NewDef: update DefMap
      buildTempDefMap(TmpDefVec, DefMap);
      LLVM_DEBUG(FOS << "DefMap:\n"; print(DefMap););

      // Update use of OLDTmp to NEWTmp:
      updateTempUse(LoopBody, TmpDefVec, DefMap);

      // Examine AFTER map OldTmp to NewTmp in LoopBody:
      LLVM_DEBUG(FOS << "After map OldTmp->NewTmp in LoopBody:\n";
                 print(LoopBody););
    }

    // Insert the per-iteration unrolled code at the end of the OuterLp
    // Note: LoopBody is cleaned after each insert
    HLNodeUtils::insertAfter(OuterLp->getLastChild(), &LoopBody);
    LLVM_DEBUG(OuterLp->dump(););
  }

  // *** Do Code Sinking ***
  // Sink each collected last store to BEFORE LastStoreMarker
  LLVM_DEBUG(print(LastInstVec));
  for (auto Inst : LastInstVec) {
    HLNodeUtils::moveBefore(LastInstMarker, Inst);
  }
  LLVM_DEBUG(OuterLp->dump(););

  // Remove the Original Nodes in OuterLp:
  HLNodeUtils::remove(OrigFirstChild, OrigLastChild);
  LLVM_DEBUG(
      FOS << "Completely Unrolled OuterLp (without those original nodes):\n";
      OuterLp->dump(); FOS << "\n";);

  // Replace marker node with the unrolled loop body, remove the OuterLp
  // (Straight-line code is now produced!)
  HLNodeUtils::moveBefore(Marker, OuterLp->child_begin(), OuterLp->child_end());
  HLNodeUtils::remove(Marker);

  LLVM_DEBUG(FOS << "AFTER doUnrollActions(.):\n"; Region->dump();
             FOS << "\n";);
  (void)Region;
}

bool HIRSymbolicTripCountCompleteUnroll::hasLocalLoadOrStore(HLInst *HInst) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif
  LLVM_DEBUG(HInst->dump(););

  // Check: LoadInst, StoreInst, BinaryOperator, and CopyInst (4) types
  const Instruction *LLVMInst = HInst->getLLVMInstruction();
  bool IsLoadInst = isa<LoadInst>(LLVMInst);
  bool IsStoreInst = isa<StoreInst>(LLVMInst);
  bool IsCopyInst = HInst->isCopyInst();
  bool IsBinaryOpInst = isa<BinaryOperator>(LLVMInst);

  if (!(IsLoadInst || IsStoreInst || IsCopyInst || IsBinaryOpInst)) {
    return false;
  }

  bool Result = false;

  if (IsLoadInst) { // MemRef can only appear on RHS (pos:1) for a load
    RegDDRef *Ref = HInst->getOperandDDRef(1);
    // LLVM_DEBUG(Ref->dump(); FOS << "\n";);

    if (HIRSymbolicTripCountCompleteUnroll::isLocalMemRef(Ref)) {
      Result = true;
    }

  } else if (IsStoreInst) { // MemRef can appear on both LHS and RHS

    for (unsigned I = 0; I <= 1; ++I) {
      RegDDRef *Ref = HInst->getOperandDDRef(I);
      // LLVM_DEBUG(Ref->dump(); FOS << "\n";);

      if (HIRSymbolicTripCountCompleteUnroll::isLocalMemRef(Ref)) {
        Result = true;
      }
    }

  } else if (IsBinaryOpInst) {
    // BinaryOperator:
    // ON by default, Turn it OFF, if there is any non-Local MemRef Operand
    Result = true;

    for (unsigned I = 0, E = HInst->getNumOperands(); I < E; ++I) {
      RegDDRef *Ref = HInst->getOperandDDRef(I);
      LLVM_DEBUG(FOS << "Ref: "; Ref->dump(); FOS << "\n";);

      if (HIRSymbolicTripCountCompleteUnroll::isNonLocalMemRef(Ref)) {
        Result = false;
      }
    }
  } else { // CopyInst: Flag is ON by default
    Result = true;
  }

  return Result;
}

bool HIRSymbolicTripCountCompleteUnroll::hasEdgeInLoop(HLLoop *Lp,
                                                       RegDDRef *Ref,
                                                       DDGraph &DDG) {
  assert(Ref->isLval() && "Expect Ref be an Lval\n");
  DDRef *OtherRef = nullptr;
  bool Result = false;

  // Iterate over each relevant DDEdge
  for (const DDEdge *Edge : DDG.outgoing(Ref)) {
    LLVM_DEBUG(Edge->print(dbgs()););

    // Setup OtherRef:
    OtherRef = Edge->getSink();

    // Test: both ends of the Edge are in the Lp
    if (!(HLNodeUtils::contains(Lp, OtherRef->getHLDDNode()))) {
      return false;
    }

    Result = true;
  }

  return Result;
}

void HIRSymbolicTripCountCompleteUnroll::clearWorkingSetMemory(void) {
  // OuterLp related:
  OuterLpNodeVec.clear();
  OuterLpInstVec.clear();
  OuterLpLabelVec.clear();
  OuterLpGotoVec.clear();

  // InnerLp related:
  InnerLpNodeVec.clear();

  // Others:
  NonLocalRefVec.clear();
  MParentRefVec.clear();
  MLibsRefVec.clear();
}

PreservedAnalyses
HIRSymbolicTripCountCompleteUnrollPass::run(llvm::Function &F,
                                            llvm::FunctionAnalysisManager &AM) {
  HIRSymbolicTripCountCompleteUnroll(AM.getResult<HIRFrameworkAnalysis>(F),
                                     AM.getResult<TargetIRAnalysis>(F),
                                     AM.getResult<HIRDDAnalysisPass>(F)).run();

  return PreservedAnalyses::all();
}

class HIRSymbolicTripCountCompleteUnrollLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRSymbolicTripCountCompleteUnrollLegacyPass() : HIRTransformPass(ID) {
    initializeHIRSymbolicTripCountCompleteUnrollLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<TargetTransformInfoWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) {
    if (skipFunction(F)) {
      LLVM_DEBUG(dbgs() << "HIR Loop Pattern Match Early Skipped\n");
      return false;
    }

    return HIRSymbolicTripCountCompleteUnroll(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
               getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F),
               getAnalysis<HIRDDAnalysisWrapperPass>().getDDA()).run();
  }
};

char HIRSymbolicTripCountCompleteUnrollLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(
    HIRSymbolicTripCountCompleteUnrollLegacyPass,
    "hir-pm-symbolic-tripcount-completeunroll",
    "HIR Symbolic TripCount CompleteUnroll Pattern Match Pass", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRSymbolicTripCountCompleteUnrollLegacyPass,
                    "hir-pm-symbolic-tripcount-completeunroll",
                    "HIR Symbolic TripCount CompleteUnroll Pattern Match Pass",
                    false, false)

FunctionPass *llvm::createHIRSymbolicTripCountCompleteUnrollLegacyPass() {
  return new HIRSymbolicTripCountCompleteUnrollLegacyPass();
}
