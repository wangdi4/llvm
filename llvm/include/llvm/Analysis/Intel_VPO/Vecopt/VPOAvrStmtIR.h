//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOAvrStmtIR.h -- Defines the Abstract Vector Representation (AVR) stmt
//   nodes for LLVM IR
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_STMT_IR_H
#define LLVM_ANALYSIS_VPO_AVR_STMT_IR_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrStmt.h"
#include "llvm/IR/Instruction.h"

namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Vectorizer Namespace

//----------AVR Assign Node for LLVM IR----------//
/// \brief Assign node abstract vector representation for LLVM IR.
///
/// See AVRAssign class for more information.
class AVRAssignIR : public AVRAssign {

private:
  /// \p Instruct - Original LLVM instruction.
  Instruction *Instruct;

protected:
  /// \brief AVRAssignIR Object Constructor.
  AVRAssignIR(Instruction *Instr);

  /// \brief AVRAssignIR Object Destructor.
  virtual ~AVRAssignIR() override {}

  /// \brief Copy Constructor.
  AVRAssignIR(const AVRAssignIR &AVRAssignIR);

  /// \brief Sets up state object.
  void initialize();

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsIR;

public:
  /// \brief Returns the LLVM instruction
  const Instruction *getLLVMInstruction() const { return Instruct; }

  /// \brief Returns the specified operand value.
  const Value *getOperand(unsigned OperandNumber);

  /// \brief Returns the number of operands for this instruction.
  unsigned getNumOperands() const;

  /// \brief Clone method for AVRAssignIR.
  AVRAssignIR *clone() const override;

  /// \brief Method for supporting type inquiry.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRAssignIRNode;
  }

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Code generation for AVR Assign.
  void codeGen();
};

//----------AVR Expression Node for LLVM IR----------//
/// \brief Avr expression node for LLVM IR.
///
/// See AVRExpression class for more information.
class AVRExpressionIR : public AVRExpression {

private:
  /// \p Instruct - LLVM Instruction which expression is built from.
  const Instruction *Instruct;

protected:
  /// \brief Constructs an AVRExpressionIR given an AVRAssignIR node and
  /// LHS/RHS specifier.
  AVRExpressionIR(AVRAssignIR *Assign, AssignOperand AOp);

  /// \brief AVRAssignIR Object Destructor.
  virtual ~AVRExpressionIR() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsIR;

public:
  /// \brief Returns the LLVM Instruction for this Expression.
  const Instruction *getLLVMInstruction() const { return Instruct; }

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRExpressionIRNode;
  }

  /// \brief Clone method for AVRExpressionIR.
  AVRExpressionIR *clone() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Retuurns the Opcode name of this expression's operation.
  virtual std::string getOpCodeName() const override;
};

//----------AVR Value Node for LLVM IR----------//
/// \brief AVR Value node for LLVM IR.
///
/// See AVRValue class for more information.
class AVRValueIR : public AVRValue {

private:
  /// \p Val - The LLVM IR Value for this operand.
  const Value *Val;

  /// \p ValType - LLVM type of this value.
  Type *ValType;

  /// \p Instruct - LLVM instruction containing the operand which
  /// this AVR Value represents.
  const Instruction *Instruct;

protected:
  /// \brief Constructs an AVRValueIR for the operand in \p Inst specified by
  /// \p V.
  AVRValueIR(const Value *V, const Instruction *Inst);

  /// \brief Destructor for this object.
  virtual ~AVRValueIR() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsIR;

public:
  /// \brief Clone method for AVRValueIR.
  AVRValueIR *clone() const override;

  /// \brief Method for supporting type inquiry.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRValueIRNode;
  }

  /// \brief Prints the AVRAssignIR node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VerbosityLevel) const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;
};

