//===- AddSubReassociate.h - Reassociate add/sub expressions ----*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
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

//
// This pass reassociates add-sub chains to improve expression reuse
//
// For example: X = A - B - C  -->  X = A - (B + C)
//              Y = A + B + C  -->  Y = A + (B + C)
//
class AddSubReassociatePass : public PassInfoMixin<AddSubReassociatePass> {
  // Forward declarations.
  class Group;

  // Typedefs.
  using InstrVecTy = SmallVector<Instruction *, 8>;
  using ValSetTy = SmallPtrSet<Value *, 16>;
  using IVMapTy = DenseMap<Instruction *, Value *>;

public:
  // Entry point for AddSub reassociation.
  bool runImpl(Function *F, ScalarEvolution *SE);
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

private:
  class OpcodeData {
    friend class Group;

  private:
    unsigned Opcode;

  public:
    OpcodeData() : Opcode(0) {}
    OpcodeData(unsigned Opcode) : Opcode(Opcode) {}
    // The Add/Sub opcode of the leaf.
    unsigned getOpcode() const { return Opcode; }
    hash_code getHash() const {
      hash_code Hash = hash_combine(Opcode);
      return Hash;
    }
    bool isUndef() const { return Opcode == 0; }
    // Compare only the canonicalized +/- opcode.
    bool hasSameAddSubOpcode(const OpcodeData &OD2) const {
      return Opcode == OD2.Opcode;
    }
    // Compare the whole opcode.
    bool operator==(const OpcodeData &OD2) const {
      return Opcode == OD2.Opcode;
    }
    bool operator!=(const OpcodeData &OD2) const { return !(*this == OD2); }
    OpcodeData getFlipped() const;
    // For debugging.
    void dump() const;
  };

  struct LeafUserPair {
    LeafUserPair(Value *L, Instruction *U, const OpcodeData &Opcode)
        : Leaf(L), User(U), Opcode(Opcode) {}
    // The tree leaf.
    Value *Leaf;
    // The trunk node that 'Leaf' is attached to.
    Instruction *User;
    // The canonicalized opcode that corresponds to the 'Leaf'.
    // This includes associative instructions like '<< 4'.
    OpcodeData Opcode;

    hash_code getHash() const {
      // TODO: Check if this hashing is not leading to conflicts.
      hash_code Hash(0);
      // A leaf is uniquely identified by the Leaf and its User within a tree.
      // However, hashing is used to compare nodes across trees. Therefore we
      // need to use all three: Leaf, User and Opcode.
      Hash = hash_combine(Hash, Leaf, User, Opcode.getHash());
      return Hash;
    }
    // Debug print.
    void dump(const unsigned Padding) const {
      dbgs().indent(Padding) << "Leaf ";
      Opcode.dump();
      dbgs() << *Leaf << "\n";
      dbgs().indent(Padding) << "   User: " << *User << "\n";
    }
    // The Leaf and User uniquely identify the LeafUserPair.
    bool operator==(const LeafUserPair &Pair2) const {
      return Leaf == Pair2.Leaf && User == Pair2.User;
    }
  };
  using LUSetTy = std::unordered_set<LeafUserPair, HashIt<LeafUserPair>>;
  using LUPairVecTy = SmallVector<LeafUserPair, 16>;

  // The expression tree.
  class Tree {
  private:
    // Unique tree identifier. Used only for debugging.
    int Id = 0;
    // The root instruction of the tree.
    Instruction *Root = nullptr;
    // A vector of all the leaves and their corresponding users.
    // NOTE: Multiple identical leaves are allowed.
    // Main data structure for leaves and their users.
    LUPairVecTy LUVec;

  public:
    Tree() : Root(nullptr) {
      static int IdCnt;
      Id = IdCnt++;
    }
    int getId() const { return Id; }
    // Update the root of the tree.
    void setRoot(Instruction *R);
    // Return the root of the tree.
    Instruction *getRoot() const { return Root; }
    unsigned getLeavesCount() const { return LUVec.size(); }
    // Append Leaf along with its user U (part of the main tree trunk).
    // For example: ... Leaf
    //                |/
    //                U
    //                |
    void appendLeaf(Instruction *User, Value *Leaf, const OpcodeData &Opcode);
    void removeLeaf(unsigned);
    // Returns the leaf & user pair at 'idx'.
    const LeafUserPair &getLeafUserPair(int Idx) const { return LUVec[Idx]; }
    bool getLeafUsers(Value *Leaf, InstrVecTy &UsersVec);
    // Returns the vector of leaves (bottom-up).
    const LUPairVecTy &getLeavesAndUsers() const { return LUVec; }
    // Return true if we can match 'Leaf' with 'Opcode'. If found, also mark it
    // in VisitedLUs.
    bool matchLeaf(Value *Leaf, const OpcodeData &Opcode,
                   LUSetTy &VisitedLUs) const;
    // Returns the opcode of one of the trunk instructions that the leaf that
    // matches 'Leaf' will be attached to in the canonicalized (linearized)
    // form. If 'OpcodeToMatch' is provided, try to match it.
    OpcodeData
    getLeafCanonOpcode(Value *Leaf, LUSetTy &VisitedLUs,
                       const OpcodeData &OpcodeToMatch = OpcodeData()) const;
    // Return the user (trunk) instruction of 'Leaf'.
    // Since a tree can contain more than one identical leaves, we use
    // 'VisitedLUs' to mark the ones already visited.
    Instruction *getNextLeafUser(Value *Leaf, LUSetTy &VisitedLUs) const;
    // 'U' is set as the user of 'Leaf'.
    bool replaceLeafUser(Value *Leaf, Instruction *OldU, Instruction *NewU);
    // Returns true if 'Leaf' is a leaf of this tree.
    bool hasLeaf(const Value *Leaf) const;
    // Returns true if this tree is larger than 'T2'.
    bool operator<(const Tree &T2) const {
      return getLeavesCount() > T2.getLeavesCount();
    }
    // Debug print.
    void dump() const;
  };

