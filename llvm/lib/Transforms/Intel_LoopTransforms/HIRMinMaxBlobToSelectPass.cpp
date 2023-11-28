//===---------------- HIRMinMaxBlobToSelect.cpp --------------------------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===------------------------------------------------------------------------===//
//
// This pass will convert a blob that represents a min or max operation into
// a select instruction. For example, assume the following loop in HIR:
//
//      %limm = (%maxchar)[0];
//   + DO i1 = 0, zext.i32.i64(%call1) + -1, 1   <DO_LOOP>
//   |   %1 = %limm;
//   |   %2 = (@mapped)[0][i1];
//   |   %limm = smax(%1, %2);
//   |   %conv8 = (%res)[i1 + sext.i32.i64(%k.027)]  +  1.000000e+00;
//   |   (%res)[i1 + sext.i32.i64(%k.027)] = %conv8;
//   + END LOOP
//      (%maxchar)[0] = %limm;
//
// The pattern of loading %limm, collecting the smax, and storing again in
// %limm can be substituted for a Select instructions. In HIR:
//
//   From:
//     %1 = %limm;
//     %2 = (@mapped)[0][i1];
//     %limm = smax(%1, %2);
//
//   To:
//     %2 = (@mapped)[0][i1];
//     %limm = (%limm >= %2) ? %limm : %2
//
// The new transformation will help the safe reduction analysis to identify
// that the loop is dealing with a safe reduction. This improves the chances
// to vectorize the loop.
//===---------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRMinMaxBlobToSelectPass.h"

#include "llvm/ADT/Statistic.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#define OPT_SWITCH "hir-min-max-blob-to-select"
#define OPT_DESC "HIR Convert min/max blobs to Select"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

STATISTIC(MinMaxInstConverted,
          "Number of instructions with min/max blob converted into select");

static cl::opt<bool>
    DisableMinMaxToSelect("disable-" OPT_SWITCH, cl::init(false), cl::Hidden,
                          cl::desc("Disable HIR convert Min/Max into select"));

class HIRMinMaxBlobToSelect {
public:
  HIRMinMaxBlobToSelect(HIRFramework &HIRF, HIRDDAnalysis &DDA,
                        HIRLoopStatistics &HLS) : HIRF(HIRF), DDA(DDA),
                        HLS(HLS) {}
  bool run();

private:
  HIRFramework &HIRF;
  HIRDDAnalysis &DDA;
  HIRLoopStatistics &HLS;

  struct CandidateEntry;

  // Analyze and collect the smax, smin, umax and umin blobs in a loop.
  void collectMinMaxBlobCandidates(HLLoop *Loop,
      SmallVectorImpl<CandidateEntry> &CandidatesVect);

  // Transform the instruction with min/max blobs into select instructions.
  void transformLoop(HLLoop *Loop,
                     SmallVectorImpl<CandidateEntry> &CandidatesVect);
};

// Helper structure used to represent a candidate instruction with min/max
// blob that will be transformed into a Select instruction. MinMaxInst
// represents the instruction with the min/max blob, and CopyInst represents
// the instruction with the temp copy. From example above:
//
//   MinMaxInst -> %limm = smax(%1, %2);
//   CopyInst -> %1 = %limm;
struct HIRMinMaxBlobToSelect::CandidateEntry {
private:
  HLInst *MinMaxInst;
  HLInst *CopyInst;
  const SCEVMinMaxExpr *MinMaxBlob;

public:
  CandidateEntry(HLInst *MinMaxInst, HLInst *CopyInst,
      const SCEVMinMaxExpr *MinMaxBlob) : MinMaxInst(MinMaxInst),
      CopyInst(CopyInst), MinMaxBlob(MinMaxBlob) {
    assert(MinMaxInst &&
           "Trying to create a candidate entry without min/max instruction");
    assert(CopyInst &&
           "Trying to create a candidate entry without copy instruction");
    assert(MinMaxBlob &&
           "Trying to create a candidate entry without min/max blob");
  }

  HLInst* getMinMaxInst() const { return MinMaxInst; }
  HLInst* getCopyInst() const { return CopyInst; }
  const SCEVMinMaxExpr* getMinMaxBlob() const { return MinMaxBlob; }
};

