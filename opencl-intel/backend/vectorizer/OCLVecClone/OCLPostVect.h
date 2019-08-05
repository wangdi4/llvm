//===-------------- OCLPostVect.h - Class definition -*- C++
//-*---------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file defines the OCLPostVect pass.
// ===--------------------------------------------------------------------=== //
#ifndef BACKEND_VECTORIZER_OCLVECCLONE_OCLPOSTVECT_H
#define BACKEND_VECTORIZER_OCLVECCLONE_OCLPOSTVECT_H

#include "OCLPassSupport.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

using namespace llvm;

class ModulePass;

namespace intel {
class OCLPostVect : public llvm::ModulePass {

private:
  void getAnalysisUsage(AnalysisUsage &AU) const override;

  bool runOnModule(Module &M) override;

  bool isKernelVectorizationProfitable(Module &M, Function *F,
                                       Function *ClonedKernel);

  /// Checks if there are openmp directives in the kernel. If not, then the
  /// kernel was vectorized.
  bool isKernelVectorized(Function *F);

public:
  static char ID;

  OCLPostVect();
};
} // namespace intel
#endif // BACKEND_VECTORIZER_OCLVECCLONE_OCLPOSTVECT_H
