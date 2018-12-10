//==--- CSALoopInfo.cpp - Dataflow loop information analysis ---------------==//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
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

using namespace llvm;

unsigned CSALoopInfo::addExit(unsigned Index) {
  assert((Index == 0 || Index == 1) && "Pick index must be 0 or 1");
  unsigned exitNum = getNumExits();
  Exits.emplace_back(Index, ArrayRef<MachineInstr *>{});
  return exitNum;
}

void CSALoopInfo::addExitSwitch(unsigned ExitNum, MachineInstr *Switch) {
  Exits[ExitNum].second.push_back(Switch);
}

void CSALoopInfo::addHeaderPick(MachineInstr *Pick) {
  Picks.push_back(Pick);
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
      (Exit.first == 0 ? "first" : "second") << " is the backedge:\n";
    for (auto MI : Exit.second) {
      OS << "    ";
      MI->print(OS);
    }
  }
}
#endif
