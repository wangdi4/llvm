//===- CSAIndependentMemopOrdering.cpp - CSA Memory Operation Ordering ----*- C++ -*--===//
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
#include <vector>

#include <time.h>

using namespace llvm;
using namespace csa_memop_ordering_shared_options;

static cl::opt<bool>
IgnoreAliasInfo(
  "csa-memop-ordering-ignore-aa",
  cl::Hidden,
  cl::desc("CSA-specific: ignore alias analysis results when constructing ordering chains and assume everything aliases."),
  cl::init(false)
);

static cl::opt<bool>
ViewMemopCFG(
  "csa-view-memop-cfg",
  cl::Hidden,
  cl::desc("CSA-specific: view memop CFG"),
  cl::init(false)
);

static cl::opt<bool>
DumpMemopCFG(
  "csa-dump-memop-cfg",
  cl::Hidden,
  cl::desc("CSA-specific: dump memop CFG"),
  cl::init(false)
);

static cl::opt<bool>
DumpOrderingChains(
  "csa-dump-ordering-chains",
  cl::Hidden,
  cl::desc("CSA-specific: dump memory ordering chains"),
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

// A guess at the number of distinct regions in a function.
constexpr unsigned REGIONS_PER_FUNCTION = 2;

// A guess at the depth of memory operations in loops. This is used as a hint
// for the number of merge-phi entries per node.
constexpr unsigned SECTION_LOOP_DEPTH = 2;

// A guess at the number of may-merge inputs to each merge.
constexpr unsigned MAY_MERGE_COUNT = 5;

#define DEBUG_TYPE "csa-memop-ordering"

// Memory ordering statistics.
STATISTIC(MemopCount, "Number of memory operations ordered");
STATISTIC(MergeCount, "Number of merges inserted");
STATISTIC(PHICount, "Number of phi nodes inserted");

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

  // A type to represent ordering token values generated from phis, memops, and
  // merges (along with the special <none> value). Since all of those live in
  // (small)vectors in nodes that aren't going to be reordered, they can all be
  // identified by their node pointer and vector index.
  struct OrdToken {

    // The node that the chain element is in.
    const Node* node {nullptr};

    // The type of element that this token value is produced by.
    enum Type {none, phi, memop, merge} type {none};

    // The index of the element in its vector.
    int idx;

    // Some basic constructors.
    OrdToken() {}
    OrdToken(const Node* node_in, Type type_in, int idx_in)
      : node{node_in}, type{type_in}, idx{idx_in} {}

    // An explicit bool conversion for testing if this token is not <none>.
    explicit operator bool() const {return type;}

    // Retrieves the virtual register number for this token after those have
    // been assigned. This should never be called on <none>.
    unsigned reg_no() const;

    // Recursively marks all of the values that this token depends on.
    void mark_not_dead() const;

    // Dumps the ordering chain for this token. Output is produced at indent
    // level indent and duplicate output is omitted using the seen map. The
    // total number of instructions printed (/elided) under this one is
    // returned.
    int dump_ordering_chain(
      raw_ostream&, std::map<OrdToken, int>& seen, int indent = 0
    ) const;
  };

  // A phi node for memory ordering tokens.
  struct PHI {

    // All of the inputs to the phi node. Their order corresponds to the order
    // of the predecessor list in the phi node's memop CFG node.
    SmallVector<OrdToken, PRED_COUNT> inputs;

    // Whether this phi node doesn't actually connect to a memory operation;
    // this is initially set, but it is cleared during dead code elimination for
    // phi nodes that are connected.
    mutable bool dead = true;

    // This phi's virtual register.
    unsigned reg_no;
  };

  // An n-ary merge of memory ordering tokens.
  struct Merge {

    // The final inputs to this merge. These are filled out after phi_val
    // pruning but before merge expansion; until then the inputs will be found
    // in must_merge and phi_val. The inputs are always in a particular order
    // with the phi_val input first and the in-node inputs afterwards in order.
    SmallVector<OrdToken, MERGE_ARITY> inputs;

    // The phi-equivalent input to this merge.
    OrdToken phi_val;

    // The in-node memop indices that must be inputs to this merge. After
    // being initially gathered, these are always kept in ascending order.
    SmallVector<int, MERGE_ARITY> must_merge;

    // The in-node memop indices that _may_ be inputs to this merge. This will
    // be a superset of the ones in must_merge and are also kept in order.
    SmallVector<int, MAY_MERGE_COUNT> may_merge;

    // The latest possible location that this merge can be put relative to the
    // memops. If it doesn't need to go before any of the memops in its node,
    // this will be the number of memops in the node.
    int memop_idx;

    // Whether this merge doesn't actually connect to a memory operation; this
    // is initially set, but it is cleared during dead code elimination for
    // merges that are connected.
    mutable bool dead = true;

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

    // Clears phi_val if it is redundant with the other merge inputs in
    // must_merge. The node that this merge is in and its location within the
    // node are passed in.
    void prune_implicit_deps(Node*, int merge_idx);
  };

  // A parallel section intrinsic instance.
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
  };

  // A holder for all of the relevant information pertaining to a single memop.
  struct Memop {

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

    // The partial merge generated in the first (intra-node) phase of chain
    // construction.
    Merge merge;

    // The parallel section states at the beginning of the node from the
    // intra-node chain construction phase.
    SectionStates in_node_states;

    // This memop's virtual register for the issued signal.
    unsigned reg_no;

    // How to query the MachineMemOperand for this memory operation.
    const MachineMemOperand* mem_operand() const;

    // Constructs a default memory operation which needs to be ordered
    // relative to everything. These will have nullptr as their MI and
    // produce nullptr as the result of mem_operand().
    Memop() {}

    // Construction from an original machine instruction. This machine
    // instruction ought to have a single memory operand.
    Memop(MachineInstr*);

    // Determine whether a given machine instruction should be assigned ordering
    // (loaded into the memop CFG as a Memop). This is the case if it has a
    // memory operand and if its last use and last def are both %ign.
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
    // case if they alias and at least one modifies memory.
    bool operator()(const Memop&, const Memop&, bool looped) const;
  };

  // The collection of per-node, per-loop_height information used by the
  // algorithm.
  struct MergePhiEntry {

    // The loop height of this merge-phi entry. This is 0 for merge-phi entries
    // on paths that do not include back edges and numbered by loop from the
    // inside out for ones that do.
    int loop_height;

    // The parallel section states used when this merge-phi entry was created.
    SectionStates states;

    // Which mergephi indices to use in each predecessor node when creating a
    // phi node corresponding to this merge-phi entry. -1 is used here for ones
    // that haven't been determined yet or should just be ignored and replaced
    // with <none>.
    SmallVector<int, PRED_COUNT> pred_mergephis;

    // A flag that gets set for mergephis when wire_phis gets called on them,
    // marking mergephis that have had their chain value calculated or are
    // having it calculated. In either case, the chain value that is currently
    // set should be used when this mergephi is encountered rather than
    // calculating a new one.
    bool wired_phi = false;

    // The partial merge constructed for this mergephi.
    Merge merge;

    // The final token value which marks the end of the partial chain in the
    // node. This will be <none> until it's been determined.
    OrdToken chain_value;
  };

  // An entry for identifying a particular loop back edge in the loop queue
  // that needs further processing.
  struct LoopQueueEntry {

    // The header for the loop and which predecessor index corresponds to this
    // back edge.
    Node* header;
    int pred_idx;

    // The mergephi index to identify the pred_mergephis entry that ought to
    // get updated in the header once the chain through the previous loop
    // iteration is complete. If there isn't a mergephi to update because the
    // header is the original one with the memory operation in it,
    // mergephi_idx is set to -1.
    int mergephi_idx;

    // The initial section states for this entry.
    SectionStates sec_states;
  };

  // A Node in the MemopCFG, corresponding to a MachineBasicBlock in the
  // MachineFunction CFG.
  struct Node {

    // The original basic block.
    MachineBasicBlock* BB;

    // The loop that this basic block is in.
    const MachineLoop* BB_loop;

    // Predecessors to this node.
    SmallVector<Node*, PRED_COUNT> preds;

    // Successors to this node.
    SmallVector<Node*, SUCC_COUNT> succs;

    // The immediate dominator of this node.
    Node* dominator;

    // The memory operations in this node. These are in the order in which
    // they appear in the original basic block.
    SmallVector<Memop, MEMOP_COUNT> memops;

    // The parallel section intrinsics. These are also in basic block order
    // and have a field to indicate where they are relative to the memops.
    SmallVector<SectionIntrinsic, SECTION_INTRINSIC_COUNT> section_intrinsics;

    // The sets of merges and phis belonging to this node.
    SmallVector<Merge, MERGE_COUNT> merges;
    SmallVector<PHI, PHI_COUNT> phis;

    // The merge-phi pair state for this node that is filled out during the
    // chain construction traversal. This is specific to each memory operation
    // and wiped between traversals to construct ordering chains.
    SmallVector<MergePhiEntry, SECTION_LOOP_DEPTH> mergephis;

    // The RequireOrdering functor to use when adding operations to merges in
    // this node.
    const RequireOrdering& require_ordering;

    // The set of tokens that have already been checked for this node during
    // can_prune and the sets of values for each that it was checked against (as
    // ordered vectors). This node doesn't have to be re-checked if any of
    // the sets here for the token is a subset of the one that would have been
    // checked.
    std::multimap<OrdToken, SmallVector<OrdToken, MERGE_ARITY>> prune_checked;

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

    // Whether this node is inside of the loop between header and latch.
    bool in_loop(const Node* header, const Node* latch) const;

    // Recursively determines whether all of the memops connected to the
    // OrdToken are also connected to something in the SmallVector, making
    // the OrdToken argument redundant with those. The SmallVector should
    // be sorted.
    bool can_prune(const OrdToken&, const SmallVectorImpl<OrdToken>&);

    // These produce de-duplicated merge/phi values: they'll check for an
    // existing value first and then only add a new merge/phi if no matching
    // ones were found. These always return the first matching merge/phi in
    // the node in the case that there are multiple matches. If preserve_may is
    // set, less agressive matching criteria are used in order to avoid
    // losing information about merges that this merge could be combined with
    // later after pruning.
    OrdToken create_or_reuse(const Merge&, bool preserve_may = true);
    OrdToken create_or_reuse(const PHI&);

    // A convenience function for creating a phi node and a merge that depends
    // on it in one call. The merge is updated in-place to add the phi to it.
    OrdToken create_or_reuse(const PHI&, Merge&);

    // Collects all of the memory operations that a given memory operation
    // (identified via OrdToken) requires ordering with or has an implicit
    // dependency on in a merge's must_order and may_order sets. Both of those
    // should be cleared before calling this function. After the function is
    // done, both sets will be sorted and not contain any duplicate values.
    // This will start at the memory operation index start_idx and work
    // backwards to the beginning of this node. Looped is passed through to
    // require_ordering in order to indicate when a loop has been traversed and
    // the alias information query needs to change. The parallel section states
    // given as the second to last parameter are updated appropriately during
    // this traversal. True will be returned if the operation was successful,
    // false if something went wrong with the parallel sections and the pass
    // needs to back out and retry without parallel sections. If a fence-like
    // memop is encountered, skip_phi will be set to true and the traversal can
    // end at this node.
    bool collect_memops(
      const OrdToken&,
      Merge&,
      int start_idx,
      bool looped,
      SectionStates&,
      bool& skip_phi
    );

    // Sets up mergephi entries for all of the blocks reachable from this one
    // for a memory operation identified by OrdToken with a particular set of
    // initial section states and returns the index of the mergephi used for
    // this block. If a state mismatch is found, status is set to false. If
    // loop_height is 0 this will use the "plain" alias information and push
    // any loop backedges that it encounters for loops which have the original
    // memory operation in them into loop_queue for later processing with
    // higher loop_height levels. At higher levels, the "looped" alias
    // information is used instead and all backedges are traversed normally.
    // If stop_node is non-null, recursion will stop once that node has had a
    // mergephi constructed for it.
    int wire_merges(
      const OrdToken&, SectionStates, bool& status,
      int loop_height, std::queue<LoopQueueEntry>& loop_queue,
      const Node* stop_node = nullptr
    );

    // Uses all of the constructed mergephis to create phi nodes (as needed)
    // and merges (also as needed) for the ordering chain. The mergphi chain
    // value constructed for this node for the given mergephi index is returned.
    OrdToken wire_phis(int mergephi_idx);

    // Finalizes each of the merges by setting its inputs based on its phi_val
    // value and must_merge set.
    void finalize_merges();

    // Expands each merge into a tree of MERGE_ARITY-ary merges.
    void expand_merge_trees();

    // Adds phi nodes to the original machine basic block.
    void emit_phis();

    // Adds merges to the original machine basic block.
    void emit_merges();

    // Wires up the original memory operations and emits fence-like movs.
    void emit_memops();
  };

  // The collection of nodes in this CFG. In general, these will be sorted by
  // original basic block number.
  std::vector<std::unique_ptr<Node>> nodes;

  // The total number of different parallel regions represented in the CFG.
  unsigned region_count;

  // The RequireOrdering functor for computing "requires ordering with".
  RequireOrdering require_ordering;

  // Finds a node corresponding to a given original basic block identified by
  // number. This node is assumed to exist.
  Node* find_node(int bb_number) const;

  // Replaces all references to a given token value with a different one,
  // decrements any references to ones with the same node and type that come
  // after it, and safely removes the merge/phi that the token refers to from
  // its node. There should never be any reason to use this on a memop or
  // <none>. Also, new_val should not be equal to to_remove or a value of the
  // same type from later in the same node.
  void replace_and_shift(const OrdToken& to_remove, const OrdToken& new_val);

  // Loads the memop CFG with memops from a given machine function using the
  // given analysis results. The graph should be empty when this is called, so
  // make sure clear gets called before this is called again to load a different
  // function. If use_parallel_sections is set, parallel section intrinsics will
  // be copied over into the CFG; otherwise, they will be ignored.
  void load(
    MachineFunction&, AAResults*, const MachineDominatorTree*,
    const MachineLoopInfo*,
    bool use_parallel_sections
  );

  // Unloads/erases the currently-loaded graph.
  void clear();

  // Constructs the ordering chains for all of the memory operations in the
  // CFG. True is returned if this succeeds; otherwise, the CFG should be
  // re-loaded without parallel section intrinsics and tried again.
  bool construct_chains();

  // Prunes chains by removing redundant inputs from merges.
  void prune_chains();

  // Expands merge trees and emits the actual ordering chain instructions to the
  // original function.
  void emit_chains();

  // Dumps the memory ordering chains for each memop.
  void dump_ordering_chains(raw_ostream&);
};

