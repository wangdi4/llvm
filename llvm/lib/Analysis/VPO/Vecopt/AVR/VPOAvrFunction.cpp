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

#define DEBUG_TYPE "avr-function-node"

using namespace llvm;
using namespace llvm::vpo;

AVRFunction::AVRFunction(Function *OrigF, const LoopInfo *LpInfo)
  : AVR(AVR::AVRFunctionNode), OriginalFunction(OrigF), LI(LpInfo) {}

BasicBlock *AVRFunction::getEntryBBlock() const {
  return &OriginalFunction->getEntryBlock();
}

BasicBlock *AVRFunction::getFirstBBlock() const {
  return &OriginalFunction->front();
}

BasicBlock *AVRFunction::getLastBBlock() const {
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

void AVRFunction::print(formatted_raw_ostream &OS, unsigned Depth,
                        unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0 ) {

    OS << Indent << "AVR_FUNCTION:\n";

    Depth++;
    for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) {
      Itr->print(OS, Depth, VerbosityLevel);
    }
  }
}

void AVRFunction::codeGen() {

  for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) {
    Itr->codeGen();
  }
}
