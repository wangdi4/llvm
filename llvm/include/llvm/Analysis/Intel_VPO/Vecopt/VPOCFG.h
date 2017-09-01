//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOCFG.h -- Defines the AVR-level Control Flow Graph.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_CFG_H
#define LLVM_ANALYSIS_VPO_CFG_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvr.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrVisitor.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/Support/DOTGraphTraits.h"
#include "llvm/IR/Dominators.h"

namespace llvm { // LLVM Namespace

namespace vpo {  // VPO Vectorizer Namespace

typedef SmallPtrSet<AVR*, 2> AvrSetTy;

class AvrCFGBase;

/// \brief A Basic block of AVR node, the building block of the AVR CFG.
class AvrBasicBlock {

public:

  typedef SmallVector<AVR*, 16> InstructionsTy;
  typedef SmallVector<AvrBasicBlock*, 2> CFGEdgesTy;

private:

  friend AvrCFGBase;

  static void link(AvrBasicBlock& Source, AvrBasicBlock& Dest) {

    Source.Successors.push_back(&Dest);
    Dest.Predecessors.push_back(&Source);
  }
  static void unlink(AvrBasicBlock& Source, AvrBasicBlock& Dest) {

    bool found = false;
    for (auto It = Source.Successors.begin(), End = Source.Successors.end();
         It != End;
         ++It) {
      if (*It == &Dest) {
        Source.Successors.erase(It);
        found = true;
        break;
      }
    }
    if (!found)
      llvm_unreachable("Dest is not a successor of Source");
    found = false;

    for (auto It = Dest.Predecessors.begin(), End = Dest.Predecessors.end();
         It != End;
         ++It) {
      if (*It == &Source) {
        Dest.Predecessors.erase(It);
        found = true;
        break;
      }
    }
    if (!found)
      llvm_unreachable("Source is not a predecessor of Dest");
  }
  static void unlink(AvrBasicBlock& ABB) {

    for (auto I1 = ABB.Successors.begin(), E1 = ABB.Successors.end();
         I1 != E1; ++I1) {
      bool found = false;
      auto& Preds = (*I1)->Predecessors;
      for (auto I2 = Preds.begin(), E2 = Preds.end(); I2 != E2; ++I2) {
        if (*I2 == &ABB) {
          Preds.erase(I2);
          found = true;
          break;
        }
      }
      if (!found)
        llvm_unreachable("Block not found in one of its successor's predecessors");
    }

    for (auto I1 = ABB.Predecessors.begin(), E1 = ABB.Predecessors.end();
         I1 != E1;
         ++I1) {
      bool found = false;
      auto& Succs = (*I1)->Successors;
      for (auto I2 = Succs.begin(), E2 = Succs.end(); I2 != E2; ++I2) {
        if (*I2 == &ABB) {
          Succs.erase(I2);
          found = true;
          break;
        }
      }
      if (!found)
        llvm_unreachable("Block not found in one of its predecessor's successors");
    }

    ABB.Successors.clear();
    ABB.Predecessors.clear();
  }

  InstructionsTy Instructions;
  CFGEdgesTy Predecessors;
  CFGEdgesTy Successors;

  static unsigned long long NextId;
  unsigned long long Id;

  void assignId() { Id = NextId++; }

  void addInstruction(AVR* A) {
    Instructions.push_back(A);
  }

public:

  AvrBasicBlock() { assignId(); }

  unsigned long long getId() const { return Id; }

  const InstructionsTy& getInstructions() const { return Instructions; }

  InstructionsTy::iterator begin() { return Instructions.begin(); }

  InstructionsTy::iterator end() { return Instructions.end(); }

  void print(raw_ostream &O,
             bool printSingleAVRs,
             VerbosityLevel VL = PrintBase) const {

    formatted_raw_ostream FOS(O);
    FOS << "AvrBB-" << Id;
    if (printSingleAVRs && Instructions.size() == 1) {
      FOS << " --> ";
      (*Instructions.begin())->shallowPrint(FOS);
    }
  }

  void printAsOperand(raw_ostream &O, bool PrintType) const {
    print(O, true, PrintType? vpo::PrintAvrType : vpo::PrintBase);
  }

  bool isSuccessorOf(AvrBasicBlock* Other) const {
    for (AvrBasicBlock* Predecessor : Predecessors)
      if (Predecessor == Other)
        return true;
    return false;
  }

