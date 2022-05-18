//===----- HIRSSADeconstruction.cpp - Deconstructs SSA for HIR ------------===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file Deconstructs SSA for HIR. It inserts copies for livein/liveout
// values to SCC and non-SCC phis. Metadata nodes are attached to the
// livein/liveout copies, to the SCC nodes and non-SCC phi node. The livein
// copies are assigned the same metadata kind node as the root/phi node so that
// they can all be assigned the same symbase by ScalarSymbaseAssignment pass.
//
// Liveout copies require metadata to indicate to ScalarEvolution analysis to
// not trace through them.
//
// Non-phi SCC nodes also require metadata to indicate to ScalarEvolution
// analysis to not trace through them because that can cause live-range
// violations. A different live range metadata type is used for these nodes.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRSSADeconstructionPass.h"

#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"

#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Metadata.h"

#include "llvm/Support/CommandLine.h"

#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRRegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRSCCFormation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/IRRegion.h"

#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"

#define DEBUG_TYPE "hir-ssa-deconstruction"

using namespace llvm;
using namespace llvm::loopopt;

namespace {

class HIRSSADeconstruction {
public:
  HIRSSADeconstruction() : ModifiedIR(false), NamingCounter(0) {}

  bool run(Function &F, DominatorTree &DT, LoopInfo &LI,
           HIRRegionIdentification &RI, HIRSCCFormation &SCCF);

private:
  /// \brief Attaches a string metadata node to instruction. This will be used
  /// by ScalarSymbaseAssignment to assign symbases. The metadata kind used for
  /// livein/liveout values is different because livein copies need to be
  /// assigned the same aymbase as other values in SCC whereas liveout copies
  /// don't. Live range type is used to indicate live range violation and
  /// suppress traceback during SCEV creation.
  void attachMetadata(Instruction *Inst, StringRef Name,
                      ScalarEvolution::HIRLiveKind Kind) const;

  /// \brief Returns a copy of Val.
  Instruction *createCopy(Value *Val, StringRef Name, bool IsLivein,
                          Module *M) const;

  /// Returns true if Inst is a livein copy for IV update: i = i + 1.
  bool isIVUpdateLiveInCopy(Instruction *Inst) const;

  /// \brief Inserts livein copy of Val at the end of BB.
  void insertLiveInCopy(Value *Val, BasicBlock *BB, StringRef Name);

  /// \brief Inserts liveout copy of Inst at the first insertion point of BB if
  /// Inst is a phi or immediately after Inst.
  Instruction *insertLiveOutCopy(Instruction *Inst, BasicBlock *BB,
                                 StringRef Name);

  /// \brief Constructs name to be used for Val.
  StringRef constructName(const Value *Val, SmallString<32> &Name);

  /// \brief Returns the SCC this phi belongs to, if any, otherwise returns
  /// null.
  const HIRSCCFormation::SCC *getPhiSCC(PHINode *Phi) const;

  /// \brief Returns true if OrigPredBB has an alternate reaching path to Phi
  /// other than the immediate successor. This means that adding a livein copy
  /// in PredBB may kill an SCC operand so we need to perform edge splitting.
  bool hasAlternatePathToPhi(const PHINode *Phi, const BasicBlock *OrigPredBB,
                             const BasicBlock *CurBB = nullptr) const;

  /// \brief Returns true if we need to split edge to insert a livein copy for
  /// this phi operand.
  bool edgeSplittingRequired(const PHINode *Phi,
                             const BasicBlock *PredBB) const;

  /// \brief Inserts copies of Phi operands livein to the SCC. If SCC is
  /// null, Phi is treated as a standalone phi and all operands are considered
  /// livein. Returns true if a livein copy was inserted.
  bool processPhiLiveins(PHINode *Phi, const HIRSCCFormation::SCC *ParSCC,
                         StringRef Name);

  /// \brief Returns true if we need to insert a liveout copy for this
  /// standalone phi.
  bool liveoutCopyRequired(const PHINode *StandAlonePhi) const;

  /// Returns true if Inst has non-SCEVable uses in ParentBB. Returns the last
  /// use in \p Inst.
  bool hasNonSCEVableUses(Instruction **Inst, BasicBlock *ParentBB) const;

  /// \brief Inserts copies of Inst if it has uses live outside the SCC and
  /// replaces the liveout uses with the copy. If SCC is null, Inst is
  /// treated as a standalone phi and this is needed to handle a special case
  /// described in the function definition.
  void processLiveouts(Instruction *Inst, const HIRSCCFormation::SCC *ParSCC,
                       StringRef Name);

  /// Returns true if this an SCC with a single non-header phi instruction.
  bool isSingleNonHeaderPhiSCC(Instruction *NonPhiInst,
                               const HIRSCCFormation::SCC &CurSCC) const;

  /// \brief Deconstructs phi by inserting copies.
  void deconstructPhi(PHINode *Phi);

  /// Processes liveouts instructions in the current region which are not part
  /// of any loop by creating a single operand phi copy of them in the region
  /// exit block.
  void processNonLoopRegionLiveouts();

  /// Splits region exit block at \p SplitPos for ease of liveout handling. It
  /// \p SplitPos is not specified, we split at the terminator.
  void splitNonLoopRegionExit(Instruction *SplitPos = nullptr);

  /// Does the following for regions containing non-loop blocks-
  /// 1) Split the entry block if required.
  /// 2) Split the exit block if required for liveout handling.
  /// 3) Create single operand phi copy of liveout instructions in non-loop
  /// blocks.
  void processNonLoopRegionBlocks();

