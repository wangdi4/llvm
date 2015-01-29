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
  HLNode(const HLNode&) LLVM_DELETED_FUNCTION;
  void operator=(const HLNode&) LLVM_DELETED_FUNCTION;

  /// \brief Destroys all objects of this class. Should only be
  /// called after code gen.
  static void destroyAll();
  /// Keeps track of objects of this class.
  static std::set< HLNode* >Objs;

  const unsigned char SubClassID;
  HLNode* Parent;

protected:
  HLNode(unsigned SCID, HLNode* Par);

  friend class HLNodeUtils;

  virtual HLNode* clone_impl() const = 0;

  /// \brief Sets the lexical parent of this HLNode.
  void setParent(HLNode* Par) { Parent = Par; }
  /// \brief Destroys the object.
  void destroy();

public:
  
  virtual ~HLNode() { }
  HLNode* clone() const;
  /// TBD how to do this
  void dump() const;
  /// TBD how to do this
  void print() const;

  /// \brief Returns the immediate lexical parent of the HLNode. 
  HLNode* getParent() const { return Parent; }

  /// \brief Return an ID for the concrete type of this object.
  ///
  /// This is used to implement the classof checks in LLVM and should't 
  ///  be used for any other purpose.
  unsigned getHLNodeID() const { return SubClassID; }

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
template<> struct ilist_traits<loopopt::HLNode> : public
  ilist_default_traits<loopopt::HLNode> {

  loopopt::HLNode *createSentinel() const {
    return static_cast<loopopt::HLNode*>(&Sentinel); 
  }

  static void destroySentinel(loopopt::HLNode*) { }

  loopopt::HLNode *provideInitialHead() const { return createSentinel(); }
  loopopt::HLNode *ensureHead(loopopt::HLNode*)const { return createSentinel();} 
  static void noteHead(loopopt::HLNode*, loopopt::HLNode*) { }

  static loopopt::HLNode *createNode(const loopopt::HLNode &) { 
    llvm_unreachable("HLNodes should be explicitly created via HLNodeUtils"
      "class"); 

    return nullptr;
  }
  static void deleteNode(loopopt::HLNode*) { }
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
