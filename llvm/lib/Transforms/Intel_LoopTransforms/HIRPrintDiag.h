//===------ HIRPrintDiag.h - Interface for print diagnosis *-- C++ --*---//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains interface for printing diagnosis.
//
// Designed with an intention to help development and diagnosis
// even using prod compiler.
// Should be called only with a guard by really hidden compiler option.
// See usages in HIRLoopInterface.cpp or HIRLoopBlocking.cpp.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRPRINTDIAG_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRPRINTDIAG_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"

namespace llvm {

namespace loopopt {

void printDiag(StringRef FuncNameKnobFromCommandLine,
               unsigned DiagLevelKnobFromCommandLine, StringRef Msg,
               StringRef FuncName, const HLLoop *Loop = nullptr,
               StringRef Header = "", unsigned DiagLevel = 1);

}
} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRPRINTDIAG_H
