//===----- OptReportPrintUtils.cpp - Utils to print Loop Reports -*- C++ -*-==//
//
// Copyright (C) 2018-2021 Intel Corporation. All rights reserved.
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

namespace llvm {

namespace OptReportUtils {
static int getMDNodeAsInt(const ConstantAsMetadata *CM) {
  return cast<ConstantInt>(CM->getValue())->getValue().getSExtValue();
}
static unsigned getMDNodeAsUnsigned(const ConstantAsMetadata *CM) {
  return cast<ConstantInt>(CM->getValue())->getValue().getZExtValue();
}

static const unsigned IntentationStep = 4;

// The purpose of this function is to create a printable remark message.
// \p OptReportRemark looks like this :
// !{!"Partially unrolled with %d factor", i32 8}
// ..and it should return "Partially unrolled with 8 factor".
// The function assumes only %s and %d format specifiers.
// TODO Alexander: Replace the format string parameter with msg ID.
//                 Add the support for %u, %f.
//                 Add unit tests for this function.
std::string formatRemarkMessage(OptRemark Remark, unsigned RemarkID) {
  std::string FormatString;
  if (RemarkID == static_cast<unsigned>(OptRemarkID::InvalidRemarkID)) {
    // Valid remark ID was not provided, format string is obtained from metadata
    // operands (2nd operand).
    // TODO: Remove this code when all remarks have valid IDs i.e. legacy
    // interface is retired.
    const MDString *R = cast<MDString>(Remark.getOperand(1));
    FormatString = std::string(R->getString());
  } else {
    // Obtain format string from message catalogue using valid remark ID.
    FormatString = std::string(OptReportDiag::getMsg(RemarkID));
  }

  // Temporary assert to validate correctness of remark id and remark message.
  // Should be dropped in future.
  assert(FormatString == cast<MDString>(Remark.getOperand(1))->getString() &&
         "Remark messages don't match.");

  std::string Msg;
  // First, reserve some space in the string to avoid memory rellocations.
  Msg.reserve(2 * FormatString.length());
  unsigned CurOpIdx = 2; // 0 -> RemarkID, 1 -> FormatString
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

void printRemark(formatted_raw_ostream &FOS, unsigned Depth, OptRemark Remark) {
  assert(Remark && "Client code is responsible for providing non-null Remark");
  FOS.indent(IntentationStep * Depth);
  std::string RemarkPrefixStr;
  unsigned RemarkID = Remark.getRemarkID();
  if (RemarkID == static_cast<unsigned>(OptRemarkID::InvalidRemarkID))
    RemarkPrefixStr = "remark: ";
  else
    RemarkPrefixStr = "remark #" + std::to_string(RemarkID) + ": ";
  FOS << RemarkPrefixStr << formatRemarkMessage(Remark, RemarkID) << "\n";
}

void printOrigin(formatted_raw_ostream &FOS, unsigned Depth, OptRemark Origin) {
  assert(Origin && "Client code is responsible for providing non-null Origin");

  FOS.indent(IntentationStep * Depth);
  unsigned RemarkID = Origin.getRemarkID();
  FOS << "<" << formatRemarkMessage(Origin, RemarkID) << ">\n";
}

void printDebugLocation(formatted_raw_ostream &FOS, unsigned Depth,
                        const DILocation *DL) {
  assert(DL &&
         "Client code is responsible for providing non-null debug location");

  auto *Scope = cast<DIScope>(DL->getScope());
  FOS << " at " << Scope->getFilename() << " (" << DL->getLine() << ", "
      << DL->getColumn() << ")\n";
}

void printNodeHeader(formatted_raw_ostream &FOS, unsigned Depth, OptReport OR) {
  FOS << '\n';
  FOS.indent(IntentationStep * Depth);
  FOS << OR.title() << " BEGIN";
}

void printNodeFooter(formatted_raw_ostream &FOS, unsigned Depth, OptReport OR) {
  FOS.indent(IntentationStep * Depth);
  FOS << OR.title() << " END\n";
}

void printNodeHeaderAndOrigin(formatted_raw_ostream &FOS, unsigned Depth,
                              OptReport OR, const DebugLoc &DL) {
  printNodeHeader(FOS, Depth, OR);

  if (DL.get())
    printDebugLocation(FOS, Depth, DL.get());
  else if (OR && OR.debugLoc())
    printDebugLocation(FOS, Depth, OR.debugLoc());
  else
    FOS << "\n";

  if (OR) {
    for (const OptRemark R : OR.origin()) {
      printOrigin(FOS, Depth, R);
    }
  }
}

void printEnclosedOptReport(formatted_raw_ostream &FOS, unsigned Depth,
                            OptReport OR) {
  assert(OR && "Client code is responsible for providing non-null OptReport");

  printNodeHeaderAndOrigin(FOS, Depth, OR, DebugLoc());

  printOptReport(FOS, Depth + 1, OR);
  printNodeFooter(FOS, Depth, OR);

  // After printing Optimization Report for the first child, we check whether it
  // has attached lost next sibling loops.
  if (OR.nextSibling())
    printEnclosedOptReport(FOS, Depth, OR.nextSibling());
}

void printOptReport(formatted_raw_ostream &FOS, unsigned Depth, OptReport OR) {
  assert(OR && "Client code is responsible for providing non-null OptReport");

  for (const OptRemark R : OR.remarks())
    printRemark(FOS, Depth, R);

  // After printing Optimization Report for the loop, we check whether it has
  // attached lost child loops opt reports.
  if (OR.firstChild())
    printEnclosedOptReport(FOS, Depth, OR.firstChild());
}
} // namespace OptReportUtils
} // namespace llvm