//----------AVR Label Node for LLVM IR----------//
/// \brief TODO
class AVRLabelIR : public AVRLabel {

private:
  /// \p SourceBlock - Basic Block of this label.
  BasicBlock *SourceBlock;

protected:
  AVRLabelIR(BasicBlock *SourceB);
  virtual ~AVRLabelIR() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsIR;

public:
  ///\brief returns BasicBlock assoicated with this Label.
  BasicBlock *getSourceBBlock() const { return SourceBlock; }

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRLabelIRNode;
  }

  AVRLabelIR *clone() const override;

  /// \brief Prints the AVRLabelIR node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VerbosityLevel) const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Code generation for AVR Label.
  void codeGen();
};

//----------AVR Phi Node for LLVM IR----------//
/// \brief TODO
class AVRPhiIR : public AVRPhi {

private:
  /// \brief Pointer to original LLVM Instruction
  Instruction *Instruct;

protected:
  AVRPhiIR(Instruction *Inst);
  virtual ~AVRPhiIR() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsIR;

public:
  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRPhiIRNode;
  }

  AVRPhiIR *clone() const override;

  /// \brief Returns the LLVM Instruction
  Instruction *getLLVMInstruction() const { return Instruct; }

  /// \brief Prints the AVRPhiIR node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VerbosityLevel) const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Code generation for AVR Phi.
  void codeGen();
};

//----------AVR Call Node for LLVM IR----------//
/// \brief An abstract vector call node for LLVM IR.
class AVRCallIR : public AVRCall {

private:
  /// \p Instruct - Original LLVM call instruction
  Instruction *Instruct;

protected:
  AVRCallIR(Instruction *Inst);
  virtual ~AVRCallIR() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsIR;

public:
  /// \brief Returns the LLVM Instruction
  const Instruction *getLLVMInstruction() const { return Instruct; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRCallIRNode;
  }

  AVRCallIR *clone() const override;

  /// \brief Prints the AVRCallIR node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VerbosityLevel) const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Code generation for AVR Call.
  void codeGen();
};

//----------AVR Branch Node for LLVM IR----------//
/// \brief An abstract vector branch node for LLVM IR.
class AVRBranchIR : public AVRBranch {

private:
  /// \p Instruct - Original LLVM instruction.
  Instruction *Instruct;

  /// \p ThenBBlock - If conditional branch, pointer to the true successor
  /// block.
  BasicBlock *ThenBBlock;

  /// \p ElseBBlock - If conditional branch, pointer to the false successor
  /// block.
  BasicBlock *ElseBBlock;

protected:
  /// \brief Create a new branch from LLVM branch instruction \p In and
  /// branch condition \p Cond (optional).
  AVRBranchIR(Instruction *In, AVR *Cond = nullptr);

  virtual ~AVRBranchIR() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsIR;

public:
  /// \brief Returns Branch Instruction
  const Instruction *getLLVMInstruction() const { return Instruct; }

  /// \brief Returns ThenBBlock successor for conditional branch.
  BasicBlock *getThenBBlock() const { return ThenBBlock; }

  /// \brief Returns ElseBBlock successor for conditional branch.
  BasicBlock *getElseBBlock() const { return ElseBBlock; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRBranchIRNode;
  }

  AVRBranchIR *clone() const override;

  /// \brief Prints the AVRFBranchIR node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VerbosityLevel) const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Code generation for AVR Forward branch.
  void codeGen();
};

//----------AVR Backedge Node for LLVM IR----------//
/// \brief An abstract vector backedge node for LLVM IR.
class AVRBackEdgeIR : public AVRBackEdge {

private:
  /// \p Instruct - Original LLVM instruction
  Instruction *Instruct;

protected:
  AVRBackEdgeIR(Instruction *Inst);
  virtual ~AVRBackEdgeIR() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsIR;

public:
  /// \brief Returns Branch Instruction
  const Instruction *getLLVMInstruction() const { return Instruct; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRBackEdgeIRNode;
  }

  AVRBackEdgeIR *clone() const override;

