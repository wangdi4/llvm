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
#include "llvm/Analysis/Intel_Andersens.h"
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
    "addsub-reassoc-memcan-max-group-size", cl::init(64), cl::Hidden,
    cl::desc(
        "The maximum group size to be considered for mem canonicalization."));

static cl::opt<int> MemCanonicalizationMaxLookupDepth(
    "addsub-reassoc-memcan-max-lookup-depth", cl::init(32), cl::Hidden,
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

static bool isFAddSubInstr(const Instruction *I, const DataLayout &DL) {
  switch (I->getOpcode()) {
  case Instruction::FAdd:
  case Instruction::FSub:
    return true;
  default:
    return false;
  }
}

static bool isFMulDivInstr(const Instruction *I, const DataLayout &DL) {
  switch (I->getOpcode()) {
  case Instruction::FMul:
  case Instruction::FDiv:
    return true;
  default:
    return false;
  }
}

// Binary operation represented by \p Opcode is positive if the following holds:
// A Opcode B == B Opcode A
static bool isPositiveOpcode(unsigned Opcode) {
  switch (Opcode) {
  case Instruction::Add:
  case Instruction::Or:
  case Instruction::FAdd:
  case Instruction::FMul:
    return true;
  }
  return false;
}

// Returns positive counterpart of \p Opcode.
static unsigned getPositiveOpcode(unsigned Opcode) {
  switch (Opcode) {
  case Instruction::Add:
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

// Binary operation represented by \p Opcode is negative if the following holds:
// A Opcode B == -(B Opcode A)
static unsigned getNegativeOpcode(unsigned Opcode) {
  switch (Opcode) {
  case Instruction::Add:
  case Instruction::Or:
  case Instruction::Sub:
    return Instruction::Sub;
  case Instruction::FAdd:
  case Instruction::FSub:
    return Instruction::FSub;
  case Instruction::FMul:
  case Instruction::FDiv:
    return Instruction::FDiv;
  default:
    llvm_unreachable("Unimplemented opcode");
  }
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

// Returns true if 'V1' and 'I2' are in the same BB, or if V1 not an
// instruction.
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
  case Instruction::FAdd:
    NewOpcode.Opcode = Instruction::FSub;
    break;
  case Instruction::Sub:
    NewOpcode.Opcode = Instruction::Add;
    break;
  case Instruction::FSub:
    NewOpcode.Opcode = Instruction::FAdd;
    break;
  case Instruction::Mul:
    NewOpcode.Opcode = Instruction::SDiv;
    break;
  case Instruction::FMul:
    NewOpcode.Opcode = Instruction::FDiv;
    break;
  case Instruction::SDiv:
  case Instruction::UDiv:
    NewOpcode.Opcode = Instruction::Mul;
    break;
  case Instruction::FDiv:
    NewOpcode.Opcode = Instruction::FMul;
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

LLVM_DUMP_METHOD void CanonNode::dump(const unsigned Padding) const {
  dbgs().indent(Padding) << "Leaf ";
  Opcode.dump();
  dbgs() << *Leaf << "\n";
}
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

void CanonForm::appendLeaf(Value *Leaf, const OpcodeData &Opcode) {
  Leaves.push_back(CanonNode(Leaf, Opcode));
}

CanonForm::NodeItTy CanonForm::findLeaf(const NodeItTy It, const Value *Leaf,
                                        const OpcodeData &Opcode) const {

  for (auto CurIt = It; CurIt != end(); CurIt++) {
    if (CurIt->getLeaf() == Leaf) {
      if (Opcode.isUndef() || CurIt->getOpcodeData() == Opcode)
        return CurIt;
    }
  }
  return end();
}

/// Tries to ensure that the last node has "positive" opcode
/// (see isPositiveOpcode) by reordering nodes if required.
bool CanonForm::simplify() {
  if (empty())
    return true;

  // Find first positive opcode and move it to beginning if required (using
  // reverse order).
  for (auto It = rbegin(); It != rend(); ++It) {
    if (isPositiveOpcode(It->getOpcodeData().getOpcode())) {
      if (It != rbegin()) {
        swapLeaves(It.base() - 1, rbegin().base() - 1);
        return true;
      }
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

  // First emit all associative instructions if any.
  for (const AssocOpcodeData &Data : llvm::reverse(Opcode)) {
    Instruction::BinaryOps BinOpcode =
        static_cast<Instruction::BinaryOps>(Data.getOpcode());
    Instruction *NewAssocI =
        BinaryOperator::Create(BinOpcode, Leaf, Data.getConst());
    NewAssocI->insertBefore(IP);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    NewAssocI->setName(Name);
#endif
    Leaf = NewAssocI;
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
  if (!GenTopZero && isPositiveOpcode(TopI->getOpcode())) {
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

void CanonForm::flipOpcodes() {
  // Flip opcode of each canonical node.
  for (const CanonNode &Leaf : *this) {
    const_cast<CanonNode &>(Leaf).Opcode = Leaf.Opcode.getFlipped();
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void CanonForm::dump() const {
  const unsigned Padding = 2;
  dbgs().indent(Padding) << "Linear form for " << getName()
                         << ". Size=" << size() << "\n";
  for (auto &Leaf : llvm::reverse(Leaves))
    Leaf.dump(Padding);
  dbgs() << "\n";
}
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Begin of AddSubReassociatePass::Tree

bool Tree::isAllowedTrunkInstr(const Value *V) const {
  auto *I = dyn_cast<Instruction>(V);
  if (!I)
    return false;

  if ((isAddSubInstr(I, DL) && (!Root || isAddSubInstr(Root, DL))) ||
      isAllowedAssocInstr(I))
    return true;

  if (((isFAddSubInstr(I, DL) && (!Root || (isFAddSubInstr(Root, DL)))) ||
       (isFMulDivInstr(I, DL) && (!Root || isFMulDivInstr(Root, DL)))) &&
      cast<FPMathOperator>(I)->getFastMathFlags().isFast())
    return true;

  return false;
}

bool Tree::hasTrunkInstruction(const Instruction *I) const {
  // Note: Currently maximum tree size is limited to a small number (64). That's
  // why we are doing full traversal instead of using a set. Please consider
  // changing this as needed.
  std::function<bool(Instruction *)> checkTreeRec =
      [&](Instruction *TreeI) -> bool {
    if (I == TreeI)
      return true;
    for (int i = 0, e = TreeI->getNumOperands(); i != e; ++i) {
      Instruction *Op = dyn_cast<Instruction>(TreeI->getOperand(i));
      if (Op != nullptr && isAllowedTrunkInstr(Op) &&
          findLeaf(begin(), Op) == end() && checkTreeRec(Op))
        return true;
    }
    return false;
  };
  return getRoot() != nullptr && checkTreeRec(getRoot());
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
    if (!I || findLeaf(begin(), V) != end())
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
      getPositiveOpcode(RootI->getOpcode()), Identity, RootI);
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
  const unsigned Padding = 2;

  // First dump canonical representation.
  CanonForm::dump();

  dbgs().indent(Padding) << "LLVM IR for " << getName() << "\n";

  // Then dump IR repesentation.
  std::function<void(Value *)> dumpTreeRec = [&](Value *V) {
    Instruction *I = dyn_cast<Instruction>(V);
    if (I && findLeaf(begin(), I) == end() && !isa<PHINode>(I))
      for (int i = 0, e = I->getNumOperands(); i != e; ++i) {
        Value *Op = I->getOperand(i);
        if (isa<Instruction>(Op))
          dumpTreeRec(Op);
      }
    // Post-order
    const char *Prefix = nullptr;
    if (findLeaf(begin(), V) != end())
      Prefix = "(Leaf)";
    else if (V == Root)
      Prefix = "(Root)";
    else
      Prefix = "      ";
    dbgs().indent(Padding) << Prefix << " " << *V << "\n";
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

  // No positive opcode was found.
  // Flip all opcodes in the group.
  flipOpcodes();

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
  auto valuesCmp = [](const CanonNode &VO1, const CanonNode &VO2) -> bool {
    Value *V1 = VO1.getLeaf();
    const OpcodeData &OD1 = VO1.getOpcodeData();
    Value *V2 = VO2.getLeaf();
    const OpcodeData &OD2 = VO2.getOpcodeData();

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
  llvm::sort(*this, valuesCmp);
}

void Group::flipOpcodes() { CanonForm::flipOpcodes(); }

// Begin of AddSubReassociatePass

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
int64_t AddSubReassociate::getSumAbsDistances(const CanonForm &G1,
                                              const CanonForm &G2) {
  assert(G1.size() == G2.size() && "Expected same size");
  int64_t Sum = 0;
  for (auto G1It = G1.begin(), G2It = G2.begin(); G1It != G1.end();
       ++G1It, ++G2It) {
    Value *V1 = G1It->getLeaf();
    Value *V2 = G2It->getLeaf();
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
    const Group &G1, const Group &G2, CanonForm G1Leaves, CanonForm G2Leaves,
    CanonForm &LastSortedG1Leaves, CanonForm &BestSortedG1Leaves,
    int64_t &BestScore) {
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
  int64_t Distance = 0;
  for (auto &G1Leaf : G1Leaves)
    if (getValDistance(G2LeafV, G1Leaf.getLeaf(), 2, Distance))
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
bool AddSubReassociate::getBestSortedLeaves(const Group &G1, const Group &G2,
                                            CanonForm &BestSortedG1Leaves) {
  int64_t BestScore = MAX_DISTANCE;
  CanonForm DummyG1SortedLeaves;

  getBestSortedScoreRec(G1, G2, G1, G2, DummyG1SortedLeaves, BestSortedG1Leaves,
                        BestScore);
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
                                             TreeSignVecTy &GroupTreeVec,
                                             GroupTreesVecTy &BestGroups) {
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
  for (auto G1It = G1.begin(), G2It = G2.begin(); G1It != G1.end();
       ++G1It, ++G2It)
    CntOpcodeMatches +=
        (G1It->getOpcodeData().getOpcode() == G2It->getOpcodeData().getOpcode())
            ? 1
            : 0;

  if (CntOpcodeMatches < G1.size() / 2) {
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
    dbgs() << TreeAndOpcodes.first->getName() << ":(";
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

LLVM_DUMP_METHOD void dumpGroupAndTrees(const Group &G,
                                    const TreeSignVecTy &GroupTrees) {
  G.dump();

  dbgs() << "Group linking:\n";
  for (auto &TreeAndSign : GroupTrees) {
    Tree *T = TreeAndSign.first;
    bool IsPositive = TreeAndSign.second;
    dbgs() << T->getName() << " ";
    dbgs() << getOpcodeSymbol(
        IsPositive ? getPositiveOpcode(T->getRoot()->getOpcode())
                   : getNegativeOpcode(T->getRoot()->getOpcode()));
    dbgs()  << " " << G.getName() << "\n";
  }
  dbgs() << "\n";
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
                                            GroupTreesVecTy &BestGroups) {
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
    // Traverse in reverse order just to match legacy behavior.
    for (auto &LUPair : llvm::reverse(*Tptr)) {
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
            if ((It = FindCompatibleOpcode(GroupOpcode, CantidateOpcodes,
                                           false)) != CantidateOpcodes.end()) {
              FoundTrees.insert({CandidateTree, (*It)->getOpcode() ==
                                                    GroupOpcode.getOpcode()});
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
    TreeSignVecTy GroupTreeVec;
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

    // Canonicalize found group based on memory accesses if enabled.
    if (EnableGroupCanonicalization) {
      if (!EnableMemCanonicalization ||
          !memCanonicalizeGroup(G, GroupTreeVec, BestGroups))
        G.sort();
    }

    LLVM_DEBUG(dbgs() << "Found Group:\n");
    LLVM_DEBUG(dumpGroupAndTrees(G, GroupTreeVec));

    LLVM_DEBUG(dumpHistTable(LeafHistVec));

    // Finally push constructed group to a list.
    BestGroups.push_back({std::move(G), std::move(GroupTreeVec)});

    // Make sure group width doesn't exceed current maximal number of candidate
    // trees.
    GroupWidth = std::min(GroupWidth, LeafHistVec.front().second.size());
  }
  LLVM_DEBUG(dbgs() << "==== End building groups ===\n");
}

// Remove all instructions from OldRootI all the way to the leaves.
void AddSubReassociate::removeOldTrees(
    const ArrayRef<Tree *> AffectedTrees) const {
  for (Tree *T : AffectedTrees) {
    T->removeTreeFromIR();
  }
}

void AddSubReassociate::removeGroupFromTree(GroupTreesVecTy &Groups) const {
  // Loop over all groups.
  for (auto &GroupAndTrees : Groups) {
    Group &G = GroupAndTrees.first;
    // Loop over all trees the group should be removed from.
    for (auto &TreeAndSign : GroupAndTrees.second) {
      Tree *T = TreeAndSign.first;
      bool IsGroupFlipped = !TreeAndSign.second;
      CanonForm::NodeItTy It = T->begin();
      // Loop over all nodes in the Group.
      for (const auto &Node : G) {
        // Match corresponding node in the tree.
        It = T->findLeaf(T->begin(), Node.getLeaf(),
                         IsGroupFlipped ? Node.getOpcodeData().getFlipped()
                                        : Node.getOpcodeData());
        assert(It != T->end() && "Can't find group leaf in a tree");
        // Go ahead and remove matched node form the tree.
        It = T->removeLeaf(It);
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
                              ? getPositiveOpcode(T->getRoot()->getOpcode())
                              : getNegativeOpcode(T->getRoot()->getOpcode());
  // Append bridge to the tree.
  T->appendLeaf(GroupChain, BridgeOpcode);
}

void AddSubReassociate::generateCode(
    GroupTreesVecTy &Groups, const ArrayRef<Tree *> AffectedTrees) const {

  // 1. Generate the code for each group.
  for (auto &GroupAndTreesPair : Groups) {
    Group &G = GroupAndTreesPair.first;
    TreeSignVecTy &TreeCluster = GroupAndTreesPair.second;

    if (G.empty())
      continue;

    // Simplify the instruction chains to get rid of redundancies like
    // '0 + Val' from the top of the chain.
    if (SimplifyChains)
      G.simplify(TreeCluster);

    // Here we implicitly assumes that last tree in a cluster is a lexically
    // first in the IR. We will use its root as an insertion point for the
    // group.
    Instruction *IP = TreeCluster.rbegin()->first->getRoot();
    Value *GroupChain = G.generateCode(IP, !SimplifyChains);

    // Link generated group to all affected trees.
    for (auto It = TreeCluster.rbegin(); It != TreeCluster.rend(); ++It)
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
LLVM_DUMP_METHOD void
AddSubReassociate::dumpGroups(const GroupTreesVecTy &Groups) const {
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

  if (!T1->isAllowedTrunkInstr(T2->getRoot()))
    return false;

  for (auto &TV : *T1)
    T1Values.insert(TV.getLeaf());

  // Count how many of T2's leaves match.
  int Matches = 0;
  for (auto &TV : *T2)
    if (T1Values.count(TV.getLeaf()))
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
      if ((std::abs((int64_t)T2->size() - (int64_t)T1->size()) /
               (double)T2->size() <
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

// Try to grow the tree upwards, towards the definitions.
// Returns true if new nodes have been added to the tree.
bool AddSubReassociate::growTree(TreeVecTy &AllTrees, Tree *T,
                                 WorkListTy &&WorkList) {
  unsigned SizeBefore = T->size();
  unsigned CntAssociations = 0;

  // Keep trying to grow tree until the WorkList is empty.
  while (!WorkList.empty()) {
    auto LastOp = WorkList.pop_back_val();

    assert(T->isAllowedTrunkInstr(LastOp.getLeaf()) &&
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
      LastOp.appendAssocInstruction(I);
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
      if (!isPositiveOpcode(I->getOpcode()) && OpIdx != 0) {
        // We flip the opcode when we cross the RHS of a SUB.
        OpCanonOpcode = LastOp.getOpcodeData().getFlipped();
      } else {
        // Reuse the opcode of the user.
        OpCanonOpcode = LastOp.getOpcodeData();
      }

      if ( // Keep the size of a tree below a maximum value.
          T->size() + 2 * WorkList.size() < MaxTreeSize && Op->hasOneUse() &&
          arePredsInSameBB(Op, I) &&
          (T->isAllowedTrunkInstr(Op) &&
           (!isAllowedAssocInstr(Op) ||
            // Check number of allowed assoc instruction.
            (++CntAssociations <= MaxUnaryAssociations)))) {
        // Push the operand to the WorkList to continue the walk up the code.
        WorkList.push_back(CanonNode(Op, OpCanonOpcode));
      } else {
        // 'Op' is a leaf node, so stop growing and add it into T's leaves.
        T->appendLeaf(Op, OpCanonOpcode);
        // If 'Op' is an add/sub and it is shared (maybe across trees maybe
        // not),
        // then this tree is a candidate for growing towards the shared
        // leaves. This is performed by 'extendTrees()'.
        if (T->isAllowedTrunkInstr(Op) && Op->getNumUses() > 1)
          T->setSharedLeafCandidate(true);
      }
    }
  }
  bool Changed = SizeBefore != T->size();
  return Changed;
}

bool AddSubReassociate::areAllUsesInsideTreeCluster(
    TreeArrayTy &TreeCluster, const Value *Leaf,
    SmallVectorImpl<std::pair<Tree *, CanonForm::NodeItTy>> &WorkList) const {
  unsigned UseCount = 0;
  for (TreePtr &Tptr : TreeCluster) {
    auto *T = Tptr.get();
    auto It = T->findLeaf(T->begin(), Leaf);
    while (It != T->end()) {
      WorkList.push_back(std::make_pair(T, It));
      ++UseCount;
      It = T->findLeaf(It + 1, Leaf);
    }
  }
  return Leaf->hasNUses(UseCount);
}

bool AddSubReassociate::getSharedLeave(
    TreeArrayTy &TreeCluster,
    SmallVectorImpl<std::pair<Tree *, CanonForm::NodeItTy>> &WorkList) {
  WorkList.clear();

  for (auto &Tptr : TreeCluster) {
    Tree *ATree = Tptr.get();

    if (!ATree->hasSharedLeafCandidate())
      continue;

    for (auto &TV : *ATree) {
      Value *LeafV = TV.getLeaf();
      Instruction *SharedLeaf = dyn_cast<Instruction>(LeafV);

      // Need to clear WorkList since it may be polluted with data from
      // previous iteration.
      WorkList.clear();

      // Only Add/Sub leaves can become parts of trees once replicated.
      if (SharedLeaf && ATree->isAllowedTrunkInstr(SharedLeaf) &&
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

  SmallVector<std::pair<Tree *, CanonForm::NodeItTy>, 8> SharedUsers;
  while (--MaxAttempts >= 0 && getSharedLeave(TreeCluster, SharedUsers)) {
    size_t SharedLeavesNum = 0;
    // Important note! Call to 'removeLeaf' invalidates all iterators pointing
    // after the removed one. For that reason we need to delete in reverse
    // order. That's not the most efficient thing to do but we expect, one leaf
    // to be used multiple times by the tree, to be a rare case.
    for (auto &SharedUser : llvm::reverse(SharedUsers)) {
      Tree *ATree = SharedUser.first;
      CanonNode LUPair = *SharedUser.second;
      ATree->removeLeaf(SharedUser.second);
      size_t OrigLeavesNum = ATree->size();
      growTree(AllTrees, ATree, SmallVector<CanonNode, 8>({LUPair}));
      if (SharedLeavesNum == 0) {
        assert((ATree->size() - OrigLeavesNum) > 0 && "No leaves unshared?");
        SharedLeavesNum = ATree->size() - OrigLeavesNum;
      } else {
        assert((ATree->size() - OrigLeavesNum) == SharedLeavesNum &&
               "Inconsistent number of unshared leaves across trees.");
        ATree->adjustSharedLeavesCount(SharedLeavesNum);
      }
    }
  }
}

void AddSubReassociate::buildInitialTrees(BasicBlock *BB, TreeVecTy &AllTrees) {
  // Returns true if 'I' is a candidate for a tree root.
  auto isRootCandidate = [&](const Instruction *I) -> bool {
    // Conditions:
    //  i.  Instruction 'I' has opcode allowed for the trunk. Please note that
    //      we don't use Tree::isAllowedTrunkInstruction directly since we don't
    //      have an instance of a Tree yet.
    //  ii. Instruction 'I' doesn't belong to another tree.
    return (isAddSubInstr(I, DL) ||
            ((isFAddSubInstr(I, DL) || isFMulDivInstr(I, DL)) &&
             cast<FPMathOperator>(I)->getFastMathFlags().isFast())) &&
           !findEnclosingTree(AllTrees, I);
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

    TreePtr UTree = std::make_unique<Tree>(DL);
    Tree *T = UTree.get();
    T->setRoot(&I);
    growTree(AllTrees, T,
             SmallVector<CanonNode, 8>(
                 {CanonNode(&I, getPositiveOpcode(I.getOpcode()))}));

    assert(1 <= T->size() && T->size() <= MaxTreeSize + 2 &&
           "Tree size should be capped");

    // Skip trivial trees.
    if (T->size() > 1)
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
        GroupTreesVecTy BestGroups;
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
          auto AssocInstCnt = G.getAssocInstrCnt();
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

        removeOldTrees(AllAffectedTrees.getArrayRef());
        removeGroupFromTree(BestGroups);

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
