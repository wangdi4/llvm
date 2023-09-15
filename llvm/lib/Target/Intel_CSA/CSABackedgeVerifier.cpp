//===-- CSABackedgeVerifier.cpp - Verify CSA Backedge Attribute -*- C++ -*-===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a pass to verify a LIC's csasim_backedge attribute.
//
// When csa-verify-backedges is set to true, this pass iterate through
// all the LICs, verifying the ones with the csasim_backedge attribute
// do complete a cycle. For a backedge LIC that does not complete a cycle,
// its csasim_backedge attribute is removed.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSATargetMachine.h"

#include "llvm/CodeGen/MachineFunctionPass.h"

using namespace llvm;

#define DEBUG_TYPE "csa-backedge-verifier"
#define PASS_NAME "CSA: Verify Backedge Attribute"

static bool findPathBetweenWkr(MachineInstr* A, MachineInstr* B, const MachineRegisterInfo* MRI, std::vector<MachineInstr*> &Visited) {
  for (auto &C: A->defs()) {
    for (auto &D: MRI->use_instructions(C.getReg())) {
      if (std::find(Visited.begin(), Visited.end(), &D) != Visited.end()) continue;
      if (&D == B) return true;
      Visited.push_back(&D);
      if (findPathBetweenWkr(&D, B, MRI, Visited)) return true;
    }
  }

  return false;
}

static bool findPathBetween(MachineInstr* A, MachineInstr* B, const MachineRegisterInfo* MRI) {
  std::vector<MachineInstr*> Visited;
  return findPathBetweenWkr(A, B, MRI, Visited);
}

namespace {

class CSABackedgeVerifier : public MachineFunctionPass {
  MachineRegisterInfo *MRI = nullptr;

public:
  static char ID;
  CSABackedgeVerifier() : MachineFunctionPass(ID) {}
  StringRef getPassName() const override { return PASS_NAME; }

  bool runOnMachineFunction(MachineFunction &) override;
};

} // namespace

char CSABackedgeVerifier::ID = 0;

INITIALIZE_PASS(CSABackedgeVerifier, DEBUG_TYPE, PASS_NAME, false, false)

MachineFunctionPass *llvm::createCSABackedgeVerifier() {
  return new CSABackedgeVerifier();
}

bool CSABackedgeVerifier::runOnMachineFunction(MachineFunction &MF) {
  const auto LMFI = MF.getInfo<CSAMachineFunctionInfo>();
  MRI             = &MF.getRegInfo();

  // Iterate through all LICs.
  bool UpdatedAttribute = false;
  for (unsigned VRegI = 0, VRegIE = MRI->getNumVirtRegs(); VRegI != VRegIE;
       ++VRegI) {
    unsigned VReg = Register::index2VirtReg(VRegI);
    if (MRI->reg_empty(VReg) or not LMFI->getIsDeclared(VReg))
      continue;

    for (StringRef K : LMFI->getLICAttributes(VReg)) {
      if (K.equals(StringRef("csasim_backedge"))) {
        bool FoundACycle = false;
        for (auto &DI : MRI->def_instructions(VReg)) {
          for (auto &UI : MRI->use_instructions(VReg)) {
            if (findPathBetween(&UI, &DI, MRI)) {
              FoundACycle = true;
              break;
            }
          }
          if (FoundACycle) break;
        }
        if (!FoundACycle) {
          // It's safe to remove the attribute here because we unconditionally
          // break out of the loop that iterates through the attributes.
          LMFI->removeLICAttribute(VReg, "csasim_backedge");
          UpdatedAttribute = true;
        }
        break;
      }
    }
  }

  return UpdatedAttribute;
}
