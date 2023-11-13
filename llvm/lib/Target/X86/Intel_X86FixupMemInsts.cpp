//===----------- Target/X86/Intel_X86FixupMemInsts.cpp ----------*- C++ -*-===//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrInfo.h"
#include "X86RegisterInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

#define DEBUG_TYPE "x86-fixup-mem-insts"
#define PASS_DESC "X86 Fixup memory instructions"

static cl::opt<bool> FixupMem("x86-fixup-mem", cl::init(false),
    cl::desc("Enable X86 Fixup memory instructions"), cl::Hidden);

namespace {
class X86FixupMemInstsPass : public MachineFunctionPass {
public:
  static char ID;

  StringRef getPassName() const override { return PASS_DESC; }

  X86FixupMemInstsPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  const X86Subtarget *STI = nullptr;
  const X86InstrInfo *TII = nullptr;
};

} // end anonymous namespace

char X86FixupMemInstsPass::ID = 0;

INITIALIZE_PASS(X86FixupMemInstsPass, DEBUG_TYPE, PASS_DESC, false, false)

FunctionPass *llvm::createX86FixupMemInstsPass() {
  return new X86FixupMemInstsPass();
}

// grep -E '^def .*m.*X86Inst|AsmString' isel.tmp | grep X86Inst -A1 | grep -E
// '"mov|"add|"sub|"and|"or|"xor|"cmp|"test' -B1 | grep -v LOCK | grep -v '\--'
// | grep X86Inst | cut -d ' ' -f 2
const unsigned PopularOpcodes[] = {
    X86::ADD16mi,      X86::ADD16mi8, X86::ADD16mr,      X86::ADD16rm,
    X86::ADD32mi,      X86::ADD32mi8, X86::ADD32mr,      X86::ADD32rm,
    X86::ADD64mi32,    X86::ADD64mi8, X86::ADD64mr,      X86::ADD64rm,
    X86::ADD8mi,       X86::ADD8mi8,  X86::ADD8mr,       X86::ADD8rm,
    X86::AND16mi,      X86::AND16mi8, X86::AND16mr,      X86::AND16rm,
    X86::AND32mi,      X86::AND32mi8, X86::AND32mr,      X86::AND32rm,
    X86::AND64mi32,    X86::AND64mi8, X86::AND64mr,      X86::AND64rm,
    X86::AND8mi,       X86::AND8mi8,  X86::AND8mr,       X86::AND8rm,
    X86::CMP16mi,      X86::CMP16mi8, X86::CMP16mr,      X86::CMP16rm,
    X86::CMP32mi,      X86::CMP32mi8, X86::CMP32mr,      X86::CMP32rm,
    X86::CMP64mi32,    X86::CMP64mi8, X86::CMP64mr,      X86::CMP64rm,
    X86::CMP8mi,       X86::CMP8mi8,  X86::CMP8mr,       X86::CMP8rm,
    X86::MOV16mi,      X86::MOV16mr,  X86::MOV16ms,      X86::MOV16rm,
    X86::MOV16sm,      X86::MOV32mi,  X86::MOV32mr,      X86::MOV32rm,
    X86::MOV64mi32,    X86::MOV64mr,  X86::MOV64rm,      X86::MOV64toPQIrm,
    X86::MOV8mi,       X86::MOV8mr,   X86::MOV8mr_NOREX, X86::MOV8rm,
    X86::MOV8rm_NOREX, X86::OR16mi,   X86::OR16mi8,      X86::OR16mr,
    X86::OR16rm,       X86::OR32mi,   X86::OR32mi8,      X86::OR32mr,
    X86::OR32rm,       X86::OR64mi32, X86::OR64mi8,      X86::OR64mr,
    X86::OR64rm,       X86::OR8mi,    X86::OR8mi8,       X86::OR8mr,
    X86::OR8rm,        X86::ORPDrm,   X86::ORPSrm,       X86::SUB16mi,
    X86::SUB16mi8,     X86::SUB16mr,  X86::SUB16rm,      X86::SUB32mi,
    X86::SUB32mi8,     X86::SUB32mr,  X86::SUB32rm,      X86::SUB64mi32,
    X86::SUB64mi8,     X86::SUB64mr,  X86::SUB64rm,      X86::SUB8mi,
    X86::SUB8mi8,      X86::SUB8mr,   X86::SUB8rm,       X86::TEST16mi,
    X86::TEST16mr,     X86::TEST32mi, X86::TEST32mr,     X86::TEST64mi32,
    X86::TEST64mr,     X86::TEST8mi,  X86::TEST8mr,      X86::XOR16mi,
    X86::XOR16mi8,     X86::XOR16mr,  X86::XOR16rm,      X86::XOR32mi,
    X86::XOR32mi8,     X86::XOR32mr,  X86::XOR32rm,      X86::XOR64mi32,
    X86::XOR64mi8,     X86::XOR64mr,  X86::XOR64rm,      X86::XOR8mi,
    X86::XOR8mi8,      X86::XOR8mr,   X86::XOR8rm};

