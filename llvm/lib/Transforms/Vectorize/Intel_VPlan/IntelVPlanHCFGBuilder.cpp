//===-- IntelVPlanHCFGBuilder.cpp -----------------------------------------===//
//
//   Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the algorithm that builds the hierarchical CFG in
/// VPlan. Further documentation can be found in document 'VPlan Hierarchical
/// CFG Builder'.
///
//===----------------------------------------------------------------------===//
#include "IntelVPlanHCFGBuilder.h"
#include "IntelLoopVectorizationLegality.h"
#include "IntelVPLoopAnalysis.h"
#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"
#include "IntelVPlanDivergenceAnalysis.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanLoopInfo.h"
#include "IntelVPlanSyncDependenceAnalysis.h"
#include "IntelVPlanUtils.h"
#include "IntelVPlanVerifier.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/IR/IntrinsicInst.h"

#define DEBUG_TYPE "VPlanHCFGBuilder"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool> DisableNonLoopSubRegions(
    "disable-vplan-subregions", cl::init(true), cl::Hidden,
    cl::desc("Disable construction of non-loop subregions in VPlan"));

cl::opt<bool> LoopMassagingEnabled(
    "vplan-enable-loop-massaging", cl::init(true), cl::Hidden,
    cl::desc("Enable loop massaging in VPlan (Multiple to Singular Exit)"));

static cl::opt<bool> VPlanPrintAfterLoopMassaging(
    "vplan-print-after-loop-massaging", cl::init(false),
    cl::desc("Print plain dump after loop massaging"));

static cl::opt<bool> DisableUniformRegions(
    "disable-uniform-regions", cl::init(false), cl::Hidden,
    cl::desc("Disable detection of uniform Regions in VPlan. All regions are "
             "set as divergent."));

#if INTEL_CUSTOMIZATION
static cl::opt<bool>
    DisableVPlanDA("disable-vplan-da", cl::init(false), cl::Hidden,
                   cl::desc("Disable VPlan divergence analysis"));

static cl::opt<bool>
    VPlanPrintSimplifyCFG("vplan-print-after-simplify-cfg", cl::init(false),
                          cl::desc("Print plain dump after VPlan simplify "
                                   "plain CFG"));

static cl::opt<bool>
    VPlanPrintHCFG("vplan-print-after-hcfg", cl::init(false),
                   cl::desc("Print plain dump after build VPlan H-CFG."));

static cl::opt<bool>
    VPlanPrintPlainCFG("vplan-print-plain-cfg", cl::init(false),
                       cl::desc("Print plain dump after VPlan buildPlainCFG."));

extern cl::opt<bool> EnableVPValueCodegen;
#endif

VPlanHCFGBuilder::VPlanHCFGBuilder(Loop *Lp, LoopInfo *LI, ScalarEvolution *SE,
                                   const DataLayout &DL,
                                   const WRNVecLoopNode *WRL, VPlan *Plan,
                                   VPOVectorizationLegality *Legal)
    : TheLoop(Lp), LI(LI), SE(SE), WRLp(WRL), Plan(Plan), Legal(Legal) {
  // TODO: Turn Verifier pointer into an object when Patch #3 of Patch Series
  // #1 lands into VPO and VPlanHCFGBuilderBase is removed.
  Verifier = std::make_unique<VPlanVerifier>(Lp, LI, DL);
  assert((!WRLp || WRLp->getTheLoop<Loop>() == TheLoop) &&
         "Inconsistent Loop information");
}

// Split loops' preheader block that are not in canonical form
void VPlanHCFGBuilder::splitLoopsPreheader(VPLoop *VPL) {

  // TODO: So far, I haven't found a test case that hits one of these asserts.
  // The code commented out below should cover the second one.

  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

  // Temporal assert to detect loop header with more than one loop external
  // predecessor
  unsigned NumExternalPreds = 0;
  for (const VPBlockBase *Pred : VPL->getHeader()->getPredecessors()) {
    if (!VPL->contains(Pred))
      ++NumExternalPreds;
  }
  assert((NumExternalPreds == 1) &&
         "Loop header's external predecessor is not 1");

  // Temporal assert to detect loop preheader with multiple successors
  assert((VPL->getLoopPreheader()->getNumSuccessors() == 1) &&
         "Loop preheader with multiple successors are not supported");

  // If PH has multiple successors, create new PH such that PH->NewPH->H
  // if (VPL->getLoopPreheader()->getNumSuccessors() > 1) {

  //  VPBlockBase *OldPreheader = VPL->getLoopPreheader();
  //  VPBlockBase *Header = VPL->getHeader();
  //  assert((DomTree.getNode(Header)->getIDom()->getBlock() == OldPreheader) &&
  //         "Header IDom is not Preheader");

  //  // Create new preheader
  //  VPBasicBlock *NewPreheader = PlanUtils.createBasicBlock();
  //  PlanUtils.insertBlockAfter(NewPreheader, OldPreheader);

  //  // Add new preheader to VPLoopInfo
  //  if (VPLoop *PHLoop = VPLInfo->getLoopFor(OldPreheader)) {
  //    PHLoop->addBasicBlockToLoop(NewPreheader, *VPLInfo);
  //  }

  //  // Update dom/postdom information

  //  // Old preheader is idom of new preheader
  //  VPDomTreeNode *NewPHDomNode =
  //      DomTree.addNewBlock(NewPreheader, OldPreheader /*IDom*/);

  //  // New preheader is idom of header
  //  VPDomTreeNode *DTHeader = DomTree.getNode(Header);
  //  assert(DTHeader && "Expected DomTreeNode for loop header");
  //  DomTree.changeImmediateDominator(DTHeader, NewPHDomNode);

  //  // Header is ipostdom of new preheader
  //  //VPDomTreeNode *NewPHPostDomNode =
  //  PostDomTree.addNewBlock(NewPreheader, Header /*IDom*/);

  //  // New preheader is not ipostdom of any block
  //
  //  // This is not true: New preheader is ipostdom of old preheader
  //  //VPDomTreeNode *PDTPreheader = PostDomTree.getNode(OldPreheader);
  //  //assert(PDTPreheader && "Expected DomTreeNode for loop preheader");
  //  //PostDomTree.changeImmediateDominator(PDTPreheader, NewPHPostDomNode);
  //}

  VPBlockBase *PH = VPL->getLoopPreheader();
  assert(PH && "Expected loop preheader");
  assert((PH->getNumSuccessors() == 1) &&
         "Expected preheader with single successor");

  // Split loop PH if:
  //    - there is no WRLp (auto-vectorization). We need an empty loop PH.
  //    - has multiple predecessors (it's a potential exit of another region).
  //    - is loop H of another loop.
  if (!WRLp || !PH->getSinglePredecessor() || VPLInfo->isLoopHeader(PH)) {
    VPBlockUtils::splitBlock(PH, VPLInfo, VPDomTree, VPPostDomTree);
  }

  // Apply simplification to subloops
  for (auto VPSL : VPL->getSubLoops()) {
    splitLoopsPreheader(VPSL);
  }
}

// The merge loop exit transformation is applied to the inner loops of a loop
// nest. It consists of the mergeLoopExits function and six support functions
// (getBlocksExitBlock, updateBlocksPhiNode, moveExitBlocksPhiNode,
// removeBlockFromVPPhiNode, allPredsInLoop, hasVPPhiNode, getFirstNonPhiVPInst,
// updatePhiNodeInLoopHeader).
//
// The algorithm has three steps:
// 1. A new loop latch is created. All the side exits are redirected to the new
// loop latch. The new loop latch is followed by a number of cascaded ifs (if it
// is needed). The cascaded ifs redirect the control-flow to the right exit
// block.
// 2. For each exit block (other than the exit block of the loop latch), a new
// intermediate block (Intermediate_BB) is generated. The new blocks are
// connected with the corresponding exiting block(s) and the new loop latch. For
// each Intermediate_BB, a new incoming value (ExitID) is added in the phi node
// of the new loop latch.
// 3. The cascaded ifs are emitted (if it is needed).
//
// The following examples show how merge loop exits transformation works on
// different test cases:
//
// CASE 1: The inner loop has one side exit. Therefore, a Intermediate_BB is
// generated for the side exit that lands on BB4. The Intermediate_BB is
// connected with the new loop latch. Since there is only one side exit, one
// cascaded if block is emitted.
// -------------------------------------------
//  FROM                        TO
// -------------------------------------------
//
// -->BB1           ----------->BB1
// |   | \          |          /   \
// |   |  \         |         /     \
// ---BB2  BB4  =>  |       BB2     Intermediate_BB
//     |            |          \    /
//     |            |  ExitID=0 \  / ExitID=1
//    BB3           --------NEW_LOOP_LATCH
//                                |
//                                |
//                        CASCADED_IF_BLOCK
//                          (if ExitID==1)
//                             F / \ T
//                              /   \
//            (LatchExitBlock)BB3   BB4
//
// CASE 2: The inner loop has one side exit which lands on the same exit block
// as the loop latch. If the exiting block is the loop header, then we create an
// Intermediate_BB. This is needed for emitting correct masks. For any other
// basic block, we just redirect the exiting edge to the new loop latch. In this
// case, the cascaded if blocks are not needed.
// -------------------------------------------
//  FROM                        TO
// -------------------------------------------
//
// -->BB1               ------->BB1
// |   | \              |        | \
// |   |  \             |        |  \
// |  BB2  \            |       BB2  \
// |   | \  |           |        | \  \
// |   |  \ |    =>     |        |  \  Intermediate_BB
// ---BB3  ||           |       BB3  | /
//     |  //            |        |  / /
//     | //             |        | / /
//    BB4               ----NEW_LOOP_LATCH
//
// CASE 3: The inner loop has two side exits that land on the same exit block.
// Therefore, the control flow for BB1 and BB2 should be redirected on the same
// exit block. For this reason, one Intermediate_BB is emitted.
// -------------------------------------------
//  FROM                          TO
// -------------------------------------------
//
// -->BB1             ----------->BB1
// |   | \            |            | \
// |   |  \           |            |  \
// |  BB2  \      =>  |           BB2  \
// |   | \  \         |            | \  \
// |   |  BB4         |            |  Intermediate_BB
// ---BB3             |           BB3   /
//     |              |            |   /
//     |              |   ExitID=0 |  / ExitID=1
//    BB5             --------NEW_LOOP_LATCH
//                                |
//                                |
//                        CASCADED_IF_BLOCK
//                          (if ExitID==1)
//                             F / \ T
//                              /   \
//            (LatchExitBlock)BB5   BB4
//
// CASE 4: The inner loop has two side exits. Therefore, two Intermediate_BBs
// and two cascaded ifs are created.
// -------------------------------------------
//  FROM                          TO
// -------------------------------------------
//
// -->BB1             ----------->BB1--------------------------
// |   | \            |            |                          |
// |   |  \           |            |                          |
// |  BB2  BB4    =>  |           BB2                         |
// |   | \            |            | \                        |
// |   |  \           |            |  \                       V
// |   |   BB5        |            | Intermediate_BB2  Intermediate_BB1
// ---BB3             |           BB3 ExitID=1             ExitID=2
//     |              |            |   /                      |
//     |              |   ExitID=0 |  /                       |
//    BB6             --------NEW_LOOP_LATCH<------------------
//                                |
//                                |
//                        CASCADED_IF_BLOCK(HEAD)
//                          (if ExitID==2)
//                             F /    \ T
//                              /      \
//                          IFBLOCK    BB4
//                      (if ExitID==1)
//                         F /   \ T
//                          /     \
//        (LatchExitBlock)BB6     BB5
//
// CASE 5: Loop rotate does not always canonicalize the while loops. In this
// case, the original loop latch has an unconditional branch. The merge loop
// exits tranformation does the same transformation.
// -------------------------------------------
//  FROM                          TO
// -------------------------------------------
//
// -->BB1                ----------->BB1--------------------------
// |   | \               |            |                          |
// |   |  \              |            |                          |
// |  BB2  \       =>    |           BB2                         |
// |   | \  \            |            | \                        |
// |   |  \  \           |            |  \                       V
// |   |   \  \          |            | Intermediate_BB2  Intermediate_BB1
// ---BB3 BB4 BB5        |           BB3 ExitID=1             ExitID=2
//          \ /          |            |   /                      |
//          BB6          |   ExitID=0 |  /                       |
//                       --------NEW_LOOP_LATCH<------------------
//                                    |
//                                    |
//                            CASCADED_IF_BLOCK(HEAD)
//                              (if ExitID==2)
//                               F /    \ T
//                                /      \
//                              BB4      BB5
//                                \      /
//                                 \    /
//                                  BB6
//
// Returns the exit block of a block.
static VPBlockBase *getBlocksExitBlock(VPBlockBase *ExitingBlock, VPLoop *VPL) {
  for (VPBlockBase *SuccBlock : ExitingBlock->getSuccessors()) {
    if (!VPL->contains(SuccBlock))
      return SuccBlock;
  }
  return nullptr;
}

// Replaces incoming block in all phi nodes of the PhiBlock.
static void updateBlocksPhiNode(VPBasicBlock *PhiBlock,
                                VPBasicBlock *PrevIncomingBlock,
                                VPBasicBlock *NewLIncomingBlock) {
  for (auto it = PhiBlock->begin(); it != PhiBlock->end(); ++it) {
    if (VPPHINode *VPPhi = dyn_cast<VPPHINode>(&*it)) {
      for (const auto *PBlk : VPPhi->blocks())
        if (PBlk == PrevIncomingBlock) {
          int Idx = VPPhi->getBlockIndex(PrevIncomingBlock);
          VPPhi->setIncomingBlock(Idx, NewLIncomingBlock);
        }
    } else
      break;
  }
}

// Move exit block's phi node to another block (new loop latch or intermediate
// block or cascaded if block). This is needed because the predecessors of the
// exit block change after disconnecting it from the exiting block. Thus, we
// might need to move the phi node of the exit block to the new loop latch or
// the intermediate block or the cascaded if block.
static void moveExitBlocksPhiNode(VPBasicBlock *ExitBlock,
                                  VPBasicBlock *NewBlock) {
  auto itNext = ExitBlock->begin();
  for (auto it = ExitBlock->begin(); it != ExitBlock->end(); it = itNext) {
    itNext = it;
    ++itNext;
    if (VPPHINode *ExitBlockVPPhi = dyn_cast<VPPHINode>(&*it)) {
      ExitBlock->removeRecipe(ExitBlockVPPhi);
      if (NewBlock->empty())
        NewBlock->addRecipe(ExitBlockVPPhi);
      else if (isa<VPPHINode>(&*NewBlock->begin()))
        NewBlock->addRecipeAfter(ExitBlockVPPhi, &*NewBlock->begin());
      else
        NewBlock->addRecipe(ExitBlockVPPhi, &*NewBlock->begin());
    } else
      break;
  }
}

