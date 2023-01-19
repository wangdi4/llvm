//===- ResolveVarTIDCall.h - Resolve TID with variable argument -*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation
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

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_RESOLVEVARTIDCALL_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_RESOLVEVARTIDCALL_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// Resolve get_*_id call with variable or out-of-bound argument.
/// If the argument is out-of-bound, it is replaced with const zero.
/// Otherwise, it is replaced with conditional get_*_id call with fixed
/// arguments of 0, 1 and 2.
///
/// This pass intends to simplify KernelAnalysis and remove limitation for
/// vectorizer and sub-group emulation.
class ResolveVarTIDCallPass : public PassInfoMixin<ResolveVarTIDCallPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  static bool isRequired() { return true; }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_RESOLVEVARTIDCALL_H
