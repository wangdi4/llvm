//===-------------- OCLVecClone.h - Class definition -*- C++
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
/// This file defines the VecClone pass class.
///
// ===--------------------------------------------------------------------=== //
#ifndef BACKEND_VECTORIZER_OCLVECCLONE_OCLVECCLONE_H
#define BACKEND_VECTORIZER_OCLVECCLONE_OCLVECCLONE_H

#include "OCLPassSupport.h"
#include "VecConfig.h"
#include "llvm/Transforms/Utils/Intel_VecClone.h"

using namespace llvm;

namespace intel {
class OCLVecCloneImpl : public VecCloneImpl {
private:
  // Configuration options
  const Intel::CPUId *CPUId;

  // OpenCL-related code transformation: 1. updates all the uses of TID calls
  // with TID + new induction variable. TIDs are updated in this way because
  // they are treated as linears. 2. hoists the TID call out of the loop and
  // updates the users. If we kept the get_global_id()/get_local_id() inside the
  // loop, then we will need a vector verison of it. Instead, we simply move it
  // outside of the loop and we generate TID+1,TID+2,TID+3 etc. Hoisting the TID
  // calls outside of the for-loop might create additional load/stores for some
  // kernels with barriers.
  void handleLanguageSpecifics(Function &F, PHINode *Phi, Function *Clone,
                               BasicBlock *EntryBlock) override;

  // Prepare OpenCL kernel for VecClone (emits vector-variant attributes).
  void languageSpecificInitializations(Module &M) override;

  /// The actions to take for the OpenCL builtin functions.
  enum class FnAction {
    MoveAndUpdateUses,       // Moves to entry block + update uses
    MoveAndUpdateUsesForDim, // Moves to entry block + update uses for a
    // specific dimension
    MoveOnly,            // Moves to entry block only
    AssertIfEncountered, // Assert false.
  };

public:
  OCLVecCloneImpl(const Intel::CPUId *CPUId);

  OCLVecCloneImpl();
};

class OCLVecClone : public ModulePass {
  OCLVecCloneImpl Impl;

protected:
  bool runOnModule(Module &M) override;

public:
  static char ID;

  OCLVecClone(const Intel::CPUId *CPUId);

  OCLVecClone();

  /// Returns the name of the pass
  llvm::StringRef getPassName() const override { return "OCLVecClone pass"; }
};

class OCLReqdSubGroupSize : public ModulePass {
private:
  bool runOnModule(Module &F) override;

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