// Given an input blob, create a RegDDRef
static RegDDRef* generateOperandDDRef(BlobTy InBlob, RegDDRef *OriginalRef,
                                      unsigned LoopLevel, DDRefUtils &DDRU) {
  assert(InBlob && "Trying to generate a RegDDRef without blob");

  RegDDRef *ReturnRef = nullptr;
  if (auto *Const = dyn_cast<SCEVConstant>(InBlob)) {
    // Handle the case when the input blob is a constant
    Value *ConstVal = Const->getValue();
    ReturnRef = DDRU.createConstDDRef(ConstVal);
  } else if (isa<SCEVUnknown>(InBlob)) {
    // Handle the case when the input blob is a temp
    BlobUtils &BU = DDRU.getBlobUtils();
    unsigned NewBlobIdx = BU.findOrInsertBlob(InBlob);
    ReturnRef = DDRU.createSelfBlobRef(NewBlobIdx, NonLinearLevel);
    ReturnRef->makeConsistent(OriginalRef, LoopLevel);
  } else {
    // Handle the case when the input blob is is an operation
    BlobUtils &BU = DDRU.getBlobUtils();
    CanonExprUtils &CEU = DDRU.getCanonExprUtils();
    unsigned NewBlobIdx = BU.findOrInsertBlob(InBlob);
    auto *CE = CEU.createCanonExpr(InBlob->getType());
    CE->setBlobCoeff(NewBlobIdx, 1);
    ReturnRef = DDRU.createScalarRegDDRef(GenericRvalSymbase, CE);
    ReturnRef->makeConsistent(OriginalRef, LoopLevel);
  }

  return ReturnRef;
}

