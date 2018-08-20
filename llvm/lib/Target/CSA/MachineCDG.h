//===- MachineCDG.h -----------------------*- C++ -*-===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// This file defines the ControlDependenceGraph class, which allows fast and
// efficient control dependence queries. It is based on Ferrante et al's "The
// Program Dependence Graph and Its Use in Optimization."
//
//===----------------------------------------------------------------------===//

#ifndef CSA_CONTROLDEPENDENCEGRAPH_H
#define CSA_CONTROLDEPENDENCEGRAPH_H

#include "CSAInstrInfo.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachinePostDominators.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/DOTGraphTraits.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include <deque>
#include <iterator>
#include <map>
#include <set>

#define _unused(x) ((void)(x))

namespace llvm {

class MachineBasicBlock;
class ControlDependenceGraphBase;

class ControlDependenceNode {
public:
  enum EdgeType { FALSE = 0, TRUE, OTHER };
  typedef std::set<ControlDependenceNode *>::iterator node_iterator;
  typedef std::set<ControlDependenceNode *>::const_iterator const_node_iterator;

  struct edge_iterator {
    typedef node_iterator::value_type value_type;
    typedef node_iterator::difference_type difference_type;
    typedef node_iterator::reference reference;
    typedef node_iterator::pointer pointer;
    typedef std::input_iterator_tag iterator_category;

    edge_iterator(ControlDependenceNode *n)
        : node(n), stage(TRUE), it(n->TrueChildren.begin()),
          end(n->TrueChildren.end()) {
      while ((stage != OTHER) && (it == end))
        this->operator++();
    }
    edge_iterator(ControlDependenceNode *n, EdgeType t, node_iterator i,
                  node_iterator e)
        : node(n), stage(t), it(i), end(e) {
      while ((stage != OTHER) && (it == end))
        this->operator++();
    }
    EdgeType type() const { return stage; }
    bool operator==(edge_iterator const &other) const {
      return (this->stage == other.stage) && (this->it == other.it);
    }
    bool operator!=(edge_iterator const &other) const {
      return !(*this == other);
    }
    reference operator*() { return *this->it; }
    pointer operator->() { return &*this->it; }
    edge_iterator &operator++() {
      if (it != end)
        ++it;
      while ((stage != OTHER) && (it == end)) {
        if (stage == TRUE) {
          it    = node->FalseChildren.begin();
          end   = node->FalseChildren.end();
          stage = FALSE;
        } else {
          it    = node->OtherChildren.begin();
          end   = node->OtherChildren.end();
          stage = OTHER;
        }
      }
      return *this;
    }
    edge_iterator operator++(int) {
      edge_iterator ret(*this);
      assert(ret.stage == OTHER || ret.it != ret.end);
      this->operator++();
      return ret;
    }

  private:
    ControlDependenceNode *node;
    EdgeType stage;
    node_iterator it, end;
  };

  edge_iterator begin() { return edge_iterator(this); }
  edge_iterator end() {
    return edge_iterator(this, OTHER, OtherChildren.end(), OtherChildren.end());
  }

  node_iterator true_begin() { return TrueChildren.begin(); }
  node_iterator true_end() { return TrueChildren.end(); }

  node_iterator false_begin() { return FalseChildren.begin(); }
  node_iterator false_end() { return FalseChildren.end(); }

  node_iterator other_begin() { return OtherChildren.begin(); }
  node_iterator other_end() { return OtherChildren.end(); }

  node_iterator parent_begin() { return Parents.begin(); }
  node_iterator parent_end() { return Parents.end(); }
  const_node_iterator parent_begin() const { return Parents.begin(); }
  const_node_iterator parent_end() const { return Parents.end(); }

  MachineBasicBlock *getBlock() const { return TheBB; }
  size_t getNumParents() const { return Parents.size(); }
  size_t getNumChildren() const {
    return TrueChildren.size() + FalseChildren.size() + OtherChildren.size();
  }
  bool isRegion() const { return TheBB == NULL; }
  const ControlDependenceNode *enclosingRegion() const;
  // bool isLatchNode();
  bool isTrueChild(ControlDependenceNode *cnode) {
    return (TrueChildren.find(cnode) != true_end());
  }

  bool isFalseChild(ControlDependenceNode *cnode) {
    return (FalseChildren.find(cnode) != false_end());
  }

  bool isOtherChild(ControlDependenceNode *cnode) {
    return (OtherChildren.find(cnode) != other_end());
  }

