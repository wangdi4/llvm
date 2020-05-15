//===- CSANormalizeDebug.cpp - Connect LICs only used by DBG_VALUEs  --===//
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
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSAInstrInfo.h"
#include "CSATargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"

using namespace llvm;

#define DEBUG_TYPE "csa-normalize-debug"
#define PASS_NAME "CSA: Debug normalization."

STATISTIC(NumDbgValueMovs,
          "Number of MOVs added to connect LICs named by DBG_VALUEs");

namespace llvm {
namespace CSA { // Register classes
// Register class for ANYC, a superclass representing a channel of any width.
// This constant is declared in "CSAGenRegisterInfo.inc", but that file is
// huge and #including it would slow compilation.
extern const TargetRegisterClass ANYCRegClass;
} // namespace CSA
} // namespace llvm

namespace {
class CSANormalizeDebug : public MachineFunctionPass {
  bool runOnMachineFunction(MachineFunction &MF) override;

  const TargetRegisterInfo *TRI;
  const MachineRegisterInfo *MRI;
  const CSAInstrInfo *TII;

public:
  static char ID; // Pass identification, replacement for typeid
  CSANormalizeDebug() : MachineFunctionPass(ID) {
    initializeCSANormalizeDebugPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return PASS_NAME;
  }

};
} // namespace

// The declaration for this factory function is in file "CSA.h"
MachineFunctionPass *llvm::createCSANormalizeDebugPass() {
  return new CSANormalizeDebug();
}

char CSANormalizeDebug::ID = 0;

INITIALIZE_PASS(CSANormalizeDebug, DEBUG_TYPE, PASS_NAME, false, false)

bool CSANormalizeDebug::runOnMachineFunction(MachineFunction &MF) {
  if (!shouldRunDataflowPass(MF))
    return false;

  if (skipFunction(MF.getFunction()))
    return false;

  bool AnyChanges = false;
  MRI             = &MF.getRegInfo();
  TRI             = MF.getSubtarget().getRegisterInfo();
  TII             = static_cast<const CSAInstrInfo *>(
    MF.getSubtarget<CSASubtarget>().getInstrInfo());

  // Look for LICs with a producer and no non-debug uses.
  for (unsigned index = 0, e = MRI->getNumVirtRegs(); index != e; ++index) {
    unsigned reg = Register::index2VirtReg(index);
    if (!TII->isLICClass(MRI->getRegClass(reg)))
      continue;

    // Skip LICs with no producer.
    if (MRI->def_empty(reg))
      continue;

    // Not all debug variables end up getting the debug flag set by this point.
    // Set the flag now, before checking for a non-debug use.
    for (auto &op : MRI->use_nodbg_operands(reg)) {
      if (op.getParent()->isDebugValue())
        op.setIsDebug();
    }

    // Skip LICs which have a non-debug use.
    if (!MRI->use_nodbg_empty(reg))
      continue;

    // Also skip lics with no uses.
    if (MRI->use_empty(reg))
      continue;

    const TargetRegisterClass *RC = MRI->getRegClass(reg);
    MachineInstr &MI = *MRI->use_instr_begin(reg);
    MachineInstr *ignMov = BuildMI(*MI.getParent(), &MI, DebugLoc(),
                                   TII->get(TII->getMoveOpcode(RC)), CSA::IGN)
                             .addReg(reg);
    ignMov->setFlag(MachineInstr::NonSequential);
    NumDbgValueMovs++;
    AnyChanges = true;
  }

  return AnyChanges;
}
