//===- Nios2InstructionSelector --------------------------------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file declares the targeting of the InstructionSelector class for
/// Nios2.
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_NIOS2_NIOS2INSTRUCTIONSELECTOR_H
#define LLVM_LIB_TARGET_NIOS2_NIOS2INSTRUCTIONSELECTOR_H

#include "llvm/CodeGen/GlobalISel/InstructionSelector.h"

namespace llvm {

class Nios2InstrInfo;
class Nios2RegisterBankInfo;
class Nios2RegisterInfo;
class Nios2Subtarget;
class Nios2TargetMachine;

class MachineFunction;
class MachineRegisterInfo;

class Nios2InstructionSelector : public InstructionSelector {
public:
  Nios2InstructionSelector(const Nios2TargetMachine &TM,
                             const Nios2Subtarget &STI,
                             const Nios2RegisterBankInfo &RBI);

  bool select(MachineInstr &I) const override;

private:
  bool selectVaStartAAPCS(MachineInstr &I, MachineFunction &MF,
                          MachineRegisterInfo &MRI) const;
  bool selectVaStartDarwin(MachineInstr &I, MachineFunction &MF,
                           MachineRegisterInfo &MRI) const;

  /// tblgen-erated 'select' implementation, used as the initial selector for
  /// the patterns that don't require complex C++.
//  bool selectImpl(MachineInstr &I) const;

  const Nios2TargetMachine &TM;
  const Nios2Subtarget &STI;
  const Nios2InstrInfo &TII;
  const Nios2RegisterInfo &TRI;
  const Nios2RegisterBankInfo &RBI;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_Nios2_Nios2INSTRUCTIONSELECTOR_H
