//===-- VPOAvrSwitchIR.cpp ------------------------------------------------===//
//
//   Copyright (C) 2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the Abstract Vector Representation (AVR) switch node
/// for LLVM IR.
///
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrSwitchIR.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrUtils.h"

#define DEBUG_TYPE "avr-switch-node"

using namespace llvm;
using namespace llvm::vpo;

AVRSwitchIR::AVRSwitchIR(Instruction *Inst)
  :AVRSwitch(AVR::AVRSwitchIRNode), Instruct(Inst) {

  if (SwitchInst *SI = dyn_cast<SwitchInst> (Inst)) {

    AVR *ANOP = nullptr;
    unsigned CaseCount = 1;

    // Insert dummy case slots with NOP AVRs for simplified node insertion
    // during LLVM IR walk.
    ANOP = AVRUtils::createAVRNOP();
    AVRUtils::insertFirstDefaultChild(this, ANOP);

    for (unsigned Itr = 1; Itr <= SI->getNumCases(); ++Itr,
         ++CaseCount) {
      this->addCase();
      ANOP = AVRUtils::createAVRNOP();
      AVRUtils::insertFirstChild(this, ANOP, CaseCount);
    }

  } else {
    llvm_unreachable("Switch instuction missing for avr generation");
  } 
}

AVRSwitchIR *AVRSwitchIR::clone() const {
  return nullptr;
}

void AVRSwitchIR::printConditionValueName(formatted_raw_ostream &OS) const {

  SwitchInst *SI = cast<SwitchInst>(Instruct);
  Value *Cond = SI->getCondition();

  Cond->printAsOperand(OS,false);
}

std::string AVRSwitchIR::getAvrValueName() const {
  return "";
}


