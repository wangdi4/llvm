//===- CSARASReplayableLoads.cpp - CSA RAS Replayable loads Detection Pass -==//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains a pass that detects instructions which read a memory
// location not altered by the other instructions in the function and marks them
// as RasReplayable.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "csa-ras-replayable-loads"
#define PASS_NAME "CSA: RAS Replayable loads Detection Pass"

namespace llvm {
void initializeCSARASReplayableLoadsDetectionPass(PassRegistry &);
}

using namespace llvm;

static cl::opt<bool> CSAEnableRASAttribute(
    "csa-enable-ras-attribute", cl::Hidden,
    cl::desc("CSA Specific: enables generation ras_replayable attribute"),
    cl::init(false));

namespace {
class CSARASReplayableLoadsDetection : public MachineFunctionPass {
public:
  static char ID;
  CSARASReplayableLoadsDetection() : MachineFunctionPass(ID) {
    initializeCSARASReplayableLoadsDetectionPass(
        *PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return PASS_NAME; }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<AAResultsWrapperPass>();
    AU.addRequired<MachineLoopInfo>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

} // namespace

char CSARASReplayableLoadsDetection::ID = 0;

MachineFunctionPass *llvm::createCSARASReplayableLoadsDetectionPass() {
  return new CSARASReplayableLoadsDetection();
}

INITIALIZE_PASS_BEGIN(CSARASReplayableLoadsDetection, DEBUG_TYPE, PASS_NAME,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_END(CSARASReplayableLoadsDetection, DEBUG_TYPE, PASS_NAME,
                    false, false)

// The main function of the pass collects loads and stores in the processing
// function and marks the loads which do not alias with any store as
// RasReplayable
bool CSARASReplayableLoadsDetection::runOnMachineFunction(MachineFunction &MF) {
  if (!CSAEnableRASAttribute)
    return false;

  AAResults *AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  MachineLoopInfo *MLI = &getAnalysis<MachineLoopInfo>();
  SmallPtrSet<MachineInstr *, 16> Loads;
  SmallPtrSet<MachineInstr *, 16> Stores;
  for (MachineBasicBlock &BB : MF) {
    for (MachineInstr &MI : BB) {
      if (MI.mayStore()) {
        Stores.insert(&MI);
        continue;
      }
      if (MI.mayLoad()) {
        bool AddToCandidates = true;
        for (const MachineMemOperand *MMO : MI.memoperands()) {
          if (MMO->isVolatile() || MMO->isAtomic()) {
            AddToCandidates = false;
            break;
          }
        }
        if (AddToCandidates)
          Loads.insert(&MI);
      }
    }
  }

  auto getMemOpndLoc = [](const MachineMemOperand *MMO,
                          bool Looped) -> MemoryLocation {
    const Value *const Val = MMO->getValue();
    auto Size = Looped ? MemoryLocation::UnknownSize : MMO->getSize();
    return MemoryLocation(Val, Size, MMO->getAAInfo());
  };

  auto isAlias = [AA, MLI, getMemOpndLoc](const MachineInstr *LD,
                                          const MachineInstr *ST) -> bool {
    bool LoopedLD = MLI->getLoopFor(LD->getParent());
    for (const MachineMemOperand *LDMMO : LD->memoperands()) {
      auto LDLoc = getMemOpndLoc(LDMMO, LoopedLD);
      for (const MachineMemOperand *STMMO : ST->memoperands()) {
        if (!STMMO->isStore())
          continue;
        bool LoopedST = MLI->getLoopFor(ST->getParent());
        auto STLoc = getMemOpndLoc(STMMO, LoopedST);
        if (!AA->isNoAlias(LDLoc, STLoc))
          return true;
      }
    }
    return false;
  };

  bool Changed = false;
  for (MachineInstr *LD : Loads) {
    bool Replayable = true;
    for (MachineInstr *ST : Stores) {
      if (isAlias(LD, ST)) {
        Replayable = false;
        break;
      }
    }

    if (Replayable) {
      LD->setFlag(MachineInstr::RasReplayable);
      Changed = true;
    }
  }

  return Changed;
}
