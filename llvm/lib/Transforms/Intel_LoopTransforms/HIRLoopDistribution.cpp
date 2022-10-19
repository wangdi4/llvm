//===----- HIRLoopDistribution.cpp - Distribution of HIR loops  -----------===//
//
// Copyright (C) 2015-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Implements HIR Loop Distribution. Works on Loops from innermost to outermost,
// distributing according to specified heuristics. Two important models are
// distribution with intent to form perfect loop nests(enables more
// optimizations such as mem set recognition, interchange) and
// distribution to break recurrences(enables vectorization)
//===----------------------------------------------------------------------===//
//

#include "llvm/ADT/SCCIterator.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSparseArrayReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "HIRLoopDistribution.h"

#define DEBUG_TYPE "hir-loop-distribute"

#define LLVM_DEBUG_DDG(X) DEBUG_WITH_TYPE("hir-loop-distribute-ddg", X)

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::distribute;

cl::opt<bool> DisableDist("disable-hir-loop-distribute",
                          cl::desc("Disable HIR Loop Distribution"), cl::Hidden,
                          cl::init(false));

cl::opt<unsigned> MaxMemResourceToDistribute(
    "hir-loop-distribute-max-mem",
    cl::desc("Number of memory references to be placed into new distributed "
             "loop chunks"),
    cl::Hidden, cl::init(20));

cl::opt<unsigned> ScalarExpansionCost(
    "hir-loop-distribute-scex-cost",
    cl::desc(
        "Number of mem operations in loop when to enable scalar expansion."),
    cl::Hidden, cl::init(20));

enum PragmaReturnCode {
  NotProcessed,
  NoDistribution,
  Success,
  UnsupportedStmts,
  TooComplex,
  TooManyDistributePoints,
  Last
};

const std::string DistributeLoopnestEnable =
    "intel.loop.distribute.loopnest.enable";

const unsigned OptReportMsg[Last] = {
    //"Distribute point pragma not processed",
    25481u,
    //"No Distribution as requested by pragma",
    25482u,
    //"Distribute point pragma processed",
    25483u,
    //"Distribute point pragma not processed: Unsupported constructs in loops",
    25484u,
    //"Distribute point pragma not processed: Loop is too complex",
    25485u,
    //"Distribute point pragma not processed: Too many Distribute points"
    25486u};

bool HIRLoopDistribution::run() {
  if (DisableDist) {
    LLVM_DEBUG(dbgs() << "LOOP DISTRIBUTION: Transform disabled\n");
    return false;
  }

  SmallVector<HLLoop *, 64> Loops;

  if (DistCostModel == DistHeuristics::BreakMemRec) {
    HIRF.getHLNodeUtils().gatherInnermostLoops(Loops);
  } else {
    HIRF.getHLNodeUtils().gatherAllLoops(Loops);
    // Work from innermost to outermost
    std::sort(Loops.begin(), Loops.end(), [](HLLoop *A, HLLoop *B) -> bool {
      return A->getNestingLevel() > B->getNestingLevel();
    });
  }

  bool Modified = false;
  OptReportBuilder &ORBuilder = HIRF.getORBuilder();

  for (auto I = Loops.begin(), E = Loops.end(); I != E; ++I) {
    HLLoop *Lp = *I;
    LLVM_DEBUG(dbgs() << "LOOP DISTRIBUTION: <" << Lp->getNumber() << ">\n");

    if (!loopIsCandidate(Lp)) {
      LLVM_DEBUG(
          dbgs() << "LOOP DISTRIBUTION: Loop is not candidate with current "
                    "heuristics \n");
      continue;
    }

    if (Lp->hasDistributePoint()) {
      unsigned RC = distributeLoopForDirective(Lp);
      if (RC != NotProcessed) {
        ORBuilder(*Lp).addRemark(OptReportVerbosity::Low, OptReportMsg[RC]);
      }
      continue;
    }

    bool AllowScalarExpansion = false;
    bool CreateControlNodes = false;

    // Sparse array reduction info is needed to create the DistPPGraph
    // and in findDistPoints while breaking the PiBlock Recurrences.
    SARA.computeSparseArrayReductionChains(Lp);

    if (DistCostModel == DistHeuristics::BreakMemRec) {
      CreateControlNodes = true;

      unsigned TotalMemOps = 0;
      TotalMemOps = HLR.getSelfLoopResource(Lp).getNumIntMemOps() +
                    HLR.getSelfLoopResource(Lp).getNumFPMemOps();

      TotalMemOps += 3 * SARA.getNumSparseArrayReductionChains(Lp);

      if (TotalMemOps >= ScalarExpansionCost) {
        AllowScalarExpansion = true;
      }

      LLVM_DEBUG(dbgs() << "[Distribution] Loop has " << TotalMemOps
                        << " memory operations which makes it "
                        << (AllowScalarExpansion ? "" : "non-")
                        << "profitable for scalar expansion\n");
    } else if (DistCostModel == DistHeuristics::NestFormation) {
      AllowScalarExpansion = Lp->isInnermost();
    }

    std::unique_ptr<PiGraph> PG(
        new PiGraph(Lp, DDA, SARA, AllowScalarExpansion, CreateControlNodes));

    if (!PG->isGraphValid()) {
      LLVM_DEBUG(
          dbgs() << "LOOP DISTRIBUTION: Distribution for loop failed due to "
                 << PG->getFailureReason() << "\n");
      continue;
    }

    if (PG->hasControlDependences() && Lp->isStripmineRequired(StripmineSize) &&
        !Lp->canStripmine(StripmineSize)) {
      // Assume stripmine is required
      LLVM_DEBUG(dbgs() << "Stripmine is not possible but assumed to be "
                           "required for loops with control dependencies\n");
      continue;
    }

    LLVM_DEBUG_DDG(dbgs() << "DDG dump:\n");
    LLVM_DEBUG_DDG(DDA.getGraph(Lp).dump());

    LLVM_DEBUG(dbgs() << "\nPiGraph dump:\n");
    LLVM_DEBUG(PG->dump());
    LLVM_DEBUG(dbgs() << "\n");

    // Single piblock graph isn't worth considering
    if (PG->size() < 2) {
      // TODO might still be able to scalar expand though...
      LLVM_DEBUG(
          dbgs()
          << "LOOP DISTRIBUTION: Deps result in single-node Piblock-Graph\n");

      if (DistCostModel != DistHeuristics::BreakMemRec) {
        continue;
      }
    }

    SmallVector<PiBlockList, 8> NewOrdering;
    findDistPoints(Lp, PG, NewOrdering);

    if (NewOrdering.size() > 1 && NewOrdering.size() < MaxDistributedLoop) {
      SmallVector<HLDDNodeList, 8> DistributedLoops;

      invalidateLoop(Lp);

      processPiBlocksToHLNodes(PG, NewOrdering, DistributedLoops);

      // Do scalar expansion analysis.
      ScalarExpansion SCEX(Lp->getNestingLevel(), false, DistributedLoops);
      LLVM_DEBUG(dbgs() << "Scalar Expansion analysis:\n"; SCEX.dump(););

      // Can't do the following assertion because scalar expansion is allowed in
      // for some cases, for example for Sparse Array Reductions.
      // assert((!SCEX.isScalarExpansionRequired() || AllowScalarExpansion) &&
      //       "Inconsistent logic: scalar expansion is required while "
      //       "being not allowed");

      // Bail out if exceed number of maximum temps allowed, but only if there
      // were no control dependencies - in this case processPiBlocksToHLNodes()
      // already made irreversible changes to HIR.
      if (!PG->hasControlDependences() &&
          SCEX.getNumTempsRequired() > MaxArrayTempsAllowed) {
        LLVM_DEBUG(dbgs() << "LOOP DISTRIBUTION: "
                          << "Number of temps required for distribution exceed "
                             "MaxArrayTempsAllowed\n");
        continue;
      }

      // Bail out if scalar expansion would introduce new dependencies that
      // require additional temps
      if (SCEX.hasBadCandidate()) {
        LLVM_DEBUG(
            dbgs()
            << "LOOP DISTRIBUTION: "
            << "Distribution disabled due to def/use in cloned if for scex\n");
        continue;
      }

      // Do not do loop distribution for loopnest formation if stripmine is
      // inevitable.
      if (DistCostModel == DistHeuristics::NestFormation &&
          SCEX.isScalarExpansionRequired()) {
        if (SCEX.isTempRequired()) {
          LLVM_DEBUG(dbgs() << "LOOP DISTRIBUTION: "
                            << "Unable to do loop distribution for loopnest "
                               "formation without stripmining.\n");
          continue;
        }

        if (!Lp->isInnermost()) {
          LLVM_DEBUG(dbgs() << "LOOP DISTRIBUTION: "
                            << "Unable to do loop distribution for loopnest "
                               "formation without scalar expansion for "
                               "non-innermost loops.\n");
          continue;
        }
      }

      distributeLoop(Lp, DistributedLoops, SCEX, ORBuilder, false);

      Modified = true;
    } else {
      LLVM_DEBUG(dbgs() << "LOOP DISTRIBUTION: "
                        << "Found no valid distribution points"
                        << "\n");
    }
  }

  return Modified;
}

