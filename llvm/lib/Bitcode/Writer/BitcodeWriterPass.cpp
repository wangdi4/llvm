//===- BitcodeWriterPass.cpp - Bitcode writing pass -----------------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
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
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// BitcodeWriterPass implementation.
//
//===----------------------------------------------------------------------===//

#include "llvm/Bitcode/BitcodeWriterPass.h"
#include "llvm/Analysis/ModuleSummaryAnalysis.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
using namespace llvm;

PreservedAnalyses BitcodeWriterPass::run(Module &M, ModuleAnalysisManager &AM) {
  // RemoveDIs: there's no bitcode representation of the DPValue debug-info,
  // convert to dbg.values before writing out.
  bool IsNewDbgInfoFormat = M.IsNewDbgInfoFormat;
  if (IsNewDbgInfoFormat)
    M.convertFromNewDbgValues();

  const ModuleSummaryIndex *Index =
      EmitSummaryIndex ? &(AM.getResult<ModuleSummaryIndexAnalysis>(M))
                       : nullptr;
  WriteBitcodeToFile(M, OS, ShouldPreserveUseListOrder, Index, EmitModuleHash);

  if (IsNewDbgInfoFormat)
    M.convertToNewDbgValues();

  return PreservedAnalyses::all();
}

namespace {
  class WriteBitcodePass : public ModulePass {
    raw_ostream &OS; // raw_ostream to print on
    bool ShouldPreserveUseListOrder = false; // INTEL
    bool EmitSummaryIndex = false;           // INTEL
    bool EmitModuleHash = false;             // INTEL

  public:
    static char ID; // Pass identification, replacement for typeid
    WriteBitcodePass() : ModulePass(ID), OS(dbgs()) {
      initializeWriteBitcodePassPass(*PassRegistry::getPassRegistry());
    }

    explicit WriteBitcodePass(raw_ostream &o, bool ShouldPreserveUseListOrder,
                              bool EmitSummaryIndex, bool EmitModuleHash)
        : ModulePass(ID), OS(o),
          ShouldPreserveUseListOrder(ShouldPreserveUseListOrder),
          EmitSummaryIndex(EmitSummaryIndex), EmitModuleHash(EmitModuleHash) {
      initializeWriteBitcodePassPass(*PassRegistry::getPassRegistry());
    }

    StringRef getPassName() const override { return "Bitcode Writer"; }

    bool runOnModule(Module &M) override {
      const ModuleSummaryIndex *Index =
          EmitSummaryIndex
              ? &(getAnalysis<ModuleSummaryIndexWrapperPass>().getIndex())
              : nullptr;
      // RemoveDIs: there's no bitcode representation of the DPValue debug-info,
      // convert to dbg.values before writing out.
      bool IsNewDbgInfoFormat = M.IsNewDbgInfoFormat;
      if (IsNewDbgInfoFormat)
        M.convertFromNewDbgValues();

      WriteBitcodeToFile(M, OS, ShouldPreserveUseListOrder, Index,
                         EmitModuleHash);

      if (IsNewDbgInfoFormat)
        M.convertToNewDbgValues();
      return false;
    }
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
      if (EmitSummaryIndex)
        AU.addRequired<ModuleSummaryIndexWrapperPass>();
    }
  };
}

char WriteBitcodePass::ID = 0;
INITIALIZE_PASS_BEGIN(WriteBitcodePass, "write-bitcode", "Write Bitcode", false,
                      true)
INITIALIZE_PASS_DEPENDENCY(ModuleSummaryIndexWrapperPass)
INITIALIZE_PASS_END(WriteBitcodePass, "write-bitcode", "Write Bitcode", false,
                    true)

ModulePass *llvm::createBitcodeWriterPass(raw_ostream &Str,
                                          bool ShouldPreserveUseListOrder,
                                          bool EmitSummaryIndex, bool EmitModuleHash) {
  return new WriteBitcodePass(Str, ShouldPreserveUseListOrder,
                              EmitSummaryIndex, EmitModuleHash);
}

bool llvm::isBitcodeWriterPass(Pass *P) {
  return P->getPassID() == (llvm::AnalysisID)&WriteBitcodePass::ID;
}