  unsigned getSuccessorsNum() const { return Successors.size(); }
  unsigned getPredecessorsNum() const { return Predecessors.size(); }

  const CFGEdgesTy& getPredecessors() const { return Predecessors; }
  const CFGEdgesTy& getSuccessors() const { return Successors; }

  CFGEdgesTy::const_iterator pred_begin() const { return Predecessors.begin(); }
  CFGEdgesTy::const_iterator pred_end() const { return Predecessors.end(); }
  CFGEdgesTy::const_iterator succ_begin() const { return Successors.begin(); }
  CFGEdgesTy::const_iterator succ_end() const { return Successors.end(); }
  CFGEdgesTy::iterator pred_begin() { return Predecessors.begin(); }
  CFGEdgesTy::iterator pred_end() { return Predecessors.end(); }
  CFGEdgesTy::iterator succ_begin() { return Successors.begin(); }
  CFGEdgesTy::iterator succ_end() { return Successors.end(); }
};

typedef DenseMap<AVR*, AvrBasicBlock*> AVRBasicBlocksMapTy;

/// \brief This class provides a Control Flow Graph view of AVR programs.
/// The CFG is external to the AVR program: the basic blocks are not AVR
/// nodes and the AVR program itself is not modified in any way.
class AvrCFGBase {

public:

  AvrCFGBase(AvrItr Begin, AvrItr End,
             const std::string& T,
             bool Deep = true,
             bool Compress = false);

  virtual ~AvrCFGBase();

  const std::string& getTitle() const { return Title; }

  AvrBasicBlock* getEntry() const { return Entry; }

  AvrBasicBlock* getExit() const { return Exit; }

  unsigned int getSize() const { return Size; }

  /// \brief Return the BasicBlock an AVR belongs to, if one exists (not all
  /// AVRs appear in the CFG).
  AvrBasicBlock* getBasicBlock(AVR* Avr) const {
    const auto& It = BasicBlocks.find(Avr);
    if (It == BasicBlocks.end())
      return nullptr;
    return It->second;
  }

  /// \brief Answer whether an AVR affects the control flow. This is true iff
  /// it is the terminator of a basic block with more than one successor.
  bool isBranchCondition(AVR* Avr) const {
    const AvrBasicBlock* BasicBlock = getBasicBlock(Avr);
    if (!BasicBlock)
      return false; // Not in the CFG, definitely not a branch condition.
    if (BasicBlock->getSuccessorsNum() < 2)
      return false; // Control flow from BasicBlock is not conditioned.
    return *(BasicBlock->getInstructions().rbegin()) == Avr;
  }

  /// \brief A type to represent a set of paths in the CFG where all paths
  /// share certain nodes in a given order. A nullptr value can be used to
  /// represent an unknown part of the path.
  typedef SmallVector<AvrBasicBlock*, 32> PathTy;

  /// \brief A type for sets of nodes used in path construction.
  typedef SmallPtrSet<AvrBasicBlock*, 32> NodeSetTy;

  static const NodeSetTy EmptyNodeSet;

  /// \brief Convenient wrapper of findSimplePaths for finding a single path.
  /// \param Schema see findSimplePaths.
  /// \param SkipFixedPointSiblings see findSimplePaths.
  PathTy findSimplePath(const PathTy& Schema,
                        bool SkipFixedPointSiblings) const;

  /// \brief Find simple paths along a given schema.
  /// \param Schema A description of the requested path based on nodes that
  /// must appear in it in specific locations (refered to here as 'fixed
  /// points') and possible gaps between them.
  /// \param JustOne Stop after finding the first path.
  /// \param SkipFixedPointSiblings If true, caller is not interested in paths
  /// that unnecessarily follow successors of nodes that have the next fixed
  /// point in the path as successors. For example, for the schema
  /// A -> ... -> B, the path A -> X -> Y -> B is of no interest if B is a
  /// successor of X.
  std::set<PathTy> findSimplePaths(const PathTy& Schema,
                                   bool JustOne,
                                   bool SkipFixedPointSiblings) const;

  void print(raw_ostream &OS, const PathTy& Path) const;

  void print(raw_ostream &OS) const;

  void printDot(raw_ostream &O, bool ShortNames = false);

private:

