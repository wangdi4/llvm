//===- AddSubReassociate.h - Reassociate add/sub expressions ----*- C++ -*-===//
//
// Copyright (C) 2018 - 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_ADDSUBREASSOCIATE_H
#define LLVM_TRANSFORMS_SCALAR_ADDSUBREASSOCIATE_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/PassManager.h"
#include <atomic>
#include <memory>
#include <unordered_set>

namespace llvm {

template <typename T> struct HashIt {
  size_t operator()(const T &Obj) const { return Obj.getHash(); }
};

namespace intel_addsubreassoc {

// You can't just cast away constness of an iterator since they are different
// types. This is a helper to take away constness with constant complexity.
template <typename Container, typename ConstIterator>
typename Container::iterator remove_constness(Container &c, ConstIterator it) {
  return c.erase(it, it);
}

// Maximum distance between two Values.
constexpr auto MAX_DISTANCE = LONG_MAX;

// This represents the associative instruction that applies to this leaf.
class AssocOpcodeData {
  unsigned Opcode;
  Constant *Const;

public:
  AssocOpcodeData(const Instruction *I);
  unsigned getOpcode() const { return Opcode; }
  Constant *getConst() const { return Const; }
  hash_code getHash() const { return hash_combine(Opcode, Const); }
  bool operator==(const AssocOpcodeData &Other) const {
    return Opcode == Other.Opcode && Const == Other.Const;
  }
  bool operator!=(const AssocOpcodeData &Other) const {
    return !(*this == Other);
  }
  // Comparator used for sorting.
  bool operator<(const AssocOpcodeData &Other) const {
    return std::tie(Opcode, Const) < std::tie(Other.Opcode, Other.Const);
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const;
#endif
};

class OpcodeData {
  using AssocDataTy = SmallVector<AssocOpcodeData, 1>;

  unsigned Opcode;
  // The Unary associative opcodes that apply to the leaf.
  AssocDataTy AssocOpcodeVec;

public:
  OpcodeData() = delete;
  OpcodeData(unsigned Opcode) : Opcode(Opcode), AssocOpcodeVec() {}

  // The Add/Sub opcode of the leaf.
  unsigned getOpcode() const { return Opcode; }
  AssocDataTy::const_iterator begin() const { return AssocOpcodeVec.begin(); }
  AssocDataTy::const_iterator end() const { return AssocOpcodeVec.end(); }
  hash_code getHash() const {
    hash_code Hash = hash_combine(Opcode);
    for (const auto &Data : AssocOpcodeVec)
      Hash = hash_combine(Hash, Data.getHash());
    return Hash;
  }
  bool isAssocEqual(const OpcodeData &Other) const {
    return AssocOpcodeVec == Other.AssocOpcodeVec;
  }
  // Compare the whole data.
  bool operator==(const OpcodeData &Other) const {
    return Opcode == Other.Opcode && isAssocEqual(Other);
  }
  // Comparator used for sorting.
  bool operator<(const OpcodeData &Other) const {
    return std::tie(Opcode, AssocOpcodeVec) <
           std::tie(Other.Opcode, Other.AssocOpcodeVec);
  }
  // Reverse the opcode
  void reverse();
  // Take a copy with the reversed opcode
  OpcodeData getReversed() const {
    OpcodeData Opc = *this;
    Opc.reverse();
    return Opc;
  }
  void appendAssocInstr(Instruction *I) {
    AssocOpcodeVec.push_back(AssocOpcodeData(I));
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const;
#endif
};

/// This is a single node of a canonical form (see CanonForm for more details).
/// It represents incoming value called a leaf and an operation used to link it
/// to a canonical form.
class CanonNode {
  /// Incoming value called a leaf.
  TrackingVH<Value> Leaf;
  /// The canonicalized opcode. In addition it keeps associative instructions
  /// like '<< 4' as well.
  OpcodeData Opcode;

public:
  CanonNode(Value *L, const OpcodeData &Opcode) : Leaf(L), Opcode(Opcode) {}

  Value *getLeaf() const { return Leaf; }
  const OpcodeData &getOpcodeData() const { return Opcode; }

