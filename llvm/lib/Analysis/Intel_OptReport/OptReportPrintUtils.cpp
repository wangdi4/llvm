//===----- OptReportPrintUtils.cpp - Utils to print Loop Reports -*- C++ -*-==//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a set of routines for printing Metadata-based
// Loop Optimization Reports.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_OptReport/OptReportPrintUtils.h"
#include "llvm/Analysis/Intel_OptReport/Diag.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/Support/FileSystem.h"

namespace llvm {

namespace OptReportUtils {
static int getMDNodeAsInt(const ConstantAsMetadata *CM) {
  return cast<ConstantInt>(CM->getValue())->getValue().getSExtValue();
}
static unsigned getMDNodeAsUnsigned(const ConstantAsMetadata *CM) {
  return cast<ConstantInt>(CM->getValue())->getValue().getZExtValue();
}

static const unsigned IntentationStep = 4;

#ifndef NDEBUG
void validateRemarkFormatArguments(OptRemark Remark) {
  const OptRemarkID RemarkID = Remark.getRemarkID();
  const char *const Format = OptReportDiag::getMsg(RemarkID);
  SmallVector<const char *> ValidSpecifiers;
  for (const char *Specifier = Format; *Specifier != '\0'; ++Specifier) {
    if (*Specifier != '%')
      continue;
    switch (Specifier[1]) {
    case '\0':
      errs() << "Remark #" << RemarkID << ": "
             << StringRef(Format, Specifier - Format) << raw_ostream::RED << "%"
             << raw_ostream::RESET << "\n";
      llvm_unreachable("Unpaired % at end of format string");
    case '%':
      ++Specifier;
      break;
    case 'd':
    case 's':
      ValidSpecifiers.push_back(Specifier);
      ++Specifier;
      break;
    default:
      errs() << "Remark #" << RemarkID << ": "
             << StringRef(Format, Specifier - Format) << raw_ostream::RED
             << StringRef(Specifier, 2) << raw_ostream::RESET << (Specifier + 2)
             << "\n";
      llvm_unreachable("Only %d and %s are supported as format specifiers");
    }
  }
  if (Remark.getNumOperands() != ValidSpecifiers.size() + 1) {
    errs() << ValidSpecifiers.size() << " specifier(s) in format:\n";
    errs() << "Remark #" << RemarkID << ": ";
    const char *FormatToPrint = Format;
    for (const char *const Specifier : ValidSpecifiers) {
      errs() << StringRef(FormatToPrint, Specifier - FormatToPrint)
             << raw_ostream::RED << StringRef(Specifier, 2)
             << raw_ostream::RESET;
      FormatToPrint = Specifier + 2;
    }
    errs() << FormatToPrint << "\n";
    errs() << (Remark.getNumOperands() - 1) << " argument(s) in metadata:\n";
    for (unsigned Idx = 1; Idx < Remark.getNumOperands(); ++Idx) {
      errs() << "  " << *Remark.getOperand(Idx) << "\n";
    }
    llvm_unreachable(
        "Number of arguments doesn't match number of format specifiers");
  }
  for (const auto [ArgIdx, Specifier] : enumerate(ValidSpecifiers)) {
    switch (Specifier[1]) {
    case 's':
      if (!isa<MDString>(Remark.getOperand(ArgIdx + 1))) {
        errs() << "Remark #" << RemarkID << ": "
               << StringRef(Format, Specifier - Format) << raw_ostream::RED
               << StringRef(Specifier, 2) << raw_ostream::RESET
               << (Specifier + 2) << "\n";
        errs() << "Argument " << (ArgIdx + 1) << ": "
               << *Remark.getOperand(ArgIdx + 1);
        llvm_unreachable("Argument for %s is not a string");
      }
      break;
    case 'd':
      if (!mdconst::hasa<ConstantInt>(Remark.getOperand(ArgIdx + 1))) {
        errs() << "Remark #" << RemarkID << ": "
               << StringRef(Format, Specifier - Format) << raw_ostream::RED
               << StringRef(Specifier, 2) << raw_ostream::RESET
               << (Specifier + 2) << "\n";
        errs() << "Argument " << (ArgIdx + 1) << ": "
               << *Remark.getOperand(ArgIdx + 1);
        llvm_unreachable("Argument for %d is not an integer");
      }
      break;
    }
  }
}
#endif // NDEBUG

// The purpose of this function is to create a printable remark message.
// \p OptReportRemark looks like this :
// !{!"Partially unrolled with %d factor", i32 8}
// ..and it should return "Partially unrolled with 8 factor".
// The function assumes only %s and %d format specifiers.
// TODO Alexander: Replace the format string parameter with msg ID.
//                 Add the support for %u, %f.
//                 Add unit tests for this function.
std::string formatRemarkMessage(OptRemark Remark, OptRemarkID RemarkID) {
  assert(RemarkID != OptRemarkID::InvalidRemarkID && "Remark ID is invalid!");
#ifndef NDEBUG
  validateRemarkFormatArguments(Remark);
#endif // NDEBUG
  std::string FormatString = std::string(OptReportDiag::getMsg(RemarkID));

  std::string Msg;
  // First, reserve some space in the string to avoid memory rellocations.
  Msg.reserve(2 * FormatString.length());
  unsigned CurOpIdx = 1; // 0 -> RemarkID
  unsigned Idx = 0;
  unsigned FormatStringLength = FormatString.length();
  while (Idx < FormatStringLength) {
    // If current character in the format string is not '%' - just copy it to
    // the result and continue.
    if (FormatString[Idx] != '%') {
      Msg.push_back(FormatString[Idx]);
      ++Idx;
      continue;
    }

    // 'Naked' percents are not supported even at the end of the format string.
    if (Idx == FormatString.length() - 1) {
      llvm_unreachable("'Naked' percents are not supported");
      return "";
    }

    // If there is escaped percent - add only one percent to the result, skip
    // it and continue.
    if (FormatString[Idx + 1] == '%') {
      Msg.push_back('%');
      Idx += 2;
      continue;
    }

    // Now, we know for sure that we will need another operand from the remark.
    // Check that we actually have it. If don't - return empty string in case of
    // the release build.
    bool HaveEnoughOperands = CurOpIdx < Remark.getNumOperands();
    assert(HaveEnoughOperands && "Not sufficient number of arguments is "
                                 "provided for the remark string");
    if (!HaveEnoughOperands)
      return "";

    switch (FormatString[Idx + 1]) {
    case 'd': {
      // Try to cast and print current operand as Constant Int
      const ConstantAsMetadata *CM =
          dyn_cast<ConstantAsMetadata>(Remark.getOperand(CurOpIdx));
      assert(CM && "Mismatch between formatted string and provided "
                   "arguments: expected format specifier was %d");
      // In case of argument mismatch for release build just return an
      // empty string.
      if (!CM)
        return "";
      int I = getMDNodeAsInt(CM);
      std::string IntStr = std::to_string(I);
      Msg.append(IntStr);
      break;
    }
    case 's': {
      const MDString *SM = dyn_cast<MDString>(Remark.getOperand(CurOpIdx));
      assert(SM && "Mismatch between formatted string and provided "
                   "arguments: expected format specifier was %s");
      // In case of argument mismatch for release build just return an
      // empty string.
      if (!SM)
        return "";
      Msg.append(std::string(SM->getString()));
      break;
    }
    default:
      llvm_unreachable("The format specifier is not supported");
      return "";
    }
    // We have to skip one more character, because of the '%' and also increase
    // the index for the current operand in the remark.
    Idx += 2;
    CurOpIdx++;
  }
  // Check that we actually used all supplied arguments.
  assert(CurOpIdx == Remark.getNumOperands() &&
         "Mismatch between the number of format specifiers and the number of "
         "actually supplied arguments ");

  return Msg;
}

void printRemark(raw_ostream &OS, unsigned Depth, OptRemark Remark) {
  assert(Remark && "Client code is responsible for providing non-null Remark");
  OS.indent(IntentationStep * Depth);
  OptRemarkID RemarkID = Remark.getRemarkID();
  OS << "remark #" << RemarkID << ": " << formatRemarkMessage(Remark, RemarkID)
     << "\n";
}

void printOrigin(raw_ostream &OS, unsigned Depth, OptRemark Origin) {
  assert(Origin && "Client code is responsible for providing non-null Origin");

  OS.indent(IntentationStep * Depth);
  OS << "<" << formatRemarkMessage(Origin, Origin.getRemarkID()) << ">\n";
}

void printDebugLocation(raw_ostream &OS, unsigned Depth, const DILocation *DL,
                        bool AbsolutePaths) {
  assert(DL &&
         "Client code is responsible for providing non-null debug location");

  auto *Scope = cast<DIScope>(DL->getScope());
  SmallVector<char, 64> Path(Scope->getFilename().bytes());
  if (AbsolutePaths)
    sys::fs::make_absolute(Scope->getDirectory(), Path);
  OS << " at " << Path << " (" << DL->getLine() << ", " << DL->getColumn()
     << ")\n";
}

void printNodeHeader(raw_ostream &OS, unsigned Depth, OptReport OR) {
  OS << '\n';
  OS.indent(IntentationStep * Depth);
  OS << OR.title() << " BEGIN";
}

void printNodeFooter(raw_ostream &OS, unsigned Depth, OptReport OR) {
  OS.indent(IntentationStep * Depth);
  OS << OR.title() << " END\n";
}

void printNodeHeaderAndOrigin(raw_ostream &OS, unsigned Depth, OptReport OR,
                              const DebugLoc &DL, bool AbsolutePaths) {
  printNodeHeader(OS, Depth, OR);

  if (DL.get())
    printDebugLocation(OS, Depth, DL.get(), AbsolutePaths);
  else if (OR && OR.debugLoc())
    printDebugLocation(OS, Depth, OR.debugLoc(), AbsolutePaths);
  else
    OS << "\n";

  if (OR) {
    for (const OptRemark R : OR.origin()) {
      printOrigin(OS, Depth, R);
    }
  }
}

void printEnclosedOptReport(raw_ostream &OS, unsigned Depth, OptReport OR,
                            bool AbsolutePaths) {
  assert(OR && "Client code is responsible for providing non-null OptReport");

  printNodeHeaderAndOrigin(OS, Depth, OR, DebugLoc(), AbsolutePaths);

  printOptReport(OS, Depth + 1, OR, AbsolutePaths);
  printNodeFooter(OS, Depth, OR);

  // After printing Optimization Report for the first child, we check whether it
  // has attached lost next sibling loops.
  if (OR.nextSibling())
    printEnclosedOptReport(OS, Depth, OR.nextSibling(), AbsolutePaths);
}

void printOptReport(raw_ostream &OS, unsigned Depth, OptReport OR,
                    bool AbsolutePaths) {
  assert(OR && "Client code is responsible for providing non-null OptReport");

  for (const OptRemark R : OR.remarks())
    printRemark(OS, Depth, R);

  // After printing Optimization Report for the loop, we check whether it has
  // attached lost child loops opt reports.
  if (OR.firstChild())
    printEnclosedOptReport(OS, Depth, OR.firstChild(), AbsolutePaths);
}
} // namespace OptReportUtils
} // namespace llvm
