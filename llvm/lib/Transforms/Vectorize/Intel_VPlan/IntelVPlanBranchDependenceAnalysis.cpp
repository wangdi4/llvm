//===--------------- VPlanBranchDependenceAnalysis.cpp --------------------===//
//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements an algorithm that returns for a divergent branch the
// set of basic blocks whose phi nodes become divergent due to divergent
// control flow. This includes logic to determine whether or not a non-uniform
// loop exit condition results in disjoint exit paths from the loop.
//
// The BranchDependenceAnalysis is used in the DivergenceAnalysis to model
// control-induced divergence in phi nodes.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlan.h"
#include "IntelVPlanBranchDependenceAnalysis.h"
#include "llvm/Support/Debug.h"
#include <stack>

#define DEBUG_TYPE "vplan-branch-dependence-analysis"

using namespace llvm;
using namespace llvm::vpo;

// This function is called by findDisjointPaths to help find multiple (in this
// case 2) disjoint paths within an SESE region from Source to Sink(s). Nodes
// are visited beginning with the Source OutNode and the path through the CFG
// to the Sink(s) is recorded such that the algorithm will not visit the same
// sequence of nodes. I.e., the recorded paths steer the algorithm toward
// searching for alternate paths.
//
// Example - For outer loop vectorization, %cond2 is divergent since it uses
// %iv as the loop exit condition. Thus, findPath() will be called to find paths
// from BB4 to BB3 (the outer phi), and from BB4 to BB4 (the inner loop phi).
// This example will focus on the latter case.
//
//                     |----------------------------|
//                     | BB3 (outer loop):          |
//                     | %iv = phi                  |<----------
//                     |                            |          |
//                     |                            |          |
//                     |----------------------------|          |
//                          BB3(OUT) |                         |
//                          BB4(IN)  |                         |
//                     |----------------------------|          |
//                     | BB4: (inner loop)          |<-----    |
//                     | %iv2 = phi                 |     |    |
//                     |                            |     |    |
//                     |                            |     |    |
//                     | %iv2next = %iv2 + 1        |------    |
//                     | %cond2 = icmp %iv2next, %iv|          |
//                     | br i1 %cond2, BB5, BB4     |          |
//                     |----------------------------|          |
//                          BB4(OUT) |                         |
//                          BB5(IN)  |                         |
//                     |----------------------------|          |
//                     | BB5 (outer latch)          |          |
//                     |                            |-----------
//                     | br i1 %cond1, BB6, BB3     | BB5(OUT)
//                     |----------------------------|
//                                   |
//                                   |
//                     |----------------------------|
//                     | BB6                        |
//                     |                            |
//                     | ret ...                    |
//                     |----------------------------|
//
// The first call to findPath() will record the following:
//
// Visit BB4(OutNode) and push BB4's successors BB5(InNode) and BB4(InNode)
// Visit BB4(InNode)
// Path is found from BB4(OutNode) -> BB4(InNode), so exit.
// Function injectFlow() records this path.
//
// The second call to findPath() will record the following:
//
// Visit BB4(OutNode) and push BB4's successor BB5(InNode) only because
// BB4(InNode) was already found in the first path.
// Visit BB5(InNode) and push BB5(OutNode)
// Visit BB5(OutNode) and push successors BB6(InNode) and BB3(InNode)
// Visit BB3(InNode) and push BB3(OutNode)
// Visit BB3(OutNode) and push BB4(InNode)
// Visit BB4(InNode)
// Path is found from BB4(OutNode) -> BB5(InNode) -> BB5(OutNode) ->
//                    BB3(InNode) -> BB3(OutNode) -> BB4(InNode)
//
// Thus, two disjoint paths exist and the inner loop phi becomes divergent.

