//===- Intel_AddSubReassociate.cpp - Reassociate AddSub expressions -------===//
//
// Copyright (C) 2018 - 2020 Intel Corporation. All rights reserved.
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
// - Currently, we can only handle simple operations for distribution (i.e. << Const).
//   This could be extended.

#include "llvm/Transforms/Scalar/Intel_AddSubReassociate.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/LoopInfo.h"
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
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include <algorithm>
#include <cassert>
#include <tuple>
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

static cl::opt<unsigned> MemCanonicalizationMaxGroupSize(
    "addsub-reassoc-memcan-max-group-size", cl::init(8), cl::Hidden,
    cl::desc(
        "The maximum group size to be considered for mem canonicalization."));

static cl::opt<int> MemCanonicalizationMaxLookupDepth(
    "addsub-reassoc-memcan-max-lookup-depth", cl::init(32), cl::Hidden,
    cl::desc("The maximum distance we are going to search for matching groups "
             "within BestGroups."));

static cl::opt<int> MaxScoringSearchDepth(
    "addsub-reassoc-max-scoring-depth", cl::init(4), cl::Hidden,
    cl::desc("The maximum search depth to find the optimal scoring."));

// Here in the pass we may exploit what is known in math as distributivity
// property. Thus far we only handle left shift by a constant operations
// for distribution (see canBeDistributed() routine for details).
// For an expression like "x + (a + b + c) * n", using distributivity property
// we can think of it as being same  as "x + a * n + b * n + c * n".
// For example, we have this (assuming x, y, z, a, b, c are all terminals):
//   S = (a + b + c) << Const
//   T = x + y + z + S
// then canonical (linear) representation of T would be: +x, +y, +z, +S.
// But if T is the only user of S then we can pull it into the tree and
// distribute left shift over "a","b",and "c"  then the tree now will be
// represented by form: +x, +y, +z, +a*, +b*, +c*,
// where "*" means additional (distributed) operation over these operands.
// This in theory may enable finding more common subexpressions.
// MaxDistributedOps parameter specifies how many ops can be subsumed for
// distribution. Value of 0 disables the feature.
static cl::opt<unsigned> MaxDistributedOps(
    "addsub-reassoc-max-distributed-instructions", cl::init(1), cl::Hidden,
    cl::desc("The maximum number of allowed operations to distribute "
             "into a tree (exploiting distributive property)."));

static cl::opt<bool> EnableSharedLeaves(
    "addsub-reassoc-unshare-leaves", cl::init(true), cl::Hidden,
    cl::desc("Enable growing the trees towards shared leaves"));

static cl::opt<bool> EnableAddSubVerifier("addsub-reassoc-verifier",
                                          cl::init(false), cl::Hidden,
                                          cl::desc("Enable addsub verifier."));

static cl::opt<int>
    MaxSharedNodesIterations("addsub-reassoc-max-unshared-leaves", cl::init(32),
                             cl::Hidden,
                             cl::desc("The maximum number of attempts for "
                                      "adding shared nodes to the trees."));

// The maximum size difference allowed for trees within a cluster.
static cl::opt<float>
    MaxTreeSizeDiffForCluster("addsub-reassoc-max-tree-size-diff",
                              cl::init(0.5), cl::Hidden,
                              cl::desc("The maximum tree size difference "
                                       "allowed within a cluster of trees."));

// The maximum size of clusters formed. This reduces the complexity of tree
// extension towards shared leaves.
static cl::opt<unsigned>
    MaxClusterSize("addsub-reassoc-max-cluster-size", cl::init(32), cl::Hidden,
                   cl::desc("The maximum size of a cluster of trees."));

// Forming clusters is quadratic to the number of trees. We limit the number of
// trees considered for clustering.
static cl::opt<unsigned> MaxClusterSearch(
    "addsub-reassoc-max-cluster-search", cl::init(32), cl::Hidden,
    cl::desc("Limit the search performed while forming clusters."));

static cl::opt<unsigned> TreeMatchThreshold(
    "addsub-reassoc-tree-match-threshold", cl::init(50), cl::Hidden,
    cl::desc("Trees match only if at least this number (%) of leaves match."));

// The minimum cluster size.
static cl::opt<unsigned> MinClusterSize(
    "addsub-reassoc-min-cluster-size", cl::init(2), cl::Hidden,
    cl::desc(
        "A cluster has to be at least this big to be considered for reassoc."));

static cl::opt<unsigned>
    MaxTreeSize("addsub-reassoc-max-tree-size", cl::init(64), cl::Hidden,
                cl::desc("Limit the size of the addsub reassoc expressions."));

static cl::opt<unsigned>
    MaxTreeCount("addsub-reassoc-max-tree-count", cl::init(0), cl::Hidden,
                 cl::desc("Maximum number of trees to build."));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Helper to print out opcode symbol.
LLVM_DUMP_METHOD static const char *getOpcodeSymbol(unsigned Opcode) {
  switch (Opcode) {
  case Instruction::Add:
    return "+";
  case Instruction::FAdd:
    return "+.";
  case Instruction::Sub:
    return "-";
  case Instruction::FSub:
    return "-.";
  case Instruction::Mul:
    return "*";
  case Instruction::FMul:
    return "*.";
  case Instruction::UDiv:
  case Instruction::SDiv:
    return "/";
  case Instruction::FDiv:
    return "/.";
  case Instruction::Shl:
    return "<<";
  case 0:
    return " ";
  }
  return "!Bad Opcode!";
}
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Return true if we can distribute the instruction operation on
// a tree leaf. Thus far we can only distribute left shift by a constant.
static inline bool canBeDistributed(const Instruction *I) {
  if (MaxDistributedOps == 0)
    return false;
  // Shifts should have a constant RHS operand.
  return I->getOpcode() == Instruction::Shl && isa<Constant>(I->getOperand(1));
}

static unsigned ReverseOpcode(unsigned Opc) {
  switch (Opc) {
  case Instruction::Add:
    return Instruction::Sub;
  case Instruction::FAdd:
    return Instruction::FSub;
  case Instruction::Sub:
    return Instruction::Add;
  case Instruction::FSub:
    return Instruction::FAdd;
  case Instruction::FMul:
    return Instruction::FDiv;
  case Instruction::FDiv:
    return Instruction::FMul;
  default:
    llvm_unreachable("Unimplemented opcode");
  }
}

// Return direct opcode form, i.e. "+" for arithmetic operations.
static unsigned getDirectFormOpcode(unsigned Opc) {
  switch (Opc) {
  case Instruction::Add:
    // Here and in other opcode manipulating routines Or is treated as Add.
    // TODO: eliminate Or handling altogether from utilities.
    // Instead, replace it with Add once proved it is legal to do.
  case Instruction::Or:
  case Instruction::Sub:
    return Instruction::Add;
  case Instruction::FAdd:
  case Instruction::FSub:
    return Instruction::FAdd;
  case Instruction::FMul:
  case Instruction::FDiv:
    return Instruction::FMul;
  default:
    llvm_unreachable("Unimplemented opcode");
  }
}
// Return reversed opcode form, i.e. "-" for arithmetic operations.
static unsigned getReversedFormOpcode(unsigned Opc) {
  return ReverseOpcode(getDirectFormOpcode(Opc));
}

static Constant *getIdentityValue(Type *Ty, unsigned Opcode) {
  switch (Opcode) {
  case Instruction::Add:
  case Instruction::Or:
  case Instruction::Sub:
    return ConstantInt::get(Ty, 0);
  case Instruction::Mul:
  case Instruction::SDiv:
  case Instruction::UDiv:
    return ConstantInt::get(Ty, 1);
  case Instruction::FAdd:
  case Instruction::FSub:
    return ConstantFP::get(Ty, 0.0);
  case Instruction::FMul:
  case Instruction::FDiv:
    return ConstantFP::get(Ty, 1.0);
  default:
    llvm_unreachable("Unimplemented opcode");
  }
}

