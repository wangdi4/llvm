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

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrIf.h"

#define DEBUG_TYPE "avr-if-node"

using namespace llvm;
using namespace llvm::vpo;

AVRIf::AVRIf(unsigned SCID) : AVR(SCID) {}

AVRIf *AVRIf::clone() const { return nullptr; }

AVR *AVRIf::getFirstElseChild() {

  if (hasElseChildren()) {
    return &*(ElseChildren.begin());
  }
  return nullptr;
}

AVR *AVRIf::getLastElseChild() {

  if (hasElseChildren()) {
    return &*(std::prev(ElseChildren.end()));
  }
  return nullptr;
}

AVR *AVRIf::getFirstThenChild() {

  if (hasThenChildren()) {
    return &*(ThenChildren.begin());
  }
  return nullptr;
}

AVR *AVRIf::getLastThenChild() {

  if (hasThenChildren()) {
    return &*(std::prev(ThenChildren.end()));
  }
  return nullptr;
}

void AVRIf::print(formatted_raw_ostream &OS, unsigned Depth,
                  VerbosityLevel VLevel) const {

  std::string Indent(Depth * TabLength, ' ');
  Depth++;
  // Print Then-children
  if (hasThenChildren()) {
    for (auto Itr = then_begin(), E = then_end(); Itr != E; ++Itr) {
      Itr->print(OS, Depth, VLevel);
    }
  }

  OS << Indent << "}\n";

  // Print Else-children
  if (hasElseChildren()) {

    OS << Indent << "else\n" << Indent << "{\n";

    for (auto Itr = else_begin(), E = else_end(); Itr != E; ++Itr) {
      Itr->print(OS, Depth, VLevel);
    }
    OS << Indent << "}\n";
  }
}

StringRef AVRIf::getAvrTypeName() const { return StringRef("if"); }