// A pass for filling out memory ordering operands on memory operations. See the
// document in the document library for an in-depth coverage of the algorithm
// implemented by this pass.
class CSAIndependentMemopOrdering : public MachineFunctionPass {
  bool runOnMachineFunction(MachineFunction &MF) override;

public:
  static char ID;
  CSAIndependentMemopOrdering() : MachineFunctionPass(ID) { }
  StringRef getPassName() const override {
    return "CSA Memory Operation Ordering";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AAResultsWrapperPass>();
    AU.addRequired<MachineDominatorTree>();
    AU.addRequired<MachineLoopInfo>();
    AU.setPreservesAll();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  AAResults* AA;
  MachineRegisterInfo* MRI;
  const MachineDominatorTree* DT;
  const MachineLoopInfo* MLI;
  MemopCFG mopcfg;

  // Inserts ordering chains for each of the memory operations in the function.
  void addMemoryOrderingConstraints(MachineFunction&);

  // Wipe out all of the intrinsics.
  void eraseParallelIntrinsics(MachineFunction *MF);
};

// An operator< that provides a unique ordering for ordering tokens.
bool operator<(
  const MemopCFG::OrdToken& a, const MemopCFG::OrdToken& b
) {

  // Order by owning node first.
  if (std::less<const MemopCFG::Node*>{}(a.node, b.node)) return true;
  if (std::less<const MemopCFG::Node*>{}(b.node, a.node)) return false;

  // Then by type.
  if (a.type < b.type) return true;
  if (b.type < a.type) return false;

  // Then by index.
  return a.idx < b.idx;
}

// Also, token equality operators.
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

raw_ostream& operator<<(raw_ostream& out, const MemopCFG::OrdToken& val) {
  using Type = MemopCFG::OrdToken::Type;
  if (not val) return out << "<none>";
  out << val.node->BB->getNumber();
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
  out << val.idx;
  return out;
}

raw_ostream& operator<<(raw_ostream& out, const MemopCFG::Memop& memop) {
  if (not memop.MI) return out << "mov0 " << memop.ready;
  const TargetInstrInfo*const TII
    = memop.MI->getParent()->getParent()->getSubtarget().getInstrInfo();
  return out << TII->getName(memop.MI->getOpcode()) << " " << memop.ready;
}

raw_ostream& operator<<(raw_ostream& out, const MemopCFG::Merge& merge) {
  using namespace std;
  out << "merge";
  if (merge.inputs.empty()) {
    out << " " << merge.phi_val;
    bool first = true;
    for (int memop : merge.must_merge) {
      out << (first ? "; " : ", ") << memop;
      first = false;
    }
    assert(includes(
      begin(merge.may_merge), end(merge.may_merge),
      begin(merge.must_merge), end(merge.must_merge)
    ));
    decltype(merge.may_merge) may_diff (
      merge.may_merge.size() - merge.must_merge.size()
    );
    set_difference(
      begin(merge.may_merge), end(merge.may_merge),
      begin(merge.must_merge), end(merge.must_merge),
      begin(may_diff)
    );
    first = true;
    out << " (";
    for (int memop : may_diff) {
      out << (first ? "" : ", ") << memop;
      first = false;
    }
    out << ")";
  } else {
    bool first = true;
    for (const MemopCFG::OrdToken& merged : merge.inputs) {
      out << (first ? " " : ", ") << merged;
      first = false;
    }
    if (merge.delayed_emit_idx) out << " [" << merge.delayed_emit_idx << "]";
  }
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

raw_ostream& operator<<(
  raw_ostream& out, const MemopCFG::SectionStates& sec_states
) {
  using State = MemopCFG::SectionStates::State;
  out << "{";
  bool first = true;
  for (State state : sec_states.states) {
    out << (first ? "" : ", ");
    switch (state) {
      case State::no_intrinsics_encountered:
        out << "no_intrinsics_encountered";
        break;
      case State::not_in_section:
        out << "not_in_section";
        break;
      case State::outside_of_section:
        out << "outside_of_section";
        break;
      case State::crossed_sections:
        out << "crossed_sections";
        break;
    }
  }
  return out << "}";
}

// Writes the body of node to out, starting each line with line_start and ending
// each line with line_end. This is broken out into a separate function so that
// it can be shared between operator<< and the DOTGraphTraits specialization.
void print_body(
  raw_ostream& out, const MemopCFG::Node& node,
  const char* line_start, const char* line_end
) {
  using OrdToken = MemopCFG::OrdToken;
  for (int phi_idx = 0; phi_idx != int(node.phis.size()); ++phi_idx) {
    out << line_start << OrdToken{&node, OrdToken::phi, phi_idx} << " = "
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
        out << line_start << OrdToken{&node, OrdToken::merge, merge_idx}
          << " = " << node.merges[merge_idx] << line_end;
      }
    }
    if (memop_idx != int(node.memops.size())) {
      out << line_start << OrdToken{&node, OrdToken::memop, memop_idx} << " = "
        << node.memops[memop_idx] << line_end;
    }
  }
}

