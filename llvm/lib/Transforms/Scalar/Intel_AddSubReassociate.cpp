//===- Intel_AddSubReassociate.cpp - Reassociate AddSub expressions -------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

// Introduction
// ============
// AddSub Reassociation performs reassociation on chains of add/sub expressions
// aiming at exposing replicated computation that may otherwise be hidden within
// the expression chain.
//
// Algorithm
// =========
// 1. buildTrees()
// ---------------
// The first step is scanning the code looking for expression trees. This is
// done in buildTrees().
//
// This includes building trees of Add/Sub instructions. The instructions in
// the body of the tree are only allowed to have a single use. The leaf nodes
// are either instructions of a different opcode, non-instruction values, or
// instructions with multiple uses.
// The trees are built in three steps:
// i. buildInitialTrees() builds the initial trees as described above.
// ii. clusterTrees() forms groups of trees that share the same leaf
// instructions and have similar sizes. These groups are good candidates for
// computation reuse across them.
// iii. extendTrees() tries to grow the trees that have
// Add/Sub leaves that are shared across trees in the cluster. During step (i)
// these instructions became leaves because they had more than a single use.
// But, if we can prove at this point that all the uses belong to trees in the
// cluster, then we can "replicate" the leaf instruction across all trees, and
// keep growing the trees towards that direction.
//
// For now, trees do not span more than a single BB. All nodes, including the
// leaves should be in the same BB as the root node.
//
// 2. buildMaxReuseGroups()
// ------------------------
// The second step is to form groups of leaves that maximizes reuse across
// all trees in each cluster. Each group lists a sequence of leaves along with
// the +/- operation associated with it. A group enforces the sequence of
// operations and leaves when applied to the tree.
// We build all possible groups based on the first tree, and we measure the
// divergence caused by applying a group across all trees in our cluster. This
// is done with the getDivergenceCost() function. The group that has the lowest
// cost and that lowers the cost compared to the original code is the one we
// select.
//
// For example: Given this two canonicalized trees:
//   Tree1: ((0 - Leaf3) - Leaf2) + Leaf1
//   Tree2: ((0 + Leaf3) + Leaf2) + Leaf1
//    - Only one operation can be reused, namely + Leaf1.
//    - The group that maximizes reuse is (+ Leaf2, + Leaf3), because it can
//    save 1 instruction, leading to this code:
//   Tree1: Leaf1 - ((0 + Leaf3) + Leaf2)
//   Tree2: Leaf1 + ((0 + Leaf3) + leaf2)
//
//
// 3. canonicalizeIRForTrees()
// ---------------------------
// We first transform the code represented by the trees into a canonicalized
// form. This form is a linearized form of the Add/Sub tree. The body of the
// tree becomes a single linearized trunk. All leaves are on the right-hand-side
// operand (operand 1) and a constant zero is placed at the top. Please note
// that we change IR at this step.
//
// For example: Leaf1 - (Leaf2 + Leaf3)
//     becomes: ((0 - Leaf3) - Leaf2) + Leaf1
//
//      Leaf2 Leaf3      0 Leaf3
//          \ /          |/
//    Leaf1  +           - Leaf2
//        \ /            |/
//         -      --->   - Leaf1
//                       |/
//                       +
//
// 4. generateCode()
// -----------------
// In this final step, we transform the code in order to expose the computation
// described by the group of the previous step. This is done by creating a
// branch off the main trunk of the canonicalized code, with all the leaves and
// their corresponding add/sub operations in the same order as listed in the
// group. This code transformation exposes the redundant computation such that
// later redundancy elimination passes (e.g., GVN) can easily remove the
// redundancies.
//
// For example:
//    Tree1: ((0 - Leaf3) + Leaf2) + Leaf1
//    Tree2: ((0 + Leaf3) + Leaf2) + Leaf1
//    Group: (+ Leaf1), (+ Leaf2)
//
//  After Code generation:
//     Tree1: (0 + Leaf1) - ((0 + Leaf3) + Leaf2)
//     Tree2: (0 + Leaf1) + ((0 + Leaf3) + leaf2)
//
//    Tree1    Tree2             Tree1            Tree2
//    -----    -----             -----            -----
//   0 Leaf3   0 Leaf3        0 Leaf1 0 Leaf3   0 Leaf1 0 Leaf3
//   |/        |/             |/      |/        |/      |/
//   - Leaf2   + Leaf2  --->  +       + Leaf2   +       + Leaf2
//   |/        |/             |       |/        |       |/
//   - Leaf1   + Leaf1        | ______+         | ______+
//   |/        |/             |/                |/
//   +         +              -                 +
//
//
// Limitations/TODOs
// =================
// - The trees are currently not allowed to cross BBs.
// - The knobs that control complexity (e.g., max tree size, max cluster size,
//   etc.) are currently set to some arbitrary values. They should be tuned.
// - Currently, the generated groups must fully match the nodes across all trees
//   in the clusters. This could be relaxed and still have profitable groups.
// - Currently, we can only handle simple "unary" associations (i.e. << Const).
//   This could be extended to arbitrary associations.

