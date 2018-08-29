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

#include "InitializePasses.h"
#include "OCLPassSupport.h"
#include "ImplicitArgsAnalysis.h"
namespace intel{
char ImplicitArgsAnalysis::ID = 0;

// Handle the Pass registration stuff necessary to use ImplicitArgsAnalysis's.

// Register comminucation channel between code generation driver and executor...
OCL_INITIALIZE_PASS(ImplicitArgsAnalysis, "implicit-args-analysis", "Implicit Args Analysis", false, true)

}
extern "C" {
  llvm::ImmutablePass * createImplicitArgsAnalysisPass(llvm::LLVMContext* C) {
    return new intel::ImplicitArgsAnalysis(C);
  }
}

