//=== Intel_InferAddressSpacesUtils.cpp - Utilities for addrspace----------===//
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides utilities to infer the address space.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/Intel_InferAddressSpacesUtils.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "infer-address-spaces"

using namespace llvm;

// The utility modifies types of variables, so that the member/element
// pointers are transformed from flat address space pointers to
// pointers to specific address space.
//
// The method is quite simple currently and only handles global variables
// of pointer type.
bool llvm::InferAddrSpacesForGlobals(unsigned FlatAddrSpace, Module &M) {
  SmallVector<GlobalVariable *, 16> GlobalsToDelete;

  for (auto &GO : M.globals()) {
    auto *GV = dyn_cast<GlobalVariable>(&GO);
    if (!GV)
      continue;

    LLVM_DEBUG(dbgs() << "Analyzing global variable: " << *GV << "\n");

    if (GV->getLinkage() != GlobalValue::InternalLinkage ||
        GV->isExternallyInitialized() ||
        (GV->hasInitializer() && !GV->getInitializer()->isZeroValue()))
      continue;

    auto *ValTy = dyn_cast<PointerType>(GV->getValueType());
    if (!ValTy)
      continue;
#if !ENABLE_OPAQUEPOINTER
    if (isa<PointerType>(ValTy->getPointerElementType()))
      // TODO: for now analyze only pointer to non-pointer globals.
      // TODO: OPAQUEPOINTER: remove this check
      continue;
#endif // !ENABLE_OPAQUEPOINTER

    if (ValTy->getAddressSpace() != FlatAddrSpace)
      continue;

    // Get rid of dead constant users to avoid analysing them.
    GV->removeDeadConstantUsers();

    SmallVector<StoreInst *, 8> StoreInstructions;
    SmallVector<LoadInst *, 8> LoadInstructions;
    bool HasUnknownUse = false;
    unsigned ProcessedUsersNum = 0;

    LLVM_DEBUG(dbgs() << "Analyzing users:\n");

    if (GV->users().empty()) {
      LLVM_DEBUG(dbgs() << "Global has no uses.\n");
      continue;
    }

    for (auto *U : GV->users()) {
      LLVM_DEBUG(dbgs() << "User: " << *U << "\n");

      if (auto *SI = dyn_cast<StoreInst>(U)) {
        if (GV == SI->getPointerOperand())
          StoreInstructions.push_back(SI);
        else
          HasUnknownUse = true;
      } else if (auto *LI = dyn_cast<LoadInst>(U))
        if (GV == LI->getPointerOperand())
          LoadInstructions.push_back(LI);
        else
          HasUnknownUse = true;
      else
        HasUnknownUse = true;

      if (HasUnknownUse) {
        LLVM_DEBUG(dbgs() << "Skipping the global due to unknown use.\n");
        break;
      }

      ++ProcessedUsersNum;
    }

    if (HasUnknownUse ||
        // It is possible that the global variable is used
        // both as a value and a pointer operand of the same
        // store instruction (e.g. with additional bit/addrspace-casts).
        // We do not want to deal with such cases, so check that
        // the number of users matches with the number of uses.
        !GV->hasNUses(ProcessedUsersNum))
      continue;

    // Look for store instructions storing a pointer to the global
    // variable. If all pointers stored to the global variable
    // are of the same specific non-flat address space,
    // then we can change the type of the global variable.
    unsigned NewAS = FlatAddrSpace;

    for (auto *SI : StoreInstructions) {
      auto *StoreVal = SI->getValueOperand();
      auto *RootStoreVal = StoreVal->stripPointerCasts();
      auto *RootStoreValTy = dyn_cast<PointerType>(RootStoreVal->getType());
      if (!RootStoreValTy) {
        NewAS = FlatAddrSpace;
        break;
      }

      unsigned AS = RootStoreValTy->getAddressSpace();
      if (NewAS == FlatAddrSpace) {
        if (AS == FlatAddrSpace)
          // The program stores a flat address space pointer
          // into the global variable, so we cannot deduce any
          // specific address space.
          break;

        NewAS = AS;
      } else if (NewAS != AS) {
        NewAS = FlatAddrSpace;
        break;
      }
    }

    if (NewAS == FlatAddrSpace) {
      LLVM_DEBUG(dbgs() <<
                 "Was not able to find new non-flat address space.\n");
      continue;
    }

    // The optimization is possible.
    auto *NewValType = PointerType::getWithSamePointeeType(ValTy, NewAS);

    // Create new global variable.
    Constant *Initializer = nullptr;
    if (GV->hasInitializer()) {
      assert(GV->getInitializer()->isZeroValue() &&
             "Only zero-initializers are supported.");
      Initializer = Constant::getNullValue(NewValType);
    }
    auto *NewGV = new GlobalVariable(
        M, NewValType, GV->isConstant(), GV->getLinkage(),
        Initializer, "", GV, GV->getThreadLocalMode(),
        NewAS, GV->isExternallyInitialized());
    NewGV->copyAttributesFrom(GV);
    NewGV->takeName(GV);

    // Transform the store instructions.
    for (auto *SI : StoreInstructions) {
      IRBuilder<> Builder(SI);

      auto *StoreVal = SI->getValueOperand();
      auto *StoreValTy = dyn_cast<PointerType>(StoreVal->getType());
      assert(StoreValTy && "Invalid store value type.");
      // Cast the stored value to the new address space.
      // This should be legal, because the stored value is based
      // on a pointer that has the NewAS address space (note that
      // we used stripPointerCasts() above to find the initial
      // pointer).
      auto *NewStoreVal = Builder.CreateAddrSpaceCast(
          StoreVal,
          PointerType::getWithSamePointeeType(StoreValTy, NewAS));
      SI->replaceUsesOfWith(StoreVal, NewStoreVal);
      SI->replaceUsesOfWith(GV, NewGV);
    }
    // Transform the load instructions.
    for (auto *LI : LoadInstructions) {
      auto *LoadValTy = dyn_cast<PointerType>(LI->getType());
      assert(LoadValTy && "Invalid load type.\n");

      // Clone the load instruction.
      auto *NewLI = LI->clone();
      NewLI->replaceUsesOfWith(GV, NewGV);
      NewLI->mutateType(
          PointerType::getWithSamePointeeType(LoadValTy, NewAS));
      NewLI->insertBefore(LI);
      // Cast the new load value to the original flat address space.
      // We could have called InferAddrSpaces() again to optimize
      // this new IR, but this seems to be unnecessary, since
      // the device compilers are able to optimize it.
      IRBuilder<> Builder(LI);
      auto *NewLoadVal = Builder.CreateAddrSpaceCast(NewLI, LoadValTy);
      LI->replaceAllUsesWith(NewLoadVal);
      LI->eraseFromParent();
    }

    // Do not delete the global variable not to invalidate
    // the iterator.
    GlobalsToDelete.push_back(GV);
  }

  for (auto *GV : GlobalsToDelete)
    GV->eraseFromParent();

  return !GlobalsToDelete.empty();
}
