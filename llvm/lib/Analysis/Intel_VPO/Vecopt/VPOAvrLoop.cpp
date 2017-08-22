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

AVRLoop::AVRLoop(unsigned SCID) : AVR(SCID) {}

AVRLoop *AVRLoop::clone() const { return nullptr; }

AVR *AVRLoop::getFirstPreheaderNode() {
  if (hasPreheader()) {
    return &*pre_begin();
  }

  return nullptr;
}

AVR *AVRLoop::getLastPreheaderNode() {
  if (hasPreheader()) {
    return &*(std::prev(pre_end()));
  }

  return nullptr;
}

AVR *AVRLoop::getFirstChild() {
  if (hasChildren()) {
    return &*child_begin();
  }

  return nullptr;
}

AVR *AVRLoop::getLastChild() {
  if (hasChildren()) {
    return &*(std::prev(child_end()));
  } else {
    return nullptr;
  }
}

AVR *AVRLoop::getFirstPostexitNode() {
  if (hasPostexit()) {
    return &*post_begin();
  }

  return nullptr;
}

AVR *AVRLoop::getLastPostexitNode() {
  if (hasPostexit()) {
    return &*(std::prev(post_end()));
  }

  return nullptr;
}

bool AVRLoop::isPreheaderChild(AVR *Node) const {

  for (auto Itr = pre_begin(), End = pre_end(); Itr != End; ++Itr)
    if (&(*Itr) == Node)
      return true;

  return false;
}

bool AVRLoop::isChild(AVR *Node) const {

  for (auto Itr = child_begin(), End = child_end(); Itr != End; ++Itr)
    if (&(*Itr) == Node)
      return true;

  return false;
}

bool AVRLoop::isPostexitChild(AVR *Node) const {

  for (auto Itr = post_begin(), End = post_end(); Itr != End; ++Itr)
    if (&(*Itr) == Node)
      return true;

  return false;
}

void AVRLoop::extractPreheader() {}

void AVRLoop::extractPostexit() {}

void AVRLoop::extractPreheaderAndPostexit() {}

AVRIf *removeZeroTripTest() { return nullptr; }

void AVRLoop::print(formatted_raw_ostream &OS, unsigned Depth,
                    VerbosityLevel VLevel) const {
#if !INTEL_PRODUCT_RELEASE

  std::string Indent((Depth * TabLength), ' ');

  // Print Loop Preheader
  for (auto Itr = pre_begin(), End = pre_end(); Itr != End; ++Itr)
    Itr->print(OS, Depth, VLevel);

  OS << Indent;

  switch (VLevel) {
  case PrintCost:
  case PrintNumber:
    OS << "(" << getNumber() << ") ";
  case PrintAvrDecomp:
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

  // Print Loop Children
  for (auto Itr = child_begin(), End = child_end(); Itr != End; ++Itr)
    Itr->print(OS, Depth, VLevel);

  // Print Loop Postexit
  for (auto Itr = post_begin(), End = post_end(); Itr != End; ++Itr)
    Itr->print(OS, Depth - 1, VLevel);

  OS << Indent << "}\n";
#endif // !INTEL_PRODUCT_RELEASE
}

void AVRLoop::shallowPrint(formatted_raw_ostream &OS) const {
#if !INTEL_PRODUCT_RELEASE

  OS << "(" << getNumber() << ") " << getAvrTypeName()
     << "( IV )"; // TODO: Add IV Info
#endif // !INTEL_PRODUCT_RELEASE
}

StringRef AVRLoop::getAvrTypeName() const { return StringRef("LOOP"); }

std::string AVRLoop::getAvrValueName() const { return ""; }

void AVRLoop::codeGen() {

  for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) {
    Itr->codeGen();
  }
}
