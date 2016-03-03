//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvr.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/IR/Instructions.h"

namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Vectorizer Namespace

// Forward Declarations
class AVRIf;
class AVRBranch;
class AVRLabel;
class AVRCompare;
class AVRExpression;

//----------AVR Assignment Node----------//
/// \brief This AVR node represents an assignment in the Abstract Layer.
///
/// An AVRAssign node is constructed when an incoming LLVM IR or HIR contiains
/// an assignment. During the abstract layer construction process this node
/// initially contains only a simple pointer to the underlying LLVM IR
/// instruction or HIR HLInst node.  As the abstract layer is optimized, this
/// node is updated more precise/granular information, such as pointers to RHS
/// and LHS nodes.
class AVRAssign : public AVR {

private:
  /// LHS - Left hand side operand of avr assignment. (Only valid after
  /// optimizeAVRExpressions())
  AVR *LHS;

  /// RHS - Right hand side operand of avr assignment. (Only valid after
  /// optimizeAVRExpressions())
  AVR *RHS;

  /// IsStore  - True is avr is store.
  bool IsStore;

protected:
  /// \brief AVRAssign Object Constructor.
  AVRAssign(unsigned SCID);

  /// \brief AVRAssign Object Destructor.
  virtual ~AVRAssign() override {}

  /// \brief Copy Constructor.
  AVRAssign(const AVRAssign &AVRAssign);

  /// \brief Sets up state object.
  void initialize();

  /// \brief Sets Node as the LHS avr of this assignment.
  void setLHS(AVR *Node) { LHS = Node; }

  /// \brief Sets Node as the RHS avr of this assignment.
  void setRHS(AVR *Node) { RHS = Node; }

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  /// \brief Clone method for AVRAssign.
  AVRAssign *clone() const override;

  /// \brief Retuns the left hand side of the avr assignment.
  AVR *getLHS() const { return LHS; }

  /// \brief Retuns the right hand side of the avr assignment.
  AVR *getRHS() const { return RHS; }

  /// \brief Returns true if LHS for this avr assignment node is not null.
  bool hasLHS() const { return LHS != nullptr; }

  /// \brief Returns true if RHS for this avr assignment node is not null.
  bool hasRHS() const { return RHS != nullptr; }

  /// \brief Method for supporting type inquiry.
  static bool classof(const AVR *Node) {
    return (Node->getAVRID() >= AVR::AVRAssignNode &&
            Node->getAVRID() < AVR::AVRAssignLastNode);
  }

  /// \brief Prints the AVRAssign node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const = 0;
};

//----------AVR Expression Node----------//
/// \brief This AVR node represents an expression in the Abstract Layer.
///
/// The AVRExpression node represents all unary, binary, and n-ary
/// expressions in the abstract layer. The initial build of the AL doesnt
//  not contain AVRExpressions.  This node is added to the Abstract
/// Layer as it is optimized.
class AVRExpression : public AVR {

protected:
  /// Operation - Operation which is executed on operands.
  unsigned Operation;

  /// Operands - Operands on which this operation executes.
  SmallVector<AVR *, 1> Operands;

  /// IsLHSExpr - True when this expression is the LHS of an assignment.
  bool IsLHSExpr;

  /// \brief Constructor used by derived classes. Should not instantiate
  /// this object at this level.
  AVRExpression(unsigned SCID);

  /// \brief Destructor for object.
  virtual ~AVRExpression() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  /// brief Clone method for AVRExpression.
  AVRExpression *clone() const override;

  /// \brief Returns the number of operands for this operator
  unsigned getNumOperands() const { return Operands.size(); }

  /// \brief Returns true if operator is binary.
  bool isBinaryOperation() const { return getNumOperands() == 2; }

  /// \brief Returns true if operator is binary.
  bool isUnaryOperation() const { return getNumOperands() == 1; }

  /// \brief Returns operation type.
  unsigned getOperation() { return Operation; }

  /// \brief Returns true is this expression is the LHS of an assignment.
  bool isLHSExpr() const { return IsLHSExpr; }

  /// \brief Returns the n-th (OpNum) operand of this expression.
  AVR *getOperand(unsigned OpNum) const { return Operands[OpNum]; }

  /// \brief Method for supporting type inquiry.
  static bool classof(const AVR *Node) {
    return (Node->getAVRID() >= AVR::AVRExpressionNode &&
            Node->getAVRID() < AVR::AVRExpressionLastNode);
  }

