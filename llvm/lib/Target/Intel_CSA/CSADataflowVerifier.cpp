//===-- CSADataflowVerifier.cpp - Verify CSA Dataflow Code ------*- C++ -*-===//
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
// This file implements a small pass to verify CSA dataflow machine IR.
//
// See docs/Intel/CSA/DataflowMIR.rst for more in-depth information on the
// conditions checked by this pass.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSATargetMachine.h"

#include "llvm/CodeGen/MachineFunctionPass.h"

using namespace llvm;

#define DEBUG_TYPE "csa-df-verifier"
#define PASS_NAME "CSA: Verify Dataflow Machine IR"

static cl::opt<bool> CheckDefWidths{
  "csa-check-def-widths", cl::Hidden,
  cl::desc("CSA Specific: Check that lics have the same widths as their defs"),
  cl::init(false)};

namespace {

class CSADataflowVerifier : public MachineFunctionPass {
  MachineRegisterInfo *MRI = nullptr;

public:
  static char ID;
  CSADataflowVerifier() : MachineFunctionPass(ID) {}
  StringRef getPassName() const override { return PASS_NAME; }

  bool runOnMachineFunction(MachineFunction &) override;

private:
  // Prints the defs and uses of a register when errors are detected to aid with
  // debugging.
  void dumpDefsAndUses(unsigned VReg) const;
};

} // namespace

char CSADataflowVerifier::ID = 0;

INITIALIZE_PASS(CSADataflowVerifier, DEBUG_TYPE, PASS_NAME, false, false)

MachineFunctionPass *llvm::createCSADataflowVerifier() {
  return new CSADataflowVerifier();
}

bool CSADataflowVerifier::runOnMachineFunction(MachineFunction &MF) {
  const auto LMFI = MF.getInfo<CSAMachineFunctionInfo>();
  MRI             = &MF.getRegInfo();
  const auto TII =
    static_cast<const CSAInstrInfo *>(MF.getSubtarget().getInstrInfo());
  const TargetRegisterInfo *const TRI = MF.getSubtarget().getRegisterInfo();

  // Iterate all lics that are not dead.
  bool UpdatedDepths = false;
  for (unsigned VRegI = 0, VRegIE = MRI->getNumVirtRegs(); VRegI != VRegIE;
       ++VRegI) {
    unsigned VReg = Register::index2VirtReg(VRegI);
    if (MRI->reg_empty(VReg) or not LMFI->getIsDeclared(VReg))
      continue;

    // Count "real" defs and init defs.
    int NumInitDefs = 0, NumNonInitDefs = 0;
    for (MachineOperand &Def : MRI->def_operands(VReg)) {
      const MachineInstr *const DefInst = Def.getParent();
      const MCInstrDesc &Desc           = DefInst->getDesc();
      if (TII->getGenericOpcode(Desc.getOpcode()) == CSA::Generic::INIT)
        ++NumInitDefs;
      else
        ++NumNonInitDefs;

      // If requested, also check that the widths match.
      if (CheckDefWidths) {
        const TargetRegisterClass *const Class = TII->getRegClass(
          Desc, DefInst->findRegisterDefOperandIdx(VReg), TRI, MF);
        if (LMFI->getLICSize(VReg) != TII->getSizeOfRegisterClass(Class)) {
          errs() << "LIC that doesn't match def width found: "
                 << printReg(VReg, TRI, 0, MRI) << "\n";
          dumpDefsAndUses(VReg);
          errs() << "\n";
        }
      }
    }

    // If there are no non-init defs, that's a problem.
    if (NumNonInitDefs == 0) {
      errs() << "LIC with no defs found: " << printReg(VReg, TRI, 0, MRI)
             << "\n";
      dumpDefsAndUses(VReg);
      llvm_unreachable("Dataflow machine code verification failed!");
    }

    // If there are multiple non-init defs, that's also a problem.
    if (NumNonInitDefs > 1) {
      errs() << "LIC with multiple defs found: " << printReg(VReg, TRI, 0, MRI)
             << "\n";
      dumpDefsAndUses(VReg);
      llvm_unreachable("Dataflow machine code verification failed!");
    }

    // There should also be uses.
    if (MRI->use_empty(VReg)) {
      errs() << "LIC with no uses found: " << printReg(VReg, TRI, 0, MRI)
             << "\n";
      dumpDefsAndUses(VReg);
      llvm_unreachable("Dataflow machine code verification failed!");
    }

    // The lic should be deep enough to hold all of its initial values. This can
    // just be fixed in this pass.
    if (LMFI->getLICDepth(VReg) < NumInitDefs) {
      LLVM_DEBUG(dbgs() << "Updating depth of " << printReg(VReg, TRI, 0, MRI)
                        << " to " << NumInitDefs << "\n");
      LMFI->setLICDepth(VReg, NumInitDefs);
      UpdatedDepths = true;
    }
  }

  return UpdatedDepths;
}

void CSADataflowVerifier::dumpDefsAndUses(unsigned VReg) const {
  errs() << "Defs:\n";
  for (MachineInstr &Def : MRI->def_instructions(VReg)) {
    errs() << "  " << Def;
  }
  errs() << "Uses:\n";
  for (MachineInstr &Use : MRI->use_instructions(VReg)) {
    errs() << "  " << Use;
  }
}
