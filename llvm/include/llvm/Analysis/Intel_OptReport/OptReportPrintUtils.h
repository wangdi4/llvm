//===------ OptReportPrintUtils.cpp - Utils to print Opt Reports -*- C++ -*-==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares a set of routines for printing Metadata-based
// Optimization Reports.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_OptReport/OptReport.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Metadata.h"

namespace llvm {
class DebugLoc;
namespace OptReportUtils {

#ifndef NDEBUG
/// Verifies that \p Remark has the correct number and type of metadata
/// arguments for its format string.
///
/// If not, this function prints an explanation of the mismatch and asserts or
/// calls llvm_unreachable.
void validateRemarkFormatArguments(OptRemark Remark);
#endif // NDEBUG

void printRemark(raw_ostream &OS, unsigned Depth, OptRemark R);
void printOrigin(raw_ostream &OS, unsigned Depth, OptRemark Origin);
void printDebugLocation(raw_ostream &OS, unsigned Depth, const DILocation *DL,
                        bool AbsolutePaths);
void printNodeHeader(raw_ostream &OS, unsigned Depth, OptReport OR);
void printNodeFooter(raw_ostream &OS, unsigned Depth, OptReport OR);
void printNodeHeaderAndOrigin(raw_ostream &OS, unsigned Depth, OptReport OR,
                              const DebugLoc &DL, bool AbsolutePaths);
// This function prints the opt report enclosed with header/footer.
// It is useful for printing first childs or next siblings.
void printEnclosedOptReport(raw_ostream &OS, unsigned Depth, OptReport OR,
                            bool AbsolutePaths);

void printOptReport(raw_ostream &OS, unsigned Depth, OptReport OR,
                    bool AbsolutePaths);
} // namespace OptReportUtils
} // namespace llvm
