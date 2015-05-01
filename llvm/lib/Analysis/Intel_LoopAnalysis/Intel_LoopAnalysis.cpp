//===-- Intel_LoopAnalysis.cpp --------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/InitializePasses.h"
#include "llvm/PassRegistry.h"

using namespace llvm;

void llvm::initializeIntel_LoopAnalysis(PassRegistry &Registry) {
  initializeRegionIdentificationPass(Registry);
  initializeHIRCreationPass(Registry);
  initializeHIRCleanupPass(Registry);
  initializeLoopFormationPass(Registry);
  initializeHIRParserPass(Registry);
  initializeSymbaseAssignmentPass(Registry);
  initializeDDAnalysisPass(Registry);
}
