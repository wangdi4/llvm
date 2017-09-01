//===----------------------------------------------------------------------===//
//
//   Copyright (C) 2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the Abstract Vector Representation (AVR) switch node.
///
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrSwitch.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrStmt.h"

#define DEBUG_TYPE "avr-switch-node"

using namespace llvm;
using namespace llvm::vpo;

AVRSwitch::AVRSwitch(unsigned SCID)
  : AVR(SCID) {}

AVRSwitch::case_child_iterator
AVRSwitch::case_child_begin_internal(unsigned CaseNumber) {
  if (CaseNumber == 0) {
    return Children.begin();
  } else {
    return CaseBegin[CaseNumber - 1];
  }
}

AVRSwitch::const_case_child_iterator
AVRSwitch::case_child_begin_internal(unsigned CaseNumber) const {
  return const_cast<AVRSwitch *>(this)->case_child_begin_internal(CaseNumber);
}

AVRSwitch::case_child_iterator
AVRSwitch::case_child_end_internal(unsigned CaseNumber) {
  if (CaseNumber == getNumCases()) {
    return Children.end();
  } else {
    return CaseBegin[CaseNumber];
  }
}

AVRSwitch::const_case_child_iterator
AVRSwitch::case_child_end_internal(unsigned CaseNumber) const {
  return const_cast<AVRSwitch *>(this)->case_child_end_internal(CaseNumber);
}

AVRSwitch::reverse_case_child_iterator
AVRSwitch::case_child_rbegin_internal(unsigned CaseNumber) {
  if (CaseNumber == getNumCases()) {
    return Children.rbegin();
  } else {
    return reverse_case_child_iterator(*CaseBegin[CaseNumber]);
  }
}

AVRSwitch::const_reverse_case_child_iterator
AVRSwitch::case_child_rbegin_internal(unsigned CaseNumber) const {
  return const_cast<AVRSwitch *>(this)->case_child_rbegin_internal(CaseNumber);
}

AVRSwitch::reverse_case_child_iterator
AVRSwitch::case_child_rend_internal(unsigned CaseNumber) {
  if (CaseNumber == 0) {
    return Children.rend();
  } else {
    return reverse_case_child_iterator(*CaseBegin[CaseNumber - 1]);
  }
}

AVRSwitch::const_reverse_case_child_iterator
AVRSwitch::case_child_rend_internal(unsigned CaseNumber) const {
  return const_cast<AVRSwitch *>(this)->case_child_rend_internal(CaseNumber);
}

AVR *AVRSwitch::getCaseFirstChildInternal(unsigned CaseNumber) {
  if (hasCaseChildrenInternal(CaseNumber)) {
    return &*(case_child_begin_internal(CaseNumber));
  }

  return nullptr;
}

AVR *AVRSwitch::getCaseLastChildInternal(unsigned CaseNumber) {
  if (hasCaseChildrenInternal(CaseNumber)) {
    return &*(std::prev(case_child_end_internal(CaseNumber)));
  }

  return nullptr;
}

void AVRSwitch::addCase() {
  CaseBegin.push_back(Children.end());
}

AVRSwitch *AVRSwitch::clone() const {
  return nullptr;
}


void AVRSwitch::printBreak(formatted_raw_ostream &OS, unsigned Depth,
                           unsigned CaseNum) const { 
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * TabLength, ' ');
  // TODO
  //if (caseBreaks())
  OS << Indent << "break;\n";
#endif // !INTEL_PRODUCT_RELEASE
}

void AVRSwitch::print(formatted_raw_ostream &OS, unsigned Depth,
                       VerbosityLevel VLevel) const {
#if !INTEL_PRODUCT_RELEASE


  std::string Indent(Depth * TabLength, ' ');
  unsigned NumberOfCases = getNumCases();
  OS << Indent;

  // Print Avr Switch Node and its case children.

  switch(VLevel) {
    case PrintCost:
    case PrintNumber:
      OS << "(" << getNumber() << ") ";
    case PrintAvrType:
    case PrintDataType:
      printSLEV(OS);
    case PrintBase:
      OS << getAvrTypeName() << "(";
      getCondition()->print(OS, 0, VLevel);
      OS <<")" << "{\n";

      // Print Switch Cases
      for (unsigned Case = 1; Case <= NumberOfCases; ++Case) {
        OS << Indent << "case " << Case << ":\n";
        for (auto Itr = case_child_begin(Case), 
                  End = case_child_end(Case);
             Itr != End; Itr++) {
          Itr->print(OS, Depth+2, VLevel); 
        }

	printBreak(OS, Depth+2, Case);
      }

      // Print Switch Default Case
      OS << Indent << "default:\n";
      for (auto Itr = default_case_child_begin(),
                End = default_case_child_end();
           (NumberOfCases > 0) && (Itr != End); Itr++) {
        Itr->print(OS, Depth + 2, VLevel); 
      }
      break;
    default:
      llvm_unreachable("Unknown Avr Print Verbosity!");
  }

  OS << Indent << "}\n";

  Depth++;
#endif // !INTEL_PRODUCT_RELEASE
}

void AVRSwitch::shallowPrint(formatted_raw_ostream &OS) const {
#if !INTEL_PRODUCT_RELEASE

  OS << "(" << getNumber() << ") ";
  printSLEV(OS);
  OS << "SWITCH (" << getCondition()->getNumber() << ")";
#endif // !INTEL_PRODUCT_RELEASE
}

StringRef AVRSwitch::getAvrTypeName() const {
  return StringRef("switch");
}
