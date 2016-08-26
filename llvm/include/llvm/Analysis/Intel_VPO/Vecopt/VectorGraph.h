//===-- VectorGraph.h -------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the vector graph nodes built for loop vectorization.
///
//===----------------------------------------------------------------------===//
#ifndef LLVM_ANALYSIS_VECTOR_GRAPH_H
#define LLVM_ANALYSIS_VECTOR_GRAPH_H

#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/ilist.h"
#include "llvm/ADT/ilist_node.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"
#include <map>

#define TabLength 2

namespace llvm { // LLVM Namespace

class Loop;
class SCEV;

class VGLoop;
class VGBlock;
class VGPredicate;

/// \brief
class VGNode : public ilist_node<VGNode> {
private:
  /// \brief Make class uncopyable.
  void operator=(const VGNode &) = delete;

  /// SubClassID - AVR Subclass Identifier
  const unsigned char SubClassID;

  /// Parent - Lexical parent of this AVR
  VGNode *Parent;

  /// GlobalNumber - A global number used for assigning unique ID to each avr
  static unsigned GlobalNumber;

  /// Number - Unique ID for AVR node.
  unsigned Number;

  /// Slev - SIMD lane evolution classification of this AVR node.
  // SLEV Slev;

  /// Predicate - The AVR node that is the predicate masking this one.
  VGPredicate *Predicate;

  /// \brief Destroys all objects of this class. Only called after Vectorizer
  /// phase code generation.
  static void destroyAll();

protected:
  /// \brief
  VGNode(unsigned SCID);
  VGNode(const VGNode &Obj);
  virtual ~VGNode() {}

  /// \brief Destroys the object.
  void destroy();

  /// Sets unique ID for this node.
  void setNumber();

  /// \brief Sets the lexical parent of this node.
  void setParent(VGNode *ParentNode) { Parent = ParentNode; }

  /// \brief Sets the predicate for this node.
  void setPredicate(VGPredicate *P) { Predicate = P; }

  /// Only this utility class should be used to modify/delete vector graph
  /// nodes.
  friend class VectorGraphUtils;

public:
  /// Virtual Clone Method
  virtual VGNode *clone() const = 0;

  /// \brief Dumps this vector graph node.
  void dump() const;

  /// \brief Virtual print method. Derived classes should implement this
  /// routine.
  virtual void print(formatted_raw_ostream &OS, unsigned Depth) const = 0;

  /// \brief Derived classes will implement, prints the type of this avr.
  virtual void printNodeKind(formatted_raw_ostream &OS) const = 0;

  /// \brief Returns the Avr nodes's unique ID number
  unsigned getNumber() const { return Number; }

  /// \brief Returns the Avr nodes's SLEV data.
  // SLEV getSLEV() const { return Slev; }

  /// \brief Returns the Avr nodes's predicating Avr node.
  VGPredicate *getPredicate() const { return Predicate; }

  /// \brief Returns the immediate lexical parent of the AVR.
  VGNode *getParent() const { return Parent; }

  /// \brief Returns the parent loop of this node, if one exists.
  VGLoop *getParentLoop() const;

  /// \brief Return an ID for the concrete type of this object.
  ///
  /// This is used to implement the classof, etc. checks in LLVM and should't
  /// be used for any other purpose.
  unsigned getVGID() const { return SubClassID; }

  /// VectorGraphNode Subclass Kinds
  enum VectorGraphNodeVal { VGLoopNode, VGBlockNode, VGPredicateNode };
};

/// \brief Traits for iplist<VGNode>
///
/// See ilist_traits<Instruction> in BasicBlock.h for details
template <>
struct ilist_traits<VGNode> : public ilist_default_traits<VGNode> {

  VGNode *createSentinel() const {
    return static_cast<VGNode *>(&Sentinel);
  }

  static void destroySentinel(VGNode *) {}

  VGNode *provideInitialHead() const { return createSentinel(); }
  VGNode *ensureHead(VGNode *) const { return createSentinel(); }
  static void noteHead(VGNode *, VGNode *) {}

  static VGNode *createNode(const VGNode &) {
    llvm_unreachable("Vector Grpah Nodes should be explicitly created via "
                     "VectorGraphNodes");

    return nullptr;
  }
  static void deleteNode(VGNode *) {}

private:
  mutable ilist_half_node<VGNode> Sentinel;
};

typedef iplist<VGNode> VectorGraphTy;

/// \brief
class VGLoop final : public VGNode {

public:
  typedef VectorGraphTy ChildNodeTy;

  /// Iterators to iterate over children nodes
  typedef ChildNodeTy::iterator child_iterator;
  typedef ChildNodeTy::const_iterator const_child_iterator;
  typedef ChildNodeTy::reverse_iterator reverse_child_iterator;
  typedef ChildNodeTy::const_reverse_iterator const_reverse_child_iterator;

private:
  /// LLVM Loop
  const Loop *LLoop;