void HIRLoopDistribution::processPiBlocksToHLNodes(
    const std::unique_ptr<PiGraph> &PGraph,
    ArrayRef<PiBlockList> GroupsOfPiBlocks,
    SmallVectorImpl<HLDDNodeList> &DistributedLoops) {

  // Maps (Original control statement, PiBlock list) -> Original or cloned HLIf.
  SmallDenseMap<std::pair<HLIf *, const PiBlockList *>, HLIf *> ControlGuards;

  unsigned LoopNum = 0;
  for (auto &PList : GroupsOfPiBlocks) {
    HLDDNodeList &CurLoopHLDDNodeList = DistributedLoops.emplace_back();

    // Combine PiBlocks within single ordering group.
    SmallVector<DistPPNode *, 32> MergedPiBlock;
    for (auto *PiBlock : PList) {
      for (auto *PPNode :
           make_range(PiBlock->dist_node_begin(), PiBlock->dist_node_end())) {
        MergedPiBlock.push_back(PPNode);
      }
    }

    std::sort(MergedPiBlock.begin(), MergedPiBlock.end(),
              [](DistPPNode *A, DistPPNode *B) {
                return A->getNode()->getTopSortNum() <
                       B->getNode()->getTopSortNum();
              });

    for (auto *PPNode : MergedPiBlock) {
      HLNode *Node = PPNode->getNode();

      // Set the existing control node for the PList.
      if (PPNode->isControlNode()) {
        HLIf *ControlNode = cast<HLIf>(Node);
        ControlGuards[{ControlNode, &PList}] = ControlNode;
      }

      auto ControlDep = PGraph->getControlDependence(PPNode);
      if (!ControlDep) {
        CurLoopHLDDNodeList.push_back(cast<HLDDNode>(Node));
        continue;
      }

      // The function returns ControlNode for the given PiNode and the current
      // chunk (PList).
      // It either returns existing ControlNode or clones one for the current
      // chunk.
      std::function<HLIf *(DistPPNode * PiNode)> GetControlNode =
          [&](DistPPNode *PiNode) -> HLIf * {
        HLIf *OrigControlNode = cast<HLIf>(PiNode->getNode());

        // Try to get an existing node.
        HLIf *&ControlNode = ControlGuards[{OrigControlNode, &PList}];

        // Do the cloning.
        if (!ControlNode) {
          ControlNode = OrigControlNode->cloneEmpty();

          // Check if the control node itself has a control dependence.
          auto InterimControlDep = PGraph->getControlDependence(PiNode);
          if (InterimControlDep) {
            // Recursive call to get the parent control node.
            auto InterimControlNode = GetControlNode(InterimControlDep->first);
            if (InterimControlDep->second) {
              HLNodeUtils::insertAsLastThenChild(InterimControlNode,
                                                 ControlNode);
            } else {
              HLNodeUtils::insertAsLastElseChild(InterimControlNode,
                                                 ControlNode);
            }

          } else {
            // Use {LoopNum, true} to indicate insert or move HLIf to its
            // final place in HIR.
            DistDirectiveNodeMap[ControlNode] = {LoopNum, true};
            CurLoopHLDDNodeList.push_back(ControlNode);
          }
        }

        return ControlNode;
      };

      // Get the HLIf node for the current chunk. It may be an original HLIf or
      // a clone.
      auto ControlNode = GetControlNode(ControlDep->first);

      // Move Node under the ControlNode if not in the right position already.
      if (ControlDep->second) {
        // Node should be placed under "true" branch of the control node.
        //
        // Do not move the node if it's already a child of the ControlNode.
        // It may happen if ControlNode is an original HLIf.
        if (!ControlNode->isThenChild(Node)) {
          HLNodeUtils::moveAsLastThenChild(ControlNode, Node);
        }
      } else {
        // Node should be placed under "false" branch of the control node.
        if (!ControlNode->isElseChild(Node)) {
          HLNodeUtils::moveAsLastElseChild(ControlNode, Node);
        }
      }
    }

    ++LoopNum;
  }
}

bool HIRLoopDistribution::piEdgeIsMemRecurrence(
    const HLLoop *Lp, const PiGraphEdge &PiEdge) const {
  for (auto &Edge :
       make_range(PiEdge.getDDEdges().begin(), PiEdge.getDDEdges().end())) {
    // Skip edges between terminals because we are looking for memory
    // recurrences.
    if (Edge->getSrc()->isTerminalRef()) {
      continue;
    }

    // TODO: Use Edge->preventsVectorization(Lp->getNestingLevel())
    //       Will require to adjust some LIT test.
    if (Edge->getDVAtLevel(Lp->getNestingLevel()) & DVKind::LT) {
      return true;
    }
  }

  return false;
}

static void updateLiveInAllocaTemp(HLLoop *Loop, unsigned SB) {

  HLLoop *Lp = Loop;
  while (Lp) {
    Lp->addLiveInTemp(SB);
    Lp = Lp->getParentLoop();
  }
}

RegDDRef *HIRLoopDistribution::createTempArrayStore(HLLoop *Lp,
                                                    RegDDRef *TempRef,
                                                    unsigned OrigLoopLevel) {

  // Generates  TEMP[i] = tx
  //  tx may be from assignments of this form:
  //  tx = ty   ;  tx = 1000
  //  Make it a self-blob to avoid IR validation error

  HLDDNode *TempRefDDNode = TempRef->getHLDDNode();

  auto ArrTy = ArrayType::get(TempRef->getDestType(), StripmineSize);

  unsigned AllocaBlobIdx =
      HNU.createAlloca(ArrTy, Lp->getParentRegion(), ".TempArray");

  RegDDRef *TmpArrayRef = HNU.getDDRefUtils().createMemRef(ArrTy, AllocaBlobIdx);

  auto IVType = Lp->getIVType();
  CanonExpr *FirstCE = TempRef->getCanonExprUtils().createCanonExpr(IVType);
  FirstCE->addIV(OrigLoopLevel, 0, 1);

  //  Create constant of 0
  CanonExpr *SecondCE = TempRef->getCanonExprUtils().createCanonExpr(IVType);
  TmpArrayRef->addDimension(SecondCE);
  TmpArrayRef->addDimension(FirstCE);

  insertTempArrayStore(Lp, TempRef, TmpArrayRef, TempRefDDNode);

  return TmpArrayRef;
}

void HIRLoopDistribution::insertTempArrayStore(HLLoop *Lp, RegDDRef *TempRef,
                                               RegDDRef *TmpArrayRef,
                                               HLDDNode *TempRefDDNode) {

  RegDDRef *RVal = TempRef->clone();
  HLInst *StoreInst = HNU.createStore(RVal, ".TempSt", TmpArrayRef);
  HLNodeUtils::insertAfter(TempRefDDNode, StoreInst);

  RVal->makeConsistent(TempRef);

  updateLiveInAllocaTemp(Lp, TmpArrayRef->getBasePtrSymbase());
  TempArraySB.push_back(TmpArrayRef->getSymbase());
}

void HIRLoopDistribution::createTempArrayLoad(RegDDRef *TempRef,
                                              RegDDRef *TmpArrayRef,
                                              HLNode *Node,
                                              bool TempRedefined) {

  // tx = TEMP[i]
  HLLoop *Lp = Node->getParentLoop();

  const std::string TempName = "scextmp";
  HLInst *LoadInst = HNU.createLoad(TmpArrayRef->clone(), TempName, TempRef);

  if (TempRedefined) {
    HLLoop *Lp = Node->getParentLoop();
    Node = cast<HLDDNode>(Lp->getFirstChild());
  }

  HLNodeUtils::insertBefore(Node, LoadInst);
  updateLiveInAllocaTemp(Lp, TmpArrayRef->getBasePtrSymbase());
  TempArraySB.push_back(TmpArrayRef->getSymbase());
}

