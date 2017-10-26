//===-- CSADataflowCanonicalization.cpp - Canonicalize dataflow operations ===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements a series of tools to provide simplifications and a
// canonicalized form for dataflow instructions.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSAInstrInfo.h"
#include "CSAMachineFunctionInfo.h"
#include "CSATargetMachine.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

namespace llvm {
  class CSADataflowCanonicalizationPass : public MachineFunctionPass {
  public:
    static char ID;
    CSADataflowCanonicalizationPass();

    StringRef getPassName() const override {
      return "CSA Dataflow simplification pass";
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
    std::vector<MachineInstr *> to_delete;

    /// The following mini pass implements a peephole pass that removes NOT
    /// operands from the control lines of picks and switches.
    bool eliminateNotPicks(MachineInstr *MI);

    /// The following mini pass replaces side-effect-free operations entering a
    /// a SWITCH one of whose outputs is ignored with SWITCHes into those
    /// operations. This helps for memory ordering issues.
    bool invertIgnoredSwitches(MachineInstr *MI);

    MachineInstr *getDefinition(const MachineOperand &MO) const;
    void getUses(const MachineOperand &MO,
        SmallVectorImpl<MachineInstr *> &uses) const;
    MachineInstr *getSingleUse(const MachineOperand &MO) const;
  };
}

char CSADataflowCanonicalizationPass::ID = 0;

CSADataflowCanonicalizationPass::CSADataflowCanonicalizationPass() : MachineFunctionPass(ID) {
}


MachineFunctionPass *llvm::createCSADataflowCanonicalizationPass() {
  return new CSADataflowCanonicalizationPass();
}

bool CSADataflowCanonicalizationPass::runOnMachineFunction(MachineFunction &MF) {
  this->MF = &MF;
  MRI = &MF.getRegInfo();
  LMFI = MF.getInfo<CSAMachineFunctionInfo>();
  TII = static_cast<const CSAInstrInfo*>(MF.getSubtarget<CSASubtarget>().getInstrInfo());

  // Run several functions one at a time on the entire graph. There is probably
  // a better way of implementing this sort of strategy (like how InstCombiner
  // does its logic), but until we have a need to go a fuller InstCombiner-like
  // route, this logic will do. Note that we can't delete instructions on the
  // fly due to how iteration works, but we do clean them up after every mini
  // pass.
  bool changed = false;
  static auto functions = {
    &CSADataflowCanonicalizationPass::eliminateNotPicks,
    &CSADataflowCanonicalizationPass::invertIgnoredSwitches
  };
  for (auto func : functions) {
    for (auto &MBB : MF) {
      for (auto &MI : MBB) {
        changed |= (this->*func)(&MI);
      }
      for (auto MI : to_delete)
        MI->eraseFromParent();
      to_delete.clear();
    }
  }

  this->MF = nullptr;
  return changed;
}

MachineInstr *CSADataflowCanonicalizationPass::getDefinition(const MachineOperand &MO) const {
  assert(MO.isReg() && "LICs to search for can only be registers");
  return MRI->getUniqueVRegDef(MO.getReg());
}

void CSADataflowCanonicalizationPass::getUses(const MachineOperand &MO,
    SmallVectorImpl<MachineInstr *> &uses) const {
  assert(MO.isReg() && "LICs to search for can only be registers");
  for (auto &use : MRI->use_instructions(MO.getReg())) {
    uses.push_back(&use);
  }
}

MachineInstr *CSADataflowCanonicalizationPass::getSingleUse(
    const MachineOperand &MO) const {
  SmallVector<MachineInstr *, 4> uses;
  getUses(MO, uses);
  return uses.size() == 1 ? uses[0] : nullptr;
}

bool CSADataflowCanonicalizationPass::eliminateNotPicks(MachineInstr *MI) {
  unsigned select_op, low_op, high_op;
  if (TII->isSwitch(MI)) {
    select_op = 2;
    low_op = 0;
    high_op = 1;
  } else if (TII->isPick(MI)) {
    select_op = 1;
    low_op = 2;
    high_op = 3;
  } else {
    return false;
  }

  if (MachineInstr *selector = getDefinition(MI->getOperand(select_op))) {
    if (selector->getOpcode() == CSA::NOT1) {
      // This means the selector is a NOT. Swap the two definitions on the
      // output, and change the selector to be the NOT's inverse.
      int reg_tmp = MI->getOperand(low_op).getReg();
      MI->getOperand(low_op).setReg(MI->getOperand(high_op).getReg());
      MI->getOperand(high_op).setReg(reg_tmp);
      MI->getOperand(select_op).setReg(
          selector->getOperand(1).getReg());
      return true;
    }
  }
  return false;
}

bool CSADataflowCanonicalizationPass::invertIgnoredSwitches(MachineInstr *MI) {
  // The value must be a switch, and one of its outputs must be ignored.
  if (!TII->isSwitch(MI))
    return false;
  if ((!MI->getOperand(0).isReg() || MI->getOperand(0).getReg() != CSA::IGN) &&
      (!MI->getOperand(1).isReg() || MI->getOperand(1).getReg() != CSA::IGN)) {
    return false;
  } else if (!MI->getOperand(3).isReg()) {
    // Switching a constant value... pass doesn't apply.
    return false;
  }

  // In order for the transform to be legal, we need:
  // 1. Not doing the switched operation before the switch must not be
  //    observable. Memory operations, SXU operations, and sequence operations
  //    all fail this test.
  // 2. There must only be a single output to be switched.
  // 3. The output must only reach the SWITCH.
  MachineInstr *switched = getDefinition(MI->getOperand(3));
  if (!switched || switched->mayLoadOrStore() ||
      switched->hasUnmodeledSideEffects() || !TII->isPure(switched) ||
      !switched->getFlag(MachineInstr::NonSequential))
    return false;
  if (switched->uses().begin() - switched->defs().begin() > 1)
    return false;
  if (getSingleUse(switched->getOperand(0)) != MI)
    return false;

  // Generate new SWITCH's for each operand of the switched operation. The
  // operation itself is modified in-place, so we need to fix up all the LIC
  // operands of the operation.
  bool is0Dead = MI->getOperand(0).getReg() == CSA::IGN;
  for (MachineOperand &MO : switched->operands()) {
    if (!MO.isReg())
      continue;
    if (MO.isDef()) {
      // We asserted above that the switched operand has exactly one definition,
      // so we only need to replace this with the output of the switch.
      MO.setReg(MI->getOperand(is0Dead ? 1 : 0).getReg());
    } else if (MO.getReg() != CSA::IGN && MO.getReg() != CSA::NA) {
      // Non-ignored registers mean that we need to allocate a new SWITCH here.
      // Insert the new SWITCH before the operand instruction to try to keep the
      // operations in topological order.
      auto licClass = TII->lookupLICRegClass(MO.getReg());
      auto newParam = (decltype(CSA::IGN))LMFI->allocateLIC(licClass);
      auto newSwitch = BuildMI(*switched->getParent(), switched,
          MI->getDebugLoc(),
          TII->get(TII->getPickSwitchOpcode(licClass, false)))
        .addReg(is0Dead ? CSA::IGN : newParam, RegState::Define)
        .addReg(is0Dead ? newParam : CSA::IGN, RegState::Define)
        .add(MI->getOperand(2))
        .add(MO);
      newSwitch->setFlag(MachineInstr::NonSequential);
      MO.setReg(newParam);
    }
  }

  // Delete the old switch.
  to_delete.push_back(MI);
  return false;
}
