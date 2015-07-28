//===---- DDAnalysis.cpp - Provides Data Dependence Analysis --------------===//
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
#include "llvm/Support/CommandLine.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

#include <vector>
#include <map>
#include <algorithm>
using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "dd-analysis"

// Rebuild nests in runOnFunction for loops of level n, 0 being whole region
// for testing.
// eg opt -dda -dda-verify=L1,L2 ... would result a walk of outermost loops,
// building
// graph along the way, then a walk of level 2 loops, rebuilding graph for those
// as well if graph isnt already cached. Best used with -verify to print graph
// afterward all builds are done
// This is useful for testing not only graph, but caching implementation as well
cl::list<DDVerificationLevel> VerifyLevelList(
    "dda-verify", cl::CommaSeparated, cl::desc("DD graph built for Levels:"),
    cl::values(clEnumVal(Region, "Build for entire region"),
               clEnumVal(L1, "Build for Loop at Level 1(Outermost)"),
               clEnumVal(L2, "Build for Loop at Level 2"),
               clEnumVal(L3, "Build for Loop at Level 3"),
               clEnumVal(L4, "Build for Loop at Level 4"),
               clEnumVal(L5, "Build for Loop at Level 5"),
               clEnumVal(L6, "Build for Loop at Level 6"),
               clEnumVal(L7, "Build for Loop at Level 7"),
               clEnumVal(L8, "Build for Loop at Level 8"),
               clEnumVal(L9, "Build for Loop at Level 9"),
               clEnumVal(Innermost, "Build for innermost loops only"),
               clEnumValEnd));

// Disable caching behavior and rebuild graph for every request.
static cl::opt<bool>
    forceDDA("force-DDA", cl::init(false), cl::Hidden,
             cl::desc("forces graph construction for every request"));

FunctionPass *llvm::createDDAnalysisPass() { return new DDAnalysis(); }

char DDAnalysis::ID = 0;
INITIALIZE_PASS_BEGIN(DDAnalysis, "dda", "Data Dependence Analysis", false,
                      true)
INITIALIZE_PASS_DEPENDENCY(SymbaseAssignment)
INITIALIZE_PASS_DEPENDENCY(HIRParser)
INITIALIZE_PASS_END(DDAnalysis, "dda", "Data Dependence Analysis", false, true)

void DDAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {

  AU.setPreservesAll();
  AU.addRequiredTransitive<HIRParser>();
  AU.addRequiredTransitive<SymbaseAssignment>();

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

  // If cl opts are present, build graph for requested loop levels
  for (unsigned I = 0; I != VerifyLevelList.size(); ++I) {
    DDVerificationLevel CurLevel = VerifyLevelList[I];
    GraphVerifier V(this, CurLevel);
    HLNodeUtils::visitAll(&V);
  }

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
  bool skipRecursion(HLNode *Node) { return false; }
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

// initializes direction vector used to test from Node's loop nesting level
// to the deepest of ref1 and ref2s level
void DDAnalysis::setInputDV(DVectorTy &InputDV, HLNode *Node, DDRef *Ref1,
                            DDRef *Ref2) {
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

  for (int I = 1; I < ShallowestLevel; ++I) {
    InputDV[I - 1] = DV::EQ;
  }

  for (int I = ShallowestLevel; I <= DeepestLevel; ++I) {
    InputDV[I - 1] = DV::ALL;
  }
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

  HLNodeUtils::visit(&Gatherer, Node, true, true, true);

  DEBUG(dbgs() << "Building graph for:\n");
  DEBUG(dumpSymBaseMap(RefMap));
  DEBUG(Node->dump());
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
          DVectorTy OutputDVForward;
          DVectorTy OutputDVBackward;
          // TODO this is incorrect, we need a direction vector of
          //= = * for 3rd level inermost loops
          DA.setInputDV(inputDV, 1, 9);

          DA.findDependences(*Ref1, *Ref2, inputDV, OutputDVForward,
                             OutputDVBackward);
          //  Sample code to check output:
          //  first check IsDependent
          //  else  check outputDVforward[0] != Dependences::DVEntry::NONE;
          //  etc

          // TODO Blindly adding edges means we have the unfortunate side effect
          // of obliterating edges if we request outermost loop graph then
          // innermost loop graph. If refinement is not possible, we should
          // keep the previous result cached somewhere.
          if (OutputDVForward[0] != DV::NONE) {
            DDEdge Edge = DDEdge(*Ref1, *Ref2, OutputDVForward);
            // DEBUG(dbgs() << "Got edge of :");
            // DEBUG(Edge.dump());
            FunctionDDGraph.addEdge(Edge);
          }

          if (OutputDVBackward[0] != DV::NONE) {
            DDEdge Edge = DDEdge(*Ref2, *Ref1, OutputDVBackward);
            // DEBUG(dbgs() << "Got back edge of :");
            // DEBUG(Edge.dump());
            FunctionDDGraph.addEdge(Edge);
          }
        }
      }
    }
  }
}

bool DDAnalysis::graphForNodeValid(HLNode *Node) {
  if (forceDDA)
    return false;

  // TODO
  return false;
}
void DDAnalysis::print(raw_ostream &OS, const Module *M) const {
  OS << "DD graph for function:\n";
  FunctionDDGraph.print(OS);
}

// Graph verifier does not build input edges
void DDAnalysis::GraphVerifier::visit(HLRegion *Region) {
  if (CurLevel == DDVerificationLevel::Region &&
      !CurDDA->graphForNodeValid(Region)) {
    CurDDA->rebuildGraph(Region, false);
  }
}

void DDAnalysis::GraphVerifier::visit(HLLoop *Loop) {
  if (Loop->getNestingLevel() == CurLevel ||
      (Loop->isInnermost() && CurLevel == DDVerificationLevel::Innermost)) {
    if (!CurDDA->graphForNodeValid(Loop)) {
      CurDDA->rebuildGraph(Loop, false);
    }
  }
}

unsigned int DDAnalysis::getNewSymBase() { return SA->getNewSymBase(); }