// Removes a basic block from VPPHINode.
static void removeBlockFromVPPhiNode(VPBasicBlock *ExitingBlock,
                                     VPBasicBlock *ExitBlock) {
  auto itNext = ExitBlock->begin();
  for (auto it = ExitBlock->begin(); it != ExitBlock->end(); it = itNext) {
    itNext = it;
    ++itNext;
    if (VPPHINode *ExitBlockVPPhi = dyn_cast<VPPHINode>(&*it))
      ExitBlockVPPhi->removeIncomingValue(ExitingBlock);
    else
      break;
  }
}

// Checks if all the predecessors belong of an exit blocks are in the current
// loop.
static bool allPredsInLoop(VPBlockBase *ExitBlock, VPLoop *VPL) {
  for (auto Pred : ExitBlock->getPredecessors())
    if (!(VPL->contains(Pred)))
      return false;
  return true;
}

// Checks if a basic block has a VPPHINode.
static bool hasVPPhiNode(VPBasicBlock *VPBB) {
  if ((!VPBB->empty()) && (isa<VPPHINode>(*VPBB->begin())))
    return true;
  return false;
}

// Returns the first instruction of a basic block that is not a phi node.
static VPInstruction *getFirstNonPhiVPInst(VPBasicBlock *LoopHeader) {
  VPInstruction *NonPhiVPInst = nullptr;
  for (auto It = LoopHeader->begin(); It != LoopHeader->end(); ++It) {
    if (!isa<VPPHINode>(It)) {
      NonPhiVPInst = dyn_cast<VPInstruction>(It);
      break;
    }
  }
  return NonPhiVPInst;
}

// Adds a new phi node in the loop header for the backedge from the new loop
// latch.
static VPPHINode *updatePhiNodeInLoopHeader(VPBasicBlock *LoopHeader,
                                            VPBasicBlock *NewLoopLatch,
                                            VPPHINode *NewLatchVPPhi,
                                            VPlan *Plan) {

  // Find the location where we should emit the new phi node.
  VPInstruction *NonPhiVPInst = getFirstNonPhiVPInst(LoopHeader);
  Type *Ty = Type::getInt32Ty(*Plan->getLLVMContext());
  VPPHINode *NewVPPhi = new VPPHINode(Ty);
  // Add the NewVPPhi before the first non phi instruction. If the basic-block
  // is empty or there are only phi instructions in it, then the NewVPPhi will
  // be added at the end of the basic block.
  LoopHeader->addRecipe(NewVPPhi, NonPhiVPInst);

  // Get the predecessors of the loop header and add them in the new phi node of
  // the loop header.
  auto &LoopHeaderPreds = LoopHeader->getPredecessors();
  unsigned PhiValue = 0;
  assert(LoopHeaderPreds.size() <= 2 &&
         "The loop header can have up to 2 predecessors!");
  for (unsigned i = 0; i < LoopHeaderPreds.size(); i++) {
    if (LoopHeaderPreds[i] == NewLoopLatch)
      NewVPPhi->addIncoming(NewLatchVPPhi, NewLoopLatch);
    else {
      VPBasicBlock *Pred = dyn_cast<VPBasicBlock>(LoopHeaderPreds[i]);
      VPConstant *PhiValueConst =
          Plan->getVPConstant(ConstantInt::get(NewVPPhi->getType(), PhiValue));
      NewVPPhi->addIncoming(PhiValueConst, Pred);
      PhiValue++;
    }
  }
  return NewVPPhi;
}

void VPlanHCFGBuilder::mergeLoopExits(VPLoop *VPL) {

  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

  // Check if the loop has multiple exits. Merge loop exits transformation is
  // only applied in the inner loops of a loop nest.
  VPLoop *ParentLoop = VPL->getParentLoop();
  SmallVector<VPBlockBase *, 2> ExitingBlocks;
  VPL->getExitingBlocks(ExitingBlocks);
  if ((ExitingBlocks.size() < 2) || ParentLoop == nullptr)
    return;

  // FIXME: Don't break SSA form during the transformation.
  if (!Plan->isFullLinearizationForced() // Already marked, so...
      && isBreakingSSA(VPL))             // don't try to analyze any more.
    Plan->markFullLinearizationForced();

  // The merge loop exits transformation kicks-in.

  LLVM_DEBUG(dbgs() << "Before merge loop exits transformation.\n");
  LLVM_DEBUG(Plan->dump());

  SmallVector<VPBlockBase *, 2> ExitBlocks;
  VPL->getUniqueExitBlocks(ExitBlocks);
  unsigned ExitID = 0;
  // ExitBlockIDPairs and ExitExitingBlocksMap are used for generating the
  // cascaded if blocks.
  SmallVector<std::pair<VPBlockBase *, VPConstant *>, 2> ExitBlockIDPairs;
  SmallDenseMap<VPBlockBase *, VPBlockBase *> ExitExitingBlocksMap;
  VPBasicBlock *OrigLoopLatch = dyn_cast<VPBasicBlock>(VPL->getLoopLatch());
  VPBasicBlock *LoopHeader = dyn_cast<VPBasicBlock>(VPL->getHeader());
  // If the loop is a while loop (case 5), then it might not have a fall-through
  // edge. Therefore, in this case, the LatchExitBlock will be null.
  VPBlockBase *LatchExitBlock = nullptr;
  // BackedgeCond checks whether the backedge is taken when the CondBit is
  // true or false.
  bool BackedgeCond = (OrigLoopLatch->getSuccessors()[0] == LoopHeader);
  if (OrigLoopLatch->getNumSuccessors() > 1) {
    // For for-loops, we get the exit block of the latch.
    assert(OrigLoopLatch->getNumSuccessors() == 2 &&
           "The loop latch should have two successors!");
    LatchExitBlock = BackedgeCond ? OrigLoopLatch->getSuccessors()[1]
                                  : OrigLoopLatch->getSuccessors()[0];
  }
  VPlanDivergenceAnalysis *VPlanDA = Plan->getVPlanDA();
  // The divergence information is later used to set the new condition bit
  // that merge loops exits transformation introduces.
  bool IsDivergent =
      llvm::any_of(ExitingBlocks, [VPlanDA](VPBlockBase *ExitingBlock) {
        return VPlanDA->isDivergent(*ExitingBlock->getCondBit());
      });

  // Step 1 : Creates a new loop latch and fills it with all the necessary
  // instructions.
  VPBasicBlock *NewLoopLatch =
      new VPBasicBlock(VPlanUtils::createUniqueName("new.loop.latch"));
  OrigLoopLatch->moveConditionalEOBTo(NewLoopLatch);
  NewLoopLatch->moveTripCountInfoFrom(OrigLoopLatch);
  VPBlockUtils::insertBlockAfter(NewLoopLatch, OrigLoopLatch);
  VPL->addBasicBlockToLoop(NewLoopLatch, *VPLInfo);
  // Remove the original loop latch from the blocks of the phi node of the loop
  // header.
  updateBlocksPhiNode(LoopHeader, OrigLoopLatch, NewLoopLatch);
  // Add a VPPHINode at the top of the new latch. This phi node shows whether
  // the control-flow reaches the new loop latch from the original loop latch or
  // one of the intermediate blocks or one of the exiting blocks.
  Type *Ty32 = Type::getInt32Ty(*Plan->getLLVMContext());
  Type *Ty1 = Type::getInt1Ty(*Plan->getLLVMContext());
  VPConstant *FalseConst = Plan->getVPConstant(ConstantInt::get(Ty1, 0));
  VPConstant *TrueConst = Plan->getVPConstant(ConstantInt::get(Ty1, 1));
  VPBuilder VPBldr;
  VPBldr.setInsertPoint(NewLoopLatch);
  VPPHINode *ExitIDVPPhi = VPBldr.createPhiInstruction(Ty32, "exit.id.phi");
  // This phi node is a marker of the backedge. It shows if the backedge is
  // taken.
  VPPHINode *NewCondBit =
      VPBldr.createPhiInstruction(Ty1, "take.backedge.cond");
  if (LatchExitBlock) {
    VPInstruction *OldCondBit =
        dyn_cast<VPInstruction>(NewLoopLatch->getCondBit());
    NewCondBit->addIncoming(OldCondBit, cast<VPBasicBlock>(OrigLoopLatch));
  } else {
    assert(BackedgeCond == true &&
           "In while loops, BackedgeCond should be true");
    NewCondBit->addIncoming(TrueConst, cast<VPBasicBlock>(OrigLoopLatch));
  }
  // Update the condbit.
  NewLoopLatch->setCondBit(NewCondBit);
  // We should only mark divergent values. DA checks if a value is in
  // DivergentValues set. If it is not there, then the value is considered
  // uniform.
  if (IsDivergent)
    VPlanDA->markDivergent(*NewCondBit);

  // Add the original loop latch in the NewLoopLatch's phi node.
  VPPHINode *NewLoopHeaderPhiNode =
      updatePhiNodeInLoopHeader(LoopHeader, NewLoopLatch, ExitIDVPPhi, Plan);
  ExitIDVPPhi->addIncoming(NewLoopHeaderPhiNode, cast<VPBasicBlock>(OrigLoopLatch));

  // This is needed for the generation of cascaded if blocks.
  if (LatchExitBlock) {
    VPConstant *ExitIDConst =
        Plan->getVPConstant(ConstantInt::get(Ty32, ExitID));
    ExitBlockIDPairs.push_back(std::make_pair(LatchExitBlock, ExitIDConst));
  }

  // Step 2: Disconnects the exit blocks from the exiting blocks. If it is
  // needed, an intermediate basic block is created. Otherwise, the exiting
  // blocks are connected with the new loop latch.
  SmallPtrSet<VPBlockBase *, 2> VisitedBlocks;
  SmallDenseMap<VPBlockBase *, VPBasicBlock *> ExitBlockNewBlockMap;
  bool phiIsMovedToNewLoopLatch = false;
  // For each exit block (apart from the new loop latch), a new basic block is
  // created and the ExitID is emitted.
  for (VPBlockBase *ExitingBlock : ExitingBlocks) {

    if (ExitingBlock == OrigLoopLatch)
      continue;

    VPBlockBase *ExitBlock = getBlocksExitBlock(ExitingBlock, VPL);
    assert(ExitBlock != nullptr && "Exiting block should have an exit block!");

    // This check is for case 2. In this case, we create a new intermediate
    // basic block only if the exiting block is the loop header. This is needed
    // for correct insertion of phis to make the loop exit condition live
    // through back edge. For any other basic block, we just have to redirect
    // the exiting block to the new loop latch.
    if (ExitBlock == LatchExitBlock) {
      VPBasicBlock *NewExitingBB = nullptr;
      if (ExitingBlock == LoopHeader) {
        NewExitingBB =
            new VPBasicBlock(VPlanUtils::createUniqueName("intermediate.bb"));
        VPL->addBasicBlockToLoop(NewExitingBB, *VPLInfo);
        VPRegionBlock *Parent = LoopHeader->getParent();
        NewExitingBB->setParent(Parent);
        Parent->setSize(Parent->getSize() + 1);
        VPBlockUtils::movePredecessor(LoopHeader, ExitBlock, NewExitingBB);
        NewExitingBB->appendSuccessor(NewLoopLatch);
        NewLoopLatch->appendPredecessor(NewExitingBB);
        // Replace the exiting block with the new exiting block in the exit
        // block's phi node.
        if (hasVPPhiNode(cast<VPBasicBlock>(ExitBlock)) &&
            allPredsInLoop(ExitBlock, VPL))
          updateBlocksPhiNode(cast<VPBasicBlock>(ExitBlock),
                              cast<VPBasicBlock>(ExitingBlock), NewExitingBB);
      } else {
        VPBlockUtils::movePredecessor(ExitingBlock, ExitBlock, NewLoopLatch);
        NewExitingBB = cast<VPBasicBlock>(ExitingBlock);
      }
      ExitID++;
      VPConstant *ExitIDConst =
          Plan->getVPConstant(ConstantInt::get(Ty32, ExitID));
      ExitIDVPPhi->addIncoming(ExitIDConst, NewExitingBB);
      // If the backedge is taken under the true condition, then the edge from
      // the exiting block is taken under the false condition.
      NewCondBit->addIncoming(BackedgeCond ? FalseConst : TrueConst,
                              NewExitingBB);

      // The VPPHINode of the exit block should be moved in the new loop latch
      // if all the predecessors are in the loop. If not, then we remove the
      // exiting block from the basic blocks of the VPPHINode. If more than one
      // blocks of the loop land on the same exit block, then one of them is
      // replaced with new loop latch in the phi node's list and the others are
      // removed from the phi node's list.
      if (hasVPPhiNode(cast<VPBasicBlock>(ExitBlock)) &&
          !phiIsMovedToNewLoopLatch) {
        if (allPredsInLoop(ExitBlock, VPL)) {
          if (VisitedBlocks.count(ExitBlock)) {
            // Remove the exiting block from the exit block's phi node.
            removeBlockFromVPPhiNode(cast<VPBasicBlock>(ExitingBlock),
                                     cast<VPBasicBlock>(ExitBlock));
          } else {
            // Move the phi node of the exit block to the new loop latch.
            moveExitBlocksPhiNode(cast<VPBasicBlock>(ExitBlock), NewLoopLatch);
            phiIsMovedToNewLoopLatch = true;
          }
        } else
          updateBlocksPhiNode(cast<VPBasicBlock>(ExitBlock),
                              cast<VPBasicBlock>(ExitingBlock), NewExitingBB);
      }
    } else {
      // This check ensures that only one Intermediate_BB is created (case 3) in
      // case more than one exiting blocks land on the same exit block.
      if (VisitedBlocks.count(ExitBlock)) {
        VPBasicBlock *NBB = ExitBlockNewBlockMap[ExitBlock];
        VPBlockUtils::movePredecessor(ExitingBlock, ExitBlock, NBB);
        if (hasVPPhiNode(cast<VPBasicBlock>(ExitBlock)))
          removeBlockFromVPPhiNode(cast<VPBasicBlock>(ExitingBlock),
                                   cast<VPBasicBlock>(ExitBlock));
        continue;
      }

      VPBasicBlock *NewBlock =
          new VPBasicBlock(VPlanUtils::createUniqueName("intermediate.bb"));
      VPRegionBlock *Parent = ExitingBlock->getParent();
      NewBlock->setParent(Parent);
      Parent->setSize(Parent->getSize() + 1);
      VPL->addBasicBlockToLoop(NewBlock, *VPLInfo);
      // Remove ExitBlock from ExitingBlock's successors and add a new
      // intermediate block to its successors. ExitBlock's predecessor will be
      // updated after emitting cascaded if blocks.
      VPBlockUtils::movePredecessor(ExitingBlock, ExitBlock, NewBlock);
      NewBlock->appendSuccessor(NewLoopLatch);
      NewLoopLatch->appendPredecessor(NewBlock);

      if (hasVPPhiNode(cast<VPBasicBlock>(ExitBlock)))
        if (allPredsInLoop(ExitBlock, VPL))
          moveExitBlocksPhiNode(cast<VPBasicBlock>(ExitBlock), NewBlock);

      // Add ExitID and update NewLoopLatch's phi node.
      ExitID++;
      VPConstant *ExitIDConst =
          Plan->getVPConstant(ConstantInt::get(Ty32, ExitID));
      ExitIDVPPhi->addIncoming(ExitIDConst, cast<VPBasicBlock>(NewBlock));
      NewCondBit->addIncoming(BackedgeCond ? FalseConst : TrueConst,
                              cast<VPBasicBlock>(NewBlock));
      ExitBlockIDPairs.push_back(std::make_pair(ExitBlock, ExitIDConst));
      ExitExitingBlocksMap[ExitBlock] = ExitingBlock;
      ExitBlockNewBlockMap[ExitBlock] = NewBlock;
    }
    VisitedBlocks.insert(ExitBlock);
  }

  // Step 3 : Creates cascaded if blocks. The head of the cascaded if blocks is
  // emitted after the new loop latch. The cascaded if blocks are not emitted
  // in case 2.

  // We need to collect all the cascaded if blocks in a small vector which will
  // be used in a separate step. Each cascaded if block should be assigned to
  // the correct loop of the loop nest. This does not happen along the
  // generation of the cascaded if blocks, but it happens in a separate step.
  // Because the choice of the correct loop depends on the loop that cascaded if
  // block's successors belong. Therefore, we first need to connect the cascaded
  // if blocks with their successors (other cascaded if blocks, exit blocks) and
  // next, we can decide which is the right loop for each cascaded if block.
  SmallVector<VPBasicBlock *, 2> CascadedIfBlocks;

  if (ExitBlocks.size() > 1) {
    VPBasicBlock *IfBlock =
        new VPBasicBlock(VPlanUtils::createUniqueName("cascaded.if.block"));
    CascadedIfBlocks.push_back(IfBlock);
    // Update the predecessors of the IfBlock.
    if (LatchExitBlock)
      VPBlockUtils::movePredecessor(NewLoopLatch, LatchExitBlock, IfBlock);
    else {
      NewLoopLatch->appendSuccessor(IfBlock);
      IfBlock->appendPredecessor(NewLoopLatch);
    }

    for (int i = 1, end = ExitBlockIDPairs.size(); i != end; ++i) {
      const auto &Pair = ExitBlockIDPairs[i];
      VPBlockBase *ExitBlock = Pair.first;
      VPConstant *ExitID = Pair.second;
      VPInstruction *CondBr =
          new VPCmpInst(ExitIDVPPhi, ExitID, CmpInst::ICMP_EQ);
      IfBlock->appendRecipe(CondBr);
      VPBasicBlock *NextIfBlock = nullptr;
      // Emit cascaded if blocks.
      if (i != end - 1) {
        NextIfBlock =
            new VPBasicBlock(VPlanUtils::createUniqueName("cascaded.if.block"));
        CascadedIfBlocks.push_back(NextIfBlock);
      } else {
        // The current NextIfBlock is the LatchExitBlock.
        NextIfBlock = cast<VPBasicBlock>(ExitBlockIDPairs[0].first);
        if (LatchExitBlock && hasVPPhiNode(cast<VPBasicBlock>(LatchExitBlock)))
          updateBlocksPhiNode(cast<VPBasicBlock>(LatchExitBlock),
                              cast<VPBasicBlock>(OrigLoopLatch), IfBlock);
      }
      // Update the successors of the IfBlock.
      IfBlock->setTwoSuccessors(CondBr, ExitBlock, NextIfBlock);
      // Update the predecessor of the NextIfBlock.
      NextIfBlock->appendPredecessor(IfBlock);
      // Add IfBlock in ExitBlock's predecessors.
      ExitBlock->appendPredecessor(IfBlock);
      // If all the predecessors of the exit block are in the loop, then the phi
      // node is moved in the if block. If not, then we replace the ExitingBlock
      // with the IfBlock in exit block's phi node.
      if (hasVPPhiNode(cast<VPBasicBlock>(ExitBlock))) {
        VPBlockBase *ExitingBlock = ExitExitingBlocksMap[ExitBlock];
        updateBlocksPhiNode(cast<VPBasicBlock>(ExitBlock),
                            cast<VPBasicBlock>(ExitingBlock), IfBlock);
      }
      IfBlock = NextIfBlock;
    }
  } else if (!LatchExitBlock && ExitBlocks.size() == 1) {
    VPBlockBase *ExitBlock = ExitBlocks[0];
    NewLoopLatch->appendSuccessor(ExitBlock);
    ExitBlock->appendPredecessor(NewLoopLatch);
  }

  // Now, we should assign the cascaded if block to the right loop nesting
  // level. The cascaded if block should belong to the same loop nesting level
  // as its successors (exit blocks). Each exit block might belong to different
  // loop nesting level than the other exit block. In this case, the cascaded if
  // block will have the same loop nesting level as the exit block which is
  // deepest in the loop hierarchy.
  while (!CascadedIfBlocks.empty()) {
    // Start from the last cascaded if block because it has two exit blocks.
    VPBasicBlock *CurrentCascadedIfBlock = CascadedIfBlocks.pop_back_val();
    assert(CurrentCascadedIfBlock->getSuccessors().size() == 2 &&
           "Two successors are expected");
    VPLoop *Succ0Loop = VPLInfo->getLoopFor(
        cast<VPBasicBlock>(CurrentCascadedIfBlock->getSuccessors()[0]));
    VPLoop *Succ1Loop = VPLInfo->getLoopFor(
        cast<VPBasicBlock>(CurrentCascadedIfBlock->getSuccessors()[1]));
    unsigned LoopDepthOfSucc0 = Succ0Loop->getLoopDepth();
    unsigned LoopDepthOfSucc1 = Succ1Loop->getLoopDepth();
    // The bigger the loop depth the deeper the loop is in the loop nest.
    VPLoop *CascadedIfBlockParentLoop =
        LoopDepthOfSucc0 >= LoopDepthOfSucc1 ? Succ0Loop : Succ1Loop;
    VPRegionBlock *ParentRegion = NewLoopLatch->getParent();
    CurrentCascadedIfBlock->setParent(ParentRegion);
    ParentRegion->setSize(ParentRegion->getSize() + 1);
    CascadedIfBlockParentLoop->addBasicBlockToLoop(CurrentCascadedIfBlock,
                                                   *VPLInfo);
  }

  VPRegionBlock *CurrentLoopRegion =
      cast<VPRegionBlock>(VPL->getHeader()->getParent());
  VPDomTree.recalculate(*CurrentLoopRegion);
  VPPostDomTree.recalculate(*CurrentLoopRegion);

  VPRegionBlock *ParentLoopRegion =
      cast<VPRegionBlock>(ParentLoop->getHeader()->getParent());
  VPDomTree.recalculate(*ParentLoopRegion);
  VPPostDomTree.recalculate(*ParentLoopRegion);

  assert(
      VPL->getExitingBlock() == NewLoopLatch &&
      "Only 1 exiting block is expeted after merge loop exits transformation");

  LLVM_DEBUG(dbgs() << "After merge loop exits transformation.\n");
  LLVM_DEBUG(Plan->dump());
}

