//===----- HIRSSADeconstruction.cpp - Deconstructs SSA for HIR ------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

#include "llvm/Pass.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"

#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Metadata.h"

#include "llvm/IR/Intel_LoopIR/IRRegion.h"

#include "llvm/Support/CommandLine.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRRegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRSCCFormation.h"

#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"

#define DEBUG_TYPE "hir-ssa-deconstruction"

using namespace llvm;
using namespace llvm::loopopt;

namespace {

class HIRSSADeconstruction : public FunctionPass {
public:
  static char ID;

  HIRSSADeconstruction()
      : FunctionPass(ID), ModifiedIR(false), NamingCounter(0) {
    initializeHIRSSADeconstructionPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<HIRRegionIdentification>();
    AU.addRequired<HIRSCCFormation>();

    // We need to preserve all the analysis computed for HIR.
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addPreserved<LoopInfoWrapperPass>();
    AU.addPreserved<ScalarEvolutionWrapperPass>();
    AU.addPreserved<HIRRegionIdentification>();
    AU.addPreserved<HIRSCCFormation>();
  }

  void releaseMemory() override {
    ModifiedIR = false;
    NamingCounter = 0;
    ProcessedSCCs.clear();
  }

private:
  enum MetadataType {
    LiveInType,
    LiveOutType,
    LiveRangeType,
  };

  /// \brief Attaches a string metadata node to instruction. This will be used
  /// by ScalarSymbaseAssignment to assign symbases. The metadata kind used for
  /// livein/liveout values is different because livein copies need to be
  /// assigned the same aymbase as other values in SCC whereas liveout copies
  /// don't. Live range type is used to indicate live range violation and
  /// suppress traceback during SCEV creation.
  void attachMetadata(Instruction *Inst, StringRef Name,
                      MetadataType MType) const;

  /// \brief Returns a copy of Val.
  Instruction *createCopy(Value *Val, StringRef Name, bool IsLivein) const;

  /// \brief Inserts copy of Val at the end of BB.
  void insertCopyAsLastInst(Value *Val, BasicBlock *BB, StringRef Name);

  /// \brief Inserts copy of Inst at the first insertion point of BB.
  Instruction *insertCopyAsFirstInst(Instruction *Inst, BasicBlock *BB,
                                     StringRef Name);

  /// \brief Constructs name to be used for Val.
  StringRef constructName(const Value *Val, SmallString<32> &Name);

  /// \brief Returns the SCC this phi belongs to, if any, otherwise returns
  /// null.
  HIRSCCFormation::SCCTy *getPhiSCC(const PHINode *Phi) const;

  /// \brief Returns true if OrigPredBB has an alternate reaching path to Phi
  /// other than the immediate successor. This means that adding a livein copy
  /// in PredBB may kill an SCC operand so we need to perform edge splitting.
  bool hasAlternatePathToPhi(const PHINode *Phi, const BasicBlock *OrigPredBB,
                             const BasicBlock *CurBB = nullptr) const;

  /// \brief Returns true if we need to split edge to insert a livein copy for
  /// this phi operand.
  bool edgeSplittingRequired(const PHINode *Phi,
                             const BasicBlock *PredBB) const;

  /// \brief Inserts copies of Phi operands livein to the SCC. If SCCNodes is
  /// null, Phi is treated as a standalone phi and all operands are considered
  /// livein. Returns true if a livein copy was inserted.
  bool processPhiLiveins(PHINode *Phi,
                         const HIRSCCFormation::SCCNodesTy *SCCNodes,
                         StringRef Name);

  /// \brief Returns true if we need to insert a liveout copy for this
  /// standalone phi.
  bool liveoutCopyRequired(const PHINode *StandAlonePhi) const;

  /// \brief Inserts copies of Phi if it has uses live outside the SCC and
  /// replaces the liveout uses with the copy. If SCCNodes is null, Phi is
  /// treated as a standalone phi and this is needed to handle a special case
  /// described in the function definition.
  void processPhiLiveouts(PHINode *Phi, HIRSCCFormation::SCCNodesTy *SCCNodes,
                          StringRef Name);

  /// \brief Deconstructs phi by inserting copies.
  void deconstructPhi(PHINode *Phi);

  /// \brief Performs SSA deconstruction on the regions.
  void deconstructSSAForRegions();

private:
  DominatorTree *DT;
  LoopInfo *LI;
  ScalarEvolution *SE;
  HIRRegionIdentification *RI;
  HIRSCCFormation *SCCF;

  bool ModifiedIR;
  unsigned NamingCounter;
  HIRRegionIdentification::iterator CurRegIt;
  SmallPtrSet<HIRSCCFormation::SCCTy *, 32> ProcessedSCCs;
};
}

char HIRSSADeconstruction::ID = 0;
INITIALIZE_PASS_BEGIN(HIRSSADeconstruction, "hir-ssa-deconstruction",
                      "HIR SSA Deconstruction", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRRegionIdentification)
