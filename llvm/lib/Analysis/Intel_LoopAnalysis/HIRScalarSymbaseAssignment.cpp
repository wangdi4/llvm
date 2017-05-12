//===- HIRScalarSymbaseAssignment.cpp - Assigns symbase to scalars --------===//
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
// This file implements the HIRScalarSymbaseAssignment pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"

#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"

#include "llvm/IR/Intel_LoopIR/HLLoop.h"
#include "llvm/IR/Intel_LoopIR/IRRegion.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRLoopFormation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRSCCFormation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRScalarSymbaseAssignment.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-scalar-symbase-assignment"

INITIALIZE_PASS_BEGIN(HIRScalarSymbaseAssignment,
                      "hir-scalar-symbase-assignment",
                      "HIR Scalar Symbase Assignment", false, true)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRRegionIdentification)
INITIALIZE_PASS_DEPENDENCY(HIRSCCFormation)
INITIALIZE_PASS_DEPENDENCY(HIRLoopFormation)
INITIALIZE_PASS_END(HIRScalarSymbaseAssignment, "hir-scalar-symbase-assignment",
                    "HIR Scalar Symbase Assignment", false, true)

char HIRScalarSymbaseAssignment::ID = 0;

FunctionPass *llvm::createHIRScalarSymbaseAssignmentPass() {
  return new HIRScalarSymbaseAssignment();
}

HIRScalarSymbaseAssignment::HIRScalarSymbaseAssignment()
    : FunctionPass(ID), GenericRvalSymbase(0) {
  initializeHIRScalarSymbaseAssignmentPass(*PassRegistry::getPassRegistry());
}

void HIRScalarSymbaseAssignment::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<HIRRegionIdentification>();
  AU.addRequired<HIRSCCFormation>();
  AU.addRequired<HIRLoopFormation>();
}

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
  assert((Symbase > ConstantSymbase) && (Symbase <= getMaxScalarSymbase()) &&
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
  assert((Symbase > ConstantSymbase) && "Symbase is out of range!");
  assert((Symbase <= getMaxScalarSymbase()) && "Symbase is out of range!");

  auto RetVal = BaseTemps[getIndex(Symbase)];

  assert(RetVal && "Unexpected null value for symbase!");
  return RetVal;
}

unsigned HIRScalarSymbaseAssignment::getMaxScalarSymbase() const {
  return BaseTemps.size() + ConstantSymbase;
}