raw_ostream& operator<<(raw_ostream& out, const MemopCFG::Node& node) {
  using Node = MemopCFG::Node;
  out << node.BB->getNumber();
  if (node.BB->getBasicBlock()) {
    out << " (" << node.BB->getBasicBlock()->getName() << ")";
  }
  out << ":\npreds:";
  for (const Node*const pred : node.preds) out << " " << pred->BB->getNumber();
  out << "\n";
  print_body(out, node, "  ", "\n");
  out << "succs:";
  for (const Node*const succ : node.succs) out << " " << succ->BB->getNumber();
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
    return to_string(node->BB->getNumber())
      + " (" + node->BB->getBasicBlock()->getName().str() + ")";
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
    return to_string((*it)->BB->getNumber());
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
    return to_string(node->preds[pred_idx]->BB->getNumber());
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

char CSAIndependentMemopOrdering::ID = 0;

MachineFunctionPass *llvm::createCSAIndependentMemopOrderingPass() {
  return new CSAIndependentMemopOrdering();
}

bool CSAIndependentMemopOrdering::runOnMachineFunction(MachineFunction &MF) {
  MRI = &MF.getRegInfo();
  AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  DT = &getAnalysis<MachineDominatorTree>();
  MLI = &getAnalysis<MachineLoopInfo>();

  if (OrderMemops) addMemoryOrderingConstraints(MF);

  eraseParallelIntrinsics(&MF);

  return true;
}

void CSAIndependentMemopOrdering::addMemoryOrderingConstraints(MachineFunction& MF) {

  // Try to generate chains with parallel sections if the user has not disabled
  // them. If something goes wrong, print an obnoxious warning message and try
  // again without them.
  mopcfg.load(MF, AA, DT, MLI, ParallelOrderMemops);
  DEBUG(errs() << "pre ordering:\n\n" << mopcfg);
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

  mopcfg.prune_chains();

  if (DumpMemopCFG) errs() << mopcfg;
  if (DumpOrderingChains) mopcfg.dump_ordering_chains(errs());
  if (ViewMemopCFG) ViewGraph(mopcfg, MF.getName());

  mopcfg.emit_chains();

  mopcfg.clear();
}

unsigned MemopCFG::OrdToken::reg_no() const {
  assert(type);
  switch (type) {
    case Type::phi: return node->phis[idx].reg_no;
    case Type::memop: return node->memops[idx].reg_no;
    case Type::merge: return node->merges[idx].reg_no;
    default: break;
  }
  llvm_unreachable("Unexpected OrdToken type value");
}

void MemopCFG::OrdToken::mark_not_dead() const {
  switch (type) {
    case Type::none: case Type::memop:
      return;
    case Type::phi: {
      const PHI& phi = node->phis[idx];
      if (not phi.dead) return;
      phi.dead = false;
      for (const OrdToken& phid : phi.inputs) phid.mark_not_dead();
    } return;
    case Type::merge: {
      const Merge& merge = node->merges[idx];
      if (not merge.dead) return;
      merge.dead = false;
      merge.phi_val.mark_not_dead();
    } return;
  }
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
      auto found = seen.find(*this);
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
      auto found = seen.find(*this);
      if (found != end(seen)) out << " +" << found->second << "\n";
      else {
        out << "\n";
        if (merge.inputs.empty()) {
          if (
            merge.phi_val.type == Type::phi
              or merge.phi_val.type == Type::merge
          ) {
            ++under;
            under += merge.phi_val.dump_ordering_chain(out, seen, indent + 1);
          }
        }
        else for (const OrdToken& merged : merge.inputs) {
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

const MachineMemOperand* MemopCFG::Memop::mem_operand() const {
  return MI ? *MI->memoperands_begin() : nullptr;
}

MemopCFG::Memop::Memop(MachineInstr* MI_in) : MI{MI_in} {
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

  // If both are loads, they don't need ordering.
  if (a->isLoad() and b->isLoad()) return false;

  // Otherwise, they only need ordering if they could alias each other.

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
    return IgnoreAliasInfo or not AA->isNoAlias(
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

  // If they're both pseudo values, guess about whether they could alias based on
  // pseudo value kind.
  // TODO: Can we do better than this?
  if (a_pseudo and b_pseudo) return a_pseudo->kind() == b_pseudo->kind();

  // Otherwise, one is an IR value and the other is a pseudo value. Determine
  // whether the pseudo value could alias _any_ IR value and give that as the
  // answer.
  if (a_pseudo) return a_pseudo->isAliased(MFI);
  return b_pseudo->isAliased(MFI);
}

void MemopCFG::Merge::prune_implicit_deps(Node* node, int merge_idx) {
  using namespace std;
  if (not phi_val) return;
  DEBUG(
    errs() << OrdToken(node, OrdToken::merge, merge_idx) << " " << phi_val
  );
  SmallVector<OrdToken, MERGE_ARITY> memop_vals;
  for (int merged : must_merge) {
    memop_vals.emplace_back(OrdToken{node, OrdToken::memop, merged});
  }
  if (node->can_prune(phi_val, memop_vals)) {
    DEBUG(errs() << " pruned\n");
    phi_val = {};
    return;
  }
  DEBUG(errs() << " unpruned\n");
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
  const MemopCFG::SectionStates& that
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
  int unnormalized_id = region_entry->getOperand(1).getImm();

  // Look it up in the map. If there's already an entry, use that. Otherwise,
  // make a new one.
  const auto found = normalized_regions.find(unnormalized_id);
  if (found != normalized_regions.end()) return found->second;
  int new_id = normalized_regions.size();
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
    memops.emplace_back();
    memops.back().is_start = true;
  }

  // Go through all of the instructions and find the ones that need ordering.
  // Also take care of locating and adding parallel section intrinsics here.
  for (MachineInstr& MI : BB->instrs()) {
    if (Memop::should_assign_ordering(MI)) memops.emplace_back(&MI);
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
      memops.emplace_back();
      memops.back().call_mi = &MI;
      memops.back().is_start = false;
      memops.emplace_back();
      memops.back().call_mi = &MI;
      memops.back().is_start = true;
    }
  }

  // If there are no successors, add in a fence-like memop for the final mov0.
  if (BB->succ_empty()) {
    memops.emplace_back();
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

bool MemopCFG::Node::in_loop(const Node* header, const Node* latch) const {
  return header->BB_loop->contains(BB);
}

bool MemopCFG::Node::can_prune(
  const OrdToken& prune_val, const SmallVectorImpl<OrdToken>& other_vals
) {
  using namespace std;

  // <none> is a subset of everything.
  if (not prune_val) return true;

  // Go through the other_val sets that have already been checked for this
  // prune_val. If any are subsets, this node doesn't need to be checked again;
  // if any are supersets, they aren't needed in the list any more.
  const auto checked_range = prune_checked.equal_range(prune_val);
  for (
    auto checked_it = checked_range.first; checked_it != checked_range.second;
  ) {
    if (includes(
        begin(other_vals), end(other_vals),
        begin(checked_it->second), end(checked_it->second)
    )) return true;
    if (includes(
        begin(checked_it->second), end(checked_it->second),
        begin(other_vals), end(other_vals)
    )) checked_it = prune_checked.erase(checked_it);
    else ++checked_it;
  }
  prune_checked.emplace_hint(
    checked_range.second, prune_val,
    decltype(prune_checked)::mapped_type{begin(other_vals), end(other_vals)}
  );

  // These hold all of the values derived from other_vals. Out-of-node values or
  // phi nodes go in phis_or_oons to be passed along to the predecessors. Memory
  // operations in this node go in memop_queue for later processing here. Merges
  // in this node don't need a datastructure because they're broken down into
  // their phi_vals and memops during iteration. If process returns true, that
  // signals that prune_val itself was found and so the iteration can stop.
  set<OrdToken> phis_or_oons;
  priority_queue<int> memop_queue;
  const auto process = [&phis_or_oons, &memop_queue, &prune_val, this](
    const OrdToken& val
  ) {
    if (val == prune_val) return true;
    if (not val) return false;
    if (val.node != this or val.type == OrdToken::phi) phis_or_oons.insert(val);
    else if (val.type == OrdToken::memop) memop_queue.push(val.idx);
    else { assert(val.type == OrdToken::merge);
      const Merge& merge = merges[val.idx];
      if (merge.phi_val) phis_or_oons.insert(merge.phi_val);
      for (int merged : merge.must_merge) memop_queue.push(merged);
    }
    return false;
  };

  // Start off with the things in other_vals.
  if (any_of(begin(other_vals), end(other_vals), process)) return true;

  // Figure out which memops are connected to prune_val and what prune_val's phi
  // node (or out-of-node) value is. The connected memops are going to be in
  // _descending_ order (to match the backwards traversal) in the range
  // [cur_memop, memops_rend).
  OrdToken phi_val;
  SmallVector<int, 1> single_memop_holder;
  SmallVectorImpl<int>::const_reverse_iterator cur_memop, memops_rend;
  if (prune_val.node != this or prune_val.type == OrdToken::phi) {
    phi_val = prune_val;
    cur_memop = single_memop_holder.rbegin();
    memops_rend = single_memop_holder.rend();
  } else if (prune_val.type == OrdToken::memop) {
    single_memop_holder.push_back(prune_val.idx);
    cur_memop = single_memop_holder.rbegin();
    memops_rend = single_memop_holder.rend();
  } else { assert(prune_val.type == OrdToken::merge);
    const Merge& merge = merges[prune_val.idx];
    phi_val = merge.phi_val;
    cur_memop = merge.must_merge.rbegin();
    memops_rend = merge.must_merge.rend();
  }

  // Go through all of the memops in this node connected to other_vals.
  while (not memop_queue.empty()) {

    // Find the next memop to process from other_vals and pop all of its copies
    // off of the queue.
    int other_memop = memop_queue.top();
    while (not memop_queue.empty() and memop_queue.top() == other_memop) {
      memop_queue.pop();
    }

    // Look at the next memop from prune_vals. If it is the same one, mark it
    // off; if it has a higher index, we missed it and prune_val cannot be
    // pruned.
    if (cur_memop != memops_rend) {
      if (*cur_memop == other_memop) ++cur_memop;
      else if (*cur_memop > other_memop) return false;
    }

    // If there aren't any memops left from prune_val and there is no phi_val,
    // there's no need to keep going.
    else if (not phi_val) return true;

    // Add any dependencies from other_memop to the sets/queues.
    if (process(memops[other_memop].ready)) return true;
  }

  // If there are still some prune_val memops left over, we missed those.
  if (cur_memop != memops_rend) return false;

  // Recurse on the predecessors and make sure that the values can be pruned
  // there too.
  if (not phi_val) return true;
  SmallVector<OrdToken, MERGE_ARITY> pred_vals;
  for (int pred_idx = 0; pred_idx != int(preds.size()); ++pred_idx) {

    // Don't traverse back edges if phi_val dominates this node - this means
    // that there can't be anything important that needs checking around the
    // loop.
    if (
      phi_val.node != this and preds[pred_idx]->nonstrictly_dominated_by(this)
    ) continue;

    OrdToken pred_prune_val = phi_val.node == this
      ? phis[phi_val.idx].inputs[pred_idx]
      : phi_val;
    if (not pred_prune_val) continue;
    for (const OrdToken& val : phis_or_oons) {
      pred_vals.push_back(
        val.node == this ? phis[val.idx].inputs[pred_idx] : val
      );
    }
    sort(begin(pred_vals), end(pred_vals));
    if (not preds[pred_idx]->can_prune(pred_prune_val, pred_vals)) return false;
    pred_vals.clear();
  }
  return true;
}

MemopCFG::OrdToken MemopCFG::Node::create_or_reuse(const Merge& merge, bool preserve_may) {
  using namespace std;

  // If inputs is set, just add (or re-use) the merge directly.
  if (not merge.inputs.empty()) {
    for (int merge_idx = 0; merge_idx != int(merges.size()); ++merge_idx) {
      if (merges[merge_idx].inputs == merge.inputs) {
        if (merges[merge_idx].memop_idx > merge.memop_idx) {
          merges[merge_idx].memop_idx = merge.memop_idx;
        }
        return {this, OrdToken::merge, merge_idx};
      }
    }
    int merge_idx = merges.size();
    merges.push_back(merge);
    return {this, OrdToken::merge, merge_idx};
  }

  // See if either phi_val or a unique entry to must_merge can just be used
  // directly. When preserve_may is set, may_merge must also be empty or
  // restricted to one input for this to be done.
  if (preserve_may) {
    if (merge.may_merge.empty()) return merge.phi_val;
    if (
      not merge.phi_val and merge.may_merge.size() == 1u
        and merge.must_merge.size() == 1u
    ) return OrdToken{this, OrdToken::memop, merge.must_merge.front()};
  } else {
    if (merge.must_merge.empty()) return merge.phi_val;
    if (merge.must_merge.size() == 1u and not merge.phi_val) {
      return OrdToken{this, OrdToken::memop, merge.must_merge.front()};
    }
  }

  // Otherwise, look through all of the existing merges and try to find
  // something that matches there. In order to be a match, the phi_vals must
  // match and each merge's must_merge inputs must be subsets of the other's
  // may_merge ones. If preserve_may is set, merges are only allowed to be
  // combined if their may_merge sets exactly match.
  for (int merge_idx = 0; merge_idx != int(merges.size()); ++merge_idx) {
    Merge& cand = merges[merge_idx];
    if (cand.phi_val != merge.phi_val) continue;
    if (preserve_may) {
      if (cand.may_merge != merge.may_merge) continue;
    } else {
      if (
        not includes(
          begin(cand.may_merge), end(cand.may_merge),
          begin(merge.must_merge), end(merge.must_merge)
        )
      ) continue;
      if (
        not includes(
          begin(merge.may_merge), end(merge.may_merge),
          begin(cand.must_merge), end(cand.must_merge)
        )
      ) continue;
    }

    // If the merges are compatible, the must_merge set of the combined one
    // must be the union of the original must_merge sets and the may_merge set
    // must be the intersection of the originals.
    {
      decltype(cand.must_merge) new_must_merge;
      new_must_merge.reserve(
        max(cand.must_merge.size(), merge.must_merge.size())
      );
      set_union(
        begin(cand.must_merge), end(cand.must_merge),
        begin(merge.must_merge), end(merge.must_merge),
        back_inserter(new_must_merge)
      );
      cand.must_merge = move(new_must_merge);

      decltype(cand.may_merge) new_may_merge;
      new_may_merge.reserve(
        min(cand.may_merge.size(), merge.may_merge.size())
      );
      set_intersection(
        begin(cand.may_merge), end(cand.may_merge),
        begin(merge.may_merge), end(merge.may_merge),
        back_inserter(new_may_merge)
      );
      cand.may_merge = move(new_may_merge);
    }

    // In addition, the memop_idx of the combined merge must be the smallest of
    // the original ones.
    cand.memop_idx = min(cand.memop_idx, merge.memop_idx);

    return {this, OrdToken::merge, merge_idx};
  }

  // If no matches were found, make a new merge.
  int merge_idx = merges.size();
  merges.push_back(merge);
  return {this, OrdToken::merge, merge_idx};
}

MemopCFG::OrdToken MemopCFG::Node::create_or_reuse(const PHI& phi) {

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
  int phi_idx = phis.size();
  phis.push_back(phi);
  return {this, OrdToken::phi, phi_idx};
}

MemopCFG::OrdToken MemopCFG::Node::create_or_reuse(
  const PHI& phi, Merge& merge
) {
  merge.phi_val = create_or_reuse(phi);
  return create_or_reuse(merge);
}

bool MemopCFG::Node::collect_memops(
  const OrdToken& orig_memop_val,
  Merge& merge,
  int start_idx,
  bool looped,
  SectionStates& sec_states,
  bool& skip_phi
) {
  using namespace std;
  const Memop& orig_memop = orig_memop_val.node->memops[orig_memop_val.idx];

  // Figure out where to start processing parallel section intrinsics.
  auto sec_intr_pos = find_if(
    section_intrinsics.rbegin(), section_intrinsics.rend(),
    [start_idx](const SectionIntrinsic& intr) {
      return intr.memop_idx <= start_idx;
    }
  );

  // Implicit dependencies that haven't been encountered yet are tracked in this
  // priority queue.
  priority_queue<int> implicit_deps;
  const auto add_merge_to_implicit_deps = [&implicit_deps](const Merge& merge) {
    for (int merged : merge.must_merge) implicit_deps.push(merged);
  };

  // If this is the node that the original memop is in and the merge isn't the
  // memop's final merge, pre-populate implicit_deps with the memop and its
  // intra-node dependencies.
  if (orig_memop_val.node == this and &orig_memop.merge != &merge) {
    implicit_deps.push(orig_memop_val.idx);
    add_merge_to_implicit_deps(orig_memop.merge);
  }

  // Also add any memops from later iterations.
  for (const MergePhiEntry& mergephi : mergephis) if (not mergephi.loop_height) {
    add_merge_to_implicit_deps(mergephi.merge);
  }

  // Go through the memops in reverse order.
  for (int cur_idx = start_idx - 1; cur_idx >= 0; --cur_idx) {

    // Handle any parallel section state transitions first.
    while (
      sec_intr_pos != section_intrinsics.rend()
        and sec_intr_pos->memop_idx > cur_idx
    ) {
      if (not sec_states.transition(*sec_intr_pos)) return false;
      ++sec_intr_pos;
    }

    // Check to see whether this input is in the implicit dependency set if
    // it is, it should be taken out of the queue and put in the may_merge set.
    const bool has_implicit_dep =
      not implicit_deps.empty() and implicit_deps.top() == cur_idx;
    while (not implicit_deps.empty() and implicit_deps.top() == cur_idx) {
      implicit_deps.pop();
    }
    if (has_implicit_dep) {
      add_merge_to_implicit_deps(memops[cur_idx].merge);
      merge.may_merge.push_back(cur_idx);
    }

    // Check the parallel section states to see whether this memop can be
    // skipped because of that.
    if (sec_states.should_ignore_memops()) continue;

    // If this memop happens to be a fence-like one, it can be added now and the
    // traversal can be stopped early.
    if (not memops[cur_idx].mem_operand()) {
      if (not has_implicit_dep) {
        merge.must_merge.push_back(cur_idx);
        merge.may_merge.push_back(cur_idx);
      }
      for (int i = cur_idx - 1; i >= 0; --i) merge.may_merge.push_back(i);
      reverse(begin(merge.must_merge), end(merge.must_merge));
      reverse(begin(merge.may_merge), end(merge.may_merge));
      return skip_phi = true;
    }

    // Otherwise, check whether this operation can be skipped anyway according
    // to the general "requires ordering with" rules.
    if (not require_ordering(orig_memop, memops[cur_idx], looped)) continue;

    // With all of those checks out of the way, this memop should be added to
    // the merge if needed.
    if (not has_implicit_dep) {
      merge.must_merge.push_back(cur_idx);
      merge.may_merge.push_back(cur_idx);

      // Any memops that it depends on in this node should also be added to
      // the implicit dependency set.
      add_merge_to_implicit_deps(memops[cur_idx].merge);
    }
  }

  // Also handle any parallel section transitions that were left over.
  while (sec_intr_pos != section_intrinsics.rend()) {
    if (not sec_states.transition(*sec_intr_pos)) return false;
    ++sec_intr_pos;
  }

  // Finalize the must_merge and may_merge sets.
  reverse(begin(merge.must_merge), end(merge.must_merge));
  reverse(begin(merge.may_merge), end(merge.may_merge));

  return true;
}

int MemopCFG::Node::wire_merges(
  const OrdToken& orig_memop_val, SectionStates sec_states, bool& status,
  int loop_height, std::queue<LoopQueueEntry>& loop_queue,
  const Node* stop_node
) {
  using namespace std;

  // Make sure that none of the existing mergephi entries have incompatible
  // section states; if they do, something is wrong with the parallel section
  // intrinsics.
  for (const MergePhiEntry& mergephi : mergephis) {
    if (sec_states.incompatible_with(mergephi.states)) {
      DEBUG(
        errs() << BB->getNumber() << ": Incompatible section states:\nprev: "
          << mergephi.states << "\ncur:  " << sec_states << "\n"
      );
      status = false;
      return -1;
    }
  }

  // Go through the existing mergephi entries; if there's one with this loop
  // height already, just use it. If its section states are not compatible,
  // though, the section intrinsics are incorrect.
  for (
    int mergephi_idx = 0; mergephi_idx != int(mergephis.size()); ++mergephi_idx
  ) {
    if (mergephis[mergephi_idx].loop_height == loop_height) return mergephi_idx;
  }

  // Otherwise, a new mergephi really does need to be constructed.
  int mergephi_idx = mergephis.size();
  {
    MergePhiEntry mergephi;
    mergephi.loop_height = loop_height;
    mergephi.states = sec_states;
    mergephi.pred_mergephis.assign(preds.size(), -1);
    mergephi.merge.memop_idx = memops.size();
    mergephis.push_back(move(mergephi));
  }

  // Fill out the merge.
  bool skip_phi = false;
  auto& mergephi_merge = mergephis[mergephi_idx].merge;
  if (
    not collect_memops(
      orig_memop_val, mergephi_merge, memops.size(),
      loop_height, sec_states, skip_phi
    )
  ) {
    status = false;
    return -1;
  }

  // If skip_phi got set, there's no need to recurse.
  if (skip_phi) {
    return mergephi_idx;
  }

  // If skip_phi hasn't been set but there are no predecessors to this node,
  // there must be something wrong with the parallel sections.
  if (preds.empty()) {
    DEBUG(errs() << "Reached the beginning of a node with no preds!\n");
    status = false;
    return -1;
  }

  // If this node is the one pointed to by stop_node, there also isn't any need
  // to recurse.
  if (stop_node == this) return mergephi_idx;

  // Otherwise, start by collecting the backedges that need to be added to the
  // loop queue, if needed.
  if (not loop_height) {
    for (int pred_idx = 0; pred_idx != int(preds.size()); ++pred_idx) {
      if (
        preds[pred_idx]->nonstrictly_dominated_by(this)
            and orig_memop_val.node->in_loop(this, preds[pred_idx])
      ) {
        loop_queue.push({this, pred_idx, mergephi_idx, sec_states});
      }
    }
  }

  // And then recurse on the predecessors to fill out pred_mergephis.
  for (int pred_idx = 0; pred_idx != int(preds.size()); ++pred_idx) {
    if (
      loop_height or not preds[pred_idx]->nonstrictly_dominated_by(this)
        or not orig_memop_val.node->in_loop(this, preds[pred_idx])
    ) {
      mergephis[mergephi_idx].pred_mergephis[pred_idx]
        = preds[pred_idx]->wire_merges(
          orig_memop_val, sec_states, status, loop_height, loop_queue, stop_node
        );
      if (not status) return -1;
    }
  }

  // The mergephi is now ready for use in wire_phis.
  return mergephi_idx;
}

MemopCFG::OrdToken MemopCFG::Node::wire_phis(int mergephi_idx) {
  MergePhiEntry& mergephi = mergephis[mergephi_idx];

  // If wiring_phi is set, just return the current value and avoid infinite
  // recursion.
  if (mergephi.wired_phi) return mergephi.chain_value;
  mergephi.wired_phi = true;

  // Recurse to construct a phi node.
  PHI phi;
  for (int pred_idx = 0; pred_idx != int(preds.size()); ++pred_idx) {
    if (mergephi.pred_mergephis[pred_idx] < 0) phi.inputs.push_back({});
    else {
      phi.inputs.push_back(
        preds[pred_idx]->wire_phis(mergephi.pred_mergephis[pred_idx])
      );
    }
  }

  // Use create_or_reuse to set up it and the mergephi's merge.
  mergephi.chain_value = create_or_reuse(phi, mergephi.merge);
  return mergephi.chain_value;
}

void MemopCFG::Node::finalize_merges() {
  for (Merge& merge : merges) {
    if (merge.phi_val) merge.inputs.push_back(merge.phi_val);
    merge.inputs.reserve(merge.inputs.size() + merge.must_merge.size());
    for (int merged : merge.must_merge) {
      merge.inputs.emplace_back(this, OrdToken::memop, merged);
    }
    merge.must_merge.clear();
    merge.may_merge.clear();
  }
}

void MemopCFG::Node::expand_merge_trees() {
  using namespace std;

  // Go through each of the existing merges.
  SmallVector<OrdToken, MERGE_ARITY> new_inputs;
  Merge new_merge;
  const int orig_merge_count = merges.size();
  for (int merge_idx = 0; merge_idx != orig_merge_count; ++merge_idx) {
    new_merge.memop_idx = merges[merge_idx].memop_idx;

    // If a merge is already small enough, there's no reason to expand it to a
    // tree.
    const int input_count = merges[merge_idx].inputs.size();
    if (input_count <= MERGE_ARITY) continue;

    // In order to keep average latencies down, the tree should be as balanced
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
      const int to_comb
        = min(MERGE_ARITY, input_count - under_leaf_count - comb_start);
      const auto cur_start = begin(merges[merge_idx].inputs) + comb_start;
      new_merge.inputs.assign(cur_start, cur_start + to_comb);
      new_inputs.push_back(create_or_reuse(new_merge));
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
        const auto cur_start = begin(merges[merge_idx].inputs) + comb_start;
        new_merge.inputs.assign(cur_start, cur_start + MERGE_ARITY);
        new_inputs.push_back(create_or_reuse(new_merge));
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

void MemopCFG::Node::emit_phis() {
  const CSAInstrInfo* TII = static_cast<const CSAInstrInfo*>(
    BB->getParent()->getSubtarget().getInstrInfo()
  );
  MachineRegisterInfo& MRI = BB->getParent()->getRegInfo();
  for (const PHI& phi : phis) {
    MachineInstrBuilder phi_instr = BuildMI(
      *BB, BB->getFirstNonPHI(), DebugLoc{}, TII->get(CSA::PHI), phi.reg_no
    );
    for (int pred_idx = 0; pred_idx < int(preds.size()); ++pred_idx) {
      const OrdToken& phid = phi.inputs[pred_idx];
      if (phid) phi_instr.addUse(phid.reg_no());
      else {

        // Machine phi nodes can't just have immediate inputs, so this creates a
        // mov in the correct predecessor block instead.
        unsigned imm_reg = MRI.createVirtualRegister(MemopRC);
        MachineBasicBlock* pred_BB = preds[pred_idx]->BB;
        BuildMI(
          *pred_BB, pred_BB->getFirstTerminator(), DebugLoc{}, TII->get(CSA::MOV1),
          imm_reg
        ).addImm(0);
        phi_instr.addUse(imm_reg);

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
      prev(end(memop.MI->uses()))->ChangeToRegister(memop.ready.reg_no(), false);
      ++MemopCount;
    }

    // Otherwise, a new sxu mov0 should be emitted.
    else {

      // If is_start is set, it goes at the beginning of the block or after the
      // call.
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
        ).addImm(0);
      }

      // Otherwise, it goes at the end or before the call.
      else {
        const MachineBasicBlock::iterator where = memop.call_mi
          ? memop.call_mi
          : BB->getFirstTerminator();
        BuildMI(
          *BB, where, DebugLoc{}, TII->get(mov_opcode),
          memop.reg_no
        ).addUse(memop.ready.reg_no());
      }
    }
  }
}

MemopCFG::Node* MemopCFG::find_node(int bb_number) const {
  using namespace std;
  return lower_bound(begin(nodes), end(nodes), bb_number,
    [](const unique_ptr<Node>& node, int bb_number) {
      return node->BB->getNumber() < bb_number;
    }
  )->get();
}

void MemopCFG::replace_and_shift(
  const OrdToken& to_remove, const OrdToken& new_val
) {
  assert(to_remove.type == OrdToken::phi or to_remove.type == OrdToken::merge);
  assert(to_remove != new_val);
  assert(
    to_remove.node != new_val.node
    or to_remove.type != new_val.type
    or to_remove.idx > new_val.idx
  );

  // A helper to update each memsignal value.
  const auto fix_val = [&to_remove, &new_val](OrdToken& val) {
    if (val.node == to_remove.node and val.type == to_remove.type) {
      if (val.idx == to_remove.idx) val = new_val;
      else if (val.idx > to_remove.idx) --val.idx;
    }
  };

  for (const std::unique_ptr<Node>& node : nodes) {

    // Update memsignal values.
    for (PHI& phi : node->phis) for (OrdToken& phid : phi.inputs) fix_val(phid);
    for (Memop& memop : node->memops) fix_val(memop.ready);
    for (Merge& merge : node->merges) fix_val(merge.phi_val);

    // Also remove the merge/phi if it is in this node.
    if (node.get() == to_remove.node) {
      if (to_remove.type == OrdToken::phi) {
        node->phis.erase(node->phis.begin() + to_remove.idx);
      } else node->merges.erase(node->merges.begin() + to_remove.idx);
    }
  }
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
    }

    // The number of regions can now be determined from normalized_regions.
    region_count = normalized_regions.size();
  }

  // Make sure the nodes are sorted. After this there's no need to re-sort
  // unless a different function needs to be loaded.
  sort(begin(nodes), end(nodes),
    [](const unique_ptr<Node>& a, const unique_ptr<Node>& b) {
      return a->BB->getNumber() < b->BB->getNumber();
    }
  );

  // Go through and wire up all of the predecessors, successors, dominators,
  // and loops.
  for (const unique_ptr<Node>& pred : nodes) {
    for (const MachineBasicBlock* succ_bb : pred->BB->successors()) {
      Node*const succ = find_node(succ_bb->getNumber());
      pred->succs.push_back(succ);
      succ->preds.push_back(pred.get());
    }
    if (
      const MachineDomTreeNode*const dom_node = DT->getNode(pred->BB)->getIDom()
    ) pred->dominator = find_node(dom_node->getBlock()->getNumber());
    else pred->dominator = nullptr;
    pred->BB_loop = MLI->getLoopFor(pred->BB);
  }
}

void MemopCFG::clear() {
  nodes.clear();
}

bool MemopCFG::construct_chains() {
  using namespace std;

  // Do an initial run through all of the memops that need ordering to figure
  // out their intra-node dependencies.
  for (const unique_ptr<Node>& node : nodes) {
    for (int memop_idx = 0; memop_idx != int(node->memops.size()); ++memop_idx) {
      Memop& memop = node->memops[memop_idx];
      OrdToken memop_val {node.get(), OrdToken::memop, memop_idx};
      if (not memop.MI and memop.is_start) continue;

      // Prepare the section states.
      memop.in_node_states.states.resize(region_count);

      // Add the intra-node dependencies to the memop's merge.
      memop.merge.memop_idx = memop_idx;
      bool skip_phi = false;
      bool status = node->collect_memops(
        memop_val, memop.merge, memop_idx, false,
        memop.in_node_states, skip_phi
      );
      if (not status) return false;

      // If skip_phi got set, the merge can just be created directly since the
      // phi won't be needed.
      if (skip_phi) {
        memop.ready = node->create_or_reuse(memop.merge);
        continue;
      }

      // If it hasn't reached a fence and there are no predecessors, there must
      // be something wrong with the parallel section intrinsics.
      if (node->preds.empty()) {
        DEBUG(errs() << "Reached the beginning of a node with no preds!\n");
        return false;
      }
    }
  }

  // Go through again and set up the phi nodes.
  queue<LoopQueueEntry> loop_queue;
  for (const unique_ptr<Node>& node : nodes) {
    for (int memop_idx = 0; memop_idx != int(node->memops.size()); ++memop_idx) {
      Memop& memop = node->memops[memop_idx];
      OrdToken memop_val {node.get(), OrdToken::memop, memop_idx};
      if (not memop.MI and memop.is_start) continue;
      if (memop.ready) continue;

      // Collect the back edges first to make sure that the loop nesting is in
      // the correct order.
      for (int pred_idx = 0; pred_idx != int(node->preds.size()); ++pred_idx) {
        if (node->preds[pred_idx]->nonstrictly_dominated_by(node.get())) {
          loop_queue.push({node.get(), pred_idx, -1, memop.in_node_states});
        }
      }

      // Collect mergephi indices for the non-loop portion of the chain.
      SmallVector<int, PRED_COUNT> pred_mergephis (node->preds.size(), -1);
      for (int pred_idx = 0; pred_idx != int(node->preds.size()); ++pred_idx) {
        if (not node->preds[pred_idx]->nonstrictly_dominated_by(node.get())) {
          bool status = true;
          pred_mergephis[pred_idx] = node->preds[pred_idx]->wire_merges(
            memop_val, memop.in_node_states, status, 0, loop_queue
          );
          if (not status) return false;
        }
      }

      // Go through the loop queue and add all of the looped portions in.
      int loop_height = 0;
      while (not loop_queue.empty()) {
        const LoopQueueEntry loop = move(loop_queue.front());
        loop_queue.pop();
        DEBUG(errs() << "adding loop " << loop.header->BB->getNumber() << " <- " << loop.header->preds[loop.pred_idx]->BB->getNumber() << "\n");
        bool status = true;
        int pred_mergephi_idx = loop.header->preds[loop.pred_idx]->wire_merges(
          memop_val, loop.sec_states, status, ++loop_height, loop_queue,
          loop.header
        );
        if (not status) return false;
        if (loop.mergephi_idx >= 0) {
          loop.header->mergephis[loop.mergephi_idx]
            .pred_mergephis[loop.pred_idx] = pred_mergephi_idx;
        } else {
          pred_mergephis[loop.pred_idx] = pred_mergephi_idx;
        }
      }

      // Create a new phi node using those mergephi indices.
      PHI final_phi;
      for (int pred_idx = 0; pred_idx != int(node->preds.size()); ++pred_idx) {
        if (pred_mergephis[pred_idx] < 0) final_phi.inputs.push_back({});
        else {
          final_phi.inputs.push_back(
            node->preds[pred_idx]->wire_phis(pred_mergephis[pred_idx])
          );
        }
      }

      // Set up the ready signal based on the new phi and the intra-node merge.
      memop.ready = node->create_or_reuse(final_phi, memop.merge);

      // Also make sure that the mergephis all get cleared.
      for (const unique_ptr<Node>& node : nodes) node->mergephis.clear();
    }
  }

  DEBUG(errs() << "after construction:\n\n" << *this);
  return true;
}

void MemopCFG::prune_chains() {

  bool did_something;
  do {
    did_something = false;

    // Go and prune all of the merges.
    for (const std::unique_ptr<Node>& node : nodes) {
      for (
        int merge_idx = 0; merge_idx != int(node->merges.size()); ++merge_idx
      ) {
	node->merges[merge_idx].prune_implicit_deps(node.get(), merge_idx);
	for (const std::unique_ptr<Node>& node : nodes) {
	  node->prune_checked.clear();
	}
      }
    }

    // Iteratively simplify any merges/phis that might be possible to simplify
    // as a result of the pruning.
    bool removed_merge_or_phi;
    do {
      removed_merge_or_phi = false;
      for (const std::unique_ptr<Node>& node : nodes) {
	for (
	  int merge_idx = 0; merge_idx != int(node->merges.size()); ++merge_idx
	) {
	  const OrdToken merge_val {node.get(), OrdToken::merge, merge_idx};
	  const OrdToken new_val = node->create_or_reuse(
            node->merges[merge_idx], false
          );
	  if (new_val != merge_val) {
	    replace_and_shift(merge_val, new_val);
	    removed_merge_or_phi = true;
	    --merge_idx;
	  }
	}
	for (int phi_idx = 0; phi_idx != int(node->phis.size()); ++phi_idx) {
	  const OrdToken phi_val {node.get(), OrdToken::phi, phi_idx};
	  const OrdToken new_val = node->create_or_reuse(node->phis[phi_idx]);
	  if (new_val != phi_val) {
	    replace_and_shift(phi_val, new_val);
	    removed_merge_or_phi = true;
	    --phi_idx;
	  }
	}
      }
      if (removed_merge_or_phi) did_something = true;
    } while (removed_merge_or_phi);
  } while (did_something);

  DEBUG(errs() << "after pruning:\n\n" << *this);

  for (const std::unique_ptr<Node>& node : nodes) {
    for (const Memop& memop : node->memops) memop.ready.mark_not_dead();
  }
  for (const std::unique_ptr<Node>& node : nodes) {
    for (
      int merge_idx = 0; merge_idx != int(node->merges.size()); ++merge_idx
    ) {
      const OrdToken merge_val {node.get(), OrdToken::merge, merge_idx};
      if (node->merges[merge_idx].dead) {
        DEBUG(errs() << "removing dead merge " << merge_val << "\n");
        replace_and_shift(merge_val, {});
        --merge_idx;
      }
    }
    for (int phi_idx = 0; phi_idx != int(node->phis.size()); ++phi_idx) {
      const OrdToken phi_val {node.get(), OrdToken::phi, phi_idx};
      if (node->phis[phi_idx].dead) {
        DEBUG(errs() << "removing dead phi " << phi_val << "\n");
        replace_and_shift(phi_val, {});
        --phi_idx;
      }
    }
  }

  DEBUG(errs() << "after DCE:\n\n" << *this);

  for (const std::unique_ptr<Node>& node : nodes) node->finalize_merges();
}

void MemopCFG::emit_chains() {

  // Expand all of the merge trees.
  for (const std::unique_ptr<Node>& node : nodes) {
    node->expand_merge_trees();
  }

  DEBUG(errs() << "after merge expansion:\n\n" << *this);

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

void CSAIndependentMemopOrdering::eraseParallelIntrinsics(MachineFunction *MF){
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
