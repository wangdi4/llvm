//===- IntelVPlanIdioms.h -------------------------------------------------===//
//
//   Copyright (C) 2018 Intel Corporation. All rights reserved.
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

#define DEBUG_TYPE "vplan-idioms"

cl::opt<bool>
    AllowMemorySpeculation("allow-memory-speculation", cl::init(false),
                           cl::desc("Enable speculative vector unit loads."));

namespace llvm {

using namespace loopopt;

namespace vpo {

static bool canSpeculate(const RegDDRef *Ref) {
  // FIXME: Unit-stride check is needed.
  if (Ref->isTerminalRef())
    return true;
  if (Ref->isMemRef()) {
    // FIXME: Copied code from CM. Has to be removed.
    int64_t ConstStride;
    unsigned NestingLevel =
        Ref->getHLDDNode()->getParentLoop()->getNestingLevel();
    if (!Ref->getConstStrideAtLevel(NestingLevel, &ConstStride) || !ConstStride)
      return false;

    // Compute stride in terms of number of elements
    auto DL = Ref->getDDRefUtils().getDataLayout();
    auto RefSizeInBytes = DL.getTypeSizeInBits(Ref->getDestType()) >> 3;
    ConstStride /= RefSizeInBytes;
    if (ConstStride != 1)
      return false;

    if (AllowMemorySpeculation)
      return true;
  }
  return false;
}

// Expect following sequence of instructions in latch block:
//    BlockPredicate
//    add
//    cmp
bool VPlanIdioms::isSafeLatchBlockForSearchLoop(const VPBasicBlock *Block) {
  unsigned NumAdds = 0;
  for (const VPRecipeBase &Recipe : Block->getRecipes()) {
    if (isa<const VPBlockPredicateRecipe>(&Recipe))
      continue;
    if (const auto Inst = dyn_cast<VPInstruction>(&Recipe)) {
      if (Inst->getOpcode() == Instruction::Add) {
        ++NumAdds;
        continue;
      }
      if (Inst->getOpcode() == Instruction::ICmp)
        continue;
    }

    return false;
  }
  return NumAdds == 1;
}

VPlanIdioms::Opcode
VPlanIdioms::isStrEqSearchLoop(const VPBasicBlock *Block,
                               const bool AllowSpeculation) {
  for (const VPRecipeBase &Recipe : Block->getRecipes()) {
    if (!isa<const VPInstruction>(&Recipe))
      continue;
    const auto Inst = cast<const VPInstruction>(&Recipe);

    if (isa<const VPBranchInst>(Inst) ||
        (Inst->HIR.isDecomposed() && Inst->HIR.isValid()))
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
      if (!RvalRef) {
        return VPlanIdioms::Unsafe;
      }
      if (RvalRef->isTerminalRef() && RvalRef->isNonLinear()) {
        LLVM_DEBUG(dbgs() << "        RegDDRef "; RvalRef->dump();
                   dbgs() << " is unsafe.\n");
        return VPlanIdioms::Unsafe;
      }

      // FIXME: Need to look on VPPredicates, not on underlying IR.
      if (!isa<HLIf>(HInst->getParent())) {
        // No support for assignment to live-out terminals or when memory
        // speculation is not allowed.
        if (!AllowSpeculation &&
            ((LvalRef->isTerminalRef() && LvalRef->isLiveOutOfParentLoop()) ||
             (!canSpeculate(LvalRef) || !canSpeculate(RvalRef)))) {
          LLVM_DEBUG(dbgs() << "        HLInst "; HInst->dump();
                     dbgs() << " is unmasked, thus it's unsafe.\n");
          return VPlanIdioms::Unsafe;
        }
      }
    }
  }
  return VPlanIdioms::SearchLoopStrEq;
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
  for (const VPRecipeBase &Recipe : Block->getRecipes()) {
    if (!isa<const VPInstruction>(&Recipe))
      continue;
    const auto Inst = cast<const VPInstruction>(&Recipe);

    if (isa<const VPBranchInst>(Inst) ||
        (Inst->HIR.isDecomposed() && Inst->HIR.isValid()))
      continue;

    const HLDDNode *DDNode = cast<HLDDNode>(Inst->HIR.getUnderlyingNode());
    if (const auto HInst = dyn_cast<const HLInst>(DDNode)) {
      const RegDDRef *LvalRef = HInst->getLvalDDRef();
      const RegDDRef *RvalRef = HInst->getRvalDDRef();
      if (!LvalRef || !RvalRef ||
          (!LvalRef->isTerminalRef() || RvalRef->isNonLinear())) {
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
                                              const bool CheckSafity) {
  const VPRegionBlock *Entry = dyn_cast<const VPRegionBlock>(Plan->getEntry());
  assert(Entry && "RegionBlock is expected.");
  // TODO: With explicit representation of peel loop, next code is not valid
  // and has to be changed.
  const VPLoopRegion *MELoop =
      dyn_cast<const VPLoopRegion>(Entry->getEntry()->getSingleSuccessor());
  LLVM_DEBUG(
      dbgs() << "Idiom recognition: Trying to recognize search loop for VPlan "
             << Plan << " with VF " << VF << ".\n");
  if (!MELoop) {
    LLVM_DEBUG(dbgs() << "    Search loop was NOT recognized.\n");
    return VPlanIdioms::Unknown;
  }

  // For the search loop idiom we expect 1-2 exit blocks and two exiting block.
  const VPLoop *VPL = MELoop->getVPLoop();
  SmallVector<VPBlockBase *, 8> Exitings, Exits;
  VPL->getExitingBlocks(Exitings);
  VPL->getExitBlocks(Exits);

  if (Exitings.size() != 2 || Exits.size() > 2) {
    LLVM_DEBUG(dbgs() << "    Search loop was NOT recognized.\n");
    return VPlanIdioms::Unknown;
  }

  SmallDenseSet<const VPBlockBase *> IgnoreBlocks;
  IgnoreBlocks.insert(MELoop->getEntry());
  const VPBasicBlock *Latch = dyn_cast<VPBasicBlock>(VPL->getLoopLatch());
  if (!isSafeLatchBlockForSearchLoop(Latch)) {
    LLVM_DEBUG(dbgs() << "    Search loop is unsafe.\n");
    return VPlanIdioms::Unsafe;
  }
  IgnoreBlocks.insert(Latch);

  for (const VPBlockBase *Exit : Exits) {
    if (!isSafeExitBlockForSearchLoop(cast<VPBasicBlock>(Exit))) {
      LLVM_DEBUG(dbgs() << "    Search loop is unsafe.n\n");
      return VPlanIdioms::Unsafe;
    }
    IgnoreBlocks.insert(Exit);
  }

  const auto Header =
      dyn_cast<const VPBasicBlock>(MELoop->getVPLoop()->getHeader());
  // Recognize specific patterns only for the header of the loop. All other
  // blocks will (except Exit block) will be treated unsafe.
  VPlanIdioms::Opcode Opcode = isStrEqSearchLoop(Header, false);
  // TODO: there're also few idiomatic search loops that have to be covered
  // here.
  if (Opcode != VPlanIdioms::SearchLoopStrEq) {
    LLVM_DEBUG(dbgs() << "    StrEq loop was not recognized.\n");
    return VPlanIdioms::Unsafe;
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

  ReversePostOrderTraversal<const VPBlockBase *> RPOT(MELoop->getEntry());
  // Visit the VPBlocks connected to "this", starting from it.
  for (const VPBlockBase *Block : RPOT) {
    // Blocks outside of the loop are safe to execute. Latch and Header blocks
    // were already visited.
    // Every other block is assumed to be unsafe for search loop vectorization.
    if (IgnoreBlocks.count(Block) || !isa<VPBasicBlock>(Block) ||
        !MELoop->getVPLoop()->contains(Block))
      continue;

    LLVM_DEBUG(dbgs() << "        " << Block->getName() << " is unsafe\n");
    LLVM_DEBUG(dbgs() << "    Search loop is unsafe.\n");
    return VPlanIdioms::Unsafe;
  }


  LLVM_DEBUG(dbgs() << "    Search loop was recognized.\n");

  return Opcode;
}

bool VPlanIdioms::isAnySearchLoop(const VPlan *Plan, const unsigned VF,
                                  const bool CheckSafity) {

  VPlanIdioms::Opcode Opcode = isSearchLoop(Plan, VF, CheckSafity);
  return Opcode == VPlanIdioms::SearchLoopStrEq ||
         Opcode == VPlanIdioms::SearchLoop;
}

} // namespace vpo

} // namespace llvm
