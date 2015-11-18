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

AVRCompareIR *AVRUtilsIR::createAVRCompareIR(Instruction *Inst) {
  return new AVRCompareIR(Inst);
}

AVRSelectIR *AVRUtilsIR::createAVRSelectIR(Instruction *Inst, AVRCompare *AComp) {
  return new AVRSelectIR(Inst, AComp);
}

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

AVRBranchIR *AVRUtilsIR::createAVRBranchIR(Instruction *Inst, AVR *Cond) {
  return new AVRBranchIR(Inst, Cond);
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

AVRIfIR *AVRUtilsIR::createAVRIfIR(AVRBranch *ABranch) {
  return new AVRIfIR(ABranch);
}

// Search Utilities