  /// \brief Performs SSA deconstruction on the regions.
  void deconstructSSAForRegions();

private:
  DominatorTree *DT;
  LoopInfo *LI;
  HIRRegionIdentification *RI;
  ScopedScalarEvolution *ScopedSE;
  HIRSCCFormation *SCCF;

  bool ModifiedIR;
  unsigned NamingCounter;
  HIRRegionIdentification::iterator CurRegIt;
  SmallPtrSet<const HIRSCCFormation::SCC *, 32> ProcessedSCCs;
  // Stores new bblocks which are sometimes created during deconstruction.
  SmallPtrSet<BasicBlock *, 8> NewRegBBlocks;
};
} // namespace

PreservedAnalyses HIRSSADeconstructionPass::run(Function &F,
                                                FunctionAnalysisManager &AM) {
  HIRSSADeconstruction HSSAD;
  bool Modified = HSSAD.run(F, AM.getResult<DominatorTreeAnalysis>(F),
                            AM.getResult<LoopAnalysis>(F),
                            AM.getResult<HIRRegionIdentificationAnalysis>(F),
                            AM.getResult<HIRSCCFormationAnalysis>(F));

  if (!Modified) {
    return PreservedAnalyses::all();
  }

  PreservedAnalyses PA;
  PA.preserve<DominatorTreeAnalysis>();
  PA.preserve<LoopAnalysis>();
  PA.preserve<ScalarEvolutionAnalysis>();
  PA.preserve<HIRRegionIdentificationAnalysis>();
  PA.preserve<HIRSCCFormationAnalysis>();
  PA.preserve<AndersensAA>();
  PA.preserve<GlobalsAA>();
  return PA;
}

class HIRSSADeconstructionLegacyPass : public FunctionPass {
public:
  static char ID;

  HIRSSADeconstructionLegacyPass() : FunctionPass(ID) {
    initializeHIRSSADeconstructionLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    auto &RI = getAnalysis<HIRRegionIdentificationWrapperPass>().getRI();

    if (skipFunction(F)) {
      // Since we are skipping deconstruction (in opt-bisect mode) the incoming
      // IR may not in the right form (consummable by HIR framework) which can
      // lead to assertion. We get around this issue by discarding all the
      // created regions.
      // Note: Pass is marked as required in new PM.
      RI.discardRegions();
      return false;
    }

    HIRSSADeconstruction HSSAD;
    return HSSAD.run(F, getAnalysis<DominatorTreeWrapperPass>().getDomTree(),
                     getAnalysis<LoopInfoWrapperPass>().getLoopInfo(), RI,
                     getAnalysis<HIRSCCFormationWrapperPass>().getSCCF());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<HIRRegionIdentificationWrapperPass>();
    AU.addRequired<HIRSCCFormationWrapperPass>();

    // We need to preserve all the analysis computed for HIR.
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addPreserved<LoopInfoWrapperPass>();
    AU.addPreserved<ScalarEvolutionWrapperPass>();
    AU.addPreserved<HIRRegionIdentificationWrapperPass>();
    AU.addPreserved<HIRSCCFormationWrapperPass>();
    AU.addPreserved<AndersensAAWrapperPass>();
    AU.addPreserved<GlobalsAAWrapperPass>();
  }
};

char HIRSSADeconstructionLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRSSADeconstructionLegacyPass, "hir-ssa-deconstruction",
                      "HIR SSA Deconstruction", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRRegionIdentificationWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSCCFormationWrapperPass)
INITIALIZE_PASS_END(HIRSSADeconstructionLegacyPass, "hir-ssa-deconstruction",
                    "HIR SSA Deconstruction", false, false)

FunctionPass *llvm::createHIRSSADeconstructionLegacyPass() {
  return new HIRSSADeconstructionLegacyPass();
}

void HIRSSADeconstruction::attachMetadata(
    Instruction *Inst, StringRef Name,
    ScalarEvolution::HIRLiveKind Kind) const {

  MDNode *Node = nullptr;

  if (Name.empty()) {
    Node = MDNode::get(Inst->getContext(), {});
  } else {
    Metadata *Args[] = {
        MDString::get(Inst->getContext(), (Name + ".de.ssa").str())};
    Node = MDNode::get(Inst->getContext(), Args);
  }

  Inst->setMetadata(ScopedSE->getHIRMDKindID(Kind), Node);
}

Instruction *HIRSSADeconstruction::createCopy(Value *Val, StringRef Name,
                                              bool IsLivein, Module *M) const {
  Type *Ty = Val->getType();
  Function *SSACopyFunc = Intrinsic::getDeclaration(M, Intrinsic::ssa_copy, Ty);
  auto CInst = CallInst::Create(FunctionCallee(SSACopyFunc), {Val},
                                Name + (IsLivein ? ".in" : ".out"));

  // Copy available DebugLoc metadata
  if (const auto *Inst = dyn_cast<Instruction>(Val)) {
    CInst->setDebugLoc(Inst->getDebugLoc());
  }

  attachMetadata(CInst, IsLivein ? Name : "",
                 IsLivein ? ScalarEvolution::HIRLiveKind::LiveIn
                          : ScalarEvolution::HIRLiveKind::LiveOut);
  return CInst;
}

