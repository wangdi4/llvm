#if INTEL_COLLAB
//===---------------- IntelVPlanSyncDependenceAnalysis.cpp ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
/// This file implements an algorithm that returns for a divergent branch
/// the set of basic blocks whose phi nodes become divergent due to divergent
/// control. These are the blocks that are reachable by two disjoint paths from
/// the branch or loop exits that have a reaching path that is disjoint from a
/// path to the loop latch.
///
/// The SyncDependenceAnalysis is used in the DivergenceAnalysis to model
/// control-induced divergence in phi nodes.
///
/// -- Summary --
/// The SyncDependenceAnalysis lazily computes sync dependences [3].
/// The analysis evaluates the disjoint path criterion [2] by a reduction
/// to SSA construction. The SSA construction algorithm is implemented as
/// a simple data-flow analysis [1].
///
/// [1] "A Simple, Fast Dominance Algorithm", SPI '01, Cooper, Harvey and
/// Kennedy [2] "Efficiently Computing Static Single Assignment Form
///     and the Control Dependence Graph", TOPLAS '91,
///           Cytron, Ferrante, Rosen, Wegman and Zadeck
/// [3] "Improving Performance of OpenCL on CPUs", CC '12, Karrenberg and Hack
/// [4] "Divergence Analysis", TOPLAS '13, Sampaio, Souza, Collange and Pereira
///
/// -- Sync dependence --
/// Sync dependence [4] characterizes the control flow aspect of the
/// propagation of branch divergence. For example,
///
///   %cond = icmp slt i32 %tid, 10
///   br i1 %cond, label %then, label %else
/// then:
///   br label %merge
/// else:
///   br label %merge
/// merge:
///   %a = phi i32 [ 0, %then ], [ 1, %else ]
///
/// Suppose %tid holds the thread ID. Although %a is not data dependent on %tid
/// because %tid is not on its use-def chains, %a is sync dependent on %tid
/// because the branch "br i1 %cond" depends on %tid and affects which value %a
/// is assigned to.
///
/// -- Reduction to SSA construction --
/// There are two disjoint paths from A to X, if a certain variant of SSA
/// construction places a phi node in X under the following set-up scheme [2].
///
/// This variant of SSA construction ignores incoming undef values.
/// That is paths from the entry without a definition do not result in
/// phi nodes.
///
///       entry
///     /      \
///    A        \
///  /   \       Y
/// B     C     /
///  \   /  \  /
///    D     E
///     \   /
///       F
/// Assume that A contains a divergent branch. We are interested
/// in the set of all blocks where each block is reachable from A
/// via two disjoint paths. This would be the set {D, F} in this
/// case.
/// To generally reduce this query to SSA construction we introduce
/// a virtual variable x and assign to x different values in each
/// successor block of A.
///           entry
///         /      \
///        A        \
///      /   \       Y
/// x = 0   x = 1   /
///      \  /   \  /
///        D     E
///         \   /
///           F
/// Our flavor of SSA construction for x will construct the following
///            entry
///          /      \
///         A        \
///       /   \       Y
/// x0 = 0   x1 = 1  /
///       \   /   \ /
///      x2=phi    E
///         \     /
///          x3=phi
/// The blocks D and F contain phi nodes and are thus each reachable
/// by two disjoins paths from A.
///
/// -- Remarks --
/// In case of loop exits we need to check the disjoint path criterion for loops
/// [2]. To this end, we check whether the definition of x differs between the
/// loop exit and the loop header (_after_ SSA construction).
///
//===----------------------------------------------------------------------===//
#include "IntelVPlanSyncDependenceAnalysis.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"

#include <stack>
#include <unordered_set>

#define DEBUG_TYPE "vplan-sync-dependence"

namespace llvm {
namespace vpo {

ConstBlockSet SyncDependenceAnalysis::EmptyBlockSet;

SyncDependenceAnalysis::SyncDependenceAnalysis(const VPBlockBase *RegionEntry,
                                               const VPDominatorTree &DT,
                                               const VPPostDominatorTree &PDT,
                                               const VPLoopInfo &LI)
    // In VPlan, the RegionEntry is not the same as DT.getRoot()->getParent(),
    // which is what is used in the community version. The valid entry point for
    // the region in VPlan is passed in to avoid seg fault in RPOT traversal.
    : RegRPOT(RegionEntry), DT(DT), PDT(PDT), LI(LI) {}

SyncDependenceAnalysis::~SyncDependenceAnalysis() {}

using RegionRPOT = ReversePostOrderTraversal<const VPBlockBase *>;

// divergence propagator for reducible CFGs
struct DivergencePropagator {
  RegionRPOT &RegRPOT;
  const VPDominatorTree &DT;
  const VPPostDominatorTree &PDT;
  const VPLoopInfo &LI;

