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
//   VPOAvrStmt.cpp -- Implements the Abstract Vector Representation (AVR)
//   statement nodes.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/ErrorHandling.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrStmt.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "avr-stmt-node"

// TODO: Properly define print routines.


//----------AVR Assign Implementation----------//
AVRAssign::AVRAssign(unsigned SCID)
  : AVR(SCID) {}

AVRAssign *AVRAssign::clone() const {
  return nullptr;
}

void AVRAssign::print(formatted_raw_ostream &OS, unsigned Depth,
                      unsigned VerbosityLevel) const {
}

//----------AVR Label Implementation----------//
AVRLabel::AVRLabel(unsigned SCID)
  : AVR(SCID) {}

AVRLabel *AVRLabel::clone() const {
  return nullptr;
}

void AVRLabel::print(formatted_raw_ostream &OS, unsigned Depth,
                     unsigned VerbosityLevel) const {
}

//----------AVR Phi Implementation----------//
AVRPhi::AVRPhi(unsigned SCID)
  : AVR(SCID) {}

AVRPhi *AVRPhi::clone() const {
  return nullptr;
}

void AVRPhi::print(formatted_raw_ostream &OS, unsigned Depth,
                     unsigned VerbosityLevel) const {
}

//----------AVR Call Implementation----------//
AVRCall::AVRCall(unsigned SCID)
  : AVR(SCID) {}

AVRCall *AVRCall::clone() const {
  return nullptr;
}

void AVRCall::print(formatted_raw_ostream &OS, unsigned Depth,
                    unsigned VerbosityLevel) const {
}

//----------AVR FBranch Implementation----------//
AVRFBranch::AVRFBranch(unsigned SCID)
  : AVR(SCID) {}

AVRFBranch *AVRFBranch::clone() const {
  return nullptr;
}

void AVRFBranch::print(formatted_raw_ostream &OS, unsigned Depth,
                       unsigned VerbosityLevel) const {
}

//----------AVR BackEdge Implementation----------//
AVRBackEdge::AVRBackEdge(unsigned SCID)
  : AVR(SCID) {}

AVRBackEdge *AVRBackEdge::clone() const {
  return nullptr;
}

void AVRBackEdge::print(formatted_raw_ostream &OS, unsigned Depth,
                        unsigned VerbosityLevel) const {
}

//----------AVR Entry Implementation----------//
AVREntry::AVREntry(unsigned SCID)
  : AVR(SCID) {}

AVREntry *AVREntry::clone() const {
  return nullptr;
}

void AVREntry::print(formatted_raw_ostream &OS, unsigned Depth,
                     unsigned VerbosityLevel) const {
}

//----------AVR Return Implementation----------//
AVRReturn::AVRReturn(unsigned SCID)
  : AVR(SCID) {}

AVRReturn *AVRReturn::clone() const {
  return nullptr;
}

void AVRReturn::print(formatted_raw_ostream &OS, unsigned Depth,
                      unsigned VerbosityLevel) const {
}

//----------------------------------------------------------------------------//
// AVR Wrn Node
//----------------------------------------------------------------------------//
AVRWrn::AVRWrn(WRNVecLoopNode *WrnSimdNode)
  : AVR(AVR::AVRWrnNode), WRegionSimdNode(WrnSimdNode) {}

AVRWrn *AVRWrn::clone() const {
  return nullptr;
}


void AVRWrn::print(formatted_raw_ostream &OS, unsigned Depth,
                   unsigned VerbosityLevel) const {

  std::string Indent(Depth * TabLength, ' ');

  if (VerbosityLevel > 0) {

    OS << Indent <<"AVR_WRN\n";

    Depth++;
    for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) { 
      Itr->print(OS, Depth, VerbosityLevel);
    }
  }
}

void AVRWrn::codeGen() {
  for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) { 
    Itr->codeGen();
  }
}