const DivPathDecider::Node *
DivPathDecider::findPath(const Node &Source, const NodeList &Sinks,
                         const EdgeSet &Flow, PredecessorMap &Parent,
                         const VPLoop *TheLoop) const {

  DenseSet<const Node *> Visited;
  std::stack<const Node *> Stack;
  Stack.push(&Source);

  while (!Stack.empty()) {
    const Node *TopNode = Stack.top();
    Stack.pop();
    Visited.insert(TopNode);
    LLVM_DEBUG(dbgs() << "Visiting " << TopNode->BB.getName() << "("
                      << TopNode->Type << ")"
                      << "\n");

    const VPBlockBase &Runner = TopNode->BB;

    // The current node on the stack is a sink node, so the path has terminated.
    auto Found = std::find(Sinks.begin(), Sinks.end(), TopNode);
    if (Found != Sinks.end()) {
      LLVM_DEBUG(dbgs() << "Found path from " << Source.BB.getName() << " to "
                        << TopNode->BB.getName() << "\n");
      return *Found;
    }

    if (TopNode->Type == Node::OUT) {
      // Successors
      for (auto *Succ : Runner.getSuccessors()) {
        const Node *Next = getInNode(*Succ);
        if ((!TheLoop || TheLoop->contains(&Runner)) &&
            Visited.count(Next) == 0 && !Flow.count({TopNode, Next})) {
          LLVM_DEBUG(dbgs() << "Pushing(OUT Successors) " << Next->BB.getName()
                            << "(IN)\n");
          Stack.push(Next);
          Parent[Next] = TopNode;
        }
      }

      // Backwards split edge - TODO: find test case for this code.
      const Node *SplitIN = getInNode(Runner);
      if (Visited.count(SplitIN) == 0 && Flow.count({SplitIN, TopNode})) {
        LLVM_DEBUG(dbgs() << "Pushing(SplitIN) " << SplitIN->BB.getName()
                          << "(IN)\n");
        Stack.push(SplitIN);
        Parent[SplitIN] = TopNode;
      }
    } else {
      // Traverse split edge
      const Node *SplitOUT = getOutNode(Runner);
      if (Visited.count(SplitOUT) == 0 && !Flow.count({TopNode, SplitOUT})) {
        LLVM_DEBUG(dbgs() << "Pushing(SplitOUT) " << SplitOUT->BB.getName()
                          << "(OUT)\n");
        Stack.push(SplitOUT);
        Parent[SplitOUT] = TopNode;
      }

      // Predecessors - TODO: find test case for this code.
      for (auto *Pred : Runner.getPredecessors()) {
        const Node *Next = getOutNode(*Pred);
        if ((!TheLoop || TheLoop->contains(&Runner)) &&
            Visited.count(Next) == 0 && Flow.count({Next, TopNode})) {
          LLVM_DEBUG(dbgs() << "Pushing(IN Predecessors) " << Next->BB.getName()
                            << "(OUT)\n");
          Stack.push(Next);
          Parent[Next] = TopNode;
        }
      }
    }
  }

  return nullptr;
}

// Determine if a conditional branch causes divergent control flow paths out of
// the loop.
bool DivPathDecider::inducesDivergentExit(const VPBasicBlock &From,
                                          const VPBasicBlock &LoopExit,
                                          const VPLoop &TheLoop) const {
  if (&From == TheLoop.getLoopLatch()) {
    return LoopExit.getUniquePredecessor() == &From;
  }

  const Node *Source = getOutNode(From);
  NodeList Sinks = {getOutNode(LoopExit), getInNode(*TheLoop.getHeader())};
  return findDisjointPaths(*Source, Sinks, 2, &TheLoop);
}

// Find N vertex-disjoint paths from A to B, this algorithm is a specialization
// of Ford-Fulkerson, that terminates after a flow of N is found. Running time
// is thus O(Edges) * N. The flow edgeset shows the possible paths from From to
// To. An InNode represents an incoming edge to a node. An OutNode represents an
// outgoing edge from a node.
//
// In the following example, it is assumed that DA is running for an outer loop
// vectorization scenario and used to determine inner loop control flow
// uniformity. Thus, DA will use this function to attempt to find all divergent
// control flow paths from the branch using %cond2 to all phi nodes that are
// within an SESE region of the branch. That means for this case that this
// function will attempt to find disjoint paths where From = BB4 and To = BB3,
// and where From = BB4 and To = BB4. The following comments will show how this
// function operates for the latter case.
//
//
//                     |----------------------------|
//                     | BB3 (outer loop):          | BB3(IN)
//                     | %iv = phi                  |<---------
//                     |                            |         |
//                     |                            |         |
//                     |----------------------------|         |
//                          BB3(OUT) |                        |
//                          BB4(IN)  |                        |
//                     |----------------------------| BB4(IN) |
//                     | BB4: (inner loop)          |<-----   |
//                     | %iv2 = phi                 |     |   |
//                     |                            |     |   |
//                     |                            |     |   |
//                     | %iv2next = %iv2 + 1        |------   |
//                     | %cond2 = icmp %iv2next, %iv|         |
//                     | br i1 %cond2, BB5, BB4     |         |
//                     |----------------------------|         |
//                          BB4(OUT) |                        |
//                          BB5(IN)  |                        |
//                     |----------------------------|         |
//                     | BB5 (outer latch)          |         |
//                     |                            |----------
//                     | br i1 %cond1, %exit, BB3   | BB5(OUT)
//                     |----------------------------|
//
// All possible paths start from BB4(OutNode) because BB4 is the From node. The
// two possible paths are the following:
//
// 1) BB4(OUT) -> BB4(IN)
// 2) BB4(OUT) -> BB5(IN) -> BB5(OUT) -> BB3(IN) -> BB3(OUT) -> BB4(IN)
//
bool DivPathDecider::findDisjointPaths(const VPBasicBlock &From,
                                       const VPBasicBlock &To,
                                       unsigned int N) const {

  const Node *Source = getOutNode(From);
  NodeList Sinks = {getInNode(To)};
  return findDisjointPaths(*Source, Sinks, N, nullptr);
}

