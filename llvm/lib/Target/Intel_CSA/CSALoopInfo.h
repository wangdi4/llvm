//==--- CSALoopInfo.h - Dataflow loop information analysis -----------------==//
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

#ifndef LLVM_LIB_TARGET_CSA_CSALOOPINFO_H
#define LLVM_LIB_TARGET_CSA_CSALOOPINFO_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunctionPass.h"

namespace llvm {
class MachineInstr;
class raw_ostream;

/// A representation of a loop on dataflow instructions.
///
/// Loops in this class are defined by pick/switch dataflow cycles. The header
/// pick refers to picks that accept new loop iterations. The original exiting
/// blocks of the loop are converted to banks of switches. In the case of a
/// bottom-tested loop, where the latch is the exiting block, then the backedge
/// of the switch is hooked directly into the backedge in the pick. In other
/// cases, then there may be computations between the switch and the pick.
///
/// Loops may have multiple exiting blocks, or even no exiting blocks (in the
/// case of infinite loops). Irreducible loops are not likely to be handled with
/// this construct.
class CSALoopInfo {
public:
  typedef std::vector<MachineInstr *>::const_iterator iterator;
  typedef iterator_range<iterator> instr_iterator_range;

  /// Return what the value of the control register of a header pick (0 or 1)
  /// needs to be to continue executing the loop.
  unsigned getPickBackedgeIndex() const { return BackedgeIndex; }
  /// Return what the value of the control register of a header pick (0 or 1)
  /// needs to be to select an iteration from the outside.
  unsigned getPickInitialIndex() const { return 1 - BackedgeIndex; }

  /// Get the number of exiting blocks in the loop.
  unsigned getNumExits() const { return Exits.size(); }

  /// For the exiting block referred to by ExitNum, return the operand of the
  /// corresponding switches that corresponds to continuing loop execution.
  unsigned getSwitchBackedgeIndex(unsigned ExitNum) const {
    return Exits[ExitNum].first;
  }

  /// For the exiting block referred to by ExitNum, return the operand of the
  /// corresponding switches that corresponds to exiting the loop.
  unsigned getSwitchLastIndex(unsigned ExitNum) const {
    return 1 - getSwitchBackedgeIndex(ExitNum);
  }

  /// Get the set of the header picks.
  ArrayRef<MachineInstr *> getHeaderPicks() const {
    return ArrayRef<MachineInstr *>(Picks);
  }

  /// Get the set of switches for the exiting block referred to by ExitNum..
  instr_iterator_range getExitSwitches(unsigned ExitNum) const {
    return instr_iterator_range(Exits[ExitNum].second);
  }

#ifndef NDEBUG
  void dump() const;
  void print(raw_ostream &OS) const;
#endif

  /// Note that the header picks will pick the loop backedge when the control
  /// register reads Index (0 or 1).
  void setPickBackedgeIndex(unsigned Index) {
    assert((Index == 0 || Index == 1) && "Pick index must be 0 or 1");
    BackedgeIndex = Index;
  }
  void addHeaderPick(MachineInstr *Pick);

  /// Note a new exiting block where the loop will continue when the control
  /// register reads SwitchBackedgeIndex (0 or 1). The number of this exiting
  /// block is returned, and can be passed to addExitSwitch.
  unsigned addExit(unsigned SwitchBackedgeIndex);
  void addExitSwitch(unsigned ExitNum, MachineInstr *Switch);
private:
  unsigned BackedgeIndex;
  std::vector<MachineInstr *> Picks;

  /// Each exiting block needs to keep track of both the switch backedge index
  /// and the list of switches.
  typedef std::pair<unsigned, std::vector<MachineInstr *>> ExitInfo;
  SmallVector<ExitInfo, 1> Exits;
};

} // namespace llvm

#endif