// The following example shows why it is needed to preserve SSA. x is defined in
// BB2 and it is used in BB1. After single-exit while loop transformation, a new
// edge is added between BB1 and the NEW_LOOP_LATCH. This means that the value
// of x can end up to the NEW_LOOP_LATCH from multiple paths. For this reason, a
// new phi node is emitted in the NEW_LOOP_LATCH.
// -----------------------------------------------------------------------------
//  BEFORE                                          AFTER
// -----------------------------------------------------------------------------
//
// -->BB1                               ------------>BB1
// | x_phi1 = [x, BB3], [0, PreHeader]  | x_phi1 = [x_phi2, BB3], [0, PreHeader]
// | ...=x                              |           ...=x_phi1
// |   |\                               |             |\
// |   | \                              |             | \
// |  BB2 \       =>                    |            BB2 \
// | x=... \                            |           x=... \
// |   |    \                           |             |    \
// |   |     \                          |             |     \
// |   |     BB4                        |             |      |
// ---BB3                               |            BB3     |
//                                      |             |      |
//                                      |             |      |
//                                      ---------NEW_LOOP_LATCH
//                                        x_phi2 = [x, BB3] [undef, BB1]
//                                                    |
//                                                    |
//                                                   BB4
//
// TODO: Update preserveSSAForLoopHeader to work with IDF.
void VPlanHCFGBuilder::preserveSSAForLoopHeader(VPBasicBlock *LoopHeader,
                                                VPBasicBlock *NewLoopLatch,
                                                VPBasicBlock *OrigLoopLatch,
                                                VPBasicBlock *ExitingBlock) {
  VPBuilder VPBldr;
  VPBldr.setInsertPoint(NewLoopLatch);

  assert(llvm::is_contained(LoopHeader->getPredecessors(), NewLoopLatch) &&
         "NewLoopLatch must be LoopHeader's predecessor!");

  for (VPPHINode &VPPhi : LoopHeader->getVPPhis()) {
    auto *IncomingValue =
        dyn_cast<VPInstruction>(VPPhi.getIncomingValue(NewLoopLatch));
    // Check if we have to generate a phi node for the current incoming value at
    // NewLoopLatch.
    if (!IncomingValue ||
        VPDomTree.dominates(IncomingValue->getParent(), ExitingBlock))
      continue;

    // Create a new phi node in the new loop latch for the values that are used
    // in the loop header.
    VPPHINode *PreserveSSAPhi =
        VPBldr.createPhiInstruction(IncomingValue->getType());
    // Update the phi nodes of the loop header with the new phi.
    VPPhi.setIncomingValue(NewLoopLatch, PreserveSSAPhi);

    // Update the new phi node of the new loop latch.
    PreserveSSAPhi->addIncoming(IncomingValue,
                                dyn_cast<VPBasicBlock>(OrigLoopLatch));
    PreserveSSAPhi->addIncoming(
        Plan->getVPConstant(UndefValue::get(IncomingValue->getType())),
        cast<VPBasicBlock>(ExitingBlock));
    VPlanDivergenceAnalysis *VPlanDA = Plan->getVPlanDA();
    // Update the DA of the SSA phi based on the DA info that the
    // condition bit has.
    if (VPlanDA->isDivergent(*IncomingValue))
      VPlanDA->markDivergent(*PreserveSSAPhi);
  }
}

bool VPlanHCFGBuilder::isBreakingSSA(VPLoop *VPL) {
  auto *VPLI = Plan->getVPLoopInfo();
  SmallVector<VPBlockBase *, 2> ExitingBlocks;
  VPL->getExitingBlocks(ExitingBlocks);

  auto *Header = VPL->getHeader();
  VPBasicBlock *LoopLatch = dyn_cast<VPBasicBlock>(VPL->getLoopLatch());
  if (!LoopLatch)
    return true;

  for (VPBlockBase *BB : VPL->getBlocks()) {
    if (VPLI->getLoopFor(BB) != VPL)
      continue; // Inner loops already handled.

    if (VPDomTree.dominates(BB, LoopLatch) &&
        all_of(ExitingBlocks, [&](VPBlockBase *ExitingBlock) {
            return VPDomTree.dominates(BB, ExitingBlock);
        }))
      // Defs in this block will be available in NewLoopLatch.
      continue;

    auto *BasicBlock = dyn_cast<VPBasicBlock>(BB);
    if (!BasicBlock)
      // We are operation on PlainCFG, so this shouldn't happen. Just be
      // conservative if that's not true for some reason.
      return true;

    for (auto &Inst : BasicBlock->vpinstructions()) {
      for (auto *User : Inst.users()) {
        auto *UserInst = dyn_cast<VPInstruction>(User);
        if (!UserInst)
          return true;

        if (!VPL->contains(UserInst))
          return true;

        if (UserInst->getParent() == Header) {
          assert(isa<VPPHINode>(UserInst) &&
                 "Can't have non-phi user in header!");
          return true;
        }
      }
    }
  }

  return false;
}

