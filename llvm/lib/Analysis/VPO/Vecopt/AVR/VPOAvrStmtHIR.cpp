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
//   VPOAvrStmtHIR.cpp -- Implements the Abstract Vector Representation (AVR)
//   statement nodes for HIR.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrStmtHIR.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "avr-stmt-node"

//----------AVR Assign for HIR Implementation----------//
AVRAssignHIR::AVRAssignHIR(HLNode * Inst)
  : AVRAssign(AVR::AVRAssignHIRNode), Instruct(Inst) {}

AVRAssignHIR *AVRAssignHIR::clone() const {
  return nullptr;
}

void AVRAssignHIR::print(formatted_raw_ostream &OS, unsigned Depth,
                      VerbosityLevel VLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VLevel > PrintBase) {
    OS << Indent << "AVR_ASSIGN:";
    Instruct->print(OS, Depth);
    OS << "\n" ;
  }
}

std::string AVRAssignHIR::getAvrValueName() const {
  return "";
}

//----------AVR Label for HIR Implementation----------//
AVRLabelHIR::AVRLabelHIR(HLNode *Inst)
  : AVRLabel(AVR::AVRLabelHIRNode), Instruct(Inst) {}

AVRLabelHIR *AVRLabelHIR::clone() const {
  return nullptr;
}

void AVRLabelHIR::print(formatted_raw_ostream &OS, unsigned Depth,
                     VerbosityLevel VLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VLevel > PrintBase) {
    OS << Indent << "AVR_LABEL:";
    Instruct->print(OS, Depth);
    OS << "\n" ;
  }
}

std::string AVRLabelHIR::getAvrValueName() const {
  return "";
}

//----------AVR Branch for HIR Implementation----------//
AVRBranchHIR::AVRBranchHIR(HLNode * Inst)
  : AVRBranch(AVR::AVRBranchHIRNode), Instruct(Inst) {}

AVRBranchHIR *AVRBranchHIR::clone() const {
  return nullptr;
}

void AVRBranchHIR::print(formatted_raw_ostream &OS, unsigned Depth,
                       VerbosityLevel VLevel) const {
  std::string Indent(Depth * TabLength, ' ');

  if (VLevel > PrintBase) {
    OS << Indent << "AVR_FBRANCH:";
    Instruct->print(OS, Depth);
    OS << "\n" ;
  }
}

std::string AVRBranchHIR::getAvrValueName() const {
  return "";
}
