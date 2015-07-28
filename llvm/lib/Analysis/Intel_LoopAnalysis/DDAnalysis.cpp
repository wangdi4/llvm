//===---- DDAnalysis.cpp - Provides Data Dependence Analysis *- C++ -*-----===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DD Analysis pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/Analysis/Intel_LoopAnalysis/DDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/SymbaseAssignment.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"
#include "llvm/Analysis/Intel_LoopAnalysis/DDTests.h"
#include "llvm/Support/Debug.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"

#include <vector>
#include <map>
#include <algorithm>
using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "dd-analysis"

FunctionPass *llvm::createDDAnalysisPass() { return new DDAnalysis(); }

char DDAnalysis::ID = 0;
INITIALIZE_PASS_BEGIN(DDAnalysis, "dda", "Data Dependence Analysis", false,
                      true)
INITIALIZE_PASS_DEPENDENCY(SymbaseAssignment)
INITIALIZE_PASS_DEPENDENCY(HIRParser)
INITIALIZE_PASS_END(DDAnalysis, "dda", "Data Dependence Analysis", false, true)

void DDAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {

  AU.setPreservesAll();
  AU.addRequired<HIRParser>();
  AU.addRequired<SymbaseAssignment>();

  // scev
  // need tbaa// or just general AA?
}

// \brief Because the graph is evaluated lazily, runOnFunction doesn't
// do any analysis
bool DDAnalysis::runOnFunction(Function &F) {

  HIRP = &getAnalysis<HIRParser>();
  SA = &getAnalysis<SymbaseAssignment>();

  // Compute TopSortNum - which is needed to determine forward or backward edges
  HLNodeUtils::resetTopSortNum();

  return false;
}

void DDAnalysis::markLoopBodyModified(HLLoop *L) {
  // TODO
}
void DDAnalysis::markLoopBoundsModified(HLLoop *L) {
  // TODO
}
void DDAnalysis::markTopLvlNonLoopNodeModified(HLRegion *R) {
  // TODO
}

DDGraph DDAnalysis::getGraph(HLNode *Node, bool InputEdgesReq) {
  // conservatively assume input edges are always invalid
  if (InputEdgesReq || !graphForNodeValid(Node)) {
    rebuildGraph(Node, InputEdgesReq);
  }
  return DDGraph(Node, &FunctionDDGraph);
}

// Visits all dd refs in Node and fills in the symbase to ref vector
// map based on symbase field of encountered dd refs. Does not assign
// symbase, assumes symbase of ddrefs is valid
// TODO make this a general DDref walker? Already did this for
// symbase assignement
class DDRefGatherer {

private:
  void visitDDNodeRefs(HLDDNode *Node);
  SymToRefs *RefMap;

public:
  DDRefGatherer(SymToRefs *CurRefs) : RefMap(CurRefs) {}

  void postVisit(HLNode *Node) {}
  void postVisit(HLDDNode *Node) {}
  void visit(HLNode *Node) {}
  void visit(HLDDNode *Node);
  bool isDone() { return false; }
  void addRef(DDRef *Ref);
};

void DDRefGatherer::addRef(DDRef *Ref) {
  unsigned int SymBase = (Ref)->getSymBase();
  (*RefMap)[SymBase].push_back(Ref);
}

void DDRefGatherer::visit(HLDDNode *Node) {
  for (auto I = Node->ddref_begin(), E = Node->ddref_end(); I != E; ++I) {
    if ((*I)->isConstant()) {
      // dont even bother gathering consts
    } else {
      // add ref and blobs to map
      addRef(*I);
      for (auto BRefI = (*I)->blob_cbegin(), BRefE = (*I)->blob_cend();
           BRefI != BRefE; ++BRefI) {
        addRef(*BRefI);
      }
    }
  }
}

void DDAnalysis::releaseMemory() {
  GraphValidityMap.clear();
  FunctionDDGraph.clear();
}

// Returns true if we must do dd testing between ref1 and ref2. We generally
// do not need to do testing between rvals, unless we need explicitly need input
// edges. There may be other reasons in the future certain refs will be excluded
// from testing
bool DDAnalysis::edgeNeeded(DDRef *Ref1, DDRef *Ref2, bool InputEdgesReq) {

  RegDDRef *RegRef1 = dyn_cast<RegDDRef>(Ref1);
  RegDDRef *RegRef2 = dyn_cast<RegDDRef>(Ref2);

  if ((RegRef1 && RegRef1->isLval()) || (RegRef2 && RegRef2->isLval()))
    return true;

  return InputEdgesReq;
}