  // identified join points
  std::unique_ptr<ConstBlockSet> JoinBlocks;

  // reached loop exits (by a path disjoint to a path to the loop header)
  SmallPtrSet<const VPBlockBase *, 4> ReachedLoopExits;

  // if DefMap[B] == C then C is the dominating definition at block B
  // if DefMap[B] ~ undef then we haven't seen B yet
  // if DefMap[B] == B then B is a join point of disjoint paths from X or B is
  // an immediate successor of X (initial value).
  using DefiningBlockMap = std::map<const VPBlockBase *, const VPBlockBase *>;
  DefiningBlockMap DefMap;

  // all blocks with pending visits
  std::unordered_set<const VPBlockBase *> PendingUpdates;

  DivergencePropagator(RegionRPOT &RegRPOT, const VPDominatorTree &DT,
                       const VPPostDominatorTree &PDT, const VPLoopInfo &LI)
      : RegRPOT(RegRPOT), DT(DT), PDT(PDT), LI(LI),
        JoinBlocks(new ConstBlockSet) {}

  // set the definition at @Block and mark @Block as pending for a visit
  void addPending(const VPBlockBase &Block, const VPBlockBase &DefBlock) {
    bool WasAdded = DefMap.emplace(&Block, &DefBlock).second;
    if (WasAdded)
      PendingUpdates.insert(&Block);
  }

  void printDefs(raw_ostream &Out) {
    Out << "Propagator::DefMap {\n";
    for (const auto *Block : RegRPOT) {
      auto It = DefMap.find(Block);
      Out << Block->getName() << " : ";
      if (It == DefMap.end()) {
        Out << "\n";
      } else {
        const auto *DefBlock = It->second;
        Out << (DefBlock ? DefBlock->getName() : "<null>") << "\n";
      }
    }
    Out << "}\n";
  }

  // process @SuccBlock with reaching definition @DefBlock
  // the original divergent branch was in @ParentLoop (if any)
  void visitSuccessor(const VPBlockBase &SuccBlock, const VPLoop *ParentLoop,
                      const VPBlockBase &DefBlock) {

    // @SuccBlock is a loop exit (i.e., ParentLoop->getExitBlock())
    //
    // Example - Here, assume divergent branch at B, SuccBlock == Y (loop exit)
    //           Def at DefBlock B is propagated to Y and the ReachedLoopExits
    //           of X and Y are cached to analyze later for divergent loop
    //           exits.
    //
    //                  H <---
    //                 / \   |
    //  Exit Block -> X   B -- Def of B is propagated to Y (Exiting block of H)
    //  of loop H     |   |
    //                |   Y <--- Exit block of loop H
    //                |   |
    //                 \  /
    //                 Exit
    //
    if (ParentLoop && !ParentLoop->contains(&SuccBlock)) {
      DefMap.emplace(&SuccBlock, &DefBlock);
      ReachedLoopExits.insert(&SuccBlock);
      return;
    }

    // Example - SuccBlocks of BB3 (BB4, BB5) are inside loop and defs of
    //           SuccBlocks are propagated from 1) BB5 to BB6 on the first
    //           pass of this function. 2) Reached the join point on path
    //           BB4 to BB6 for join of two defs from BB4 and BB5.
    //
    //            BB3 <------------
    //           /   \            |
    // (x = 1) BB4   BB5 (x = 2)  |
    //           \   /            |
    //            BB6 -------------
    //
    // When this function is first called, the successors of BB3 (i.e., BB4,
    // BB5) will be in DefMap, where DefMap[BB4] = BB4 and DefMap[BB5] = BB5.
    // That is, BB4 and BB5 are immediate successors of BB3 and each define a
    // value for 'x'. First reaching def will be BB5 (due to RPOT) and this
    // function will early exit at the first return below. At this point,
    // DefMap[BB6] = BB5 since the def from BB5 is the only def of 'x' that has
    // been propagated to BB6. In the next invocation of this function, BB4
    // will be the second reaching def to BB6 and we will see in the second if
    // below that there was already a different reaching def recorded in DefMap.
    // Thus, we will now know there are two reaching defs and we're at a join
    // point.

    // first reaching def?
    auto ItLastDef = DefMap.find(&SuccBlock);
    if (ItLastDef == DefMap.end()) {
      addPending(SuccBlock, DefBlock);
      return;
    }

    // a join of at least two definitions
    if (ItLastDef->second != &DefBlock) {
      // do we know this join already?
      if (!JoinBlocks->insert(&SuccBlock).second)
        return;

      // update the definition
      addPending(SuccBlock, SuccBlock);
    }
  }