  bool isChild(ControlDependenceNode *cnode) {
    return isTrueChild(cnode) || isFalseChild(cnode) || isOtherChild(cnode);
  }

  bool isParent(ControlDependenceNode *pnode) {
    return Parents.find(pnode) != parent_end();
  }

private:
  MachineBasicBlock *TheBB;
  struct Node_Compare {
    bool operator()(const ControlDependenceNode *A,
                    const ControlDependenceNode *B) const {
      if (A->getBlock() == NULL)
        return true;
      if (B->getBlock() == NULL)
        return false;
      return A->getBlock()->getNumber() < B->getBlock()->getNumber();
    }
  };
  std::set<ControlDependenceNode *, Node_Compare> Parents;
  std::set<ControlDependenceNode *, Node_Compare> TrueChildren;
  std::set<ControlDependenceNode *, Node_Compare> FalseChildren;
  std::set<ControlDependenceNode *, Node_Compare> OtherChildren;

  friend class ControlDependenceGraphBase;

  void clearAllChildren() {
    TrueChildren.clear();
    FalseChildren.clear();
    OtherChildren.clear();
  }

  void clearAllParents() { Parents.clear(); }

  void addTrue(ControlDependenceNode *Child);
  void addFalse(ControlDependenceNode *Child);
  void addOther(ControlDependenceNode *Child);
  void addParent(ControlDependenceNode *Parent);
  void removeTrue(ControlDependenceNode *Child);
  void removeFalse(ControlDependenceNode *Child);
  void removeOther(ControlDependenceNode *Child);
  void removeParent(ControlDependenceNode *Child);

  ControlDependenceNode() : TheBB(NULL) {
    clearAllChildren();
    clearAllParents();
  }
  ControlDependenceNode(MachineBasicBlock *bb) : TheBB(bb) {}
};

template <> struct GraphTraits<ControlDependenceNode *> {
  typedef ControlDependenceNode NodeType;
  typedef ControlDependenceNode *NodeRef;
  typedef NodeType::edge_iterator ChildIteratorType;

  static NodeType *getEntryNode(NodeType *N) { return N; }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->begin();
  }
  static inline ChildIteratorType child_end(NodeType *N) { return N->end(); }

  typedef df_iterator<ControlDependenceNode *> nodes_iterator;

  static nodes_iterator nodes_begin(ControlDependenceNode *N) {
    return df_begin(getEntryNode(N));
  }
  static nodes_iterator nodes_end(ControlDependenceNode *N) {
    return df_end(getEntryNode(N));
  }
};

struct CDGRegion {
  SetVector<ControlDependenceNode *> nodes;
  unsigned NewRegion;
};

class ControlDependenceGraphBase {
public:
  ControlDependenceGraphBase() : root(NULL) {
    nodes.clear();
    bb2cdg.clear();
    cdg2bb.clear();
    cdg2rgn.clear();
  }
  virtual ~ControlDependenceGraphBase() { releaseMemory(); }
  virtual void releaseMemory() {
    for (ControlDependenceNode::node_iterator n = nodes.begin(),
                                              e = nodes.end();
         n != e; ++n)
      delete *n;
    for (unsigned i = 0; i < regions.size(); i++) {
      CDGRegion *r = regions[i];
      delete r;
    }
    nodes.clear();
    bb2cdg.clear();
    cdg2bb.clear();
    cdg2rgn.clear();
    root = NULL;
  }

  void graphForFunction(MachineFunction &F, MachinePostDominatorTree &pdt);
  void regionsForGraph(MachineFunction &F, MachinePostDominatorTree &pdt);
  void dumpRegions();
  ControlDependenceNode *getRoot() { return root; }
  const ControlDependenceNode *getRoot() const { return root; }
  ControlDependenceNode *operator[](const MachineBasicBlock *BB) {
    return getNode(BB);
  }
  const ControlDependenceNode *operator[](const MachineBasicBlock *BB) const {
    return getNode(BB);
  }
  ControlDependenceNode *getNode(const MachineBasicBlock *BB) {
    return bb2cdg[BB];
  }
  const ControlDependenceNode *getNode(const MachineBasicBlock *BB) const {
    return (bb2cdg.find(BB) != bb2cdg.end()) ? bb2cdg.find(BB)->second : NULL;
  }
  CDGRegion *getRegion(ControlDependenceNode *anode) { return cdg2rgn[anode]; }
  ControlDependenceNode::EdgeType getEdgeType(MachineBasicBlock *,
                                              MachineBasicBlock *,
                                              bool confirmAnalysiable = false);
  // ControlDependenceNode* getNonLatchParent(ControlDependenceNode* anode, bool
  // oneAndOnly);
  bool controls(MachineBasicBlock *A, MachineBasicBlock *B) const;
  bool influences(MachineBasicBlock *A, MachineBasicBlock *B) const;
  const ControlDependenceNode *enclosingRegion(MachineBasicBlock *BB) const;
  MachineFunction *thisMF;
  const TargetInstrInfo *TII;
  MachinePostDominatorTree *thisPDT;

private:
  ControlDependenceNode *root;
  std::set<ControlDependenceNode *> nodes;
  DenseMap<const MachineBasicBlock *, ControlDependenceNode *> bb2cdg;
  DenseMap<ControlDependenceNode *, MachineBasicBlock *> cdg2bb;
  SmallDenseMap<ControlDependenceNode *, CDGRegion *> cdg2rgn;
  SmallVector<CDGRegion *, 64> regions;