bool ScalarExpansion::isScalarExpansionCandidate(const DDRef *Ref) const {

  // We need to scalar expand scalar global vars besides temps.
  // When Memref cleanup is suppessed for distribute point pragma,
  // Scalar global vars are not turning into temp.
  // Global vars should be scalar expanded also, to avoid loss of
  // functionality

  const RegDDRef *RegRef = dyn_cast<RegDDRef>(Ref);
  if (!RegRef) {
    return true;
  }

  bool IsMemRef = RegRef->isMemRef();

  if (IsMemRef && HasDistributePoint && RegRef->isSingleDimension()) {
    auto BaseCE = RegRef->getBaseCE();
    return !BaseCE->isNonLinear() && RegRef->getDimensionIndex(1)->isZero();
  }

  return !IsMemRef;
}

ScalarExpansion::ScalarExpansion(unsigned Level, bool HasDistributePoint,
                                 ArrayRef<HLDDNodeList> Chunks)
    : Level(Level),
      HasDistributePoint(HasDistributePoint), HasBadCandidate(false) {
  analyze(Chunks);
}

bool ScalarExpansion::findDepInst(const RegDDRef *RVal,
                                  const HLInst *&DepInst) {
  const HLInst *BlobNode = dyn_cast<HLInst>(RVal->getHLDDNode());

  for (int I = 0; I < 2; ++I) {
    BlobNode = dyn_cast_or_null<HLInst>(BlobNode->getPrevNode());
    if (!BlobNode) {
      return false;
    }

    const RegDDRef *LVal = BlobNode->getLvalDDRef();
    if (!LVal || LVal->getSymbase() != RVal->getSymbase()) {
      continue;
    }

    // Should be safe to clone and equal to DepInst if one is already required.
    if (!DepInst || DepInst == BlobNode) {
      DepInst = BlobNode;
      return true;
    }
  }

  return false;
}

bool ScalarExpansion::isSafeToRecompute(const RegDDRef *SrcRef,
                                        unsigned ChunkIdx,
                                        const SymbaseLoopSetTy &SymbaseLoopSet,
                                        const SparseBitVector<> &ModifiedBases,
                                        const HLInst *&DepInst) {
  assert(SrcRef->isLval() && "SrcRef is expected to be LVal");

  const HLInst *Inst = cast<HLInst>(SrcRef->getHLDDNode());

  auto CheckRVal = [&](const RegDDRef *RVal) -> bool {
    unsigned SB = RVal->getSymbase();

    if (RVal->isMemRef() && ModifiedBases.test(RVal->getBasePtrBlobIndex())) {
      return false;
    }

    if (RVal->isLinearAtLevel(Level)) {
      return true;
    }

    if (RVal->isSelfBlob()) {
      bool Ret =
          RVal->isLinearAtLevel(Level) || SymbaseLoopSet.count({SB, ChunkIdx});

      return Ret || (findDepInst(RVal, DepInst) &&
                     isSafeToRecompute(DepInst->getLvalDDRef(), ChunkIdx,
                                       SymbaseLoopSet, ModifiedBases, DepInst));
    }

    for (auto &Blob : make_range(RVal->blob_begin(), RVal->blob_end())) {
      if (!Blob->isLinearAtLevel(Level) &&
          SymbaseLoopSet.count({Blob->getSymbase(), ChunkIdx}) == 0) {
        return false;
      }
    }

    return true;
  };

  unsigned SrcRValLevel = 0;
  bool IsSafeToRecompute =
      !Inst->isCallInst() || !Inst->isUnsafeSideEffectsCallInst();

  if (IsSafeToRecompute) {
    for (const RegDDRef *RVal :
         make_range(Inst->rval_op_ddref_begin(), Inst->rval_op_ddref_end())) {

      if (!CheckRVal(RVal)) {
        IsSafeToRecompute = false;
        break;
      }

      // Find the level where RVal is completely defined.
      unsigned RValLevel = RVal->getDefinedAtLevel();
      SrcRValLevel = std::max(SrcRValLevel,
                              RValLevel != NonLinearLevel ? RValLevel : Level);

      // Find if it has IVs greater than found level.
      for (unsigned IV = SrcRValLevel + 1; IV <= MaxLoopNestLevel; ++IV) {
        if (RVal->hasIV(IV)) {
          SrcRValLevel = IV;
        }
      }
    }
  }

  // It also should be safe to recompute SrcRef at level of DstRef.
  return IsSafeToRecompute && Level >= SrcRValLevel;
}

// For SCEX store/loads for distributed loops, we don't want the default
// behavior of loading at the beginning of each loop if the original def
// is under an if node. Take the follow example:
//
// DISTLOOP1:
//   if (x > 0)
//     x = x+1
//   -- distpoint --
//     A[i] = B[x];
//
// Scalar Expansion inserts a temparray store after the definition
// of x and a temparray load before the use of B[x]. However, because
// the original use is under an if-node, we want the corresponding load
// to be inside the if-node, as opposed to before the if.
static HLNode *getInsertionNodeForIf(RegDDRef *Src, DDRef *Dst,
                                     HLNode *FirstNode) {
  HLIf *SrcParentIf = dyn_cast_or_null<HLIf>(Src->getHLDDNode()->getParent());
  HLIf *DstParentIf = dyn_cast_or_null<HLIf>(Dst->getHLDDNode()->getParent());

  // TODO: Dst could be used in an If predicate. Need to handle when Src Node
  // is inside the same If.
  // bool DstInIfPred = true;
  // HLIf *DstParentIf = dyn_cast<HLIf>(Dst->getHLDDNode());
  // if (!DstParentIf) {
  //   DstParentIf = dyn_cast_or_null<HLIf>(Dst->getHLDDNode()->getParent());
  //   DstInIfPred = false;
  // }

  if (!SrcParentIf || !DstParentIf) {
    return FirstNode;
  }

  // Find equivalent HLIf parent for both Src and Dst. The Dst may be at a
  // deeper if nesting than the src, so we traverse up the Dst parent chain.
  // If there is a deeper nesting, save the if ancestor that is the immediate
  // child of the matching Dst Ifparent. We use that child to verify the
  // then/else path of the ifs. This is due to Dst being detached HIR at this
  // time.
  bool FoundEqualIf = false;
  HLNode *DstNodeParent = DstParentIf;
  HLIf *DstIfAncestor = nullptr;
  while (DstNodeParent && isa<HLIf>(DstNodeParent)) {
    DstParentIf = cast<HLIf>(DstNodeParent);
    if (HLNodeUtils::areEqualConditions(SrcParentIf, DstParentIf)) {
      FoundEqualIf = true;
      break;
    }
    DstIfAncestor = DstParentIf;
    DstNodeParent = DstNodeParent->getParent();
  }

  if (!FoundEqualIf) {
    return FirstNode;
  }

  auto isNodeInThenPath = [&](HLIf *If, HLNode *Node) -> bool {
    for (auto ThenIt = If->then_begin(), ThenItEnd = If->then_end();
         ThenIt != ThenItEnd; ++ThenIt) {
      if (Node == &*ThenIt) {
        return true;
      }
    }
    return false;
  };

  bool SrcPath = SrcParentIf->isThenChild(Src->getHLDDNode());

  // TODO: Return the Node in DstPath corresponding to the def in the first
  // chunk for predicate case.
  // if (DstInIfPred) {
  //   return SrcPath ? DstParentIf->getFirstThenChild()
  //                  : DstParentIf->getFirstElseChild();
  // }

  // Use DstIfAncestor that we saved earlier as it is the direct child of the If
  // node where the original Ref is defined. DstPath is detached so we need to
  // manually iterate the nodes
  bool DstPath = isNodeInThenPath(
      DstParentIf, DstIfAncestor ? DstIfAncestor : Dst->getHLDDNode());

  if (SrcPath != DstPath) {
    return FirstNode;
  }

  return DstPath ? DstParentIf->getFirstThenChild()
                 : DstParentIf->getFirstElseChild();
}

