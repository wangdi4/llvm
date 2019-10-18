//===---- HIRDDAnalysis.cpp - Provides Data Dependence Analysis -----------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_StdContainerAA.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScopedNoAliasAA.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDTests.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

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
               clEnumVal(Innermost, "Build for innermost loops only")));

static cl::list<int> DumpGraphForNodeNumbers(
    "hir-dd-analysis-dump-nodes", cl::CommaSeparated,
    cl::desc("List of node numbers to dump their DD graph"));

// Disable caching behavior and rebuild graph for every request.
static cl::opt<bool>
    ForceDDA("force-hir-dd-analysis", cl::init(false), cl::Hidden,
             cl::desc("forces graph construction for every request"));

// A low number of queries should suffice in practice. If a
// dominating/post-dominating temp definition which kills a redundant edge
// exists, it should be 'close by'.
static cl::opt<unsigned> DominationQueryThreshold(
    "hir-dd-domination-query-threshold", cl::init(3), cl::Hidden,
    cl::desc("Approximate threshold for number of queries allowed (per temp "
             "pair) to refine edges based on control flow."));

enum ConstructDDEdgeType { None, Forward, Backward, Both };

FunctionPass *llvm::createHIRDDAnalysisPass() {
  return new HIRDDAnalysisWrapperPass();
}

AnalysisKey HIRDDAnalysisPass::Key;
HIRDDAnalysis HIRDDAnalysisPass::run(Function &F, FunctionAnalysisManager &AM) {
  AAResults *AAR = new AAResults(AM.getResult<TargetLibraryAnalysis>(F));
  if (auto *AAResult = AM.getCachedResult<ScopedNoAliasAA>(F)) {
    AAR->addAAResult(*AAResult);
  }
  if (auto *AAResult = AM.getCachedResult<TypeBasedAA>(F)) {
    AAR->addAAResult(*AAResult);
  }
  if (auto *AAResult = AM.getCachedResult<StdContainerAA>(F)) {
    AAR->addAAResult(*AAResult);
  }

  auto &MAMProxy = AM.getResult<ModuleAnalysisManagerFunctionProxy>(F);
  auto &MAM = MAMProxy.getManager();
  if (auto *AAResult = MAM.getCachedResult<AndersensAA>(*F.getParent())) {
    AAR->addAAResult(*AAResult);
  }

  if (auto *AAResult = AM.getCachedResult<BasicAA>(F)) {
    AAR->addAAResult(*AAResult);
  }

  return HIRDDAnalysis(AM.getResult<HIRFrameworkAnalysis>(F), AAR);
}

char HIRDDAnalysisWrapperPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRDDAnalysisWrapperPass, "hir-dd-analysis",
                      "HIR Data Dependence Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScopedNoAliasAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TypeBasedAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(BasicAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(StdContainerAAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRDDAnalysisWrapperPass, "hir-dd-analysis",
                    "HIR Data Dependence Analysis", false, true)

void HIRDDAnalysisWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<TargetLibraryInfoWrapperPass>();
  AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
  // Loop Statistics is not used by this pass directly but it used by
  // HLNodeUtils::dominates() utility. This is a workaround to keep the pass
  // manager from freeing it.
  AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();

  AU.addUsedIfAvailable<ScopedNoAliasAAWrapperPass>();
  AU.addUsedIfAvailable<TypeBasedAAWrapperPass>();
  AU.addUsedIfAvailable<StdContainerAAWrapperPass>();
  AU.addUsedIfAvailable<BasicAAWrapperPass>();
  // TODO: Do we need to add scev alias analysis??
}