// Return true if I1 and its operands that are instructions are all in
// the same BB as I2. I2 must be an instruction.
// I1 can be dyn_cast<Instruction> in which case it returns
// true if it turns out not an instruction.
static bool areInSameBB(Instruction *I1, Instruction *I2) {
  if (!I1)
    return true;
  if (I1->getParent() != I2->getParent())
    return false;
  for (int OpI = 0, E = I1->getNumOperands(); OpI != E; ++OpI) {
    auto *Op = dyn_cast<Instruction>(I1->getOperand(OpI));
    // don't care about non-instructions
    if (Op && Op->getParent() != I2->getParent())
      return false;
  }
  return true;
}

// Begin of AddSubReassociatePass::OpcodeData

void OpcodeData::reverse() { Opcode = ReverseOpcode(Opcode); }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void OpcodeData::dump() const {
  dbgs() << "(" << getOpcodeSymbol(Opcode) << ")";

  for (const DistributedOp &Op : getDistributedOps())
    dbgs() << "(" << getOpcodeSymbol(Op.first) << " " << *Op.second << ")";
}

LLVM_DUMP_METHOD void CanonNode::dump(unsigned Padding) const {
  dbgs().indent(Padding) << "Leaf ";
  Opcode.dump();
  getLeaf()->printAsOperand(dbgs());
  dbgs() << "\n";
}
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

/// Tries to ensure that the last node has commutative op (in many places
/// referenced here as "positive") by reordering nodes if required.
bool CanonForm::simplify() {
  if (empty())
    return true;

  // TODO: can we employ Instruction::isCommutative some way?
  auto isCommutativeOp = [](unsigned Opcode) -> bool {
    return Opcode == Instruction::Add || Opcode == Instruction::Or ||
           Opcode == Instruction::FAdd || Opcode == Instruction::FMul;
  };

  // Find first positive opcode and move it to beginning if required (using
  // reverse order).
  for (auto It = rbegin(); It != rend(); ++It) {
    if (isCommutativeOp(It->getOpcodeData().getOpcode())) {
      if (It != rbegin())
        swapLeaves(It.base() - 1, rbegin().base() - 1);
      return true;
    }
  }
  return false;
}

// Returned instruction is inserted before the specified insertion point \p IP
// and has 'undef' as first operand.
Instruction *CanonForm::generateInstruction(const OpcodeData &Opcode,
                                            Value *Leaf,
                                            Instruction *IP) const {
  Instruction *ResInst = nullptr;
  Value *Undef0 = UndefValue::get(Leaf->getType());
  FastMathFlags MathFlags;

  // Currently we only apply optimization for "fast" math operations only.
  MathFlags.setFast(true);

  // First emit distributed operations (if any).
  for (const OpcodeData::DistributedOp &Op : Opcode.getDistributedOps()) {
    auto Opcode = static_cast<Instruction::BinaryOps>(Op.first);
    Instruction *I = BinaryOperator::Create(Opcode, Leaf, Op.second);
    I->insertBefore(IP);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    I->setName(Name);
#endif
    Leaf = I;
  }

  // Second generate resulting instruction itself.
  switch (Opcode.getOpcode()) {
  case Instruction::Add:
    ResInst = BinaryOperator::CreateAdd(Undef0, Leaf);
    break;
  case Instruction::FAdd:
    ResInst = BinaryOperator::CreateFAdd(Undef0, Leaf);
    ResInst->setFastMathFlags(MathFlags);
    break;
  case Instruction::Sub:
    ResInst = BinaryOperator::CreateSub(Undef0, Leaf);
    break;
  case Instruction::FSub:
    ResInst = BinaryOperator::CreateFSub(Undef0, Leaf);
    ResInst->setFastMathFlags(MathFlags);
    break;
  case Instruction::Mul:
    ResInst = BinaryOperator::CreateMul(Undef0, Leaf);
    break;
  case Instruction::FMul:
    ResInst = BinaryOperator::CreateFMul(Undef0, Leaf);
    ResInst->setFastMathFlags(MathFlags);
    break;
  case Instruction::SDiv:
    ResInst = BinaryOperator::CreateSDiv(Undef0, Leaf);
    break;
  case Instruction::UDiv:
    ResInst = BinaryOperator::CreateUDiv(Undef0, Leaf);
    break;
  case Instruction::FDiv:
    ResInst = BinaryOperator::CreateFDiv(Undef0, Leaf);
    ResInst->setFastMathFlags(MathFlags);
    break;
  default:
    llvm_unreachable("Unsupported instruction generation request;");
  }

  // Insert before IP.
  ResInst->insertBefore(IP);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  ResInst->setName(Name);
#endif

  return ResInst;
}

