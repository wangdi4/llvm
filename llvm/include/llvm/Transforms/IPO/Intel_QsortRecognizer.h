//===----------- Intel_QsortRecognizer.h ----------------------------------===//
//
// Copyright (C) 2019-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// Qsort Recognizer
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_QSORTRECOGNIZER_H
#define LLVM_TRANSFORMS_IPO_INTEL_QSORTRECOGNIZER_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

///
/// Pass to perform qsort recognition.
///
class QsortRecognizerPass : public PassInfoMixin<QsortRecognizerPass> {
public:
  QsortRecognizerPass(void);
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm
#endif // LLVM_TRANSFORMS_IPO_INTEL_QSORTRECOGNIZER_H
