//===- AddSubReassociate.h - Reassociate add/sub expressions ----*- C++ -*-===//
//
// Copyright (C) 2018 - 2019 Intel Corporation. All rights reserved.
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
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/PassManager.h"
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
  bool operator==(const AssocOpcodeData &Data2) const {
    return Opcode == Data2.Opcode && Const == Data2.Const;
  }
  bool operator!=(const AssocOpcodeData &Data2) const {
    return !(*this == Data2);
  }
  // Comparator used for sorting.
  bool operator<(const AssocOpcodeData &Data2) const {
    if (Opcode != Data2.Opcode)
      return Opcode < Data2.Opcode;
    if (Const != Data2.Const)
      return Const < Data2.Const;
    return false;
  }
  // For debugging.
  void dump() const;
};

class OpcodeData {
  using AssocDataTy = SmallVector<AssocOpcodeData, 1>;

  unsigned Opcode = 0;
  // The Unary associative opcodes that apply to the leaf.
  AssocDataTy AssocOpcodeVec;

public:
  OpcodeData() {}
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
  bool isUndef() const { return Opcode == 0; }
  // Compare only the canonicalized +/- opcode.
  bool hasSameAddSubOpcode(const OpcodeData &OD2) const {
    return Opcode == OD2.Opcode;
  }
  // Compare the whole opcode.
  bool operator==(const OpcodeData &OD2) const {
    return Opcode == OD2.Opcode && AssocOpcodeVec == OD2.AssocOpcodeVec;
  }
  bool operator!=(const OpcodeData &OD2) const { return !(*this == OD2); }
  // Comparator used for sorting.
  bool operator<(const OpcodeData &OD2) const {
    if (Opcode != OD2.Opcode)
      return Opcode < OD2.Opcode;
    if (AssocOpcodeVec.size() != OD2.AssocOpcodeVec.size())
      return AssocOpcodeVec.size() < OD2.AssocOpcodeVec.size();
    for (size_t I = 0, E = AssocOpcodeVec.size(); I != E; ++I)
      if (AssocOpcodeVec[I] != OD2.AssocOpcodeVec[I])
        return AssocOpcodeVec[I] < OD2.AssocOpcodeVec[I];
    return false;
  }
  OpcodeData getFlipped() const;
  void appendAssocInstr(Instruction *I) {
    AssocOpcodeVec.push_back(AssocOpcodeData(I));
  }
  // For debugging.
  void dump() const;
};

/// This is a single node of a canonical from (see CanonForm for more details).
/// It represents incoming value called a leaf and an operation used to link it
/// to a canonical form.
class CanonNode {
  friend class CanonForm;
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

  hash_code getHash() const {
    //hash_code LeafHC = hash_value();
    return hash_combine(Leaf.getValPtr(), Opcode.getHash()); }
  /// Two nodes are equal if they have the same incoming leaf. Please note that
  /// we don't take opcode into account.
  bool operator==(const CanonNode &Pair2) const { return Leaf == Pair2.Leaf; }

  // Debug print.
  void dump(const unsigned Padding) const;
};

/// Canonical form is a special form of a binary tree where one (say left)
/// child is always a leaf and another one is either canonical form or zero.
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
///        d 0
///
/// Main property of a canonical form is that reordering of any two nodes
/// doesn't invalidate tree semantic.
class CanonForm {
public:
  using NodeVecTy = SmallVector<CanonNode, 16>;
  using NodeItTy = NodeVecTy::const_iterator;
  using NodeRItTy = NodeVecTy::const_reverse_iterator;

private:
  // Vector of all the nodes.
  NodeVecTy Leaves;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // TODO:
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
  void appendLeaf(Value *Leaf, const OpcodeData &Opcode);
  /// Removes node from the form specified by iterator \p It.
  /// NOTE: All iterators pointing after removed node are invalidated.
  NodeItTy removeLeaf(NodeItTy It) { return Leaves.erase(It); }
  /// Reorders two nodes in the form.
  /// NOTE: This operation doesn't invalidate any iterators.
  void swapLeaves(NodeItTy LLeaf, NodeItTy RLeaf) {
    std::iter_swap(remove_constness(Leaves, LLeaf),
                   remove_constness(Leaves, RLeaf));
  }
  /// Returns node matching \p Leaf and optionally \p Opcode starting from
  /// position given by \p It. If \p Opcode argument is omitted then only
  /// \p Leaf is taken into consideration. By specifying \p It you exclude
  /// all nodes preceding \p It from the search. This may be useful if you need
  /// to handle nodes with identical leaves.
  NodeItTy findLeaf(const NodeItTy It, const Value *Leaf,
                    const OpcodeData &Opcode = OpcodeData()) const;

