//===-- CSAMemopOrdering.cpp - Standard memop ordering pass -----*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
///===---------------------------------------------------------------------===//
/// \file
///
/// This file implements the standard CSA memop ordering pass.
///
///===---------------------------------------------------------------------===//

#include "CSAMemopOrdering.h"
#include "CSATargetMachine.h"
#include "Intel_CSA/Transforms/Scalar/CSALowerParallelIntrinsics.h"

#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/CodeGen/MachineMemOperand.h" // To get CSA_LOCAL_CACHE_METADATA_KEY.
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsCSA.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/GraphWriter.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <map>
#include <queue>

using namespace llvm;

static cl::opt<bool> RacyLoops{
  "csa-memop-ordering-racy-loops", cl::Hidden,
  cl::desc("CSA-specific: Ignore all ordering across loop backedges."),
  cl::init(false)};

static cl::opt<bool> IgnoreAliasInfo{
  "csa-memop-ordering-ignore-aa", cl::Hidden,
  cl::desc("CSA-specific: ignore alias analysis results when constructing "
           "ordering chains and assume everything aliases."),
  cl::init(false)};

static cl::opt<bool> IgnoreAnnotations{
  "csa-memop-ordering-ignore-annotations", cl::Hidden,
  cl::desc("CSA-specific: Ignore parallel region/section annotations during "
           "memop ordering."),
  cl::init(false)};

static cl::opt<bool> IgnoreSelfDeps{
  "csa-memop-ordering-explicit-self-deps", cl::Hidden,
  cl::desc("CSA-specific: Explicitly enforce self dependencies during memop "
           "ordering."),
  cl::init(false)};

static cl::opt<bool> IgnoreDataDeps{
  "csa-memop-ordering-explicit-data-deps", cl::Hidden,
  cl::desc("CSA-specific: Explicitly enforce data dependencies during memop "
           "ordering."),
  cl::init(false)};

static cl::opt<bool> GateParams{
  "csa-memop-ordering-gate-params", cl::Hidden,
  cl::desc("CSA-specific: Gate parameters on the input ordering signal to "
           "implicitly order more memops with the function entry."),
  cl::init(false)};

static cl::opt<bool> ViewMemopCFG{"csa-view-memop-cfg", cl::Hidden,
                                  cl::desc("CSA-specific: view memop CFG"),
                                  cl::init(false)};

static cl::opt<bool> ViewPreOrderingMemopCFG{
  "csa-view-pre-ordering-memop-cfg", cl::Hidden,
  cl::desc("CSA-specific: view memop CFG before memory ordering, to help debug "
           "bad section intrinsics"),
  cl::init(false)};

static cl::opt<bool> ViewDepMemopCFG{
  "csa-view-dep-memop-cfg", cl::Hidden,
  cl::desc("CSA-specific: view memop CFG after dependency calculation"),
  cl::init(false)};

static cl::opt<bool> DumpMemopCFG{"csa-dump-memop-cfg", cl::Hidden,
                                  cl::desc("CSA-specific: dump memop CFG"),
                                  cl::init(false)};

// A guess at the number of predecessors per basic block.
constexpr unsigned PRED_COUNT = 2;

// A guess at the number of successors per basic block.
constexpr unsigned SUCC_COUNT = 2;

// A guess at the number of inputs per merge.
constexpr unsigned INPUTS_PER_MERGE = 4;

// A guess at the typical number of live dependencies.
constexpr unsigned LIVE_DEP_COUNT = 8;

// A guess at the number of orderable memory operations per basic block.
constexpr unsigned MEMOP_COUNT = 4;

// A guess at the number of parallel section intrinsics per basic block.
constexpr unsigned SECTION_INTRINSIC_COUNT = 1;

// A guess at the number of merges per MemopCFG node.
constexpr unsigned MERGE_COUNT = 4;

// A guess at the number of phi nodes per MemopCFG node.
constexpr unsigned PHI_COUNT = 4;

// A guess at the number of MemopCFG nodes per function.
constexpr unsigned NODES_PER_FUNCTION = 16;

// A guess at the number of distinct regions in a function.
constexpr unsigned REGIONS_PER_FUNCTION = 2;

// A guess at the number of loops per function.
constexpr unsigned LOOPS_PER_FUNCTION = 8;

// A guess at the loop depth of loops.
constexpr unsigned LOOP_DEPTH = 3;

// A guess at the number of MemopCFG nodes per loop.
constexpr unsigned NODES_PER_LOOP = 4;

// A guess at the number of explicit memory dependencies per memop.
constexpr unsigned EXP_DEPS_PER_MEMOP = 4;

// A guess at the number of implicit dependencies per memop.
constexpr unsigned IMP_DEPS_PER_MEMOP = 4;

// A guess at the number of phibits per MemopCFG node.
constexpr unsigned PHIBITS_PER_NODE = 8;

// A guess at the number of loop-local storage allocation pools per
// function.
constexpr unsigned POOLS_PER_FUNCTION = 2;

#define DEBUG_TYPE "csa-memop-ordering"
constexpr auto PASS_DESC = "CSA: Memory operation ordering";

// Memory ordering statistics.
STATISTIC(AliasQueryCount, "Number of queries made to alias analysis");

namespace {

// A type which represents a copy of the CFG with non-memory/non-intrinsic
// operations stripped and with extra bookkeeping fields in each node to
// aid in ordering chain construction.
struct MemopCFG {

  struct Node;
  struct Loop;

  // A type to represent dependencies, in the form of direct memops, phibits, or
  // eventually other marker elements.
  struct Dep {

    // The node that the dependency is in.
    Node *node{nullptr};

    // Its type.
    enum Type { phibit, memop } type;

    // Its index in the correct vector in the node.
    int idx;

    // Some basic constructors.
    Dep() {}
    Dep(Node *node_in, Type type_in, int idx_in)
        : node{node_in}, type{type_in}, idx{idx_in} {}

    // Whether another dependency is implied by this one.
    bool implies(const Dep &) const;

    // Whether another dependency is implied by this one _and_ is different
    // from it.
    bool strictly_implies(const Dep &) const;
  };

  // A standard type for holding dependencies.
  using DepVec = SmallVector<Dep, LIVE_DEP_COUNT>;

  // A type to represent ordering token values generated from phis, memops, and
  // merges (along with the special <none> value). Since all of those live in
  // (small)vectors in nodes that aren't going to be reordered, they can all be
  // identified by their node pointer and vector index.
  struct OrdToken {

    // The node that the chain element is in.
    Node *node{nullptr};

    // The type of element that this token value is produced by.
    enum Type { none, phi, memop, merge } type{none};

    // The index of the element in its vector.
    int idx;

    // Some basic constructors.
    OrdToken() {}
    OrdToken(Node *node_in, Type type_in, int idx_in)
        : node{node_in}, type{type_in}, idx{idx_in} {}

    // An explicit bool conversion for testing if this token is not <none>.
    explicit operator bool() const { return type; }
  };

  // A fragment of a phi node representing an dependency from only one
  // predecessor.
  struct PHIBit {

    // The predecessor that the dependency is coming from.
    Node *pred;

    // The dependency from that predecessor.
    Dep dep;

    // If this phibit or any phibit that it refers to goes across a loop
    // backedge, the Loop corresponding to that. Otherwise, nullptr. Note that
    // by construction a dependency can cross at most one backedge given the
    // model used for calculating them.
    const Loop *loop;
  };

  // A phi node for memory ordering tokens.
  struct PHI {

    // All of the inputs to the phi node. Their order corresponds to the order
    // of the predecessor list in the phi node's memop CFG node.
    SmallVector<OrdToken, PRED_COUNT> inputs;

    // The IR phi node created for this phi node.
    PHINode *I = nullptr;
  };

  // An n-ary merge of memory ordering tokens.
  struct Merge {

    // The inputs to this merge, in sorted order.
    SmallVector<OrdToken, INPUTS_PER_MERGE> inputs;

    // Must-merge and may-merge sets for determining which merges this one can
    // be combined with without introducing any extra memory ordering
    // constraints.
    DepVec must_merge, may_merge;

    // The latest possible location that this merge can be put relative to the
    // memops. If it doesn't need to go before any of the memops in its node,
    // this will be the number of memops in the node.
    int memop_idx;

    // The IR all0 intrinsic created for this merge.
    Value *All0 = nullptr;
  };

  // A parallel region/section intrinsic.
  struct RegSecIntrinsic {

    // The index of the first memop after this intrinsic (or the number of
    // memops in the node if there isn't one).
    int memop_idx;

    // The (normalized) id of the region that this section intrinsic belongs
    // to.
    int region;

    // The type of this intrinsic.
    enum Type { region_entry, region_exit, section_entry, section_exit } type;

    // A constructor from a intrinsic instruction.
    RegSecIntrinsic(int memop_idx, const IntrinsicInst *II,
                    std::map<int, int> &normalized_regions);
  };

  // A holder for all of the relevant information pertaining to a single memop.
  struct Memop {

    // The node containing this memop; will not be nullptr.
    Node *parent;

    // The original instruction; also will not be nullptr.
    Instruction *I;

    // The token for the ready signal for this memory operation, or <none> if
    // the ordering chain for it hasn't been built yet.
    OrdToken ready;

    // This memop's outord value.
    Value *outord = nullptr;

    // This memop's explicit and implicit dependencies.
    DepVec exp_deps, imp_deps;

    // This memop's AtomicOrdering and volatility, pre-calculated as the memop
    // is loaded.
    AtomicOrdering AOrdering;
    bool Volatile;

    // This memop's loop-local allocation pool numbers. If this memop is a
    // csa.pipeline.depth.token.* call, this will have exactly one entry which
    // is the allocation pool number. Otherwise, it could have any number of
    // entries each corresponding to a different allocation pool that the memop
    // might alias with. These are generally sorted and unique'd.
    SmallVector<int, POOLS_PER_FUNCTION> Pools;

    // If this memop is a prefetch, PrefLoop will be set to its deepest
    // containing loop which also contains a relevent memop. If this memop is
    // not a prefetch or there is no such loop, pref_loop will be nullptr.
    const Loop *PrefLoop = nullptr;

    // Construction from an original IR instruction.
    Memop(Node *parent, Instruction *);

    // Determines the <rw> parameter for a prefetch: 0 for read prefetches, 1
    // for write prefetches.
    int prefetch_rw() const;

    // Whether this memop would be eligible for ordering with a read (rw = 0) or
    // write (rw = 1) prefetch.
    bool orders_with_prefetch(int RW) const;
  };

  // A functor type for computing the non-parallel region/section component of
  // the "requires ordering with" relation.
  class RequireOrdering {

    AAResults *AA;

  public:
    RequireOrdering() {}

    // The constructor. The functor needs access to the alias information, so
    // that's supplied here.
    RequireOrdering(AAResults *);

    // Determines whether A needs to be ordered before B. In general,
    // this is the case if they alias and at least one modifies memory. The
    // Looped parameter indicates whether a special arbitrary size query should
    // be done to account for the crossing of a loop backedge.
    bool operator()(const Memop &A, const Memop &B, bool Looped) const;
  };

  // A collection of parallel section states.
  struct SectionStates {

    // The state type.
    enum class State {
      no_intrinsics_encountered,
      not_in_section,
      outside_of_section,
      crossed_sections,
      left_region
    };

    // An array of those states, one for each region in the function. The
    // regions are identified by their normalized region ids that are assigned
    // when the function is being loaded, so those can just be used directly
    // as indices when accessing this array.
    SmallVector<State, REGIONS_PER_FUNCTION> states;

    // Whether memory operations should be ignored with the current values in
    // this state set.
    bool should_ignore_memops() const;

    // Transitions the state set given a parallel region/section intrinsic. If
    // this is an invalid transition, false will be returned.
    bool transition(const RegSecIntrinsic &);

    // Whether this parallel section state is incompatible with another one,
    // indicating an incorrect use of parallel section intrinsics.
    bool incompatible_with(const SectionStates &) const;

    // Merges the given state set with this one to make this one more specific.
    // If they are incompatible, this will return false.
    bool merge_from(const SectionStates &);
  };

  // A Node in the MemopCFG, corresponding to a MachineBasicBlock in the
  // MachineFunction CFG.
  struct Node {

    // The original basic block.
    BasicBlock *BB;

    // Predecessors to this node.
    SmallVector<Node *, PRED_COUNT> preds;

    // Successors to this node.
    SmallVector<Node *, SUCC_COUNT> succs;

    // The immediate dominator of this node.
    Node *dominator;

    // If this node is in a loop, the deepest loop that it is in. Otherwise,
    // nullptr. Note that if this node is a loop header this will be the loop
    // it is a header for.
    Loop *deepest_loop{nullptr};

    // The index of this node according to the topological sort.
    int topo_num;

    // The memory operations in this node. These are in the order in which
    // they appear in the original basic block.
    SmallVector<Memop, MEMOP_COUNT> memops;

    // The parallel region/section intrinsics. These are also in basic block
    // order and have a field to indicate where they are relative to the memops.
    SmallVector<RegSecIntrinsic, SECTION_INTRINSIC_COUNT> regsec_intrinsics;

    // The sets of merges and phis belonging to this node.
    SmallVector<Merge, MERGE_COUNT> merges;
    SmallVector<PHI, PHI_COUNT> phis;

    // The set of phibits for the node.
    SmallVector<PHIBit, PHIBITS_PER_NODE> phibits;

    // The set of active chain tips as of the last processing of this block.
    // These do not need to be sorted.
    DepVec chaintips;

    // The collection of sets used to calculate dependencies for each memop.
    struct DepSetEntry {

      // Whether this is the first successor updating this DepSetEntry. If it
      // is, connected and section_states are set directly rather than updated.
      bool first_succ = true;

      // A priority queue containing the union of disconnected (possible)
      // dependencies for the current memop at the end of this node as
      // calculated by successor nodes. This is emptied by step_backwards.
      std::priority_queue<Dep> disconnected;

      // The (intersected) set of connected dependencies for the current memop
      // at the end of this node as calculated by successor nodes. This is also
      // emptied by step_backwards and should always be in sorted order.
      DepVec connected;

      // The set of disconnected operations that should be connected inside of
      // this node. This should generally be in sorted order.
      DepVec to_connect;

