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
//   VPOAvrStmtIR.h -- Defines the Abstract Vector Representation (AVR) stmt
//   nodes for LLVM IR
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_STMT_IR_H
#define LLVM_ANALYSIS_VPO_AVR_STMT_IR_H

#include "llvm/IR/Instruction.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrStmt.h"

namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Vectorizer Namespace

//----------AVR Assign Node for LLVM IR----------//
/// \brief Assign node abstract vector representation for LLVM IR.
///
/// An AVRAssignIR node represents an assignment found in LLVM IR.
class AVRAssignIR : public AVRAssign {

private:

  /// Instruct - Original LLVM instruction.
  Instruction *Instruct;

protected:

  /// \brief AVRAssignIR Object Constructor.
  AVRAssignIR(Instruction *Instr);

  /// \brief AVRAssignIR Object Destructor.
  virtual ~AVRAssignIR() override {}

  /// \brief Copy Constructor. 
  AVRAssignIR (const AVRAssignIR &AVRAssignIR);

  /// \bried Sets up state object.
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

  AVRAssignIR *clone() const override;

  /// \brief Method for supporting type inquiry.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRAssignIRNode;
  }

  /// \brief Prints the AVRAssignIR node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VerbosityLevel) const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Code generation for AVR Assign.
  void codeGen();

};


//----------AVR Label Node for LLVM IR----------//
/// \brief TODO
class AVRLabelIR : public AVRLabel {

private:

  /// SourceBlock - Basic Block of this label.
  BasicBlock *SourceBlock;

protected:

  AVRLabelIR(BasicBlock *SourceB);
  virtual ~AVRLabelIR() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsIR;

public:

  ///\brief returns BasicBlock assoicated with this Label.
  BasicBlock *getSourceBBlock () const { return SourceBlock; }

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
  const Instruction *getLLVMInstruction() const { return Instruct; }

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

  /// Instruct - Original LLVM call instruction
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

  /// Instruct - Original LLVM instruction.
  Instruction *Instruct;

  /// ThenBBlock - If conditional branch, pointer to the true successor block.
  BasicBlock *ThenBBlock;

  /// ElseBBlock - If conditional branch, pointer to the false successor block.
  BasicBlock *ElseBBlock;

protected:

  /// \brief Create a new branch from LLVM branch instruction In  and
  /// branch condition Cond (optional).
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

  /// Instruct - Original LLVM instruction
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

  /// Instruct - Original LLVM entry instruction.
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

  /// Instruct - Original LLVM return instruction
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

//----------------------------------------------------------------------------//
// AVR Select Node for LLVM
//----------------------------------------------------------------------------//
/// \brief An abstract vector select node. 
///
class AVRSelectIR : public AVRSelect {

private:

  /// Instruct - Originial LLVM Instruction
  Instruction *Instruct;

protected:

  AVRSelectIR(Instruction *Inst, AVRCompare *AComp);
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

//----------------------------------------------------------------------------//
// AVR Compare Node for LLVM
//----------------------------------------------------------------------------//
/// \brief An abstract vector select node. 
///
class AVRCompareIR : public AVRCompare {

private:

  /// Instruct - Originial LLVM select instruction
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


} // End VPO Vectorizer Namespace
} // End LLVM Namespace 

#endif  // LLVM_ANALYSIS_VPO_AVR_STMT_IR_H
