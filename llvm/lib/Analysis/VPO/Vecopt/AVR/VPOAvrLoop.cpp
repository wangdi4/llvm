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
//   VPOAvrLoop.cpp -- Implements the Abstract Vector Representation (AVR)
//   loop node.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrLoop.h"

#define DEBUG_TYPE "avr-loop"

using namespace llvm;
using namespace llvm::vpo;

AVRLoop::AVRLoop(const Loop *Lp)
  : AVR(AVR::AVRLoopNode), WrnLoopNode(nullptr), LLVMLoop(Lp) {

  setNestingLevel(0);               // TODO
  setNumberOfExits(0);              // TODO
  setIsDoWhileLoop(true);           // TODO
  setIsInnerMost(false);            // TODO
  setVectorCandidate(true);
  setAutoVectorCandidate(false);    // TODO 
  setExplicitVectorCandidate(true); // TODO 
}

AVRLoop *AVRLoop::clone() const {
  return nullptr;
}

AVR *AVRLoop::getLastChild() {
  if (hasChildren()){
    return std::prev(child_end());
  }
  else {
    return nullptr;
  }
}

void AVRLoop::print(formatted_raw_ostream &OS, unsigned Depth,
                    unsigned VerbosityLevel) const {

  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0 ) {

    OS << Indent  <<"AVR_LOOP:\n";

    Depth++;
    for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) { 
      Itr->print(OS, Depth, VerbosityLevel);
    }
  }
}


void AVRLoop::codeGen() {

  for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) { 
    Itr->codeGen();
  }
}
