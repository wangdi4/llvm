//===-- VPOAvrSwitchHIR.h ---------------------------------------*- C++ -*-===//
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
/// HIR.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_SWITCH_HIR_H
#define LLVM_ANALYSIS_VPO_AVR_SWITCH_HIR_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrSwitch.h"
#include "llvm/IR/Intel_LoopIR/HLSwitch.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"

using namespace llvm::loopopt;

namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Vectorizer Namespace

/// \brief Switch abstract vector representation. An AVRSwitchHIR node is
/// constructed to represent switch instructions found in HIR.
class AVRSwitchHIR : public AVRSwitch {

private:
  /// Instruct - Underlying HIR switch instruction
  HLSwitch *Instruct;

  /// Condition - DDref which represents switch condition
  RegDDRef *Condition;

protected:
  /// \brief Constructs an AVRSwitchHIR object from given HIR switch
  /// instruction.
  AVRSwitchHIR(HLSwitch *SwitchIn);

  /// \brief Copy constructor.
  AVRSwitchHIR(AVRSwitchHIR *ASwitch);

  /// \brief Object destructor.
  virtual ~AVRSwitchHIR() override {}

  /// \brief Sets the condition for this switch.
  void setCondition(RegDDRef *Cond) { Condition = Cond; }

  /// \brief Set up the object state.
  void initialize();

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsHIR;

public:
  /// \brief AVRSwitchIR clone method.
  AVRSwitchHIR *clone() const override;

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRSwitchHIRNode;
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

#endif // LLVM_ANALYSIS_VPO_AVR_SWITCH_HIR_H
