//===- LegacyPassManager.h - Legacy Container for Passes --------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the legacy PassManager class.  This class is used to hold,
// maintain, and optimize execution of Passes.  The PassManager class ensures
// that analysis results are available before a pass runs, and that Pass's are
// destroyed when the PassManager is destroyed.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_LEGACYPASSMANAGER_H
#define LLVM_IR_LEGACYPASSMANAGER_H

#include "llvm/Pass.h"
#include "llvm/Support/CBindingWrapping.h"

namespace llvm {

class Function;
class Pass;
class Module;

namespace legacy {

// Whether or not -debug-pass has been specified. For use to check if it's
// specified alongside the new PM.
#if !INTEL_PRODUCT_RELEASE
bool debugPassSpecified();
#endif // !INTEL_PRODUCT_RELEASE

class PassManagerImpl;
class FunctionPassManagerImpl;

/// PassManagerBase - An abstract interface to allow code to add passes to
/// a pass manager without having to hard-code what kind of pass manager
/// it is.
class PassManagerBase {
public:
  virtual ~PassManagerBase();

  /// Add a pass to the queue of passes to run.  This passes ownership of
  /// the Pass to the PassManager.  When the PassManager is destroyed, the pass
  /// will be destroyed as well, so there is no need to delete the pass.  This
  /// may even destroy the pass right away if it is found to be redundant. This
  /// implies that all passes MUST be allocated with 'new'.
  virtual void add(Pass *P) = 0;
};

#if INTEL_CUSTOMIZATION
// Helper wrapper around normal pass manager that "limits" the pass using the
// \p Limiter. In cases when it's statically known that the pass must or must
// not be run it does that as well using \p ForceSkip and \p ForceRun state.\
//
// Part of "dynamic loopopt" implementation.
class LoopOptLimitingPassManager : public PassManagerBase {
  PassManagerBase &UnderlyingPM;
  LoopOptLimiter Limiter;
  // Under some conditions loopopt is always disabled (e.g. O1) without the need
  // to check function attributes.
  bool ForceSkip;
  // Similar to above used for NoLoopOpt passes when it's statically known that
  // LoopOpt is disabled (e.g. O1).
  bool ForceRun;

public:
  LoopOptLimitingPassManager(PassManagerBase &UnderlyingPM,
                             LoopOptLimiter Limiter, bool ForceSkip = false,
                             bool ForceRun = false)
      : UnderlyingPM(UnderlyingPM), Limiter(Limiter), ForceSkip(ForceSkip),
        ForceRun(ForceRun) {}

  ~LoopOptLimitingPassManager() override = default;
  void add(Pass *P) override;
};
#endif // INTEL_CUSTOMIZATION

/// PassManager manages ModulePassManagers
class PassManager : public PassManagerBase {
public:

  PassManager();
  ~PassManager() override;

  void add(Pass *P) override;

  /// run - Execute all of the passes scheduled for execution.  Keep track of
  /// whether any of the passes modifies the module, and if so, return true.
  bool run(Module &M);

private:
  /// PassManagerImpl_New is the actual class. PassManager is just the
  /// wraper to publish simple pass manager interface
  PassManagerImpl *PM;
};

/// FunctionPassManager manages FunctionPasses.
class FunctionPassManager : public PassManagerBase {
public:
  /// FunctionPassManager ctor - This initializes the pass manager.  It needs,
  /// but does not take ownership of, the specified Module.
  explicit FunctionPassManager(Module *M);
  ~FunctionPassManager() override;

  void add(Pass *P) override;

  /// run - Execute all of the passes scheduled for execution.  Keep
  /// track of whether any of the passes modifies the function, and if
  /// so, return true.
  ///
  bool run(Function &F);

  /// doInitialization - Run all of the initializers for the function passes.
  ///
  bool doInitialization();

  /// doFinalization - Run all of the finalizers for the function passes.
  ///
  bool doFinalization();

private:
  FunctionPassManagerImpl *FPM;
  Module *M;
};

// Check if Function or a Loop pass \P should be run on the function \F based
// on its loopopt pipeline limitation and log the decision according -debug-pass
// verbosity level.
bool doesLoopOptPipelineAllowToRunWithDebug(Pass *P, Function &F);

} // End legacy namespace

// Create wrappers for C Binding types (see CBindingWrapping.h).
DEFINE_STDCXX_CONVERSION_FUNCTIONS(legacy::PassManagerBase, LLVMPassManagerRef)

} // End llvm namespace

#endif
