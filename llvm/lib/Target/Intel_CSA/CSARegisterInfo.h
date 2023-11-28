//===-- CSARegisterInfo.h - CSA Register Information Impl -------*- C++ -*-===//
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
// This file contains the CSA implementation of the MRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAREGISTERINFO_H
#define LLVM_LIB_TARGET_CSA_CSAREGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "CSAGenRegisterInfo.inc"

namespace llvm {

struct CSARegisterInfo : public CSAGenRegisterInfo {
public:
  const TargetInstrInfo &TII;

  CSARegisterInfo(const TargetInstrInfo &tii);

  /// Code Generation virtual methods...
  const MCPhysReg *
  getCalleeSavedRegs(const MachineFunction *MF = nullptr) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  const TargetRegisterClass *
  getPointerRegClass(const MachineFunction &MF,
                     unsigned Kind = 0) const override;

  void eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  // Debug information queries.
  Register getFrameRegister(const MachineFunction &MF) const override;
};

} // end namespace llvm

#endif