bool X86FixupMemInstsPass::runOnMachineFunction(MachineFunction &MF) {
  STI = &MF.getSubtarget<X86Subtarget>();
  TII = STI->getInstrInfo();
  if (!FixupMem || !STI->is64Bit())
    return false;

  bool Changed = false;
  const unsigned *S = PopularOpcodes;
  const unsigned *E = PopularOpcodes + sizeof(PopularOpcodes);
  for (MachineBasicBlock &MBB : MF) {
    SmallSet<unsigned, 2> FreeRegs;
    FreeRegs.insert(X86::R10);
    FreeRegs.insert(X86::R11);
    auto UpdateFreeReg = [&FreeRegs](unsigned Reg) {
      if (Reg == X86::R10B || Reg == X86::R10W || Reg == X86::R10D ||
          Reg == X86::R10)
        FreeRegs.erase(X86::R10);
      if (Reg == X86::R11B || Reg == X86::R11W || Reg == X86::R11D ||
          Reg == X86::R11)
        FreeRegs.erase(X86::R11);
    };
    for (MachineBasicBlock::RegisterMaskPair RegMask : MBB.liveins()) {
      UpdateFreeReg(RegMask.PhysReg);
    }
    for (MachineBasicBlock::iterator I = MBB.begin(); I != MBB.end(); ++I) {
      MachineInstr &MI = *I;

      if (FreeRegs.size())
        for (auto &Def : MI.defs())
          if (Def.isReg())
            UpdateFreeReg(Def.getReg());

      // Instruction must be load or store.
      if (!MI.mayLoadOrStore())
        continue;

      // Get the number of the first memory operand.
      const MCInstrDesc &Desc = MI.getDesc();
      int MemOpNo = X86II::getMemoryOperandNo(Desc.TSFlags);

      // If instruction has no memory operand - skip it.
      if (MemOpNo < 0)
        continue;

      MemOpNo += X86II::getOperandBias(Desc);
      unsigned BaseReg = MI.getOperand(MemOpNo + X86::AddrBaseReg).getReg();
      if (BaseReg == X86::RSP || BaseReg == X86::RIP)
        continue;

      unsigned IndexReg = MI.getOperand(MemOpNo + X86::AddrIndexReg).getReg();
      if (std::binary_search(S, E, MI.getOpcode()) && FreeRegs.size()) {
        unsigned Reg = *(FreeRegs.begin());
        if (IndexReg != X86::NoRegister) {
          BuildMI(MBB, I, I->getDebugLoc(), TII->get(X86::LEA64r), Reg)
              .addReg(BaseReg)
              .addImm(MI.getOperand(MemOpNo + X86::AddrScaleAmt).getImm())
              .addReg(IndexReg)
              .addDisp(MI.getOperand(MemOpNo + X86::AddrDisp), 0)
              .addReg(0);
          MI.getOperand(MemOpNo + X86::AddrBaseReg).setReg(Reg);
          MI.getOperand(MemOpNo + X86::AddrScaleAmt).setImm(1);
          MI.getOperand(MemOpNo + X86::AddrIndexReg).setReg(0);
          MI.getOperand(MemOpNo + X86::AddrDisp).ChangeToImmediate(0);
          Changed = true;
          continue;
        }

        if (Desc.TSFlags & X86II::REX_W)
          continue;

        if (BaseReg >= X86::R8 || IndexReg >= X86::R8)
          continue;

        BuildMI(MBB, I, I->getDebugLoc(), TII->get(X86::NOOP));
        Changed = true;
        continue;
      }

      BuildMI(MBB, I, I->getDebugLoc(), TII->get(X86::NOOPL))
          .addReg(BaseReg)
          .addImm(MI.getOperand(MemOpNo + X86::AddrScaleAmt).getImm())
          .addReg(IndexReg)
          .addDisp(MI.getOperand(MemOpNo + X86::AddrDisp), 0)
          .addReg(0);
      Changed = true;
    }
  }
  return Changed;
}
