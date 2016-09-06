//===-- VPOAvrUtilsHIR.cpp ------------------------------------------------===//
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
/// This file implements the Abstract Vector Representation (AVR) utilities
/// specific to HIR.
///
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrUtilsHIR.h"

#define DEBUG_TYPE "avr-utilities"

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::loopopt;

// AVR Creation Utilities

AVRLoopHIR *AVRUtilsHIR::createAVRLoopHIR(HLLoop *Lp) {
  return new AVRLoopHIR(Lp);
}

AVRAssignHIR *AVRUtilsHIR::createAVRAssignHIR(HLInst *Inst) {
  return new AVRAssignHIR(Inst);
}

AVRLabelHIR *AVRUtilsHIR::createAVRLabelHIR(HLNode *Inst) {
  return new AVRLabelHIR(Inst);
}

AVRBranchHIR *AVRUtilsHIR::createAVRBranchHIR(HLNode *Inst) {
  return new AVRBranchHIR(Inst);
}

AVRIfHIR *AVRUtilsHIR::createAVRIfHIR(HLIf *If) {
  AVRIfHIR* AIf = new AVRIfHIR(If);

  AVRExpressionHIR *Root = nullptr;
  AVRExpressionHIR *Last;
  for (auto It = If->pred_begin(), E = If->pred_end(); It != E; ++It) {
    Last = createAVRExpressionHIR(AIf, It);
    if (Root) {

      // Create an expression for the implicit AND between predicates.
      Root = createAVRExpressionHIR(Root, Last);
    }
    else {
      Root = Last;
    }
  }

  AIf->Condition = Root;

  return AIf;
}

AVRExpressionHIR *AVRUtilsHIR::createAVRExpressionHIR(AVRAssignHIR *HLAssign,
                                                      AssignOperand AOp) {
  return new AVRExpressionHIR(HLAssign, AOp);
}

AVRExpressionHIR *AVRUtilsHIR::createAVRExpressionHIR(AVRIfHIR *AIf,
                                                HLIf::const_pred_iterator& It) {

  return new AVRExpressionHIR(AIf, It);
}

AVRExpressionHIR *AVRUtilsHIR::createAVRExpressionHIR(AVRExpressionHIR* LHS,
                                                      AVRExpressionHIR* RHS) {

  return new AVRExpressionHIR(LHS, RHS);
}

AVRValueHIR *AVRUtilsHIR::createAVRValueHIR(RegDDRef *DDRef,
                                            HLNode *HNode,
                                            AVR *Parent) {
  return new AVRValueHIR(DDRef, HNode, Parent);
}

AVRSwitchHIR *AVRUtilsHIR::createAVRSwitchHIR(HLSwitch *HSwitch) {

  AVRSwitchHIR *ASwitchHIR = new AVRSwitchHIR(HSwitch);

  ASwitchHIR->ConditionValue = createAVRValueHIR(HSwitch->getConditionDDRef(),
                                                 HSwitch,
                                                 ASwitchHIR);

  return ASwitchHIR;
}

AVRUnreachableHIR *AVRUtilsHIR::createAVRUnreachableHIR(HLNode *HUnreach) {
  return new AVRUnreachableHIR(HUnreach);
}