void ScalarExpansion::analyze(ArrayRef<HLDDNodeList> Chunks) {
  // Symbase-to-Candidate map
  DenseMap<unsigned, unsigned> CandidatesSymbaseIndex;
  auto GetCandidateForSymbase = [&](unsigned Symbase) -> Candidate & {
    unsigned &Index = CandidatesSymbaseIndex[Symbase];
    if (Index == 0) {
      Candidates.emplace_back();
      Index = Candidates.size();
      return Candidates.back();
    }

    return Candidates[Index - 1];
  };

  SmallVector<Gatherer::VectorTy, 8> RefGroups;
  RefGroups.reserve(Chunks.size());

  SparseBitVector<> ModifiedBases;

  for (auto &HLNodeList : Chunks) {
    RefGroups.emplace_back();
    auto &CurGroup = RefGroups.back();

    for (HLDDNode *Node : HLNodeList) {
      Gatherer::gather(Node, CurGroup);
    }

    std::for_each(CurGroup.begin(), CurGroup.end(), [&](const DDRef *Ref) {
      if (Ref->isLval() && !Ref->isTerminalRef()) {
        ModifiedBases.set(cast<RegDDRef>(Ref)->getBasePtrBlobIndex());
      }
    });
  }

  SymbaseLoopSetTy SymbaseLoopSet;

  for (unsigned I = 0, E = RefGroups.size(); I < E - 1; ++I) {
    for (DDRef *SrcRef : RefGroups[I]) {

      if (SrcRef->isRval()) {
        continue;
      }

      if (!isScalarExpansionCandidate(SrcRef)) {
        continue;
      }

      unsigned SB = SrcRef->getSymbase();
      RegDDRef *SrcRegRef = cast<RegDDRef>(SrcRef);

      for (unsigned J = I + 1; J < E; ++J) {
        bool TempRedefined = false;

        for (DDRef *DstRef : RefGroups[J]) {
          // For Pragma, we support global scalar (*tx) to be scalar
          // expanded. In case *tx, *ty are mapped to same Symbase,
          // they will be treated as different

          if (SrcRef->getSymbase() != DstRef->getSymbase() ||
              (SrcRegRef->isMemRef() &&
               (!DDRefUtils::areEqual(SrcRef, DstRef)))) {
            continue;
          }

          // If the temp is unconditionally defined in the J chunk we don't need
          // to materialize it from a temp.
          if (DstRef->isLval() &&
              isa<HLLoop>(DstRef->getHLDDNode()->getParent())) {
            break;
          }

          if (!isScalarExpansionCandidate(DstRef)) {
            TempRedefined = true;
            continue;
          }

          // If scalar expansion would introduce extra dependencies from SrcLoop
          // to DstLoop, set SCEX.hasBadCandidate.
          if (isa<HLIf>(DstRef->getHLDDNode()) && isa<HLIf>(SrcRef->getParent())
            && HLNodeUtils::areEqualConditions(cast<HLIf>(DstRef->getHLDDNode()), cast<HLIf>(SrcRef->getParent()))) {
            this->HasBadCandidate = true;
            break;
          }

          Candidate &Cand = GetCandidateForSymbase(SB);

          const HLInst *DepInst = nullptr;
          Cand.SafeToRecompute &= isSafeToRecompute(
              SrcRegRef, J, SymbaseLoopSet, ModifiedBases, DepInst);

          if (Cand.SrcRefs.empty() || Cand.SrcRefs.back() != SrcRegRef) {
            Cand.SrcRefs.push_back(SrcRegRef);
          }

          //  Loading is needed once per loop
          if (SymbaseLoopSet.count({SB, J})) {
            continue;
          }

          HLNode *FirstNode =
              getInsertionNodeForIf(SrcRegRef, DstRef, Chunks[J].front());

          // Push first ref in J group, this is where SrcRegRef should be loaded
          // or recomputed, once per group J.
          Cand.DstRefs.push_back({DstRef, FirstNode, TempRedefined, DepInst});
          LLVM_DEBUG(dbgs() << "SCEX candidate: "; Cand.dump();
                     dbgs() << "\n";);
          SymbaseLoopSet.insert({SB, J});
        }
      }
    }
  }
}

void HIRLoopDistribution::replaceWithArrayTemp(
    unsigned OrigLoopLevel, ArrayRef<ScalarExpansion::Candidate> Candidates) {

  for (auto &Candidate : Candidates) {
    if (!Candidate.isTempRequired()) {
      HLInst *SrcInst = cast<HLInst>(Candidate.SrcRefs.front()->getHLDDNode());

      for (auto &DstCand : Candidate.DstRefs) {
        HLNode *DstNode = DstCand.FirstNode;

        if (DstCand.IsTempRedefined) {
          HLLoop *Lp = DstNode->getParentLoop();
          DstNode = cast<HLDDNode>(Lp->getFirstChild());
        }

        if (DstCand.DepInstForRecompute) {
          HLNodeUtils::insertBefore(DstNode,
                                    DstCand.DepInstForRecompute->clone());
        }

        HLNodeUtils::insertBefore(DstNode, SrcInst->clone());
      }

      continue;
    }

    RegDDRef *TmpArrayRef = nullptr;

    // Create TEMP[i] = tx and insert
    for (RegDDRef *SrcRef : Candidate.SrcRefs) {
      HLLoop *Lp = SrcRef->getLexicalParentLoop();

      if (!TmpArrayRef) {
        TmpArrayRef = createTempArrayStore(Lp, SrcRef, OrigLoopLevel);
      } else {
        insertTempArrayStore(Lp, SrcRef, TmpArrayRef->clone(),
                             SrcRef->getHLDDNode());
      }
    }

    // Insert tx = TEMP[i]
    assert(TmpArrayRef && "Temp Store missing");

    for (auto &DstRefPair : Candidate.DstRefs) {
      DDRef *DstRef = DstRefPair.Ref;

      // Prepare LVal for the load from temp array. SinkRef could be
      // either a temp %t or an invariant memref %a[0].
      auto *SinkTempRef =
          DstRef->isTerminalRef()
              // Terminal SinkRef may have a linear form of RVal,
              // create a proper LVal.
              ? HNU.getDDRefUtils().createSelfBlobRef(
                    HNU.getBlobUtils().findOrInsertTempBlobIndex(
                        DstRef->getSymbase()))
              // Use a clone in case of memref.
              : cast<RegDDRef>(DstRef->clone());

      createTempArrayLoad(SinkTempRef, TmpArrayRef, DstRefPair.FirstNode,
                          DstRefPair.IsTempRedefined);
    }
  }
}

// Returns the index of the first distributed loop which depends on the loop
// preheader. Always keeping the preheader in the first loop prevents
// distribution of outer loops.
//
// For example, the flow edge for t1 prevents i1 loop distribution-
//
// DO i1
//    t1 =
//   DO i2
//     // t1 is unused
//   END DO
//
//   DO i2
//     = t1
//   END DO
// END DO
static unsigned
getPreheaderLoopIndex(HLLoop *Loop,
                      const SmallVectorImpl<HLDDNodeList> &DistributedLoops,
                      DistHeuristics DistCostModel) {

  // Early-exit checks.
  if ((DistCostModel != DistHeuristics::NestFormation) ||
      !Loop->hasPreheader() || !Loop->isInnermost() || !Loop->getParentLoop()) {
    return 0;
  }

  auto &BU = Loop->getHLNodeUtils().getBlobUtils();

  SmallVector<unsigned, 8> PreheaderLvalTemps;
  // Iterate through preheader nodes and check whether it can cause any
  // side-effects. Also collect all the lval temps.
  for (auto &Node : make_range(Loop->pre_begin(), Loop->pre_end())) {
    auto *Inst = &cast<HLInst>(Node);

    if (Inst->isCallInst()) {
      return 0;
    }

    for (auto *Ref : make_range(Inst->ddref_begin(), Inst->ddref_end())) {
      if (Ref->isMemRef()) {
        return 0;
      }

      if (Ref->isLval()) {
        unsigned Index = Ref->isSelfBlob()
                             ? Ref->getSelfBlobIndex()
                             : BU.findTempBlobIndex(Ref->getSymbase());

        if (Index != InvalidBlobIndex) {
          PreheaderLvalTemps.push_back(Index);
        }
      } else if (Ref->isNonLinear()) {
        // Bail out as some temp may be getting redefined inside the loop.
        return 0;
      }
    }
  }

  // Iterate through loop nodes and return the first loop which uses preheader
  // temp.
  for (auto &List : enumerate(DistributedLoops)) {
    unsigned LoopIndex = List.index();

    // We are already at the last loop, just return its index.
    if (LoopIndex == DistributedLoops.size() - 1) {
      return LoopIndex;
    }

    for (HLDDNode *Node : List.value()) {
      auto *Inst = dyn_cast<HLInst>(Node);

      // Give up on non-inst nodes for simplicity.
      if (!Inst) {
        return LoopIndex;
      }

      for (auto *Ref : make_range(Node->ddref_begin(), Node->ddref_end())) {
        for (auto PreheaderIndex : PreheaderLvalTemps) {
          if (Ref->usesTempBlob(PreheaderIndex)) {
            return LoopIndex;
          }
        }
      }
    }
  }

  return 0;
}