      // The sets of connected dependencies for each predecessor, used to avoid
      // unnecessary edges. Each of these should be in sorted order. This is
      // cleared by step_forwards.
      SmallVector<DepVec, PRED_COUNT> pred_connected;

      // The section states as of the end of this node. This is set by the
      // successors.
      SectionStates section_states;

      // Resets this DepSetEntry to cut down on unnecessary memory allocation.
      void clear();
    };

    // An array of depset entries, one for each loop level. Use get_depset to
    // access this.
    SmallVector<DepSetEntry, LOOP_DEPTH> depsets;

    // The RequireOrdering functor to use when adding operations to merges in
    // this node.
    const RequireOrdering &require_ordering;

    // A topological ordering for Node*s.
    struct topo_order {
      bool operator()(const Node *a, const Node *b) {
        return a->topo_num < b->topo_num;
      }
    };

    // Construction from a source basic block. This won't fill in the
    // predecessors/successors, but those will get taken care of later in
    // MemopCFG::load. If use_parallel_sections is true, parallel section
    // intrinsics will be loaded into the node along with memory operations;
    // otherwise, they will be ignored. If use_parallel_sections is set,
    // normalized_regions is a map from original region ids to normalized ones
    // that is filled out and used as regions are encountered from parallel
    // sections intrinsics.
    Node(BasicBlock *, int topo_num, const RequireOrdering &,
         const std::function<bool(Instruction &)> &needs_ordering_edges,
         bool use_parallel_sections, std::map<int, int> &normalized_regions);

    // Whether this node is strictly dominated by another node.
    bool dominated_by(const Node *) const;

    // Whether this node is non-strictly dominated by another one.
    bool nonstrictly_dominated_by(const Node *) const;

    // The lowest common dominator of this node and another one.
    const Node *lowest_common_dominator(const Node *) const;

    // Obtains the depset entry corresponding to back_loop. Since all of those
    // allocated ahead of time, there's no risk of invalidating the reference
    // returned by this function.
    DepSetEntry &get_depset(const Loop *back_loop = nullptr);

    // Produces a (possibly new) phibit given a predecessor and a dependency
    // from it.
    Dep get_phibit(Node *pred, const Dep &);

    // Produces a (possibly new) phi given a phi to base it off of.
    OrdToken get_phi(PHI &&);

    // Produces a (possibly new) merge given a merge to base it off of.
    OrdToken get_merge(Merge &&);

    // Whether the predecessor pred should be ignored given the current status
    // of the iteration (ordering a memop in memop_node in the loop loop
    // traversed backwards into back_loop). If this predecessor should not be
    // ignored, pred_back_loop will be set to the correct back_loop value for
    // this predecessor. If pref_loop is set, all predecessors outside of
    // pref_loop will be ignored.
    bool ignore_pred(const Node *pred, const Node *memop_node, const Loop *loop,
                     const Loop *back_loop, const Loop *pref_loop,
                     const Loop *&pred_back_loop) const;

    // Whether a given dependency is a phibit that can be ignored in this node
    // because it goes across a loop backedge for a loop that this node is not
    // in.
    bool irrelevant_loop_phibit(const Dep &) const;

    // Processes memops in this node to update their dependencies and updates
    // chaintips. Returns false if dependency calculation fails due to bad
    // section intrinsics.
    bool calculate_deps(const Loop *);

    // Updates predecessors' DepSetEntries based on the current DepSetEntry for
    // this node. cur_memop is the memop currently being ordered, loop is the
    // loop that that memop is currently being ordered in, and back_loop is the
    // loop that the backwards traversal is currently in if it had crossed a
    // backedge. Returns false if bad section intrinsics are encountered.
    // Section intrinsic processing will start at memop_idx and continue
    // backwards from there.
    bool step_backwards(const Memop &cur_memop, const Loop *loop,
                        const Loop *back_loop, int memop_idx);

    // Filters predecessor values and adds them to to_connect in this node's
    // current DepSetEntry.
    void step_forwards(const Node *memop_node, const Loop *loop,
                       const Loop *back_loop, const Loop *pref_loop);

    // Constructs merges and phi nodes to order each memop according to the
    // dependencies calculated for it.
    void construct_chains();

    // Inserts merges and phi nodes as needed to satisfy the dependencies in
    // must before the memop at memop_idx in this node (or at the end if
    // memop_idx is memops.size()). Dependencies in may are also allowed to be
    // merged in if it helps with CSE.
    OrdToken insert_merges_and_phis(DepVec &must, const DepVec &may,
                                    int memop_idx);
  };

  // A MemopCFG Loop and all related information.
  struct Loop {

    // The original IR Loop.
    const llvm::Loop *IRL;

    // The parent loop of this loop.
    const Loop *parent = nullptr;

    // The set of nodes in this loop, in topological order.
    SmallVector<Node *, NODES_PER_LOOP> nodes;

    // The loop depth of this loop.
    int depth;

    // The loop is marked Parallel by CSALowerParallelIntrinsics.
    bool IsParallel{false};

    // Whether the loop contains operations that a read or write prefetch could
    // be ordered with.
    bool RPrefEligible{false}, WPrefEligible{false};

    // Whether this loop contains a given node.
    bool contains(const Node *) const;

    // Collects loop-carried loop-carried dependencies of memop B on memop A
    // around this loop's backedge(s).
    void collect_loop_imp_deps(const Dep &A, const Dep &B) const;

  private:
    // Collects deps on the given memop at a node in the loop that do not cross
    // backedges. reach_cache keeps track of deps that were already computed to
    // avoid extra work.
    DepVec reachable_deps(Node *, const Dep &memop,
                          std::map<Node *, DepVec> &reach_cache) const;
  };

  // The collection of nodes in this MemopCFG. These will be topologically
  // (reverse post order) sorted.
  SmallVector<std::unique_ptr<Node>, NODES_PER_FUNCTION> nodes;

  // The set of loops in this MemopCFG, with subloops preceding their containing
  // loops. These aren't updated after the MemopCFG is loaded, so pointers into
  // this can safely be used during memory ordering.
  SmallVector<Loop, LOOPS_PER_FUNCTION> loops;

  // The total number of different parallel regions represented in the MemopCFG.
  unsigned region_count;

  // The RequireOrdering functor for computing "requires ordering with".
  RequireOrdering require_ordering;

  // A mapping from BasicBlock pointers to Node pointers to make it easier to
  // find nodes given original basic blocks.
  DenseMap<const BasicBlock *, Node *> nodes_for_bbs;

  // A mapping from Instruction pointers to Deps to make it easier to find
  // memops given original instructions.
  DenseMap<const Instruction *, Dep> memops_for_insts;

  // Loads the MemopCFG with memops from a given machine function using the
  // given analysis results. The graph should be empty when this is called, so
  // make sure clear gets called before this is called again to load a different
  // function. If use_parallel_sections is set, parallel section intrinsics will
  // be copied over into the MemopCFG; otherwise, they will be ignored.
  void load(Function &, AAResults *, const DominatorTree &, const LoopInfo &,
            ReversePostOrderTraversal<Function *> &,
            const std::function<bool(Instruction &)> &needs_ordering_edges,
            bool can_speculate, bool use_parallel_sections);

  // Unloads/erases the currently-loaded graph.
  void clear();

  // Constructs the ordering chains for all of the memory operations in the
  // MemopCFG. True is returned if this succeeds; otherwise, the MemopCFG should
  // be re-loaded without parallel section intrinsics and tried again.
  bool construct_chains();

  /// \brief Emit optimization remarks regarding pipelined/non-pipelined loops.
  void emit_opt_report(OptimizationRemarkEmitter &);

private:
  // Recursively collects loop information to build the loops array. This is
  // called inside of load.
  void collect_loops(const llvm::Loop *);

  // Marks loops that are eligible for prefetch ordering and marks prefetches
  // with their corresponding loops.
  void mark_prefetch_loops();

  // Calculates allocation pool numbers. Also called inside of load.
  void calculate_pool_numbers();

  // Recursively marks memop users of I as corresponding to a loop-local
  // allocation pool PN. VisitedPHIs keeps track of seen phi notes to avoid
  // infinite recursion. If IgnoreTTR is true, token_takes and token_returns
  // will be ignored.
  void markLoopLocalMemops(const Instruction *I, int PN, bool IgnoreTTR,
                           SmallPtrSetImpl<const PHINode *> &VisitedPHIs);

  // Determines implicit dependencies. Also called inside of load.
  void calculate_imp_deps(bool can_speculate);

  // Marks implied dependencies from token returns to token takes around
  // loops. Called as part of calculate_imp_deps.
  void markReturnTakeDeps();

  // Calculates self dependencies, appending any that it finds to memops'
  // imp_deps field. Also called as part of calculate_imp_deps.
  void calculate_self_deps();

  // Calculates data dependencies, appending any that it finds to memops'
  // imp_deps field. Also called as part of calculate_imp_deps.
  void calculate_data_deps();
};

} // namespace

namespace llvm {

/// The standard memop ordering pass for CSA.
class CSAMemopOrdering : public CSAMemopOrderingBase {
public:
  static char ID;
  CSAMemopOrdering(const CSATargetMachine *TMIn = nullptr)
      : CSAMemopOrderingBase{ID}, TM{TMIn} {}
  StringRef getPassName() const override;
  void getAnalysisUsage(AnalysisUsage &) const override;

protected:
  void order(Function &) override;

private:
  const CSATargetMachine *TM;

  /// Retrieves the outord value corresponding to a given OrdToken. PHIs and
  /// dominating memops should already be created/ordered when this is called;
  /// merges are created on demand.
  Value *getOutord(const MemopCFG::OrdToken &);

  /// Emits the memop ordering code in a MemopCFG to this function.
  void emit(MemopCFG &);
};

} // namespace llvm