Value *CanonForm::generateCode(Instruction *IP, bool GenTopZero) const {
  Value *Res = nullptr;
  Instruction *BottomI = nullptr;
  Instruction *TopI = nullptr;
  Instruction *PrevI = nullptr;

  if (empty())
    return nullptr;

  // Generate instructions for each node one by one and link them to a chain.
  for (auto It = begin(); It != end(); ++It) {
    PrevI = TopI;
    TopI = generateInstruction(It->getOpcodeData(), It->getLeaf(), IP);
    if (!BottomI)
      BottomI = TopI;
    else
      PrevI->setOperand(0, TopI);
    IP = TopI;
  }

  // First generated instruction (given by BottomI) will be returned if no
  // later transformations happen.
  Res = BottomI;

  // Check if it's legal to omit generation of top "zero".
  if (!GenTopZero && TopI->isCommutative()) {
    // Reconnect operand #1 of TopI to appropriate location and remove TopI.
    if (PrevI != nullptr)
      PrevI->setOperand(0, TopI->getOperand(1));
    else
      Res = TopI->getOperand(1);

    TopI->eraseFromParent();
  } else {
    // Set operand #0 of TopI to "zero".
    TopI->setOperand(0, getIdentityValue(TopI->getType(), TopI->getOpcode()));
  }
  return Res;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void CanonForm::dump() const {
  constexpr unsigned Padding = 2;
  dbgs() << getName() << ":\n";
  dbgs().indent(Padding) << "Linear form (size:" << size() << "):\n";
  for (const CanonNode &Leaf : llvm::reverse(Leaves))
    Leaf.dump(Padding);
  dbgs() << "\n";
}
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Return true if \p I is not null and is valid to be part of a tree trunk.
bool isLegalTrunkInstr(const Instruction *I, const Instruction *Root,
                       const DataLayout &DL) {
  auto isFAddSub = [](const Instruction *I) {
    return I->getOpcode() == Instruction::FAdd ||
           I->getOpcode() == Instruction::FSub;
  };
  auto isFMulDiv = [](const Instruction *I) {
    return I->getOpcode() == Instruction::FMul ||
           I->getOpcode() == Instruction::FDiv;
  };
  auto isAddSubKind = [&DL](const Instruction *I) {
    switch (I->getOpcode()) {
    case Instruction::Add:
    case Instruction::Sub:
      return true;
    case Instruction::Or:
      return haveNoCommonBitsSet(I->getOperand(0), I->getOperand(1), DL);
    }
    return false;
  };

  if (!I)
    return false;

  // If root has already been esteblished all trunk instructions have to be
  // "compatible" with the root.
  // Otherwise check whether the instruction is okay for root.
  if (Root) {
    if ((isAddSubKind(Root) && !(isAddSubKind(I) || canBeDistributed(I))) ||
        (isFAddSub(Root) && !isFAddSub(I)) ||
        (isFMulDiv(Root) && !isFMulDiv(I)))
      return false;
  } else if (!isAddSubKind(I) && !isFAddSub(I) && !isFMulDiv(I))
    return false;

  // If this is an FP op make sure fast math flags are set.
  auto *FPMathOp = dyn_cast<FPMathOperator>(I);
  if (FPMathOp && !FPMathOp->getFastMathFlags().isFast())
    return false;

  return true;
}

// Begin of AddSubReassociatePass::Tree

bool Tree::hasTrunkInstruction(const Instruction *I) const {
  // Note: Currently maximum tree size is limited to a small number (64). That's
  // why we are doing full traversal instead of using a set. Please consider
  // changing this as needed.
  std::function<bool(Instruction *)> checkTreeRec =
      [&](Instruction *TreeI) -> bool {
    if (I == TreeI)
      return true;
    for (Value *V : TreeI->operands()) {
      auto *Op = dyn_cast<Instruction>(V);
      if (isLegalTrunkInstr(Op, Root, DL) && !hasLeaf(Op) && checkTreeRec(Op))
        return true;
    }
    return false;
  };
  return Root && checkTreeRec(Root);
}

void Tree::clear() {
  Root = nullptr;
  HasSharedLeafCandidate = false;
  SharedLeavesCount = 0;
  CanonForm::clear();
}

void Tree::removeTreeFromIR() {
  SmallVector<Instruction *, 16> POT;
  Instruction *RootI = getRoot();

  // Walk up the instructions until the leaves are reached.
  // Push the instructions into the POT vector.
  std::function<void(Value *)> GetPOT = [&](Value *V) {
    Instruction *I = dyn_cast<Instruction>(V);
    if (!I || hasLeaf(V))
      return;
    for (int i = 0, e = I->getNumOperands(); i != e; ++i)
      GetPOT(I->getOperand(i));
    // Post-order
    POT.push_back(I);
  };
  GetPOT(RootI);

  // Generate dummy "no-op" instruction to keep track of "tree location" in IR.
  // This serves as an insertion point during code generation.
  Value *Identity = getIdentityValue(RootI->getType(), RootI->getOpcode());
  Instruction *NewRootI = generateInstruction(
      getDirectFormOpcode(RootI->getOpcode()), Identity, RootI);
  NewRootI->setOperand(0, Identity);
  RootI->replaceAllUsesWith(NewRootI);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  NewRootI->setName("DummyRoot_");
#endif
  setRoot(NewRootI);

  // Erase in Reverse Post-Order Traversal.
  for (Instruction *I : llvm::reverse(POT)) {
    // Instructions may be shared across trees. Remove only if no uses left.
    if (I->use_empty())
      I->eraseFromParent();
  }
}

Value *Tree::generateCode() {
  Instruction *OldRoot = getRoot();
  Value *Res = CanonForm::generateCode(OldRoot, !SimplifyTrunks);
  assert(isa<Instruction>(Res) && "Tree root must be an instruction");

  Instruction *NewRoot = cast<Instruction>(Res);

  // Unlink old tree representation (which should be single "no-op" instruction)
  // in the IR by replacing all uses with NewRoot.
  OldRoot->replaceAllUsesWith(NewRoot);
  OldRoot->eraseFromParent();
  setRoot(NewRoot);

  return NewRoot;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void Tree::dump() const {
  // First dump canonical representation.
  CanonForm::dump();

  dbgs().indent(2) << "LLVM IR for " << getName() << "\n";

  // Then dump IR repesentation.
  std::function<void(Value *)> dumpTreeRec = [this, &dumpTreeRec](Value *V) {
    auto *I = dyn_cast<Instruction>(V);
    if (I && !hasLeaf(I) && !isa<PHINode>(I))
      for (Value *Op : I->operands())
        if (isa<Instruction>(Op))
          dumpTreeRec(Op);
    // Post-order
    const char *Prefix = "      ";
    if (hasLeaf(V))
      Prefix = "(Leaf)";
    else if (V == Root)
      Prefix = "(Root)";

    dbgs().indent(2) << Prefix << " " << *V << "\n";
  };

  if (Root && size())
    dumpTreeRec(Root);
}
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Begin of AddSubReassociatePass::Group

bool Group::simplify(TreeSignVecTy &GroupTrees) {
  assert(!empty() && "Attempt to simplify empty group");

  // First try basic simplification.
  if (CanonForm::simplify())
    return true;

  // No direct opcode was found.
  // Reverse all the group.
  reverseOpcodes();

  // And adjust "sign" used to link the group to a tree.
  for (auto &TreeAndSign : GroupTrees)
    TreeAndSign.second = !TreeAndSign.second;

  return true;
}

bool Group::isSimilar(const Group &G2) {
  // Sizes should match.
  if (G2.size() != size())
    return false;

  // Now check that opcodes matches.
  SmallSet<unsigned, 8> G2Opcodes;
  for (auto &GV2 : G2)
    G2Opcodes.insert(GV2.getOpcodeData().getOpcode());

  for (auto &GV : *this)
    if (!G2Opcodes.count(GV.getOpcodeData().getOpcode()))
      return false;
  return true;
}

void Group::sort() {
  llvm::sort(*this, [](const CanonNode &VO1, const CanonNode &VO2) {
    const auto *I1 = dyn_cast<Instruction>(VO1.getLeaf());
    const auto *I2 = dyn_cast<Instruction>(VO2.getLeaf());
    if (I1 && I2) {
      // If instr opcodes differ, sort by their opcode.
      if (I1->getOpcode() != I2->getOpcode())
        return I1->getOpcode() < I2->getOpcode();
      return VO1.getOpcodeData() < VO2.getOpcodeData();
    }
    return I2 != nullptr;
  });
}

// Begin of AddSubReassociatePass
Optional<int64_t> AddSubReassociate::findLoadDistance(Value *V1, Value *V2,
                                                      unsigned MaxDepth) const {

  SmallVector<std::tuple<Value *, Value *, unsigned>, 4> Stack(1, {V1, V2, 0});

  while (!Stack.empty()) {
    Value *Left;
    Value *Right;
    unsigned Depth;
    std::tie(Left, Right, Depth) = Stack.pop_back_val();

    auto *I1 = dyn_cast<Instruction>(Left);
    auto *I2 = dyn_cast<Instruction>(Right);
    // Both must be instructions with same opcode
    if (!I1 || !I2 || I1->getOpcode() != I2->getOpcode())
      continue;
    auto *LI1 = dyn_cast<LoadInst>(I1);
    auto *LI2 = dyn_cast<LoadInst>(I2);
    if (LI1 && LI2) {
      if (LI1->getPointerAddressSpace() != LI2->getPointerAddressSpace())
        continue;
      // Check pointers
      Value *Ptr1 = LI1->getPointerOperand();
      Value *Ptr2 = LI2->getPointerOperand();
      const SCEV *Scev1 = SE->getSCEV(Ptr1);
      const SCEV *Scev2 = SE->getSCEV(Ptr2);
      const SCEV *Diff = SE->getMinusSCEV(Scev1, Scev2);
      if (const auto *DiffConst = dyn_cast<SCEVConstant>(Diff))
        return DiffConst->getAPInt().getSExtValue();
      continue;
    }
    if (Depth == MaxDepth)
      continue;
    ++Depth;
    for (int I = I1->getNumOperands() - 1; I >= 0; --I)
      Stack.emplace_back(I1->getOperand(I), I2->getOperand(I), Depth);
  }
  return None;
}

// Returns the sum of the absolute distances of SortedLeaves and G2.
int64_t AddSubReassociate::getSumAbsDistances(const CanonForm &G1,
                                              const CanonForm &G2) {
  assert(G1.size() == G2.size() && "Expected same size");
  int64_t Sum = 0;
  for (auto G1It = G1.begin(), G2It = G2.begin(); G1It != G1.end();
       ++G1It, ++G2It) {
    Value *V1 = G1It->getLeaf();
    Value *V2 = G2It->getLeaf();
    Optional<int64_t> Distance = findLoadDistance(V1, V2);
    if (Distance)
      Sum += std::abs(Distance.value());
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
int64_t AddSubReassociate::getBestSortedScore_rec(
    const Group &G1, const Group &G2, CanonForm G1Leaves, CanonForm G2Leaves,
    CanonForm &LastSortedG1Leaves, CanonForm &BestSortedG1Leaves,
    int64_t &BestScore, int depth) {
  if (depth > MaxScoringSearchDepth)
    return MAX_DISTANCE;

  // If we reached the bottom, return the total distance.
  if (G2Leaves.empty()) {
    assert(G1Leaves.empty() && "G1Leaves and G2Leaves out of sync.");
    // Now get the sum of the absolute distances between SortedLeaves and G2.
    return getSumAbsDistances(LastSortedG1Leaves, G2);
  }
  // Let's try to match G2's G2LeafIdx'th leaf.
  Value *G2LeafV = G2Leaves.begin()->getLeaf();
  G2Leaves.removeLeaf(G2Leaves.begin());

  // Go through the remaining G1 leaves looking for matches with G2LeafV.
  SmallVector<CanonNode, 4> Matches;
  for (auto &G1Leaf : G1Leaves)
    if (findLoadDistance(G2LeafV, G1Leaf.getLeaf()))
      Matches.push_back(G1Leaf);
  // Early exit if no match.
  if (Matches.empty())
    return MAX_DISTANCE;

  // Recursively try all matches to find the one that leads to the best score.
  for (auto &G1Leaf : Matches) {
    // Create copies, one for each of the matched leaves.
    CanonForm SortedG1LeavesCopy = LastSortedG1Leaves;
    SortedG1LeavesCopy.appendLeaf(G1Leaf.getLeaf(), G1Leaf.getOpcodeData());
    CanonForm G1LeavesCopy = G1Leaves;
    G1LeavesCopy.removeLeaf(llvm::find(G1LeavesCopy, G1Leaf));
    int64_t Score = getBestSortedScore_rec(
        G1, G2, G1LeavesCopy, G2Leaves, SortedG1LeavesCopy, BestSortedG1Leaves,
        BestScore, depth + 1);
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

// Entry point for getBestSortedScore_rec().
// Returns false if we did not manage to get a good ordering that matches G2.
bool AddSubReassociate::getBestSortedLeaves(const Group &G1, const Group &G2,
                                            CanonForm &BestSortedG1Leaves) {
  int64_t BestScore = MAX_DISTANCE;
  CanonForm DummyG1SortedLeaves;

  getBestSortedScore_rec(G1, G2, G1, G2, DummyG1SortedLeaves,
                         BestSortedG1Leaves, BestScore, /*depth=*/0);
  if (BestSortedG1Leaves.size() != G1.size())
    return false;
  assert(std::is_permutation(BestSortedG1Leaves.begin(),
                             BestSortedG1Leaves.end(), G1.begin()) &&
         "The sorted vector should be a permutation of current G1 leaves.");
  return true;
}

// Canonicalize: (i) the order of the values in the group, (ii) the trunk
// opcodes, to match the ones in G2. This value ordering takes into account the
// memory accesses primarily.
// Returns true on success.
bool AddSubReassociate::memCanonicalizeGroupBasedOn(Group &G1, const Group &G2,
                                                    ScalarEvolution *SE) {
  Group SortedG1;
  if (!getBestSortedLeaves(G1, G2, SortedG1))
    return false;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Keep same name.
  SortedG1.setName(G1.getName());
#endif
  // Update the current group.
  G1 = SortedG1;
  return true;
}

// Massage 'G' into a form that matches a similar group in 'BestGroups'.
// Returns true on success.
bool AddSubReassociate::memCanonicalizeGroup(Group &G,
                                             TreeSignVecTy &GroupTreeVec) {
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

  // If more than half of the opcodes don't match, reverse the entire group.
  unsigned CntOpcodeMatches = 0;
  Group &G1 = G;
  Group &G2 = *MainGroup;
  for (auto G1It = G1.begin(), G2It = G2.begin(); G1It != G1.end();
       ++G1It, ++G2It)
    CntOpcodeMatches +=
        (G1It->getOpcodeData().getOpcode() == G2It->getOpcodeData().getOpcode())
            ? 1
            : 0;

  if (CntOpcodeMatches < G1.size() / 2) {
    G1.reverseOpcodes();
    // Change sign of a "bridge" for all trees.
    for (auto &TreeAndSignPair : GroupTreeVec)
      TreeAndSignPair.second = !TreeAndSignPair.second;
  }
  return true;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// TODO: This looks ugly. Probably the reason these dump routines were
// implemented via templates is use of somewhat overcomplicated data structures
// defined via stack of aliases. Need to find some way to untie this knot,
// likely via redesigning data structures for intermediate representation.
template <typename T>
LLVM_DUMP_METHOD void dumpHistTableElem(const T TableElem) {
  // Fisrt print a value (leaf) of interest.
  auto *V = cast<Value>(TableElem.first);
  dbgs().indent(2);
  V->printAsOperand(dbgs());
  dbgs() << "-> {";
  // Then print out name of each tree where the value is used followed by a
  // list of opcodes (symbols) associated with each use of the value in the
  // tree.
  for (const auto &TreeAndOpcodes : TableElem.second) {
    dbgs() << TreeAndOpcodes.first->getName() << ":(";
    for (auto &Opcode : TreeAndOpcodes.second)
      dbgs() << getOpcodeSymbol(Opcode->getOpcode()) << " ";
    dbgs() << ") ";
  }
  dbgs() << "}\n";
}

template <typename T>
LLVM_DUMP_METHOD void dumpHistTable(const T &Table,
                                    const char *Banner = nullptr) {
  dbgs() << (Banner ? Banner : "HistTable") << " {\n";
  for (const auto &ValueAndTreesPair : Table) {
    dumpHistTableElem(ValueAndTreesPair);
  }
  dbgs() << "}\n";
}

LLVM_DUMP_METHOD void dumpGroupAndTrees(const Group &G,
                                        const TreeSignVecTy &GroupTrees) {
  G.dump();
  dbgs() << "Group to trees linking:\n";
  for (const std::pair<Tree *, bool> &TreeAndSign : GroupTrees) {
    Tree *T = TreeAndSign.first;
    dbgs() << T->getName() << " "
           << getOpcodeSymbol(
                  TreeAndSign.second
                      ? getDirectFormOpcode(T->getRoot()->getOpcode())
                      : getReversedFormOpcode(T->getRoot()->getOpcode()))
           << " " << G.getName() << "\n";
  }
  dbgs() << "\n";
}
#endif

static auto findCompatibleOpcode(const OpcodeData &Opc,
                                 ArrayRef<const OpcodeData *> Opcodes) {
  return find_if(Opcodes, [&Opc](const OpcodeData *OD) {
    return OD->areDistributedOpsEqual(Opc);
  });
}

static auto findMatchingOpcode(const OpcodeData &Opc,
                               ArrayRef<const OpcodeData *> Opcodes) {
  return find_if(Opcodes, [&Opc](const OpcodeData *OD) { return *OD == Opc; });
}

// Scan all trees in \p Cluster and try to find common leaves used across the
// trees (referred as a "group" later on). Group G can be applied to tree T if
// each leaf in G has either same or the opposite operation as same leaf in T
// consistently across all leaves in G. [Using more conventional terms: the
// group G is basically a common subexpression in a tree T, so that each tree
// in the cluster can be rewritten as T' + G or T'-G].
// For example, consider these two trees:
//   T1: +X +B +C
//   T2: +Y -B -C.
// For these trees we can build group G:  +B, +C
// The group contributes to both trees with opposite operations thus both
// trees can be rewritten as:
//   T1: +X+G
//   T2: +Y-G.
// We iteratively try to build a group which spans across maximal number of
// trees.
// Number of trees a group can be applied to is referred as group width.
// Number of leaves in a group is referred as group size.
// The goal is to build wider groups rather than bigger in size.
// Thus if addition of a leaf to a group will result in decrease of group's
// width then we prefer to build a group which is smaller in size than
// the current one.
void AddSubReassociate::buildMaxReuseGroups(
    const MutableArrayRef<TreePtr> &Cluster) {
  constexpr int MaxNumTrees = 16;

  if (Cluster.empty())
    return;

  LLVM_DEBUG(dbgs() << "==== Start building groups for cluster ===\n");

  LLVM_DEBUG(dumpCluster(Cluster));

  // Since one leaf can appear in a tree (and as a result in a group) more than
  // once we need to be able to keep list of opcodes it appears with.
  using OpcodesTy = SmallVector<const OpcodeData *, 4>;
  using TreeAndOpcodesTy = std::pair<Tree *, OpcodesTy>;
  using TreeCollectionTy = SmallVector<TreeAndOpcodesTy, MaxNumTrees>;

  // For each leaf we keep list of tree and opcodes pair. Thus for each leaf
  // we know in which trees it appears and what opcodes are.
  MapVector<Value *, TreeCollectionTy> LeafHistTable;
  // Build initial map.
  for (const TreePtr &T : Cluster) {
    Tree *Tree = T.get();
    // Traverse in reverse order just to match legacy behavior.
    for (const CanonNode &LUPair : llvm::reverse(*T)) {
      TreeCollectionTy &Trees = LeafHistTable[LUPair.getLeaf()];
      // Find current tree.
      auto TreeAndOpcodesIt =
          find_if(Trees, [&Tree](const TreeAndOpcodesTy &Elem) {
            return Elem.first == Tree;
          });

      // Add current tree to the list if doesn't exist.
      if (TreeAndOpcodesIt == Trees.end()) {
        Trees.emplace_back(Tree, OpcodesTy());
        TreeAndOpcodesIt = Trees.end() - 1;
      }

      // Add opcode the leaf has in current tree.
      TreeAndOpcodesIt->second.push_back(&LUPair.getOpcodeData());
    }
  }

  // Early bail out. Later code depends on the table not being empty.
  if (LeafHistTable.empty())
    return;

  // Now we need to sort by number of trees each leaf appears in (also known
  // as a group width). So here map representation is "converted" into a vector
  // for sorting. Elements of the vector are pairs: {Value*, TreeCollectionTy}.
  auto LeafHistVec = LeafHistTable.takeVector();

  // Sorting predicate.
  auto SortPred = [](const decltype(LeafHistVec)::value_type &LHS,
                     const decltype(LeafHistVec)::value_type &RHS) {
    return LHS.second.size() > RHS.second.size();
  };

  // Perform sorting. Use stable sort to generate same code from run to run.
  std::stable_sort(LeafHistVec.begin(), LeafHistVec.end(), SortPred);

  LLVM_DEBUG(dumpHistTable(LeafHistVec, "Initial histogram for the cluster:"));

  // Start with maximum possible width.
  size_t GroupWidth = LeafHistVec.front().second.size();
  // ... and iterate while we have leaves shared across two or more trees.
  while (GroupWidth > 1) {
    LLVM_DEBUG(dbgs() << "=== Search for next group of width " << GroupWidth
                      << " ===\n");

    // This is more convenient representation of a group used in the algorithm.
    // It not only gives us a group itself but also list of tries the group
    // is applicable to. The later is needed to delete information about found
    // group from the histogram table.
    using HistVecItTy = decltype(LeafHistVec)::iterator;
    SmallVector<std::pair<HistVecItTy, const OpcodeData>, 16> LocalGroup;
    // FoundTrees keeps set of trees where current leaf appears.
    // The boolean part of the pair records whether opcode was the exact
    // match (if not the exact match then the opcode is reversed).
    SmallDenseMap<Tree *, bool, MaxNumTrees> FoundTrees;

    // Iterate through all leaves in sorted table.
    for (auto TableIt = LeafHistVec.begin(); TableIt != LeafHistVec.end();
         ++TableIt) {
      std::pair<Value *, TreeCollectionTy> &CurElem = *TableIt;
      TreeCollectionTy &CandidateTrees = CurElem.second;
      const size_t CandidateTreesNum = CandidateTrees.size();

      // Bail out if current leaf is not common across at least GroupWidth
      // trees. No need to go over remaining elements because data is sorted.
      if (CandidateTreesNum < GroupWidth)
        // No need to go other remaining elements since the table is sorted.
        break;

      // Try to find any 'GroupWidth' trees legal for group extension. Double
      // loop nest is needed to try out all possible combinations (that should
      // be fine since current cluster size is limited to 16).
      for (size_t I = 0; I < CandidateTreesNum - GroupWidth + 1; ++I) {
        size_t FoundTreeNum = 0;
        Optional<OpcodeData> GroupOpcode = None;
        for (size_t J = I; J < CandidateTreesNum && FoundTreeNum < GroupWidth;
             ++J) {
          Tree *CandidateTree = CandidateTrees[J].first;
          OpcodesTy &CandidateOpcodes = CandidateTrees[J].second;

          // Check if this is going to be the first leaf in the group.
          if (LocalGroup.empty()) {
            // We use the same opcode for the group as in the first tree.
            if (FoundTreeNum == 0) {
              GroupOpcode = *CandidateOpcodes.front();
              // The first one is always the exact opcode match.
              FoundTrees.try_emplace(CandidateTree, true);
              ++FoundTreeNum;
              continue;
            }

            auto OpcIt = findCompatibleOpcode(*GroupOpcode, CandidateOpcodes);
            if (OpcIt == CandidateOpcodes.end())
              continue;

            FoundTrees.try_emplace(CandidateTree, (*OpcIt)->getOpcode() ==
                                                      GroupOpcode->getOpcode());
            ++FoundTreeNum;
            continue;
          }
          // Ok, there is at least one leaf in the group which spans across
          // trees in 'FoundTrees'. Find out if current leaf appears in the
          // same trees with compatible opcodes.
          auto FoundTreeIt = FoundTrees.find(CandidateTree);
          if (FoundTreeIt == FoundTrees.end())
            continue;
          bool IsDirect = FoundTreeIt->second;
          // We use the same opcode for the group as in first tree.
          if (FoundTreeNum == 0) {
            GroupOpcode = IsDirect ? *CandidateOpcodes.front()
                                   : (*CandidateOpcodes.front()).getReversed();
            ++FoundTreeNum;
            continue;
          }
          // Count only the exact match.
          if (findMatchingOpcode(IsDirect ? *GroupOpcode
                                          : GroupOpcode->getReversed(),
                                 CandidateOpcodes) != CandidateOpcodes.end())
            ++FoundTreeNum;
        }

        // Check if were able to find 'GroupWidth' trees for the current leaf.
        if (FoundTreeNum == GroupWidth) {
          // Add current leaf (pointed by 'TableIt') and Opcode to the group.
          LocalGroup.emplace_back(TableIt, *GroupOpcode);
          // Stop search trees for current leaf.
          break;
        }

        // If we were not able to find 'GroupWidth' for the very first leaf
        // candidate we need to clear 'FoundTrees' to start building it from
        // the beginning for next slice of trees.
        if (LocalGroup.empty())
          FoundTrees.clear();
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
    for (const std::pair<HistVecItTy, const OpcodeData> &Elem : LocalGroup)
      G.appendLeaf(Elem.first->first /*Leaf*/, Elem.second /*Opcode*/);

    // Populate list of trees current group is applicable to. Please note that
    // we have to preserve incoming order (order in Cluster) since code
    // generation assumes that later tree in Cluster appears earlier in
    // program order.
    TreeSignVecTy GroupTreeVec;
    for (const TreePtr &T : Cluster) {
      auto It = FoundTrees.find(T.get());
      if (It != FoundTrees.end())
        GroupTreeVec.emplace_back(It->first, It->second);
    }

    // Remove current group from the 'LeafHistVec'.
    for (const std::pair<HistVecItTy, const OpcodeData> &Elem : LocalGroup) {
      // The "Elem.first" iterator points to a {Value*, TreeCollectionTy} pair.
      TreeCollectionTy &TreeVec = Elem.first->second;
      const OpcodeData GroupOpcode = Elem.second;

      TreeVec.erase(
          remove_if(TreeVec,
                    [&FoundTrees, &GroupOpcode](
                        TreeCollectionTy::value_type &TreeAndOpcodes) {
                      Tree *Tree = TreeAndOpcodes.first;
                      auto It = FoundTrees.find(Tree);
                      if (It == FoundTrees.end())
                        return false;
                      OpcodesTy &Opcodes = TreeAndOpcodes.second;
                      auto OpcIt = findMatchingOpcode(
                          It->second /*IsDirect*/ ? GroupOpcode
                                                  : GroupOpcode.getReversed(),
                          Opcodes);
                      assert(OpcIt != Opcodes.end() &&
                             "Can't find requested opcode for the group");
                      Opcodes.erase(OpcIt);
                      return Opcodes.empty();
                    }),
          TreeVec.end());
    }

    // Sort 'LeafHistVec' after removal of the group.
    // Use stable sort to generate same code from run to run.
    std::stable_sort(LeafHistVec.begin(), LeafHistVec.end(), SortPred);

    // Canonicalize found group based on memory accesses if enabled.
    if (EnableGroupCanonicalization && !memCanonicalizeGroup(G, GroupTreeVec))
      G.sort();

    LLVM_DEBUG(dbgs() << "Found Group:\n");
    LLVM_DEBUG(dumpGroupAndTrees(G, GroupTreeVec));

    LLVM_DEBUG(dumpHistTable(LeafHistVec, "Histogram after removal of the group:"));

    // Finally push constructed group to a list.
    BestGroups.emplace_back(std::move(G), std::move(GroupTreeVec));

    // Make sure group width doesn't exceed current maximal number of candidate
    // trees.
    GroupWidth = std::min(GroupWidth, LeafHistVec.front().second.size());
  }
  LLVM_DEBUG(dbgs() << "==== End building groups ===\n");
}

void AddSubReassociate::removeCommonNodes() {
  // Loop over all groups.
  for (std::pair<Group, TreeSignVecTy> &GroupAndTrees : BestGroups) {
    Group &G = GroupAndTrees.first;
    // Loop over all trees the group should be removed from.
    for (std::pair<Tree *, bool> &TreeAndSign : GroupAndTrees.second) {
      Tree *T = TreeAndSign.first;
      bool IsDirect = TreeAndSign.second;
      // Loop over all nodes in the Group.
      for (const CanonNode &Node : G) {
        // Match corresponding node in the tree.
        auto It = T->findLeaf(Node.getLeaf(),
                              IsDirect ? Node.getOpcodeData()
                                       : Node.getOpcodeData().getReversed());
        assert(It != T->end() && "Can't find group leaf in a tree");
        // Go ahead and remove matched node form the tree.
        T->removeLeaf(It);
      }
    }
  }
}

void AddSubReassociate::linkGroup(Value *GroupChain,
                                  TreeSignTy &TreeAndSign) const {
  assert(GroupChain && "Attempt to link empty group.");

  Tree *T = TreeAndSign.first;
  // Figure out the 'Bridge' opcode.
  unsigned BridgeOpcode = TreeAndSign.second
                              ? getDirectFormOpcode(T->getRoot()->getOpcode())
                              : getReversedFormOpcode(T->getRoot()->getOpcode());
  // Append bridge to the tree.
  T->appendLeaf(GroupChain, BridgeOpcode);
}

void AddSubReassociate::generateCode(const ArrayRef<Tree *> AffectedTrees) {

  // 1. Generate the code for each group.
  for (std::pair<Group, TreeSignVecTy> &GroupAndTrees : BestGroups) {
    Group &G = GroupAndTrees.first;
    TreeSignVecTy &Cluster = GroupAndTrees.second;

    if (G.empty())
      continue;

    // Simplify the instruction chains to get rid of redundancies like
    // '0 + Val' from the top of the chain.
    if (SimplifyChains)
      G.simplify(Cluster);

    // Here we implicitly assumes that last tree in a cluster is a lexically
    // first in the IR. We will use its root as an insertion point for the
    // group.
    Instruction *IP = Cluster.rbegin()->first->getRoot();
    Value *GroupChain = G.generateCode(IP, !SimplifyChains);

    // Link generated group to all affected trees.
    for (auto It = Cluster.rbegin(); It != Cluster.rend(); ++It)
      linkGroup(GroupChain, *It);
  }

  // 2. Generate code for each affected tree.
  for (Tree *T : AffectedTrees) {
    if (SimplifyTrunks)
      T->simplify();
    T->generateCode();
#ifndef NDEBUG
    if (EnableAddSubVerifier)
      assert(!verifyFunction(*T->getRoot()->getParent()->getParent(), &dbgs()));
#endif
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void AddSubReassociate::dumpGroups() const {
  int Cnt = 0;
  for (const std::pair<Group, TreeSignVecTy> &GroupAndTrees : BestGroups) {
    dbgs() << "Group " << Cnt++ << "\n";
    GroupAndTrees.first.dump();
  }
}

LLVM_DUMP_METHOD void AddSubReassociate::dumpTrees() const {
  int Cnt = 0;
  for (const TreePtr &T : Trees) {
    dbgs().indent(2) << "--- Tree " << Cnt++ << "/" << Trees.size() << " ---\n";
    T->dump();
  }
}

LLVM_DUMP_METHOD void
AddSubReassociate::dumpCluster(const MutableArrayRef<TreePtr> &Cluster) const {
  dbgs() << "Cluster:\n";
  int Cnt = 0;
  for (const TreePtr &T : Cluster) {
    dbgs().indent(2) << "--- Tree " << Cnt++ << "/" << Cluster.size()
                     << " ---\n";
    T->dump();
  }
}

LLVM_DUMP_METHOD void AddSubReassociate::dumpClusters() const {
  int Cnt = 0;
  for (const auto &C : Clusters) {
    dbgs() << "Cluster " << Cnt++ << "/" << Clusters.size()
           << " NumTrees: " << C.size() << "\n";
    dbgs() << "============================\n";
    dumpCluster(C);
    dbgs() << "\n\n";
  }
}
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

/// Return true if more than TreeMatchThreshold per cent of
/// leaves do match in T1 and in T2.
static bool TreesMatch(const Tree *T1, const Tree *T2) {
  // TODO: Complexity is linear to the size of the trees.
  // Ideally this should be a constant time calculation.

  SmallPtrSet<Value *, 8> T1Values;
  for (auto &TV : *T1)
    T1Values.insert(TV.getLeaf());

  // Count how many of T2's leaves match.
  int Matches = 0;
  for (auto &TV : *T2)
    if (T1Values.count(TV.getLeaf()))
      Matches++;

  return (Matches * 100) / T1Values.size() > TreeMatchThreshold;
}

void AddSubReassociate::clusterTrees() {
  // TODO: (1) Tune the clustering heuristics.
  //       (2) This is currently quadratic to Trees size.
  //           Reduce complexity by sorting (based on size?)
  MutableArrayRef<TreePtr> TreeVecArray(Trees);
  unsigned ClusterSize = 1;
  for (size_t i = 0, e = Trees.size(); i != e; i += ClusterSize) {
    const Tree *T1 = Trees[i].get();
    // Now look for other trees within Trees.
    ClusterSize = 1;
    // Break quadratic complexity by reducing the maximum search.
    int EndOfSearch = std::min(e, i + 1 + MaxClusterSearch);
    for (int j = i + 1; j != EndOfSearch; ++j) {
      Tree *T2 = Trees[j].get();
      // If i)  size difference is within limits.
      //    ii) the trees share a min number of values.
      if ((std::abs((int64_t)T2->size() - (int64_t)T1->size()) /
               (double)T2->size() <
           MaxTreeSizeDiffForCluster) &&
          isLegalTrunkInstr(T2->getRoot(), T1->getRoot(), DL) &&
          TreesMatch(T2, T1)) {
        // Move this element next to the last member of the cluster.
        auto First = Trees.begin() + i + ClusterSize;
        auto Middle = Trees.begin() + j;
        auto Last = Middle + 1;
        std::rotate(First, Middle, Last);
        // Reduce complexity by placing a cap on the size of the cluster
        if (++ClusterSize > MaxClusterSize)
          break;
      }
    }
    // Create a cluster only if the size is adequate.
    if (ClusterSize >= MinClusterSize) {
      MutableArrayRef<TreePtr> Cluster = TreeVecArray.slice(i, ClusterSize);
      Clusters.push_back(Cluster);
    }
  }
}

unsigned AddSubReassociate::growTree(Tree *Tree, unsigned GrowthLimit,
                                     SmallVectorImpl<CanonNode> &&WorkList) {
  unsigned NumDistributedIns = 0;
  // Keep trying to grow tree until the WorkList is empty
  // or growth allowance reached.
  unsigned LeavesAdded = 0;
  const Instruction *Root = Tree->getRoot();
  while (!WorkList.empty()) {
    CanonNode LastOp = WorkList.pop_back_val();
    auto *I = cast<Instruction>(LastOp.getLeaf());

    assert(isLegalTrunkInstr(I, Root, DL) &&
           "Work list item can't be trunk instruction.");

    // If current instruction starts another tree then just clear that tree
    // since it will become part of the growing tree.
    auto It = find_if(Trees, [&I, &Tree](const TreePtr &T) {
      return T.get() != Tree && T->getRoot() == I;
    });
    if (It != Trees.end())
      It->get()->clear();

    // Remember the operand number if one is pulled in
    int PulledOpNum = canBeDistributed(I) ? LastOp.addDistributedOp(I) : -1;

    for (int OpIdx : {0, 1}) {
      // Skip the operand which was pulled in as it does not contribute to
      // the tree directly.
      if (PulledOpNum == OpIdx)
        continue;

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
      OpcodeData OpCanonOpcode =
          !I->isCommutative() && OpIdx != 0
              ?
              // We reverse the opcode when we cross the RHS of a SUB.
              LastOp.getOpcodeData().getReversed()
              :
              // Reuse the opcode of the user.
              LastOp.getOpcodeData();

      auto *OpI = dyn_cast<Instruction>(Op);
      if (OpI && Op->hasOneUse() &&
          // Check against growth limit
          (LeavesAdded + 2 * WorkList.size()) < GrowthLimit &&
          areInSameBB(OpI, I) && isLegalTrunkInstr(OpI, Root, DL) &&
          // Check number of allowed ops we can distribute.
          (!canBeDistributed(OpI) ||
           ++NumDistributedIns <= MaxDistributedOps)) {
        // Push the operand to the WorkList to continue the walk up the code.
        WorkList.push_back(CanonNode(Op, OpCanonOpcode));
      } else {
        ++LeavesAdded;
        // Op is a leaf node, so stop growing and add it into Tree's leaves.
        Tree->appendLeaf(Op, OpCanonOpcode);
        // If Op has multiple uses and is legal for trunk then this tree is
        // a candidate for trying to grow towards the shared leaves.
        // Those leaves shared across trees are candidates while those
        // having multiple uses within same tree are not. It is determined later
        // in findSharedleaves() routine and further tree growth is performed by
        // extendTrees().
        if (Op->hasNUsesOrMore(2) && isLegalTrunkInstr(OpI, Root, DL))
          Tree->setSharedLeafCandidate(true);
      }
    }
  }
  return Tree->size();
}

/// Returns true if we were able to find a leaf with multiple uses from
/// trees in \p Cluster only, false otherwise. Each found use is pushed
/// to a \p FoundLeaves as a Tree* and Leaf index pair.
static bool findSharedLeaves(
    MutableArrayRef<AddSubReassociate::TreePtr> &Cluster,
    SmallVectorImpl<std::pair<Tree *, CanonForm::NodeItTy>> &FoundLeaves,
    const DataLayout &DL) {

  auto AllUsesInsideCluster = [&Cluster,
                               &FoundLeaves](const Value *Leaf) -> bool {
    unsigned UseCount = 0;
    for (auto &T : Cluster) {
      auto *Tree = T.get();
      auto It = Tree->findLeaf(Leaf);
      // It is important that for each tree we only take into account one use
      // that belongs to the tree. If a leaf is shared within the same tree we
      // cannot put it in trunk as all trunk instructions only allowed to have
      // single use within a tree.
      if (It != Tree->end()) {
        FoundLeaves.push_back(std::make_pair(Tree, It));
        ++UseCount;
      }
    }
    return Leaf->hasNUses(UseCount);
  };

  FoundLeaves.clear();

  for (auto &T : Cluster) {
    auto *Tree = T.get();

    if (!Tree->hasSharedLeafCandidate())
      continue;

    for (auto &TV : *Tree) {
      Value *LeafV = TV.getLeaf();
      auto *I = dyn_cast<Instruction>(LeafV);
      if (!I)
        continue;

      // Need to clear FoundLeaves since it may be polluted with data from
      // previous iteration.
      FoundLeaves.clear();

      // Only legal for trunk leaves can become parts of trees once replicated.
      if (I->hasNUsesOrMore(2) && isLegalTrunkInstr(I, Tree->getRoot(), DL) &&
          areInSameBB(I, Tree->getRoot()) && AllUsesInsideCluster(I))
        return true;
    }
    Tree->setSharedLeafCandidate(false);
  }
  return false;
}

void AddSubReassociate::extendTrees() {
  for (MutableArrayRef<TreePtr> &Cluster : Clusters) {
    SmallVector<std::pair<Tree *, CanonForm::NodeItTy>, 8> SharedUsers;
    int Attempts = MaxSharedNodesIterations;
    unsigned TheBiggestTreeSize = 0;
    for (const TreePtr &T : Cluster)
      TheBiggestTreeSize =
          std::max<unsigned>(TheBiggestTreeSize, T.get()->size());

    while (--Attempts >= 0) {
      // Stop here if we have no more room to grow.
      if (TheBiggestTreeSize >= MaxTreeSize)
        break;

      if (!findSharedLeaves(Cluster, SharedUsers, DL))
        break;
      unsigned GrowthLimit = MaxTreeSize - TheBiggestTreeSize;
      unsigned SharedLeavesNum = 0;
      // Important note! Call to removeLeaf invalidates all iterators pointing
      // after the removed one. For that reason we need to delete in reverse
      // order. That's not the most efficient thing to do but we expect, one
      // leaf to be used multiple times by the tree, to be a rare case.
      for (auto &SharedUser : llvm::reverse(SharedUsers)) {
        Tree *T = SharedUser.first;
        CanonNode LUPair = *SharedUser.second;
        T->removeLeaf(SharedUser.second);
        unsigned OrigLeavesNum = T->size();
        unsigned NewSize =
            growTree(T, GrowthLimit, SmallVector<CanonNode, 8>({LUPair}));
        // Keep tracking the biggest tree size.
        TheBiggestTreeSize = std::max<unsigned>(TheBiggestTreeSize, NewSize);
        if (SharedLeavesNum == 0) {
          assert(NewSize > OrigLeavesNum && "No leaves unshared?");
          SharedLeavesNum = NewSize - OrigLeavesNum;
        } else {
          assert((NewSize - OrigLeavesNum) == SharedLeavesNum &&
                 "Inconsistent number of unshared leaves across trees.");
          T->adjustSharedLeavesCount(SharedLeavesNum);
        }
      }
    }
  }
}

void AddSubReassociate::buildInitialTrees(BasicBlock *BB) {
  // Scan the BB in reverse and build a tree.
  for (Instruction &I : make_range(BB->rbegin(), BB->rend())) {
    // Check that number of built trees doesn't exceed specified limits.
    // '0' means "unlimited" unless it is set by the user.
    if ((MaxTreeCount.getNumOccurrences() > 0 || MaxTreeCount > 0) &&
        Trees.size() >= (unsigned)MaxTreeCount)
      break;

    // A tree is rooted at a legal instruction and the instruction does not
    // belong to any other tree.
    if (!isLegalTrunkInstr(&I, nullptr, DL) ||
        any_of(Trees,
               [&I](const TreePtr &T) { return T->hasTrunkInstruction(&I); }))
      continue;

    TreePtr UTree = std::make_unique<Tree>(DL);
    Tree *T = UTree.get();
    T->setRoot(&I);
    unsigned Size =
        growTree(T, MaxTreeSize,
                 SmallVector<CanonNode, 8>(
                     {CanonNode(&I, getDirectFormOpcode(I.getOpcode()))}));

    assert(1 <= Size && Size <= MaxTreeSize + 2 &&
           "Tree size should be capped");

    // Skip trivial trees.
    if (Size > 1)
      Trees.push_back(std::move(UTree));
  }
}

// Build Add/Sub trees with instructions from BB.
void AddSubReassociate::buildTrees(BasicBlock *BB, bool UnshareLeaves) {
  // 1. Scan the code in BB and build initial trees.
  buildInitialTrees(BB);
  if (Trees.empty())
    return;

  // 2. Create clusters of trees out of the trees in Trees. Each cluster
  // contains trees that: i.  share leaf nodes, and ii. have similar sizes.
  // The trees in a cluster are good candidates for AddSub reassociation.
  //
  // It is important to form clusters at this point because
  // extendTrees() is quadratic to the number of trees
  // processed. This, could potentially reduce the effectiveness of that
  // function. The assumption here is that extending the trees is beneficial
  // only if the trees are already similar enough.
  // TODO: It might be worth to have two separate clustering steps, one here
  // and one before the group formation with different heuristics/parameters
  // each.
  //
  // NOTE: This reorders the elements in Trees.
  clusterTrees();

  // 3. Try to extend the trees by including leaves with multiple uses across
  // multiple trees.
  if (EnableSharedLeaves && UnshareLeaves)
    extendTrees();
}

// Entry point for AddSub reassociation.
// Returns true if we need to invalidate analysis.
bool AddSubReassociate::run() {

  if (!AddSubReassocEnable)
    return false;

  bool Changed = false;
  ReversePostOrderTraversal<Function *> RPOT(F);

  // Scan the code for opportunities of AddSub reassociation.
  // Make a "pairmap" of how often each operand pair occurs.
  for (BasicBlock *BB : RPOT) {
    bool KeepTrying = true;
    for (unsigned I = 0; KeepTrying && I < MaxBBIters; ++I) {
      KeepTrying = false;
      Trees.clear();
      Clusters.clear();

      // 1. Build as many trees as we can find in BB. We need to perform leaves
      // unsharing on the first iteration only since all consequent iterations
      // will observe already unshared leaves from the first iteration.
      buildTrees(BB, I == 0);
      // We did not collect any clusters. Continue looking for more trees.
      if (Clusters.empty())
        continue;

      for (MutableArrayRef<TreePtr> &Cluster : Clusters) {
        // 2. Form groups of nodes that reduce divergence
        BestGroups.clear();
        buildMaxReuseGroups(Cluster);
        if (BestGroups.empty())
          continue;

        // The score is the instruction count savings (# before - # after).
        // If we apply group G, we are:
        //  i. introducing G.size()-1 new instructions (for the tmp value), and
        //  ii. removing G.size()*Cluster.size() instructions from the trees
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
        SmallSetVector<Tree *, 16> AffectedTrees;
        for (std::pair<Group, TreeSignVecTy> &GroupAndTrees : BestGroups)
          for (std::pair<Tree *, bool> &TreeAndSign : GroupAndTrees.second)
            AffectedTrees.insert(TreeAndSign.first);

        int Score = 0;
        for (Tree *T : AffectedTrees)
          Score -= T->getSharedLeavesCount();

        // Compute difference in instruction count for each group.
        for (std::pair<Group, TreeSignVecTy> &GroupAndTrees : BestGroups) {
          Group &G = GroupAndTrees.first;
          int UniqDistributions, TotalDistributions;
          std::tie(UniqDistributions, TotalDistributions) =
              G.collectDistributedOpsStatistics();
          int TreeNum = GroupAndTrees.second.size();
          int OrigCost = (G.size() + UniqDistributions) * TreeNum;
          int NewCost = (G.size() - 1 + TotalDistributions + TreeNum);
          Score += OrigCost - NewCost;
        }

        // Original tree (before canonicalization) could have one instruction
        // less. Lets be conservative and assume this is always the case.
        Score -= AffectedTrees.size();

        // Check if transformation is profitable.
        if (Score < 0) {
          LLVM_DEBUG(dbgs() << "Discarding groups. Score=" << Score << "\n");
          continue;
        }

        LLVM_DEBUG(dbgs() << "Applying groups. Score=" << Score << "\n");

        // For each tree in AffectedTrees remove its IR representation.
        for (Tree *T : AffectedTrees)
          T->removeTreeFromIR();

        removeCommonNodes();

        // 4. Now that we've got the best groups we can generate code.
        generateCode(AffectedTrees.getArrayRef());

        KeepTrying = true;
        Changed = true;
      }
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
    PA.preserve<AndersensAA>();
    return PA;
  }

  return PreservedAnalyses::all();
}

char AddSubReassociateLegacyPass::ID = 0;

AddSubReassociateLegacyPass::AddSubReassociateLegacyPass() : FunctionPass(ID) {
  initializeAddSubReassociateLegacyPassPass(*PassRegistry::getPassRegistry());
}

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
  AU.addPreserved<AndersensAAWrapperPass>();
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