  void computeDependencies(MachineFunction &F, MachinePostDominatorTree &pdt);
  void insertRegions(MachinePostDominatorTree &pdt);
};

class ControlDependenceGraph : public MachineFunctionPass,
                               public ControlDependenceGraphBase {
public:
  static char ID;
  ControlDependenceGraph();
  virtual ~ControlDependenceGraph() {}
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<MachineDominatorTree>();
    AU.addRequired<MachinePostDominatorTree>();
    AU.setPreservesAll();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
  void writeDotGraph(StringRef fname);
  void viewMachineCDG(void);
  void viewMachineCFG(void);
  void viewMachinePDT(void);
  void viewMachineDT(void);
  virtual bool runOnMachineFunction(MachineFunction &F);

  StringRef getPassName() const override {
    return "CSA: Machine Control Dependence Graph Construction";
  }
};

template <>
struct GraphTraits<ControlDependenceGraph *>
    : public GraphTraits<ControlDependenceNode *> {
  static NodeType *getEntryNode(ControlDependenceGraph *CD) {
    return CD->getRoot();
  }

  static nodes_iterator nodes_begin(ControlDependenceGraph *CD) {
    if (getEntryNode(CD))
      return df_begin(getEntryNode(CD));
    else
      return df_end(getEntryNode(CD));
  }

  static nodes_iterator nodes_end(ControlDependenceGraph *CD) {
    return df_end(getEntryNode(CD));
  }
};

template <>
struct GraphTraits<MachinePostDominatorTree *>
    : public GraphTraits<MachineDomTreeNode *> {
  static NodeRef getEntryNode(MachinePostDominatorTree *DT) {
    return DT->getRootNode();
  }
  typedef df_iterator<MachineDomTreeNode *> nodes_iterator;
  static nodes_iterator nodes_begin(MachinePostDominatorTree *N) {
    if (getEntryNode(N))
      return df_begin(getEntryNode(N));
    else
      return df_end(getEntryNode(N));
  }

  static nodes_iterator nodes_end(MachinePostDominatorTree *N) {
    return df_end(getEntryNode(N));
  }
};

template <>
struct DOTGraphTraits<ControlDependenceGraph *> : public DefaultDOTGraphTraits {
  DOTGraphTraits(bool isSimple = false) : DefaultDOTGraphTraits(isSimple) {}

  static std::string getGraphName(ControlDependenceGraph *Graph) {
    std::string fName(Graph->thisMF->getName());
    return "Machine CDG for '" + fName + "' function";
  }

  std::string getNodeLabel(ControlDependenceNode *Node,
                           ControlDependenceGraph *Graph) {
    if (Node->isRegion()) {
      return "REGION";
    } else {
#if 0
        return Node->getBlock()->getFullName();
#else
      std::string blkNumber = std::to_string(Node->getBlock()->getNumber());
      std::string name      = "BB#" + blkNumber;
      return name;
#endif
    }
  }

  static std::string
  getEdgeSourceLabel(ControlDependenceNode *Node,
                     ControlDependenceNode::edge_iterator I) {
    switch (I.type()) {
    case ControlDependenceNode::TRUE:
      return "T";
    case ControlDependenceNode::FALSE:
      return "F";
    case ControlDependenceNode::OTHER:
      return "";
    default:
      assert(false && "unknown edge type");
      return "";
    }
  }
};

