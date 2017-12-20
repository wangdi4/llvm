//===- CSADepCalcMemopOrdering.cpp - CSA Memory Operation Ordering ----*- C++ -*--===//
//
//===----------------------------------------------------------------------===//
//
// This file implements a machine function pass for the CSA target that
// ensures that memory operations occur in the correct order.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSATargetMachine.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <vector>

using namespace llvm;
using namespace csa_memop_ordering_shared_options;

static cl::opt<bool> LinearOrdering {
  "csa-memop-ordering-linear", cl::Hidden,
  cl::desc("CSA-specific: Ignore any possible parallelism between memops during memop ordering. Really bad for performance, but helpful for tracking down bugs."),
  cl::init(false)
};

static cl::opt<bool> IgnoreAnnotations {
  "csa-memop-ordering-ignore-annotations", cl::Hidden,
  cl::desc("CSA-specific: Ignore parallel region/section annotations during memop ordering."),
  cl::init(false)
};

static cl::opt<bool>
ViewPreOrderingMemopCFG(
  "csa-view-pre-ordering-memop-cfg",
  cl::Hidden,
  cl::desc("CSA-specific: view memop CFG before memory ordering, to help debug bad section intrinsics"),
  cl::init(false)
);

static cl::opt<bool>
ViewDepMemopCFG(
  "csa-view-dep-memop-cfg",
  cl::Hidden,
  cl::desc("CSA-specific: view memop CFG after dependency calculation"),
  cl::init(false)
);

// This value is not for tuning: it is the arity of the merge instruction which
// is used for generating merge trees. Since the instruction used for merges is
// currently the quaternary all0, this must be 4.
constexpr int MERGE_ARITY = 4;

// These values are used for tuning LLVM datastructures; correctness is not at
// stake if they are off.

// A guess at the number of predecessors per basic block.
constexpr unsigned PRED_COUNT = 2;

// A guess at the number of successors per basic block.
constexpr unsigned SUCC_COUNT = 2;

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

// A guess at the typical number of live dependencies.
constexpr unsigned LIVE_DEP_COUNT = 8;

#define DEBUG_TYPE "csa-memop-ordering"

// Memory ordering statistics.
STATISTIC(MemopCount, "Number of memory operations ordered");
STATISTIC(MergeCount, "Number of merges inserted");
STATISTIC(PHICount, "Number of phi nodes inserted");
STATISTIC(AliasQueryCount, "Number of queries made to alias analysis");

namespace {

// The register class we are going to use for all the memory-op
// dependencies.  Technically they could be I0, but I don't know how
// happy LLVM will be with that.
const TargetRegisterClass* MemopRC = &CSA::I1RegClass;

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
    Node* node {nullptr};

    // Its type.
    enum Type {phibit, memop} type;

    // Its index in the correct vector in the node.
    int idx;

    // Some basic constructors.
    Dep() {}
    Dep(Node* node_in, Type type_in, int idx_in)
      : node{node_in}, type{type_in}, idx{idx_in} {}