// Traverse through the instructions in the loop and identify if there are
// instructions with smax, umax, smin and/or umin blobs. If there are no
// unsafe calls, and there are possible cycles between a temp and the min/max
// blob then collect them. Basically, we will identify the following:
//
//  %1 = %limm;             \
//                         cycle
//  %limm = smax(%1, %2);   /
//
// There is a cycle between these two instructions, therefore add them into
// the candidates map.
void HIRMinMaxBlobToSelect::collectMinMaxBlobCandidates(HLLoop *Loop,
    SmallVectorImpl<CandidateEntry> &CandidatesVect) {
  assert(Loop && "Trying to analyze a loop that is set as null");
  assert(Loop->isInnermost() && "Loop is not innermost");

  if (!Loop->isDo())
    return;

  auto LoopStats = HLS.getTotalStatistics(Loop);
  if (LoopStats.hasCallsWithUnsafeSideEffects() ||
      LoopStats.hasCallsWithUnknownAliasing()) {
    LLVM_DEBUG(dbgs() << "Loop has unsafe calls\n");
    return;
  }

  // Collect the instructions with min/max blobs
  SmallVector<std::pair<HLInst *, const SCEVMinMaxExpr *>, 4> MinMaxInsts;
  for(HLNode &Child : make_range(Loop->child_begin(), Loop->child_end())) {
    HLInst *Inst = dyn_cast<HLInst>(&Child);
    if (!Inst)
      continue;

    if (Inst->isCallInst())
      continue;

    auto *LHS = Inst->getLvalDDRef();
    if (!LHS || !LHS->isTerminalRef())
      continue;

    auto *RHS = Inst->getRvalDDRef();
    if (!RHS || !RHS->isStandAloneBlob() || !RHS->isNonLinear())
      continue;

    auto *CE = RHS->getSingleCanonExpr();
    unsigned BlobIdx = CE->getSingleBlobIndex();
    const BlobUtils &BU = CE->getBlobUtils();
    auto *MinMaxBlob = dyn_cast<SCEVMinMaxExpr>(BU.getBlob(BlobIdx));
    if (!MinMaxBlob)
      continue;

    // The min/max operation must have 2 operands
    if (MinMaxBlob->getNumOperands() != 2)
      continue;

    // At least one of the operands must be a temp
    if (!isa<SCEVUnknown>(MinMaxBlob->getOperand(0)) &&
        !isa<SCEVUnknown>(MinMaxBlob->getOperand(1)))
      continue;

    MinMaxInsts.push_back(std::make_pair(Inst, MinMaxBlob));
  }

  if (MinMaxInsts.empty()) {
    LLVM_DEBUG(dbgs() << "Loop does not have min/max blobs\n");
    return;
  }

  DDGraph DDG = DDA.getGraph(Loop);
  for (auto Pair : MinMaxInsts) {
    auto *MinMaxInst = Pair.first;
    auto *MinMaxBlob = Pair.second;

    assert(MinMaxInst && "Trying to analyze min/max without instruction");
    assert(MinMaxBlob && "Trying to analyze min/max without blob");

    auto *LHS = MinMaxInst->getLvalDDRef();
    assert(LHS && "Min/Max instruction without left hand side");

    if (DDG.getNumIncomingEdges(LHS) != 1 ||
        DDG.getNumOutgoingEdges(LHS) != 1)
      continue;

    // Check the incoming and the outgoing edges of the left hand side of
    // the min/max instruction.
    auto *OriginalOutgoingEdge = *(DDG.outgoing_edges_begin(LHS));
    auto *OriginalIncomingEdge = *(DDG.incoming_edges_begin(LHS));

    // The incoming edge needs to be anti-dependent and the outgoing edge needs
    // to be flow-dependent. The outgoing edge will be the current min/max
    // instruction.
    if (!OriginalIncomingEdge->isAnti() || !OriginalOutgoingEdge->isFlow())
      continue;

    // The incoming edge is not embedded inside a blob in a canon expr.
    if (!isa<RegDDRef>(OriginalIncomingEdge->getSrc()))
      continue;

    // The incoming edge should be the temp define instruction. Analyze the
    // left hand side of it. It must not have any other incoming edges, and the
    // only outgoing edge must be the same instruction. This means that there
    // is only one possible define for it.
    auto *CopyInst =
        dyn_cast<HLInst>(OriginalIncomingEdge->getSrc()->getHLDDNode());
    if (!CopyInst)
      continue;

    if (!CopyInst->isCopyInst())
      continue;

    auto *LHSTempDefine = CopyInst->getLvalDDRef();
    if (DDG.getNumIncomingEdges(LHSTempDefine) != 0 ||
        DDG.getNumOutgoingEdges(LHSTempDefine) != 1)
      continue;

    // There will be only one incoming edge and one outgoing edge for the
    // right hand side of the temp define instruction. The incoming edge
    // represents the original min/max instruction, and the outgoing edge
    // represents the temp define instruction.
    auto *RHSTempDefine = CopyInst->getRvalDDRef();
    if (DDG.getNumIncomingEdges(RHSTempDefine) != 1 ||
        DDG.getNumOutgoingEdges(RHSTempDefine) != 1)
      continue;

    auto *RHSOutgoingEdge = *(DDG.outgoing_edges_begin(RHSTempDefine));
    auto *RHSIncomingEdge = *(DDG.incoming_edges_begin(RHSTempDefine));

    // The incoming edge needs to be flow-dependent and the outgoing edge needs
    // to be anti-dependent
    if (!RHSIncomingEdge->isFlow() || !RHSOutgoingEdge->isAnti())
      return;

    const BlobUtils &BU = MinMaxInst->getBlobUtils();
    auto *LHSTempDefineBlob = BU.getBlob(LHSTempDefine->getSelfBlobIndex());
    if (MinMaxBlob->getOperand(0) != LHSTempDefineBlob &&
        MinMaxBlob->getOperand(1) != LHSTempDefineBlob)
      continue;

    CandidatesVect.emplace_back(MinMaxInst, CopyInst, MinMaxBlob);
  }

}

