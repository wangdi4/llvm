//===-- CSADataflowSimplify.cpp - CSA simplifications to dataflow ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements a few optimizations that work on the dataflow graph of
// the CSA representation.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSAInstrInfo.h"
#include "CSATargetMachine.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "csa-df-simplify"

namespace llvm {
  class CSADataflowSimplifyPass : public MachineFunctionPass {
  public:
    static char ID;
    CSADataflowSimplifyPass();

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
    const CSAInstrInfo *TII;
    std::vector<MachineInstr *> to_delete;

    /// The following mini pass implements a peephole pass that removes NOT
    /// operands from the control lines of picks and switches.
    bool eliminateNotPicks(MachineInstr *MI);

    /// The following mini pass converts loads and stores that are dependent on
    /// strides into streaming loads and stores.
    bool makeStreamMemOp(MachineInstr *MI);

    MachineInstr *getDefinition(MachineOperand &MO) const;
    void getUses(MachineOperand &MO,
        SmallVectorImpl<MachineInstr *> &uses) const;
    MachineInstr *getSingleUse(MachineOperand &MO) const;
  };
}

char CSADataflowSimplifyPass::ID = 0;

CSADataflowSimplifyPass::CSADataflowSimplifyPass() : MachineFunctionPass(ID) {
}


MachineFunctionPass *llvm::createCSADataflowSimplifyPass() {
  return new CSADataflowSimplifyPass();
}

