#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelLoopVectorizationPlanner.h"
#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlan.h"
#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanDivergenceAnalysis.h"
#include "IntelVPlanTestBase.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace llvm::vpo;

namespace {

class CloneVPlan : public vpo::VPlanTestBase {};

DenseMap<const VPBasicBlock *, const VPBasicBlock *>
CompareGraphsAndCreateClonedOrigVPBBsMap(VPlan *ClonedVPlan,
                                         VPlanVector *OrigVPlan,
                                         VPlanVector::UpdateDA UDA) {
  SmallVector<std::pair<const VPBasicBlock *, const VPBasicBlock *>, 5>
      Worklist;
  const VPBasicBlock *ClonedEntryVPBB = &ClonedVPlan->getEntryBlock();
  const VPBasicBlock *OrigEntryVPBB = &OrigVPlan->getEntryBlock();
  Worklist.push_back(std::make_pair(ClonedEntryVPBB, OrigEntryVPBB));
  DenseMap<const VPBasicBlock *, const VPBasicBlock *> ClonedOrigVPBBsMap;

  while (!Worklist.empty()) {
    const auto &Pair = Worklist.back();
    Worklist.pop_back();
    const VPBasicBlock *ClonedVPBB = Pair.first;
    const VPBasicBlock *OrigVPBB = Pair.second;

    auto It = ClonedOrigVPBBsMap.find(ClonedVPBB);
    // Nodes that have already been visited, they should not be revisited.
    EXPECT_TRUE(It == ClonedOrigVPBBsMap.end());
    ClonedOrigVPBBsMap[ClonedVPBB] = OrigVPBB;
    EXPECT_EQ(ClonedVPBB->getNumSuccessors(), OrigVPBB->getNumSuccessors());

    for (unsigned Idx = 0; Idx < ClonedVPBB->getNumSuccessors(); ++Idx) {
      EXPECT_TRUE(OrigVPBB->getSuccessor(Idx));
      auto It = ClonedOrigVPBBsMap.find(ClonedVPBB->getSuccessor(Idx));
      if (It != ClonedOrigVPBBsMap.end())
        // This applies to hammocks and backedges.
        EXPECT_EQ(It->second, OrigVPBB->getSuccessor(Idx));
      else
        Worklist.push_back(std::make_pair(ClonedVPBB->getSuccessor(Idx),
                                          OrigVPBB->getSuccessor(Idx)));
    }

    // Check if ClonedVPBB and OrigVPBB have instructions with the same
    // opcode.
    for (auto ItO = OrigVPBB->begin(), ItC = ClonedVPBB->begin();
         ItO != OrigVPBB->end(); ++ItO, ++ItC) {
      EXPECT_TRUE(ItC != ClonedVPBB->end());
      EXPECT_EQ((&*ItO)->getOpcode(), (&*ItC)->getOpcode());
      // Check DA
      if (UDA == VPlanVector::UpdateDA::CloneDA ||
          UDA == VPlanVector::UpdateDA::RecalculateDA) {
        EXPECT_FALSE(OrigVPlan->getVPlanDA()->shapesAreDifferent(
            OrigVPlan->getVPlanDA()->getVectorShape(*ItO),
            ClonedVPlan->getVPlanDA()->getVectorShape(*ItC)));
      }
    }

    auto OrigLiveInRange = OrigVPlan->liveInValues();
    auto ClonedLiveInRange = ClonedVPlan->liveInValues();
    EXPECT_EQ(OrigVPlan->getLiveInValuesSize(),
              ClonedVPlan->getLiveInValuesSize());
    for (auto ItO = OrigLiveInRange.begin(), ItO_End = OrigLiveInRange.end(),
              ItC = ClonedLiveInRange.begin();
         ItO != ItO_End; ++ItO, ++ItC) {
      EXPECT_EQ((*ItO)->getMergeId(), (*ItC)->getMergeId());
    }

    auto OrigLiveOutRange = OrigVPlan->liveOutValues();
    auto ClonedLiveOutRange = ClonedVPlan->liveOutValues();
    EXPECT_EQ(OrigVPlan->getLiveOutValuesSize(),
              ClonedVPlan->getLiveOutValuesSize());
    for (auto ItO = OrigLiveOutRange.begin(), ItO_End = OrigLiveOutRange.end(),
              ItC = ClonedLiveOutRange.begin();
         ItO != ItO_End; ++ItO, ++ItC) {
      EXPECT_EQ(cast<VPInstruction>((*ItO)->getOperand(0))->getOpcode(),
                cast<VPInstruction>((*ItC)->getOperand(0))->getOpcode());
      EXPECT_EQ((*ItO)->getMergeId(), (*ItC)->getMergeId());
    }
  }
  return ClonedOrigVPBBsMap;
}

TEST_F(CloneVPlan, TestCloneVPlan) {
  const char *ModuleString =
      "define void @f() {\n"
      "entry:\n"
      "  br label %header\n"
      "header:\n"
      "  %iv = phi i32 [ 0, %entry ], [ %iv.next, %latch ]\n"
      "  %cond1 = icmp eq i32 %iv, 8\n"
      "  br i1 %cond1, label %bb1, label %latch\n"
      "bb1:\n"
      "  %x1 = add nsw i32 %iv, 1\n"
      "  %cond2 = icmp eq i32 %x1, 16\n"
      "  br i1 %cond2, label %bb2, label %bb3\n"
      "bb2:\n"
      "  %x2 = add nsw i32 %x1, 1\n"
      "  br label %latch\n"
      "bb3:\n"
      "  %x3 = add nsw i32 %x1, 2\n"
      "  br label %latch\n"
      "latch:\n"
      "  %iv.next = add nsw i32 %iv, 1\n"
      "  %bottom_test = icmp eq i32 %iv.next, 128\n"
      "  br i1 %bottom_test, label %header, label %end\n"
      "end:\n"
      "  ret void\n"
      "}\n";

  Module &M = parseModule(ModuleString);
  Function *F = M.getFunction("f");
  BasicBlock *LoopHeader = F->getEntryBlock().getSingleSuccessor();
  std::unique_ptr<VPlanNonMasked> OrigVPlan = buildHCFG(LoopHeader);
  ScalarEvolution SE(*F, *TLI.get(), *AC.get(), *DT.get(), *LI.get());
  VPAnalysesFactory VPAF(SE, *(LI.get())->begin(), DT.get(), AC.get(),
                         DL.get());
  auto VPDA = std::make_unique<VPlanDivergenceAnalysis>();
  OrigVPlan->setVPlanDA(std::move(VPDA));
  OrigVPlan->computeDA();
  std::unique_ptr<VPlanVector> ClonedVPlan(
      OrigVPlan->clone(VPAF, VPlanVector::UpdateDA::RecalculateDA));
  EXPECT_EQ(OrigVPlan->size(), ClonedVPlan->size());

  // Compare ClonedVPlan and OrigVPlan graphs.
  CompareGraphsAndCreateClonedOrigVPBBsMap(ClonedVPlan.get(), OrigVPlan.get(),
                                           VPlanVector::UpdateDA::RecalculateDA);
}

TEST_F(CloneVPlan, TestCloneLoop) {
  const char *ModuleString =
      "define void @f() {\n"
      "entry:\n"
      "  br label %outer.header\n"
      "outer.header:\n"
      "  %outer.iv = phi i32 [ 0, %entry ], [ %outer.iv.next, %outer.latch ]\n"
      "  br label %inner.header\n"
      "inner.header:\n"
      "  %inner.iv = phi i32 [ 0, %entry ], [ %inner.iv.next, %inner.latch ]\n"
      "  %inner.cond = icmp eq i32 %inner.iv, 8\n"
      "  br i1 %inner.cond, label %inner.bb, label %inner.latch\n"
      "inner.bb:\n"
      "  %x = add nsw i32 %inner.iv, 1\n"
      "  br label %inner.latch\n"
      "inner.latch:\n"
      "  %inner.iv.next = add nsw i32 %inner.iv, 1\n"
      "  %inner.bottom_test = icmp eq i32 %inner.iv.next, 128\n"
      "  br i1 %inner.bottom_test, label %inner.header, label %outer.latch\n"
      "outer.latch:\n"
      "  %outer.iv.next = add nsw i32 %outer.iv, 1\n"
      "  %outer.bottom_test = icmp eq i32 %outer.iv.next, 128\n"
      "  br i1 %outer.bottom_test, label %outer.header, label %end\n"
      "end:\n"
      "  ret void\n"
      "}\n";

  Module &M = parseModule(ModuleString);
  Function *F = M.getFunction("f");
  BasicBlock *LoopHeader = F->getEntryBlock().getSingleSuccessor();
  std::unique_ptr<VPlanNonMasked> OrigVPlan = buildHCFG(LoopHeader);
  ScalarEvolution SE(*F, *TLI.get(), *AC.get(), *DT.get(), *LI.get());
  VPAnalysesFactory VPAF(SE, *(LI.get())->begin(), DT.get(), AC.get(),
                         DL.get());
  auto VPDA = std::make_unique<VPlanDivergenceAnalysis>();
  OrigVPlan->setVPlanDA(std::move(VPDA));
  OrigVPlan->computeDA();
  std::unique_ptr<VPlanVector> ClonedVPlan(
      OrigVPlan->clone(VPAF, VPlanVector::UpdateDA::RecalculateDA));
  EXPECT_EQ(OrigVPlan->size(), ClonedVPlan->size());

  // Create a map between the loops of the original and cloned plans.
  SmallVector<std::pair<const VPLoop *, const VPLoop *>, 2> LoopWorklist;
  const VPLoop *ClonedTopLoop = *ClonedVPlan->getVPLoopInfo()->begin();
  const VPLoop *OrigTopLoop = *OrigVPlan->getVPLoopInfo()->begin();
  LoopWorklist.push_back(std::make_pair(ClonedTopLoop, OrigTopLoop));
  DenseMap<const VPLoop *, const VPLoop *> ClonedOrigVPLoopsMap;

  while (!LoopWorklist.empty()) {
    const auto &Pair = LoopWorklist.back();
    LoopWorklist.pop_back();
    const VPLoop *ClonedVPLoop = Pair.first;
    const VPLoop *OrigVPLoop = Pair.second;
    // Sanity check.
    auto it = ClonedOrigVPLoopsMap.find(ClonedVPLoop);
    ASSERT_TRUE(it == ClonedOrigVPLoopsMap.end());
    ClonedOrigVPLoopsMap[ClonedVPLoop] = OrigVPLoop;

    for (auto itC = ClonedVPLoop->begin(), itO = OrigVPLoop->begin();
         itC != ClonedVPLoop->end(); ++itC, ++itO) {
      LoopWorklist.push_back(std::make_pair(*itC, *itO));
    }
  }

  // Check if the loops have the right parent loop.
  for (const auto &Pair : ClonedOrigVPLoopsMap) {
    const VPLoop *ClonedVPLoop = Pair.first;
    const VPLoop *OrigVPLoop = Pair.second;
    bool isParentForClonedVPLoop = ClonedVPLoop->getParentLoop() != nullptr;
    bool isParentForOrigVPLoop = OrigVPLoop->getParentLoop() != nullptr;
    EXPECT_EQ(isParentForClonedVPLoop, isParentForOrigVPLoop);

    if (isParentForClonedVPLoop) {
      EXPECT_EQ(OrigVPLoop->getParentLoop(),
                ClonedOrigVPLoopsMap[ClonedVPLoop->getParentLoop()]);
    }
  }

  // Create the map between ClonedVPBBs and OrigVPBBs.
  DenseMap<const VPBasicBlock *, const VPBasicBlock *> ClonedOrigVPBBsMap =
      CompareGraphsAndCreateClonedOrigVPBBsMap(
          ClonedVPlan.get(), OrigVPlan.get(), VPlanVector::UpdateDA::RecalculateDA);

  VPLoopInfo *OrigVPLI = OrigVPlan->getVPLoopInfo();
  VPLoopInfo *ClonedVPLI = ClonedVPlan->getVPLoopInfo();
  // Check if the loops have the same basic blocks.
  for (const auto &Pair : ClonedOrigVPBBsMap) {
    const VPBasicBlock *ClonedVPBB = Pair.first;
    const VPBasicBlock *OrigVPBB = Pair.second;

    VPLoop *ClonedVPLoop = ClonedVPLI->getLoopFor(ClonedVPBB);
    VPLoop *OrigVPLoop = OrigVPLI->getLoopFor(OrigVPBB);
    EXPECT_EQ(OrigVPLoop, ClonedOrigVPLoopsMap[ClonedVPLoop]);
  }
}

TEST_F(CloneVPlan, TestCloneDA) {
  const char *ModuleString =
      "define void @f() {\n"
      "entry:\n"
      "  br label %outer.header\n"
      "outer.header:\n"
      "  %outer.iv = phi i32 [ 0, %entry ], [ %outer.iv.next, %outer.latch ]\n"
      "  br label %inner.header\n"
      "inner.header:\n"
      "  %inner.iv = phi i32 [ 0, %entry ], [ %inner.iv.next, %inner.latch ]\n"
      "  %add.iv = add nsw i32 %inner.iv, %outer.iv\n"
      "  %inner.cond = icmp eq i32 %add.iv, 8\n"
      "  br i1 %inner.cond, label %inner.bb1, label %inner.latch\n"
      "inner.bb1:\n"
      "  %x = add nsw i32 %inner.iv, 1\n"
      "  %x.cond = icmp eq i32 %x, %outer.iv\n"
      "  br i1 %inner.cond, label %inner.bb2, label %inner.bb3\n"
      "inner.bb2:\n"
      "  %y = add nsw i32 %x, 1\n"
      "  br label %inner.bb3\n"
      "inner.bb3:\n"
      "  %phi = phi i32 [ %x, %inner.bb1 ], [ %y, %inner.bb2 ]\n"
      "  %z = add nsw i32 %phi, 1\n"
      "  br label %inner.latch\n"
      "inner.latch:\n"
      "  %inner.iv.next = add nsw i32 %inner.iv, 1\n"
      "  %inner.bottom_test = icmp eq i32 %inner.iv.next, 128\n"
      "  br i1 %inner.bottom_test, label %inner.header, label %outer.bb1\n"
      "outer.bb1:\n"
      "  %lcssa.phi = phi i32 [ %z, %inner.latch ]\n"
      "  %add = add nsw i32 %lcssa.phi, 1\n"
      "  br label %outer.latch\n"
      "outer.latch:\n"
      "  %outer.iv.next = add nsw i32 %outer.iv, 1\n"
      "  %outer.bottom_test = icmp eq i32 %outer.iv.next, 128\n"
      "  br i1 %outer.bottom_test, label %outer.header, label %end\n"
      "end:\n"
      "  ret void\n"
      "}\n";

  Module &M = parseModule(ModuleString);
  Function *F = M.getFunction("f");
  BasicBlock *LoopHeader = F->getEntryBlock().getSingleSuccessor();
  std::unique_ptr<VPlanNonMasked> OrigVPlan = buildHCFG(LoopHeader);
  ScalarEvolution SE(*F, *TLI.get(), *AC.get(), *DT.get(), *LI.get());
  VPAnalysesFactory VPAF(SE, *(LI.get())->begin(), DT.get(), AC.get(),
                         DL.get());
  auto VPDA = std::make_unique<VPlanDivergenceAnalysis>();
  OrigVPlan->setVPlanDA(std::move(VPDA));
  OrigVPlan->computeDA();
  std::unique_ptr<VPlanVector> ClonedVPlan(
      OrigVPlan->clone(VPAF, VPlanVector::UpdateDA::CloneDA));
  EXPECT_EQ(OrigVPlan->size(), ClonedVPlan->size());

  // Create the map between ClonedVPBBs and OrigVPBBs.
  CompareGraphsAndCreateClonedOrigVPBBsMap(ClonedVPlan.get(), OrigVPlan.get(),
                                           VPlanVector::UpdateDA::CloneDA);
}

TEST_F(CloneVPlan, TestCloneVPLiveInOut) {
  const char *ModuleString =
      "define void @f() {\n"
      "entry:\n"
      "  br label %forbody\n"
      "forbody:\n"
      "  %iv = phi i32 [ 0, %entry ], [ %iv.next, %forbody ]\n"
      "  %iv.next = add nsw i32 %iv, 1\n"
      "  %bottom_test = icmp eq i32 %iv.next, 128\n"
      "  br i1 %bottom_test, label %loopexit, label %forbody\n"
      "loopexit:\n"
      "  %lcssa.phi = phi i32 [ %iv.next, %forbody ]\n"
      "  %x = add nsw i32 %lcssa.phi, 1\n"
      "  br label %end\n"
      "end:\n"
      "  ret void\n"
      "}\n";

  Module &M = parseModule(ModuleString);
  Function *F = M.getFunction("f");
  BasicBlock *LoopHeader = F->getEntryBlock().getSingleSuccessor();
  std::unique_ptr<VPlanNonMasked> OrigVPlan = buildHCFG(LoopHeader);
  ScalarEvolution SE(*F, *TLI.get(), *AC.get(), *DT.get(), *LI.get());
  VPAnalysesFactory VPAF(SE, *(LI.get())->begin(), DT.get(), AC.get(),
                         DL.get());
  // Compute DA.
  auto VPDA = std::make_unique<VPlanDivergenceAnalysis>();
  OrigVPlan->setVPlanDA(std::move(VPDA));
  OrigVPlan->computeDA();

  std::unique_ptr<VPlanVector> ClonedVPlan(
      OrigVPlan->clone(VPAF, VPlanVector::UpdateDA::CloneDA));
  EXPECT_EQ(OrigVPlan->size(), ClonedVPlan->size());

  // Compare ClonedVPlan and OrigVPlan graphs.
  CompareGraphsAndCreateClonedOrigVPBBsMap(ClonedVPlan.get(), OrigVPlan.get(),
                                           VPlanVector::UpdateDA::CloneDA);
}
} // namespace