// returns a direction vector used to test from Node's loop nesting level
// to the deepest of ref1 and ref2s level
DirectionVector DDAnalysis::getInputDV(HLNode *Node, DDRef *Ref1, DDRef *Ref2) {
  HLLoop *Parent1 = dyn_cast<HLLoop>(Ref1->getHLDDNode());
  HLLoop *Parent2 = dyn_cast<HLLoop>(Ref2->getHLDDNode());
  Parent1 = Parent1 ? Parent1 : Ref1->getHLDDNode()->getLexicalParentLoop();
  Parent2 = Parent2 ? Parent2 : Ref2->getHLDDNode()->getLexicalParentLoop();

  int Level1 = Parent1 ? Parent1->getNestingLevel() : 0;
  int Level2 = Parent2 ? Parent2->getNestingLevel() : 0;
  int DeepestLevel = std::max(Level1, Level2);

  if (DeepestLevel == 0)
    DeepestLevel = 1;

  int ShallowestLevel = 0;
  if (HLLoop *L = dyn_cast<HLLoop>(Node)) {
    ShallowestLevel = L->getNestingLevel();
  } else if (isa<HLRegion>(Node)) {
    ShallowestLevel = 1;
  } else {
    llvm_unreachable("Unexpected Node type in DD ");
  }
  assert(ShallowestLevel <= DeepestLevel && "Incorrect Input DV calculation");

  DirectionVector InputDV;
  for (int i = 1; i < ShallowestLevel; ++i) {
    InputDV.setDVAtLevel(DirectionVector::Direction::EQ, i);
  }

  for (int i = ShallowestLevel; i <= DeepestLevel; ++i) {
    InputDV.setDVAtLevel(DirectionVector::Direction::ALL, i);
  }

  return InputDV;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DDAnalysis::dumpSymBaseMap(SymToRefs &RefMap) {
  for (auto SymVecPair = RefMap.begin(), Last = RefMap.end();
       SymVecPair != Last; ++SymVecPair) {
    std::vector<DDRef *> &RefVec = SymVecPair->second;
    dbgs() << "Symbase " << SymVecPair->first << " contains: \n";
    for (auto Ref = RefVec.begin(), E = RefVec.end(); Ref != E; ++Ref) {
      dbgs() << "\t";
      (*Ref)->dump();
      dbgs() << "\n";
    }
  }
}
#endif

void DDAnalysis::rebuildGraph(HLNode *Node, bool BuildInputEdges) {
  //  collect all refs into symbase vector
  SymToRefs RefMap;
  DDRefGatherer Gatherer(&RefMap);

  HLNodeUtils::visit(&Gatherer, Node, true, true);

  DEBUG(dumpSymBaseMap(RefMap));
  // pairwise testing among all refs sharing a symbase
  for (auto SymVecPair = RefMap.begin(), Last = RefMap.end();
       SymVecPair != Last; ++SymVecPair) {
    std::vector<DDRef *> &RefVec = SymVecPair->second;
    if (RefVec.size() < 2)
      continue;

    for (auto Ref1 = RefVec.begin(), E = RefVec.end(); Ref1 != E; ++Ref1) {
      for (auto Ref2 = Ref1; Ref2 != E; ++Ref2) {
        if (edgeNeeded(*Ref1, *Ref2, BuildInputEdges)) {
          DDtest DA;
          DVectorTy inputDV;
          DVectorTy outputDVforward;
          DVectorTy outputDVbackward;
          DA.setInputDV(inputDV, 1, 9);

          bool IsDependent = DA.findDependences(
              *Ref1, *Ref2, inputDV, outputDVforward, outputDVbackward);
          //  Sample code to check output:
          //  first check IsDependent
          //  else  check outputDVforward[0] != Dependences::DVEntry::NONE;
          //  etc
          if (IsDependent) {
            DEBUG(dbgs() << " Found Dependence!\n");
          }
        }
      }
    }
  }
}

bool DDAnalysis::graphForNodeValid(HLNode *Node) {
  // TODO
  return false;
}

unsigned int DDAnalysis::getNewSymBase() { return SA->getNewSymBase(); }