  VGBlock *LoopLatch;

  /// Children - Contains the children basic blocks of this loop.
  ChildNodeTy Children;

  /// LoopIVs and strides
  SmallDenseMap<Instruction *, const SCEV *> LoopIVs;

  /// Loop upper bound
  SCEV *UpperBound;

  /// Loop lower bound
  SCEV *LowerBound;

  /// Entry - entry node to the CFG. Typically the first AVR encountered by
  /// iterating the given AVR range for which a CFGInstruction exists, unless
  /// that AVR is not a true entry (i.e. has a predecessor), in which
  /// case this is an empty basic block.
  VGBlock* Entry;

  /// Exit - exit node of the CFG, either a real AVR or, if the CFG happens
  /// to have multiple exit nodes, an empty basic block.
  VGBlock* Exit;

  /// Size - the number of nodes in the CFG.
  unsigned int Size;

protected:
  VGLoop(Loop *Lp);
  virtual ~VGLoop() override {}

  friend class VectorGraphUtils;

public:

  /// CFG building class members
  // quick lookup for LLVM basic block to VGBlock
  std::map<BasicBlock*, VGBlock*> BlockMap;

  void addSuccessors(Loop *Lp, VGBlock*);
  VGBlock* getOrInsertBlock(Loop *Lp, BasicBlock *BB);
  void insertLoopExitBlock(Loop *Lp);

  /// Loop Children Iterators

  child_iterator child_begin() { return Children.begin(); }
  const_child_iterator child_begin() const { return Children.begin(); }
  reverse_child_iterator child_rbegin() { return Children.rbegin(); }
  const_reverse_child_iterator child_rbegin() const {
    return Children.rbegin();
  }

  child_iterator child_end() { return Children.end(); }
  const_child_iterator child_end() const { return Children.end(); }
  reverse_child_iterator child_rend() { return Children.rend(); }
  const_reverse_child_iterator child_rend() const { return Children.rend(); }

  typedef iterator_range<child_iterator> LoopNodesRange;

  LoopNodesRange nodes() { return LoopNodesRange(child_begin(), child_end()); }

  /// Children access Methods
  VGBlock* getLoopLatch() { return LoopLatch; }

  /// \brief Returns the first child if it exists, otherwise returns null.
  VGNode *getFirstChild();

  /// \brief Returns the first child if it exists, otherwise returns null.
  VGNode *getFirstChild() const {
    return const_cast<VGLoop *>(this)->getFirstChild();
  }

  /// \brief Returns the last child if it exists, otherwise returns null.
  VGNode *getLastChild();

  /// \brief Returns const pointer to last child if it exisits.
  VGNode *getLastChild() const {
    return const_cast<VGLoop *>(this)->getLastChild();
  }

  /// \brief Returns the number of children.
  unsigned getNumChildren() const { return Children.size(); }

  /// \brief Returns true if it has children.
  bool hasChildren() const { return !Children.empty(); }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VGNode *Node) {
    return Node->getVGID() == VGNode::VGLoopNode;
  }

  // For now, since we are working with innermost loops only, we can assume
  // other nodes in the graph are VGBlock*. This will need to be changed to
  // VGNode* once we start dealing with multiple loop levels.
  VGBlock* getEntry() const { return cast<VGBlock>(getFirstChild()); }

  VGBlock* getExit() const { return cast<VGBlock>(getLastChild()); }

  unsigned int getSize() const { return Size; }

  const Loop* getLoop() { return LLoop; }

  /// \brief
  void print(formatted_raw_ostream &OS, unsigned Depth) const override;

  /// \brief Prints the type name of this avr.
  void printNodeKind(formatted_raw_ostream &OS) const override;

  /// \brief Creates a clone of this vector graph
  VGLoop *clone() const override;
};

/// \brief
class VGBlock final : public VGNode {

private:
  SmallVector<VGBlock *, 2> Predecessors;
  SmallVector<VGBlock *, 2> Successors;
  SmallPtrSet<VGBlock *, 2> SchedConstraints;

  // Use BBlock handle to access instructions.
  BasicBlock *BBlock;

  /// Condition - pointer to the instruction  which generates the true/false bit
  /// for
  /// that selects between (the two) successors.
  Value *Condition;

  void setCondition(Value *C) { Condition = C; }

  void addSuccessor(VGBlock *Successor) {
    assert(Successor && "Null successor?");
    Successors.push_back(Successor);
    Successor->Predecessors.push_back(this);
    Successor->addSchedulingConstraint(this);
  }

  void addSchedulingConstraint(VGBlock *Block) {
    SchedConstraints.insert(Block);
  }

protected:
  VGBlock(BasicBlock *BB);
  virtual ~VGBlock() override {}

