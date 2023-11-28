// ===- HIRMinMaxRecognition.cpp - Implement HIR MinMaxRecognition pass --===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===---------------------------------------------------------------------===//
//
//===---------------------------------------------------------------------===//
//
// This file implements HIR min/max idiom recognition and transformation.
// The transofmation benefits when loop is vectorized due to changing of masked
// store to unmasked store. Given an innermost loop consising of loads, if and
// store - try to recognize min/max pattern:
//
//  Before:
//  + DO i3 = 0, 53, 1   <DO_LOOP>
//  |   %x = (@ARRAY)[0][i2][i3];
//  |   if (%x > (@BUF)[0][i1][2][i3])
//  |   {
//  |      (@BUF)[0][i1][2][i3] = %x;
//  |   }
//  + END LOOP
//
//  After:
//  + DO i3 = 0, 53, 1   <DO_LOOP>
//  |   %x = (@ARRAY)[0][i2][i3];
//  |   %call = @llvm.maxnum.f64(%x, (@BUF)[0][i1][2][i3]);
//  |   (@BUF)[0][i1][2][i3] = %call;
//  + END LOOP
//
// Another example with Ext and Trunc is:
// Before:
//   + DO i2 = 0, 53, 1   <DO_LOOP>
//   |   %x = (%0)[i1][i2];
//   |   %y = fpext.float.double((%5)[%9][%2][i1 + 8][i2 + 4]);
//   |   if (%x > %y)
//   |   {
//   |      %z = fptrunc.double.float(%x);
//   |      (%5)[%9][%2][i1 + 8][i2 + 4] = %z;
//   |   }
//   + END LOOP
//
// After:
//   + DO i2 = 0, 53, 1   <DO_LOOP>
//   |   %x = (%0)[i1][i2];
//   |   %y = fpext.float.double((%5)[%9][%2][i1 + 8][i2 + 4]);
//   |   %call = @llvm.maxnum.f64(%x, %y);
//   |   %z = fptrunc.double.float(%call);
//   |   (%5)[%9][%2][i1 + 8][i2 + 4] = %z;
//   + END LOOP
//
// Available options:
// -hir-minmax-recognition:          perform HIR min/max recognition
// -disable-hir-minmax-recognition:  Flag to disable HIR MinMaxRecognition
//
// TODO: Current implementation could be changed to recognize more variants:
//   - switch arguments of min/max
//   - recognize Ext/Trunc for integer types
//

#include "llvm/Transforms/Intel_LoopTransforms/HIRMinMaxRecognitionPass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"

#include "HIRMinMaxRecognition.h"

#define OPT_SWITCH "hir-minmax-recognition"
#define OPT_DESC "HIR MinMax Recognition"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool>
    DisableHIRMinMaxRecognition("disable-" OPT_SWITCH, cl::init(false),
                                cl::Hidden,
                                cl::desc("Disable HIR MinMaxRecognition pass"));

static cl::opt<unsigned> SmallTCThreshold(
    OPT_SWITCH "-small-tc-threshold", cl::init(12), cl::Hidden,
    cl::desc("Generate small trip count check while " OPT_DESC));

