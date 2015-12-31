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
                      VerbosityLevel VLevel) const {}

StringRef AVRAssign::getAvrTypeName() const {
  return StringRef("ASSIGN");
}

//----------AVR Label Implementation----------//
AVRLabel::AVRLabel(unsigned SCID)
  : AVR(SCID) {}

AVRLabel *AVRLabel::clone() const {
  return nullptr;
}

void AVRLabel::print(formatted_raw_ostream &OS, unsigned Depth,
                     VerbosityLevel VLevel) const {}

StringRef AVRLabel::getAvrTypeName() const {
  return StringRef("LABEL");
}

//----------AVR Phi Implementation----------//
AVRPhi::AVRPhi(unsigned SCID)
  : AVR(SCID) {}

AVRPhi *AVRPhi::clone() const {
  return nullptr;
}

void AVRPhi::print(formatted_raw_ostream &OS, unsigned Depth,
                   VerbosityLevel VLevel) const {}

StringRef AVRPhi::getAvrTypeName() const {
  return StringRef("PHI");
}

//----------AVR Call Implementation----------//
AVRCall::AVRCall(unsigned SCID)
  : AVR(SCID) {}

AVRCall *AVRCall::clone() const {
  return nullptr;
}

void AVRCall::print(formatted_raw_ostream &OS, unsigned Depth,
                    VerbosityLevel VLevel) const {}

StringRef AVRCall::getAvrTypeName() const {
  return StringRef("CALL");
}

//----------------------------------------------------------------------------//
// AVR Branch Node
//----------------------------------------------------------------------------//
AVRBranch::AVRBranch(AVRLabel *ALabel)
  : AVR(AVR::AVRBranchNode), IsConditional(false), IsIndirect(false),
    IsBottomTest(false) {
  addSuccessor(ALabel);
}

AVRBranch::AVRBranch(unsigned SCID, bool IsInd, AVR *Cond)
  : AVR(SCID), IsIndirect(IsInd), IsBottomTest(false), Condition(Cond) {}

AVRBranch::AVRBranch(unsigned SCID)
  : AVR(SCID), IsBottomTest(false) {}

AVRBranch *AVRBranch::clone() const {
  return nullptr;
}

void AVRBranch::print(formatted_raw_ostream &OS, unsigned Depth,
                      VerbosityLevel VLevel) const {}

StringRef AVRBranch::getAvrTypeName() const {
  return StringRef("BRANCH");
}


std::string AVRBranch::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);

  assert(!IsConditional && getNumSuccessors() > 0 &&
	 "Invalid Avr Branch");

  return Successors[0]->getAvrValueName();
}


//----------AVR BackEdge Implementation----------//
AVRBackEdge::AVRBackEdge(unsigned SCID)
  : AVR(SCID) {}

AVRBackEdge *AVRBackEdge::clone() const {
  return nullptr;
}

void AVRBackEdge::print(formatted_raw_ostream &OS, unsigned Depth,
                        VerbosityLevel VLevel) const {}

StringRef AVRBackEdge::getAvrTypeName() const {
  return StringRef("BACKEDGE");
}

//----------AVR Entry Implementation----------//
AVREntry::AVREntry(unsigned SCID)
  : AVR(SCID) {}

AVREntry *AVREntry::clone() const {
  return nullptr;
}

void AVREntry::print(formatted_raw_ostream &OS, unsigned Depth,
                     VerbosityLevel VLevel) const {}

StringRef AVREntry::getAvrTypeName() const {
  return StringRef("ENTRY");
}

//----------AVR Return Implementation----------//
AVRReturn::AVRReturn(unsigned SCID)
  : AVR(SCID) {}

AVRReturn *AVRReturn::clone() const {
  return nullptr;
}

void AVRReturn::print(formatted_raw_ostream &OS, unsigned Depth,
                      VerbosityLevel VLevel) const {}

StringRef AVRReturn::getAvrTypeName() const {
  return StringRef("RETURN");
}

//----------------------------------------------------------------------------//
// AVR Select Node
//----------------------------------------------------------------------------//
AVRSelect::AVRSelect(unsigned SCID, AVR *AComp)
  : AVR(SCID), Compare(AComp) {}

AVRSelect *AVRSelect::clone() const {
  return nullptr;
}

void AVRSelect::print(formatted_raw_ostream &OS, unsigned Depth,
                      VerbosityLevel VLevel) const {}

StringRef AVRSelect::getAvrTypeName() const {
  return StringRef("SELECT");
}


//----------------------------------------------------------------------------//
// AVR Compare Node
//----------------------------------------------------------------------------//
AVRCompare::AVRCompare(unsigned SCID)
  : AVR(SCID) {}

AVRCompare *AVRCompare::clone() const {
  return nullptr;
}

void AVRCompare::print(formatted_raw_ostream &OS, unsigned Depth,
                       VerbosityLevel VLevel) const {}

StringRef AVRCompare::getAvrTypeName() const {
  return StringRef("COMPARE");
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
                   VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;

  switch (VLevel) {
    case PrintNumber:
      OS << "(" << getNumber() << ") ";
    case PrintType:
      OS << getAvrTypeName();
    case PrintBase:
      OS << "{" << getAvrValueName() << "}\n";
      break;
    default:
      llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  Depth++;
  for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) { 
    Itr->print(OS, Depth, VLevel);
  }
}

StringRef AVRWrn::getAvrTypeName() const {
  return StringRef("WRN");
}

std::string AVRWrn::getAvrValueName() const {
  return "";
}

void AVRWrn::codeGen() {
  for (auto Itr = child_begin(), E = child_end(); Itr != E; ++Itr) { 
    Itr->codeGen();
  }
}