  friend class VectorGraphUtils;

public:
  const SmallVectorImpl<VGBlock *> &getPredecessors() const {
    return Predecessors;
  }
  const SmallVectorImpl<VGBlock *> &getSuccessors() const { return Successors; }
  const SmallPtrSetImpl<VGBlock *> &getSchedConstraints() {
    return SchedConstraints;
  }

  SmallVectorImpl<VGBlock *>::const_iterator pred_begin() const {
    return Predecessors.begin();
  }
  SmallVectorImpl<VGBlock *>::const_iterator pred_end() const {
    return Predecessors.end();
  }
  SmallVectorImpl<VGBlock *>::const_iterator succ_begin() const {
    return Successors.begin();
  }
  SmallVectorImpl<VGBlock *>::const_iterator succ_end() const {
    return Successors.end();
  }
  SmallVectorImpl<VGBlock *>::iterator pred_begin() {
    return Predecessors.begin();
  }
  SmallVectorImpl<VGBlock *>::iterator pred_end() { return Predecessors.end(); }
  SmallVectorImpl<VGBlock *>::iterator succ_begin() {
    return Successors.begin();
  }
  SmallVectorImpl<VGBlock *>::iterator succ_end() { return Successors.end(); }

  unsigned getSuccessorOrdinal(VGBlock *Successor) {
    unsigned Ordinal = 0;
    for (VGBlock *VBlock : Successors) {
      if (Successor == VBlock)
        return Ordinal;
      ++Ordinal;
    }
    llvm_unreachable("Block is not a successor");
  }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VGNode *Node) {
    return Node->getVGID() == VGNode::VGBlockNode;
  }

  /// \brief Clone method for AVRUnreachable.
  VGBlock *clone() const override;

  /// \brief Prints the AVR Unreachable node.
  void print(formatted_raw_ostream &OS, unsigned Depth) const override;

  /// \brief Prints the AVR Unreachable node.
  void print(raw_ostream &OS, unsigned Depth) const;

  /// \brief Prints the type name of this avr.
  void printNodeKind(formatted_raw_ostream &OS) const override;

  /// \brief Prints the type name of this avr.
  void printNodeKind(raw_ostream &OS) const;

  void printAsOperand(raw_ostream &OS, bool PrintType) const {
    print(OS, 0);
  }

  BasicBlock* getBasicBlock() { return BBlock; }

  /// \brief Answer whether a VGBlock affects the control flow. This is true iff
  /// it is the terminator of a basic block with more than one successor.
  bool hasBranchCondition() const {
    if (Successors.size() < 2)
      return false; // Control flow from BasicBlock is not conditioned.
    return true;
  }

  Value* getBranchCondition() {
    if (hasBranchCondition()) {
      TerminatorInst *TermInst = BBlock->getTerminator();
      if (BranchInst *BrInst = dyn_cast<BranchInst>(TermInst)) {
        return BrInst->getCondition();
      }
    }
    return nullptr;
  }
};

/// \brief
class VGPredicate : public VGNode {
public:
  /// \brief A type representing an incoming value to the AVRPredicate and the
  /// VGNode corresponding to the basic block it originates from.
  typedef std::pair<VGPredicate *, Value *> IncomingTy;

private:
  /// \brief Incoming AVR values and their corresponding AVR labels.
  SmallVector<IncomingTy, 2> IncomingPredicates;

  VGPredicate();
  virtual ~VGPredicate() override {}

  ///\brief Adds an incoming AVRPredicate when some condition holds.
  void addIncoming(VGPredicate *VPredicate, Value *Condition) {
    IncomingPredicates.push_back(std::make_pair(VPredicate, Condition));
  }

  friend class VectorGraphUtils;

public:
  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const VGNode *Node) {
    return (Node->getVGID() == VGNode::VGPredicateNode);
  }

  VGPredicate *clone() const override;

  /// \brief Prints the AVRPredicate node.
  void print(formatted_raw_ostream &OS, unsigned Depth) const override;

  /// \brief Prints the type name of this avr.
  void printNodeKind(formatted_raw_ostream &OS) const override;

  /// \brief Returns the incoming values of this AVR predicate.
  const SmallVectorImpl<IncomingTy> &getIncoming() {
    return IncomingPredicates;
  }
};

template <class GraphT, class GT = GraphTraits<GraphT>>
class standard_df_iterator
    : public std::iterator<std::forward_iterator_tag, typename GT::NodeType> {
private:
  df_iterator<GraphT> impl;

  standard_df_iterator() {}

public:
  typedef std::iterator<std::forward_iterator_tag, typename GT::NodeType> super;

  standard_df_iterator(const GraphT &G, bool Begin)
      : impl(Begin ? df_iterator<GraphT>::begin(G)
                   : df_iterator<GraphT>::end(G)) {}

  typename super::reference operator*() const { return *(*impl); }

  bool operator==(const standard_df_iterator &x) const {
    return impl == x.impl;
  }

  bool operator!=(const standard_df_iterator &x) const { return !(*this == x); }

  standard_df_iterator &operator++() { // Preincrement
    impl++;
    return *this;
  }

  standard_df_iterator operator++(int) { // Postincrement
    standard_df_iterator tmp = *this;
    ++*this;
    return tmp;
  }
};

