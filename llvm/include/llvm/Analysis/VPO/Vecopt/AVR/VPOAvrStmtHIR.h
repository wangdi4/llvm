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
//   VPOAvrStmtHIR.h -- Defines the Abstract Vector Representation (AVR) stmt
//   nodes for HIR
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_STMT_HIR_H
#define LLVM_ANALYSIS_VPO_AVR_STMT_HIR_H

#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrStmt.h"

#include "llvm/IR/Intel_LoopIR/HLNode.h"

using namespace llvm::loopopt;

namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Vectorizer Namespace

//----------AVR Assign Node for HIR----------//
/// \brief Assign node abstract vector representation for HIR.
///
/// An AVRAssignIR node represents an assignment found in HIR.
class AVRAssignHIR : public AVRAssign {

private:
  HLNode *Instruct;

protected:
  /// \brief AVRAssignHIR Object Constructor.
  AVRAssignHIR(HLNode *Instr);

  /// \brief AVRAssignHIR Object Destructor.
  virtual ~AVRAssignHIR() override {}

  /// \brief Copy Constructor. 
  AVRAssignHIR (const AVRAssignHIR &AVRAssignHIR);

  /// \brief Sets up state object.
  void initialize();

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsHIR;

public:

  /// \brief Returns the HIR Instruction
  const HLNode *getHIRInstruction() const { return Instruct; }

  AVRAssignHIR *clone() const override;

  /// \brief Method for supporting type inquiry.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRAssignHIRNode;
  }

  /// \brief Prints the AVRAssignHIR node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;
};

//----------AVR Label Node for HIR----------//
/// \brief Avr Label node for HIR.
class AVRLabelHIR : public AVRLabel {

private:
  HLNode *Instruct;

protected:

  AVRLabelHIR(HLNode *Instr);
  virtual ~AVRLabelHIR() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsHIR;

public:
  /// \brief returns HIR Instruction assoicated with this Label.
  const HLNode *getHIRInstruction () const { return Instruct; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRLabelHIRNode;
  }

  AVRLabelHIR *clone() const override;

  /// \brief Prints the AVRLabelHIR node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;
};

//----------AVR Branch Node for HIR----------//
/// \brief An abstract vector forward branch node for HIR.
class AVRBranchHIR : public AVRBranch {

private:
  HLNode *Instruct;
  // TODO: Add Member Data

protected:
  AVRBranchHIR(HLNode *Inst);
  virtual ~AVRBranchHIR() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsHIR;

public:
  // TODO: Add Member Functions

  /// \brief Returns FBranch Instruction
  const HLNode *getHIRInstruction() const { return Instruct; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRBranchHIRNode;
  }

  AVRBranchHIR *clone() const override;

  /// \brief Prints the AVRFBranchHIR node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;
};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace 

#endif  // LLVM_ANALYSIS_VPO_AVR_STMT_HIR_H
