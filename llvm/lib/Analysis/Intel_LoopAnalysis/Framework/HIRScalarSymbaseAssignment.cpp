//===- HIRScalarSymbaseAssignment.cpp - Assigns symbase to scalars --------===//
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
// This file implements the HIRScalarSymbaseAssignment pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRSCCFormation.h"

#include "HIRLoopFormation.h"
#include "HIRScalarSymbaseAssignment.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-scalar-symbase-assignment"

unsigned HIRScalarSymbaseAssignment::insertBaseTemp(const Value *Temp) {
  BaseTemps.push_back(Temp);

  return getMaxScalarSymbase();
}

void HIRScalarSymbaseAssignment::updateBaseTemp(unsigned Symbase,
                                                const Value *Temp,
                                                const Value **OldTemp) {

  // If a copy instruction was added as a base temp, we need to update it to
  // copy's associated phi. Using copies as base temps messes up (value -> base
  // value) mapping in parsing because ScalarEvolution can optimize away simple
  // copies such as t = 0 during simplification.

  auto BaseTemp = BaseTemps[getIndex(Symbase)];

  // Base is already a phi.
  if (isa<PHINode>(BaseTemp)) {
    return;
  }

  auto PhiBase = dyn_cast<PHINode>(Temp);

  if (!PhiBase) {
    return;
  }

  if (OldTemp) {
    *OldTemp = BaseTemp;
  }

  BaseTemps[getIndex(Symbase)] = PhiBase;
}

void HIRScalarSymbaseAssignment::insertTempSymbase(const Value *Temp,
                                                   unsigned Symbase) {
  assert((Symbase > GenericRvalSymbase) && (Symbase <= getMaxScalarSymbase()) &&
         "Symbase is out of range!");

  auto Ret = TempSymbaseMap.insert(std::make_pair(Temp, Symbase));
  (void)Ret;
  assert(Ret.second && "Attempt to overwrite Temp symbase!");
}

unsigned HIRScalarSymbaseAssignment::getTempSymbase(const Value *Temp) const {

  auto Iter = TempSymbaseMap.find(Temp);

  if (Iter != TempSymbaseMap.end()) {
    return Iter->second;
  }

  return InvalidSymbase;
}

unsigned HIRScalarSymbaseAssignment::assignTempSymbase(const Value *Temp) {
  auto Symbase = insertBaseTemp(Temp);
  insertTempSymbase(Temp, Symbase);

  return Symbase;
}

unsigned HIRScalarSymbaseAssignment::getOrAssignTempSymbase(const Value *Temp) {
  auto Symbase = getTempSymbase(Temp);

  if (Symbase == InvalidSymbase) {
    Symbase = assignTempSymbase(Temp);
  }

  return Symbase;
}

const Value *HIRScalarSymbaseAssignment::getBaseScalar(unsigned Symbase) const {
  assert((Symbase > GenericRvalSymbase) && "Symbase is out of range!");
  assert((Symbase <= getMaxScalarSymbase()) && "Symbase is out of range!");

  auto RetVal = BaseTemps[getIndex(Symbase)];

  assert(RetVal && "Unexpected null value for symbase!");
  return RetVal;
}

unsigned HIRScalarSymbaseAssignment::getMaxScalarSymbase() const {
  return BaseTemps.size() + GenericRvalSymbase;
}

const Value *HIRScalarSymbaseAssignment::traceSingleOperandPhis(
    const Value *Scalar, const IRRegion &IRReg) const {

  auto PhiInst = dyn_cast<PHINode>(Scalar);

  while (PhiInst && (1 == PhiInst->getNumIncomingValues())) {

    // Do not trace back outside the region as it can lead to creation of
    // additional live-out values for another region which cannot be handled
    // correctly. Here's a simple example-
    //
    // ; Begin Region 1
    //
    // L1:
    //   %t1 = phi [0, %t1']
    //   ...
    //   %t1' = add %t1, %t2
    //   ...
    //   br i1 cond %L1, %exit1
    //
    // ; End Region 1
    // exit1:
    //   %lcssa = phi [%t1']
    //   br label %L2
    //
    // ; Begin Region 2
    //
    // L2:
    //   %i1 = phi[%lcssa, %i1']
    //   ...
    //   br i1 cond %L2, %exit2
    //
    // ; End Region2
    //
    // In the above example %lcssa is livein to Region 2 but we shouldn't be
    // tracing it back to %t1' in Region 1 because the value %t1' is invalid
    // (converted to symbase) if we generate code for Region 1. Liveins are
    // always initialized using existing LLVM values.
    if (!IRReg.containsBBlock(PhiInst->getParent())) {
      break;
    }

    auto Op = PhiInst->getOperand(0);

    if (!isa<Instruction>(Op)) {
      break;
    }

    Scalar = Op;

    PhiInst = dyn_cast<PHINode>(Scalar);
  }

  return Scalar;
}

