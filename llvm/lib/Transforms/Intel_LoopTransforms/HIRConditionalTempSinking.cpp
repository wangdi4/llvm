//===------------------ HIRConditionalTempSinking.cpp ---------------------===//
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
// This pass sinks temp definitions from inside conditions to just after them.
// This can help in recognizing safe reductions.
//
// For example:
//
// DO i1
//  if () {
//    t1 = t1 + A[i1];
//  } else {
//    t1 = t1 + B[i1];
//  }
// END DO
//
// ==>
//
// DO i1
//  if () {
//    t2 = A[i1];
//  } else {
//    t2 = B[i1];
//  }
//  t1 = t1 + t2;  // unconditional safe reduction
// END DO
//
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRConditionalTempSinking.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#define OPT_SWITCH "hir-conditional-temp-sinking"
#define OPT_DESC "HIR Conditional Temp Sinking"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

namespace {

class HIRConditionalTempSinkingLegacyPass : public HIRTransformPass {
public:
  static char ID;
  HIRConditionalTempSinkingLegacyPass() : HIRTransformPass(ID) {
    initializeHIRConditionalTempSinkingLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.setPreservesAll();
  }
};

} // namespace

char HIRConditionalTempSinkingLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRConditionalTempSinkingLegacyPass, OPT_SWITCH, OPT_DESC,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRConditionalTempSinkingLegacyPass, OPT_SWITCH, OPT_DESC,
                    false, false)

FunctionPass *llvm::createHIRConditionalTempSinkingPass() {
  return new HIRConditionalTempSinkingLegacyPass();
}

class HIRConditionalTempSinking {
  HIRLoopStatistics &HLS;

public:
  HIRConditionalTempSinking(HIRLoopStatistics &HLS) : HLS(HLS) {}
  bool run(HLLoop *Lp);
};

namespace {

// Contains info about a temp candidate which can be sinked.
struct CandidateInfo {
  unsigned Opcode;
  RegDDRef *LvalRef;
  RegDDRef *FirstRvalRef;
  bool HasNSW;
  bool HasNUW;

  CandidateInfo()
      : Opcode(0), LvalRef(nullptr), FirstRvalRef(nullptr), HasNSW(false),
        HasNUW(false) {}

  bool matches(const HLInst *Inst) const {
    auto *LLVMInst = Inst->getLLVMInstruction();
    auto *FPMathOp = dyn_cast<FPMathOperator>(LLVMInst);
    auto *OBinOp = dyn_cast<OverflowingBinaryOperator>(LLVMInst);

    return (Opcode == LLVMInst->getOpcode()) &&
           DDRefUtils::areEqual(LvalRef, Inst->getLvalDDRef()) &&
           DDRefUtils::areEqual(FirstRvalRef, Inst->getOperandDDRef(1)) &&
           (Inst->isCopyInst() ||
            ((!FPMathOp || FPMathOp->isFast()) &&
             (!OBinOp || ((HasNSW == OBinOp->hasNoSignedWrap()) &&
                          (HasNUW == OBinOp->hasNoUnsignedWrap())))));
  }
};

// Finds an appropriate temp to be sinked and populates all its definitions in
// CandidateDefs.
class SinkCandidateFinder final : public HLNodeVisitorBase {
  bool Found;
  bool HasMissingDefs;

  HLLoop *Lp;
  SmallVectorImpl<HLInst *> &CandidateDefs;

public:
  SinkCandidateFinder(HLLoop *Lp, SmallVectorImpl<HLInst *> &CandidateDefs)
      : Found(false), HasMissingDefs(false), Lp(Lp),
        CandidateDefs(CandidateDefs) {}

  bool gatherCandidateDefs(HLNode *Node, const CandidateInfo &CandInfo,
                           unsigned &NumMissingDefs);
  bool gatherCandidateDefs(HLIf *If, const CandidateInfo &CandInfo,
                           unsigned &NumMissingDefs);

  void visit(HLNode *) {}
  void visit(HLIf *If);

  void postVisit(HLNode *) {}