  /// \brief Prints the AVRBackEdgeIR node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VerbosityLevel) const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Code generation for AVR Backedge.
  void codeGen();
};

//----------AVR Entry Node for LLVM IR----------//
/// \brief An abstract vector entry node for LLVM IR.
class AVREntryIR : public AVREntry {

private:
  /// \p Instruct - Original LLVM entry instruction.
  Instruction *Instruct;

protected:
  AVREntryIR(Instruction *Inst);
  virtual ~AVREntryIR() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsIR;

public:
  /// \brief Returns Branch Instruction
  const Instruction *getLLVMInstruction() const { return Instruct; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVREntryIRNode;
  }

  AVREntryIR *clone() const override;

  /// \brief Prints the AVREntryIR node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VerbosityLevel) const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Code generation for AVR Entry.
  void codeGen();
};

//----------AVR Return Node for LLVM IR----------//
/// \brief An abstract vector return node for LLVM IR.
class AVRReturnIR : public AVRReturn {

private:
  /// \p Instruct - Original LLVM return instruction
  Instruction *Instruct;

protected:
  AVRReturnIR(Instruction *Inst);
  virtual ~AVRReturnIR() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsIR;

public:
  /// \brief Returns Branch Instruction
  const Instruction *getLLVMInstruction() const { return Instruct; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRReturnIRNode;
  }

  AVRReturnIR *clone() const override;

  /// \brief Prints the AVRReturnIR node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VerbosityLevel) const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Code generation for AVR Return.
  void codeGen();
};

//----------AVR Select Node for LLVM IR----------//
/// \brief An abstract vector select node.
///
class AVRSelectIR : public AVRSelect {

private:

  /// \p Instruct - Originial LLVM Instruction
  Instruction *Instruct;

protected:

  /// \brief Construct an avr select for llvm ir node.
  AVRSelectIR(Instruction *Inst, AVR *ACond);

  /// \brief Virutal destructor.
  virtual ~AVRSelectIR() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsIR;

public:

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRSelectIRNode;
  }

  /// \brief Returns the underlying LLVM instruction.
  Instruction *getLLVMInstruction() { return Instruct; }

  AVRSelectIR *clone() const override;

  /// \brief Prints the AVRSelectIR node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VerbosityLevel) const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Code generation for AVR Return.
  void codeGen();
};

//----------AVR Compare Node for LLVM IR----------//
/// \brief An abstract vector select node.
///
class AVRCompareIR : public AVRCompare {

private:
  /// \p Instruct - Originial LLVM select instruction
  Instruction *Instruct;

protected:
  AVRCompareIR(Instruction *Inst);
  virtual ~AVRCompareIR() override {}

  // TODO: Add Member Functions
  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsIR;

public:
  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRCompareIRNode;
  }

  /// \brief Returns the underlying LLVM instruction.
  Instruction *getLLVMInstruction() { return Instruct; }

  AVRCompareIR *clone() const override;

  /// \brief Prints the AVRSelectIR node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VerbosityLevel) const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Code generation for AVR Return.
  void codeGen();
};

//----------AVR Unreachable Node for LLVM IR----------//
/// \brief An abstract vector unreachable node for LLVM IR.
class AVRUnreachableIR : public AVRUnreachable {

private:

  /// \p Instruct - Original LLVM return instruction
  Instruction *Instruct;

protected:

  /// \brief AVRUnreachableIR object constructor. 
  AVRUnreachableIR(Instruction *Inst);

  /// \brief Object destructor.
  virtual ~AVRUnreachableIR() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsIR;

public:

  /// \brief Returns unreachable instruction.
  const Instruction *getLLVMInstruction() const { return Instruct; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRUnreachableIRNode;
  }

  /// \brief Clone method for AVRUnreachable.
  AVRUnreachableIR *clone() const override;

};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_ANALYSIS_VPO_AVR_STMT_IR_H
