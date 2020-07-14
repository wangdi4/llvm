//===------ Diag.h - Diagnostics for HIR -------*- C++ -*---------===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
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

#ifndef LLVM_IR_INTEL_LOOPIR_DIAG_H
#define LLVM_IR_INTEL_LOOPIR_DIAG_H

namespace llvm {
namespace loopopt {

class OptReportDiag {
  int Id;
  const char *Msg;
  OptReportDiag(int Id, const char *Msg) : Id(Id), Msg(Msg) {}
  const char *getMsg() { return Msg; }
  int getMsgId() { return Id; }
  /// \brief Array of actual diagnistics IDs and messages.
  static OptReportDiag Diags[];
  /// \brief Size of Diags[] array.
  static int DiagsMax;
  /// \brief Vec-report ID starts at 15300.
  static const int VecBegin = 15300;
  /// \brief Vec-report ID ends at 15553. "Not vectorized message" has to be
  /// below 15555 (15555 - 15300 = 255) in order to fit within 8 bits for
  /// VectorAdvisor communication.
  ///
  /// The issue with VectorAdvisor communication has to be addressed after
  /// optimization reports high level design for xmain is approved.
  ///
  /// 15552 and below from ICC, the rest is xmain-specific.
  static const int VecEnd = 15557;
  /// \brief Loop-report ID used in ASM/OBJ starts at 25481.
  static const int LoopBegin = 25481;
  /// \brief Loop-report ID used in ASM/OBJ ends at 25531.
  static const int LoopEnd = 25531;

public:
  /// \brief Retrieve message string from the diagnostic ID.
  static const char *getMsg(int Id);
};

} // End loopopt namespace
} // End llvm namespace

#endif