bool HIRSSADeconstruction::isIVUpdateLiveInCopy(Instruction *Inst) const {

  if (!isa<CallInst>(Inst) || !ScopedSE->isSCEVable(Inst->getType())) {
    return false;
  }

  if (!ScopedSE->getHIRMetadata(Inst, ScalarEvolution::HIRLiveKind::LiveIn)) {
    return false;
  }

  auto ParentBB = Inst->getParent();
  auto ParentLp = LI->getLoopFor(ParentBB);

  // We are only interested in the IV update copy in the loop latch.
  if (!ParentLp || (ParentLp->getLoopLatch() != ParentBB)) {
    return false;
  }

  auto *Phi = dyn_cast<PHINode>(Inst->getOperand(0));
  // IV update instruction is typically something like an add/sub/gep etc.
  // Header phis as livein copy operand indicate phi dependency and treating
  // them as IV update can result in incorrect deconstruction.
  if (Phi && RI->isHeaderPhi(Phi)) {
    return false;
  }

  return SCCF->isConsideredLinear(Inst);
}

void HIRSSADeconstruction::insertLiveInCopy(Value *Val, BasicBlock *BB,
                                            StringRef Name) {
  auto TermInst = BB->getTerminator();
  auto CopyInst = createCopy(Val, Name, true, TermInst->getModule());

  auto InsertionPoint = TermInst->getIterator();

  // We need to keep IV update copies last in the bblock or we may encounter a
  // live-range issue when IV is parsed as a blob in one of the non-linear
  // values.
  // The following loop moves the insertion point to point to first IV update
  // copy.
  for (auto FirstInst = BB->begin(); InsertionPoint != FirstInst;) {
    auto PrevInst = std::prev(InsertionPoint);

    if (!isIVUpdateLiveInCopy(&*PrevInst)) {
      break;
    }

    InsertionPoint = PrevInst;
  }

  CopyInst->insertBefore(&*InsertionPoint);

  ModifiedIR = true;
}

Instruction *HIRSSADeconstruction::insertLiveOutCopy(Instruction *Inst,
                                                     BasicBlock *BB,
                                                     StringRef Name) {
  auto CopyInst = createCopy(Inst, Name, false, Inst->getModule());

  if (isa<PHINode>(Inst)) {
    CopyInst->insertBefore(&*(BB->getFirstInsertionPt()));
  } else {
    CopyInst->insertAfter(Inst);
  }

  ModifiedIR = true;

  return CopyInst;
}

const HIRSCCFormation::SCC *
HIRSSADeconstruction::getPhiSCC(PHINode *Phi) const {
  for (auto SCCIt = SCCF->begin(CurRegIt), E = SCCF->end(CurRegIt); SCCIt != E;
       ++SCCIt) {

    // Present in this SCC.
    if (SCCIt->contains(Phi)) {
      return &(*SCCIt);
    }
  }

  return nullptr;
}

// If this phi is used in another phi in the same basic block, then we can
// potentially have ordering issues with the insertion of livein copies
// for the phis. This is because the use of phi operands is deemed to
// occur on the edge of the basic block which means that the 'use' takes
// the value from the previous execution of the bblock, not the merged
// value in the current basic block.
//
// Let us consider the example below.
//
// for.body:              ; preds = %entry, %for.body
//   %b.addr.08 = phi i32 [ %c.addr.09, %for.body ], [ %b, %entry ]
//   %a.addr.07 = phi i32 [ %b.addr.08, %for.body ], [ %a, %entry ]
//   ...
//   br i1 %exitcond, label %for.end, label %for.body
//
// After inserting livein copies for the two phis, the basic block would
// look like this-
//
// for.body:              ; preds = %entry, %for.body
//   %b.addr.08 = phi i32 [ %c.addr.09, %for.body ], [ %b, %entry ]
//   !in.de.ssa
//   %a.addr.07 = phi i32 [ %b.addr.08, %for.body ], [ %a, %entry ]
//   !in.de.ssa
//   ...
//   %b.addr.08.in = %c.addr.09 !in.de.ssa
//   %a.addr.07.in = %b.addr.08 !in.de.ssa
//   br i1 %exitcond, label %for.end, label %for.body
//
// The livein copies %a.addr.07.in and %b.addr.08.in would be
// assigned the same symbase as %a.addr.07 and %b.addr.08, repectively.
// This means that the value of %a.addr.07 after the execution of the
// basic block would be the updated value of %b.addr.08 through the copy,
// which is wrong.
//
// To fix this problem, we create a liveout copy of %b.addr.08 so it
// looks like this-
//
// for.body:              ; preds = %entry, %for.body
//   %b.addr.08 = phi i32 [ %c.addr.09, %for.body ], [ %b, %entry ]
//   !in.de.ssa
//   %a.addr.07 = phi i32 [ %b.addr.08, %for.body ], [ %a, %entry ]
//   !in.de.ssa
//   %b.addr.08.out = %b.addr.08 !out.de.ssa
//   ...
//   %b.addr.08.in = %c.addr.09 !in.de.ssa
//   %a.addr.07.in = %b.addr.08.out !in.de.ssa
//   br i1 %exitcond, label %for.end, label %for.body
//
// Note that reordering the livein copies which produces a cleaner HIR works in
// some cases but cannot resolve phi cycles. In comparison, adding a liveout
// copy always works. Looking for a cycle would take more compile time so this
// seems like an acceptable solution. If a cleaner HIR is desired we can
// possibly get HIRSCCFormation to provide the cycle information.
//
// Here's an example of a phi cycle-
//
// for(i=0; i<n; i++) {
//   A[i] = a;
//   t = a;
//   a = b;
//   b = c;
//   c = t;
// }
//
// A phi can be indirectly used in another phi so we need to check the SCEV for
// SCEVable phis.
//
bool HIRSSADeconstruction::liveoutCopyRequired(
    const PHINode *StandAlonePhi) const {

  const Value *PhiVal = StandAlonePhi;
  bool SCEVablePhi = ScopedSE->isSCEVable(PhiVal->getType());

  const SCEV *PhiSCEV =
      SCEVablePhi ? ScopedSE->getUnknown(const_cast<Value *>(PhiVal)) : nullptr;

  // If the 'user' phi occurs before definition phi the copies are inserted
  // in the correct order (on the assumption that we traverse the bblock
  // instructions in order) so we only need to check phis which occur after this
  // one.
  for (auto InstIt = BasicBlock::const_iterator(StandAlonePhi),
            EndIt = StandAlonePhi->getParent()->end();
       InstIt != EndIt; ++InstIt) {

    auto UserPhi = dyn_cast<PHINode>(InstIt);

    if (!UserPhi) {
      break;
    }

    // Check usage of StandAlonePhi in UserPhi operands.
    for (unsigned I = 0, E = UserPhi->getNumIncomingValues(); I != E; ++I) {
      auto UserPhiOp = UserPhi->getIncomingValue(I);
      auto UserPhiOpInst = dyn_cast<Instruction>(UserPhiOp);

      if (!UserPhiOpInst || (LI->getLoopFor(UserPhiOpInst->getParent()) !=
                             LI->getLoopFor(StandAlonePhi->getParent()))) {
        continue;
      }

      // Compare values.
      if (UserPhiOp == PhiVal) {
        return true;
      }

      // Check usage in SCEV.
      if (SCEVablePhi && ScopedSE->isSCEVable(UserPhiOp->getType())) {
        auto SC = ScopedSE->getSCEV(UserPhiOp);
        if (ScopedSE->hasOperand(SC, PhiSCEV)) {
          return true;
        }
      }
    }
  }

  return false;
}