// Go through each candidate collected for the input loop and convert them
// into Select instructions, and remove the temp define. For example, convert:
//
//   %limm = smax(%1, %2)   -->  %limm = (%limm >= %2) ? %limm : %2
void HIRMinMaxBlobToSelect::transformLoop(HLLoop *Loop,
    SmallVectorImpl<CandidateEntry> &CandidatesVect) {

  LLVM_DEBUG(dbgs() << "Loop: " << Loop->getNumber() << "\n");
  auto &HLNU = Loop->getHLNodeUtils();
  auto &DDRU = HLNU.getDDRefUtils();
  auto &BU = DDRU.getBlobUtils();
  HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(Loop);
  for (auto const &Entry : CandidatesVect) {

    HLInst *MinMaxInst = Entry.getMinMaxInst();
    HLInst *CopyInst = Entry.getCopyInst();
    const SCEVMinMaxExpr *MinMaxBlob = Entry.getMinMaxBlob();

    LLVM_DEBUG({
      dbgs() << "  Min/Max instruction: ";
      MinMaxInst->dump();
      dbgs() << "  Cycles with: ";
      CopyInst->dump();
    });

    HLInst *NewSelect = nullptr;
    auto *CopyLHS = CopyInst->getLvalDDRef();
    unsigned CopyLHSBlobIdx = CopyLHS->getSelfBlobIndex();

    auto *MinMaxRHS = MinMaxInst->getRvalDDRef();
    auto *Operand1Blob = MinMaxBlob->getOperand(0);
    auto *Operand2Blob = MinMaxBlob->getOperand(1);

    // Identify which blob is not the copy LHS, and generate a RegDDRef for it.
    auto *CopyLHSBlob = BU.getBlob(CopyLHSBlobIdx);
    RegDDRef *Operand2 = nullptr;
    unsigned Level = Loop->getNestingLevel();
    if (CopyLHSBlob == Operand1Blob)
      Operand2 = generateOperandDDRef(Operand2Blob, MinMaxRHS, Level, DDRU);
    else if (CopyLHSBlob == Operand2Blob)
      Operand2 = generateOperandDDRef(Operand1Blob, MinMaxRHS, Level, DDRU);

    assert(Operand2 && "Second operand for Select predicate not generated");

    RegDDRef *Operand1 =
        MinMaxInst->removeOperandDDRef(MinMaxInst->getLvalDDRef());

    // Create the proper select instruction for smax, umax, smin and umin
    // depending on the SCEVMinMaxExpr type.
    if (isa<SCEVSMaxExpr>(MinMaxBlob))
      NewSelect = HLNU.createMax(Operand1, Operand2, Operand1->clone(),
                                 true /* IsSigned */, true /* IsFPOrdered */,
                                 FastMathFlags(), "smax");
    else if (isa<SCEVUMaxExpr>(MinMaxBlob))
      NewSelect = HLNU.createMax(Operand1, Operand2, Operand1->clone(),
                                 false /* IsSigned */, true /* IsFPOrdered */,
                                 FastMathFlags(), "umax");
    else if (isa<SCEVSMinExpr>(MinMaxBlob))
      NewSelect = HLNU.createMin(Operand1, Operand2, Operand1->clone(),
                                 true /* IsSigned */, true /* IsFPOrdered */,
                                 FastMathFlags(), "smin");
    else if (isa<SCEVUMinExpr>(MinMaxBlob))
      NewSelect = HLNU.createMin(Operand1, Operand2, Operand1->clone(),
                                 false /* IsSigned */, true /* IsFPOrdered */,
                                 FastMathFlags(), "umin");
    else
      llvm_unreachable("Operation is not a min/max");

    // Replace the instruction with min/max blob with the new Select
    // instruction and remove the copy instruction.
    HLNodeUtils::replace(MinMaxInst, NewSelect);
    HLNodeUtils::remove(CopyInst);

    MinMaxInstConverted++;

    LLVM_DEBUG({
      dbgs() << "  New Min/Max instruction: ";
      NewSelect->dump();
      dbgs() << "\n";
    });
  }
}

bool HIRMinMaxBlobToSelect::run() {
  if (DisableMinMaxToSelect)
    return false;

  SmallVector<HLLoop *, 16> InnerMostLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(InnerMostLoops);

  bool Modified = false;
  for (auto *Loop : InnerMostLoops) {
    SmallVector<CandidateEntry, 4> CandidatesVect;
    collectMinMaxBlobCandidates(Loop, CandidatesVect);
    if (CandidatesVect.empty())
      continue;

    Modified = true;
    transformLoop(Loop, CandidatesVect);
  }

  return Modified;
}

PreservedAnalyses
HIRMinMaxBlobToSelectPass::runImpl(Function &F, FunctionAnalysisManager &AM,
                                   HIRFramework &HIRF) {

  ModifiedHIR =
      HIRMinMaxBlobToSelect(HIRF, AM.getResult<HIRDDAnalysisPass>(F),
                            AM.getResult<HIRLoopStatisticsAnalysis>(F))
          .run();

  return PreservedAnalyses::all();
}