template <> struct GraphTraits<VGBlock *> {
  typedef VGBlock NodeType;
  typedef SmallVectorImpl<VGBlock *>::iterator ChildIteratorType;
  typedef standard_df_iterator<VGBlock *> nodes_iterator;

  static NodeType *getEntryNode(VGBlock *N) { return N; }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->succ_begin();
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->succ_end();
  }

  static nodes_iterator nodes_begin(VGBlock *N) {
    return nodes_iterator(N, true);
  }

  static nodes_iterator nodes_end(VGBlock *N) {
    return nodes_iterator(N, false);
  }
};

template <> struct GraphTraits<Inverse<VGBlock *>> {
  typedef VGBlock NodeType;
  typedef SmallVectorImpl<VGBlock *>::iterator ChildIteratorType;
  typedef standard_df_iterator<VGBlock *> nodes_iterator;

  static NodeType *getEntryNode(Inverse<VGBlock *> G) { return G.Graph; }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->pred_begin();
  }

  static inline ChildIteratorType child_end(NodeType *N) {
    return N->pred_end();
  }

  static nodes_iterator nodes_begin(VGBlock *N) {
    return nodes_iterator(N, true);
  }

  static nodes_iterator nodes_end(VGBlock *N) {
    return nodes_iterator(N, false);
  }
};

template <> struct GraphTraits<const VGBlock *> {
  typedef const VGBlock NodeType;
  typedef SmallVectorImpl<VGBlock *>::const_iterator ChildIteratorType;
  typedef standard_df_iterator<const VGBlock *> nodes_iterator;

  static NodeType *getEntryNode(const VGBlock *N) { return N; }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->succ_begin();
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->succ_end();
  }

  static nodes_iterator nodes_begin(const VGBlock *N) {
    return nodes_iterator(N, true);
  }

  static nodes_iterator nodes_end(const VGBlock *N) {
    return nodes_iterator(N, false);
  }
};

template <> struct GraphTraits<Inverse<const VGBlock *>> {
  typedef const VGBlock NodeType;
  typedef SmallVectorImpl<VGBlock *>::const_iterator ChildIteratorType;
  typedef standard_df_iterator<const VGBlock *> nodes_iterator;

  static NodeType *getEntryNode(Inverse<const VGBlock *> G) {
    return G.Graph;
  }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->pred_begin();
  }

  static inline ChildIteratorType child_end(NodeType *N) {
    return N->pred_end();
  }

  static nodes_iterator nodes_begin(const VGBlock *N) {
    return nodes_iterator(N, true);
  }

  static nodes_iterator nodes_end(const VGBlock *N) {
    return nodes_iterator(N, false);
  }
};

template <> struct GraphTraits<VGLoop *>
  : public GraphTraits<VGBlock *> {

  static NodeType *getEntryNode(VGLoop *VGL) {
    return VGL->getEntry();
  }

  static nodes_iterator nodes_begin(VGLoop *VGL) {
    return nodes_iterator(getEntryNode(VGL), true);
  }

  static nodes_iterator nodes_end(VGLoop *VGL) {
    return nodes_iterator(getEntryNode(VGL), false);
  }

  static unsigned size(VGLoop *VGL) {
    return VGL->getSize();
  }
};

template <> struct GraphTraits<Inverse<VGLoop *> >
  : public GraphTraits<Inverse<VGBlock *> > {

  static NodeType *getEntryNode(Inverse<VGLoop *> VGL) {
    return VGL.Graph->getExit();
  }
};

template <> struct GraphTraits<const VGLoop *>
  : public GraphTraits<const VGBlock *> {

  static NodeType *getEntryNode(const VGLoop *VGL) {
    return VGL->getEntry();
  }

  static nodes_iterator nodes_begin(const VGLoop *VGL) {
    return nodes_iterator(getEntryNode(VGL), true);
  }

  static nodes_iterator nodes_end(const VGLoop *VGL) {
    return nodes_iterator(getEntryNode(VGL), false);
  }

  static unsigned size(const VGLoop *VGL) {
    return VGL->getSize();
  }
};

template <> struct GraphTraits<Inverse<const VGLoop *> >
  : public GraphTraits<Inverse<const VGBlock *> > {

  static NodeType *getEntryNode(Inverse<const VGLoop *> VGL) {
    return VGL.Graph->getExit();
  }
};

} // End LLVM Namespace

#endif // LLVM_ANALYSIS_VPO_AVR_H