  /// Performs massaging aimed at more optimal code generation.
  bool simplify();
  /// Emits IR representation of the canonical form by inserting new
  /// instructions before \p IP. If \p GenZero controls if we need putting
  /// "ending" zero to IR or note. Please note even if \p GenZero is false there
  /// are cases when we have to generate "ending" zero to IR.
  Value *generateCode(Instruction *IP, bool GenZero) const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  std::string getName() const { return Name; }
  void setName(const Twine &N) { Name = N.str(); }
#endif

  void dump() const;

protected:
  /// Generates binary instruction and inserts it before \p IP. Generated
  /// instruction has opcode specified by \p Opcode and second operand equal to
  /// \p Leaf. First operand is not set. Please note if \p Opcode has
  /// associative instructions they will be emitted before returned one.
  Instruction *generateInstruction(const OpcodeData &Opcode, Value *Leaf,
                                   Instruction *IP) const;

  /// Flips opcodes of all nodes in the form.
  void flipOpcodes();
};

/// This is an abstraction for dealing with original (IR) and canonical
/// representations of expression tree. It has all properties of the canonical
/// form since it directly inherits from it. In addition it provides access to
/// underlying IR tree which consists of all instruction starting from \p Root
/// and ending at leaves handled by \p CanonFrom.
class Tree : public CanonForm {
  const DataLayout &DL;
  // Unique tree identifier. Used only for debugging.
  int Id = 0;
  // The root instruction of the tree.
  Instruction *Root = nullptr;
  // Set to true if this tree contains shared leaves candidates.
  // This is used to avoid searching through the leaves of a tree.
  bool HasSharedLeafCandidate = false;
  // Number of shared leaves became part of a trunk. In other words,
  // that many leaves have been unshared during tree constructions.
  int SharedLeavesCount = 0;

public:
  Tree(const DataLayout &DL) : CanonForm(), DL(DL) {
    static std::atomic<int> IdCnt;
    Id = IdCnt++;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    setName((Twine("Trunk_") + Twine(Id) + Twine("_")).str());
#endif
  }
  virtual ~Tree() {}

  /// Set/Update the root of the tree.
  void setRoot(Instruction *R) { Root = R; }
  /// Returns tree root.
  Instruction *getRoot() const { return Root; }
  /// Returns true if \p V is an instruction and valid to be part of the tree
  /// trunk.
  bool isAllowedTrunkInstr(const Value *V) const;
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
  /// Debug print.
  void dump() const;
};

using TreeSignTy = std::pair<Tree *, bool>;
using TreeSignVecTy = SmallVector<TreeSignTy, 16>;

