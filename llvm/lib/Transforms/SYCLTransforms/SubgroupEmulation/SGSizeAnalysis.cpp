//=---- SGSizeAnalysis.cpp - Analyze emulation size for functions- C++ -*----=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/SYCLTransforms/SubgroupEmulation/SGSizeAnalysis.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"

#define DEBUG_TYPE "sycl-kernel-sg-size-analysis"

using namespace llvm;
using namespace SYCLKernelMetadataAPI;

// Provide a definition for the static class member used to identify passes.
AnalysisKey SGSizeAnalysisPass::Key;

SGSizeInfo SGSizeAnalysisPass::run(Module &M, ModuleAnalysisManager &AM) {
  SGSizeInfo SSIResult;
  SSIResult.analyzeModule(M);
  return SSIResult;
}

PreservedAnalyses SGSizeAnalysisPrinter::run(Module &M,
                                             ModuleAnalysisManager &AM) {
  AM.getResult<SGSizeAnalysisPass>(M).print(OS);
  return PreservedAnalyses::all();
}

void SGSizeInfo::analyzeModule(Module &M) {
  CallGraph CG(M);

  for (auto *Kernel : KernelList(&M)) {
    auto KIMD = KernelInternalMetadataAPI(Kernel);
    // Emulation only works when kernel has subgroup and can't be vectorized.
    // In such cases, KernelHasSubgroups is true and there is a metadata
    // SubgroupEmuSize which indicates the required / auto sub-group size.
    if (!(KIMD.KernelHasSubgroups.hasValue() && KIMD.KernelHasSubgroups.get()))
      continue;
    if (!KIMD.SubgroupEmuSize.hasValue())
      continue;

    unsigned EmuSize = KIMD.SubgroupEmuSize.get();
    FuncToEmuSizes[Kernel].insert(EmuSize);

    CallGraphNode *Node = CG[Kernel];
    for (auto It = df_begin(Node), End = df_end(Node); It != End;) {
      Function *F = It->getFunction();
      if (!F || F->isDeclaration()) {
        It = It.skipChildren();
        continue;
      }
      LLVM_DEBUG(dbgs() << F->getName() << " (" << F
                        << ") Emu Size: " << EmuSize << "\n");
      FuncToEmuSizes[F].insert(EmuSize);
      It++;
    }
  }
}

void SGSizeInfo::print(raw_ostream &OS) const {
  for (auto &Item : FuncToEmuSizes) {
    const Function *F = Item.first;
    const std::set<unsigned> &SGEmuSizes = Item.second;
    OS << "Function<" << F->getName() << "> Emu Sizes: ";
    for (auto EmuSize : SGEmuSizes)
      OS << EmuSize << " ";
    OS << "\n";
  }
}