/// distributeLoop handles distribution to enable perfect loop nests and
/// breaking of
///   memref recurrences. In addition, loops with a lot of memory references
///   will be distributed

/// Distribution with Scalar Expansion:
/// t1 =  ..  ;  = t1;  enables more expressions to be distributed into
/// different loop nests. Temps need to be changed to small arrays by
/// stripmining.
///  - main focus is to split up loops that have too many memrefs because
///    HW prefetcher will give up and Strength reduction code cannot not handle
///    well.
///  - Ideally, maximal distribution should be done and let fusion fuses it
///  back.
///    But product compiler cannot afford the long compile time.
///  - We can replace the temp by temp array before distribution.
///    Temp array is then cleaned up later if Dead store can be done.
///    It's fine for compiler that has expressions trees.
///    But LLVM has a lot of temps. We need to use a different approach.
/// -  The trick is to avoid the backedge in Dist graph for scalar temps that
/// have DV (=)
/// -  Replaces them later with Array temp when it is needed after distribution.
/// -  There is an advantage of LLVM with many temps. We don't need to do node
/// splitting.
/// -  But if we relax too much for not creating the backedge for temps,
///     some of the live range of the temp can become larger.
/// -  We can add extra dist edge for lexical links. That may also create cases
/// that hinder distribution
///     because there could be other back edges, which is not unknown until we
///     build the pi-graph.
/// -  In summary, without using maximal distribution, the solution cannot be
/// perfect.

void HIRLoopDistribution::distributeLoop(
    HLLoop *Loop, SmallVectorImpl<HLDDNodeList> &DistributedLoops,
    const ScalarExpansion &SCEX, OptReportBuilder &ORBuilder,
    bool ForDirective) {
  assert(DistributedLoops.size() < MaxDistributedLoop &&
         "Number of distributed chunks exceed threshold. Expected the caller "
         "to check before calling this function.");

  unsigned LoopCount = DistributedLoops.size();
  assert(LoopCount > 1 && "Invalid loop distribution");
  LLVM_DEBUG(dbgs() << "LOOP DISTRIBUTION : " << LoopCount
                    << " way distributed\n");

  TempArraySB.clear();
  HLLoop *LoopNode;
  unsigned LoopLevel = Loop->getNestingLevel();
  HLRegion *RegionNode = Loop->getParentRegion();

  bool ShouldStripmine =
      SCEX.isTempRequired() && Loop->isStripmineRequired(StripmineSize);
  if (ShouldStripmine && !Loop->canStripmine(StripmineSize)) {
    // It's fine to bail out for loops without control flow dependencies.
    // Loops with such dependencies do preliminary checks.
    return;
  }

  unsigned PreheaderLoopIndex =
      getPreheaderLoopIndex(Loop, DistributedLoops, DistCostModel);

  for (auto &List : enumerate(DistributedLoops)) {
    // Each PiBlockList forms a new loop
    // Clone Empty Loop. Copy preheader for 1st loop and
    // postexit for last loop
    // TODO: Determine which loop needs preheader/postexit

    LoopNode = Loop->cloneEmpty();
    unsigned CurLoopIndex = List.index();
    NewLoops[CurLoopIndex] = LoopNode;

    if (CurLoopIndex == 0) {
      if (ForDirective) {
        ORBuilder(*LoopNode).addRemark(OptReportVerbosity::Low,
                                       OptReportMsg[Success]);
      }
      // Loop distributed (%d way)
      ORBuilder(*LoopNode).addRemark(OptReportVerbosity::Low, 25426u,
                                     LoopCount);
    }

    if (CurLoopIndex == PreheaderLoopIndex) {
      HLNodeUtils::moveAsFirstPreheaderNodes(LoopNode, Loop->pre_begin(),
                                             Loop->pre_end());
    }

    if (CurLoopIndex == LoopCount - 1) {
      HLNodeUtils::moveAsFirstPostexitNodes(LoopNode, Loop->post_begin(),
                                            Loop->post_end());
    }

    for (HLDDNode *Node : List.value()) {
      if (DistDirectiveNodeMap[Node].second) {
        HLNodeUtils::insertAsLastChild(LoopNode, Node);
      } else {
        HLNodeUtils::moveAsLastChild(LoopNode, Node);
      }
    }

    ORBuilder(*LoopNode).addOrigin("Distributed chunk%d",
                                   (int)CurLoopIndex + 1);
  }

  // The loop is now empty, all its children are moved to new loops
  // except for user pragma, where some of the IF needs to be cloned

  assert((!Loop->hasChildren() || Loop->hasDistributePoint()) &&
         "Loop Distribution failed to account for all Loop Children");

  // Attach new loops into HIR.
  for (unsigned I = 0; I < LoopCount; ++I) {
    HLNodeUtils::insertBefore(Loop, NewLoops[I]);
  }

  if (SCEX.isScalarExpansionRequired()) {
    replaceWithArrayTemp(LoopLevel, SCEX.getCandidates());

    // For constant trip count <= StripmineSize, no stripmine is done
    if (ShouldStripmine) {
      HIRTransformUtils::stripmine(NewLoops[0], NewLoops[LoopCount - 1],
                                   StripmineSize);
      // Fix TempArray index if stripmine is peformed: 64 * i1 + i2 => i2
      fixTempArrayCoeff(NewLoops[0]->getParentLoop());
    }
  }

  for (unsigned I = 0; I < LoopCount; ++I) {
    // Distributed flag is used by Loop Fusion to skip loops that are
    // distributed.  Need to set for Memory related distribution only.
    // Distributed loops for enabling perfect loop nest, can still be fused
    // after interchange is done
    if (DistCostModel == DistHeuristics::BreakMemRec) {
      NewLoops[I]->setDistributedForMemRec();
    }

    HLNodeUtils::removeEmptyNodes(NewLoops[I], false);
  }

  HLNodeUtils::remove(Loop);

  // Distribution for perfect loopnest is not profitable by itself, it is only
  // used for enabling other transformations so we should not mark region as
  // modified.
  if (DistCostModel != DistHeuristics::NestFormation) {
    RegionNode->setGenCode();
  }

  LLVM_DEBUG(dbgs() << "New Region with Transformed Loops:\n";
             RegionNode->dump(););
}

void HIRLoopDistribution::fixTempArrayCoeff(HLLoop *Loop) {

  // After stripemine, change coeff from  of TempArray
  //  from  i1 * 64 + i2  to   i2
  unsigned Level = Loop->getNestingLevel();

  ForEach<HLDDNode>::visitRange(
      Loop->child_begin(), Loop->child_end(), [this, Level](HLDDNode *Node) {
        for (RegDDRef *Ref :
             llvm::make_range(Node->ddref_begin(), Node->ddref_end())) {
          if (std::find(TempArraySB.begin(), TempArraySB.end(),
                        Ref->getSymbase()) == TempArraySB.end()) {
            continue;
          }

          for (CanonExpr *CE :
               llvm::make_range(Ref->canon_begin(), Ref->canon_end())) {
            CE->removeIV(Level);
          }
        }
      });
}