bool DivPathDecider::findDisjointPaths(const Node &Source,
                                       const NodeList &Sinks, unsigned int N,
                                       const VPLoop *TheLoop) const {
  EdgeSet Flow;

  for (unsigned i = 0; i < N; ++i) {
    PredecessorMap Parent;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    dbgs() << "\nWill attempt to find path " << (i + 1) << " of:\n";
    for (auto *S : Sinks)
      dbgs() << "Source " << Source.BB.getName() << " to Sink "
             << S->BB.getName() << "\n";
#endif
    const Node *Sink = findPath(Source, Sinks, Flow, Parent, TheLoop);
    if (!Sink) {
      LLVM_DEBUG(dbgs() << "Could not find another disjoint path!\n\n");
      return false;
    }

    injectFlow(Source, *Sink, Parent, Flow);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    for (auto FlowIt : make_range(Flow.begin(), Flow.end())) {
      const Node *Src = FlowIt.first;
      const Node *Sink = FlowIt.second;
      dbgs() << "Edge from " << Src->BB.getName() << "(" << Src->Type << ") to "
             << Sink->BB.getName() << "(" << Sink->Type << ")\n";
    }
#endif
  }

  return true;
}

// This function is executed during findDisjointPaths() for each disjoint path
// that is found from Start to End for a given SESE. Its purpose is to record
// the flow through the CFG from Start to End using forward edges only for each
// path. i.e., it does not keep backward edges between nodes in the case of
// modeling backward control flow for loops/gotos.
//
// Example output is taken from above example from findDisjointPaths. Please
// see these comments for a more complete description on how the paths are
// determined.
//
// Notice that the backward edge is missing from BB4 -> BB4 and is instead
// represented with a distinct OutNode and InNode with a forward edge.
//
// 1) BB4(OUT) -> BB4(IN)
// 2) BB4(OUT) -> BB5(IN) -> BB5(OUT) -> BB3(IN) -> BB3(OUT) -> BB4(IN)
//
void DivPathDecider::injectFlow(const Node &Start, const Node &End,
                                const PredecessorMap &Parent,
                                EdgeSet &Flow) const {
  const Node *Prev;
  for (const Node *Tail = &End; Tail && Tail != &Start; Tail = Prev) {
    Prev = Parent.find(Tail)->second;

    // Backwards edge reset
    if (Flow.erase({Tail, Prev}))
      continue;
    else
      // Ordinary edge insert
      Flow.insert({Prev, Tail});
  }
}

const DivPathDecider::Node *
DivPathDecider::getInNode(const VPBlockBase &BB) const {
  auto Found = InNodes.find(&BB);
  if (Found != InNodes.end()) {
    return &Found->second;
  }

  auto ItInsert = InNodes.insert(std::make_pair(&BB, Node(Node::IN, BB))).first;
  return &ItInsert->second;
}

const DivPathDecider::Node *
DivPathDecider::getOutNode(const VPBlockBase &BB) const {
  auto Found = OutNodes.find(&BB);
  if (Found != OutNodes.end()) {
    return &Found->second;
  }

  auto ItInsert =
      OutNodes.insert(std::make_pair(&BB, Node(Node::OUT, BB))).first;
  return &ItInsert->second;
}

VPlanBranchDependenceAnalysis::VPlanBranchDependenceAnalysis(
    VPBlockBase *RegionEntry, const VPDominatorTree *DT,
    const VPPostDominatorTree *PDT, const VPLoopInfo *VPLI)
    : RegionEntry(RegionEntry), DomTree(DT), PostDomTree(PDT), VPLI(VPLI) {}

