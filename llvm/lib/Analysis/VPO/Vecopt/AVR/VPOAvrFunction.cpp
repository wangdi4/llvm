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
//   VPOAvrFunction.cpp -- Implements the Abstract Vector Representation (AVR)
//   function node.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrFunction.h"

using namespace llvm;
using namespace llvm::vpo;

AVRFunction::AVRFunction(Function *OrigF)
  : AVR(AVR::AVRFunctionNode), OriginalFunction(OrigF) {}

BasicBlock *AVRFunction::getEntryBasicBlock() const {
  return &OriginalFunction->getEntryBlock();
}

BasicBlock *AVRFunction::getFirstBasicBlock() const {
  return &OriginalFunction->front();
}

BasicBlock *AVRFunction::getLastBasicBlock() const {
  return &OriginalFunction->back();
}

AVRFunction *AVRFunction::clone() const {
  return nullptr;
}

AVR *AVRFunction::getLastChild() {
  if (hasChildren()){
    return std::prev(child_end());
  }
  else {
    return nullptr;
  }
}

void AVRFunction::print() const {
  DEBUG(dbgs() <<"AVR_FUNCTION\n");
}

void AVRFunction::dump() const {
  print();
  for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) {
    Itr->print();
  }
}

void AVRFunction::codeGen() {

  for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) {
    Itr->codeGen();
  }
}