  void appendAssocInstruction(Instruction *I) { Opcode.appendAssocInstr(I); }
  void reverseOpcode() { Opcode.reverse(); }
  hash_code getHash() const {
    return hash_combine(Leaf.getValPtr(), Opcode.getHash());
  }
  /// Two nodes are equal if they have the same incoming leaf. Please note that
  /// we don't take opcode into account.
  bool operator==(const CanonNode &Pair2) const { return Leaf == Pair2.Leaf; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(unsigned Padding) const;
#endif
};

/// Canonical form is a linerized representation of an original binary tree
/// where one (say left) operand is always a terminal node and the other
/// operand is either a canonical form or implied zero. Each terminal node
/// has associated operation and the two form a leaf.
///
/// For example for (a+b)-(c-d) looks the following way:
///  +
///  |\
///  a +
///    |\
///    b -
///      |\
///      c +
///        |\
///        d 0 <== implied zero
/// which also can be written as string of leaves: +d,-c,+b,+a.
///
/// Core property of a canonical form is that reordering of its leaves does not
/// affect tree semantics.
class CanonForm {
public:
  using NodeVecTy = SmallVector<CanonNode, 16>;
  using NodeItTy = NodeVecTy::const_iterator;
  using NodeRItTy = NodeVecTy::const_reverse_iterator;

private:
  // Vector of all the nodes.
  NodeVecTy Leaves;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  std::string Name;
#endif

public:
  // Let's be explicit and specify each compiler generated member.
  explicit CanonForm() {}
  CanonForm(const CanonForm &Src) = default;
  CanonForm &operator=(const CanonForm &Src) = default;
  CanonForm(CanonForm &&Src) = default;
  CanonForm &operator=(CanonForm &&Src) = default;
  virtual ~CanonForm() {}

  /// Returns true if there is no nodes in the form, false otherwise.
  bool empty() const { return Leaves.empty(); }
  /// Returns number of nodes in the form.
  size_t size() const { return Leaves.size(); }
  void clear() { Leaves.clear(); }

  // Set of iterators over nodes.
  NodeItTy begin() const { return Leaves.begin(); }
  NodeItTy end() const { return Leaves.end(); }
  NodeVecTy::iterator begin() { return Leaves.begin(); }
  NodeVecTy::iterator end() { return Leaves.end(); }
  NodeRItTy rbegin() const { return Leaves.rbegin(); }
  NodeRItTy rend() const { return Leaves.rend(); }

  /// Adds new node with \p Leaf and \Opcode as the last one to the form.
  void appendLeaf(Value *Leaf, const OpcodeData &Opcode) {
    Leaves.emplace_back(Leaf, Opcode);
  }
  /// Removes node from the form specified by iterator \p It.
  /// NOTE: All iterators pointing after removed node are invalidated.
  NodeItTy removeLeaf(NodeItTy It) { return Leaves.erase(It); }
  /// Reorders two nodes in the form.
  /// NOTE: This operation doesn't invalidate any iterators.
  void swapLeaves(NodeItTy LLeaf, NodeItTy RLeaf) {
    std::iter_swap(remove_constness(Leaves, LLeaf),
                   remove_constness(Leaves, RLeaf));
  }
  /// Returns iterator pointing to a node that has \p Leaf value matching
  /// It does not take opcode data in the form into account.
  NodeItTy findLeaf(const Value *Leaf) const {
    return find_if(Leaves, [Leaf](const CanonNode &Node) {
      return Node.getLeaf() == Leaf;
    });
  }
  /// Returns iterator pointing to a node that has \p Leaf value matching
  /// and opcode data in the form is equal to \p OpData.
  NodeItTy findLeaf(const Value *Leaf, const OpcodeData &OpData) const {
    return find_if(Leaves, [&OpData, Leaf](const CanonNode &Node) {
      return Node.getLeaf() == Leaf && Node.getOpcodeData() == OpData;
    });
  }
  /// Convinience interface to check whether a Leaf is in the Form.
  bool hasLeaf(const Value *Leaf) const {
    return findLeaf(Leaf) != end();
  }
  /// Performs massaging aimed at more optimal code generation.
  // TODO: describe what exactly it does.
  bool simplify();
  /// Emits IR representation of the canonical form by inserting new
  /// instructions before \p IP. If \p GenZero controls if we need putting
  /// "ending" zero to IR or note. Please note even if \p GenZero is false there
  /// are cases when we have to generate "ending" zero to IR.
  Value *generateCode(Instruction *IP, bool GenZero) const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  std::string getName() const { return Name; }
  void setName(const Twine &N) { Name = N.str(); }

