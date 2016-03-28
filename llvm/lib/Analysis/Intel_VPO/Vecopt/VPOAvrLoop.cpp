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
//   VPOAvrLoop.cpp -- Implements the Abstract Vector Representation (AVR)
//   loop node.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrLoop.h"

#define DEBUG_TYPE "avr-loop"

using namespace llvm;
using namespace llvm::vpo;

AVRLoop::AVRLoop(const Loop *Lp)
    : AVR(AVR::AVRLoopNode), WrnLoopNode(nullptr), LLVMLoop(Lp) {

  setNestingLevel(0);     // TODO
  setNumberOfExits(0);    // TODO
  setIsDoWhileLoop(true); // TODO
  setIsInnerMost(false);  // TODO
  setVectorCandidate(true);
  setAutoVectorCandidate(false);    // TODO
  setExplicitVectorCandidate(true); // TODO
}

AVRLoop *AVRLoop::clone() const { return nullptr; }

AVR *AVRLoop::getLastChild() {
  if (hasChildren()) {
    return &*(std::prev(child_end()));
  } else {
    return nullptr;
  }
}

void AVRLoop::print(formatted_raw_ostream &OS, unsigned Depth,
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
  for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) {
    Itr->print(OS, Depth, VLevel);

    // OS << Indent  <<"END_AVR_LOOP\n";
  }

  OS << Indent << "}\n";
}

StringRef AVRLoop::getAvrTypeName() const { return StringRef("LOOP"); }

std::string AVRLoop::getAvrValueName() const { return ""; }

void AVRLoop::codeGen() {

  for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) {
    Itr->codeGen();
  }
}
