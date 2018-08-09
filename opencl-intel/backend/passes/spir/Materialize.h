// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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