namespace {

// Operator<s that provide unique orderings for dependencies and dependency
// token values.
bool operator<(const MemopCFG::Dep &a, const MemopCFG::Dep &b) {

  // Order topologically by owning node first. The chain construction code
  // heavily exploits this ordering when splitting up and delegating deps.
  if (a.node->topo_num < b.node->topo_num)
    return true;
  if (b.node->topo_num < a.node->topo_num)
    return false;

  // Then by type.
  if (a.type < b.type)
    return true;
  if (b.type < a.type)
    return false;

  // Then by index.
  return a.idx < b.idx;
}
bool operator<(const MemopCFG::OrdToken &a, const MemopCFG::OrdToken &b) {
  if (a.node->topo_num < b.node->topo_num)
    return true;
  if (b.node->topo_num < a.node->topo_num)
    return false;
  if (a.type < b.type)
    return true;
  if (b.type < a.type)
    return false;
  return a.idx < b.idx;
}

// Also, equality operators for both.
bool operator==(const MemopCFG::Dep &a, const MemopCFG::Dep &b) {
  return a.type == b.type and a.node == b.node and a.idx == b.idx;
}
bool operator!=(const MemopCFG::Dep &a, const MemopCFG::Dep &b) {
  return not(a == b);
}
bool operator==(const MemopCFG::OrdToken &a, const MemopCFG::OrdToken &b) {
  return a.type == b.type and (not a or (a.node == b.node and a.idx == b.idx));
}
bool operator!=(const MemopCFG::OrdToken &a, const MemopCFG::OrdToken &b) {
  return not(a == b);
}

// It's often useful to search for deps by node; this functor can be used for
// doing that with lower_bound and upper_bound.
struct comp_dep_by_node {
  bool operator()(const MemopCFG::Dep &dep, const MemopCFG::Node *node) const {
    return MemopCFG::Node::topo_order{}(dep.node, node);
  }
  bool operator()(const MemopCFG::Node *node, const MemopCFG::Dep &dep) const {
    return MemopCFG::Node::topo_order{}(node, dep.node);
  }
};

// -- Various output operators for diagnostics --

raw_ostream &operator<<(raw_ostream &out, const MemopCFG::Dep &dep) {
  using Type = MemopCFG::Dep::Type;
  out << dep.node->topo_num;
  switch (dep.type) {
  case Type::phibit: {
    const MemopCFG::PHIBit &phibit = dep.node->phibits[dep.idx];
    return out << ":" << phibit.pred->topo_num << ">" << phibit.dep;
  }
  case Type::memop:
    return out << "o" << dep.idx;
  }
  llvm_unreachable("expected valid Dep type");
}

raw_ostream &operator<<(raw_ostream &out, const MemopCFG::OrdToken &val) {
  using Type = MemopCFG::OrdToken::Type;
  if (not val)
    return out << "<none>";
  out << val.node->topo_num;
  switch (val.type) {
  case Type::phi:
    out << "p";
    break;
  case Type::memop:
    out << "o";
    break;
  case Type::merge:
    out << "m";
    break;
  default:
    break;
  }
  return out << val.idx;
}

raw_ostream &operator<<(raw_ostream &out, const MemopCFG::Memop &memop) {
  assert(memop.I);
  out << memop.I->getOpcodeName();
  if (memop.AOrdering != AtomicOrdering::NotAtomic)
    out << " atomic";
  if (memop.Volatile)
    out << " volatile";
  if (const auto LdI = dyn_cast<LoadInst>(memop.I)) {
    out << " ";
    LdI->getPointerOperand()->printAsOperand(out);
  } else if (const auto StI = dyn_cast<StoreInst>(memop.I)) {
    out << " ";
    StI->getPointerOperand()->printAsOperand(out);
  } else if (const auto XI = dyn_cast<AtomicCmpXchgInst>(memop.I)) {
    out << " ";
    XI->getPointerOperand()->printAsOperand(out);
  } else if (const auto RI = dyn_cast<AtomicRMWInst>(memop.I)) {
    out << " ";
    RI->getPointerOperand()->printAsOperand(out);
  } else if (const auto CallI = dyn_cast<CallInst>(memop.I)) {
    out << " @" << CallI->getCalledFunction()->getName();
  }
  if (memop.AOrdering != AtomicOrdering::NotAtomic) {
    out << " " << toIRString(memop.AOrdering);
  }
  if (memop.exp_deps.empty())
    out << " " << memop.ready;
  else {
    bool first = true;
    for (const MemopCFG::Dep &dep : memop.exp_deps) {
      out << (first ? " [" : ", ") << dep;
      first = false;
    }
    out << "]";
  }
  if (not memop.imp_deps.empty()) {
    bool first = true;
    for (const MemopCFG::Dep &dep : memop.imp_deps) {
      out << (first ? " (" : ", ") << dep;
      first = false;
    }
    out << ")";
  }
  if (not memop.Pools.empty()) {
    out << " (pools";
    for (int Pool : memop.Pools)
      out << " " << Pool;
    out << ")";
  }
  return out;
}

raw_ostream &operator<<(raw_ostream &out, const MemopCFG::Merge &merge) {
  using namespace std;
  out << "merge";
  bool first = true;
  for (const MemopCFG::OrdToken &merged : merge.inputs) {
    out << (first ? " " : ", ") << merged;
    first = false;
  }
  if (not merge.may_merge.empty()) {
    MemopCFG::DepVec may_diff;
    may_diff.reserve(merge.may_merge.size() - merge.must_merge.size());
    set_difference(begin(merge.may_merge), end(merge.may_merge),
                   begin(merge.must_merge), end(merge.must_merge),
                   back_inserter(may_diff));
    first = true;
    out << " (";
    for (const MemopCFG::Dep &must_merged : merge.must_merge) {
      out << (first ? "" : ", ") << must_merged;
      first = false;
    }
    first = true;
    for (const MemopCFG::Dep &may_merged : may_diff) {
      out << (first ? " | " : ", ") << may_merged;
      first = false;
    }
    out << ")";
  }
  return out;
}

raw_ostream &operator<<(raw_ostream &out, const MemopCFG::PHI &phi) {
  out << "phi ";
  bool first = true;
  for (const MemopCFG::OrdToken &phid : phi.inputs) {
    out << (first ? "" : ", ") << phid;
    first = false;
  }
  return out;
}

raw_ostream &operator<<(raw_ostream &out,
                        const MemopCFG::RegSecIntrinsic &intr) {
  using Type = MemopCFG::RegSecIntrinsic::Type;
  switch (intr.type) {
  case Type::region_entry:
    return out << "region entry " << intr.region;
  case Type::region_exit:
    return out << "region exit " << intr.region;
  case Type::section_entry:
    return out << "section entry " << intr.region;
  case Type::section_exit:
    return out << "section exit " << intr.region;
  default:
    llvm_unreachable("Bad region/section intrinsic type");
  }
}

// Writes the body of node to out, starting each line with line_start and ending
// each line with line_end. This is broken out into a separate function so that
// it can be shared between operator<< and the DOTGraphTraits specialization.
void print_body(raw_ostream &out, const MemopCFG::Node &node,
                const char *line_start, const char *line_end) {
  using OrdToken = MemopCFG::OrdToken;

  // A nasty const_cast to be able to reuse OrdToken's operator<<.
  MemopCFG::Node *const node_ptr = const_cast<MemopCFG::Node *>(&node);

  if (not node.phibits.empty()) {
    out << "(" << node.phibits.size() << " phibits)" << line_end;
  }
  for (int phi_idx = 0; phi_idx != int(node.phis.size()); ++phi_idx) {
    out << line_start << OrdToken{node_ptr, OrdToken::phi, phi_idx} << " = "
        << node.phis[phi_idx] << line_end;
  }
  auto cur_intr = node.regsec_intrinsics.begin();
  for (int memop_idx = 0; memop_idx <= int(node.memops.size()); ++memop_idx) {
    while (cur_intr != node.regsec_intrinsics.end() and
           cur_intr->memop_idx == memop_idx)
      out << line_start << *cur_intr++ << line_end;
    for (int merge_idx = 0; merge_idx != int(node.merges.size()); ++merge_idx) {
      if (node.merges[merge_idx].memop_idx == memop_idx) {
        out << line_start << OrdToken{node_ptr, OrdToken::merge, merge_idx}
            << " = " << node.merges[merge_idx] << line_end;
      }
    }
    if (memop_idx != int(node.memops.size())) {
      out << line_start << OrdToken{node_ptr, OrdToken::memop, memop_idx}
          << " = " << node.memops[memop_idx] << line_end;
    }
  }
}

raw_ostream &operator<<(raw_ostream &out, const MemopCFG::Node &node) {
  using Node = MemopCFG::Node;
  out << node.topo_num << " (" << node.BB->getName() << "):\npreds:";
  for (const Node *const pred : node.preds)
    out << " " << pred->topo_num;
  out << "\n";
  print_body(out, node, "  ", "\n");
  out << "succs:";
  for (const Node *const succ : node.succs)
    out << " " << succ->topo_num;
  return out << "\n";
}

#ifndef NDEBUG
raw_ostream &operator<<(raw_ostream &out, const MemopCFG::Loop &loop) {
  if (loop.nodes.empty())
    return out << "-";
  return out << loop.nodes.front()->topo_num << "-"
             << loop.nodes.back()->topo_num;
}
#endif

raw_ostream &operator<<(raw_ostream &out, const MemopCFG &cfg) {
  if (not cfg.nodes.empty()) {
    out << cfg.nodes.front()->BB->getParent()->getName() << ":\n\n";
  }
  for (const std::unique_ptr<MemopCFG::Node> &node : cfg.nodes) {
    out << *node << "\n";
  }
  return out;
}

// A simple iterator adaptor which basically forwards all of its operations to
// its base type except that it calls base->get() to dereference instead of just
// doing *base. This allows a range<unique_ptr<T>> to be exposed as a range<T*>.
template <typename BaseIt> class GetAdaptorIterator {
  BaseIt base;

public:
  GetAdaptorIterator(const BaseIt &base_in) : base{base_in} {}
  decltype(base->get()) operator*() const { return base->get(); }
  decltype(base->get()) operator->() const { return base->get(); }
  GetAdaptorIterator &operator++() {
    ++base;
    return *this;
  }
  friend bool operator==(const GetAdaptorIterator &a,
                         const GetAdaptorIterator &b) {
    return a.base == b.base;
  }
  friend bool operator!=(const GetAdaptorIterator &a,
                         const GetAdaptorIterator &b) {
    return a.base != b.base;
  }
};

} // namespace

namespace llvm {

// A specialization of llvm::GraphTraits for MemopCFG so that LLVM knows how to
// traverse a MemopCFG.
template <> struct GraphTraits<const MemopCFG> {
  using NodeRef           = MemopCFG::Node *;
  using ChildIteratorType = decltype(MemopCFG::Node::succs)::const_iterator;
  static NodeRef getEntryNode(const MemopCFG &mopcfg) {
    return mopcfg.nodes.empty() ? nullptr : mopcfg.nodes.front().get();
  }
  static ChildIteratorType child_begin(NodeRef N) { return N->succs.begin(); }
  static ChildIteratorType child_end(NodeRef N) { return N->succs.end(); }

  using nodes_iterator =
    GetAdaptorIterator<decltype(MemopCFG::nodes)::const_iterator>;

  static nodes_iterator nodes_begin(const MemopCFG &mopcfg) {
    return mopcfg.nodes.begin();
  }
  static nodes_iterator nodes_end(const MemopCFG &mopcfg) {
    return mopcfg.nodes.end();
  }

  static unsigned size(const MemopCFG &mopcfg) { return mopcfg.nodes.size(); }
};
template <> struct GraphTraits<MemopCFG> : GraphTraits<const MemopCFG> {};

// Another specialization for llvm::DOTGraphTraits to tell LLVM how to write a
// MemopCFG as a dot graph.
template <> struct DOTGraphTraits<const MemopCFG> : DefaultDOTGraphTraits {

  using DefaultDOTGraphTraits::DefaultDOTGraphTraits;

  // The title of the graph as a whole.
  static std::string getGraphName(const MemopCFG &mopcfg) {
    if (mopcfg.nodes.empty())
      return "";
    return "MemopCFG for '" +
           mopcfg.nodes.front()->BB->getParent()->getName().str() +
           "' function";
  }

  // The label for each node with its number and corresponding IR name.
  static std::string getNodeLabel(const MemopCFG::Node *node,
                                  const MemopCFG &mopcfg) {
    using namespace std;
    return to_string(node->topo_num) + " (" + node->BB->getName().str() + ")";
  }

  // The description for each node with the code inside of it.
  static std::string getNodeDescription(const MemopCFG::Node *node,
                                        const MemopCFG &mopcfg) {
    using namespace std;
    string conts;
    raw_string_ostream sout{conts};
    print_body(sout, *node, "", "\\l");
    return sout.str();
  }

  // Output port labels with successor block numbers.
  template <typename EdgeIter>
  static std::string getEdgeSourceLabel(const MemopCFG::Node *node,
                                        EdgeIter it) {
    using namespace std;
    return to_string((*it)->topo_num);
  }

  // Input port labels with predecessor block numbers (in the correct order for
  // reading phis)
  static bool hasEdgeDestLabels() { return true; }
  static unsigned numEdgeDestLabels(const MemopCFG::Node *node) {
    return node->preds.size();
  }
  static std::string getEdgeDestLabel(const MemopCFG::Node *node,
                                      unsigned pred_idx) {
    using namespace std;
    return to_string(node->preds[pred_idx]->topo_num);
  }

  // This is a truly bizarre interface, but this does seem to be the correct way
  // to specify which input ports to map each output port to on its successor
  // node.
  template <typename EdgeIter>
  static bool edgeTargetsEdgeSource(const void *, EdgeIter) {
    return true;
  }
  template <typename EdgeIter>
  static EdgeIter getEdgeTarget(const MemopCFG::Node *node, EdgeIter it) {
    using namespace std;
    const auto found = find((*it)->preds, node);
    assert(found != end((*it)->preds));
    return next(begin((*it)->succs), distance(begin((*it)->preds), found));
  }
};
template <> struct DOTGraphTraits<MemopCFG> : DOTGraphTraits<const MemopCFG> {
  using DOTGraphTraits<const MemopCFG>::DOTGraphTraits;
};

} // namespace llvm

bool MemopCFG::Dep::implies(const Dep &that) const {

  // There are three rules for determining whether a Dep A implies a Dep B:
  //
  // 1. A and B are both the same memop.
  // 2. A and B are both phibits in the same node from the same predecessor and
  //    A->dep implies B->dep.
  // 3. B is a phibit and A implies something along B's path. For non-cyclic
  //    code, this means that B indicates a dependence on the same token as A
  //    but on a subset of A's paths. Around loops, this means that B indicates
  //    a dependence on a previous iteration's version of the A token, which
  //    implies B through a lic ordering implicit dependency.

  // Do a fast check to see if this and that are the same. If they are, this
  // implies that by rules (1) and (2).
  if (*this == that)
    return true;

  // If that isn't a phibit, it can't be implied by this if they aren't equal.
  if (that.type != phibit)
    return false;

  // Now that has been confirmed to be a phibit.
  const PHIBit &that_phibit = that.node->phibits[that.idx];

  // If this is also a phibit in the same node, check rule (2). In any case
  // where the deps are both phibits in the same node from the same predecessor
  // and rule (3) holds, rule (2) will also hold because this_phibit.dep must
  // also imply something along that_phibit.dep's path. Therefore if rule (2)
  // fails in these cases we skip the check for rule (3).
  if (type == phibit and node == that.node) {
    const PHIBit &this_phibit = node->phibits[idx];
    if (this_phibit.pred == that_phibit.pred)
      return this_phibit.dep.implies(that_phibit.dep);
  }

  // Otherwise, check rule (3).
  return implies(that_phibit.dep);
}

bool MemopCFG::Dep::strictly_implies(const Dep &that) const {
  return *this != that and implies(that);
}

// Recursively searches for an intrinsic def for V through phi nodes. PHI nodes
// in visited_phis are ignored to avoid infinite recursion.
static const IntrinsicInst *
find_intr_def(const Value *V, std::set<const PHINode *> &visited_phis) {

  // If the value is from an intrinsic, return it.
  if (const auto II = dyn_cast<IntrinsicInst>(V))
    return II;

  // If the value is from a phi node, try searching back through it.
  if (const auto PI = dyn_cast<PHINode>(V)) {
    if (visited_phis.count(PI))
      return nullptr;
    visited_phis.insert(PI);

    for (const Value *PV : PI->incoming_values()) {
      const IntrinsicInst *const PVI = find_intr_def(PV, visited_phis);
      if (PVI)
        return PVI;
    }
  }

  // Otherwise, return nullptr. Hopefully there's an intrinsic along a
  // different path.
  return nullptr;
}

// Determines the normalized region id for a given parallel region/section
// intrinsic.
static int find_normalized_region(const IntrinsicInst *II,
                                  std::map<int, int> &normalized_regions) {
  switch (II->getIntrinsicID()) {
  case Intrinsic::csa_parallel_region_entry: {
    if (not isa<ConstantInt>(II->getArgOperand(0)))
      report_fatal_error("Parallel intrinsic with non-constant region id");
    const int unnormalized_id =
      cast<ConstantInt>(II->getArgOperand(0))->getLimitedValue();

    const auto found = normalized_regions.lower_bound(unnormalized_id);
    if (found != normalized_regions.end() and found->first == unnormalized_id)
      return found->second;
    const int new_id = normalized_regions.size();
    normalized_regions.emplace_hint(found, unnormalized_id, new_id);
    return new_id;
  }

  case Intrinsic::csa_parallel_region_exit:
  case Intrinsic::csa_parallel_section_entry:
  case Intrinsic::csa_parallel_section_exit: {
    std::set<const PHINode *> visited_phis;
    const IntrinsicInst *const intr_def =
      find_intr_def(II->getArgOperand(0), visited_phis);
    if (not intr_def)
      report_fatal_error("Parallel intrinsic not connected to entry?");
    return find_normalized_region(intr_def, normalized_regions);
  }

  // No other kinds of intrinsics should be showing up here.
  default:
    LLVM_DEBUG(dbgs() << "Bad instruction is:" << *II << "\n");
    llvm_unreachable("Bad region/section markings");
  }
}

// Maps intrinsic ids to RegSecIntrinsic types.
static MemopCFG::RegSecIntrinsic::Type get_regsec_type(Intrinsic::ID id) {
  using RSI = MemopCFG::RegSecIntrinsic;
  switch (id) {
  case Intrinsic::csa_parallel_region_entry:
    return RSI::region_entry;
  case Intrinsic::csa_parallel_region_exit:
    return RSI::region_exit;
  case Intrinsic::csa_parallel_section_entry:
    return RSI::section_entry;
  case Intrinsic::csa_parallel_section_exit:
    return RSI::section_exit;
  default:
    break;
  }
  llvm_unreachable("Bad region/section intrinsic type");
}