    // Whether another dependency is implied by this one.
    bool implies(const Dep&) const;
  };

  // A standard type for holding dependencies.
  using DepVec = SmallVector<Dep, LIVE_DEP_COUNT>;

  // A type to represent ordering token values generated from phis, memops, and
  // merges (along with the special <none> value). Since all of those live in
  // (small)vectors in nodes that aren't going to be reordered, they can all be
  // identified by their node pointer and vector index.
  struct OrdToken {

    // The node that the chain element is in.
    Node* node {nullptr};

    // The type of element that this token value is produced by.
    enum Type {none, phi, memop, merge} type {none};

    // The index of the element in its vector.
    int idx;

    // Some basic constructors.
    OrdToken() {}
    OrdToken(Node* node_in, Type type_in, int idx_in)
      : node{node_in}, type{type_in}, idx{idx_in} {}

    // An explicit bool conversion for testing if this token is not <none>.
    explicit operator bool() const {return type;}

    // Retrieves the virtual register number for this token after those have
    // been assigned. This should never be called on <none>.
    unsigned reg_no() const;

    // Dumps the ordering chain for this token. Output is produced at indent
    // level indent and duplicate output is omitted using the seen map. The
    // total number of instructions printed (/elided) under this one is
    // returned.
    int dump_ordering_chain(
      raw_ostream&, std::map<OrdToken, int>& seen, int indent = 0
    ) const;
  };

  // A fragment of a phi node representing an dependency from only one
  // predecessor.
  struct PHIBit {

    // The predecessor that the dependency is coming from.
    Node* pred;

    // The dependency from that predecessor.
    Dep dep;

    // If this phibit or any phibit that it refers to goes across a loop
    // backedge, the Loop corresponding to that. Otherwise, nullptr. Note that
    // by construction a dependency can cross at most one backedge given the
    // model used for calculating them.
    const Loop* loop;

    // Determines the original dependency that this phibit refers to that isn't
    // a phibit.
    Dep orig_dep() const;
  };

  // A phi node for memory ordering tokens.
  struct PHI {

    // All of the inputs to the phi node. Their order corresponds to the order
    // of the predecessor list in the phi node's memop CFG node.
    SmallVector<OrdToken, PRED_COUNT> inputs;

    // This phi's virtual register.
    unsigned reg_no;
  };

  // An n-ary merge of memory ordering tokens.
  struct Merge {

    // The inputs to this merge, in sorted order.
    SmallVector<OrdToken, MERGE_ARITY> inputs;

    // The latest possible location that this merge can be put relative to the
    // memops. If it doesn't need to go before any of the memops in its node,
    // this will be the number of memops in the node.
    int memop_idx;

    // This merge's virtual register.
    unsigned reg_no;

    // If this is nonzero, the merge should not be emitted directly and should
    // be delayed until just before the merge at delayed_emit_idx is emitted.
    // Merges must be topologically sorted when they are emitted in order to
    // make sure that there are no uses before defs; in most instances this is
    // already the case since the original merges produced by the ordering
    // algorithm can't depend on each other and the ones produced during merge
    // tree expansion cannot depend on later merges by construction. However,
    // the original merges are updated in-place during merge tree expansion to
    // depend on later ones, so this member is used to make sure that those
    // don't get emitted until the rest of their tree is. This value
    // monotonically increases within a node, so a normal (non-priority) queue
    // can be used when delaying these merges.
    int delayed_emit_idx = 0;
  };

  // A parallel section intrinsic.
  struct SectionIntrinsic {

    // The index of the first memop after this intrinsic (or the number of
    // memops in the node if there isn't one).
    int memop_idx;

    // The (normalized) id of the region that this section intrinsic belongs
    // to.
    int region;

    // Whether this intrinsic is an entry (true) or exit (false).
    bool is_entry;
  };

  // A holder for all of the relevant information pertaining to a single memop.
  struct Memop {

    // The node containing this memop.
    Node* parent = nullptr;

    // The original instruction.
    MachineInstr* MI = nullptr;

    // Information about where to emit a fence-like sxu mov if MI is nullptr. If
    // is_start is set, this sxu mov produces an ordering token and should be
    // placed at the beginning of the basic block or after a call. Otherwise, it
    // consumes a token and should be placed at the end of the basic block or
    // before the call. call_mi indicates the call that this sxu mov should be
    // placed relative to, or is nullptr if there isn't one.
    bool is_start;
    MachineInstr* call_mi = nullptr;

    // The token for the ready signal for this memory operation, or <none> if
    // the ordering chain for it hasn't been built yet.
    OrdToken ready;

    // This memop's virtual register for the issued signal.
    unsigned reg_no;

    // This memop's explicit and implicit dependencies.
    DepVec exp_deps, imp_deps;

    // How to query the MachineMemOperand for this memory operation. Returns
    // false if it's fence-like.
    const MachineMemOperand* mem_operand() const;

    // Constructs a default fence-like memory operation which needs to be
    // ordered relative to everything. These will have nullptr as their MI and
    // produce nullptr as the result of mem_operand().
    Memop(Node* parent);

    // Construction from an original machine instruction. This machine
    // instruction ought to have a single memory operand.
    Memop(Node* parent, MachineInstr*);

    // Determines whether a given machine instruction should be assigned
    // ordering (loaded into the memop CFG as a Memop). This is the case if it
    // has a memory operand and if its last use and last def are both %ign.
    static bool should_assign_ordering(const MachineInstr& MI);
  };

  // A functor type for computing the path-independent component of the
  // "requires ordering with" relation.
  class RequireOrdering {

    AAResults* AA;
    const MachineFrameInfo* MFI;

  public:

    RequireOrdering() {}

    // The constructor. The functor needs access to the alias information and
    // MachineFrameInfo for the function, so that's supplied here.
    RequireOrdering(AAResults*, const MachineFrameInfo*);

    // Determines whether two memops require ordering. In general, this is the
    // case if they alias and at least one modifies memory. The looped parameter
    // indicates whether a special arbitrary size query should be done to
    // account for the crossing of a loop backedge.
    bool operator()(const Memop&, const Memop&, bool looped) const;
  };

  // A collection of parallel section states.
  struct SectionStates {

    // The state type.
    enum class State {
      no_intrinsics_encountered,
      not_in_section,
      outside_of_section,
      crossed_sections
    };

    // An array of those states, one for each region in the function. The
    // regions are identified by their normalized region ids that are assigned
    // when the function is being loaded, so those can just be used directly
    // as indices when accessing this array.
    SmallVector<State, REGIONS_PER_FUNCTION> states;

    // Whether memory operations should be ignored with the current values in
    // this state set.
    bool should_ignore_memops() const;

    // Transitions the state set given a parallel section intrinsic. If this
    // is an invalid transition, false will be returned.
    bool transition(const SectionIntrinsic&);

    // Whether this parallel section state is incompatible with another one,
    // indicating an incorrect use of parallel section intrinsics.
    bool incompatible_with(const SectionStates&) const;

    // Merges the given state set with this one to make this one more specific.
    // If they are incompatible, this will return false.
    bool merge_from(const SectionStates&);
  };

  // A Node in the MemopCFG, corresponding to a MachineBasicBlock in the
  // MachineFunction CFG.
  struct Node {

    // The original basic block.
    MachineBasicBlock* BB;

    // Predecessors to this node.
    SmallVector<Node*, PRED_COUNT> preds;

    // Successors to this node.
    SmallVector<Node*, SUCC_COUNT> succs;

    // The immediate dominator of this node.
    Node* dominator;

    // If this node is in a loop, the deepest loop that it is in. Otherwise,
    // nullptr. Note that if this node is a loop header this will be the loop
    // it is a header for.
    const Loop* deepest_loop {nullptr};

    // The index of this node according to the topological sort.
    int topo_num;

    // The memory operations in this node. These are in the order in which
    // they appear in the original basic block.
    SmallVector<Memop, MEMOP_COUNT> memops;

    // The parallel section intrinsics. These are also in basic block order
    // and have a field to indicate where they are relative to the memops.
    SmallVector<SectionIntrinsic, SECTION_INTRINSIC_COUNT> section_intrinsics;

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
    const RequireOrdering& require_ordering;

    // The virtual register that this node should use to populate <none> phi
    // arguments. 0 indicates that it has not been calculated yet.
    unsigned none_init_mov = 0;

    // A topological ordering for Node*s.
    struct topo_order {
      bool operator()(const Node* a, const Node* b) {
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
    Node(
      MachineBasicBlock*,
      const RequireOrdering&,
      bool use_parallel_sections,
      std::map<int, int>& normalized_regions
    );

    // Whether this node is strictly dominated by another node.
    bool dominated_by(const Node*) const;

    // Whether this node is non-strictly dominated by another one.
    bool nonstrictly_dominated_by(const Node*) const;

    // Obtains the depset entry corresponding to back_loop. Since all of those
    // allocated ahead of time, there's no risk of invalidating the reference
    // returned by this function.
    DepSetEntry& get_depset(const Loop* back_loop = nullptr);

    // Produces a (possibly new) phibit given a predecessor and a dependency
    // from it.
    Dep get_phibit(Node* pred, const Dep&);

    // Produces a (possibly new) phi given a phi to base it off of.
    OrdToken get_phi(PHI&&);

    // Produces a (possibly new) merge given a merge to base it off of.
    OrdToken get_merge(Merge&&);

    // Whether the predecessor pred should be ignored given the current status
    // of the iteration (ordering a memop in memop_node in the loop loop
    // traversed backwards into back_loop). If this predecessor should not be
    // ignored, pred_back_loop will be set to the correct back_loop value for
    // this predecessor.
    bool ignore_pred(
      const Node* pred, const Node* memop_node, const Loop* loop,
      const Loop* back_loop, const Loop*& pred_back_loop
    ) const;

    // Whether a given dependency is a phibit that can be ignored in this node
    // because it goes across a loop backedge for a loop that this node is not
    // in.
    bool irrelevant_loop_phibit(const Dep&) const;

    // Processes memops in this node to update their dependencies and updates
    // chaintips. Returns false if dependency calculation fails due to bad
    // section intrinsics.
    bool calculate_deps(const Loop*);

    // Updates predecessors' DepSetEntries based on the current DepSetEntry for
    // this node. cur_memop is the memop currently being ordered, loop is the
    // loop that that memop is currently being ordered in, and back_loop is the
    // loop that the backwards traversal is currently in if it had crossed a
    // backedge. Returns false if bad section intrinsics are encountered.
    // Section intrinsic processing will start at memop_idx and continue
    // backwards from there.
    bool step_backwards(
      const Memop& cur_memop, const Loop* loop, const Loop* back_loop,
      int memop_idx
    );

    // Filters predecessor values and adds them to to_connect in this node's
    // current DepSetEntry.
    void step_forwards(
      const Node* memop_node, const Loop* loop, const Loop* back_loop
    );

    // Constructs merges and phi nodes to order each memop according to the
    // dependencies calculated for it.
    void construct_chains();

    // Inserts merges and phi nodes as needed to produce an equivalent value to
    // the the given DepVec before the memop at memop_idx in this node (or at
    // the end if memop_idx is memops.size()).
    OrdToken insert_merges_and_phis(DepVec&, int memop_idx);

    // Expands each merge into a tree of MERGE_ARITY-ary merges.
    void expand_merge_trees();

    // Locates the virtual register output of the mov instruction that should
    // be used as the input to phi nodes that depend on this node but have no
    // real value coming from it.
    unsigned get_none_init_mov_virtreg();

    // Adds phi nodes to the original machine basic block.
    void emit_phis();

    // Adds merges to the original machine basic block.
    void emit_merges();

    // Wires up the original memory operations and emits fence-like movs.
    void emit_memops();
  };

  // A MemopCFG Loop and all related information.
  struct Loop {

    // The set of nodes in this loop, in topological order.
    SmallVector<Node*, NODES_PER_LOOP> nodes;

    // The loop depth of this loop.
    int depth;

    // Whether this loop contains a given node.
    bool contains(const Node*) const;
  };

  // The collection of nodes in this MemopCFG. In general, these will be
  // topologically sorted.
  SmallVector<std::unique_ptr<Node>, NODES_PER_FUNCTION> nodes;

  // The set of loops in this MemopCFG, with subloops preceding their containing
  // loops. These aren't updated after the MemopCFG is loaded, so pointers into
  // this can safely be used during memory ordering.
  SmallVector<Loop, LOOPS_PER_FUNCTION> loops;

  // The total number of different parallel regions represented in the MemopCFG.
  unsigned region_count;

  // The RequireOrdering functor for computing "requires ordering with".
  RequireOrdering require_ordering;

  // A mapping from MachineBasicBlock pointers to Node pointers to make it
  // easier to find nodes given original basic blocks.
  std::map<const MachineBasicBlock*, Node*> nodes_for_bbs;

  // Loads the MemopCFG with memops from a given machine function using the
  // given analysis results. The graph should be empty when this is called, so
  // make sure clear gets called before this is called again to load a different
  // function. If use_parallel_sections is set, parallel section intrinsics will
  // be copied over into the MemopCFG; otherwise, they will be ignored.
  void load(
    MachineFunction&, AAResults*, const MachineDominatorTree*,
    const MachineLoopInfo*,
    bool use_parallel_sections
  );

  // Performs a topological sort of the nodes. This is called inside of load.
  void topological_sort();

  // Recursively collects loop information to build the loops array. This is
  // also called inside of load.
  void collect_loops(const MachineLoop*);

  // Determines implicit dependencies. These aren't really implemented yet, so
  // this currently does nothing. It is also called inside of load.
  void calculate_imp_deps();

  // Unloads/erases the currently-loaded graph.
  void clear();

  // Constructs the ordering chains for all of the memory operations in the
  // MemopCFG. True is returned if this succeeds; otherwise, the MemopCFG should
  // be re-loaded without parallel section intrinsics and tried again.
  bool construct_chains();

  // Expands merge trees and emits the actual ordering chain instructions to the
  // original function.
  void emit_chains();

  // Dumps the memory ordering chains for each memop.
  void dump_ordering_chains(raw_ostream&);
};

// A pass for filling out memory ordering operands on memory operations. Full
// documentation pending future updates.
class CSAMemopOrdering : public MachineFunctionPass {
  bool runOnMachineFunction(MachineFunction &MF) override;

public:
  static char ID;
  CSAMemopOrdering() : MachineFunctionPass(ID) { }
  StringRef getPassName() const override {
    return "CSA Memory Operation Ordering";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AAResultsWrapperPass>();
    AU.addRequired<MachineDominatorTree>();
    AU.addRequired<MachineLoopInfo>();
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  AAResults* AA;
  MachineRegisterInfo* MRI;
  const MachineDominatorTree* DT;
  const MachineLoopInfo* MLI;
  MemopCFG mopcfg;

  // Adds ordering constraints to relevant memops in the given function.
  void addMemoryOrderingConstraints(MachineFunction&);

  // Wipes out all of the intrinsics.
  void eraseParallelIntrinsics(MachineFunction *MF);
};

// Operator<s that provide unique orderings for dependencies and dependency
// token values.
bool operator<(
  const MemopCFG::Dep& a, const MemopCFG::Dep& b
) {

  // Order by owning node first.
  if (a.node->topo_num < b.node->topo_num) return true;
  if (b.node->topo_num < a.node->topo_num) return false;

  // Then by type.
  if (a.type < b.type) return true;
  if (b.type < a.type) return false;

  // Then by index.
  return a.idx < b.idx;
}
bool operator<(
  const MemopCFG::OrdToken& a, const MemopCFG::OrdToken& b
) {
  if (a.node->topo_num < b.node->topo_num) return true;
  if (b.node->topo_num < a.node->topo_num) return false;
  if (a.type < b.type) return true;
  if (b.type < a.type) return false;
  return a.idx < b.idx;
}

// Also, equality operators for both.
bool operator==(const MemopCFG::Dep& a, const MemopCFG::Dep& b) {
  return a.type == b.type and a.node == b.node and a.idx == b.idx;
}
bool operator!=(const MemopCFG::Dep& a, const MemopCFG::Dep& b) {
  return not (a == b);
}
bool operator==(
  const MemopCFG::OrdToken& a, const MemopCFG::OrdToken& b
) {
  return a.type == b.type and (
    not a or (a.node == b.node and a.idx == b.idx)
  );
}
bool operator!=(
  const MemopCFG::OrdToken& a, const MemopCFG::OrdToken& b
) {
  return not (a == b);
}

// -- Various output operators for diagnostics --

raw_ostream& operator<<(raw_ostream& out, const MemopCFG::Dep& dep) {
  using Type = MemopCFG::Dep::Type;
  out << dep.node->topo_num;
  switch (dep.type) {
    case Type::phibit: {
      const MemopCFG::PHIBit& phibit = dep.node->phibits[dep.idx];
      return out << ":" << phibit.pred->topo_num << ">" << phibit.dep;
    }
    case Type::memop:
      return out << "o" << dep.idx;
  }
  llvm_unreachable("expected valid Dep type");
}

raw_ostream& operator<<(raw_ostream& out, const MemopCFG::OrdToken& val) {
  using Type = MemopCFG::OrdToken::Type;
  if (not val) return out << "<none>";
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

raw_ostream& operator<<(raw_ostream& out, const MemopCFG::Memop& memop) {
  if (not memop.MI) out << "mov0";
  else {
    const TargetInstrInfo*const TII
      = memop.MI->getParent()->getParent()->getSubtarget().getInstrInfo();
    out << TII->getName(memop.MI->getOpcode());
  }
  if (memop.exp_deps.empty()) out << " " << memop.ready;
  else {
    bool first = true;
    for (const MemopCFG::Dep& dep : memop.exp_deps) {
      out << (first ? " [" : ", ") << dep;
      first = false;
    }
    out << "]";
  }
  if (not memop.imp_deps.empty()) {
    bool first = true;
    for (const MemopCFG::Dep& dep : memop.imp_deps) {
      out << (first ? " (" : ", ") << dep;
      first = false;
    }
    out << ")";
  }
  return out;
}

raw_ostream& operator<<(raw_ostream& out, const MemopCFG::Merge& merge) {
  using namespace std;
  out << "merge";
  bool first = true;
  for (const MemopCFG::OrdToken& merged : merge.inputs) {
    out << (first ? " " : ", ") << merged;
    first = false;
  }
  if (merge.delayed_emit_idx) out << " [" << merge.delayed_emit_idx << "]";
  return out;
}

raw_ostream& operator<<(raw_ostream& out, const MemopCFG::PHI& phi) {
  out << "phi ";
  bool first = true;
  for (const MemopCFG::OrdToken& phid : phi.inputs) {
    out << (first ? "" : ", ") << phid;
    first = false;
  }
  return out;
}

raw_ostream& operator<<(
  raw_ostream& out, const MemopCFG::SectionIntrinsic& intr
) {
  return out << (intr.is_entry ? "entry " : "exit ") << intr.region;
}

// Writes the body of node to out, starting each line with line_start and ending
// each line with line_end. This is broken out into a separate function so that
// it can be shared between operator<< and the DOTGraphTraits specialization.
void print_body(
  raw_ostream& out, const MemopCFG::Node& node,
  const char* line_start, const char* line_end
) {
  using OrdToken = MemopCFG::OrdToken;

  // A nasty const_cast to be able to reuse OrdToken's operator<<.
  MemopCFG::Node*const node_ptr = const_cast<MemopCFG::Node*>(&node);

  if (not node.phibits.empty()) {
    out << "(" << node.phibits.size() << " phibits)" << line_end;
  }
  for (int phi_idx = 0; phi_idx != int(node.phis.size()); ++phi_idx) {
    out << line_start << OrdToken{node_ptr, OrdToken::phi, phi_idx} << " = "
      << node.phis[phi_idx] << line_end;
  }
  auto cur_intr = node.section_intrinsics.begin();
  for (int memop_idx = 0; memop_idx <= int(node.memops.size()); ++memop_idx) {
    while (
      cur_intr != node.section_intrinsics.end()
        and cur_intr->memop_idx == memop_idx
    ) out << line_start << *cur_intr++ << line_end;
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

raw_ostream& operator<<(raw_ostream& out, const MemopCFG::Node& node) {
  using Node = MemopCFG::Node;
  out << node.topo_num << " (" << node.BB->getNumber();
  if (node.BB->getBasicBlock()) {
    out << " " << node.BB->getBasicBlock()->getName();
  }
  out << "):\npreds:";
  for (const Node*const pred : node.preds) out << " " << pred->topo_num;
  out << "\n";
  print_body(out, node, "  ", "\n");
  out << "succs:";
  for (const Node*const succ : node.succs) out << " " << succ->topo_num;
  return out << "\n";
}

raw_ostream& operator<<(raw_ostream& out, const MemopCFG& cfg) {
  if (not cfg.nodes.empty()) {
    out << cfg.nodes.front()->BB->getParent()->getName() << ":\n\n";
  }
  for (const std::unique_ptr<MemopCFG::Node>& node : cfg.nodes) {
    out << *node << "\n";
  }
  return out;
}

// A simple iterator adaptor which basically forwards all of its operations to
// its base type except that it calls base->get() to dereference instead of just
// doing *base. This allows a range<unique_ptr<T>> to be exposed as a range<T*>.
template <typename BaseIt>
class GetAdaptorIterator {
  BaseIt base;
public:
  GetAdaptorIterator(const BaseIt& base_in) : base{base_in} {}
  decltype(base->get()) operator*() const { return base->get(); }
  decltype(base->get()) operator->() const { return base->get(); }
  GetAdaptorIterator& operator++() { ++base; return *this; }
  friend bool operator==(
    const GetAdaptorIterator& a, const GetAdaptorIterator& b
  ) {
    return a.base == b.base;
  }
  friend bool operator!=(
    const GetAdaptorIterator& a, const GetAdaptorIterator& b
  ) {
    return a.base != b.base;
  }
};

}

namespace llvm {

// A specialization of llvm::GraphTraits for MemopCFG so that LLVM knows how to
// traverse a MemopCFG.
template <> struct GraphTraits<const MemopCFG> {
  using NodeRef = MemopCFG::Node*;
  using ChildIteratorType = decltype(MemopCFG::Node::succs)::const_iterator;
  static NodeRef getEntryNode(const MemopCFG& mopcfg) {
    return mopcfg.nodes.empty() ? nullptr : mopcfg.nodes.front().get();
  }
  static ChildIteratorType child_begin(NodeRef N) { return N->succs.begin(); }
  static ChildIteratorType child_end(NodeRef N) { return N->succs.end(); }

  using nodes_iterator
    = GetAdaptorIterator<decltype(MemopCFG::nodes)::const_iterator>;

  static nodes_iterator nodes_begin(const MemopCFG& mopcfg) {
    return mopcfg.nodes.begin();
  }
  static nodes_iterator nodes_end(const MemopCFG& mopcfg) {
    return mopcfg.nodes.end();
  }

  static unsigned size(const MemopCFG& mopcfg) {
    return mopcfg.nodes.size();
  }
};
template <> struct GraphTraits<MemopCFG> : GraphTraits<const MemopCFG> {};

// Another specialization for llvm::DOTGraphTraits to tell LLVM how to write a
// MemopCFG as a dot graph.
template <> struct DOTGraphTraits<const MemopCFG> : DefaultDOTGraphTraits {

  using DefaultDOTGraphTraits::DefaultDOTGraphTraits;

  // The title of the graph as a whole.
  static std::string getGraphName(const MemopCFG& mopcfg) {
    if (mopcfg.nodes.empty()) return "";
    return "MemopCFG for '"
      + mopcfg.nodes.front()->BB->getParent()->getName().str() + "' function";
  }

  // The label for each node with its number and corresponding IR name.
  static std::string getNodeLabel(
    const MemopCFG::Node* node, const MemopCFG& mopcfg
  ) {
    using namespace std;
    return to_string(node->topo_num) + " (" + to_string(node->BB->getNumber())
      + (node->BB->getBasicBlock()
        ? string{" "} + node->BB->getBasicBlock()->getName().str()
        : string{""}
      ) + ")";
  }

  // The description for each node with the code inside of it.
  static std::string getNodeDescription(
    const MemopCFG::Node* node, const MemopCFG& mopcfg
  ) {
    using namespace std;
    string conts;
    raw_string_ostream sout {conts};
    print_body(sout, *node, "", "\\l");
    return sout.str();
  }

  // Output port labels with successor block numbers.
  template <typename EdgeIter>
  static std::string getEdgeSourceLabel(
    const MemopCFG::Node* node, EdgeIter it
  ) {
    using namespace std;
    return to_string((*it)->topo_num);
  }

  // Input port labels with predecessor block numbers (in the correct order for
  // reading phis)
  static bool hasEdgeDestLabels() { return true; }
  static unsigned numEdgeDestLabels(const MemopCFG::Node* node) {
    return node->preds.size();
  }
  static std::string getEdgeDestLabel(
    const MemopCFG::Node* node, unsigned pred_idx
  ) {
    using namespace std;
    return to_string(node->preds[pred_idx]->topo_num);
  }

  // This is a truly bizarre interface, but this does seem to be the correct way
  // to specify which input ports to map each output port to on its successor
  // node.
  template <typename EdgeIter>
  static bool edgeTargetsEdgeSource(const void*, EdgeIter) {
    return true;
  }
  template <typename EdgeIter>
  static EdgeIter getEdgeTarget(const MemopCFG::Node* node, EdgeIter it) {
    using namespace std;
    const auto found = find((*it)->preds, node);
    assert(found != end((*it)->preds));
    return next(begin((*it)->succs), distance(begin((*it)->preds), found));
  }
};
template <> struct DOTGraphTraits<MemopCFG> : DOTGraphTraits<const MemopCFG> {
  using DOTGraphTraits<const MemopCFG>::DOTGraphTraits;
};

}

char CSAMemopOrdering::ID = 0;

MachineFunctionPass *llvm::createCSADepCalcMemopOrderingPass() {
  return new CSAMemopOrdering();
}

bool CSAMemopOrdering::runOnMachineFunction(MachineFunction &MF) {
  MRI = &MF.getRegInfo();
  AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  DT = &getAnalysis<MachineDominatorTree>();
  MLI = &getAnalysis<MachineLoopInfo>();

  if (OrderMemops) addMemoryOrderingConstraints(MF);

  eraseParallelIntrinsics(&MF);

  return true;
}

void CSAMemopOrdering::addMemoryOrderingConstraints(MachineFunction& MF) {

  // Try to generate chains with parallel sections if the user has not disabled
  // them. If something goes wrong, print an obnoxious warning message and try
  // again without them.
  mopcfg.load(MF, AA, DT, MLI, not IgnoreAnnotations);
  DEBUG(dbgs() << "pre ordering:\n\n" << mopcfg);
  if (ViewPreOrderingMemopCFG) ViewGraph(mopcfg, MF.getName());
  if (not mopcfg.construct_chains()) {
    mopcfg.clear();
    errs() << "\n";
    errs().changeColor(raw_ostream::BLUE, true);
    errs() << "!! WARNING: BAD PARALLEL SECTION INTRINSICS !!";
    errs().resetColor();
    errs() << R"warn(
An inconsistency was discovered when processing parallel section intrinsics for
)warn";
    errs() << MF.getName();
    errs() << R"warn(.

Because of this, parallel regions and sections will be ignored for this
function. Please re-examine the parallel section intrinsics in the function to
make sure that they dominate/post-dominate the memory operations in their
sections correctly.

)warn";
    mopcfg.load(MF, AA, DT, MLI, false);
    bool made_chains = mopcfg.construct_chains();
    assert(made_chains);
  }

  if (DumpMemopCFG) errs() << mopcfg;
  if (DumpOrderingChains) mopcfg.dump_ordering_chains(errs());
  if (ViewMemopCFG) ViewGraph(mopcfg, MF.getName());

  mopcfg.emit_chains();

  mopcfg.clear();
}

bool MemopCFG::Dep::implies(const Dep& that) const {

  // If this is not a phibit...
  if (type != phibit) {

    // If that is also not a phibit, this only implies that if they are the same
    // Dep.
    if (that.type != phibit) return *this == that;

    // Otherwise, this only implies that if that refers to this.
    else return that.node->phibits[that.idx].orig_dep() == *this;
  }

  // If this is a phibit, it cannot imply a non-phibit.
  if (that.type != phibit) return false;

  // Now both Deps have been confirmed to be phibits.
  const PHIBit& this_phibit = node->phibits[idx];
  const PHIBit& that_phibit = that.node->phibits[that.idx];

  // If they are in the same node...
  if (node == that.node) {

    // This cannot imply that if that's predecessor is not dominated by this'
    // predecessor.
    if (not that_phibit.pred->nonstrictly_dominated_by(this_phibit.pred)) {
      return false;
    }

    // If it is, though, it is safe to recurse into both phibits.
    return this_phibit.dep.implies(that_phibit.dep);
  }

  // If they are not in the same node, this cannot imply that if this does not
  // dominate that.
  if (not that.node->dominated_by(node)) return false;

  // If this does dominate that, recurse into that only.
  return implies(that_phibit.dep);
}

unsigned MemopCFG::OrdToken::reg_no() const {
  assert(type);
  switch (type) {
    case Type::phi:   return node->phis[idx].reg_no;
    case Type::memop: return node->memops[idx].reg_no;
    case Type::merge: return node->merges[idx].reg_no;
    default: break;
  }
  llvm_unreachable("Unexpected OrdToken type value");
}

int MemopCFG::OrdToken::dump_ordering_chain(
  raw_ostream& out, std::map<OrdToken, int>& seen, int indent
) const {
  using namespace std;
  assert(type);
  int under = 0;
  out << format("%*d", indent + 1, indent) << " " << *this << " = ";
  switch (type) {
    case Type::phi: {
      const PHI& phi = node->phis[idx];
      out << phi;
      const auto found = seen.find(*this);
      if (found != end(seen)) out << " +" << found->second << "\n";
      else {
        out << "\n";
        for (const OrdToken& phid : phi.inputs) {
          if (phid.type == OrdToken::phi or phid.type == OrdToken::merge) {
            ++under;
            under += phid.dump_ordering_chain(out, seen, indent + 1);
          }
        }
        seen.emplace(*this, under);
      }
    } break;
    case Type::memop: {
      out << node->memops[idx] << "\n";
      const OrdToken& ready = node->memops[idx].ready;
      if (ready.type == Type::phi or ready.type == Type::merge) {
        ++under;
        under += ready.dump_ordering_chain(out, seen, indent + 1);
      }
    } break;
    case Type::merge: {
      const Merge& merge = node->merges[idx];
      out << merge;
      const auto found = seen.find(*this);
      if (found != end(seen)) out << " +" << found->second << "\n";
      else {
        out << "\n";
        for (const OrdToken& merged : merge.inputs) {
          if (merged.type == Type::phi or merged.type == Type::merge) {
            ++under;
            under += merged.dump_ordering_chain(out, seen, indent + 1);
          }
        }
        seen.emplace(*this, under);
      }
    } break;
    default: break;
  }
  return under;
}

MemopCFG::Dep MemopCFG::PHIBit::orig_dep() const {
  return dep.type == Dep::phibit
    ? dep.node->phibits[dep.idx].orig_dep()
    : dep;
}

const MachineMemOperand* MemopCFG::Memop::mem_operand() const {
  return MI ? *MI->memoperands_begin() : nullptr;
}

MemopCFG::Memop::Memop(Node* parent_in) : parent{parent_in} {
  assert(parent);
}

MemopCFG::Memop::Memop(
  Node* parent, MachineInstr* MI_in
) : Memop{parent} {
  MI = MI_in;
  assert(MI and MI->hasOneMemOperand());
}

bool MemopCFG::Memop::should_assign_ordering(const MachineInstr& MI) {
  using namespace std;
  return not MI.memoperands_empty()
    and begin(MI.defs()) != end(MI.defs())
    and begin(MI.uses()) != end(MI.uses())
    and prev(end(MI.defs()))->isReg()
    and prev(end(MI.defs()))->getReg() == CSA::IGN
    and prev(end(MI.uses()))->isReg()
    and prev(end(MI.uses()))->getReg() == CSA::IGN;
}

MemopCFG::RequireOrdering::RequireOrdering(
  AAResults* AA_in, const MachineFrameInfo* MFI_in
) : AA{AA_in}, MFI{MFI_in} {}

bool MemopCFG::RequireOrdering::operator()(
  const Memop& a_memop, const Memop& b_memop, bool looped
) const {

  // Grab the memory operands of both inputs and use those.
  const MachineMemOperand*const a = a_memop.mem_operand();
  const MachineMemOperand*const b = b_memop.mem_operand();

  // If either is nullptr, they need to be ordered.
  if (not a or not b) return true;

  // If both are loads, they don't need ordering. However, if either is a
  // prefetch this check should be ignored.
  const bool neither_is_prefetch =
    a_memop.MI->getOpcode() != CSA::PREFETCH
    and a_memop.MI->getOpcode() != CSA::PREFETCHI
    and a_memop.MI->getOpcode() != CSA::PREFETCHW
    and a_memop.MI->getOpcode() != CSA::PREFETCHWI
    and b_memop.MI->getOpcode() != CSA::PREFETCH
    and b_memop.MI->getOpcode() != CSA::PREFETCHI
    and b_memop.MI->getOpcode() != CSA::PREFETCHW
    and b_memop.MI->getOpcode() != CSA::PREFETCHWI;
  if (neither_is_prefetch and a->isLoad() and b->isLoad()) return false;

  // Otherwise, they only need ordering if they could alias each other. If
  // IgnoreAliasInfo is set, assume that they do.
  if (IgnoreAliasInfo) return true;

  // An instruction (/operand) can always alias itself.
  if (a == b) return true;

  // Check for IR Values and PseudoSourceValues.
  const PseudoSourceValue*const a_pseudo = a->getPseudoValue();
  const PseudoSourceValue*const b_pseudo = b->getPseudoValue();
  const Value*const a_value = a->getValue(), *const b_value = b->getValue();

  // Give up if there's no information about either operand.
  if (not a_pseudo and not a_value) return true;
  if (not b_pseudo and not b_value) return true;

  // If neither is a pseudo value (the most common case, hopefully), query alias
  // analysis.
  if (a_value and b_value) {
    ++AliasQueryCount;
    return not AA->isNoAlias(
      MemoryLocation{
        a_value,
        looped ? MemoryLocation::UnknownSize : a->getSize(),
        a->getAAInfo()
      },
      MemoryLocation{
        b_value,
        looped ? MemoryLocation::UnknownSize : b->getSize(),
        b->getAAInfo()
      }
    );
  }

  // If they're both pseudo values, guess about whether they could alias based
  // on pseudo value kind.
  // TODO: Can we do better than this?
  if (a_pseudo and b_pseudo) return a_pseudo->kind() == b_pseudo->kind();

  // Otherwise, one is an IR value and the other is a pseudo value. Determine
  // whether the pseudo value could alias _any_ IR value and give that as the
  // answer.
  if (a_pseudo) return a_pseudo->isAliased(MFI);
  return b_pseudo->isAliased(MFI);
}

bool MemopCFG::SectionStates::should_ignore_memops() const {
  using namespace std;

  // Memops should be ignored if any section states are crossed_sections.
  return any_of(begin(states), end(states), [](State s) {
    return s == State::crossed_sections;
  });
}

bool MemopCFG::SectionStates::transition(const SectionIntrinsic& intr) {

  // Determine which state needs to update.
  State& to_update = states[intr.region];

  switch (to_update) {

    // For no_intrinsics_encountered, transition to either not_in_section or
    // outside_of_section.
    case State::no_intrinsics_encountered:
      if (intr.is_entry) to_update = State::outside_of_section;
      else to_update = State::not_in_section;
      return true;

    // For not_in_section, no further state transitions need to be taken.
    case State::not_in_section:
      return true;

    // For outside_of_section, exits should transition to crossed_sections and
    // entries should not be encountered.
    case State::outside_of_section:
      if (intr.is_entry) return false;
      else to_update = State::crossed_sections;
      return true;

    // For crossed_sections, entries should transition to outside_of_section and
    // exits should not be encountered.
    case State::crossed_sections:
      if (intr.is_entry) to_update = State::outside_of_section;
      else return false;
      return true;
  }

  return false;
}

bool MemopCFG::SectionStates::incompatible_with(
  const SectionStates& that
) const {
  using namespace std;

  // Look at each pair of states.
  for (int region = 0; region < int(states.size()); ++region) {
    State state_a = states[region];
    State state_b = that.states[region];

    // If they're the same, they clearly aren't incompatible.
    if (state_a == state_b) continue;

    // If they're different and neither is no_intrinsics_encountered, they're
    // incompatible.
    if (
      state_a != State::no_intrinsics_encountered
        and state_b != State::no_intrinsics_encountered
    ) return true;

    // If one is no_intrinsics_encountered and the other is outside_of_section,
    // those are also incompatible.
    if (state_b == State::no_intrinsics_encountered) swap(state_a, state_b);
    if (state_b == State::outside_of_section) return true;

    // Otherwise, this combination is not incompatible.
  }

  return false;
}

bool MemopCFG::SectionStates::merge_from(const SectionStates& that) {
  if (incompatible_with(that)) return false;

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
  while (not disconnected.empty()) disconnected.pop();
  connected.clear();
  first_succ = true;
  to_connect.clear();
  for (DepVec& pc : pred_connected) pc.clear();
  fill(
    begin(section_states.states), end(section_states.states),
    SectionStates::State::no_intrinsics_encountered
  );
}

// Recursively searches for a virtual register def through phi nodes given a
// virtual register number and a set of phi nodes that shouldn't be visited
// again in order to avoid infinite recursion.
static const MachineInstr* find_non_phi_def(
  unsigned vreg,
  std::set<const MachineInstr*>& visited_phis,
  const MachineRegisterInfo& MRI
) {

  // Look up the def of the virtual register.
  const MachineInstr*const def = MRI.getUniqueVRegDef(vreg);
  assert(def && "Unexpected non-SSA-form virtual register");

  // If this is an implicit def, ignore it.
  if (def->isImplicitDef()) return nullptr;

  // If it's not a phi node, we're done.
  if (not def->isPHI()) return def;

  // If it's a phi node that's already been visited, it doesn't need to be
  // visited again. Otherwise, it should be added to the list so that it isn't
  // visited again later.
  if (visited_phis.count(def)) return nullptr;
  visited_phis.insert(def);

  // Explore each of the phi's value operands to look for non-phi defs for them.
  for (unsigned i = 1; i < def->getNumOperands(); i += 2) {
    const MachineOperand& phi_operand = def->getOperand(i);
    if (phi_operand.isReg()) {
      const MachineInstr*const found_def
        = find_non_phi_def(phi_operand.getReg(), visited_phis, MRI);
      if (found_def) return found_def;
    }
  }

  // If this is reached, this register doesn't seem to have a non-phi def for
  // some reason. It probably means that there is some weird constant
  // propagation thing going on.
  return nullptr;
}

// Determines the normalized region id for a given parallel section intrinsic.
static int find_normalized_region(
  const MachineInstr& MI,
  std::map<int, int>& normalized_regions
) {
  std::set<const MachineInstr*> visited_phis;
  const MachineRegisterInfo& MRI = MI.getParent()->getParent()->getRegInfo();

  // In order to find a region entry, a section entry needs to be found first.
  const MachineInstr* section_entry;

  // If the section intrinsic _is_ an entry, that's pretty easy to do.
  if (MI.getOpcode() == CSA::CSA_PARALLEL_SECTION_ENTRY) section_entry = &MI;

  // Otherwise, the section intrinsic is an exit and its operand needs to be
  // traced to find the section entry.
  else {
    assert(MI.getOperand(0).isReg());
    section_entry
      = find_non_phi_def(MI.getOperand(0).getReg(), visited_phis, MRI);
    visited_phis.clear();
  }

  assert(
    section_entry
      and section_entry->getOpcode() == CSA::CSA_PARALLEL_SECTION_ENTRY
  );

  // Now look for the region entry too.
  assert(section_entry->getOperand(1).isReg());
  const MachineInstr*const region_entry = find_non_phi_def(
    section_entry->getOperand(1).getReg(), visited_phis, MRI
  );
  visited_phis.clear();
  assert(
    region_entry
      and region_entry->getOpcode() == CSA::CSA_PARALLEL_REGION_ENTRY
  );

  // The unnormalized region id ought to be the region entry's operand.
  assert(region_entry->getOperand(1).isImm());
  const int unnormalized_id = region_entry->getOperand(1).getImm();

  // Look it up in the map. If there's already an entry, use that. Otherwise,
  // make a new one.
  const auto found = normalized_regions.find(unnormalized_id);
  if (found != normalized_regions.end()) return found->second;
  const int new_id = normalized_regions.size();
  normalized_regions.emplace(unnormalized_id, new_id);
  return new_id;
}

MemopCFG::Node::Node(
  MachineBasicBlock* BB_in,
  const RequireOrdering& require_ordering_in,
  bool use_parallel_sections,
  std::map<int, int>& normalized_regions
) : BB{BB_in}, require_ordering(require_ordering_in) {

  // The corresponding nodes for the other basic blocks won't be known yet until
  // they're all constructed, but at least we know how many there will be now.
  preds.reserve(BB->pred_size());
  succs.reserve(BB->succ_size());

  // If there are no predecessors, add in a fence-like memop for the initial
  // mov0.
  if (BB->pred_empty()) {
    memops.emplace_back(this);
    memops.back().is_start = true;
  }

  // Go through all of the instructions and find the ones that need ordering.
  // Also take care of locating and adding parallel section intrinsics here.
  for (MachineInstr& MI : BB->instrs()) {
    if (Memop::should_assign_ordering(MI)) memops.emplace_back(this, &MI);
    else if (
      use_parallel_sections
        and (MI.getOpcode() == CSA::CSA_PARALLEL_SECTION_ENTRY
          or MI.getOpcode() == CSA::CSA_PARALLEL_SECTION_EXIT)
    ) {
      section_intrinsics.push_back({
        int(memops.size()),
        find_normalized_region(MI, normalized_regions),
        MI.getOpcode() == CSA::CSA_PARALLEL_SECTION_ENTRY
      });
    }
    else if (MI.getOpcode() == CSA::JSR or MI.getOpcode() == CSA::JSRi) {
      memops.emplace_back(this);
      memops.back().call_mi = &MI;
      memops.back().is_start = false;
      memops.emplace_back(this);
      memops.back().call_mi = &MI;
      memops.back().is_start = true;
    }
  }

  // If there are no successors, add in a fence-like memop for the final mov0.
  if (BB->succ_empty()) {
    memops.emplace_back(this);
    memops.back().is_start = false;
  }
}

bool MemopCFG::Node::dominated_by(const Node* possi_dom) const {
  const Node* cur_node = dominator;
  while (cur_node and cur_node != possi_dom) cur_node = cur_node->dominator;
  return cur_node;
}

bool MemopCFG::Node::nonstrictly_dominated_by(const Node* possi_dom) const {
  return this == possi_dom or dominated_by(possi_dom);
}

MemopCFG::Node::DepSetEntry& MemopCFG::Node::get_depset(const Loop* back_loop) {
  return depsets[back_loop ? back_loop->depth : 0];
}

MemopCFG::Dep MemopCFG::Node::get_phibit(Node* pred, const Dep& dep) {

  // Look for an existing phibit.
  for (int idx = 0; idx != int(phibits.size()); ++idx) {
    if (phibits[idx].pred == pred and phibits[idx].dep == dep) {
      return {this, Dep::phibit, idx};
    }
  }

  // If there isn't one, make a new one.
  const int idx = phibits.size();
  const Loop* loop = pred->topo_num < topo_num ? nullptr : deepest_loop;
  if (dep.type == Dep::phibit and dep.node->phibits[dep.idx].loop) {
    assert(not loop);
    loop = dep.node->phibits[dep.idx].loop;
  }
  phibits.push_back({pred, dep, loop});
  return {this, Dep::phibit, idx};
}

MemopCFG::OrdToken MemopCFG::Node::get_phi(PHI&& phi) {
  using namespace std;

  // Look through to see how many unique non-none inputs there are to this phi.
  OrdToken first_set_phi_input;
  bool multiple_set_phi_inputs = false;
  for (const OrdToken& phid : phi.inputs) if (phid) {
    if (not first_set_phi_input) first_set_phi_input = phid;
    else if (phid != first_set_phi_input) multiple_set_phi_inputs = true;
  }

  // If there aren't actually any non-none phi inputs, <none> can just be used
  // directly.
  if (not first_set_phi_input) return {};

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

MemopCFG::OrdToken MemopCFG::Node::get_merge(Merge&& merge) {
  using namespace std;

  // If there's no inputs, the value should be <none>.
  if (merge.inputs.empty()) return {};

  // If there's only one, use that directly.
  if (merge.inputs.size() == 1) return merge.inputs.front();

  // Otherwise, try to find a matching merge. Look all of the way up the
  // dominator tree in case there's a matching one in a dominator too.
  for (Node* cur_dom = this; cur_dom; cur_dom = cur_dom->dominator) {
    for (
      int merge_idx = 0; merge_idx != int(cur_dom->merges.size()); ++merge_idx
    ) {
      if (cur_dom->merges[merge_idx].inputs == merge.inputs) {
        if (cur_dom == this and merges[merge_idx].memop_idx > merge.memop_idx) {
          merges[merge_idx].memop_idx = merge.memop_idx;
        }
        return {cur_dom, OrdToken::merge, merge_idx};
      }
    }
  }

  // Otherwise, make a new merge.
  const int merge_idx = merges.size();
  merges.push_back(move(merge));
  return {this, OrdToken::merge, merge_idx};
}

bool MemopCFG::Node::ignore_pred(
  const Node* pred, const Node* memop_node, const Loop* loop,
  const Loop* back_loop, const Loop*& pred_loop
) const {

  // If this predecessor is outside of either loop or back_loop, it should be
  // ignored.
  if (back_loop and not back_loop->contains(pred)) return true;
  if (loop and not loop->contains(pred)) return true;

  // If this predecessor is a backedge...
  if (pred->topo_num >= topo_num) {

    // If a backedge has already been traversed, this one should be ignored.
    if (back_loop) return true;

    // Since this is a backedge, we would really hope that we're in a loop.
    assert(deepest_loop);

    // Only consider backedges that are exactly one loop level deeper than the
    // current loop depth.
    const int loop_depth = loop ? loop->depth : 0;
    if (deepest_loop->depth != loop_depth + 1) return true;

    // And ones where the loop actually contains the memop.
    if (not deepest_loop->contains(memop_node)) return true;

    // If this predecessor isn't ignored, it should use the header_for loop for
    // back_loop.
    pred_loop = deepest_loop;
  }

  // Otherwise, just propagate back_loop.
  else pred_loop = back_loop;

  return false;
}

bool MemopCFG::Node::irrelevant_loop_phibit(const Dep& dep) const {
  return dep.type == Dep::phibit
    and dep.node->phibits[dep.idx].loop
    and not dep.node->phibits[dep.idx].loop->contains(this);
}

bool MemopCFG::Node::calculate_deps(const Loop* loop) {
  using namespace std;
  using namespace std::placeholders;

  // A node and the back_loop value it was visited with.
  struct NodeLoop {
    Node* node;
    const Loop* back_loop;
    bool operator==(const NodeLoop& that) const {
      return node == that.node and back_loop == that.back_loop;
    }
  };

  // These are ordered by the node's topological order and then by pointer order
  // for back_loop.
  struct node_loop_topo_order {
    bool operator()(const NodeLoop& a, const NodeLoop& b) {
      topo_order top;
      if (top(a.node, b.node)) return true;
      if (top(b.node, a.node)) return false;
      return std::less<const Loop*>{}(a.back_loop, b.back_loop);
    }
  };

  // Calculate the initial chaintips based on the predecessors. Any dependencies
  // that dominate this node are automatically promoted, while phibits are
  // generated for other dependencies.
  DepVec tips;
  for (Node*const pred : preds) {
    const Loop* pred_loop;
    if (ignore_pred(pred, this, loop, nullptr, pred_loop)) continue;
    for (const Dep& dep : pred->chaintips) {
      if (not irrelevant_loop_phibit(dep)) {
        tips.push_back(
          dominated_by(dep.node) ? dep : get_phibit(pred, dep)
        );
      }
    }
  }

  // Make sure that the chaintips are unique. They don't really need to be
  // sorted here, but that's the simplest way to do it.
  sort(begin(tips), end(tips));
  tips.erase(unique(begin(tips), end(tips)), end(tips));

  if (loop) {
    DEBUG(
      dbgs() << loop->depth << " "
        << loop->nodes.front()->topo_num << "-" << loop->nodes.back()->topo_num
        << " "
    );
  } else DEBUG(dbgs() << "0 - ");
  DEBUG(dbgs() << topo_num << ": " << tips.size() << "\n");

  for (int memop_idx = 0; memop_idx != int(memops.size()); ++memop_idx) {
    Memop& memop = memops[memop_idx];

    // If linear ordering is requested, just hook up all of the chaintips
    // directly and move on to the next memop.
    if (LinearOrdering) {
      memop.exp_deps.append(begin(tips), end(tips));
      sort(begin(memop.exp_deps), end(memop.exp_deps));
      memop.exp_deps.erase(
        unique(begin(memop.exp_deps), end(memop.exp_deps)), end(memop.exp_deps)
      );
      tips.assign(1, {this, Dep::memop, memop_idx});
      continue;
    }

    // A queue for nodes to be processed in the backwards traversal.
    priority_queue<NodeLoop, vector<NodeLoop>, node_loop_topo_order> node_queue;

    // The nodes traversed by the backwards traversal, reversed to be in the
    // correct order for the forward traversal.
    SmallVector<NodeLoop, NODES_PER_FUNCTION> nodes_traversed;

    // Start with the current memop as the only connected one, the disconnected
    // queue populated with the chaintip values, and this node on the node
    // queue.
    DepSetEntry& depset = get_depset();
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

      if (
        not cur_nodeloop.node->step_backwards(
          memop, loop, cur_nodeloop.back_loop,
          cur_nodeloop.node == this and not cur_nodeloop.back_loop
            ? memop_idx : cur_nodeloop.node->memops.size()
        )
      ) return false;

      for (Node*const pred : cur_nodeloop.node->preds) {
        const Loop* pred_loop;
        if (cur_nodeloop.node->ignore_pred(
          pred, this, loop, cur_nodeloop.back_loop, pred_loop
        )) continue;
        node_queue.push({pred, pred_loop});
      }
    }
    reverse(begin(nodes_traversed), end(nodes_traversed));

    // Use nodes_traversed to do the forwards traversal.
    for (const NodeLoop cur_nodeloop : nodes_traversed) {
      cur_nodeloop.node->step_forwards(this, loop, cur_nodeloop.back_loop);
    }

    // Pull all of the new dependencies for this memop out of to_connect.
    const DepVec& to_connect = depset.to_connect;
    memop.exp_deps.append(begin(to_connect), end(to_connect));
    sort(begin(memop.exp_deps), end(memop.exp_deps));
    assert(
      unique(begin(memop.exp_deps), end(memop.exp_deps)) == end(memop.exp_deps)
    );

    // Reset all of the DepSetEntries involved.
    for (const NodeLoop cur_nodeloop : nodes_traversed) {
      for (DepSetEntry& depset : cur_nodeloop.node->depsets) depset.clear();
    }

    // Clean out any chaintips that got connected.
    tips.erase(remove_if(begin(tips), end(tips), [&memop](const Dep& dep) {
      return any_of(
        begin(memop.exp_deps), end(memop.exp_deps),
        bind(&Dep::implies, _1, dep)
      ) or any_of(
        begin(memop.imp_deps), end(memop.imp_deps),
        bind(&Dep::implies, _1, dep)
      );
    }), end(tips));

    // Add the current memop to the chaintip set.
    tips.emplace_back(this, Dep::memop, memop_idx);
  }

  chaintips = move(tips);

  return true;
}

bool MemopCFG::Node::step_backwards(
  const Memop& cur_memop, const Loop* loop, const Loop* back_loop,
  int memop_idx
) {
  using namespace std;

  DepSetEntry& depset = get_depset(back_loop);
  DepVec& connected = depset.connected;
  auto& disconnected = depset.disconnected;
  DepVec& to_connect = depset.to_connect;
  auto& pred_connected = depset.pred_connected;

  // Make a copy of section states for processing in this node.
  SectionStates sec_states = depset.section_states;

  // Start up a priority queue for connected dependencies.
  priority_queue<Dep> connected_queue {begin(connected), end(connected)};
  connected.clear();

  // Some helper functions for making dealing with the queues a little nicer.
  const auto top_is_memop_in_node = [this](const priority_queue<Dep>& queue) {
    return not queue.empty() and queue.top().node == this
      and queue.top().type == Dep::memop;
  };
  const auto push_deps_to_queue = [this, &cur_memop](
    const Dep& memop_dep, priority_queue<Dep>& queue
  ) {
    assert(memop_dep.node == this and memop_dep.type == Dep::memop);
    const Memop& memop = memops[memop_dep.idx];
    for (const Dep& dep : memop.exp_deps) {
      if (not cur_memop.parent->irrelevant_loop_phibit(dep)) queue.push(dep);
    }
    for (const Dep& dep : memop.imp_deps) {
      if (not cur_memop.parent->irrelevant_loop_phibit(dep)) queue.push(dep);
    }
  };

  // Take care of any disconnected memops in this node.
  auto cur_intr = find_if(
    section_intrinsics.rbegin(), section_intrinsics.rend(),
    [memop_idx](const SectionIntrinsic& intr) {
      return intr.memop_idx <= memop_idx;
    }
  );
  while (top_is_memop_in_node(disconnected)) {
    const Dep cur_dis = disconnected.top();
    while (not disconnected.empty() and disconnected.top() == cur_dis) {
      disconnected.pop();
    }
    const Memop& dis_memop = memops[cur_dis.idx];

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

    // Handle any relevant section intrinsics.
    while (
      cur_intr != section_intrinsics.rend()
      and cur_intr->memop_idx > cur_dis.idx
    ) {
      if (not sec_states.transition(*cur_intr)) return false;
      ++cur_intr;
    }

    // Otherwise, check whether this memop needs to be ordered with the current
    // one. It does unless one of these applies:
    const bool previously_ordered =
      cur_dis.node->deepest_loop
      and not back_loop
      and (not loop or loop->depth < cur_dis.node->deepest_loop->depth)
      and cur_dis.node->deepest_loop->contains(cur_memop.parent);
    if (

      // The two memops were already ordered in a previous pass through this
      // loop.
      not previously_ordered

      // The section states indicate that no ordering is needed.
      and not sec_states.should_ignore_memops()

      // require_ordering indicates that no ordering is needed.
      and require_ordering(cur_memop, dis_memop, back_loop)

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

  // And any remaining section intrinsics.
  while (cur_intr != section_intrinsics.rend()) {
    if (not sec_states.transition(*cur_intr)) return false;
    ++cur_intr;
  }

  // Populate pred_[dis]connected.
  const auto populate_preds = [this](
    priority_queue<Dep>& queue, decltype(pred_connected)& pred_arrays
  ) {
    while (not queue.empty() and queue.top().node == this) {
      const Dep top = queue.top();
      assert(top.type == Dep::phibit);
      while (not queue.empty() and queue.top() == top) queue.pop();
      const auto found_pred = find(
        begin(preds), end(preds), phibits[top.idx].pred
      );
      assert(found_pred != end(preds));
      pred_arrays[distance(begin(preds), found_pred)].push_back(
        phibits[top.idx].dep
      );
    }

    while (not queue.empty()) {
      const Dep top = queue.top();
      while (not queue.empty() and queue.top() == top) queue.pop();
      for (DepVec& pred_array : pred_arrays) pred_array.push_back(top);
    }
  };
  populate_preds(connected_queue, pred_connected);
  SmallVector<DepVec, PRED_COUNT> pred_disconnected (preds.size());
  populate_preds(disconnected, pred_disconnected);

  // Make sure to_connect gets sorted.
  sort(begin(to_connect), end(to_connect));

  // Update the predecessors.
  DepVec tmp;
  for (int pred_idx = 0; pred_idx != int(preds.size()); ++pred_idx) {
    Node*const pred = preds[pred_idx];
    const Loop* pred_loop;
    if (
      ignore_pred(pred, cur_memop.parent, loop, back_loop, pred_loop)
    ) continue;
    DepSetEntry& pred_depset = pred->get_depset(pred_loop);
    DepVec& pc = pred_connected[pred_idx];
    sort(begin(pc), end(pc));
    if (pred_depset.first_succ) {
      pred_depset.connected = pc;
      pred_depset.section_states = sec_states;
      pred_depset.first_succ = false;
    } else {
      if (not pred_depset.section_states.merge_from(sec_states)) return false;
      set_intersection(
        begin(pred_depset.connected), end(pred_depset.connected),
        begin(pc), end(pc),
        back_inserter(tmp)
      );
      swap(pred_depset.connected, tmp);
      tmp.clear();
    }
    for (const Dep& dep : pred_disconnected[pred_idx]) {
      pred_depset.disconnected.push(dep);
    }
  }
  return true;
}

void MemopCFG::Node::step_forwards(
  const Node* memop_node, const Loop* ord_loop, const Loop* back_loop
) {
  using namespace std;

  DepSetEntry& depset = get_depset(back_loop);
  DepVec& to_connect = depset.to_connect;
  auto& pred_connected = depset.pred_connected;

  // Go through and pick out the common dependencies from predecessors'
  // to_connect fields.
  DepVec commons;
  DepVec tmp;
  bool first_pred = true;
  for (Node*const pred : preds) {
    const Loop* pred_loop;
    if (ignore_pred(pred, memop_node, ord_loop, back_loop, pred_loop)) continue;
    const DepVec& pred_to_connect = pred->get_depset(pred_loop).to_connect;
    if (first_pred) {
      commons = pred_to_connect;
      first_pred = false;
    } else {
      set_intersection(
        begin(commons), end(commons),
        begin(pred_to_connect), end(pred_to_connect),
        back_inserter(tmp)
      );
      swap(tmp, commons);
      tmp.clear();
    }
  }

  // Get rid of common dependencies that do not dominate this node and add the
  // rest to to_connect.
  commons.erase(remove_if(
    begin(commons), end(commons), [this](const Dep& dep) {
      return not dominated_by(dep.node);
    }
  ), end(commons));
  to_connect.append(begin(commons), end(commons));

  // Round up all of the non-common inputs which are not in pred_connected.
  for (int pred_idx = 0; pred_idx != int(preds.size()); ++pred_idx) {
    Node*const pred = preds[pred_idx];
    const Loop* pred_loop;
    if (ignore_pred(pred, memop_node, ord_loop, back_loop, pred_loop)) continue;
    const DepVec& pred_to_connect = pred->get_depset(pred_loop).to_connect;
    for (const Dep& dep : pred_to_connect) {
      if (
        not binary_search(begin(commons), end(commons), dep)
        and not binary_search(
          begin(pred_connected[pred_idx]), end(pred_connected[pred_idx]), dep
        )
      ) to_connect.push_back(get_phibit(pred, dep));
    }
    pred_connected[pred_idx].clear();
  }

  // Make sure that to_connect is sorted and unique.
  sort(begin(to_connect), end(to_connect));
  to_connect.erase(unique(begin(to_connect), end(to_connect)), end(to_connect));
}

void MemopCFG::Node::construct_chains() {
  using namespace std;
  for (int memop_idx = 0; memop_idx != int(memops.size()); ++memop_idx) {
    memops[memop_idx].ready = insert_merges_and_phis(
      memops[memop_idx].exp_deps, memop_idx
    );
  }
}

MemopCFG::OrdToken MemopCFG::Node::insert_merges_and_phis(
  DepVec& deps, int memop_idx
) {
  using namespace std;

  // The merge that is being constructed.
  Merge merge;
  merge.memop_idx = memop_idx;

  // Separate out phibits that aren't in this node and have those nodes handle
  // them instead.
  const auto external_phibit_split = partition(
    begin(deps), end(deps), [this](const Dep& dep) {
      return dep.type != Dep::phibit or dep.node == this;
    }
  );
  if (external_phibit_split != end(deps)) {
    sort(external_phibit_split, end(deps));
    SmallVector<OrdToken, LIVE_DEP_COUNT> ext_phis;
    for (auto exb = external_phibit_split; exb != end(deps);) {
      const auto exe = find_if(
        exb, end(deps),
        [exb](const Dep& dep) {
          return dep.node != exb->node;
        }
      );
      DepVec external_deps {exb, exe};
      ext_phis.push_back(exb->node->insert_merges_and_phis(
        external_deps, exb->node->memops.size()
      ));
      exb = exe;
    }
    deps.erase(external_phibit_split, end(deps));
    merge.inputs.append(begin(ext_phis), end(ext_phis));
  }

  // Separate out phibits for each predecessor and recurse to handle those.
  PHI phi;
  const auto phibit_split = partition(
    begin(deps), end(deps),
    [](const Dep& dep) {
      return dep.type != Dep::phibit;
    }
  );
  if (phibit_split != end(deps)) {
    for (Node*const pred : preds) {
      DepVec pred_deps;
      for (auto it = phibit_split; it != end(deps); ++it) {
        assert(it->type == Dep::phibit and it->node == this);
        if (phibits[it->idx].pred == pred) {
          pred_deps.push_back(phibits[it->idx].dep);
        }
      }
      phi.inputs.push_back(
        pred->insert_merges_and_phis(pred_deps, pred->memops.size())
      );
    }
    deps.erase(phibit_split, end(deps));
    merge.inputs.push_back(get_phi(move(phi)));
  }

  // Translate the remaining dependencies in deps to the merge inputs.
  transform(
    begin(deps), end(deps), back_inserter(merge.inputs), [](const Dep& dep) {
      assert(dep.type == Dep::memop);
      return OrdToken{dep.node, OrdToken::memop, dep.idx};
    }
  );
  deps.clear();

  // Make a merge.
  sort(begin(merge.inputs), end(merge.inputs));
  return get_merge(move(merge));
}

void MemopCFG::Node::expand_merge_trees() {
  using namespace std;

  // Go through each of the existing merges.
  SmallVector<OrdToken, MERGE_ARITY> new_inputs;
  const int orig_merge_count = merges.size();
  for (int merge_idx = 0; merge_idx != orig_merge_count; ++merge_idx) {

    // If a merge is already small enough, there's no reason to expand it to a
    // tree.
    const int input_count = merges[merge_idx].inputs.size();
    if (input_count <= MERGE_ARITY) continue;

    // In order to keep maximum latencies down, the tree should be as balanced
    // as possible. This means that each level of the tree except for possibly
    // the leaf level must be completely full, so the total capacity of the leaf
    // level is the smallest power of MERGE_ARITY that can accomodate all of the
    // inputs.
    int total_leaf_capacity = MERGE_ARITY;
    while (total_leaf_capacity < input_count) total_leaf_capacity *= MERGE_ARITY;

    // There may be empty slots in the leaf level. If we pair MERGE_ARITY-1
    // empty slots with any input, we can move it down a level closer to the
    // root. Figure out how many times this can be done.
    const int empty_slot_count = total_leaf_capacity - input_count;
    const int under_leaf_count = empty_slot_count/(MERGE_ARITY - 1);

    // Combine the remaining inputs to form the merges that are actually present
    // at the leaf level of the tree.
    for (
      int comb_start = 0;
      comb_start < input_count - under_leaf_count;
      comb_start += MERGE_ARITY
    ) {
      Merge new_merge;
      new_merge.memop_idx = merges[merge_idx].memop_idx;
      const int to_comb
        = min(MERGE_ARITY, input_count - under_leaf_count - comb_start);
      const auto cur_start = begin(merges[merge_idx].inputs) + comb_start;
      new_merge.inputs.assign(cur_start, cur_start + to_comb);
      new_inputs.push_back(get_merge(move(new_merge)));
    }

    // Then add the inputs that are getting moved down a level and swap them
    // into the merge.
    {
      const auto orig_end = end(merges[merge_idx].inputs);
      new_inputs.append(orig_end - under_leaf_count, orig_end);
      swap(merges[merge_idx].inputs, new_inputs);
      new_inputs.clear();
    }

    // Now the merge inputs should be a power of MERGE_ARITY. Keep combining
    // them until the merge size has reached MERGE_ARITY.
    while (int(merges[merge_idx].inputs.size()) > MERGE_ARITY) {
      const int cur_input_count = int(merges[merge_idx].inputs.size());
      assert(cur_input_count%MERGE_ARITY == 0 && "Not a power of MERGE_ARITY?");
      for (
        int comb_start = 0;
        comb_start < cur_input_count;
        comb_start += MERGE_ARITY
      ) {
        Merge new_merge;
        new_merge.memop_idx = merges[merge_idx].memop_idx;
        const auto cur_start = begin(merges[merge_idx].inputs) + comb_start;
        new_merge.inputs.assign(cur_start, cur_start + MERGE_ARITY);
        new_inputs.push_back(get_merge(move(new_merge)));
      }
      swap(merges[merge_idx].inputs, new_inputs);
      new_inputs.clear();
    }

    // Make sure that the original merge gets delayed until after the merges
    // that make up the tree are emitted so that its uses don't come before
    // their defs.
    merges[merge_idx].delayed_emit_idx = merges.size();
  }
}

unsigned MemopCFG::Node::get_none_init_mov_virtreg() {

  // No new instruction is needed if one is already present or if there is a
  // dominator that should handle that instead.
  if (none_init_mov) return none_init_mov;
  if (dominator) return none_init_mov = dominator->get_none_init_mov_virtreg();

  const CSAInstrInfo* TII = static_cast<const CSAInstrInfo*>(
    BB->getParent()->getSubtarget().getInstrInfo()
  );
  MachineRegisterInfo& MRI = BB->getParent()->getRegInfo();

  // Otherwise, a new mov will need to be added.
  none_init_mov = MRI.createVirtualRegister(MemopRC);
  BuildMI(
    *BB, BB->getFirstNonPHI(), DebugLoc{},
    TII->get(CSA::MOV1), none_init_mov
  ).addImm(0);
  return none_init_mov;
}

void MemopCFG::Node::emit_phis() {
  const CSAInstrInfo* TII = static_cast<const CSAInstrInfo*>(
    BB->getParent()->getSubtarget().getInstrInfo()
  );
  for (const PHI& phi : phis) {
    MachineInstrBuilder phi_instr = BuildMI(
      *BB, BB->getFirstNonPHI(), DebugLoc{}, TII->get(CSA::PHI), phi.reg_no
    );
    for (int pred_idx = 0; pred_idx < int(preds.size()); ++pred_idx) {
      const OrdToken& phid = phi.inputs[pred_idx];
      if (phid) phi_instr.addUse(phid.reg_no());
      else {

        // Machine phi nodes can't just have immediate inputs, so find a <none>
        // init mov instead.
        phi_instr.addUse(preds[pred_idx]->get_none_init_mov_virtreg());

      }
      phi_instr.addMBB(preds[pred_idx]->BB);
    }
    ++PHICount;
  }
}

void MemopCFG::Node::emit_merges() {
  using namespace std;
  const CSAInstrInfo* TII = static_cast<const CSAInstrInfo*>(
    BB->getParent()->getSubtarget().getInstrInfo()
  );

  // Go through each of the memory operation indeces that merges could be put in
  // front of.
  for (int memop_idx = 0; memop_idx <= int(memops.size()); ++memop_idx) {

    // Figure out where to insert new merges. This will be in front of the
    // current instruction if there is one; otherwise, this will be nullptr
    // which indicates that it should be put at the end. If the instruction is
    // an initial mov0, there should never be any merges in front of it.
    MachineInstr* insert_pt = memop_idx != int(memops.size())
      ? memops[memop_idx].MI
      : nullptr;

    // The logic for emitting a merge. Each of the inputs that is set is filled
    // out first and the rest are padded with %ign.
    const auto emit_merge = [this, TII, insert_pt](const Merge& merge) {
      MachineInstrBuilder merge_instr = insert_pt
        ? BuildMI(*BB, insert_pt, DebugLoc{}, TII->get(CSA::ALL0), merge.reg_no)
        : BuildMI(
            *BB, BB->getFirstTerminator(), DebugLoc{},
            TII->get(CSA::ALL0), merge.reg_no
          );
      for (const OrdToken& merged : merge.inputs) {
        merge_instr.addUse(merged.reg_no());
      }
      while (merge_instr->getNumOperands() < 1 + MERGE_ARITY) {
        merge_instr.addUse(CSA::IGN);
      }
      ++MergeCount;
    };

    // Go through all of the merges and emit the ones that need to go here.
    // Throw any that need to be delayed onto the queue until they can also be
    // emitted.
    queue<int> delayed_emit_queue;
    for (int merge_idx = 0; merge_idx < int(merges.size()); ++merge_idx) {
      if (merges[merge_idx].memop_idx != memop_idx) continue;
      while (
        not delayed_emit_queue.empty()
          and merges[delayed_emit_queue.front()].delayed_emit_idx < merge_idx
      ) {
        emit_merge(merges[delayed_emit_queue.front()]);
        delayed_emit_queue.pop();
      }
      if (merges[merge_idx].delayed_emit_idx) {
        delayed_emit_queue.push(merge_idx);
      } else emit_merge(merges[merge_idx]);
    }
    while (not delayed_emit_queue.empty()) {
      emit_merge(merges[delayed_emit_queue.front()]);
      delayed_emit_queue.pop();
    }
  }
}

void MemopCFG::Node::emit_memops() {
  using namespace std;
  const CSAInstrInfo* TII = static_cast<const CSAInstrInfo*>(
    BB->getParent()->getSubtarget().getInstrInfo()
  );
  const unsigned mov_opcode = TII->getMemTokenMOVOpcode();
  for (const Memop& memop : memops) {

    // If this memop has a corresponding instruction, that instruction should
    // be updated.
    if (memop.MI) {
      prev(end(memop.MI->defs()))->ChangeToRegister(memop.reg_no, true);
      if (memop.ready) {
        prev(end(memop.MI->uses()))->ChangeToRegister(
          memop.ready.reg_no(), false
        );
      }
      ++MemopCount;
    }

    // Otherwise, a new sxu mov0 should be emitted.
    else {

      // If is_start is set, it goes at the beginning of the block or after the
      // call. RA is used as an input to make sure that it doesn't get hoisted.
      if (memop.is_start) {
        const MachineBasicBlock::iterator where = memop.call_mi
          ? (
            memop.call_mi->getNextNode()
              ? memop.call_mi->getNextNode()
              : BB->getFirstTerminator()
          )
          : BB->getFirstNonPHI();
        BuildMI(
          *BB, where, DebugLoc{}, TII->get(mov_opcode), memop.reg_no
        ).addUse(CSA::RA);
      }

      // Otherwise, it goes at the end or before the call.
      else {
        const MachineBasicBlock::iterator where = memop.call_mi
          ? memop.call_mi
          : BB->getFirstTerminator();
        BuildMI(
          *BB, where, DebugLoc{}, TII->get(mov_opcode),
          memop.reg_no
        ).addUse(memop.ready ? memop.ready.reg_no() : unsigned(CSA::IGN));
      }
    }
  }
}

bool MemopCFG::Loop::contains(const Node* node) const {
  using namespace std;
  return binary_search(begin(nodes), end(nodes), node, Node::topo_order{});
}

void MemopCFG::load(
  MachineFunction& MF, AAResults* AA, const MachineDominatorTree* DT,
  const MachineLoopInfo* MLI,
  bool use_parallel_sections
) {
  using namespace std;
  using namespace std::placeholders;

  // Update require_ordering.
  require_ordering = RequireOrdering{AA, &MF.getFrameInfo()};

  // Create all of the nodes from the basic blocks.
  {
    map<int, int> normalized_regions;
    for (MachineBasicBlock& BB : MF) {
      nodes.push_back(unique_ptr<Node>{new Node{
        &BB, require_ordering, use_parallel_sections, normalized_regions
      }});
      nodes_for_bbs.emplace(&BB, nodes.back().get());
    }

    // The number of regions can now be determined from normalized_regions.
    region_count = normalized_regions.size();
  }

  // Go through and wire up all of the predecessors, successors and dominators.
  for (const unique_ptr<Node>& pred : nodes) {
    for (const MachineBasicBlock*const succ_bb : pred->BB->successors()) {
      Node*const succ = nodes_for_bbs[succ_bb];
      pred->succs.push_back(succ);
      succ->preds.push_back(pred.get());
    }
    if (
      const MachineDomTreeNode*const dom_node = DT->getNode(pred->BB)->getIDom()
    ) pred->dominator = nodes_for_bbs[dom_node->getBlock()];
    else pred->dominator = nullptr;
  }

  topological_sort();

  // Collect all of the loops and set loop information for the nodes
  // appropriately.
  for (const MachineLoop*const L : *MLI) collect_loops(L);
  for (const Loop& loop : loops) for (Node*const node : loop.nodes) {
    if (not node->deepest_loop) node->deepest_loop = &loop;
  }

  // Also preallocate all of the DepSetEntries
  for (const unique_ptr<Node>& node : nodes) {
    node->depsets.resize(
      node->deepest_loop ? node->deepest_loop->depth + 1 : 1
    );
    for (Node::DepSetEntry& depset : node->depsets) {
      depset.pred_connected.resize(node->preds.size());
      depset.section_states.states.resize(region_count);
    }
  }

  calculate_imp_deps();
}

void MemopCFG::topological_sort() {
  using namespace std;

  // Determine how many non-backedge predecessors each node has that need to be
  // processed before it is.
  map<Node*, int> preds_left;
  for (const unique_ptr<Node>& node : nodes) {
    preds_left.emplace(
      node.get(),
      count_if(begin(node->preds), end(node->preds), [&node](const Node* pred) {
        return not pred->nonstrictly_dominated_by(node.get());
      })
    );
  }

  // Start with nodes that have no predecessors.
  stack<Node*> ready;
  for (const auto& pl : preds_left) if (pl.second == 0) ready.push(pl.first);

  // Assign ordering to nodes on the ready stack until there are none left.
  int topo_num = 0;
  while (not ready.empty()) {
    Node*const node = ready.top();
    ready.pop();
    node->topo_num = topo_num++;

    // Update the predecessor count for non-backedge successors.
    for (Node*const succ : node->succs) {
      if (node->nonstrictly_dominated_by(succ)) continue;
      int& succ_pl = preds_left[succ];
      assert(succ_pl > 0);
      --succ_pl;

      // If this was the last required predecessor, this successor is ready to
      // be ordered.
      if (succ_pl == 0) ready.push(succ);
    }
  }

  // If any nodes have predecessors remaining, that's a problem. It probably
  // means that the CFG wasn't really that irreducible.
  for (const auto& pl : preds_left) assert(pl.second == 0);

  // Reorder the nodes array so that the indices correspond to topo_num.
  for (int i = 0; i != int(nodes.size()); ++i) {
    while (nodes[i]->topo_num != i) swap(nodes[i], nodes[nodes[i]->topo_num]);
  }
}

void MemopCFG::collect_loops(const MachineLoop* L) {
  using namespace std;

  // Do a post-order traversal so that loops are listed in the right order.
  for (const MachineLoop*const subloop : L->getSubLoops()) {
    collect_loops(subloop);
  }

  // Create a new Loop and transfer the set of basic blocks.
  Loop new_loop;
  new_loop.depth = L->getLoopDepth();
  for (const MachineBasicBlock*const BB : L->getBlocks()) {
    new_loop.nodes.push_back(nodes_for_bbs[BB]);
  }
  sort(begin(new_loop.nodes), end(new_loop.nodes), [](Node* a, Node* b) {
    return a->topo_num < b->topo_num;
  });
  loops.push_back(move(new_loop));
}

void MemopCFG::calculate_imp_deps() {
  using namespace std;

  /*
  // NOTE: Experimental code for calculating data dependencies. Do not resurrect
  // until we get a good answer as to exactly which ones we are allowed to use.

  const TargetRegisterInfo*const TRI
    = nodes.front()->BB->getParent()->getSubtarget().getRegisterInfo();
  const MachineRegisterInfo& MRI = nodes.front()->BB->getParent()->getRegInfo();

  // Keep track of which machine instructions have corresponding memops.
  map<const MachineInstr*, OrdToken> memops_for_mis;
  for (const unique_ptr<Node>& node : nodes) {
    for (int idx = 0; idx != int(node->memops.size()); ++idx) {
      if (node->memops[idx].MI) {
        memops_for_mis.emplace(
          node->memops[idx].MI, OrdToken{node.get(), OrdToken::memop, idx}
        );
      }
    }
  }

  // And keep track of which memops each machine instruction depends on.
  map<const MachineInstr*, set<OrdToken>> memop_deps;

  // Adds dependencies to memop_deps for machine instructions in a given node's
  // basic block.
  const auto collect_deps = [this, &memops_for_mis, &memop_deps, TRI, &MRI](
    Node* node
  ) {
    const MachineBasicBlock*const BB = node->BB;
    for (const MachineInstr& MI : *BB) {
      if (MI.isPHI()) {
        for (unsigned i = 1; i < MI.getNumOperands(); i += 2) {
          assert(i+1 < MI.getNumOperands());
          assert(MI.getOperand(i+1).isMBB());
          Node*const pred = nodes_for_bbs[MI.getOperand(i+1).getMBB()];
          const MachineOperand& op = MI.getOperand(i);
          if (
            not op.isReg() or not op.isUse()
            or not TRI->isVirtualRegister(op.getReg())
          ) continue;
          const MachineInstr*const def = MRI.getUniqueVRegDef(op.getReg());
          if (not def) continue;
          const auto found_memop = memops_for_mis.find(def);
          if (found_memop != end(memops_for_mis)) {
            memop_deps[&MI].insert(node->get_phibit(pred, found_memop->second));
          } else {
            const auto& def_deps = memop_deps[def];
            auto& deps = memop_deps[&MI];
            for (const OrdToken& dep : def_deps) {
              deps.insert(node->get_phibit(pred, dep));
            }
          }
        }
      } else {
        for (const MachineOperand& op : MI.operands()) {
          if (
            not op.isReg() or not op.isUse()
            or not TRI->isVirtualRegister(op.getReg())
          ) continue;
          const MachineInstr*const def = MRI.getUniqueVRegDef(op.getReg());
          if (not def) continue;
          const auto found_memop = memops_for_mis.find(def);
          if (found_memop != end(memops_for_mis)) {
            memop_deps[&MI].insert(found_memop->second);
          } else {
            const auto& def_deps = memop_deps[def];
            memop_deps[&MI].insert(begin(def_deps), end(def_deps));
          }
        }
      }
    }
  };

  // Calculate dependencies in loops first.
  for (Loop& loop : loops) for (Node*const node : loop.nodes) {
    collect_deps(node);
  }

  // Then, go through the entire function.
  for (const unique_ptr<Node>& node : nodes) collect_deps(node.get());

  // Fill out the imp_deps fields on all of the memops.
  for (const unique_ptr<Node>& node : nodes) for (Memop& memop : node->memops) {
    auto& deps = memop_deps[memop.MI];

    // Go ahead and eliminate redundant phibits first, though.
    for (auto it = begin(deps); it != end(deps);) {
      if (
        it->type == OrdToken::phibit
        and deps.count(it->node->phibits[it->idx].orig_val())
      ) it = deps.erase(it);
      else ++it;
    }

    memop.imp_deps.assign(begin(deps), end(deps));
  }
  */
}

void MemopCFG::clear() {
  nodes.clear();
  loops.clear();
  nodes_for_bbs.clear();
}

bool MemopCFG::construct_chains() {
  using namespace std;

  // Calculate dependencies.
  for (const Loop& loop : loops) {
    for (Node*const node : loop.nodes) {
      if (not node->calculate_deps(&loop)) return false;
    }
  }
  for (const unique_ptr<Node>& node : nodes) {
    if (not node->calculate_deps(nullptr)) return false;
  }

  DEBUG(dbgs() << "after dependency calculation:\n\n" << *this);
  if (ViewDepMemopCFG) {
    assert(not nodes.empty());
    ViewGraph(*this, nodes.front()->BB->getParent()->getName());
  }

  // Construct chains based on those.
  for (const unique_ptr<Node>& node : nodes) node->construct_chains();

  DEBUG(dbgs() << "after chain construction:\n\n" << *this);

  return true;
}

void MemopCFG::emit_chains() {

  // Expand all of the merge trees.
  for (const std::unique_ptr<Node>& node : nodes) {
    node->expand_merge_trees();
  }

  DEBUG(dbgs() << "after merge expansion:\n\n" << *this);

  // All of the extra ordering instructions should be ready now. Assign virtual
  // registers to everything.
  MachineRegisterInfo* MRI = &nodes.front()->BB->getParent()->getRegInfo();
  for (const std::unique_ptr<Node>& node : nodes) {
    for (PHI& phi : node->phis) {
      phi.reg_no = MRI->createVirtualRegister(MemopRC);
    }
    for (Memop& memop : node->memops) {

      // Terminating mov0 memops need registers with a special class in order
      // to make sure they're on the SXU.
      if (not memop.MI and not memop.is_start) {
          memop.reg_no = MRI->createVirtualRegister(&CSA::RI1RegClass);
      } else memop.reg_no = MRI->createVirtualRegister(MemopRC);
    }
    for (Merge& merge : node->merges) {
      merge.reg_no = MRI->createVirtualRegister(MemopRC);
    }
  }

  // Emit all of the phi nodes, memops, and merges.
  for (const std::unique_ptr<Node>& node : nodes) {
    node->emit_phis();
    node->emit_merges();
    node->emit_memops();
  }
}

void MemopCFG::dump_ordering_chains(raw_ostream& out) {
  using namespace std;
  map<OrdToken, int> seen;
  for (const unique_ptr<Node>& node : nodes) {
    for (int memop_idx = 0; memop_idx != int(node->memops.size()); ++memop_idx) {
      OrdToken{node.get(), OrdToken::memop, memop_idx}
        .dump_ordering_chain(out, seen);
    }
  }
}

void CSAMemopOrdering::eraseParallelIntrinsics(MachineFunction *MF){
  bool needDeadPHIRemoval = false;
  std::set<MachineInstr*> toErase;
  for(MachineBasicBlock &mbb : *MF)
    for(MachineInstr &mi : mbb)
      if (mi.getOpcode() == CSA::CSA_PARALLEL_SECTION_ENTRY ||
          mi.getOpcode() == CSA::CSA_PARALLEL_SECTION_EXIT  ||
          mi.getOpcode() == CSA::CSA_PARALLEL_REGION_ENTRY  ||
          mi.getOpcode() == CSA::CSA_PARALLEL_REGION_EXIT) {
        toErase.insert(&mi);
        // Any token users should also go away.
        for (MachineInstr &tokenUser :
            MRI->use_nodbg_instructions(mi.getOperand(0).getReg()))
          toErase.insert(&tokenUser);
      }

  for(MachineInstr* mi : toErase) {
    mi->eraseFromParentAndMarkDBGValuesForRemoval();
    needDeadPHIRemoval = true;
  }

  // We've removed all of the intrinsics, but their tokens may have been
  // flowing through PHI nodes. Look for dead PHI nodes and remove them.
  while (needDeadPHIRemoval) {
    needDeadPHIRemoval= false;
    toErase.clear();
    for(MachineBasicBlock &mbb: *MF)
      for(MachineInstr &mi : mbb)
        if (mi.isPHI() && mi.getOperand(0).isReg() &&
            MRI->use_nodbg_empty(mi.getOperand(0).getReg()))
          toErase.insert(&mi);
    for(MachineInstr *mi : toErase) {
      mi->eraseFromParentAndMarkDBGValuesForRemoval();
      needDeadPHIRemoval = true;
    }
  }
}
