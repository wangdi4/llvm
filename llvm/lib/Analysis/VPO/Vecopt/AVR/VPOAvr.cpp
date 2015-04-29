//===--- VPOAvr.cpp - Implements AVR Base class ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the the AVR Base class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/ErrorHandling.h"
#include "llvm/IR/CFG.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrFunction.h"

using namespace llvm;
using namespace intel;

AVR::AVR(unsigned SCID)
  : SubClassID(SCID), Parent(nullptr), Number(0) {}

void AVR::print() const {
  DEBUG(dbgs() <<"Base AVR Node\n");
}
/*
void AVR::dump() const {
  print();
}
*/
