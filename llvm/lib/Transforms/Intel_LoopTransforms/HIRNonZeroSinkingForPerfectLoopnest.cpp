//===---------  HIRNonZeroSinkingForPerfectLoopnest.cpp --------------//
//---- Implements Non-Zero Sinking For Perfect Loopnest class -----===//
//
// Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass implements non-zero sinking for perfect loopnest to facilitate
// loop  blocking or unroll & jam.
//
// Case 1:
//
// We are transforming this-
// do i1
//   %t = A[0][i1];
//   if(%t != 0){
//     do i2
//       %mul = %t * B[i1][i2];
//       %add = C[0][i2] +/- %mul;
//       C[0][i2] = %add;
//     enddo
//   }
// enddo
//
// To-
// do i1
//   do i2
//     %mul = A[0][i1] * B[i1][i2];
//     %add = C[0][i2] +/- %mul;
//     C[0][i2] = %add;
//   enddo
// enddo/
//
// Case 2:
//
// We are transforming this-
// do i1
//   %t1 = A[0][i1];
//   if(%t1 != 0){
//     %t = %t1 * D[i1];
//     do i2
//       %mul = %t * B[i1][i2];
//       %add = C[0][i2] +/- %mul;
//       C[0][i2] = %add;
//     enddo
//   }
// enddo
//
// To-
// do i1
//   %t = A[0][i1] * %D[i1];
//   do i2
//     %mul = %t * B[i1][i2];
//     %add = C[0][i2] +/- %mul;
//     C[0][i2] = %add;
//   enddo
// enddo
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRNonZeroSinkingForPerfectLoopnest.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#define OPT_SWITCH "hir-non-zero-sinking-for-perfect-loopnest"
#define OPT_DESC "HIR Non-Zero Sinking For Perfect Loopnest"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

namespace {

class HIRNonZeroSinkingForPerfectLoopnest {
  HIRFramework &HIRF;

public:
  HIRNonZeroSinkingForPerfectLoopnest(HIRFramework &HIRF) : HIRF(HIRF) {}

  bool run();

private:
  void doNonZeroSinkingForPerfectLoopnest(HLLoop *Lp, HLInst *CandidateInst);

  bool doAnalysis(HLLoop *InnermostLp, HLInst *&CandidateInst);
};
} // namespace

// Iterate over each outgoing edge and find the candidate inst which needs
// replace the temp
// We do pattern matching here:
// do i1
//   do i2
//     %temp = B[i1][i2];
//     if(%temp != 0.0)
//        do i3
//          %mul = %temp * A[i2][i3];
//          %add = C[i1][i3] +/- %mul;
//          C[i1][i3] = %add;
//        enddo
//   enddo
// enddo
// Go through the loop children and check whether the pattern can be matched
static HLInst *findReplacementCandidate(HLLoop *Lp, RegDDRef *LoadRef,
                                        const RegDDRef *TerminalRef) {
  HLNode *FirstChild = Lp->getFirstChild();

  // Get %mul = %temp * A[i2][i3];
  HLInst *MulInst = dyn_cast<HLInst>(FirstChild);

  if (!MulInst) {
    return nullptr;
  }

  unsigned Opcode = MulInst->getLLVMInstruction()->getOpcode();

  if (Opcode != Instruction::Mul && Opcode != Instruction::FMul) {
    return nullptr;
  }

  RegDDRef *MulLvalRef = MulInst->getLvalDDRef();

  if (!MulLvalRef->isTerminalRef()) {
    return nullptr;
  }

  unsigned OperandNum = MulInst->getOperandDDRef(1)->isMemRef() ? 2 : 1;

  RegDDRef *TempRef = MulInst->getOperandDDRef(OperandNum);

  if (TempRef->getSymbase() != TerminalRef->getSymbase()) {
    return nullptr;
  }

  RegDDRef *MemRefInMulInst = MulInst->getOperandDDRef(3 - OperandNum);

  if (!MemRefInMulInst->isMemRef()) {
    return nullptr;
  }

  // Get %add = C[i1][i3] +/- %mul;
  HLInst *SumInst = dyn_cast<HLInst>(FirstChild->getNextNode());

  if (!SumInst) {
    return nullptr;
  }

  Opcode = SumInst->getLLVMInstruction()->getOpcode();

  if (Opcode != Instruction::Add && Opcode != Instruction::FAdd &&
      Opcode != Instruction::Sub && Opcode != Instruction::FSub) {
    return nullptr;
  }

  RegDDRef *OperandRef1 = SumInst->getOperandDDRef(1);
  RegDDRef *OperandRef2 = SumInst->getOperandDDRef(2);
  RegDDRef *MemRefInAddInst = nullptr;

  if (MulLvalRef->getSymbase() == OperandRef1->getSymbase()) {
    // If it is a subtraction operation, we need to have the pattern C[i1][i3]
    // - %mul
    if (Opcode == Instruction::Sub || Opcode == Instruction::FSub) {
      return nullptr;
    }

    MemRefInAddInst = OperandRef2;
  } else if (MulLvalRef->getSymbase() == OperandRef2->getSymbase()) {
    MemRefInAddInst = OperandRef1;
  } else {
    return nullptr;
  }

  // A[i2][i3] cannot have the same symbase as C[i1][i3]
  if (MemRefInAddInst->getSymbase() == MemRefInMulInst->getSymbase()) {
    return nullptr;
  }

  // Get C[i1][i3] = %add;
  HLInst *StoreHInst = dyn_cast<HLInst>(Lp->getLastChild());

  if (!StoreHInst) {
    return nullptr;
  }

  if (!isa<StoreInst>(StoreHInst->getLLVMInstruction())) {
    return nullptr;
  }

  // C[i1][i3]
  RegDDRef *StoreRef = StoreHInst->getLvalDDRef();

  if (SumInst->getLvalDDRef()->getSymbase() !=
      StoreHInst->getRvalDDRef()->getSymbase()) {
    return nullptr;
  }

  if (!DDRefUtils::areEqual(StoreRef, MemRefInAddInst)) {
    return nullptr;
  }

  if (StoreRef->getSymbase() == LoadRef->getSymbase()) {
    return nullptr;
  }

  return MulInst;
}

