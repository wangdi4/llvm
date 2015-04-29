//===-------- VPOAvrGenerate.cpp - Creates AVR Nodes ------*- C++ -*-------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the AVRGenerate Pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrGenerate.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;
using namespace intel;

static RegisterPass<AVRGenerate> X("avr-generate","AVR Generate", false, true);

char AVRGenerate::ID = 0;

FunctionPass *llvm::createAVRGeneratePass() { return new AVRGenerate(); }

AVRGenerate::AVRGenerate() : FunctionPass(ID) {
 //llvm::initializeAvrGeneratePass(*PassRegistry::getPassRegistry());
}

void AVRGenerate::getAnalysisUsage(AnalysisUsage &AU) const
{
  AU.setPreservesAll();

  // Eric: Set WRN Analysis 
  // AU.addRequiredTransitive<WRNLoopInfo>();
}

void AVRGenerate::create() {
  DEBUG(dbgs() << "\nBuilding AVR Tree.\n");

  //DEBUG(dbgs() << "\nFunction = \n");
  //DEBUG(dbgs() << *this->Func );

  AVRFunction *FuncNode = AVRUtils::createAVRFunction(Func);
  buildBody(FuncNode);

  AVRList.push_back(FuncNode);
}


bool AVRGenerate::runOnFunction(Function &F) {
  this->Func = &F;

  // Get Required Analyses
  // WRN = &getAnalysis<WRNLoopInfo>();

  create();
  return false;
}

void AVRGenerate::buildBody(AVRFunction *AVRFunc) {

  Function *Func = AVRFunc->getOrigFunction();
  AVRContainerTy::iterator InsertionItr = AVRFunc->child_begin();

  //TODO: Need to change algorithm and build body recursively.

  // Iterate over basic blocks.
  for (auto BBItr = Func->begin(), BBEnd = Func->end(); BBItr != BBEnd; ++BBItr){

    // Create BasicBlock Label
    AVRLabel *Label = AVRUtils::createAVRLabel(BBItr);

    if (AVRFunc->getNumChildren() == 0) {
      AVRUtils::insertFirstChildAVR(AVRFunc, Label);
    }
    else {
      AVRUtils::insertAVRAfter(InsertionItr, Label);
    }

    // Update iterator to last inserted node.
    InsertionItr = AVRFunc->getLastChild();

    // Iterate over instructions.
    for (auto IItr = BBItr->begin(), IEnd = BBItr->end(); IItr != IEnd; ++IItr,
         ++InsertionItr) {

      AVR *NewNode;
      switch(IItr->getOpcode()) {
        case Instruction::Call:
          NewNode = AVRUtils::createAVRCall(IItr);
          break;
        case Instruction::PHI:
          NewNode = AVRUtils::createAVRPhi(IItr);
	  break;
        case Instruction::Br:
          NewNode = AVRUtils::createAVRFBranch(IItr);
	  break;
	  //case Instruction::BackEdge:
	  //break;
	  //case Instruction::Entry:
	  //break;
        case Instruction::Ret:
	  NewNode = AVRUtils::createAVRReturn(IItr);
	  break;
        case Instruction::ICmp:
        case Instruction::FCmp:
	  NewNode = AVRUtils::createAVRIf(IItr);
	  // TODO: Recursively build body
	  //       This is not quite correct, need to properly set predication
          break;
	  //case Instruction::Loop:
	  //break;
        default:
          NewNode = AVRUtils::createAVRAssign(IItr);
      } 

      // Hook into AVR List Children Container
      AVRUtils::insertAVRAfter(InsertionItr, NewNode);
    }
  }
}

void AVRGenerate::print() {

  if (!AVRList.empty()) {
    DEBUG(dbgs() << "\nBEGIN: Generated AVR List\n");
    // TODO: Support proper printing
    AVR *ANode = &AVRList.back();
    ANode->dump();
    DEBUG(dbgs() << "END: Generated AVR List\n\n\n");
  }
  else {
    DEBUG(dbgs() << "No AVRs Generated\n\n");
  }
}
