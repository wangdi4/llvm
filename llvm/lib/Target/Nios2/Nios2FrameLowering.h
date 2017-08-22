//===-- Nios2FrameLowering.h - Define frame lowering for Nios2 ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_NIOS2_NIOS2FRAMELOWERING_H
#define LLVM_LIB_TARGET_NIOS2_NIOS2FRAMELOWERING_H

#include "Nios2.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
  class Nios2Subtarget;

class Nios2FrameLowering : public TargetFrameLowering {
protected:
  const Nios2Subtarget &STI;

public:
  explicit Nios2FrameLowering(const Nios2Subtarget &sti, unsigned Alignment)
    : TargetFrameLowering(StackGrowsDown, Alignment, 0, Alignment),
      STI(sti) {
  }

  static const Nios2FrameLowering *create(const Nios2Subtarget &ST);

  bool hasFP(const MachineFunction &MF) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF,
                                  MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator I) const override;
};

/// Create Nios2FrameLowering objects.
const Nios2FrameLowering *createNios2SEFrameLowering(const Nios2Subtarget &ST);

} // End llvm namespace

#endif

