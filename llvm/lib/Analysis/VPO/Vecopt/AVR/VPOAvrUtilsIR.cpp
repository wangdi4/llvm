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
//   VPOAvrUtilsIR.cpp -- Implements the Abstract Vector Representation (AVR)
//   utilities specific to LLVM IR.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrUtils.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrUtilsIR.h"

#define DEBUG_TYPE "avr-utilities"

using namespace llvm;
using namespace llvm::vpo;

// AVR Creation Utilities
AVRAssignIR *AVRUtilsIR::createAVRAssignIR(Instruction *Inst) {
  return new AVRAssignIR(Inst);
}

AVRLabelIR *AVRUtilsIR::createAVRLabelIR(BasicBlock *Block) {
  return new AVRLabelIR(Block);
}

AVRPhiIR *AVRUtilsIR::createAVRPhiIR(Instruction *Inst) {
  return new AVRPhiIR(Inst);
}

AVRCallIR *AVRUtilsIR::createAVRCallIR(Instruction *Inst) {
  return new AVRCallIR(Inst);
}

AVRFBranchIR *AVRUtilsIR::createAVRFBranchIR(Instruction *Inst) {
  return new AVRFBranchIR(Inst);
}

AVRBackEdgeIR *AVRUtilsIR::createAVRBackEdgeIR(Instruction *Inst) {
  return new AVRBackEdgeIR(Inst);
}

AVREntryIR *AVRUtilsIR::createAVREntryIR(Instruction *Inst) {
  return new AVREntryIR(Inst);
}

AVRReturnIR *AVRUtilsIR::createAVRReturnIR(Instruction *Inst) {
  return new AVRReturnIR(Inst);
}

AVRIfIR *AVRUtilsIR::createAVRIfIR(Instruction *Inst) {
  return new AVRIfIR(Inst);
}

// Search Utilities

AVRLabelIR *AVRUtilsIR::getAvrLabelForBB(BasicBlock *BB, AVR *ParentNode) {

  assert(BB && "Missing Basic Block!");
  assert(ParentNode && "Missing Avr Node!");

  AVRLabelIR *AvrLabelNode = nullptr;
  AVRContainerTy *Children = AVRUtils::getAvrChildren(ParentNode);

  if (Children) {
    for (auto I = Children->begin(), E = Children->end(); I != E; ++I) {
      AvrLabelNode = AVRUtilsIR::getAvrLabelForBB(BB, I);
      if (AvrLabelNode)
	return AvrLabelNode; // Found Node
    }
  }
  else {
    if (AVRLabelIR *AvrLb = dyn_cast<AVRLabelIR>(ParentNode)) {
      if (AvrLb->getSourceBBlock() == BB) {
        AvrLabelNode = AvrLb;
      }
    }
  }

  return AvrLabelNode;
}

AVRFBranchIR *AVRUtilsIR::getAvrBranchForTerm(Instruction *Terminator, AVR *ParentNode) {

  assert(Terminator && "Missing Basic Block terminator!");
  assert(ParentNode && "Missing Avr Node!");

  AVRFBranchIR *AvrBNode = nullptr;
  AVRContainerTy *Children = AVRUtils::getAvrChildren(ParentNode);

  if (Children) {
    for (auto I = Children->begin(), E = Children->end(); I != E; ++I) {
      AvrBNode = AVRUtilsIR::getAvrBranchForTerm(Terminator, I);
      if (AvrBNode)
	return AvrBNode; // Found Node
    }
  }
  else {
    if (AVRFBranchIR *AvrB = dyn_cast<AVRFBranchIR>(ParentNode)) {
      if (AvrB->getLLVMInstruction() == Terminator) {
        AvrBNode = AvrB;
      }
    }
  }

  return AvrBNode;
}

