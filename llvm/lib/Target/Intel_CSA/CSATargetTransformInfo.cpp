//===-- CSATargetTransformInfo.cpp - CSA specific TTI pass ----------------===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements a TargetTransformInfo analysis pass specific to the
/// CSA target machine. It uses the target's detailed information to provide
/// more precise answers to certain TTI queries, while letting the target
/// independent and default TTI implementations handle the rest.
///
//===----------------------------------------------------------------------===//

#include "CSATargetTransformInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/BasicTTIImpl.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "csatti"

//===----------------------------------------------------------------------===//
//
// CSA cost model.
//
//===----------------------------------------------------------------------===//

bool CSATTIImpl::areInlineCompatible(const Function *Caller,
                                     const Function *Callee) const {
  const TargetMachine &TM = getTLI()->getTargetMachine();

  // Work this as a subsetting of subtarget features.
  const FeatureBitset &CallerBits =
      TM.getSubtargetImpl(*Caller)->getFeatureBits();
  const FeatureBitset &CalleeBits =
      TM.getSubtargetImpl(*Callee)->getFeatureBits();

  // FIXME: This is likely too limiting as it will include subtarget features
  // that we might not care about for inlining, but it is conservatively
  // correct.
  return (CallerBits & CalleeBits) == CalleeBits;
}