MemopCFG::RegSecIntrinsic::RegSecIntrinsic(
  int memop_idx_in, const IntrinsicInst *II,
  std::map<int, int> &normalized_regions)
    : memop_idx{memop_idx_in}, region{find_normalized_region(
                                 II, normalized_regions)},
      type{get_regsec_type(II->getIntrinsicID())} {}

// Retrieves the atomic ordering of a given instruction, or NotAtomic if the
// instruction isn't atomic. This just uses the success ordering for cmpxchg
// because it is at least as strict as the failure ordering.
static AtomicOrdering getAtomicOrdering(const Instruction *I) {
  if (const auto LI = dyn_cast<LoadInst>(I))
    return LI->getOrdering();
  if (const auto SI = dyn_cast<StoreInst>(I))
    return SI->getOrdering();
  if (const auto CI = dyn_cast<AtomicCmpXchgInst>(I))
    return CI->getSuccessOrdering();
  if (const auto FI = dyn_cast<FenceInst>(I))
    return FI->getOrdering();
  if (const auto RI = dyn_cast<AtomicRMWInst>(I))
    return RI->getOrdering();

  // Function calls might also involve atomic operations; LLVM doesn't have any
  // special markings for this but it does avoid adding other markings that
  // could be used to optimize the calls. That's enough for calls to have the
  // right ordering (at least locally), so they don't need to be identified
  // here.

  return AtomicOrdering::NotAtomic;
}

// Determines whether the given instruction is volatile.
static bool isVolatile(const Instruction *I) {
  if (const auto LI = dyn_cast<LoadInst>(I))
    return LI->isVolatile();
  if (const auto SI = dyn_cast<StoreInst>(I))
    return SI->isVolatile();
  if (const auto CI = dyn_cast<AtomicCmpXchgInst>(I))
    return CI->isVolatile();
  if (const auto RI = dyn_cast<AtomicRMWInst>(I))
    return RI->isVolatile();

  // Calls can also involve volatile operations, but don't need special
  // treatment here for similar reasons to atomics.

  return false;
}

MemopCFG::Memop::Memop(Node *parent_in, Instruction *I_in)
    : parent{parent_in}, I{I_in}, AOrdering{getAtomicOrdering(I)},
      Volatile{isVolatile(I)} {
  assert(parent);
  assert(I);
}

int MemopCFG::Memop::prefetch_rw() const {
  const auto II = dyn_cast<IntrinsicInst>(I);
  assert(II and II->getIntrinsicID() == Intrinsic::prefetch);
  const auto RwOpnd = dyn_cast<ConstantInt>(II->getArgOperand(1));
  assert(RwOpnd && "Non-constant <rw> input to prefetch?");
  return RwOpnd->getLimitedValue();
}

bool MemopCFG::Memop::orders_with_prefetch(int RW) const {

  // Mementries aren't ordered with prefetches, because prefetch ordering is
  // always function-local. Prefetches also aren't ordered with other
  // prefetches.
  if (const auto II = dyn_cast<IntrinsicInst>(I))
    if (II->getIntrinsicID() == Intrinsic::csa_mementry or
        II->getIntrinsicID() == Intrinsic::prefetch)
      return false;

  // Otherwise, this operation is eligible for ordering with write prefetches if
  // it may read or write and with read prefetches if it only reads. Load
  // prefetches are also ordered with function calls that may read from memory
  // because they might contain relevant loads in some cases.
  if (RW == 1) {
    return I->mayReadOrWriteMemory();
  } else {
    return I->mayReadFromMemory() and
           (isa<CallInst>(I) or not I->mayWriteToMemory());
  }
}

MemopCFG::RequireOrdering::RequireOrdering(AAResults *AA_in) : AA{AA_in} {}

// Determines whether an instruction is a csa.pipeline.depth.token.* call.
static bool isDepthTokenCall(const IntrinsicInst *II) {
  if (not II)
    return false;
  switch (II->getIntrinsicID()) {
  case Intrinsic::csa_pipeline_depth_token_take:
  case Intrinsic::csa_pipeline_depth_token_return:
    return true;
  default:
    return false;
  }
}

// Determine if an instruction is the cache region begin intrinsic.
static bool isCacheRegionBegin(const IntrinsicInst *II) {
  if (not II)
    return false;

  return II->getIntrinsicID() == Intrinsic::csa_local_cache_region_begin;
}

// Determine if an instruction is the cache region end intrinsic.
static bool isCacheRegionEnd(const IntrinsicInst *II) {
  if (not II)
    return false;

  return II->getIntrinsicID() == Intrinsic::csa_local_cache_region_end;
}

// Determines whether a memop belongs to the same pool as a
// csa.pipeline.depth.token.* call.
static bool inSamePool(const MemopCFG::Memop &PM, const MemopCFG::Memop &M) {
  using std::end;
  assert(PM.Pools.size() == 1);
  return is_contained(M.Pools, PM.Pools.front());
}

// Determines MemoryLocations for prefetches, as the normal query doesn't
// return one for them.
static MemoryLocation getPrefetchMemoryLocation(const IntrinsicInst *II) {
  assert(II->getIntrinsicID() == Intrinsic::prefetch);
  AAMDNodes AATags;
  II->getAAMetadata(AATags);
  return MemoryLocation{II->getArgOperand(0), MemoryLocation::UnknownSize,
                        AATags};
}

bool MemopCFG::RequireOrdering::operator()(const Memop &A, const Memop &B,
                                           bool Looped) const {

  const IntrinsicInst *const AII = dyn_cast<IntrinsicInst>(A.I);
  const IntrinsicInst *const BII = dyn_cast<IntrinsicInst>(B.I);

  // Check for invalid queries. Mementries don't have an input edge and returns
  // don't have an output edge.
  assert(not(BII and BII->getIntrinsicID() == Intrinsic::csa_mementry) &&
         "Invalid query on mementry");
  assert(not isa<ReturnInst>(A.I) && "Invalid query on return");

  if (AII and AII->getIntrinsicID() == Intrinsic::trap or
      BII and BII->getIntrinsicID() == Intrinsic::trap)
    return true;

  // Prefetches have very special ordering rules: nothing gets ordered after
  // prefetches...
  if (AII and AII->getIntrinsicID() == Intrinsic::prefetch)
    return false;

  // ...and the set of operations that prefetches are ordered after is different
  // from normal memops.
  if (BII and BII->getIntrinsicID() == Intrinsic::prefetch) {

    // Check whether the other op is eligible for ordering with this prefetch.
    if (not A.orders_with_prefetch(B.prefetch_rw()))
      return false;

    // Prefetching local storage is sort of silly, but if someone does it the
    // prefetch should be ordered normally with those intrinsics.
    if (isDepthTokenCall(AII))
      return inSamePool(A, B);

    // Get a memory location for both ops, assuming aliasing if the other op
    // doesn't have one and blanking out its size field to perform an
    // arbitrary-size query.
    std::optional<MemoryLocation> AOML = MemoryLocation::getOrNone(A.I);
    if (not AOML)
      return true;
    MemoryLocation &AML      = AOML.getValue();
    AML.Size                 = MemoryLocation::UnknownSize;
    const MemoryLocation BML = getPrefetchMemoryLocation(BII);

    // The prefetch should be ordered if the query returns NoAlias.
    ++AliasQueryCount;
    return not AA->isNoAlias(AML, BML);
  }

  // Every operation has to be ordered after the function's mementry.
  if (AII and AII->getIntrinsicID() == Intrinsic::csa_mementry)
    return true;

  // Return instructions must be ordered after everything.
  if (isa<ReturnInst>(B.I))
    return true;

  // csa.pipeline.deth.token.* calls are only ordered with other memops in the
  // same pool.
  if (isDepthTokenCall(AII))
    return inSamePool(A, B);
  if (isDepthTokenCall(BII))
    return inSamePool(B, A);

  // A llvm.csa.local.cache.region.begin call is ordered with the writes
  // that come before it.
  if (isCacheRegionBegin(BII)) {
    return A.I->mayWriteToMemory();
  }

  const char *Key = CSA_LOCAL_CACHE_METADATA_KEY;

  // A read inside the region is ordered with csa.local.cache.region.begin.
  if (isCacheRegionBegin(AII)) {
    if (!B.I->mayReadFromMemory() || !B.I->hasMetadata(Key))
      return false;

    assert(A.I->hasMetadata(Key) && "No local cache ID assigned");

    return A.I->getMetadata(Key) == B.I->getMetadata(Key);
  }

  // A llvm.csa.local.cache.region.end call is ordered with the writes
  // in the region.
  if (isCacheRegionEnd(BII)) {
    if (!A.I->mayWriteToMemory() || !A.I->hasMetadata(Key))
      return false;

    assert(B.I->hasMetadata(Key) && "No local cache ID assigned");

    return A.I->getMetadata(Key) == B.I->getMetadata(Key);
  }

  // All memops after the region are ordered with the end of the region.
  if (isCacheRegionEnd(AII)) {
    return true;
  }

  // Order calls strictly with respect to everything else.
  // TODO: This is a stopgap to avoid triggering CMPLRLLVM-7634. Remove this
  // when we have a fix for that.
  if ((not AII and isa<CallInst>(A.I)) or (not BII and isa<CallInst>(B.I)))
    return true;

  // At this point, if either memop is marked as not touching memory (because it
  // is a call to a function which doesn't use non-local memory) it doesn't need
  // any further ordering.
  if (not A.I->mayReadOrWriteMemory() or not B.I->mayReadOrWriteMemory())
    return false;

  // All memops ahead of a release operation need to be ordered ahead of it.
  // TODO: These edges really need global ordering.
  if (isReleaseOrStronger(B.AOrdering))
    return true;

  // All memops following an acquire operation need to be ordered after it.
  // TODO: These edges really need global ordering.
  if (isAcquireOrStronger(A.AOrdering))
    return true;

  // If both operations are volatile, they need ordering. Non-volatile
  // operations don't generally need extra ordering with volatile ones though.
  // These edges do _not_ need to be global.
  if (A.Volatile and B.Volatile)
    return true;

  // If neither operation can write, they don't need ordering. However,
  // prefetches should be ordered with loads and so are treated like stores.
  if (not A.I->mayWriteToMemory() and not B.I->mayWriteToMemory())
    return false;

  // Figure out the memory locations for both instructions.
  std::optional<MemoryLocation> AOML = MemoryLocation::getOrNone(A.I);
  std::optional<MemoryLocation> BOML = MemoryLocation::getOrNone(B.I);

  // If either don't have one (as is the case with calls), assume strict
  // ordering. Otherwise, both are valid.
  if (not AOML or not BOML)
    return true;
  MemoryLocation &AML = AOML.getValue();
  MemoryLocation &BML = BOML.getValue();

  // If operating in Looped mode, use arbitrary-size queries.
  // TODO: This is still a hack and might not be correct with fancier alias
  // analysis. Replace this with better loop analysis.
  if (Looped)
    AML.Size = BML.Size = MemoryLocation::UnknownSize;

  // These two instructions need ordering if they are not NoAlias.
  ++AliasQueryCount;
  return not AA->isNoAlias(AML, BML);
}

bool MemopCFG::SectionStates::should_ignore_memops() const {
  using namespace std;

  // Memops should be ignored if any section states are crossed_sections.
  return any_of(begin(states), end(states),
                [](State s) { return s == State::crossed_sections; });
}

bool MemopCFG::SectionStates::transition(const RegSecIntrinsic &intr) {

  // Determine which state needs to update.
  State &to_update = states[intr.region];

  switch (to_update) {

  // For no_intrinsics_encountered, transition to not_in_section,
  // outside_of_section, or left_region depending on which intrinsic is
  // encountered first.
  case State::no_intrinsics_encountered:
    switch (intr.type) {
    case RegSecIntrinsic::region_entry:
      to_update = State::left_region;
      return true;
    case RegSecIntrinsic::region_exit:
    case RegSecIntrinsic::section_exit:
      to_update = State::not_in_section;
      return true;
    case RegSecIntrinsic::section_entry:
      to_update = State::outside_of_section;
      return true;
    }

  // For not_in_section or left_region, no further state transitions need to
  // be taken.
  case State::not_in_section:
  case State::left_region:
    return true;

  // For outside_of_section, section exits should transition to
  // crossed_sections, region entries should transition to outside_of_region,
  // and section entries and region exits should not be encountered.
  case State::outside_of_section:
    switch (intr.type) {
    case RegSecIntrinsic::section_exit:
      to_update = State::crossed_sections;
      return true;
    case RegSecIntrinsic::region_entry:
      to_update = State::left_region;
      return true;
    case RegSecIntrinsic::region_exit:
    case RegSecIntrinsic::section_entry:
      return false;
    }

  // For crossed_sections, section entries should transition to
  // outside_of_section and no other types of intrinsics should be
  // encountered.
  case State::crossed_sections:
    if (intr.type == RegSecIntrinsic::section_entry) {
      to_update = State::outside_of_section;
      return true;
    } else
      return false;
  }

  llvm_unreachable("Invalid section state?");
}

bool MemopCFG::SectionStates::incompatible_with(
  const SectionStates &that) const {
  using namespace std;

  // Look at each pair of states.
  for (int region = 0; region < int(states.size()); ++region) {
    State state_a = states[region];
    State state_b = that.states[region];

    // If they're the same, they clearly aren't incompatible.
    if (state_a == state_b)
      continue;

    // If they're different and neither is no_intrinsics_encountered, they're
    // incompatible.
    if (state_a != State::no_intrinsics_encountered and
        state_b != State::no_intrinsics_encountered)
      return true;

    // If one is no_intrinsics_encountered and the other is outside_of_section,
    // those are also incompatible.
    if (state_b == State::no_intrinsics_encountered)
      swap(state_a, state_b);
    if (state_b == State::outside_of_section)
      return true;

    // Otherwise, this combination is not incompatible.
  }

  return false;
}