  bool isDone() const { return Found; }
  bool found() const { return Found; }
  bool hasMissingDefs() const { return HasMissingDefs; }
};

// Finds any other occurence of temp apart from known occurences passed in by
// the caller.
class ExtraOccurenceFinder final : public HLNodeVisitorBase {
  bool Found;
  unsigned TempIndex;
  const SmallVectorImpl<HLInst *> &KnownOccurrences;

public:
  ExtraOccurenceFinder(unsigned TempIndex,
                       const SmallVectorImpl<HLInst *> &KnownOccurrences)
      : Found(false), TempIndex(TempIndex), KnownOccurrences(KnownOccurrences) {
  }

  void visit(const HLDDNode *Node);
  void visit(const HLNode *) {}
  void postVisit(const HLNode *) {}

  bool isDone() const { return Found; }
  bool found() const { return Found; }
};

} // namespace

void ExtraOccurenceFinder::visit(const HLDDNode *Node) {

  // Ignore known uses.
  if (auto *Inst = dyn_cast<HLInst>(Node)) {
    for (auto *KnownInst : KnownOccurrences) {
      if (Inst == KnownInst) {
        return;
      }
    }
  }

  for (auto *Ref : make_range(Node->ddref_begin(), Node->ddref_end())) {
    if (Ref->usesTempBlob(TempIndex)) {
      Found = true;
      return;
    }
  }
}

bool SinkCandidateFinder::gatherCandidateDefs(HLNode *Node,
                                              const CandidateInfo &CandInfo,
                                              unsigned &NumMissingDefs) {

  if (auto *If = dyn_cast<HLIf>(Node)) {
    return gatherCandidateDefs(If, CandInfo, NumMissingDefs);

  } else if (auto *Inst = dyn_cast<HLInst>(Node)) {

    // If all attributes of instruction match, add it as a new def otherwise
    // classify as a missing definition case.
    if (CandInfo.matches(Inst)) {
      CandidateDefs.push_back(Inst);
    } else {
      ++NumMissingDefs;
    }

    return true;
  }

  // Only handle HLIf and HLInst for now.
  return false;
}

bool SinkCandidateFinder::gatherCandidateDefs(HLIf *If,
                                              const CandidateInfo &CandInfo,
                                              unsigned &NumMissingDefs) {

  // We only check the last then/else case node for now. This does not affect
  // legality, only profitability.
  if (If->hasThenChildren()) {
    if (!gatherCandidateDefs(If->getLastThenChild(), CandInfo,
                             NumMissingDefs)) {
      return false;
    }
  } else {
    ++NumMissingDefs;
  }

  if (If->hasElseChildren()) {
    if (!gatherCandidateDefs(If->getLastElseChild(), CandInfo,
                             NumMissingDefs)) {
      return false;
    }
  } else {
    ++NumMissingDefs;
  }

  return true;
}

static bool isReductionLike(HLLoop *Lp, HLInst *Inst, CandidateInfo &CandInfo) {

  auto *LLVMInst = Inst->getLLVMInstruction();
  auto *BOp = dyn_cast<BinaryOperator>(LLVMInst);

  // Only handling binary operators for now.
  if (!BOp) {
    return false;
  }

  unsigned Opcode;
  if (!Inst->isReductionOp(&Opcode)) {
    return false;
  }

  bool HasNSW = false, HasNUW = false;

  if (auto *FPMathOp = dyn_cast<FPMathOperator>(LLVMInst)) {
    // Only handle fast math operations.
    if (!FPMathOp->isFast()) {
      return false;
    }

  } else if (auto *OBinOp = dyn_cast<OverflowingBinaryOperator>(LLVMInst)) {
    HasNSW = OBinOp->hasNoSignedWrap();
    HasNUW = OBinOp->hasNoUnsignedWrap();
  }

  auto *LvalRef = Inst->getLvalDDRef();

  if (!LvalRef->isTerminalRef()) {
    return false;
  }

  auto *FirstRvalRef = Inst->getOperandDDRef(1);
  // LvalRef should be equal to first rval operand.
  if (!DDRefUtils::areEqual(LvalRef, FirstRvalRef)) {
    return false;
  }

  if (!Lp->isLiveIn(LvalRef->getSymbase()) ||
      !Lp->isLiveOut(LvalRef->getSymbase())) {
    return false;
  }

  CandInfo.Opcode = Opcode;
  CandInfo.LvalRef = LvalRef;
  CandInfo.FirstRvalRef = FirstRvalRef;
  CandInfo.HasNSW = HasNSW;
  CandInfo.HasNUW = HasNUW;

  return true;
}

