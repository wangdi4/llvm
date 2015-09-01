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

#include "llvm/Support/CommandLine.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Dominators.h"

#define DEBUG_TYPE "avr-generation"

using namespace llvm;
using namespace llvm::vpo;

INITIALIZE_PASS_BEGIN(AVRGenerate, "avr-generate", "AVR Generate", false, true)
INITIALIZE_PASS_DEPENDENCY(IdentifyVectorCandidates)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(AVRGenerate, "avr-generate", "AVR Generate", false, true)

char AVRGenerate::ID = 0;

static cl::opt<bool>AvrStressTest("avr-stress-test", cl::init(false),
  cl::desc("Construct full Avrs for stress testing"));


FunctionPass *llvm::createAVRGeneratePass() { return new AVRGenerate(); }

AVRGenerate::AVRGenerate() : FunctionPass(ID) {
 llvm::initializeAVRGeneratePass(*PassRegistry::getPassRegistry());

 setLLVMFunction(nullptr);
 setAvrFunction(nullptr);
 setAvrWrn(nullptr);
 setLoopInfo(nullptr);
 setStressTest(AvrStressTest);
 AVRList.clear();
}

void AVRGenerate::getAnalysisUsage(AnalysisUsage &AU) const
{
  AU.setPreservesAll();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<IdentifyVectorCandidates>();
}

bool AVRGenerate::runOnFunction(Function &F)
{
  VC = &getAnalysis<IdentifyVectorCandidates>();
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

  setLLVMFunction(&F);

  // Build the base Abstract Layer representation. 
  buildAbstractLayer();

  // Optimize the abstract layer representation by indentifying and marking loops.
  formAvrLoops();

  // Optimize the abstract layer represenation by identifying and marking
  // if/splits
  formAvrSplits();

  return false;
}

void AVRGenerate::buildAbstractLayer()
{
  if (ScalarStressTest) {
     DEBUG(dbgs() << "\nAVR: Generating AVRs for whole function.\n");

    // Build complete AVR node representation for function in stress testing mode
    buildAvrsForFunction();
  }
  else {
    DEBUG(dbgs() << "\nAVR: Generating AVRs for vector candidates.\n");

    // Build AVR node representation for incoming vector candidates
    buildAvrsForVectorCandidates();
  }
}


