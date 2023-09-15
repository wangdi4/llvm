#if INTEL_FEATURE_MARKERCOUNT
//====- Intel_PseudoMarkerCountInserter.cpp - Insert pseudo marker count  -===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements PseudoMarkerCountInserter pass, which inserts pseudo
// marker count at prolog/epilog of the function and loop headers.
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_MarkerCountInfo.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Target/TargetMachine.h"

#define DEBUG_TYPE "pseudo-markercount-inserter"

using namespace llvm;

namespace {
class PseudoMarkerCountInserter : public MachineFunctionPass {
private:
  std::map<std::string, unsigned> OverrideMarkerCount;
  unsigned MarkerCountKind;

  unsigned getMarkerCount(StringRef FunctionName) const {
    std::string Name = FunctionName.str();
    return OverrideMarkerCount.count(Name) ? OverrideMarkerCount.at(Name)
                                           : MarkerCountKind;
  }

public:
  static char ID;

  PseudoMarkerCountInserter(unsigned MarkerCountKind = 0,
                            StringRef OverrideMarkerCountFile = "")
      : MachineFunctionPass(ID), MarkerCountKind(MarkerCountKind) {
    initializePseudoMarkerCountInserterPass(*PassRegistry::getPassRegistry());
    MarkerCount::parseMarkerCountFile(OverrideMarkerCount, MarkerCountKind,
                                      OverrideMarkerCountFile);
  }

  StringRef getPassName() const override {
    return "Pseudo marker count Inserter";
  }

  bool skipFunction(const Function &F) const override {
    unsigned MCK = getMarkerCount(F.getName());
    if (!(MCK & MarkerCount::BE))
      return true;

    return FunctionPass::skipFunction(F);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    if (MarkerCountKind & MarkerCount::Loop_BE ||
        !OverrideMarkerCount.empty()) {
      AU.addRequired<MachineLoopInfo>();
      AU.addPreserved<MachineLoopInfo>();
    }
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    if (skipFunction(MF.getFunction()))
      return false;

    unsigned MCK = getMarkerCount(MF.getName());
    bool MarkPrologEpilog = MCK & MarkerCount::Function_BE;
    bool MarkLoopHeader = MCK & MarkerCount::Loop_BE;
    assert((MarkPrologEpilog || MarkLoopHeader) &&
           "expect at least one kind of marker count");
    // Bail out early
    if (!MarkPrologEpilog && getAnalysis<MachineLoopInfo>().empty())
      return false;

    const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
    bool Changed = false;
    DebugLoc DL;

    // Loopless since function has only one entry basic block
    if (MarkPrologEpilog) {
      MachineBasicBlock &MBB = MF.front();
      BuildMI(MBB, MBB.getFirstNonPHI(), DL,
              TII->get(TargetOpcode::PSEUDO_FUNCTION_PROLOG));
      Changed = true;
    }

    for (MachineBasicBlock &MBB : MF) {
      if (MBB.isReturnBlock() && MarkPrologEpilog) {
        BuildMI(MBB, MBB.getFirstTerminator(), DL,
                TII->get(TargetOpcode::PSEUDO_FUNCTION_EPILOG));
        assert(Changed &&
               "should already be set due to the insertion in entry block");
      }
      if (MarkLoopHeader && getAnalysis<MachineLoopInfo>().isLoopHeader(&MBB)) {
        BuildMI(MBB, MBB.getFirstNonPHI(), DL,
                TII->get(TargetOpcode::PSEUDO_LOOP_HEADER));
        Changed = true;
      }
    }
    return Changed;
  }
};
} // namespace

char PseudoMarkerCountInserter::ID = 0;
char &llvm::PseudoMarkerCountInserterID = PseudoMarkerCountInserter::ID;

INITIALIZE_PASS_BEGIN(
    PseudoMarkerCountInserter, DEBUG_TYPE,
    "Insert pseudo marker count at prolog, eplilog and loop header", false,
    false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_END(
    PseudoMarkerCountInserter, DEBUG_TYPE,
    "Insert pseudo marker count at prolog, eplilog and loop header", false,
    false)

FunctionPass *
llvm::createPseudoMarkerCountInserter(unsigned MarkerCountKind,
                                      StringRef OverrideMarkerCountFile) {
  return new PseudoMarkerCountInserter(MarkerCountKind,
                                       OverrideMarkerCountFile);
}
#endif // INTEL_FEATURE_MARKERCOUNT
