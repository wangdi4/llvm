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
//   VPOAvrIf.cpp -- Implements the Abstract Vector Representation (AVR)
//   if node.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrIf.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#define DEBUG_TYPE "avr-if-node"

using namespace llvm;
using namespace llvm::vpo;

AVRIf::AVRIf(Instruction *CompInst)
  : AVR(AVR::AVRIfNode), CompareInstruction(CompInst) {}

AVRIf *AVRIf::clone() const {
  return nullptr;
}

void AVRIf::print(formatted_raw_ostream &OS, unsigned Depth,
                  unsigned VerbosityLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0) { 
    OS << Indent << "AVR_IF:     ";
    CompareInstruction->print(OS);
    OS << "\n";
  }
}


void AVRIf::codeGen() {
  Instruction *inst;

  DEBUG(CompareInstruction->dump());
  inst = CompareInstruction->clone();

  if (!inst->getType()->isVoidTy())
    inst->setName(CompareInstruction->getName() + 
                  ".VPOClone");

  ReplaceInstWithInst(CompareInstruction, inst);
  DEBUG(inst->dump());
}
