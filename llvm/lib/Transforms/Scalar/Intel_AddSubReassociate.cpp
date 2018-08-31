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

static cl::opt<bool>
    SimplifyTrunks("addsub-reassoc-simplify-trunks", cl::init(true), cl::Hidden,
                   cl::desc("Enable simplification of trunks."));

static cl::opt<bool>
    SimplifyChains("addsub-reassoc-simplify-chains", cl::init(true), cl::Hidden,
                   cl::desc("Enable simplification of chains."));


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

static cl::opt<bool>
    ReuseChain("addsub-reassoc-reuse-chain", cl::init(true), cl::Hidden,
               cl::desc("Enables chains reuse during code generation."));

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

static inline bool isAllowedTrunkInstr(const Value *V) {
  return isAddSubInstr(V) || isAllowedAssocInstr(V);
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

// Begin of AddSubReassociatePass::AssocOpcodeData

AddSubReassociatePass::AssocOpcodeData::AssocOpcodeData(const Instruction *I) {
  assert(isAllowedAssocInstr(I) && "Expected Assoc Instr");
  Opcode = I->getOpcode();
  assert((isa<Constant>(I->getOperand(0)) || isa<Constant>(I->getOperand(1))) &&
         "Expected exactly one constant operand");
  Const = isa<Constant>(I->getOperand(0)) ? cast<Constant>(I->getOperand(0))
                                          : cast<Constant>(I->getOperand(1));
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void AddSubReassociatePass::AssocOpcodeData::dump() const {
  dbgs() << "(" << getOpcodeSymbol(Opcode);
  if (Const)
    dbgs() << " " << *Const;
  dbgs() << ")";
}
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

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
  for (auto &AssocOpcode : AssocOpcodeVec)
    AssocOpcode.dump();
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
    if (L == skipAssocs(LUPair.Opcode, Leaf) && !VisitedLUs.count(LUPair)) {
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

bool AddSubReassociatePass::Tree::hasLeaf(Value *Leaf) const {
  for (const auto &LUPair : LUVec)
    if (skipAssocs(LUPair.Opcode, Leaf) == LUPair.Leaf)
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

bool AddSubReassociatePass::Group::containsValue(Value *V) const {
  for (auto &Pair : Values)
    if (Pair.first == skipAssocs(Pair.second, V))
      return true;
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

#ifndef NDEBUG
// Checks if the code from the root to the leaves is in canonical form.
void AddSubReassociatePass::checkCanonicalized(Tree &T) {
  Value *TrunkV = T.getRoot();
  while (isa<Instruction>(TrunkV)) {
    Instruction *TrunkI = cast<Instruction>(TrunkV);
    // Operand 0 of trunk should never be a leaf. It should be either another
    // trunk node or zero.
    assert(isAddSubInstr(TrunkI) && "Only Add/Sub instrs allowed in the trunk");
    TrunkV = TrunkI->getOperand(0);
  }
  assert(isa<Constant>(TrunkV) && cast<Constant>(TrunkV)->isNullValue() &&
         "The only non-instr trunk node allowed is the zero at the top");
}
#endif

Value *AddSubReassociatePass::skipAssocs(const OpcodeData &OD,
                                               Value *Leaf) {
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

bool AddSubReassociatePass::canonicalizeIRForTrees(
    const TreeArrayTy &TreeArray) const {
  bool Changed = false;
  for (TreePtr &Tptr : TreeArray) {
    Changed |= canonicalizeIRForTree(*Tptr);
#ifndef NDEBUG
    if (EnableAddSubVerifier) {
      LUSetTy VisitedLUs;
      for (Instruction *TrunkI = Tptr->getRoot(); TrunkI;
           TrunkI = dyn_cast<Instruction>(TrunkI->getOperand(0))) {
        Value *Leaf = TrunkI->getOperand(1);
        unsigned TrunkOpcode = TrunkI->getOpcode();
        assert(TrunkOpcode ==
                   Tptr->getLeafCanonOpcode(Leaf, VisitedLUs, TrunkOpcode)
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
void AddSubReassociatePass::generateCode(Group &G, Tree *T,
                                         Instruction *GroupChain) const {
  Instruction *TopChainI = nullptr;
  Instruction *BottomChainI = nullptr;
  Instruction *MainOp0 = nullptr;
  // The trunk instructions that will become part of the chain.
  SmallVector<Instruction *, 16> BottomUpChainIVec;

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

  assert(
      BottomUpChainIVec.size() == G.size() &&
      "Check if Codegen Works correctly. We have a leaf with multiple users.");

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

  Value *GroupLeaf =
      GroupChain ? GroupChain->getOperand(1) : G.getValues()[0].first;
  const unsigned GroupOpcode = GroupChain ? GroupChain->getOpcode()
                                          : G.getValues()[0].second.getOpcode();

  LUSetTy VisitedLUss;
  const OpcodeData TreeOpData =
      T->getLeafCanonOpcode(GroupLeaf, VisitedLUss, GroupOpcode);
  bool MustFlipTreeOpcodes = TreeOpData.getOpcode() != GroupOpcode;

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
      // The tree can contain multiple identical leaves == Leaf, so avoid them.
      Instruction *ChainI = T->getNextLeafUser(Leaf, VisitedLUs);
      assert(std::find(BottomUpChainIVec.begin(), BottomUpChainIVec.end(),
                       ChainI) != BottomUpChainIVec.end() &&
             "Not found?");
      ChainInstrsInGroupOrder.push_back(ChainI);
    }
    // 3.a. We first move the chain instructions back-to-back. Without this step
    //      we cannot legally reorder the chain instrs freely to reflect groups.
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

    // 3.b. Now we can safely reorder them to reflect the order in 'G', without
    //      having to worry about the position of the Leaf instructions.
    //        ChainI_3
    //        ChainI_1
    //        ChainI_2
    //
    // We iterate throught the group leaves bottom-up and move the chain
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

  // 6. As a final step simplify the instruction chains to get rid of
  // redundancies like '0 + Val' from the top of the chain.
  if (!GroupChain && SimplifyChains)
    T->setRoot(simplifyTree(Bridge, false));

#ifndef NDEBUG
  if (EnableAddSubVerifier)
    assert(!verifyFunction(*Bridge->getParent()->getParent(), &dbgs()));
#endif
}

// Fix the '0' at the top of the trunk/chain. Returns possibly updated bridge
// instruction.
Instruction *AddSubReassociatePass::simplifyTree(Instruction *Bridge,
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

void AddSubReassociatePass::emitAssocInstrs(Tree *T) const {
  // If we have associative instructions, we need to emit them here.

  // STEP1 : Emit only the assoc instructions that are in groups.
  assert(EnableUnaryAssociations);
  for (const auto &LUPair : T->getLeavesAndUsers()) {
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
#ifndef NDEBUG
  if (EnableAddSubVerifier)
    assert(!verifyFunction(
        *cast<Instruction>(T->getRoot())->getParent()->getParent(), &dbgs()));
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
void AddSubReassociatePass::generateCode(GroupsVec &Groups,
                                         TreeArrayTy &TreeCluster) const {

  if (EnableUnaryAssociations)
    for (const TreePtr &Tptr : TreeCluster) {
      // If we have assoc instructions, emit them now.
      // NOTE: This makes it hard to get the leaves from the trunks. Using
      // TrunkI->getOperand(1) won't work. We have to use TrunkToLeafMap
      // instead.
      emitAssocInstrs(Tptr.get());
    }

  // For each tree in 'TreeCluster' generate the code.
  for (Group &G : Groups) {
    // Apply each group onto the tree.
    Instruction *GroupChain = nullptr;
    for (auto Titr = TreeCluster.rbegin(); Titr != TreeCluster.rend(); ++Titr) {
      Tree *T = Titr->get();
      generateCode(G, T, GroupChain);
      if (ReuseChain && !GroupChain) {
        GroupChain = cast<Instruction>(T->getRoot()->getOperand(1));
      }
    }
  }

  // Optimization: Remove the top zero constants.
  if (SimplifyTrunks) {
    for (const TreePtr &Tptr : TreeCluster) {
      simplifyTree(Tptr.get()->getRoot(), true);
    }
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void
AddSubReassociatePass::dumpGroups(const GroupsVec &Groups) const {
  int Cnt = 0;
  for (const Group &G : Groups) {
    dbgs() << "Group " << Cnt++ << "\n";
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

// Remove all instructions from OldRootI all the way to the leaves.
void AddSubReassociatePass::removeDeadTrunkInstrs(Tree *T,
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
bool AddSubReassociatePass::canonicalizeIRForTree(Tree &T) const {
  // Now that we know all the +/- opcodes associated to each leaf, we can build
  // the canonicalized tree.
  Value *Undef0 = UndefValue::get(T.getRoot()->getType());
  Instruction *LastTrunkI = nullptr;
  Instruction *RootTrunkI = nullptr;
  Instruction *InsertionPt = T.getRoot();

  // NOTE: We iterate the leaves bottom-up because we emit their corresponding
  // trunk instructions bottom-up.
  for (auto &LUPair : T.getLeavesAndUsers()) {
    Value *Leaf = LUPair.Leaf;
    const OpcodeData &Opcode = LUPair.Opcode;
    Instruction *OldUser = LUPair.User;
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
bool AddSubReassociatePass::growTree(Tree *T, WorkListTy &&WorkList) {
  unsigned SizeBefore = T->getLeavesCount();
  unsigned CntAssociations = 0;

  // Keep trying to grow tree until the WorkList is empty.
  while (!WorkList.empty()) {
    auto LastOp = WorkList.pop_back_val();

    assert(isAllowedTrunkInstr(LastOp.Leaf) &&
           "Work list item can't be trunk instruction.");
    Instruction *I = cast<Instruction>(LastOp.Leaf);
    bool IsAllowedAssocInstrI = isAllowedAssocInstr(I);

    if (IsAllowedAssocInstrI) {
      // Collect the unary associative instructions that apply for 'I'.
      LastOp.Opcode.appendAssocInstr(I);
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
        OpCanonOpcode = LastOp.Opcode.getFlipped();
      } else {
        // Reuse the opcode of the user.
        OpCanonOpcode = LastOp.Opcode;
      }

      if ( // Keep the size of a tree below a maximum value.
          T->getLeavesCount() + 2 * WorkList.size() < MaxTreeSize &&
          Op->hasOneUse() && arePredsInSameBB(Op, I) &&
          (isAddSubInstr(Op) ||
           (isAllowedAssocInstr(Op) &&
            // Check number of allowed assoc instruction.
            (++CntAssociations <= MaxUnaryAssociations)))) {
        // Push the operand to the WorkList to continue the walk up the code.
        WorkList.push_back(LeafUserPair(Op, I, OpCanonOpcode));
      } else {
        // 'Op' is a leaf node, so stop growing and add it into T's leaves.
        T->appendLeaf(I, Op, OpCanonOpcode);
        // If 'Op' is an add/sub and it is shared (maybe across trees maybe
        // not),
        // then this tree is a candidate for growing towards the shared leaves.
        // This is performed by 'extendTrees()'.
        if (isAllowedTrunkInstr(Op) && Op->getNumUses() > 1)
          T->setSharedLeafCandidate(true);
      }
    }
  }
  bool Changed = SizeBefore != T->getLeavesCount();
  return Changed;
}

bool AddSubReassociatePass::areAllUsesInsideTreeCluster(
    TreeArrayTy &TreeVec, const Value *Leaf,
    SmallVectorImpl<std::pair<Tree *, unsigned>> &WorkList) const {
  for (auto *U : Leaf->users()) {
    bool Found = false;
    for (TreePtr &Tptr : TreeVec) {
      auto *ATree = Tptr.get();
      auto LUVec = ATree->getLeavesAndUsers();
      for (unsigned I = 0; I < ATree->getLeavesCount(); ++I) {
        auto &LUPair = LUVec[I];
        if (LUPair.Leaf == Leaf && LUPair.User == U) {
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

bool AddSubReassociatePass::getSharedLeave(
    TreeArrayTy &TreeVec,
    SmallVectorImpl<std::pair<Tree *, unsigned>> &WorkList) {
  WorkList.clear();

  for (auto &Tptr : TreeVec) {
    Tree *ATree = Tptr.get();

    if (!ATree->hasSharedLeafCandidate())
      continue;

    for (auto &LUPair : ATree->getLeavesAndUsers()) {
      Value *LeafV = LUPair.Leaf;
      Instruction *SharedLeaf = dyn_cast<Instruction>(LeafV);

      // Need to clear WorkList since it may be polluted with data from previous
      // iteration.
      WorkList.clear();

      // Only Add/Sub leaves can become parts of trees once replicated.
      if (SharedLeaf && isAllowedTrunkInstr(SharedLeaf) &&
          SharedLeaf->getNumUses() > 1 &&
          arePredsInSameBB(SharedLeaf, ATree->getRoot()) &&
          areAllUsesInsideTreeCluster(TreeVec, SharedLeaf, WorkList)) {
        return true;
      }
    }
    ATree->setSharedLeafCandidate(false);
  }
  return false;
}

void AddSubReassociatePass::extendTrees(TreeArrayTy &Trees) {
  int MaxAttempts = MaxSharedNodesIterations;

  SmallVector<std::pair<Tree *, unsigned>, 8> SharedUsers;
  while (--MaxAttempts >= 0 && getSharedLeave(Trees, SharedUsers)) {
    for (auto &SharedUser : SharedUsers) {
      Tree *ATree = SharedUser.first;
      LeafUserPair LUPair = ATree->getLeafUserPair(SharedUser.second);
      ATree->removeLeaf(SharedUser.second);
      growTree(ATree, SmallVector<LeafUserPair, 8>({LUPair}));
      ATree->adjustSharedLeavesCount(1);
    }
    // In fact we don't need to clone a leaf for the first tree and may use
    // original instance. Adjust a counter by one.
    SharedUsers[0].first->adjustSharedLeavesCount(-1);
  }
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
    T->setRoot(&I);
    growTree(T, SmallVector<LeafUserPair, 8>(
                    {LeafUserPair(&I, nullptr, Instruction::Add)}));

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

  // 3. Try to extend the trees by including leaves with multiple uses across
  // multiple trees.
  if (EnableSharedLeaves) {
    for (TreeArrayTy &Cluster : Clusters)
      extendTrees(Cluster);
  }
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
      for (auto &UTree : TreeCluster)
        Score -= UTree.get()->getSharedLeavesCount();

      // Compute instruction difference for each group.
      for (auto &G : BestGroups) {
        auto AssocInstCnt = G.geAssocInstrCnt();
        int OrigScore = (G.size() + AssocInstCnt.first) * TreeCluster.size();
        int NewScore =
            (G.size() - 1 + AssocInstCnt.second + TreeCluster.size());
        Score += OrigScore - NewScore;
      }

      // Original tree (before canonicalization) could have one instruction
      // less. Lets be conservative and assume this is always the case.
      Score -= TreeCluster.size();

      // Check if transformation is profitable.
      if (Score <= 0)
        continue;

      // 3. Canonicalize the IR into a single linearized chain.
      Changed |= canonicalizeIRForTrees(TreeCluster);

      // 4. Now that we've got the best groups we can generate code.
      generateCode(BestGroups, TreeCluster);
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
