//====-- Intel_X86PRAExpandPseudo - X86 PreRegAlloc PSEUDO expansion -------====
//
//      Copyright (c) 2016 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//

#include "X86.h"
#include "X86InstrInfo.h"
#include "X86MachineFunctionInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/Passes.h"

using namespace llvm;

#define DEBUG_TYPE "x86-pra-pseudo"

namespace {

class X86PRAExpandPseudoPass : public MachineFunctionPass {
public:
  X86PRAExpandPseudoPass() : MachineFunctionPass(ID) {}

  const X86Subtarget *STI = nullptr;
  const X86InstrInfo *TII = nullptr;
  const X86RegisterInfo *TRI = nullptr;
  MachineRegisterInfo *MRI = nullptr;

  bool runOnMachineFunction(MachineFunction &MF) override;
  StringRef getPassName() const override {
    return "X86 Pre-RA pseudo instruction expansion pass";
  }

  /// Pass identification, replacement for typeid.
  static char ID;

private:
  bool ExpandMI(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI);
  bool ExpandMBB(MachineBasicBlock &MBB);
};

} // end anonymous namespace

bool X86PRAExpandPseudoPass::runOnMachineFunction(MachineFunction &MF) {
  LLVM_DEBUG(dbgs() << "********** " << getPassName() << " : " << MF.getName()
                    << " **********\n");

  STI = &static_cast<const X86Subtarget &>(MF.getSubtarget());
  TII = STI->getInstrInfo();
  TRI = STI->getRegisterInfo();
  MRI = &MF.getRegInfo();

  bool Changed = false;

  for (MachineBasicBlock &MBB : MF)
    Changed |= ExpandMBB(MBB);
  return Changed;
}

// Expand all pseudo instructions contained in MBB.  Returns true if any
// expansion occurred for MBB.
bool X86PRAExpandPseudoPass::ExpandMBB(MachineBasicBlock &MBB) {
  bool Modified = false;

  // MBBI may be invalidated by the expansion.
  MachineBasicBlock::iterator MBBI = MBB.begin(), E = MBB.end();
  while (MBBI != E) {
    MachineBasicBlock::iterator NMBBI = std::next(MBBI);
    Modified |= ExpandMI(MBB, MBBI);
    MBBI = NMBBI;
  }

  return Modified;
}

// If MBBI is a pseudo instruction, this method expands it to the corresponding
// (sequence of) actual instruction(s). returns true if MBBI has been expanded.
bool X86PRAExpandPseudoPass::ExpandMI(MachineBasicBlock &MBB,
                                      MachineBasicBlock::iterator MBBI) {
  MachineInstr &MI = *MBBI;
  unsigned Opcode = MI.getOpcode();
  const DebugLoc &DL = MBBI->getDebugLoc();
  switch (Opcode) {
  default:
    return false;
  case X86::CPUID_G: {
    // If ebx is reserved, we need to use mov and xchg to preserved it and
    // mov,cpuid,xchg should be bundled. If CPUID_G is expanded here, RA may
    // insert a spill instruction using ebx between cpuid and xchg. This is
    // devastating so we need to expand CPUID_G later in X86ExpandPseudo pass.
    if (TRI->getReservedRegs(*MI.getMF()).test(X86::EBX))
      return false;

    Register VEBX = MBBI->getOperand(0).getReg();
    BuildMI(MBB, MBBI, DL, TII->get(X86::CPUID));
    BuildMI(MBB, MBBI, DL, TII->get(X86::COPY), VEBX).addUse(X86::EBX);
    MI.eraseFromParent();
    return true;
  }
  case X86::KMOVBki:
  case X86::KMOVWki: {
    bool IsKMOVB = Opcode == X86::KMOVBki;
    Register Sub = MRI->createVirtualRegister(IsKMOVB ? &X86::GR8RegClass
                                                      : &X86::GR16RegClass);
    Register GR32 = MRI->createVirtualRegister(STI->is32Bit() && IsKMOVB
                                                   ? &X86::GR32_ABCDRegClass
                                                   : &X86::GR32RegClass);
    Register UDef = MRI->createVirtualRegister(&X86::GR32RegClass);
    BuildMI(MBB, MI, DL, TII->get(IsKMOVB ? X86::MOV8ri : X86::MOV16ri), Sub)
        .addImm(MI.getOperand(1).getImm());
    BuildMI(MBB, MI, DL, TII->get(X86::IMPLICIT_DEF), UDef);
    BuildMI(MBB, MI, DL, TII->get(X86::INSERT_SUBREG), GR32)
        .addReg(UDef, RegState::Kill)
        .addReg(Sub, RegState::Kill)
        .addImm(IsKMOVB ? X86::sub_8bit : X86::sub_16bit);
#if INTEL_CUSTOMIZATION
    bool HasEGPR = STI->hasEGPR();
    BuildMI(MBB, MI, DL,
            TII->get(IsKMOVB ? (HasEGPR ? X86::KMOVBkr_EVEX : X86::KMOVBkr)
                             : (HasEGPR ? X86::KMOVWkr_EVEX : X86::KMOVWkr)),
#endif // INTEL_CUSTOMIZATION
            MI.getOperand(0).getReg())
        .addReg(GR32, RegState::Kill);
    MI.eraseFromParent();
    return true;
  }
  } // end switch
  llvm_unreachable("Previous switch has a fallthrough?");
}

char X86PRAExpandPseudoPass::ID = 0;

INITIALIZE_PASS(X86PRAExpandPseudoPass, DEBUG_TYPE,
                "X86 Pre-RA pseudo instruction expanding", false, false)

FunctionPass *llvm::createX86PRAExpandPseudoPass() {
  return new X86PRAExpandPseudoPass();
}
