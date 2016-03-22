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

#include "llvm/Analysis/ScalarEvolution.h"

#include "llvm/IR/Intel_LoopIR/IRRegion.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRSCCFormation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRScalarSymbaseAssignment.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-scalar-symbase-assignment"

INITIALIZE_PASS_BEGIN(HIRScalarSymbaseAssignment,
                      "hir-scalar-symbase-assignment",
                      "HIR Scalar Symbase Assignment", false, true)
INITIALIZE_PASS_DEPENDENCY(HIRRegionIdentification)
INITIALIZE_PASS_DEPENDENCY(HIRSCCFormation)
INITIALIZE_PASS_END(HIRScalarSymbaseAssignment, "hir-scalar-symbase-assignment",
                    "HIR Scalar Symbase Assignment", false, true)

char HIRScalarSymbaseAssignment::ID = 0;

FunctionPass *llvm::createHIRScalarSymbaseAssignmentPass() {
  return new HIRScalarSymbaseAssignment();
}

HIRScalarSymbaseAssignment::HIRScalarSymbaseAssignment() : FunctionPass(ID) {
  initializeHIRScalarSymbaseAssignmentPass(*PassRegistry::getPassRegistry());
}

void HIRScalarSymbaseAssignment::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<HIRRegionIdentification>();
  AU.addRequired<HIRSCCFormation>();
}

void HIRScalarSymbaseAssignment::insertHIRLval(const Value *Lval,
                                               unsigned Symbase) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  ScalarLvalSymbases[Symbase] = Lval;
#endif
}

unsigned HIRScalarSymbaseAssignment::insertBaseTemp(const Value *Temp) {
  BaseTemps.push_back(Temp);

  return getMaxScalarSymbase();
}

void HIRScalarSymbaseAssignment::insertTempSymbase(const Value *Temp,
                                                   unsigned Symbase) {
  assert((Symbase > ConstantSymbase) && (Symbase <= getMaxScalarSymbase()) &&
         "Symbase is out of range!");

  auto Ret = TempSymbaseMap.insert(std::make_pair(Temp, Symbase));
  (void)Ret;
  assert(Ret.second && "Attempt to overwrite Temp symbase!");
}

unsigned HIRScalarSymbaseAssignment::getOrAssignTempSymbase(const Value *Temp) {
  auto Symbase = getTempSymbase(Temp);

  if (!Symbase) {
    Symbase = insertBaseTemp(Temp);
    insertTempSymbase(Temp, Symbase);
  }

  return Symbase;
}

const Value *HIRScalarSymbaseAssignment::traceSingleOperandPhis(
    const Value *Scalar, const IRRegion *IRReg) const {

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
    if (!IRReg->containsBBlock(PhiInst->getParent())) {
      break;
    }

    Scalar = PhiInst->getOperand(0);

    assert(isa<Instruction>(Scalar) &&
           "Single phi operand is not an instruction!");

    PhiInst = dyn_cast<PHINode>(Scalar);
  }

  return Scalar;
}

unsigned HIRScalarSymbaseAssignment::getTempSymbase(const Value *Temp) const {

  auto Iter = TempSymbaseMap.find(Temp);

  if (Iter != TempSymbaseMap.end()) {
    return Iter->second;
  }

  return InvalidSymbase;
}

const Value *HIRScalarSymbaseAssignment::getBaseScalar(unsigned Symbase) const {
  const Value *RetVal = nullptr;

  assert((Symbase > ConstantSymbase) && "Symbase is out of range!");

  if (Symbase <= getMaxScalarSymbase()) {
    RetVal = BaseTemps[Symbase - ConstantSymbase - 1];
  } else {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    // Symbase can be out of range for new temps created by HIR transformations.
    // These temps are registered by framework utils for printing in debug mode.
    auto It = ScalarLvalSymbases.find(Symbase);
    assert((It != ScalarLvalSymbases.end()) && "Symbase not present in map!");
    RetVal = It->second;
#else
    // We shouldn't reach here in prod mode. ScalarLvalSymbases is only
    // maintained in debug mode for printing.
    llvm_unreachable("Couldn't find base temp!");
#endif
  }

  assert(RetVal && "Unexpected null value in RetVal");
  return RetVal;
}