template <>
struct DOTGraphTraits<MachinePostDominatorTree *>
    : public DefaultDOTGraphTraits {
  DOTGraphTraits(bool isSimple = false) : DefaultDOTGraphTraits(isSimple) {}

  static std::string getGraphName(MachinePostDominatorTree *Graph) {
    //PDT root is a fake exit node which has no corresponding block.
    std::string fName(Graph->getRootNode()->getChildren().front()->getBlock()->getParent()->getName());
    return "Machine PDT for '" + fName + "' function";
  }

  std::string getNodeLabel(MachineDomTreeNode *Node,
                           MachinePostDominatorTree *Graph) {
#if 0
      return Node->getBlock()->getFullName();
#else
    std::string blkNumber = Node->getBlock() ? std::to_string(Node->getBlock()->getNumber()) :
                            "Exit";
    std::string name = "BB#" + blkNumber;
    return name;
#endif
  }
};

template <>
struct DOTGraphTraits<MachineDominatorTree *> : public DefaultDOTGraphTraits {
  DOTGraphTraits(bool isSimple = false) : DefaultDOTGraphTraits(isSimple) {}

  static std::string getGraphName(MachineDominatorTree *Graph) {
    std::string fName(Graph->getRootNode()->getBlock()->getParent()->getName());
    return "Machine DT for '" + fName + "' function";
  }

  std::string getNodeLabel(MachineDomTreeNode *Node,
                           MachineDominatorTree *Graph) {
#if 0
        return Node->getBlock()->getFullName();
#else
    std::string blkNumber = std::to_string(Node->getBlock()->getNumber());
    std::string name = "BB#" + blkNumber;
    return name;
#endif
  }
};

template <>
struct DOTGraphTraits<MachineFunction *> : public DefaultDOTGraphTraits {
  DOTGraphTraits(bool isSimple = false) : DefaultDOTGraphTraits(isSimple) {}

  static std::string getGraphName(MachineFunction *Graph) {
    std::string fName = Graph->getName();
    return "Machine CFG for '" + fName + "' function";
  }

  std::string getNodeLabel(MachineBasicBlock *Node, MachineFunction *Graph) {
#if 0
        return Node->getFullName();
#else
    std::string blkNumber = std::to_string(Node->getNumber());
    std::string name = "BB#" + blkNumber;
    return name;
#endif
  }
};

class CSASSANode {
public:
  MachineInstr *minstr;
  SmallVector<CSASSANode *, 4> children;
  CSASSANode(MachineInstr *ainstr) { this->minstr = ainstr; }
};

template <> struct GraphTraits<CSASSANode *> {
  typedef CSASSANode NodeType;
  typedef CSASSANode *NodeRef;
  typedef SmallVectorImpl<CSASSANode *>::iterator ChildIteratorType;

  static NodeType *getEntryNode(NodeType *N) { return N; }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->children.begin();
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->children.end();
  }

  typedef df_iterator<CSASSANode *> nodes_iterator;

  static nodes_iterator nodes_begin(CSASSANode *N) {
    return df_begin(getEntryNode(N));
  }
  static nodes_iterator nodes_end(CSASSANode *N) {
    return df_end(getEntryNode(N));
  }
};

class CSASSAGraph {
  CSASSANode *root;
  MachineRegisterInfo *MRI;
  const CSAInstrInfo *TII;

public:
  DenseMap<MachineInstr *, CSASSANode *> instr2ssan;
  CSASSANode *getRoot() { return root; }
  void BuildCSASSAGraph(MachineFunction &F, bool ignCtrl = false);
  ~CSASSAGraph() {
    delete root;
    for (DenseMap<MachineInstr *, CSASSANode *>::iterator
           i2n    = instr2ssan.begin(),
           i2nEnd = instr2ssan.end();
         i2n != i2nEnd; ++i2n) {
      delete i2n->second;
    }
  }
};

template <>
struct GraphTraits<CSASSAGraph *> : public GraphTraits<CSASSANode *> {
  static NodeType *getEntryNode(CSASSAGraph *SSAG) { return SSAG->getRoot(); }

  static nodes_iterator nodes_begin(CSASSAGraph *SSAG) {
    if (getEntryNode(SSAG))
      return df_begin(getEntryNode(SSAG));
    else
      return df_end(getEntryNode(SSAG));
  }

  static nodes_iterator nodes_end(CSASSAGraph *SSAG) {
    return df_end(getEntryNode(SSAG));
  }
};

} // namespace llvm

#endif // CSA_CONTROLDEPENDENCEGRAPH_H
