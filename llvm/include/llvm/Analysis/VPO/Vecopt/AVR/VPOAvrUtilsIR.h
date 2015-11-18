//===------------------------------------------------------------*- C++ -*-===//
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
//   VPOAvrUtilsIR.h -- Defines the utilities class for AVR nodes 
//   that are LLVM IR specific.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANAYSIS_VPO_AVR_UTILS_IR_H
#define LLVM_ANAYSIS_VPO_AVR_UTILS_IR_H

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrStmtIR.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrIfIR.h"

namespace llvm { // LLVM Namespace

class LoopInfo;

namespace vpo {  // VPO Vectorizer Namespace


/// \brief This class defines the LLVM IR specific utilies for AVR nodes.
///
/// It contains functions which are used to create, modify, and destroy
/// AVR nodes.
///
class AVRUtilsIR {

public:

  // Creation Utilities

  /// \brief Returns a new AVRCompareIR node.
  static AVRCompareIR *createAVRCompareIR(Instruction *Inst);

  /// \brief Returns a new AVRSelectIR node.
  static AVRSelectIR *createAVRSelectIR(Instruction *Inst, AVRCompare *AComp);

  /// \brief Returns a new AVRAssignIR node.
  static AVRAssignIR *createAVRAssignIR(Instruction *Inst);

  /// \brief Returns a new AVRLabelIR node.
  static AVRLabelIR *createAVRLabelIR(BasicBlock *Block);

  /// \brief Returns a new AVRPhiIR node.
  static AVRPhiIR *createAVRPhiIR(Instruction *Inst);

  /// \brief Returns a new AVRCallIR node.
  static AVRCallIR *createAVRCallIR(Instruction *Inst);

  /// \brief Returns a new AVRBranchIR node.
  static AVRBranchIR *createAVRBranchIR(Instruction *Inst, AVR *Cond = nullptr);

  /// \brief Returns a new AVRBackEdgeIR node.
  static AVRBackEdgeIR *createAVRBackEdgeIR(Instruction *Inst);

  /// \brief Returns a new AVREntryIR node.
  static AVREntryIR *createAVREntryIR(Instruction *Inst);

  /// \brief Returns a new AVRReturnIR node.
  static AVRReturnIR *createAVRReturnIR(Instruction *Inst);

  /// \brief Returns a new AVRIfIR node.
  static AVRIfIR *createAVRIfIR(AVRBranch *ABranch);

  // Search Utilities

};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_ANAYSIS_VPO_AVR_UTILS_IR_H