// The single-exit while tranformation creates a new loop latch where the
// side-exit is redirected.
// -------------------------------------------
//  BEFORE                         AFTER
// -------------------------------------------
//
// -->BB1                ----------->BB1
// |   |\               |            |\
// |   | \              |            | \
// |  BB2 \       =>    |           BB2 \
// |   |   \            |            |   \
// |   |    \           |            |    \
// |   |    BB4         |            |     |
// ---BB3               |           BB3    |
//                      |            |     |
//                      |            |     |
//                       --------NEW_LOOP_LATCH
//                                   |
//                                   |
//                                  BB4
//
void VPlanHCFGBuilder::singleExitWhileLoopCanonicalization(VPLoop *VPL) {

  VPBasicBlock *OrigLoopLatch = dyn_cast<VPBasicBlock>(VPL->getLoopLatch());
  if (OrigLoopLatch->getNumSuccessors() > 1)
    return;

  VPLoop *ParentLoop = VPL->getParentLoop();
  if (!VPL->getExitingBlock() || ParentLoop == nullptr)
    return;

  // FIXME: Don't break SSA form during the transformation.
  if (!Plan->isFullLinearizationForced() // Already marked, so...
      && isBreakingSSA(VPL))             // don't try to analyze any more.
    Plan->markFullLinearizationForced();

  LLVM_DEBUG(dbgs() << "Before single exit while loop transformation.\n");
  LLVM_DEBUG(Plan->dump());

  // Create new loop latch
  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();
  VPBasicBlock *NewLoopLatch = VPBlockUtils::splitBlock(
      OrigLoopLatch, VPLInfo, VPDomTree, VPPostDomTree);
  NewLoopLatch->setName(VPlanUtils::createUniqueName("new.loop.latch"));
  // Update the control-flow for the ExitingBlock, the NewLoopLatch and the
  // ExitBlock.
  VPBlockBase *ExitingBlock = VPL->getExitingBlock();
  VPBlockBase *ExitBlock = getBlocksExitBlock(ExitingBlock, VPL);
  assert(ExitBlock && "Exiting block should have an exit block!");
  VPBlockUtils::movePredecessor(ExitingBlock, ExitBlock, NewLoopLatch);
  VPBlockUtils::connectBlocks(NewLoopLatch, ExitBlock);
  // Update the blocks of the phi node (if it exists) in the exit block.
  updateBlocksPhiNode(cast<VPBasicBlock>(ExitBlock),
                      cast<VPBasicBlock>(ExitingBlock), NewLoopLatch);

  // Fill-in NewLoopLatch with new instructions.
  VPBasicBlock *LoopHeader = dyn_cast<VPBasicBlock>(VPL->getHeader());
  // Emit phi nodes that will preserve SSA form if it is needed.
  preserveSSAForLoopHeader(LoopHeader, NewLoopLatch, OrigLoopLatch,
                           cast<VPBasicBlock>(ExitingBlock));
  // Emit the backedge condition.
  Type *Int1Ty = Type::getInt1Ty(*Plan->getLLVMContext());
  VPConstant *FalseConst = Plan->getVPConstant(ConstantInt::get(Int1Ty, 0));
  VPConstant *TrueConst = Plan->getVPConstant(ConstantInt::get(Int1Ty, 1));
  VPBuilder VPBldr;
  VPBldr.setInsertPoint(NewLoopLatch);
  VPPHINode *TakeBackedgeCond =
      VPBldr.createPhiInstruction(Int1Ty, "TakeBackedgeCond");
  // TODO: The while-loop canonicalization does not preserve the SSA form. If a
  // value from the OrigLoopLatch feeds the phi in the BB1, then this value
  // stops dominating its use (not defined on the BB2->NEW_LOOP_LATCH edge).
  // Currently, this is not a problem because the proper dominance will be
  // restored after CFG linearization by predicator.
  TakeBackedgeCond->addIncoming(TrueConst, cast<VPBasicBlock>(OrigLoopLatch));
  TakeBackedgeCond->addIncoming(FalseConst, cast<VPBasicBlock>(ExitingBlock));
  NewLoopLatch->setCondBit(TakeBackedgeCond);
  // Update DA.
  VPlanDivergenceAnalysis *VPlanDA = Plan->getVPlanDA();
  VPInstruction *OldCondBit =
      dyn_cast_or_null<VPInstruction>(ExitingBlock->getCondBit());
  assert(OldCondBit && "ExitingBlock does not have a CondBit\n");
  // We should only mark divergent values. DA checks if a value is in
  // DivergentValues set. If it is not there, then the value is considered
  // uniform.
  if (VPlanDA->isDivergent(*OldCondBit))
    VPlanDA->markDivergent(*TakeBackedgeCond);

  // TODO: CMPLRLLVM-9535 Update VPDomTree and VPPostDomTree instead of
  // recalculating it.
  VPRegionBlock *CurrentLoopRegion =
      cast<VPRegionBlock>(VPL->getHeader()->getParent());
  VPDomTree.recalculate(*CurrentLoopRegion);
  VPPostDomTree.recalculate(*CurrentLoopRegion);

  VPRegionBlock *ParentLoopRegion =
      cast<VPRegionBlock>(ParentLoop->getHeader()->getParent());
  VPDomTree.recalculate(*ParentLoopRegion);
  VPPostDomTree.recalculate(*ParentLoopRegion);

  assert(VPL->getExitingBlock() == NewLoopLatch &&
         "Only 1 exiting block is expeted after single-exit while loop "
         "canonicalization");

  LLVM_DEBUG(dbgs() << "After single exit while loop transformation.\n");
  LLVM_DEBUG(Plan->dump());
}

#if INTEL_CUSTOMIZATION
// Return the nearest common post dominator of all the VPBlockBases in \p
// InputVPBlocks.
static VPBlockBase *getNearestCommonPostDom(
    const VPPostDominatorTree &VPPostDomTree,
    const SmallVectorImpl<VPBlockBase *> &InputVPBlocks) {
  assert(InputVPBlocks.size() > 0 && "Expected at least one input block!");
  VPBlockBase *NearestDom = *InputVPBlocks.begin();

  if (InputVPBlocks.size() == 1)
    return NearestDom;

  for (auto *InputVPB : InputVPBlocks) {
    NearestDom = VPPostDomTree.findNearestCommonDominator(InputVPB, NearestDom);
    assert(NearestDom && "Nearest post dominator can't be null!");
  }

  return NearestDom;
}
#endif // INTEL_CUSTOMIZATION

// Split loops' exit block that are not in canonical form
void VPlanHCFGBuilder::splitLoopsExit(VPLoop *VPL) {

  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

#if INTEL_CUSTOMIZATION
  SmallVector<VPBlockBase *, 4> LoopExits;
  VPL->getUniqueExitBlocks(LoopExits);
  // If loop has single exit, the actual block to be potentially split is the
  // single exit. If loop has multiple exits, the actual block to be potentially
  // split is the common landing pad (nearest post dom) of all the exits.
  // TODO: If the CFG after the loop exits gets more complicated, we can get
  // the common post-dom block of all the exits.
  VPBlockBase *Exit = getNearestCommonPostDom(VPPostDomTree, LoopExits);
#else
  VPBlockBase *Exit = VPL->getUniqueExitBlock();
  assert(Exit && "Only single-exit loops expected");
#endif // INTEL_CUSTOMIZATION

  // Split loop exit with multiple successors or that is preheader of another
  // loop
  VPBlockBase *PotentialH = Exit->getSingleSuccessor();
  if (!PotentialH ||
      (VPLInfo->isLoopHeader(PotentialH) &&
       VPLInfo->getLoopFor(PotentialH)->getLoopPreheader() == Exit))
    VPBlockUtils::splitBlock(Exit, VPLInfo, VPDomTree, VPPostDomTree);

  // Apply simplification to subloops
  for (auto VPSL : VPL->getSubLoops()) {
    splitLoopsExit(VPSL);
  }
}

// Split basic blocks to increase the number of non-loop regions detected during
// the construction of the hierarchical CFG.
void VPlanHCFGBuilder::simplifyNonLoopRegions() {

  assert(isa<VPRegionBlock>(Plan->getEntry()) &&
         "VPlan entry is not a VPRegionBlock");
  VPRegionBlock *TopRegion = cast<VPRegionBlock>(Plan->getEntry());

  SmallVector<VPBlockBase *, 32> WorkList;
  SmallPtrSet<VPBlockBase *, 32> Visited;

  WorkList.push_back(TopRegion->getEntry());

  while (!WorkList.empty()) {

    // Get Current and skip it if visited.
    VPBlockBase *CurrentBlock = WorkList.back();
    WorkList.pop_back();
    if (Visited.count(CurrentBlock))
      continue;

    // Set Current to visited
    Visited.insert(CurrentBlock);

    // Potential VPRegion entry
    if (CurrentBlock->getNumSuccessors() > 1) {

      // Currently, this rule covers:
      //   - Loop H with multiple successors
      //   - Region exit that is another region entry
      //   - Loop latch+exiting block with multiple predecessors
      //
      // TODO: skip single basic block loops?
      if (CurrentBlock->getNumPredecessors() > 1) {
        VPBlockUtils::splitBlock(CurrentBlock, Plan->getVPLoopInfo(), VPDomTree,
                                 VPPostDomTree);
      }

      // TODO: WIP. The code below has to be revisited. It will enable the
      // construction of VPRegions that currently are not built because they
      // share entry/exit nodes with other VPRegions. This transformation would
      // require to introduce new recipes to split original phi instructions
      // that are in the problematic basic blocks.

      // VPBlockBase *PostDom =
      //    PostDomTree.getNode(CurrentBlock)->getIDom()->getBlock();
      // VPBlockBase *Dom = DomTree.getNode(PostDom)->getIDom()->getBlock();
      // assert(isa<VPBasicBlock>(PostDom) &&
      //       "Expected VPBasicBlock as post-dominator");
      // assert(isa<VPBasicBlock>(Dom) && "Expected VPBasicBlock as dominator");

      // TODO: This condition is currently too generic. It needs refinement.
      // However, if detecting more specific cases is expensive, we may want to
      // leave as it is.
      //
      // When we need to insert a fake exit block:
      //   - PostDom is exit of a region and entry of another region (PostDom
      //   numSucc > 1)
      //   - Dom != CurrentBlock:
      //       - Nested region shares exit with parent region. We need a fake
      //       exit for nested region to be created. With fake exit, Dom ==
      //       CurrentBlock
      //       - Dom != CurrentBlock even if we introduce the fake exit. We
      //       won't create region for these cases so we don't want to introduce
      //       fake exit. (TODO: We are currently introducing fake exit for this
      //       case).
      //       - Loops with multiple exiting blocks and region sharing exit
      //       (TODO)
      //       - Anything else?
      //
      // if (Dom != CurrentBlock || PostDom->getNumSuccessors() > 1) {

      //  // New fake exit
      //  VPBasicBlock *FakeExit = PlanUtils.createBasicBlock();
      //  PlanUtils.setBlockParent(FakeExit, TopRegion);

      //  // Set Predecessors
      //  if (Dom != CurrentBlock) {
      //    // Move only those predecessors from PostDom that are part of the
      //    // nested region (i.e. they are dominated by Dom)
      //    for (auto Pred : PostDom->getPredecessors()) {
      //      if (DomTree.dominates(Dom, Pred)) {
      //        PlanUtils.movePredecessor(Pred, PostDom /*From*/,
      //                                  FakeExit /*To*/);
      //      }
      //    }
      //  } else {
      //    // All the predecessors will be in the same region. Move them all
      //    from
      //    // PostDom to FakeExit
      //    PlanUtils.movePredecessors(PostDom, FakeExit);
      //  }

      //  // Add PostDom as single successor
      //  PlanUtils.setSuccessor(FakeExit, PostDom);

      //}
    }

    // Add successors to the worklist
    for (VPBlockBase *Succ : CurrentBlock->getSuccessors())
      WorkList.push_back(Succ);
  }
}

// Main function that canonicalizes the plain CFG and applyies transformations
// that enable the detection of more regions during the hierarchical CFG
// construction.
void VPlanHCFGBuilder::simplifyPlainCFG() {

  assert(isa<VPRegionBlock>(Plan->getEntry()) &&
         "VPlan entry is not a VPRegionBlock");
  VPRegionBlock *TopRegion = cast<VPRegionBlock>(Plan->getEntry());
  (void)TopRegion;
  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

  assert((VPLInfo->size() == 1) && "Expected only 1 top-level loop");
  VPLoop *TopLoop = *VPLInfo->begin();

  splitLoopsPreheader(TopLoop);

  LLVM_DEBUG(dbgs() << "Dominator Tree Before mergeLoopExits\n";
             VPDomTree.print(dbgs()));

  if (LoopMassagingEnabled) {
    for (auto *VPL : post_order(TopLoop)) {
      singleExitWhileLoopCanonicalization(VPL);
      mergeLoopExits(VPL);
      LLVM_DEBUG(Verifier->verifyHierarchicalCFG(Plan, TopRegion));
    }
#if INTEL_CUSTOMIZATION
    if (VPlanPrintAfterLoopMassaging) {
      errs() << "Print after loop massaging:\n";
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      Plan->dump();
#endif // !NDEBUG || LLVM_ENABLE_DUMP
    }
#endif /* INTEL_CUSTOMIZATION */

    LLVM_DEBUG(dbgs() << "Dominator Tree After mergeLoopExits\n";
               VPDomTree.print(dbgs()));
  }

  splitLoopsExit(TopLoop);
  simplifyNonLoopRegions();
}

// Create new LoopRegion's using VPLoopInfo analysis and introduce them into the
// hierarchical CFG. This function doesn't traverse the whole CFG and region's
// size and block's parent are not properly updated. They are updated in
// buildNonLoopRegions.
void VPlanHCFGBuilder::buildLoopRegions() {

  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

  // Auxiliary function that implements the main functionality of
  // buildLoopRegions
  std::function<void(VPLoop *)> buildLoopRegionsImpl = [&](VPLoop *VPL) {

    // Create new loop region
    VPLoopRegion *VPLR = createLoopRegion(VPL);

    // Set VPLoop's entry and exit.
    // Entry = loop preheader, Exit = loop single exit
#if INTEL_CUSTOMIZATION
    // (or nearest common post dom for multi-exit loops).
#endif
    VPBlockBase *RegionEntry = VPL->getLoopPreheader();
    assert(RegionEntry && isa<VPBasicBlock>(RegionEntry) &&
           "Unexpected loop preheader");
#if INTEL_CUSTOMIZATION
    SmallVector<VPBlockBase *, 4> LoopExits;
    VPL->getUniqueExitBlocks(LoopExits);
    VPBasicBlock *RegionExit =
        cast<VPBasicBlock>(getNearestCommonPostDom(VPPostDomTree, LoopExits));
#else
    assert(VPL->getUniqueExitBlock() && "Only single-exit loops expected");
    VPBasicBlock *RegionExit = cast<VPBasicBlock>(VPL->getUniqueExitBlock());
#endif // INTEL_CUSTOMIZATION

    LLVM_DEBUG(dbgs() << "Creating new VPLoopRegion " << VPLR->getName() << "\n"
                      << "   Entry: " << RegionEntry->getName() << "\n"
                      << "   Exit: " << RegionExit->getName() << "\n");

    // Connect loop region to graph
    VPBlockUtils::insertRegion(VPLR, RegionEntry, RegionExit,
                               false /*recomputeSize*/);

    // Update VPLoopInfo. Add new VPLoopRegion to region entry's loop (loop PH)
    // which, as expected, is not contained in this VPLoopRegion's VPLoop.
    if (VPLoop *Loop = VPLInfo->getLoopFor(RegionEntry)) {
      Loop->addBasicBlockToLoop(VPLR, *VPLInfo);
    }

    // Recursively build loop regions inside this loop
    for (VPLoop *SubVPL : VPL->getSubLoops())
      buildLoopRegionsImpl(SubVPL);
  };

  LLVM_DEBUG(dbgs() << "Building LoopRegion's\n");

  for (VPLoop *VPL : make_range(VPLInfo->begin(), VPLInfo->end()))
    buildLoopRegionsImpl(VPL);
}

