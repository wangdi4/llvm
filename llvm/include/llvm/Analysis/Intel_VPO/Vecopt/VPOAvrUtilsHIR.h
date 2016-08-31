//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOAvrUtilsHIR.h -- Defines the utilities class for AVR nodes 
//   that are HIR specific.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANAYSIS_VPO_AVR_UTILS_HIR_H
#define LLVM_ANAYSIS_VPO_AVR_UTILS_HIR_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrStmtHIR.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrIfHIR.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrSwitchHIR.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrLoopHIR.h"

using namespace llvm::loopopt;

namespace llvm { // LLVM Namespace

namespace vpo {  // VPO Vectorizer Namespace


/// \brief This class defines the HIR specific utilies for AVR nodes.
///
/// It contains functions which are used to create, modify, and destroy
/// AVR nodes.
///
class AVRUtilsHIR {

public:

  // Creation Utilities

  /// \brief Returns a new AVRLoopHIR node.
  static AVRLoopHIR *createAVRLoopHIR(HLLoop *Lp);

  /// \brief Returns a new AVRAssignHIR node.
  static AVRAssignHIR *createAVRAssignHIR(HLInst *Inst);

  /// \brief Returns a new AVRLabelHIR node.
  static AVRLabelHIR *createAVRLabelHIR(HLNode *Inst);

  /// \brief Returns a new AVRBranchHIR node.
  static AVRBranchHIR *createAVRBranchHIR(HLNode *Inst);

  /// \brief Returns a new AVRIfHIR node.
  static AVRIfHIR *createAVRIfHIR(HLIf *Inst);

  /// \brief Returns a new AVRExpressionHIR node.
  static AVRExpressionHIR *createAVRExpressionHIR(AVRAssignHIR *HLAssign,
                                                  AssignOperand AOp);

  /// \brief Returns a new AVRExpressionHIR node.
  static AVRExpressionHIR *createAVRExpressionHIR(AVRIfHIR *AIf,
                                             HLIf::const_pred_iterator& PredIt);

  /// \brief Returns a new AVRExpressionHIR node.
  static AVRExpressionHIR *createAVRExpressionHIR(AVRExpressionHIR* LHS,
                                                  AVRExpressionHIR* RHS);

  /// \brief Returns a new AVRExpressionHIR node.
  static AVRExpressionHIR *createAVRExpressionHIR(AVR *LHS, AVR *RHS, Type *Ty,
                                                  unsigned Opcode);

  /// \brief Returns a new AVRValueHIR node.
  static AVRValueHIR *createAVRValueHIR(RegDDRef *DDRef,
                                        HLNode *HNode,
                                        AVR *Parent);

  /// \brief Returns a new AVRValueHIR node.
  static AVRValueHIR *createAVRValueHIR(BlobDDRef *DDRef,
                                        AVR *Parent);

  /// \brief Returns a new AVRValueHIR node.
  static AVRValueHIR *createAVRValueHIR(CanonExpr *CE, unsigned IVIndex,
                                        Type *Ty, AVR *Parent);

  /// \brief Returns a new constant AVRValueHIR node.
  static AVRValueHIR *createAVRValueHIR(Constant *Const, AVR *Parent);

  /// \brief Returns a new AVRSwitchHIR node.
  static AVRSwitchHIR *createAVRSwitchHIR(HLSwitch *HSwitch);

  /// \brief Returns a new AVRUnreachableHIR node.
  static AVRUnreachableHIR *createAVRUnreachableHIR(HLNode *Node);
};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_ANAYSIS_VPO_AVR_UTILS_HIR_H