#include "llvm/Transforms/Scalar/Intel_AddSubReassociate.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Utils/Local.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include <algorithm>
#include <cassert>
#include <utility>

using namespace llvm;

#define DEBUG_TYPE "addsub-reassoc"

static cl::opt<bool>
    AddSubReassocEnable("addsub-reassoc-enable", cl::init(true), cl::Hidden,
                        cl::desc("Enable addsub reassociation."));

static cl::opt<bool> EnableAddSubVerifier("addsub-reassoc-verifier",
                                          cl::init(false), cl::Hidden,
                                          cl::desc("Enable addsub verifier."));

// The maximum size difference allowed for trees within a cluster.
static cl::opt<unsigned>
    MaxTreeSizeDiffForCluster("addsub-reassoc-max-tree-size-diff", cl::init(1),
                              cl::Hidden,
                              cl::desc("The maximum tree size difference "
                                       "allowed within a cluster of TreeVec."));

// The maximum size of clusters formed. This reduces the complexity of tree
// extension towards shared leaves.
static cl::opt<unsigned>
    MaxClusterSize("addsub-reassoc-max-cluster-size", cl::init(16), cl::Hidden,
                   cl::desc("The maximum size of a cluster of trees."));

// Forming clusters is quadratic to the number of trees. We limit the number of
// trees considered for clustering.
static cl::opt<unsigned> MaxClusterSearch(
    "addsub-reassoc-max-cluster-search", cl::init(16), cl::Hidden,
    cl::desc("Limit the search performed while forming clusters."));

static cl::opt<unsigned> TreeMatchThreshold(
    "addsub-reassoc-tree-match-threshold", cl::init(50), cl::Hidden,
    cl::desc("Trees match only if at least this number (%) of leaves match."));

// The minimum cluster size.
static cl::opt<unsigned> MinClusterSize(
    "addsub-reassoc-min-cluster-size", cl::init(2), cl::Hidden,
    cl::desc(
        "A cluster has to be at least this big to be considered for reassoc."));

static cl::opt<int> BestVariantScoreThreshold(
    "addsub-reassoc-best-score-threshold", cl::init(40), cl::Hidden,
    cl::desc("If we don't reach this % score, we don't consider the variant."));

static cl::opt<unsigned>
    MaxTreeSize("addsub-reassoc-max-tree-size", cl::init(16), cl::Hidden,
                cl::desc("Limit the size of the addsub reassoc expressions."));

static cl::opt<unsigned> MaxTreeCount("addsub-reassoc-max-tree-count", cl::init(0),
                                 cl::Hidden,
                                 cl::desc("Maximum number of trees to build."));

static inline bool isAddSubInstr(const Instruction *I) {
  switch (I->getOpcode()) {
  case Instruction::Add:
  case Instruction::Sub:
    return true;
  default:
    return false;
  }
}

