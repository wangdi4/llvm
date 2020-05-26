//===-- CSANameLICs.cpp - Propagate DBG_VALUE to LIC names ----------------===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a pass that adds names to LICs based on DBG_VALUE
// instructions.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSAInstrInfo.h"
#include "CSAMachineFunctionInfo.h"
#include "CSATargetMachine.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "csa-name-lics"
#define PASS_NAME "CSA: Name LICs pass"

using namespace llvm;

namespace llvm {
  class CSANameLICsPass : public MachineFunctionPass {
  public:
    static char ID;
    CSANameLICsPass();

    StringRef getPassName() const override {
      return PASS_NAME;
    }

    bool runOnMachineFunction(MachineFunction &MF) override;
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
      MachineFunctionPass::getAnalysisUsage(AU);
    }

  private:
    MachineFunction *MF;
    const MachineRegisterInfo *MRI;
    CSAMachineFunctionInfo *LMFI;
    const CSAInstrInfo *TII;
    StringMap<unsigned> names;

    void nameLIC(MachineInstr &MI);
    void nameTerminator(const MachineBasicBlock &MBB, const MachineOperand &MO);
  };
}

char CSANameLICsPass::ID = 0;

INITIALIZE_PASS(CSANameLICsPass, DEBUG_TYPE, PASS_NAME, false, false)

CSANameLICsPass::CSANameLICsPass() : MachineFunctionPass(ID) {
  initializeCSANameLICsPassPass(*PassRegistry::getPassRegistry());
}


MachineFunctionPass *llvm::createCSANameLICsPass() {
  return new CSANameLICsPass();
}

bool CSANameLICsPass::runOnMachineFunction(MachineFunction &MF) {
  this->MF = &MF;
  MRI = &MF.getRegInfo();
  LMFI = MF.getInfo<CSAMachineFunctionInfo>();
  TII = static_cast<const CSAInstrInfo*>(MF.getSubtarget<CSASubtarget>().getInstrInfo());

  for (auto &MBB : MF) {
    for (auto &MI : MBB) {
      if (MI.isDebugValue())
        nameLIC(MI);
    }
    MachineBasicBlock *TBB = nullptr, *FBB = nullptr;
    SmallVector<MachineOperand, 2> cond;
    if (!TII->analyzeBranch(MBB, TBB, FBB, cond, false) && TBB && FBB) {
      nameTerminator(MBB, cond[1]);
    }
  }
  names.clear();

  this->MF = nullptr;
  return false;
}

void CSANameLICsPass::nameLIC(MachineInstr &MI) {
  if (!MI.getOperand(0).isReg())
    return;
  MI.setFlag(MachineInstr::NonSequential);
  auto name = MI.getDebugVariable()->getName();
  unsigned reg = MI.getOperand(0).getReg();
  // CMPLRLLVM-7598: some optimizations (e.g. dead instructions elimination)
  //                 invalidate DBG_VALUE instructions by making their zero
  //                 operand $noreg.  Such DBG_VALUE instructions are supposed
  //                 to be fixed by LiveDebugVariables analysis after
  //                 the register allocation.  Some issue is causing most
  //                 DBG_VALUE instructions to have $noreg "definition"
  //                 after register allocation for CSA, so we cannot easily
  //                 move CSANameLICsPass after the register allocation.
  //                 The temporary workaround is to just check for $noreg.
  if (Register::isPhysicalRegister(reg) || reg == 0)
    return;
  LMFI->setLICName(reg, name);
}

void CSANameLICsPass::nameTerminator(const MachineBasicBlock &MBB,
    const MachineOperand &MO) {
  // If there's no basic block name, don't name it.
  if (MBB.getName().empty())
    return;
  if (!MO.isReg())
    return;
  unsigned reg = MO.getReg();
  if (Register::isPhysicalRegister(reg))
    return;
  // Don't try to change the name of something already set.
  if (!LMFI->getLICName(reg).empty())
    return;
  LMFI->setLICName(reg, "switch." + Twine(MBB.getName()) + ".cond");
}
