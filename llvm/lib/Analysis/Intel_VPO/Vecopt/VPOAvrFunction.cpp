//===----------------------------------------------------------------------===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the Abstract Vector Representation (AVR) function node.
///
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrFunction.h"

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

AVRFunction *AVRFunction::clone() const { return nullptr; }

AVR *AVRFunction::getLastChild() {
  if (hasChildren())
    return &*(std::prev(child_end()));
  else
    return nullptr;
}

void AVRFunction::print(formatted_raw_ostream &OS, unsigned Depth,
                        VerbosityLevel VLevel) const {
#if !INTEL_PRODUCT_RELEASE

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;

  switch (VLevel) {
  case PrintCost:
  case PrintNumber:
    OS << "(" << getNumber() << ") ";
  case PrintAvrDecomp:
  case PrintAvrType:
    OS << getAvrTypeName();
  case PrintDataType:
  case PrintBase: {
    OS << getAvrValueName() << "(";

    // Print Function Arguments
    if (!OriginalFunction->arg_empty()) {

      Function::arg_iterator ArgListIt = OriginalFunction->arg_begin();
      Function::arg_iterator ArgListEnd = OriginalFunction->arg_end();

      OS << *ArgListIt;
      ++ArgListIt;
      for (; ArgListIt != ArgListEnd; ++ArgListIt) {
        OS << ", " << *ArgListIt;
      }
    }

    OS << ")\n" << Indent << "{\n";
    break;
  }
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  Depth++;

  for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) {
    Itr->print(OS, Depth, VLevel);
  }

  OS << Indent << "}\n";
#endif // !INTEL_PRODUCT_RELEASE
}

StringRef AVRFunction::getAvrTypeName() const { return StringRef("FUNCTION "); }

std::string AVRFunction::getAvrValueName() const {
  return OriginalFunction->getName();
}

void AVRFunction::codeGen() {

  for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) {
    Itr->codeGen();
  }
}
