//===---- HIRDDAnalysis.cpp - Provides Data Dependence Analysis -----------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScopedNoAliasAA.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/DDTests.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGatherer.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#include <algorithm>
#include <map>
#include <vector>

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-dd-analysis"

// Rebuild nests in runOnFunction for loops of level n, 0 being whole region
// for testing.
// eg opt -hir-dd-analysis -hir-dd-analysis-verify=L1,L2 ... would result a walk
// of outermost loops,
// building
// graph along the way, then a walk of level 2 loops, rebuilding graph for those
// as well if graph isnt already cached. Best used with -verify to print graph
// afterward all builds are done
// This is useful for testing not only graph, but caching implementation as well
cl::list<DDVerificationLevel> VerifyLevelList(
    "hir-dd-analysis-verify", cl::CommaSeparated,
    cl::desc("DD graph built for Levels:"),
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
    ForceDDA("force-hir-dd-analysis", cl::init(false), cl::Hidden,
             cl::desc("forces graph construction for every request"));

FunctionPass *llvm::createHIRDDAnalysisPass() { return new HIRDDAnalysis(); }

char HIRDDAnalysis::ID = 0;
INITIALIZE_PASS_BEGIN(HIRDDAnalysis, "hir-dd-analysis",
                      "HIR Data Dependence Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScopedNoAliasAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TypeBasedAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_END(HIRDDAnalysis, "hir-dd-analysis",
                    "HIR Data Dependence Analysis", false, true)

void HIRDDAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {

  AU.setPreservesAll();
  AU.addRequiredTransitive<TargetLibraryInfoWrapperPass>();
  AU.addRequiredTransitive<HIRFramework>();

  AU.addUsedIfAvailable<ScopedNoAliasAAWrapperPass>();
  AU.addUsedIfAvailable<TypeBasedAAWrapperPass>();
  // TODO: Do we need to add scev alias analysis??
}

// \brief Because the graph is evaluated lazily, runOnFunction doesn't
// do any analysis
bool HIRDDAnalysis::runOnFunction(Function &F) {

  AAR.reset(
      new AAResults(getAnalysis<TargetLibraryInfoWrapperPass>().getTLI()));

  if (auto *Pass = getAnalysisIfAvailable<ScopedNoAliasAAWrapperPass>()) {
    AAR->addAAResult(Pass->getResult());
  }

  if (auto *Pass = getAnalysisIfAvailable<TypeBasedAAWrapperPass>()) {
    AAR->addAAResult(Pass->getResult());
  }

  HIRF = &getAnalysis<HIRFramework>();

  // If cl opts are present, build graph for requested loop levels
  for (unsigned I = 0; I != VerifyLevelList.size(); ++I) {
    DDVerificationLevel CurLevel = VerifyLevelList[I];
    GraphVerifier V(this, CurLevel);
    HLNodeUtils::visitAll(V);
  }

  return false;
}

void HIRDDAnalysis::markLoopBodyModified(const HLLoop *Loop) {
  invalidateGraph(Loop, false);
}

void HIRDDAnalysis::markLoopBoundsModified(const HLLoop *Loop) {
  invalidateGraph(Loop, true);
}

void HIRDDAnalysis::markNonLoopRegionModified(const HLRegion *Region) {
  ValidationMap[Region] = GraphState::Invalid;
}

DDGraph HIRDDAnalysis::getGraphImpl(const HLNode *Node, bool InputEdgesReq) {
  auto State = ValidationMap[Node];

  // conservatively assume input edges are always invalid
  if (ForceDDA || InputEdgesReq || State == GraphState::Invalid) {

    // Clean whole graph if a graph is requested for invalid Node.
    FunctionDDGraph.clear();
    ValidationMap.clear();

    buildGraph(Node, InputEdgesReq);
  } else if (State != GraphState::Valid) {
    buildGraph(Node, InputEdgesReq);
  }
  return DDGraph(Node, &FunctionDDGraph);
}

void HIRDDAnalysis::releaseMemory() {
  ValidationMap.clear();
  FunctionDDGraph.clear();
}

void HIRDDAnalysis::verifyAnalysis() const {}

// Returns true if we must do dd testing between ref1 and ref2. We generally
// do not need to do testing between rvals, unless we need explicitly need input
// edges. There may be other reasons in the future certain refs will be excluded
// from testing
bool HIRDDAnalysis::edgeNeeded(DDRef *Ref1, DDRef *Ref2, bool InputEdgesReq) {

  RegDDRef *RegRef1 = dyn_cast<RegDDRef>(Ref1);
  RegDDRef *RegRef2 = dyn_cast<RegDDRef>(Ref2);

  if ((RegRef1 && RegRef1->isLval()) || (RegRef2 && RegRef2->isLval())) {
    return true;
  }

  return InputEdgesReq;
}

// initializes direction vector used to test from Node's loop nesting level
// to the deepest of ref1 and ref2s level
void HIRDDAnalysis::setInputDV(DirectionVector &InputDV, HLNode *Node,
                               DDRef *Ref1, DDRef *Ref2) {
  HLLoop *Parent1 = dyn_cast<HLLoop>(Ref1->getHLDDNode());
  HLLoop *Parent2 = dyn_cast<HLLoop>(Ref2->getHLDDNode());
  Parent1 = Parent1 ? Parent1 : Ref1->getHLDDNode()->getLexicalParentLoop();
  Parent2 = Parent2 ? Parent2 : Ref2->getHLDDNode()->getLexicalParentLoop();

  int Level1 = Parent1 ? Parent1->getNestingLevel() : 0;
  int Level2 = Parent2 ? Parent2->getNestingLevel() : 0;
  int DeepestLevel = std::max(Level1, Level2);

  if (DeepestLevel == 0) {
    DeepestLevel = 1;
  }

  int ShallowestLevel = 0;
  if (HLLoop *L = dyn_cast<HLLoop>(Node)) {
    ShallowestLevel = L->getNestingLevel();
  } else if (isa<HLRegion>(Node)) {
    ShallowestLevel = 1;
  } else {
    llvm_unreachable("Unexpected Node type in DD ");
  }
  assert(ShallowestLevel <= DeepestLevel && "Incorrect Input DV calculation");

  InputDV.setAsInput(ShallowestLevel, DeepestLevel);
}

class HIRDDAnalysis::GraphStateUpdater final : public HLNodeVisitorBase {
  decltype(HIRDDAnalysis::ValidationMap) &ValidityMapRef;
  GraphState State;
public:
  GraphStateUpdater(decltype(HIRDDAnalysis::ValidationMap) &ValidityMapRef,
                      GraphState State)
      : ValidityMapRef(ValidityMapRef), State(State) {}

  void visit(const HLLoop *Loop) {
    ValidityMapRef[Loop] = State;
  }

  void visit(const HLRegion *Region) {
    ValidityMapRef[Region] = State;
  }

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
};

void HIRDDAnalysis::invalidateGraph(const HLLoop *Loop,
                                    bool InvalidateInnerLoops) {

  if (InvalidateInnerLoops) {
    GraphStateUpdater Visitor(ValidationMap, GraphState::Invalid);
    HLNodeUtils::visit(Visitor, Loop);
  }

  const HLLoop *Node = Loop;
  const HLLoop *PrevNode = nullptr;

  do {
    ValidationMap[Node] = GraphState::Invalid;
    PrevNode = Node;
  } while ((Node = Node->getParentLoop()));

  ValidationMap[PrevNode->getParentRegion()] = GraphState::Invalid;
}

const HLNode *HIRDDAnalysis::getDDRefRegionLoopContainer(const DDRef *Ref) {
  const HLNode *Node = Ref->getHLDDNode();
  const HLLoop *ParentLoop = Node->getLexicalParentLoop();
  if (ParentLoop) {
    return ParentLoop;
  }

  return Node->getParentRegion();
}

bool HIRDDAnalysis::isEdgeValid(const DDRef *Ref1, const DDRef *Ref2) {
  auto RefParent1 = getDDRefRegionLoopContainer(Ref1);
  auto RefParent2 = getDDRefRegionLoopContainer(Ref2);
  return ValidationMap[RefParent1] == GraphState::Valid &&
         ValidationMap[RefParent2] == GraphState::Valid;
}

void HIRDDAnalysis::buildGraph(const HLNode *Node, bool BuildInputEdges) {
  assert((isa<HLRegion>(Node) || isa<HLLoop>(Node)) &&
         "Node should be HLLoop or HLRegion");

  DEBUG(dbgs() << "buildGraph() for:\n");
  DEBUG(Node->dump());

  NonConstantRefGatherer::MapTy RefMap;

  if (const HLLoop *Loop = dyn_cast<HLLoop>(Node)) {
    NonConstantRefGatherer::gatherRange(Loop->child_begin(), Loop->child_end(),
                                        RefMap);
  } else {
    NonConstantRefGatherer::gather(Node, RefMap);
  }

  DEBUG(dbgs() << "References:\n");
  DEBUG(NonConstantRefGatherer::dump(RefMap));

  // pairwise testing among all refs sharing a symbase
  for (auto SymVecPair = RefMap.begin(), Last = RefMap.end();
       SymVecPair != Last; ++SymVecPair) {
    auto &RefVec = SymVecPair->second;
    
    for (auto Ref1I = RefVec.begin(), E = RefVec.end(); Ref1I != E; ++Ref1I) {
      DDRef *Ref1 = *Ref1I;

      for (auto Ref2I = Ref1I; Ref2I != E; ++Ref2I) {
        DDRef *Ref2 = *Ref2I;

        if (edgeNeeded(Ref1, Ref2, BuildInputEdges) &&
            !isEdgeValid(Ref1, Ref2)) {
          DDTest DA(*AAR);
          DirectionVector InputDV;
          DirectionVector OutputDVForward;
          DirectionVector OutputDVBackward;
          bool IsLoopIndepDepTemp = false;
          // TODO this is incorrect, we need a direction vector of
          //= = * for 3rd level inermost loops
          InputDV.setAsInput();

          DA.findDependences(Ref1, Ref2, InputDV, OutputDVForward,
                             OutputDVBackward, &IsLoopIndepDepTemp);
          //  Sample code to check output:
          //  first check IsDependent
          //  else  check outputDVforward[0] != Dependences::DVEntry::NONE;
          //  etc

          // TODO Blindly adding edges means we have the unfortunate side effect
          // of obliterating edges if we request outermost loop graph then
          // innermost loop graph. If refinement is not possible, we should
          // keep the previous result cached somewhere.
          if (OutputDVForward[0] != DVKind::NONE) {
            DDEdge Edge =
                DDEdge(Ref1, Ref2, OutputDVForward, IsLoopIndepDepTemp);

            // DEBUG(dbgs() << "Got edge of :");
            // DEBUG(Edge.dump());
            FunctionDDGraph.addEdge(std::move(Edge));
          }

          if (OutputDVBackward[0] != DVKind::NONE) {
            DDEdge Edge = DDEdge(Ref2, Ref1, OutputDVBackward);
            // DEBUG(dbgs() << "Got back edge of :");
            // DEBUG(Edge.dump());
            FunctionDDGraph.addEdge(std::move(Edge));
          }
        }
      }
    }
  }

  GraphStateUpdater Visitor(ValidationMap, GraphState::Valid);
  HLNodeUtils::visit(Visitor, Node);
}

bool HIRDDAnalysis::refineDV(DDRef *SrcDDRef, DDRef *DstDDRef,
                             unsigned InnermostNestingLevel,
                             unsigned OutermostNestingLevel,
                             DirectionVector &RefinedDV, bool *IsIndependent) {

  bool IsDVRefined = false;
  *IsIndependent = false;
  RegDDRef *RegDDref = dyn_cast<RegDDRef>(DstDDRef);

  if (RegDDref && !(RegDDref->isTerminalRef())) {
    DDTest DA(*AAR);
    DirectionVector InputDV;
    InputDV.setAsInput(InnermostNestingLevel, OutermostNestingLevel);
    auto Result = DA.depends(SrcDDRef, DstDDRef, InputDV);
    if (Result == nullptr) {
      *IsIndependent = true;
      return true;
    }
    for (unsigned I = 1; I <= Result->getLevels(); ++I) {
      RefinedDV[I - 1] = Result->getDirection(I);
      IsDVRefined = true;
    }
  }

  return IsDVRefined;
}

bool HIRDDAnalysis::graphForNodeValid(const HLNode *Node) {
  if (ForceDDA) {
    return false;
  }

  return ValidationMap[Node] == GraphState::Valid;
}

void HIRDDAnalysis::print(raw_ostream &OS, const Module *M) const {
  OS << "DD graph for function:\n";
  FunctionDDGraph.print(OS);
}

// Graph verifier does not build input edges
void HIRDDAnalysis::GraphVerifier::visit(HLRegion *Region) {
  if (CurLevel == DDVerificationLevel::Region &&
      !CurDDA->graphForNodeValid(Region)) {
    CurDDA->buildGraph(Region, false);
  }
}

void HIRDDAnalysis::GraphVerifier::visit(HLLoop *Loop) {
  if (Loop->getNestingLevel() == CurLevel ||
      (Loop->isInnermost() && CurLevel == DDVerificationLevel::Innermost)) {
    if (!CurDDA->graphForNodeValid(Loop)) {
      CurDDA->buildGraph(Loop, false);
    }
  }
}
