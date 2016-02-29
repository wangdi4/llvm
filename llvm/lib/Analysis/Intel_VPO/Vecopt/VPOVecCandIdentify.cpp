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

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOVecCandIdentify.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/Passes.h"

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvr.h"

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

//
// Use with the WRNVisitor class (in WRegionUtils.h) to walk the WRGraph
// (DFS) and put WRNVecLoopNodes in VecCandidates.
// TODO: Vec owner should review this and modify as needed
// 
class VecNodeVisitor {
public:
  IdentifyVectorCandidates::VecCandidatesTy &VecCandidates;
  VecNodeVisitor(IdentifyVectorCandidates::VecCandidatesTy &VC) : VecCandidates(VC) {}
  void preVisit(WRegionNode* W)  {} 
  void postVisit(WRegionNode* W) { 
    if (WRNVecLoopNode *WRN = dyn_cast<WRNVecLoopNode>(W)) {
      VectorCandidate *VC = new VectorCandidate(WRN);
      VecCandidates.push_back(VC);
    }
  }
  bool quitVisit(WRegionNode* W) { return false; }
};

void IdentifyVectorCandidates::identifyExplicitCandidates()
{
  //DEBUG(dbgs() << "\nENG: Idenitfy Vector Candidates\n");
  VecNodeVisitor Visitor(VecCandidates);
  WRegionUtils::forwardVisit(Visitor, WR->getWRGraph());
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
