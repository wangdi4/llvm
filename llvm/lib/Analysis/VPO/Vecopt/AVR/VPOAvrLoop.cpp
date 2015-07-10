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

using namespace llvm;
using namespace llvm::vpo;

AVRLoop::AVRLoop(const LoopInfo *LLVMLoop, bool IsDoWh)
  : AVR(AVR::AVRLoopNode), OrigLoop(LLVMLoop), NestingLevel(0),
    IsDoWhile(IsDoWh), IsInnerMost(true), IsVectorCandidate(true),
    IsAutoVectorCandidate(false), IsExplicitVectorCandidate(true) {}


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

void AVRLoop::print() const {
  DEBUG(dbgs() <<"AVR_Loop\n");
}

void AVRLoop::dump() const {
  print();
  for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) { 
    Itr->print();
  }
}

void AVRLoop::codeGen() {

  for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) { 
    Itr->codeGen();
  }
}
