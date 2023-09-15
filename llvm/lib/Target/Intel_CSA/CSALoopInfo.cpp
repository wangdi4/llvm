//==--- CSALoopInfo.cpp - Dataflow loop information analysis ---------------==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains the analysis data for loop information in a dataflow
// conversion pass.
//
//===----------------------------------------------------------------------===//

#include "CSALoopInfo.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

unsigned CSALoopInfo::addExit(unsigned Index) {
  assert((Index == 0 || Index == 1) && "Pick index must be 0 or 1");
  unsigned exitNum = getNumExits();
  Exits.emplace_back(Index, ArrayRef<MachineInstr *>{});
  return exitNum;
}

void CSALoopInfo::addExitSwitch(unsigned ExitNum, MachineInstr *Switch) {
  Exits[ExitNum].ExitSwitches.push_back(Switch);
}

void CSALoopInfo::addHeaderPick(MachineInstr *Pick) {
  Picks.push_back(Pick);
}

void CSALoopInfo::removePickSwitch(MachineInstr *Pick, MachineInstr *Switch) {
  for (auto PIter = Picks.begin(); PIter != Picks.end(); PIter++) {
    if ((*PIter) == Pick) {
      Picks.erase(PIter);
      break;
    }
  }

  for (auto &Exit : Exits) {
    auto &Switches = Exit.ExitSwitches;
    for (auto SIter = Switches.begin(); SIter != Switches.end(); SIter++) {
      if ((*SIter) == Switch) {
        Switches.erase(SIter);
        return;
      }
    }
  }
}

void CSALoopInfo::addILPLSpinningReg(unsigned Reg) {
  ILPLSpinningRegs.insert(Reg);
}

bool CSALoopInfo::isILPLSpinningReg(unsigned Reg) const {
  return ILPLSpinningRegs.count(Reg);
}

#ifndef NDEBUG
void CSALoopInfo::dump() const {
  print(dbgs());
}

void CSALoopInfo::print(raw_ostream &OS) const {
  OS << "CSA dataflow loop with " << Exits.size() << " exits:\n";
  OS << "  Loop header picks, " <<
    (BackedgeIndex == 0 ? "first" : "second") << " is the backedge:\n";
  for (auto MI : Picks) {
    OS << "    ";
    MI->print(OS);
  }
  for (unsigned i = 0; i < Exits.size(); i++) {
    const ExitInfo &Exit = Exits[i];
    OS << "  Exit #" << i << ", " <<
      (Exit.SwitchBackedgeIndex == 0 ? "first" : "second") << " is the backedge:\n";
    for (auto MI : Exit.ExitSwitches) {
      OS << "    ";
      MI->print(OS);
    }
  }
}
#endif

char CSALoopInfoPass::ID = 0;
INITIALIZE_PASS_BEGIN(CSALoopInfoPass, "csa-loops",
                      "CSA Dataflow Loop Info", true, false)
INITIALIZE_PASS_END(CSALoopInfoPass, "csa-loops",
                    "CSA Dataflow Loop Info", true, false)

void CSALoopInfoPass::addLoop(CSALoopInfo loop) {
  Loops.emplace_back(std::move(loop));
}
