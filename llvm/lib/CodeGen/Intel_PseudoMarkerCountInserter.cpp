#if INTEL_FEATURE_MARKERCOUNT
//====- Intel_PseudoMarkerCountInserter.cpp - Insert pseudo marker count  -===//
//
// Copyright (C) 2016-2022 Intel Corporation. All rights reserved.
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

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/LineIterator.h"
#include "llvm/Support/MemoryBuffer.h"

#define DEBUG_TYPE "pseudo-markercount-inserter"

using namespace llvm;

static cl::opt<std::string> FilterMarkerCountFile(
    "filtered-markercount-file",
    cl::desc("Do not insert marker count in functions listed in the file"),
    cl::Optional, cl::Hidden);

namespace {
class PseudoMarkerCountInserter : public MachineFunctionPass {
private:
  // Avoid inserting instructions for markercount in these functions
  std::set<std::string> FilterdFunctions;

public:
  static char ID;

  PseudoMarkerCountInserter() : MachineFunctionPass(ID) {
    initializePseudoMarkerCountInserterPass(*PassRegistry::getPassRegistry());

    if (!FilterMarkerCountFile.empty()) {
      ErrorOr<std::unique_ptr<MemoryBuffer>> Buffer =
          llvm::MemoryBuffer::getFile(FilterMarkerCountFile, true);
      if (!Buffer) {
        errs() << DEBUG_TYPE << ": failed to read file " << FilterMarkerCountFile
               << ": " << Buffer.getError().message() << "\n";
        return;
      }
      for (line_iterator LI(*Buffer.get()); LI != line_iterator(); ++LI) {
        std::string Function = LI->trim().str();
        FilterdFunctions.insert(Function);
      }
    }
  }

  StringRef getPassName() const override {
    return "Pseudo markercount Inserter";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    if (skipFunction(MF.getFunction()))
      return false;

    if (FilterdFunctions.find(MF.getName().str()) != FilterdFunctions.end())
      return false;

    const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
    bool Changed = false;
    DebugLoc DL;
    for (MachineBasicBlock &MBB : MF) {
      auto &K = MBB.getMarkerCount();
      // Please gurantee the order:
      //  PSEUDO_FUNCTION_PROLOG, PSEUDO_LOOP_HEADER, PSEUDO_FUNCTION_EPILOG
      // if they occur in the same basic block.
      //
      // Remove the corresponding MCK of the bb after the pseudo
      // instruction is inserted b/c the bb may be duplicated or removed later.
      if (K.hasKind(MCK::Epilog)) {
        BuildMI(MBB, MBB.getFirstNonPHI(), DL,
                TII->get(TargetOpcode::PSEUDO_FUNCTION_EPILOG));
        K.removeKinds(MCK::Epilog);
        Changed = true;
      }
      if (K.hasKind(MCK::LoopHeader)) {
        BuildMI(MBB, MBB.getFirstNonPHI(), DL,
                TII->get(TargetOpcode::PSEUDO_LOOP_HEADER));
        K.removeKinds(MCK::LoopHeader);
        Changed = true;
      }
      if (K.hasKind(MCK::Prolog)) {
        BuildMI(MBB, MBB.getFirstNonPHI(), DL,
                TII->get(TargetOpcode::PSEUDO_FUNCTION_PROLOG));
        K.removeKinds(MCK::Prolog);
        Changed = true;
      }
    }
    return Changed;
  }
};
} // namespace

char PseudoMarkerCountInserter::ID = 0;
INITIALIZE_PASS_BEGIN(
    PseudoMarkerCountInserter, DEBUG_TYPE,
    "Insert pseudo marker count at prolog, eplilog and loop header", false,
    false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_END(
    PseudoMarkerCountInserter, DEBUG_TYPE,
    "Insert pseudo marker count at prolog, eplilog and loop header", false,
    false)

FunctionPass *llvm::createPseudoMarkerCountInserter() {
  return new PseudoMarkerCountInserter();
}
#endif // INTEL_FEATURE_MARKERCOUNT
