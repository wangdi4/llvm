//===-- Nios2SEInstrInfo.h - Nios232/64 Instruction Information ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Nios232/64 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_NIOS2_NIOS2SEINSTRINFO_H
#define LLVM_LIB_TARGET_NIOS2_NIOS2SEINSTRINFO_H

#include "Nios2InstrInfo.h"
#include "Nios2SERegisterInfo.h"
#include "Nios2MachineFunction.h"

namespace llvm {

class Nios2SEInstrInfo : public Nios2InstrInfo {
  const Nios2SERegisterInfo RI;

public:
  explicit Nios2SEInstrInfo(const Nios2Subtarget &STI);

  const Nios2RegisterInfo &getRegisterInfo() const override;

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                   const DebugLoc &DL, unsigned DestReg, unsigned SrcReg,
                   bool KillSrc) const override;

  void storeRegToStack(MachineBasicBlock &MBB,
                       MachineBasicBlock::iterator MI,
                       unsigned SrcReg, bool isKill, int FrameIndex,
                       const TargetRegisterClass *RC,
                       const TargetRegisterInfo *TRI,
                       int64_t Offset) const override;

  void loadRegFromStack(MachineBasicBlock &MBB,
                        MachineBasicBlock::iterator MI,
                        unsigned DestReg, int FrameIndex,
                        const TargetRegisterClass *RC,
                        const TargetRegisterInfo *TRI,
                        int64_t Offset) const override;

//@expandPostRAPseudo
  bool expandPostRAPseudo(MachineInstr &MI) const override;

  /// Adjust SP by Amount bytes.
  void adjustStackPtr(unsigned SP, int64_t Amount, MachineBasicBlock &MBB,
                      MachineBasicBlock::iterator I) const override;

  /// Emit a series of instructions to load an immediate. If NewImm is a
  /// non-NULL parameter, the last instruction is not emitted, but instead
  /// its immediate operand is returned in NewImm.
  unsigned loadImmediate(int64_t Imm, MachineBasicBlock &MBB,
                         MachineBasicBlock::iterator II, const DebugLoc &DL,
                         unsigned *NewImm) const;
private:
  void expandRetLR(MachineBasicBlock &MBB, MachineBasicBlock::iterator I) const;

  unsigned getOppositeBranchOpc(unsigned Opc) const override;
  
  void expandEhReturn(MachineBasicBlock &MBB,
                      MachineBasicBlock::iterator I) const;
};

}

#endif