bool HIRScalarSymbaseAssignment::isConstant(const Value *Val) {
  // TODO: add other types
  if (isa<ConstantData>(Val) || isa<ConstantVector>(Val)) {
    return true;
  }

  return false;
}

MDString *
HIRScalarSymbaseAssignment::getInstMDString(const Instruction *Inst) const {
  // We only care about livein copies here because unlike liveout copies, livein
  // copies need to be assigned the same symbase as other values in the SCC.
  auto MDNode = SE.getHIRMetadata(Inst, ScalarEvolution::HIRLiveKind::LiveIn);

  if (!MDNode) {
    return nullptr;
  }

  assert((MDNode->getNumOperands() == 1) && "Invalid metadata node!");

  auto &MD = MDNode->getOperand(0);
  assert(isa<MDString>(MD) && "Invalid metadata node!");

  return cast<MDString>(MD);
}

unsigned HIRScalarSymbaseAssignment::getOrAssignScalarSymbaseImpl(
    const Value *Scalar, const IRRegion &IRReg, bool Assign,
    const Value **OldBaseScalar) {
  unsigned Symbase = InvalidSymbase;

  // TODO: assign constant symbase to metadata types as they do not cause data
  // dependencies.
  if (isConstant(Scalar)) {
    return ConstantSymbase;
  }

  Scalar = traceSingleOperandPhis(Scalar, IRReg);

  if (auto Inst = dyn_cast<Instruction>(Scalar)) {
    // First check if this instruction has metdadata attached by SSA
    // deconstruction pass. If so, we need to access/update StrSymbaseMap.
    auto MDStr = getInstMDString(Inst);

    if (MDStr) {
      auto StrRef = MDStr->getString();
      auto It = StrSymbaseMap.find(StrRef);

      if (It != StrSymbaseMap.end()) {
        Symbase = It->getValue();
        updateBaseTemp(Symbase, Inst, OldBaseScalar);

      } else {
        if (Assign) {
          Symbase = insertBaseTemp(Inst);
          StrSymbaseMap.insert(std::make_pair(StrRef, Symbase));
        }
      }

    } else {
      // No metadata attached, look it up in underlying TempSymbase map.
      Symbase = Assign ? getOrAssignTempSymbase(Inst) : getTempSymbase(Inst);
    }

  } else {
    // Not an instruction, look it up in underlying TempSymbase map.
    Symbase = Assign ? getOrAssignTempSymbase(Scalar) : getTempSymbase(Scalar);
  }

  return Symbase;
}

unsigned HIRScalarSymbaseAssignment::getOrAssignScalarSymbase(
    const Value *Scalar, const IRRegion &IRReg, const Value **OldBaseScalar) {
  return getOrAssignScalarSymbaseImpl(Scalar, IRReg, true, OldBaseScalar);
}

unsigned HIRScalarSymbaseAssignment::getScalarSymbase(const Value *Scalar,
                                                      const IRRegion &IRReg) {
  return getOrAssignScalarSymbaseImpl(Scalar, IRReg, false, nullptr);
}

