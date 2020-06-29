#include "../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlan.h"
#include "IntelVPlanTestBase.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace llvm::vpo;

namespace {

class CloneVPlan : public vpo::VPlanTestBase {};

DenseMap<const VPBasicBlock *, const VPBasicBlock *>
CompareGraphsAndCreateClonedOrigVPBBsMap(VPlan *ClonedVPlan, VPlan *OrigVPlan) {
  SmallVector<std::pair<const VPBasicBlock *, const VPBasicBlock *>, 5>
      Worklist;
  const VPBasicBlock *ClonedEntryVPBB = ClonedVPlan->getEntryBlock();
  const VPBasicBlock *OrigEntryVPBB = OrigVPlan->getEntryBlock();
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
  std::unique_ptr<VPlan> OrigVPlan = buildHCFG(LoopHeader);
  ScalarEvolution SE(*F, *TLI.get(), *AC.get(), *DT.get(), *LI.get());
  VPAnalysesFactory VPAF(SE, *(LI.get())->begin(), DT.get(), AC.get(), DL.get(),
                         true);
  std::unique_ptr<VPlan> ClonedVPlan = OrigVPlan->clone(VPAF);
  EXPECT_EQ(OrigVPlan->size(), ClonedVPlan->size());

  // Compare ClonedVPlan and OrigVPlan graphs.
  CompareGraphsAndCreateClonedOrigVPBBsMap(ClonedVPlan.get(), OrigVPlan.get());
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
  std::unique_ptr<VPlan> OrigVPlan = buildHCFG(LoopHeader);
  ScalarEvolution SE(*F, *TLI.get(), *AC.get(), *DT.get(), *LI.get());
  VPAnalysesFactory VPAF(SE, *(LI.get())->begin(), DT.get(), AC.get(), DL.get(),
                         true);
  std::unique_ptr<VPlan> ClonedVPlan = OrigVPlan->clone(VPAF);
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
      CompareGraphsAndCreateClonedOrigVPBBsMap(ClonedVPlan.get(),
                                               OrigVPlan.get());

  VPLoopInfo *ClonedVPLI = ClonedVPlan->getVPLoopInfo();
  VPLoopInfo *OrigVPLI = OrigVPlan->getVPLoopInfo();
  // Check if the loops have the same basic blocks.
  for (const auto &Pair : ClonedOrigVPBBsMap) {
    const VPBasicBlock *ClonedVPBB = Pair.first;
    const VPBasicBlock *OrigVPBB = Pair.second;

    VPLoop *ClonedVPLoop = ClonedVPLI->getLoopFor(ClonedVPBB);
    VPLoop *OrigVPLoop = OrigVPLI->getLoopFor(OrigVPBB);
    EXPECT_EQ(OrigVPLoop, ClonedOrigVPLoopsMap[ClonedVPLoop]);
  }
}
} // namespace