const Value *
HIRScalarSymbaseAssignment::getBaseScalar(const Value *Scalar) const {
  auto Symbase = getTempSymbase(Scalar);

  if (Symbase) {
    return getBaseScalar(Symbase);
  } else {
    return Scalar;
  }
}

unsigned HIRScalarSymbaseAssignment::getMaxScalarSymbase() const {
  return BaseTemps.size() + ConstantSymbase;
}

void HIRScalarSymbaseAssignment::setGenericLoopUpperSymbase() {
  auto Symbase = insertBaseTemp(Func);
  insertTempSymbase(Func, Symbase);
}

const Value *HIRScalarSymbaseAssignment::getGenericLoopUpperVal() const {
  return Func;
}

bool HIRScalarSymbaseAssignment::isConstant(const Value *Scalar) const {
  // TODO: add other types
  if (isa<ConstantInt>(Scalar) || isa<ConstantFP>(Scalar) ||
      isa<ConstantPointerNull>(Scalar)) {
    return true;
  }

  return false;
}

MDString *
HIRScalarSymbaseAssignment::getInstMDString(const Instruction *Inst) const {
  // We only care about livein copies here because unlike liveout copies, livein
  // copies need to be assigned the same symbase as other values in the SCC.
  auto MDNode = Inst->getMetadata(HIR_LIVE_IN_STR);

  if (!MDNode) {
    return nullptr;
  }

  assert((MDNode->getNumOperands() == 1) && "Invalid metadata node!");

  auto &MD = MDNode->getOperand(0);
  assert(isa<MDString>(MD) && "Invalid metadata node!");

  return cast<MDString>(MD);
}

unsigned HIRScalarSymbaseAssignment::getOrAssignScalarSymbaseImpl(
    const Value *Scalar, const IRRegion *IRReg, bool Assign) {
  unsigned Symbase = InvalidSymbase;

  // TODO: assign constant symbase to metadata types as they do not cause data
  // dependencies.
  if (isConstant(Scalar)) {
    return ConstantSymbase;
  }

  Scalar = traceSingleOperandPhis(Scalar, IRReg);

  if (auto Inst = dyn_cast<Instruction>(Scalar)) {
    // First check if this instruction is a copy instruction inserted by SSA
    // deconstruction pass. If so, we need to access/update StrSymbaseMap.
    auto MDStr = getInstMDString(Inst);

    if (MDStr) {
      auto StrRef = MDStr->getString();
      auto It = StrSymbaseMap.find(StrRef);

      if (It != StrSymbaseMap.end()) {
        Symbase = It->getValue();

        // Insert into TempSymbaseMap so that the base temp can be retrieved
        // using getBaseScalar().
        if (Assign && (InvalidSymbase == getTempSymbase(Inst))) {
          insertTempSymbase(Inst, Symbase);
        }
      } else {
        if (Assign) {
          Symbase = getOrAssignTempSymbase(Inst);
          StrSymbaseMap.insert(std::make_pair(StrRef, Symbase));
        } else {
          Symbase = getTempSymbase(Inst);
        }
      }
    } else {
      Symbase = Assign ? getOrAssignTempSymbase(Inst) : getTempSymbase(Inst);
    }
  } else {
    Symbase = Assign ? getOrAssignTempSymbase(Scalar) : getTempSymbase(Inst);
  }

  return Symbase;
}

unsigned
HIRScalarSymbaseAssignment::getOrAssignScalarSymbase(const Value *Scalar,
                                                     const IRRegion *IRReg) {
  return getOrAssignScalarSymbaseImpl(Scalar, IRReg, true);
}

