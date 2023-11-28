//===--- CSAFrameLowering.h - Define frame lowering for CSA -----*- C++ -*-===//
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
//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAFRAMELOWERING_H
#define LLVM_LIB_TARGET_CSA_CSAFRAMELOWERING_H

#include "CSA.h"
#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {
class CSAFrameLowering : public TargetFrameLowering {

public:
  explicit CSAFrameLowering()
      : TargetFrameLowering(TargetFrameLowering::StackGrowsUp,
                            /* align */ Align(16),
                            /* local area offset */ 0,
                            /* transient align */ Align(16)) {}

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  bool hasFP(const MachineFunction &MF) const override;

  void processFunctionBeforeFrameFinalized(
    MachineFunction &MF, RegScavenger *RS = nullptr) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const override;
};

} // namespace llvm

#endif