bool HIRNonZeroSinkingForPerfectLoopnest::doAnalysis(HLLoop *InnermostLp,
                                                     HLInst *&CandidateInst) {

  if (!InnermostLp->isDo() || InnermostLp->hasPreheader() ||
      InnermostLp->hasPostexit()) {
    return false;
  }

  // Only allow 1 single summation operation to avoid of side effect for the if
  // condition
  if (InnermostLp->getNumChildren() != 3) {
    return false;
  }

  HLLoop *ParentLp = InnermostLp->getParentLoop();

  if (!ParentLp) {
    return false;
  }

  if (ParentLp->getNumChildren() != 2) {
    return false;
  }

  HLInst *FirstInst = dyn_cast<HLInst>(ParentLp->getFirstChild());

  if (!FirstInst) {
    return false;
  }

  if (!isa<LoadInst>(FirstInst->getLLVMInstruction())) {
    return false;
  }

  HLIf *IfNode = dyn_cast<HLIf>(ParentLp->getLastChild());

  if (!IfNode) {
    return false;
  }

  if (IfNode->hasElseChildren()) {
    return false;
  }

  if (IfNode->getNumPredicates() != 1) {
    return false;
  }

  // Check there are at most two nodes in the IfNode, and there must be one
  // loop.
  unsigned NumThenChildren = IfNode->getNumThenChildren();

  if (!(NumThenChildren == 1 || NumThenChildren == 2)) {
    return false;
  }

  if (!isa<HLLoop>(IfNode->getLastThenChild())) {
    return false;
  }

  auto PredI = IfNode->pred_begin();

  const RegDDRef *LHSPredRef = IfNode->getLHSPredicateOperandDDRef(PredI);
  const RegDDRef *RHSPredRef = IfNode->getRHSPredicateOperandDDRef(PredI);

  if (!RHSPredRef->isZero()) {
    return false;
  }

  unsigned FirstInstLvalSB = FirstInst->getLvalDDRef()->getSymbase();

  if (ParentLp->isLiveOut(FirstInstLvalSB)) {
    return false;
  }

  RegDDRef *FirstInstRval = FirstInst->getRvalDDRef();

  // Case 2:
  if (NumThenChildren == 2) {
    HLInst *InstBeforeInnermostLp =
        dyn_cast<HLInst>(IfNode->getFirstThenChild());

    if (!InstBeforeInnermostLp) {
      return false;
    }

    unsigned Opcode = InstBeforeInnermostLp->getLLVMInstruction()->getOpcode();

    if (Opcode != Instruction::Mul && Opcode != Instruction::FMul) {
      return false;
    }

    RegDDRef *Operand1 = InstBeforeInnermostLp->getOperandDDRef(1);
    RegDDRef *Operand2 = InstBeforeInnermostLp->getOperandDDRef(2);

    if (Operand1->getSymbase() != LHSPredRef->getSymbase() &&
        Operand2->getSymbase() != LHSPredRef->getSymbase()) {
      return false;
    }

    if (!findReplacementCandidate(InnermostLp, FirstInstRval,
                                  InstBeforeInnermostLp->getLvalDDRef())) {
      return false;
    }

    CandidateInst = InstBeforeInnermostLp;
    return true;
  }

  // Case 1:
  if (LHSPredRef->getSymbase() != FirstInstLvalSB) {
    return false;
  }

  CandidateInst =
      findReplacementCandidate(InnermostLp, FirstInstRval, LHSPredRef);

  if (!CandidateInst) {
    return false;
  }

  return true;
}