  // find all blocks reachable by two disjoint paths from @rootTerm.
  // This method works for both divergent TerminatorInsts and loops with
  // divergent exits.
  // @RootBlock is either the block containing the branch or the header of the
  // divergent loop.
  // @NodeSuccessors is the set of successors of the node (Loop or Terminator)
  // headed by @RootBlock.
  // @ParentLoop is the parent loop of the Loop or the loop that contains the
  // Terminator.

  // The overall logic of computeJoinPoints works as follows:
  //
  // 1) Start by calculating the region that may be influenced by a divergent
  //    branch instruction. Here, the post-dominator of the block containing the
  //    divergent branch is used to determine the region.
  //
  // 2) Follow the successors of the divergent branch and propagate defs through
  //    the CFG using RPO traversal until the immediate post dominator block of
  //    RootBlock is reached. Essentially, this is the mechanism for how we
  //    determine if multiple defs reach a join point from the divergent branch,
  //    thus identifying a block for which divergence should be propagated.
  //    E.g., a phi that becomes divergent. Consequently, users of such phis
  //    also become divergent. DefMap keeps track of the paths visited and where
  //    the def at each block in the map is propagated from. If we reach a point
  //    (block) in the CFG where a def has already been propaged previously to
  //    it, then we know we've hit a divergent join point.
  //
  // 3) Divergence introduced by loop exits is handled separately at the end of
  //    this function. As each block in the CFG is visited, the algorithm marks
  //    when it sees a block that is also a loop exit. Later, these loop exits
  //    are analyzed to see where the branches of a loop exit cause divergence.
  //    This is done by determining if the def at the loop header is different
  //    from the def at the loop exit. See example at the bottom of this
  //    function.
  //
  // Source of code divergence with the community is with how NodeSuccessors
  // is passed into this function. The community uses an iterable range of
  // successors (succ_const_range), while we just use a SmallVector.
  std::unique_ptr<ConstBlockSet>
  computeJoinPoints(const VPBlockBase &RootBlock,
                    const SmallVectorImpl<VPBlockBase *> &NodeSuccessors,
                    const VPLoop *ParentLoop) {

    assert(JoinBlocks &&
           "JoinBlocks have already been computed for some divergent region. "
           "That analysis should finish before calling this function again");

    // immediate post dominator (no join block beyond that block)
    const auto *PdNode = PDT.getNode(const_cast<VPBlockBase *>(&RootBlock));
    assert(PdNode && "Immediate post dominator node is null.");
    const auto *IpdNode = PdNode->getIDom();
    const auto *PdBoundBlock = IpdNode ? IpdNode->getBlock() : nullptr;

    // bootstrap with branch targets
    for (const auto *SuccBlock : NodeSuccessors) {
      DefMap.emplace(SuccBlock, SuccBlock);

      if (ParentLoop && !ParentLoop->contains(SuccBlock)) {
        // immediate loop exit from node.
        ReachedLoopExits.insert(SuccBlock);
        continue;
      }

      // regular successor
      PendingUpdates.insert(SuccBlock);
    }

    auto ItBeginRPO = RegRPOT.begin();

    // skip until term (TODO RPOT won't let us start at @term directly)
    for (; *ItBeginRPO != &RootBlock; ++ItBeginRPO) {
    }

    auto ItEndRPO = RegRPOT.end();
    assert(ItBeginRPO != ItEndRPO);

    // propagate definitions at the immediate successors of the node in RPO
    auto ItBlockRPO = ItBeginRPO;
    while (++ItBlockRPO != ItEndRPO && *ItBlockRPO != PdBoundBlock) {
      const auto *Block = *ItBlockRPO;

      // skip @Block if not pending update
      auto ItPending = PendingUpdates.find(Block);
      if (ItPending == PendingUpdates.end())
        continue;
      PendingUpdates.erase(ItPending);

      // propagate definition at @Block to its successors
      auto ItDef = DefMap.find(Block);
      const auto *DefBlock = ItDef->second;
      assert(DefBlock);

      auto *BlockLoop = LI.getLoopFor(Block);
      if (ParentLoop &&
          (ParentLoop != BlockLoop && ParentLoop->contains(BlockLoop))) {
        // if the successor is the header of a nested loop pretend its a
        // single node with the loop's exits as successors
        //
        // Example:
        //
        //                  ------------
        //                  |          |
        //                  | ParentLp |<---------------------
        //  divergent br -> |          |                     |
        //                  ------------                     |
        //                 /            \                    |
        //                /            --\-----------------  |
        //               /             |  \               |  |
        //     -----------             |  -----------     |  |
        //     |         |             |  |         |     |  |
        //     |  x = 0  |             |  | BlockLp |<--- |  |
        //     |         |             |  |   x++   |   | |  |
        //     -----------     Single  |  -----------   | |  |
        //          |           Node ->|       |        | |  |
        //          |                  |       |        | |  |
        //          |            x = n |  -----------   | |  |
        //          |                  |  |         |   | |  |
        //          |                  |  |         |---- |  |
        //          |                  |  |         |     |  |
        //          |                  |  -----------     |  |
        //          |                  |       |          |  |
        //          |                  --------|-----------  |
        //          |                          |             |
        //          |                     -----------        |
        //          ----------------------|         |        |
        //                                |         |---------
        //                  BlockLoopExit |         |
        //                  (Join Point)  -----------
        //
        //
        SmallVector<VPBlockBase *, 4> BlockLoopExits;
        BlockLoop->getExitBlocks(BlockLoopExits);
        for (const auto *BlockLoopExit : BlockLoopExits)
          visitSuccessor(*BlockLoopExit, ParentLoop, *DefBlock);

      } else {
        // the successors are either on the same loop level or loop exits
        // In VPlan, VPBasicBlocks are not inherited from BasicBlock, so the
        // successors interface is not available.
        for (const auto *SuccBlock : Block->getSuccessors())
          visitSuccessor(*SuccBlock, ParentLoop, *DefBlock);
      }
    }

    // We need to know the definition at the parent loop header to decide
    // whether the definition at the header is different from the definition at
    // the loop exits, which would indicate a divergent loop exits.
    //
    // A // loop header
    // |
    // B // nested loop header
    // |
    // C -> X (exit from B loop) -..-> (A latch)
    // |
    // D -> back to B (B latch)
    // |
    // proper exit from both loops
    //
    // D post-dominates B as it is the only proper exit from the "A loop".
    // If C has a divergent branch, propagation will therefore stop at D.
    // That implies that B will never receive a definition.
    // But that definition can only be the same as at D (D itself in this case)
    // because all paths to anywhere have to pass through D.
    //
    //
    //
    // The following example below demonstrates how a def is propagated to a
    // ParentLoopHeader when there is a divergent branch at RootBlock. In other
    // words, does the loop header see a different definition than the one at
    // the post-dominator of RootBlock (PdBoundBlock) due to the divergence at
    // RootBlock?
    //
    // Case 1: RootBlock = H (block of divergent branch), ParentLoopHeader = H,
    //         PdBoundBlock (post-dominator of H) = X. Here, PdBoundBlock X is
    //         not within loop H, so the def from X is not the same as the def
    //         seen at H. i.e., some lanes may re-enter the loop, but others
    //         will reach X. Thus, the observed def at X will be divergent.
    //
    // Case 2: RootBlock = H, ParentLoopHeader = G, PdBoundBlock = X (RootBlock
    //         is still H). Here, PdBoundBlock of H is within loop G, so we know
    //         that the def at X will be the same at G. Remember, PdBoundBlock
    //         is a convergence point. Any def here will be the same def for
    //         any non-divergent successors (e.g., here X -> G).
    //
    //                -----
    //                |   |
    //        ------->| G | (ParentLoopHeader1)
    //        |       |   |
    //        |       -----
    //        |         |
    //        |       -----
    //        |   --->|   |
    //        |   |   | H | (RootBlock & ParentLoopHeader2)
    //        |   ----|   |<--- divergent br
    //        |       -----
    //        |         |
    //        |       -----
    //        |       |   |
    //        --------| X | (PdBoundBlock)
    //                |   |
    //                -----
    //                  |
    //                -----
    //                |   |
    //                | Y |
    //                |   |
    //                -----
    //
    //
    const VPBlockBase *ParentLoopHeader =
        ParentLoop ? ParentLoop->getHeader() : nullptr;
    if (ParentLoop && ParentLoop->contains(PdBoundBlock))
      DefMap[ParentLoopHeader] = DefMap[PdBoundBlock];

    // analyze all reached loop exits - Do we have a divergent loop exit?
    // This means that from the root divergent branch instruction at RootBlock,
    // we have reached a loop exit block (NOT exiting block) that is divergent.
    //
    // Example:
    //
    //
    //                ---------
    //                |       |
    //                | Entry |-----
    //   uniform br ->| x = 0 |    |
    //                ---------    |
    //                    |        |
    //   LoopHeader   ---------    |
    //   (RootBlock)  |       |    |
    //   ------------>|   H   |    |
    //   | uniform x  | div br|    |
    //   |            ---------    |
    //   |           /         \   |
    //   |   ---------         ---------
    //   |   |       |         |       | <--- phi(xEntry, xH)
    //   ----|   X   |         |   B   | LoopExit
    //       |  x++  |         |       |
    //       ---------         ---------
    //           |            /  |
    //       ---------       /   |
    //       |       |______/    |
    //       |   Y   |           |
    //       |       | LoopExit  |
    //       ---------           |
    //           |               |
    //           |       ---------
    //           |       |       |
    //           --------| Exit  | Post Dominator of LoopHeader
    //                   |       |
    //                   ---------
    //
    // Step 2 described above begins by finding join points from the divergent
    // branch at H to post dominator Exit. As such, it finds that blocks Y and
    // Exit are divergent. Now, determine if B and Y are divergent loop exits.
    // A divergent loop exit means that some lanes will exit the loop at
    // different points than others. In the example above, this means that some
    // lanes will see the incremented value of 'x' at H, but the lanes that take
    // an early exit at B will see a different value of 'x'. Note that 'x' is
    // uniform for all lanes executing the loop and it is not until all lanes
    // exit the loop where different values of 'x' will be observed (i.e.,
    // temporal divergence at B). Also note that if all lanes come from Entry
    // to B, then there is no divergence due to the uniform branch, but if all
    // lanes go through H, then 'x' will be observed differently at H and B. As
    // such, the phi at B must be divergent.
    //
    // For this example, DefMap is defined as follows:
    //
    // DefMap {
    //   H : X
    //   X : X
    //   B : B
    //   Y : X
    //   Exit : N/A
    // }
    //
    // For loop exit B, we know the def of x is defined at B (hence, B : B
    // in the map). This value of 'x' comes from the phi. The def for the
    // LoopHeader H is coming from X. Note that even since there is a loop
    // pre-header of Entry defining 'x,' 'x' is redefined in B or X and the
    // only def that can make it back to the loop header is from X. Since the
    // def at H is different from the def at B, B is marked as a divergent loop
    // exit and any phis in this block are marked as divergent. (i.e., the
    // observed value of 'x' at B is divergent). For Y, we know the def of x is
    // defined at X and the def for the LoopHeader H is also coming from X.
    // Thus, the loop exit Y is not divergent. Note: why does the DefMap show
    // the def of x for Y comes from X and not B? B is a loop exit block and
    // those are not inserted into the set of blocks pending a visit
    // (PendingUpdates) for the 1st part of def propagation. The code below
    // handles loop exits separately. Thus, X is the only def of x for Y found
    // in DefMap.
    //
    if (!ReachedLoopExits.empty()) {
      assert(ParentLoopHeader &&
             "cannot determine def for missing loop header");
      const auto *HeaderDefBlock = DefMap[ParentLoopHeader];
      LLVM_DEBUG(printDefs(dbgs()));
#if !INTEL_CUSTOMIZATION
      assert(HeaderDefBlock && "no definition in header of carrying loop");
#endif

      for (const auto *ExitBlock : ReachedLoopExits) {
        auto ItExitDef = DefMap.find(ExitBlock);
        assert((ItExitDef != DefMap.end()) &&
               "no reaching def at reachable loop exit");
        // If the def at the loop header is not the same as the def at the loop
        // exit, then the loop exit block is divergent.
#if INTEL_CUSTOMIZATION
        // It's possible that propagation of defs stops at the post-dom block
        // of the loop header without carrying a def to the loop header. E.g.,
        //
        //       --------------------
        //       | BB165            |
        //  ---> | ParentLoopHeader |---------------
        //  |    |                  |              |
        //  |    --------------------              |
        //  |                                      |
        //  |                                      v
        //  |                               ---------------
        //  |                               | BB167       |
        //  |                               |  RootBlock  |
        //  |                         ------|             |-------------
        //  |                         |     |  div br     |            |
        //  |                         |     ---------------            |
        //  |                         |                                |
        //  |                         |                                |
        //  |                         v                                |
        //  |                    ------------                          |
        //  |                    |          |                          |
        //  |            --------|  BB168   |                          |
        //  |            |       |          |                          |
        //  |            |       ------------                          |
        //  |            |            |                                |
        //  |            |            |                                v
        //  |            |            |                         ----------------
        //  |            |            |                         | BB169        |
        //  |            |            ------------------------->| PdBoundBlock |
        //  |            |                                      | (loop exit)  |
        //  |            |                                      ----------------
        //  |            |                                             |
        //  |            |                                             |
        //  |            |                                             |
        //  |            v                                             v
        //  |    ------------
        //  |    |          |
        //  -----|  BB166   |
        //       |          |
        //       ------------
        //
        // In this example, def propagation will terminate once PdBoundBlock
        // (BB169) is reached via RPO. See while loop at the top of this
        // function. This will lead to a def not being set for BB165
        // (ParentLoopHeader). As such, the assert "no definition in header
        // of carrying loop" would be triggered. Here, the def at BB166 should
        // be propagated to BB165 and the def at BB165 would be different than
        // the def at BB169 anyway. In any case, it is safe to assume the block
        // is divergent and will be marked as such by insertion into JoinBlocks.
#endif
        if (!HeaderDefBlock || ItExitDef->second != HeaderDefBlock) // INTEL
          JoinBlocks->insert(ExitBlock);
      }
    }

    return std::move(JoinBlocks);
  }
};

const ConstBlockSet &SyncDependenceAnalysis::joinBlocks(const VPLoop &Loop) {
  using LoopExitVec = SmallVector<VPBlockBase *, 4>;
  LoopExitVec LoopExits;
  Loop.getExitBlocks(LoopExits);
  if (LoopExits.size() < 1)
    return EmptyBlockSet;

  // already available in cache?
  auto ItCached = CachedLoopExitJoins.find(&Loop);
  if (ItCached != CachedLoopExitJoins.end())
    return *ItCached->second;

  // compute all join points
  DivergencePropagator Propagator{RegRPOT, DT, PDT, LI};
  auto JoinBlocks = Propagator.computeJoinPoints(*Loop.getHeader(), LoopExits,
                                                 Loop.getParentLoop());

  auto ItInserted = CachedLoopExitJoins.emplace(&Loop, std::move(JoinBlocks));
  assert(ItInserted.second);
  return *ItInserted.first->second;
}

const ConstBlockSet &
SyncDependenceAnalysis::joinBlocks(const VPBlockBase &TermBlock) {
  // trivial case
  if (TermBlock.getNumSuccessors() < 1)
    return EmptyBlockSet;

  // already available in cache?
  auto ItCached = CachedBranchJoins.find(&TermBlock);
  if (ItCached != CachedBranchJoins.end())
    return *ItCached->second;

  // compute all join points
  DivergencePropagator Propagator{RegRPOT, DT, PDT, LI};
  auto JoinBlocks = Propagator.computeJoinPoints(
      TermBlock, TermBlock.getSuccessors(), LI.getLoopFor(&TermBlock));

  auto ItInserted =
      CachedBranchJoins.emplace(&TermBlock, std::move(JoinBlocks));
  assert(ItInserted.second);
  return *ItInserted.first->second;
}

} // namespace vpo
} // namespace llvm
#endif //INTEL_COLLAB
