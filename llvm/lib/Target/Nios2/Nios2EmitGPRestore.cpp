//===-- Nios2EmitGPRestore.cpp - Emit GP Restore Instruction ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass emits instructions that restore $gp right
// after jalr instructions.
//
//===----------------------------------------------------------------------===//

#include "Nios2.h"
#ifdef ENABLE_GPRESTORE

#include "Nios2TargetMachine.h"
#include "Nios2MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/ADT/Statistic.h"

using namespace llvm;

#define DEBUG_TYPE "emit-gp-restore"

namespace {
  struct Inserter : public MachineFunctionPass {

    TargetMachine &TM;

    static char ID;
    Inserter(TargetMachine &tm)
      : MachineFunctionPass(ID), TM(tm) { }

    StringRef getPassName() const override { return "Nios2 Emit GP Restore"; }    //virtual const char *getPassName() const {
//      return "Nios2 Emit GP Restore";
//    }

    bool runOnMachineFunction(MachineFunction &F);
  };
  char Inserter::ID = 0;
} // end of anonymous namespace

bool Inserter::runOnMachineFunction(MachineFunction &F) {
  Nios2FunctionInfo *Nios2FI = F.getInfo<Nios2FunctionInfo>();
  const TargetSubtargetInfo *STI =  TM.getSubtargetImpl(*(F.getFunction()));
  const TargetInstrInfo *TII = STI->getInstrInfo();

  if ((TM.getRelocationModel() != Reloc::PIC_) ||
      (!Nios2FI->globalBaseRegFixed()))
    return false;

  bool Changed = false;
  int FI = Nios2FI->getGPFI();

  for (MachineFunction::iterator MFI = F.begin(), MFE = F.end();
       MFI != MFE; ++MFI) {
    MachineBasicBlock& MBB = *MFI;
    MachineBasicBlock::iterator I = MFI->begin();
    
    /// isEHPad - Indicate that this basic block is entered via an
    /// exception handler.
    // If MBB is a landing pad, insert instruction that restores $gp after
    // EH_LABEL.
    if (MBB.isEHPad()) {
      // Find EH_LABEL first.
      for (; I->getOpcode() != TargetOpcode::EH_LABEL; ++I) ;

      // Insert ld.
      ++I;
      DebugLoc dl = I != MBB.end() ? I->getDebugLoc() : DebugLoc();
      BuildMI(MBB, I, dl, TII->get(Nios2::LDW_R1), Nios2::GP).addFrameIndex(FI) // TODO: nios2 r2 support
                                                       .addImm(0);
      Changed = true;
    }

    while (I != MFI->end()) {
      if (I->getOpcode() != Nios2::CALLR_R1) { // TODO: Nios2 R2 support
        ++I;
        continue;
      }

      DebugLoc dl = I->getDebugLoc();
      // emit ld $gp, ($gp save slot on stack) after jalr
      BuildMI(MBB, ++I, dl, TII->get(Nios2::LDW_R1), Nios2::GP).addFrameIndex(FI) // TODO: nios2 r2 support
                                                         .addImm(0);
      Changed = true;
    }
  }

  return Changed;
}

/// createNios2EmitGPRestorePass - Returns a pass that emits instructions that
/// restores $gp clobbered by jalr instructions.
FunctionPass *llvm::createNios2EmitGPRestorePass(Nios2TargetMachine &tm) {
  return new Inserter(tm);
}

#endif
