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
//   VPOAvrIfIR.h -- Defines the Abstract Vector Representation (AVR) if node
//   for LLVM IR.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_IF_IR_H
#define LLVM_ANALYSIS_VPO_AVR_IF_IR_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrIf.h"

namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Vectorizer Namespace

/// \brief If node abstract vector representation
///
/// An AVRIfIR node represents an if-statement found in LLVM IR.
class AVRIfIR : public AVRIf {
private:

  /// Condition - Avr which contains condition that conditional branch uses
  /// to branch to Then and Else blocks.
  AVR *Condition;

  /// Branch - Conditional branch that jumps to then and else blocks.
  AVRBranch *AvrBranch;

protected:

  /// \brief Construct an AVRIf from LLVM. 
  AVRIfIR(AVRBranch *ABranch);  

  /// \brief Copy Constructor
  AVRIfIR (const AVRIfIR &AVRIfIR);
  virtual ~AVRIfIR() override {}

  /// \brief Sets up state object.
  void initialize();

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsIR;

public:

  AVRIfIR *clone() const override;
 
  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRIfIRNode;
  }
  /// \brief Returns the number of operands for this instruction.
  unsigned getNumOperands() const;

  /// \brief Returns the Conditional AvrBranch for AvrIf.
  AVRBranch *getAvrBranch() const { return AvrBranch; }

  /// \brief Returns the AVR Condition for AvrIf.
  AVR *getCondition() const { return Condition; }

  /// \brief Prints the AvrIf node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
	     VerbosityLevel VLevel) const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Code generation for AVR IF
  void codeGen()  override;
};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif  // LLVM_ANALYSIS_VPO_AVR_IF_IR_H
