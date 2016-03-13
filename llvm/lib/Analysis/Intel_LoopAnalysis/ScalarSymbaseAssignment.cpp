//===- ScalarSymbaseAssignment.cpp - Assigns symbase to scalars -----------===//
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
// This file implements the ScalarSymbaseAssignment pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Metadata.h"

#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/ScalarEvolution.h"

#include "llvm/IR/Intel_LoopIR/IRRegion.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/Intel_LoopAnalysis/ScalarSymbaseAssignment.h"
#include "llvm/Analysis/Intel_LoopAnalysis/SCCFormation.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-scalar-symbase-assignment"

INITIALIZE_PASS_BEGIN(ScalarSymbaseAssignment, "hir-scalar-symbase-assignment",
                      "HIR Scalar Symbase Assignment", false, true)
INITIALIZE_PASS_DEPENDENCY(RegionIdentification)
INITIALIZE_PASS_DEPENDENCY(SCCFormation)
INITIALIZE_PASS_END(ScalarSymbaseAssignment, "hir-scalar-symbase-assignment",
                    "HIR Scalar Symbase Assignment", false, true)

char ScalarSymbaseAssignment::ID = 0;

FunctionPass *llvm::createScalarSymbaseAssignmentPass() {
  return new ScalarSymbaseAssignment();
}

ScalarSymbaseAssignment::ScalarSymbaseAssignment() : FunctionPass(ID) {
  initializeScalarSymbaseAssignmentPass(*PassRegistry::getPassRegistry());
}

void ScalarSymbaseAssignment::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<RegionIdentification>();
  AU.addRequired<SCCFormation>();
}

void ScalarSymbaseAssignment::insertHIRLval(const Value *Lval,
                                            unsigned Symbase) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  ScalarLvalSymbases[Symbase] = Lval;
#endif
}

unsigned ScalarSymbaseAssignment::insertBaseTemp(const Value *Temp) {
  BaseTemps.push_back(Temp);

  return getMaxScalarSymbase();
}

void ScalarSymbaseAssignment::insertTempSymbase(const Value *Temp,
                                                unsigned Symbase) {
  assert((Symbase > CONSTANT_SYMBASE) && (Symbase <= getMaxScalarSymbase()) &&
         "Symbase is out of range!");

  auto Ret = TempSymbaseMap.insert(std::make_pair(Temp, Symbase));
  (void)Ret;
  assert(Ret.second && "Attempt to overwrite Temp symbase!");
}

unsigned ScalarSymbaseAssignment::getOrAssignTempSymbase(const Value *Temp) {
  auto Symbase = getTempSymbase(Temp);

  if (!Symbase) {
    Symbase = insertBaseTemp(Temp);
    insertTempSymbase(Temp, Symbase);
  }

  return Symbase;
}

const Value *
ScalarSymbaseAssignment::traceSingleOperandPhis(const Value *Scalar) const {
                                                   
  auto PhiInst = dyn_cast<PHINode>(Scalar);

  while (PhiInst && (1 == PhiInst->getNumIncomingValues())) {

    Scalar = PhiInst->getOperand(0);

    assert(isa<Instruction>(Scalar) &&
           "Single phi operand is not an instruction!");

    PhiInst = dyn_cast<PHINode>(Scalar);
  }

  return Scalar;
}

unsigned ScalarSymbaseAssignment::getTempSymbase(const Value *Temp) const {

  auto Iter = TempSymbaseMap.find(Temp);

  if (Iter != TempSymbaseMap.end()) {
    return Iter->second;
  }

  return INVALID_SYMBASE;
}

