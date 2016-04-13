//===-- VPOAvrUtilsIR.cpp -------------------------------------------------===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file Implements the Abstract Vector Representation (AVR) utilities
/// specific to LLVM IR.
///
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrUtils.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrUtilsIR.h"

#define DEBUG_TYPE "avr-utilities"

using namespace llvm;
using namespace llvm::vpo;

// AVR Creation Utilities

AVRLoopIR *AVRUtilsIR::createAVRLoopIR(Loop *Lp) {
  return new AVRLoopIR(Lp);
}

AVRCompareIR *AVRUtilsIR::createAVRCompareIR(Instruction *Inst) {
  return new AVRCompareIR(Inst);
}

AVRSelectIR *AVRUtilsIR::createAVRSelectIR(Instruction *Inst,
                                           AVR*ACond) {
  return new AVRSelectIR(Inst, ACond);
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

AVRExpressionIR *AVRUtilsIR::createAVRExpressionIR(AVRAssignIR *Assign,
                                                   AssignOperand AOp) {
  return new AVRExpressionIR(Assign, AOp);
}

AVRValueIR *AVRUtilsIR::createAVRValueIR(const Value *V,
                                         const Instruction *Inst) {
  return new AVRValueIR(V, Inst);
}

AVRSwitchIR *AVRUtilsIR::createAVRSwitchIR(Instruction *SwitchI) {
  return new AVRSwitchIR(SwitchI);
}

AVRUnreachableIR *AVRUtilsIR::createAVRUnreachableIR(Instruction *UI) {
  return new AVRUnreachableIR(UI);
}

// Modification Utilites

// Search Utilities
