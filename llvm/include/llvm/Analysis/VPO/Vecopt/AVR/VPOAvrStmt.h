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
//   VPOAvrStmt.h -- Defines the Abstract Vector Representation (AVR) stmt nodes
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_STMT_H
#define LLVM_ANALYSIS_VPO_AVR_STMT_H

#include "llvm/IR/Instruction.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvr.h"

namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Vectorizer Namespace

// Eric: Think about this.
// TODO: Need to combine Call, Assign, Label, Phi, Fbranch, BackEdge, Entry,
//       and Return into derived classes of AVR STMT.
//   Ex:  AVR (Base Class) -> AVR-STMT
//                              |-> AVR Assign (derived from STMT)
//                              |-> AVR Label (derived from STMT)
//                              |-> AVR Phi (derived from STMT)
//                              |-> AVR Fbranch (derived from STMT)
//                              |-> AVR BackEdge (derived from STMT)
//                              |-> AVR Entry (derived from STMT)
//                              |-> AVR Return  (derived from STMT)
//                              |-> Anything else? 

// TODO: Alternatively, one single AVR_STMT CLass with bit set to determine
//       what kind/type.


//----------AVR Assign Node----------//
/// \brief Assign node abstract vector representation
///
/// An AVRAssign node represents an assignment found in LLVM IR or LoopOpt HIR.
class AVRAssign : public AVR {

private:
  Instruction *Instruct;

protected:
  /// \brief AVRAssign Object Constructor.
  AVRAssign(Instruction *Instr);

  /// \brief AVRAssign Object Destructor.
  ~AVRAssign();

  /// \brief Copy Constructor. 
  AVRAssign (const AVRAssign &AVRAssign);

  /// \bried Sets up state object.
  void initialize();

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:

  /// \brief Returns the LLVM Instruction
  const Instruction *getLLVMInstruction() const { return Instruct; }

  /// \brief Returns the specified operand value.
  const Value *getOperand(unsigned OperandNumber);

  /// \brief Returns the number of operands for this instruction.
  unsigned getNumOperands() const;

  AVRAssign *clone() const override;

  /// \brief Method for supporting type inquiry.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRAssignNode;
  }

  /// \brief Print Method for AVR Assign.
  void print() const override;

  void dump() const override;

  /// \brief Code generation for AVR Assign.
  void codeGen() override;

};

//----------AVR Label Node----------//
/// \brief TODO
class AVRLabel : public AVR {

private:
  BasicBlock *SourceBlock;

protected:

  AVRLabel(BasicBlock *SourceB);
  ~AVRLabel() {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  ///\brief returns BasicBlock assoicated with this Label.
  BasicBlock *getSourceBBlock () const { return SourceBlock; }

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRLabelNode;
  }

  AVRLabel *clone() const override;

  void print() const override;
  void dump() const override;

  /// \brief Code generation for AVR Label.
  void codeGen() override;

};

//----------AVR Expr Node----------//
/// \brief TODO
class AVRExpr : public AVR{

};

//----------AVR Phi Node----------//
/// \brief TODO
class AVRPhi : public AVR {

private:

  /// \brief Pointer to original LLVM Instruction
  Instruction *Instruct;

  // TODO: Add member data

protected:

  AVRPhi(Instruction *Inst);
  ~AVRPhi();

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  // TODO: Add member functions

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRPhiNode;
  }

  AVRPhi *clone() const override;
  void print() const override;
  void dump() const override;

  /// \brief Code generation for AVR Phi.
  void codeGen() override;

};


//----------AVR Call Node----------//
/// \brief An abstract vector call node.
class AVRCall : public AVR {

private:
  Instruction *Instruct;
  // TODO: Add Member Data

protected:
  AVRCall(Instruction *Inst);
  ~AVRCall();

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

  // TODO: Add Member Functions
public:

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRCallNode;
  }

  AVRCall *clone() const override;
  void print() const override;
  void dump() const override;

  /// \brief Code generation for AVR Call.
  void codeGen() override;

};

//----------AVR Fbranch Node----------//
/// \brief An abstract vector forward branch node.
class AVRFBranch : public AVR {

private:
  Instruction *Instruct;
  // TODO: Add Member Data

protected:
  AVRFBranch(Instruction *Inst);
  ~AVRFBranch();

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  // TODO: Add Member Functions

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRFBranchNode;
  }

  AVRFBranch *clone() const override;
  void print() const override;
  void dump() const override;

  /// \brief Code generation for AVR Forward branch.
  void codeGen() override;

};


//----------AVR Backedge Node----------//
/// \brief An abstract vector backedge node.
class AVRBackEdge : public AVR {

private:
  Instruction *Instruct;
  // TODO: Add Member Data

protected:
  AVRBackEdge(Instruction *Inst);
  ~AVRBackEdge();

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  // TODO: Add Member Functions

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRBackEdgeNode;
  }

  AVRBackEdge *clone() const override;
  void print() const override;
  void dump() const override;

  /// \brief Code generation for AVR Backedge.
  void codeGen() override;

};


//----------AVR Entry Node----------//
/// \brief An abstract vector entry node.
class AVREntry : public AVR {

private:
  Instruction *Instruct;
  // TODO: Add Member Data

protected:
  AVREntry(Instruction *Inst);
  ~AVREntry();

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

  // TODO: Add Member Functions
public:
  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVREntryNode;
  }

  AVREntry *clone() const override;
  void print() const override;
  void dump() const override;

  /// \brief Code generation for AVR Entry.
  void codeGen() override;

};

//----------AVR Return Node----------//
/// \brief An abstract vector return node.
class AVRReturn : public AVR {

private:
  Instruction *Instruct;
  // TODO: Add Member Data

protected:
  AVRReturn(Instruction *Inst);
  ~AVRReturn();

  // TODO: Add Member Functions
  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRReturnNode;
  }

  AVRReturn *clone() const override;
  void print() const override;
  void dump() const override;

  /// \brief Code generation for AVR Return.
  void codeGen() override;

};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace 

#endif  // LLVM_ANALYSIS_VPO_AVR_STMT_H
