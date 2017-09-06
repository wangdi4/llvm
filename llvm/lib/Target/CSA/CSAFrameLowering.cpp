//===-- CSAFrameLowering.cpp - CSA Frame Information ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the CSA implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "frame-lower"

#include "CSAFrameLowering.h"
#include "CSAInstrInfo.h"
#include "CSAMachineFunctionInfo.h"
#include "CSASubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

// These are the register flags that ought to be used when specifying %ign as
// an memory ordering output channel when constructing memory operations.
// RegState::Dead is used here because values stored into %ign are dead but
// LLVM won't know that unless they're explicitly marked.
constexpr unsigned ISSUED_REGSTATE = RegState::Define | RegState::Dead;

void CSAFrameLowering::emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const {
  assert(&MBB == &MF.front() && "Shrink-wrapping not yet implemented");
  MachineFrameInfo &MFI  = MF.getFrameInfo();
  CSAMachineFunctionInfo *LMFI  = MF.getInfo<CSAMachineFunctionInfo>();
  const CSAInstrInfo &TII =
    *static_cast<const CSAInstrInfo*>(MF.getSubtarget().getInstrInfo());
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

  // Get the number of bytes to allocate from the FrameInfo.
  unsigned StackSize     = MFI.getStackSize();
  unsigned CallFrameSize = MFI.getMaxCallFrameSize();

  // We are going to round CallFrameSize so that it is a multiple
  // of 8 so that the RA can be stored at a correct offset
  // Note that CallFrameSize is not initially a multiple of 8 because
  // for varargs, we allow slots to be of size 4 for example and overflow
  // arguments (not in registers after 8) can be of any size
  CallFrameSize = (CallFrameSize + 7) & (-8);

  // If this is a dynamic frame, the outbound arguments are allocated
  // below the dynamic portion, and do not figure into the offsets
  if (hasFP(MF))
    CallFrameSize = 0;

  //// This seems like a fix???
  ////  StackSize += CallFrameSize;

  DEBUG(errs() << "Stack info:\n"
               << "StackSize  : " << StackSize << "\n"
	      << "CallFrameSize : " << CallFrameSize << "\n");

  if (StackSize == 0) return;

  // Adjust stack : sub sp, sp, imm
  BuildMI(MBB, MBBI, dl, TII.get(CSA::SUB64), CSA::SP)
    .addReg(CSA::SP).addImm(StackSize);

  if (MFI.hasCalls()) {
    assert(LMFI->getRAFrameIndex() != -1
           && "No spill location for Return Address created.");

    // store64  stack_loc($sp), $ra
    int RAOffset =
      MFI.getObjectOffset(LMFI->getRAFrameIndex()) + CallFrameSize;
    assert(RAOffset%8 == 0 && "RA offset not multiple of 8");
    DEBUG(errs() << "RAOffset : " << RAOffset << "\n");

    if (RAOffset >= (int)StackSize) {
	assert(RAOffset < (int)StackSize && "Bad RA offset");
    }

    BuildMI(MBB, MBBI, dl, TII.get(CSA::ST64D)).addReg(CSA::IGN, ISSUED_REGSTATE)
      .addReg(CSA::SP).addImm(RAOffset).addReg(CSA::RA).addImm(CSA::MEMLEVEL_T0)
      .addReg(CSA::IGN);
  }

  if (hasFP(MF)) {
    assert(LMFI->getFPFrameIndex() != -1
           && "No spill location found for Frame Pointer.");

    // store64  stack_loc($sp), $fp
    int FPOffset =
      MFI.getObjectOffset(LMFI->getFPFrameIndex()) + CallFrameSize;

    assert(FPOffset%8 == 0 && "FP offset not multiple of 8");
    if (FPOffset >= (int)StackSize) {
	DEBUG(errs() << "Stack FPOffset bug!\n"
	      << "FPOffset : " << FPOffset << "\n"
              << "StackSize  : " << StackSize << "\n"
	      << "CallFrameSize : " << CallFrameSize << "\n");
	assert(FPOffset < (int)StackSize && "Bad FP offset");
    }

    BuildMI(MBB, MBBI, dl, TII.get(CSA::ST64D)).addReg(CSA::IGN, ISSUED_REGSTATE)
      .addReg(CSA::SP).addImm(FPOffset).addReg(CSA::FP).addImm(CSA::MEMLEVEL_T0)
      .addReg(CSA::IGN);

    // move $fp, $sp
    BuildMI(MBB, MBBI, dl, TII.get(CSA::MOV64), CSA::FP)
      .addReg(CSA::SP);
  }

}

