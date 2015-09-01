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
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"

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
  virtual ~AVRAssign() override {}

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

  /// \brief Prints the AVRAssign node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             unsigned VerbosityLevel) const override;

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
  virtual ~AVRLabel() override {}

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

  /// \brief Prints the AVRLabel node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             unsigned VerbosityLevel) const override;

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
  virtual ~AVRPhi() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  // TODO: Add member functions

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRPhiNode;
  }

  AVRPhi *clone() const override;

  /// \brief Prints the AVRPhi node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             unsigned VerbosityLevel) const override;

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
  virtual ~AVRCall() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

  // TODO: Add Member Functions
public:

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRCallNode;
  }

  AVRCall *clone() const override;

  /// \brief Prints the AVRCall node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             unsigned VerbosityLevel) const override;

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
  virtual ~AVRFBranch() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  // TODO: Add Member Functions

  /// \brief Returns FBranch Instruction
  const Instruction *getLLVMInstruction() const { return Instruct; }

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRFBranchNode;
  }

  AVRFBranch *clone() const override;

  /// \brief Prints the AVRFBranch node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             unsigned VerbosityLevel) const override;

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
  virtual ~AVRBackEdge() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  // TODO: Add Member Functions

  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRBackEdgeNode;
  }

  AVRBackEdge *clone() const override;

  /// \brief Prints the AVRBackEdge node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             unsigned VerbosityLevel) const override;

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
  virtual ~AVREntry() override {}

  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

  // TODO: Add Member Functions
public:
  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVREntryNode;
  }

  AVREntry *clone() const override;

  /// \brief Prints the AVREntry node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             unsigned VerbosityLevel) const override;

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
  virtual ~AVRReturn() override {}

  // TODO: Add Member Functions
  /// Only this utility class should be used to modify/delete AVR nodes.
  friend class AVRUtils;

public:
  ///\brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRReturnNode;
  }

  AVRReturn *clone() const override;

  /// \brief Prints the AVRReturn node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             unsigned VerbosityLevel) const override;

  /// \brief Code generation for AVR Return.
  void codeGen() override;

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
  typedef WRegionBSetTy::iterator wrnbbset_iterator;
  typedef WRegionBSetTy::const_iterator const_wrnbbset_iterator;
  typedef WRegionBSetTy::reverse_iterator reverse_wrnbbset_iterator;
  typedef WRegionBSetTy::const_reverse_iterator
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
  WRNVecLoopNode *getWrnNode() { return WRegionSimdNode; }

  /// \brief Returns Entry Basic Block of WRN Region
  BasicBlock *getEntryBBlock() const {
    return WRegionSimdNode->getEntryBBlock();
  }
  /// \brief Returns Entry Basic Block of WRN Region
  BasicBlock *getExitBBlock() const {
     return WRegionSimdNode->getExitBBlock();
  }

  // Methods for accessing and modifying WRegionSimdNode's basic block set.

  /// Iterators to traverse the WRegionSimdNode's basic block set.
  wrnbbset_iterator wrnbbset_begin() {
    return WRegionSimdNode->bbset_begin();
  }
  const_wrnbbset_iterator wrnbbset_begin() const {
    return WRegionSimdNode->bbset_begin();
  }
  wrnbbset_iterator wrnbbset_end() {
    return WRegionSimdNode->bbset_end();
  }
  const_wrnbbset_iterator wrnbbset_end() const {
     return WRegionSimdNode->bbset_end();
  }
  reverse_wrnbbset_iterator wrnbbset_rbegin() {
   return  WRegionSimdNode->bbset_rbegin();
  }
  const_reverse_wrnbbset_iterator wrnbbset_rbegin() const {
    return  WRegionSimdNode->bbset_rbegin();
  }
  reverse_wrnbbset_iterator wrnbbset_rend() {
    return  WRegionSimdNode->bbset_rend();
  }
  const_reverse_wrnbbset_iterator wrnbbset_rend() const {
    return  WRegionSimdNode->bbset_rend();
  }

  /// \brief On-demand populate WRegionSimdNode's basic block set with
  /// given EntryBB and ExitBB. 
  void populateWrnBBSet() { WRegionSimdNode->populateBBlockSet(); }

  /// \brief Returns the number of Basic Blocks in WRNRegionSimd BBlockSet.
  unsigned getBBSetSize() const { return WRegionSimdNode->getBBSetSize(); }

  /// \brief Returns true if the WRNSimdNode Basic Block Set is emtpy.
  bool isWrnBBSetEmpty() const {
    return WRegionSimdNode->isBasicBlockSetEmpty();
  }

  /// \Brief Set the WRegionSimdNode BBlockSet pointer to null.
  void clearBBSet() { return WRegionSimdNode->resetBBSetPtr(); }

  // Methods for access this AVRWrn's Children

  /// Iterators to traverse AVRWrn's Children nodes.
  child_iterator child_begin() { return Children.begin(); }
  const_child_iterator child_begin() const { return Children.begin(); }
  child_iterator child_end() { return Children.end(); }
  const_child_iterator child_end() const { return Children.end(); }

  reverse_child_iterator child_rbegin() { return Children.rbegin(); }
  const_reverse_child_iterator child_rbegin() const { return Children.rbegin(); }
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
             unsigned VerbosityLevel) const override;

  /// \brief Code generation for AVR Return.
  void codeGen() override;

};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace 

#endif  // LLVM_ANALYSIS_VPO_AVR_STMT_H