/// Essentially Group is a canonical form with a number of specific operations
/// like sorting.
class Group : public CanonForm {
  // Unique identifier. Used only for debugging.
  int Id = 0;

public:
  Group() {
    static std::atomic<int> IdCnt;
    Id = IdCnt++;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
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
  std::pair<unsigned, unsigned> getAssocInstrCnt() const {
    std::unordered_set<AssocOpcodeData, HashIt<AssocOpcodeData>> Set;

    unsigned Total = 0;
    for (auto &TreeLeaf : *this) {
      for (const auto &AOD : TreeLeaf.getOpcodeData()) {
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
  /// Flips opcodes of entire group.
  void flipOpcodes();
};

/// This pass reassociates add-sub chains to improve expression reuse
///
/// For example: X = A - B - C  -->  X = A - (B + C)
///              Y = A + B + C  -->  Y = A + (B + C)
class AddSubReassociate {
  // Typedefs.
  using TreePtr = std::unique_ptr<Tree>;
  using TreeVecTy = SmallVector<TreePtr, 16>;
  using TreeArrayTy = MutableArrayRef<TreePtr>;
  using WorkListTy = SmallVectorImpl<CanonNode>;
  using GroupTreesVecTy = SmallVector<std::pair<Group, TreeSignVecTy>, 4>;

  const DataLayout &DL;
  ScalarEvolution *SE;
  Function *F;

public:
  AddSubReassociate(const DataLayout &DL, ScalarEvolution *SE, Function *F)
      : DL(DL), SE(SE), F(F){};

  /// Main entry point to the optimization.
  bool run();

private:
  /// Scans through \p AllTrees and returns the first one which containing \p I.
  static Tree *findEnclosingTree(TreeVecTy &AllTrees, const Instruction *I);
  /// Scans through \p AllTrees and returns the first one which has root \p I.
  static Tree *findTreeWithRoot(TreeVecTy &AllTrees, const Instruction *I,
                                const Tree *skipTree);

  /// Returns true if we were able to compute distance of V1 and V2 or one of
  /// their operands, false otherwise.
  bool getValDistance(Value *V1, Value *V2, int MaxDepth, int64_t &Distance);
  /// Returns the sum of the absolute distances of G1 and G2.
  int64_t getSumAbsDistances(const CanonForm &G1, const CanonForm &G2);
  /// Recursively calls itself to explore the different orderings of G1's leaves
  /// in order to match them best against G2.
  int64_t getBestSortedScoreRec(const Group &G1, const Group &G2,
                                CanonForm G1Leaves, CanonForm G2Leaves,
                                CanonForm &SortedG1Leaves,
                                CanonForm &BestSortedG1Leaves,
                                int64_t &BestScore);
  /// Returns false if we did not manage to get a good ordering that matches G2.
  bool getBestSortedLeaves(const Group &G1, const Group &G2,
                           CanonForm &BestSortedG1Leaves);
  /// Canonicalize: (i) the order of the values in G1, (ii) the trunk opcodes,
  /// to match the ones in G2.
  bool memCanonicalizeGroupBasedOn(Group &G1, const Group &G2,
                                   ScalarEvolution *SE);
  /// Canonicalize 'G' based on 'BestGroups' memory accesses and opcodes.
  bool memCanonicalizeGroup(Group &G, TreeSignVecTy &GroupTreeVec,
                            GroupTreesVecTy &BestGroups);
  /// Form groups of nodes that reduce divergence across trees in TreeCluster.
  void buildMaxReuseGroups(const TreeArrayTy &TreeCluster,
                           GroupTreesVecTy &AllBestGroups);
  /// For each tree in \p AffectedTrees removes its IR representation.
  void removeOldTrees(const ArrayRef<Tree *> AffectedTrees) const;
  /// Removes all nodes common for group and affected tree(s) from the tree(s).
  /// This is done using canonical representation of the group and tree(s).
  /// Note that this may invalidate IR representation of the tree(s).
  void removeGroupFromTree(GroupTreesVecTy &Groups) const;
  /// Adds additional node to the canonical representation of the tree given by
  /// \p TreeAndSign where \p GroupChain becomes a new leaf and opcode is
  /// determined by sign coming from \p TreeAndSign.
  void linkGroup(Value *GroupChain, TreeSignTy &TreeAndSign) const;
  /// Generates IR representation for all groups in \p Groups and all affected
  /// trees given by \p AffectedTrees.
  void generateCode(GroupTreesVecTy &Groups,
                    const ArrayRef<Tree *> AffectedTrees) const;
  /// Returns true if T1 and T2 contain similar values.
  bool treesMatch(const Tree *T1, const Tree *T2) const;
  /// Create clusters of the trees in AllTrees.
  void clusterTrees(TreeVecTy &AllTrees,
                    SmallVectorImpl<TreeArrayTy> &TreeClusters);
  /// Grow the tree upwards, towards the definitions.
  bool growTree(TreeVecTy &AllTrees, Tree *T, WorkListTy &&WorkList);
  /// Returns true if all uses of a \p Leaf are from one of a tree in
  /// \p TreeCluster, false otherwise. Additionally for each such use a Tree *
  /// and Leaf index pair is put to \p WorkList.
  bool areAllUsesInsideTreeCluster(
      TreeArrayTy &TreeCluster, const Value *Leaf,
      SmallVectorImpl<std::pair<Tree *, Tree::NodeItTy>> &WorkList) const;
  /// Returns true if we were able to find a leaf with multiple uses from trees
  /// in \p TreeCluster only, false otherwise. Each found use is pushed to a \p
  /// WorkList as a LinearTree* and Leaf index pair.
  bool
  getSharedLeave(TreeArrayTy &TreeCluster,
                 SmallVectorImpl<std::pair<Tree *, Tree::NodeItTy>> &WorkList);
  /// Enlarge trees in \p TreeCluster by growing them towards shared leaves.
  void extendTrees(TreeVecTy &AllTrees, TreeArrayTy &TreeCluster);
  /// Build initial trees from code in \p BB and put them to \p AllTrees.
  void buildInitialTrees(BasicBlock *BB, TreeVecTy &AllTrees);
  /// Build all trees within \p BB.
  void buildTrees(BasicBlock *BB, TreeVecTy &AllTrees,
                  SmallVector<TreeArrayTy, 8> &Clusters, bool UnshareLeaves);

  /// Debug print functions.
  void dumpTreeVec(const TreeVecTy &TreeVec) const;
  void dumpTreeArray(const TreeArrayTy &TreeVec) const;
  void dumpTreeArrayVec(SmallVectorImpl<TreeArrayTy> &Clusters) const;
  void dumpGroups(const GroupTreesVecTy &Groups) const;
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
