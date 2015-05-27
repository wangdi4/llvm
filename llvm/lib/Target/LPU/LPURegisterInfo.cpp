//===-- LPURegisterInfo.cpp - LPU Register Information --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the LPU implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "LPURegisterInfo.h"
#include "LPU.h"
#include "LPUMachineFunctionInfo.h"
#include "LPUTargetMachine.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

#define DEBUG_TYPE "lpu-reg-info"

#define GET_REGINFO_TARGET_DESC
#include "LPUGenRegisterInfo.inc"

// FIXME: Provide proper call frame setup / destroy opcodes.
LPURegisterInfo::LPURegisterInfo()
  : LPUGenRegisterInfo(LPU::R0) {}


const MCPhysReg*
LPURegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
/*
  const TargetFrameLowering *TFI = MF->getSubtarget().getFrameLowering();
  const Function* F = MF->getFunction();
  static const MCPhysReg CalleeSavedRegs[] = {
    LPU::FP, LPU::R5, LPU::R6, LPU::R7,
    LPU::R8, LPU::R9, LPU::R10, LPU::R11,
    0
  };
  static const MCPhysReg CalleeSavedRegsFP[] = {
    LPU::R5, LPU::R6, LPU::R7,
    LPU::R8, LPU::R9, LPU::R10, LPU::R11,
    0
  };
  static const MCPhysReg CalleeSavedRegsIntr[] = {
    LPU::FP,  LPU::R5,  LPU::R6,  LPU::R7,
    LPU::R8,  LPU::R9,  LPU::R10, LPU::R11,
    LPU::R12, LPU::R13, LPU::R14, LPU::R15,
    0
  };
  static const MCPhysReg CalleeSavedRegsIntrFP[] = {
    LPU::R5,  LPU::R6,  LPU::R7,
    LPU::R8,  LPU::R9,  LPU::R10, LPU::R11,
    LPU::R12, LPU::R13, LPU::R14, LPU::R15,
    0
  };

  if (TFI->hasFP(*MF))
    return (F->getCallingConv() == CallingConv::LPU_INTR ?
            CalleeSavedRegsIntrFP : CalleeSavedRegsFP);
  else
    return (F->getCallingConv() == CallingConv::LPU_INTR ?
            CalleeSavedRegsIntr : CalleeSavedRegs);
*/
  return NULL;
}

BitVector LPURegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  /*
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();

  // Mark 4 special registers with subregisters as reserved.
  Reserved.set(LPU::PCB);
  Reserved.set(LPU::SPB);
  Reserved.set(LPU::SRB);
  Reserved.set(LPU::CGB);
  Reserved.set(LPU::PC);
  Reserved.set(LPU::SP);
  Reserved.set(LPU::SR);
  Reserved.set(LPU::CG);

  // Mark frame pointer as reserved if needed.
  if (TFI->hasFP(MF)) {
    Reserved.set(LPU::FPB);
    Reserved.set(LPU::FP);
  }
  */
  return Reserved;
}
/*
const TargetRegisterClass *
LPURegisterInfo::getPointerRegClass(const MachineFunction &MF, unsigned Kind)
                                                                         const {
  return &LPU::GR16RegClass;
}
*/
void
LPURegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                        int SPAdj, unsigned FIOperandNum,
                                        RegScavenger *RS) const {
  /*
  assert(SPAdj == 0 && "Unexpected");

  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();
  DebugLoc dl = MI.getDebugLoc();
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();

  unsigned BasePtr = (TFI->hasFP(MF) ? LPU::FP : LPU::SP);
  int Offset = MF.getFrameInfo()->getObjectOffset(FrameIndex);

  // Skip the saved PC
  Offset += 2;

  if (!TFI->hasFP(MF))
    Offset += MF.getFrameInfo()->getStackSize();
  else
    Offset += 2; // Skip the saved FP

  // Fold imm into offset
  Offset += MI.getOperand(FIOperandNum + 1).getImm();

  if (MI.getOpcode() == LPU::ADD16ri) {
    // This is actually "load effective address" of the stack slot
    // instruction. We have only two-address instructions, thus we need to
    // expand it into mov + add
    const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

    MI.setDesc(TII.get(LPU::MOV16rr));
    MI.getOperand(FIOperandNum).ChangeToRegister(BasePtr, false);

    if (Offset == 0)
      return;

    // We need to materialize the offset via add instruction.
    unsigned DstReg = MI.getOperand(0).getReg();
    if (Offset < 0)
      BuildMI(MBB, std::next(II), dl, TII.get(LPU::SUB16ri), DstReg)
        .addReg(DstReg).addImm(-Offset);
    else
      BuildMI(MBB, std::next(II), dl, TII.get(LPU::ADD16ri), DstReg)
        .addReg(DstReg).addImm(Offset);

    return;
  }

  MI.getOperand(FIOperandNum).ChangeToRegister(BasePtr, false);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
*/
}

unsigned LPURegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  /*
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();

  return TFI->hasFP(MF) ? LPU::FP : LPU::SP;
  */
  return LPU::R0;
}

