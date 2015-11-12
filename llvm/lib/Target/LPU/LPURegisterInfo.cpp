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
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"

using namespace llvm;

#define DEBUG_TYPE "lpu-reg-info"

#define GET_REGINFO_TARGET_DESC
#include "LPUGenRegisterInfo.inc"

// FIXME: Provide proper call frame setup / destroy opcodes.
LPURegisterInfo::LPURegisterInfo(const TargetInstrInfo &tii)
  : LPUGenRegisterInfo(LPU::RA), TII(tii) {}


const MCPhysReg*
LPURegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  static const MCPhysReg CalleeSavedRegs[] = {
    0
  };
  static const MCPhysReg CalleeSavedRegsFP[] = {
    LPU::FP,
    0
  };

  const TargetFrameLowering *TFI = MF->getSubtarget().getFrameLowering();
  return (TFI->hasFP(*MF)) ? CalleeSavedRegsFP : CalleeSavedRegs;
}

BitVector LPURegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  const TargetFrameLowering *TFL = MF.getSubtarget().getFrameLowering();

  //  Reserved.set(LPU::IGN);
  //  Reserved.set(LPU::C0);
  //  Reserved.set(LPU::C1);

  Reserved.set(LPU::SP);
  Reserved.set(LPU::RA);

  // The frame pointer register is reserved, but only if we have a frame.
  if (TFL->hasFP(MF))
    Reserved.set(LPU::FP);

  return Reserved;
}

void LPURegisterInfo::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const {
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();

  if (!TFI->hasReservedCallFrame(MF)) {
    int64_t Amount = I->getOperand(0).getImm();

    if (Amount) {
      // Keep the stack 8 byte aligned
      Amount = (Amount + 7) & ~7;

      DebugLoc DL = I != MBB.end() ? I->getDebugLoc() : DebugLoc();

      if (I->getOpcode() == LPU::ADJCALLSTACKDOWN) {
	BuildMI(MBB, I, DL, TII.get(LPU::SUB64i), LPU::SP).addReg(LPU::SP).addImm(Amount);
      } else {
	BuildMI(MBB, I, DL, TII.get(LPU::ADD64i), LPU::SP).addReg(LPU::SP).addImm(Amount);
      }
    }
  }

  MBB.erase(I);
}


void
LPURegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                        int SPAdj, unsigned FIOperandNum,
                                        RegScavenger *RS) const {
  MachineInstr    &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();

  unsigned opndNum = 0;
  while (!MI.getOperand(opndNum).isFI()) {
    ++opndNum;
    assert(opndNum < MI.getNumOperands() && "Instr doesn't have FrameIndex operand!");
  }

  DEBUG(errs() << "\nFunction : " << MF.getFunction()->getName() << "\n";
        errs() << "<--------->\n" << MI);

  // If this is already a move, just replace the operand
  if (MI.getOpcode() == LPU::MOV64) {
    MI.getOperand(opndNum).ChangeToRegister(getFrameRegister(MF), false);
    return;
  }

  int FrameIndex = MI.getOperand(opndNum).getIndex();
  int StackSize  = MF.getFrameInfo()->getStackSize();
  int spOffset   = MF.getFrameInfo()->getObjectOffset(FrameIndex);
  // Through here matches MIPS - then eliminateFI(MI, i, FrameIndex, stackSize, spOffset)
  int ArgSize    = MF.getFrameInfo()->getMaxCallFrameSize();
  ArgSize = (ArgSize + 7) & (-8); // Align to 8 bytes
  // If variable sized objects, the outgoing arguments are below the variable allocation
  // and do not figure into the stack offsets for the fixed part of the frame...
  if (MF.getFrameInfo()->hasVarSizedObjects())
    ArgSize = 0;
  int Offset     = spOffset < 0 ? -spOffset+StackSize-8 : spOffset+ArgSize;
  Offset += MI.getOperand(opndNum+1).getImm();

  DEBUG(errs() << "FrameIndex : " << FrameIndex << "\n"
               << "StackSize  : " << StackSize << "\n"
               << "ArgSize    : " << ArgSize << "\n"
               << "spOffset   : " << spOffset << "\n"
               << "Offset     : " << Offset << "\n");

  // Special handling of dbg_value instructions
  // REC: This is copied from ARM's code and hopefully will work
  // at any rate, it should not cause too much problem
  // It basically says that it is doing something to the FrameReg
  // at the offset for this register
  if(MI.isDebugValue()) {
    MI.getOperand(opndNum).ChangeToRegister(LPU::SP, false /* isDef */);
    MI.getOperand(opndNum+1).ChangeToImmediate(Offset);
    DEBUG(errs() << "Debug value, changed to register and ignored\n");
    return;
  }

  unsigned opc = MI.getOpcode();
  bool changeToMove = false;
  switch(opc) {
  case LPU::ADD64i:
  case LPU::SUB64i:
    changeToMove = (Offset == 0) ? true : false;
    break;
  default:
    break;
  }

  if (changeToMove) {
    BuildMI(*MI.getParent(), II, MI.getDebugLoc(), TII.get(LPU::MOV64),
            MI.getOperand(0).getReg())
      .addReg(getFrameRegister(MF));
    II->getParent()->erase(II);
    DEBUG(errs() << "Changing to move\n");
  }
  else {
    MI.getOperand(opndNum).ChangeToRegister(getFrameRegister(MF), false);
    MI.getOperand(opndNum+1).ChangeToImmediate(Offset);
    DEBUG(errs() << "Changed to an immediate (offset fits): "<<Offset<<"\n");
  }
}

unsigned LPURegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();
  return TFI->hasFP(MF) ? LPU::FP : LPU::SP;
}