// This function implements min/max recognition.
bool HIRMinMaxRecognition::isMinOrMaxPattern(HLLoop *Loop,
                                             MinMaxCandidate &MMCand) {
  LLVM_DEBUG(dbgs() << "\n run Min/Max recognition ");
  unsigned NumChildren = Loop->getNumChildren();

  if (NumChildren > 3 || NumChildren < 2) {
    return false;
  }

  // First instruction ecpected to be a load of the first min/max argument.
  HLInst *Load = dyn_cast<HLInst>(Loop->getFirstChild());
  if (!Load || !isa<LoadInst>(Load->getLLVMInstruction())) {
    return false;
  }

  // If there are extension/truncation involved, then we recognize extension
  // here.
  auto *NextNode = Load->getNextNode();
  HLInst *Ext = dyn_cast<HLInst>(NextNode);
  if (Ext && isa<FPExtInst>(Ext->getLLVMInstruction())) {
    NextNode = NextNode->getNextNode();
  } else if (NumChildren != 2) {
    // We only expect 3 statements in the loop if FPExt present.
    return false;
  }

  // Next instruction should be comparison of the arguments. 'If' statement
  // expected to have only 'then' branch.
  HLIf *If = dyn_cast<HLIf>(NextNode);
  if (!If || (If->getNumPredicates() != 1) || (If->getNumThenChildren() > 2) ||
      If->hasElseChildren()) {
    return false;
  }

  // If previously we had extension, then the first instruction in the 'then'.
  // branch should be truncation.
  NextNode = If->getFirstThenChild();
  HLInst *Trunc = dyn_cast<HLInst>(NextNode);
  if (Trunc && isa<FPTruncInst>(Trunc->getLLVMInstruction()) && Ext) {
    NextNode = NextNode->getNextNode();
  } else if (If->getNumThenChildren() != 1) {
    // We only expect two statements in the 'then' branch if one of them is
    // FPTrunc and we previously found an Ext.
    return false;
  }

  assert(NextNode && "Expected one more statement in the loop");

  // Finally, we have a store.
  HLInst *Store = dyn_cast<HLInst>(NextNode);
  if (!Store || !isa<StoreInst>(Store->getLLVMInstruction())) {
    return false;
  }

  auto *LoadLHS = Load->getLvalDDRef();

  auto *StoreLHS = Store->getLvalDDRef();
  auto *StoreRHS = Store->getRvalDDRef();

  auto IfPredIt = If->pred_begin();

  auto *IfPredLHS = If->getLHSPredicateOperandDDRef(IfPredIt);
  auto *IfPredRHS = If->getRHSPredicateOperandDDRef(IfPredIt);

  // Use predicate to decide which intrinsic should be generated.
  PredicateTy Pred = *IfPredIt;
  if (Pred == PredicateTy::ICMP_SGE || Pred == PredicateTy::ICMP_SGT) {
    MMCand.IsMin = false;
    MMCand.IsSigned = true;
    MMCand.IsFloat = false;
  } else if (Pred == PredicateTy::ICMP_SLE || Pred == PredicateTy::ICMP_SLT) {
    MMCand.IsMin = true;
    MMCand.IsSigned = true;
    MMCand.IsFloat = false;
  } else if (Pred == PredicateTy::ICMP_UGE || Pred == PredicateTy::ICMP_UGT) {
    MMCand.IsMin = false;
    MMCand.IsSigned = false;
    MMCand.IsFloat = false;
  } else if (Pred == PredicateTy::ICMP_ULE || Pred == PredicateTy::ICMP_ULT) {
    MMCand.IsMin = true;
    MMCand.IsSigned = false;
    MMCand.IsFloat = false;
  } else if (Pred == PredicateTy::FCMP_OGE || Pred == PredicateTy::FCMP_OGT) {
    MMCand.IsMin = false;
    MMCand.IsSigned = false;
    MMCand.IsFloat = true;
  } else if (Pred == PredicateTy::FCMP_OLE || Pred == PredicateTy::FCMP_OLT) {
    MMCand.IsMin = true;
    MMCand.IsSigned = false;
    MMCand.IsFloat = true;
  } else {
    return false;
  }

  if (Ext && Trunc) {
    // Consider case with extension and truncation.
    auto *ExtLHS = Ext->getLvalDDRef();
    auto *ExtRHS = Ext->getRvalDDRef();
    auto *FpTruncLHS = Trunc->getLvalDDRef();
    auto *FpTruncRHS = Trunc->getRvalDDRef();

    // Check that all LHSs and RHSs match the min/max pattern.
    if (!DDRefUtils::areEqual(LoadLHS, IfPredLHS) ||
        !DDRefUtils::areEqual(LoadLHS, FpTruncRHS) ||
        !DDRefUtils::areEqual(ExtLHS, IfPredRHS) ||
        !DDRefUtils::areEqual(ExtRHS, StoreLHS) ||
        !DDRefUtils::areEqual(FpTruncLHS, StoreRHS)) {
      return false;
    }

    // Initialize min/max candidate structure fields.
    MMCand.Ext = Ext;
    MMCand.Trunc = Trunc;
    MMCand.MinMaxOp2 = ExtLHS;
  } else {
    // Consider case without extension and truncation.
    // Check that all LHSs and RHSs match the min/max pattern.
    if (!DDRefUtils::areEqual(LoadLHS, IfPredLHS) ||
        !DDRefUtils::areEqual(IfPredLHS, StoreRHS) ||
        !DDRefUtils::areEqual(StoreLHS, IfPredRHS)) {
      return false;
    }

    // Initialize min/max candidate structure fields.
    MMCand.MinMaxOp2 = IfPredRHS;
  }

  // Initialize min/max candidate structure fields.
  MMCand.Store = Store;
  MMCand.If = If;
  MMCand.MinMaxOp1 = LoadLHS;

  return true;
}