void HIRScalarSymbaseAssignment::handleLoopExitLiveoutPhi(
    const PHINode *Phi, unsigned Symbase) const {

  // Checks if phi is in the loop exit bblock. The deconstructed definition
  // lies inside the loop which makes it liveout of the loop. This most likely
  // happens for multi-exit loops but can also happen for single-exit loops if
  // the single opternd phi operand is not an instruction.
  //
  // Example-
  //
  // loop:
  //    %t1.in = 0                    <<< deconstructed definition
  // br %cond %loopexit, %looplatch
  //
  // looplatch:
  //    %t1.in1 = 1                   <<< deconstructed definition
  // br %cond %loopexit, %loop
  //
  // loopexit:
  //    %t1 = phi [ 1, %looplatch, 0, %loop ]

  if (!Phi) {
    return;
  }

  unsigned NumOperands = Phi->getNumIncomingValues();

  auto *DefLp = LI.getLoopFor(Phi->getParent());

  for (unsigned I = 0; I < NumOperands; ++I) {
    auto *PredLp = LI.getLoopFor(Phi->getIncomingBlock(I));

    if (PredLp && (PredLp != DefLp)) {
      assert((!DefLp || DefLp->contains(PredLp)) &&
             "Incoming IR is not in LCSSA form!");
      auto *PredLoop = LF.findHLLoop(PredLp);
      auto *DefLoop = LF.findHLLoop(DefLp);

      assert(PredLoop && "Could not find predecessor bblock's HLLoop!");

      do {
        PredLoop->addLiveOutTemp(Symbase);
        PredLoop = PredLoop->getParentLoop();
      } while (PredLoop != DefLoop);
    }
  }
}
#if INTEL_FEATURE_SHARED_SW_ADVANCED

void HIRScalarSymbaseAssignment::populateLoopLiveouts(const Instruction *Inst,
                                                      unsigned Symbase) const {

  Loop *Lp = LI.getLoopFor(Inst->getParent());
  HLLoop *DefLoop = Lp ? LF.findHLLoop(Lp) : nullptr;

  auto BaseScalar = getBaseScalar(Symbase);
  auto BaseInst = cast<Instruction>(BaseScalar);

  // BaseInst can be different from Inst if either Inst is part of SCC or Inst
  // is a single operand phi. Former case is handled in
  // populateLoopSCCPhiLiveouts(). This is for the latter case where BaseInst is
  // defined at a deeper level than Inst making it live out of inner loops.
  if (BaseInst != Inst) {

    Loop *BaseLp = LI.getLoopFor(BaseInst->getParent());
    HLLoop *BaseDefLoop = BaseLp ? LF.findHLLoop(BaseLp) : nullptr;

    // BaseInst for a single operand phi can be outside the current region in
    // which case there is nothing to mark as liveout.
    if (!BaseDefLoop) {
      return;
    }

    if (!DefLoop ||
        (BaseDefLoop->getNestingLevel() > DefLoop->getNestingLevel())) {
      DefLoop = BaseDefLoop;
    }
  }

  while (DefLoop) {
    DefLoop->addLiveOutTemp(Symbase);
    DefLoop = DefLoop->getParentLoop();
  }

  handleLoopExitLiveoutPhi(dyn_cast<PHINode>(Inst), Symbase);
}

void HIRScalarSymbaseAssignment::populateRegionLiveouts(
    HIRRegionIdentification::iterator RegIt) {
  // Traverse region basic blocks.
  for (auto BBIt = RegIt->bb_begin(), EndIt = RegIt->bb_end(); BBIt != EndIt;
       ++BBIt) {

    // Check if any instructions inside the basic blocks are live outside the
    // region.
    for (auto InstIt = (*BBIt)->begin(), EndI = (*BBIt)->end(); InstIt != EndI;
         ++InstIt) {

      auto *Inst = &*InstIt;

      if (SCCF.isRegionLiveOut(RegIt, Inst)) {
        auto Symbase = getOrAssignScalarSymbase(Inst, *RegIt);
        RegIt->addLiveOutTemp(Symbase, Inst);
        populateLoopLiveouts(Inst, Symbase);

        // If the single operand liveout phi's operand is an instruction from
        // outside the region, we need to mark it as region livein.
        auto *BaseInst =
            dyn_cast<Instruction>(traceSingleOperandPhis(Inst, *RegIt));

        if (BaseInst && (BaseInst != Inst) &&
            !RegIt->containsBBlock(BaseInst->getParent())) {
          RegIt->addLiveInTemp(Symbase, BaseInst);
        }
      }
    }
  }
}