AVR *AVRGenerate::preorderTravAvrBuild(BasicBlock *BB, AVR *InsertionPos)
{
  assert(BB && InsertionPos && "Avr preorder traversal failed!");

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

void AVRGenerate::buildAvrsForVectorCandidates()
{
  // Temporary implemtation uses vector of Vector Candidate objects to
  // build AVRs.  Will move away from usage of this object and use
  // vistor for WRN graph when available.

  for (auto I = VC->begin(), E = VC->end(); I != E; ++I) {

    AvrWrn = AVRUtils::createAVRWrn((*I)->getWrnNode());

    preorderTravAvrBuild((*I)->getEntryBBlock(), AvrWrn);
    AVRList.push_back(AvrWrn);
  }
}



AVR* AVRGenerate::generateAvrInstSeqForBB(BasicBlock *BB, AVR *InsertionPos)
{
  AVR *NewNode = AVRUtils::createAVRLabel(BB);
 
  // First BB of loop, function, split is inserted as first child
  if (isa<AVRLoop>(InsertionPos) || isa<AVRFunction>(InsertionPos) ||
      isa<AVRWrn>(InsertionPos)) {
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
void AVRGenerate::buildAvrsForFunction()
{
  AvrFunction = AVRUtils::createAVRFunction(Func, LI);

  buildBody();

  // Add generated AVRs to AVR List.
  AVRList.push_back(AvrFunction);
}

// This routine for building the body of a function is mainly for stress 
// testing and debugging purposes. Given a Function, this routine walks
// it's basic blocks and generates a 1:1 mapping of AVRs for each LLVM 
// instruction in each basic block.
//
void AVRGenerate::buildBody() {

  AVR *LastAvr = getAvrFunction();

  // Iterate over basic blocks.
  for (auto BBItr = Func->begin(), BBEnd = Func->end(); BBItr != BBEnd; ++BBItr) {

    LastAvr = generateAvrInstSeqForBB(BBItr, LastAvr);
  }
}

void AVRGenerate::formAvrLoops() {

  if (!isAvrGenerateListEmpty()) {

    DEBUG(dbgs() << "\nInserting Avr Loops.\n");

    // AVRGenerate has created a collection of AVR sequences which represent 
    // candidate loops for vectorization. At this point these AVR sequences do not
    // have any control flow AVRs in them.
    //
    // The control flow is not added in the first build of AVR for two reasons:
    //   1. If there is an error in control flow analysis, we still want a base 
    //      set of AVRS to fall back on for vectorization.
    // 
    //   2. The algorithm for detecting loop control flow and insert nodes is 
    //      simplier when done as a post processing on an exisiting AVR list.
    //
    // This walk will iterate through each AVR sequence (which represents a 
    // candidate loop nest) and insert AVRLoop nodes, and move the AVR nodes
    // which represent the body of the loop into AVRLoop's children, where
    // necessary.

    // TODO: Change iteration to vistor. In case of nested
    // WRN Nodes this will not properly recursively build loops
    // and link to WRN
    for (auto I = begin(), E = end(); I != E; ++I) {
      formAvrLoopNest(I);  
    }
  }
}


void AVRGenerate::formAvrLoopNest(AVRFunction *AvrFunction) {

  Function *Func = AvrFunction->getOrigFunction();
  const LoopInfo *LI = AvrFunction->getLoopInfo();
    
  for (auto I = Func->begin(), E = Func->end(); I != E; ++I) {

    if (!LI->isLoopHeader(I)) 
      continue;

    Loop *Lp = LI->getLoopFor(I);
    assert(Lp &&  "Loop not found for Loop Header BB!");

    BasicBlock *LoopLatchBB = Lp->getLoopLatch();
    assert(LoopLatchBB &&  "Loop Latch BB not found!");

    AVR *AvrLbl = AVRUtils::getAvrLabelForBB(I, AvrFunction); 
    AVR *AvrTerm = AVRUtils::getAvrBranchForTerm(LoopLatchBB->getTerminator(), 
                                                 AvrFunction);
    if (AvrLbl && AvrTerm) {

      // Create AvrLoop
      AVRLoop *AvrLoop = AVRUtils::createAVRLoop(Lp);
 
      // Hook AVR Loop into AVR Sequence
      AVRUtils::insertAVRBefore(AvrLbl, AvrLoop);
      AVRUtils::moveAsFirstChildren(AvrLoop, AvrLbl, AvrTerm);
    }

  }
}

void AVRGenerate::formAvrLoopNest(AVRWrn *AvrWrn) {

  const LoopInfo *LI = AvrWrn->getLoopInfo();
  AvrWrn->populateWrnBBSet();

  for (auto I = AvrWrn->wrnbbset_begin(), E = AvrWrn->wrnbbset_end();
       I != E; ++I) {

    // TODO: FIX THIS ASAP - Should not be using const_casts.
    // The BBSet build in WRN is returning const BBlocks, but the interfaces
    // for loop info cannot handle these.
    BasicBlock *LoopHeaderBB = const_cast<BasicBlock*>(*I);

    if (!LI->isLoopHeader(LoopHeaderBB)) 
      continue;

    Loop *Lp = LI->getLoopFor(LoopHeaderBB);
    assert(Lp &&  "Loop not found for Loop Header BB!");

    BasicBlock *LoopLatchBB = Lp->getLoopLatch();
    assert(LoopLatchBB &&  "Loop Latch BB not found!");

    AVR *AvrLbl = AVRUtils::getAvrLabelForBB(LoopHeaderBB, AvrWrn); 
    AVR *AvrTerm = AVRUtils::getAvrBranchForTerm(LoopLatchBB->getTerminator(), 
                                                 AvrWrn);
    if (AvrLbl && AvrTerm) {

      // Create AvrLoop
      AVRLoop *AvrLoop = AVRUtils::createAVRLoop(Lp);
 
      // TODO: For nested WRN, this needs to only be set for
      // top-level loop of WRN.
      AvrLoop->setWrnVecLoopNode(AvrWrn->getWrnNode());

      // Hook AVR Loop into AVR Sequence
      AVRUtils::insertAVRBefore(AvrLbl, AvrLoop);
      AVRUtils::moveAsFirstChildren(AvrLoop, AvrLbl, AvrTerm);
    }
  } 

  cleanupAvrWrnNodes();
}

void AVRGenerate::formAvrLoopNest(AVR *AvrNode) {

  if (AVRWrn *AvrWrn = dyn_cast<AVRWrn>(AvrNode)) {
    formAvrLoopNest(AvrWrn);
  } 
  else if (AVRFunction *AvrFunction = dyn_cast<AVRFunction>(AvrNode)) {
    formAvrLoopNest(AvrFunction);
  }
  else {
    assert (0 && "Unexpected Avr node for Loop formation!"); 
  }
}

void AVRGenerate::cleanupAvrWrnNodes() {
  // TODO
}


void AVRGenerate::formAvrSplits() {
  // TODO
}

void AVRGenerate::print(raw_ostream &OS, unsigned Depth, 
                        unsigned VerbosityLevel) const {

  formatted_raw_ostream FOS(OS);

  if (AVRList.empty()) {
    FOS << "No AVRs Generated!\n";
    return;
  }

  for (auto I = begin(), E = end(); I != E; ++I) {
    I->print(FOS, Depth, VerbosityLevel);
  }
}

void AVRGenerate::print(raw_ostream &OS, const Module *M) const {
  this->print(OS, 1, 1);
}

void AVRGenerate::dump(unsigned Level) const {
  formatted_raw_ostream OS(dbgs());
  this->print(OS, 1, Level);
}

bool AVRGenerate::codeGen() {

  if (!AVRList.empty()) {
    AVR *ANode = &AVRList.back();
    ANode->codeGen();
    return true;
  }

  return false;
}

void AVRGenerate::releaseMemory()
{
  AVRList.clear();

  // TODO: Free up all generated AVRs.
}