bool HIRSSADeconstruction::hasNonSCEVableUses(Instruction **Inst,
                                              BasicBlock *ParentBB) const {

  auto CurInst = *Inst;
  auto UseBB = CurInst->getParent();

  if (UseBB == ParentBB) {
    // Cannot handle phis in the same bblock.
    if (isa<PHINode>(CurInst)) {
      return false;
    }

  } else if (auto SuccBB = ParentBB->getSingleSuccessor()) {

    // If the use is in a non-header phi in the single successor bblock,
    // consider it as being at the end of the bblock as the use takes place on
    // the edge.
    if (UseBB != SuccBB) {
      return false;
    }

    auto Phi = dyn_cast<PHINode>(CurInst);

    if (!Phi ||
        (ScopedSE->isSCEVable(Phi->getType()) && RI->isHeaderPhi(Phi))) {
      return false;
    }

    *Inst = ParentBB->getTerminator();
    return true;

  } else {
    // Give up on uses outside ParentBB.
    return false;
  }

  // If the instruction itself is non-SCEVable return true.
  // Compare instructions are propagated to their uses in if/select by parser so
  // they should be considered as SCEVable.
  // Note that we are only checking for commonly occuring non-scevable
  // instructions.
  if ((!isa<CmpInst>(CurInst) && !ScopedSE->isSCEVable(CurInst->getType())) ||
      isa<LoadInst>(CurInst) ||
      (isa<CallInst>(CurInst) && !isa<IntrinsicInst>(CurInst))) {
    return true;
  }

  // Give up if instruction has more than one use.
  if (!CurInst->hasOneUse()) {
    return false;
  }

  auto Use = cast<Instruction>(*(CurInst->user_begin()));

  // Point Inst to last use in bblock.
  *Inst = Use;

  return hasNonSCEVableUses(Inst, ParentBB);
}

static bool isLoopLiveOut(Instruction *Inst, Loop *Lp, LoopInfo *LI) {

  if (!Lp->contains(LI->getLoopFor(Inst->getParent()))) {
    return true;
  }

  auto *Phi = dyn_cast<PHINode>(Inst);

  // Trace single operand phis recursively because they are considered as the
  // same value in HIR.
  if (Phi && (Phi->getNumIncomingValues() == 1)) {

    for (auto *User : make_range(Phi->user_begin(), Phi->user_end())) {
      if (isLoopLiveOut(cast<Instruction>(User), Lp, LI)) {
        return true;
      }
    }
  }

  return false;
}