static bool isTempCopy(HLInst *Inst, CandidateInfo &CandInfo) {
  if (!Inst->isCopyInst()) {
    return false;
  }

  CandInfo.Opcode = Inst->getLLVMInstruction()->getOpcode();
  CandInfo.LvalRef = Inst->getLvalDDRef();
  CandInfo.FirstRvalRef = Inst->getRvalDDRef();

  return true;
}

static bool
hasNonReductionOccurences(unsigned ReductionTempIndex, const HLLoop *Lp,
                          const SmallVectorImpl<HLInst *> &ReductionDefs) {
  ExtraOccurenceFinder EF(ReductionTempIndex, ReductionDefs);

  HLNodeUtils::visitRange(EF, Lp->child_begin(), Lp->child_end());

  return EF.found();
}

void SinkCandidateFinder::visit(HLIf *If) {
  // Remove stale defs.
  CandidateDefs.clear();

  if (!If->hasThenChildren()) {
    return;
  }

  auto *LastThenInst = dyn_cast<HLInst>(If->getLastThenChild());

  if (!LastThenInst) {
    return;
  }

  CandidateInfo CandInfo;

  bool IsReductionLike = isReductionLike(Lp, LastThenInst, CandInfo);

  if (!IsReductionLike) {
    if (!isTempCopy(LastThenInst, CandInfo)) {
      return;
    }
  }

  unsigned NumMissingDefs = 0;

  if (!gatherCandidateDefs(If, CandInfo, NumMissingDefs)) {
    return;
  }

  assert(!CandidateDefs.empty() && "At least one definition expected!");

  // Profitability check for handling missing temp defs.
  if (IsReductionLike) {
    if (CandidateDefs.size() < 2 * NumMissingDefs) {
      return;
    }
  } else if (NumMissingDefs > 0) {
    return;
  }

  if (IsReductionLike) {
    unsigned ReductionTempIndex = CandInfo.LvalRef->getSelfBlobIndex();

    if (hasNonReductionOccurences(ReductionTempIndex, Lp, CandidateDefs)) {
      return;
    }

    HasMissingDefs = (NumMissingDefs != 0);
  }

  Found = true;
}

static bool findSinkCandidate(HLLoop *Lp,
                              SmallVectorImpl<HLInst *> &CandidateDefs,
                              bool &RequiresInitialization) {
  SinkCandidateFinder SCF(Lp, CandidateDefs);

  HLNodeUtils::visitRange(SCF, Lp->child_begin(), Lp->child_end());

  RequiresInitialization = SCF.hasMissingDefs();

  return SCF.found();
}