// Form perfect loop candidates by grouping stmt only piblocks
void HIRLoopDistribution::formPerfectLoopNests(
    std::unique_ptr<PiGraph> const &PGraph,
    SmallVectorImpl<PiBlockList> &DistPoints) const {

  // All piblocks that are only stmts and are roots in DAG can form their
  // own distributed loop
  PiBlockList StmtRootBlocks;

  PiBlockList CurLoopPiBlkList;
  const HLLoop *InnermostLoop;

  for (auto N = PGraph->node_begin(), E = PGraph->node_end(); N != E; ++N) {
    PiBlock *Blk = *N;
    PiBlock::PiBlockType BlockType = Blk->getBlockType();
    if (PGraph->incoming_edges_begin(Blk) == PGraph->incoming_edges_end(Blk)) {
      // If blk has no incoming edges it must be the root of a component of
      // top sorted graph
      if (BlockType == PiBlock::PiBlockType::SingleStmt ||
          BlockType == PiBlock::PiBlockType::MultipleStmt) {
        // stmt only root blocks form their own perfect loop
        StmtRootBlocks.push_back(Blk);
      } else if (BlockType == PiBlock::PiBlockType::SingleLoop) {
        HLLoop *SingleLoop =
            dyn_cast<HLLoop>((*(Blk->dist_node_begin()))->getNode());
        assert(SingleLoop && "SingleLoop piblock did not contain a loop");
        // perfect subloops are distributed into their own loop
        if (SingleLoop->isInnermost() ||
            HLNodeUtils::isPerfectLoopNest(SingleLoop, &InnermostLoop)) {
          DistPoints.push_back(PiBlockList(1, Blk));
        } else {
          CurLoopPiBlkList.push_back(Blk);
        }
      } else {
        // piblocks of mixed loop/stmts cannot form their own perfect loop
        // add them to the current loop
        CurLoopPiBlkList.push_back(Blk);
      }
    } else {
      if (BlockType == PiBlock::PiBlockType::SingleLoop) {
        HLLoop *SingleLoop =
            dyn_cast<HLLoop>((*(Blk->dist_node_begin()))->getNode());
        assert(SingleLoop && "SingleLoop piblock did not contain a loop");
        if (SingleLoop->isInnermost() ||
            HLNodeUtils::isPerfectLoopNest(SingleLoop, &InnermostLoop)) {
          // terminate our current loop and append it to loop list
          if (!CurLoopPiBlkList.empty()) {
            DistPoints.push_back(CurLoopPiBlkList);
            CurLoopPiBlkList.clear();
          }
          // and make the perfect loop its own loop nest, appended to loop
          // list in order to maintain program order
          PiBlockList PerfectLoop;
          PerfectLoop.push_back(Blk);
          DistPoints.push_back(PerfectLoop);
        } else {
          CurLoopPiBlkList.push_back(Blk);
        }
      } else {
        CurLoopPiBlkList.push_back(Blk);
      }
    }
  }

  // Terminate current loop if we haven't already
  if (!CurLoopPiBlkList.empty()) {
    DistPoints.push_back(CurLoopPiBlkList);
    CurLoopPiBlkList.clear();
  }

  // The loop represented by this list must come before all others,
  if (!StmtRootBlocks.empty()) {
    DistPoints.insert(DistPoints.begin(), StmtRootBlocks);
  }
}

void HIRLoopDistribution::splitSpatialLocalityGroups(
    const HLLoop *Lp, std::unique_ptr<PiGraph> const &PiGraph,
    SmallVectorImpl<PiBlockList> &DistPoints) const {

  HIRLoopLocality::RefGroupVecTy SpatialGroups;
  HLL.populateSpatialLocalityGroups(Lp, SpatialGroups);

  LLVM_DEBUG(dbgs() << "Using spatial locality info:\n");
  LLVM_DEBUG(DDRefGrouping::dump(SpatialGroups));

  SmallVector<PiBlock *, 8> SpatialGroupEnds;

  // Select key blocks by looking at "write" SpatialGroup's first and last ref.
  for (auto &Group : SpatialGroups) {
    bool IsWrite =
        std::any_of(Group.begin(), Group.end(),
                    [](const RegDDRef *Ref) { return Ref->isLval(); });
    if (!IsWrite) {
      continue;
    }

    SpatialGroupEnds.push_back(PiGraph->getPiBlockFromRef(Group.back()));
  }

  SpatialGroupEnds.pop_back();
  SpatialGroupEnds.push_back(*std::prev(PiGraph->node_end()));

  LLVM_DEBUG(dbgs() << "Will split into " << SpatialGroupEnds.size()
                    << " groups\n");

  // Find distribution points
  auto FirstI = PiGraph->node_begin();
  auto CutI = SpatialGroupEnds.begin();
  for (auto I = FirstI, E = PiGraph->node_end(); I < E; ++I) {
    if (CutI == SpatialGroupEnds.end()) {
      break;
    }

    if (*I != *CutI) {
      continue;
    }

    auto NextI = std::next(I);
    PiBlockList List(FirstI, NextI);
    DistPoints.push_back(List);

    FirstI = NextI;
    ++CutI;
  }
}

bool HIRLoopDistribution::loopIsCandidate(HLLoop *Lp) const {
  // TODO This will miss some opportunities
  // Ex. L has 6 PiBlocks with the first 5 having an edge to 6, which is
  // comprised only of a loop, making L not the innermost. If the first 5
  // piblocks are themselves comprised only of a single stmt
  // then we would want to distribute them into their own single vectorizable
  // loop.
  // However, considering other loops may create more loops unnecessarily
  // do i
  //  do j
  //    PiBlock 1
  //    PiBlock 2
  // Single edge 1->2 indicating recurrence
  // Distributing j forms two vectorizable loops. There will still be an
  // edge between the two new loops when considering I. Distributing
  // again won't enable vectorization, but create more loop overhead.

  // Disabled distribution of loop inside SIMD region as scalar expansion can
  // introduce new allocas which are not marked as private for outer SIMD loop.

  if (Lp->hasUnrollEnablingPragma() || Lp->hasVectorizeEnablingPragma() ||
      !Lp->isDo() || Lp->isInSIMDRegion()) {
    return false;
  }

  // Handle DistributeLoopnestEnable metadata for the NestFormation mode.
  if (DistCostModel == DistHeuristics::NestFormation &&
      Lp->getLoopStringMetadata(DistributeLoopnestEnable)) {
    Lp->removeLoopMetadata(DistributeLoopnestEnable);

    HLLoop *ParentLoop = Lp->getParentLoop();
    if (ParentLoop &&
        !ParentLoop->getLoopStringMetadata(DistributeLoopnestEnable)) {
      ParentLoop->addInt32LoopMetadata(DistributeLoopnestEnable, 1);
    }

    return true;
  }

  if (DistCostModel == DistHeuristics::NestFormation && Lp->isInnermost()) {
    return false;
  }

  uint64_t TripCount;
  // Skip  some constant trip counts loops:  small, looks like copy stmt
  if (Lp->isConstTripLoop(&TripCount)) {
    if (TripCount < 5 || HLR.getTotalLoopResource(Lp).getNumFPOps() == 0) {
      return false;
    }
  }

  if (DistCostModel == DistHeuristics::NestFormation) {
    // Skipping innermost may create fewer perfect nests, but its
    // not necessarily bad.
    // Example:
    // do i
    //  do j
    //    S1 (PiBlock 1)
    //    do k
    //      S2-S4 (PiBlock 2)
    //      S5-S8 (PiBlock 3)
    // Single Edge 1->3
    // Distributing j loop will consider k loop a single node. We might split
    // that entire node into its own loop nest. Second distribution on i loop
    // will result in two perfect nests.
    // If we consider innermost a candidate we might end up with 3 perfect
    // nests if blocks 2 and 3 are distributed once for K loop and again at
    // j loop

    const HLLoop *InnermostLoop;

    // Why ruin perfection
    // Should we run distribution in perfect loopnest mode on innermost loops?

    if (!Lp->isInnermost() &&
        HLNodeUtils::isPerfectLoopNest(Lp, &InnermostLoop)) {
      return false;
    }

    // For compile time consideration, throttle for
    // more than 3 innermost loops or nesting level > 3
    // Forming Perfect Loop Nest is primarily to enable interchange

    SmallVector<HLLoop *, 12> InnermostLPVector;

    HNU.gatherInnermostLoops(InnermostLPVector, const_cast<HLLoop *>(Lp));
    if (InnermostLPVector.size() > 2) {
      return false;
    }
    bool NonUnitStride = false;
    for (auto &Loop : InnermostLPVector) {
      if ((Loop->getNestingLevel() - Lp->getNestingLevel()) > 2) {
        return false;
      }
      if (!NonUnitStride && HLNodeUtils::hasNonUnitStrideRefs(Loop)) {
        NonUnitStride = true;
      }
    }
    if (!NonUnitStride) {
      return false;
    }
  }

  return true;
}

// Right now we are checking whether this PiBlock contains any sparse
// array reduction instructions. Later we may want to modify to match more
// patterns like in 435.gromacs
bool containsSparseArrayReductions(PiBlock *SrcBlk,
                                   HIRSparseArrayReductionAnalysis &SARA) {
  for (auto NodeI = SrcBlk->nodes_begin(), E = SrcBlk->nodes_end(); NodeI != E;
       ++NodeI) {
    HLDDNode *Node = cast<HLDDNode>(*NodeI);
    HLInst *Inst = dyn_cast<HLInst>(Node);
    if (Inst && SARA.isSparseArrayReduction(Inst)) {
      return true;
    }
  }
  return false;
}

