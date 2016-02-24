//===-- VPOAvrSwitchHIR.cpp -----------------------------------------------===//
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
/// for HIR.
///
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrSwitchHIR.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrUtils.h"

#define DEBUG_TYPE "avr-switch-node"

using namespace llvm;
using namespace llvm::vpo;

AVRSwitchHIR::AVRSwitchHIR(HLSwitch *SI)
  :AVRSwitch(AVR::AVRSwitchIRNode), Instruct(SI) {

  // Switch case children are populated by the HIR walker.
  setCondition(SI->getConditionDDRef());
}

AVRSwitchHIR *AVRSwitchHIR::clone() const {
  return nullptr;
}

void AVRSwitchHIR::printConditionValueName(formatted_raw_ostream &OS) const {

  Condition->print(OS, false);
  // SwitchInst *SI = cast<SwitchInst>(Instruct);
  //alue *Cond = SI->getCondition();
  //Cond->printAsOperand(OS,false);
}


std::string AVRSwitchHIR::getAvrValueName() const {
  return "";
}


