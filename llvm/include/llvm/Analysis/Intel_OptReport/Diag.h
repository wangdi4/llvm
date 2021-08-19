//===------ Diag.h - Diagnostics for HIR -------*- C++ -*---------===//
//
// Copyright (C) 2015-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===--------------------------------------------------------------------===//
//
// This file defines the diagnostics in high level IR.
//
// This is a temporary solution until we move to the diagnostics
// mechanism using derived classes from DiagnosticInfo.
//
//===--------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_OPTREPORT_DIAG_H
#define LLVM_ANALYSIS_INTEL_OPTREPORT_DIAG_H

#include "llvm/ADT/DenseMap.h"

namespace llvm {

class OptReportDiag {
  static const DenseMap<unsigned, const char *> Diags;

public:
  /// Retrieve message string from the diagnostic ID.
  static const char *getMsg(unsigned Id);
  /// ID to represent invalid remarks i.e. remarks which are not present in
  /// Diags table.
  static const unsigned InvalidRemarkID = 0;
};

} // namespace llvm

#endif // LLVM_ANALYSIS_INTEL_OPTREPORT_DIAG_H
