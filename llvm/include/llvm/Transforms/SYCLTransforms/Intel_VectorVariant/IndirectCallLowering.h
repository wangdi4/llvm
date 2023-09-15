//===------------------- IndirectCallLowering.h -*- C++ -*-----------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_VECTORIZER_VECTORVARIANT_INDIRECT_CALL_LOWERING_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_VECTORIZER_VECTORVARIANT_INDIRECT_CALL_LOWERING_H

#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class VectorVariantDiagInfo : public DiagnosticInfoWithLocationBase {
  const Twine &Msg;

public:
  static DiagnosticKind Kind;
  VectorVariantDiagInfo(const Function &F, const Twine &Msg,
                        DiagnosticSeverity Severity = DS_Error)
      : DiagnosticInfoWithLocationBase(Kind, Severity, F, DiagnosticLocation()),
        Msg(Msg) {}

  static bool classof(const DiagnosticInfo *DI) {
    return DI->getKind() == Kind;
  }

  void print(DiagnosticPrinter &DP) const override {
    DP << "Function '" << getFunction().getName() << "': " << Msg;
  }
};

/// Lowering __intel_indirect_call scalar calls
class IndirectCallLowering : public PassInfoMixin<IndirectCallLowering> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
  bool runImpl(Module &M);
  static bool isRequired() { return true; }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_VECTORIZER_VECTORVARIANT_INDIRECT_CALL_LOWERING_H
