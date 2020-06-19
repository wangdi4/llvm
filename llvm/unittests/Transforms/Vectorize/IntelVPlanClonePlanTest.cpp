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

} // namespace