void HIRScalarSymbaseAssignment::initGenericRvalSymbase() {
  GenericRvalSymbase = assignTempSymbase(Func);
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
  auto MDNode = SE->getHIRMetadata(Inst, ScalarEvolution::HIRLiveKind::LiveIn);

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

void HIRScalarSymbaseAssignment::populateLoopLiveouts(const Instruction *Inst,
                                                      unsigned Symbase) const {

  Loop *Lp = LI->getLoopFor(Inst->getParent());
  HLLoop *DefLoop = Lp ? LF->findHLLoop(Lp) : nullptr;

  auto BaseScalar = getBaseScalar(Symbase);
  auto BaseInst = cast<Instruction>(BaseScalar);

  // BaseInst can be different from Inst if either Inst is part of SCC or Inst
  // is a single operand phi. Former case is handled in
  // populateLoopSCCPhiLiveouts(). This is for the latter case where BaseInst is
  // defined at a deeper level than Inst making it live out of inner loops.
  if (BaseInst != Inst) {
    Loop *BaseLp = LI->getLoopFor(BaseInst->getParent());
    assert(BaseLp && "Could not find base instruction's loop!");
    HLLoop *BaseDefLoop = LF->findHLLoop(BaseLp);
    assert(BaseDefLoop && "Could not find base instruction's HLLoop!");

    if (!DefLoop ||
        (BaseDefLoop->getNestingLevel() > DefLoop->getNestingLevel())) {
      DefLoop = BaseDefLoop;
    }
  }

  while (DefLoop) {
    DefLoop->addLiveOutTemp(Symbase);
    DefLoop = DefLoop->getParentLoop();
  }
}

void HIRScalarSymbaseAssignment::populateRegionLiveouts(
    HIRRegionIdentification::iterator RegIt) {
  // Traverse region basic blocks.
  for (auto BBIt = RegIt->bb_begin(), EndIt = RegIt->bb_end(); BBIt != EndIt;
       ++BBIt) {

    // Check if any instructions inside the basic blocks are live outside the
    // region.
    for (auto Inst = (*BBIt)->begin(), EndI = (*BBIt)->end(); Inst != EndI;
         ++Inst) {

      if (SCCF->isRegionLiveOut(RegIt, &*Inst)) {
        auto Symbase = getOrAssignScalarSymbase(&*Inst, *RegIt);
        RegIt->addLiveOutTemp(Symbase, &*Inst);
        populateLoopLiveouts(&*Inst, Symbase);
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
    const Instruction *SCCInst, unsigned Symbase) {

  // We need special logic to mark symbase as live out of the inner loops of the
  // SCC. This is non-trivial to do during parsing as it is not aware of def-use
  // cycle of SCCs. Liveins are handled correctly by the parser using the
  // root/base instruction mechanism.

  if (!isa<PHINode>(SCCInst)) {
    return;
  }

  auto ParentBB = SCCInst->getParent();

  Loop *Lp = LI->getLoopFor(ParentBB);
  assert(Lp && "SCC phi does not have parent loop!");

  // Ignore non-header phis.
  if (ParentBB != Lp->getHeader()) {
    return;
  }

  HLLoop *DefLoop = LF->findHLLoop(Lp);
  assert(DefLoop && "SCC phi does not have parent HLLoop!");

  // We found an inner SCC loop, mark symbase as live out of this loop.
  DefLoop->addLiveOutTemp(Symbase);
}

void HIRScalarSymbaseAssignment::populateRegionPhiLiveins(
    HIRRegionIdentification::iterator RegIt) {
  // Traverse SCCs associated with the region.
  for (auto SCCIt = SCCF->begin(RegIt), EndIt = SCCF->end(RegIt);
       SCCIt != EndIt; ++SCCIt) {

    bool SCCLiveInProcessed = false;
    // This call sets SCC's root node as the base temp.
    unsigned Symbase = getOrAssignScalarSymbase(SCCIt->getRoot(), *RegIt);

    // Traverse SCC instructions
    for (auto SCCInstIt = SCCIt->begin(), EndIt = SCCIt->end();
         SCCInstIt != EndIt; ++SCCInstIt) {

      if ((*SCCInstIt) != SCCIt->getRoot()) {
        // Assign same symbase to all instructions in the SCC.
        insertTempSymbase(*SCCInstIt, Symbase);
        populateLoopSCCPhiLiveouts(*SCCInstIt, Symbase);
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

bool HIRScalarSymbaseAssignment::runOnFunction(Function &F) {
  Func = &F;
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  SCCF = &getAnalysis<HIRSCCFormation>();
  RI = &getAnalysis<HIRRegionIdentification>();
  LF = &getAnalysis<HIRLoopFormation>();

  // Assign a generic symbase.
  initGenericRvalSymbase();

  for (auto RegIt = RI->begin(), EndRegIt = RI->end(); RegIt != EndRegIt;
       ++RegIt) {
    populateRegionPhiLiveins(RegIt);
    populateRegionLiveouts(RegIt);
  }

  return false;
}

void HIRScalarSymbaseAssignment::releaseMemory() {
  BaseTemps.clear();
  TempSymbaseMap.clear();
  StrSymbaseMap.clear();
}

void HIRScalarSymbaseAssignment::print(raw_ostream &OS, const Module *M) const {

  auto RegBegin = RI->begin();

  for (auto RegIt = RI->begin(), EndRegIt = RI->end(); RegIt != EndRegIt;
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
    for (auto LiveOutIt = RegIt->live_out_begin(),
              EndIt = RegIt->live_out_end();
         LiveOutIt != EndIt; ++LiveOutIt) {
      if (LiveOutIt != RegIt->live_out_begin()) {
        OS << ", ";
      }
      LiveOutIt->second->printAsOperand(OS, false);
    }

    OS << "\n";
  }
}

void HIRScalarSymbaseAssignment::verifyAnalysis() const {
  // TODO: Implement later
}