INITIALIZE_PASS_DEPENDENCY(HIRSCCFormation)
INITIALIZE_PASS_END(HIRSSADeconstruction, "hir-ssa-deconstruction",
                    "HIR SSA Deconstruction", false, false)

FunctionPass *llvm::createHIRSSADeconstructionPass() {
  return new HIRSSADeconstruction();
}

void HIRSSADeconstruction::attachMetadata(Instruction *Inst, StringRef Name,
                                          MetadataType MType) const {
  Twine NewName(Name, ".de.ssa");

  Metadata *Args[] = {MDString::get(Inst->getContext(), NewName.str())};
  MDNode *Node = MDNode::get(Inst->getContext(), Args);

  if (MType == LiveInType) {
    Inst->setMetadata(HIR_LIVE_IN_STR, Node);
  } else if (MType == LiveOutType) {
    Inst->setMetadata(HIR_LIVE_OUT_STR, Node);
  } else {
    Inst->setMetadata(HIR_LIVE_RANGE_STR, Node);
  }
}

Instruction *HIRSSADeconstruction::createCopy(Value *Val, StringRef Name,
                                              bool IsLivein) const {
  Twine NewName(Name, (IsLivein ? ".in" : ".out"));

  auto CInst =
      CastInst::Create(Instruction::BitCast, Val, Val->getType(), NewName);

  attachMetadata(CInst, Name, IsLivein ? LiveInType : LiveOutType);

  return CInst;
}

void HIRSSADeconstruction::insertCopyAsLastInst(Value *Val, BasicBlock *BB,
                                                StringRef Name) {
  // Create a copy.
  auto CopyInst = createCopy(Val, Name, true);

  // Insert at the end of BB.
  CopyInst->insertBefore(BB->getTerminator());

  // Indicate that we have modified the IR by inserting a copy.
  ModifiedIR = true;
}

Instruction *HIRSSADeconstruction::insertCopyAsFirstInst(Instruction *Inst,
                                                         BasicBlock *BB,
                                                         StringRef Name) {
  // Create a copy.
  auto CopyInst = createCopy(Inst, Name, false);

  // Insert at the first insertion point of BB.
  CopyInst->insertBefore(&*(BB->getFirstInsertionPt()));

  // Indicate that we have modified the IR by inserting a copy.
  ModifiedIR = true;

  return CopyInst;
}

HIRSCCFormation::SCCTy *
HIRSSADeconstruction::getPhiSCC(const PHINode *Phi) const {
  for (auto SCCIt = SCCF->begin(CurRegIt), E = SCCF->end(CurRegIt); SCCIt != E;
       ++SCCIt) {

    // Present in this SCC.
    if (((*SCCIt)->Nodes).count(Phi)) {
      return *SCCIt;
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

  if (SCCF->isConsideredLinear(StandAlonePhi)) {
    return false;
  }

  const Value *PhiVal = StandAlonePhi;
  bool SCEVablePhi = SE->isSCEVable(PhiVal->getType());

  const SCEV *PhiSCEV =
      SCEVablePhi ? SE->getUnknown(const_cast<Value *>(PhiVal)) : nullptr;

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
      if (SCEVablePhi && SE->isSCEVable(UserPhiOp->getType())) {
        auto SC = SE->getSCEV(UserPhiOp);
        if (SE->hasOperand(SC, PhiSCEV)) {
          return true;
        }
      }
    }
  }

  return false;
}

