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

#if INTEL_INCLUDE_DTRANS
#include "Intel_DTrans/Transforms/PaddedPointerPropagation.h"
#endif // INTEL_INCLUDE_DTRANS

#define DEBUG_TYPE "vplan-idioms"

cl::opt<bool>
    AllowMemorySpeculation("allow-memory-speculation", cl::init(false),
                           cl::desc("Enable speculative vector unit loads."));

static cl::opt<bool>
    UsePaddingInformation("vplan-use-padding-info", cl::init(true),
                          cl::desc("Enable use of IPO's padding information"));

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

#if INTEL_INCLUDE_DTRANS
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
#endif // INTEL_INCLUDE_DTRANS

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
      map_range(Block->vpinstructions(),
                [](const VPInstruction &Inst) { return &Inst; }));
  if (Insts.size() != 1)
    return false;
  if (Insts[0]->getOpcode() != Instruction::And)
    return false;

  SmallVector<const VPInstruction *, 4> PredInsts(
      map_range(SinglePred->vpinstructions(),
                [](const VPInstruction &Inst) { return &Inst; }));
  if (PredInsts.size() != 4)
    return false;
  if (PredInsts[0]->getOpcode() != VPInstruction::Pred)
    return false;
  if (PredInsts[1]->getOpcode() != Instruction::Add)
    return false;
  if (PredInsts[2]->getOpcode() != Instruction::ICmp)
    return false;
  if (PredInsts[3]->getOpcode() != VPInstruction::Not)
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

  for (const VPRecipeBase &Recipe : Block->getRecipes()) {
    if (!isa<const VPInstruction>(&Recipe))
      continue;
    const auto Inst = cast<const VPInstruction>(&Recipe);

    if (isa<const VPBranchInst>(Inst) ||
        (Inst->HIR.isDecomposed() && Inst->isUnderlyingIRValid()))
      continue;

    if (Inst->getOpcode() == VPInstruction::Not)
      continue;

    // FIXME: Without proper decomposition we have to parse predicates of
    // underlying IR.
    // Ideally, it's enough to visit all BBs and check safety for loads and
    // stores. No need to do that for:
    //    1) non-trivial exit BBs
    //    2) Any BB that postdominate any exiting node.
    // in both case exit mask is known and these operations can be done safely.
    const HLDDNode *DDNode = cast<HLDDNode>(Inst->HIR.getUnderlyingNode());
    if (const auto If = dyn_cast<const HLIf>(DDNode)) {
      unsigned NumPredicates = 0;
      HasIf = true;

      for (auto I = If->pred_begin(), E = If->pred_end(); I != E; ++I) {
        const RegDDRef *LhsRef = If->getPredicateOperandDDRef(I, true);
        const RegDDRef *RhsRef = If->getPredicateOperandDDRef(I, false);
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
        } else if (RvalRef->getNumDimensions() != 1) {
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

/// Recognize following pattern in the Header:
///   if (a[i1] == &b[x])   // a[i1] can be executed speculatively and b[x] is
///                         // loop-invariant.
///
/// If the above predicate is found in the loop header, then ensure that loop
/// contains only the predicate along with optional  live-out instructions to
/// capture the found result. Effectively we try to recognize a need-in-haystack
/// search loop where an array of struct pointers is searched.
///
///  + DO i1 = 0, UB, 1 <DO_MULTI_EXIT_LOOP>
///  |   if ((%a)[i1] == &((%b)[0]))
///  |   {
///  |      %needle = &((%a)[i1]);
///  |      goto needle_found;
///  |   }
///  + END LOOP
///
VPlanIdioms::Opcode
VPlanIdioms::isStructPtrEqSearchLoop(const VPBasicBlock *Block,
                                     const bool AllowMemorySpeculation,
                                     RegDDRef *&PeelArrayRef) {
  // Item that is found in the list being searched.
  const RegDDRef *ListItemRef = nullptr;

  for (const VPRecipeBase &Recipe : Block->getRecipes()) {
    if (!isa<const VPInstruction>(&Recipe))
      continue;
    const auto Inst = cast<const VPInstruction>(&Recipe);

    if (isa<const VPBranchInst>(Inst) ||
        (Inst->HIR.isDecomposed() && Inst->isUnderlyingIRValid()))
      continue;

    if (Inst->getOpcode() == VPInstruction::Not)
      continue;

    // FIXME: Without proper decomposition we have to parse predicates of
    // underlying IR
    const HLDDNode *DDNode = cast<HLDDNode>(Inst->HIR.getUnderlyingNode());

    // Only the if-block is expected in the search loop. Other allowed
    // VPInstructions in the loop Header are loop IV related instructions which
    // have HLLoop as underlying DDNode.
    if (isa<HLLoop>(DDNode))
      continue;

    auto *If = dyn_cast<HLIf>(DDNode);
    if (!If) {
      LLVM_DEBUG(
          dbgs() << "        Search loop expected to have only HLIf node.\n");
      return VPlanIdioms::Unsafe;
    }

    unsigned NumPredicates = If->getNumPredicates();

    if (If->getNextNode() || If->getPrevNode()) {
      LLVM_DEBUG(
          dbgs() << "        Search is expected to have HLIf node only\n.");
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

    if (*PredIt != PredicateTy::ICMP_EQ) {
      LLVM_DEBUG(dbgs() << "        PredicateTy " << *PredIt;
                 dbgs() << " is unsafe.\n");
      return VPlanIdioms::Unsafe;
    }

    const RegDDRef *PredLhs = If->getPredicateOperandDDRef(PredIt, true);
    Type *PredLhsType = PredLhs->getDestType();
    const RegDDRef *PredRhs = If->getPredicateOperandDDRef(PredIt, false);

    LLVM_DEBUG(dbgs() << "StructPtrEq: Pred:" << *PredIt << "\nPredLhs:";
               PredLhs->dump(true); dbgs() << "\nPredRhs:"; PredRhs->dump(true);
               dbgs() << "\n");

    if (!canSpeculate(PredLhs, false /*CheckPadding*/) ||
        !PredLhsType->isPointerTy() ||
        !PredLhsType->getPointerElementType()->isStructTy()) {
      LLVM_DEBUG(dbgs() << "        RegDDRef "; PredLhs->dump();
                 dbgs() << " is unsafe.\n");
      return VPlanIdioms::Unsafe;
    }
    // LHS of predicate matched, save for later checks.
    ListItemRef = PredLhs;

    // Check that struct pointer size is same as loop bound's size.
    HLNodeUtils &HNU = If->getParentLoop()->getHLNodeUtils();
    CanonExprUtils &CEU = HNU.getCanonExprUtils();
    unsigned PtrSize = CEU.getTypeSizeInBytes(PredLhsType);
    const RegDDRef *LoopUB = If->getParentLoop()->getUpperDDRef();
    assert(LoopUB && "Cannot find UB DDRef of loop.");
    unsigned LoopUBTypeSize = CEU.getTypeSizeInBytes(LoopUB->getDestType());

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

    // Predicate checks passed, now check for nodes in then branch of HLIf.
    if (!checkStructPtrEqThenNodes(If, ListItemRef)) {
      LLVM_DEBUG(
          dbgs() << "        Unsafe instructions in then branch of HLIf.\n");
      return VPlanIdioms::Unsafe;
    }
  }

  // All checks passed, idiom is recognized.
  assert(ListItemRef && "List item for search loop idiom not found.\n");
  PeelArrayRef = const_cast<RegDDRef *>(ListItemRef);
  return VPlanIdioms::SearchLoopStructPtrEq;
}

/// Checks if the nodes in the then-branch of \p If match the StructPtrEq idiom.
bool VPlanIdioms::checkStructPtrEqThenNodes(const HLIf *If,
                                            const RegDDRef *ListItemRef) {
  bool ListItemCaptured = false;
  for (const HLNode &ThenNode : make_range(If->then_begin(), If->then_end())) {
    if (const auto HInst = dyn_cast<const HLInst>(&ThenNode)) {
      if (ListItemCaptured) {
        LLVM_DEBUG(dbgs() << "        List item was already captured. More "
                             "instructions in then-branch are unsafe.\n");
        return false;
      }

      // Restrict allowed HLInsts to copy instructions or GEPs.
      auto UnderlyingInst = HInst->getLLVMInstruction();
      if (!HInst->isCopyInst() && !isa<GetElementPtrInst>(UnderlyingInst)) {
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
      if (!RvalRef->isAddressOf()) {
        LLVM_DEBUG(dbgs() << "        RegDDRef "; RvalRef->dump();
                   dbgs() << " is unsafe.\n");
        return false;
      }

      if (!RvalRef->getDDRefUtils().areEqualWithoutAddressOf(RvalRef,
                                                             ListItemRef)) {
        LLVM_DEBUG(dbgs() << "        RegDDRef "; RvalRef->dump();
                   dbgs() << " is unsafe.\n");
        return false;
      }

      // The found list item is captured inside the if-block.
      ListItemCaptured = true;

    } else if (!isa<HLGoto>(&ThenNode)) {
      LLVM_DEBUG(dbgs() << "        Node "; ThenNode.dump();
                 dbgs() << " is unexpected.\n");
      return false;
    } else if (!ListItemCaptured) {
      LLVM_DEBUG(
          dbgs() << "        Found list item is not captured inside loop.\n");
      return false;
    }
  }

  // All saefty checks passed.
  return true;
}

// In some cases vectorizer creates additional basic block with mask
// mask computations, which are safe to vectorize.
bool VPlanIdioms::isSafeBlockForSearchLoop(const VPBasicBlock *Block) {
  for (const VPRecipeBase &Recipe : Block->getRecipes()) {
    if (!isa<const VPInstruction>(&Recipe))
      continue;
    return false;
  }
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
        (VPInst.HIR.isDecomposed() && VPInst.isUnderlyingIRValid()))
      continue;

    const HLDDNode *DDNode = cast<HLDDNode>(VPInst.HIR.getUnderlyingNode());
    if (const auto HInst = dyn_cast<const HLInst>(DDNode)) {
      const RegDDRef *LvalRef = HInst->getLvalDDRef();
      const RegDDRef *RvalRef = HInst->getRvalDDRef();
      if (!LvalRef || !RvalRef ||
          (!LvalRef->isTerminalRef() || RvalRef->isNonLinear() ||
           RvalRef->getNumDimensions() != 1)) {
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

VPlanIdioms::Opcode VPlanIdioms::isSearchLoop(const VPlan *Plan,
                                              const unsigned VF,
                                              const bool CheckSafety,
                                              RegDDRef *&PeelArrayRef) {
  // TODO: With explicit representation of peel loop, next code is not valid
  // and has to be changed. Also, any newly created VPInstruction in preheader
  // could probably require bailout too. Seems to work for now though, and
  // should be heavily refactored soon enough to be moved from cost modeling
  // stage to early vectorizer transforms.
  const VPLoopInfo *VPLI = Plan->getVPLoopInfo();
  assert(std::distance(VPLI->begin(), VPLI->end()) == 1
         && "Expected single outermost loop!");

  // For the search loop idiom we expect 1-2 exit blocks and two exiting block.
  const VPLoop *VPL = *VPLI->begin();
  SmallVector<VPBlockBase *, 8> Exitings, Exits;
  VPL->getExitingBlocks(Exitings);
  VPL->getExitBlocks(Exits);

  if (Exitings.size() != 2 || Exits.size() > 2) {
    LLVM_DEBUG(dbgs() << "    Search loop was NOT recognized.\n");
    return VPlanIdioms::Unknown;
  }

  SmallDenseSet<const VPBlockBase *> IgnoreBlocks;
  const VPBasicBlock *Latch = dyn_cast<VPBasicBlock>(VPL->getLoopLatch());
  assert(Latch && "VPLoop does not have loop latch block.");
  if (!isSafeLatchBlockForSearchLoop(Latch)) {
    LLVM_DEBUG(dbgs() << "    Search loop is unsafe.\n");
    return VPlanIdioms::Unsafe;
  }
  IgnoreBlocks.insert(Latch);
  IgnoreBlocks.insert(Latch->getSinglePredecessor());

  for (const VPBlockBase *Exit : Exits) {
    if (!isSafeExitBlockForSearchLoop(cast<VPBasicBlock>(Exit))) {
      LLVM_DEBUG(dbgs() << "    Search loop is unsafe.n\n");
      return VPlanIdioms::Unsafe;
    }
    IgnoreBlocks.insert(Exit);
  }

  const auto Header =
      cast<const VPBasicBlock>(VPL->getHeader());
  // Recognize specific patterns only for the header of the loop. All other
  // blocks will (except Exit block) will be treated unsafe.
  VPlanIdioms::Opcode Opcode = isStrEqSearchLoop(Header, false);
  // TODO: there're also few idiomatic search loops that have to be covered
  // here.
  if (Opcode != VPlanIdioms::SearchLoopStrEq) {
    // Array being searched for if current search loop matches StructPtrEq
    // idiom.
    RegDDRef *ArrayRef = nullptr;
    Opcode = isStructPtrEqSearchLoop(Header, false, ArrayRef);
    if (Opcode != VPlanIdioms::SearchLoopStructPtrEq) {
      LLVM_DEBUG(
          dbgs() << "    StrEq and StructPtrEq loop was not recognized.\n");
      return VPlanIdioms::Unsafe;
    } else {
      // StructPtrEq was recognized, ArrayRef cannot be null
      assert(ArrayRef && "StructPtrEq loop does not have PeelArrayRef.\n");
      LLVM_DEBUG(dbgs() << "    StructPtrEq loop has PeelArray:";
                 ArrayRef->dump(); dbgs() << "\n");
      PeelArrayRef = ArrayRef;
    }
  }
  IgnoreBlocks.insert(Header);

  if (const VPBlockBase *Succ = Header->getSingleSuccessor())
    if (const auto *BB = dyn_cast<VPBasicBlock>(Succ)) {
      if (!isSafeBlockForSearchLoop(BB)) {
        LLVM_DEBUG(dbgs() << "    Search loop is unsafe.\n");
        return VPlanIdioms::Unsafe;
      }
      IgnoreBlocks.insert(BB);
    }

  ReversePostOrderTraversal<const VPBlockBase *> RPOT(Header);
  // Visit the VPBlocks connected to "this", starting from it.
  for (const VPBlockBase *Block : RPOT) {
    // Blocks outside of the loop are safe to execute. Latch and Header blocks
    // were already visited.
    // Every other block is assumed to be unsafe for search loop vectorization.
    if (IgnoreBlocks.count(Block) || !isa<VPBasicBlock>(Block) ||
        !VPL->contains(Block))
      continue;

    LLVM_DEBUG(dbgs() << "        " << Block->getName() << " is unsafe\n");
    LLVM_DEBUG(dbgs() << "    Search loop is unsafe.\n");
    return VPlanIdioms::Unsafe;
  }


  LLVM_DEBUG(dbgs() << "    Search loop was recognized.\n");

  return Opcode;
}

bool VPlanIdioms::isAnySearchLoop(const VPlan *Plan, const unsigned VF,
                                  const bool CheckSafety) {
  RegDDRef *PeelArrayRef = nullptr;
  return isAnySearchLoop(isSearchLoop(Plan, VF, CheckSafety, PeelArrayRef));
}

} // namespace vpo

} // namespace llvm