// Create new non-loop VPRegionBlock's and update the information of all the
// blocks in the hierarchical CFG. The hierarchical CFG is stable and contains
// consisten information after this step.
void VPlanHCFGBuilder::buildNonLoopRegions(VPRegionBlock *ParentRegion) {

  VPLoopInfo *VPLInfo = Plan->getVPLoopInfo();

  LLVM_DEBUG(
      dbgs() << "Building Non-Loop Regions for " << ParentRegion->getName()
             << "\n"
             << "   Entry: " << ParentRegion->getEntry()->getName() << "\n"
             << "   Exit: " << ParentRegion->getExit()->getName() << "\n");

  SmallVector<VPBlockBase *, 16> WorkList;
  SmallPtrSet<VPBlockBase *, 16> Visited;
  WorkList.push_back(ParentRegion->getEntry());

  unsigned ParentSize = 0;
  bool ParentIsDivergent = false;

  while (!WorkList.empty()) {

    // Get Current and skip it if visited.
    VPBlockBase *Current = WorkList.back();
    WorkList.pop_back();
    if (Visited.count(Current))
      continue;

    Visited.insert(Current);
    LLVM_DEBUG(dbgs() << "Visiting " << Current->getName()
                      << "(Entry: " << Current->getEntryBasicBlock() << ")"
                      << "\n";);

    // If you hit this assert, the input CFG is very likely to be not compliant
    // either because it contains a loop that is not supported or because loops
    // are not in canonical form.
    assert((isa<VPLoopRegion>(Current) || isa<VPBasicBlock>(Current)) &&
           "Expected VPBasicBlock or VPLoopRegion");

    // Increase ParentRegion's size
    ++ParentSize;

    // Pointer to a new subregion or existing VPLoopRegion subregion
    VPRegionBlock *SubRegion = dyn_cast<VPLoopRegion>(Current);
    VPBlockBase *RegionExit;

    // Non-loop VPRegion detection.
    if (!DisableNonLoopSubRegions && !SubRegion /* Skip VPLoopRegions */ &&
        isNonLoopRegion(Current, ParentRegion, RegionExit /*output*/)) {

      // Create new region and connect it to graph
      SubRegion = new VPRegionBlock(VPBlockBase::VPRegionBlockSC,
                                    VPlanUtils::createUniqueName("region"));

      LLVM_DEBUG(dbgs() << "Creating new VPRegion " << SubRegion->getName()
                        << "\n"
                        << "   Entry: " << Current->getName() << "\n"
                        << "   Exit: " << RegionExit->getName() << "\n");
      assert(RegionExit && "RegionExit cannot be null");

      VPBlockUtils::insertRegion(SubRegion, Current /*Entry*/, RegionExit,
                                 false /*recomputeSize*/);

      // Add new region to VPLoopInfo.
      if (VPLoop *Loop = VPLInfo->getLoopFor(SubRegion->getEntry())) {
        Loop->addBasicBlockToLoop(SubRegion, *VPLInfo);
      }
    }

    // New region was built or Current is a LoopRegion.
    if (SubRegion) {
      // Set SubRegion's parent
      SubRegion->setParent(ParentRegion);

      // Add SubRegion's successors to worklist.
      for (auto Succ : SubRegion->getSuccessors()) {
        LLVM_DEBUG(dbgs() << "Adding " << Succ->getName() << " to WorkList"
                          << "\n");
        WorkList.push_back(Succ);
      }

      // Recursively build non-regions inside subregion
      buildNonLoopRegions(SubRegion);

    } else {
      // Current is a VPBasicBlock that didn't trigger the creation of a new
      // region.

      // Set Current's parent
      Current->setParent(ParentRegion);

      // Check if Current causes parent region to be divergent.
      ParentIsDivergent |= isDivergentBlock(Current);

      // No new region has been detected. Add Current's successors.
      for (auto Succ : Current->getSuccessors()) {
        LLVM_DEBUG(dbgs() << "Adding " << Succ->getName() << " to WorkList"
                          << "\n");
        WorkList.push_back(Succ);
      }
    }
  }

  ParentRegion->setSize(ParentSize);
  ParentRegion->setDivergent(ParentIsDivergent);

  LLVM_DEBUG(dbgs() << "End of HCFG build for " << ParentRegion->getName()
                    << "\n");
}


// Go through the blocks in Region, collecting uniforms.
void VPlanHCFGBuilder::collectUniforms(VPRegionBlock *Region) {
  for (VPBlockBase *Block :
       make_range(df_iterator<VPRegionBlock *>::begin(Region),
                  df_iterator<VPRegionBlock *>::end(Region))) {
    if (auto *VPBB = dyn_cast<VPBasicBlock>(Block)) {
      if (Block->getNumSuccessors() >= 2) {
        // Multiple successors. Checking uniformity of Condition Bit
        // Instruction.
        VPValue *CBV = VPBB->getCondBit();
        assert(CBV && "Expected condition bit value.");

        bool isUniform = Legal->isUniformForTheLoop(CBV->getUnderlyingValue());
        if (isUniform)
          Plan->UniformCBVs.insert(CBV);
      }
    }
  }
}

static TripCountInfo readIRLoopMetadata(Loop *Lp) {
  TripCountInfo TCInfo;
  MDNode *LoopID = Lp->getLoopID();
  if (!LoopID)
    // Default construct to trigger usage of the default estimated trip count
    // later.
    return TCInfo;

  for (const MDOperand &MDOp : LoopID->operands()) {
    const auto *MD = dyn_cast<MDNode>(MDOp);
    if (!MD)
      continue;
    const auto *S = dyn_cast<MDString>(MD->getOperand(0));
    if (!S)
      continue;

    auto ExtractValue = [S, MD](auto &TCInfoField, StringRef MetadataName,
                                StringRef ReadableString) -> void {
      (void)ReadableString;
      if (S->getString().equals(MetadataName))
        TCInfoField =
            mdconst::extract<ConstantInt>(MD->getOperand(1))->getZExtValue();
      LLVM_DEBUG(dbgs() << ReadableString << " trip count is " << TCInfoField
                        << " set by pragma loop count.\n";);
    };
    ExtractValue(TCInfo.MaxTripCount, "llvm.loop.intel.loopcount_maximum",
                 "Max");
    ExtractValue(TCInfo.MinTripCount, "llvm.loop.intel.loopcount_minimum",
                 "Min");
    ExtractValue(TCInfo.TripCount, "llvm.loop.intel.loopcount_average",
                 "Average");
  }

  return TCInfo;
}

void VPlanHCFGBuilder::populateVPLoopMetadata(VPLoopInfo *VPLInfo) {
  for (VPLoop *VPL : VPLInfo->getLoopsInPreorder()) {
    auto *VPLatch = cast_or_null<VPBasicBlock>(VPL->getLoopLatch());
    assert(VPLatch && "No dedicated latch!");
    BasicBlock *Latch = VPLatch->getOriginalBB();
    assert(Latch && "Loop massaging happened before VPLoop's creation?");
    Loop *Lp = LI->getLoopFor(Latch);
    assert(Lp &&
           "VPLoopLatch does not correspond to Latch, massaging happened?");
    TripCountInfo TCInfo = readIRLoopMetadata(Lp);
    TCInfo.calculateEstimatedTripCount();
    VPL->setTripCountInfo(TCInfo);
  }
}

void VPlanHCFGBuilder::buildHierarchicalCFG() {

  VPLoopEntityConverterList CvtVec;

  // Build Top Region enclosing the plain CFG
  Plan->setEntry(buildPlainCFG(CvtVec));
  VPRegionBlock *TopRegion = Plan->getEntry();

  // Collect divergence information
  collectUniforms(TopRegion);

  LLVM_DEBUG(Plan->setName("HCFGBuilder: Plain CFG\n"); dbgs() << *Plan);
  LLVM_DEBUG(Verifier->verifyHierarchicalCFG(Plan, TopRegion));

  // Compute dom tree for the plain CFG for VPLInfo. We don't need post-dom tree
  // at this point.
  VPDomTree.recalculate(*TopRegion);
  LLVM_DEBUG(dbgs() << "Dominator Tree After buildPlainCFG\n";
             VPDomTree.print(dbgs()));

  // TODO: If more efficient, we may want to "translate" LoopInfo to VPLoopInfo.
  // Compute VPLInfo and keep it in VPlan
  Plan->setVPLoopInfo(std::make_unique<VPLoopInfo>());
  auto *VPLInfo = Plan->getVPLoopInfo();
  VPLInfo->analyze(VPDomTree);
  populateVPLoopMetadata(VPLInfo);

  // LLVM_DEBUG(dbgs() << "Loop Info:\n"; LI->print(dbgs()));
  LLVM_DEBUG(dbgs() << "VPLoop Info After buildPlainCFG:\n";
             VPLInfo->print(dbgs()));

  passEntitiesToVPlan(CvtVec);
  // Remove any duplicate induction PHIs collected during importing
  Plan->getOrCreateLoopEntities(*VPLInfo->begin())
      ->replaceDuplicateInductionPHIs();

  // Compute postdom tree for the plain CFG.
  VPPostDomTree.recalculate(*TopRegion);
  LLVM_DEBUG(dbgs() << "PostDominator Tree After buildPlainCFG:\n";
             VPPostDomTree.print(dbgs()));

#if INTEL_CUSTOMIZATION
  // simplifyPlainCFG inserts empty blocks with CondBit recipes. This messes up
  // determining the influence region of a branch instruction. i.e., the
  // immediate post-dominator becomes this empty block instead of the actual
  // convergence point containing the phi. Running DA here allows reuse of the
  // current Dominator Trees and results in fewer modifications to the DA
  // algorithm since it was designed to run over a plain CFG. We should also
  // be able to leverage DA for use in the inner loop control flow uniformity
  // massaging for outer loop vectorization (done in simplifyPlainCFG). That
  // way, we only have to transform the CFG for inner loops known to be non-
  // uniform.
  // TODO: Right now DA is computed per VPlan for the outermost loop of the
  // VPlan region. We will need additional information provided to DA if we wish
  // to vectorize more than one loop, or vectorize a specific loop within the
  // VPlan that is not the outermost one.
  // TODO: Check to see how this ordering impacts loops with multiple exits in
  // mergeLoopExits(). It's possible that we may want to delay DA from running
  // until after loops with multiple exits are canonicalized to a single loop
  // exit. But, this means that the DA algorithm will have to be changed to have
  // to deal with empty loop pre-header blocks unless we can run mergeLoopExits
  // before the empty pre-header blocks are inserted.
  if (!DisableVPlanDA) {
    // TODO: Determine if we want to have a separate DA instance for each VF.
    // Currently, there is only one instance and no distinction between VFs.
    // i.e., values are either uniform or divergent for all VFs.
    VPLoop *CandidateLoop = *VPLInfo->begin();
    auto VPDA = std::make_unique<VPlanDivergenceAnalysis>();
    VPDA->compute(Plan, CandidateLoop, VPLInfo, VPDomTree, VPPostDomTree, true);
    Plan->setVPlanDA(std::move(VPDA));
  }

  if (VPlanPrintPlainCFG) {
    errs() << "Print after buildPlainCFG\n";
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    Plan->dump();
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  }
#endif /* INTEL_CUSTOMIZATION */

  // Prepare/simplify CFG for hierarchical CFG construction
  simplifyPlainCFG();

#if INTEL_CUSTOMIZATION
  if (VPlanPrintSimplifyCFG) {
    errs() << "Print after simplify plain CFG\n";
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    Plan->dump();
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  }
#endif

  LLVM_DEBUG(Plan->setName("HCFGBuilder: After simplifyPlainCFG\n");
             dbgs() << *Plan);
  LLVM_DEBUG(dbgs() << "Dominator Tree After simplifyPlainCFG\n";
             VPDomTree.print(dbgs()));
  LLVM_DEBUG(dbgs() << "PostDominator Tree After simplifyPlainCFG:\n";
             VPPostDomTree.print(dbgs()));
  LLVM_DEBUG(dbgs() << "VPLoop Info After simplifyPlainCFG:\n";
             VPLInfo->print(dbgs()));

#if INTEL_CUSTOMIZATION
  LLVM_DEBUG(Verifier->verifyHierarchicalCFG(Plan, TopRegion));
#else
  LLVM_DEBUG(Verifier->verifyHierarchicalCFG(TopRegion));
#endif

  // Build hierarchical CFG in two step: buildLoopRegions and
  // buildNonLoopRegions. There are two important things to notice:
  //    1. Regions' size and blocks' parent are not consistent after
  //       buildLoopRegions. buildLoopRegions doesn't require to traverse the
  //       CFG. It's more effient to recompute this information while traversing
  //       the CFG in buildNonLoopRegions.
  //    2. Dom/Postdom trees for the plain CFG are no longer valid after
  //       buildLoopRegions (there is no plain CFG anymore). However, we can
  //       still use them to build non-loop regions.
  //
  buildLoopRegions();
  buildNonLoopRegions(TopRegion);

  LLVM_DEBUG(Plan->setName("HCFGBuilder: After building HCFG\n");
             dbgs() << *Plan;);

#if INTEL_CUSTOMIZATION
  if (VPlanPrintHCFG) {
    errs() << "Print after building H-CFG:\n";
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    Plan->dump();
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  }
  LLVM_DEBUG(Plan->dumpLivenessInfo(dbgs()));

  LLVM_DEBUG(Verifier->setVPLoopInfo(VPLInfo);
             Verifier->verifyHierarchicalCFG(Plan, TopRegion));
#else
  LLVM_DEBUG(Verifier->setVPLoopInfo(VPLInfo);
             Verifier->verifyHierarchicalCFG(TopRegion));
#endif
}

// Return true if a non-loop region can be formed from \p Entry. If so, \p Exit
// returns region's exit for the detected region.
bool VPlanHCFGBuilder::isNonLoopRegion(VPBlockBase *Entry,
                                       VPRegionBlock *ParentRegion,
                                       VPBlockBase *&Exit) const {

  // Region's entry must have multiple successors and must be a VPBasicBlock at
  // this point. Also skip ParentRegion's Entry to prevent infinite recursion
  if (Entry == ParentRegion->getEntry() || Entry->getNumPredecessors() != 1 ||
      Entry->getNumSuccessors() < 2 || !isa<VPBasicBlock>(Entry))
    return false;

  VPBlockBase *PotentialExit =
      VPPostDomTree.getNode(Entry)->getIDom()->getBlock();
  // For now, assert when the exit cannot be found for the entry block. Later,
  // we may want to return false here so that a new region is not constructed
  // for these cases. But, this will require a little further study.
  assert(PotentialExit && "Could not find the Exit block for the region Entry");
  // Region's exit must have a single successor
  if (PotentialExit->getNumSuccessors() != 1 ||
      !isa<VPBasicBlock>(PotentialExit) ||
      // TODO: Temporal check to skip regions that share exit node with parent
      // region.
      ParentRegion->getExit() == PotentialExit)
    return false;

  VPBlockBase *Dom = VPDomTree.getNode(PotentialExit)->getIDom()->getBlock();
  if (Dom != Entry ||
      !regionIsBackEdgeCompliant(Entry, PotentialExit, ParentRegion))
    return false;

  Exit = PotentialExit;
  return true;
}

