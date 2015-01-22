//===----------- HLDDNode.h - High level IR node ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the HLDDnode node.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_IR_INTEL_LOOPIR_HLDDNODE_H
#define LLVM_IR_INTEL_LOOPIR_HLDDNODE_H

#include "llvm/IR/Intel_LoopIR/HLNode.h"
#include <vector>
#include <iterator>

namespace llvm {

class BasicBlock;

namespace loopopt {

class DDRef;

/// \brief Base class for high level nodes which can contain DDRefs
class HLDDNode : public HLNode {
public:
  typedef std::vector<DDRef*> DDRefTy;

  /// Iterators to iterate over DDRefs
  typedef DDRefTy::iterator ddref_iterator;
  typedef DDRefTy::const_iterator const_ddref_iterator;
  typedef DDRefTy::reverse_iterator reverse_ddref_iterator;
  typedef DDRefTy::const_reverse_iterator const_reverse_ddref_iterator;

private:
  /// Unique number associated with HLDDNodes
  unsigned Number;
  /// Topological sort number of HLDDNode
  unsigned TopSortNum;
  /// Global number used for assigning unique numbers to HLDDNodes
  static unsigned GlobalNum;

protected:
  HLDDNode(unsigned SCID, HLNode* Par);
  virtual ~HLDDNode() { };

  friend class HLNodeUtils;

  /// The DDRef indices correspond to the operand number in the instruction
  /// with the first DDRef being for lval, if applicable.
  DDRefTy DDRefs;

public:
  /// \brief Returns the unique number associated with this HLDDNode.
  unsigned getNumber() const { return Number; }

  /// \brief Returns the number of this node in the topological sort order.
  unsigned getTopSortNum() const { return TopSortNum; }
  void setTopSortNum (unsigned Num) { TopSortNum = Num; }

  /// \brief Returns the parent loop of this node, if one exists.
  HLLoop* getParentLoop() const;

  /// DDRef iterator methods
  ddref_iterator               ddref_begin()        { return DDRefs.begin(); }
  const_ddref_iterator         ddref_begin()  const { return DDRefs.begin(); }
  ddref_iterator               ddref_end()          { return DDRefs.end(); }
  const_ddref_iterator         ddref_end()    const { return DDRefs.end(); }

  reverse_ddref_iterator       ddref_rbegin()       { return DDRefs.rbegin(); }
  const_reverse_ddref_iterator ddref_rbegin() const { return DDRefs.rbegin(); }
  reverse_ddref_iterator       ddref_rend()         { return DDRefs.rend(); }
  const_reverse_ddref_iterator ddref_rend()   const { return DDRefs.rend(); }


  /// DDRef acess methods
  size_t         numDDRefs() const   { return DDRefs.size();  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
