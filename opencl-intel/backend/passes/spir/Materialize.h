/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __MATERIALIZE_H__
#define __MATERIALIZE_H__

#include "llvm/Pass.h"
#include "BuiltinLibInfo.h"

namespace intel {
///////////////////////////////////////////////////////////////////////////////
// @name  SpirMaterializer
// @brief Adjusts the given module to be processed by the BE.
// More concretely:
// - replaces SPIR artifacts with Intel-implementation specific stuff.
// - updates LLVM IR to version supported by back-end compiler
///////////////////////////////////////////////////////////////////////////////
class SpirMaterializer : public llvm::ModulePass {
public:
  SpirMaterializer();

  bool runOnModule(llvm::Module &);

  virtual StringRef getPassName() const {
    return "spir materializer";
  }

  static char ID;

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const {
    AU.addRequired<BuiltinLibInfo>();
  }

};
}

#endif //__MATERIALIZE_H__