// Check if current PiBlock has instructions which are not identified as
// reductions but may prevent vectorization for this loop at \p LoopLevel.
// Return true for PiBlocks we want to create a new loop for, false otherwise.
bool preventsVectorization(PiBlock *Blk, DDGraph DDG, unsigned LoopLevel) {
  // Ignore large blocks as they are expensive to analyze and unlikely
  // to benefit distribution
  if (Blk->size() > 5) {
    return false;
  }

  for (auto NodeI = Blk->nodes_begin(), E = Blk->nodes_end(); NodeI != E;
       ++NodeI) {
    HLInst *HInst = dyn_cast<HLInst>(*NodeI);

    // Looking for inst which vectorizer cannot handle like this:
    // %ref = (cond) ? 0 : %ref  --- where %ref is a bad edge
    if (HInst && isa<SelectInst>(HInst->getLLVMInstruction())) {
      RegDDRef *LvalRef = HInst->getLvalDDRef();
      for (auto *Edge : DDG.outgoing(LvalRef)) {
        if (Edge->preventsVectorization(LoopLevel)) {
          return true;
        }
      }
    }
  }
  return false;
}

void HIRLoopDistribution::breakPiBlockRecurrences(
    const HLLoop *Lp, std::unique_ptr<PiGraph> const &PGraph,
    SmallVectorImpl<PiBlockList> &DistPoints) const {

  PiBlockList CurLoopPiBlkList;
  // Walk through topsorted nodes of Pigraph, keeping in mind the fact that each
  // of those nodes can legally form its own loop if the loops(distributed
  // chunks) are ordered in same topsort order of nodes.
  // Add the node to current loop. Look for outgoing edges that indicate
  // recurrences. If none, continue on. Otherwise terminate the current loop,
  // start a new one. Src and sink of recurrence will be in different loops,
  // breaking the recurrence.
  unsigned NumRefCounter = 0;
  SmallVector<unsigned, 12> MemRefSBVector;

  // To help vectorization we also split ranges of consecutive pi-blocks
  // containing user calls, because loops with user calls are less likely to be
  // vectorized.
  bool UserCallSeen = false;
  bool PrevUserCallSeen;

  SRA.computeSafeReductionChains(Lp);
  bool HasSparseArrayReductions = SARA.getNumSparseArrayReductionChains(Lp) > 0;
  PiBlockList DistributedBlocks;

  // Note: DDG was just computed in SARA
  DDGraph DDG = DDA.getGraph(Lp);

  auto CommitCurrentBlockList = [&]() {
    DistPoints.push_back(CurLoopPiBlkList);
    CurLoopPiBlkList.clear();
    NumRefCounter = 0;
  };

  // Get number of loads/stores, needed to decide if threshold is exceeded.
  // Arrays with same SB, in general, have locality, and do not need to be
  // added twice.  Will need some fine tuning later

  for (auto N = PGraph->node_begin(), E = PGraph->node_end(); N != E; ++N) {
    PrevUserCallSeen = UserCallSeen;
    UserCallSeen = false;

    PiBlock *SrcBlk = *N;

    bool NoOutgoingDeps = PGraph->outgoing(SrcBlk).empty();
    // If this current block has sparse array reduction instructions,
    // we need to break the recurrence before this block so that sparse array
    // reductions can be distributed into another separate loop.
    if (HasSparseArrayReductions && !CurLoopPiBlkList.empty() &&
        containsSparseArrayReductions(SrcBlk, SARA)) {

      // If there is no outgoing dependencies from sparse array reduction block,
      // we can combine such blocks at the end.
      if (NoOutgoingDeps) {
        DistributedBlocks.push_back(SrcBlk);
        continue;
      }
    }

    if (preventsVectorization(SrcBlk, DDG, Lp->getNestingLevel())) {
      if (NoOutgoingDeps) {
        DistributedBlocks.push_back(SrcBlk);
        continue;
      }
    }

    for (auto NodeI = SrcBlk->nodes_begin(), E = SrcBlk->nodes_end();
         NodeI != E; ++NodeI) {
      HLDDNode *Node = cast<HLDDNode>(*NodeI);

      // Split blocks with user calls.
      if (HLInst *Inst = dyn_cast<HLInst>(Node)) {
        if (Inst->isCallInst() && !Inst->getIntrinCall()) {
          UserCallSeen = true;
        }
      }

      for (auto RefIt = Node->ddref_begin(), E = Node->ddref_end(); RefIt != E;
           ++RefIt) {
        const RegDDRef *Ref = *RefIt;
        if (Ref->isMemRef()) {
          unsigned SB = Ref->getSymbase();
          if (std::find(MemRefSBVector.begin(), MemRefSBVector.end(), SB) !=
              MemRefSBVector.end()) {
            continue;
          }
          MemRefSBVector.push_back(SB);
          if (Ref->isRval()) {
            NumRefCounter++;
          } else {
            NumRefCounter += 2;
          }
        }
      }
    }

    // Split loops over blocks containing user calls.
    if (!CurLoopPiBlkList.empty() && PrevUserCallSeen && !UserCallSeen) {
      CommitCurrentBlockList();
    }

    CurLoopPiBlkList.push_back(SrcBlk);

    if (NumRefCounter >= MaxMemResourceToDistribute) {
      CommitCurrentBlockList();
      continue;
    }

    for (auto *Edge : PGraph->outgoing(SrcBlk)) {
      // TODO this is overly aggressive for at least two reasons.
      // Case1: 3 block graph with 2 edges,
      // 1->3, 2->3. This would create 3 loops, but 1 and 2 could have been
      // combined. OTOH, if piblock 1 would form a non vectorizable loop and
      // 2 doesnt, we would want them separate.
      // Case2: 2 block graph, with single recurrence edge between the two
      // Once split, the two loops are non vectorizable. If legality was
      // the only concern, we can iterate over all dd edges indirectly by
      // looking at distppedges whos src/sink are in piblock
      if (piEdgeIsMemRecurrence(Lp, *Edge)) {
        CommitCurrentBlockList();
        break;
      }
      // TODO if sink blk is known to be non vectorizable and src blk(s) is
      // vectorizble, we would want to split as well even if edge is not a
      // recurrence
    }
  }
  if (!CurLoopPiBlkList.empty()) {
    DistPoints.push_back(CurLoopPiBlkList);
  }

  if (!DistributedBlocks.empty()) {
    DistPoints.push_back(DistributedBlocks);
  }
}

void HIRLoopDistribution::findDistPoints(
    const HLLoop *Lp, std::unique_ptr<PiGraph> const &PGraph,
    SmallVectorImpl<PiBlockList> &DistPoints) const {

  if (DistCostModel == DistHeuristics::BreakMemRec) {
    breakPiBlockRecurrences(Lp, PGraph, DistPoints);
  } else if (DistCostModel == DistHeuristics::NestFormation) {
    if (!Lp->isInnermost()) {
      formPerfectLoopNests(PGraph, DistPoints);
    } else {
      splitSpatialLocalityGroups(Lp, PGraph, DistPoints);
    }
  }

  LLVM_DEBUG(dbgs() << "Loop Dist proposes " << DistPoints.size()
                    << " Loops\n");
}

