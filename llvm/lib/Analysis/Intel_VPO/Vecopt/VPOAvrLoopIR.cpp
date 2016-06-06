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
//   VPOAvrLoopIR.cpp -- Implements the Abstract Vector Representation (AVR)
//   loop node for LLVM IR.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrLoopIR.h"

#define DEBUG_TYPE "avr-loop"

using namespace llvm;
using namespace llvm::vpo;

AVRLoopIR::AVRLoopIR(Loop *Lp) : AVRLoop(AVR::AVRLoopIRNode) {

  setNestingLevel(0);     // TODO
  setNumberOfExits(0);    // TODO
  setIsDoWhileLoop(true); // TODO
  setIsInnerMost(false);  // TODO
  setVectorCandidate(true);
  setAutoVectorCandidate(false);    // TODO
  setExplicitVectorCandidate(true); // TODO
  setLoop(Lp);
  setLowerBound(nullptr);
  setUpperBound(nullptr);
  setWrnVecLoopNode(nullptr);
}

AVRLoopIR *AVRLoopIR::clone() const { return nullptr; }

StringRef AVRLoopIR::getAvrTypeName() const { return StringRef("LOOP"); }

std::string AVRLoopIR::getAvrValueName() const { return ""; }

void AVRLoopIR::codeGen() {

  for (auto Itr = child_begin(), End = child_end(); Itr != End; ++Itr) {
    Itr->codeGen();
  }
}
