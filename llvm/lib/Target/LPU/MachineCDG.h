//===- MachineCDG.h -----------------------*- C++ -*-===//
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
#include "llvm/ADT/SetVector.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/DOTGraphTraits.h"
#include "llvm/CodeGen/MachinePostDominators.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include <map>
#include <set>
#include <deque>
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
      reference operator*() { return *this->it; }
      pointer   operator->() { return &*this->it; }
      edge_iterator& operator++() {
        if (it != end) ++it;
        while ((stage != OTHER) && (it == end)) {
          if (stage == TRUE) {
            it = node->FalseChildren.begin();
            end = node->FalseChildren.end();
            stage = FALSE;
          }
          else {
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
    edge_iterator end() { return edge_iterator(this, OTHER, OtherChildren.end(), OtherChildren.end()); }

    node_iterator true_begin() { return TrueChildren.begin(); }
    node_iterator true_end() { return TrueChildren.end(); }

    node_iterator false_begin() { return FalseChildren.begin(); }
    node_iterator false_end() { return FalseChildren.end(); }

    node_iterator other_begin() { return OtherChildren.begin(); }
    node_iterator other_end() { return OtherChildren.end(); }

    node_iterator parent_begin() { return Parents.begin(); }
    node_iterator parent_end() { return Parents.end(); }
    const_node_iterator parent_begin() const { return Parents.begin(); }
    const_node_iterator parent_end()   const { return Parents.end(); }

    MachineBasicBlock *getBlock() const { return TheBB; }
    size_t getNumParents() const { return Parents.size(); }
    size_t getNumChildren() const {
      return TrueChildren.size() + FalseChildren.size() + OtherChildren.size();
    }
    bool isRegion() const { return TheBB == NULL; }
    const ControlDependenceNode *enclosingRegion() const;
    bool isLatchNode();

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

  struct CDGRegion {
    SetVector<ControlDependenceNode *> nodes;
    unsigned NewRegion;
  };
 
  class ControlDependenceGraphBase {
  public:
    ControlDependenceGraphBase() : root(NULL) {}
    virtual ~ControlDependenceGraphBase() { releaseMemory(); }
    virtual void releaseMemory() {
      for (ControlDependenceNode::node_iterator n = nodes.begin(), e = nodes.end();
        n != e; ++n) delete *n;
      for (unsigned i = 0; i < regions.size(); i++) {
        CDGRegion *r = regions[i];
        delete r;
      }
      nodes.clear();
      bb2cdg.clear();
      cdg2bb.clear();
      root = NULL;
    }

    void graphForFunction(MachineFunction &F, MachinePostDominatorTree &pdt);
    void regionsForGraph(MachineFunction &F, MachinePostDominatorTree &pdt);
    void dumpRegions();
    ControlDependenceNode *getRoot() { return root; }
    const ControlDependenceNode *getRoot() const { return root; }
    ControlDependenceNode *operator[](const MachineBasicBlock *BB) { return getNode(BB); }
    const ControlDependenceNode *operator[](const MachineBasicBlock *BB) const { return getNode(BB); }
    ControlDependenceNode *getNode(const MachineBasicBlock *BB) {
      return bb2cdg[BB];
    }
    const ControlDependenceNode *getNode(const MachineBasicBlock *BB) const {
      return (bb2cdg.find(BB) != bb2cdg.end()) ? bb2cdg.find(BB)->second : NULL;
    }
    CDGRegion* getRegion(ControlDependenceNode *anode) {
      return cdg2rgn[anode];
    }
    bool controls(MachineBasicBlock *A, MachineBasicBlock *B) const;
    bool influences(MachineBasicBlock *A, MachineBasicBlock *B) const;
    const ControlDependenceNode *enclosingRegion(MachineBasicBlock *BB) const;
    MachineFunction *thisMF;
    const TargetInstrInfo *TII;

  private:
    ControlDependenceNode *root;
    std::set<ControlDependenceNode *> nodes;
    DenseMap<const MachineBasicBlock *, ControlDependenceNode *> bb2cdg;
    DenseMap<ControlDependenceNode *, MachineBasicBlock *> cdg2bb;
    SmallDenseMap<ControlDependenceNode *, CDGRegion *> cdg2rgn;
    SmallVector<CDGRegion *, 64> regions;
    ControlDependenceNode::EdgeType getEdgeType(MachineBasicBlock *, MachineBasicBlock *);
    void computeDependencies(MachineFunction &F, MachinePostDominatorTree &pdt);
    void insertRegions(MachinePostDominatorTree &pdt);
  };


  class ControlDependenceGraph : public MachineFunctionPass, public ControlDependenceGraphBase {
  public:
    static char ID;

    ControlDependenceGraph();
    virtual ~ControlDependenceGraph() { }
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<MachineDominatorTree>();
      AU.addRequired<MachinePostDominatorTree>();
      AU.setPreservesAll();
      MachineFunctionPass::getAnalysisUsage(AU);
    }
    void writeDotGraph(StringRef fname);
    virtual bool runOnMachineFunction(MachineFunction &F) {
      thisMF = &F;
      TII = thisMF->getSubtarget().getInstrInfo();
      MachinePostDominatorTree &pdt = getAnalysis<MachinePostDominatorTree>();
      graphForFunction(F, pdt);
      writeDotGraph(F.getName());
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
      }
      else {
        return Node->getBlock()->getFullName();
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



  template <> struct DOTGraphTraits<MachinePostDominatorTree *>
    : public DefaultDOTGraphTraits {
    DOTGraphTraits(bool isSimple = false) : DefaultDOTGraphTraits(isSimple) {}

    static std::string getGraphName(MachinePostDominatorTree *Graph) {
      return "Machine Post Dominator Tree";
    }

    std::string getNodeLabel(MachineDomTreeNode *Node, MachinePostDominatorTree *Graph) {
      return Node->getBlock()->getFullName();
    }
  };

} // namespace llvm

#endif // LPU_CONTROLDEPENDENCEGRAPH_H
