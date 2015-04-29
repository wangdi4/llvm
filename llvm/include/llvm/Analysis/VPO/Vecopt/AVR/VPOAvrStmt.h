//===------------- VectorAVRStmt.h - AVR Loop Node---------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the Vectorizer's AVR Stmt node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_STMT_H
#define LLVM_ANALYSIS_VPO_AVR_STMT_H

#include "llvm/IR/Instruction.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvr.h"

namespace intel { // VPO Vectorizer Namespace

using namespace llvm;

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

public:

  /// \brief AVRAssign Object Constructor.
  AVRAssign(Instruction *Instr);

  /// \brief AVRAssign Object Destructor.
  ~AVRAssign();

  /// \brief Copy Constructor. 
  AVRAssign (const AVRAssign &AVRAssign);

  /// \bried Sets up state object.
  void initialize();

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

};

//----------AVR Label Node----------//
/// \brief TODO
class AVRLabel : public AVR {

private:
  BasicBlock *SourceBlock;

public:

  AVRLabel(BasicBlock *SourceB);
  ~AVRLabel() {}

  ///\brief returns BasicBlock assoicated with this Label.
  BasicBlock *getSourceBBlock () const { return SourceBlock; }

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRLabelNode;
  }

  AVRLabel *clone() const override;

  void print() const override;
  void dump() const override;

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

public:

  AVRPhi(Instruction *Inst);
  ~AVRPhi();

  // TODO: Add member functions

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRPhiNode;
  }

  AVRPhi *clone() const override;
  void print() const override;
  void dump() const override;
};


//----------AVR Call Node----------//
/// \brief An abstract vector call node.
class AVRCall : public AVR {

private:
  Instruction *Instruct;
  // TODO: Add Member Data

public:
  AVRCall(Instruction *Inst);
  ~AVRCall();

  // TODO: Add Member Functions

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRCallNode;
  }

  AVRCall *clone() const override;
  void print() const override;
  void dump() const override;

};

//----------AVR Fbranch Node----------//
/// \brief An abstract vector forward branch node.
class AVRFBranch : public AVR {

private:
  Instruction *Instruct;
  // TODO: Add Member Data

public:
  AVRFBranch(Instruction *Inst);
  ~AVRFBranch();

  // TODO: Add Member Functions

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRFBranchNode;
  }

  AVRFBranch *clone() const override;
  void print() const override;
  void dump() const override;

};


//----------AVR Backedge Node----------//
/// \brief An abstract vector backedge node.
class AVRBackEdge : public AVR {

private:
  Instruction *Instruct;
  // TODO: Add Member Data

public:
  AVRBackEdge(Instruction *Inst);
  ~AVRBackEdge();

  // TODO: Add Member Functions

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRBackEdgeNode;
  }

  AVRBackEdge *clone() const override;
  void print() const override;
  void dump() const override;

};


//----------AVR Entry Node----------//
/// \brief An abstract vector entry node.
class AVREntry : public AVR {

private:
  Instruction *Instruct;
  // TODO: Add Member Data

public:
  AVREntry(Instruction *Inst);
  ~AVREntry();

  // TODO: Add Member Functions

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVREntryNode;
  }

  AVREntry *clone() const override;
  void print() const override;
  void dump() const override;

};

//----------AVR Return Node----------//
/// \brief An abstract vector return node.
class AVRReturn : public AVR {

private:
  Instruction *Instruct;
  // TODO: Add Member Data

public:
  AVRReturn(Instruction *Inst);
  ~AVRReturn();

  // TODO: Add Member Functions

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRReturnNode;
  }

  AVRReturn *clone() const override;
  void print() const override;
  void dump() const override;

};


}  // End VPO Vectorizer Namespace
#endif  // LLVM_ANALYSIS_VPO_AVR_STMT_H