bool HIRScalarSymbaseAssignment::processRegionPhiLivein(
    HIRRegionIdentification::iterator RegIt, const PHINode *Phi,
    unsigned Symbase) {
  bool Ret = false;

  // Check whether phi operands are live in to the region.
  for (unsigned I = 0, E = Phi->getNumIncomingValues(); I != E; ++I) {

    if (!RegIt->containsBBlock(Phi->getIncomingBlock(I))) {
      RegIt->addLiveInTemp(Symbase, Phi->getIncomingValue(I));

      Ret = true;
      break;
    }
  }

  return Ret;
}

void HIRScalarSymbaseAssignment::populateLoopSCCPhiLiveouts(
    const Instruction *SCCInst, unsigned Symbase, const IRRegion &IRReg) {

  // We need special logic to mark symbase as live out of the inner loops of the
  // SCC. This is non-trivial to do during parsing as it is not aware of def-use
  // cycle of SCCs. Liveins are handled correctly by the parser using the
  // root/base instruction mechanism.

  auto *Phi = dyn_cast<PHINode>(SCCInst);

  if (!Phi) {
    return;
  }

  auto ParentBB = SCCInst->getParent();

  Loop *Lp = LI.getLoopFor(ParentBB);
  assert(Lp && "SCC phi does not have parent loop!");

  HLLoop *DefLoop = LF.findHLLoop(Lp);

  // The loop may have been optimized away during redundant HLIf elimination in
  // HIRCleanup phase.
  if (!DefLoop) {
    return;
  }

  if (Phi->getNumIncomingValues() == 1) {

    // Since this is a SCC inst, the incoming value has to be an instruction.
    auto *IncomingInst = cast<Instruction>(traceSingleOperandPhis(Phi, IRReg));

    // Let SCC phi instruction be handled by another call to this function.
    if (isa<PHINode>(IncomingInst)) {
      return;
    }

    Loop *InnerLp = LI.getLoopFor(IncomingInst->getParent());
    assert(InnerLp && "SCC phi does not have parent loop!");
    assert(Lp->contains(InnerLp) && "Unexpected loop structure!");

    // Nothing to process.
    if (Lp == InnerLp) {
      return;
    }

    HLLoop *InnerDefLoop = LF.findHLLoop(InnerLp);
    // The loop may have been optimized away during redundant HLIf elimination
    // in HIRCleanup phase.
    if (!InnerDefLoop) {
      return;
    }

    while (InnerDefLoop != DefLoop) {
      InnerDefLoop->addLiveOutTemp(Symbase);
      InnerDefLoop = InnerDefLoop->getParentLoop();
    }

  } else if (ParentBB == Lp->getHeader()) {
    // Ignore non-header phis.

    // We found an inner SCC loop, mark symbase as live out of this loop.
    DefLoop->addLiveOutTemp(Symbase);
  }
}

void HIRScalarSymbaseAssignment::populateRegionPhiLiveins(
    HIRRegionIdentification::iterator RegIt) {
  // Traverse SCCs associated with the region.
  for (auto SCCIt = SCCF.begin(RegIt), EndIt = SCCF.end(RegIt); SCCIt != EndIt;
       ++SCCIt) {

    bool SCCLiveInProcessed = false;
    // This call sets SCC's root node as the base temp.
    unsigned Symbase = getOrAssignScalarSymbase(SCCIt->getRoot(), *RegIt);

    // Traverse SCC instructions
    for (auto SCCInstIt = SCCIt->begin(), EndIt = SCCIt->end();
         SCCInstIt != EndIt; ++SCCInstIt) {

      if ((*SCCInstIt) != SCCIt->getRoot()) {
        // Assign same symbase to all instructions in the SCC.
        insertTempSymbase(*SCCInstIt, Symbase);
        populateLoopSCCPhiLiveouts(*SCCInstIt, Symbase, *RegIt);
      }

      if (SCCLiveInProcessed) {
        continue;
      }

      auto SCCPhiInst = dyn_cast<PHINode>(*SCCInstIt);

      if (SCCPhiInst && processRegionPhiLivein(RegIt, SCCPhiInst, Symbase)) {
        SCCLiveInProcessed = true;
      }
    }
  }

  // Process phis in the entry bblock that are not part of any SCC.
  for (auto InstIt = RegIt->getEntryBBlock()->begin(),
            EndIt = RegIt->getEntryBBlock()->end();
       InstIt != EndIt && isa<PHINode>(InstIt); ++InstIt) {
    // Has been processed already?
    if (getTempSymbase(&*InstIt)) {
      continue;
    }

    processRegionPhiLivein(RegIt, cast<PHINode>(InstIt),
                           getOrAssignScalarSymbase(&*InstIt, *RegIt));
  }
}
#endif // INTEL_FEATURE_SHARED_SW_ADVANCED

