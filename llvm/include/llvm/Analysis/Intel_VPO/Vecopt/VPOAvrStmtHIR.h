//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This header file defines the Abstract Vector Representation (AVR) stmt
/// nodes for HIR
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_STMT_HIR_H
#define LLVM_ANALYSIS_VPO_AVR_STMT_HIR_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrStmt.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLIf.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNode.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"

using namespace llvm::loopopt;

namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Vectorizer Namespace

// forward declarations
class AVRIfHIR;

//----------AVR Assign Node for HIR----------//
/// \brief Assign node abstract vector representation for HIR.
///
/// See AVRAssign class for more information.
class AVRAssignHIR : public AVRAssign {

private:
  /// HLInst - Underlying HIR HLInst node for this Assignment
  HLInst *Instruct;

protected:
  /// \brief AVRAssignHIR Object Constructor.
  AVRAssignHIR(HLInst *Instr);

  /// \brief AVRAssignHIR Object Destructor.
  virtual ~AVRAssignHIR() override {}

  /// \brief Copy Constructor.
  AVRAssignHIR(const AVRAssignHIR &AVRAssignHIR);

  /// \brief Sets up state object.
  void initialize();

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsHIR;

public:
  /// \brief Returns the HIR Instruction
  HLInst *getHIRInstruction() { return Instruct; }

  /// \brief Clone methode for AVRAssignHIR.
  AVRAssignHIR *clone() const override;

  /// \brief Method for supporting type inquiry.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRAssignHIRNode;
  }

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;
};

//----------AVR Expression Node for HIR----------//
/// \brief Avr expression node for HIR.
///
/// See AVRExpression class for more information.
class AVRExpressionHIR : public AVRExpression {

private:
  /// HLAssign - HIR Inst node which this is expression is built from.
  HLNode *HIRNode;

  /// Opcode - The LLVM opcode for this expression.
  unsigned Opcode;

protected:
  /// \brief Constructs an AVRExpressionHIR given an AVRAssignHIR node and
  /// LHS/RHS specifier.
  AVRExpressionHIR(AVRAssignHIR *HLAssign, AssignOperand AOp);

  /// \brief Constructs an AVRExpressionHIR given an AVRIfHIR node and
  /// predicate iterator.
  AVRExpressionHIR(AVRIfHIR *AIf, HLIf::const_pred_iterator &PredIt);

  /// \brief Constructs an AND AVRExpressionHIR of two given HIR expressions.
  AVRExpressionHIR(AVRExpressionHIR *LHS, AVRExpressionHIR *RHS);

  /// \brief Constructs a binary AVRExpressionHIR from two given AVRs.
  AVRExpressionHIR(AVR *LHS, AVR *RHS, Type *Ty, unsigned Opcode);

  /// \brief Destructor for this object.
  virtual ~AVRExpressionHIR() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsHIR;

public:
  /// \brief Clone method for AVRExpressionHIR.
  AVRExpressionHIR *clone() const override;

  /// \brief Returns HIRNode for this expression.
  HLNode *getHIRNode() { return HIRNode; }

  /// \brief Method for supporting type inquiry.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRExpressionHIRNode;
  }

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;
};

//----------AVR Value Node for HIR----------//
/// \brief Avr Value node for HIR.
///
/// See AVRValue class for more information.
class AVRValueHIR : public AVRValue {

public:
  /// Information at AVR level for an AVRValueHIR that represents an IV
  struct IVValueInfo {
    // CanonExpr that contains the IV
    CanonExpr *CE;
    // Level of the IV in the CanonExpr
    unsigned Level;

    IVValueInfo(CanonExpr *CanonE, unsigned Lvl) : CE(CanonE), Level(Lvl) {}
  };

private:
  // The "value" of an AVRValueHIR can be a RegDDReg, a BlobDDRef, an
  // IVValueInfo representing an IV or a Constant. This is a temporal
  // implementation for prototyping.
  // We may want to have several types of AVRValueHIR (1 for RegDDRefs, 1 for
  // BlobDDRefs, 1 for IVs, 1 for Constants (this one at AVRValue level))
  // or just deal with this implementation where some fields can be nullptrs.

  /// Val - The HIR RegDDRef/BlobDDRef for this AVR Value
  DDRef *Val;

  /// IVVal - The IV information for this AVR Value
  IVValueInfo *IVVal = nullptr;

  /// HLNode - Underlying HLNode which produced this value.
  HLNode *HNode;

protected:
  /// \brief Constructs an AVRValueHIR node for the operand in HLInst node
  ///  specified by DDRef. /p isMemoryOperation indicates whether the operand
  ///  represents an address used in a memory operation (loads/store)
  AVRValueHIR(RegDDRef *DDRef, HLNode *Node, AVR *Parent,
              bool isMemoryOperation);

  /// \brief Constructs an AVRValueHIR node for an IV
  AVRValueHIR(IVValueInfo *IVV, Type *Ty, AVR *Parent);

  /// \brief Constructs an AVRValueHIR node for a BlobDDRef
  AVRValueHIR(BlobDDRef *DDRef, AVR *Parent);

  /// \brief Destructor for this object.
  virtual ~AVRValueHIR() {
    if (IVVal != nullptr)
      delete (IVVal);
  }

  /// \brief Sets the IV info for this node.
  void setIVValue(IVValueInfo *IVV) { IVVal = IVV; }

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsHIR;

public:
  /// \brief Clone method for AVRValueHIR.
  AVRValueHIR *clone() const override;

  /// \brief Method for supporting type inquiry.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRValueHIRNode;
  }

  /// \brief Prints the AVRAssignHIR node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns »the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Returns whether the AVRValueHIR represents a DDRef.Note that
  /// isIVValue() and isDDRefValue() are not mutually exclusive (a RegDDRef can
  /// be a standalone IV)
  bool isDDRefValue() { return Val != nullptr && IVVal == nullptr; }

  /// \brief Returns the RegDDRef associated with this node.
  DDRef *getValue() { return Val; }

  /// \brief Returns whether the AVRValueHIR represents an IV. Note that
  /// isIVValue() and isDDRefValue() are not mutually exclusive (a RegDDRef
  /// can be a standalone IV)
  bool isIVValue() { return IVVal != nullptr; }

  /// \brief Returns the IV info associated with this node.
  IVValueInfo *getIVValue() { return IVVal; }

  /// \brief Returns the HLNode associated with this node.
  HLNode *getNode() { return HNode; }
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
  const HLNode *getHIRInstruction() const { return Instruct; }

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
  // CHECKME: Is this going to be an HLNode other than a goto?
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

//----------AVR Unreachable Node for HIR----------//
/// \brief An abstract vector unreachable node for HIR.
class AVRUnreachableHIR : public AVRUnreachable {

private:
  /// \p Instruct - HIR node which genreated the unreachable avr.
  HLNode *Instruct;

protected:
  /// \brief AVRUnreachableHIR object constructor.
  AVRUnreachableHIR(HLNode *Inst);

  /// \brief Object destructor.
  virtual ~AVRUnreachableHIR() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtilsHIR;

public:
  /// \brief Returns unreachable instruction.
  const HLNode *getHIRInstruction() const { return Instruct; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRUnreachableHIRNode;
  }

  /// \brief Clone method for AVRUnreachable.
  AVRUnreachableHIR *clone() const override;
};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_ANALYSIS_VPO_AVR_STMT_HIR_H