  /// \brief Prints the AVRAssignIR node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VerbosityLevel) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const = 0;

  /// \brief Returns the Opcode name of this expression's operation.
  virtual std::string getOpCodeName() const = 0;
};

//----------AVR Value Node----------//
/// \brief AVR node represents a value in the Abstract Layer.
///
/// The AVRValue node represents a value, found in the incoming IR. For
/// HIR, the AVRValue is constructed for each RegDDRef found in HLInst
/// nodes. For LLVM IR, the AVRValue nodes are constructed for each operand
/// of an LLVM instruction. AVRValue nodes are not built in the initial
/// Abstract Layer build, they are added later as part of an AL optimization.
class AVRValue : public AVR {

private:
  /// MemRefInfo - Holds memory reference infomation. Information is set via
  /// memory reference anaylsis or simd attributes. (To be added in future
  /// changest)
  // MemRefInfo *MRI;

protected:
  /// \brief Constructor used by derived classes. Should not instantiate
  /// this object at this level.
  AVRValue(unsigned SCID);

  /// \brief Destructor for this object.
  virtual ~AVRValue() override {}

  // Only the AVR Utility class should create these objects.
  friend class AVRUtils;

public:
  /// \brief Clone method for AVRValue.
  AVRValue *clone() const override;

  /// \brief Method for supporting type inquiry.
  static bool classof(const AVR *Node) {
    return (Node->getAVRID() >= AVR::AVRValueNode &&
            Node->getAVRID() < AVR::AVRValueLastNode);
  }

  /// \brief Prints the AVRAssignIR node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VerbosityLevel) const = 0;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns value name of this node.
  virtual std::string getAvrValueName() const = 0;
};

//----------AVR Label Node----------//
/// \brief TODO
class AVRLabel : public AVR {

private:
  /// Terminator - Terminator avr for this label.
  AVR *Terminator;

protected:
  AVRLabel(unsigned SCID);
  virtual ~AVRLabel() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  /// \brief Returns the AVRBranch terminator for this label
  AVR *getTerminator() { return Terminator; }

  /// \brief Sets the terminator (AVRBranch) for this label
  void setTerminator(AVR *AB) { Terminator = AB; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return (Node->getAVRID() >= AVR::AVRLabelNode &&
            Node->getAVRID() < AVR::AVRLabelLastNode);
  }

  AVRLabel *clone() const override;

  /// \brief Prints the AVRLabel node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const = 0;
};

//----------AVR Expr Node----------//
/// \brief TODO
class AVRExpr : public AVR {};

//----------AVR Phi Node----------//
/// \brief TODO
class AVRPhi : public AVR {

protected:
  AVRPhi(unsigned SCID);
  virtual ~AVRPhi() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return (Node->getAVRID() >= AVR::AVRPhiNode &&
            Node->getAVRID() < AVR::AVRPhiLastNode);
  }

  AVRPhi *clone() const override;

  /// \brief Prints the AVRPhi node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const = 0;
};

//----------AVR Call Node----------//
/// \brief An abstract vector call node.
class AVRCall : public AVR {

private:
  // TODO: Add Member Data

protected:
  AVRCall(unsigned SCID);
  virtual ~AVRCall() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

  // TODO: Add Member Functions
public:
  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return (Node->getAVRID() >= AVR::AVRCallNode &&
            Node->getAVRID() < AVR::AVRCallLastNode);
  }

  AVRCall *clone() const override;

  /// \brief Prints the AVRCall node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const = 0;
};

//----------------------------------------------------------------------------//
// AVR Branch Node
//----------------------------------------------------------------------------//
/// \brief An abstract vector branch node. AVRBranches are generated for
/// conditional, non-conditional, and indirect branches. For loop backedge
/// branches we also generate AVRBranch.
class AVRBranch : public AVR {

private:
  /// IsConditional - True if branch is conditional
  bool IsConditional;

  /// IsIndirect - True if branch is indirect.
  bool IsIndirect;

  /// BottomTest - True when branch is loop's bottom test branch
  bool IsBottomTest;

  /// Condition - If conditional branch, pointer to the AVR which generates the
  /// true/false bit for conditional branch.
  AVR *Condition;

