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
//   VPOAvrUtilsHIR.h -- Defines the utilities class for AVR nodes 
//   that are HIR specific.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANAYSIS_VPO_AVR_UTILS_HIR_H
#define LLVM_ANAYSIS_VPO_AVR_UTILS_HIR_H

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrStmtHIR.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrIfHIR.h"

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

  /// \brief Returns a new AVRAssignHIR node.
  static AVRAssignHIR *createAVRAssignHIR(HLNode *Inst);

  /// \brief Returns a new AVRLabelHIR node.
  static AVRLabelHIR *createAVRLabelHIR(HLNode *Inst);

  /// \brief Returns a new AVRBranchHIR node.
  static AVRBranchHIR *createAVRBranchHIR(HLNode *Inst);

  /// \brief Returns a new AVRIfHIR node.
  static AVRIfHIR *createAVRIfHIR(HLIf *Inst);
};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_ANAYSIS_VPO_AVR_UTILS_HIR_H