bool MemopCFG::SectionStates::merge_from(const SectionStates &that) {
  if (incompatible_with(that))
    return false;

  // New states that are not no_intrinsics_encountered should supercede old
  // states.
  for (int region = 0; region != int(states.size()); ++region) {
    if (that.states[region] != State::no_intrinsics_encountered) {
      states[region] = that.states[region];
    }
  }

  return true;
}

void MemopCFG::Node::DepSetEntry::clear() {
  using namespace std;
  while (not disconnected.empty())
    disconnected.pop();
  connected.clear();
  first_succ = true;
  to_connect.clear();
  for (DepVec &pc : pred_connected)
    pc.clear();
  fill(begin(section_states.states), end(section_states.states),
       SectionStates::State::no_intrinsics_encountered);
}

MemopCFG::Node::Node(
  BasicBlock *BB_in, int topo_num_in,
  const RequireOrdering &require_ordering_in,
  const std::function<bool(Instruction &)> &needs_ordering_edges,
  bool use_parallel_sections, std::map<int, int> &normalized_regions)
    : BB{BB_in}, topo_num{topo_num_in}, require_ordering(require_ordering_in) {

  // The corresponding nodes for the other basic blocks won't be known yet until
  // they're all constructed, but at least we know how many there will be now.
  preds.reserve(pred_size(BB));
  succs.reserve(succ_size(BB));

  // Go through all of the instructions and find the ones that need to be added
  // to the MemopCFG.
  for (Instruction &I : *BB) {
    if (needs_ordering_edges(I)) {
      memops.emplace_back(this, &I);
    } else if (const auto II = dyn_cast<IntrinsicInst>(&I)) {

      // Mementry intrinsics should also be added as memops. They are their own
      // outord edges.
      if (II->getIntrinsicID() == Intrinsic::csa_mementry) {
        memops.emplace_back(this, II);
        memops.back().outord = II;
      }

      // If use_parallel_sections is set, add parallel region/section intrinsics
      // too.
      else if (use_parallel_sections and
               (II->getIntrinsicID() == Intrinsic::csa_parallel_region_entry or
                II->getIntrinsicID() == Intrinsic::csa_parallel_region_exit or
                II->getIntrinsicID() == Intrinsic::csa_parallel_section_entry or
                II->getIntrinsicID() == Intrinsic::csa_parallel_section_exit)) {
        regsec_intrinsics.emplace_back(memops.size(), II, normalized_regions);
      }
    }
  }
}

bool MemopCFG::Node::dominated_by(const Node *possi_dom) const {
  const Node *cur_node = dominator;
  while (cur_node and cur_node != possi_dom)
    cur_node = cur_node->dominator;
  return cur_node;
}

bool MemopCFG::Node::nonstrictly_dominated_by(const Node *possi_dom) const {
  return this == possi_dom or dominated_by(possi_dom);
}

// A helper function to determine the "dominator depth" of a node, which is
// equivalent to the number of nodes that (non-strictly) dominate it.
static int dom_depth(const MemopCFG::Node *node) {
  int dom_depth = 0;
  while (node)
    node = node->dominator, ++dom_depth;
  return dom_depth;
}

const MemopCFG::Node *
MemopCFG::Node::lowest_common_dominator(const Node *that) const {
  int depth_diff           = dom_depth(this) - dom_depth(that);
  const Node *this_matched = this, *that_matched = that;
  if (depth_diff > 0) {
    while (depth_diff--)
      this_matched = this_matched->dominator;
  } else {
    while (depth_diff++)
      that_matched = that_matched->dominator;
  }
  while (this_matched != that_matched) {
    this_matched = this_matched->dominator;
    that_matched = that_matched->dominator;
  }
  return this_matched;
}

MemopCFG::Node::DepSetEntry &MemopCFG::Node::get_depset(const Loop *back_loop) {
  return depsets[back_loop ? back_loop->depth : 0];
}

MemopCFG::Dep MemopCFG::Node::get_phibit(Node *pred, const Dep &dep) {

  // Look for an existing phibit.
  for (int idx = 0; idx != int(phibits.size()); ++idx) {
    if (phibits[idx].pred == pred and phibits[idx].dep == dep) {
      return {this, Dep::phibit, idx};
    }
  }

  // If there isn't one, make a new one.
  const int idx    = phibits.size();
  const Loop *loop = pred->topo_num < topo_num ? nullptr : deepest_loop;
  if (dep.type == Dep::phibit and dep.node->phibits[dep.idx].loop) {
    assert(not loop);
    loop = dep.node->phibits[dep.idx].loop;
  }
  phibits.push_back({pred, dep, loop});
  return {this, Dep::phibit, idx};
}

MemopCFG::OrdToken MemopCFG::Node::get_phi(PHI &&phi) {
  using namespace std;

  // Look through to see how many unique non-none inputs there are to this phi.
  OrdToken first_set_phi_input;
  bool multiple_set_phi_inputs = false;
  for (const OrdToken &phid : phi.inputs)
    if (phid) {
      if (not first_set_phi_input)
        first_set_phi_input = phid;
      else if (phid != first_set_phi_input)
        multiple_set_phi_inputs = true;
    }

  // If there aren't actually any non-none phi inputs, <none> can just be used
  // directly.
  if (not first_set_phi_input)
    return {};

  // Otherwise, if there's just one it might be possible to use that directly.
  // The phi node has to be dominated by that value.
  if (not multiple_set_phi_inputs and dominated_by(first_set_phi_input.node)) {
    return first_set_phi_input;
  }

  // Otherwise, try to find a matching phi.
  for (int phi_idx = 0; phi_idx != int(phis.size()); ++phi_idx) {
    if (phis[phi_idx].inputs == phi.inputs) {
      return {this, OrdToken::phi, phi_idx};
    }
  }

  // Otherwise, make a new phi.
  const int phi_idx = phis.size();
  phis.push_back(move(phi));
  return {this, OrdToken::phi, phi_idx};
}

MemopCFG::OrdToken MemopCFG::Node::get_merge(Merge &&merge) {
  using namespace std;

  // If there's no inputs, the value should be <none>.
  if (merge.inputs.empty())
    return {};

  // If there's only one, use that directly.
  if (merge.inputs.size() == 1)
    return merge.inputs.front();

  // Otherwise, try to find a matching merge. Look all of the way up the
  // dominator tree in case there's a matching one in a dominator too.
  for (Node *cur_dom = this; cur_dom; cur_dom = cur_dom->dominator) {
    for (int merge_idx = 0; merge_idx != int(cur_dom->merges.size());
         ++merge_idx) {
      Merge &cand = cur_dom->merges[merge_idx];

      // If there's a may_merge set, matches are valid if each merge's
      // must_merge set is a subset of the other's may_merge set.
      if (not merge.may_merge.empty()) {
        if (not includes(begin(merge.may_merge), end(merge.may_merge),
                         begin(cand.must_merge), end(cand.must_merge)))
          continue;
        if (not includes(begin(cand.may_merge), end(cand.may_merge),
                         begin(merge.must_merge), end(merge.must_merge)))
          continue;
      }

      // Otherwise, the inputs must match exactly.
      else {
        if (cand.inputs != merge.inputs)
          continue;
      }

      // If the merges are compatible and matched by must/may merge sets, the
      // combined one must have inputs and must_merge sets which are the unions
      // of the originals and a may_merge set that is the intersection of the
      // originals. Otherwise, the input sets already match so no further work
      // is needed.
      if (not merge.may_merge.empty()) {
        decltype(cand.inputs) new_inputs;
        new_inputs.reserve(max(cand.inputs.size(), merge.inputs.size()));
        set_union(begin(cand.inputs), end(cand.inputs), begin(merge.inputs),
                  end(merge.inputs), back_inserter(new_inputs));
        cand.inputs = move(new_inputs);

        decltype(cand.must_merge) new_must_merge;
        new_must_merge.reserve(
          max(cand.must_merge.size(), merge.must_merge.size()));
        set_union(begin(cand.must_merge), end(cand.must_merge),
                  begin(merge.must_merge), end(merge.must_merge),
                  back_inserter(new_must_merge));
        cand.must_merge = move(new_must_merge);

        decltype(cand.may_merge) new_may_merge;
        new_may_merge.reserve(
          min(cand.may_merge.size(), merge.may_merge.size()));
        set_intersection(begin(cand.may_merge), end(cand.may_merge),
                         begin(merge.may_merge), end(merge.may_merge),
                         back_inserter(new_may_merge));
        cand.may_merge = move(new_may_merge);
      }

      // Also move the matched merge earlier if needed.
      if (cur_dom == this and cand.memop_idx > merge.memop_idx) {
        cand.memop_idx = merge.memop_idx;
      }

      return {cur_dom, OrdToken::merge, merge_idx};
    }
  }

  // Otherwise, make a new merge.
  const int merge_idx = merges.size();
  merges.push_back(move(merge));
  return {this, OrdToken::merge, merge_idx};
}

bool MemopCFG::Node::ignore_pred(const Node *pred, const Node *memop_node,
                                 const Loop *loop, const Loop *back_loop,
                                 const Loop *pref_loop,
                                 const Loop *&pred_loop) const {

  // If this predecessor is outside of loop or back_loop or pref_loop, it should
  // be ignored.
  if (back_loop and not back_loop->contains(pred))
    return true;
  if (loop and not loop->contains(pred))
    return true;
  if (pref_loop and not pref_loop->contains(pred))
    return true;

  // If this predecessor is a backedge...
  if (pred->topo_num >= topo_num) {

    // If a backedge has already been traversed, this one should be ignored.
    if (back_loop)
      return true;

    // Since this is a backedge, we would really hope that we're in a loop.
    assert(deepest_loop);

    // Only consider backedges that are exactly one loop level deeper than the
    // current loop depth.
    const int loop_depth = loop ? loop->depth : 0;
    if (deepest_loop->depth != loop_depth + 1)
      return true;

    // And ones where the loop actually contains the memop.
    if (not deepest_loop->contains(memop_node))
      return true;

    // If this predecessor isn't ignored, it should use the header_for loop for
    // back_loop.
    pred_loop = deepest_loop;
  }

  // Otherwise, just propagate back_loop.
  else
    pred_loop = back_loop;

  return false;
}

bool MemopCFG::Node::irrelevant_loop_phibit(const Dep &dep) const {
  return dep.type == Dep::phibit and dep.node->phibits[dep.idx].loop and
         not dep.node->phibits[dep.idx].loop->contains(this);
}

bool MemopCFG::Node::calculate_deps(const Loop *loop) {
  using namespace std;
  using namespace std::placeholders;

  // A node and the back_loop value it was visited with.
  struct NodeLoop {
    Node *node;
    const Loop *back_loop;
    bool operator==(const NodeLoop &that) const {
      return node == that.node and back_loop == that.back_loop;
    }
  };

  // These are ordered by the node's topological order and then by pointer order
  // for back_loop.
  struct node_loop_topo_order {
    bool operator()(const NodeLoop &a, const NodeLoop &b) {
      topo_order top;
      if (top(a.node, b.node))
        return true;
      if (top(b.node, a.node))
        return false;
      return std::less<const Loop *>{}(a.back_loop, b.back_loop);
    }
  };

  // Calculate the initial chaintips based on the predecessors. Any dependencies
  // that dominate this node are automatically promoted, while phibits are
  // generated for other dependencies.
  DepVec tips;
  for (Node *const pred : preds) {
    const Loop *pred_loop;
    if (ignore_pred(pred, this, loop, nullptr, nullptr, pred_loop))
      continue;
    for (const Dep &dep : pred->chaintips) {
      if (not irrelevant_loop_phibit(dep)) {
        tips.push_back(dominated_by(dep.node) ? dep : get_phibit(pred, dep));
      }
    }
  }

  // Make sure that the chaintips are unique. They don't really need to be
  // sorted here, but that's the simplest way to do it.
  std::sort(begin(tips), end(tips));
  tips.erase(unique(begin(tips), end(tips)), end(tips));

  if (loop) {
    LLVM_DEBUG(dbgs() << loop->depth << " " << *loop << " ");
  } else
    LLVM_DEBUG(dbgs() << "0 - ");
  LLVM_DEBUG(dbgs() << topo_num << ": " << tips.size() << "\n");

  for (int memop_idx = 0; memop_idx != int(memops.size()); ++memop_idx) {
    Memop &memop = memops[memop_idx];

    // A queue for nodes to be processed in the backwards traversal.
    priority_queue<NodeLoop, vector<NodeLoop>, node_loop_topo_order> node_queue;

    // The nodes traversed by the backwards traversal, reversed to be in the
    // correct order for the forward traversal.
    SmallVector<NodeLoop, NODES_PER_FUNCTION> nodes_traversed;

    // Start with the current memop as the only connected one, the disconnected
    // queue populated with the chaintip values, and this node on the node
    // queue.
    DepSetEntry &depset = get_depset();
    depset.connected.emplace_back(this, Dep::memop, memop_idx);
    depset.disconnected = priority_queue<Dep>{begin(tips), end(tips)};
    node_queue.push({this, nullptr});

    // Do the backwards traversal using the node queue and push nodes (and
    // loops) to nodes_traversed as they are visited.
    while (not node_queue.empty()) {
      const NodeLoop cur_nodeloop = node_queue.top();
      while (not node_queue.empty() and node_queue.top() == cur_nodeloop) {
        node_queue.pop();
      }
      nodes_traversed.push_back(cur_nodeloop);

      if (not cur_nodeloop.node->step_backwards(
            memop, loop, cur_nodeloop.back_loop,
            cur_nodeloop.node == this and not cur_nodeloop.back_loop
              ? memop_idx
              : cur_nodeloop.node->memops.size()))
        return false;

      for (Node *const pred : cur_nodeloop.node->preds) {
        const Loop *pred_loop;
        if (cur_nodeloop.node->ignore_pred(pred, this, loop,
                                           cur_nodeloop.back_loop,
                                           memop.PrefLoop, pred_loop))
          continue;
        node_queue.push({pred, pred_loop});
      }
    }
    reverse(begin(nodes_traversed), end(nodes_traversed));

    // Use nodes_traversed to do the forwards traversal.
    for (const NodeLoop cur_nodeloop : nodes_traversed) {
      cur_nodeloop.node->step_forwards(this, loop, cur_nodeloop.back_loop,
                                       memop.PrefLoop);
    }

    // Pull all of the new dependencies for this memop out of to_connect.
    const DepVec &to_connect = depset.to_connect;
    memop.exp_deps.append(begin(to_connect), end(to_connect));
    std::sort(begin(memop.exp_deps), end(memop.exp_deps));
    assert(unique(begin(memop.exp_deps), end(memop.exp_deps)) ==
           end(memop.exp_deps));

    // Reset all of the DepSetEntries involved.
    for (const NodeLoop cur_nodeloop : nodes_traversed) {
      for (DepSetEntry &depset : cur_nodeloop.node->depsets)
        depset.clear();
    }

    // Clean out any chaintips that got connected.
    tips.erase(
      remove_if(begin(tips), end(tips),
                [&memop](const Dep &dep) {
                  return any_of(begin(memop.exp_deps), end(memop.exp_deps),
                                bind(&Dep::implies, _1, dep)) or
                         any_of(begin(memop.imp_deps), end(memop.imp_deps),
                                bind(&Dep::implies, _1, dep));
                }),
      end(tips));

    // Add the current memop to the chaintip set.
    tips.emplace_back(this, Dep::memop, memop_idx);
  }

  chaintips = move(tips);

  return true;
}

