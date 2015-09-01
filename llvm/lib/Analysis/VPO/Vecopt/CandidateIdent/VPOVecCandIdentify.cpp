//===----------------------------------------------------------------------===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOVecCandIdentify.cpp -- Implements the vector loop candidate
//   identification pass.
//
//   This is a wrapper pass for WRN Info Analysis.  This pass builds a
//   a collection of candidate loops for vectorization.  These candidate loops
//   are identified from the incoming LLVM IR by: 
//     (1) WRN Info Analysis - For explicit vectorization
//     (2) Computed locally - For auto vectorization. (Not implemented yet)
//   
//     # 2 is a temporary solution until WRN Info Analysis can idenitfy
//     candidate loops for auto-vectorization. At that time we can 
//     eliminate this pass.
//
//   The collection of candidate loops for vectorization, that were identified 
//   by this pass, is stored in a vector container which is used by AVRGenerate
//   to build the Abstract Layer with AVRs. 
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/VPO/Vecopt/CandidateIdent/VPOVecCandIdentify.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/VPO/Vecopt/Passes.h"

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvr.h"

#define DEBUG_TYPE "identify-vector-candidates"

using namespace llvm;
using namespace llvm::vpo;

INITIALIZE_PASS_BEGIN(IdentifyVectorCandidates, "avr-id-cand",
  "AVR Identify Candidates", false, true)
INITIALIZE_PASS_DEPENDENCY(WRegionInfo)
INITIALIZE_PASS_END(IdentifyVectorCandidates, "avr-id-cand",
  "AVR Identify Candidates", false, true)

char IdentifyVectorCandidates::ID = 0;

FunctionPass *llvm::createIdentifyVectorCandidatesPass()
 { return new IdentifyVectorCandidates(); }

IdentifyVectorCandidates::IdentifyVectorCandidates() : FunctionPass(ID) {
 llvm::initializeIdentifyVectorCandidatesPass(*PassRegistry::getPassRegistry());
}

void IdentifyVectorCandidates::getAnalysisUsage(AnalysisUsage &AU) const
{
  AU.setPreservesAll();
  AU.addRequiredTransitive<WRegionInfo>();
}

bool IdentifyVectorCandidates::runOnFunction(Function &F)
{
  WR = &getAnalysis<WRegionInfo>();

  if (!WR->WRGraphIsEmpty()) {
    identifyVectorCandidates();
  }
  else {
    DEBUG(dbgs() << "\nNo WRN Candidates for Vectorization\n");
  }

  return false; // Does not modify IR
}

void IdentifyVectorCandidates::identifyVectorCandidates()
{
  identifyExplicitCandidates();

  if (AutoVectorizationEnabled) {
    identifyAutoCandidates();
  }
}

void IdentifyVectorCandidates::identifyExplicitCandidates()
{
  //DEBUG(dbgs() << "\nENG: Idenitfy Vector Candidates\n");
  // Walk the the top-level WRN graph nodes.
  // Replace with WRN vistor once its implemented
  for (auto I = WR->begin(), E = WR->end(); I != E;  ++I) {
 
    if (WRNVecLoopNode *WRN = dyn_cast<WRNVecLoopNode>(I)) {
      VectorCandidate *VC = new VectorCandidate(WRN);
      VecCandidates.push_back(VC);
    }
  }
}

void IdentifyVectorCandidates::releaseMemory() {
  VecCandidates.clear();
}

void IdentifyVectorCandidates::identifyAutoCandidates()
{
  // TODO: Implement Auto-vectorization loop candidate
  // logic here
}

void IdentifyVectorCandidates:: print() 
{
  // TODO: Implement print
}

void IdentifyVectorCandidates::verifyAnalysis() const
{
  // TODO: Implement analysis verification
}