void HIRNonZeroSinkingForPerfectLoopnest::doNonZeroSinkingForPerfectLoopnest(
    HLLoop *Lp, HLInst *MulInst) {
  HLLoop *ParentLp = Lp->getParentLoop();

  HLInst *FirstInst = dyn_cast<HLInst>(ParentLp->getFirstChild());

  RegDDRef *MemRef = FirstInst->removeRvalDDRef();
  RegDDRef *LvalRef = FirstInst->getLvalDDRef();

  // Replace the temp with memref in the MulInst
  unsigned OperandNum =
      MulInst->getOperandDDRef(1)->getSymbase() == LvalRef->getSymbase() ? 1
                                                                         : 2;
  MulInst->setOperandDDRef(MemRef, OperandNum);

  for (auto Blob : make_range(MemRef->blob_begin(), MemRef->blob_end())) {
    Lp->addLiveInTemp(Blob->getSymbase());
  }

  // Delete the HLIf
  HLIf *IfNode = dyn_cast<HLIf>(ParentLp->getLastChild());
  HLNodeUtils::replaceNodeWithBody(IfNode, true);

  // Delete the load inst
  HLNodeUtils::remove(FirstInst);

  return;
}

bool HIRNonZeroSinkingForPerfectLoopnest::run() {
  if (DisablePass) {
    LLVM_DEBUG(
        dbgs() << "HIR Non-Zero Sinking For Perfect Loopnest Disabled \n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIR Non-Zero Sinking For Perfect Loopnest : "
                    << HIRF.getFunction().getName() << "\n");

  // Gather all inner-most loops as Candidates
  SmallVector<HLLoop *, 64> InnermostLoops;

  HLNodeUtils &HNU = HIRF.getHLNodeUtils();
  HNU.gatherInnermostLoops(InnermostLoops);

  if (InnermostLoops.empty()) {
    LLVM_DEBUG(dbgs() << HIRF.getFunction().getName()
                      << "() has no inner-most loop\n ");
    return false;
  }

  bool Result = false;

  for (auto &Lp : InnermostLoops) {
    HLInst *CandidateInst;

    // Analyze the loop and check if there is a candidate for non-zero sinking
    if (!doAnalysis(Lp, CandidateInst)) {
      continue;
    }

    doNonZeroSinkingForPerfectLoopnest(Lp, CandidateInst);
    Result = true;

    HIRInvalidationUtils::invalidateBody(Lp);
    HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(Lp);
  }

  return Result;
}

PreservedAnalyses HIRNonZeroSinkingForPerfectLoopnestPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  HIRNonZeroSinkingForPerfectLoopnest(HIRF).run();
  return PreservedAnalyses::all();
}

class HIRNonZeroSinkingForPerfectLoopnestLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRNonZeroSinkingForPerfectLoopnestLegacyPass() : HIRTransformPass(ID) {
    initializeHIRNonZeroSinkingForPerfectLoopnestLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRNonZeroSinkingForPerfectLoopnest(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR())
        .run();
  }
};

char HIRNonZeroSinkingForPerfectLoopnestLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRNonZeroSinkingForPerfectLoopnestLegacyPass, OPT_SWITCH,
                      OPT_DESC, false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_END(HIRNonZeroSinkingForPerfectLoopnestLegacyPass, OPT_SWITCH,
                    OPT_DESC, false, false)

FunctionPass *llvm::createHIRNonZeroSinkingForPerfectLoopnestPass() {
  return new HIRNonZeroSinkingForPerfectLoopnestLegacyPass();
}