  // TODO: Consolidate Successors/ThenBBlock/ElseBBlock.
  /// Successors - Vector containing avr labels which are the labels of the
  /// successor basic blocks of this branch.
  SmallVector<AVRLabel *, 2> Successors;

protected:
  /// \brief Create a new non-conditional branch to AVRLabel AL.
  AVRBranch(AVRLabel *AL);

  /// \brief Subclass constructor
  AVRBranch(unsigned SCID, bool IsInd, AVR *Cond);

  /// \brief Subclass constructor
  AVRBranch(unsigned SCID);

  virtual ~AVRBranch() override {}

  /// \brief Sets the conditional branch flag.
  void setIsConditional(bool IC) { IsConditional = IC; }

  /// \brief Sets the Avr Condition node for a conditional branch.
  void setCondition(AVR *Cond) { Condition = Cond; }

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  /// \brief Add the AVRLabel of a successor BBlock to successor vector.
  void addSuccessor(AVRLabel *AL) { Successors.push_back(AL); }

  /// \brief Returns number of successors to this branch.
  unsigned getNumSuccessors() const { return Successors.size(); }

  /// \brief Returns the condition avr for conditional branch.
  AVR *getCondition() { return Condition; }

  /// \brief Returns true if the forward branch is conditional
  bool isConditional() { return IsConditional; }

  /// \brief Sets Bottom Test
  void setBottomTest(bool BT) { IsBottomTest = BT; }

  /// \brief Returns true is branch is a BottomTest
  bool isBottomTest() const { return IsBottomTest; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return (Node->getAVRID() >= AVR::AVRBranchNode &&
            Node->getAVRID() < AVR::AVRBranchLastNode);
  }

  AVRBranch *clone() const override;

  /// \brief Prints the AVRBranch node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;
};

//----------AVR Backedge Node----------//
/// \brief An abstract vector backedge node.
class AVRBackEdge : public AVR {

private:
  // TODO: Add Member Data

protected:
  AVRBackEdge(unsigned SCID);
  virtual ~AVRBackEdge() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  // TODO: Add Member Functions

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return (Node->getAVRID() >= AVR::AVRBackEdgeNode &&
            Node->getAVRID() < AVR::AVRBackEdgeLastNode);
  }

  AVRBackEdge *clone() const override;

  /// \brief Prints the AVRBackEdge node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const = 0;
};

//----------AVR Entry Node----------//
/// \brief An abstract vector entry node.
class AVREntry : public AVR {

private:
  // TODO: Add Member Data

protected:
  AVREntry(unsigned SCID);
  virtual ~AVREntry() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

  // TODO: Add Member Functions
public:
  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return (Node->getAVRID() >= AVR::AVREntryNode &&
            Node->getAVRID() < AVR::AVREntryLastNode);
  }

  AVREntry *clone() const override;

  /// \brief Prints the AVREntry node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const = 0;
};

//----------AVR Return Node----------//
/// \brief An abstract vector return node.
class AVRReturn : public AVR {

private:
  // TODO: Add Member Data

protected:
  AVRReturn(unsigned SCID);
  virtual ~AVRReturn() override {}

  // TODO: Add Member Functions
  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return (Node->getAVRID() >= AVR::AVRReturnNode &&
            Node->getAVRID() < AVR::AVRReturnLastNode);
  }

  AVRReturn *clone() const override;

  /// \brief Prints the AVRReturn node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const = 0;
};

//----------------------------------------------------------------------------//
// AVR Select Node
//----------------------------------------------------------------------------//
/// \brief An abstract vector select node.
///
class AVRSelect : public AVR {

protected:

  /// Condition - The avr generated for the condition of this select.
  AVR *Condition;

  /// \brief Object constructor.
  AVRSelect(unsigned SCID);

  /// \brief Destructor for object.
  virtual ~AVRSelect() override {}

  /// \brief Sets the condition avr for this select node. 
  void setCondition(AVR *Cond) { Condition = Cond; }

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return (Node->getAVRID() >= AVR::AVRSelectNode &&
            Node->getAVRID() < AVR::AVRSelectLastNode);
  }

  AVRSelect *clone() const override;

  /// \brief Prints the AVRSelect node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const = 0;
};

//----------------------------------------------------------------------------//
// AVR Compare Node
//----------------------------------------------------------------------------//
/// \brief An abstract vector compare node. During first traversal of LLVM IR,
/// Vectorizer generates AVRCompare nodes for icmp/fcmp LLVM IR instructions
/// encountered.  Later in AVR optimization, this AVR Cmpare is converted to
/// AVRIf.
//
class AVRCompare : public AVR {

private:
  /// Select - AVR Select generated for this compare
  AVRSelect *Select;