bool MemopCFG::Node::step_backwards(const Memop &cur_memop, const Loop *loop,
                                    const Loop *back_loop, int memop_idx) {
  using namespace std;

  DepSetEntry &depset  = get_depset(back_loop);
  DepVec &connected    = depset.connected;
  auto &disconnected   = depset.disconnected;
  DepVec &to_connect   = depset.to_connect;
  auto &pred_connected = depset.pred_connected;

  // Make a copy of section states for processing in this node.
  SectionStates sec_states = depset.section_states;

  // Start up a priority queue for connected dependencies.
  priority_queue<Dep> connected_queue{begin(connected), end(connected)};
  connected.clear();

  // Some helper functions for making dealing with the queues a little nicer.
  const auto top_is_memop_in_node = [this](const priority_queue<Dep> &queue) {
    return not queue.empty() and queue.top().node == this and
           queue.top().type == Dep::memop;
  };
  const auto push_deps_to_queue =
    [this, &cur_memop](const Dep &memop_dep, priority_queue<Dep> &queue) {
      assert(memop_dep.node == this and memop_dep.type == Dep::memop);
      const Memop &memop = memops[memop_dep.idx];
      for (const Dep &dep : memop.exp_deps) {
        if (not cur_memop.parent->irrelevant_loop_phibit(dep))
          queue.push(dep);
      }
      for (const Dep &dep : memop.imp_deps) {
        if (not cur_memop.parent->irrelevant_loop_phibit(dep))
          queue.push(dep);
      }
    };

  // Take care of any disconnected memops in this node.
  auto cur_intr = find_if(regsec_intrinsics.rbegin(), regsec_intrinsics.rend(),
                          [memop_idx](const RegSecIntrinsic &intr) {
                            return intr.memop_idx <= memop_idx;
                          });
  while (top_is_memop_in_node(disconnected)) {
    const Dep cur_dis = disconnected.top();
    while (not disconnected.empty() and disconnected.top() == cur_dis) {
      disconnected.pop();
    }
    const Memop &dis_memop = memops[cur_dis.idx];

    // Step along the connected queue until cur_dis is reached.
    while (not connected_queue.empty() and cur_dis < connected_queue.top()) {
      const Dep top = connected_queue.top();
      while (not connected_queue.empty() and connected_queue.top() == top) {
        connected_queue.pop();
      }
      push_deps_to_queue(top, connected_queue);
    }

    // If the disconnected memop actually shows up in the connected queue, it
    // can just be dropped.
    if (not connected_queue.empty() and cur_dis == connected_queue.top()) {
      continue;
    }

    // Handle any relevant region/section intrinsics.
    while (cur_intr != regsec_intrinsics.rend() and
           cur_intr->memop_idx > cur_dis.idx) {
      if (not sec_states.transition(*cur_intr))
        return false;
      ++cur_intr;
    }

    // Otherwise, check whether this memop needs to be ordered with the current
    // one. It does unless one of these applies:
    const bool previously_ordered =
      cur_dis.node->deepest_loop and not back_loop and
      (not loop or loop->depth < cur_dis.node->deepest_loop->depth) and
      cur_dis.node->deepest_loop->contains(cur_memop.parent);
    bool IgnoreParallelDep = back_loop and (RacyLoops or back_loop->IsParallel);
    const bool ignore_ordering =
      not cur_memop.PrefLoop and
      (IgnoreParallelDep or sec_states.should_ignore_memops());
    if (

      // The two memops were already ordered in a previous pass through this
      // loop.
      not previously_ordered

      // This memop was reached through a backedge of a loop with parallel
      // markings or along a path with a parallel section crossing. Both of
      // these conditions are ignored if the memop is a prefetch.
      and not ignore_ordering

      // require_ordering indicates that no ordering is needed.
      and require_ordering(dis_memop, cur_memop, back_loop)

    ) {

      // If it does, the disconnected memop needs to get connected.
      to_connect.push_back(cur_dis);
      push_deps_to_queue(cur_dis, connected_queue);

    } else {

      // Otherwise, it needs to get peeled.
      push_deps_to_queue(cur_dis, disconnected);
    }
  }

  // Take care of any remaining connected memops in this node.
  while (top_is_memop_in_node(connected_queue)) {
    const Dep top = connected_queue.top();
    while (not connected_queue.empty() and connected_queue.top() == top) {
      connected_queue.pop();
    }
    push_deps_to_queue(top, connected_queue);
  }

  // And any remaining region/section intrinsics.
  while (cur_intr != regsec_intrinsics.rend()) {
    if (not sec_states.transition(*cur_intr))
      return false;
    ++cur_intr;
  }

  // Populate pred_[dis]connected.
  const auto populate_preds = [this](priority_queue<Dep> &queue,
                                     decltype(pred_connected) &pred_arrays) {
    while (not queue.empty() and queue.top().node == this) {
      const Dep top = queue.top();
      assert(top.type == Dep::phibit);
      while (not queue.empty() and queue.top() == top)
        queue.pop();
      const auto found_pred =
        find(begin(preds), end(preds), phibits[top.idx].pred);
      assert(found_pred != end(preds));
      pred_arrays[distance(begin(preds), found_pred)].push_back(
        phibits[top.idx].dep);
    }

    while (not queue.empty()) {
      const Dep top = queue.top();
      while (not queue.empty() and queue.top() == top)
        queue.pop();
      for (DepVec &pred_array : pred_arrays)
        pred_array.push_back(top);
    }
  };
  populate_preds(connected_queue, pred_connected);
  SmallVector<DepVec, PRED_COUNT> pred_disconnected(preds.size());
  populate_preds(disconnected, pred_disconnected);

  // Make sure to_connect gets sorted.
  std::sort(begin(to_connect), end(to_connect));

  // Update the predecessors.
  DepVec tmp;
  for (int pred_idx = 0; pred_idx != int(preds.size()); ++pred_idx) {
    Node *const pred = preds[pred_idx];
    const Loop *pred_loop;
    if (ignore_pred(pred, cur_memop.parent, loop, back_loop, cur_memop.PrefLoop,
                    pred_loop))
      continue;
    DepSetEntry &pred_depset = pred->get_depset(pred_loop);
    DepVec &pc               = pred_connected[pred_idx];
    std::sort(begin(pc), end(pc));
    if (pred_depset.first_succ) {
      pred_depset.connected      = pc;
      pred_depset.section_states = sec_states;
      pred_depset.first_succ     = false;
    } else {
      if (not pred_depset.section_states.merge_from(sec_states))
        return false;
      set_intersection(begin(pred_depset.connected), end(pred_depset.connected),
                       begin(pc), end(pc), back_inserter(tmp));
      swap(pred_depset.connected, tmp);
      tmp.clear();
    }
    for (const Dep &dep : pred_disconnected[pred_idx]) {
      pred_depset.disconnected.push(dep);
    }
  }
  return true;
}

void MemopCFG::Node::step_forwards(const Node *memop_node, const Loop *ord_loop,
                                   const Loop *back_loop,
                                   const Loop *pref_loop) {
  using namespace std;

  DepSetEntry &depset  = get_depset(back_loop);
  DepVec &to_connect   = depset.to_connect;
  auto &pred_connected = depset.pred_connected;

  // Go through and pick out the common dependencies from predecessors'
  // to_connect fields.
  DepVec commons;
  DepVec tmp;
  bool first_pred = true;
  for (Node *const pred : preds) {
    const Loop *pred_loop;
    if (ignore_pred(pred, memop_node, ord_loop, back_loop, pref_loop,
                    pred_loop))
      continue;
    const DepVec &pred_to_connect = pred->get_depset(pred_loop).to_connect;
    if (first_pred) {
      commons    = pred_to_connect;
      first_pred = false;
    } else {
      set_intersection(begin(commons), end(commons), begin(pred_to_connect),
                       end(pred_to_connect), back_inserter(tmp));
      swap(tmp, commons);
      tmp.clear();
    }
  }

  // Get rid of common dependencies that do not dominate this node and add the
  // rest to to_connect.
  commons.erase(
    remove_if(begin(commons), end(commons),
              [this](const Dep &dep) { return not dominated_by(dep.node); }),
    end(commons));
  to_connect.append(begin(commons), end(commons));

  // Round up all of the non-common inputs which are not in pred_connected.
  for (int pred_idx = 0; pred_idx != int(preds.size()); ++pred_idx) {
    Node *const pred = preds[pred_idx];
    const Loop *pred_loop;
    if (ignore_pred(pred, memop_node, ord_loop, back_loop, pref_loop,
                    pred_loop))
      continue;
    const DepVec &pred_to_connect = pred->get_depset(pred_loop).to_connect;
    for (const Dep &dep : pred_to_connect) {
      if (not binary_search(begin(commons), end(commons), dep) and
          not binary_search(begin(pred_connected[pred_idx]),
                            end(pred_connected[pred_idx]), dep))
        to_connect.push_back(get_phibit(pred, dep));
    }
    pred_connected[pred_idx].clear();
  }

  // Make sure that to_connect is sorted and unique.
  std::sort(begin(to_connect), end(to_connect));
  to_connect.erase(unique(begin(to_connect), end(to_connect)), end(to_connect));
}

void MemopCFG::Node::construct_chains() {
  using namespace std;
  for (int memop_idx = 0; memop_idx != int(memops.size()); ++memop_idx) {
    DepVec &exp_deps = memops[memop_idx].exp_deps;
    DepVec &imp_deps = memops[memop_idx].imp_deps;
    DepVec may;
    may.reserve(exp_deps.size() + imp_deps.size());
    set_union(begin(exp_deps), end(exp_deps), begin(imp_deps), end(imp_deps),
              back_inserter(may));
    memops[memop_idx].ready = insert_merges_and_phis(exp_deps, may, memop_idx);
  }
}

MemopCFG::OrdToken MemopCFG::Node::insert_merges_and_phis(DepVec &must,
                                                          const DepVec &may,
                                                          int memop_idx) {
  using namespace std;

  // Figure out where the may deps for this node start.
  const auto this_may_beg =
    lower_bound(begin(may), end(may), this, comp_dep_by_node{});

  // The merge that is being constructed.
  Merge merge;
  merge.memop_idx = memop_idx;

  // Separate out phibits that aren't in this node and have those nodes handle
  // them instead.
  const auto ext_phi_split =
    partition(begin(must), end(must), [this](const Dep &dep) {
      return dep.type != Dep::phibit or dep.node == this;
    });
  if (ext_phi_split != end(must)) {
    std::sort(ext_phi_split, end(must));

    // Iterate each node that has phibits from the must set and extract those
    // phibits. Since deps order topologically by node, all of the must phibits
    // for each node appear contiguously. Note that each of these nodes must
    // dominate the current one.
    SmallVector<OrdToken, LIVE_DEP_COUNT> ext_phis;
    for (auto ext_must_beg = ext_phi_split; ext_must_beg != end(must);) {
      Node *dom = ext_must_beg->node;
      const auto ext_must_end =
        upper_bound(ext_must_beg, end(must), dom, comp_dep_by_node{});
      DepVec ext_must{ext_must_beg, ext_must_end};

      // For the may set, pass along every value from nodes up to dom in order.
      // Since these nodes (including dom) all dominate the current node and
      // none of them appears after dom in topological order, these nodes must
      // all (nonstrictly) dominate dom.
      const auto ext_may_end =
        upper_bound(begin(may), this_may_beg, dom, comp_dep_by_node{});
      DepVec ext_may{begin(may), ext_may_end};

      ext_phis.push_back(
        dom->insert_merges_and_phis(ext_must, ext_may, dom->memops.size()));

      ext_must_beg = ext_must_end;
    }
    merge.inputs.append(begin(ext_phis), end(ext_phis));
  }

  // Separate out phibits for each predecessor and recurse to handle those.
  PHI phi;
  const auto phi_split =
    partition(begin(must), ext_phi_split,
              [](const Dep &dep) { return dep.type != Dep::phibit; });
  if (phi_split != ext_phi_split) {
    for (Node *const pred : preds) {
      DepVec pred_must;
      for (auto it = phi_split; it != ext_phi_split; ++it) {
        assert(it->type == Dep::phibit and it->node == this);
        if (phibits[it->idx].pred == pred) {
          pred_must.push_back(phibits[it->idx].dep);
        }
      }

      // The may set should include everything up to this node's common
      // dominator with pred along with the matching phibits from this node.
      DepVec pred_may;
      const Node *const common_dom = lowest_common_dominator(pred);
      if (common_dom) {
        const auto may_dom_end =
          upper_bound(begin(may), this_may_beg, common_dom, comp_dep_by_node{});
        pred_may.assign(begin(may), may_dom_end);
      }
      for (auto it = this_may_beg; it != end(may); ++it) {
        if (it->type == Dep::phibit and phibits[it->idx].pred == pred) {
          pred_may.push_back(phibits[it->idx].dep);
        }
      }
      std::sort(begin(pred_may), end(pred_may));

      phi.inputs.push_back(
        pred->insert_merges_and_phis(pred_must, pred_may, pred->memops.size()));
    }
    merge.inputs.push_back(get_phi(move(phi)));
  }

  // Translate the remaining dependencies in must to the merge inputs.
  transform(begin(must), phi_split, back_inserter(merge.inputs),
            [](const Dep &dep) {
              assert(dep.type == Dep::memop);
              return OrdToken{dep.node, OrdToken::memop, dep.idx};
            });

  // Make a merge.
  std::sort(begin(merge.inputs), end(merge.inputs));
  std::sort(begin(must), end(must));
  swap(merge.must_merge, must);
  merge.may_merge = may;
  return get_merge(move(merge));
}

