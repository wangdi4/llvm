//====-- Intel_X86PRAExpandPseudo - X86 PreRegAlloc PSEUDO expansion ---------====
//
//      Copyright (c) 2016-2020 Intel Corporation.
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
#include "X86Subtarget.h"
#include "X86InstrInfo.h"
#include "X86MachineFunctionInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/Passes.h"

using namespace llvm;

#define DEBUG_TYPE "x86-pra-pseudo"

namespace {

class X86PRAExpandPseudoPass : public MachineFunctionPass {
public:
  X86PRAExpandPseudoPass() : MachineFunctionPass(ID) { }

  StringRef getPassName() const override {
    return "X86 Pre-RA pseudo instruction expansion pass";
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    LLVM_DEBUG(dbgs() << "********** " << getPassName() << " : " << MF.getName()
                      << " **********\n");

    if (MF.begin() == MF.end())
      // Nothing to do for a degenerate empty function...
      return false;

    const X86Subtarget *STI =
        &static_cast<const X86Subtarget &>(MF.getSubtarget());
    const X86InstrInfo *TII = STI->getInstrInfo();
    MachineRegisterInfo &MRI = MF.getRegInfo();
    bool Changed = false;

    for (MachineBasicBlock &MBB : MF) {
      for (MachineBasicBlock::iterator MBBI = MBB.begin(), MBBE = MBB.end();
           MBBI != MBBE;) {
        MachineInstr &MI = *MBBI++;
        unsigned Opcode = MI.getOpcode();
        if (Opcode != X86::KMOVBki && Opcode != X86::KMOVWki)
          continue;
        DebugLoc DL = MI.getDebugLoc();
        bool IsKMOVB = Opcode == X86::KMOVBki;
        Register Sub = MRI.createVirtualRegister(IsKMOVB ? &X86::GR8RegClass
                                                         : &X86::GR16RegClass);
        Register GR32 = MRI.createVirtualRegister(STI->is32Bit() && IsKMOVB
                                                      ? &X86::GR32_ABCDRegClass
                                                      : &X86::GR32RegClass);
        Register UDef = MRI.createVirtualRegister(&X86::GR32RegClass);
        BuildMI(MBB, MI, DL, TII->get(IsKMOVB ? X86::MOV8ri : X86::MOV16ri),
                Sub)
            .addImm(MI.getOperand(1).getImm());
        BuildMI(MBB, MI, DL, TII->get(X86::IMPLICIT_DEF), UDef);
        BuildMI(MBB, MI, DL, TII->get(X86::INSERT_SUBREG), GR32)
            .addReg(UDef, RegState::Kill)
            .addReg(Sub, RegState::Kill)
            .addImm(IsKMOVB ? X86::sub_8bit : X86::sub_16bit);
        BuildMI(MBB, MI, DL, TII->get(IsKMOVB ? X86::KMOVBkr : X86::KMOVWkr),
                MI.getOperand(0).getReg())
            .addReg(GR32, RegState::Kill);
        MI.eraseFromParent();
        Changed = true;
      }
    }

    return Changed;
  }

  /// Pass identification, replacement for typeid.
  static char ID;
};

} // end anonymous namespace

char X86PRAExpandPseudoPass::ID = 0;

INITIALIZE_PASS_BEGIN(X86PRAExpandPseudoPass, DEBUG_TYPE,
                      "X86 Pre-RA pseudo instruction expanding", false, false)
INITIALIZE_PASS_END(X86PRAExpandPseudoPass, DEBUG_TYPE,
                    "X86 Pre-RA pseudo instruction expanding", false, false)

FunctionPass *llvm::createX86PRAExpandPseudoPass() {
  return new X86PRAExpandPseudoPass();
}
