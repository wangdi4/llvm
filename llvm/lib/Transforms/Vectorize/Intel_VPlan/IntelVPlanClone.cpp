//===------------------- IntelVPlanClone.cpp - Cloning functions for VPlan -==//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanClone.h"
#include "VPlanHIR/IntelVPLoopRegionHIR.h"

namespace llvm {
namespace vpo {

namespace {

class VPlanNotifier {
public:
  void notifyRegion(VPRegionBlock *) {}
  void notifyLoop(VPLoopRegion *) {}
  void notifyBasicBlock(VPBasicBlock *) {}
};

template <typename NotifierT> class VPlanVisitor {
private:
  static void recursiveVisitor(NotifierT &Notifier, VPBlockBase *Block) {
    ReversePostOrderTraversal<VPBlockBase *> RPOT(Block);
    for (VPBlockBase *Block : RPOT) {
      if (auto BasicBlock = dyn_cast<VPBasicBlock>(Block))
        visit(Notifier, BasicBlock);
      else if (auto Loop = dyn_cast<VPLoopRegion>(Block))
        visit(Notifier, Loop);
      else if (auto Region = dyn_cast<VPRegionBlock>(Block))
        visit(Notifier, Region);
    }
  }

public:
  static void visit(NotifierT &Notifier, VPlan *Plan) {
    recursiveVisitor(Notifier, Plan->getEntry());
  }

  static void visit(NotifierT &Notifier, VPBlockBase *Block) {
    recursiveVisitor(Notifier, Block);
  }

  static void visit(NotifierT &Notifier, VPRegionBlock *Region) {
    Notifier.notifyRegion(Region);
    recursiveVisitor(Notifier, Region->getEntry());
  }

  static void visit(NotifierT &Notifier, VPLoopRegion *Loop) {
    Notifier.notifyLoop(Loop);
    visit(Notifier, Loop->getEntry());
  }

  static void visit(NotifierT &Notifier, VPBasicBlock *Block) {
    Notifier.notifyBasicBlock(Block);
  }
};

} // end of anonymous namespace

/// Main function that recursively remaps cloned instructions in HCFG.
void VPValueMapper::remapHCFG(VPBlockBase *Block) {
  class RecursiveInstructionRemapper : public VPlanNotifier {
  private:
    VPValueMapper &VM;

    void fixCondBit(VPBlockBase *Block) const {
      VPValue *CondBit = Block->getCondBit();
      if (!VM.Block2BlockMap.count(Block) && CondBit) {
        if (auto CondBitInst = dyn_cast<VPInstruction>(CondBit)) {
          VPBlockBase *Parent = CondBitInst->getParent();
          VM.remapInstruction(CondBitInst);
          auto It = VM.Block2BlockMap.find(Parent);
          if (It != VM.Block2BlockMap.end()) {
            CondBitInst->Parent = cast<VPBasicBlock>(It->second);
          }
        }
      }
    }

  public:
    explicit RecursiveInstructionRemapper(VPValueMapper &VM) : VM(VM) {}
    void notifyBasicBlock(VPBasicBlock *Block) {
      if (VM.Block2BlockMap.count(Block)) {
        // Block has not been cloned, because in this map key is original Block,
        // and value is a cloned Block. No need to update original Block's
        // instructions.
        return;
      }

      for (auto &VPInst : Block->vpinstructions())
        VM.remapInstruction(&VPInst);

      fixCondBit(Block);
    }
    void notifyLoop(VPLoopRegion *Loop) { fixCondBit(Loop); }
    void notifyRegion(VPRegionBlock *Region) { fixCondBit(Region); }
  };

  RecursiveInstructionRemapper IR(*this);
  VPlanVisitor<RecursiveInstructionRemapper>::visit(IR, Block);
}

/// Updates VPLoopInfo of the \p Plan according to cloned VPBB from \p
/// OriginalPlan.
/// FIXME: Known issues: top level loops could be not in the right order, due
///        use of addTopLevelLoop(). For instance, when peel loop is added
///        \p Plan, VPLoop for a peel loop will follow VPLoop for a main loop,
///        which is not correct.
/// NOTE: Cannot use VPLoopInfo->analyze() due to issue with
///       GraphTraits<VPBlockBase *>.
void VPValueMapper::cloneVPLoop(VPlan *Plan, const VPlan *OriginalPlan) {
  const VPLoopInfo *OLI = OriginalPlan->getVPLoopInfo();
  VPLoopInfo *LI = Plan->getVPLoopInfo();

  // FIXME: With VPlanLoop no need to look from a loop within given VPlan
  // FIXME: As long as HCFG is shared across all VPlans, need to look for
  // 'second' VPLoopRegion in HCFG, because it represents main loop.
  const VPLoopRegion *OLR = VPlanUtils::findNthLoopDFS(OriginalPlan, 2);

  // At this point cloned VPLoopRegion has been inserted into HCFG and
  // it either has some VPLoopRegion as a parent or it doesn't.
  // NOTE: Assume that the parent, if one exists, has valid VPLoop.

  VPLoopRegion *LR = cast<VPLoopRegion>(Block2BlockMap.find(OLR)->second);
  VPLoop *ParentLoop = LR->getParent() ? LR->getVPLoop() : nullptr;

  for (const VPLoop *OTopLoop : make_range(OLI->begin(), OLI->end())) {
    std::function<void(const VPLoop *, VPLoop *)> Dfs =
        [&](const VPLoop *OLoop, VPLoop *ParentLoop) -> void {
      VPLoop *Loop = nullptr;
      if (entireLoopWasCloned(OLoop)) {
        Loop = LI->AllocateLoop();
        if (ParentLoop)
          ParentLoop->addChildLoop(Loop);
        else
          LI->addTopLevelLoop(Loop);

        for (auto OB : OLoop->getBlocks())
          Loop->addBasicBlockToLoop(Block2BlockMap.find(OB)->second, *LI);
      }

      for (const VPLoop *OSubLoop : make_range(OLoop->begin(), OLoop->end()))
        Dfs(OSubLoop, Loop);

    };

    Dfs(OTopLoop, ParentLoop);
  }
}

/// Clone VPLoop from \p OriginalPlan into \p Plan and update VPLoopRegion in \p
/// Plan.
void VPValueMapper::remapVPLoop(VPlan *Plan, const VPlan *OriginalPlan) {
  cloneVPLoop(Plan, OriginalPlan);
  updateLoopRegionsAfterCloning(Plan);
}

/// Simply traverse HCFG of the \p Plan and sets corresponding VPLoop on cloned
/// VPLoopRegions.
void VPValueMapper::updateLoopRegionsAfterCloning(VPlan *Plan) {
  class VPLoopSetter : public VPlanNotifier {
  private:
    VPlan *Plan;

  public:
    explicit VPLoopSetter(VPlan *Plan) : Plan(Plan) {}
    void notifyLoop(VPLoopRegion *LoopRegion) {
      if (!LoopRegion->getVPLoop()) {
        ReversePostOrderTraversal<VPBlockBase *> RPOT(LoopRegion->getEntry());
        for (auto Block : make_range(RPOT.begin(), RPOT.end())) {
          if (VPLoop *Loop = Plan->getVPLoopInfo()->getLoopFor(Block)) {
            LoopRegion->setVPLoop(Loop);
            break;
          }
        }
        assert(LoopRegion->getVPLoop() &&
               "VPLoop has been lost for VPLoopRegion.");
      }
    }
  };

  VPLoopSetter LS(Plan);
  VPlanVisitor<VPLoopSetter>::visit(LS, Plan);
}

/// Remap all operands from cloned \p Inst instruction.
void VPValueMapper::remapInstruction(VPInstruction *Inst) {
  for (unsigned I = 0, E = Inst->getNumOperands(); I != E; ++I) {
    Inst->setOperand(I, remapValue(Value2ValueMap, Inst->getOperand(I)));
  }

  if (auto Phi = dyn_cast<VPPHINode>(Inst)) {
    for (auto &B : Phi->blocks()) {
      B = cast<VPBasicBlock>(remapValue(Block2BlockMap, B));
    }
  }
}

/// Verify that either entire \p Loop has been copied and \return true.
/// If it has been copied partially, \return false.
bool VPValueMapper::entireLoopWasCloned(const VPLoop *Loop) {
  int32_t Flags = 0x0;
  for (auto Block : Loop->getBlocks()) {
    Flags |= 1 << Block2BlockMap.count(Block);
  }
  assert(Flags && "At least one BB is expected.");
  return Flags == 0x2;
}

/// Generic function that accepts any \p Block and clones it.
/// Remapping happens later by VPValueMapper which must be called by user.
VPBlockBase *VPCloneUtils::cloneBlockBase(VPBlockBase *Block,
                                          std::string Prefix,
                                          Block2BlockMapTy &BlockMap,
                                          Value2ValueMapTy &ValueMap) {
  VPBlockBase *ClonedBlock = nullptr;

  Prefix = Prefix == "" ? "cloned." : Prefix;
  std::string Name = VPlanUtils::createUniqueName((Prefix + Block->getName()));

  if (auto BasicBlock = dyn_cast<VPBasicBlock>(Block)) {
    ClonedBlock = cloneBasicBlock(BasicBlock, Name, ValueMap);
  } else if (auto Region = dyn_cast<VPRegionBlock>(Block)) {
    ClonedBlock = cloneRegion(Region, Name, BlockMap, ValueMap);
  } else {
    llvm_unreachable("Unknown way to clone.");
  }
  BlockMap.insert({Block, ClonedBlock});
  if (auto CondBit = Block->getCondBit()) {
    VPValue *ClonedCondBit = CondBit;
    if (auto CondBitInst = dyn_cast<VPInstruction>(CondBit)) {
      ClonedCondBit = CondBitInst->clone();
      // Parent of the cloned condition bit will be updated later by
      // VPValueMapper.
      cast<VPInstruction>(ClonedCondBit)->Parent = CondBitInst->getParent();
    }
    ClonedBlock->setCondBit(ClonedCondBit);
    ValueMap.insert({CondBit, ClonedCondBit});
  }
  return ClonedBlock;
}

/// Clone \p Block and its instructions.
/// Remapping happens later by VPValueMapper which must be called by user.
VPBasicBlock *VPCloneUtils::cloneBasicBlock(VPBasicBlock *Block,
                                            std::string Prefix,
                                            Value2ValueMapTy &ValueMap) {
  VPBasicBlock *ClonedBlock = new VPBasicBlock(Prefix);

  for (auto &Recipe : Block->getRecipes()) {
    if (auto Inst = dyn_cast<VPInstruction>(&Recipe)) {
      auto ClonedInst = Inst->clone();
      ClonedBlock->appendRecipe(ClonedInst);
      ValueMap.insert(std::make_pair(Inst, ClonedInst));
    } else {
      llvm_unreachable("Don't know how to clone this type of recipe.");
    }
  }

  return ClonedBlock;
}

/// Clone \p Region.
/// Remapping happens later by VPValueMapper which must be called by user.
VPRegionBlock *VPCloneUtils::cloneRegion(VPRegionBlock *Region,
                                         std::string Prefix,
                                         Block2BlockMapTy &BlockMap,
                                         Value2ValueMapTy &ValueMap) {
  Prefix = Prefix == "" ? "cloned." : Prefix;
  VPBlockBase *Entry = Region->getEntry(), *Exit = Region->getExit();
  VPBlockBase *ClonedEntry = nullptr, *ClonedExit = nullptr;

  std::function<VPBlockBase *(VPBlockBase *)> Cloning =
      [&](VPBlockBase *Block) -> VPBlockBase * {
    auto It = BlockMap.find(Block);
    if (It != BlockMap.end())
      return It->second;

    VPBlockBase *ClonedBlock =
        cloneBlockBase(Block, Prefix, BlockMap, ValueMap);
    BlockMap.insert({Block, ClonedBlock});

    if (Block == Exit)
      ClonedExit = ClonedBlock;

    for (auto &Succ : Block->getSuccessors()) {
      VPBlockBase *ClonedSucc = Cloning(Succ);
      ClonedBlock->appendSuccessor(ClonedSucc);
      ClonedSucc->appendPredecessor(ClonedBlock);
    }

    return ClonedBlock;
  };

  ClonedEntry = Cloning(Entry);

  VPRegionBlock *ClonedR = nullptr;
  if (auto LRHIR = dyn_cast<VPLoopRegionHIR>(Region))
    ClonedR = new VPLoopRegionHIR(
        VPlanUtils::createUniqueName(Prefix + Region->getName()), nullptr,
        LRHIR->getHLLoop());
  else if (isa<VPLoopRegion>(Region))
    ClonedR = new VPLoopRegion(
        VPlanUtils::createUniqueName(Prefix + Region->getName()), nullptr);
  else
    ClonedR = new VPRegionBlock(VPBlockBase::VPRegionBlockSC,
        VPlanUtils::createUniqueName(Prefix + Region->getName()));

  BlockMap.insert({Region, ClonedR});

  ClonedR->setEntry(ClonedEntry);
  ClonedR->setExit(ClonedExit);
  VPBlockUtils::setParentRegionForBody(ClonedR);
  return ClonedR;
}

VPLoopRegion *VPCloneUtils::cloneLoopRegion(VPLoopRegion *LR, std::string Prefix,
                                       Block2BlockMapTy &BlockMap,
                                       Value2ValueMapTy &ValueMap) {

  return cast<VPLoopRegion>(cloneRegion(LR, Prefix, BlockMap, ValueMap));
}

} // namespace vpo
} // namespace llvm