bool MemopCFG::Loop::contains(const Node *node) const {
  using namespace std;
  return binary_search(begin(nodes), end(nodes), node, Node::topo_order{});
}

void MemopCFG::Loop::collect_loop_imp_deps(const Dep &A, const Dep &B) const {
  using namespace std;
  using namespace std::placeholders;
  assert(B.type == Dep::memop);
  DepVec &imp_deps = B.node->memops[B.idx].imp_deps;
  map<Node *, DepVec> reach_cache;

  // Collect self deps from each latch.
  Node *const header = nodes.front();
  for (Node *const latch : header->preds) {
    if (latch->topo_num < header->topo_num)
      continue;
    const DepVec latch_deps = reachable_deps(latch, A, reach_cache);
    transform(begin(latch_deps), end(latch_deps), back_inserter(imp_deps),
              bind(&Node::get_phibit, header, latch, _1));
  }
}

MemopCFG::DepVec
MemopCFG::Loop::reachable_deps(Node *node, const Dep &memop,
                               std::map<Node *, DepVec> &reach_cache) const {
  using namespace std;
  using namespace std::placeholders;

  // If the node isn't in the loop, ignore it.
  if (not contains(node))
    return {};

  // If the memop's node dominates this node, just point to it directly.
  if (node->nonstrictly_dominated_by(memop.node))
    return {memop};

  // If it's in the cache, just use that value.
  const auto found = reach_cache.lower_bound(node);
  if (found != end(reach_cache) and found->first == node)
    return found->second;

  // Try the node's dominator first.
  DepVec self_deps = reachable_deps(node->dominator, memop, reach_cache);
  if (not self_deps.empty())
    return self_deps;

  // If that didn't work, collect values from the predecessors instead. Ignore
  // loop backedges.
  for (Node *const pred : node->preds)
    if (pred->topo_num < node->topo_num) {
      const DepVec pred_deps = reachable_deps(pred, memop, reach_cache);
      transform(begin(pred_deps), end(pred_deps), back_inserter(self_deps),
                bind(&Node::get_phibit, node, pred, _1));
    }
  reach_cache.emplace_hint(found, node, self_deps);
  return self_deps;
}

void MemopCFG::load(
  Function &F, AAResults *AA, const DominatorTree &DT, const LoopInfo &LI,
  ReversePostOrderTraversal<Function *> &RPOT,
  const std::function<bool(Instruction &)> &needs_ordering_edges,
  bool can_speculate, bool use_parallel_sections) {
  using namespace std;
  using namespace std::placeholders;

  // Update require_ordering.
  require_ordering = RequireOrdering{AA};

  // Create all of the nodes from the basic blocks.
  {
    map<int, int> normalized_regions;
    for (BasicBlock *const BB : RPOT) {
      nodes.push_back(unique_ptr<Node>{
        new Node{BB, int(nodes.size()), require_ordering, needs_ordering_edges,
                 use_parallel_sections, normalized_regions}});
      nodes_for_bbs.insert({BB, nodes.back().get()});
    }

    // The number of regions can now be determined from normalized_regions.
    region_count = normalized_regions.size();
  }

  // Go through and wire up all of the predecessors, successors and dominators.
  for (const unique_ptr<Node> &pred : nodes) {
    for (const BasicBlock *const succ_bb : successors(pred->BB)) {
      Node *const succ = nodes_for_bbs[succ_bb];
      pred->succs.push_back(succ);
      succ->preds.push_back(pred.get());
    }
    if (const DomTreeNode *const dom_node = DT.getNode(pred->BB)->getIDom())
      pred->dominator = nodes_for_bbs[dom_node->getBlock()];
    else
      pred->dominator = nullptr;
  }

  // Collect all of the loops and set loop information for the nodes
  // appropriately.
  for (const llvm::Loop *const L : LI)
    collect_loops(L);
  for (Loop &loop : loops)
    for (Node *const node : loop.nodes) {
      if (not node->deepest_loop) {
        node->deepest_loop = &loop;
      } else if (not node->deepest_loop->parent) {
        node->deepest_loop->parent = &loop;
      }
    }

  // Also preallocate all of the DepSetEntries
  for (const unique_ptr<Node> &node : nodes) {
    node->depsets.resize(node->deepest_loop ? node->deepest_loop->depth + 1
                                            : 1);
    for (Node::DepSetEntry &depset : node->depsets) {
      depset.pred_connected.resize(node->preds.size());
      depset.section_states.states.resize(region_count);
    }
  }

  // Set up memops_for_insts.
  for (const unique_ptr<Node> &node : nodes) {
    for (int idx = 0; idx != int(node->memops.size()); ++idx) {
      if (node->memops[idx].I) {
        memops_for_insts.insert(
          {node->memops[idx].I, {node.get(), Dep::memop, idx}});
      }
    }
  }

  mark_prefetch_loops();

  calculate_pool_numbers();

  calculate_imp_deps(can_speculate);
}

void MemopCFG::collect_loops(const llvm::Loop *L) {
  using namespace std;

  // Do a post-order traversal so that loops are listed in the right order.
  for (const llvm::Loop *const subloop : L->getSubLoops()) {
    collect_loops(subloop);
  }

  // Create a new Loop and transfer the set of basic blocks.
  Loop new_loop;
  new_loop.IRL   = L;
  new_loop.depth = L->getLoopDepth();

  //
  // Check if this loop was transformed by CSALowerParallelIntrinsics pass.
  //
  if (auto *LoopID = L->getLoopID()) {
    LLVM_DEBUG(dbgs() << "Loop with metadata: " << L->getHeader()->getName()
                      << "\n");
    for (unsigned Indx = 1; Indx < LoopID->getNumOperands(); ++Indx) {
      if (auto *T = dyn_cast<MDTuple>(LoopID->getOperand(Indx)))
        if (T->getNumOperands() != 0)
          if (auto *S = dyn_cast<MDString>(T->getOperand(0)))
            if (S->getString() == CSALoopTag::Parallel) {
              LLVM_DEBUG(dbgs() << "The loop is marked with Parallel.\n");
              new_loop.IsParallel = true;
            }
    }
  }

  for (const BasicBlock *const BB : L->getBlocks()) {
    new_loop.nodes.push_back(nodes_for_bbs[BB]);
  }
  std::sort(begin(new_loop.nodes), end(new_loop.nodes),
            [](Node *a, Node *b) { return a->topo_num < b->topo_num; });
  loops.push_back(move(new_loop));
}

void MemopCFG::mark_prefetch_loops() {
  using std::unique_ptr;

  // Visit all loops, in post-order.
  for (Loop &L : loops) {

    // Examine the nodes of this loop to see if any contain prefetch-orderable
    // ops.
    for (const Node *const N : L.nodes) {

      // This node may have already been examined as part of a subloop; if so,
      // mark this loop too.
      assert(N->deepest_loop);
      if (N->deepest_loop != &L) {
        L.RPrefEligible |= N->deepest_loop->RPrefEligible;
        L.WPrefEligible |= N->deepest_loop->WPrefEligible;
        if (L.RPrefEligible and L.WPrefEligible)
          break;
      }

      // Otherwise, check each of the memops in this node. If at any point both
      // flags are set, there's no point to continuing the search and both
      // levels of loop can exit.
      for (const Memop &M : N->memops) {
        L.RPrefEligible |= M.orders_with_prefetch(0);
        L.WPrefEligible |= M.orders_with_prefetch(1);
        if (L.RPrefEligible and L.WPrefEligible)
          break;
      }
      if (L.RPrefEligible and L.WPrefEligible)
        break;
    }
  }

  // Visit all prefetches.
  for (const unique_ptr<Node> &N : nodes) {
    for (Memop &M : N->memops) {
      const auto II = dyn_cast<IntrinsicInst>(M.I);
      if (not II or II->getIntrinsicID() != Intrinsic::prefetch)
        continue;

      // Find the deepest containing loop compatible with this prefetch type.
      const int RW = M.prefetch_rw();
      for (const Loop *L = M.parent->deepest_loop; L; L = L->parent) {
        if (RW == 1 ? L->WPrefEligible : L->RPrefEligible) {
          M.PrefLoop = L;
          break;
        }
      }
    }
  }
}

void MemopCFG::calculate_pool_numbers() {
  using std::begin;
  using std::end;
  using std::unique;
  using std::unique_ptr;

  // The next pool number to use.
  int PN = 0;

  // Iterate the function to identify pipeline.depth.token.takes.
  for (const unique_ptr<Node> &N : nodes) {
    for (Memop &M : N->memops) {
      const auto TT = dyn_cast<IntrinsicInst>(M.I);
      if (not TT)
        continue;
      if (TT->getIntrinsicID() != Intrinsic::csa_pipeline_depth_token_take)
        continue;

      // Grab the token take's first parameter. This should be its corresponding
      // CsaMemAlloc or a GEP based on the CsaMemAlloc.
      const auto PoolStart = dyn_cast<Instruction>(TT->getArgOperand(0));
      auto CMA             = dyn_cast_or_null<CallInst>(PoolStart);
      if (not CMA) {
        auto GEP = dyn_cast_or_null<GetElementPtrInst>(PoolStart);
        if (GEP) {
          CMA = dyn_cast<CallInst>(GEP->getPointerOperand());
        }
      }
      if (not CMA or CMA->getCalledOperand()->getName() != "CsaMemAlloc")
        report_fatal_error("Token takes should be direct users of CsaMemAlloc "
                           "calls or indirect users through a GEP");

      // Mark the CsaMemAlloc call and its non-token_take/return users with the
      // pool number.
      const auto CMAMI = memops_for_insts.find(CMA);
      assert(CMAMI != end(memops_for_insts));
      assert(CMAMI->second.type == Dep::memop);
      Memop &CMAM = CMAMI->second.node->memops[CMAMI->second.idx];
      CMAM.Pools.push_back(PN);
      SmallPtrSet<const PHINode *, 4> VisitedPHIs;
      markLoopLocalMemops(CMA, PN, true, VisitedPHIs);

      // Also mark the token_take and its users.
      const auto TTMI = memops_for_insts.find(TT);
      assert(TTMI != end(memops_for_insts));
      assert(TTMI->second.type == Dep::memop);
      Memop &TTM = TTMI->second.node->memops[TTMI->second.idx];
      assert(TTM.Pools.empty());
      TTM.Pools.push_back(PN);
      markLoopLocalMemops(TT, PN, false, VisitedPHIs);

      // Move to the next pool number.
      ++PN;
    }
  }

  // Sort+unique all of the pool numbers.
  for (const unique_ptr<Node> &N : nodes) {
    for (Memop &M : N->memops) {
      sort(M.Pools);
      M.Pools.erase(unique(begin(M.Pools), end(M.Pools)), end(M.Pools));
    }
  }
}

void MemopCFG::markLoopLocalMemops(
  const Instruction *I, int PN, bool IgnoreTTR,
  SmallPtrSetImpl<const PHINode *> &VisitedPHIs) {
  if (const auto PI = dyn_cast<PHINode>(I)) {
    if (VisitedPHIs.count(PI))
      return;
    VisitedPHIs.insert(PI);
  }
  for (const User *U : I->users()) {
    const auto UI = dyn_cast<Instruction>(U);
    if (not UI)
      continue;
    if (IgnoreTTR and isDepthTokenCall(dyn_cast<IntrinsicInst>(UI)))
      continue;
    const auto MII = memops_for_insts.find(UI);
    if (MII != end(memops_for_insts)) {
      const Dep UMD = MII->second;
      assert(UMD.type == Dep::memop);
      UMD.node->memops[UMD.idx].Pools.push_back(PN);
    } else {
      markLoopLocalMemops(UI, PN, IgnoreTTR, VisitedPHIs);
    }
  }
}

void MemopCFG::calculate_imp_deps(bool can_speculate) {
  using llvm::sort;
  using std::begin;
  using std::end;
  using std::unique;
  using std::unique_ptr;

  // Mark token return-take dependencies.
  markReturnTakeDeps();

  // If we're using self dependencies, mark all of them.
  if (not IgnoreSelfDeps)
    calculate_self_deps();

  // If we're using data dependencies, mark those too.
  if (not can_speculate and not IgnoreDataDeps)
    calculate_data_deps();

  // Clean up each memop's imp_deps. Sort + unique and then remove redundant
  // phibits.
  for (const unique_ptr<Node> &node : nodes) {
    for (Memop &memop : node->memops) {
      DepVec &imp_deps = memop.imp_deps;
      sort(begin(imp_deps), end(imp_deps));
      imp_deps.erase(unique(begin(imp_deps), end(imp_deps)), end(imp_deps));
      for (auto it = begin(imp_deps); it != end(imp_deps);) {
        using namespace std::placeholders;
        if (it->type == Dep::phibit and
            any_of(begin(imp_deps), end(imp_deps),
                   bind(&Dep::strictly_implies, _1, *it)))
          it = imp_deps.erase(it);
        else
          ++it;
      }
    }
  }
}

void MemopCFG::markReturnTakeDeps() {
  using std::unique_ptr;

  for (const unique_ptr<Node> &N : nodes) {
    for (int MI = 0; MI < int(N->memops.size()); ++MI) {
      const auto TTI = dyn_cast<IntrinsicInst>(N->memops[MI].I);
      if (not TTI)
        continue;
      if (TTI->getIntrinsicID() != Intrinsic::csa_pipeline_depth_token_take)
        continue;

      Dep TR;
      for (const User *U : TTI->getArgOperand(0)->users()) {
        const auto TRI = dyn_cast<IntrinsicInst>(U);
        if (not TRI)
          continue;
        if (TRI->getIntrinsicID() != Intrinsic::csa_pipeline_depth_token_return)
          continue;
        TR = memops_for_insts[TRI];
        break;
      }
      assert(TR.node);

      Dep TT{N.get(), Dep::memop, MI};
      for (const Loop &L : loops) {
        if (L.contains(N.get()))
          L.collect_loop_imp_deps(TR, TT);
      }
    }
  }
}

