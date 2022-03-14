//===-- ValueLatticeUtils.cpp - Utils for solving lattices ------*- C++ -*-===//
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
  return F->hasLocalLinkage() &&
         !F->hasAddressTaken(nullptr, /*IgnoreCallbackUses=*/AllowCallbacks);
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
    // Currently all users of a global variable have to be non-volatile loads
    // or stores of the global type, and the global cannot be stored itself.
    if (auto *Store = dyn_cast<StoreInst>(U))
      return Store->getValueOperand() != GV && !Store->isVolatile() &&
             Store->getValueOperand()->getType() == GV->getValueType();
    if (auto *Load = dyn_cast<LoadInst>(U))
      return !Load->isVolatile() && Load->getType() == GV->getValueType();

    return false;
  });
}
