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

bool AVRIf::isThenChild(AVR *Node) const {

  for (auto Itr = then_begin(), End = then_end(); Itr != End; ++Itr)
    if (&(*Itr) == Node)
      return true;

  return false;
}

bool AVRIf::isElseChild(AVR *Node) const {

  for (auto Itr = else_begin(), End = else_end(); Itr != End; ++Itr)
    if (&(*Itr) == Node)
      return true;

  return false;
}

void AVRIf::print(formatted_raw_ostream &OS, unsigned Depth,
                  VerbosityLevel VLevel) const {
#if !INTEL_PRODUCT_RELEASE

  std::string Indent(Depth * TabLength, ' ');
  OS << Indent;

  // Print If header
  switch (VLevel) {
  case PrintCost:
    OS << "$(" << getCondition()->getCost() << ") ";
  case PrintNumber:
    OS << "(" << getNumber() << ") ";
  case PrintAvrDecomp:
  case PrintAvrType:
  case PrintDataType:
    printSLEV(OS);
  case PrintBase:
    OS << getAvrTypeName();
    if (getPredicate())
      OS << " /P" << getPredicate()->getNumber() << "/ ";
    OS << " (";
    getCondition()->print(OS, 0, VLevel);
    OS << ") ";
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  OS << Indent << "{\n";

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
#endif // !INTEL_PRODUCT_RELEASE
}

void AVRIf::shallowPrint(formatted_raw_ostream &OS) const {
#if !INTEL_PRODUCT_RELEASE

  OS << "(" << getNumber() << ") ";
  printSLEV(OS);
  OS << "IF (" << getCondition()->getNumber() << ")";
#endif // !INTEL_PRODUCT_RELEASE
}

StringRef AVRIf::getAvrTypeName() const { return StringRef("if"); }
