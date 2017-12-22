//==-- GeneralUtils.cpp - General Utilities for VPO Analysis     -*- C++ -*-==//
//
// Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file provides a set of general analysis utilities for VPO
///
// ===--------------------------------------------------------------------=== //

#include "llvm/Support/Debug.h"
#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"

#define DEBUG_TYPE "VPOAnalysisUtils"

using namespace llvm;
using namespace llvm::vpo;

bool VPOAnalysisUtils::isCallOfName(Instruction *I, StringRef Name) {
  CallInst *Call = dyn_cast<CallInst>(I);
  if (Call) {
    StringRef FuncName = Call->getCalledFunction()->getName();
    if (FuncName.equals(Name))
      return true;
  }
  return false;
}

AllocaInst *VPOAnalysisUtils::findAllocaInst(Value *V) {
  if (AllocaInst *Alloca = dyn_cast<AllocaInst>(V))
    return Alloca;

  if (CastInst *Cast = dyn_cast<CastInst>(V)) {
    // DEBUG(dbgs() << "Found CastInst: " << *Cast << "\n");
    return findAllocaInst(Cast->getOperand(0));
  }
  return nullptr; // not found
}

/// \brief \returns true if the function has the string attribute
/// "may-have-openmp-directive" set to "true"
bool VPOAnalysisUtils::mayHaveOpenmpDirective(Function &F) {
  return F.getFnAttribute("may-have-openmp-directive").getValueAsString()
                                                                  == "true";
}

/// \brief \returns !mayHaveOpenmpDirective(F). This is mainly used in
/// passes required by OpenMP that would otherwise be skipped at -O0.
bool VPOAnalysisUtils::skipFunctionForOpenmp(Function &F) {
  return !mayHaveOpenmpDirective(F);
}