  /// \brief Make a given basic block the single new successor of another basic
  /// block.
  void insertAfter(AvrBasicBlock* BB, AvrBasicBlock* NewSuccessor);

  AvrBasicBlock* createBasicBlock(AVR* A = nullptr) {
    AvrBasicBlock* BB = new AvrBasicBlock();
    Size++;
    if (A) {
      BB->addInstruction(A);
      BasicBlocks.insert(std::make_pair(A, BB));
    }
    return BB;
  }

  /// \brief Safely delete a basic block from the CFG. The method always
  /// unlinks the basic block from its precessors and successors. Default
  /// behavior is to also link its predecessors with its successors to fix
  /// the graph.
  void deleteBasicBlock(AvrBasicBlock* BB,
                        bool ConnectPredecessorsToSuccessors = true);

  /// \brief If this basic block dominates a single successor, absorb that
  /// single successor into this basic block.
  bool mergeWithDominatedSuccessor(AvrBasicBlock* Source);

  /// \brief Implementation of findSimplePaths.
  /// \param Schema see findSimplePaths.
  /// \param Index Current position in schema.
  /// \param JustOne see findSimplePaths.
  /// \param SkipFixedPointSiblings see findSimplePaths.
  /// \param SkipFixedPointSiblings see findSimplePaths.
  /// \param InPath Nodes already in the path being constructed.
  /// \param PathSoFar The path being constructed.
  /// \param Result The set of paths found.
  bool findSimplePathsImpl(const PathTy& Schema,
                           unsigned Index,
                           bool FindJustOne,
                           bool SkipFixedPointSiblings,
                           NodeSetTy& InPath,
                           PathTy& PathSoFar,
                           std::set<PathTy>& Result) const;

  /// \brief Compress the CFG by merging chains of basic blocks.
  void compress();

  /// \brief A descriptive title for the CFG in favor of debug printing.
  std::string Title;

  /// Entry - entry node to the CFG. Typically the first AVR encountered by
  /// iterating the given AVR range for which a CFGInstruction exists, unless
  /// that AVR is not a true entry (i.e. has a predecessor), in which
  /// case this is an empty basic block.
  AvrBasicBlock* Entry;

  /// Exit - exit node of the CFG, either a real AVR or, if the CFG happens
  /// to have multiple exit nodes, an empty basic block.
  AvrBasicBlock* Exit;

  /// Size - the number of nodes in the CFG.
  unsigned int Size;

  /// BasicBlocks - map of AVRBasicBlocks allocated for AVRs
  AVRBasicBlocksMapTy BasicBlocks;

  /// PendingIncomingPHIValues - map of AVRValues feeding AVRPhi nodes which
  /// need to go into a specific basic block (marked by an AVRLabel) in order
  /// to correctly behave as Def reaching from the phi's predecessor.
  DenseMap<AVRLabel*, SmallPtrSet<AvrBasicBlock*, 5>> PendingIncomingPHIValues;

  class BuilderBase {

  protected:

    AvrCFGBase& CFG;

    AvrBasicBlock* getOrCreateInstruction(AVR* A);

    inline void setCurrentPredecessor(AvrBasicBlock* P) {
      CurrentPredecessors.clear();
      CurrentPredecessors.insert(P);
    }

    void clearCurrentPredecessors() { CurrentPredecessors.clear(); }

    void linkWithCurrentPredecessors(AvrBasicBlock* VI);

    void setNextInstruction(AvrBasicBlock* VI);

    SmallPtrSet<AvrBasicBlock*, 2> CurrentPredecessors;

    BuilderBase(AvrCFGBase& C, AvrBasicBlock* CurrentPredecessor = nullptr);

    // Common construction methods

    void construct(AVRCall *ACall);
    void construct(AVRBackEdge *ABackEdge);
    void construct(AVREntry *AEntry);
    void construct(AVRReturn *AReturn);
    void construct(AVRSelect *ASelect);
    void construct(AVRCompare *ACompare);
    void construct(AVRLabel *ALabel);
    void construct(AVRBranch *ABranch);

  public:

    virtual ~BuilderBase() {}

    const SmallPtrSetImpl<AvrBasicBlock*>& getPredecessors() {
      return CurrentPredecessors;
    }
  };

