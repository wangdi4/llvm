//===-------------- OCLVecClone.h - Class definition -*- C++
//-*---------------===//
//
// Copyright (C) 2018-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file defines the OCLReqdSubGroupSize pass class.
///
// ===--------------------------------------------------------------------=== //
#ifndef BACKEND_VECTORIZER_OCLVECCLONE_OCLVECCLONE_H
#define BACKEND_VECTORIZER_OCLVECCLONE_OCLVECCLONE_H

#include "llvm/Pass.h"

namespace intel {
class OCLReqdSubGroupSize : public llvm::ModulePass {
private:
  bool runOnModule(llvm::Module &M) override;

public:
  static char ID;

  OCLReqdSubGroupSize();

  /// Returns the name of the pass
  llvm::StringRef getPassName() const override {
    return "OCLReqdSubGroupSize pass";
  }
};
} // namespace intel
#endif // BACKEND_VECTORIZER_OCLVECCLONE_OCLVECCLONE_H