void HIRSSADeconstruction::processLiveouts(Instruction *Inst,
                                           const HIRSCCFormation::SCC *ParSCC,
                                           StringRef Name) {

  Instruction *CopyInst = nullptr;
  bool IgnoreUsesInsideLoop = false;
  bool CopyRequired = false;
  auto ParentBB = Inst->getParent();
  Loop *Lp = nullptr;

  // For standalone header phis we need to create liveout copies in two cases-
  // 1) If it is used in another phi in the same bblock all its uses need to be
  // replaced by the copy.
  // 2) The uses outside the current loop need to be replaced by the copy.
  if (!ParSCC) {
    auto Phi = cast<PHINode>(Inst);

    if (!RI->isHeaderPhi(Phi)) {
      return;
    }

    // Ignore uses for linear phis.
    IgnoreUsesInsideLoop = SCCF->isConsideredLinear(Phi);

    if (!IgnoreUsesInsideLoop) {
      CopyRequired = liveoutCopyRequired(Phi);
    }

    if (!CopyRequired) {
      Lp = LI->getLoopFor(ParentBB);

      ScopedSE->setBackedgeTakenCountLoop(Lp);
      // We give up on non-linear phis inside unknown loops because in some
      // cases the use can be in the bottom test of the loop and can cause live
      // range violation.
      IgnoreUsesInsideLoop =
          IgnoreUsesInsideLoop ||
          !isa<SCEVCouldNotCompute>(ScopedSE->getBackedgeTakenCount(Lp));

      ScopedSE->resetBackedgeTakenCountLoop();
    }
  }

  for (auto UserIt = Inst->user_begin(), EndIt = Inst->user_end();
       UserIt != EndIt;) {
    assert(isa<Instruction>(*UserIt) && "Use is not an instruction!");
    auto UserInst = cast<Instruction>(*UserIt);

    Use &Us = UserIt.getUse();
    // Increment it before it gets invalidated later in the iteration.
    ++UserIt;

    // Handle a SCC instruction.
    if (ParSCC) {
      // Check if the use is outside SCC.
      if (ParSCC->contains(UserInst)) {
        continue;
      }

      auto LastUseInst = UserInst;
      // Ignore use if it can be proven to not cause live-range issues.
      // If the use is in a phi in the same bblock, it cannot be ignored.
      // Uses in same bblock can be ignored for non-SCEVable types or
      // non-SCEVable uses if another SCC instruction doesn't occurs between def
      // and use.
      if (hasNonSCEVableUses(&LastUseInst, ParentBB)) {

        bool LiveRangeViolation = false;

        for (auto It = std::next(Inst->getIterator()),
                  E = LastUseInst->getIterator();
             It != E; ++It) {
          if (ParSCC->contains(&*It)) {
            LiveRangeViolation = true;
            break;
          }
        }

        if (!LiveRangeViolation) {
          continue;
        }
      }

    } else if (!CopyRequired) {

      // Add a liveout copy if this phi is used outside its parent loop as these
      // uses can cause live range violation.
      if (IgnoreUsesInsideLoop && !isLoopLiveOut(UserInst, Lp, LI)) {
        continue;
      }
    }

    // Insert copy, if it doesn't exist.
    if (!CopyInst) {
      CopyInst = insertLiveOutCopy(Inst, ParentBB, Name);
    }

    // Replace liveout use by copy.
    Us.set(CopyInst);

    // Invalidate cached SCEV of the user, if any.
    ScopedSE->forgetValue(UserInst);
  }
}

bool HIRSSADeconstruction::hasAlternatePathToPhi(
    const PHINode *Phi, const BasicBlock *OrigPredBB,
    const BasicBlock *CurBB) const {

  auto PhiBB = Phi->getParent();
  auto PhiLp = LI->getLoopFor(PhiBB);
  assert(PhiLp && "Loop containing phi not found!");

  if (!CurBB) {
    CurBB = OrigPredBB;
  }

  for (auto SuccI = succ_begin(CurBB), E = succ_end(CurBB); SuccI != E;
       ++SuccI) {
    auto SuccBB = *SuccI;

    // We reached Phi, return true if this is a new path.
    if (SuccBB == PhiBB) {
      if (CurBB != OrigPredBB) {
        return true;
      }
      continue;
    }

    // Skip if we reach the loop header during the traversal to avoid cycling.
    if (PhiLp->getHeader() == SuccBB) {
      continue;
    }

    auto Lp = LI->getLoopFor(SuccBB);

    // Skip if we are outside any loop.
    if (!Lp) {
      continue;
    }

    if (Lp != PhiLp) {
      // If we reach an inner loop during traversal conservatively return true
      // as the analysis becomes difficult.
      if (PhiLp->contains(Lp)) {
        return true;
      }
      // Skip if we reach an outer loop.
      continue;
    }

    // Recurse on SuccBB.
    if (hasAlternatePathToPhi(Phi, OrigPredBB, SuccBB)) {
      return true;
    }
  }

  return false;
}

bool HIRSSADeconstruction::edgeSplittingRequired(
    const PHINode *Phi, const BasicBlock *PredBB) const {

  // Here's a IR snippet of a loop depicting a case where edge splitting is
  // required-
  //
  // while.body.126:                       ; preds = %entry, %if.end.150
  //   %0 = phi i32 [ 0, %entry ], [ %2, %if.end.150 ]
  //   %1 = load i32, i32* %arrayidx131, align 4
  //   br i1 %cmp, label %if.then.128, label %if.else.139
  //
  // if.then.128:                          ; preds = %while.body.126
  //   %cmp132 = icmp sgt i32 %0, %1
  //   br i1 %cmp132, label %if.end.150, label %if.then.134
  //
  // if.then.134:                          ; preds = %if.then.128
  //   store i32 %0, i32* %arrayidx131, align 4
  //   br label %if.end.150
  //
  // if.end.150:                           ; preds = %if.then.134, %if.then.128
  //   %2 = phi i32 [ %0, %if.then.134 ], [ %1, %if.then.128 ]
  //   %sub152 = sub nsw i32 0, %storemerge.149
  //   %tobool125 = icmp eq i32 %sub130, 0
  //   br i1 %tobool125, label %while.end.153.loopexit, label %while.body.126
  //
  //
  // We want to deconstruct phi %2 which is part of SCC containing %0 and %2 but
  // we cannot insert the livein copy for operand %1 in the predecessor
  // if.then.128 because that will kill %0 which flows through the same bblock.
  // %0 is killed because it is assigned the same symbase as the livein copy. To
  // resolve this we split the edge (if.then.128 -> if.end.150) and insert the
  // copy in the new bblock.

  return hasAlternatePathToPhi(Phi, PredBB);
}