const Value *ScalarSymbaseAssignment::getBaseScalar(unsigned Symbase) const {
  const Value *RetVal = nullptr;

  assert((Symbase > CONSTANT_SYMBASE) && "Symbase is out of range!");

  if (Symbase <= getMaxScalarSymbase()) {
    RetVal = BaseTemps[Symbase - CONSTANT_SYMBASE - 1];
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

const Value *ScalarSymbaseAssignment::getBaseScalar(const Value *Scalar) const {
  auto Symbase = getTempSymbase(Scalar);

  if (Symbase) {
    return getBaseScalar(Symbase);
  } else {
    return Scalar;
  }
}

unsigned ScalarSymbaseAssignment::getMaxScalarSymbase() const {
  return BaseTemps.size() + CONSTANT_SYMBASE;
}

void ScalarSymbaseAssignment::setGenericLoopUpperSymbase() {
  auto Symbase = insertBaseTemp(Func);
  insertTempSymbase(Func, Symbase);
}

const Value *ScalarSymbaseAssignment::getGenericLoopUpperVal() const {
  return Func;
}

bool ScalarSymbaseAssignment::isConstant(const Value *Scalar) const {
  // TODO: add other types
  if (isa<ConstantInt>(Scalar) || isa<ConstantFP>(Scalar) ||
      isa<ConstantPointerNull>(Scalar)) {
    return true;
  }

  return false;
}

MDString *
ScalarSymbaseAssignment::getInstMDString(const Instruction *Inst) const {
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

unsigned
ScalarSymbaseAssignment::getOrAssignScalarSymbaseImpl(const Value *Scalar,
                                                      bool Assign) {
  unsigned Symbase = INVALID_SYMBASE;

  // TODO: assign constant symbase to metadata types as they do not cause data
  // dependencies.
  if (isConstant(Scalar)) {
    return CONSTANT_SYMBASE;
  }

  Scalar = traceSingleOperandPhis(Scalar);

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
        if (Assign && (INVALID_SYMBASE == getTempSymbase(Inst))) {
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
ScalarSymbaseAssignment::getOrAssignScalarSymbase(const Value *Scalar) {
  return getOrAssignScalarSymbaseImpl(Scalar, true);
}

unsigned ScalarSymbaseAssignment::getScalarSymbase(const Value *Scalar) {
  return getOrAssignScalarSymbaseImpl(Scalar, false);
}

void ScalarSymbaseAssignment::populateRegionLiveouts(
    RegionIdentification::iterator RegIt) {
  // Traverse region basic blocks.
  for (auto BBIt = (*RegIt)->bb_begin(), EndIt = (*RegIt)->bb_end();
       BBIt != EndIt; ++BBIt) {

    // Check if any instructions inside the basic blocks are live outside the
    // region.
    for (auto Inst = (*BBIt)->begin(), EndI = (*BBIt)->end(); Inst != EndI;
         ++Inst) {

      if (SCCF->isRegionLiveOut(RegIt, &*Inst)) {
        auto Symbase = getOrAssignScalarSymbase(&*Inst);
        (*RegIt)->addLiveOutTemp(&*Inst, Symbase);
      }
    }
  }
}

bool ScalarSymbaseAssignment::processRegionPhiLivein(
    RegionIdentification::iterator RegIt, const PHINode *Phi,
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

void ScalarSymbaseAssignment::populateRegionPhiLiveins(
    RegionIdentification::iterator RegIt) {
  // Traverse SCCs associated with the region.
  for (auto SCCIt = SCCF->begin(RegIt), EndIt = SCCF->end(RegIt);
       SCCIt != EndIt; ++SCCIt) {

    bool SCCLiveInProcessed = false;
    unsigned Symbase = getOrAssignScalarSymbase((*SCCIt)->Root);

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
                           getOrAssignScalarSymbase(&*InstIt));
  }
}

bool ScalarSymbaseAssignment::runOnFunction(Function &F) {
  Func = &F;
  SCCF = &getAnalysis<SCCFormation>();
  RI = &getAnalysis<RegionIdentification>();

  // Assign a generic symbase representing loop uppers.
  setGenericLoopUpperSymbase();

  for (auto RegIt = RI->begin(), EndRegIt = RI->end(); RegIt != EndRegIt;
       ++RegIt) {
    populateRegionPhiLiveins(RegIt);
    populateRegionLiveouts(RegIt);
  }

  return false;
}

void ScalarSymbaseAssignment::releaseMemory() {
  BaseTemps.clear();
  TempSymbaseMap.clear();
  StrSymbaseMap.clear();
  ScalarLvalSymbases.clear();
}

void ScalarSymbaseAssignment::print(raw_ostream &OS, const Module *M) const {

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

void ScalarSymbaseAssignment::verifyAnalysis() const {
  // TODO: Implement later
}
