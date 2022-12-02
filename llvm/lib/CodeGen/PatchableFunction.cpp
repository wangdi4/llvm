//===-- PatchableFunction.cpp - Patchable prologues for LLVM -------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
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
// This file implements edits function bodies in place to support the
// "patchable-function" attribute.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"

using namespace llvm;

namespace {
struct PatchableFunction : public MachineFunctionPass {
  static char ID; // Pass identification, replacement for typeid
  PatchableFunction() : MachineFunctionPass(ID) {
    initializePatchableFunctionPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &F) override;
   MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().set(
        MachineFunctionProperties::Property::NoVRegs);
  }
};
}

<<<<<<< HEAD
/// Returns true if instruction \p MI will not result in actual machine code
/// instructions.
static bool doesNotGeneratecode(const MachineInstr &MI) {
  // TODO: Introduce an MCInstrDesc flag for this
  switch (MI.getOpcode()) {
  default: return false;
  case TargetOpcode::IMPLICIT_DEF:
  case TargetOpcode::KILL:
  case TargetOpcode::CFI_INSTRUCTION:
  case TargetOpcode::EH_LABEL:
  case TargetOpcode::NOTIFY_LABEL: // INTEL
  case TargetOpcode::GC_LABEL:
  case TargetOpcode::DBG_VALUE:
  case TargetOpcode::DBG_LABEL:
    return true;
  }
}

=======
>>>>>>> 3f3438a596d4883ecd934fc725a9d5f620082c3b
bool PatchableFunction::runOnMachineFunction(MachineFunction &MF) {
  if (MF.getFunction().hasFnAttribute("patchable-function-entry")) {
    MachineBasicBlock &FirstMBB = *MF.begin();
    const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
    // The initial .loc covers PATCHABLE_FUNCTION_ENTER.
    BuildMI(FirstMBB, FirstMBB.begin(), DebugLoc(),
            TII->get(TargetOpcode::PATCHABLE_FUNCTION_ENTER));
    return true;
  }

  if (!MF.getFunction().hasFnAttribute("patchable-function"))
    return false;

#ifndef NDEBUG
  Attribute PatchAttr = MF.getFunction().getFnAttribute("patchable-function");
  StringRef PatchType = PatchAttr.getValueAsString();
  assert(PatchType == "prologue-short-redirect" && "Only possibility today!");
#endif

  auto &FirstMBB = *MF.begin();
  auto *TII = MF.getSubtarget().getInstrInfo();

  MachineBasicBlock::iterator FirstActualI = llvm::find_if(
      FirstMBB, [](const MachineInstr &MI) { return !MI.isMetaInstruction(); });

  if (FirstActualI == FirstMBB.end()) {
    // As of Microsoft documentation on /hotpatch feature, we must ensure that
    // "the first instruction of each function is at least two bytes, and no
    // jump within the function goes to the first instruction"

    // When the first MBB is empty, insert a patchable no-op. This ensures the
    // first instruction is patchable in two special cases:
    // - the function is empty (e.g. unreachable)
    // - the function jumps back to the first instruction, which is in a
    // successor MBB.
    BuildMI(&FirstMBB, DebugLoc(), TII->get(TargetOpcode::PATCHABLE_OP))
        .addImm(2)
        .addImm(TargetOpcode::PATCHABLE_OP);
    MF.ensureAlignment(Align(16));
    return true;
  }

  auto MIB = BuildMI(FirstMBB, FirstActualI, FirstActualI->getDebugLoc(),
                     TII->get(TargetOpcode::PATCHABLE_OP))
                 .addImm(2)
                 .addImm(FirstActualI->getOpcode());

  for (auto &MO : FirstActualI->operands())
    MIB.add(MO);

  FirstActualI->eraseFromParent();
  MF.ensureAlignment(Align(16));
  return true;
}

char PatchableFunction::ID = 0;
char &llvm::PatchableFunctionID = PatchableFunction::ID;
INITIALIZE_PASS(PatchableFunction, "patchable-function",
                "Implement the 'patchable-function' attribute", false, false)
