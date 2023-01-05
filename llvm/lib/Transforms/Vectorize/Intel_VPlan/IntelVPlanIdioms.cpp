//===- IntelVPlanIdioms.h -------------------------------------------------===//
//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements idioms recognition that are specific for vectorization.
//===----------------------------------------------------------------------===//

#include "IntelVPlanIdioms.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLIf.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Support/CommandLine.h"

#if INTEL_FEATURE_SW_DTRANS
#include "Intel_DTrans/Transforms/PaddedPointerPropagation.h"
#endif // INTEL_FEATURE_SW_DTRANS

#define DEBUG_TYPE "vplan-idioms"

cl::opt<bool>
    AllowMemorySpeculation("allow-memory-speculation", cl::init(false),
                           cl::desc("Enable speculative vector unit loads."));

#if INTEL_FEATURE_SW_DTRANS
static cl::opt<bool>
    UsePaddingInformation("vplan-use-padding-info", cl::init(true),
                          cl::desc("Enable use of IPO's padding information"));
#endif // INTEL_FEATURE_SW_DTRANS

namespace llvm {

using namespace loopopt;

namespace vpo {

static bool canSpeculate(const RegDDRef *Ref, bool CheckPadding = true) {
  // FIXME: Unit-stride check is needed.
  if (Ref->isTerminalRef())
    return true;
  if (Ref->isMemRef()) {
    // FIXME: Copied code from CM. Has to be removed.
    int64_t ConstStride;
    const HLLoop *Loop = Ref->getParentLoop();
    assert(Loop && "Expected the RegDDRef to have a valid parent-loop");
    unsigned NestingLevel = Loop->getNestingLevel();
    if (!Ref->getConstStrideAtLevel(NestingLevel, &ConstStride) || !ConstStride)
      return false;

    // Compute stride in terms of number of elements
    auto DL = Ref->getDDRefUtils().getDataLayout();
    auto RefSizeInBytes = DL.getTypeSizeInBits(Ref->getDestType()) >> 3;
    ConstStride /= RefSizeInBytes;
    if (ConstStride != 1)
      return false;

    // If it is known that padding is not needed, then return early after unit
    // stride tests. This is applicable for search loop idioms which require
    // explicit peel loops for aligning the memory accesses.
    if (!CheckPadding)
      return true;

#if INTEL_FEATURE_SW_DTRANS
    if (UsePaddingInformation)
      if (Value *Base = Ref->getTempBaseValue()) {
        // TODO: Need to look into data type, VF and return padded size.
        if (llvm::getPaddingForValue(Base) > 0)
          return true;
        else
          for (auto NodeIt = Loop->pre_begin(), NodeEndIt = Loop->pre_end();
               NodeIt != NodeEndIt; ++NodeIt)
            if (const auto *Inst = dyn_cast<HLInst>(&*NodeIt))
              if (Inst->getLLVMInstruction() == Base) {
                const RegDDRef *RvalRef = Inst->getRvalDDRef();
                if (RvalRef->isAddressOf()) {
                  Base = RvalRef->getTempBaseValue();
                  if (Base && llvm::getPaddingForValue(Base) > 0)
                    return true;
                }
              }
      }
#endif // INTEL_FEATURE_SW_DTRANS

    if (AllowMemorySpeculation)
      return true;
  }
  return false;
}

// Expect following sequence of instructions/blocks
//  latch->getSinglePredecessor():
//    BlockPredicate
//    add
//    cmp
//    not
//
//  latch:
//    and
bool VPlanIdioms::isSafeLatchBlockForSearchLoop(const VPBasicBlock *Block) {
  auto *SinglePred = dyn_cast_or_null<VPBasicBlock>(Block->getSinglePredecessor());
  if (!SinglePred)
    return false;

  SmallVector<const VPInstruction *, 1> Insts(
      map_range(*Block, [](const VPInstruction &Inst) { return &Inst; }));
  if (Insts.size() != 2)
    return false;
  if (Insts[0]->getOpcode() != Instruction::And)
    return false;
  if (Insts[1]->getOpcode() != Instruction::Br)
    return false;

  SmallVector<const VPInstruction *, 4> PredInsts(
      map_range(*SinglePred, [](const VPInstruction &Inst) { return &Inst; }));
  if (PredInsts.size() != 5)
    return false;
  if (PredInsts[0]->getOpcode() != VPInstruction::Pred)
    return false;
  if (PredInsts[1]->getOpcode() != Instruction::Add)
    return false;
  if (PredInsts[2]->getOpcode() != Instruction::ICmp)
    return false;
  if (PredInsts[3]->getOpcode() != VPInstruction::Not)
    return false;
  if (PredInsts[4]->getOpcode() != Instruction::Br)
    return false;

  return true;
}

/// Recognize following pattern in the Header:
///   %1 = i1 + ...;    // rhs is a linear RegDDRef. This instruction is
///                     // optional.
///   < set of similar instructions >
///   if (a[i1] != b[i1])   // a[i1] and b[i1] can be executed speculatively.
///
VPlanIdioms::Opcode
VPlanIdioms::isStrEqSearchLoop(const VPBasicBlock *Block,
                               const bool AllowSpeculation) {
  bool HasIf = false;

  for (const VPInstruction &InstRef : *Block) {
    const auto Inst = cast<const VPInstruction>(&InstRef);

    if (isa<const VPBranchInst>(Inst) ||
        (Inst->HIR().isDecomposed() && Inst->isUnderlyingIRValid()))
      continue;

    if (Inst->getOpcode() == VPInstruction::Not)
      continue;

    // Declare loop as unsafe if any instruction does not have underlying IR.
    if (!Inst->HIR().getUnderlyingNode())
      return VPlanIdioms::Unsafe;

    // FIXME: Without proper decomposition we have to parse predicates of
    // underlying IR.
    // Ideally, it's enough to visit all BBs and check safety for loads and
    // stores. No need to do that for:
    //    1) non-trivial exit BBs
    //    2) Any BB that postdominate any exiting node.
    // in both case exit mask is known and these operations can be done safely.
    const HLDDNode *DDNode = cast<HLDDNode>(Inst->HIR().getUnderlyingNode());
    if (const auto If = dyn_cast<const HLIf>(DDNode)) {
      unsigned NumPredicates = 0;
      HasIf = true;

      for (auto I = If->pred_begin(), E = If->pred_end(); I != E; ++I) {
        const RegDDRef *LhsRef = If->getLHSPredicateOperandDDRef(I);
        const RegDDRef *RhsRef = If->getRHSPredicateOperandDDRef(I);
        if (*I != PredicateTy::ICMP_NE) {
          LLVM_DEBUG(dbgs() << "        PredicateTy " << *I;
                     dbgs() << " is unsafe.\n");
          return VPlanIdioms::Unsafe;
        }
        if (!canSpeculate(LhsRef) ||
            LhsRef->getDestType()->getScalarSizeInBits() != 8) {
          LLVM_DEBUG(dbgs() << "        RegDDRef "; LhsRef->dump();
                     dbgs() << " is unsafe.\n");
          return VPlanIdioms::Unsafe;
        }
        if (!canSpeculate(RhsRef) ||
            RhsRef->getDestType()->getScalarSizeInBits() != 8) {
          LLVM_DEBUG(dbgs() << "        RegDDRef "; RhsRef->dump();
                     dbgs() << " is unsafe.\n");
          return VPlanIdioms::Unsafe;
        }
        ++NumPredicates;
      }
      if (NumPredicates != 1) {
        LLVM_DEBUG(dbgs() << "        Only one predicate is expected\n.");
        return VPlanIdioms::Unsafe;
      }
    }
    // Insure that any other instruction are safe to speculate. Currently, look
    // for simple conditional scalar assignment of linear variable.
    else if (const auto HInst = dyn_cast<const HLInst>(DDNode)) {
      const RegDDRef *LvalRef = HInst->getLvalDDRef();
      if (LvalRef && LvalRef->isMemRef() && !AllowSpeculation) {
        LLVM_DEBUG(dbgs() << "        RegDDRef "; LvalRef->dump();
                   dbgs() << " is unsafe.\n");
        return VPlanIdioms::Unsafe;
      }
      const RegDDRef *RvalRef = HInst->getRvalDDRef();
      // FIXME: Ideally we have to dig into RvalRef and analyse each RegDDRef
      // there.
      // Currently limits this only for single linear or memref in rhs.
      if (!RvalRef)
        return VPlanIdioms::Unsafe;

      if (RvalRef->isTerminalRef() && RvalRef->isNonLinear()) {
        LLVM_DEBUG(dbgs() << "        RegDDRef "; RvalRef->dump();
                   dbgs() << " is unsafe.\n");
        return VPlanIdioms::Unsafe;
      }

      // FIXME: Need to look on VPPredicates, not on underlying IR.
      if (!isa<HLIf>(HInst->getParent())) {
        if (LvalRef && !canSpeculate(LvalRef)) {
          LLVM_DEBUG(dbgs() << "        HLInst "; HInst->dump();
                     dbgs() << " is unmasked, thus it's unsafe.\n");
          return VPlanIdioms::Unsafe;
        }
        // FIXME: Current CG cannot handle multiple dimensions for
        // live-out computation in early exit loop. Bail-out by now.
        if (!RvalRef->isMemRef() || !canSpeculate(RvalRef)) {
          LLVM_DEBUG(dbgs() << "        HLInst "; HInst->dump();
                     dbgs() << " is unmasked, thus it's unsafe.\n");
          return VPlanIdioms::Unsafe;
        } else if (!RvalRef->isSingleDimension()) {
          LLVM_DEBUG(dbgs() << "        RvalRef "; RvalRef->dump();
                     dbgs() << "  has multiple dimensions which is not "
                               "supported by current CG.\n");
          return VPlanIdioms::Unsafe;
        }
      }
    }
  }
  return HasIf ? VPlanIdioms::SearchLoopStrEq : VPlanIdioms::Unknown;
}

// Check if given \p Ref is a loop-invariant address ref.
static bool isLoopInvariantAddressRef(const RegDDRef *Ref) {
  if (!Ref->isAddressOf())
    return false;

  unsigned NestingLevel = Ref->getParentLoop()->getNestingLevel();
  return Ref->isStructurallyInvariantAtLevel(NestingLevel);
}

/// Recognize search loop idioms that require peeling for correctness.
/// Currently we have two such idioms:  PtrEq and ValueCmp.
///
/// For the PtrEq idiom, recognize the following pattern in the Header:
///   if (a[i1] == &b[x])   // a[i1] can be executed speculatively and b[x] is
///                         // loop-invariant.
///
/// If the above predicate is found in the loop header, then ensure the loop
/// contains only the predicate along with optional live-out instructions to
/// capture the found result. Effectively we try to recognize a needle-
/// in-haystack search loop where an array of pointers is searched.
///
///  + DO i1 = 0, UB, 1 <DO_MULTI_EXIT_LOOP>
///  |   if ((%a)[i1] == &((%b)[0]))
///  |   {
///  |      %needle = &((%a)[i1]);
///  |      goto needle_found;
///  |   }
///  + END LOOP
///
/// For the ValueCmp idiom, recognize the following pattern in the Header:
///   if (a[i1] <cmpop> b)  // a[i1] can be executed speculatively, <cmpop>
///                         // is any reasonable comparison operator, and b
///                         // is loop-invariant.
///
/// If the above predicate is found in the loop header, then ensure that the
/// loop contains only the predicate along with optional live-out instructions
/// to capture the identified loop index.
///
///  + DO i1 = 0, UB, 1 <DO_MULTI_EXIT_LOOP>
///  |   if ((%a)[i1] <cmpop> %b)   (*)
///  |   {
///  |      %index = i1;
///  |      goto index_found;
///  |   }
///  + END LOOP
///
/// (*) Note that the DDRef could have more dimensions, e.g., (%a)[0][i1].
VPlanIdioms::Opcode
VPlanIdioms::isSearchLoopNeedingPeeling(const VPBasicBlock *Block,
                                        const bool AllowMemorySpeculation,
                                        RegDDRef *&PeelArrayRef,
                                        const VPlanIdioms::Opcode Idiom) {
  bool CheckPtrEq = Idiom == VPlanIdioms::SearchLoopPtrEq;
  bool CheckValueCmp = Idiom == VPlanIdioms::SearchLoopValueCmp;
  assert((CheckPtrEq || CheckValueCmp)
         && "Wrong idiom for peeled search loop");

  // Item that is found in the list being searched.
  const RegDDRef *ListItemRef = nullptr;

  // For ValueCmp, index to be returned.
  const CanonExpr *PredLhsIndex = nullptr;

  for (const VPInstruction &InstRef : *Block) {
    const auto Inst = cast<const VPInstruction>(&InstRef);

    if (isa<const VPBranchInst>(Inst) ||
        (Inst->HIR().isDecomposed() && Inst->isUnderlyingIRValid()))
      continue;

    if (Inst->getOpcode() == VPInstruction::Not)
      continue;

    // Declare loop as unsafe if any instruction does not have underlying IR.
    if (!Inst->HIR().getUnderlyingNode())
      return VPlanIdioms::Unsafe;

    // FIXME: Without proper decomposition we have to parse predicates of
    // underlying IR
    const HLDDNode *DDNode = cast<HLDDNode>(Inst->HIR().getUnderlyingNode());

    // Only the if-block is expected in the search loop. Other allowed
    // VPInstructions in the loop Header are loop IV related instructions which
    // have HLLoop as underlying DDNode.
    if (isa<HLLoop>(DDNode))
      continue;

    auto *If = dyn_cast<HLIf>(DDNode);
    if (!If) {
      LLVM_DEBUG(
          dbgs() << "        Search loop expected to have an HLIf node.\n");
      return VPlanIdioms::Unsafe;
    }

    unsigned NumPredicates = If->getNumPredicates();

    if (If->getNextNode() || If->getPrevNode()) {
      LLVM_DEBUG(
          dbgs() << "        Search is expected to have only an HLIf node\n.");
      return VPlanIdioms::Unsafe;
    }

    if (NumPredicates != 1) {
      LLVM_DEBUG(dbgs() << "        Only one predicate is expected\n.");
      return VPlanIdioms::Unsafe;
    }

    if (If->hasElseChildren()) {
      LLVM_DEBUG(dbgs() << "        Search loop is expected to have HLIf node "
                           "without else children\n.");
      return VPlanIdioms::Unsafe;
    }

    auto PredIt = If->pred_begin();

    if ((CheckPtrEq && *PredIt != PredicateTy::ICMP_EQ)
        || (CheckValueCmp && (*PredIt == PredicateTy::FCMP_FALSE
                              || *PredIt == PredicateTy::FCMP_ORD
                              || *PredIt == PredicateTy::FCMP_UNO
                              || *PredIt == PredicateTy::FCMP_TRUE))) {
      LLVM_DEBUG(dbgs() << "        PredicateTy " << *PredIt;
                 dbgs() << " is unsafe.\n");
      return VPlanIdioms::Unsafe;
    }

    const RegDDRef *PredLhs = If->getLHSPredicateOperandDDRef(PredIt);
    Type *PredLhsType = PredLhs->getDestType();
    const RegDDRef *PredRhs = If->getRHSPredicateOperandDDRef(PredIt);

    LLVM_DEBUG(dbgs() << "PtrEq: Pred:" << *PredIt << "\nPredLhs:";
               PredLhs->dump(true); dbgs() << "\nPredRhs:"; PredRhs->dump(true);
               dbgs() << "\n");

    if (!canSpeculate(PredLhs, false /*CheckPadding*/) ||
        (CheckPtrEq && !PredLhsType->isPointerTy())) {
      LLVM_DEBUG(dbgs() << "        RegDDRef "; PredLhs->dump();
                 dbgs() << " is unsafe.\n");
      return VPlanIdioms::Unsafe;
    }
    // LHS of predicate matched, save for later checks.
    ListItemRef = PredLhs;

    const HLLoop *IfParLoop = If->getParentLoop();
    assert(IfParLoop && "Expected the HLIf to have a valid parent-loop");
    HLNodeUtils &HNU = IfParLoop->getHLNodeUtils();
    CanonExprUtils &CEU = HNU.getCanonExprUtils();
    const RegDDRef *LoopUB = IfParLoop->getUpperDDRef();
    assert(LoopUB && "Cannot find UB DDRef of loop.");
    unsigned LoopUBTypeSize = CEU.getTypeSizeInBytes(LoopUB->getDestType());

    if (CheckPtrEq) {
      // Check that struct pointer size is same as loop bound's size.
      // TODO: It would be good to relax this at some point.  See the TODO
      // in HLLoop::generatePeelLoop().  We can miss opportunities when
      // the pointer type is i64 and the LoopUB is i32, for example.
      unsigned PtrSize = CEU.getTypeSizeInBytes(PredLhsType);

      if (PtrSize != LoopUBTypeSize) {
        LLVM_DEBUG(dbgs() << "        Array ref to peel "; ListItemRef->dump();
                   dbgs() << " is not same size as loop bounds.\n");
        return VPlanIdioms::Unsafe;
      }

      if (!isLoopInvariantAddressRef(PredRhs)) {
        // TODO: Should we check that the struct type match exactly?
        LLVM_DEBUG(dbgs() << "        RegDDRef "; PredRhs->dump();
                   dbgs() << " is unsafe.\n");
        return VPlanIdioms::Unsafe;
      }

    } else {

      if (!PredLhs->isMemRef()) {
        LLVM_DEBUG(dbgs() << "        RegDDRef "; PredLhs->dump();
                   dbgs() << " is not an array access.\n");
        return VPlanIdioms::Unsafe;
      }

      // Check that the size of a pointer to an array element is the same
      // as the loop bound's size, when the loop bound isn't constant.
      // TODO: It would be good to relax this at some point.  See the TODO
      // in HLLoop::generatePeelLoop().  We can miss opportunities when
      // the pointer type is i64 and the LoopUB is i32, for example.
      if (!LoopUB->isIntConstant()) {
        const CanonExpr *PeelArrayBase = ListItemRef->getBaseCE();
        if (CEU.getTypeSizeInBytes(PeelArrayBase->getSrcType())
            != LoopUBTypeSize) {
          LLVM_DEBUG(dbgs() << "        Array ref to peel ";
                     ListItemRef->dump();
                     dbgs() << " is not same size as loop bounds.\n");
          return VPlanIdioms::Unsafe;
        }
      }

      // The rightmost index is the expected search value.
      PredLhsIndex = PredLhs->getDimensionIndex(1);

      unsigned NestingLevel = If->getParentLoop()->getNestingLevel();
      if (!PredRhs->isStructurallyInvariantAtLevel(NestingLevel)) {
        LLVM_DEBUG(dbgs() << "        Comparand "; PredRhs->dump();
                   dbgs() << " is unsafe.\n");
        return VPlanIdioms::Unsafe;
      }
    }


    // Predicate checks passed, now check for nodes in then branch of HLIf.
    if (!checkThenNodes(If, ListItemRef, PredLhsIndex, Idiom)) {
      LLVM_DEBUG(
          dbgs() << "        Unsafe instructions in then branch of HLIf.\n");
      return VPlanIdioms::Unsafe;
    }
  }

  if (!ListItemRef) {
    LLVM_DEBUG(
        dbgs() << "        List item for search loop idiom not found.\n");
    return VPlanIdioms::Unsafe;
  }
  // All checks passed, idiom is recognized.
  PeelArrayRef = const_cast<RegDDRef *>(ListItemRef);
  return Idiom;
}

/// Checks if the nodes in the then-branch of \p If match the expected
/// idiom (either PtrEq or ValueCmp).  For PtrEq, the then-branch should
/// be of the form:
///  |   {
///  |      %needle = &((%a)[i1]);
///  |      goto needle_found;
///  |   }
/// For ValueCmp, the then-branch should be of the form:
///  |   {
///  |      %index = i1;
///  |      goto index_found;
///  |   }
/// In both cases, i1 is the index of the search loop.
bool VPlanIdioms::checkThenNodes(const HLIf *If,
                                 const RegDDRef *ListItemRef,
                                 const CanonExpr *LastIndex,
                                 const VPlanIdioms::Opcode Idiom) {

  bool CheckPtrEq = Idiom == VPlanIdioms::SearchLoopPtrEq;
  bool CheckValueCmp = Idiom == VPlanIdioms::SearchLoopValueCmp;
  bool ListItemOrLastIndexCaptured = false;

  for (const HLNode &ThenNode : make_range(If->then_begin(), If->then_end())) {
    if (const auto HInst = dyn_cast<const HLInst>(&ThenNode)) {
      if (ListItemOrLastIndexCaptured) {
        LLVM_DEBUG(dbgs() << "        List item or last index was already "
                   "captured. More instructions in then-branch are unsafe.\n");
        return false;
      }

      // Restrict allowed HLInsts to copy instructions, or GEPs for PtrEq.
      auto UnderlyingInst = HInst->getLLVMInstruction();
      if (!(HInst->isCopyInst()
            || (CheckPtrEq && isa<GetElementPtrInst>(UnderlyingInst)))) {
        LLVM_DEBUG(dbgs() << "        HLInst "; HInst->dump();
                   dbgs() << " is unsafe.\n");
        return false;
      }

      const RegDDRef *LvalRef = HInst->getLvalDDRef();
      if (!LvalRef->isTerminalRef()) {
        LLVM_DEBUG(dbgs() << "        RegDDRef "; LvalRef->dump();
                   dbgs() << " is unsafe.\n");
        return false;
      }

      const RegDDRef *RvalRef = HInst->getRvalDDRef();
      if (!((CheckPtrEq && RvalRef->isAddressOf())
            || (CheckValueCmp && RvalRef->isTerminalRef()))) {
        LLVM_DEBUG(dbgs() << "        RegDDRef "; RvalRef->dump();
                   dbgs() << " is unsafe.\n");
        return false;
      }

      if (CheckPtrEq) {
        if (!DDRefUtils::areEqualWithoutAddressOf(RvalRef,
                                                  ListItemRef)) {
          LLVM_DEBUG(dbgs() << "        RegDDRef "; RvalRef->dump();
                     dbgs() << " is unsafe.\n");
          return false;
        }
      } else {
        const CanonExpr *CE = RvalRef->getSingleCanonExpr();
        if (!CanonExprUtils::areEqual(CE, LastIndex)) {
          LLVM_DEBUG(dbgs() << "        RegDDRef "; RvalRef->dump();
                     dbgs() << " is unsafe.\n");
          return false;
        }
      }

      // The found list item or last index is captured inside the if-block.
      ListItemOrLastIndexCaptured = true;

    } else if (!isa<HLGoto>(&ThenNode)) {
      LLVM_DEBUG(dbgs() << "        Node "; ThenNode.dump();
                 dbgs() << " is unexpected.\n");
      return false;
    } else if (!ListItemOrLastIndexCaptured) {
      LLVM_DEBUG(
          dbgs() << "        Found list item or last index is not "
          "captured inside loop.\n");
      return false;
    }
  }

  // All safety checks passed.
  return true;
}

// Check that all VPInstructions in non-trivial exit block are supported.
// This function is more important for what CG can handle, rather then for
// vectorization legality.
bool VPlanIdioms::isSafeExitBlockForSearchLoop(const VPBasicBlock *Block) {
  for (const VPInstruction &VPInst : Block->getNonPredicateInstructions()) {
    // Ignore instruction used for setting up uniform inner loop control flow.
    if (VPInst.getOpcode() == VPInstruction::AllZeroCheck)
      continue;

    if (isa<const VPBranchInst>(VPInst) ||
        (VPInst.HIR().isDecomposed() && VPInst.isUnderlyingIRValid()))
      continue;

    const HLDDNode *DDNode = cast<HLDDNode>(VPInst.HIR().getUnderlyingNode());
    if (const auto HInst = dyn_cast<const HLInst>(DDNode)) {
      const RegDDRef *LvalRef = HInst->getLvalDDRef();
      const RegDDRef *RvalRef = HInst->getRvalDDRef();
      if (!LvalRef || !RvalRef ||
          (!LvalRef->isTerminalRef() || RvalRef->isNonLinear() ||
           !RvalRef->isSingleDimension())) {
        LLVM_DEBUG(dbgs() << "      HLInst "; HInst->dump();
                   dbgs() << " is not supported by CG.\n");
        return false;
      }
    }
    else
      return false;
  }
  return true;
}

VPlanIdioms::Opcode VPlanIdioms::isSearchLoop(const VPlanVector *Plan,
                                              const bool CheckSafety,
                                              RegDDRef *&PeelArrayRef) {
  // TODO: With explicit representation of peel loop, next code is not valid
  // and has to be changed. Also, any newly created VPInstruction in preheader
  // could probably require bailout too. Seems to work for now though, and
  // should be heavily refactored soon enough to be moved from cost modeling
  // stage to early vectorizer transforms.

  if (Plan->getVPLoopInfo()->size() != 1) {
    LLVM_DEBUG(dbgs() << "    Search loop was NOT recognized.\n");
    return VPlanIdioms::Unknown;
  }

  // For the search loop idiom we expect 1-2 exit blocks and two exiting block.
  const VPLoop *VPL = Plan->getMainLoop(true);
  SmallVector<VPBasicBlock *, 8> Exitings, Exits;
  VPL->getExitingBlocks(Exitings);
  VPL->getExitBlocks(Exits);

  if (Exitings.size() != 2 || Exits.size() > 2) {
    LLVM_DEBUG(dbgs() << "    Search loop was NOT recognized.\n");
    return VPlanIdioms::Unknown;
  }

  SmallDenseSet<const VPBasicBlock *> IgnoreBlocks;
  const VPBasicBlock *Latch = VPL->getLoopLatch();
  assert(Latch && "VPLoop does not have loop latch block.");
  if (!isSafeLatchBlockForSearchLoop(Latch)) {
    LLVM_DEBUG(dbgs() << "    Search loop is unsafe.\n");
    return VPlanIdioms::Unsafe;
  }
  IgnoreBlocks.insert(Latch);
  IgnoreBlocks.insert(Latch->getSinglePredecessor());

  for (const VPBasicBlock *Exit : Exits) {
    if (!isSafeExitBlockForSearchLoop(Exit)) {
      LLVM_DEBUG(dbgs() << "    Search loop is unsafe.n\n");
      return VPlanIdioms::Unsafe;
    }
    IgnoreBlocks.insert(Exit);
  }

  const VPBasicBlock *Header = VPL->getHeader();
  // Recognize specific patterns only for the header of the loop. All other
  // blocks will (except Exit block) will be treated unsafe.
  VPlanIdioms::Opcode Opcode = isStrEqSearchLoop(Header, false);
  // TODO: there're also few idiomatic search loops that have to be covered
  // here.
  if (Opcode != VPlanIdioms::SearchLoopStrEq) {
    // Array being searched for if current search loop matches PtrEq
    // or ValueCmp idiom.
    RegDDRef *ArrayRef = nullptr;
    Opcode = isSearchLoopNeedingPeeling(Header, false, ArrayRef,
                                        VPlanIdioms::SearchLoopPtrEq);
    if (Opcode == VPlanIdioms::SearchLoopPtrEq) {
      // PtrEq was recognized, ArrayRef cannot be null
      assert(ArrayRef && "PtrEq loop does not have PeelArrayRef.\n");
      LLVM_DEBUG(dbgs() << "    PtrEq loop has PeelArray:";
                 ArrayRef->dump(); dbgs() << "\n");
      PeelArrayRef = ArrayRef;
    } else {
      ArrayRef = nullptr;
      Opcode = isSearchLoopNeedingPeeling(Header, false, ArrayRef,
                                    VPlanIdioms::SearchLoopValueCmp);
      if (Opcode == VPlanIdioms::SearchLoopValueCmp) {
        // ValueCmp was recognized, ArrayRef cannot be null
        assert(ArrayRef && "ValueCmp loop does not have PeelArrayRef.\n");
        LLVM_DEBUG(dbgs() << "    ValueCmp loop has PeelArray:";
                   ArrayRef->dump(); dbgs() << "\n");
        PeelArrayRef = ArrayRef;
      } else {
        LLVM_DEBUG(dbgs() << "    Search loop idiom was not recognized.\n");
        return VPlanIdioms::Unsafe;
      }
    }
  }
  IgnoreBlocks.insert(Header);

  if (const VPBasicBlock *Succ = Header->getSingleSuccessor()) {
    // Search loop idiom expects the successor block to be empty.
    if (!Succ->empty()) {
      LLVM_DEBUG(dbgs() << "    Search loop is unsafe.\n");
      return VPlanIdioms::Unsafe;
    }
    IgnoreBlocks.insert(Succ);
  }

  ReversePostOrderTraversal<const VPBasicBlock *> RPOT(Header);
  // Visit the VPBlocks connected to "this", starting from it.
  for (const VPBasicBlock *Block : RPOT) {
    // Blocks outside of the loop are safe to execute. Latch and Header blocks
    // were already visited.
    // Every other block is assumed to be unsafe for search loop vectorization.
    if (IgnoreBlocks.count(Block) || !VPL->contains(Block))
      continue;

    LLVM_DEBUG(dbgs() << "        " << Block->getName() << " is unsafe\n");
    LLVM_DEBUG(dbgs() << "    Search loop is unsafe.\n");
    return VPlanIdioms::Unsafe;
  }


  LLVM_DEBUG(dbgs() << "    Search loop was recognized.\n");

  return Opcode;
}

bool VPlanIdioms::isAnySearchLoop(const VPlanVector *Plan,
                                  const bool CheckSafety) {
  RegDDRef *PeelArrayRef = nullptr;
  return isAnySearchLoop(isSearchLoop(Plan, CheckSafety, PeelArrayRef));
}

} // namespace vpo

} // namespace llvm