  /// IsCompareSelect - True if compare is part of compare-select sequence.
  bool IsCompareSelect;

  /// AVR Branch generated for this compare
  AVRBranch *Branch;

protected:
  AVRCompare(unsigned SCID);
  virtual ~AVRCompare() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  /// \brief Sets the AVRBranch for this compare
  void setBranch(AVRBranch *BR) { Branch = BR; }

  /// \brief Sets IsCompareSelect bit.
  void setIsCompareSelect(bool CS) { IsCompareSelect = CS; }

  /// \brief Sets the AVRSelect for this compare node.
  void setSelect(AVRSelect *ASelect) { Select = ASelect; }

  /// \brief Returns true is this compare has a select
  bool isCompareSelect() { return IsCompareSelect; }

  /// \brief Returns the AvrSelect for this compare.
  AVRSelect *getSelect() { return Select; }

  /// \brief Returns branch generated for this compare
  AVRBranch *getBranch() { return Branch; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return (Node->getAVRID() >= AVR::AVRCompareNode &&
            Node->getAVRID() < AVR::AVRCompareLastNode);
  }

  AVRCompare *clone() const override;

  /// \brief Prints the AVRCompare node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const = 0;
};

//----------------------------------------------------------------------------//
// AVR Wrn Node
//----------------------------------------------------------------------------//
/// \brief An abstract vector information node. This node holds information
/// such as LoopInfo, BBSets, Goto Labels, plus more for a given AVR nest. This
/// type of node is generated for each incoming vector candidate loop nest. It
/// is currently used as a temporary placeholder for information until AVRLoop
/// nodes are generated and info can be set in that node.
//
class AVRWrn : public AVR {

public:
  // Setup types for AVRWrn's children and WRegionSimd's BBset.
  typedef AVRContainerTy ChildrenTy;

  /// Iterators to iterate for children nodes.
  typedef ChildrenTy::iterator child_iterator;
  typedef ChildrenTy::const_iterator const_child_iterator;
  typedef ChildrenTy::reverse_iterator reverse_child_iterator;
  typedef ChildrenTy::const_reverse_iterator const_reverse_child_iterator;

  /// Iterators to iterate for children nodes.
  typedef WRegionBBSetTy::iterator wrnbbset_iterator;
  typedef WRegionBBSetTy::const_iterator const_wrnbbset_iterator;
  typedef WRegionBBSetTy::reverse_iterator reverse_wrnbbset_iterator;
  typedef WRegionBBSetTy::const_reverse_iterator
      const_reverse_wrnbbset_iterator;

private:
  /// Pointer to the WRNVecLoopNode generated from WRN node creatation. Contains
  /// information needed to build AVRLoops.
  WRNVecLoopNode *WRegionSimdNode;

  /// Children of AVRInfo node is the entire candidate vector loop nest
  /// associated with this LoopInfo.
  ChildrenTy Children;

protected:
  AVRWrn(WRNVecLoopNode *WrnSimdNode);
  virtual ~AVRWrn() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  /// \brief Returns LoopInfo from WrnSimdNode
  LoopInfo *getLoopInfo() { return WRegionSimdNode->getLoopInfo(); }

  /// \brief Return WRegionSimdNode
  WRNVecLoopNode *getWrnNode() const { return WRegionSimdNode; }

  /// \brief Returns Entry Basic Block of WRN Region
  BasicBlock *getEntryBBlock() const {
    return WRegionSimdNode->getEntryBBlock();
  }
  /// \brief Returns Entry Basic Block of WRN Region
  BasicBlock *getExitBBlock() const { return WRegionSimdNode->getExitBBlock(); }

  /// \brief Return SIMD vector length
  int getSimdVectorLength() const {
    return getWrnNode()->getSimdlen();
  }

  // Methods for accessing and modifying WRegionSimdNode's basic block set.