  /// \brief Utility class hiding the AVR visit functions used for graph
  /// construction in an inner class.
  class DeepBuilder : public BuilderBase {

  public:
    
    DeepBuilder(AvrCFGBase& C, AvrBasicBlock* CurrentPredecessor = nullptr) :
        BuilderBase(C, CurrentPredecessor) {}

    /// Visit Functions
    void visit(AVR* ANode) {}
    void visit(AVRValue* AValue);
    void postVisit(AVRExpression* AExpr);
    void visit(AVRPhi *APhi);
    void visit(AVRCall *ACall) { construct(ACall); }
    void visit(AVRBackEdge *ABackEdge) { construct(ABackEdge); }
    void visit(AVREntry *AEntry) { construct(AEntry); }
    void visit(AVRReturn *AReturn) { construct(AReturn); }
    void visit(AVRSelect *ASelect) { construct(ASelect); }
    void visit(AVRCompare *ACompare) { construct(ACompare); }
    void visit(AVRLabel *ALabel) { construct(ALabel); }
    void visit(AVRBranch *ABranch) { construct(ABranch); }
    void visit(AVRIf *AIf);
    void visit(AVRSwitch *ASwitch);
    void visit(AVRLoopHIR *ALoopHIR);
    void postVisit(AVR* ANode) {}
    bool isDone() { return false; }
    bool skipRecursion(AVR *ANode) {
      return (isa<AVRIf>(ANode) ||     // We recurse using separate visitors
              isa<AVRSwitch>(ANode) || // We recurse using separate visitors
              isa<AVRPhi>(ANode) ||    // Phi values handled by Phi visit
              isa<AVRLoopHIR>(ANode)); // We recurse using separate visitors
    }
  };

  class ShallowBuilder : public BuilderBase {

  public:

    ShallowBuilder(AvrCFGBase& C, AvrBasicBlock* CurrentPredecessor = nullptr) :
        BuilderBase(C, CurrentPredecessor) {}

    /// Visit Functions
    void visit(AVR* ANode) {}
    void visit(AVRAssign *AAssign);
    void visit(AVRPhi *APhi);
    void visit(AVRCall *ACall) { construct(ACall); }
    void visit(AVRBackEdge *ABackEdge) { construct(ABackEdge); }
    void visit(AVREntry *AEntry) { construct(AEntry); }
    void visit(AVRReturn *AReturn) { construct(AReturn); }
    void visit(AVRSelect *ASelect) { construct(ASelect); }
    void visit(AVRCompare *ACompare) { construct(ACompare); }
    void visit(AVRLabel *ALabel) { construct(ALabel); }
    void visit(AVRBranch *ABranch) { construct(ABranch); }
    void visit(AVRIf *AIf);
    void visit(AVRSwitch *ASwitch);
    void visit(AVRLoop *ALoop);
    void postVisit(AVR* ANode) {}
    bool isDone() { return false; }
    bool skipRecursion(AVR *ANode) {
      return (isa<AVRIf>(ANode) ||     // We recurse using separate visitors
              isa<AVRSwitch>(ANode) || // We recurse using separate visitors
              isa<AVRPhi>(ANode) ||    // Phi values handled by Phi visit
              isa<AVRLoop>(ANode)); // We recurse using separate visitors
    }
  };

protected:

  /// \brief Create a CFG for an AVR program.
  void runOnAvr(AvrItr Begin, AvrItr End, bool Deep, bool Compress);

};

/// \brief AVR CFG analysis variant for LLVM-IR.
///
/// This class provides the function-pass level CFG of the AVR as provided by
/// AVRGenerate.
class AvrCFG  : public FunctionPass {

public:

  // Pass Identification
  static char ID;

  AvrCFG();

  virtual ~AvrCFG();

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AVRGenerate>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override;

  void print(raw_ostream &OS, const Module* = nullptr) const override;

private:

  AvrCFGBase* CFG = nullptr;
};

/// \brief AVR CFG analysis variant for HIR.
///
/// This class provides the function-pass level CFG of the AVR as provided by
/// AVRGenerateHIR.
class AvrCFGHIR : public FunctionPass {

public:

  // Pass Identification
  static char ID;

  AvrCFGHIR();

  virtual ~AvrCFGHIR();

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AVRGenerateHIR>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override;

  void print(raw_ostream &OS, const Module* = nullptr) const override;

private:

  AvrCFGBase* CFG = nullptr;
};

} // End VPO Vectorizer Namespace

/// \brief A helper class to overcome the non-standard behavior of df_iterator.
///
/// LLVM's df_iterator does not provide the standard std-like operator*(), which
/// makes it incompatible with LLVM's dominator tree construction facilities.
/// This class is a minimal wrapper to bridge this gap.
template<class GraphT, class GT = GraphTraits<GraphT> >
class standard_df_iterator2 :
    public std::iterator<std::forward_iterator_tag,
                         typename GT::NodeType> {

private:

  df_iterator<GraphT> impl;

  standard_df_iterator2() {}

public:

  typedef std::iterator<std::forward_iterator_tag,
                        typename GT::NodeType> super;

  standard_df_iterator2(const GraphT &G, bool Begin)
      : impl(Begin ?
             df_iterator<GraphT>::begin(G) :
             df_iterator<GraphT>::end(G)) {}

  typename super::pointer operator*() const {
    return *impl;
  }

  bool operator==(const standard_df_iterator2 &x) const {
    return impl == x.impl;
  }

  bool operator!=(const standard_df_iterator2 &x) const {
    return !(*this == x);
  }

  standard_df_iterator2 &operator++() { // Preincrement
    impl++;
    return *this;
  }

  standard_df_iterator2 operator++(int) { // Postincrement
    standard_df_iterator2 tmp = *this;
    ++*this;
    return tmp;
  }

};

template <> struct GraphTraits<vpo::AvrBasicBlock*> {
  typedef vpo::AvrBasicBlock NodeType;
  typedef vpo::AvrBasicBlock *NodeRef;
  typedef vpo::AvrBasicBlock::CFGEdgesTy::iterator ChildIteratorType;
  typedef standard_df_iterator2<vpo::AvrBasicBlock *> nodes_iterator;

  static NodeType *getEntryNode(vpo::AvrBasicBlock *N) {
    return N;
  }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->succ_begin();
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->succ_end();
  }

  static nodes_iterator nodes_begin(vpo::AvrBasicBlock *N) {
    return nodes_iterator(N, true);
  }

  static nodes_iterator nodes_end(vpo::AvrBasicBlock *N) {
    return nodes_iterator(N, false);
  }

};

template <> struct GraphTraits<Inverse<vpo::AvrBasicBlock*> > {
  typedef vpo::AvrBasicBlock NodeType;
  typedef vpo::AvrBasicBlock *NodeRef;
  typedef vpo::AvrBasicBlock::CFGEdgesTy::iterator ChildIteratorType;
  typedef standard_df_iterator2<vpo::AvrBasicBlock *> nodes_iterator;

  static NodeType *getEntryNode(Inverse<vpo::AvrBasicBlock *> G) {
    return G.Graph;
  }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->pred_begin();
  }

  static inline ChildIteratorType child_end(NodeType *N) {
    return N->pred_end();
  }

  static nodes_iterator nodes_begin(vpo::AvrBasicBlock *N) {
    return nodes_iterator(N, true);
  }

  static nodes_iterator nodes_end(vpo::AvrBasicBlock *N) {
    return nodes_iterator(N, false);
  }
};

template <> struct GraphTraits<const vpo::AvrBasicBlock*> {
  typedef const vpo::AvrBasicBlock NodeType;
  typedef const vpo::AvrBasicBlock *NodeRef;
  typedef vpo::AvrBasicBlock::CFGEdgesTy::const_iterator ChildIteratorType;
  typedef standard_df_iterator2<const vpo::AvrBasicBlock *> nodes_iterator;

  static NodeType *getEntryNode(const vpo::AvrBasicBlock *N) {
    return N;
  }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->succ_begin();
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->succ_end();
  }

  static nodes_iterator nodes_begin(const vpo::AvrBasicBlock *N) {
    return nodes_iterator(N, true);
  }

  static nodes_iterator nodes_end(const vpo::AvrBasicBlock *N) {
    return nodes_iterator(N, false);
  }

};

