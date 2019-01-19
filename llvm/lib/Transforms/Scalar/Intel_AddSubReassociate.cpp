//===- Intel_AddSubReassociate.cpp - Reassociate AddSub expressions -------===//
//
// Copyright (C) 2018 - 2019 Intel Corporation. All rights reserved.
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
#include "llvm/ADT/MapVector.h"
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
using namespace intel_addsubreassoc;

#define DEBUG_TYPE "addsub-reassoc"

static cl::opt<bool>
    AddSubReassocEnable("addsub-reassoc-enable", cl::init(true), cl::Hidden,
                        cl::desc("Enable addsub reassociation."));

static cl::opt<unsigned>
    MaxBBIters("addsub-reassoc-max-bb-iters", cl::init(1), cl::Hidden,
                cl::desc("Maximum number of times we try to perform "
                         "reassociation in basic block."));

static cl::opt<bool>
    SimplifyTrunks("addsub-reassoc-simplify-trunks", cl::init(true), cl::Hidden,
                   cl::desc("Enable simplification of trunks."));

static cl::opt<bool>
    SimplifyChains("addsub-reassoc-simplify-chains", cl::init(true), cl::Hidden,
                   cl::desc("Enable simplification of chains."));

static cl::opt<bool>
    EnableGroupCanonicalization("addsub-reassoc-canonicalize-group",
                                cl::init(true), cl::Hidden,
                                cl::desc("Enable canonicalization of groups."));

static cl::opt<bool> EnableMemCanonicalization(
    "addsub-reassoc-canonicalize-mem", cl::init(true), cl::Hidden,
    cl::desc(
        "Enable canonicalization of groups based on the memory accesses."));

static cl::opt<unsigned> MemCanonicalizationMaxGroupSize(
    "addsub-reassoc-memcan-max-group-size", cl::init(8), cl::Hidden,
    cl::desc(
        "The maximum group size to be considered for mem canonicalization."));

static cl::opt<int> MemCanonicalizationMaxLookupDepth(
    "addsub-reassoc-memcan-max-lookup-depth", cl::init(4), cl::Hidden,
    cl::desc("The maximum distance we are going to search for matching groups "
             "within BestGroups."));

static cl::opt<bool> EnableUnaryAssociations(
    "addsub-reassoc-memcan-enable-unary-associations", cl::init(true),
    cl::Hidden,
    cl::desc("Enable non-add/sub tree members in the tree, e.g. (<< 4)."));

static cl::opt<unsigned> MaxUnaryAssociations(
    "addsub-reassoc-max-unary-associations", cl::init(1), cl::Hidden,
    cl::desc(
        "The maximum number of allowed non-add/sub associations in the tree."));

static cl::opt<bool> OptimizeUnaryAssociations(
    "addsub-reassoc-optimize-unary-associations", cl::init(false), cl::Hidden,
    cl::desc("Optimize code generation for the associative instructions."));

static cl::opt<bool> EnableSharedLeaves(
    "addsub-reassoc-unshare-leaves", cl::init(true), cl::Hidden,
    cl::desc("Enable growing the trees towards shared leaves"));

static cl::opt<bool> EnableAddSubVerifier("addsub-reassoc-verifier",
                                          cl::init(false), cl::Hidden,
                                          cl::desc("Enable addsub verifier."));

static cl::opt<int>
    MaxSharedNodesIterations("addsub-reassoc-max-unshared-leaves", cl::init(8),
                             cl::Hidden,
                             cl::desc("The maximum number of attempts for "
                                      "adding shared nodes to the trees."));

// The maximum size difference allowed for trees within a cluster.
static cl::opt<unsigned>
    MaxTreeSizeDiffForCluster("addsub-reassoc-max-tree-size-diff", cl::init(1),
                              cl::Hidden,
                              cl::desc("The maximum tree size difference "
                                       "allowed within a cluster of trees."));

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

static cl::opt<unsigned>
    MaxTreeCount("addsub-reassoc-max-tree-count", cl::init(0), cl::Hidden,
                 cl::desc("Maximum number of trees to build."));

static cl::opt<bool>
    ReuseChain("addsub-reassoc-reuse-chain", cl::init(true), cl::Hidden,
               cl::desc("Enables chains reuse during code generation."));

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

// Returns true if 'I' is an associative instruction of the allowed opcode.
// We currently allow instructions with one constant operand and one add/sub.
static inline bool isAllowedAssocInstr(const Instruction *I) {
  if (!EnableUnaryAssociations)
    return false;

  switch (I->getOpcode()) {
  case Instruction::Shl:
    // Shifts should have a constant RHS operand.
    return isa<Constant>(I->getOperand(1));
  default:
    return false;
  }
}

// Returns true if 'V' is an Associative instruction.
static inline bool isAllowedAssocInstr(const Value *V) {
  return isa<Instruction>(V) && isAllowedAssocInstr(cast<Instruction>(V));
}

static bool isAddSubInstr(const Instruction *I, const DataLayout &DL) {
  switch (I->getOpcode()) {
  case Instruction::Add:
  case Instruction::Sub:
    return true;
  case Instruction::Or:
    return haveNoCommonBitsSet(I->getOperand(0), I->getOperand(1), DL);
  default:
    return false;
  }
}