  void dump() const;
#endif

protected:
  /// Generates binary instruction and inserts it before \p IP. Generated
  /// instruction has opcode specified by \p Opcode and second operand equal to
  /// \p Leaf. First operand is not set. Please note if \p Opcode has
  /// associative instructions they will be emitted before returned one.
  Instruction *generateInstruction(const OpcodeData &Opcode, Value *Leaf,
                                   Instruction *IP) const;
  /// Reverse opcode of each node in the form.
  void reverseOpcodes() {
    for (CanonNode &Leaf : *this)
      Leaf.reverseOpcode();
  }
};

/// This is an abstraction for dealing with original (IR) and canonical
/// representations of expression tree. It has all properties of the canonical
/// form since it directly inherits from it. In addition it provides access to
/// underlying IR tree which consists of all instruction starting from \p Root
/// and ending at leaves handled by \p CanonFrom.
class Tree : public CanonForm {
  const DataLayout &DL;
  // The root instruction of the tree.
  Instruction *Root = nullptr;
  // Set to true if this tree contains shared leaves candidates.
  // This is used to avoid searching through the leaves of a tree.
  bool HasSharedLeafCandidate = false;
  // Number of shared leaves became part of a trunk. In other words,
  // that many leaves have been unshared during tree constructions.
  int SharedLeavesCount = 0;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  int Id = 0;
#endif
public:
  Tree(const DataLayout &DL) : CanonForm(), DL(DL) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    static std::atomic<int> IdCnt;
    Id = IdCnt++;
    setName((Twine("Trunk_") + Twine(Id) + Twine("_")).str());
#endif
  }
  virtual ~Tree() {}

  /// Set/Update the root of the tree.
  void setRoot(Instruction *R) { Root = R; }
  /// Returns tree root.
  Instruction *getRoot() const { return Root; }
  /// Returns true if \p I belongs to truck of the tree in IR representation.
  /// Please note that canonical representation has no trunk instructions.
  bool hasTrunkInstruction(const Instruction *I) const;
  /// Returns number of shared leaves that are part of the trunk.
  int getSharedLeavesCount() const { return SharedLeavesCount; }
  /// Increases/decreases number of shared leaves.
  void adjustSharedLeavesCount(int Count) { SharedLeavesCount += Count; }
  /// Return true if this tree contains shared leaf candidate nodes.
  bool hasSharedLeafCandidate() const { return HasSharedLeafCandidate; }
  /// Set the shared leaf candidate flag.
  void setSharedLeafCandidate(bool Flag) { HasSharedLeafCandidate = Flag; }
  /// Returns true if this tree is larger than 'T2'.
  bool operator<(const Tree &T2) const { return size() > T2.size(); }
  /// Restores original tree state as of a construction time. After the call
  /// the tree is in valid state and empty.
  void clear();
  /// Removes the tree from IR. After this call original trunk instructions
  /// are removed from the IR and a dummy (no-op) instruction becomes a new
  /// tree root. This dummy instruction is needed to track tree location in IR.
  void removeTreeFromIR();
  /// Generates IR representation from canonical representation. After this call
  /// IR and canonical representations are semantically equivalent.
  Value *generateCode();
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const;
#endif
};

using TreeSignTy = std::pair<Tree *, bool>;
using TreeSignVecTy = SmallVector<TreeSignTy, 16>;

/// Essentially Group is a canonical form with a number of specific operations
/// like sorting.
class Group : public CanonForm {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  int Id = 0;
#endif
public:
  Group() {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    static std::atomic<int> IdCnt; // really need atomic?
    Id = IdCnt++;
    setName(Twine("Chain_") + Twine(Id) + Twine("_"));
#endif
  }
  virtual ~Group() {}

  /// Performs massaging aimed at more optimal code generation. In addition
  /// to simplifications done for general canonical form we can change the sign
  /// of the entire group and update related information in \p GroupTrees.
  bool simplify(TreeSignVecTy &GroupTrees);

  /// Returns a pair of unique and total number of associative instructions in
  /// the group.
  std::pair<int /*unique*/, int /*total*/> getAssocInstrCnt() const {
    std::unordered_set<AssocOpcodeData, HashIt<AssocOpcodeData>> Set;

    int Total = 0;
    for (const CanonNode &TreeLeaf : *this) {
      for (const AssocOpcodeData &AOD : TreeLeaf.getOpcodeData()) {
        ++Total;
        Set.insert(AOD);
      }
    }
    return {Set.size(), Total};
  }
  /// Returns true if groups have equal sizes and all common opcodes only.
  bool isSimilar(const Group &G2);
  /// Canonicalize the group by sorting by opcode.
  void sort();
  /// Reverse opcodes of the entire group.
  void reverseOpcodes() { CanonForm::reverseOpcodes(); }
};

/// This pass reassociates add-sub chains to improve expression reuse
///
/// For example: X = A - B - C  -->  X = A - (B + C)
///              Y = A + B + C  -->  Y = A + (B + C)
class AddSubReassociate {
public:
  using TreePtr = std::unique_ptr<Tree>;