unsigned HIRLoopDistribution::distributeLoopForDirective(HLLoop *Lp) {

  //  Process user pragma Loop Directive for distribution
  //  - No data dependency checking is needed
  //  - Multiple distribution points are allowed
  //  - User can put the pragma inside nested If statements, which is not
  //    handled in automatic  distribution
  //  IfNodeMap stores lowest and highest Loop Num

  if (DistCostModel != DistHeuristics::BreakMemRec || !Lp->isInnermost()) {
    return NotProcessed;
  }

  if (!Lp->canStripmine(StripmineSize)) {
    return TooComplex;
  }

  HLDDNode *Node = cast<HLDDNode>(Lp->getFirstChild());
  // Distribute point placed right before first stmt implies
  // no distribution
  if (Node && Node->isDistributePoint()) {
    Node->setDistributePoint(false);
    return NoDistribution;
  }

  // - Mark each HLNode with the Loop Num
  // - For if stmt - identify lowest and highest loop Num
  // - For each Node in HLNNode, walk top down
  //   - if loopNum != current new LoopNum
  //      push HLDDNodeList in DistributedLoops
  //   - if not IF stmt or current loopnum == lowest and highest loopnum
  //         -- save Node in HLDDNodeList because it can be moved as a nest
  //         -- continue
  //   (If stmt from there on)
  //   - Clone empty IF stmt
  //   - Walk all stmts in IF nest,
  //     - if loopNum in stmt matches current loop
  //         attach HLNode to new IF depending on T/F path

  DistDirectiveNodeMap.clear();
  IfNodeMap.clear();

  HLDDNodeList CurLoopHLDDNodeList;
  HLDDNode *TopIfNode = nullptr;
  unsigned TopIfNodeLoopNum = 0;
  unsigned DistLoopNum = 1;
  PragmaReturnCode UnsupportedRC = Success;

  ForEach<HLNode>::visitRange(
      Lp->child_begin(), Lp->child_end(),
      [this, &DistLoopNum, &TopIfNode, &TopIfNodeLoopNum,
       &UnsupportedRC](HLNode *Node) {
        HLDDNode *HNode = dyn_cast<HLDDNode>(Node);

        if (!HNode || isa<HLSwitch>(HNode)) {
          UnsupportedRC = UnsupportedStmts;
          return;
        }

        if (HNode->isDistributePoint()) {
          HNode->setDistributePoint(false);
          DistLoopNum++;
          if (DistLoopNum >= MaxDistributedLoop) {
            UnsupportedRC = TooManyDistributePoints;
            return;
          }
        }
        // 2nd argument indicates insert/move (for cloned/existing)
        DistDirectiveNodeMap[HNode] = std::make_pair(DistLoopNum, false);
        if (!(isa<HLIf>(HNode->getParent()))) {
          if (TopIfNode) {
            IfNodeMap[TopIfNode] =
                std::make_pair(TopIfNodeLoopNum, DistLoopNum);
          }
          if (isa<HLIf>(HNode)) {
            TopIfNode = HNode;
            TopIfNodeLoopNum = DistLoopNum;
          } else {
            TopIfNode = nullptr;
          }
        }
      });

  if (UnsupportedRC != Success) {
    return UnsupportedRC;
  }
  if (TopIfNode) {
    // TopIfNode is live out
    IfNodeMap[TopIfNode] = std::make_pair(TopIfNodeLoopNum, DistLoopNum);
  }

  SmallVector<HLDDNodeList, 8> DistributedLoops;
  collectHNodesForDirective(Lp, DistributedLoops, CurLoopHLDDNodeList);
  ScalarExpansion SCEX(Lp->getNestingLevel(), true, DistributedLoops);
  invalidateLoop(Lp);
  distributeLoop(Lp, DistributedLoops, SCEX, HIRF.getORBuilder(), true);
  return Success;
}

void HIRLoopDistribution::collectHNodesForDirective(
    HLLoop *Lp, SmallVectorImpl<HLDDNodeList> &DistributedLoops,
    HLDDNodeList &CurLoopHLDDNodeList) {

  unsigned DistLoopNum = 1;

  for (auto NodeIt = Lp->child_begin(), E = Lp->child_end(); NodeIt != E;
       ++NodeIt) {
    HLDDNode *HNode = cast<HLDDNode>(&*NodeIt);
    if (DistLoopNum != DistDirectiveNodeMap[HNode].first) {
      DistributedLoops.push_back(CurLoopHLDDNodeList);
      CurLoopHLDDNodeList.clear();
      DistLoopNum++;
    }

    if ((!isa<HLIf>(HNode) &&
         DistLoopNum == DistDirectiveNodeMap[HNode].first) ||
        (isa<HLIf>(HNode) && DistLoopNum == IfNodeMap[HNode].first &&
         DistLoopNum == IfNodeMap[HNode].second)) {
      CurLoopHLDDNodeList.push_back(HNode);
      continue;
    }

    if (isa<HLIf>(HNode)) {
      for (unsigned LoopNum = IfNodeMap[HNode].first;
           LoopNum <= IfNodeMap[HNode].second; ++LoopNum) {
        HLDDNode *NewIf =
            processPragmaForIf(HNode, HNode, CurLoopHLDDNodeList, LoopNum);
        if (NewIf) {
          CurLoopHLDDNodeList.push_back(NewIf);
        }
        if (LoopNum != IfNodeMap[HNode].second) {
          DistributedLoops.push_back(CurLoopHLDDNodeList);
          CurLoopHLDDNodeList.clear();
        }
        DistLoopNum = LoopNum;
      }
    }
  }

  if (CurLoopHLDDNodeList.size()) {
    DistributedLoops.push_back(CurLoopHLDDNodeList);
  }
}

void HIRLoopDistribution::moveIfChildren(HLContainerTy::iterator Begin,
                                         HLContainerTy::iterator End,
                                         HLIf *NewHLIf, HLDDNode *TopIfHNode,
                                         HLDDNodeList &CurLoopHLDDNodeList,
                                         unsigned TopIfLoopNum,
                                         bool IsThenChild) {

  unsigned NodeLoopNum = 0;
  HLDDNode *NewHLIfChild = nullptr;

  for (auto Iter = Begin; Iter != End;) {

    HLDDNode *Node = cast<HLDDNode>(&*Iter);

    NodeLoopNum = DistDirectiveNodeMap[Node].first;
    // Iter needs to be bumped up here because Node is changed
    ++Iter;
    if (isa<HLIf>(Node)) {
      NewHLIfChild = processPragmaForIf(TopIfHNode, Node, CurLoopHLDDNodeList,
                                        TopIfLoopNum);
      if (NewHLIfChild) {
        IsThenChild ? HLNodeUtils::insertAsLastThenChild(NewHLIf, NewHLIfChild)
                    : HLNodeUtils::insertAsLastElseChild(NewHLIf, NewHLIfChild);
      }
    } else if (NodeLoopNum == TopIfLoopNum) {
      IsThenChild ? HLNodeUtils::moveAsLastThenChild(NewHLIf, Node)
                  : HLNodeUtils::moveAsLastElseChild(NewHLIf, Node);
    }
  }
}

HLDDNode *HIRLoopDistribution::processPragmaForIf(
    HLDDNode *TopIfHNode, HLDDNode *CurrentIfHNode,
    HLDDNodeList &CurLoopHLDDNodeList, unsigned TopIfLoopNum) {

  HLNode *Node = CurrentIfHNode;
  HLIf *IfParent = cast<HLIf>(Node);
  HLIf *NewHLIf = IfParent->cloneEmpty();

  if (TopIfHNode == CurrentIfHNode) {
    DistDirectiveNodeMap[NewHLIf] = std::make_pair(TopIfLoopNum, true);
  }

  moveIfChildren(IfParent->then_begin(), IfParent->then_end(), NewHLIf,
                 TopIfHNode, CurLoopHLDDNodeList, TopIfLoopNum, true);
  moveIfChildren(IfParent->else_begin(), IfParent->else_end(), NewHLIf,
                 TopIfHNode, CurLoopHLDDNodeList, TopIfLoopNum, false);

  // For nested If statements, pragma can be inserted anywhere and
  // ends up with empty If.

  if (!NewHLIf->hasThenChildren() && !NewHLIf->hasElseChildren()) {
    NewHLIf = nullptr;
  }
  return NewHLIf;
}

void HIRLoopDistribution::invalidateLoop(loopopt::HLLoop *Loop) const {
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<HIRLoopStatistics>(
      Loop);
  HIRInvalidationUtils::invalidateBody(Loop);
}

void HIRLoopDistributionLegacyPass::getAnalysisUsage(
    llvm::AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
  // Loop Statistics is not used by this pass directly but it used by
  // HLNodeUtils::dominates() utility. This is a workaround to keep the pass
  // manager from freeing it.
  AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
  AU.addRequiredTransitive<HIRLoopResourceWrapperPass>();
  AU.addRequiredTransitive<HIRLoopLocalityWrapperPass>();
  AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
  AU.addRequiredTransitive<HIRSafeReductionAnalysisWrapperPass>();
  AU.addRequiredTransitive<HIRSparseArrayReductionAnalysisWrapperPass>();
}

bool HIRLoopDistributionLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    return false;
  }

  return HIRLoopDistribution(
             getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
             getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
             getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR(),
             getAnalysis<HIRSparseArrayReductionAnalysisWrapperPass>()
                 .getHSAR(),
             getAnalysis<HIRLoopResourceWrapperPass>().getHLR(),
             getAnalysis<HIRLoopLocalityWrapperPass>().getHLL(), DistCostModel)
      .run();
}
