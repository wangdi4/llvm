//===-- VPOAvrStmt.cpp ----------------------------------------------------===//
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
/// This file implements the Abstract Vector Representation (AVR) statement
/// nodes.
///
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrStmt.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "avr-stmt-node"

//----------AVR Assign Implementation----------//
AVRAssign::AVRAssign(unsigned SCID) : AVR(SCID), LHS(nullptr), RHS(nullptr) {}

AVRAssign *AVRAssign::clone() const { return nullptr; }

void AVRAssign::print(formatted_raw_ostream &OS, unsigned Depth,
                      VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');
  OS << Indent;

  // Print Avr Assign Node.
  switch (VLevel) {
  case PrintNumber:
    OS << "(" << getNumber() << ") ";
  case PrintAvrType:
    OS << getAvrTypeName() << "{";
  case PrintDataType:
  case PrintBase:
    if (hasLHS() && hasRHS()) {

      // Print avr assign node which contains avr expressions.
      this->getLHS()->print(OS, 0, VLevel);
      OS << " = ";
      this->getRHS()->print(OS, 0, VLevel);

    } else {

      // Print non-expression containing avr assign node.
      OS << getAvrValueName();
    }
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  // Close up open braces
  if (VLevel >= PrintAvrType)
    OS << "}";

  OS << "\n";
}

StringRef AVRAssign::getAvrTypeName() const { return StringRef("ASSIGN"); }

//----------AVR Expression Implementation----------//
AVRExpression::AVRExpression(unsigned SCID) : AVR(SCID) {}

AVRExpression *AVRExpression::clone() const { return nullptr; }

void AVRExpression::print(formatted_raw_ostream &OS, unsigned Depth,
                          VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');

  // Print AVR Expression Node.
  switch (VLevel) {
  case PrintNumber:
    OS << "(" << getNumber() << ")";
  case PrintAvrType:
    OS << getAvrTypeName() << "{";
  case PrintDataType:
  case PrintBase:
    if (isUnaryOperation()) {

      if (!isLHSExpr())
        OS << getOpCodeName() << " ";

      this->Operands.back()->print(OS, Depth, VLevel);

    } else if (isBinaryOperation()) {

      this->Operands[0]->print(OS, Depth, VLevel);
      OS << " " << getOpCodeName() << " ";
      this->Operands[1]->print(OS, Depth, VLevel);

    } else {

      OS << getOpCodeName() << " ";
      for (auto Itr : Operands) {
        Itr->print(OS, Depth, VLevel);
        OS << " ";
      }
    }
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  // Close up open braces
  if (VLevel >= PrintAvrType)
    OS << "}";
}

StringRef AVRExpression::getAvrTypeName() const { return StringRef("EXPR"); }

//----------AVR Value Implementation----------//
AVRValue::AVRValue(unsigned SCID) : AVR(SCID) {}

AVRValue *AVRValue::clone() const { return nullptr; }

StringRef AVRValue::getAvrTypeName() const { return StringRef("VALUE"); }

//----------AVR Label Implementation----------//
AVRLabel::AVRLabel(unsigned SCID) : AVR(SCID) {}

AVRLabel *AVRLabel::clone() const { return nullptr; }

void AVRLabel::print(formatted_raw_ostream &OS, unsigned Depth,
                     VerbosityLevel VLevel) const {}

StringRef AVRLabel::getAvrTypeName() const { return StringRef("LABEL"); }

//----------AVR Phi Implementation----------//
AVRPhi::AVRPhi(unsigned SCID) : AVR(SCID) {}

AVRPhi *AVRPhi::clone() const { return nullptr; }

void AVRPhi::print(formatted_raw_ostream &OS, unsigned Depth,
                   VerbosityLevel VLevel) const {}

StringRef AVRPhi::getAvrTypeName() const { return StringRef("PHI"); }

//----------AVR Call Implementation----------//
AVRCall::AVRCall(unsigned SCID) : AVR(SCID) {}

AVRCall *AVRCall::clone() const { return nullptr; }

void AVRCall::print(formatted_raw_ostream &OS, unsigned Depth,
                    VerbosityLevel VLevel) const {}

StringRef AVRCall::getAvrTypeName() const { return StringRef("CALL"); }

//----------AVR Branch Implementation---------//
AVRBranch::AVRBranch(AVRLabel *ALabel)
    : AVR(AVR::AVRBranchNode), IsConditional(false), IsIndirect(false),
      IsBottomTest(false) {
  addSuccessor(ALabel);
}

AVRBranch::AVRBranch(unsigned SCID, bool IsInd, AVR *Cond)
    : AVR(SCID), IsIndirect(IsInd), IsBottomTest(false), Condition(Cond) {}

AVRBranch::AVRBranch(unsigned SCID) : AVR(SCID), IsBottomTest(false) {}

AVRBranch *AVRBranch::clone() const { return nullptr; }

void AVRBranch::print(formatted_raw_ostream &OS, unsigned Depth,
                      VerbosityLevel VLevel) const {}

StringRef AVRBranch::getAvrTypeName() const { return StringRef("BRANCH"); }

std::string AVRBranch::getAvrValueName() const {
  std::string IString;
  llvm::raw_string_ostream RSO(IString);

  assert(!IsConditional && getNumSuccessors() > 0 && "Invalid Avr Branch");

  return Successors[0]->getAvrValueName();
}

//----------AVR BackEdge Implementation----------//
AVRBackEdge::AVRBackEdge(unsigned SCID) : AVR(SCID) {}

AVRBackEdge *AVRBackEdge::clone() const { return nullptr; }

void AVRBackEdge::print(formatted_raw_ostream &OS, unsigned Depth,
                        VerbosityLevel VLevel) const {}

StringRef AVRBackEdge::getAvrTypeName() const { return StringRef("BACKEDGE"); }

//----------AVR Entry Implementation----------//
AVREntry::AVREntry(unsigned SCID) : AVR(SCID) {}

AVREntry *AVREntry::clone() const { return nullptr; }

void AVREntry::print(formatted_raw_ostream &OS, unsigned Depth,
                     VerbosityLevel VLevel) const {}

StringRef AVREntry::getAvrTypeName() const { return StringRef("ENTRY"); }

//----------AVR Return Implementation----------//
AVRReturn::AVRReturn(unsigned SCID) : AVR(SCID) {}

AVRReturn *AVRReturn::clone() const { return nullptr; }

void AVRReturn::print(formatted_raw_ostream &OS, unsigned Depth,
                      VerbosityLevel VLevel) const {}

StringRef AVRReturn::getAvrTypeName() const { return StringRef("RETURN"); }

//----------AVR Select Implementation----------//
AVRSelect::AVRSelect(unsigned SCID) : AVR(SCID) {}

AVRSelect *AVRSelect::clone() const { return nullptr; }

void AVRSelect::print(formatted_raw_ostream &OS, unsigned Depth,
                      VerbosityLevel VLevel) const {}

StringRef AVRSelect::getAvrTypeName() const { return StringRef("SELECT"); }

//----------AVR Compare Implementation----------//
AVRCompare::AVRCompare(unsigned SCID) : AVR(SCID) {}

AVRCompare *AVRCompare::clone() const { return nullptr; }

void AVRCompare::print(formatted_raw_ostream &OS, unsigned Depth,
                       VerbosityLevel VLevel) const {}

StringRef AVRCompare::getAvrTypeName() const { return StringRef("COMPARE"); }

//----------AVR Wrn Implementation----------//
AVRWrn::AVRWrn(WRNVecLoopNode *WrnSimdNode)
    : AVR(AVR::AVRWrnNode), WRegionSimdNode(WrnSimdNode) {}

AVRWrn *AVRWrn::clone() const { return nullptr; }

void AVRWrn::print(formatted_raw_ostream &OS, unsigned Depth,
                   VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');

  OS << Indent;

  switch (VLevel) {
  case PrintNumber:
    OS << "(" << getNumber() << ") ";
  case PrintAvrType:
  case PrintDataType:
  case PrintBase:
    OS << getAvrTypeName() << "\n";
    OS << Indent << "{\n";
    Depth++;

    for (auto Itr = child_begin(), End = child_end(); Itr != End; ++Itr)
      Itr->print(OS, Depth, VLevel);

    OS << Indent << "}\n";
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }
}

StringRef AVRWrn::getAvrTypeName() const { return StringRef("WRN"); }

std::string AVRWrn::getAvrValueName() const { return ""; }

void AVRWrn::codeGen() {
  for (auto Itr = child_begin(), End = child_end(); Itr != End; ++Itr)
    Itr->codeGen();
}

//----------AVR NOP Implementation----------//
AVRNOP::AVRNOP() : AVR(AVR::AVRNOPNode) {}

AVRNOP *AVRNOP::clone() const { return nullptr; }

void AVRNOP::print(formatted_raw_ostream &OS, unsigned Depth,
                   VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');
  OS << Indent;

  // Print AVR NOP Node.
  switch (VLevel) {
  case PrintNumber:
    OS << "(" << getNumber() << ")";
  case PrintAvrType:
    OS << getAvrTypeName() << "{";
  case PrintDataType:
  case PrintBase:
    OS << getAvrValueName();
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  // Close up open braces
  if (VLevel >= PrintAvrType)
    OS << "}";

  OS << "\n";
}

StringRef AVRNOP::getAvrTypeName() const { return StringRef("NOP"); }

std::string AVRNOP::getAvrValueName() const { return "No Operation"; }

//----------AVR Unreachable Implementation----------//
AVRUnreachable::AVRUnreachable(unsigned SCID) : AVR(SCID) {}

AVRUnreachable *AVRUnreachable::clone() const { return nullptr; }

void AVRUnreachable::print(formatted_raw_ostream &OS, unsigned Depth,
                           VerbosityLevel VLevel) const {

  std::string Indent((Depth * TabLength), ' ');
  OS << Indent;

  // Print AVR Unreachable Node.
  switch (VLevel) {
  case PrintNumber:
    OS << "(" << getNumber() << ")";
  case PrintAvrType:
    OS << getAvrTypeName() << "{";
  case PrintDataType:
  case PrintBase:
    OS << getAvrValueName();
    break;
  default:
    llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  // Close up open braces
  if (VLevel >= PrintAvrType)
    OS << "}";

  OS << "\n";
}

StringRef AVRUnreachable::getAvrTypeName() const {
  return StringRef("Unreachable");
}

std::string AVRUnreachable::getAvrValueName() const { return "unreachable"; }
