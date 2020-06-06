#if INTEL_COLLAB
//==-- GeneralUtils.cpp - General Utilities for VPO Analysis     -*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file provides a set of general analysis utilities for VPO
///
// ===--------------------------------------------------------------------=== //

#include "llvm/ADT/Triple.h"
#include "llvm/Support/Debug.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"

#define DEBUG_TYPE "VPOAnalysisUtils"

using namespace llvm;
using namespace llvm::vpo;

bool VPOAnalysisUtils::isCallOfName(Instruction *I, StringRef Name) {
  CallInst *Call = dyn_cast<CallInst>(I);
  if (Call) {
    Function *Func = Call->getCalledFunction();
    if (!Func)
      return false; // Indirect function call.

    StringRef FuncName = Func->getName();
    if (FuncName.equals(Name))
      return true;
  }
  return false;
}

AllocaInst *VPOAnalysisUtils::findAllocaInst(Value *V) {
  if (AllocaInst *Alloca = dyn_cast<AllocaInst>(V))
    return Alloca;

  if (CastInst *Cast = dyn_cast<CastInst>(V)) {
    // LLVM_DEBUG(dbgs() << "Found CastInst: " << *Cast << "\n");
    return findAllocaInst(Cast->getOperand(0));
  }
  return nullptr; // not found
}

/// \brief Returns true if we are compiling for SPIRV target.
bool VPOAnalysisUtils::isTargetSPIRV(Module *M) {
  Triple TargetTriple(M->getTargetTriple());
  return TargetTriple.getArch() == Triple::ArchType::spir ||
         TargetTriple.getArch() == Triple::ArchType::spir64;
}

#endif // INTEL_COLLAB
