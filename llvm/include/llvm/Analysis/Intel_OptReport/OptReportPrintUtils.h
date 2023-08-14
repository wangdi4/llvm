//===------ OptReportPrintUtils.cpp - Utils to print Opt Reports -*- C++ -*-==//
//
// Copyright (C) 2017-2021 Intel Corporation. All rights reserved.
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
#include "llvm/Support/FormattedStream.h"

namespace llvm {
class DebugLoc;
namespace OptReportUtils {
void printRemark(formatted_raw_ostream &FOS, unsigned Depth, OptRemark R);
void printOrigin(formatted_raw_ostream &FOS, unsigned Depth, OptRemark Origin);
void printDebugLocation(formatted_raw_ostream &FOS, unsigned Depth,
                        const DILocation *DL, bool AbsolutePaths);
void printNodeHeader(formatted_raw_ostream &FOS, unsigned Depth, OptReport OR);
void printNodeFooter(formatted_raw_ostream &FOS, unsigned Depth, OptReport OR);
void printNodeHeaderAndOrigin(formatted_raw_ostream &FOS, unsigned Depth,
                              OptReport OR, const DebugLoc &DL,
                              bool AbsolutePaths);
// This function prints the opt report enclosed with header/footer.
// It is useful for printing first childs or next siblings.
void printEnclosedOptReport(formatted_raw_ostream &FOS, unsigned Depth,
                            OptReport OR, bool AbsolutePaths);

void printOptReport(formatted_raw_ostream &FOS, unsigned Depth, OptReport OR,
                    bool AbsolutePaths);
} // namespace OptReportUtils
} // namespace llvm
