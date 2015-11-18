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
//   VPOAvrKinds.h -- Defines the Abstract Vector Representation (AVR) node
///  kinds.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_KINDS_H
#define LLVM_ANALYSIS_VPO_AVR_KINDS_H

#ifdef LLVM_ANALYSIS_VPO_AVR_H    // Only include the contents in VPOAvr.h

/// \brief Enumeration for the concrete subclasses of AVR.
///
/// Note that for each of the statement kinds, we have a variant for each of
/// the IR kinds that the AL supports - for LLVM IR, HIR so on. For each of
/// the statement kinds, the enum values are delimited by a value that is used
/// for the base class for that AVR kind and the value with Last. As an example,
/// the enum values for an assignment statement for different IRs will fall in
/// the range AVRAssignNode and AVRAssignLastNode. We need to ensure that this
/// ordering is maintained for things to work correctly.
enum AVRVal {
  AVRLoopNode,
  AVRFunctionNode,

  AVRIfNode,
  AVRIfIRNode,
  AVRIfHIRNode,
  AVRIfLastNode,

  AVRAssignNode,
  AVRAssignIRNode,
  AVRAssignHIRNode,
  AVRAssignLastNode,

  AVRExprNode,

  AVRLabelNode,
  AVRLabelIRNode,
  AVRLabelHIRNode,
  AVRLabelLastNode,

  AVRPhiNode,
  AVRPhiIRNode,
  AVRPhiLastNode,

  AVRCallNode,
  AVRCallIRNode,
  AVRCallLastNode,

  AVRBranchNode,
  AVRBranchIRNode,
  AVRBranchHIRNode,
  AVRBranchLastNode,

  AVRBackEdgeNode,
  AVRBackEdgeIRNode,
  AVRBackEdgeLastNode,

  AVREntryNode,
  AVREntryIRNode,
  AVREntryLastNode,

  AVRReturnNode,
  AVRReturnIRNode,
  AVRReturnLastNode,

  AVRCompareNode,
  AVRCompareIRNode,
  AVRCompareLastNode,

  AVRSelectNode,
  AVRSelectIRNode,
  AVRSelectLastNode,

  AVRWrnNode
};

#endif // LLVM_ANALYSIS_VPO_AVR_H

#endif // LLVM_ANALYSIS_VPO_AVR_KINDS_H
