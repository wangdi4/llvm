//===-- VPOParoptTransformCSA.cpp - Transformation of W-Region for CSA --===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation. and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements CSA specific lowering for work regions.
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptTransform.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-transform-csa"

// Insert a pair of CSA parallel region enter and exit intrinsic calls around
// the parallel work region body as follows
//
//   %region = call i32 @llvm.csa.parallel.region.entry(i32 2002);
//   <parallel construct body>
//   call void @llvm.csa.parallel.region.exit(i32 %region);
//
// These calls mark the beginning and end of the parallel region for back-end.
//
Value* VPOParoptTransform::genCSAParallelRegion(WRegionNode *W) {
  assert(isTargetCSA() && "unexpected target");

  auto *Module = F->getParent();

  // CSA parallel region entry/exit intrinsics
  auto *RegionEntry = Intrinsic::getDeclaration(Module,
    Intrinsic::csa_parallel_region_entry);

  auto *RegionExit = Intrinsic::getDeclaration(Module,
    Intrinsic::csa_parallel_region_exit);

  // Insert a "entry" call into the work region's entry bblock. The argument
  // is a unique ID which is a work region's unique number plus 2000 (using
  // 2000 as base to avoid conflicts with IDs inserted by CSA builtin parallel
  // loop intrinsics lowering pass which uses IDs starting from 1000).
  IRBuilder<> Builder(W->getEntryBBlock()->getTerminator());
  auto *UniqueID = Builder.getInt32(2000u + W->getNumber());
  auto *RegionID = Builder.CreateCall(RegionEntry, { UniqueID }, "region");

  // And an "exit" call into the work region's exit bblock.
  Builder.SetInsertPoint(&*W->getExitBBlock()->getFirstInsertionPt());
  Builder.CreateCall(RegionExit, { RegionID }, {});

  return RegionID;
}

// Transform "omp parallel for" construct for CSA by inserting CSA builtins
// which annotate loop as parallel for the back-end passes.
// After transformation the annotated loop looks as follows
//
//   [%spmd = call i32 @llvm.csa.spmdization.entry(<num_threads>);]*
//   %region = call i32 @llvm.csa.parallel.region.entry(i32 2002);
//   for (...) {
//     %section = call i32 @llvm.csa.parallel.section.entry(i32 %region);
//     <loop body>
//     call void @llvm.csa.parallel.section.exit(i32 %section);
//   }
//   call void @llvm.csa.parallel.region.exit(i32 %region);
//   [call void @llvm.csa.spmdization.exit(i32 %spmd);]*
//
// *Optional spmdization entry/exit calls are inserted only if parallel for
// has num_threads clause.
//
bool VPOParoptTransform::genCSAParallelLoop(WRegionNode *W) {
  assert(isTargetCSA() && "unexpected target");

  auto *Module = F->getParent();

  auto *Loop = W->getWRNLoopInfo().getLoop();
  assert(Loop->isLoopSimplifyForm());

  // Annotating loop with spmdization entry/exit intrinsic calls if parallel
  // region has num_threads clause.
  auto *NumThreads = W->getNumThreads();
  if (NumThreads && dyn_cast<ConstantInt>(NumThreads)) {
    auto *Entry = Intrinsic::getDeclaration(Module,
      Intrinsic::csa_spmdization_entry);

    auto *Exit = Intrinsic::getDeclaration(Module,
      Intrinsic::csa_spmdization_exit);

    auto makeString = [&](const StringRef &Str) -> Constant* {
      auto &C = F->getContext();
      auto *Arr = ConstantDataArray::getString(C, Str);
      auto *Var = new GlobalVariable(*Module, Arr->getType(), true,
                                     GlobalValue::InternalLinkage, Arr,
                                     "spmd.mode");
      Var->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

      Value *Zero = ConstantInt::get(Type::getInt32Ty(C), 0u);
      return ConstantExpr::getGetElementPtr(Arr->getType(), Var,
                                            { Zero, Zero });
    };

    // Determine SPMDization mode, it depends on a schedule clause. We support
    // only static schedule with no chunk size so far.
    Value *Mode = nullptr;
    const auto &Sched = W->getSchedule();
    if (Sched.getChunkExpr() && Sched.getKind() == WRNScheduleStatic)
      // Three options for the chunk size
      //   Sched.getChunk() == 0 => chunk was not specified
      //   Sched.getChunk() > 0  => chunk is a compile time constant
      //   Sched.getChunk() < 0  => chunk is an expression
      // So far we handle only the "no chunk" case.
      if (Sched.getChunk() == 0)
        Mode = makeString("blocked");
    if (!Mode)
      Mode = makeString("cyclic");

    IRBuilder<> Builder(W->getEntryBBlock()->getTerminator());
    auto *SpmdID = Builder.CreateCall(Entry, { NumThreads, Mode }, "spmd");

    Builder.SetInsertPoint(&*W->getExitBBlock()->getFirstInsertionPt());
    Builder.CreateCall(Exit, { SpmdID }, {});
  }

  // Insert parallel region entry/exit calls
  auto *RegionID = genCSAParallelRegion(W);

  // CSA parallel section entry/exit intrinsics
  auto *SectionEntry = Intrinsic::getDeclaration(Module,
    Intrinsic::csa_parallel_section_entry);

  auto *SectionExit = Intrinsic::getDeclaration(Module,
    Intrinsic::csa_parallel_section_exit);

  // Insert section entry call into the beginning of the loop header.
  IRBuilder<> Builder(&*Loop->getHeader()->getFirstInsertionPt());
  auto *SectionID = Builder.CreateCall(SectionEntry, { RegionID }, "section");

  // Section exit call should be inserted right before the induction
  // variable increment in the loop latch.
  auto *IV = WRegionUtils::getOmpCanonicalInductionVariable(Loop);
  assert(IV && "no induction variable");

  // The value comming from the latch block is the increment instruction.
  auto *IVLatch = IV->getIncomingValueForBlock(Loop->getLoopLatch());
  assert(IVLatch && "no incomming value from the loop latch");

  auto *IVInc = dyn_cast<Instruction>(IVLatch);
  assert(IVInc && "no increment instruction for induction variable");

  // Insert a "csa.parallel.section.exit" before the increment instruction.
  Builder.SetInsertPoint(IVInc);
  Builder.CreateCall(SectionExit, { SectionID }, {});

  return true;
}

