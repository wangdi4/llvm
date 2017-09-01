//===-- HIRTransformPass.h - Base class for HIR transformations -*- C++ -*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This header file declares the base class for HIR transformation passes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRTRANSFORMPASS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRTRANSFORMPASS_H

#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"

namespace llvm {

namespace loopopt {

/// \brief - All HIR transformation passes should derive from this class.
///
/// HIR pass setup requirements (see HIRCompleteUnroll.cpp for ref)-
///
/// - Define under Intel_LoopTransforms directory.
/// - Use the INITIALIZE_PASS* macros for initialization.
/// - Declare initialize<PassName>Pass() in llvm/InitializePasses.h and add a
///   call in Intel_LoopTransforms.cpp.
/// - Declare create<PassName>Pass() in Intel_LoopTransforms/Passes.h, define
///   it in your file and add a call in llvm/LinkAllPasses.h (so it is not
///   optimized away) and PassManagerBuilder.cpp (to add it to clang opt
///   pipeline).
/// - Declare a boolean option to enable/disable the transformation.
/// - Define pass under anonymous(preferred) or loopopt namespace.
/// - Declare HIRFramework analysis as a required pass to access HIR and blob
///   utilities  like findBlob() etc.
/// - Declare DDAnalysis pass as required to have an access to DD information.
/// - Always call setPreservesAll() in getAnalysisUsage().
class HIRTransformPass : public FunctionPass {
public:
  HIRTransformPass(char &ID) : FunctionPass(ID) {}

#if !INTEL_PRODUCT_RELEASE
  /// \brief Overrides FunctionPass's printer pass to return one which prints
  /// HIR instead of LLVM IR.
  FunctionPass *createPrinterPass(raw_ostream &OS,
                                  const std::string &Banner) const override {
    return createHIRPrinterPass(OS, Banner);
  }
#endif // !INTEL_PRODUCT_RELEASE
};

} // End namespace loopopt

} // End namespace llvm

#endif
