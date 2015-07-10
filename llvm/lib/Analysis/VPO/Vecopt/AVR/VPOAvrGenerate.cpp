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
//   VPOAvrGenerate.cpp -- Implements the AVR Generation Pass
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrGenerate.h"
#include "llvm/Analysis/VPO/Vecopt/CandidateIdent/VPOVecCandIdentify.h"
#include "llvm/Analysis/VPO/Vecopt/Passes.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Dominators.h"

using namespace llvm;
using namespace llvm::vpo;

INITIALIZE_PASS_BEGIN(AVRGenerate, "avr-generate", "AVR Generate", false, true)
INITIALIZE_PASS_DEPENDENCY(IdentifyVectorCandidates)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(AVRGenerate, "avr-generate", "AVR Generate", false, true)

char AVRGenerate::ID = 0;

FunctionPass *llvm::createAVRGeneratePass() { return new AVRGenerate(); }

AVRGenerate::AVRGenerate() : FunctionPass(ID) {
 llvm::initializeAVRGeneratePass(*PassRegistry::getPassRegistry());
}

void AVRGenerate::getAnalysisUsage(AnalysisUsage &AU) const
{
  AU.setPreservesAll();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<IdentifyVectorCandidates>();

}

bool AVRGenerate::runOnFunction(Function &F)
{
  this->Func = &F;
  VC = &getAnalysis<IdentifyVectorCandidates>();
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();

  // Change this flag to true to test full AVR generation for incoming
  // function
  ScalarStressTest = false;

  create();

  return false;
}

void AVRGenerate::create()
{
  if (ScalarStressTest) {
     DEBUG(dbgs() << "\nAVR: Generating AVRs for whole function.\n");

    // Build complete AVR node representation for function in stress testing mode
    buildAvrForFunction();
  }
  else {
    DEBUG(dbgs() << "\nAVR: Generating AVRs for vector candidates.\n");

    // Build AVR node representation for incoming vector candidates
    buildAvrForVectorCandidates();
  }
}


AVR *AVRGenerate::preorderTravAvrBuild(BasicBlock *BB, AVR* InsertionPos)
{
  
  auto *DomNode = DT->getNode(BB);

  // Build AVR node sequence for current basic block
  AVR *LastAvrPos = generateAvrInstSeqForBB(BB, InsertionPos);

  // Traverse dominator children
  for (auto I = DomNode->begin(), E = DomNode->end(); I != E; ++I) {

    // TODO: Properly handle split/switch

    BasicBlock *DomChildBB = (*I)->getBlock();
    LastAvrPos = preorderTravAvrBuild(DomChildBB, LastAvrPos);
  }

  return LastAvrPos;
}


void AVRGenerate::buildAvrForVectorCandidates()
{
  // Temporary implemtation uses vector of Vector Candidate objects to
  // build AVRs.  Will move away from usage of this object and use
  // vistor for WRN graph when available.

  for (auto I = VC->begin(), E = VC->end(); I != E; ++I) {

    // Each VectorCandidate / WRN Regions specifies a loop
    // for which we build an AVR representation. 
    const LoopInfo *LpInfo = (*I)->getLoopInfo();
    AvrLoop = AVRUtils::createAVRLoop(LpInfo, true); 

    preorderTravAvrBuild((*I)->getEntryBBlock(), AvrLoop); 
    AVRList.push_back(AvrLoop);
  }
}


AVR* AVRGenerate::generateAvrInstSeqForBB(BasicBlock *BB, AVR *InsertionPos)
{
  AVR *NewNode = AVRUtils::createAVRLabel(BB);
 
  // First BB of loop, function, split is inserted as first child
  if (isa<AVRLoop>(InsertionPos) || isa<AVRFunction>(InsertionPos)) {
    AVRUtils::insertFirstChildAVR(InsertionPos, NewNode);
  }
  else{
    AVRUtils::insertAVRAfter(InsertionPos, NewNode);
  }

  InsertionPos = NewNode;

  for (auto I = BB->begin(), E = (BB->end()); I != E; ++I) {

    switch(I->getOpcode()) {
      case Instruction::Call:
        NewNode = AVRUtils::createAVRCall(I);
        break;
      case Instruction::PHI:
        NewNode = AVRUtils::createAVRPhi(I);
        break;
      case Instruction::Br:
        NewNode = AVRUtils::createAVRFBranch(I);
        break;
      //case Instruction::BackEdge:
        //break;
      //case Instruction::Entry:
        //break;
      case Instruction::Ret:
        NewNode = AVRUtils::createAVRReturn(I);
        break;
      case Instruction::ICmp:
      case Instruction::FCmp:
        NewNode = AVRUtils::createAVRIf(I);
        // TODO: Recursively build body
        //       This is not quite correct, need to properly set predication
        break;
      //case Instruction::Loop:
        //break;
      default:
        NewNode = AVRUtils::createAVRAssign(I);
    }

    AVRUtils::insertAVRAfter(InsertionPos, NewNode);
    InsertionPos = NewNode;
  } 

  // TODO: Generate BB terminator
 
  return InsertionPos;
}


// For explicit vectorization of loops and functions, the vectorizer
// should not generate AVRFunction nodes. Building AVR for function
// is for stress testing only.
void AVRGenerate::buildAvrForFunction()
{
  AVRFunction *FuncNode = AVRUtils::createAVRFunction(Func);
  buildBody(FuncNode);
  AVRList.push_back(FuncNode);
}

// This routine for building the body of a function is mainly for stress 
// testing and debugging purposes. Given a Function, this routine walks
// it's basic blocks and generates a 1:1 mapping of AVRs for each LLVM 
// instruction in each basic block.
//
void AVRGenerate::buildBody(AVRFunction *AVRFunc) {

  Function *Func = AVRFunc->getOrigFunction();
  AVR *LastAvr = AVRFunc;

  // Iterate over basic blocks.
  for (auto BBItr = Func->begin(), BBEnd = Func->end(); BBItr != BBEnd; ++BBItr) {

    LastAvr = generateAvrInstSeqForBB(BBItr, LastAvr);
  }
}

void AVRGenerate::print() {

  if (!AVRList.empty()) {
    DEBUG(dbgs() << "\nAVR LISTING BEGIN:\n");
    // TODO: Support proper printing
    AVR *ANode = &AVRList.back();
    ANode->dump();
    DEBUG(dbgs() << "AVR LISTING END\n\n\n");
  }
  else {
    DEBUG(dbgs() << "No AVRs Generated\n\n");
  }
}

bool AVRGenerate::codeGen() {

  if (!AVRList.empty()) {
    AVR *ANode = &AVRList.back();
    ANode->codeGen();
    return true;
  }

  return false;
}