// This function implements transformation.
void HIRMinMaxRecognition::doMinMaxTransformation(HLLoop *Loop,
                                                  MinMaxCandidate &MMCand) {
  LLVM_DEBUG(dbgs() << "\n run doMinMaxTransformation\n");
  LLVM_DEBUG(MMCand.dump());

  auto *Op1Ref = MMCand.MinMaxOp1->clone();
  auto *Op2Ref = MMCand.MinMaxOp2->clone();

  // Choose intrinsic based on 'if' predicate.
  Intrinsic::ID ID;
  if (MMCand.IsFloat) {
    if (MMCand.IsMin)
      ID = Intrinsic::minnum;
    else
      ID = Intrinsic::maxnum;
  } else {
    if (MMCand.IsMin) {
      if (MMCand.IsSigned)
        ID = Intrinsic::smin;
      else
        ID = Intrinsic::umin;
    } else {
      if (MMCand.IsSigned)
        ID = Intrinsic::smax;
      else
        ID = Intrinsic::umax;
    }
  }

  // Create min/max call.
  Function *F = Intrinsic::getDeclaration(&HIRF.getModule(), ID,
                                          MMCand.MinMaxOp1->getDestType());
  HLNodeUtils &HNU = HIRF.getHLNodeUtils();
  HLInst *Call = HNU.createCall(F, {Op1Ref, Op2Ref});
  auto *ResRef = Call->getLvalDDRef()->clone();

  // Replase 'if' with min/max call.
  HLNodeUtils::replace(MMCand.If, Call);

  // Unlink 'store'.
  HLNodeUtils::remove(MMCand.Store);

  // Unlink and update 'trunc' or update 'store' if no 'trunc' found.
  auto *InsertPoint = Call;
  if (MMCand.Trunc) {
    HLNodeUtils::remove(MMCand.Trunc);
    MMCand.Trunc->replaceOperandDDRef(MMCand.Trunc->getRvalDDRef(), ResRef);
    HLNodeUtils::insertAfter(InsertPoint, MMCand.Trunc);
    InsertPoint = MMCand.Trunc;
  } else {
    MMCand.Store->replaceOperandDDRef(MMCand.Store->getRvalDDRef(), ResRef);
  }

  // Insert 'store' after min/max call.
  HLNodeUtils::insertAfter(InsertPoint, MMCand.Store);
}

static bool isSmallCountLoop(const HLLoop *Loop) {
  if (SmallTCThreshold == 0) {
    return false;
  }

  auto MaxTCEstimate = Loop->getMaxTripCountEstimate();
  if (MaxTCEstimate != 0 && MaxTCEstimate <= SmallTCThreshold) {
    return true;
  }

  uint64_t ConstTC;
  if (Loop->isConstTripLoop(&ConstTC) && ConstTC <= SmallTCThreshold) {
    return true;
  }

  return false;
}

bool HIRMinMaxRecognition::run() {
  if (DisableHIRMinMaxRecognition) {
    LLVM_DEBUG(dbgs() << "HIR Min/Max Recognition Disabled or Skipped\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIR MinMax Recognition on Function : "
                    << HIRF.getFunction().getName() << "\n");

  SmallVector<HLLoop *, 64> InnermostLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(InnermostLoops);

  if (InnermostLoops.empty()) {
    return false;
  }

  bool Modified = false;
  for (auto *Loop : InnermostLoops) {
    LLVM_DEBUG(dbgs() << "\nProcessing Loop: <" << Loop->getNumber() << ">\n");

    if (!Loop->isDo() || !Loop->isNormalized() ||
        Loop->hasVectorizeDisablingPragma()) {
      LLVM_DEBUG(
          dbgs()
          << "Skipping - non-DO-Loop / non-Normalized /non-Vec pragma loop\n");
      // TODO: should be 'continue' instead?
      return false;
    }

    if (isSmallCountLoop(Loop)) {
      LLVM_DEBUG(dbgs() << "Skipping small trip count loops\n");
      // TODO: should be 'continue' instead?
      return false;
    }

    MinMaxCandidate MMCand;
    // Try to recognize min/max pattern.
    if (isMinOrMaxPattern(Loop, MMCand)) {
      // Transform loop.
      doMinMaxTransformation(Loop, MMCand);

      // Mark region as modified.
      Loop->getParentRegion()->setGenCode();

      // Loop body should be invalidated.
      HIRInvalidationUtils::invalidateBody(Loop);
      Modified = true;
    }
  }

  return Modified;
}

PreservedAnalyses HIRMinMaxRecognitionPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  ModifiedHIR = HIRMinMaxRecognition(HIRF).run();

  return PreservedAnalyses::all();
}