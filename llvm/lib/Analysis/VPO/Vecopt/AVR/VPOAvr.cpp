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
//   VPOAvr.cpp -- Implements the abstract vector representation base node.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/ErrorHandling.h"
#include "llvm/IR/CFG.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrFunction.h"

#define DEBUG_TYPE "avr-node"

using namespace llvm;
using namespace llvm::vpo;

AVR::AVR(unsigned SCID)
  : SubClassID(SCID), Parent(nullptr), Number(0) {}

void AVR::destroy() {
  delete this;
}

/// Should this be made pure virtual?
void AVR::codeGen()  {
}

void AVR::dump() const {
  this->dump(1);
}

void AVR::dump(unsigned VerbosityLevel) const {
  formatted_raw_ostream OS(dbgs());
  print(OS, 1, VerbosityLevel);
}