bool HIRSSADeconstruction::processPhiLiveins(PHINode *Phi,
                                             const HIRSCCFormation::SCC *ParSCC,
                                             StringRef Name) {
  bool Ret = false;

  // Insert a copy in the predecessor bblock for each phi operand which
  // lies outside the SCC(livein values).
  for (unsigned I = 0, E = Phi->getNumIncomingValues(); I != E; ++I) {
    auto PredBB = Phi->getIncomingBlock(I);

    // Ignore if this value is region live-in.
    if (!CurRegIt->containsBBlock(PredBB) && !NewRegBBlocks.count(PredBB)) {
      continue;
    }

    auto PhiOp = Phi->getIncomingValue(I);

    if (ParSCC) {
      auto InstPhiOp = dyn_cast<Instruction>(PhiOp);

      // Ignore if InstPhiOp belongs to the same SCC.
      if (InstPhiOp && ParSCC->contains(InstPhiOp)) {
        continue;
      }

      // Split edge first, if required.
      if (edgeSplittingRequired(Phi, PredBB)) {
        PredBB = SplitCriticalEdge(
            PredBB, Phi->getParent(),
            CriticalEdgeSplittingOptions(DT, LI).setPreserveLCSSA());
        assert(PredBB &&
               "Could not split edge, SplitCriticalEdge() returned null!");

        // Add the new bblock to the set of new bblocks. It will be added to
        // the current region later.
        NewRegBBlocks.insert(PredBB);
      }
    }

    // Insert copy.
    insertLiveInCopy(PhiOp, PredBB, Name);
    Ret = true;
  }

  return Ret;
}

StringRef HIRSSADeconstruction::constructName(const Value *Val,
                                              SmallString<32> &Name) {
  raw_svector_ostream VOS(Name);

  if (Val->hasName()) {
    VOS << Val->getName();
  } else {
    // This string has to be unique across SCCs as it is used to assign
    // symbases. A longer name is used to avoid risk of name conflict with any
    // other instructions.
    VOS << "hir.de.ssa.copy" << NamingCounter++;
  }

  return VOS.str();
}

bool HIRSSADeconstruction::isSingleNonHeaderPhiSCC(
    Instruction *NonPhiInst, const HIRSCCFormation::SCC &CurSCC) const {

  // If the SCC has only two instructions, at least one of them is a header phi.
  if (CurSCC.size() == 2) {
    return true;
  }

  for (auto Inst : CurSCC) {
    auto Phi = dyn_cast<PHINode>(Inst);

    if (Phi) {
      if (!RI->isHeaderPhi(Phi)) {
        return false;
      }
    } else if ((Inst != NonPhiInst)) {
      return false;
    }
  }

  return true;
}

void HIRSSADeconstruction::deconstructPhi(PHINode *Phi) {
  SmallString<32> Name;

  // Phi is part of SCC
  if (auto PhiSCC = getPhiSCC(Phi)) {

    // Return if this SCC has been processed already.
    if (ProcessedSCCs.count(PhiSCC)) {
      return;
    }

    ProcessedSCCs.insert(PhiSCC);

    bool LiveinCopyInserted = false;
    bool NonPhiFound = false;
    bool ProcessNonPhiLiveouts = false;
    bool IsSCEVable = ScopedSE->isSCEVable(Phi->getType());

    constructName(PhiSCC->getRoot(), Name);

    for (auto SCCInst : *PhiSCC) {

      if (auto SCCPhiInst = dyn_cast<PHINode>(SCCInst)) {

        // Skip livein processing for single operand phis as the incoming value
        // has to belong to the SCC.
        if (SCCPhiInst->getNumIncomingValues() != 1) {

          LiveinCopyInserted =
              processPhiLiveins(SCCPhiInst, PhiSCC, Name.str()) ||
              LiveinCopyInserted;
        }

        processLiveouts(SCCPhiInst, PhiSCC, Name.str());

        if (IsSCEVable && !RI->isHeaderPhi(SCCPhiInst)) {
          // Attach live range type metadata to suppress SCEV traceback.
          attachMetadata(SCCPhiInst, "", ScalarEvolution::HIRLiveKind::LiveRange);
          // Tell SCEV to reparse the instruction.
          ScopedSE->forgetValue(SCCPhiInst);
        }

      } else {

        if (!NonPhiFound) {
          ProcessNonPhiLiveouts = !isSingleNonHeaderPhiSCC(SCCInst, *PhiSCC);
          NonPhiFound = true;
        }

        if (ProcessNonPhiLiveouts) {
          processLiveouts(SCCInst, PhiSCC, Name.str());
        }

        if (IsSCEVable) {
          // Attach live range type metadata to suppress SCEV traceback.
          attachMetadata(SCCInst, "", ScalarEvolution::HIRLiveKind::LiveRange);
          // Tell SCEV to reparse the instruction.
          ScopedSE->forgetValue(SCCInst);
        }
      }
    }

    if (LiveinCopyInserted) {
      // Attach metadata to the root node to connect the SCC to its livein
      // copies.
      attachMetadata(PhiSCC->getRoot(), Name.str(),
                     ScalarEvolution::HIRLiveKind::LiveIn);
    }

  } else {
    // This is a standalone phi such as the one which occurs at an if-else join.
    // Deconstruct all the operands.
    //
    // Shown below is an example of a standalone phi case where output will have
    // a phi at the if-else join.
    //
    // if (cond) {
    //   output = a;
    // }
    // else {
    //   output = b;
    // }
    //
    // In some cases the standalone phi can occur in loop headers as well.
    // Example test case-
    //
    // for(i=0; i<n; i++) {
    //   A[i] = a;
    //   a = b;
    //   b = c;
    //   c += i;
    // }
    //

    // Standalone single operand phis with instruction operand do not need to be
    // processed.
    if ((Phi->getNumIncomingValues() == 1) &&
        isa<Instruction>(Phi->getOperand(0))) {
      return;
    }

    constructName(Phi, Name);

    // Attach metadata to Phi to connect it to its copies.
    attachMetadata(Phi, Name.str(), ScalarEvolution::HIRLiveKind::LiveIn);

    processPhiLiveins(Phi, nullptr, Name.str());
    processLiveouts(Phi, nullptr, Name.str());
  }
}

