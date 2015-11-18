//===----------------------------------------------------------------------===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrIfHIR.h"

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

  if (VLevel > PrintBase) { 
    OS << Indent << "AVR_IF: ";
    CompareInstruction->printHeader(OS, 0, false);
    OS << "\n";
  }

  AVRIf::print(OS, Depth, VLevel);
}

StringRef AVRIfHIR::getAvrValueName() const {
  return StringRef("",0);
}

void AVRIfHIR::codeGen() {
}