static void sinkReduction(SmallVectorImpl<HLInst *> &CandidateDefs,
                          bool RequiresInitialization) {

  auto *SinkInst = CandidateDefs.front();
  auto &HNU = SinkInst->getHLNodeUtils();

  auto *If = cast<HLIf>(SinkInst->getParent());
  auto *Ty = SinkInst->getLvalDDRef()->getDestType();

  RegDDRef *NewTemp;

  if (!RequiresInitialization) {
    NewTemp = HNU.createTemp(Ty, "tmp");

  } else {
    // Insert initialization for the new reduction operand to cover cases where
    // the definition is missing. For example;
    //
    // if () {
    //   t1 = t1 + ...
    // } else {
    //   // missing definition of t1
    // }
    //
    // changes to-
    // t2 = 0 // init inst.
    // if () {
    //   t2 = ...
    // } else {
    // }
    // t1 = t1 + t2
    auto *Const = HLInst::getRecurrenceIdentity(
        SinkInst->getLLVMInstruction()->getOpcode(), Ty);
    RegDDRef *InitRef = nullptr;

    if (auto *IntConst = dyn_cast<ConstantInt>(Const)) {
      int64_t Val = IntConst->getSExtValue();
      InitRef = HNU.getDDRefUtils().createConstDDRef(Ty, Val);
    } else {
      InitRef = HNU.getDDRefUtils().createConstDDRef(Const);
    }

    auto *InitInst = HNU.createCopyInst(InitRef, "tmp");

    NewTemp = InitInst->getLvalDDRef();

    HLNodeUtils::insertBefore(If, InitInst);
  }

  // Replace all definitions with copies/loads. For example-
  // if () {
  //   t1 = t1 + A[i]
  // }
  //
  // changes to-
  //
  // if () {
  //   t2 = A[i];
  // }
  for (auto *TempDef : CandidateDefs) {
    auto *RvalRef = TempDef->removeOperandDDRef(2);
    auto *LvalRef = NewTemp->clone();

    auto *CopyInst = RvalRef->isMemRef()
                         ? HNU.createLoad(RvalRef, "tmp", LvalRef)
                         : HNU.createCopyInst(RvalRef, "tmp", LvalRef);

    HLNodeUtils::replace(TempDef, CopyInst);
  }

  // Insert the first def after the 'if' after replacing its operand with the
  // new temp.
  SinkInst->setOperandDDRef(NewTemp->clone(), 2);
  HLNodeUtils::insertAfter(If, SinkInst);
}

static void sinkCopy(SmallVectorImpl<HLInst *> &CandidateDefs) {

  // Transforms -
  //
  // if () {
  //   t1 = t2;
  // } else {
  //   t1 = t2;
  // }
  //
  // Into-
  //
  // if () {
  //
  // } else {
  //
  // }
  // t1 = t2;
  //
  auto *FirstCopy = CandidateDefs.front();
  auto *If = cast<HLIf>(FirstCopy->getParent());

  for (auto *TempCopy : CandidateDefs) {
    HLNodeUtils::remove(TempCopy);
  }

  HLNodeUtils::insertAfter(If, FirstCopy);

  // HLIf can become empty after sinking so we need to run empty node removal.
  HLNodeUtils::removeEmptyNodes(If);
}

bool HIRConditionalTempSinking::run(HLLoop *Lp) {
  // Profitability check.
  if (!Lp->isDo()) {
    return false;
  }

  auto &LoopStats = HLS.getTotalLoopStatistics(Lp);

  // Nothing to sink.
  if (!LoopStats.hasIfs()) {
    return false;
  }

  SmallVector<HLInst *, 16> CandidateDefs;
  bool RequiresInitialization;

  // Only finds one candidate per loop for now, can be extended later.
  if (!findSinkCandidate(Lp, CandidateDefs, RequiresInitialization)) {
    return false;
  }

  if (CandidateDefs.front()->isReductionOp()) {
    sinkReduction(CandidateDefs, RequiresInitialization);
  } else {
    sinkCopy(CandidateDefs);
  }

  HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(Lp);
  return true;
}

static bool doConditionalTempSinking(HIRFramework &HIRF,
                                     HIRLoopStatistics &HLS) {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << "HIR Conditional Temp Sinking Disabled \n");
    return false;
  }

  SmallVector<HLLoop *, 64> CandidateLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(CandidateLoops);

  HIRConditionalTempSinking CTS(HLS);

  bool Result = false;

  for (auto *Lp : CandidateLoops) {
    Result = CTS.run(Lp) || Result;
  }

  return Result;
}

bool HIRConditionalTempSinkingLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    return false;
  }

  bool Result = doConditionalTempSinking(
      getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
      getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS());
  return Result;
}

PreservedAnalyses
HIRConditionalTempSinkingPass::run(llvm::Function &F,
                                   llvm::FunctionAnalysisManager &AM) {
  doConditionalTempSinking(AM.getResult<HIRFrameworkAnalysis>(F),
                           AM.getResult<HIRLoopStatisticsAnalysis>(F));
  return PreservedAnalyses::all();
}

void HIRConditionalTempSinkingLegacyPass::releaseMemory() {}