void HIRSSADeconstruction::processPhiLiveouts(
    PHINode *Phi, HIRSCCFormation::SCCNodesTy *SCCNodes, StringRef Name) {

  Instruction *CopyInst = nullptr;
  bool CopyRequired = false;
  Loop *Lp = nullptr;

  // For standalone header phis we need to create liveout copies in two cases-
  // 1) If it is used in another phi in the same bblock all its uses need to be
  // replaced by the copy.
  // 2) The uses outside the current loop need to be replaced by the copy.
  if (!SCCNodes) {
    if (!RI->isHeaderPhi(Phi)) {
      return;
    }

    if (!(CopyRequired = liveoutCopyRequired(Phi))) {
      Lp = LI->getLoopFor(Phi->getParent());
    }
  }

  for (auto UserIt = Phi->user_begin(), EndIt = Phi->user_end();
       UserIt != EndIt;) {
    assert(isa<Instruction>(*UserIt) && "Use is not an instruction!");
    auto UserInst = cast<Instruction>(*UserIt);

    Use &Us = UserIt.getUse();
    // Increment it before it gets invalidated later in the iteration.
    ++UserIt;

    // Handle a SCC phi.
    if (SCCNodes) {
      // Check if the use is outside SCC.
      if (SCCNodes->count(UserInst)) {
        continue;
      }

    } else if (!CopyRequired) {

      // Add a liveout copy if this phi is used outside its parent loop as these
      // uses can cause live range violation.
      if (Lp->contains(LI->getLoopFor(UserInst->getParent()))) {
        continue;
      }
    }

    // Insert copy, if it doesn't exist.
    if (!CopyInst) {
      CopyInst = insertCopyAsFirstInst(Phi, Phi->getParent(), Name);
    }

    // Replace liveout use by copy.
    Us.set(CopyInst);

    // Invalidate cached SCEV of the user, if any.
    SE->forgetValue(UserInst);
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

bool HIRSSADeconstruction::processPhiLiveins(
    PHINode *Phi, const HIRSCCFormation::SCCNodesTy *SCCNodes, StringRef Name) {
  bool Ret = false;

  // Insert a copy in the predecessor bblock for each phi operand which
  // lies outside the SCC(livein values).
  for (unsigned I = 0, E = Phi->getNumIncomingValues(); I != E; ++I) {
    auto PredBB = Phi->getIncomingBlock(I);

    // Ignore if this value is region live-in.
    if (!(*CurRegIt)->containsBBlock(PredBB)) {
      continue;
    }

    auto PhiOp = Phi->getIncomingValue(I);

    if (SCCNodes) {
      auto InstPhiOp = dyn_cast<Instruction>(PhiOp);

      // Ignore if InstPhiOp belongs to the same SCC.
      if (InstPhiOp && SCCNodes->count(InstPhiOp)) {
        continue;
      }

      // Split edge first, if required.
      if (edgeSplittingRequired(Phi, PredBB)) {
        PredBB = SplitCriticalEdge(PredBB, Phi->getParent(),
                                   CriticalEdgeSplittingOptions(DT, LI));
        assert(PredBB &&
               "Could not split edge, SplitCriticalEdge() returned null!");

        // Add the new bblock to the current region.
        (*CurRegIt)->addBBlock(PredBB);
      }
    }

    // Insert copy.
    insertCopyAsLastInst(PhiOp, PredBB, Name);
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

    constructName(PhiSCC->Root, Name);

    for (auto const &SCCInst : PhiSCC->Nodes) {

      if (auto SCCPhiInst = dyn_cast<PHINode>(SCCInst)) {
        LiveinCopyInserted =
            processPhiLiveins(const_cast<PHINode *>(SCCPhiInst), &PhiSCC->Nodes,
                              Name.str()) ||
            LiveinCopyInserted;

        // TODO: Find a test case where a non-phi instruction in an SCC is live
        // outside the region and is not the lexically last statement of the
        // SCC. The current code doesn't handle this case.
        processPhiLiveouts(const_cast<PHINode *>(SCCPhiInst), &PhiSCC->Nodes,
                           Name.str());

      } else {

        // Attach live range type metadata to suppress SCEV traceback.
        attachMetadata(const_cast<Instruction *>(SCCInst), Name.str(),
                       LiveRangeType);
        // Tell SCEV to reparse the instruction.
        SE->forgetValue(const_cast<Instruction *>(SCCInst));
      }
    }

    if (LiveinCopyInserted) {
      // Attach metadata to the root node to connect the SCC to its livein
      // copies.
      attachMetadata(const_cast<Instruction *>(PhiSCC->Root), Name.str(),
                     LiveInType);
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

    // Single operand phis do not need to be deconstructed.
    if (1 == Phi->getNumIncomingValues()) {
      return;
    }

    constructName(Phi, Name);

    // Attach metadata to Phi to connect it to its copies.
    attachMetadata(Phi, Name.str(), LiveInType);

    processPhiLiveins(Phi, nullptr, Name.str());
    processPhiLiveouts(Phi, nullptr, Name.str());
  }
}

void HIRSSADeconstruction::deconstructSSAForRegions() {
  // Traverse regions.
  for (auto RegIt = RI->begin(), EndRegIt = RI->end(); RegIt != EndRegIt;
       ++RegIt) {

    // Set current region
    CurRegIt = RegIt;

    // Traverse region basic blocks.
    for (auto BBIt = (*RegIt)->bb_begin(), EndBBIt = (*RegIt)->bb_end();
         BBIt != EndBBIt; ++BBIt) {

      // Process instructions inside the basic blocks.
      for (auto Inst = (*BBIt)->begin(), EndI = (*BBIt)->end(); Inst != EndI;
           ++Inst) {
        if (auto Phi = dyn_cast<PHINode>(Inst)) {
          deconstructPhi(const_cast<PHINode *>(Phi));
        }
      }
    }
  }
}

bool HIRSSADeconstruction::runOnFunction(Function &F) {
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  RI = &getAnalysis<HIRRegionIdentification>();
  SCCF = &getAnalysis<HIRSCCFormation>();

  deconstructSSAForRegions();

  return ModifiedIR;
}
