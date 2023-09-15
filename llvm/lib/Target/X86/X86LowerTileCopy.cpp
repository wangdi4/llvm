//===-- X86LowerTileCopy.cpp - Expand Tile Copy Instructions---------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the pass which lower AMX tile copy instructions. Since
// there is no tile copy instruction, we need store tile register to stack
// and load from stack to another tile register. We need extra GR to hold
// the stride, and we need stack slot to hold the tile data register.
// We would run this pass after copy propagation, so that we don't miss copy
// optimization. And we would run this pass before prolog/epilog insertion,
// so that we can allocate stack slot.
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrBuilder.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineDominators.h" // INTEL
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h" // INTEL

using namespace llvm;

#define DEBUG_TYPE "x86-lower-tile-copy"

namespace {

class X86LowerTileCopy : public MachineFunctionPass {
  MachineDominatorTree *MDT = nullptr; // INTEL

public:
  static char ID;

  X86LowerTileCopy() : MachineFunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  bool runOnMachineFunction(MachineFunction &MF) override;
#if INTEL_CUSTOMIZATION
  bool coalesceTileCopy(MachineFunction &MF);
  bool transformTileCopy(MachineFunction &MF);

  bool replaceTileReg(MachineBasicBlock::iterator Begin,
                      MachineBasicBlock::iterator End,
                      Register CopySrcReg, Register CopyDstReg);
#endif // INTEL_CUSTOMIZATION

  StringRef getPassName() const override { return "X86 Lower Tile Copy"; }
};

} // namespace

char X86LowerTileCopy::ID = 0;

INITIALIZE_PASS_BEGIN(X86LowerTileCopy, "lowertilecopy", "Tile Copy Lowering",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree) // INTEL
INITIALIZE_PASS_END(X86LowerTileCopy, "lowertilecopy", "Tile Copy Lowering",
                    false, false)

void X86LowerTileCopy::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<MachineDominatorTree>(); // INTEL
  AU.setPreservesAll();
  MachineFunctionPass::getAnalysisUsage(AU);
}

FunctionPass *llvm::createX86LowerTileCopyPass() {
  return new X86LowerTileCopy();
}

#if INTEL_CUSTOMIZATION
bool X86LowerTileCopy::replaceTileReg(MachineBasicBlock::iterator Begin,
                                      MachineBasicBlock::iterator End,
                                      Register CopySrcReg,
                                      Register CopyDstReg) {
  for (MachineBasicBlock::iterator MII = Begin; MII != End; MII++) {
    MachineInstr &MI = *MII;
    if (MI.getOpcode() == X86::PLDTILECFGV)
      break;

    for (unsigned I = 0; I < MI.getNumOperands(); I++) {
      MachineOperand &MO = MI.getOperand(I);
      if (!MO.isReg())
        continue;
      Register Reg = MO.getReg();
      if (Reg != CopyDstReg)
        continue;

      assert(MI.getOpcode() == X86::PTILESTOREDV && "Only expected tilestore!");
      MO.setReg(CopySrcReg);
      return true;
    }
  }
  return false;
}

// In O0, COPY of tile should be coalesced in fast register allocation.
// But for sub-reg of tile-pair, COPY didn't coalesced. These COPYs do
// nothing but just will be store after the key-amx instruction. Here to
// escape config these COPY dest tile registers, we coalesce them.
bool X86LowerTileCopy::coalesceTileCopy(MachineFunction &MF) {
  bool Changed = false;
  for (MachineBasicBlock &MBB : MF) {
    SmallVector<MachineInstr *> TileCopies;
    for (MachineBasicBlock::iterator MII = MBB.begin(), MIE = MBB.end();
         MII != MIE;) {
      MachineInstr &MI = *MII++;
      if (!MI.isCopy())
        continue;
      MachineOperand &DstMO = MI.getOperand(0);
      MachineOperand &SrcMO = MI.getOperand(1);
      Register SrcReg = SrcMO.getReg();
      Register DstReg = DstMO.getReg();
      if (!X86::TILERegClass.contains(DstReg, SrcReg))
        continue;
      replaceTileReg(MII, MIE, SrcReg, DstReg);
      TileCopies.push_back(&MI);
      Changed = true;
    }

    for (auto *MI : TileCopies)
      MBB.erase(MI);
  }
  return Changed;
}

