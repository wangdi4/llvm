//===-- Nios2InstrInfo.cpp - Nios2 Instruction Information ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Nios2 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "Nios2InstrInfo.h"

#include "Nios2TargetMachine.h"
#include "Nios2MachineFunction.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "Nios2GenInstrInfo.inc"

// Pin the vtable to this file.
void Nios2InstrInfo::anchor() {}

//@Nios2InstrInfo {
Nios2InstrInfo::Nios2InstrInfo(const Nios2Subtarget &STI)
    : 
      Nios2GenInstrInfo(Nios2::ADJCALLSTACKDOWN, Nios2::ADJCALLSTACKUP),
      Subtarget(STI) {}

const Nios2InstrInfo *Nios2InstrInfo::create(Nios2Subtarget &STI) {
  return llvm::createNios2SEInstrInfo(STI);
}

MachineMemOperand *
Nios2InstrInfo::GetMemOperand(MachineBasicBlock &MBB, int FI,
                             MachineMemOperand::Flags Flags) const {

  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  unsigned Align = MFI.getObjectAlignment(FI);

  return MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(MF, FI),
                                 Flags, MFI.getObjectSize(FI), Align);
}

//@GetInstSizeInBytes {
/// Return the number of bytes of code the specified instruction may be.
unsigned Nios2InstrInfo::GetInstSizeInBytes(const MachineInstr &MI) const {
//@GetInstSizeInBytes - body
  switch (MI.getOpcode()) {
  default:
    return MI.getDesc().getSize();
  case  TargetOpcode::INLINEASM: {       // Inline Asm: Variable size.
    const MachineFunction *MF = MI.getParent()->getParent();
    const char *AsmStr = MI.getOperand(0).getSymbolName();
    return getInlineAsmLength(AsmStr, *MF->getTarget().getMCAsmInfo());
  }
  }
}

unsigned Nios2InstrInfo::insertBranch(MachineBasicBlock &MBB,
                                      MachineBasicBlock *TBB,
                                      MachineBasicBlock *FBB,
                                      ArrayRef<MachineOperand> Cond,
                                      const DebugLoc &DL,
                                      int *BytesAdded) const {
  // Shouldn't be a fall through.
  assert(TBB && "insertBranch must not be told to insert a fallthrough");
  assert(!BytesAdded && "code size not handled");

  // # of condition operands:
  //  Unconditional branches: 0
  //  Int Branch: 3 (opc, reg0, reg1)
  assert((Cond.size() <= 3) &&
         "# of Nios2 branch conditions must be <= 3!");

  unsigned uncondBrOpc = Nios2::BR_R1;
  // Two-way Conditional branch.
  if (FBB) {
    BuildCondBr(MBB, TBB, Cond, DL);
    BuildMI(&MBB, DL, get(uncondBrOpc)).addMBB(FBB);
    return 2;
  }

  // One way branch.
  // Unconditional branch.
  if (Cond.empty())
    BuildMI(&MBB, DL, get(uncondBrOpc)).addMBB(TBB);
  else // Conditional branch.
    BuildCondBr(MBB, TBB, Cond, DL);
  return 1;
}

void Nios2InstrInfo::BuildCondBr(MachineBasicBlock &MBB,
                                 MachineBasicBlock *TBB,
                                 ArrayRef<MachineOperand> Cond,
                                 const DebugLoc &DL) const {
  unsigned Opc = Cond[0].getImm();
  const MCInstrDesc &MCID = get(Opc);
  MachineInstrBuilder MIB = BuildMI(&MBB, DL, MCID);

  for (unsigned i = 1; i < Cond.size(); ++i) {
    if (Cond[i].isReg())
      MIB.addReg(Cond[i].getReg());
    else if (Cond[i].isImm())
      MIB.addImm(Cond[i].getImm());
    else
       assert(false && "Cannot copy operand");
  }
  MIB.addMBB(TBB);
}
