//===--- VPOAvrFunction.cpp - Implements AVRFunction class ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the the AVRFuction class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrFunction.h"

using namespace llvm;
using namespace intel;

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
  DEBUG(dbgs() <<"AVR_Function\n");
}

void AVRFunction::dump() const {
  print();
  for (auto Itr = child_begin(); Itr !=child_end(); ++Itr) {
    Itr->print();
  }
}
