//===-- HIRTransformPass.h - Base class for HIR transformations -*- C++ -*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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
#include "llvm/IR/PassManager.h"
#include "llvm/IR/PrintPasses.h"
#include "llvm/Support/Debug.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
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
};

// Base class for HIR passes in the new pass manager.
// Passed should be derived like this-
//
// class HIRTempCleanupPass : public HIRPassInfoMixin<HIRTempCleanupPass> {
// public:
//   // PassName should match the name using which the pass is registered in
//   // PassRegistery.def.
//   static constexpr auto PassName = "hir-temp-cleanup";
//
//   // Derived class should define runImpl() instead of run() with the
//   // following signature-
//   PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
//                             HIRFramework &HIRF);
// };

template <typename DerivedTy>
class HIRPassInfoMixin : public PassInfoMixin<DerivedTy> {
protected:
  // Once we fix all the derived passes to set this flag correctly, we can
  // reinitialize it to false.
  bool ModifiedHIR = true;

public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {

    auto &HIRF = AM.getResult<HIRFrameworkAnalysis>(F);

    auto *DerivedPtr = static_cast<DerivedTy*>(this);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    bool ShouldPrintFunc = isFunctionInPrintList(F.getName());

    // HIR before hir-temp-cleanup acts as the reference HIR with 'PrintChanged'
    // option.
    if (ShouldPrintFunc &&
        (shouldPrintBeforePass(DerivedTy::PassName) ||
         (PrintChanged != ChangePrinter::None &&
          StringRef(DerivedTy::PassName).equals("hir-temp-cleanup")))) {
      dbgs() << "*** IR Dump Before " << DerivedPtr->name() << " ***\n";
      dbgs() << "Function: " << F.getName() << "\n";
      HIRF.print(false, dbgs());
    }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    auto PA = DerivedPtr->runImpl(F, AM, HIRF);

    // Run framework verifier.
    if (ModifiedHIR) {
      HIRF.verify();
    }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    // TODO: We can skip printing of HIR in 'quiet' mode of 'PrintChanged' and
    // just print the banner.
    if (ShouldPrintFunc &&
        (shouldPrintAfterPass(DerivedTy::PassName) ||
         (PrintChanged != ChangePrinter::None && ModifiedHIR))) {
      dbgs() << "*** IR Dump After " << DerivedPtr->name() << " ***\n";
      dbgs() << "Function: " << F.getName() << "\n";
      HIRF.print(false, dbgs());
    }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    return PA;
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
