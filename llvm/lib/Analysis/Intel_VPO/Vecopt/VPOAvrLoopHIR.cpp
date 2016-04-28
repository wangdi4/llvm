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
//   VPOAvrLoopHIR.cpp -- Implements the Abstract Vector Representation (AVR)
//   loop node for HIR.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrLoopHIR.h"

#define DEBUG_TYPE "avr-loop"

using namespace llvm;
using namespace llvm::vpo;

AVRLoopHIR::AVRLoopHIR(const HLLoop *Lp)
    : AVRLoop(AVR::AVRLoopHIRNode) {

  setNestingLevel(0);     // TODO
  setNumberOfExits(0);    // TODO
  setIsDoWhileLoop(true); // TODO
  setIsInnerMost(false);  // TODO
  setVectorCandidate(true);
  setAutoVectorCandidate(false);    // TODO
  setExplicitVectorCandidate(true); // TODO
  setLoop(Lp);
  setWrnVecLoopNode(nullptr);
}

AVRLoopHIR *AVRLoopHIR::clone() const { return nullptr; }

void AVRLoopHIR::print(formatted_raw_ostream &OS, unsigned Depth,
                       VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;

  switch (VLevel) {
  case PrintNumber:
    OS << "(" << getNumber() << ") ";
  case PrintAvrType:
  // Always print avr loop type name.
  case PrintDataType:
  case PrintBase:
    OS << getAvrTypeName();
    // TODO: Add IV Info
    OS << "( IV )\n";
    OS << Indent << "{\n";
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  Depth++;
  for (auto Itr = child_begin(), End = child_end(); Itr != End; ++Itr) {
    Itr->print(OS, Depth, VLevel);

    // OS << Indent  <<"END_AVR_LOOP\n";
  }

  OS << Indent << "}\n";
}

StringRef AVRLoopHIR::getAvrTypeName() const { return StringRef("LOOP"); }

std::string AVRLoopHIR::getAvrValueName() const { return ""; }

void AVRLoopHIR::codeGen() {

  for (auto Itr = child_begin(), End = child_end(); Itr != End; ++Itr) {
    Itr->codeGen();
  }
}
