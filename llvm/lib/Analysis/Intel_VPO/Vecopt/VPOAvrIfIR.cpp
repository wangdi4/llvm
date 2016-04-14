//===----------------------------------------------------------------------===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOAvrIfIR.cpp -- Implements the Abstract Vector Representation (AVR)
//   if node.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrIfIR.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrUtils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#define DEBUG_TYPE "avr-if-node"

using namespace llvm;
using namespace llvm::vpo;

AVRIfIR::AVRIfIR(AVRBranch *ABranch)
    : AVRIf(AVR::AVRIfIRNode), AvrBranch(ABranch) {

  assert(ABranch->isConditional() && "Branch for AvrIf is non-conditional!");
  Condition = AvrBranch->getCondition();

  // Set NOP Then-Else first children for downstream simplified node insertion. 
  AVR *ANOP = AVRUtils::createAVRNOP();
  AVRUtils::insertFirstThenChild(this, AvrItr(ANOP));

  ANOP = AVRUtils::createAVRNOP();
  AVRUtils::insertFirstElseChild(this, AvrItr(ANOP));
}

AVRIfIR *AVRIfIR::clone() const { return nullptr; }

void AVRIfIR::print(formatted_raw_ostream &OS, unsigned Depth,
                    VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;

  switch (VLevel) {
  case PrintNumber:
    OS << "(" << getNumber() << ") ";
  case PrintAvrType:
  case PrintDataType:
  case PrintBase:
    OS << getAvrTypeName();
    OS << "( ";
    OS << Condition->getAvrValueName();
    OS << " )\n";
    OS << Indent << "{\n";
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  AVRIf::print(OS, Depth, VLevel);
}

std::string AVRIfIR::getAvrValueName() const { return ""; }

void AVRIfIR::codeGen() {}
