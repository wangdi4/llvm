//===- CSAGraphSplitter.cpp - Remove graph's cross dependencies -*- C++ -*-===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Implementation of a pass which eliminates cross dependencies between
/// different CSA graphs by cloning functions that are called by multiple
/// graphs.
///
//===----------------------------------------------------------------------===//

#include "Intel_CSA/Transforms/Scalar/CSAGraphSplitter.h"
#include "Intel_CSA/CSAIRPasses.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <queue>

using namespace llvm;

#define DEBUG_TYPE "csa-graph-splitter"
STATISTIC(NumClonedFuncs, "Number of cloned functions");

namespace {

class GraphSplitter {
  class SCCNode {
    GraphSplitter &GS;
    SmallVector<CallGraphNode *, 8u> CGNodes;
    SmallVector<SCCNode *, 8u> Callees;

  private:
    template <class T>
    SCCNode(GraphSplitter &GS, T &&CGNodes)
        : GS(GS), CGNodes(CGNodes.begin(), CGNodes.end()) {}

  public:
    template <class T>
    static SCCNode *create(GraphSplitter &GS, T &&CGNodes) {
      // Create new node.
      auto *NewSCC = new SCCNode(GS, CGNodes);
      GS.SCCs.push_back(NewSCC);
      if (NewSCC->isTargetEntry())
        GS.Entries.push_back(NewSCC);
      for (auto *Node : CGNodes)
        GS.CGNode2SCC[Node] = NewSCC;

      // Setup node's callees.
      SmallPtrSet<SCCNode *, 8u> Callees;
      for (auto *Node : CGNodes)
        for (auto &CR : *Node) {
          auto *Callee = GS[CR.second];
          assert(Callee && "no SCC for the callee");
          if (Callee == NewSCC)
            continue;
          Callees.insert(Callee);
        }
      NewSCC->Callees.insert(NewSCC->Callees.begin(), Callees.begin(),
                             Callees.end());
      return NewSCC;
    }

    const SmallVectorImpl<SCCNode *> &getCallees() const { return Callees; }

    // Return true is this SCC contains an outlined target entry.
    bool isTargetEntry() const {
      if (CGNodes.size() == 1u)
        if (const auto *Func = CGNodes.front()->getFunction())
          return Func->hasFnAttribute("omp.target.entry");
      return false;
    }

    // Clone functions which belong to this SCC, creates new SCC node and fixes
    // up calls within cloned SCC.
    SCCNode *
    clone(SmallDenseMap<CallGraphNode *, CallGraphNode *> &Node2Clone) const {
      // Clone all functions from this SCC.
      SmallVector<CallGraphNode *, 8u> CGClones(CGNodes.size());
      transform(CGNodes, CGClones.begin(), [&](CallGraphNode *Node) {
        auto *Func = Node->getFunction();
        if (!Func || Func->isDeclaration() || Func->isIntrinsic())
          return Node;

        // Clone function and insert it to the CG.
        ValueToValueMapTy VMap;
        auto *NewFunc = CloneFunction(Func, VMap);
        auto *NewNode = GS.CG.getOrInsertFunction(NewFunc);

        // FIXME: This code is stolen from CallGraph::addToCallGraph(Function*),
        // which happens to be private. It is better for this functionality to
        // be exposed by the CallGraph.
        for (auto &I : instructions(NewFunc))
          if (auto *CB = dyn_cast<CallBase>(&I)) {
            const auto *Callee = CB->getCalledFunction();
            if (!Callee || !Intrinsic::isLeaf(Callee->getIntrinsicID()))
              // Indirect calls of intrinsics are not allowed so no need to
              // check. We can be more precise here by using TargetArg returned
              // by Intrinsic::isLeaf.
              NewNode->addCalledFunction(CB, GS.CG.getCallsExternalNode());
            else if (!Callee->isIntrinsic())
              NewNode->addCalledFunction(CB, GS.CG.getOrInsertFunction(Callee));
          }

        Node2Clone[Node] = NewNode;
        ++NumClonedFuncs;
        return NewNode;
      });

      // Fixup calls within cloned SCC.
      for (auto *Clone : CGClones)
        for (auto &CR : *Clone)
          if (auto *NewNode = Node2Clone.lookup(CR.second)) {
            auto *CB = cast<CallBase>(CR.first);
            CB->setCalledFunction(NewNode->getFunction());
            Clone->replaceCallEdge(*CB, *CB, NewNode);
          }

      // And finally create new SCC node.
      return SCCNode::create(GS, CGClones);
    }

