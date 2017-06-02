//===----------- HLNode.h - High level IR node ------------------*- C++ -*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the High level IR node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_HLNODE_H
#define LLVM_IR_INTEL_LOOPIR_HLNODE_H

#include "llvm/ADT/simple_ilist.h"

#include "llvm/Support/ErrorHandling.h"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Operator.h"

#include "llvm/IR/Intel_LoopIR/HLNodeMapper.h"

#include <set>

namespace llvm {

class formatted_raw_ostream;

namespace loopopt {

class HLNode;
class HLGoto;
class HLLabel;
class HLLoop;
class HLRegion;
class HLNodeUtils;
class DDRefUtils;
class CanonExprUtils;
class BlobUtils;

// Typedef for a list of HLNodes.
typedef simple_ilist<HLNode> HLContainerTy;

// Container for Goto's
typedef SmallVector<HLGoto *, 16> GotoContainerTy;

// Map for Old Label and New Label
typedef SmallDenseMap<const HLLabel *, HLLabel *, 16> LabelMapTy;

// Defining predicate type
typedef CmpInst::Predicate PredicateTy;
const PredicateTy UNDEFINED_PREDICATE = PredicateTy::BAD_FCMP_PREDICATE;

struct HLPredicate {
  PredicateTy Kind;
  FastMathFlags FMF;
  DebugLoc DbgLoc;

  HLPredicate() {}
  HLPredicate(PredicateTy Kind, FastMathFlags FMF = FastMathFlags(),
              const DebugLoc &Loc = DebugLoc())
      : Kind(Kind), FMF(FMF), DbgLoc(Loc) {
    assert((!FMF.any() || CmpInst::isFPPredicate(Kind)) &&
           "FastMathFlags are set on non-FP predicate");
  }

  bool operator==(PredicateTy Kind) const { return this->Kind == Kind; }
  bool operator!=(PredicateTy Kind) const { return !(*this == Kind); }
  bool operator==(const HLPredicate &Pred) const { return *this == Pred.Kind; }
  bool operator!=(const HLPredicate &Pred) const { return !(*this == Pred); }

  operator PredicateTy() const { return Kind; }
};

/// \brief High level IR node base class
///
/// This represents a node of the High level IR. It is used to represent
/// the incoming LLVM IR in program/lexical order.
///
/// This class (hierarchy) disallows creating objects on stack.
/// Objects are created/destroyed using HLNodeUtils friend class.
class HLNode : public ilist_node<HLNode> {

private:
  /// \brief Make class uncopyable.
  void operator=(const HLNode &) = delete;

  /// Reference to parent utils object. This is needed to access util functions.
  HLNodeUtils &HNU;

  /// ID to differentiate between concrete subclasses.
  const unsigned char SubClassID;

  /// Lexical parent of HLNode.
  HLNode *Parent;

  /// Unique number associated with HLNodes.
  unsigned Number;

  /// Topological sort number of HLNode.
  unsigned TopSortNum;

  /// Maximum topological sort number of HLNode across its children.
  unsigned MaxTopSortNum;

  /// \brief Sets the number of this node in the topological sort order.
  void setTopSortNum(unsigned Num) { TopSortNum = Num; }

  /// \brief Sets the maximum sort number of HLNode across its children.
  void setMaxTopSortNum(unsigned Num) {
    MaxTopSortNum = Num;
    if (HLNode *Parent = getParent()) {
      if (Parent->getMaxTopSortNum() < Num) {
        Parent->setMaxTopSortNum(Num);
      }
    }
  }

  /// \brief Virtual Clone Implementation
  /// This function populates the GotoList with Goto branching within the
  /// region and LabelMap with Old and New Labels.
  virtual HLNode *cloneImpl(GotoContainerTy *GotoList, LabelMapTy *LabelMap,
                            HLNodeMapper *NodeMapper) const = 0;

  /// Implements getPrevNode()/getNextNode().
  HLNode *getPrevNextNodeImpl(bool Prev);

protected:
  HLNode(HLNodeUtils &HNU, unsigned SCID);
  HLNode(const HLNode &HLNodeObj);
  virtual ~HLNode() {}

  friend class HLNodeUtils;

  /// IndentWidth used to print HLNodes.
  static const unsigned IndentWidth = 3;

  /// \brief Sets the lexical parent of this HLNode.
  void setParent(HLNode *Par) { Parent = Par; }

  /// \brief Returns true if Pred is TRUE or FALSE.
  static bool isPredicateTrueOrFalse(PredicateTy Pred) {
    return Pred == PredicateTy::FCMP_TRUE || Pred == PredicateTy::FCMP_FALSE;
  }

