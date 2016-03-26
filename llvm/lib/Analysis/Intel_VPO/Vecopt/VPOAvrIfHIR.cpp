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
//   VPOAvrIfHIR.cpp -- Implements the Abstract Vector Representation (AVR)
//   if node for HIR.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrIfHIR.h"

#define DEBUG_TYPE "avr-if-node"

using namespace llvm;
using namespace llvm::vpo;

AVRIfHIR::AVRIfHIR(HLIf *CompInst)
  : AVRIf(AVR::AVRIfHIRNode), CompareInstruction(CompInst) {}

AVRIfHIR *AVRIfHIR::clone() const {
  return nullptr;
}

void AVRIfHIR::print(formatted_raw_ostream &OS, unsigned Depth,
                     VerbosityLevel VLevel) const {

  std::string Indent(Depth * TabLength, ' ');
  OS << Indent;

  // Print AVR If Node.
  switch (VLevel) {
    case PrintNumber:
      OS << "("  << getNumber() << ")";
    case PrintAvrType:
    case PrintDataType:
    case PrintBase:
      CompareInstruction->printHeader(OS, 0);
      OS << Indent << "{\n";
      break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  AVRIf::print(OS, Depth, VLevel);
}

std::string AVRIfHIR::getAvrValueName() const {
  return "";
}

void AVRIfHIR::codeGen() {
}