static IntrinsicInst *findRegionEntryIntrinsic(BasicBlock *BB) {

  // Walk backwards to find the last entry intrinsic in the block.
  for (auto &Inst : make_range(BB->rbegin(), BB->rend())) {
    auto *Intrin = dyn_cast<IntrinsicInst>(&Inst);

    if (!Intrin) {
      continue;
    }

    if (Intrin->getIntrinsicID() == Intrinsic::directive_region_entry) {
      return Intrin;
    }
  }

  return nullptr;
}

class SCEVInvalidator {
  ScalarEvolution &SE;
  BasicBlock *BB;

public:
  SCEVInvalidator(ScalarEvolution &SE, BasicBlock *BB) : SE(SE), BB(BB) {}

  bool follow(const SCEV *SC) {
    auto *UnknownSC = dyn_cast<SCEVUnknown>(SC);

    if (!UnknownSC) {
      return true;
    }

    auto *Inst = dyn_cast<Instruction>(UnknownSC->getValue());

    if (!Inst || (Inst->getParent() != BB)) {
      return false;
    }

    SE.forgetValue(Inst);

    return false;
  }

  bool isDone() const { return false; }
};

static void invalidateSCEVableInsts(ScalarEvolution &SE, Instruction *Inst) {
  if (!SE.isSCEVable(Inst->getType())) {
    return;
  }

  SE.forgetValue(Inst);

  SCEVInvalidator Invalidator(SE, Inst->getParent());
  visitAll(SE.getSCEV(Inst), Invalidator);
}

void HIRSSADeconstruction::processNonLoopRegionLiveouts() {
  auto *ExitingBB = CurRegIt->getExitBBlock();
  auto *ExitBB = CurRegIt->getSuccBBlock();

  for (auto BBIt = CurRegIt->non_loop_bb_begin(),
            EndIt = CurRegIt->non_loop_bb_end();
       BBIt != EndIt; ++BBIt) {

    BasicBlock *BB = const_cast<BasicBlock *>(*BBIt);

    /// Replace liveouts uses of instructions by creating a single operand phi
    /// copy in the region exit block.
    for (Instruction &Inst : *BB) {
      PHINode *LiveoutPhi = nullptr;

      for (auto UserIt = Inst.user_begin(), EndIt = Inst.user_end();
           UserIt != EndIt;) {

        auto *UserInst = cast<Instruction>(*UserIt);

        Use &LiveoutUse = UserIt.getUse();
        // Increment explicitly as it may gets invalidated later in the
        // iteration.
        ++UserIt;

        if (!CurRegIt->containsBBlock(UserInst->getParent())) {

          // Create a single operand phi copy of Inst in the exit block.
          if (!LiveoutPhi) {
            ModifiedIR = true;

            LiveoutPhi = PHINode::Create(Inst.getType(), 1, "liveoutcopy",
                                         &*ExitBB->begin());
            LiveoutPhi->addIncoming(&Inst, ExitingBB);

            // Attach live range type metadata to suppress SCEV traceback.
            attachMetadata(LiveoutPhi, "",
                           ScalarEvolution::HIRLiveKind::LiveRange);

            invalidateSCEVableInsts(*ScopedSE, &Inst);
          }

          // Replace liveout use by single operand phi.
          LiveoutUse.set(LiveoutPhi);
        }
      }
    }
  }
}

void HIRSSADeconstruction::splitNonLoopRegionExit(Instruction *SplitPos) {
  auto *RegionExitBB = CurRegIt->getExitBBlock();

  // Split exit block to maintain single predecessor/successor relationship for
  // ease of liveout handling.

  // We can skip splitting if there are no successors.
  if (succ_begin(RegionExitBB) == succ_end(RegionExitBB)) {
    return;
  }

  auto *SuccessorBB = RegionExitBB->getSingleSuccessor();

  if (SplitPos || !SuccessorBB || !SuccessorBB->getSinglePredecessor()) {
    ModifiedIR = true;

    auto *NewSplitBB =
        SplitBlock(RegionExitBB,
                   SplitPos ? SplitPos : RegionExitBB->getTerminator(), DT, LI);

    if (SplitPos) {
      // Check if the exit block of this region is also the entry block of the
      // next region. If so, split the edge so we don't jump directly from one
      // region to another and also update next region's entry block.
      auto NextRegIt = std::next(CurRegIt);
      if ((NextRegIt != RI->end()) &&
          (NextRegIt->getEntryBBlock() == RegionExitBB)) {
        SplitEdge(RegionExitBB, NewSplitBB, DT, LI);
        NextRegIt->replaceEntryBBlock(NewSplitBB);
      }
    }
  }
}

