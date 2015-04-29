//===--------------- VPOAvrIf.cpp - Implements AVRIf class ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the the AVRIf class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrIf.h"

using namespace llvm;
using namespace intel;

AVRIf::AVRIf(Instruction *CompInst)
  : AVR(AVR::AVRIfNode), CompareInstruction(CompInst) {}

AVRIf *AVRIf::clone() const {
  return nullptr;
}

void AVRIf::print() const {
  DEBUG(dbgs() <<"AVR_If\n");
}

void AVRIf::dump() const {
  print();
}
