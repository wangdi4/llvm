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

AVRLoopHIR::AVRLoopHIR(HLLoop *Lp) : AVRLoop(AVR::AVRLoopHIRNode) {

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

StringRef AVRLoopHIR::getAvrTypeName() const { return StringRef("LOOP"); }

std::string AVRLoopHIR::getAvrValueName() const { return ""; }

void AVRLoopHIR::codeGen() {

  for (auto Itr = child_begin(), End = child_end(); Itr != End; ++Itr) {
    Itr->codeGen();
  }
}