  AddSubReassociate(const DataLayout &DL, ScalarEvolution *SE, Function *F)
      : DL(DL), SE(SE), F(F){};

  /// Main entry point to the optimization.
  bool run();

private:
  const DataLayout &DL;
  ScalarEvolution *SE;
  Function *F;

  // Trees and tree clusters are built for single basic block
  // (one that is currently processed)
  SmallVector<TreePtr, 16> Trees;
  SmallVector<MutableArrayRef<TreePtr>, 8> Clusters;
  // Group of trees most profitable for reassosiation.
  SmallVector<std::pair<Group, TreeSignVecTy>, 4> BestGroups;

  /// Routine to compute distance between \p V1 and \p V2.
  /// If both V1 and V2 are load instructions return distance between
  /// pointers otherwise step down through operands seeking for load
  /// instructions until the operands remain instructions with same opcode.
  /// Search for loads as far as \p MaxDepth.
  /// Return distance if able to compute it.
  Optional<int64_t> findLoadDistance(Value *V1, Value *V2,
                                     unsigned MaxDepth = 2) const;
  /// Returns the sum of the absolute distances of G1 and G2.
  int64_t getSumAbsDistances(const CanonForm &G1, const CanonForm &G2);
  /// Recursive function to explore the different orderings of G1's leaves
  /// in order to match best one against G2.
  int64_t getBestSortedScore_rec(const Group &G1, const Group &G2,
                                 CanonForm G1Leaves, CanonForm G2Leaves,
                                 CanonForm &SortedG1Leaves,
                                 CanonForm &BestSortedG1Leaves,
                                 int64_t &BestScore, int depth = 0);
  /// Returns false if we did not manage to get a good ordering that matches G2.
  bool getBestSortedLeaves(const Group &G1, const Group &G2,
                           CanonForm &BestSortedG1Leaves);
  /// Canonicalize: (i) the order of the values in G1, (ii) the trunk opcodes,
  /// to match the ones in G2.
  bool memCanonicalizeGroupBasedOn(Group &G1, const Group &G2,
                                   ScalarEvolution *SE);
  /// Canonicalize group \p G based on BestGroups memory accesses and opcodes.
  bool memCanonicalizeGroup(Group &G, TreeSignVecTy &GroupTreeVec);

  /// Form groups of nodes in BestGroups that reduce divergence across trees in
  /// given \p Cluster.
  void buildMaxReuseGroups(const MutableArrayRef<TreePtr> &Cluster);

  /// Traverses BestGroups and removes all nodes common for a group and affected
  /// tree(s) from the tree(s). This is done using canonical representation of
  /// the group and tree(s). Note that this may invalidate IR representation of
  /// the tree(s).
  void removeCommonNodes();

  /// Adds additional node to the canonical representation of the tree given by
  /// \p TreeAndSign where \p GroupChain becomes a new leaf and opcode is
  /// determined by sign coming from \p TreeAndSign.
  void linkGroup(Value *GroupChain, TreeSignTy &TreeAndSign) const;

  /// Generates IR representation for all groups in BestGroups and all affected
  /// trees in \p AffectedTrees.
  void generateCode(const ArrayRef<Tree *> AffectedTrees);

  /// Find clusters amongst Trees with similar i) size, ii) values
  /// and populate Clusters.
  void clusterTrees();

  /// Try to grow tree \p Tree up toward definitions. \p GrowthLimit value
  /// determines how much the tree is allowed to grow.
  /// Returns size of the tree.
  unsigned growTree(Tree *Tree, unsigned GrowthLimit,
                    SmallVectorImpl<CanonNode> &&WorkList);

  /// Enlarge trees in Clusters by growing them towards shared leaves.
  void extendTrees();

  /// Build initial trees from IR in \p BB
  void buildInitialTrees(BasicBlock *BB);

  /// Build all trees within \p BB.
  void buildTrees(BasicBlock *BB, bool UnshareLeaves);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dumpTrees() const;
  void dumpCluster(const MutableArrayRef<TreePtr> &Cluster) const;
  void dumpClusters() const;
  void dumpGroups() const;
#endif
};
} // end namespace intel_addsubreassoc

class AddSubReassociatePass : public PassInfoMixin<AddSubReassociatePass> {
public:
  // Entry point for AddSub reassociation.
  bool runImpl(Function *F, ScalarEvolution *SE);
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

class AddSubReassociateLegacyPass : public FunctionPass {
  AddSubReassociatePass Impl;

public:
  static char ID; // Pass identification, replacement for typeid

  AddSubReassociateLegacyPass();

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_ADDSUBREASSOCIATE_H
