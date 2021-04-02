//=------------------------ SGSizeAnalysis.cpp -*- C++ -*--------------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "SGSizeAnalysis.h"

#include "MetadataAPI.h"
#include "OCLPassSupport.h"

#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/CallGraph.h"

#define DEBUG_TYPE "sg-size-analysis"

using namespace Intel::MetadataAPI;

namespace intel {

char SGSizeAnalysis::ID = 0;

OCL_INITIALIZE_PASS(SGSizeAnalysis, DEBUG_TYPE,
                    "Analyze the emulation size for functions", false, true)

bool SGSizeAnalysis::runOnModule(Module &M) {
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
      LLVM_DEBUG(dbgs() << F->getName() << " Emu Size: " << EmuSize << "\n");
      FuncToEmuSizes[F].insert(EmuSize);
      It++;
    }
  }
  return false;
}

void SGSizeAnalysis::print(raw_ostream &OS, const Module * /*M*/) const {
  for (auto &Item : FuncToEmuSizes) {
    const Function *F = Item.first;
    const std::set<unsigned> &SGEmuSizes = Item.second;
    OS << "Function<" << F->getName() << "> Emu Sizes: ";
    for (auto EmuSize : SGEmuSizes)
      OS << EmuSize << " ";
    OS << "\n";
  }
}

} // namespace intel
