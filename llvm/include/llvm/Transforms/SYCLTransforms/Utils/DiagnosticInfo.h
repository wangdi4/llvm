//===- DiagnosticInfo.h - SYCLTransforms Diagnostic Declaration--*- C++ -*-===//
//
// Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_DIAGNOSTICINFO_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_DIAGNOSTICINFO_H

#include "llvm/ADT/Twine.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"

namespace llvm {

class OptimizationErrorDiagInfo : public DiagnosticInfo {
  const Twine &Msg;

public:
  static DiagnosticKind Kind;
  OptimizationErrorDiagInfo(const Twine &Msg)
      : DiagnosticInfo(Kind, DS_Error), Msg(Msg) {}

  static bool classof(const DiagnosticInfo *DI) {
    return DI->getKind() == Kind;
  }

  void print(DiagnosticPrinter &DP) const override { DP << Msg; }
};

class OptimizationWarningDiagInfo : public DiagnosticInfo {
  const Twine &Msg;

public:
  static DiagnosticKind Kind;
  OptimizationWarningDiagInfo(const Twine &Msg)
      : DiagnosticInfo(Kind, DS_Warning), Msg(Msg) {}

  static bool classof(const DiagnosticInfo *DI) {
    return DI->getKind() == Kind;
  }

  void print(DiagnosticPrinter &DP) const override { DP << Msg; }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_DIAGNOSTICINFO_H