static bool isAllowedTrunkInstr(const Value *V, const DataLayout &DL) {
  auto *I = dyn_cast<Instruction>(V);
  return (I != nullptr) ? isAddSubInstr(I, DL) || isAllowedAssocInstr(I)
                        : false;
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

// Replace an Add/Sub 'I' with an equivalent Sub/Add instruction.
static Instruction *flipInstruction(Instruction *I, bool FlipOperands = false) {
  Instruction *NewI = nullptr;
  Value *Op0 = I->getOperand(0);
  Value *Op1 = I->getOperand(1);

  if (FlipOperands)
    std::swap(Op0, Op1);

  switch (I->getOpcode()) {
  case Instruction::Add:
    NewI = BinaryOperator::CreateSub(Op0, Op1, I->getName());
    break;
  case Instruction::Sub:
    NewI = BinaryOperator::CreateAdd(Op0, Op1, I->getName());
    break;
  default:
    llvm_unreachable("Only Add/Sub allowed.");
  }
  NewI->insertBefore(I);
  I->replaceAllUsesWith(NewI);
  I->dropAllReferences();
  I->eraseFromParent();
  return NewI;
}

static Value *skipAssocs(const OpcodeData &OD, Value *Leaf) {
  if (!EnableUnaryAssociations)
    return Leaf;

  for (auto &AOD : OD) {
    if (isAllowedAssocInstr(Leaf)) {
      const Instruction *I = cast<const Instruction>(Leaf);
      AssocOpcodeData CheckAOD = AssocOpcodeData(I);
      if (AOD == CheckAOD) {
        Leaf = I->getOperand(0);
        continue;
      }
    }
  }
  return Leaf;
}

// Begin of AddSubReassociatePass::AssocOpcodeData

AssocOpcodeData::AssocOpcodeData(const Instruction *I) {
  assert(isAllowedAssocInstr(I) && "Expected Assoc Instr");
  Opcode = I->getOpcode();
  assert((isa<Constant>(I->getOperand(0)) || isa<Constant>(I->getOperand(1))) &&
         "Expected exactly one constant operand");
  Const = isa<Constant>(I->getOperand(0)) ? cast<Constant>(I->getOperand(0))
                                          : cast<Constant>(I->getOperand(1));
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void AssocOpcodeData::dump() const {
  dbgs() << "(" << getOpcodeSymbol(Opcode);
  if (Const)
    dbgs() << " " << *Const;
  dbgs() << ")";
}
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Begin of AddSubReassociatePass::OpcodeData

OpcodeData OpcodeData::getFlipped() const {
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
LLVM_DUMP_METHOD void OpcodeData::dump() const {
  dbgs() << "(" << getOpcodeSymbol(Opcode) << ")";
  for (auto &AssocOpcode : AssocOpcodeVec)
    AssocOpcode.dump();
}
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Begin of AddSubReassociatePass::Tree

void Tree::setRoot(Instruction *R) { Root = R; }

void Tree::appendLeaf(Instruction *User, Value *Leaf,
                      const OpcodeData &Opcode) {
  LUVec.push_back(LeafUserPair(Leaf, User, Opcode));
}

// The Leaf and User should uniquely identify the LeafPair.
void Tree::removeLeaf(unsigned pos) { LUVec.erase(LUVec.begin() + pos); }

bool Tree::matchLeaf(Value *Leaf, const OpcodeData &Opcode,
                     LUSetTy &VisitedLUs) const {
  for (const auto &LUPair : LUVec) {
    Value *L = LUPair.getLeaf();
    const OpcodeData &LOpcode = LUPair.getOpcodeData();
    if (L == Leaf && LOpcode == Opcode && !VisitedLUs.count(LUPair)) {
      VisitedLUs.insert(LUPair);
      return true;
    }
  }
  return false;
}

// Return the opcode of one of the leaves that match 'Leaf'.
// If no OpcodeToMatch is given, return the first match.
OpcodeData Tree::getLeafCanonOpcode(
    Value *Leaf, LUSetTy &VisitedLUs,
    const OpcodeData &OpcodeToMatch /*=OpcodeData(0)*/) const {
  assert(Leaf && hasLeaf(Leaf) && "Leaf not in tree.");
  const LeafUserPair *Match = nullptr;
  for (const auto &LUPair : LUVec) {
    Value *L = LUPair.getLeaf();
    if (L == skipAssocs(LUPair.getOpcodeData(), Leaf) &&
        !VisitedLUs.count(LUPair)) {
      Match = &LUPair;
      if (!OpcodeToMatch.isUndef() || LUPair.getOpcodeData() == OpcodeToMatch)
        break;
    }
  }
  assert(Match && "No match?");
  VisitedLUs.insert(*Match);
  return Match->getOpcodeData();
}

LeafUserPair &Tree::getNextLeafUserPair(Value *Leaf, LUSetTy &VisitedLUs) const {
  assert(Leaf && hasLeaf(Leaf) && "Leaf not in tree.");
  for (auto &LUPair : LUVec) {
    Value *L = LUPair.getLeaf();
    if (L == Leaf && !VisitedLUs.count(LUPair) && LUPair.getUser() != nullptr) {
      VisitedLUs.insert(LUPair);
      return const_cast<LeafUserPair &>(LUPair);
    }
  }
  llvm_unreachable("No leaf found!");
}

bool Tree::replaceLeafUser(Value *Leaf, Instruction *OldU, Instruction *NewU) {
  bool IsFound = false;
  for (auto &LUPair : LUVec) {
    Value *L = LUPair.getLeaf();
    Value *U = LUPair.getUser();
    if (L == Leaf && U == OldU) {
      IsFound = true;
      LUPair.User = NewU;
    }
  }
  assert(IsFound && "Leaf not found!");
  return IsFound;
}

bool Tree::hasLeaf(Value *Leaf) const {
  for (const auto &LUPair : LUVec)
    if (skipAssocs(LUPair.getOpcodeData(), Leaf) == LUPair.getLeaf())
      return true;
  return false;
}

bool Tree::hasTrunkInstruction(const Instruction *I) const {
  // Note: Currently maximum tree size is limited to 16. That's why we are doing
  // full traversal instead of using set. Please consider changing this as
  // needed.
  std::function<bool(Instruction *)> visitTreeRec =
      [&](Instruction *TreeI) -> bool {
    if (I == TreeI)
      return true;
    for (int i = 0, e = TreeI->getNumOperands(); i != e; ++i) {
      Instruction *Op = dyn_cast<Instruction>(TreeI->getOperand(i));
      if (Op != nullptr && isAllowedTrunkInstr(Op, DL) && !hasLeaf(Op) &&
          visitTreeRec((Op)))
        return true;
    }
    return false;
  };
  return getRoot() != nullptr && visitTreeRec(getRoot());
}

void Tree::clear() {
  LUVec.clear();
  Root = nullptr;
  HasSharedLeafCandidate = false;
  SharedLeavesCount = 0;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void Tree::dump() const {
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

bool Group::containsValue(Value *V) const {
  for (auto &Pair : Values)
    if (Pair.first == skipAssocs(Pair.second, V))
      return true;
  return false;
}

bool Group::isSimilar(const Group &G2) {
  // Sizes should match.
  if (G2.size() != size())
    return false;
  SmallSet<unsigned, 8> G2Opcodes;
  for (size_t I = 0, e = G2.size(); I != e; ++I)
    G2Opcodes.insert(G2.getTrunkOpcode(I));

  for (size_t I = 0, e = size(); I != e; ++I)
    if (!G2Opcodes.count(getTrunkOpcode(I)))
      return false;
  return true;
}

void Group::sort() {
  auto valuesCmp = [](const ValOpTy &VO1, const ValOpTy &VO2) -> bool {
    Value *V1 = VO1.first;
    const OpcodeData &OD1 = VO1.second;
    Value *V2 = VO2.first;
    const OpcodeData &OD2 = VO2.second;

    Instruction *I1 = dyn_cast<Instruction>(V1);
    Instruction *I2 = dyn_cast<Instruction>(V2);
    if (I1 && I2) {
      // If instr opcodes differ, sort by their opcode.
      if (I1->getOpcode() != I2->getOpcode())
        return I1->getOpcode() < I2->getOpcode();
      // If the trunk opcodes differ, sort by them.
      if (OD1 != OD2)
        return OD1 < OD2;
      return false;
    }
    if ((I1 && !I2) || (I1 && !I2))
      return (bool)I1 < (bool)I2;
    return false;
  };
  std::sort(Values.begin(), Values.end(), valuesCmp);
}

void Group::flipOpcodes() {
  for (auto &Pair : Values)
    Pair.second = Pair.second.getFlipped();
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Extended dump of a group. We also print the leaf operands to some depth.
LLVM_DUMP_METHOD void Group::dumpDepth(int Depth /* = 1 */) const {
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
    assert(Opcode.getOpcode() == Instruction::Add ||
           Opcode.getOpcode() == Instruction::Sub);
    dumpDepth_rec(V, Depth);
    dbgs() << " /\n";
    Opcode.dump();
    dbgs() << "\n";
  }
  dbgs() << "\n";
}

LLVM_DUMP_METHOD void Group::dump() const { dumpDepth(0); }
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Begin of AddSubReassociatePass

#ifndef NDEBUG
// Checks if the code from the root to the leaves is in canonical form.
void AddSubReassociate::checkCanonicalized(Tree &T) const {
  Value *TrunkV = T.getRoot();
  while (isa<Instruction>(TrunkV)) {
    Instruction *TrunkI = cast<Instruction>(TrunkV);
    // Operand 0 of trunk should never be a leaf. It should be either another
    // trunk node or zero.
    assert(isAddSubInstr(TrunkI, DL) &&
           "Only Add/Sub instrs allowed in the trunk");
    TrunkV = TrunkI->getOperand(0);
  }
  assert(isa<Constant>(TrunkV) && cast<Constant>(TrunkV)->isNullValue() &&
         "The only non-instr trunk node allowed is the zero at the top");
}
#endif

Tree *AddSubReassociate::findEnclosingTree(TreeVecTy &AllTrees,
                                           const Instruction *I) {
  for (auto &TreePtr : AllTrees)
    if (TreePtr->hasTrunkInstruction(I))
      return TreePtr.get();
  return nullptr;
}

Tree *AddSubReassociate::findTreeWithRoot(TreeVecTy &AllTrees,
                                          const Instruction *Root,
                                          const Tree *skipTree) {
  for (auto &TreePtr : AllTrees)
    if (TreePtr.get() != skipTree && TreePtr->getRoot() == Root)
      return TreePtr.get();
  return nullptr;
}

// Returns true if we were able to compute distance of V1 and V2 or one of their
// operands, false otherwise.
bool AddSubReassociate::getValDistance(Value *V1, Value *V2, int MaxDepth,
                                       int64_t &Distance) {
  if (MaxDepth == 0)
    return false;
  Instruction *I1 = dyn_cast<Instruction>(V1);
  Instruction *I2 = dyn_cast<Instruction>(V2);
  if ((I1 && !I2) || (!I1 && I2))
    return false;
  if (I1 && I2) {
    if (I1->getOpcode() != I2->getOpcode())
      return false;

    LoadInst *LI1 = dyn_cast<LoadInst>(I1);
    LoadInst *LI2 = dyn_cast<LoadInst>(I2);
    if (LI1 && LI2) {
      if (LI1->getPointerAddressSpace() != LI2->getPointerAddressSpace())
        return false;
      // Check pointers
      Value *Ptr1 = LI1->getPointerOperand();
      Value *Ptr2 = LI2->getPointerOperand();
      const SCEV *Scev1 = SE->getSCEV(Ptr1);
      const SCEV *Scev2 = SE->getSCEV(Ptr2);
      const SCEV *Diff = SE->getMinusSCEV(Scev1, Scev2);
      if (const SCEVConstant *DiffConst = dyn_cast<SCEVConstant>(Diff)) {
        Distance = DiffConst->getAPInt().getSExtValue();
        return true;
      }
    }
    for (unsigned I = 0, e = I1->getNumOperands(); I != e; ++I) {
      if (getValDistance(I1->getOperand(I), I2->getOperand(I), MaxDepth - 1,
                         Distance))
        return true;
    }
    return false;
  }
  return false;
}

// Returns the sum of the absolute distances of SortedLeaves and G2.
int64_t AddSubReassociate::getSumAbsDistances(Group::ValVecTy &G1Leaves,
                                              const Group &G2) {
  assert(G1Leaves.size() == G2.size() && "Expected same size");
  int64_t Sum = 0;
  for (size_t I = 0, e = G2.size(); I != e; ++I) {
    Value *V1 = G1Leaves[I].first;
    Value *V2 = G2.getValues()[I].first;
    int64_t AbsDist = 0;
    if (getValDistance(V1, V2, 2, AbsDist))
      Sum += std::abs(AbsDist);
    else
      return MAX_DISTANCE;
  }
  return Sum;
}

// Recursively calls itself to explore the different orderings of G1's leaves in
// order to match them best against G2.
// Upon each run, we extend LastSortedG1Leaves by one of the unused leaves of G1
// (by checking the remaining leaves in G1Leaves), and we try to match it
// against the first leaf in G2Leaves. The best ordering is held in
// BestSortedG1Leaves and the best score in BestScore.
// It returns the best score in post-order.
int64_t AddSubReassociate::getBestSortedScoreRec(
    const Group &G1, const Group &G2, Group::ValVecTy G1Leaves,
    Group::ValVecTy G2Leaves, Group::ValVecTy &LastSortedG1Leaves,
    Group::ValVecTy &BestSortedG1Leaves, int64_t &BestScore) {
  // If we reached the bottom, return the total distance.
  if (G2Leaves.empty()) {
    assert(G1Leaves.empty() && "G1Leaves and G2Leaves out of sync.");
    // Now get the sum of the absolute distances between SortedLeaves and G2.
    return getSumAbsDistances(LastSortedG1Leaves, G2);
  }
  // Let's try to match G2's G2LeafIdx'th leaf.
  Value *G2LeafV = G2Leaves.front().first;
  G2Leaves.erase(G2Leaves.begin());

  // Go through the remaining G1 leaves looking for matches with G2LeafV.
  SmallVector<Group::ValOpTy, 4> Matches;
  int64_t Distance = 0;
  for (auto &G1Leaf : G1Leaves)
    if (getValDistance(G2LeafV, G1Leaf.first, 2, Distance))
      Matches.push_back(G1Leaf);
  // Early exit if no match.
  if (Matches.empty())
    return MAX_DISTANCE;

  // Recursively try all matches to find the one that leads to the best score.
  for (auto &G1Leaf : Matches) {
    // Create copies, one for each of the matched leaves.
    Group::ValVecTy SortedG1LeavesCopy = LastSortedG1Leaves;
    SortedG1LeavesCopy.push_back(G1Leaf);
    Group::ValVecTy G1LeavesCopy = G1Leaves;
    G1LeavesCopy.erase(
        std::find(G1LeavesCopy.begin(), G1LeavesCopy.end(), G1Leaf));
    // Get the score by recursively calling self.
    int64_t Score = getBestSortedScoreRec(G1, G2, G1LeavesCopy, G2Leaves,
                                          SortedG1LeavesCopy,
                                          BestSortedG1Leaves, BestScore);
    // Keep the best scores.
    if (Score < BestScore) {
      BestScore = Score;
      if (SortedG1LeavesCopy.size() == G1.size())
        BestSortedG1Leaves = SortedG1LeavesCopy;
    }
  }
  // Post-order
  return BestScore;
}

// Entry point for getBestSortedScoreRec().
// Returns false if we did not manage to get a good ordering that matches G2.
bool AddSubReassociate::getBestSortedLeaves(
    const Group &G1, const Group &G2, Group::ValVecTy &BestSortedG1Leaves) {
  int64_t BestScore = MAX_DISTANCE;
  Group::ValVecTy DummyG1SortedLeaves;
  getBestSortedScoreRec(G1, G2, G1.getValues(), G2.getValues(),
                        DummyG1SortedLeaves, BestSortedG1Leaves, BestScore);
  if (BestSortedG1Leaves.size() != G1.size())
    return false;
  assert(std::is_permutation(BestSortedG1Leaves.begin(),
                             BestSortedG1Leaves.end(),
                             G1.getValues().begin()) &&
         "The sorted vector should be a permutation of current G1 leaves.");
  return true;
}

// Canonicalize: (i) the order of the values in the group, (ii) the trunk
// opcodes, to match the ones in G2. This value ordering takes into account the
// memory accesses primarily.
// Returns true on success.
bool AddSubReassociate::memCanonicalizeGroupBasedOn(Group &G1, const Group &G2,
                                                    ScalarEvolution *SE) {
  Group::ValVecTy SortedG1Leaves;
  if (!getBestSortedLeaves(G1, G2, SortedG1Leaves))
    return false;
  // Update the current group.
  G1.setValues(std::move(SortedG1Leaves));
  return true;
}

// Massage 'G' into a form that matches a similar group in 'BestGroups'.
// Returns true on success.
bool AddSubReassociate::memCanonicalizeGroup(Group &G,
                                             GroupTreesTy &GroupTreeVec,
                                             GroupsTy &BestGroups) {
  // This is an expensive canonicalization, so limit the group size.
  if (G.size() > MemCanonicalizationMaxGroupSize)
    return false;
  // Find a similar group in BestGroups.
  Group *MainGroup = nullptr;
  int LookupDepth = 0;
  for (auto It = BestGroups.rbegin();
       It != BestGroups.rend() &&
       LookupDepth++ < MemCanonicalizationMaxLookupDepth;
       ++It) {
    if (G.isSimilar(It->first)) {
      MainGroup = &It->first;
      break;
    }
  }

  // Canonicalize G based on SimilarG.
  if (MainGroup == nullptr || !memCanonicalizeGroupBasedOn(G, *MainGroup, SE))
    return false;

  // If more than half of the opcodes don't match, flip entire group.
  unsigned CntOpcodeMatches = 0;
  Group &G1 = G;
  Group &G2 = *MainGroup;
  for (size_t I = 0, E = G1.getValues().size(); I != E; ++I)
    CntOpcodeMatches += (G1.getTrunkOpcode(I) == G2.getTrunkOpcode(I)) ? 1 : 0;

  if (CntOpcodeMatches < G1.getValues().size() / 2) {
    G1.flipOpcodes();
    // Change sign of a "bridge" for all trees.
    for (auto &TreeAndSignPair : GroupTreeVec) {
      TreeAndSignPair.second = !TreeAndSignPair.second;
    }
  }
  return true;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
template <typename T>
LLVM_DUMP_METHOD void dumpHistTableElem(const T TableElem) {
  dbgs().indent(2) << *TableElem.first << "-> {";
  for (const auto &TreeAndOpcodes : TableElem.second) {
    dbgs() << TreeAndOpcodes.first->getId() << ":(";
    for (auto &Opcode : TreeAndOpcodes.second)
      dbgs() << getOpcodeSymbol(Opcode->getOpcode()) << " ";
    dbgs() << ") ";
  }
  dbgs() << "}\n";
}

template <typename T> LLVM_DUMP_METHOD void dumpHistTable(const T &Table) {
  dbgs() << "HistTable {\n";
  for (const auto &ValueAndTreesPair : Table) {
    dumpHistTableElem(ValueAndTreesPair);
  }
  dbgs() << "}\n";
}
#endif

// Scans all trees in \p TreeCluster and tries to find common leaves used across
// trees (referred as a Group later on). Group G can be applied to tree T if
// for any leaf G and T have the same operation or opposite one. For example,
// consider two trees T1=A1+B+C and T2=A2-B-C. For these trees we can build a
// group G=B+C. G and T1 have all the same operations while G & T2 have all
// opposite operations. As a result we can rewrite the code as T1=A1+G and
// T2=A2-G.
//
// Algorithm details:
//   We try to iteratively build a group which spawns across a maximal number of
// trees. Number of trees a group can be applied to called group's width. Number
// of leaves in the group is called group's size. It is easy to see that it is
// always more profitable to build a wider groups rather than bigger in size.
// Thus if addition of a leaf to a group will result in decrease of group's
// width then we prefer to build another group which is narrow than the current
// one.
void AddSubReassociate::buildMaxReuseGroups(const TreeArrayTy &TreeCluster,
                                            GroupsTy &BestGroups) {
  constexpr int MaxNumTrees = 16;

  if (TreeCluster.empty())
    return;

  LLVM_DEBUG(dbgs() << "==== Start building groups ===\n");

  // Since one leaf can appear in a tree (and as a result in a group) more than
  // once we need to be able to keep list of opcodes it appears with.
  using OpcodesTy = SmallVector<const OpcodeData *, 4>;
  using TreeAndOpcodesTy = std::pair<Tree *, OpcodesTy>;
  using TreeCollectionTy = SmallVector<TreeAndOpcodesTy, MaxNumTrees>;
  // For each leaf we keep list of tree and opcodes pair. Thus for each leaf
  // we know in which trees it appears and what opcodes are.
  using HistogramTableT = MapVector<Value *, TreeCollectionTy>;

  // Scans through \p Opcodes and tries to either find exact or compatible
  // opcode to \p OpcodeToMatch depending on \p FindExactMatch flag.
  // Please note that trees in a cluster are guaranteed to have compatible
  // opcodes only. So there is no need for additional check.
  auto FindCompatibleOpcode = [&](const OpcodeData &OpcodeToMatch,
                                  OpcodesTy &Opcodes, bool FindExactMatch) {
    auto isAssocEqual = [](const OpcodeData &OD1, const OpcodeData &OD2) {
      auto It1 = OD1.begin(), It2 = OD2.begin();
      for (; It1 != OD1.end() && It2 != OD2.end(); ++It1, ++It2)
        if (*It1 != *It2)
          return false;

      if (It1 == OD1.end() && It2 == OD2.end())
        return true;

      return false;
    };
    for (OpcodesTy::iterator It = Opcodes.begin(); It != Opcodes.end(); ++It) {
      const OpcodeData *OD = *It;
      if (isAssocEqual(OpcodeToMatch, *OD)) {
        // Check if we have exact match.
        if (!FindExactMatch || OpcodeToMatch.getOpcode() == OD->getOpcode()) {
          return It;
        }
      }
    }
    return Opcodes.end();
  };
  // Build initial map.
  HistogramTableT LeafHistTable;
  for (const TreePtr &Tptr : TreeCluster) {
    for (const LeafUserPair &LUPair : Tptr->getLeavesAndUsers()) {
      TreeCollectionTy &Trees = LeafHistTable[LUPair.getLeaf()];

      // Find current tree.
      auto TreeAndOpcodesIt = find_if(
          Trees, [&](const TreeAndOpcodesTy &elem) -> bool {
            return elem.first == Tptr.get();
          });

      // Add current tree to the list if doesn't exist.
      if (TreeAndOpcodesIt == Trees.end()) {
        Trees.push_back({Tptr.get(), OpcodesTy()});
        TreeAndOpcodesIt = Trees.end() - 1;
      }

      // Add opcode the leaf has in current tree.
      TreeAndOpcodesIt->second.push_back(&LUPair.getOpcodeData());
    }
  }

  // Early bail out. Later code depends on the table not being empty.
  if (LeafHistTable.empty())
    return;

  // Now we need to sort by number of trees each leaf appears in. For that we
  // "convert" map to a vector.
  auto LeafHistVec = LeafHistTable.takeVector();

  // Sorting predicate.
  auto SortPred = [&](const decltype(LeafHistVec)::value_type &lhs,
                      const decltype(LeafHistVec)::value_type &rhs) -> bool {
    return lhs.second.size() > rhs.second.size();
  };

  // Perform sorting. Use stable sort to generate same code from run to run.
  std::stable_sort(LeafHistVec.begin(), LeafHistVec.end(), SortPred);

  LLVM_DEBUG(dumpHistTable(LeafHistVec));

  // Start with maximum possible width.
  size_t GroupWidth = LeafHistVec.front().second.size();
  // ... and iterate while we have leaves shared across two or more trees.
  while (GroupWidth > 1) {
    // This is more convenient representation of a group used in the algorithm.
    // It not only gives us a group itself but also list of tries the group
    // is applicable to. The later is needed to delete information about found
    // group from the histogram table.
    SmallVector<std::pair<decltype(LeafHistVec)::iterator, const OpcodeData>,
                16>
        LocalGroup;
    // Set of trees where current leaf appears.
    SmallDenseMap<Tree *, bool, MaxNumTrees> FoundTrees;

    // Iterate through all leaves in sorted table.
    for (auto TableIt = LeafHistVec.begin(); TableIt != LeafHistVec.end();
         ++TableIt) {
      // TODO:
      auto &TableElem = *TableIt;
      TreeCollectionTy &CandidateTrees = TableElem.second;
      const size_t CandidateTreesNum = CandidateTrees.size();

      // Check if current leaf is common across at least N trees.
      if (CandidateTreesNum < GroupWidth)
        // No need to go other remaining elements since the table is sorted.
        break;

      // Try to find any 'GroupWidth' trees legal for group extension. Double
      // loop nest is needed to try out all possible combinations (that should
      // be fine since current cluster size is limited to 16).
      for (size_t I = 0; I < CandidateTreesNum - GroupWidth + 1; ++I) {
        size_t FoundTreeNum = 0;
        OpcodeData GroupOpcode;
        for (size_t J = I; J < CandidateTreesNum && FoundTreeNum < GroupWidth;
             ++J) {
          Tree *CandidateTree = CandidateTrees[J].first;
          OpcodesTy &CantidateOpcodes = CandidateTrees[J].second;

          // Check if this is going to be the first leaf in the group.
          if (LocalGroup.empty()) {
            // We use the same opcode for the group as in first tree.
            if (FoundTreeNum == 0)
              GroupOpcode = *CantidateOpcodes.front();

            // Check if current tree has compatible opcode.
            OpcodesTy::iterator It;
            if ((It =
                    FindCompatibleOpcode(GroupOpcode, CantidateOpcodes,
                                         false)) != CantidateOpcodes.end()) {
              FoundTrees.insert(
                  {CandidateTree, (*It)->getOpcode() == GroupOpcode.getOpcode()});
              ++FoundTreeNum;
            }
          } else {
            // Ok, there is at least one leaf in the group which spawns across
            // trees in 'FoundTrees'. Find out if current leaf appears in the
            // same trees with compatible opcodes.
            auto FoundTreeIt = FoundTrees.find(CandidateTree);
            if (FoundTreeIt != FoundTrees.end()) {
              // We use the same opcode for the group as in first tree.
              if (FoundTreeNum == 0) {
                GroupOpcode = FoundTreeIt->second
                                  ? *CantidateOpcodes.front()
                                  : (*CantidateOpcodes.front()).getFlipped();
                ++FoundTreeNum;
              } else {
                const OpcodeData OpcodeToMatch = FoundTreeIt->second
                                                     ? GroupOpcode
                                                     : GroupOpcode.getFlipped();
                if (FindCompatibleOpcode(OpcodeToMatch, CantidateOpcodes,
                                         true) != CantidateOpcodes.end()) {
                  ++FoundTreeNum;
                }
              }
            }
          }
        }

        // Check if were able to find 'GroupWidth' trees for the current leaf.
        if (FoundTreeNum == GroupWidth) {
          // Add current leaf (pointed by 'TableIt') and Opcode to the group.
          LocalGroup.push_back({TableIt, GroupOpcode});
          // Stop search trees for current leaf.
          break;
        }

        // If we were not able to find 'GroupWidth' for the very first leaf
        // candidate we need to clear 'FoundTrees' to start building it from
        // the beginning for next slice of tress.
        if (LocalGroup.empty()) {
          FoundTrees.clear();
        }
      }
    }

    // Check how big the group is. Discard anything less than two elements.
    if (LocalGroup.size() < 2) {
      // Proceed with the next group width.
      --GroupWidth;
      continue;
    }

    // "Transform" local group to an external format.
    Group G;
    for (const auto &Elem : LocalGroup) {
      G.appendLeaf(Elem.first->first, Elem.second);
    };

    // Populate list of trees current group is applicable to. Please note that
    // we have to preserve incoming order (order in 'TreeCluster') since code
    // generation assumes that later tree in 'TreeCluster' appears earlier in
    // program order.
    GroupTreesTy GroupTreeVec;
    for (const TreePtr &TreePtr : TreeCluster) {
      auto It = FoundTrees.find(TreePtr.get());
      if (It != FoundTrees.end()) {
        GroupTreeVec.push_back({It->first, It->second});
      }
    }

    // Remove current group from the 'LeafHistVec'.
    for (auto &GroupElem : LocalGroup) {
      TreeCollectionTy &TreeVec = GroupElem.first->second;
      const OpcodeData GroupOpcode = GroupElem.second;

      TreeVec.erase(
          remove_if(TreeVec,
                    [&](TreeCollectionTy::value_type &TreeAndOpcodes) {
                      Tree *Tree = TreeAndOpcodes.first;
                      OpcodesTy &Opcodes = TreeAndOpcodes.second;
                      if (FoundTrees.count(Tree) > 0) {
                        auto It =
                            FindCompatibleOpcode(GroupOpcode, Opcodes, false);
                        assert(It != Opcodes.end() &&
                               "Can't find requested opcode for the group");
                        Opcodes.erase(It);
                        return Opcodes.empty();
                      }
                      return false;
                    }),
          TreeVec.end());
    }

    // Sort 'LeafHistVec' after removal of the group.
    // Use stable sort to generate same code from run to run.
    std::stable_sort(LeafHistVec.begin(), LeafHistVec.end(), SortPred);

    LLVM_DEBUG(dbgs() << "Found Group:\n");
    LLVM_DEBUG(G.dump());

    LLVM_DEBUG(dumpHistTable(LeafHistVec));

    // Canonicalize found group based on memory accesses if enabled.
    if (EnableGroupCanonicalization) {
      if (!EnableMemCanonicalization ||
          !memCanonicalizeGroup(G, GroupTreeVec, BestGroups))
        G.sort();
    }

    // Finally push constructed group to a list.
    BestGroups.push_back({std::move(G), std::move(GroupTreeVec)});

    // Make sure group width doesn't exceed current maximal number of candidate
    // trees.
    GroupWidth = std::min(GroupWidth, LeafHistVec.front().second.size());
  }
  LLVM_DEBUG(dbgs() << "==== End building groups ===\n");
}

bool AddSubReassociate::canonicalizeIRForTrees(
    const ArrayRef<Tree *> Trees) const {
  bool Changed = false;
  for (Tree *T : Trees) {
    Changed |= canonicalizeIRForTree(*T);
#ifndef NDEBUG
    if (EnableAddSubVerifier) {
      LUSetTy VisitedLUs;
      for (Instruction *TrunkI = T->getRoot(); TrunkI;
           TrunkI = dyn_cast<Instruction>(TrunkI->getOperand(0))) {
        Value *Leaf = TrunkI->getOperand(1);
        unsigned TrunkOpcode = TrunkI->getOpcode();
        assert(TrunkOpcode ==
                   T->getLeafCanonOpcode(Leaf, VisitedLUs, TrunkOpcode)
                       .getOpcode() &&
               "Bad canonicalizeTree() OR CanonOpcode");
      }
    }
#endif
  }
  return Changed;
}

// Given a Group, we branch out of the main trunk, build a chain of operations
// and attach it back to the main trunk.
// The Group enforces:
//  1) an ordering for the leaves in the chain, and
//  2) the opcodes connected to the leaves (if possible).
//
// Example 1: (+ A, + B)
// ---------------------
//                    chain
//                    -----
//     C              0 A
//   |/               |/
//   + B   --->     C + B
//   |/           |/  |/
//   + A          +   +
//   |/           | /
//   +            +
//
// Example 2: (- A, - B)
// ---------------------
//  The group has an opcode of '-', so the code needs updating.
//
//                    chain
//                    -----
//     C              0 A
//   |/               |/
//   + B   --->     C - B
//   |/           |/  |/
//   + A          +   -
//   |/           | /
//   +            -
//     C              0 A
//   |/               |/
//   - B   --->     C - B
//   |/           |/  |/
//   - A          -   -
//   |/           | /
//   -            +
//
void AddSubReassociate::generateCode(Group &G, GroupTreeSignTy &TreeAndSign,
                                     Instruction *GroupChain) const {
  Instruction *TopChainI = nullptr;
  Instruction *BottomChainI = nullptr;
  Instruction *MainOp0 = nullptr;
  // The trunk instructions that will become part of the chain.
  SmallVector<Instruction *, 16> BottomUpChainIVec;
  Tree *T = TreeAndSign.first;
  bool MustFlipTreeOpcodes = !TreeAndSign.second;

  // Collect the trunk instructions that need to be updated in bottom up.
  // While doing so, also find the top TrunkI, the BottomTrunkI and MainOp0.
  for (Instruction *TrunkI = T->getRoot(); TrunkI;
       TrunkI = dyn_cast<Instruction>(TrunkI->getOperand(0))) {
    Value *Leaf = TrunkI->getOperand(1);
    // TODO: There might be multiple identical Leaf values in G. What then?
    if (!G.containsValue(Leaf)) {
      // MainOp0 is the first operand of the trunk that is not in G.
      if (!MainOp0)
        MainOp0 = TrunkI;
      continue;
    }
    BottomUpChainIVec.push_back(TrunkI);

    // Remember the current TrunkI.
    TopChainI = TrunkI;
    if (!BottomChainI)
      BottomChainI = TrunkI;
  }

  assert(BottomUpChainIVec.size() == G.size() &&
         "Check if Codegen Works correctly. We have a leaf with multiple "
         "users.");

  // 4. Create a new ADD/SUB bridge to connect the chain to the trunk.
  //
  //                    0      LeafN
  //                    |     /
  //                  TopChainI
  //                    |
  //                   ...    Leaf1
  //                    |     /
  //           MainOp0 BottomChainI
  //               |  /
  // MainOp0      +/-   <-- Bridge
  //  /|\   -->   /|\
  // Users       Users
  //
  Value *Undef = UndefValue::get(BottomChainI->getType());
  // Figure out the 'Bridge' opcode.

  Instruction::BinaryOps BridgeOpcode =
      (!MustFlipTreeOpcodes) ? BinaryOperator::Add : BinaryOperator::Sub;
  Instruction *Bridge = BinaryOperator::Create(BridgeOpcode, Undef, Undef);
  // Bridge is now the root of the tree. We need to keep the root up to date.
#ifndef NDEBUG
  Bridge->setName(Twine("Bridge_T") + Twine(T->getId()) + Twine("_"));
#endif

  Bridge->insertAfter(T->getRoot());
  T->getRoot()->replaceAllUsesWith(Bridge);
  T->setRoot(Bridge);

  // Iterate through the chain instrs bottom-up and build a detached chain or
  // remove it if it has already been build.
  Instruction *LastChainI = nullptr;
  for (Instruction *ChainI : BottomUpChainIVec) {
#ifndef NDEBUG
    ChainI->setName(Twine("Chain_T") + Twine(T->getId()) + Twine("_"));
#endif

    // 1. Bypass TrunkI
    //    Go through each operand in G and remove its operand from the trunk.
    //
    //                   Main Trunk   Chain (detached)
    //                   ----------   -----
    //        Op0 Leaf      Op0           Leaf
    //         | /           |           /
    //      TrunkI      -->  |       ChainI   <-- TopChainI
    //        /|\           /|\
      //       Users         Users
    Value *Op0 = ChainI->getOperand(0);
    ChainI->replaceAllUsesWith(Op0);

    // 2a. Remove chain instructions if we have already build it.
    if (GroupChain) {
      ChainI->dropAllReferences();
      ChainI->eraseFromParent();
    } else {
      // 2b. Build a chain out of the detached 'ChainI's.
      //            Leaf2
      //     ...   /
      //      |   / Leaf1
      //   ChainI2 /   <-- TopChainI
      //      |   /
      //   ChainI1     <-- BottomChainI
      //      |
      if (LastChainI)
        LastChainI->setOperand(0, ChainI);
      LastChainI = ChainI;
    }
  }

  if (!GroupChain) {
    Value *Zero = ConstantInt::get(TopChainI->getType(), 0);
    // We are at the top of the chain, so set operand 0 to zero.
    TopChainI->setOperand(0, Zero);

    // 3. Reschedule the chain to reflect the order in the group.
    //
    // 3.a. As a preliminary step collect all the group instructions that are
    // also in the tree in the same order as they appear in the group.
    SmallVector<Instruction *, 8> ChainInstrsInGroupOrder;
    LUSetTy VisitedLUs;
    for (auto &Pair : G.getValues()) {
      Value *Leaf = Pair.first;
      if (!T->hasLeaf(Leaf))
        continue;
      // The tree can contain multiple identical leaves == Leaf, so avoid
      // them.
      auto &LUPair = T->getNextLeafUserPair(Leaf, VisitedLUs);
      assert(std::find(BottomUpChainIVec.begin(), BottomUpChainIVec.end(),
                       LUPair.getUser()) != BottomUpChainIVec.end() &&
             "Not found?");
      ChainInstrsInGroupOrder.push_back(LUPair.getUser());
      LUPair.User = nullptr;
    }
    // 3.a. We first move the chain instructions back-to-back. Without this
    // step we cannot legally reorder the chain instrs freely to reflect groups.
    //        Leaf3
    //        Leaf2
    //        Leaf1
    //         ...
    //        ChainI_3
    //        ChainI_2
    //        ChainI_1
    for (Instruction *ChainI : llvm::reverse(make_range(
             std::next(BottomUpChainIVec.begin()), BottomUpChainIVec.end())))
      ChainI->moveBefore(BottomChainI);

    // 3.b. Now we can safely reorder them to reflect the order in 'G',
    // without having to worry about the position of the Leaf instructions.
    //        ChainI_3
    //        ChainI_1
    //        ChainI_2
    //
    // We iterate through the group leaves bottom-up and move the chain
    // instructions before the bottom of the chain.
    // Meanwhile we fix the operand(0) of the instructions to form the chain.
    Instruction *LastInChain = BottomChainI;
    for (Instruction *ChainI : ChainInstrsInGroupOrder) {
      ChainI->moveBefore(LastInChain);
      // Fix the operand(0) of the chain.
      LastInChain->setOperand(0, ChainI);
      LastInChain = ChainI;
    }
    LastInChain->setOperand(0, Zero);

    // WARNING: Variables pointing to instrs are no longer consistent with the
    // code. Update the ones we need here.
    TopChainI = ChainInstrsInGroupOrder.back();
    BottomChainI = ChainInstrsInGroupOrder.front();

    // 5. If we used a Sub for the bridge node, we need to flip the opcodes.
    if (MustFlipTreeOpcodes) {
      for (Instruction *ChainI = BottomChainI, *FlippedChainI = nullptr; ChainI;
           ChainI = dyn_cast<Instruction>(FlippedChainI->getOperand(0))) {
        FlippedChainI = flipInstruction(ChainI);
        // WARNING: We may have invalidated all variables that point to the
        // chain (e.g. BottomChainI). Since we need TopChainI for the next
        // step,
        // keep it consistent.
        if (ChainI == TopChainI)
          TopChainI = FlippedChainI;
        if (ChainI == BottomChainI)
          BottomChainI = FlippedChainI;
        // WARNING: the tree does not reflect the changes caused by codegen,
        // but we are not going to use it again so don't bother fixing it.
      }
    }
  } else {
    TopChainI = nullptr;
    BottomChainI = GroupChain;
  }

  if (MainOp0)
    Bridge->setOperand(0, MainOp0);
  else
    Bridge->setOperand(0, ConstantInt::get(BottomChainI->getType(), 0));

  Bridge->setOperand(1, BottomChainI);

#ifndef NDEBUG
  if (EnableAddSubVerifier)
    assert(!verifyFunction(*Bridge->getParent()->getParent(), &dbgs()));
#endif
}

// Fix the '0' at the top of the trunk/chain. Returns possibly updated bridge
// instruction.
Instruction *AddSubReassociate::simplifyTree(Instruction *Bridge,
                                             bool OptTrunk) const {
  Instruction *TopI = Bridge;
  Instruction *AddI = nullptr;

  // Step1. Scan through the tree and find top most instruction and last ADD
  // instruction if any.
  for (Value *CurVal = Bridge->getOperand(OptTrunk ? 0 : 1);
       !isa<Constant>(CurVal); CurVal = TopI->getOperand(0)) {
    TopI = cast<Instruction>(CurVal);
    assert(TopI->hasOneUse() && "Canonical tree should have one use only");
    if (TopI->getOpcode() == Instruction::Add)
      AddI = TopI;
  }

  // Step2. Switch 'top' and 'add' instructions if both exists.
  if (AddI != nullptr && AddI != TopI) {
    Instruction *TopUserI = TopI->user_back();
    if (TopUserI == AddI) {
      TopUserI = TopI;
    }
    TopI->setOperand(0, AddI->getOperand(0));
    AddI->replaceAllUsesWith(TopI);
    TopUserI->setOperand(0, AddI);
    TopI->moveAfter(AddI);
    TopI = AddI;
  }

  // If we still don't have an add instruction at the top that means all
  // instructions are SUBs and we need to flip sign of a bridge.
  bool NeedToFlip = (TopI->getOpcode() != Instruction::Add);

  // Can't do anything useful if we optimize trunk and all instructions are
  // SUBs.
  if (NeedToFlip && OptTrunk && Bridge->getOpcode() != Instruction::Add)
    return Bridge;

  // Step3. Handle convoluted case of 0 instructions in the tree separately.
  if (TopI == Bridge) {
    if (!OptTrunk || Bridge->getOpcode() == Instruction::Add) {
      Bridge->replaceAllUsesWith(Bridge->getOperand(OptTrunk ? 1 : 0));
      Bridge->dropAllReferences();
      Bridge->eraseFromParent();
    }
    return Bridge;
  }

  // Step4. Remove 'top' instruction.
  assert(TopI->hasOneUse() && "Top instruction is expected to have one use");
  Instruction *NewTopI = TopI->user_back();
  TopI->replaceAllUsesWith(TopI->getOperand(1));
  TopI->dropAllReferences();
  TopI->removeFromParent();
  TopI = NewTopI;

  // Step5. Flip the tree if necessary.
  if (NeedToFlip) {
    // All instruction in the trunk/chain are SUBs. Replace all of them
    // with ADDs.
    for (Instruction *CurI = TopI; CurI != Bridge; CurI = CurI->user_back()) {
      CurI = flipInstruction(CurI);
    }
    Bridge = flipInstruction(Bridge, OptTrunk);
  }

#ifndef NDEBUG
  if (EnableAddSubVerifier)
    assert(!verifyFunction(*Bridge->getParent()->getParent(), &dbgs()));
#endif

  return Bridge;
}

void Tree::emitAssocInstrutions() {
  // If we have associative instructions, we need to emit them here.

  // STEP1 : Emit only the assoc instructions that are in groups.
  assert(EnableUnaryAssociations);

  // Check if we have not emitted associative instructions.
  if (!hasAssocInstr)
    return;

  for (const auto &LUPair : getLeavesAndUsers()) {
    Value *Leaf = LUPair.Leaf;
    Instruction *UserRunner = LUPair.User;
    const OpcodeData &Opcode = LUPair.Opcode;
    // Emit them in the given order, just before the user.
    for (const AssocOpcodeData &Data : Opcode) {
      Instruction::BinaryOps BinOpcode =
          static_cast<Instruction::BinaryOps>(Data.getOpcode());
      Instruction *NewAssocI =
          BinaryOperator::Create(BinOpcode, Leaf, Data.getConst());
      NewAssocI->insertBefore(UserRunner);
      UserRunner->replaceUsesOfWith(Leaf, NewAssocI);
      UserRunner = NewAssocI;
    }
  }

  // Once all associative instructions are emitted reset the related flag.
  hasAssocInstr = false;

#ifndef NDEBUG
  if (EnableAddSubVerifier)
    assert(!verifyFunction(
        *cast<Instruction>(getRoot())->getParent()->getParent(), &dbgs()));
#endif

  // TODO: This was never implemented properly. Just leaving comment in case
  // we decide to do that.
  // STEP2 : The rest of them need to be emitted separately from the leaves in
  //         order to minimize replication of the assoc instruction.
  //         We generate a minimum number of chains out of them and we apply
  //         them to multiple leaves.
  // For example if we have two identical (<<2) assoc instrs:
  //
  //                       0 L2
  //                       |/
  //                       + L1
  //                       |/
  //    L2 (+)(<<2)        +  2
  //  |/                   | /
  //  + L1 (+)(<<2)    ... <<
  //  |/                | /
  //  +                 +
  //
}

// Apply all groups in 'Groups' onto each tree in 'TreeCluster'.
void AddSubReassociate::generateCode(
    GroupsTy &Groups, const ArrayRef<Tree *> AffectedTrees) const {

  // Generate the code for each group and tree.
  for (auto &GroupAndTreesPair : Groups) {
    auto &G = GroupAndTreesPair.first;
    auto &TreeCluster = GroupAndTreesPair.second;
    // Apply each group onto the tree.
    Instruction *GroupChain = nullptr;
    for (auto It = TreeCluster.rbegin(); It != TreeCluster.rend(); ++It) {
      Tree *T = It->first;
      if (EnableUnaryAssociations)
        T->emitAssocInstrutions();
      generateCode(G, *It, GroupChain);

      if (!GroupChain) {
        // Keep track if sign of a group has been reversed as a result of
        // of simplification. If this is the case and group is reused across
        // trees we will need to reverse sign of the group for remaining trees.
        bool IsGroupFlipped = false;

        // Simplify the instruction chains to get rid of redundancies like
        // '0 + Val' from the top of the chain.
        if (SimplifyChains) {
          Instruction *newBridge = simplifyTree(T->getRoot(), false);
          if (T->getRoot() != newBridge) {
            assert(T->getRoot()->getOpcode() != newBridge->getOpcode() &&
                   "Expected to have different opcodes");
            IsGroupFlipped = true;
            T->setRoot(newBridge);
          }
        }

        if (ReuseChain) {
          GroupChain = cast<Instruction>(T->getRoot()->getOperand(1));
          if (IsGroupFlipped)
            for (auto ItTmp = It + 1; ItTmp != TreeCluster.rend(); ++ItTmp)
              ItTmp->second = !ItTmp->second;
        }
      }
    }
  }

  // Optimization: Remove the top zero constants.
  if (SimplifyTrunks) {
    for (Tree *T : AffectedTrees)
      simplifyTree(T->getRoot(), true);
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void
AddSubReassociate::dumpGroups(const GroupsTy &Groups) const {
  int Cnt = 0;
  for (const auto &GroupAndTreesPair : Groups) {
    dbgs() << "Group " << Cnt++ << "\n";
    GroupAndTreesPair.first.dump();
  }
}

LLVM_DUMP_METHOD void
AddSubReassociate::dumpTreeVec(const TreeVecTy &TreeVec) const {
  int Cnt = 0;
  for (const TreePtr &T : TreeVec) {
    dbgs().indent(2) << "--- Tree " << Cnt++ << "/" << TreeVec.size()
                     << " ---\n";
    T->dump();
  }
}

LLVM_DUMP_METHOD void
AddSubReassociate::dumpTreeArray(const TreeArrayTy &TreeVec) const {
  int Cnt = 0;
  for (const TreePtr &T : TreeVec) {
    dbgs().indent(2) << "--- Tree " << Cnt++ << "/" << TreeVec.size()
                     << " ---\n";
    T->dump();
  }
}

LLVM_DUMP_METHOD void AddSubReassociate::dumpTreeArrayVec(
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
bool AddSubReassociate::treesMatch(const Tree *T1, const Tree *T2) const {
  SmallPtrSet<Value *, 8> T1Values;
  for (auto &LUPair : T1->getLeavesAndUsers())
    T1Values.insert(LUPair.getLeaf());

  // Count how many of T2's leaves match.
  int Matches = 0;
  for (auto &LUPair : T2->getLeavesAndUsers())
    if (T1Values.count(LUPair.getLeaf()))
      Matches++;

  return (Matches * 100) / T1Values.size() > TreeMatchThreshold;
}

// Form clusters of Trees with similar i) size, ii) values.
// This is currently quadratic to AllTrees size.
// TODO: Tune the clustering heuristics.
// TODO: Reduce complexity by sorting based on size?
void AddSubReassociate::clusterTrees(
    TreeVecTy &AllTrees, SmallVectorImpl<TreeArrayTy> &TreeClusters) {
  TreeArrayTy TreeVecArray(AllTrees);
  unsigned ClusterSize = 1;
  for (size_t i = 0, e = AllTrees.size(); i != e; i += ClusterSize) {
    const Tree *T1 = AllTrees[i].get();
    // Now look for other trees within AllTrees.
    ClusterSize = 1;
    // Break quadratic complexity by reducing the maximum search.
    int EndOfSearch = std::min(e, i + 1 + MaxClusterSearch);
    for (int j = i + 1; j != EndOfSearch; ++j) {
      Tree *T2 = AllTrees[j].get();
      // If i)  size difference is within limits.
      //    ii) the trees share a min number of values.
      if ((std::abs((int64_t)T2->getLeavesCount() -
                    (int64_t)T1->getLeavesCount()) <
           MaxTreeSizeDiffForCluster) &&
          treesMatch(T2, T1)) {
        // Move this element next to the last member of the cluster.
        auto First = AllTrees.begin() + i + ClusterSize;
        auto Middle = AllTrees.begin() + j;
        auto Last = Middle + 1;
        std::rotate(First, Middle, Last);
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

// Remove all instructions from OldRootI all the way to the leaves.
void AddSubReassociate::removeDeadTrunkInstrs(Tree *T,
                                              Instruction *OldRootI) const {
  SmallVector<Instruction *, 16> POT;
  // Walk up the instructions until the leaves are reached.
  // Push the instructions into the POT vector.
  std::function<void(Value *)> GetPOT = [&](Value *V) {
    Instruction *I = dyn_cast<Instruction>(V);
    if (!I || T->hasLeaf(V))
      return;
    for (int i = 0, e = I->getNumOperands(); i != e; ++i)
      GetPOT(I->getOperand(i));
    // Post-order
    POT.push_back(I);
  };
  GetPOT(OldRootI);
  // Erase in Reverse Post-Order Traversal.
  for (Instruction *I : llvm::reverse(POT)) {
    // Instructions may be shared across trees. Remove only if no uses left.
    if (I->use_empty())
      I->eraseFromParent();
  }
}

// Canonicalize AddSub expression tree: All Add/Sub operations should
// be in a single branch and all the leaf nodes should be on separate
// branches. For example, X = A - B + C should be in this form:
//  0 C
//  |/
//  + B
//  |/
//  - A
//  |/
//  +
//  |
//  X
// Returns true if the code was modified
bool AddSubReassociate::canonicalizeIRForTree(Tree &T) const {
  // Now that we know all the +/- opcodes associated to each leaf, we can
  // build the canonicalized tree.
  Value *Undef0 = UndefValue::get(T.getRoot()->getType());
  Instruction *LastTrunkI = nullptr;
  Instruction *RootTrunkI = nullptr;
  Instruction *InsertionPt = T.getRoot();

  // NOTE: We iterate the leaves bottom-up because we emit their corresponding
  // trunk instructions bottom-up.
  for (auto &LUPair : T.getLeavesAndUsers()) {
    Value *Leaf = LUPair.getLeaf();
    const OpcodeData &Opcode = LUPair.getOpcodeData();
    Instruction *OldUser = LUPair.getUser();
    Instruction *TrunkI = nullptr;
    switch (Opcode.getOpcode()) {
    case Instruction::Add:
      TrunkI = BinaryOperator::CreateAdd(Undef0, Leaf);
      break;
    case Instruction::Sub:
      TrunkI = BinaryOperator::CreateSub(Undef0, Leaf);
      break;
    default:
      llvm_unreachable("Only Add/Sub instructions are allowed in the tree");
    }

#ifndef NDEBUG
    TrunkI->setName(Twine("Trunk_T") + Twine(T.getId()) + Twine("_"));
#endif

    TrunkI->insertBefore(InsertionPt);

    // Update leaf user to TrunkI.
    T.replaceLeafUser(Leaf, OldUser, TrunkI);

    // Connect it with the last trunk instr
    if (LastTrunkI)
      LastTrunkI->setOperand(0, TrunkI);
    if (!RootTrunkI)
      RootTrunkI = TrunkI;
    LastTrunkI = TrunkI;
    InsertionPt = TrunkI;
  }
  // The topmost operand is a zero.
  LastTrunkI->setOperand(0, ConstantInt::get(LastTrunkI->getType(), 0));
  T.getRoot()->replaceAllUsesWith(RootTrunkI);
  // Update the root node of the tree
  Instruction *OldRoot = T.getRoot();
  T.setRoot(RootTrunkI);

  // Remove old internal trunk instructions that will no longer be used after
  // canonicalization. NOTE: Associative instructions are also removed.
  // Remove them in reverse post-order.
  removeDeadTrunkInstrs(&T, OldRoot);

#ifndef NDEBUG
  checkCanonicalized(T);
  if (EnableAddSubVerifier)
    assert(!verifyFunction(*T.getRoot()->getParent()->getParent(), &dbgs()));
#endif

  return true;
}

// Try to grow the tree upwards, towards the definitions.
// Returns true if new nodes have been added to the tree.
bool AddSubReassociate::growTree(TreeVecTy &AllTrees, Tree *T,
                                 WorkListTy &&WorkList) {
  unsigned SizeBefore = T->getLeavesCount();
  unsigned CntAssociations = 0;

  // Keep trying to grow tree until the WorkList is empty.
  while (!WorkList.empty()) {
    auto LastOp = WorkList.pop_back_val();

    assert(isAllowedTrunkInstr(LastOp.getLeaf(), DL) &&
           "Work list item can't be trunk instruction.");
    Instruction *I = cast<Instruction>(LastOp.getLeaf());

    // If current instruction starts another tree then just clear that tree
    // since it will become part of the growing tree.
    if (Tree *ATree = findTreeWithRoot(AllTrees, I, T)) {
      ATree->clear();
    }

    bool IsAllowedAssocInstrI = isAllowedAssocInstr(I);
    if (IsAllowedAssocInstrI) {
      // Collect the unary associative instructions that apply for 'I'.
      T->addAssocInstruction(LastOp, I);
    }

    for (int OpIdx : {0, 1}) {
      Value *Op = I->getOperand(OpIdx);

      // Skip self referencing instructions.
      if (Op == I)
        continue;

      // Skip the constant operand of a UnaryAssociative instruction. It
      // should not be part of the tree.
      if (IsAllowedAssocInstrI && isa<Constant>(Op)) {
        continue;
      }

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
        OpCanonOpcode = LastOp.getOpcodeData().getFlipped();
      } else {
        // Reuse the opcode of the user.
        OpCanonOpcode = LastOp.getOpcodeData();
      }

      if ( // Keep the size of a tree below a maximum value.
          T->getLeavesCount() + 2 * WorkList.size() < MaxTreeSize &&
          Op->hasOneUse() && arePredsInSameBB(Op, I) &&
          (isAllowedTrunkInstr(Op, DL) &&
           (!isAllowedAssocInstr(Op) ||
            // Check number of allowed assoc instruction.
            (++CntAssociations <= MaxUnaryAssociations)))) {
        // Push the operand to the WorkList to continue the walk up the code.
        WorkList.push_back(LeafUserPair(Op, I, OpCanonOpcode));
      } else {
        // 'Op' is a leaf node, so stop growing and add it into T's leaves.
        T->appendLeaf(I, Op, OpCanonOpcode);
        // If 'Op' is an add/sub and it is shared (maybe across trees maybe
        // not),
        // then this tree is a candidate for growing towards the shared
        // leaves. This is performed by 'extendTrees()'.
        if (isAllowedTrunkInstr(Op, DL) && Op->getNumUses() > 1)
          T->setSharedLeafCandidate(true);
      }
    }
  }
  bool Changed = SizeBefore != T->getLeavesCount();
  return Changed;
}

bool AddSubReassociate::areAllUsesInsideTreeCluster(
    TreeArrayTy &TreeCluster, const Value *Leaf,
    SmallVectorImpl<std::pair<Tree *, unsigned>> &WorkList) const {
  for (auto *U : Leaf->users()) {
    bool Found = false;
    for (TreePtr &Tptr : TreeCluster) {
      auto *ATree = Tptr.get();
      auto LUVec = ATree->getLeavesAndUsers();
      for (unsigned I = 0; I < ATree->getLeavesCount(); ++I) {
        auto &LUPair = LUVec[I];
        if (LUPair.getLeaf() == Leaf && LUPair.getUser() == U) {
          WorkList.push_back(std::make_pair(ATree, I));
          Found = true;
          break;
        }
      }
      if (Found)
        break;
    }
    // Check if the use is inside any tree.
    if (!Found)
      return false;
  }
  return true;
}

bool AddSubReassociate::getSharedLeave(
    TreeArrayTy &TreeCluster,
    SmallVectorImpl<std::pair<Tree *, unsigned>> &WorkList) {
  WorkList.clear();

  for (auto &Tptr : TreeCluster) {
    Tree *ATree = Tptr.get();

    if (!ATree->hasSharedLeafCandidate())
      continue;

    for (auto &LUPair : ATree->getLeavesAndUsers()) {
      Value *LeafV = LUPair.getLeaf();
      Instruction *SharedLeaf = dyn_cast<Instruction>(LeafV);

      // Need to clear WorkList since it may be polluted with data from
      // previous iteration.
      WorkList.clear();

      // Only Add/Sub leaves can become parts of trees once replicated.
      if (SharedLeaf && isAllowedTrunkInstr(SharedLeaf, DL) &&
          SharedLeaf->getNumUses() > 1 &&
          arePredsInSameBB(SharedLeaf, ATree->getRoot()) &&
          areAllUsesInsideTreeCluster(TreeCluster, SharedLeaf, WorkList))
        return true;
    }
    ATree->setSharedLeafCandidate(false);
  }
  return false;
}

void AddSubReassociate::extendTrees(TreeVecTy &AllTrees,
                                    TreeArrayTy &TreeCluster) {
  int MaxAttempts = MaxSharedNodesIterations;

  SmallVector<std::pair<Tree *, unsigned>, 8> SharedUsers;
  while (--MaxAttempts >= 0 && getSharedLeave(TreeCluster, SharedUsers)) {
    for (auto &SharedUser : SharedUsers) {
      Tree *ATree = SharedUser.first;
      LeafUserPair LUPair = ATree->getLeafUserPair(SharedUser.second);
      ATree->removeLeaf(SharedUser.second);
      growTree(AllTrees, ATree, SmallVector<LeafUserPair, 8>({LUPair}));
      ATree->adjustSharedLeavesCount(1);
    }
    // In fact we don't need to clone a leaf for the first tree and may use
    // original instance. Adjust a counter by one.
    SharedUsers[0].first->adjustSharedLeavesCount(-1);
  }
}

void AddSubReassociate::buildInitialTrees(BasicBlock *BB, TreeVecTy &AllTrees) {
  // Returns true if 'I' is a candidate for a tree root.
  auto isRootCandidate = [&](const Instruction *I) -> bool {
    // Conditions: i.  Add/Sub opcode,
    //             ii. Doesn't belong to any tree.
    return isAddSubInstr(I, DL) && !findEnclosingTree(AllTrees, I);
  };

  // Scan the BB in reverse and build a tree.
  for (Instruction &I : make_range(BB->rbegin(), BB->rend())) {
    // Check that number of built trees doesn't exceed specified limits.
    // '0' means "unlimited" unless it is set by the user.
    if ((MaxTreeCount.getNumOccurrences() > 0 || MaxTreeCount > 0) &&
        AllTrees.size() >= (unsigned)MaxTreeCount)
      break;

    // A tree is rooted at an Add/Sub instruction that is not already in a
    // tree.
    if (!isRootCandidate(&I))
      continue;

    TreePtr UTree = llvm::make_unique<Tree>(DL);
    Tree *T = UTree.get();
    T->setRoot(&I);
    growTree(AllTrees, T,
             SmallVector<LeafUserPair, 8>(
                 {LeafUserPair(&I, nullptr, Instruction::Add)}));

    assert(1 <= T->getLeavesCount() && T->getLeavesCount() <= MaxTreeSize + 2 &&
           "Tree size should be capped");

    // Skip trivial trees.
    if (T->getLeavesCount() > 1)
      AllTrees.push_back(std::move(UTree));
  }
}

// Build Add/Sub trees with instructions from BB.
void AddSubReassociate::buildTrees(BasicBlock *BB, TreeVecTy &AllTrees,
                                   SmallVector<TreeArrayTy, 8> &Clusters,
                                   bool UnshareLeaves) {
  // 1. Scan the code in BB and build initial trees.
  buildInitialTrees(BB, AllTrees);
  if (AllTrees.empty())
    return;

  // 2. Create clusters of trees out of the trees in AllTrees. Each cluster
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
  // NOTE: This reorders the elements in AllTrees.
  clusterTrees(AllTrees, Clusters);

  // 3. Try to extend the trees by including leaves with multiple uses across
  // multiple trees.
  if (EnableSharedLeaves && UnshareLeaves) {
    for (TreeArrayTy &Cluster : Clusters)
      extendTrees(AllTrees, Cluster);
  }
}

// Entry point for AddSub reassociation.
// Returns true if we need to invalidate analysis.
bool AddSubReassociate::run() {
  bool Changed = false;
  ReversePostOrderTraversal<Function *> RPOT(F);

  if (!AddSubReassocEnable)
    return false;

  // Scan the code for opportunities of AddSub reassociation.
  // Make a "pairmap" of how often each operand pair occurs.
  for (BasicBlock *BB : RPOT) {
    bool IsBBChanged = true;
    bool IsFirstPass = true;
    unsigned Iter = 0;
    while (IsBBChanged && Iter++ < MaxBBIters) {

      TreeVecTy AllTrees;
      SmallVector<TreeArrayTy, 8> TreeClusters;

      IsBBChanged = false;

      // 1. Build as many trees as we can find in BB. We need to perform leaves
      // unsharing on the first iteration only since all consequent iterations
      // will observe already unshared leaves from the first iteration.
      buildTrees(BB, AllTrees, TreeClusters, IsFirstPass);
      // We did not collect any clusters. Continue looking for more trees.
      if (TreeClusters.empty())
        continue;

      for (TreeArrayTy &TreeCluster : TreeClusters) {
        // 2. Form groups of nodes that reduce divergence
        GroupsTy BestGroups;
        buildMaxReuseGroups(TreeCluster, BestGroups);
        if (BestGroups.empty())
          continue;

        // The score is the instruction count savings (# before  - # after).
        // If we apply group G, we are:
        //  i. introducing G.size()-1 new instructions (for the tmp value), and
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
        // But first we need to find out all affected trees.
        SmallSetVector<Tree *, 16> AllAffectedTrees;
        for (auto &GroupAndTrees : BestGroups)
          for (auto &TreeAndSign : GroupAndTrees.second)
            AllAffectedTrees.insert(TreeAndSign.first);

        int Score = 0;
        for (auto *T : AllAffectedTrees)
          Score -= T->getSharedLeavesCount();

        // Compute instruction difference for each group.
        for (auto &GroupAndTreesPair : BestGroups) {
          Group &G = GroupAndTreesPair.first;
          auto AssocInstCnt = G.geAssocInstrCnt();
          const size_t TreeNum = GroupAndTreesPair.second.size();
          int OrigCost = (G.size() + AssocInstCnt.first) * TreeNum;
          int NewCost = (G.size() - 1 + AssocInstCnt.second + TreeNum);
          Score += OrigCost - NewCost;
        }

        // Original tree (before canonicalization) could have one instruction
        // less. Lets be conservative and assume this is always the case.
        Score -= AllAffectedTrees.size();

        // Check if transformation is profitable.
        if (Score < 0) {
          LLVM_DEBUG(dbgs() << "Discarding groups. Score=" << Score << "\n");
          continue;
        }

        LLVM_DEBUG(dbgs() << "Applying groups. Score=" << Score << "\n");

        // 3. Canonicalize the IR into a single linearized chain.
        canonicalizeIRForTrees(AllAffectedTrees.getArrayRef());

        // 4. Now that we've got the best groups we can generate code.
        generateCode(BestGroups, AllAffectedTrees.getArrayRef());

        IsBBChanged = true;
        Changed = true;
      }
      IsFirstPass = false;
    }
  }
  return Changed;
}

bool AddSubReassociatePass::runImpl(Function *F, ScalarEvolution *SE) {
  return AddSubReassociate(F->getParent()->getDataLayout(), SE, F).run();
}

PreservedAnalyses AddSubReassociatePass::run(Function &F,
                                             FunctionAnalysisManager &AM) {
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

char AddSubReassociateLegacyPass::ID = 0;

bool AddSubReassociateLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  auto *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  return Impl.runImpl(&F, SE);
}

void AddSubReassociateLegacyPass::getAnalysisUsage(AnalysisUsage &AU) const {
  FunctionPass::getAnalysisUsage(AU);
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.setPreservesCFG();
  AU.addPreserved<GlobalsAAWrapperPass>();
}

INITIALIZE_PASS_BEGIN(AddSubReassociateLegacyPass, "addsub-reassoc",
                      "AddSub Reassociation", false, false)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_END(AddSubReassociateLegacyPass, "addsub-reassoc",
                    "AddSub Reassociation", false, false)

// Public interface to the Reassociate pass
FunctionPass *llvm::createAddSubReassociatePass() {
  return new AddSubReassociateLegacyPass();
}