// \brief Because the graph is evaluated lazily, runOnFunction doesn't
// do any analysis
bool HIRDDAnalysisWrapperPass::runOnFunction(Function &F) {
  AAResults *AAR =
      new AAResults(getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F));

  if (auto *Pass = getAnalysisIfAvailable<ScopedNoAliasAAWrapperPass>()) {
    AAR->addAAResult(Pass->getResult());
  }

  if (auto *Pass = getAnalysisIfAvailable<TypeBasedAAWrapperPass>()) {
    AAR->addAAResult(Pass->getResult());
  }

  if (auto *Pass = getAnalysisIfAvailable<StdContainerAAWrapperPass>()) {
    AAR->addAAResult(Pass->getResult());
  }

  if (auto *Pass = getAnalysisIfAvailable<AndersensAAWrapperPass>()) {
    AAR->addAAResult(Pass->getResult());
  }

  if (auto *Pass = getAnalysisIfAvailable<BasicAAWrapperPass>()) {
    AAR->addAAResult(Pass->getResult());
  }

  DDA.reset(
      new HIRDDAnalysis(getAnalysis<HIRFrameworkWrapperPass>().getHIR(), AAR));

  return false;
}

HIRDDAnalysis::HIRDDAnalysis(llvm::loopopt::HIRFramework &HIRF,
                             llvm::AAResults *AAR)
    : HIRAnalysis(HIRF), AAR(AAR) {}

void HIRDDAnalysisWrapperPass::releaseMemory() { DDA.reset(); }