  /// Prints fast math flags.
  static void printFMF(raw_ostream &OS, FastMathFlags FMF);

  /// \brief Pretty prints predicates.
  static void printPredicate(formatted_raw_ostream &OS, PredicateTy Pred);

  /// \brief Base Clone Implementation
  /// This is the protected base clone implementation as the subclasses cannot
  /// directly call the cloneImpl of other subclasses.
  /// For e.g. Loop->cloneBaseImpl(child, GL, LM) will return child clone.
  static HLNode *cloneBaseImpl(const HLNode *Node, GotoContainerTy *GotoList,
                               LabelMapTy *LabelMap, HLNodeMapper *NodeMapper);

  /// \brief Returns the parent region of this node, if one exists, else returns
  /// null.
  HLRegion *getParentRegionImpl() const;

public:
  /// Returns parent HLNodeUtils object.
  HLNodeUtils &getHLNodeUtils() const { return HNU; }

  /// Returns DDRefUtils object.
  DDRefUtils &getDDRefUtils() const;

  /// Returns CanonExprUtils object.
  CanonExprUtils &getCanonExprUtils() const;

  /// Returns BlobUtils object.
  BlobUtils &getBlobUtils() const;

  /// Virtual Clone Method
  /// If \p NodeMapper is not null, every node will be mapped to the cloned
  /// node. This is used for accessing clones having original node pointers.
  virtual HLNode *clone(HLNodeMapper *NodeMapper = nullptr) const;

  /// \brief Dumps HLNode.
  void dump() const;

  /// \brief Dumps HLNode with details.
  void dump(bool Detailed) const;

  /// \brief Indents nodes for printing.
  void indent(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief Prints HLNode with details.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed = false) const = 0;

  /// Returns true if this node is attached to a HIR region.
  bool isAttached() const { return getParentRegionImpl(); }

  /// \brief Returns the immediate lexical parent of the HLNode.
  HLNode *getParent() const { return Parent; }

  /// \brief Returns the parent loop of this node, if one exists.
  HLLoop *getParentLoop() const;

  /// \brief Returns the strictly lexical parent loop of this node, if one
  /// exists.
  /// This is different for HLInsts which are located in loop
  /// preheader/postexit.
  HLLoop *getLexicalParentLoop() const;

  /// \brief Returns the outermost parent loop of this node, if one exists.
  HLLoop *getOutermostParentLoop() const;

  /// Returns parent loop with a nesting level of \p Level. Starts checking from
  /// the current node which could be a loop. Asserts if it cannot find such a
  /// loop.
  HLLoop *getParentLoopAtLevel(unsigned Level) const;

  /// \brief Returns the Level of HLNode.
  /// The level is computed from the node's lexical parent loop.
  unsigned getNodeLevel() const;

  /// \brief Returns the parent region of this node. Asserts if the parent
  /// region doesn't exist.
  HLRegion *getParentRegion() const;

  /// \brief Return an ID for the concrete type of this object.
  ///
  /// This is used to implement the classof checks in LLVM and should't
  /// be used for any other purpose.
  unsigned getHLNodeID() const { return SubClassID; }

  /// \brief Returns the unique number associated with this HLNode.
  unsigned getNumber() const { return Number; }

  /// \brief Returns the number of this node in the topological sort order.
  unsigned getTopSortNum() const { return TopSortNum; }

  /// \brief Returns the minimum topological sort number across its children.
  /// Minimum top sort num differs from normal top sort num for loops which have
  /// preheader nodes.
  unsigned getMinTopSortNum() const;

  /// \brief Returns the maximum topological sort number across its children.
  unsigned getMaxTopSortNum() const { return MaxTopSortNum; }

  /// Returns the previous node, if any, else return nullptr.
  const HLNode *getPrevNode() const {
    return const_cast<HLNode *>(this)->getPrevNode();
  }
  HLNode *getPrevNode();

  /// Returns the next node, if any, else return nullptr.
  const HLNode *getNextNode() const {
    return const_cast<HLNode *>(this)->getNextNode();
  }
  HLNode *getNextNode();

  /// \brief An enumeration to keep track of the concrete subclasses of HLNode.
  enum HLNodeVal {
    HLRegionVal,
    HLLoopVal,
    HLIfVal,
    HLInstVal,
    HLLabelVal,
    HLGotoVal,
    HLSwitchVal
  };

  /// \brief Verifies HLNode integrity.
  virtual void verify() const;

  // Returns current node debug location.
  virtual const DebugLoc getDebugLoc() const { return DebugLoc(); }
};

} // End loopopt namespace

} // End llvm namespace

#endif
