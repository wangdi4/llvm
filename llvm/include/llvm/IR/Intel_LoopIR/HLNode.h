//===----------- HLNode.h - High level IR node ------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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
#include "llvm/Support/ErrorHandling.h"
#include <set>

namespace llvm {

namespace loopopt {

class HLLoop;

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
  void operator=(const HLNode &) LLVM_DELETED_FUNCTION;

  /// \brief Destroys all objects of this class. Should only be
  /// called after code gen.
  static void destroyAll();
  /// Keeps track of objects of this class.
  static std::set<HLNode *> Objs;
  /// Global number used for assigning unique numbers to HLNodes.
  static unsigned GlobalNum;

  /// ID to differentitate bwtween concrete subclasses.
  const unsigned char SubClassID;
  /// Lexical parent of HLNode.
  HLNode *Parent;

  /// Unique number associated with HLNodes.
  unsigned Number;
  /// Topological sort number of HLNode.
  unsigned TopSortNum;

  /// \brief Sets the unique number associated with this HLNode.
  void setNextNumber();
  /// \brief Sets the number of this node in the topological sort order.
  void setTopSortNum(unsigned Num) { TopSortNum = Num; }

protected:
  HLNode(unsigned SCID);
  HLNode(const HLNode &HLNodeObj);

  friend class HLNodeUtils;

  /// \brief Sets the lexical parent of this HLNode.
  void setParent(HLNode *Par) { Parent = Par; }
  /// \brief Destroys the object.
  void destroy();

public:
  virtual ~HLNode() {}

  /// Virtual Clone Method
  virtual HLNode *clone() const = 0;
  /// TBD how to do this
  void dump() const;
  /// TBD how to do this
  void print() const;

  /// \brief Returns the immediate lexical parent of the HLNode.
  HLNode *getParent() const { return Parent; }

  /// \brief Returns the parent loop of this node, if one exists.
  HLLoop *getParentLoop() const;

  /// \brief Return an ID for the concrete type of this object.
  ///
  /// This is used to implement the classof checks in LLVM and should't
  ///  be used for any other purpose.
  unsigned getHLNodeID() const { return SubClassID; }

  /// \brief Returns the unique number associated with this HLNode.
  unsigned getNumber() const { return Number; }

  /// \brief Returns the number of this node in the topological sort order.
  unsigned getTopSortNum() const { return TopSortNum; }

  /// \brief An enumeration to keep track of the concrete subclasses of HLNode
  enum HLNodeVal {
    HLRegionVal,
    HLLoopVal,
    HLIfVal,
    HLInstVal,
    HLLabelVal,
    HLGotoVal,
    HLSwitchVal
  };

  //
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
  static void deleteNode(loopopt::HLNode *) {}

private:
  mutable ilist_half_node<loopopt::HLNode> Sentinel;
};
/// Global definitions

namespace loopopt {

typedef iplist<HLNode> HLContainerTy;

/// Top level HLNodes (regions)
extern HLContainerTy HLRegions;

} // End loopopt namespace

} // End llvm namespace

#endif