void HIRDDAnalysis::forceBuild() {
  // If cl opts are present, build graph for requested loop levels
  for (unsigned I = 0; I != VerifyLevelList.size(); ++I) {
    DDVerificationLevel CurLevel = VerifyLevelList[I];
    GraphVerifier V(this, CurLevel);
    HIRF.getHLNodeUtils().visitAll(V);
  }
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

DDGraph HIRDDAnalysis::getGraphImpl(const HLRegion *Region,
                                    const HLNode *Node) {
  DDGraphTy &DDG = RegionDDGraph[Region];
  auto State = ValidationMap[Node];

  if (ForceDDA || State == GraphState::Invalid ||
      // If loop is marked NoData but the parent region is marked as Invalid, we
      // should rebuild DD from scratch as some nodes could have been moved from
      // an old loop to this loop.
      (isa<HLLoop>(Node) && (State == GraphState::NoData) &&
       (ValidationMap[Region] == GraphState::Invalid))) {

    // Clean whole graph if a graph is requested for invalid Node.
    DDG.clear();
    GraphStateUpdater GSU(ValidationMap, GraphState::NoData);
    HLNodeUtils::visit(GSU, Region);

    buildGraph(DDG, Node);
  } else if (State != GraphState::Valid) {
    buildGraph(DDG, Node);
  }

  return DDGraph(Node, &DDG);
}

// Returns true if we must do dd testing between ref1 and ref2. We generally
// do not need to do testing between rvals, unless we need explicitly need input
// edges. There may be other reasons in the future certain refs will be excluded
// from testing
static ConstructDDEdgeType edgeNeeded(RefVectorTy<DDRef>::iterator Ref1It,
                                      RefVectorTy<DDRef>::iterator Ref2It,
                                      RefVectorTy<DDRef>::iterator BeginIt,
                                      RefVectorTy<DDRef>::iterator EndIt,
                                      bool IsGraphForInnermostLoop) {

  DDRef *Ref1 = *Ref1It;
  DDRef *Ref2 = *Ref2It;

  if (Ref1->isRval() && Ref2->isRval()) {
    return ConstructDDEdgeType::None;
  }

  if (!Ref1->isTerminalRef()) {
    return ConstructDDEdgeType::Both;
  }

  // We do not build self edges
  if (Ref1 == Ref2) {
    return ConstructDDEdgeType::None;
  }

  auto Ref1Node = Ref1->getHLDDNode();
  auto Ref2Node = Ref2->getHLDDNode();

  // Look refs in-between Ref1 and Ref2 to ignore the forward edge
  bool NeedForwardEdge = true;
  unsigned QueryCount = 0;
  for (auto It = std::next(Ref1It), E = Ref2It; It != E; ++It) {
    if (QueryCount == DominationQueryThreshold) {
      break;
    }

    DDRef *InBetweenRef = *It;
    bool IsLval = InBetweenRef->isLval();

    if (IsLval) {
      ++QueryCount;

      if (HLNodeUtils::postDominates(InBetweenRef->getHLDDNode(), Ref1Node) ||
          HLNodeUtils::dominates(InBetweenRef->getHLDDNode(), Ref2Node)) {
        LLVM_DEBUG(dbgs() << "Skipping forward edge from "
                          << Ref1->getHLDDNode()->getNumber() << "("
                          << (Ref1->isLval() ? "lval" : "rval") << ")"
                          << " to " << Ref2->getHLDDNode()->getNumber() << "("
                          << (Ref2->isLval() ? "lval" : "rval") << ")"
                          << " because of inbetween def at "
                          << InBetweenRef->getHLDDNode()->getNumber() << "("
                          << (InBetweenRef->isLval() ? "lval" : "rval") << ")"
                          << "\n");
        NeedForwardEdge = false;
        break;
      }
    }
  }

  auto Parent1 = Ref1Node->getLexicalParentLoop();
  if (!IsGraphForInnermostLoop &&
      (!Parent1 || !Parent1->isInnermost() ||
       (Parent1 != Ref2Node->getLexicalParentLoop()))) {
    return NeedForwardEdge ? ConstructDDEdgeType::Both
                           : ConstructDDEdgeType::Backward;
  }

  // Look refs before Ref1 to ignore the backward edge
  bool NeedBackwardEdge = true;
  std::reverse_iterator<decltype(Ref1It)> RI(Ref1It);
  std::reverse_iterator<decltype(BeginIt)> RE(BeginIt);
  QueryCount = 0;

  for (auto It = RI; It != RE; ++It) {
    if (QueryCount == DominationQueryThreshold) {
      break;
    }

    DDRef *UpwardRef = *It;
    if (!IsGraphForInnermostLoop &&
        (UpwardRef->getLexicalParentLoop() != Parent1)) {
      break;
    }

    bool IsLval = UpwardRef->isLval();
    if (IsLval) {
      ++QueryCount;

      if (HLNodeUtils::dominates(UpwardRef->getHLDDNode(), Ref1Node)) {
        LLVM_DEBUG(dbgs() << "Skipping backward edge from "
                          << Ref2->getHLDDNode()->getNumber() << "("
                          << (Ref2->isLval() ? "lval" : "rval") << ")"
                          << " to " << Ref1->getHLDDNode()->getNumber() << "("
                          << (Ref1->isLval() ? "lval" : "rval") << ")"
                          << " because of upward def at "
                          << UpwardRef->getHLDDNode()->getNumber() << "("
                          << (UpwardRef->isLval() ? "lval" : "rval") << ")"
                          << "\n");
        NeedBackwardEdge = false;
        break;
      }
    }
  }

  // Look refs after Ref2 to see if we can still ignore the backward edge
  if (NeedBackwardEdge) {
    QueryCount = 0;
    for (auto It = std::next(Ref2It), E = EndIt; It != E; ++It) {
      if (QueryCount == DominationQueryThreshold) {
        break;
      }

      DDRef *DownwardRef = *It;
      if (!IsGraphForInnermostLoop &&
          (DownwardRef->getLexicalParentLoop() != Parent1)) {
        break;
      }

      bool IsLval = DownwardRef->isLval();
      if (IsLval) {
        ++QueryCount;

        if (HLNodeUtils::postDominates(DownwardRef->getHLDDNode(), Ref2Node)) {
          LLVM_DEBUG(dbgs() << "Skipping backward edge from "
                            << Ref2->getHLDDNode()->getNumber() << "("
                            << (Ref2->isLval() ? "lval" : "rval") << ")"
                            << " to " << Ref1->getHLDDNode()->getNumber() << "("
                            << (Ref1->isLval() ? "lval" : "rval") << ")"
                            << " because of downward def at "
                            << DownwardRef->getHLDDNode()->getNumber() << "("
                            << (DownwardRef->isLval() ? "lval" : "rval") << ")"
                            << "\n");
          NeedBackwardEdge = false;
          break;
        }
      }
    }
  }

  if (NeedForwardEdge && NeedBackwardEdge) {
    return ConstructDDEdgeType::Both;
  }

  if (NeedForwardEdge) {
    return ConstructDDEdgeType::Forward;
  }

  if (NeedBackwardEdge) {
    return ConstructDDEdgeType::Backward;
  }

  return ConstructDDEdgeType::None;
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

std::tuple<const HLNode *, bool>
HIRDDAnalysis::getDDRefRegionLoopContainer(const DDRef *Ref) {
  const HLNode *Node = Ref->getHLDDNode();
  const HLLoop *ParentLoop = Node->getLexicalParentLoop();
  if (ParentLoop) {
    return std::make_tuple(ParentLoop, true);
  }

  return std::make_tuple(Node->getParentRegion(), false);
}

bool HIRDDAnalysis::isEdgeValid(const DDRef *Ref1, const DDRef *Ref2) {
  bool IsParentLoop1;
  bool IsParentLoop2;
  const HLNode *RefParent1;
  const HLNode *RefParent2;

  std::tie(RefParent1, IsParentLoop1) = getDDRefRegionLoopContainer(Ref1);
  std::tie(RefParent2, IsParentLoop2) = getDDRefRegionLoopContainer(Ref2);

  if (!IsParentLoop1) {
    return ValidationMap[RefParent1] == GraphState::Valid;
  } else if (!IsParentLoop2) {
    return ValidationMap[RefParent2] == GraphState::Valid;
  }

  const HLLoop *RefParentLoop1 = cast<HLLoop>(RefParent1);
  const HLLoop *RefParentLoop2 = cast<HLLoop>(RefParent2);

  const HLLoop *Ancestor =
      HLNodeUtils::getLowestCommonAncestorLoop(RefParentLoop1, RefParentLoop2);
  if (Ancestor) {
    return ValidationMap[Ancestor] == GraphState::Valid;
  }

  const HLNode *TopLoop =
      RefParentLoop1->getNestingLevel() < RefParentLoop2->getNestingLevel()
          ? RefParent1
          : RefParent2;
  return ValidationMap[TopLoop->getParentRegion()] == GraphState::Valid;
}

void HIRDDAnalysis::buildGraph(DDGraphTy &DDG, const HLNode *Node) {
  assert((isa<HLRegion>(Node) || isa<HLLoop>(Node)) &&
         "Node should be HLLoop or HLRegion");

  LLVM_DEBUG(dbgs() << "buildGraph() for:\n");
  LLVM_DEBUG(Node->dump());

  DDARefGatherer::MapTy RefMap;
  bool IsGraphForInnermostLoop = false;

  if (const HLLoop *Loop = dyn_cast<HLLoop>(Node)) {
    DDARefGatherer::gatherRange(Loop->child_begin(), Loop->child_end(), RefMap);
    IsGraphForInnermostLoop = Loop->isInnermost();
  } else {
    DDARefGatherer::gather(Node, RefMap);
  }

  LLVM_DEBUG(dbgs() << "References:\n");
  LLVM_DEBUG(DDARefGatherer::dump(RefMap));

  // pairwise testing among all refs sharing a symbase
  for (auto SymVecPair = RefMap.begin(), Last = RefMap.end();
       SymVecPair != Last; ++SymVecPair) {
    auto &RefVec = SymVecPair->second;
    auto BeginIt = RefVec.begin();
    auto EndIt = RefVec.end();

    for (auto Ref1It = BeginIt; Ref1It != EndIt; ++Ref1It) {
      DDRef *Ref1 = *Ref1It;

      for (auto Ref2It = Ref1It; Ref2It != EndIt; ++Ref2It) {
        DDRef *Ref2 = *Ref2It;

        ConstructDDEdgeType NeededEdgeType =
            edgeNeeded(Ref1It, Ref2It, BeginIt, EndIt, IsGraphForInnermostLoop);
        if (NeededEdgeType != ConstructDDEdgeType::None &&
            !isEdgeValid(Ref1, Ref2)) {
          DDTest DT(*AAR, Node->getHLNodeUtils());
          DirectionVector InputDV;
          DirectionVector OutputDVForward;
          DirectionVector OutputDVBackward;
          DistanceVector OutputDistVForward;
          DistanceVector OutputDistVBackward;

          bool IsLoopIndepDepTemp = false;
          // TODO this is incorrect, we need a direction vector of
          //= = * for 3rd level inermost loops
          InputDV.setAsInput();

          DT.findDependencies(Ref1, Ref2, InputDV, OutputDVForward,
                              OutputDVBackward, OutputDistVForward,
                              OutputDistVBackward, &IsLoopIndepDepTemp);

          //  Sample code to check output:
          //  first check IsDependent
          //  else  check outputDVforward[0] != Dependences::DVEntry::NONE;
          //  etc

          // TODO Blindly adding edges means we have the unfortunate side
          // effect of obliterating edges if we request outermost loop graph
          // then innermost loop graph. If refinement is not possible, we
          // should keep the previous result cached somewhere.

          // Sample code to check for Dep Distance
          // flow (< >) (2 -4)
          //
          //  DistTy Distance = Edge->getDistanceAtLevel(Level);
          //  if (DepDist != UnknownDistance) {
          //     legal to unroll i1 loop with factor up to Distance
          //  }
          //  Assuming we want to unroll & jam for loop i2, it should not hit
          //  the code
          //  for checking distance -2, otherwise something is invalid
          //  flow (< > >) (2 -2 -1)

          if (OutputDVForward[0] != DVKind::NONE &&
              (NeededEdgeType & ConstructDDEdgeType::Forward)) {
            DDEdge Edge = DDEdge(Ref1, Ref2, OutputDVForward,
                                 OutputDistVForward, IsLoopIndepDepTemp);
            // LLVM_DEBUG(dbgs() << "Got edge of :");
            // LLVM_DEBUG(Edge.dump());
            DDG.addEdge(std::move(Edge));
          }

          if (OutputDVBackward[0] != DVKind::NONE &&
              (NeededEdgeType & ConstructDDEdgeType::Backward)) {
            DDEdge Edge =
                DDEdge(Ref2, Ref1, OutputDVBackward, OutputDistVBackward);
            // LLVM_DEBUG(dbgs() << "Got back edge of :");
            // LLVM_DEBUG(Edge.dump());
            DDG.addEdge(std::move(Edge));
          }
        }
      }
    }
  }

  GraphStateUpdater GSU(ValidationMap, GraphState::Valid);
  HLNodeUtils::visit(GSU, Node);
}

bool HIRDDAnalysis::isRefinableDepAtLevel(const DDEdge *Edge,
                                          unsigned Level) const {

  const DirectionVector *DV = &Edge->getDV();
  if (!DV->isRefinableAtLevel(Level)) {
    return false;
  }

  const RegDDRef *SrcDDRef = dyn_cast<RegDDRef>(Edge->getSrc());
  if (!SrcDDRef) {
    return false;
  }
  const RegDDRef *DstDDRef = dyn_cast<RegDDRef>(Edge->getSink());
  if (!DstDDRef) {
    return false;
  }

  if (!SrcDDRef->isMemRef()) {
    return false;
  }

  assert(DstDDRef->isMemRef() && "MemRef expected");

  // There are very few cases that the first DD build, testing for all *,
  // do not produce a precise DV.  Restrict it to limited cases to save
  // compile time
  // We can extend to more cases as needed.
  if (!DDTest::isDelinearizeCandidate(SrcDDRef) ||
      !DDTest::isDelinearizeCandidate(DstDDRef)) {
    return false;
  }
  return true;
}

RefinedDependence HIRDDAnalysis::refineDV(const DDRef *SrcDDRef,
                                          const DDRef *DstDDRef,
                                          unsigned StartNestingLevel,
                                          unsigned DeepestNestingLevel,
                                          bool ForFusion) const {

  RefinedDependence Dep;

  const RegDDRef *RegRef = dyn_cast<RegDDRef>(DstDDRef);

  if (RegRef && !(RegRef->isTerminalRef())) {
    DDTest DT(*AAR, RegRef->getHLDDNode()->getHLNodeUtils());

    DirectionVector &InputDV = Dep.getDV();
    //  For Start = 3, Deepest = 3, when testing for innermost loop dep,
    //  DV constructed: (= = *)
    InputDV.setAsInput(StartNestingLevel, DeepestNestingLevel);

    auto Result = DT.depends(SrcDDRef, DstDDRef, InputDV, false, ForFusion);

    if (Result != nullptr) {
      Dep.setRefined();

      DirectionVector &RefinedDV = Dep.getDV();
      DistanceVector &RefinedDistV = Dep.getDist();

      for (unsigned I = 1; I <= Result->getLevels(); ++I) {
        RefinedDV[I - 1] = Result->getDirection(I);
        RefinedDistV[I - 1] = DT.mapDVToDist(RefinedDV[I - 1], I, *Result);
      }
    } else {
      Dep.setIndependent();
    }
  }

  return Dep;
}

void HIRDDAnalysis::printAnalysis(raw_ostream &OS) const {

  // Need a non-const pointer to force build for opt -analyze mode.
  auto NonConstDDA = const_cast<HIRDDAnalysis *>(this);

  if (DumpGraphForNodeNumbers.empty()) {
    NonConstDDA->forceBuild();
    OS << "DD graph for function " << HIRF.getFunction().getName() << ":\n";
    for (auto &Pair : RegionDDGraph) {
      OS << "Region " << Pair.first->getNumber() << ":\n";
      Pair.second.print(OS);
      OS << "\n";
    }
  } else {
    // Dump graph for each node specified by the cl opts.
    for (int NodeNumber : DumpGraphForNodeNumbers) {
      ForEach<HLNode>::visitRange(
          HIRF.hir_begin(), HIRF.hir_end(),
          [NodeNumber, NonConstDDA](const HLNode *Node) {
            if (NodeNumber == -1 ||
                NodeNumber == static_cast<int>(Node->getNumber())) {
              DDGraph DDG(nullptr, nullptr);

              if (const HLRegion *Region = dyn_cast<HLRegion>(Node)) {
                DDG = NonConstDDA->getGraph(Region);
              } else if (const HLLoop *Loop = dyn_cast<HLLoop>(Node)) {
                DDG = NonConstDDA->getGraph(Loop);
              } else {
                return;
              }

              dbgs() << "Graph for node <" << Node->getNumber() << ">\n";
              DDG.dump();
            }
          });
    }
  }
}

// Graph verifier does not build input edges
void HIRDDAnalysis::GraphVerifier::visit(HLRegion *Region) {
  if (CurLevel == DDVerificationLevel::Region) {
    CurDDA->getGraph(Region);
  }
}

void HIRDDAnalysis::GraphVerifier::visit(HLLoop *Loop) {
  if (Loop->getNestingLevel() == CurLevel ||
      (Loop->isInnermost() && CurLevel == DDVerificationLevel::Innermost)) {
    CurDDA->getGraph(Loop);
  }
}

bool HIRDDAnalysis::doRefsAlias(const RegDDRef *SrcRef,
                                const RegDDRef *DstRef) const {
  DDTest DT(*AAR, SrcRef->getHLDDNode()->getHLNodeUtils());
  return !DT.queryAAIndep(SrcRef, DstRef);
}
