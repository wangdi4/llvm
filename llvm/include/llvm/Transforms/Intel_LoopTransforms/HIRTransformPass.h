//===-- HIRTransformPass.h - Base class for HIR transformations -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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

class HIRCreation;

/// \brief - All HIR transformation passes should derive from this class.
///
/// HIR pass setup requirements (see HIRCompleteUnroll.cpp for ref)-
///
/// - Define under Intel_LoopTransforms directory.
/// - Use the INITIALIZE_PASS* macros for initialization.
/// - Declare initialize<PassName>Pass() in llvm/InitializePasses.h and add a
///   call in initializeIntel_LoopTransforms.cpp.
/// - Declare create<PassName>Pass() in Intel_LoopTransforms/Passes.h, define
///   it in your file and add a call in llvm/LinkAllPasses.h (so it is not
///   optimized away) and PassManagerBuilder.cpp (to add it to clang opt
///   pipeline).
/// - Declare a boolean option to enable/disable the transformation.
/// - Define pass under anonymous(preferred) or loopopt namespace.
/// - Declare HIRCreation analysis as a required pass to access HIR. Use base
///   class's 'HIR' data member to store it.
/// - Declare HIRParser analysis as a required pass to access blob utilities
///   like findBlob() etc.
/// - Always call setPreservesAll() in getAnalysisUsage().
class HIRTransformPass : public FunctionPass {
protected:
  HIRCreation *HIR;

public:
  HIRTransformPass(char &ID) : FunctionPass(ID) {}

  /// \brief Overrides FunctionPass's printer pass to return one which prints
  /// HIR instead of LLVM IR.
  FunctionPass *createPrinterPass(raw_ostream &OS,
                                  const std::string &Banner) const override {
    return createHIRPrinterPass(OS, Banner);
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
