//===- LoopOptReportPrintUtils.cpp - Utils to print Loop Reports -*- C++ -*-==//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares a set of routines for printing Metadata-based
// Loop Optimization Reports.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_OptReport/LoopOptReport.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/FormattedStream.h"

namespace llvm {
class DebugLoc;
namespace OptReportUtils {
void printRemark(formatted_raw_ostream &FOS, unsigned Depth, LoopOptRemark R);
void printOrigin(formatted_raw_ostream &FOS, unsigned Depth,
                 LoopOptRemark Origin);
void printDebugLocation(formatted_raw_ostream &FOS, unsigned Depth,
                        const DILocation *DL);
void printLoopHeader(formatted_raw_ostream &FOS, unsigned Depth);
void printLoopFooter(formatted_raw_ostream &FOS, unsigned Depth);
void printLoopHeaderAndOrigin(formatted_raw_ostream &FOS, unsigned Depth,
                              LoopOptReport OptReport, const DebugLoc &DL);
// This function prints the opt report enclosed with header/footer.
// It is useful for printing first childs or next siblings.
void printEnclosedOptReport(formatted_raw_ostream &FOS, unsigned Depth,
                            LoopOptReport OptReport);

void printOptReport(formatted_raw_ostream &FOS, unsigned Depth,
                    LoopOptReport OptReport);
} // namespace OptReportUtils
} // namespace llvm
