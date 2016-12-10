//===--- LPUFrameLowering.h - Define frame lowering for LPU -----*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_LPU_LPUFRAMELOWERING_H
#define LLVM_LIB_TARGET_LPU_LPUFRAMELOWERING_H

#include "LPU.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
class LPUFrameLowering : public TargetFrameLowering {

public:
  explicit LPUFrameLowering()
    : TargetFrameLowering(TargetFrameLowering::StackGrowsUp,
                          /* align */ 16,
                          /* local area offset */ 0,
                          /* transient align */ 16) {}

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  bool hasFP(const MachineFunction &MF) const override;

  void processFunctionBeforeFrameFinalized(MachineFunction &MF,
                                           RegScavenger *RS=nullptr) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const override;
};

} // End llvm namespace

#endif
