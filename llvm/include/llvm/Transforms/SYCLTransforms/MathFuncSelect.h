//===-- MathFuncSelect.h - Select math builtin for required accruacy -----===//
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
//===---------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_MATH_FUNC_SELECT_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_MATH_FUNC_SELECT_H

#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class RequiredAccuracyDiagInfo : public DiagnosticInfoWithLocationBase {
  const Twine &Msg;

public:
  static DiagnosticKind Kind;
  RequiredAccuracyDiagInfo(const Function &F, const Twine &Msg,
                           DiagnosticSeverity Severity = DS_Warning)
      : DiagnosticInfoWithLocationBase(Kind, Severity, F, DiagnosticLocation()),
        Msg(Msg) {}

  static bool classof(const DiagnosticInfo *DI) {
    return DI->getKind() == Kind;
  }

  void print(DiagnosticPrinter &DP) const override {
    DP << "FP builtin error requirement not met in function \""
       << getFunction().getName() << "\": " << Msg;
  }
};

class MathFuncSelectPass : public PassInfoMixin<MathFuncSelectPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  static bool isRequired() { return true; }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_MATH_FUNC_SELECT_H