static bool isDefinedOnceByTileZero(const MachineOperand &Operand,
                                    const MachineFunction &MF,
                                    MachineDominatorTree *MDT,
                                    const MachineInstr &TC) {
  // Not a register operand, so it cannot be defined by TileZero.
  if (!Operand.isReg())
    return false;
  unsigned int RegisterNumber = Operand.getReg();
  bool HasConfig = false;
  bool HasTileZero = false;

  // FIXME: Should Traverse all the path from root to TC.getParent(),
  // check the nearest definition is TILEZERO.
  // For now, we only search for A's only definition dominate "B = tilecopy A"
  const MachineRegisterInfo *MRI = &MF.getRegInfo();
  if (std::distance(MRI->def_instr_begin(RegisterNumber),
                    MRI->def_instr_end()) != 2)
    return false;
  for (MachineInstr &DefMI : MRI->def_instructions(RegisterNumber)) {
    if (DefMI.getOpcode() == X86::PLDTILECFGV) {
#ifdef EXPENSIVE_CHECKS
      assert(MDT->dominates(&DefMI, &TC) &&
             "PLDTILECFGV must dominate B = tilecopy A!");
#endif
      HasConfig = true;
      continue;
    }
    // Check if the register is defined by TileZero in this instruction
    if (DefMI.getOpcode() == X86::PTILEZEROV && MDT->dominates(&DefMI, &TC))
      HasTileZero = true;
  }
  return HasConfig && HasTileZero;
}

bool X86LowerTileCopy::transformTileCopy(MachineFunction &MF) {
  const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
  const X86InstrInfo *TII = ST.getInstrInfo();
  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : llvm::make_early_inc_range(MBB)) {
      if (!MI.isCopy())
        continue;
      MachineOperand &DstMO = MI.getOperand(0);
      MachineOperand &SrcMO = MI.getOperand(1);
      Register SrcReg = SrcMO.getReg();
      Register DstReg = DstMO.getReg();
      if (!X86::TILERegClass.contains(DstReg, SrcReg))
        continue;

      // directly transform B = tilecopy A into tilezero if A is defined once:
      // bb1:
      //  A = tilezero
      // bb2:
      //  B = tilecopy A
      const DebugLoc &DL = MI.getDebugLoc();
      if (isDefinedOnceByTileZero(SrcMO, MF, MDT, MI)) {
        BuildMI(MBB, MI, DL, TII->get(X86::TILEZERO), DstReg);
        MI.eraseFromParent();
        Changed = true;
        continue;
      }
      const TargetRegisterInfo *TRI = ST.getRegisterInfo();
      // Allocate stack slot for tile register
      unsigned Size = TRI->getSpillSize(X86::TILERegClass);
      Align Alignment = TRI->getSpillAlign(X86::TILERegClass);
      int TileSS = MF.getFrameInfo().CreateSpillStackObject(Size, Alignment);
      // Allocate stack slot for stride register
      Size = TRI->getSpillSize(X86::GR64RegClass);
      Alignment = TRI->getSpillAlign(X86::GR64RegClass);
      int StrideSS = MF.getFrameInfo().CreateSpillStackObject(Size, Alignment);

      // TODO: Pick a killed regiter to avoid save/reload. There is problem
      // to get live interval in this stage.
      Register GR64Cand = X86::RAX;

      // mov %rax (%sp)
      BuildMI(MBB, MI, DL, TII->get(X86::IMPLICIT_DEF), GR64Cand);
      addFrameReference(BuildMI(MBB, MI, DL, TII->get(X86::MOV64mr)), StrideSS)
          .addReg(GR64Cand);
      // mov 64 %rax
      BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri), GR64Cand).addImm(64);
      // tilestored %tmm, (%sp, %idx)
      unsigned Opc = X86::TILESTORED;
      MachineInstr *NewMI =
          addFrameReference(BuildMI(MBB, MI, DL, TII->get(Opc)), TileSS)
              .addReg(SrcReg, getKillRegState(SrcMO.isKill()));
      MachineOperand &MO = NewMI->getOperand(2);
      MO.setReg(GR64Cand);
      MO.setIsKill(true);
      // tileloadd (%sp, %idx), %tmm
      Opc = X86::TILELOADD;
      NewMI = addFrameReference(BuildMI(MBB, MI, DL, TII->get(Opc), DstReg),
                                TileSS);
      // restore %rax
      // mov (%sp) %rax
      addFrameReference(BuildMI(MBB, MI, DL, TII->get(X86::MOV64rm), GR64Cand),
                        StrideSS);
      MI.eraseFromParent();
      Changed = true;
    }
  }
  return Changed;
}

bool X86LowerTileCopy::runOnMachineFunction(MachineFunction &MF) {
  MDT = &getAnalysis<MachineDominatorTree>();
  if (MF.getTarget().getOptLevel() == CodeGenOptLevel::None)
    return coalesceTileCopy(MF);
  else
    return transformTileCopy(MF);
}
#endif // INTEL_CUSTOMIZATION
