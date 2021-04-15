//===-- llvm/unittest/Transforms/Vectorize/IntelVPlanCGTest.cpp------------===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===---------------------------------------------------------------------===//

#include "IntelVPlanTestBase.h"
#include "gtest/gtest.h"
#include <array>
#include <memory>

namespace llvm {
namespace vpo {
namespace {
class VPlanVPCGTest : public VPlanTestBase {};

TEST_F(VPlanVPCGTest, TestVPlanCGLoopCloning) {
  std::array<StringRef, 2> ModuleStrings = {{
      R"(
    define dso_local void @f() {
      entry:
        br label %for.body.lr.ph
      for.body.lr.ph:
        br label %SIMD1
      SIMD1:
        br label %for.body
      for.body:
        %iv = phi i64 [ %iv.next, %test.cond ], [ 0, %SIMD1 ]
        br label %seq.body.1
      seq.body.1:
        %if = icmp eq i64 %iv, 10
        br i1 %if, label %if.body, label %else.body
      if.body:
        br label %join.block
      else.body:
        br label %join.block
      join.block:
        br label %test.cond
      test.cond:
        %iv.next = add nuw nsw i64 %iv, 1
        %exitcond.not = icmp eq i64 %iv.next, 1024
        br i1 %exitcond.not, label %exit.split.edge, label %for.body
      exit.split.edge:
        br label %END.SIMD2
      END.SIMD2:
        ret void
    }
    )",
      R"(
    define dso_local void @f() {
      entry:
        br label %o.i.f.b.ph
      o.i.f.b.ph:
        br label %D.I.O.1
      D.I.O.1:
        br label %o.i.f.b
      o.i.f.b:
        %iv24 = phi i64 [ %iv.next, %for.cond.cleanup ], [ 0, %D.I.O.1 ]
        br label %for.body
      for.cond.cleanup:
        %iv.next= add nuw nsw i64 %iv24, 1
        %exit = icmp eq i64 %iv.next, 1024
        br i1 %exit, label %critical_edge, label %o.i.f.b
      for.body:
        %iv = phi i64 [ 0, %o.i.f.b ], [ %iv.next1, %for.body ]
        %iv.next1 = add nuw nsw i64 %iv, 1
        %exitcond = icmp eq i64 %iv.next1, 1024
        br i1 %exitcond, label %for.cond.cleanup, label %for.body
      critical_edge: ; preds = %for.cond.cleanup
        br label %DIR.OMP.END.SIMD.2
      DIR.OMP.END.SIMD.2:
        ret void
    }
    )"}};

  for (const auto ModuleString : ModuleStrings) {
    Module &M = parseModule(ModuleString.data());
    Function *F = M.getFunction("f");
    BasicBlock *LoopHeader = F->getEntryBlock()
                                 .getSingleSuccessor()
                                 ->getSingleSuccessor()
                                 ->getSingleSuccessor();
    auto VPOCG = getVPOCodeGen(LoopHeader, 4, 4);
    Loop *Lp = LI->getLoopFor(LoopHeader);
    BasicBlock *NewLoopPred = Lp->getLoopPreheader()->getSinglePredecessor();
    BasicBlock *NewLoopSucc = Lp->getLoopPreheader();
    Loop *NewLoop = VPOCG->cloneScalarLoop(Lp, NewLoopPred, NewLoopSucc,
                                           nullptr, Twine(".dup"));
    EXPECT_EQ(F->getName(), "f");

    // Check that the old and the new loop have the same number of blocks.
    EXPECT_EQ(Lp->getNumBlocks(), NewLoop->getNumBlocks());

    // Make sure that the specified predecessor connects to the new-loop's
    // preheader.
    EXPECT_EQ(NewLoopPred->getSingleSuccessor(), NewLoop->getLoopPreheader());

    // Check that the new-loop exit block is connected to the specified
    // NewLoopSucc.
    EXPECT_EQ(NewLoop->getUniqueExitBlock(), NewLoopSucc);

    // Check that each block in the existing and new loop has the same number of
    // successors.
    // TODO: Add a more stringent check for target-block name etc.
    auto &OrigLpBlocks = Lp->getBlocksVector();
    auto &NewLpBlocks = NewLoop->getBlocksVector();

    for (unsigned I = 0; I < OrigLpBlocks.size(); ++I) {
      auto *OrigTerminatorI =
          cast<BranchInst>(OrigLpBlocks[I]->getTerminator());
      auto *NewTerminatorI = cast<BranchInst>(NewLpBlocks[I]->getTerminator());
      EXPECT_EQ(OrigTerminatorI->getNumSuccessors(),
                NewTerminatorI->getNumSuccessors());
    }
  }
}

} // namespace
} // namespace vpo
} // namespace llvm