void HIRSSADeconstruction::processNonLoopRegionBlocks() {

  auto *RegionEntryBB = CurRegIt->getEntryBBlock();

  if (CurRegIt->isFunctionLevel()) {
    // Split the function entry block which is also the region entry block.
    // Function entry block is used for inserting dummy instructions so it
    // cannot be part of the region.
    assert((RegionEntryBB == &RegionEntryBB->getParent()->getEntryBlock()) &&
           "function level region's entry block is expected to be the same as "
           "function entry block!");
    auto *NewEntryBB =
        SplitBlock(RegionEntryBB, RegionEntryBB->getTerminator(), DT, LI);
    CurRegIt->replaceEntryBBlock(NewEntryBB);

    ModifiedIR = true;
    return;
  }

  if (!CurRegIt->hasNonLoopBBlocks()) {
    return;
  }

  if (CurRegIt->isLoopMaterializationCandidate()) {
    ModifiedIR = true;

    // Always split entry block of loop materialization candidate to avoid
    // cross-region code generation complications. For example, this bblock may
    // be an early exit block of the loop which is in another region.
    auto *NewEntryBB =
        SplitBlock(RegionEntryBB, RegionEntryBB->getFirstNonPHI(), DT, LI);
    CurRegIt->replaceEntryBBlock(NewEntryBB);

    // If the terminator instruction is a conditinal branch and the condition is
    // the previous instruction, split the bblock at the condition. The
    // condition may be a ztt for a loop. If we don't move the condition along
    // with the branch, we will lose the ztt recognition due to live range
    // metadata suppressing traceback of liveout values.
    Instruction *SplitPos = nullptr;
    auto *ExitBB = CurRegIt->getExitBBlock();
    auto *TermInst = ExitBB->getTerminator();

    auto *BrInst = dyn_cast<BranchInst>(TermInst);
    if (BrInst && BrInst->isConditional()) {
      auto *CondInst = dyn_cast<Instruction>(BrInst->getCondition());

      if (CondInst && (CondInst->getNextNode() == TermInst)) {
        SplitPos = CondInst;
      }
    }

    splitNonLoopRegionExit(SplitPos);

  } else if (auto *RegionEntryIntrin =
                 findRegionEntryIntrinsic(RegionEntryBB)) {
    // Look for region entry intrinsic in the entry bblock and split the bblock
    // starting at that instruction if-
    // 1) It is not the first bblock instruction, Or
    // 2) The region entry block is also the function entry block, Or
    // 3) Entry bblock is the same as previous region's successor bblock.

    if ((RegionEntryIntrin != &(*RegionEntryBB->begin())) ||
        (RegionEntryBB == &RegionEntryBB->getParent()->getEntryBlock()) ||
        ((CurRegIt != RI->begin()) &&
         (std::prev(CurRegIt)->getSuccBBlock() == RegionEntryBB))) {
      auto *NewEntryBB = SplitBlock(RegionEntryBB, RegionEntryIntrin, DT, LI);
      CurRegIt->replaceEntryBBlock(NewEntryBB);

      ModifiedIR = true;
    }

    // Split the exit bblock after region exit intrinsic.
    auto *RegionExitIntrin =
        cast<Instruction>(*(RegionEntryIntrin->user_begin()));

    splitNonLoopRegionExit(RegionExitIntrin->getNextNode());

  } else {
    // Region created for fusion.

    // Exit is from the loop latch so we split the exiting edge instead of
    // splitting the exit block.
    auto *SuccessorBB = CurRegIt->getSuccBBlock();

    if (!SuccessorBB->getSinglePredecessor()) {
      SplitEdge(CurRegIt->getExitBBlock(), SuccessorBB, DT, LI);
      ModifiedIR = true;
    }
  }

  processNonLoopRegionLiveouts();
}

void HIRSSADeconstruction::deconstructSSAForRegions() {

  // Traverse regions.
  for (auto RegIt = RI->begin(), EndIt = RI->end(); RegIt != EndIt; ++RegIt) {

    // Set current region
    CurRegIt = RegIt;

    ScopedSE->setScope(RegIt->getOutermostLoops());

    processNonLoopRegionBlocks();

    // Traverse region basic blocks.
    for (auto BBIt = RegIt->bb_begin(), EndBBIt = RegIt->bb_end();
         BBIt != EndBBIt; ++BBIt) {

      for (auto Inst = (*BBIt)->begin(), EndI = (*BBIt)->end(); Inst != EndI;
           ++Inst) {
        auto Phi = dyn_cast<PHINode>(Inst);

        if (!Phi) {
          break;
        }

        deconstructPhi(const_cast<PHINode *>(Phi));
      }
    }

    // Add new bblocks created during deconstruction to the region.
    for (auto NewBB : NewRegBBlocks) {
      CurRegIt->addBBlock(NewBB);
    }

    NewRegBBlocks.clear();
  }
}

bool HIRSSADeconstruction::run(Function &F, DominatorTree &DT, LoopInfo &LI,
                               HIRRegionIdentification &RI,
                               HIRSCCFormation &SCCF) {
  this->DT = &DT;
  this->LI = &LI;
  this->RI = &RI;
  this->ScopedSE = &RI.getScopedSE();
  this->SCCF = &SCCF;

#if INTEL_PRODUCT_RELEASE
  // Set a flag to induce an error if anyone attempts to write the IR
  // to a file after this pass has been run.
  F.getParent()->setIntelProprietary();
#endif // INTEL_PRODUCT_RELEASE

  deconstructSSAForRegions();

  return ModifiedIR;
}
