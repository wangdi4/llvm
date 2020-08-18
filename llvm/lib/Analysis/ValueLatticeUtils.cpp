//===-- ValueLatticeUtils.cpp - Utils for solving lattices ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements common functions useful for performing data-flow
// analyses that propagate values across function boundaries.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/ValueLatticeUtils.h"
#include "llvm/IR/AbstractCallSite.h" // INTEL
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
using namespace llvm;

#if INTEL_CUSTOMIZATION
bool llvm::canTrackArgumentsInterprocedurally(Function *F,
                                              bool AllowCallbacks) {
  if (!F->hasLocalLinkage())
    return false;

  // Check if function has any uses other than block addresses, direct calls or
  // callback calls (if allowed). If there are such uses function is considered
  // address-taken and therefore it cannot be used for interprocedural argument
  // tracking.
  for (const Use &U : F->uses()) {
    const User *FU = U.getUser();
    if (isa<BlockAddress>(FU))
      continue;

    auto *Call = dyn_cast<CallBase>(FU);
    if (Call && Call->isCallee(&U))
      continue;

    if (!AllowCallbacks)
      return false;

    AbstractCallSite ACS(&U);
    if (!ACS || !ACS.isCallbackCall() || !ACS.isCallee(&U))
      return false;
  }
  return true;
#endif // INTEL_CUSTOMIZATION
}

bool llvm::canTrackReturnsInterprocedurally(Function *F) {
  return F->hasExactDefinition() && !F->hasFnAttribute(Attribute::Naked);
}

bool llvm::canTrackGlobalVariableInterprocedurally(GlobalVariable *GV) {
  if (GV->isConstant() || !GV->hasLocalLinkage() ||
      !GV->hasDefinitiveInitializer())
    return false;
  return all_of(GV->users(), [&](User *U) {
    // Currently all users of a global variable have to be none-volatile loads
    // or stores and the global cannot be stored itself.
    if (auto *Store = dyn_cast<StoreInst>(U))
      return Store->getValueOperand() != GV && !Store->isVolatile();
    if (auto *Load = dyn_cast<LoadInst>(U))
      return !Load->isVolatile();

    return false;
  });
}