// This is a temporal implementation to detect and discard non-loop regions
// whose entry and exit blocks are in different graph cycles. At this point, the
// only cycles we have to care about are those created by loop latches. This
// means that problematic potential non-loop regions will have entry and/or exit
// blocks immediately nested inside a VPLoopRegion (i.e., block's parent will be
// a VPLoopRegion). In order to detect such cases, we currently check whether
// the loop header is reachable starting from region's entry block up to
// region's exit block.
bool VPlanHCFGBuilder::regionIsBackEdgeCompliant(
    const VPBlockBase *Entry, const VPBlockBase *Exit,
    VPRegionBlock *ParentRegion) const {

  // If the immediate parent region is not a loop region, current region won't
  // have any problem with loop cycles, so it's back edge compliant
  if (!isa<VPLoopRegion>(ParentRegion))
    return true;

  // Expensive check: check if loop header is inside the region
  VPLoop *ParentLoop = cast<VPLoopRegion>(ParentRegion)->getVPLoop();
  VPBlockBase *LoopHeader = ParentLoop->getHeader();
  assert(ParentLoop->contains(Entry) &&
         "Potential entry blocks should be inside the loop");
  assert(ParentLoop->contains(Exit) &&
         "Potential exit blocks should be inside the loop");

  SmallVector<const VPBlockBase *, 32> WorkList;
  SmallPtrSet<const VPBlockBase *, 32> Visited;
  WorkList.push_back(Entry);

  while (!WorkList.empty()) {
    const VPBlockBase *Current = WorkList.back();
    WorkList.pop_back();

    if (Visited.count(Current))
      continue;
    Visited.insert(Current);

    if (Current == LoopHeader)
      return false;

    // Add successors but skip Exit successors
    if (Current != Exit)
      for (auto Succ : Current->getSuccessors())
        WorkList.push_back(Succ);
  }

  return true;
}

// TODO
// Return true if \p Block is a VPBasicBlock that contains a successor selector
// (CondBit) that is not uniform. If Block is a VPRegionBlock,
// it returns false since a region can only have a single successor (by now).
bool VPlanHCFGBuilder::isDivergentBlock(VPBlockBase *Block) const {
  if (DisableUniformRegions)
    return true;

  if (auto *VPBB = dyn_cast<VPBasicBlock>(Block)) {
    unsigned NumSuccs = Block->getNumSuccessors();
    if (NumSuccs < 2) {
      assert(!VPBB->getCondBit() && "Unexpected condition bit instruction");
      return false;
    } else {
      // Multiple successors. Checking uniformity of Condition Bit Instruction.
      VPValue *CBV = VPBB->getCondBit();
      assert(CBV && "Expected condition bit value.");

      // TODO: Temporal implementation for HIR
      return !Plan->UniformCBVs.count(CBV);
    }
  }

  // Regions doesn't change parent region divergence.
  assert(Block->getSinglePredecessor() && "Region with multiple successors");
  return false;
}

class PrivatesListCvt;

namespace {
// Build plain CFG from incomming IR using only VPBasicBlock's that contain
// VPInstructions. Return VPRegionBlock that encloses all the VPBasicBlock's
// of the plain CFG.
class PlainCFGBuilder {

  /// Outermost loop of the input loop nest.
  Loop *TheLoop = nullptr;

  LoopInfo *LI = nullptr;
  // TODO: This should be removed together with the UniformCBVs set.
  LoopVectorizationLegality *Legal;

  // Output TopRegion. Owned during the PlainCFG build process, moved
  // afterwards.
  std::unique_ptr<VPRegionBlock> TopRegion;

  // Number of VPBasicBlocks in TopRegion.
  unsigned TopRegionSize = 0;

  VPlan *Plan = nullptr;

  // Builder of the VPlan instruction-level representation.
  VPBuilder VPIRBuilder;

  // NOTE: The following maps are intentionally destroyed after the plain CFG
  // construction because subsequent VPlan-to-VPlan transformation may
  // invalidate them.
  // Map incoming BasicBlocks to their newly-created VPBasicBlocks.
  DenseMap<BasicBlock *, VPBasicBlock *> BB2VPBB;
  // Map incoming Value definitions to their newly-created VPValues.
  DenseMap<Value *, VPValue *> IRDef2VPValue;
  /// Map the branches to the condition VPInstruction they are controlled by
  /// (Possibly at a different VPBB).
  DenseMap<Value *, VPValue *> BranchCondMap;

  // Hold phi node's that need to be fixed once the plain CFG has been built.
  SmallVector<PHINode *, 8> PhisToFix;

  // Auxiliary functions
  void setVPBBPredsFromBB(VPBasicBlock *VPBB, BasicBlock *BB);
  void fixPhiNodes();
  VPBasicBlock *getOrCreateVPBB(BasicBlock *BB);
  bool isExternalDef(Value *Val) const;
  // Check whether Val has uses outside the vectorized loop and create
  // VPExternalUse-s for NewVPInst accordingly.
  void addExternalUses(Value *Val, VPValue *NewVPInst);

  void createVPInstructionsForVPBB(VPBasicBlock *VPBB, BasicBlock *BB);

  // Create a VPInstruction based on the input IR instruction.
  VPInstruction *createVPInstruction(Instruction *Inst);

  // Check if the given IR instruction is present in the loop-body.
  bool loopContains(Instruction *Inst) {
    assert(Inst && "Expect a non-null instruction passed into this function.");
    return TheLoop->contains(Inst);
  }

  // Reset the insertion point.
  void resetInsertPoint() { VPIRBuilder.clearInsertionPoint(); }

public:
  friend PrivatesListCvt;

  PlainCFGBuilder(Loop *Lp, LoopInfo *LI, LoopVectorizationLegality *Legal,
                  VPlan *Plan)
      : TheLoop(Lp), LI(LI), Legal(Legal), Plan(Plan) {}

  std::unique_ptr<VPRegionBlock> buildPlainCFG();
  void
  convertEntityDescriptors(LoopVectorizationLegality *Legal,
                           VPlanHCFGBuilder::VPLoopEntityConverterList &Cvts);
  VPValue *getOrCreateVPOperand(Value *IRVal);
};
} // anonymous namespace

// Set predecessors of \p VPBB in the same order as they are in LLVM \p BB.
void PlainCFGBuilder::setVPBBPredsFromBB(VPBasicBlock *VPBB, BasicBlock *BB) {
  SmallVector<VPBlockBase *, 8> VPBBPreds;
  // Collect VPBB predecessors.
  for (BasicBlock *Pred : predecessors(BB))
    VPBBPreds.push_back(getOrCreateVPBB(Pred));

  VPBB->setPredecessors(VPBBPreds);
}

static void assertIsSingleElementAlloca(Value *CurValue) {
  if (!CurValue)
    return;
  if (auto AllocaI = dyn_cast<AllocaInst>(CurValue)) {
    Value *ArrSize = AllocaI->getArraySize();
    assert((isa<ConstantInt>(ArrSize) && cast<ConstantInt>(ArrSize)->isOne()) &&
           "Alloca is unsupported for privatization");
    (void)ArrSize;
  }
}

// Base class for VPLoopEntity conversion functors.
class VPEntityConverterBase {
public:
  using InductionList = VPOVectorizationLegality::InductionList;
  using LinearListTy = VPOVectorizationLegality::LinearListTy;
  using ReductionList = VPOVectorizationLegality::ReductionList;
  using ExplicitReductionList = VPOVectorizationLegality::ExplicitReductionList;
  using InMemoryReductionList = VPOVectorizationLegality::InMemoryReductionList;
  using PrivatesListTy = VPOVectorizationLegality::PrivatesListTy;

  VPEntityConverterBase(PlainCFGBuilder &Bld) : Builder(Bld) {}

protected:
  PlainCFGBuilder &Builder;
};

// Conversion functor for auto-recognized reductions
class ReductionListCvt : public VPEntityConverterBase {
public:
  ReductionListCvt(PlainCFGBuilder &Bld) : VPEntityConverterBase(Bld) {}

  void operator()(ReductionDescr &Descriptor,
                  const ReductionList::value_type &CurValue) {
    Descriptor.clear();
    const RecurrenceDescriptor &RD = CurValue.second;
    Descriptor.setStartPhi(
        dyn_cast<VPInstruction>(Builder.getOrCreateVPOperand(CurValue.first)));
    Descriptor.setStart(
        Builder.getOrCreateVPOperand(RD.getRecurrenceStartValue()));
    Descriptor.setExit(dyn_cast<VPInstruction>(
        Builder.getOrCreateVPOperand(RD.getLoopExitInstr())));
    Descriptor.setKind(RD.getRecurrenceKind());
    Descriptor.setMinMaxKind(RD.getMinMaxRecurrenceKind());
    Descriptor.setRecType(RD.getRecurrenceType());
    Descriptor.setSigned(RD.isSigned());
    Descriptor.setAllocaInst(nullptr);
    Descriptor.setLinkPhi(nullptr);
  }
};
// Conversion functor for explicit reductions
class ExplicitReductionListCvt : public VPEntityConverterBase {
public:
  ExplicitReductionListCvt(PlainCFGBuilder &Bld) : VPEntityConverterBase(Bld) {}

  void operator()(ReductionDescr &Descriptor,
                  const ExplicitReductionList::value_type &CurValue) {
    Descriptor.clear();
    const RecurrenceDescriptor &RD = CurValue.second.first;
    Descriptor.setStartPhi(
        dyn_cast<VPInstruction>(Builder.getOrCreateVPOperand(CurValue.first)));
    Descriptor.setStart(
        Builder.getOrCreateVPOperand(RD.getRecurrenceStartValue()));
    Descriptor.addUpdateVPInst(dyn_cast<VPInstruction>(
        Builder.getOrCreateVPOperand(RD.getLoopExitInstr())));
    // Exit is not set here, it is determined based on some analyses in Phase 2
    Descriptor.setExit(nullptr);
    Descriptor.setKind(RD.getRecurrenceKind());
    Descriptor.setMinMaxKind(RD.getMinMaxRecurrenceKind());
    Descriptor.setRecType(RD.getRecurrenceType());
    Descriptor.setSigned(RD.isSigned());
    assertIsSingleElementAlloca(CurValue.second.second);
    Descriptor.setAllocaInst(
        Builder.getOrCreateVPOperand(CurValue.second.second));
    Descriptor.setLinkPhi(nullptr);
  }
};
// Conversion functor for in-memory reductions
class InMemoryReductionListCvt : public VPEntityConverterBase {
public:
  InMemoryReductionListCvt(PlainCFGBuilder &Bld) : VPEntityConverterBase(Bld) {}

  void operator()(ReductionDescr &Descriptor,
                  const InMemoryReductionList::value_type &CurValue) {
    Descriptor.clear();
    assertIsSingleElementAlloca(CurValue.first);
    VPValue *AllocaInst = Builder.getOrCreateVPOperand(CurValue.first);
    Descriptor.setStartPhi(nullptr);
    Descriptor.setStart(AllocaInst);
    Descriptor.setExit(nullptr);
    Descriptor.setKind(CurValue.second.first);
    Descriptor.setMinMaxKind(CurValue.second.second);
    Descriptor.setRecType(nullptr);
    Descriptor.setSigned(false);
    Descriptor.setAllocaInst(AllocaInst);
    Descriptor.setLinkPhi(nullptr);
  }
};

// Conversion functor for auto-recognized inductions
class InductionListCvt : public VPEntityConverterBase {
public:
  InductionListCvt(PlainCFGBuilder &Bld) : VPEntityConverterBase(Bld) {}

  void operator()(InductionDescr &Descriptor,
                  const InductionList::value_type &CurValue) {
    Descriptor.clear();
    const InductionDescriptor &ID = CurValue.second;
    Descriptor.setStartPhi(dyn_cast<VPInstruction>(
        Builder.getOrCreateVPOperand(CurValue.first)));
    Descriptor.setKind(ID.getKind());
    Descriptor.setStart(Builder.getOrCreateVPOperand(ID.getStartValue()));
    const SCEV *Step = ID.getStep();
    Value *V = nullptr;
    if (auto UndefStep = dyn_cast<SCEVUnknown>(Step))
      V = UndefStep->getValue();
    else if (auto ConstStep = dyn_cast<SCEVConstant>(Step))
      V = ConstStep->getValue();
    if (V)
      Descriptor.setStep(Builder.getOrCreateVPOperand(V));
    else {
      // Step of induction is variable, populate it later via VPlan
      Descriptor.setStep(nullptr);
    }
    if (ID.getInductionBinOp()) {
      Descriptor.setInductionBinOp(dyn_cast<VPInstruction>(
          Builder.getOrCreateVPOperand(ID.getInductionBinOp())));
      Descriptor.setBinOpcode(Instruction::BinaryOpsEnd);
    } else {
      assert(Descriptor.getStartPhi() &&
             "Induction descriptor does not have starting PHI.");
      Type *IndTy = Descriptor.getStartPhi()->getType();
      (void)IndTy;
      assert((IndTy->isIntegerTy() || IndTy->isPointerTy()) &&
             "unexpected induction type");
      Descriptor.setInductionBinOp(nullptr);
      Descriptor.setBinOpcode(Instruction::Add);
    }
    Descriptor.setAllocaInst(nullptr);
  }
};

// Conversion functor for explcit linears
class LinearListCvt : public VPEntityConverterBase {
public:
  LinearListCvt(PlainCFGBuilder &Bld) : VPEntityConverterBase(Bld) {}