unsigned HIRScalarSymbaseAssignment::getScalarSymbase(const Value *Scalar,
                                                      const IRRegion *IRReg) {
  return getOrAssignScalarSymbaseImpl(Scalar, IRReg, false);
}

void HIRScalarSymbaseAssignment::populateRegionLiveouts(
    HIRRegionIdentification::iterator RegIt) {
  // Traverse region basic blocks.
  for (auto BBIt = (*RegIt)->bb_begin(), EndIt = (*RegIt)->bb_end();
       BBIt != EndIt; ++BBIt) {

    // Check if any instructions inside the basic blocks are live outside the
    // region.
    for (auto Inst = (*BBIt)->begin(), EndI = (*BBIt)->end(); Inst != EndI;
         ++Inst) {

      if (SCCF->isRegionLiveOut(RegIt, &*Inst)) {
        auto Symbase = getOrAssignScalarSymbase(&*Inst, *RegIt);
        (*RegIt)->addLiveOutTemp(&*Inst, Symbase);
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

    if (!(*RegIt)->containsBBlock(Phi->getIncomingBlock(I))) {
      (*RegIt)->addLiveInTemp(Symbase, Phi->getIncomingValue(I));

      Ret = true;
      break;
    }
  }

  return Ret;
}

void HIRScalarSymbaseAssignment::populateRegionPhiLiveins(
    HIRRegionIdentification::iterator RegIt) {
  // Traverse SCCs associated with the region.
  for (auto SCCIt = SCCF->begin(RegIt), EndIt = SCCF->end(RegIt);
       SCCIt != EndIt; ++SCCIt) {

    bool SCCLiveInProcessed = false;
    unsigned Symbase = getOrAssignScalarSymbase((*SCCIt)->Root, *RegIt);

    // Traverse SCC instructions
    for (auto SCCInstIt = (*SCCIt)->Nodes.begin(),
              EndIt = (*SCCIt)->Nodes.end();
         SCCInstIt != EndIt; ++SCCInstIt) {

      // Assign same symbase to all instructions in the SCC.
      if ((*SCCInstIt) != (*SCCIt)->Root) {
        insertTempSymbase(*SCCInstIt, Symbase);
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
  for (auto InstIt = (*RegIt)->getEntryBBlock()->begin(),
            EndIt = (*RegIt)->getEntryBBlock()->end();
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
  SCCF = &getAnalysis<HIRSCCFormation>();
  RI = &getAnalysis<HIRRegionIdentification>();

  // Assign a generic symbase representing loop uppers.
  setGenericLoopUpperSymbase();

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
  ScalarLvalSymbases.clear();
}

void HIRScalarSymbaseAssignment::print(raw_ostream &OS, const Module *M) const {

  auto RegBegin = RI->begin();

  for (auto RegIt = RI->begin(), EndRegIt = RI->end(); RegIt != EndRegIt;
       ++RegIt) {
    OS << "\nRegion " << (RegIt - RegBegin + 1);

    OS << "\n   Phi LiveIns: ";
    for (auto LiveInIt = (*RegIt)->live_in_begin(),
              EndIt = (*RegIt)->live_in_end();
         LiveInIt != EndIt; ++LiveInIt) {

      if (LiveInIt != (*RegIt)->live_in_begin()) {
        OS << ", ";
      }

      getBaseScalar(LiveInIt->first)->printAsOperand(OS, false);
      OS << "(";
      LiveInIt->second->printAsOperand(OS, false);
      OS << ")";
    }

    OS << "\n   LiveOuts: ";
    for (auto LiveOutIt = (*RegIt)->live_out_begin(),
              EndIt = (*RegIt)->live_out_end();
         LiveOutIt != EndIt; ++LiveOutIt) {
      if (LiveOutIt != (*RegIt)->live_out_begin()) {
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
