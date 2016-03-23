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

#include "llvm/ADT/ilist.h"
#include "llvm/ADT/ilist_node.h"
#include "llvm/ADT/DenseMap.h"

#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"

#include "llvm/IR/InstrTypes.h"

#include <set>

namespace llvm {

namespace loopopt {

class HLNode;
class HLGoto;
class HLLabel;
class HLLoop;
class HLRegion;

// Typedef for a list of HLNodes.
typedef iplist<HLNode> HLContainerTy;

// Container for Goto's
typedef SmallVector<HLGoto *, 16> GotoContainerTy;

// Map for Old Label and New Label
typedef SmallDenseMap<const HLLabel *, HLLabel *, 16> LabelMapTy;

// Defining predicate type
typedef CmpInst::Predicate PredicateTy;
const PredicateTy UNDEFINED_PREDICATE = PredicateTy::BAD_FCMP_PREDICATE;

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

  /// \brief Destroys all objects of this class. Should only be
  /// called after code gen.
  static void destroyAll();
  /// Keeps track of objects of this class.
  static std::set<HLNode *> Objs;
  /// Global number used for assigning unique numbers to HLNodes.
  static unsigned GlobalNum;

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

  /// \brief Sets the unique number associated with this HLNode.
  void setNextNumber();

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

protected:
  HLNode(unsigned SCID);
  HLNode(const HLNode &HLNodeObj);
  virtual ~HLNode() {}

  friend class HLNodeUtils;

  /// IndentWidth used to print HLNodes.
  static const unsigned IndentWidth = 3;

  /// \brief Sets the lexical parent of this HLNode.
  void setParent(HLNode *Par) { Parent = Par; }

  /// \brief Destroys the object.
  void destroy();

  /// \brief Indents nodes for printing.
  void indent(formatted_raw_ostream &OS, unsigned Depth) const;

  /// \brief Returns true if Pred is TRUE or FALSE.
  static bool isPredicateTrueOrFalse(PredicateTy Pred) {
    return Pred == PredicateTy::FCMP_TRUE || Pred == PredicateTy::FCMP_FALSE;
  }

  /// \brief Pretty prints predicates.
  static void printPredicate(formatted_raw_ostream &OS, PredicateTy Pred);

  /// \brief Virtual Clone Implementation
  /// This function populates the GotoList with Goto branching within the
  /// region and LabelMap with Old and New Labels.
  virtual HLNode *cloneImpl(GotoContainerTy *GotoList,
                            LabelMapTy *LabelMap) const = 0;

  /// \brief Base Clone Implementation
  /// This is the protected base clone implementation as the subclasses cannot
  /// directly call the cloneImpl of other subclasses.
  /// For e.g. Loop->cloneBaseImpl(child, GL, LM) will return child clone.
  HLNode *cloneBaseImpl(const HLNode *Node, GotoContainerTy *GotoList,
                        LabelMapTy *LabelMap) const {
    return Node->cloneImpl(GotoList, LabelMap);
  }

public:
  /// Virtual Clone Method
  virtual HLNode *clone() const = 0;
  /// \brief Dumps HLNode.
  void dump() const;

  /// \brief Dumps HLNode with details.
  void dump(bool Detailed) const;

  /// \brief Prints HLNode with details.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth,
                     bool Detailed = false) const = 0;

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

  /// \brief Returns the Level of HLNode.
  /// The level is computed from the node's lexical parent loop.
  unsigned getNodeLevel() const;

  /// \brief Returns the parent region of this node, if one exists.
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

  /// \brief Returns the maximum topological sort number across its children.
  unsigned getMaxTopSortNum() const { return MaxTopSortNum; }

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
};

} // End loopopt namespace
/// \brief traits for iplist<HLNode>
///
/// Refer to ilist_traits<Instruction> in BasicBlock.h for explanation.
template <>
struct ilist_traits<loopopt::HLNode>
    : public ilist_default_traits<loopopt::HLNode> {

  loopopt::HLNode *createSentinel() const {
    return static_cast<loopopt::HLNode *>(&Sentinel);
  }

  static void destroySentinel(loopopt::HLNode *) {}

  loopopt::HLNode *provideInitialHead() const { return createSentinel(); }
  loopopt::HLNode *ensureHead(loopopt::HLNode *) const {
    return createSentinel();
  }
  static void noteHead(loopopt::HLNode *, loopopt::HLNode *) {}

  static loopopt::HLNode *createNode(const loopopt::HLNode &) {
    llvm_unreachable("HLNodes should be explicitly created via HLNodeUtils"
                     "class");

    return nullptr;
  }

  // Deletion of nodes intentionally leaved empty to save compile time
  static void deleteNode(loopopt::HLNode *Node) {}

private:
  mutable ilist_node<loopopt::HLNode> Sentinel;
};

} // End llvm namespace

#endif