  void operator()(InductionDescr &Descriptor,
                  const LinearListTy::value_type &CurValue) {
    Descriptor.clear();
    Descriptor.setStartPhi(nullptr);
    Descriptor.setStart(Builder.getOrCreateVPOperand(CurValue.first));

    Type *IndTy = CurValue.first->getType();
    assert(IndTy->isPointerTy() &&
           "expected pointer type for explicit induction");
    IndTy = IndTy->getPointerElementType();
    Type *StepTy = IndTy;
    if (IndTy->isIntegerTy())
      Descriptor.setKind(InductionDescriptor::IK_IntInduction);
    else if (IndTy->isPointerTy()) {
      Descriptor.setKind(InductionDescriptor::IK_PtrInduction);
      assert(isa<Instruction>(CurValue.first) &&
             "Linear descriptor is not an instruction.");
      const DataLayout &DL =
          cast<Instruction>(CurValue.first)->getModule()->getDataLayout();
      StepTy = DL.getIntPtrType(IndTy);
    } else {
      assert(IndTy->isFloatingPointTy() && "unexpected induction type");
      Descriptor.setKind(InductionDescriptor::IK_FpInduction);
    }
    Value *Cstep = ConstantInt::get(StepTy, CurValue.second);
    Descriptor.setStep(Builder.getOrCreateVPOperand(Cstep));

    Descriptor.setInductionBinOp(nullptr);
    Descriptor.setBinOpcode(Instruction::Add);
    assertIsSingleElementAlloca(CurValue.first);
    Descriptor.setAllocaInst(
        isa<AllocaInst>(CurValue.first) ? Descriptor.getStart() : nullptr);
    Descriptor.setIsExplicitInduction(true);
  }
};

// Convert data from Privates list
class PrivatesListCvt : public VPEntityConverterBase {

  // This method collects aliases that lie outside the loop-region. We are not
  // concerned with aliases within the loop as they would be acquired
  // when required (e.g., escape analysis).
  void collectAliases(PrivateDescr &Descriptor, Value *Alloca) {
    SetVector<Value *> WorkList;

    // Start with the Alloca Inst.
    WorkList.insert(Alloca);

    while (!WorkList.empty()) {
      Value *Head = WorkList.back();
      WorkList.pop_back();
      for (auto *Use : Head->users()) {
        if (isa<IntrinsicInst>(Use) &&
            VPOAnalysisUtils::isOpenMPDirective(cast<IntrinsicInst>(Use)))
          continue;

        // Check that the use of this alias is within the loop-region and it is
        // an alias-able instruction to begin with.
        // Rather than the more generic 'aliasing', we are more concerned here
        // with finding if the pointer here is based on another pointer.
        // LLVM Aliasing instructions -
        // https://llvm.org/docs/LangRef.html#pointer-aliasing-rules
        Instruction *Inst = cast<Instruction>(Use);

        // Lambda which determines if the 'User' is within the loop.
        bool AliasesWithinLoop = llvm::any_of(Inst->users(), [=](Value *User) {
          return Builder.loopContains(cast<Instruction>(User));
        });

        if ((isTrivialPointerAliasingInst(Inst) || isa<PtrToIntInst>(Inst)) &&
            AliasesWithinLoop) {
          auto *NewVPOperand = Builder.getOrCreateVPOperand(Inst);
          assert((isa<VPExternalDef>(NewVPOperand) ||
                  isa<VPInstruction>(NewVPOperand)) &&
                 "Expecting a VPExternalDef or a VPInstruction.");
          if (isa<VPExternalDef>(NewVPOperand)) {
            // Reset the insert-point. We do not want the instructions to be
            // currently put into any existing basic block.
            Builder.resetInsertPoint();
            WorkList.insert(Inst);
            VPInstruction *VPInst = Builder.createVPInstruction(Inst);
            assert(VPInst && "Expect a valid VPInst to be created.");
            Descriptor.addAlias(NewVPOperand, VPInst);
          }
        }
      }
    }
  }

public:
  PrivatesListCvt(PlainCFGBuilder &Bld, bool IsCond = false,
                  bool IsLast = false)
      : VPEntityConverterBase(Bld), IsCondPriv(IsCond), IsLastPriv(IsLast) {}

  void operator()(PrivateDescr &Descriptor,
                  const PrivatesListTy::value_type &CurValue) {

    Descriptor.clear();
    assertIsSingleElementAlloca(CurValue);
    auto *VPAllocaVal = Builder.getOrCreateVPOperand(CurValue);

    // Collect the out-of-loop aliases corresponding to this AllocaVal.
    // TODO: This is a temporary solution. Aliases to the private descriptor
    // should be collected earlier with new descriptor representation in
    // VPOLegality.
    collectAliases(Descriptor, CurValue);

    Descriptor.setAllocaInst(VPAllocaVal);
    Descriptor.setIsConditional(IsCondPriv);
    Descriptor.setIsLast(IsLastPriv);
    Descriptor.setIsExplicit(true);
    Descriptor.setIsMemOnly(true);
  }

private:
  bool IsCondPriv;
  bool IsLastPriv;
};

class Loop2VPLoopMapper {
public:
  Loop2VPLoopMapper() = delete;
  explicit Loop2VPLoopMapper(const Loop *TheLoop, const VPlan *Plan) {
    DenseMap<const BasicBlock *, const Loop *> Head2Loop;
    // First fill in the header->loop map
    std::function<void(const Loop *)> getLoopHeaders = [&](const Loop *L) {
      Head2Loop[L->getHeader()] = L;
      for (auto Loop : *L)
        getLoopHeaders(Loop);
    };
    getLoopHeaders(TheLoop);
    // Next fill in the Loop->VPLoop map
    std::function<void(const VPLoop *)> mapLoop2VPLoop =
        [&](const VPLoop *VPL) {
          VPBasicBlock *BB = cast<VPBasicBlock>(VPL->getHeader());
          const Loop *L = Head2Loop[BB->getOriginalBB()];
          assert(L != nullptr && "Can't find Loop");
          LoopMap[L] = VPL;
          for (auto VLoop : *VPL)
            mapLoop2VPLoop(VLoop);
        };
    const VPLoop *TopLoop = *(Plan->getVPLoopInfo()->begin());
    mapLoop2VPLoop(TopLoop);
  }

  const VPLoop *operator[](const Loop *L) const {
    auto Iter = LoopMap.find(L);
    return Iter == LoopMap.end() ? nullptr : Iter->second;
  }

protected:
  DenseMap<const Loop *, const VPLoop *> LoopMap;
};

// Specialization of reductions and inductions converters.
using ReductionConverter = VPLoopEntitiesConverter<ReductionDescr, Loop, Loop2VPLoopMapper>;
using InductionConverter = VPLoopEntitiesConverter<InductionDescr, Loop, Loop2VPLoopMapper>;
using PrivatesConverter  = VPLoopEntitiesConverter<PrivateDescr, Loop, Loop2VPLoopMapper>;

/// Convert incoming loop entities to the VPlan format.
void PlainCFGBuilder::convertEntityDescriptors(
    VPOVectorizationLegality *Legal,
    VPlanHCFGBuilder::VPLoopEntityConverterList &Cvts) {

  using InductionList = VPOVectorizationLegality::InductionList;
  using LinearListTy = VPOVectorizationLegality::LinearListTy;
  using ReductionList = VPOVectorizationLegality::ReductionList;
  using ExplicitReductionList = VPOVectorizationLegality::ExplicitReductionList;
  using InMemoryReductionList = VPOVectorizationLegality::InMemoryReductionList;
  using PrivatesListTy = VPOVectorizationLegality::PrivatesListTy;

  ReductionConverter *RedCvt = new ReductionConverter(Plan);
  InductionConverter *IndCvt = new InductionConverter(Plan);
  PrivatesConverter *PrivCvt = new PrivatesConverter(Plan);

  // TODO: create legality and import descriptors for all inner loops too.

  const InductionList *IL = Legal->getInductionVars();
  iterator_range<InductionList::const_iterator> InducRange(IL->begin(), IL->end());
  InductionListCvt InducListCvt(*this);

  const LinearListTy *LL = Legal->getLinears();
  iterator_range<LinearListTy::const_iterator> LinearRange(LL->begin(), LL->end());
  LinearListCvt LinListCvt(*this);

  const ReductionList *RL = Legal->getReductionVars();
  iterator_range<ReductionList::const_iterator> ReducRange(RL->begin(), RL->end());
  ReductionListCvt RedListCvt(*this);

  const ExplicitReductionList *ERL = Legal->getExplicitReductionVars();
  iterator_range<ExplicitReductionList::const_iterator> ExplicitReductionRange(
      ERL->begin(), ERL->end());
  ExplicitReductionListCvt ExpRLCvt(*this);

  const InMemoryReductionList *IMRL = Legal->getInMemoryReductionVars();
  iterator_range<InMemoryReductionList::const_iterator> InMemoryReductionRange(
      IMRL->begin(), IMRL->end());
  InMemoryReductionListCvt IMRLCvt(*this);
  auto ReducPair = std::make_pair(ReducRange, RedListCvt);
  auto ExplicitRedPair = std::make_pair(ExplicitReductionRange, ExpRLCvt);
  auto InMemoryRedPair = std::make_pair(InMemoryReductionRange, IMRLCvt);

  // TODO: VPOLegality stores Privates, LastPrivates and CondPrivates in
  // different lists. This is different from the way HIRLegality store this
  // information. Till we have a unified way of storing the information and
  // accessing it, we will have to do with the following hack where we we go
  // through the Privates, which is a superset, and check membership of elements
  // within ConPrivates and LastPrivates. This helps us separate out paivates
  // based on types. This code will be simplified when we have the correct
  // implementation for Privates in VPOLegality.
  const PrivatesListTy &PrivatesList = Legal->getPrivates();
  const PrivatesListTy &CondPrivatesList = Legal->getCondPrivates();
  const PrivatesListTy &LastPrivatesList = Legal->getLastPrivates();

  PrivatesListTy NewPrivatesList;
  PrivatesListTy NewCondPrivatesList;
  PrivatesListTy NewLastPrivatesList;

  for (auto Val : PrivatesList) {
    if (CondPrivatesList.count(Val))
      NewCondPrivatesList.insert(Val);
    else if (LastPrivatesList.count(Val))
      NewLastPrivatesList.insert(Val);
    else
      NewPrivatesList.insert(Val);
  }

  iterator_range<PrivatesListTy::const_iterator> PrivatesRange(
      NewPrivatesList.begin(), NewPrivatesList.end());
  iterator_range<PrivatesListTy::const_iterator> CondPrivatesRange(
      NewCondPrivatesList.begin(), NewCondPrivatesList.end());
  iterator_range<PrivatesListTy::const_iterator> LastPrivatesRange(
      NewLastPrivatesList.begin(), NewLastPrivatesList.end());

  PrivatesListCvt PrivListCvt(*this);
  PrivatesListCvt CondPrivListCvt(*this, true /*IsCond*/, false /*IsLast*/);
  PrivatesListCvt LastPrivListCvt(*this, false /*IsCond*/, true /*IsLast*/);

  RedCvt->createDescrList(TheLoop, ReducPair, ExplicitRedPair, InMemoryRedPair);

  auto InducPair = std::make_pair(InducRange, InducListCvt);
  auto LinearPair = std::make_pair(LinearRange, LinListCvt);

  auto PrivatesPair = std::make_pair(PrivatesRange, PrivListCvt);
  auto CondPrivatesPair = std::make_pair(CondPrivatesRange, CondPrivListCvt);
  auto LastPrivatesPair = std::make_pair(LastPrivatesRange, LastPrivListCvt);

  IndCvt->createDescrList(TheLoop, InducPair, LinearPair);

  PrivCvt->createDescrList(TheLoop, PrivatesPair, CondPrivatesPair,
                           LastPrivatesPair);

  Cvts.push_back(std::unique_ptr<VPLoopEntitiesConverterBase>(RedCvt));
  Cvts.push_back(std::unique_ptr<VPLoopEntitiesConverterBase>(IndCvt));
  Cvts.push_back(std::unique_ptr<VPLoopEntitiesConverterBase>(PrivCvt));
}

// Set operands to VPInstructions representing phi nodes from the input IR.
// VPlan Phi nodes were created without operands in a previous step of the H-CFG
// construction because those operands might not have been created in VPlan at
// that time despite the RPO traversal. This function expects all the
// instructions to have a representation in VPlan so operands of VPlan phis can
// be properly set.
void PlainCFGBuilder::fixPhiNodes() {
  for (auto *Phi : PhisToFix) {
    assert(IRDef2VPValue.count(Phi) && "Missing VPInstruction for PHINode.");
    VPValue *VPVal = IRDef2VPValue[Phi];
#if INTEL_CUSTOMIZATION
    assert(isa<VPPHINode>(VPVal) && "Expected VPPHINode for phi node.");
    auto *VPPhi = cast<VPPHINode>(VPVal);
    assert(VPPhi->getNumOperands() == 0 &&
           "Expected VPInstruction with no operands.");

    for (unsigned I = 0, E = Phi->getNumIncomingValues(); I != E; ++I)
      VPPhi->addIncoming(getOrCreateVPOperand(Phi->getIncomingValue(I)),
                         getOrCreateVPBB(Phi->getIncomingBlock(I)));
#else
    assert(isa<VPInstruction>(VPVal) && "Expected VPInstruction for phi node.");
    auto *VPPhi = cast<VPInstruction>(VPVal);
    for (Value *Op : Phi->operands())
      VPPhi->addOperand(getOrCreateVPOperand(Op));
#endif // INTEL_CUSTOMIZATION
  }
}

// Create a new empty VPBasicBlock for an incoming BasicBlock or retrieve an
// existing one if it was already created.
VPBasicBlock *PlainCFGBuilder::getOrCreateVPBB(BasicBlock *BB) {

  VPBasicBlock *VPBB;
  auto BlockIt = BB2VPBB.find(BB);

  if (BlockIt == BB2VPBB.end()) {
    // New VPBB
    LLVM_DEBUG(dbgs() << "Creating VPBasicBlock for " << BB->getName() << "\n");
    VPBB = new VPBasicBlock(VPlanUtils::createUniqueName("BB"));
    BB2VPBB[BB] = VPBB;
    VPBB->setOriginalBB(BB);
    VPBB->setParent(TopRegion.get());
    ++TopRegionSize;
  } else {
    // Retrieve existing VPBB
    VPBB = BlockIt->second;
  }

  return VPBB;
}

// Return true if \p Val is considered an external definition in the context of
// the plain CFG construction.
//
// An external definition is either:
#if INTEL_CUSTOMIZATION
// 1. A Value that is neither a Constant nor an Instruction.
#else
// 1. A Value that is not an Instruction. This will be refined in the future.
#endif
// 2. An Instruction that is outside of the CFG snippet represented in VPlan.
// However, since we don't represent loop Instructions in loop PH/Exit as
// VPInstructions during plain CFG construction, those are also considered
// external definitions in this particular context.
bool PlainCFGBuilder::isExternalDef(Value *Val) const {
#if INTEL_CUSTOMIZATION
  assert(!isa<Constant>(Val) &&
         "Constants should have been processed separately.");
  assert(!isa<MetadataAsValue>(Val) &&
         "MetadataAsValue should have been processed separately.");
#endif
  // All the Values that are not Instructions are considered external
  // definitions for now.
  Instruction *Inst = dyn_cast<Instruction>(Val);
  if (!Inst)
    return true;

  // Check whether Instruction definition is within the loop nest.
  return !TheLoop->contains(Inst);
}

