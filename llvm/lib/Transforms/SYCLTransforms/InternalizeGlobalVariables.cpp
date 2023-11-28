//===--- InternalizeGlobalVariables.cpp -------------------------*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation
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

#include "llvm/Transforms/SYCLTransforms/InternalizeGlobalVariables.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ImplicitArgsUtils.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-internalize-global-variables"

PreservedAnalyses
InternalizeGlobalVariablesPass::run(Module &M, ModuleAnalysisManager &AM) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

bool InternalizeGlobalVariablesPass::runImpl(Module &M) {
  bool Changed = false;

  for (auto &GVar : M.globals()) {
    if (GVar.hasInternalLinkage())
      continue;
    // According to llvm/LangRef, unreferenced globals of common, weak and
    // weak_odr linkage may not be discarded.
    unsigned AS = GVar.getAddressSpace();
    bool MayNotDiscardLinkage =
        (CompilationUtils::ADDRESS_SPACE_GLOBAL == AS ||
         CompilationUtils::ADDRESS_SPACE_CONSTANT == AS) &&
        (GVar.hasCommonLinkage() || GVar.hasExternalLinkage() ||
         GVar.hasWeakLinkage() || GVar.hasWeakODRLinkage());
    if (MayNotDiscardLinkage ||
        (GVar.hasName() && GVar.getName().startswith("llvm.")))
      continue;
    GVar.setLinkage(GlobalValue::InternalLinkage);
    Changed = true;
  }
  return Changed;
}
