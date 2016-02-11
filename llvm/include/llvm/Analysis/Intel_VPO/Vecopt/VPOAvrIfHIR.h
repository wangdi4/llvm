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
//   VPOAvrIfHIR.h -- Defines the Abstract Vector Representation (AVR) if node
//   for HIR.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_IF_HIR_H
#define LLVM_ANALYSIS_VPO_AVR_IF_HIR_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrIf.h"
#include "llvm/IR/Intel_LoopIR/HLIf.h"

using namespace llvm::loopopt;

namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Vectorizer Namespace

/// \brief If node abstract vector representation
///
/// An AVRIfHIR node represents an if-statement found in HIR.
class AVRIfHIR : public AVRIf {
private:
  /// If comparison instruction
  HLIf *CompareInstruction;

protected:
  AVRIfHIR(HLIf *CompareInst);
  AVRIfHIR(const AVRIfHIR &AVRIfHIR);
  virtual ~AVRIfHIR() override {}

  /// \brief Sets up state object.
  void initialize();

  // TODO: Get Predicate
  // TODO: Get Conjuntion

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsHIR;

public:
  AVRIfHIR *clone() const override;

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRIfHIRNode;
  }
  /// \brief Returns the number of operands for this instruction.
  unsigned getNumOperands() const;

  /// \brief Returns the underlying HLIf node for this AvrIf.
  const HLIf *getCompareInstruction() const { return CompareInstruction; }

  /// \brief Prints the AvrIf node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Code generation for AVR IF
  void codeGen() override;
};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_ANALYSIS_VPO_AVR_IF_HIR_H