// Check whether any use of Val is outside
void PlainCFGBuilder::addExternalUses(Value *Val, VPValue *NewVPInst) {
  for (User *U : Val->users())
    if (auto Inst = dyn_cast<Instruction>(U))
      if (!TheLoop->contains(Inst)) {
        VPExternalUse *User = Plan->getVPExternalUse(Inst);
        User->addOperandWithUnderlyingValue(NewVPInst, Val);
      }
}

// Create a new VPValue or retrieve an existing one for the Instruction's
// operand \p IROp. This function must only be used to create/retrieve VPValues
// for *Instruction's operands* and not to create regular VPInstruction's. For
// the latter, please, look at 'createVPInstructionsForVPBB'.
VPValue *PlainCFGBuilder::getOrCreateVPOperand(Value *IRVal) {
#if INTEL_CUSTOMIZATION
  // Constant operand
  if (Constant *IRConst = dyn_cast<Constant>(IRVal))
    return Plan->getVPConstant(IRConst);

  if (MetadataAsValue *MDAsValue = dyn_cast<MetadataAsValue>(IRVal))
    return Plan->getVPMetadataAsValue(MDAsValue);
#endif

  auto VPValIt = IRDef2VPValue.find(IRVal);
  if (VPValIt != IRDef2VPValue.end())
    // Operand has an associated VPInstruction or VPValue that was previously
    // created.
    return VPValIt->second;

#if INTEL_CUSTOMIZATION
  // Operand is not Constant or MetadataAsValue and doesn't have a previously
  // created VPInstruction/VPValue. This means that operand is:
#else
  // Operand doesn't have a previously created VPInstruction/VPValue. This
  // means that operand is:
#endif
  //   A) a definition external to VPlan,
  //   B) any other Value without specific representation in VPlan.
  // For now, we use VPValue to represent A and B and classify both as external
  // definitions. We may introduce specific VPValue subclasses for them in the
  // future.
  assert(isExternalDef(IRVal) && "Expected external definition as operand.");
  // A and B: Create VPValue and add it to the pool of external definitions and
  // to the Value->VPValue map.
  VPExternalDef *ExtDef = Plan->getVPExternalDef(IRVal);
  IRDef2VPValue[IRVal] = ExtDef;
  return ExtDef;
}

VPInstruction *PlainCFGBuilder::createVPInstruction(Instruction *Inst) {

  if (auto *Br = dyn_cast<BranchInst>(Inst)) {
    // Branch instruction is not explicitly represented in VPlan but we need
    // to represent its condition bit when it's conditional.
    if (Br->isConditional())
      getOrCreateVPOperand(Br->getCondition());

    // Skip the rest of the Instruction processing for Branch instructions.
    return nullptr;
  }

  VPInstruction *NewVPInst{nullptr};
  if (auto *Phi = dyn_cast<PHINode>(Inst)) {
    // Phi node's operands may have not been visited at this point. We create
    // an empty VPInstruction that we will fix once the whole plain CFG has
    // been built.
#if INTEL_CUSTOMIZATION
    NewVPInst = cast<VPInstruction>(VPIRBuilder.createPhiInstruction(Inst));
#else
    NewVPInst = cast<VPInstruction>(
        VPIRBuilder.createNaryOp(Inst->getOpcode(), {} /*No operands*/, Inst));
#endif // INTEL_CUSTOMIZATION
    PhisToFix.push_back(Phi);
    return NewVPInst;
  } else {
    // Translate LLVM-IR operands into VPValue operands and set them in the
    // new VPInstruction.
    SmallVector<VPValue *, 4> VPOperands;
    for (Value *Op : Inst->operands())
      VPOperands.push_back(getOrCreateVPOperand(Op));

#if INTEL_CUSTOMIZATION
    if (CmpInst *CI = dyn_cast<CmpInst>(Inst)) {
      assert(VPOperands.size() == 2 && "Expected 2 operands in CmpInst.");
      NewVPInst = VPIRBuilder.createCmpInst(VPOperands[0], VPOperands[1], CI);
    } else if (auto *GEP = dyn_cast<GetElementPtrInst>(Inst)) {
      // Build VPGEPInstruction to represent GEP instructions
      SmallVector<VPValue *, 3> IdxList(VPOperands.begin() + 1,
                                        VPOperands.end());
      if (GEP->isInBounds())
        NewVPInst = VPIRBuilder.createInBoundsGEP(VPOperands[0], IdxList, Inst);
      else
        NewVPInst = VPIRBuilder.createGEP(VPOperands[0], IdxList, Inst);
    } else
#endif
      // Build VPInstruction for any arbitraty Instruction without specific
      // representation in VPlan.
      NewVPInst = cast<VPInstruction>(VPIRBuilder.createNaryOp(
          Inst->getOpcode(), Inst->getType(), VPOperands, Inst));
  }
  return NewVPInst;
}

// Create new VPInstructions in a VPBasicBlock, given its BasicBlock
// counterpart. This function must be invoked in RPO so that the operands of a
// VPInstruction in \p BB have been visited before. VPInstructions representing
// Phi nodes are created without operands to honor the RPO traversal. They will
// be fixed later by 'fixPhiNodes'.
void PlainCFGBuilder::createVPInstructionsForVPBB(VPBasicBlock *VPBB,
                                                  BasicBlock *BB) {
  VPIRBuilder.setInsertPoint(VPBB);
  for (Instruction &InstRef : *BB) {
    // There shouldn't be any VPValue for Inst at this point. Otherwise, we
    // visited Inst when we shouldn't, breaking the RPO traversal order.
    assert(!IRDef2VPValue.count(&InstRef) &&
           "Instruction shouldn't have been visited.");
    VPInstruction *NewVPInst = createVPInstruction(&InstRef);
    // createVPInstruction can return nullptr in case of a BranchInst.
    if (NewVPInst) {
      if (TheLoop->contains(&InstRef))
        addExternalUses(&InstRef, NewVPInst);

      IRDef2VPValue[&InstRef] = NewVPInst;
    }
  }
}

std::unique_ptr<VPRegionBlock> PlainCFGBuilder::buildPlainCFG() {
  // 1. Create the Top Region. It will be the parent of all VPBBs.
  TopRegion = std::make_unique<VPRegionBlock>(
      VPBlockBase::VPRegionBlockSC, VPlanUtils::createUniqueName("region"));
  TopRegionSize = 0;

  // 2. Scan the body of the loop in a topological order to visit each basic
  // block after having visited its predecessor basic blocks.Create a VPBB for
  // each BB and link it to its successor and predecessor VPBBs. Note that
  // predecessors must be set in the same order as they are in the incomming IR.
  // Otherwise, there might be problems with existing phi nodes and algorithms
  // based on predecessors traversal.

  // Create loop PH. PH needs to be explicitly processed since it's not taken
  // into account by LoopBlocksDFS below. Since the loop PH may contain any
  // Instruction, related or not to the loop nest, we do not create
  // VPInstructions for them. Those Instructions used within the loop nest will
  // be modeled as external definitions.
  BasicBlock *PreheaderBB = TheLoop->getLoopPreheader();
  assert((PreheaderBB->getTerminator()->getNumSuccessors() == 1) &&
         "Unexpected loop preheader");
  VPBasicBlock *PreheaderVPBB = getOrCreateVPBB(PreheaderBB);
  // Create empty VPBB for Loop H so that we can link PH->H. H's VPInstructions
  // will be created during RPO traversal.
  VPBlockBase *HeaderVPBB = getOrCreateVPBB(TheLoop->getHeader());
  // Preheader's predecessors will be set during the loop RPO traversal below.
  PreheaderVPBB->setOneSuccessor(HeaderVPBB);

  LoopBlocksRPO RPO(TheLoop);
  RPO.perform(LI);

  for (BasicBlock *BB : RPO) {
    // Create or retrieve the VPBasicBlock for this BB and create its
    // VPInstructions.
    VPBasicBlock *VPBB = getOrCreateVPBB(BB);
    createVPInstructionsForVPBB(VPBB, BB);

    // Set VPBB successors. We create empty VPBBs for successors if they don't
    // exist already. Recipes will be created when the successor is visited
    // during the RPO traversal.
    Instruction *TI = BB->getTerminator();
    assert(TI && "Terminator expected");
    unsigned NumSuccs = TI->getNumSuccessors();

    if (NumSuccs == 1) {
      VPBasicBlock *SuccVPBB = getOrCreateVPBB(TI->getSuccessor(0));
      assert(SuccVPBB && "VPBB Successor not found");
      VPBB->setOneSuccessor(SuccVPBB);
      VPBB->setCBlock(BB);
      VPBB->setTBlock(TI->getSuccessor(0));
    } else if (NumSuccs == 2) {
      VPBasicBlock *SuccVPBB0 = getOrCreateVPBB(TI->getSuccessor(0));
      assert(SuccVPBB0 && "Successor 0 not found");
      VPBasicBlock *SuccVPBB1 = getOrCreateVPBB(TI->getSuccessor(1));
      assert(SuccVPBB1 && "Successor 1 not found");

      // Set VPBB's condition bit.
      assert(isa<BranchInst>(TI) && "Unsupported terminator!");
      auto *Br = cast<BranchInst>(TI);
      Value *BrCond = Br->getCondition();
#if INTEL_CUSTOMIZATION
      VPValue *VPCondBit;
      if (Constant *ConstBrCond = dyn_cast<Constant>(BrCond))
        // Create new VPConstant for constant branch condition.
        VPCondBit = Plan->getVPConstant(ConstBrCond);
      else {
        // Look up the branch condition to get the corresponding VPValue
        // representing the condition bit in VPlan (which may be in another
        // VPBB).
        assert(IRDef2VPValue.count(BrCond) &&
               "Missing condition bit in IRDef2VPValue!");
        VPCondBit = IRDef2VPValue[BrCond];
      }
#else
      // Look up the branch condition to get the corresponding VPValue
      // representing the condition bit in VPlan (which may be in another VPBB).
      assert(IRDef2VPValue.count(BrCond) &&
             "Missing condition bit in IRDef2VPValue!");
      VPValue *VPCondBit = IRDef2VPValue[BrCond];
#endif
      VPBB->setTwoSuccessors(VPCondBit, SuccVPBB0, SuccVPBB1);

      VPBB->setCBlock(BB);
      VPBB->setTBlock(TI->getSuccessor(0));
      VPBB->setFBlock(TI->getSuccessor(1));

    } else {
      llvm_unreachable("Number of successors not supported");
    }

    // Set VPBB predecessors in the same order as they are in the incoming BB.
    setVPBBPredsFromBB(VPBB, BB);
  }

  // 3. Process outermost loop exit. We created an empty VPBB for the loop
  // exit BBs during the RPO traversal of the loop nest but their predecessors
  // have to be properly set. Since a loop exit may contain any Instruction,
  // related or not to the loop nest, we do not create VPInstructions for them.
  SmallVector<BasicBlock *, 2> LoopExits;
  TheLoop->getUniqueExitBlocks(LoopExits);
  for (BasicBlock *BB : LoopExits) {
    VPBasicBlock *VPBB = BB2VPBB[BB];
    // Loop exit was already set as successor of the loop exiting BB.
    // We only set its predecessor VPBB now.
    setVPBBPredsFromBB(VPBB, BB);
  }

  // 4. The whole CFG has been built at this point so all the input Values must
  // have a VPlan couterpart. Fix VPlan phi nodes by adding their corresponding
  // VPlan operands.
  fixPhiNodes();

  // 5. Final Top Region setup.
  // Create a dummy block as Top Region's entry
  VPBlockBase *RegionEntry =
      new VPBasicBlock(VPlanUtils::createUniqueName("BB"));
  ++TopRegionSize;
  RegionEntry->setParent(TopRegion.get());
  VPBlockUtils::connectBlocks(RegionEntry, PreheaderVPBB);

  // Create a dummy block as Top Region's exit
  VPBlockBase *RegionExit =
      new VPBasicBlock(VPlanUtils::createUniqueName("BB"));
  ++TopRegionSize;
  RegionExit->setParent(TopRegion.get());

  // Connect dummy Top Region's exit.
  if (LoopExits.size() == 1) {
    VPBasicBlock *LoopExitVPBB = BB2VPBB[LoopExits.front()];
    VPBlockUtils::connectBlocks(LoopExitVPBB, RegionExit);
  } else {
    // If there are multiple exits in the outermost loop, we need another dummy
    // block as landing pad for all of them.
    assert(LoopExits.size() > 1 && "Wrong number of exit blocks");

    VPBlockBase *LandingPad =
        new VPBasicBlock(VPlanUtils::createUniqueName("BB"));
    ++TopRegionSize;
    LandingPad->setParent(TopRegion.get());

    // Connect multiple exits to landing pad
    for (auto ExitBB : make_range(LoopExits.begin(), LoopExits.end())) {
      VPBasicBlock *ExitVPBB = BB2VPBB[ExitBB];
      VPBlockUtils::connectBlocks(ExitVPBB, LandingPad);
    }

    // Connect landing pad to Top Region's exit
    VPBlockUtils::connectBlocks(LandingPad, RegionExit);
  }

  TopRegion->setEntry(RegionEntry);
  TopRegion->setExit(RegionExit);
  TopRegion->setSize(TopRegionSize);

  return std::move(TopRegion);
}

std::unique_ptr<VPRegionBlock>
VPlanHCFGBuilder::buildPlainCFG(VPLoopEntityConverterList &Cvts) {
  PlainCFGBuilder PCFGBuilder(TheLoop, LI, Legal, Plan);
  std::unique_ptr<VPRegionBlock> TopRegion = PCFGBuilder.buildPlainCFG();
  // Converting loop enities.
  if (LoopEntityImportEnabled)
    PCFGBuilder.convertEntityDescriptors(Legal, Cvts);
  return TopRegion;
}

void VPlanHCFGBuilder::passEntitiesToVPlan(VPLoopEntityConverterList &Cvts) {
  typedef VPLoopEntitiesConverterTempl<Loop2VPLoopMapper> BaseConverter;

  Loop2VPLoopMapper Mapper(TheLoop, Plan);
  for (auto &Cvt : Cvts) {
    BaseConverter *Converter = dyn_cast<BaseConverter>(Cvt.get());
    Converter->passToVPlan(Plan, Mapper);
  }
}