// Returns true only if 'V' is an Add or a Sub.
static inline bool isAddSubInstr(const Value *V) {
  return isa<Instruction>(V) && isAddSubInstr(cast<Instruction>(V));
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Helper to print out opcode symbol.
LLVM_DUMP_METHOD static const char *getOpcodeSymbol(unsigned Opcode) {
  switch (Opcode) {
  case Instruction::Add:
    return "+";
  case Instruction::Sub:
    return "-";
  case Instruction::Shl:
    return "<<";
  case 0:
    return " ";
  }
  llvm_unreachable("Bad Opcode");
}
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

static inline bool isAllowedTrunkInstr(const Value *V) {
  return isAddSubInstr(V);
}

// Returns true if 'V1' and 'I2' are in the same BB, or if V1 not an instr.
static bool areInSameBB(Value *V1, Instruction *I2) {
  Instruction *I1 = dyn_cast<Instruction>(V1);
  return !I1 || I1->getParent() == I2->getParent();
}

// Returns true if 'V1' along with its immediate predecessors are in the same BB
// as 'I2'.
static bool arePredsInSameBB(Value *V1, Instruction *I2) {
  Instruction *I1 = dyn_cast<Instruction>(V1);
  if (!I1)
    return true;
  if (!areInSameBB(I1, I2))
    return false;
  for (int OpI = 0, e = I1->getNumOperands(); OpI != e; ++OpI) {
    Value *Op = I1->getOperand(OpI);
    if (!areInSameBB(Op, I2))
      return false;
  }
  return true;
}

// Begin of AddSubReassociatePass::OpcodeData

AddSubReassociatePass::OpcodeData
AddSubReassociatePass::OpcodeData::getFlipped() const {
  OpcodeData NewOpcode = *this;

  switch (Opcode) {
  case Instruction::Add:
    NewOpcode.Opcode = Instruction::Sub;
    break;
  case Instruction::Sub:
    NewOpcode.Opcode = Instruction::Add;
    break;
  default:
    llvm_unreachable("Bad opcode");
  };

  return NewOpcode;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void AddSubReassociatePass::OpcodeData::dump() const {
  dbgs() << "(" << getOpcodeSymbol(Opcode) << ")";
}
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Begin of AddSubReassociatePass::Tree

void AddSubReassociatePass::Tree::setRoot(Instruction *R) {
  assert(isAddSubInstr(R) && "The tree should contain only Add/Sub.");
  Root = R;
}

void AddSubReassociatePass::Tree::appendLeaf(Instruction *User, Value *Leaf,
                                             const OpcodeData &Opcode) {
  LUVec.push_back(LeafUserPair(Leaf, User, Opcode));
}

// The Leaf and User should uniquely identify the LeafPair.
void AddSubReassociatePass::Tree::removeLeaf(unsigned pos) {
  LUVec.erase(LUVec.begin() + pos);
}

bool AddSubReassociatePass::Tree::getLeafUsers(Value *Leaf,
                                               InstrVecTy &UsersVec) {
  bool IsFound = false;

  for (auto &LUPair : LUVec) {
    Value *L = LUPair.Leaf;
    if (Leaf == L) {
      IsFound = true;
      UsersVec.push_back(LUPair.User);
    }
  }
  assert(IsFound && "Leaf not found!");
  return IsFound;
}

bool AddSubReassociatePass::Tree::matchLeaf(Value *Leaf,
                                            const OpcodeData &Opcode,
                                            LUSetTy &VisitedLUs) const {
  for (const auto &LUPair : LUVec) {
    Value *L = LUPair.Leaf;
    const OpcodeData &LOpcode = LUPair.Opcode;
    if (L == Leaf && LOpcode == Opcode && !VisitedLUs.count(LUPair)) {
      VisitedLUs.insert(LUPair);
      return true;
    }
  }
  return false;
}

// Return the opcode of one of the leaves that match 'Leaf'.
// If no OpcodeToMatch is given, return the first match.
AddSubReassociatePass::OpcodeData
AddSubReassociatePass::Tree::getLeafCanonOpcode(
    Value *Leaf, LUSetTy &VisitedLUs,
    const OpcodeData &OpcodeToMatch /*=OpcodeData(0)*/) const {
  assert(Leaf && hasLeaf(Leaf) && "Leaf not in tree.");
  const LeafUserPair *Match = nullptr;
  for (const auto &LUPair : LUVec) {
    Value *L = LUPair.Leaf;
    if (L == Leaf && !VisitedLUs.count(LUPair)) {
      Match = &LUPair;
      if (!OpcodeToMatch.isUndef() || LUPair.Opcode == OpcodeToMatch)
        break;
    }
  }
  assert(Match && "No match?");
  VisitedLUs.insert(*Match);
  return Match->Opcode;
}

Instruction *
AddSubReassociatePass::Tree::getNextLeafUser(Value *Leaf,
                                             LUSetTy &VisitedLUs) const {
  assert(Leaf && hasLeaf(Leaf) && "Leaf not in tree.");
  for (auto &LUPair : LUVec) {
    Value *L = LUPair.Leaf;
    if (L == Leaf && !VisitedLUs.count(LUPair)) {
      VisitedLUs.insert(LUPair);
      return LUPair.User;
    }
  }
  llvm_unreachable("No leaf found!");
}

bool AddSubReassociatePass::Tree::replaceLeafUser(Value *Leaf,
                                                  Instruction *OldU,
                                                  Instruction *NewU) {
  bool IsFound = false;
  for (auto &LUPair : LUVec) {
    Value *L = LUPair.Leaf;
    Value *U = LUPair.User;
    if (L == Leaf && U == OldU) {
      IsFound = true;
      LUPair.User = NewU;
    }
  }
  assert(IsFound && "Leaf not found!");
  return IsFound;
}

bool AddSubReassociatePass::Tree::hasLeaf(const Value *Leaf) const {
  for (const auto &LUPair : LUVec)
    if (Leaf == LUPair.Leaf)
      return true;
  return false;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void AddSubReassociatePass::Tree::dump() const {
  const unsigned Padding = 2;
  dbgs().indent(Padding) << "Tree gid: " << Id << " Size: " << getLeavesCount()
         << "\n";

  std::function<void(Value *)> dumpTreeRec = [&](Value *V) {
    Instruction *I = dyn_cast<Instruction>(V);
    if (I && !hasLeaf(I) && !isa<PHINode>(I))
      for (int i = 0, e = I->getNumOperands(); i != e; ++i) {
        Value *Op = I->getOperand(i);
        if (isa<Instruction>(Op))
          dumpTreeRec(Op);
      }
    // Post-order
    const char *Prefix = nullptr;
    if (hasLeaf(V))
      Prefix = "(Leaf)";
    else if (V == Root)
      Prefix = "(Root)";
    else
      Prefix = "      ";
    dbgs().indent(Padding) << Prefix << " " << *V << "\n";
  };

  dbgs().indent(Padding) << "Whole Tree dump in post-order:\n";
  if (Root && getLeavesCount())
    dumpTreeRec(Root);
  dbgs() << "\n";

  dbgs().indent(Padding) << "Actual contents of Tree in top-down:\n";
  for (auto &LUPair : llvm::reverse(LUVec))
    LUPair.dump(Padding);
  dbgs().indent(Padding) << "Root: ";
  if (Root)
    dbgs().indent(Padding) << *Root;
  else
    dbgs().indent(Padding) << "NULL";
  dbgs() << "\n";
  dbgs() << "\n";
}
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Begin of AddSubReassociatePass::Group

bool AddSubReassociatePass::Group::containsValue(const Value *const V) const {
  for (auto &Pair : Values) {
    const Value *CheckV = V;
    if (Pair.first == CheckV)
      return true;
  }
  return false;
}

void AddSubReassociatePass::Group::flipOpcodes() {
  for (auto &Pair : Values)
    Pair.second = Pair.second.getFlipped();
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Extended dump of a group. We also print the leaf operands to some depth.
LLVM_DUMP_METHOD void
AddSubReassociatePass::Group::dumpDepth(int Depth /* = 1 */) const {
  // Recursive helper dump function.
  std::function<void(Value *, int)> dumpDepth_rec = [&](Value *V, int Depth) {
    Instruction *I = dyn_cast<Instruction>(V);
    if (I && Depth > 0) {
      for (int i = 0, e = I->getNumOperands(); i != e; ++i)
        dumpDepth_rec(I->getOperand(i), Depth - 1);
    }
    dbgs() << *V << "\n";
  };

  dbgs() << "Top-Down Values:\n";
  for (auto &Pair : llvm::reverse(Values)) {
    Value *V = Pair.first;
    const OpcodeData &Opcode = Pair.second;
    assert(Opcode.Opcode == Instruction::Add ||
           Opcode.Opcode == Instruction::Sub);
    dumpDepth_rec(V, Depth);
    dbgs() << " /\n";
    Opcode.dump();
    dbgs() << "\n";
  }
  dbgs() << "\n";
}

LLVM_DUMP_METHOD void AddSubReassociatePass::Group::dump() const {
  dumpDepth(0);
}
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Begin of AddSubReassociatePass

// Get the leaves that are common across all trees in TreeCluster insert them
// into CommonLeaves.
// TODO: A faster implementation of this would be nice.
void AddSubReassociatePass::getCommonLeaves(
    const TreeArrayTy &TreeCluster, SmallPtrSet<Value *, 8> &CommonLeaves) {
  const TreePtr &Tree0 = TreeCluster[0];
  for (auto &LUPair : Tree0->getLeavesAndUsers()) {
    Value *Leaf0 = LUPair.Leaf;
    for (TreePtr &Tptr :
         make_range(std::next(TreeCluster.begin()), TreeCluster.end())) {
      if (Tptr->hasLeaf(Leaf0))
        CommonLeaves.insert(Leaf0);
    }
  }
}

// Returns true if group 'G' can be applied to all trees in 'TreeCluster'.
// This happens only if:
//  i. All opcodes match
// ii. Opcodes match after they are all flipped.
//
// For example, Group (+ A, + B)
//
//    B    B    B
//  |/   |/   |/
//  + A  - A  - A
//  |/   |/   |/
//  +    -    +
//  T0   T1   T2
//
// - can be applied to T0 with no changes
// - can also be applied to T1 with a single opcode flip
// - but *cannot* be applied to T2 in any way.
// Therefore, the group is *not* legal.
//
bool AddSubReassociatePass::isGroupLegal(Group &G,
                                         const TreeArrayTy &TreeCluster) {
  Value *Leaf0 = G.getValues()[0].first;
  const OpcodeData &Opcode0 = G.getValues()[0].second;

  // For each tree check if G can be applied.
  for (const TreePtr &Tptr : TreeCluster) {
    // Leaf0 not found, therefore the group is not legal.
    if (!Tptr->hasLeaf(Leaf0))
      return false;
    // Use the first Leaf of G against the first tree in TreeCluster to check if
    // opcodes should flip or not for this tree.
    LUSetTy VisitedLUs;
    const OpcodeData &Opcode =
      Tptr->getLeafCanonOpcode(Leaf0, VisitedLUs, Opcode0);
    bool MustFlipTreeOpcodes = !Opcode.hasSameAddSubOpcode(Opcode0);
    VisitedLUs.clear();

    // Check if the group can be applied either as it is or with an opcode flip.
    // Go through all Leaves in G.
    for (auto &Pair : G.getValues()) {
      Value *LeafG = Pair.first;
      if (!Tptr->hasLeaf(LeafG))
        return false;
      const OpcodeData &OpcodeG = Pair.second;
      const OpcodeData &TreeOpcode =
          Tptr->getLeafCanonOpcode(LeafG, VisitedLUs);
      const OpcodeData &TreeOpcodeFixed =
          (MustFlipTreeOpcodes) ? TreeOpcode.getFlipped() : TreeOpcode;
      if (TreeOpcodeFixed != OpcodeG)
        return false;
    }
  }
  return true;
}

// Go through each tree in TreeCluster.
// Form all possible groups of size 2 and try to see whether it increases
// instruction reuse across all trees. If it does, keep trying to add more nodes
// to it until it gets to a maximum size.
// A group lists the Leaves their Add/Sub operations in a fixed ordering.
// The group should be small enough that we can apply it across all trees.
//
// Group formation is quadtratic to the size of the tree, so we should keep the
// tree size to a small number.
// Group evaluation across trees is linear to the number of trees in TreeVec.
// TODO: We could use sorted leaves and a search window to reduce complexity.
void AddSubReassociatePass::buildMaxReuseGroups(const TreeArrayTy &TreeCluster,
                                                GroupsVec &BestGroups) {
  if (TreeCluster.empty())
    return;
  SmallSet<Value *, 2> AlreadyInGroup;

  // Get the common leaves across the TreeCluster.
  SmallPtrSet<Value *, 8> CommonLeaves;
  getCommonLeaves(TreeCluster, CommonLeaves);
  assert(!CommonLeaves.empty() && "There should be some common leaves!");

  // TODO: Building the group based on the first tree is not always best.
  //       We need a heuristic to choose which tree to use.
  const TreePtr &Tree0 = TreeCluster[0];
  const unsigned LeavesCount = Tree0->getLeavesCount();
  for (unsigned I = 0; I < LeavesCount ; ++I) {
    // Initialize group with a leaf from the Tree.
    const auto &Pair = Tree0->getLeafUserPair(I);
    Value *Leaf0 = Pair.Leaf;
    const OpcodeData &CanonOpcode0 = Pair.Opcode;
    // Skip if the leaf is not present in all trees or if already in the group.
    if (!CommonLeaves.count(Leaf0) || AlreadyInGroup.count(Leaf0))
      continue;

    Group G(Leaf0, CanonOpcode0);
    // Try to append as many nodes as possible to G to minimize cost.
    for (unsigned J = I + 1; J < LeavesCount; ++J) {
      // Extend the group with another leaf from the Tree.
      const auto &Pair1 = Tree0->getLeafUserPair(J);
      Value *Leaf1 = Pair1.Leaf;
      const OpcodeData &CanonOpcode1 = Pair1.Opcode;
      // Skip if already in a group
      if (!CommonLeaves.count(Leaf1) || AlreadyInGroup.count(Leaf1))
        continue;

      G.appendLeaf(Leaf1, CanonOpcode1);

      // Check if we can apply the group to all trees.
      if (!isGroupLegal(G, TreeCluster)) {
        G.pop_back();
        continue;
      }
    }

    // Skip single entry groups.
    if (G.size() < 2)
      continue;

    assert(isGroupLegal(G, TreeCluster) && "Not legal?");

    // At this point G is the maximum profitable group starting at Tree0[I].
    // We save this group.
    for (auto &Pair : G.getValues())
      AlreadyInGroup.insert(Pair.first);

    // Finally push constructed group to a list.
    BestGroups.push_back(std::move(G));
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void
AddSubReassociatePass::dumpGroups(const GroupsVec &Groups) const {
  int cnt = 0;
  for (const Group &G : Groups) {
    dbgs() << "Group " << cnt++ << "\n";
    G.dump();
  }
}

LLVM_DUMP_METHOD void
AddSubReassociatePass::dumpTreeVec(const TreeVecTy &TreeVec) const {
  int Cnt = 0;
  for (const TreePtr &T : TreeVec) {
    dbgs().indent(2) << "--- Tree " << Cnt++ << "/" << TreeVec.size()
           << " ---\n";
    T->dump();
  }
}

LLVM_DUMP_METHOD void
AddSubReassociatePass::dumpTreeArray(const TreeArrayTy &TreeVec) const {
  int Cnt = 0;
  for (const TreePtr &T : TreeVec) {
    dbgs().indent(2) << "--- Tree " << Cnt++ << "/" << TreeVec.size()
           << " ---\n";
    T->dump();
  }
}

LLVM_DUMP_METHOD void AddSubReassociatePass::dumpTreeArrayVec(
    SmallVectorImpl<TreeArrayTy> &Clusters) const {
  int Cnt = 0;
  for (const TreeArrayTy &TreeArray : Clusters) {
    dbgs() << "Cluster " << Cnt++ << "/" << Clusters.size()
           << " NumTrees: " << TreeArray.size() << "\n";
    dbgs() << "============================\n";
    dumpTreeArray(TreeArray);
    dbgs() << "\n\n";
  }
}
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Returns true if T1 and T2 contain similar values.
// This is linear to the size of the trees.
// TODO: Ideally this should be a constant time calculation.
bool AddSubReassociatePass::treesMatch(const Tree *T1, const Tree *T2) const {
  SmallPtrSet<Value *, 8> T1Values;
  for (auto &LUPair : T1->getLeavesAndUsers())
    T1Values.insert(LUPair.Leaf);

  // Count how many of T2's leaves match.
  int Matches = 0;
  for (auto &LUPair : T2->getLeavesAndUsers())
    if (T1Values.count(LUPair.Leaf))
      Matches++;

  return (Matches * 100) / T1Values.size() > TreeMatchThreshold;
}

// Form clusters of Trees with similar i) size, ii) values.
// This is currently quadratic to TreeVec size.
// TODO: Tune the clustering heuristics.
// TODO: Reduce complexity by sorting based on size?
void AddSubReassociatePass::clusterTrees(
    TreeVecTy &TreeVec, SmallVectorImpl<TreeArrayTy> &TreeClusters) {
  TreeArrayTy TreeVecArray(TreeVec);
  unsigned ClusterSize = 1;
  for (size_t i = 0, e = TreeVec.size(); i != e; i += ClusterSize) {
    const Tree *T1 = TreeVec[i].get();
    // Now look for other trees within TreeVec.
    ClusterSize = 1;
    // Break quadratic complexity by reducing the maximum search.
    int EndOfSearch = std::min(e, i + 1 + MaxClusterSearch);
    for (int j = i + 1; j != EndOfSearch; ++j) {
      Tree *T2 = TreeVec[j].get();
      // If i)  size difference is within limits.
      //    ii) the trees share a min number of values.
      if ((std::abs((int64_t)T2->getLeavesCount() -
                    (int64_t)T1->getLeavesCount()) <
           MaxTreeSizeDiffForCluster) &&
          treesMatch(T2, T1)) {
        // Move this element next to the last member of the cluster.
        auto First = TreeVec.begin() + i + ClusterSize;
        auto Middle = TreeVec.begin() + j;
        auto Last = Middle + 1;
        std::rotate(First,  Middle, Last);
        // Reduce complexity by placing a cap on the size of the cluster
        if (++ClusterSize > MaxClusterSize)
          break;
      }
    }
    // Create a cluster only if the size is adequate.
    if (ClusterSize >= MinClusterSize) {
      TreeArrayTy Cluster = TreeVecArray.slice(i, ClusterSize);
      TreeClusters.push_back(Cluster);
    }
  }
}

// Try to grow the tree upwards, towards the definitions.
// Returns true if new nodes have been added to the tree.
bool AddSubReassociatePass::growTree(Tree *T, WorkListTy &WorkList) {
  unsigned SizeBefore = T->getLeavesCount();

  // Keep trying to grow tree until the WorkList is empty.
  while (!WorkList.empty()) {
    auto LastOp = WorkList.pop_back_val();

    assert(isAllowedTrunkInstr(LastOp.Leaf) &&
           "Work list item can't be trunk instruction.");
    Instruction *I = cast<Instruction>(LastOp.Leaf);

    for (int OpIdx : {0, 1}) {
      Value *Op = I->getOperand(OpIdx);

      // Skip self referencing instructions.
      if (Op == I)
        continue;

      // As we build the tree bottom-up we need to keep track of the
      // 'canonicalized opcode'. This is the opcode that the trunk node will
      // have in the canonicalized form of the tree. For example:
      //   L2  L3
      //    \ /
      // L1  -
      //  \ /
      //   -
      //   |
      //  Root
      // The canonicalized opcodes are: L1:'+', L2:'-', L3:'+'.
      OpcodeData OpCanonOpcode;
      if (I->getOpcode() == Instruction::Sub && OpIdx != 0) {
        // We flip the opcode when we cross the RHS of a SUB.
        OpCanonOpcode = LastOp.Opcode.getFlipped();
      } else {
        // Reuse the opcode of the user.
        OpCanonOpcode = LastOp.Opcode;
      }

      if ( // Keep the size of a tree below a maximum value.
          T->getLeavesCount() + 2 * WorkList.size() < MaxTreeSize &&
          Op->hasOneUse() && arePredsInSameBB(Op, I) &&
          isAddSubInstr(Op)) {
        // Push the operand to the WorkList to continue the walk up the code.
        WorkList.push_back(LeafUserPair(Op, I, OpCanonOpcode));
      } else {
        // 'Op' is a leaf node, so stop growing and add it into T's leaves.
        T->appendLeaf(I, Op, OpCanonOpcode);
      }
    }
  }
  bool Changed = SizeBefore != T->getLeavesCount();
  return Changed;
}

void AddSubReassociatePass::buildInitialTrees(BasicBlock *BB,
                                              TreeVecTy &TreeVec) {
  // Returns true if 'I' is a candidate for a tree root.
  // We don't need to check whether 'I' is in other trees, because if it is
  // considered as a rootCandidate, it must have more than one uses,
  // therefore it is not part of a tree.
  // NOTE: This will miss out root nodes in case of trees that have been
  // truncated because of their size.
  auto isRootCandidate = [&](const Instruction *I) {
    // Conditions: i.  Add/Sub opcode,
    //             ii. Multiple users, or single user that is not an Add/Sub
    //                 or a candidate AssocInstr.
    if (!isAddSubInstr(I))
      return false;

    for (auto *U : I->users()) {
      const Instruction *UI = dyn_cast<const Instruction>(U);
      if (!isAllowedTrunkInstr(UI)) {
        // This instruction is a good root candidate since it is Add/Sub and
        // can't be part of another tree.
        return true;
      }
    }
    return false;
  };

  // Scan the BB in reverse and build a tree.
  for (Instruction &I : make_range(BB->rbegin(), BB->rend())) {
    SmallVector<LeafUserPair, 8> WorkList;

    // Check that number of built trees doesn't exceed specified limits.
    // '0' means "unlimited" unless it is set by the user.
    if ((MaxTreeCount.getNumOccurrences() > 0 || MaxTreeCount > 0) &&
        TreeVec.size() >= (unsigned)MaxTreeCount)
      break;

    // A tree is rooted at an Add/Sub instruction that is not already in a tree.
    if (!isRootCandidate(&I))
      continue;

    TreePtr UTree = llvm::make_unique<Tree>();
    Tree *T = UTree.get();
    WorkList.push_back(LeafUserPair(&I, nullptr, Instruction::Add));
    T->setRoot(&I);
    growTree(T, WorkList);

    assert(1 <= T->getLeavesCount() && T->getLeavesCount() <= MaxTreeSize + 2 &&
           "Tree size should be capped");

    // Skip trivial trees.
    if (T->getLeavesCount() > 1)
      TreeVec.push_back(std::move(UTree));
  }
}

// Build Add/Sub trees with instructions from BB.
void AddSubReassociatePass::buildTrees(BasicBlock *BB, TreeVecTy &TreeVec,
                                       SmallVector<TreeArrayTy, 8> &Clusters) {
  // 1. Scan the code in BB and build initial trees.
  buildInitialTrees(BB, TreeVec);
  if (TreeVec.empty())
    return;

  // 2. Create clusters of trees out of the trees in TreeVec. Each cluster
  // contains trees that: i.  share leaf nodes, and ii. have similar sizes.
  // The trees in a cluster are good candidates for AddSub reassociation.
  //
  // It is important to form clusters at this point because
  // 'extendTrees()' is quadratic to the number of trees
  // processed. This, could potentially reduce the effectiveness of that
  // function. The assumption here is that extending the trees is beneficial
  // only if the trees are already similar enough.
  // TODO: It might be worth to have two separate clustering steps, one here
  // and one before the group formation with different heuristics/parameters
  // each.
  //
  // NOTE: This reorders the elements in TreeVec.
  clusterTrees(TreeVec, Clusters);
}

// Entry point for AddSub reassociation.
// Returns true if we need to invalidate analysis.
bool AddSubReassociatePass::runImpl(Function *F, ScalarEvolution *Se) {
  bool Changed = false;
  ReversePostOrderTraversal<Function *> RPOT(F);

  if (!AddSubReassocEnable)
    return false;

  // TODO: Is there a better way of doing this?
  SE = Se;

  // Make a "pairmap" of how often each operand pair occurs.
  for (BasicBlock *BB : RPOT) {
    TreeVecTy TreeVec;
    SmallVector<TreeArrayTy, 8> TreeClusters;
    // 1. Build as many trees as we can find in BB.
    buildTrees(BB, TreeVec, TreeClusters);
    // We did not collect any clusters. Continue looking for more trees.
    if (TreeClusters.empty())
      continue;

    for (TreeArrayTy &TreeCluster : TreeClusters) {
      // 2. Form groups of nodes that reduce divergence
      GroupsVec BestGroups;
      buildMaxReuseGroups(TreeCluster, BestGroups);
      if (BestGroups.empty())
        continue;

      // The score is the instruction count savings (# before  - # after).
      // If we apply group G, we are:
      //  i.  introducing G.size()-1 new instructions (for the tmp value), and
      //  ii. removing G.size()*TreeCluster.size() instructions from the trees
      // Example:
      //  Group: (+ A + B)
      //     B     B           tmp = A + B
      //   |/    |/
      //   + A   - A        --> | tmp | tmp
      //   |/    |/             |/    |/
      //   +     -              +     -
      //  Tree0  Tree1         Tree0 Tree1
      //  total: 4 instrs      total: 3 instrs

      // First take total number of cloned instructions into account.
      int Score = 0;
      // Compute instruction difference for each group.
      for (auto &G : BestGroups) {
        int OrigScore =
            G.size() * TreeCluster.size();
        int NewScore = (G.size() - 1 + TreeCluster.size());
        Score += OrigScore - NewScore;
      }

      // Original tree (before canonicalization) could have one instruction
      // less. Lets be conservative and assume this is always the case.
      Score -= TreeCluster.size();

      // Check if transformation is profitable.
      if (Score <= 0)
        continue;
    }
  }
  return Changed;
}

PreservedAnalyses AddSubReassociatePass::run(Function &F,
                                             FunctionAnalysisManager &AM) {
  // Scan the code for opportunities of AddSub reassociation.
  auto *SE = &AM.getResult<ScalarEvolutionAnalysis>(F);
  bool Changed = runImpl(&F, SE);

  if (Changed) {
    PreservedAnalyses PA;
    PA.preserveSet<CFGAnalyses>();
    PA.preserve<GlobalsAA>();
    return PA;
  }

  return PreservedAnalyses::all();
}

namespace {
class AddSubReassociateLegacyPass : public FunctionPass {
  AddSubReassociatePass Impl;

public:
  static char ID; // Pass identification, replacement for typeid

  AddSubReassociateLegacyPass() : FunctionPass(ID) {
    initializeAddSubReassociateLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    auto *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    return Impl.runImpl(&F, SE);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    FunctionPass::getAnalysisUsage(AU);
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.setPreservesCFG();
    AU.addPreserved<GlobalsAAWrapperPass>();
  }
};
} // end anonymous namespace

char AddSubReassociateLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(AddSubReassociateLegacyPass, "addsub-reassoc",
                      "AddSub Reassociation", false, false)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_END(AddSubReassociateLegacyPass, "addsub-reassoc",
                    "AddSub Reassociation", false, false)

// Public interface to the Reassociate pass
FunctionPass *llvm::createAddSubReassociatePass() {
  return new AddSubReassociateLegacyPass();
}
