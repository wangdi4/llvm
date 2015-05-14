//===--------- WRegionNode.h - W-Region Graph Node --------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the W-Region Graph node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGIONNODE_H
#define LLVM_ANALYSIS_VPO_WREGIONNODE_H

#include "llvm/ADT/ilist.h"
#include "llvm/ADT/ilist_node.h"

#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"

#include "llvm/IR/InstrTypes.h"

#include <set>

namespace llvm {

namespace vpo {

class WRegion;

/// \brief WRegion Node base class
///
/// This represents a node of the W-Region. It is used to represent
/// the incoming LLVM IR in program order.
///
/// This class (hierarchy) disallows creating objects on stack.
/// Objects are created/destroyed using WRegionNodeUtils friend class.
class WRegionNode : public ilist_node<WRegionNode> {
private:
  /// \brief Make class uncopyable.
  //void operator=(const WRegionNode &) LLVM_DELETED_FUNCTION;

  /// \brief Destroys all objects of this class. Should only be
  /// called after code gen.
  static void destroyAll();

  /// Keeps track of objects of this class.
  static std::set<WRegionNode *> Objs;

  /// Number used for assigning unique numbers to WRegionNodes.
  static unsigned UniqueNum;

  /// ID to differentitate bwtween concrete subclasses.
  const unsigned SubClassID;

  /// Enclosing parent of WRegionNode in CFG.
  WRegionNode *Parent;

  /// Unique number associated with WRegionNodes.
  unsigned Number;

  /// \brief Sets the unique number associated with this WRegionNode.
  void setNextNumber();

protected:
  WRegionNode(unsigned SCID);
  WRegionNode(const WRegionNode &WRegionNodeObj);

  // friend class WRegionNodeUtils;

  /// IndentWidth used to print WRegionNodes.
  static const unsigned IndentWidth = 3;

  /// \brief Destroys the object.
  void destroy();

  /// \brief Indents nodes for printing.
  void indent(formatted_raw_ostream &OS, unsigned Depth) const;

public:
  
  virtual ~WRegionNode() {}

  /// Virtual Clone Method
  virtual WRegionNode *clone() const = 0;

  /// \brief Dumps WRegionNode.
  void dump() const;

  /// \brief Prints WRegionNode.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth) const = 0;

  /// \brief Sets the lexical parent of this WRegionNode.
  void setParent(WRegionNode *P) { Parent = P; }

  /// \brief Returns the immediate enclosing parent of the WRegionNode.
  WRegionNode *getParent() const { return Parent; }

  /// \brief Returns the parent region of this node, if one exists.
  WRegion *getParentRegion() const;

  /// \brief Return an ID for the concrete type of this object.
  ///
  /// This is used to implement the classof checks in LLVM and should't
  ///  be used for any other purpose.
  unsigned getWRegionKindID() const { return SubClassID; }

  /// \brief Returns the unique number associated with this WRegionNode.
  unsigned getNumber() const { return Number; }

  /// \brief An enumeration to keep track of the concrete subclasses of 
  /// WRegionNode
  enum WRegionNodeKind{
    VPO_PAR_REGION,
    VPO_PAR_LOOP,
    VPO_PAR_SECTIONs,
    VPO_PAR_TASK,
    VPO_WKS_LOOP,
    VPO_VEC_LOOP,
    VPO_WKS_SECTIONS,
    VPO_SECTION,
    VPO_SINGLE,
    VPO_MASTER,
    VPO_ORDERED,
    VPO_ATOMIC,
    VPO_CRITICAL,
    VPO_TASKGROUP,
    VPO_BARRIER,
    VPO_CANCEL
  };
  //
};

} // End vpo namespace

/// \brief traits for iplist<WRegionNode>
///
/// Refer to ilist_traits<Instruction> in BasicBlock.h for explanation.
template <>
struct ilist_traits<vpo::WRegionNode>
    : public ilist_default_traits<vpo::WRegionNode> {

  vpo::WRegionNode *createSentinel() const {
    return static_cast<vpo::WRegionNode *>(&Sentinel);
  }

  static void destroySentinel(vpo::WRegionNode *) {}

  vpo::WRegionNode *provideInitialHead() const { return createSentinel(); }
  vpo::WRegionNode *ensureHead(vpo::WRegionNode *) const {
    return createSentinel();
  }
  static void noteHead(vpo::WRegionNode *, vpo::WRegionNode *) {}

  static vpo::WRegionNode *createWRegionNode(const vpo::WRegionNode &) {
    llvm_unreachable("WRegionNodes should be explicitly created via" 
                     "WRegionNodeUtils class");
    return nullptr;
  }
  static void deleteWRegionNode(vpo::WRegionNode *) {}

private:
  mutable ilist_half_node<vpo::WRegionNode> Sentinel;
};
/// Global definitions

namespace vpo {

typedef iplist<WRegionNode> WRContainerTy;

/// TODO: Remove this.
/// Top level WRegionNodes (regions)
extern WRContainerTy WRegions;

} // End vpo namespace

} // End llvm namespace

#endif