bool CSADataflowSimplifyPass::runOnMachineFunction(MachineFunction &MF) {
  this->MF = &MF;
  TII = static_cast<const CSAInstrInfo*>(MF.getSubtarget<CSASubtarget>().getInstrInfo());

  // Run several functions one at a time on the entire graph. There is probably
  // a better way of implementing this sort of strategy (like how InstCombiner
  // does its logic), but until we have a need to go a fuller InstCombiner-like
  // route, this logic will do. Note that we can't delete instructions on the
  // fly due to how iteration works, but we do clean them up after every mini
  // pass.
  bool changed = false;
  static auto functions = {
    &CSADataflowSimplifyPass::eliminateNotPicks,
    &CSADataflowSimplifyPass::makeStreamMemOp
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

MachineInstr *CSADataflowSimplifyPass::getDefinition(MachineOperand &MO) const {
  assert(MO.isReg() && "LICs to search for can only be registers");
  for (auto &MBB : *MF) {
    for (auto &MI : MBB) {
      for (auto &def : MI.defs()) {
        if (def.isReg() && def.getReg() == MO.getReg())
          return &MI;
      }
    }
  }

  return nullptr;
}

void CSADataflowSimplifyPass::getUses(MachineOperand &MO,
    SmallVectorImpl<MachineInstr *> &uses) const {
  assert(MO.isReg() && "LICs to search for can only be registers");
  for (auto &MBB : *MF) {
    for (auto &MI : MBB) {
      for (auto &use : MI.uses()) {
        if (use.isReg() && use.getReg() == MO.getReg())
          uses.push_back(&MI);
      }
    }
  }
}

MachineInstr *CSADataflowSimplifyPass::getSingleUse(MachineOperand &MO) const {
  SmallVector<MachineInstr *, 4> uses;
  getUses(MO, uses);
  return uses.size() == 1 ? uses[0] : nullptr;
}

bool CSADataflowSimplifyPass::eliminateNotPicks(MachineInstr *MI) {
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
      // This means the selector is a not. Swap the two definitions on the
      // output, and change the selector to be the not's inverse.
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

bool CSADataflowSimplifyPass::makeStreamMemOp(MachineInstr *MI) {
  bool isLoad;
  switch (MI->getOpcode()) {
  case CSA::LD8: case CSA::LD16: case CSA::LD32: case CSA::LD64:
    isLoad = true;
    break;
  case CSA::ST8: case CSA::ST16: case CSA::ST32: case CSA::ST64:
    isLoad = false;
    break;
  default:
    return false;
  }

  // The address needs to be a stride.
  MachineInstr *address = getDefinition(MI->getOperand(isLoad ? 2 : 1));
  if (!address || address->getOpcode() != CSA::STRIDE64) {
    return false;
  }

  // The input order needs to be a repeat.
  MachineInstr *repeat_in = getDefinition(MI->getOperand(isLoad ? 4 : 3));
  if (!repeat_in || repeat_in->getOpcode() != CSA::REPEAT1) {
    return false;
  }

  // The output order needs to be a switch.
  MachineInstr *switch_out = getSingleUse(MI->getOperand(isLoad ? 1 : 0));
  if (!switch_out || !TII->isSwitch(switch_out)) {
    return false;
  }
  // Furthermore, the other output of the switch needs to be ignored.
  if (switch_out->getOperand(0).getReg() != 2) {
    return false;
  }

  // Next check: all three must be controlled by the same sequence operator.
  // The predicate must control the repeat and the stride, and the last must
  // control the switch.
  MachineInstr *ctrl_addr = getDefinition(address->getOperand(1));
  MachineInstr *ctrl_in = getDefinition(repeat_in->getOperand(1));
  MachineInstr *ctrl_out = getDefinition(switch_out->getOperand(2));
  if (ctrl_addr != ctrl_in || ctrl_addr != ctrl_out) {
    return false;
  }
  unsigned pred_reg = ctrl_addr->getOperand(1).getReg();
  unsigned last_reg = ctrl_addr->getOperand(3).getReg();
  if (address->getOperand(1).getReg() != pred_reg ||
      repeat_in->getOperand(1).getReg() != pred_reg ||
      switch_out->getOperand(2).getReg() != last_reg) {
    return false;
  }

  DEBUG(errs() << "Candidate memory op for turning into streaming access: ");
  DEBUG(MI->dump());

  // Now that the load is legal, we have to make the transform:
  // * The base address and stride comes from the stride operator.
  // * The length is computed from the sequence operator and the base address.
  // * The in memory order becomes the consumed value from the repeat.
  // * The out memory order replaces the output of the switch.
  unsigned opcode;
  switch (MI->getOpcode()) {
  case CSA::LD8: opcode = CSA::SLD8; break;
  case CSA::LD16: opcode = CSA::SLD16; break;
  case CSA::LD32: opcode = CSA::SLD32; break;
  case CSA::LD64: opcode = CSA::SLD64; break;
  case CSA::ST8: opcode = CSA::SST8; break;
  case CSA::ST16: opcode = CSA::SST16; break;
  case CSA::ST32: opcode = CSA::SST32; break;
  case CSA::ST64: opcode = CSA::SST64; break;
  default:
    assert(false && "Unknown opcode for streaming IR load");
  }

  // Compute the length of the stride.
  unsigned lengthRegister;
  if (ctrl_addr->getOpcode() == CSA::SEQOTNE64) {
    assert(ctrl_addr->getOperand(5).getImm() == 0 && "Expected count to 0");
    assert(ctrl_addr->getOperand(6).getImm() == -1 && "Expected count to 0");
    lengthRegister = ctrl_addr->getOperand(4).getReg();
  } else {
    DEBUG(errs() << "Not handling instruction\n");
    DEBUG(ctrl_addr->dump());
    return false;
  }

  auto builder = BuildMI(*MI->getParent(), MI, MI->getDebugLoc(),
      TII->get(opcode));
  if (isLoad)
    builder.addReg(MI->getOperand(0).getReg(), RegState::Define);
  builder.addReg(switch_out->getOperand(1).getReg(), RegState::Define);
  builder.add(address->getOperand(2)); // Address
  builder.addReg(lengthRegister); // Length
  builder.add(address->getOperand(3)); // Stride
  if (!isLoad)
    builder.add(MI->getOperand(4)); // Data
  builder.add(MI->getOperand(3)); // Memory level
  builder.add(repeat_in->getOperand(2)); // In memory order
  builder->setFlag(MachineInstr::NonSequential);

  // Remove the old switch and the old load. Dead elimination should remove the
  // rest.
  to_delete.push_back(MI);
  to_delete.push_back(switch_out);

  return true;
}
