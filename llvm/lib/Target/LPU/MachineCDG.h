//===- MachineCDG.h -----------------------*- C++ -*-===//
//
//                      Static Program Analysis for LLVM
//
// This file is distributed under a Modified BSD License (see LICENSE.TXT).
//
//===----------------------------------------------------------------------===//
//
// This file defines the ControlDependenceGraph class, which allows fast and 
// efficient control dependence queries. It is based on Ferrante et al's "The 
// Program Dependence Graph and Its Use in Optimization."
//
//===----------------------------------------------------------------------===//

#ifndef LPU_CONTROLDEPENDENCEGRAPH_H
#define LPU_CONTROLDEPENDENCEGRAPH_H

#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/DOTGraphTraits.h"
#include "llvm/CodeGen/MachinePostDominators.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include <map>
#include <set>
#include <iterator>

namespace llvm {

class MachineBasicBlock;
class ControlDependenceGraphBase;

class ControlDependenceNode {
public:
  enum EdgeType { TRUE, FALSE, OTHER };
  typedef std::set<ControlDependenceNode *>::iterator       node_iterator;
  typedef std::set<ControlDependenceNode *>::const_iterator const_node_iterator;

  struct edge_iterator {
    typedef node_iterator::value_type      value_type;
    typedef node_iterator::difference_type difference_type;
    typedef node_iterator::reference       reference;
    typedef node_iterator::pointer         pointer;
    typedef std::input_iterator_tag        iterator_category;

    edge_iterator(ControlDependenceNode *n) : 
      node(n), stage(TRUE), it(n->TrueChildren.begin()), end(n->TrueChildren.end()) {
      while ((stage != OTHER) && (it == end)) this->operator++();
    }
    edge_iterator(ControlDependenceNode *n, EdgeType t, node_iterator i, node_iterator e) :
      node(n), stage(t), it(i), end(e) {
      while ((stage != OTHER) && (it == end)) this->operator++();
    }
    EdgeType type() const { return stage; }
    bool operator==(edge_iterator const &other) const { 
      return (this->stage == other.stage) && (this->it == other.it);
    }
    bool operator!=(edge_iterator const &other) const { return !(*this == other); }
    reference operator*()  { return *this->it; }
    pointer   operator->() { return &*this->it; }
    edge_iterator& operator++() {
      if (it != end) ++it;
      while ((stage != OTHER) && (it == end)) {
	if (stage == TRUE) {
	  it = node->FalseChildren.begin();
	  end = node->FalseChildren.end();
	  stage = FALSE;
	} else {
	  it = node->OtherChildren.begin();
	  end = node->OtherChildren.end();
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
  edge_iterator end()   { return edge_iterator(this, OTHER, OtherChildren.end(), OtherChildren.end()); }

  node_iterator true_begin()   { return TrueChildren.begin(); }
  node_iterator true_end()     { return TrueChildren.end(); }

  node_iterator false_begin()  { return FalseChildren.begin(); }
  node_iterator false_end()    { return FalseChildren.end(); }

  node_iterator other_begin()  { return OtherChildren.begin(); }
  node_iterator other_end()    { return OtherChildren.end(); }

  node_iterator parent_begin() { return Parents.begin(); }
  node_iterator parent_end()   { return Parents.end(); }
  const_node_iterator parent_begin() const { return Parents.begin(); }
  const_node_iterator parent_end()   const { return Parents.end(); }

  MachineBasicBlock *getBlock() const { return TheBB; }
  size_t getNumParents() const { return Parents.size(); }
  size_t getNumChildren() const { 
    return TrueChildren.size() + FalseChildren.size() + OtherChildren.size();
  }
  bool isRegion() const { return TheBB == NULL; }
  const ControlDependenceNode *enclosingRegion() const;

private:
  MachineBasicBlock *TheBB;
  std::set<ControlDependenceNode *> Parents;
  std::set<ControlDependenceNode *> TrueChildren;
  std::set<ControlDependenceNode *> FalseChildren;
  std::set<ControlDependenceNode *> OtherChildren;

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

  ControlDependenceNode() : TheBB(NULL) {}
  ControlDependenceNode(MachineBasicBlock *bb) : TheBB(bb) {}
};

template <> struct GraphTraits<ControlDependenceNode *> {
  typedef ControlDependenceNode NodeType;
  typedef NodeType::edge_iterator ChildIteratorType;

  static NodeType *getEntryNode(NodeType *N) { return N; }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->begin();
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->end();
  }

  typedef df_iterator<ControlDependenceNode *> nodes_iterator;

  static nodes_iterator nodes_begin(ControlDependenceNode *N) {
    return df_begin(getEntryNode(N));
  }
  static nodes_iterator nodes_end(ControlDependenceNode *N) {
    return df_end(getEntryNode(N));
  }
};
  
class ControlDependenceGraphBase {
public:
  ControlDependenceGraphBase() : root(NULL) {}
  virtual ~ControlDependenceGraphBase() { releaseMemory(); }
  virtual void releaseMemory() {
    for (ControlDependenceNode::node_iterator n = nodes.begin(), e = nodes.end();
	 n != e; ++n) delete *n;
    nodes.clear();
    bbMap.clear();
    root = NULL;
  }

  void graphForFunction(MachineFunction &F, MachinePostDominatorTree &pdt);

  ControlDependenceNode *getRoot()             { return root; }
  const ControlDependenceNode *getRoot() const { return root; }
  ControlDependenceNode *operator[](const MachineBasicBlock *BB)             { return getNode(BB); }
  const ControlDependenceNode *operator[](const MachineBasicBlock *BB) const { return getNode(BB); }
  ControlDependenceNode *getNode(const MachineBasicBlock *BB) { 
    return bbMap[BB];
  }
  const ControlDependenceNode *getNode(const MachineBasicBlock *BB) const {
    return (bbMap.find(BB) != bbMap.end()) ? bbMap.find(BB)->second : NULL;
  }
  bool controls(MachineBasicBlock *A, MachineBasicBlock *B) const;
  bool influences(MachineBasicBlock *A, MachineBasicBlock *B) const;
  const ControlDependenceNode *enclosingRegion(MachineBasicBlock *BB) const;
  //MachineFunction *thisMF;
  TargetInstrInfo *TII;

private:
  ControlDependenceNode *root;
  std::set<ControlDependenceNode *> nodes;
  std::map<const MachineBasicBlock *,ControlDependenceNode *> bbMap;
  ControlDependenceNode::EdgeType getEdgeType(MachineBasicBlock *, MachineBasicBlock *);
  void computeDependencies(MachineFunction &F, MachinePostDominatorTree &pdt);
  void insertRegions(MachinePostDominatorTree &pdt);
};

class ControlDependenceGraph : public MachineFunctionPass, public ControlDependenceGraphBase {
public:
  static char ID;

  ControlDependenceGraph() : MachineFunctionPass(ID), ControlDependenceGraphBase() {}
  virtual ~ControlDependenceGraph() { }
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<MachinePostDominatorTree>();
    AU.setPreservesAll();
  }
  virtual bool runOnMachineFunction(MachineFunction &F) {
    MachinePostDominatorTree &pdt = getAnalysis<MachinePostDominatorTree>();
    graphForFunction(F,pdt); 
    return false;
  }
};

template <> struct GraphTraits<ControlDependenceGraph *>
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


