//===-- CSARegisterInfo.cpp - CSA Register Information --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the CSA implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "CSARegisterInfo.h"
#include "CSA.h"
#include "CSAMachineFunctionInfo.h"
#include "CSATargetMachine.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"

using namespace llvm;

#define DEBUG_TYPE "csa-reg-info"

#define GET_REGINFO_TARGET_DESC
#include "CSAGenRegisterInfo.inc"

// FIXME: Provide proper call frame setup / destroy opcodes.
CSARegisterInfo::CSARegisterInfo(const TargetInstrInfo &tii)
  : CSAGenRegisterInfo(CSA::RA), TII(tii) {}


const MCPhysReg*
CSARegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  static const MCPhysReg CalleeSavedRegs[] = {
    0
  };
  static const MCPhysReg CalleeSavedRegsFP[] = {
    CSA::FP,
    0
  };

  const TargetFrameLowering *TFI = MF->getSubtarget().getFrameLowering();
  return (TFI->hasFP(*MF)) ? CalleeSavedRegsFP : CalleeSavedRegs;
}

BitVector CSARegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  const TargetFrameLowering *TFL = MF.getSubtarget().getFrameLowering();

  BitVector Reserved(getNumRegs());
  Reserved.set(CSA::TP);
  Reserved.set(CSA::SP);
  Reserved.set(CSA::RA);

  // The frame pointer register is reserved, but only if we have a frame.
  if (TFL->hasFP(MF))
    Reserved.set(CSA::FP);

  return Reserved;
}

const TargetRegisterClass *
CSARegisterInfo::getPointerRegClass(const MachineFunction &MF, unsigned Kind)
                                                                         const {
  return &CSA::I64RegClass;
}

void
CSARegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                        int SPAdj, unsigned FIOperandNum,
                                        RegScavenger *RS) const {
  MachineInstr    &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();

  unsigned opndNum = 0;
  while (!MI.getOperand(opndNum).isFI()) {
    ++opndNum;
    assert(opndNum < MI.getNumOperands() && "Instr doesn't have FrameIndex operand!");
  }

  DEBUG(llvm::errs() << "\nFunction : " << MF.getFunction()->getName() << "\n";
        llvm::errs() << "<--------->\n" << MI);

  unsigned opc = MI.getOpcode();

  int FrameIndex = MI.getOperand(opndNum).getIndex();
  int StackSize  = MF.getFrameInfo().getStackSize();
  int spOffset   = MF.getFrameInfo().getObjectOffset(FrameIndex);
  // Through here matches MIPS - then eliminateFI(MI, i, FrameIndex, stackSize, spOffset)
  int ArgSize    = MF.getFrameInfo().getMaxCallFrameSize();
  ArgSize = (ArgSize + 7) & (-8); // Align to 8 bytes
  // If variable sized objects, the outgoing arguments are below the variable allocation
  // and do not figure into the stack offsets for the fixed part of the frame...
  if (MF.getFrameInfo().hasVarSizedObjects())
    ArgSize = 0;
  int Offset     = spOffset < 0 ? -spOffset+StackSize-8 : spOffset+ArgSize;
  // If this is something other than a move, it should have a displacement/literal with it
  if (opc != CSA::MOV64) {
    Offset += MI.getOperand(opndNum+1).getImm();
  }

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
    MI.getOperand(opndNum).ChangeToRegister(CSA::SP, false /* isDef */);
    MI.getOperand(opndNum+1).ChangeToImmediate(Offset);
    DEBUG(errs() << "Debug value, changed to register and ignored\n");
    return;
  }

  unsigned new_mem_opc = 0;
  bool     new_is_st = false;
  bool changeToMove = false;
  switch(opc) {
  case CSA::MOV64:
    if (!Offset) { // no offset - leave as MOV
      MI.getOperand(opndNum).ChangeToRegister(getFrameRegister(MF), false);
    } else {
      // Non-0 offset - change to add with offset
      MI.setDesc(TII.get(CSA::ADD64));
      MI.getOperand(opndNum).ChangeToRegister(getFrameRegister(MF), false);
      MI.addOperand(MachineOperand::CreateImm(Offset));
      DEBUG(errs() << "Converted MOV to ADD immediate: "<<Offset<<"\n");
    }
    return;
    // These were ADD64i/SUB64i.  Is this still valid?
  case CSA::ADD64:
  case CSA::SUB64:
    changeToMove = (Offset == 0) ? true : false;
    break;
  case CSA::LD8:    new_mem_opc = CSA::LD8D;    break;
  case CSA::LD16:   new_mem_opc = CSA::LD16D;   break;
  case CSA::LD16f:  new_mem_opc = CSA::LD16fD;  break;
  case CSA::LD32:   new_mem_opc = CSA::LD32D;   break;
  case CSA::LD32f:  new_mem_opc = CSA::LD32fD;  break;
  case CSA::LD64:   new_mem_opc = CSA::LD64D;   break;
  case CSA::LD64f:  new_mem_opc = CSA::LD64fD;  break;
  case CSA::ST8:    new_mem_opc = CSA::ST8D;    new_is_st = true;  break;
  case CSA::ST16:   new_mem_opc = CSA::ST16D;   new_is_st = true;  break;
  case CSA::ST16f:  new_mem_opc = CSA::ST16fD;  new_is_st = true;  break;
  case CSA::ST32:   new_mem_opc = CSA::ST32D;   new_is_st = true;  break;
  case CSA::ST32f:  new_mem_opc = CSA::ST32fD;  new_is_st = true;  break;
  case CSA::ST64:   new_mem_opc = CSA::ST64D;   new_is_st = true;  break;
  case CSA::ST64f:  new_mem_opc = CSA::ST64fD;  new_is_st = true;  break;
  default:
    break;
  }

  if (changeToMove) {
    BuildMI(*MI.getParent(), II, MI.getDebugLoc(), TII.get(CSA::MOV64),
            MI.getOperand(0).getReg())
      .addReg(getFrameRegister(MF));
    II->getParent()->erase(II);
    DEBUG(errs() << "Changing to move\n");
  }
  else {
    // If new_mem_opc is set, we need to convert from a non-displacement to
    // displacement form.  e.g.:
    //  ldx v, a => ldxD v, a, d
    //  stx ack, a, v => stxD ack, a, d, v
    if (new_mem_opc) {
      MI.setDesc(TII.get(new_mem_opc));
      if (new_is_st) {
        // For stores, move the current operand 2 to 3, and insert a disp of 0
        MI.addOperand( MI.getOperand(2) );
        MI.getOperand(2).ChangeToImmediate(0);
      } else {
        MI.addOperand(MachineOperand::CreateImm(0));
      }
    }
    MI.getOperand(opndNum).ChangeToRegister(getFrameRegister(MF), false);
    MI.getOperand(opndNum+1).ChangeToImmediate(Offset);
    DEBUG(errs() << "Changed to immediate: "<<Offset<<"\n");
  }
}

unsigned CSARegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();
  return TFI->hasFP(MF) ? CSA::FP : CSA::SP;
}
