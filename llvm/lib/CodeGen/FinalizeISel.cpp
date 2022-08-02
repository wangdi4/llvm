//===-- llvm/CodeGen/FinalizeISel.cpp ---------------------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
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
/// This pass expands Pseudo-instructions produced by ISel, fixes register
/// reservations and may do machine frame information adjustments.
/// The pseudo instructions are used to allow the expansion to contain control
/// flow, such as a conditional move implemented with a conditional branch and a
/// phi, or an atomic operation implemented with a loop.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_MARKERCOUNT
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/Support/CommandLine.h"
#endif // INTEL_FEATURE_MARKERCOUNT
#endif // INTEL_CUSTOMIZATION
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/InitializePasses.h"

using namespace llvm;

#define DEBUG_TYPE "finalize-isel"

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_MARKERCOUNT
cl::opt<bool>
    MarkPrologEpilog("mark-prolog-epilog",
                     cl::desc("Emit marker count at prolog and epilog"),
                     cl::init(false));
cl::opt<bool> MarkLoopHeader("mark-loop-header",
                             cl::desc("Emit marker count at loop header"),
                             cl::init(false));
#endif // INTEL_FEATURE_MARKERCOUNT
#endif // INTEL_CUSTOMIZATION

namespace {
  class FinalizeISel : public MachineFunctionPass {
  public:
    static char ID; // Pass identification, replacement for typeid
    FinalizeISel() : MachineFunctionPass(ID) {}

  private:
    bool runOnMachineFunction(MachineFunction &MF) override;

    void getAnalysisUsage(AnalysisUsage &AU) const override {
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_MARKERCOUNT
      if (MarkLoopHeader) {
        AU.addRequired<MachineLoopInfo>();
        AU.addPreserved<MachineLoopInfo>();
      }
#endif // INTEL_FEATURE_MARKERCOUNT
#endif // INTEL_CUSTOMIZATION
      MachineFunctionPass::getAnalysisUsage(AU);
    }
  };
} // end anonymous namespace

char FinalizeISel::ID = 0;
char &llvm::FinalizeISelID = FinalizeISel::ID;
INITIALIZE_PASS(FinalizeISel, DEBUG_TYPE,
                "Finalize ISel and expand pseudo-instructions", false, false)

bool FinalizeISel::runOnMachineFunction(MachineFunction &MF) {
  bool Changed = false;
  const TargetLowering *TLI = MF.getSubtarget().getTargetLowering();

  // Iterate through each instruction in the function, looking for pseudos.
  for (MachineFunction::iterator I = MF.begin(), E = MF.end(); I != E; ++I) {
    MachineBasicBlock *MBB = &*I;
    for (MachineBasicBlock::iterator MBBI = MBB->begin(), MBBE = MBB->end();
         MBBI != MBBE; ) {
      MachineInstr &MI = *MBBI++;

      // If MI is a pseudo, expand it.
      if (MI.usesCustomInsertionHook()) {
        Changed = true;
        MachineBasicBlock *NewMBB = TLI->EmitInstrWithCustomInserter(MI, MBB);
        // The expansion may involve new basic blocks.
        if (NewMBB != MBB) {
          MBB = NewMBB;
          I = NewMBB->getIterator();
          MBBI = NewMBB->begin();
          MBBE = NewMBB->end();
        }
      }
    }
  }

  TLI->finalizeLowering(MF);

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_MARKERCOUNT
  // Insert marker count at prolog, epilog and loop header.
  if (MarkLoopHeader || MarkPrologEpilog) {
    for (MachineBasicBlock &MBB : MF) {
      if (MarkLoopHeader) {
        MachineLoopInfo *MLI = &getAnalysis<MachineLoopInfo>();
        if (MLI->isLoopHeader(&MBB))
          MBB.getMarkerCount().addKinds(MCK::LoopHeader);
      }
      if (MBB.isEntryBlock() && MarkPrologEpilog)
        MBB.getMarkerCount().addKinds(MCK::Prolog);
      if (MBB.isReturnBlock() && MarkPrologEpilog)
        MBB.getMarkerCount().addKinds(MCK::Epilog);
    }
  }
#endif // INTEL_FEATURE_MARKERCOUNT
#endif // INTEL_CUSTOMIZATION

  return Changed;
}
