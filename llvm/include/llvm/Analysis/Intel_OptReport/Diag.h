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

namespace llvm {

class OptReportDiag {
  unsigned Id;
  const char *Msg;
  OptReportDiag(unsigned Id, const char *Msg) : Id(Id), Msg(Msg) {}
  const char *getMsg() { return Msg; }
  unsigned getMsgId() { return Id; }
  /// Array of actual diagnistics IDs and messages.
  static OptReportDiag Diags[];
  /// Size of Diags[] array.
  static unsigned DiagsMax;
  /// Vec-report ID starts at 15300.
  static const unsigned VecBegin = 15300;
  /// Vec-report ID ends at 15553. "Not vectorized message" has to be
  /// below 15555 (15555 - 15300 = 255) in order to fit within 8 bits for
  /// VectorAdvisor communication.
  ///
  /// The issue with VectorAdvisor communication has to be addressed after
  /// optimization reports high level design for xmain is approved.
  ///
  /// 15552 and below from ICC, the rest is xmain-specific.
  static const unsigned VecEnd = 15557;
  /// Loop-report ID used in ASM/OBJ starts at 25481.
  static const unsigned LoopBegin = 25422;
  /// Loop-report ID used in ASM/OBJ ends at 25531.
  static const unsigned LoopEnd = 25531;

public:
  /// Retrieve message string from the diagnostic ID.
  static const char *getMsg(unsigned Id);
  /// ID to represent invalid remarks i.e. remarks which are not present in
  /// Diags table.
  static const unsigned InvalidRemarkID = 0;
};

} // namespace llvm

#endif // LLVM_ANALYSIS_INTEL_OPTREPORT_DIAG_H