  /// Iterators to traverse the WRegionSimdNode's basic block set.
  wrnbbset_iterator wrnbbset_begin() { return WRegionSimdNode->bbset_begin(); }
  const_wrnbbset_iterator wrnbbset_begin() const {
    return WRegionSimdNode->bbset_begin();
  }
  wrnbbset_iterator wrnbbset_end() { return WRegionSimdNode->bbset_end(); }
  const_wrnbbset_iterator wrnbbset_end() const {
    return WRegionSimdNode->bbset_end();
  }
  reverse_wrnbbset_iterator wrnbbset_rbegin() {
    return WRegionSimdNode->bbset_rbegin();
  }
  const_reverse_wrnbbset_iterator wrnbbset_rbegin() const {
    return WRegionSimdNode->bbset_rbegin();
  }
  reverse_wrnbbset_iterator wrnbbset_rend() {
    return WRegionSimdNode->bbset_rend();
  }
  const_reverse_wrnbbset_iterator wrnbbset_rend() const {
    return WRegionSimdNode->bbset_rend();
  }

  /// \brief On-demand populate WRegionSimdNode's basic block set with
  /// given EntryBB and ExitBB.
  void populateWrnBBSet() { WRegionSimdNode->populateBBSet(); }

  /// \brief Returns the number of Basic Blocks in WRNRegionSimd BBlockSet.
  unsigned getBBSetSize() const { return WRegionSimdNode->getBBSetSize(); }

  /// \brief Returns true if the WRNSimdNode Basic Block Set is emtpy.
  bool isWrnBBSetEmpty() const { return WRegionSimdNode->isBBSetEmpty(); }

  /// \Brief Set the WRegionSimdNode BBlockSet pointer to null.
  void clearBBSet() { return WRegionSimdNode->resetBBSet(); }

  // Methods for access this AVRWrn's Children

  /// Iterators to traverse AVRWrn's Children nodes.
  child_iterator child_begin() { return Children.begin(); }
  const_child_iterator child_begin() const { return Children.begin(); }
  child_iterator child_end() { return Children.end(); }
  const_child_iterator child_end() const { return Children.end(); }

  reverse_child_iterator child_rbegin() { return Children.rbegin(); }
  const_reverse_child_iterator child_rbegin() const {
    return Children.rbegin();
  }
  reverse_child_iterator child_rend() { return Children.rend(); }
  const_reverse_child_iterator child_rend() const { return Children.rend(); }

  AVR *getFirstChild();

  /// \brief Returns the first child if it exists, otherwise returns null.
  const AVR *getFirstChild() const {
    return const_cast<AVRWrn *>(this)->getFirstChild();
  }

  /// \brief Returns the last child if it exists, otherwise returns null.
  AVR *getLastChild();

  /// \brief Returns const pointer to last child if it exisits.
  const AVR *getLastChild() const {
    return const_cast<AVRWrn *>(this)->getLastChild();
  }

  /// \brief Returns the number of children.
  unsigned getNumChildren() const { return Children.size(); }

  /// \brief Returns true if it has children.
  bool hasChildren() const { return !Children.empty(); }

  /// \brief Sets the AVR Loop Node
  void setAvrLoop(AVRLoop *AvrLoop) {}

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRWrnNode;
  }

  /// \brief Clones the current AVRWrn node.
  AVRWrn *clone() const override;

  /// \brief Prints the AVRWrn node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Code generation for AVR Return.
  void codeGen();
};

//----------AVR NOP Node----------//
/// \brief An abstract vector NO-OP node. This avr node is a no operation node.
/// It's use is to simplify abstract layer node insertion and deletion. NOP
/// nodes should be removed before idiom recognition.
class AVRNOP : public AVR {

protected:

  AVRNOP();
  virtual ~AVRNOP() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRNOPNode;
  }

  AVRNOP *clone() const override;

  /// \brief Prints the AVRNOP node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

};

//----------AVR Unreachable Node----------//
/// \brief An abstract vector unreachable node. This avr node represents an
/// unreachable instruction.
class AVRUnreachable : public AVR {

protected:

  /// \brief AVRUnreachable object constructor. This object should only be
  /// instantiated from its derived classes.
  AVRUnreachable(unsigned SCID);

  /// \brief Virtual object destructor.
  virtual ~AVRUnreachable() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return (Node->getAVRID() >= AVR::AVRUnreachableNode &&
            Node->getAVRID() < AVR::AVRUnreachableLastNode);
  }

  /// \brief Clone method for AVRUnreachable.
  AVRUnreachable *clone() const override;

  /// \brief Prints the AVR Unreachable node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_ANALYSIS_VPO_AVR_STMT_H
