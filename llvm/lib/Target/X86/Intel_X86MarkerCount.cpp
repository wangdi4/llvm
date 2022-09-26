#if INTEL_FEATURE_MARKERCOUNT
//=- Intel_X86MarkerCount.cpp - Convert pseudo marker count to X86 instruction -=//
//
// Copyright (C) 2016-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements X86MarkerCount pass, which transform pseudo
// marker count into X86 instruction.
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrBuilder.h"
#include "X86Subtarget.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/Function.h"

#define DEBUG_TYPE "x86-markercount"

using namespace llvm;

namespace {
class X86MarkerCountPass : public MachineFunctionPass {
public:
  static char ID;

  X86MarkerCountPass() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override { return "X86 marker count"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};
} // namespace

bool X86MarkerCountPass::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()))
    return false;

  const X86Subtarget *Subtarget = &MF.getSubtarget<X86Subtarget>();
  const X86InstrInfo *TII = Subtarget->getInstrInfo();
  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      unsigned Opcode = MI.getOpcode();
      if (Opcode == TargetOpcode::PSEUDO_LOOP_HEADER) {
        MI.setDesc(TII->get(X86::MARKER_COUNT_LOOP_HEADER));
        Changed = true;
      } else if (Opcode == TargetOpcode::PSEUDO_FUNCTION_PROLOG) {
        MI.setDesc(TII->get(X86::MARKER_COUNT_FUNCTION));
        MI.setAsmPrinterFlag(X86::AC_PROLOG);
        Changed = true;
      } else if (Opcode == TargetOpcode::PSEUDO_FUNCTION_EPILOG) {
        MI.setDesc(TII->get(X86::MARKER_COUNT_FUNCTION));
        MI.setAsmPrinterFlag(X86::AC_EPILOG);
        Changed = true;
      }
    }
  }

  return Changed;
}

char X86MarkerCountPass::ID = 0;
INITIALIZE_PASS_BEGIN(X86MarkerCountPass, DEBUG_TYPE, "X86 marker count", false,
                      false)
INITIALIZE_PASS_END(X86MarkerCountPass, DEBUG_TYPE, "X86 marker count", false,
                    false)

FunctionPass *llvm::createX86MarkerCountPass() {
  return new X86MarkerCountPass();
}
#endif // INTEL_FEATURE_MARKERCOUNT