void CSAFrameLowering::emitEpilogue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = --MBB.end();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  CSAMachineFunctionInfo *LMFI = MF.getInfo<CSAMachineFunctionInfo>();
  DebugLoc dl = MBBI->getDebugLoc();
  const CSAInstrInfo &TII =
    *static_cast<const CSAInstrInfo*>(MF.getSubtarget().getInstrInfo());

  // Get the number of bytes to allocate from the FrameInfo.
  unsigned StackSize     = MFI.getStackSize();
  unsigned CallFrameSize = MFI.getMaxCallFrameSize();

  CallFrameSize = (CallFrameSize + 7) & (-8);

  // If this is a dynamic frame, the outbound arguments are allocated
  // below the dynamic portion, and do not figure into the offsets
  if (hasFP(MF))
    CallFrameSize = 0;

  if (StackSize == 0) return;

  // if framepointer enabled, restore it and restore the
  // stack pointer
  if (hasFP(MF)) {
    assert(   LMFI->getFPFrameIndex() != -1
           && "No spill location found for Frame Pointer.");

    // move $sp, $fp
    BuildMI(MBB, MBBI, dl, TII.get(CSA::MOV64), CSA::SP)
      .addReg(CSA::FP);

    int FPOffset =
      MFI.getObjectOffset(LMFI->getFPFrameIndex()) + CallFrameSize;
    assert(FPOffset%8 == 0 && "FP offset not multiple of 8");

    BuildMI(MBB, MBBI, dl, TII.get(CSA::LD64D), CSA::FP)
      .addReg(CSA::IGN, ISSUED_REGSTATE)
      .addReg(CSA::SP).addImm(FPOffset).addImm(CSA::MEMLEVEL_T0).addReg(CSA::IGN);
  }

  if (MFI.hasCalls()) {
    assert(   LMFI->getRAFrameIndex() != -1
           && "No spill location for Return Address created.");

    int RAOffset =
      MFI.getObjectOffset(LMFI->getRAFrameIndex()) + CallFrameSize;

    assert(RAOffset%8 == 0 && "RA offset not multiple of 8");

    BuildMI(MBB, MBBI, dl, TII.get(CSA::LD64D), CSA::RA)
      .addReg(CSA::IGN, ISSUED_REGSTATE)
      .addReg(CSA::SP).addImm(RAOffset).addImm(CSA::MEMLEVEL_T0).addReg(CSA::IGN);
  }

  // Adjust stack : add sp, sp, imm
  BuildMI(MBB, MBBI, dl, TII.get(CSA::ADD64), CSA::SP)
    .addReg(CSA::SP).addImm(StackSize);
}

bool CSAFrameLowering::hasFP(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();

  // No frame pointer unless really needed...
  return (//MF.getTarget().Options.DisableFramePointerElim(MF) ||
          MF.getFrameInfo().hasVarSizedObjects() ||
          MFI.isFrameAddressTaken());
}

void CSAFrameLowering::
processFunctionBeforeFrameFinalized(MachineFunction &MF,
                                    RegScavenger *RS) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  CSAMachineFunctionInfo *LMFI = MF.getInfo<CSAMachineFunctionInfo>();

  if (hasFP(MF))
    LMFI->setFPFrameIndex(MFI.CreateSpillStackObject(8, 8));

  if (MFI.hasCalls())
    LMFI->setRAFrameIndex(MFI.CreateSpillStackObject(8, 8));
}

MachineBasicBlock::iterator CSAFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {
  const CSAInstrInfo &TII = *MF.getSubtarget<CSASubtarget>().getInstrInfo();

  if (!hasReservedCallFrame(MF)) {
    int64_t Amount = I->getOperand(0).getImm();

    if (Amount) {
      // Keep the stack 16 byte aligned
      Amount = (Amount + 7) & (-8);

      DebugLoc DL = I != MBB.end() ? I->getDebugLoc() : DebugLoc();

      if (I->getOpcode() == CSA::ADJCALLSTACKDOWN) {
        BuildMI(MBB, I, DL, TII.get(CSA::SUB64), CSA::SP).addReg(CSA::SP).
          addImm(Amount);
      } else {
        BuildMI(MBB, I, DL, TII.get(CSA::ADD64), CSA::SP).addReg(CSA::SP).
          addImm(Amount);
      }
    }
  }

  return MBB.erase(I);
}

