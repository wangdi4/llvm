//===--- AddTLSGlobals.cpp - AddTLSGlobals pass - C++ -* ------------------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/AddTLSGlobals.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/SYCLTransforms/ImplicitArgsAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/LocalBufferAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ImplicitArgsUtils.h"

using namespace llvm;

// Command line option for other passes to use TLS mode
bool EnableTLSGlobals;
static cl::opt<bool, true> OptEnableTLSGlobals(
    "sycl-kernel-enable-tls-globals", cl::desc("Enable TLS globals"),
    cl::location(EnableTLSGlobals), cl::init(false), cl::Hidden);

PreservedAnalyses AddTLSGlobalsPass::run(Module &M, ModuleAnalysisManager &AM) {
  ImplicitArgsInfo *IAInfo = &AM.getResult<ImplicitArgsAnalysis>(M);
  LocalBufferInfo *LBInfo = &AM.getResult<LocalBufferAnalysis>(M);

  bool NoBarrier =
      M.getFunction(CompilationUtils::nameSpecialBuffer()) == nullptr;
  bool NoLocalBuffer = llvm::all_of(M.functions(), [&](const Function &F) {
    return F.isDeclaration() ? true : !LBInfo->getDirectLocalsMap().count(&F);
  });

  // Create TLS globals
  for (unsigned I = 0; I < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++I) {
    if (I == ImplicitArgsUtils::IA_SLM_BUFFER && NoLocalBuffer)
      continue;
    if (I == ImplicitArgsUtils::IA_BARRIER_BUFFER && NoBarrier)
      continue;

    // TODO handle name conflicts
    assert(!M.getGlobalVariable(ImplicitArgsUtils::getArgName(I)));
    Type *ArgType = IAInfo->getArgType(I);
    GlobalVariable *GV = new GlobalVariable(
        M, ArgType, false, GlobalValue::LinkOnceODRLinkage,
        UndefValue::get(ArgType), ImplicitArgsUtils::getArgName(I), nullptr,
        GlobalValue::GeneralDynamicTLSModel);
    GV->setAlignment(M.getDataLayout().getPreferredAlign(GV));
  }

  PreservedAnalyses PA;
  PA.preserve<ImplicitArgsAnalysis>();
  PA.preserve<LocalBufferAnalysis>();
  return PA;
}