  using TreePtr = std::unique_ptr<Tree>;
  using TreeVecTy = SmallVector<TreePtr, 2>;
  using TreeArrayTy = MutableArrayRef<TreePtr>;
  using WorkListTy = SmallVectorImpl<LeafUserPair>;

  // Represents a group of values that should be enclosed in parentheses.
  class Group {
  public:
    using ValOpTy = std::pair<Value *, OpcodeData>;
    using ValVecTy = SmallVector<ValOpTy, 2>;
    // Pairs of Leaves and user opcodes in bottom-up order.
    ValVecTy Values;

  public:
    Group() {}
    Group(Value *V, const OpcodeData &UserOpc) {
      Values.push_back({V, UserOpc});
    }
    bool empty() const { return Values.empty(); }
    // Return the vector of Leaves and user opcodes in bottom-up order.
    const ValVecTy &getValues() const { return Values; }
    void appendLeaf(Value *Leaf, const OpcodeData &Opcode) {
      Values.push_back({Leaf, Opcode});
    }
    void pop_back() { Values.pop_back(); }
    size_t size() const { return Values.size(); }
    ValOpTy operator[](int idx) { return Values[idx]; }
    OpcodeData getOpcodeFor(Value *V) const {
      for (auto &Pair : Values) {
        if (Pair.first == V) {
          return Pair.second;
        }
      }
      llvm_unreachable("V not found in group");
    }
    bool containsValue(const Value *const V) const;
    // Return the opcode of the Idx'th trunk instruction.
    unsigned getTrunkOpcode(int Idx) const { return Values[Idx].second.Opcode; }
    // Change the trunk opcodes from Add to Sub and vice versa.
    void flipOpcodes();
    // Debug dump.
    void dumpDepth(int Depth = 1) const;
    void dump() const;
  };

  using GroupsVec = SmallVector<Group, 4>;

  // Checks that instructions between root and leaves are in
  // canonical form, otherwise asserts with an error.
  static void checkCanonicalized(Tree &T);

  // Find the common leaves across the trees in TreeCluster.
  void getCommonLeaves(const TreeArrayTy &TreeCluster,
                       SmallPtrSet<Value *, 8> &CommonLeaves);
  // Returns true if group 'G' can be legally applied to all trees in
  // 'TreeCluster'.
  bool isGroupLegal(Group &G, const TreeArrayTy &TreeCluster);
  // Form groups of nodes that reduce divergence across trees in TreeCluster.
  void buildMaxReuseGroups(const TreeArrayTy &TreeCluster,
                           GroupsVec &AllBestGroups);
  // Remove the old dead trunk instructions.
  void removeDeadTrunkInstrs(Tree *T, Instruction *OldRootI) const;
  // Massage the code in T to be a flat single-branch +/- expression tree.
  bool canonicalizeIRForTree(Tree &T) const;
  // Linearize the code that corresponds to the trees in TreeVec.
  bool canonicalizeIRForTrees(const TreeArrayTy &TreeArray) const;
  // Returns true if T1 and T2 contain similar values.
  bool treesMatch(const Tree *T1, const Tree *T2) const;
  // Create clusters of the trees in TreeVec.
  void clusterTrees(TreeVecTy &TreeVec,
                    SmallVectorImpl<TreeArrayTy> &TreeClusters);
  // Grow the tree upwards, towards the definitions.
  bool growTree(Tree *T, WorkListTy &WorkList);
  // Build Add/Sub trees from code in BB.
  void buildInitialTrees(BasicBlock *BB, TreeVecTy &TreeVec);
  // Build all trees within BB.
  void buildTrees(BasicBlock *BB, TreeVecTy &TreeVec,
                  SmallVector<TreeArrayTy, 8> &Clusters);

  // Debug print functions.
  void dumpTreeVec(const TreeVecTy &TreeVec) const;
  void dumpTreeArray(const TreeArrayTy &TreeVec) const;
  void dumpTreeArrayVec(SmallVectorImpl<TreeArrayTy> &Clusters) const;
  void dumpGroups(const GroupsVec &Groups) const;

private:
  ScalarEvolution *SE = nullptr;
};

} // end namespace llvm
#endif // LLVM_TRANSFORMS_SCALAR_ADDSUBREASSOCIATE_H