const ConstBlockSet &
VPlanBranchDependenceAnalysis::joinBlocks(VPBasicBlock *TermBlock) const {

  auto FindIt = JoinBlocks.find(TermBlock);
  if (FindIt != JoinBlocks.end())
    return FindIt->second;

  auto *TermDomNode = DomTree->getNode(TermBlock);
  auto *TermPostDomNode = PostDomTree->getNode(TermBlock);
  auto *PostDomBoundNode =
      TermPostDomNode ? TermPostDomNode->getIDom() : nullptr;

  ConstBlockSet TermJoinBlocks;

  ReversePostOrderTraversal<VPBlockBase *> RPOT(RegionEntry);
  for (VPBlockBase *PhiBlockBase : make_range(RPOT.begin(), RPOT.end())) {
    assert(!isa<VPRegionBlock>(PhiBlockBase) &&
           "Disjoint path query does not support VPRegionBlocks");
    auto PhiBlock = dyn_cast<VPBasicBlock>(PhiBlockBase);
    if (PhiBlock) {
      VPBasicBlock::RecipeListTy &RecipeList = PhiBlock->getRecipes();
      // Don't worry about computing paths to empty blocks such as the region
      // entry/exit blocks since they obviously won't contain any PHI nodes.
      if (RecipeList.empty())
        continue;
      const VPRecipeBase &Recipe = RecipeList.front();
      if (const VPInstruction *VPInst = dyn_cast<VPInstruction>(&Recipe)) {
        unsigned OpCode = VPInst->getOpcode();
        if (OpCode != Instruction::PHI)
          continue;
      } else
        llvm_unreachable("Recipes are not supported for disjoint path queries");
    }

    // Determine if the block containing the divergent VPCondBitVPVal
    // (TermBlock) and the block containing the PHI node (PhiBlock) are in a
    // valid SESE region. In VPlan, conditional branch instructions are not
    // represented explicitly, so TermBlock is determined by getting the
    // VPCondBitVPVal of a VPBasicBlock. If a valid SESE region is found
    // from TermBlock to PhiBlock, then find out if there are at least 2
    // disjoint paths from TermBlock to PhiBlock. A valid SESE region for
    // TermBlock and PhiBlock is bounded by DomBoundNode and PostDomBoundNode.
    // Note that these bounds will not always be TermBlock and PhiBlock in
    // the if/else case. For loops, the SESE region will be bounded such that
    // DomBoundNode != TermDomNode && PostDomBoundNode != PhiPostDomNode. See
    // Example 2, where DomBoundNode and PostDomBoundNode surround the SESE
    // region of BB4. Likewise, in Example 2, if a disjoint path query were
    // run from BB4 to BB3, DomBoundNode = BB2 and PostDomBoundNode = exit,
    // indicating an SESE region for the outer loop.
    //
    // Example 1 (if/else):
    //
    //                     |------------------------------|   --------------
    //                     | BB3:                         |                |
    //        TermDomNode  |                              |                |
    //        TermBlock    |   %vp123 = icmp %vp345 i32 0 |                |
    //        DomBoundNode | CondBit: %vp123 (BB3)        |                |
    //                     |------------------------------|                |
    //                          |                   |                      |
    //                     True |                   | False                |
    //                          |                   |                      |
    // |------------------------------|             |              SESE    |
    // | BB4:                         |             |              Region  |
    // | %vp567 = ...;                |             |                      |
    // |------------------------------|             |                      |
    //                            \                 |                      |
    //                             \                |                      |
    //                              \               |                      |
    //               PhiPostDomNode   |------------------------------|     |
    //               PhiBlock         | BB5:                         |     |
    //               PostDomBoundNode | %vp780 = phi %vp567, 0;      |     |
    //                                |------------------------------|  ----
    //
    //
    // Example 2 (determine SESE region of %exitcond2 to %indvars.iv2):
    //
    //                     |----------------------------|
    //                     | BB2                        |
    //                     |                            |
    //                     | br BB3                     |
    //                     |----------------------------|
    //                                   |
    //                                   |
    //                     |----------------------------|   -------------------
    //                     | BB3 (outer loop):          |                     |
    //       DomBoundNode  | %indvars.iv = phi          |<---------           |
    //                     |                            |         |           |
    //                     |                            |         |           |
    //                     |----------------------------|         |           |
    //                                   |                        |           |
    //                                   |                        |           |
    //                     |----------------------------|         |           |
    //                     | BB4: (inner loop)          |<-----   |           |
    //      TermDomNode    | %indvars.iv2 = phi         |     |   |           |
    //      TermBlock      |                            |     |   |    SESE   |
    //      PhiPostDomNode |                            |     |   |    Region |
    //      PhiBlock       |                            |     |   |           |
    //                     |                            |------   |           |
    //                     | br i1 %exitcond2, BB5, BB4 |         |           |
    //                     |----------------------------|         |           |
    //                                   |                        |           |
    //                                   |                        |           |
    //                     |----------------------------|         |           |
    //                     | BB5 (outer latch)          |         |           |
    //    PostDomBoundNode |                            |----------           |
    //                     |                            |                     |
    //                     | br i1 %exitcond, %exit, BB3|                     |
    //                     |----------------------------|   -------------------
    //                                   |
    //                                   |
    //                     |----------------------------|
    //                     | exit                       |
    //                     |                            |
    //                     | ret ...                    |
    //                     |----------------------------|
    //
    // Definitions:
    //
    // TermBlock - block containing the conditional branch for which the
    //             SESE region is being computed.
    //
    // PhiBlock - block containing the phi node for which the SESE region is
    //            being computed.
    //
    // TermDomNode - the node in the dominator tree representing TermBlock.
    //
    // TermPostDomNode - the node in the post-dominator tree representing
    //                   TermBlock.
    //
    // PostDomBoundNode - the immediate post-dominator of TermPostDomNode.
    //
    // PhiDomNode - the node in the dominator tree representing PhiBlock.
    //
    // PhiPostDomNode - the node in the post-dominator tree representing the
    //                  PhiBlock.
    //
    // DomBoundNode - the immediate dominator of PhiDomNode.
    //
    // In all cases DomBoundNode should dominate TermDomNode and
    // PostDomBoundNode should post-dominate PhiPostDomNode. The next two
    // dominates() calls do just that and ensure that the block containing the
    // conditional branch and the block containing the phi node are in the same
    // SESE region. Otherwise, a valid SESE region does not exist.
    //
    auto *PhiDomNode = DomTree->getNode(PhiBlockBase);
    assert(PhiDomNode);
    auto *DomBoundNode = PhiDomNode->getIDom();
    if (DomBoundNode && !DomTree->dominates(DomBoundNode, TermDomNode))
      continue;

    auto *PhiPostDomNode = PostDomTree->getNode(PhiBlockBase);
    if (PostDomBoundNode &&
        !DomTree->dominates(PostDomBoundNode, PhiPostDomNode))
      continue;

    LLVM_DEBUG(dbgs() << "TermBlock: " << TermBlock->getName() << "\n");
    LLVM_DEBUG(dbgs() << "PhiBlock: " << PhiBlock->getName() << "\n");
    LLVM_DEBUG(dbgs() << "Immediate Dominator of PhiBlock: "
                      << DomBoundNode->getBlock()->getName() << "\n");
    LLVM_DEBUG(dbgs() << "Immediate PostDominator of TermBlock: "
                      << PostDomBoundNode->getBlock()->getName() << "\n");

    // The above conditions were satisfied, so a valid SESE region exists.
    // Run a disjoint path query.
    if (DPD.findDisjointPaths(*TermBlock, *PhiBlock)) {
      TermJoinBlocks.insert(PhiBlock);
    }
  }

  // Find divergent loop exits. For VPlan DA, this code is probably not
  // necessary because we re-wire multi-exit loops to a single loop exit during
  // HCFG construction. Thus, findDisjointPaths() should work the same for all
  // code. For now, keep it just in case it's ever needed since it certainly
  // won't hurt anything.
  if (const VPLoop *TheLoop = VPLI->getLoopFor(cast<VPBlockBase>(TermBlock))) {
    LLVM_DEBUG(dbgs() << "\nFinding divergent loop exits ...\n");
    LLVM_DEBUG(dbgs() << "TermBlock: " << TermBlock->getName() << "\n");

    SmallVector<VPBlockBase *, 4> LoopExits;
    TheLoop->getExitBlocks(LoopExits);

    for (const VPBlockBase *LoopExit : LoopExits) {
      LLVM_DEBUG(dbgs() << "LoopExit: " << LoopExit->getName() << "\n");
      if (DPD.inducesDivergentExit(*TermBlock, cast<VPBasicBlock>(*LoopExit),
                                   *TheLoop))
        TermJoinBlocks.insert(cast<VPBasicBlock>(LoopExit));
    }
  }

  auto InsertIt = JoinBlocks.insert(std::make_pair(TermBlock, TermJoinBlocks));
  assert(InsertIt.second);
  return InsertIt.first->second;
}
