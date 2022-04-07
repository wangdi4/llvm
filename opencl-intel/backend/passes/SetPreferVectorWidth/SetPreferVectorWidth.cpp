// ===------- -- SetPreferVectorWidth.cpp -------------------*- C++ -*----=== //
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// Set 'prefer-vector-width' function attribute based on CPU architecture.
///
// ===--------------------------------------------------------------------=== //

#include "SetPreferVectorWidth.h"
#include "OCLPassSupport.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

static cl::opt<unsigned> ForcedVecWidth("force-prefer-vector-width",
                                        cl::init(0), cl::Hidden);

extern "C"
ModulePass *createSetPreferVectorWidthPass(const CPUIDTy *CPUID) {
  return new intel::SetPreferVectorWidth(CPUID);
}

const char *PreferVecWidth = "prefer-vector-width";

static bool setPreferVectorWidth(Module &M, const CPUIDTy *CPUID) {
  bool Changed = false;
  unsigned VecWidth;
  if (ForcedVecWidth)
    VecWidth = ForcedVecWidth;
  else if (CPUID->HasAVX512Core())
    VecWidth = 512;
  else if (CPUID->HasAVX2())
    VecWidth = 256;
  else
    VecWidth = 128;

  for (Function &F : M) {
    if (F.isIntrinsic())
      continue;
    if (F.hasFnAttribute(PreferVecWidth))
      continue;
    F.addFnAttr(PreferVecWidth, utostr(VecWidth));
    Changed = true;
  }
  return Changed;
}

namespace intel {

char SetPreferVectorWidth::ID = 0;

OCL_INITIALIZE_PASS(
    SetPreferVectorWidth, "set-prefer-vector-width",
    "Set 'prefer-vector-width' function attribute based on CPU architecture",
    false, false)

SetPreferVectorWidth::SetPreferVectorWidth(const ::CPUIDTy *CPUID)
  : ModulePass(ID), CPUID(CPUID) {
  initializeSetPreferVectorWidthPass(*PassRegistry::getPassRegistry());
}

bool SetPreferVectorWidth::runOnModule(Module &M) {
  return setPreferVectorWidth(M, CPUID);
}

} // namespace intel