template <> struct GraphTraits<Inverse<const vpo::AvrBasicBlock*> > {
  typedef const vpo::AvrBasicBlock NodeType;
  typedef const vpo::AvrBasicBlock *NodeRef;
  typedef vpo::AvrBasicBlock::CFGEdgesTy::const_iterator ChildIteratorType;
  typedef standard_df_iterator2<const vpo::AvrBasicBlock *> nodes_iterator;

  static NodeType *getEntryNode(Inverse<const vpo::AvrBasicBlock *> G) {
    return G.Graph;
  }

  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->pred_begin();
  }

  static inline ChildIteratorType child_end(NodeType *N) {
    return N->pred_end();
  }

  static nodes_iterator nodes_begin(const vpo::AvrBasicBlock *N) {
    return nodes_iterator(N, true);
  }

  static nodes_iterator nodes_end(const vpo::AvrBasicBlock *N) {
    return nodes_iterator(N, false);
  }
};

template <> struct GraphTraits<vpo::AvrCFGBase*>
  : public GraphTraits<vpo::AvrBasicBlock*> {

  static NodeType *getEntryNode(vpo::AvrCFGBase *G) {
    return G->getEntry();
  }

  static nodes_iterator nodes_begin(vpo::AvrCFGBase *N) {
    return nodes_iterator(getEntryNode(N), true);
  }

  static nodes_iterator nodes_end(vpo::AvrCFGBase *N) {
    return nodes_iterator(getEntryNode(N), false);
  }

  static unsigned size(vpo::AvrCFGBase *G) {
    return G->getSize();
  }
};

template <> struct GraphTraits<Inverse<vpo::AvrCFGBase*> >
  : public GraphTraits<Inverse<vpo::AvrBasicBlock*> > {

  static NodeType *getEntryNode(Inverse<vpo::AvrCFGBase*> G) {
    return G.Graph->getExit();
  }
};

template <> struct GraphTraits<const vpo::AvrCFGBase*>
  : public GraphTraits<const vpo::AvrBasicBlock*> {

  static NodeType *getEntryNode(const vpo::AvrCFGBase *G) {
    return G->getEntry();
  }

  static nodes_iterator nodes_begin(const vpo::AvrCFGBase *N) {
    return nodes_iterator(getEntryNode(N), true);
  }

  static nodes_iterator nodes_end(const vpo::AvrCFGBase *N) {
    return nodes_iterator(getEntryNode(N), false);
  }

  static unsigned size(const vpo::AvrCFGBase *G) {
    return G->getSize();
  }
};

template <> struct GraphTraits<Inverse<const vpo::AvrCFGBase*> >
  : public GraphTraits<Inverse<const vpo::AvrBasicBlock*> > {

  static NodeType *getEntryNode(Inverse<const vpo::AvrCFGBase*> G) {
    return G.Graph->getExit();
  }
};

template<>
struct DOTGraphTraits<vpo::AvrBasicBlock*> : public DefaultDOTGraphTraits {

  DOTGraphTraits(bool isSimple=false)
    : DefaultDOTGraphTraits(isSimple) {}

  static std::string getGraphName(vpo::AvrBasicBlock *I) {
    return "AVR CFG";
  }

  std::string getNodeLabel(vpo::AvrBasicBlock *Node, vpo::AvrBasicBlock *G) {
    std::string LabelString;
    llvm::raw_string_ostream LSO(LabelString);
    formatted_raw_ostream FOS(LSO);

    Node->print(FOS, false);
    LSO.flush();
    return LabelString;
  }

  std::string getNodeDescription(vpo::AvrBasicBlock *Node,
                                 vpo::AvrBasicBlock *G) {
    std::string DescriptionString;
    llvm::raw_string_ostream LSO(DescriptionString);
    formatted_raw_ostream FOS(LSO);

    bool isFirst = true;
    for (vpo::AVR* Avr : Node->getInstructions()) {
      if (!isFirst)
        FOS << "\n";
      Avr->shallowPrint(FOS);
      isFirst = false;
    }

    LSO.flush();
    return DescriptionString;
  }

};

/// \brief Template specialization of the standard LLVM dominator tree utility
/// for AVR CFGs.
class AvrDominatorTree : public DominatorTreeBase<vpo::AvrBasicBlock> {

public:

  AvrDominatorTree(bool isPostDom) :
      DominatorTreeBase<vpo::AvrBasicBlock>(isPostDom) {}

  virtual ~AvrDominatorTree() {}
};

} // End LLVM Namespace 

#endif  // LLVM_ANALYSIS_VPO_CFG_H