// Determines whether invocations of I are executed strictly in order. This is
// a general property of memory ops on CSA, but does not hold for dataflow
// function calls.
static bool isSelfOrdered(const Instruction *I) {

  // Calls are not self-ordered.
  if (const auto CI = dyn_cast<CallInst>(I)) {

    // But intrinsics (currently prefetches and csa.pipeline.depth.token.*)
    // _are_ self-ordered.
    return isa<IntrinsicInst>(CI);
  }

  // Other instructions are self-ordered.
  return true;
}

void MemopCFG::calculate_self_deps() {
  for (const Loop &loop : loops) {
    for (Node *const node : loop.nodes) {
      for (int memop_idx = 0; memop_idx != int(node->memops.size());
           ++memop_idx) {

        const Dep memop{node, Dep::memop, memop_idx};
        if (isSelfOrdered(node->memops[memop_idx].I))
          loop.collect_loop_imp_deps(memop, memop);
      }
    }
  }
}

// Determines whether an instruction will be adequately ordered through any of
// its data inputs or outputs and if each of its outputs can be used to order
// with the inputs. This is true in general of the CSA operations that are
// representable as instructions in IR, but not of dataflow calls or returns.
static bool isDataOrderable(const Instruction *I) {

  // Calls are not data-orderable.
  if (const auto CI = dyn_cast<CallInst>(I)) {

    // But some intrinsics (prefetches and csa.pipeline.depth.token.* for now)
    // _are_ data-orderable.
    // TODO: We can expand this list [CMPLRLLVM-9168]
    if (const auto II = dyn_cast<IntrinsicInst>(CI)) {
      return II->getIntrinsicID() == Intrinsic::prefetch or
             isDepthTokenCall(II);
    }

    return false;
  }

  // Returns aren't either.
  if (isa<ReturnInst>(I))
    return false;

  // Other instructions are data-orderable.
  return true;
}

void MemopCFG::calculate_data_deps() {
  using llvm::sort;
  using std::back_inserter;
  using std::begin;
  using std::end;
  using std::unique;
  using std::unique_ptr;
  using std::upper_bound;

  // And keep track of which memops each instruction depends on. The DepVec
  // values here are maintained in sorted order.
  DenseMap<const Instruction *, DepVec> memop_deps;

  // Adds dependencies to memop_deps for machine instructions in a given
  // node's basic block.
  const auto collect_deps = [this, &memop_deps](Node *node) {
    const BasicBlock *const BB = node->BB;
    for (const Instruction &I : *BB) {

      // For phi nodes, look for phibit dependencies from predecessors.
      if (const auto PI = dyn_cast<PHINode>(&I)) {
        for (const BasicBlock *const PVB : PI->blocks()) {
          const Instruction *const PVI =
            dyn_cast<Instruction>(PI->getIncomingValueForBlock(PVB));

          // Ignore incoming values that aren't data-orderable.
          if (not PVI or not isDataOrderable(PVI))
            continue;

          // If the incoming value is a memop, add it directly. If not, add its
          // memop dependencies. Drop deps that are crossing their second loop
          // backedge.
          const auto found_memop = memops_for_insts.find(PVI);
          DepVec &deps           = memop_deps[&I];
          Node *const pred       = nodes_for_bbs[PVB];
          if (found_memop != end(memops_for_insts)) {
            Dep new_dep = node->get_phibit(pred, found_memop->second);
            deps.insert(upper_bound(begin(deps), end(deps), new_dep), new_dep);
          } else {
            // Avoid using [] operator to look for def's memory dependencies,
            // because this may invalidate 'deps' reference.  Moreover,
            // if def does not have incoming memory depencies, there is
            // no sense in sorting and uniquing of the unchanged 'deps'
            // vector.
            auto DefDepsIt = memop_deps.find(PVI);
            if (DefDepsIt != end(memop_deps)) {
              const DepVec &def_deps = DefDepsIt->second;
              for (const Dep &dep : def_deps) {
                if (dep.type == Dep::phibit and
                    dep.node->phibits[dep.idx].loop and
                    pred->topo_num >= node->topo_num)
                  continue;
                deps.push_back(node->get_phibit(pred, dep));
              }
              sort(begin(deps), end(deps));
              deps.erase(unique(begin(deps), end(deps)), end(deps));
            }
          }
        }
      }

      // For other non-call/return instructions, look for normal dominating
      // dependencies.
      else if (isDataOrderable(&I)) {
        for (const Value *const V : I.operand_values()) {
          const Instruction *const VI = dyn_cast<Instruction>(V);
          if (not VI or not isDataOrderable(VI))
            continue;

          const auto found_memop = memops_for_insts.find(VI);
          DepVec &deps           = memop_deps[&I];
          if (found_memop != end(memops_for_insts)) {
            Dep &new_dep = found_memop->second;
            deps.insert(upper_bound(begin(deps), end(deps), new_dep), new_dep);
          } else {
            // Avoid using [] operator to look for def's memory dependencies,
            // because this may invalidate 'deps' reference.  Moreover,
            // if def does not have incoming memory depencies, there is
            // no sense in creating the union the way we do it below.
            auto DefDepsIt = memop_deps.find(VI);
            if (DefDepsIt != end(memop_deps)) {
              const DepVec &def_deps = DefDepsIt->second;
              const DepVec old_deps  = deps;
              deps.clear();
              set_union(begin(old_deps), end(old_deps), begin(def_deps),
                        end(def_deps), back_inserter(deps));
            }
          }
        }
      }
    }
  };

  // Look for data dependencies in loops first.
  for (Loop &loop : loops)
    for (Node *const node : loop.nodes)
      collect_deps(node);

  // Then, go through the entire function.
  for (const unique_ptr<Node> &node : nodes)
    collect_deps(node.get());

  // Add to each memop's imp_deps.
  for (const unique_ptr<Node> &node : nodes) {
    for (Memop &memop : node->memops) {
      const DepVec &deps = memop_deps[memop.I];
      memop.imp_deps.append(begin(deps), end(deps));
    }
  }
}

void MemopCFG::clear() {
  nodes.clear();
  loops.clear();
  nodes_for_bbs.clear();
  memops_for_insts.clear();
}

bool MemopCFG::construct_chains() {
  using namespace std;

  // Calculate dependencies.
  for (const Loop &loop : loops) {
    for (Node *const node : loop.nodes) {
      if (not node->calculate_deps(&loop))
        return false;
    }
  }
  for (const unique_ptr<Node> &node : nodes) {
    if (not node->calculate_deps(nullptr))
      return false;
  }

  LLVM_DEBUG(dbgs() << "after dependency calculation:\n\n" << *this);
  if (ViewDepMemopCFG) {
    assert(not nodes.empty());
    ViewGraph(*this, nodes.front()->BB->getParent()->getName());
  }

  // Construct chains based on those.
  for (const unique_ptr<Node> &node : nodes)
    node->construct_chains();

  LLVM_DEBUG(dbgs() << "after chain construction:\n\n" << *this);

  return true;
}

void MemopCFG::emit_opt_report(OptimizationRemarkEmitter &ORE) {

  for (const auto &Loop : loops) {
    // Find MachineLoop corresponding to this loop.
    auto *FrontNode = Loop.nodes.front();
    assert(FrontNode && "Empty Loop in CSA memory operation ordering.");
    auto *BB = FrontNode->BB;

    // Use unknown debug location, if we cannot get it from the loop.
    auto *CurrentLoop = Loop.IRL;
    auto LoopLoc      = CurrentLoop ? CurrentLoop->getStartLoc() : DebugLoc();

    if (FrontNode->phis.empty()) {
      // The PHI nodes are always inserted into the header.
      // As long as the loop nodes are sorted topologically,
      // the header corresponds to the first node in Loop.nodes.
      OptimizationRemark Remark(DEBUG_TYPE, "CSAPipelining: ", LoopLoc, BB);
      ORE.emit(Remark << " loop does not have loop-carried "
                         "memory dependencies");
    } else {
      // TODO (vzakhari 5/17/2018): the presence of the PHI node
      //       does not necessarily mean there is a loop carried
      //       memory dependence.  To make this right, we need
      //       to traverse the loop and check if there are uses
      //       of the PHI nodes.
      OptimizationRemarkMissed Remark(DEBUG_TYPE,
                                      "CSAPipeliningMissed: ", LoopLoc, BB);
      ORE.emit(Remark << " loop with loop-carried memory dependencies "
                         "cannot be pipelined");
    }
  }
}

Pass *llvm::createStandardCSAMemopOrderingPass(const CSATargetMachine *TM) {
  return new CSAMemopOrdering{TM};
}

INITIALIZE_PASS_BEGIN(CSAMemopOrdering, DEBUG_TYPE, PASS_DESC, false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(OptimizationRemarkEmitterWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_END(CSAMemopOrdering, DEBUG_TYPE, PASS_DESC, false, false)

char CSAMemopOrdering::ID = 0;

StringRef CSAMemopOrdering::getPassName() const { return PASS_DESC; }

void CSAMemopOrdering::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
  AU.setPreservesCFG();
  return CSAMemopOrderingBase::getAnalysisUsage(AU);
}

void CSAMemopOrdering::order(Function &F) {
  using namespace std;
  using namespace std::placeholders;

  DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  LoopInfo &LI      = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  OptimizationRemarkEmitter &ORE =
    getAnalysis<OptimizationRemarkEmitterWrapperPass>().getORE();
  ReversePostOrderTraversal<Function *> RPOT{&F};
  bool can_speculate = TM ? TM->getSubtargetImpl(F)->canSpeculate() : false;
  if (not TM) {
    LLVM_DEBUG(
      dbgs() << "No target machine specified; assuming no speculation\n");
  }

  // This memory ordering pass can't handle irreducible loops.
  if (containsIrreducibleCFG<BasicBlock *>(RPOT, LI))
    report_fatal_error("Irreducible CFG located");

  // Try to generate chains with parallel sections if the user has not disabled
  // them. If something goes wrong, print an obnoxious warning message and try
  // again without them.
  MemopCFG mopcfg;
  const function<bool(Instruction &)> needs_ordering_edges =
    bind(&CSAMemopOrdering::needsOrderingEdges, this, _1);
  mopcfg.load(F, AA, DT, LI, RPOT, needs_ordering_edges, can_speculate,
              not IgnoreAnnotations);
  LLVM_DEBUG(dbgs() << "pre ordering:\n\n" << mopcfg);
  if (ViewPreOrderingMemopCFG)
    ViewGraph(mopcfg, F.getName());
  if (not mopcfg.construct_chains()) {
    mopcfg.clear();
    errs() << "\n";
    errs().changeColor(raw_ostream::BLUE, true);
    errs() << "!! WARNING: BAD PARALLEL SECTION INTRINSICS !!";
    errs().resetColor();
    errs() << R"warn(
An inconsistency was discovered when processing parallel section intrinsics for
)warn";
    errs() << F.getName();
    errs() << R"warn(.

Because of this, parallel regions and sections will be ignored for this
function. Please re-examine the parallel section intrinsics in the function to
make sure that they dominate/post-dominate the memory operations in their
sections correctly.

)warn";
    mopcfg.load(F, AA, DT, LI, RPOT, needs_ordering_edges, can_speculate,
                false);
    bool made_chains = mopcfg.construct_chains();
    assert(made_chains);
    (void)made_chains;
  }

  if (DumpMemopCFG)
    errs() << mopcfg;
  if (ViewMemopCFG)
    ViewGraph(mopcfg, F.getName());

  mopcfg.emit_opt_report(ORE);
  emit(mopcfg);
}

Value *CSAMemopOrdering::getOutord(const MemopCFG::OrdToken &OT) {
  using std::back_inserter;
  using std::bind;
  using std::placeholders::_1;
  using OTType = MemopCFG::OrdToken::Type;
  switch (OT.type) {
  case OTType::none:
    return NoneVal;
  case OTType::phi:
    assert(OT.node->phis[OT.idx].I);
    return OT.node->phis[OT.idx].I;
  case OTType::memop:
    assert(OT.node->memops[OT.idx].outord);
    return OT.node->memops[OT.idx].outord;
  case OTType::merge: {
    MemopCFG::Merge &merge = OT.node->merges[OT.idx];
    if (merge.All0)
      return merge.All0;
    SmallVector<Value *, INPUTS_PER_MERGE> Ins;
    Ins.reserve(merge.inputs.size());
    transform(merge.inputs, back_inserter(Ins),
              bind(&CSAMemopOrdering::getOutord, this, _1));
    Instruction *Where = (merge.memop_idx < int(OT.node->memops.size()))
                           ? OT.node->memops[merge.memop_idx].I
                           : OT.node->BB->getTerminator();
    return merge.All0 = createAll0(Ins, Where,
                                   "memop." + Twine{OT.node->topo_num} + "m" +
                                     Twine{OT.idx});
  }
  }
  report_fatal_error("Unexpected OrdToken type value");
}

void CSAMemopOrdering::emit(MemopCFG &mopcfg) {

  // Create all of the phi nodes first; their inputs will be connected later.
  for (const auto &node : mopcfg.nodes) {
    int phi_idx = 0;
    for (MemopCFG::PHI &phi : node->phis) {
      phi.I = createPHI(node->BB, "memop." + Twine{node->topo_num} + "p" +
                                    Twine{phi_idx++});
    }
  }

  // Connect all of the memops.
  for (const auto &node : mopcfg.nodes) {
    int memop_idx = 0;
    for (MemopCFG::Memop &memop : node->memops) {
      if (not memop.outord)
        memop.outord = createOrderingEdges(memop.I, getOutord(memop.ready),
                                           "memop." + Twine{node->topo_num} +
                                             "o" + Twine{memop_idx});
      ++memop_idx;
    }
  }

  // Connect all of the phis.
  for (const auto &node : mopcfg.nodes) {
    for (const MemopCFG::PHI &phi : node->phis) {
      for (int pred_idx = 0; pred_idx < int(node->preds.size()); ++pred_idx) {
        phi.I->addIncoming(getOutord(phi.inputs[pred_idx]),
                           node->preds[pred_idx]->BB);
      }
    }
  }
}