inline unsigned HIRScalarSymbaseAssignment::getIndex(unsigned Symbase) const {
  return Symbase - GenericRvalSymbase - 1;
}

#if INTEL_FEATURE_SHARED_SW_ADVANCED
void HIRScalarSymbaseAssignment::run() {
  Func = &HNU.getFunction();

  for (auto RegIt = RI.begin(), EndRegIt = RI.end(); RegIt != EndRegIt;
       ++RegIt) {
    populateRegionPhiLiveins(RegIt);
    populateRegionLiveouts(RegIt);
  }
}
#endif // INTEL_FEATURE_SHARED_SW_ADVANCED

void HIRScalarSymbaseAssignment::print(raw_ostream &OS) const {

  auto RegBegin = RI.begin();

  for (auto RegIt = RI.begin(), EndRegIt = RI.end(); RegIt != EndRegIt;
       ++RegIt) {
    OS << "\nRegion " << (RegIt - RegBegin + 1);

    OS << "\n   Phi LiveIns: ";
    for (auto LiveInIt = RegIt->live_in_begin(), EndIt = RegIt->live_in_end();
         LiveInIt != EndIt; ++LiveInIt) {

      if (LiveInIt != RegIt->live_in_begin()) {
        OS << ", ";
      }

      getBaseScalar(LiveInIt->first)->printAsOperand(OS, false);
      OS << "(";
      LiveInIt->second->printAsOperand(OS, false);
      OS << ")";
    }

    OS << "\n   LiveOuts: ";
    bool FirstLiveout = true;
    for (auto LiveOutIt = RegIt->live_out_begin(),
              EndIt = RegIt->live_out_end();
         LiveOutIt != EndIt; ++LiveOutIt) {

      for (auto Inst : LiveOutIt->second) {
        if (FirstLiveout) {
          OS << ", ";
        }
        Inst->printAsOperand(OS, false);
        FirstLiveout = false;
      }
    }

    OS << "\n";
  }
}

const Loop *
HIRScalarSymbaseAssignment::getDeepestSCCLoop(const Instruction *BaseInst,
                                              const Loop *UseLoop,
                                              const IRRegion &IRReg) const {
  assert(UseLoop && "UseLoop is null!");

  auto CurRegIt = RI.begin();
  bool Found = false;

  // Unfortunately, SCCs are only available through region iterators.
  // This is possibly because SCCFormation data structures were over-engineered.
  for (auto EndRegIt = RI.end(); CurRegIt != EndRegIt; ++CurRegIt) {
    if (&*CurRegIt == &IRReg) {
      Found = true;
      break;
    }
  }

  assert(Found && "Region iterator not found!");

  // Check if BaseInst belong to one of the region SCCs. If not, we return
  // nullptr.
  Found = false;
  auto CurSCCIt = SCCF.begin(CurRegIt);

  for (auto EndIt = SCCF.end(CurRegIt); CurSCCIt != EndIt; ++CurSCCIt) {
    if (CurSCCIt->getRoot() == BaseInst) {
      Found = true;
      break;
    }
  }

  if (!Found) {
    return nullptr;
  }

  // Iterate through all SCC insts and set the deepest loop w.r.t UseLoop.
  Loop *DeepestLoop = nullptr;
  for (auto SCCInstIt = CurSCCIt->begin(), EndIt = CurSCCIt->end();
       SCCInstIt != EndIt; ++SCCInstIt) {
    auto *CurLoop = LI.getLoopFor((*SCCInstIt)->getParent());
    assert(CurLoop && "Cannot find loop of scc inst!");

    if (!DeepestLoop || DeepestLoop->contains(CurLoop) ||
        // This is the case where CurLoop and DeepestLoop are sibling loops but
        // CurLoop is the parent loop of UseLoop.
        (CurLoop->contains(UseLoop) && !DeepestLoop->contains(UseLoop))) {
      DeepestLoop = CurLoop;
    }
  }

  return DeepestLoop;
}
