//===-- VPOAvrSwitchIR.h ----------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the Abstract Vector Representation (AVR) switch node for
/// LLVM IR.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_SWITCH_IR_H
#define LLVM_ANALYSIS_VPO_AVR_SWITCH_IR_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrSwitch.h"

namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Vectorizer Namespace

/// \brief Switch abstract vector representation. An AVRSwitchIR node is
/// constructed to represent switch instructions found in LLVM IR.
class AVRSwitchIR : public AVRSwitch {

private:
  /// Instruct - Underyling LLVM IR switch instruction.
  Instruction *Instruct;

protected:
  /// \brief Constructs an AVRSwitchIR object from given switch instruction.
  AVRSwitchIR(Instruction *SwitchIn);

  /// \brief Copy constructor.
  AVRSwitchIR(AVRSwitchIR *ASwitch);

  /// \brief Object destructor.
  virtual ~AVRSwitchIR() override {}

  /// \brief Set up the object state.
  void initialize();

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsIR;

public:
  /// \brief AVRSwitchIR clone method.
  AVRSwitchIR *clone() const override;

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRSwitchIRNode;
  }

  /// \brief Returns the number of operands for this instruction.
  unsigned getNumOperands() const;

  /// \brief Returns a constant StringRef for the codition of the switch.
  virtual void
  printConditionValueName(formatted_raw_ostream &OS) const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;
};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_ANALYSIS_VPO_AVR_SWITCH_IR_H