  template <> struct GraphTraits<MachinePostDominatorTree*>
    : public GraphTraits<MachineDomTreeNode*> {
    static NodeType *getEntryNode(MachinePostDominatorTree *DT) {
      return DT->getRootNode();
    }
    typedef df_iterator<MachineDomTreeNode*> nodes_iterator;
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


template <> struct DOTGraphTraits<ControlDependenceGraph *>
  : public DefaultDOTGraphTraits {
  DOTGraphTraits(bool isSimple = false) : DefaultDOTGraphTraits(isSimple) {}

  static std::string getGraphName(ControlDependenceGraph *Graph) {
    return "Control dependence graph";
  }

  std::string getNodeLabel(ControlDependenceNode *Node, ControlDependenceGraph *Graph) {
    if (Node->isRegion()) {
      return "REGION";
    } else {
      return (Node->getBlock()->getName() != NULL) ? Node->getBlock()->getName() : "ENTRY";
    }
  }

  static std::string getEdgeSourceLabel(ControlDependenceNode *Node, ControlDependenceNode::edge_iterator I) {
    switch (I.type()) {
    case ControlDependenceNode::TRUE:
      return "T";
    case ControlDependenceNode::FALSE:
      return "F";
    case ControlDependenceNode::OTHER:
      return "";
    }
  }
};


/***************************
class ControlDependenceGraphs : public ModulePass {
public:
  static char ID;

  ControlDependenceGraphs() : ModulePass(ID) {}
  virtual ~ControlDependenceGraphs() {
    graphs.clear();
  }

  virtual bool runOnModule(Module &M) {
    for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
      if (F->isDeclaration())
	continue;
      ControlDependenceGraphBase &cdg = graphs[F];
      PostDominatorTree &pdt = getAnalysis<PostDominatorTree>(*F);
      cdg.graphForFunction(*F,pdt);
    }
    return false;
  }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<PostDominatorTree>();
    AU.setPreservesAll();
  }

  ControlDependenceGraphBase &operator[](const Function *F) { return graphs[F]; }
  ControlDependenceGraphBase &graphFor(const Function *F) { return graphs[F]; }
private:
  std::map<const Function *, ControlDependenceGraphBase> graphs;
};
************************/
} // namespace llvm

#endif // LPU_CONTROLDEPENDENCEGRAPH_H