    // Clones callee at the given index and rewires all calls from this node to
    // the cloned instance. Returns cloned callee node.
    SCCNode *replaceCalleeWithClone(unsigned Idx) {
      // Clone SCC.
      SmallDenseMap<CallGraphNode *, CallGraphNode *> Node2Clone;
      auto *Clone = Callees[Idx]->clone(Node2Clone);

      // Replace calls to callee's nodes with clones.
      for (auto *Node : CGNodes)
        for (auto &CR : *Node)
          if (auto *NewNode = Node2Clone.lookup(CR.second)) {
            auto *CB = cast<CallBase>(CR.first);
            CB->setCalledFunction(NewNode->getFunction());
            Node->replaceCallEdge(*CB, *CB, NewNode);
          }
      Callees[Idx] = Clone;
      return Clone;
    }
  };
  friend SCCNode;

  // Returns an associated SCC for the provided call graph node.
  SCCNode *operator[](const CallGraphNode *Node) const {
    return CGNode2SCC.lookup(Node);
  }

private:
  CallGraph &CG;
  SmallDenseMap<CallGraphNode *, SCCNode *> CGNode2SCC;
  SmallVector<SCCNode *, 8u> SCCs;
  SmallVector<SCCNode *, 8u> Entries;

public:
  GraphSplitter(CallGraph &CG) : CG(CG) {}
  ~GraphSplitter() { DeleteContainerPointers(SCCs); }

  bool run();
};

} // end anonymous namespace

bool GraphSplitter::run() {
  // Setup SCC tree.
  for (auto I = scc_begin(&CG), E = scc_end(&CG); I != E; ++I)
    SCCNode::create(*this, *I);

  bool Changed = false;
  SmallPtrSet<SCCNode *, 16u> Visited;
  for (auto *E : Entries) {
    std::queue<SCCNode *> Worklist({E});
    while (!Worklist.empty()) {
      auto *Node = Worklist.front();
      Worklist.pop();
      Visited.insert(Node);

      auto &Callees = Node->getCallees();
      for (size_t I = 0u; I < Callees.size(); ++I) {
        auto *Callee = Callees[I];
        if (Visited.count(Callee)) {
          // We have already seen this node, thus it has to be cloned.
          Callee = Node->replaceCalleeWithClone(I);
          Changed = true;
        }
        Visited.insert(Callee);
        Worklist.push(Callee);
      }
    }
  }
  return Changed;
}

PreservedAnalyses CSAGraphSplitterPass::run(Module &M,
                                            ModuleAnalysisManager &AM) {
  auto &CG = AM.getResult<CallGraphAnalysis>(M);
  if (!GraphSplitter(CG).run())
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

namespace {

struct CSAGraphSplitter : public ModulePass {
  static char ID;

  CSAGraphSplitter() : ModulePass(ID) {
    initializeCSAGraphSplitterPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<CallGraphWrapperPass>();
    AU.addPreserved<CallGraphWrapperPass>();
    ModulePass::getAnalysisUsage(AU);
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    auto &CG = getAnalysis<CallGraphWrapperPass>().getCallGraph();
    return GraphSplitter(CG).run();
  }
};

} // end anonymous namespace

char CSAGraphSplitter::ID = 0;

INITIALIZE_PASS_BEGIN(CSAGraphSplitter, DEBUG_TYPE,
                      "CSA: Eliminate graph's cross dependencies", false, false)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_END(CSAGraphSplitter, DEBUG_TYPE,
                    "CSA: Eliminate graph's cross dependencies", false, false)

Pass *llvm::createCSAGraphSplitterPass() { return new CSAGraphSplitter(); }
