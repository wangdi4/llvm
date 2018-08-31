//===- VPOParoptTransformCSA.cpp - Transformation of W-Region for CSA -*- C++ -*-===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
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
#include "llvm/IR/DiagnosticPrinter.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-transform-csa"

namespace {

class CSADiagInfo : public DiagnosticInfoWithLocationBase {
  const Twine &Msg;

public:
  CSADiagInfo(const Function &F, const DiagnosticLocation &Loc,
              const Twine &Msg, DiagnosticSeverity DS = DS_Warning)
    : DiagnosticInfoWithLocationBase(
        static_cast<DiagnosticKind>(DK_FirstPluginKind), DS, F, Loc),
      Msg(Msg)
  {}

  void print(DiagnosticPrinter &DP) const override {
    if (isLocationAvailable())
      DP << getLocationStr() << ": ";
    DP << "CSA - " << Msg;
    if (!isLocationAvailable())
      DP << " (use -g for location info)";
  }
};

} // anonymous namespace

bool VPOParoptTransform::isSupportedOnCSA(WRegionNode *W) {
  switch (W->getWRegionKindID()) {
    case WRegionNode::WRNParallelSections:
      // num_threads
      if (W->getNumThreads())
        reportCSAWarning(W, "ignoring unsupported num_threads clause");

      // [first|last]private
      if (!W->getPriv().empty())
        reportCSAWarning(W, "ignoring unsupported private clause");
      if (!W->getFpriv().empty())
        reportCSAWarning(W, "ignoring unsupported firstprivate clause");
      if (!W->getLpriv().empty())
        reportCSAWarning(W, "ignoring unsupported lastprivate clause");

      // reduction
      if (!W->getRed().empty())
        reportCSAWarning(W, "ignoring unsupported reduction clause");

      // fallthru

    case WRegionNode::WRNParallelLoop:
      // copyin
      if (!W->getCopyin().empty())
        reportCSAWarning(W, "ignoring unsupported copyin clause");

      // proc_bind
      if (W->getProcBind() != WRNProcBindAbsent)
        reportCSAWarning(W, "ignoring unsupported proc_bind clause");

      // Loop clauses.
      if (W->getWRegionKindID() == WRegionNode::WRNParallelLoop) {
        // linear
        if (!W->getLinear().empty())
          reportCSAWarning(W, "ignoring unsupported linear clause");

        // ordered
        if (W->getOrdered() >= 0)
          reportCSAWarning(W, "ignoring unsupported ordered clause");
      }
      return true;

    case WRegionNode::WRNAtomic:	// not fully supported, but may work
    case WRegionNode::WRNSection:
      return true;
  }
  reportCSAWarning(W, "ignoring unsupported \"omp " + W->getName() + "\"");
  return false;
}

void VPOParoptTransform::reportCSAWarning(WRegionNode *W, const Twine &Msg) {
  DebugLoc Loc;
  if (auto *I = W->getEntryBBlock()->getFirstNonPHI())
    Loc = I->getDebugLoc();
  F->getContext().diagnose(CSADiagInfo(*F, Loc,  Msg));
}

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
//   [%spmd = call i32 @llvm.csa.spmdization.entry(<num_threads>, <mode>);]*
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

  // Check schedule clause of there is one.
  const auto *Sched = &W->getSchedule();
  if (!Sched->getChunkExpr())
    Sched = nullptr;

  // So far we support only static and auto schedule types.
  if (Sched && Sched->getKind() != WRNScheduleStatic &&
      Sched->getKind() != WRNScheduleAuto) {
    reportCSAWarning(W, "ignoring unsupported schedule type");
    Sched = nullptr;
  }

  // Three options for the chunk size
  //   Sched->getChunk() == 0 => chunk was not specified
  //   Sched->getChunk() > 0  => chunk is a compile time constant
  //   Sched->getChunk() < 0  => chunk is an expression (unsupported)
  if (Sched && Sched->getChunk() < 0) {
    reportCSAWarning(W, "schedule chunk must be a compile time constant");
    Sched = nullptr;
  }

  // Validate num_threads if it was specified.
  auto *NumThreads = W->getNumThreads();
  if (NumThreads && !dyn_cast<ConstantInt>(NumThreads)) {
    reportCSAWarning(W, "num_threads must be a compile time constant");
    NumThreads = nullptr;
  }

  // Annotating loop with spmdization entry/exit intrinsic calls if parallel
  // region has num_threads clause.
  if (NumThreads) {
    auto *Entry = Intrinsic::getDeclaration(Module,
      Intrinsic::csa_spmdization_entry);

    auto *Exit = Intrinsic::getDeclaration(Module,
      Intrinsic::csa_spmdization_exit);

    // Determine SPMDization mode, it depends on a schedule clause.
    //   No schedule    => cyclic SPMD          (1)
    //   schedule(auto) => cyclic SPMD          (1)
    //   schedule(static) => blocked SPMD       (0)
    //     (chunksize > 1) => hybrid SPMD       (chunksize)
    Value *Mode = ConstantInt::get(Type::getInt32Ty(F->getContext()),
      !Sched || Sched->getKind() != WRNScheduleStatic ? 1u :
        Sched->getChunk() <= 1 ? 0u : Sched->getChunk());

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

bool VPOParoptTransform::genCSAIsLast(WRegionNode *W, AllocaInst *&IsLastVal) {
  // No need to do aynthing if W doesn't have any lastprivate var.
  if (!W->canHaveLastprivate() || W->getLpriv().empty())
    return false;

  Loop *Loop = W->getWRNLoopInfo().getLoop();
  assert(Loop->isLoopSimplifyForm());

  LLVMContext &C = F->getContext();
  IntegerType *Int32Ty = Type::getInt32Ty(C);
  const DataLayout &DL = F->getParent()->getDataLayout();

  ConstantInt *Zero = ConstantInt::getSigned(Int32Ty, 0u);
  ConstantInt *One = ConstantInt::getSigned(Int32Ty, 1u);

  // Create %is.last and initialize it with zero.
  IsLastVal = new AllocaInst(Int32Ty, DL.getAllocaAddrSpace(), "is.last",
                             &(W->getEntryBBlock()->front()));
  auto *Init = new StoreInst(Zero, IsLastVal);
  Init->insertAfter(IsLastVal);

  // Compare IV with the upper bound and store result to %is.last
  auto *IV = WRegionUtils::getOmpCanonicalInductionVariable(Loop);
  assert(IV && "no induction variable");

  auto *UB = WRegionUtils::getOmpLoopUpperBound(Loop);
  assert(UB && "no upper bound for the loop");

  auto *Cmp = new ICmpInst(ICmpInst::ICMP_EQ, IV, UB);
  Cmp->insertAfter(IV);

  auto *Select = SelectInst::Create(Cmp, One, Zero);
  Select->insertAfter(Cmp);

  auto *Store = new StoreInst(Select, IsLastVal);
  Store->insertAfter(Select);

  return true;
}

bool VPOParoptTransform::genCSAParallelSections(WRegionNode *W) {
  assert(isTargetCSA() && "unexpected target");

  auto *M = F->getParent();

  // Insert parallel region entry/exit calls
  auto *RID = genCSAParallelRegion(W);

  // CSA parallel section entry/exit intrinsics
  auto *SEntry = Intrinsic::getDeclaration(M,
    Intrinsic::csa_parallel_section_entry);

  auto *SExit = Intrinsic::getDeclaration(M,
    Intrinsic::csa_parallel_section_exit);

  // Insert section entry/exit calls in child work regions which all are
  // supposed to be sections.
  for (auto *WSec : W->getChildren()) {
    assert(WSec->getWRegionKindID() == WRegionNode::WRNSection &&
           "Unexpected work region kind");

    // Add section entry/exit calls
    IRBuilder<> Builder(WSec->getEntryBBlock()->getTerminator());
    auto *SID = Builder.CreateCall(SEntry, { RID }, "section");

    Builder.SetInsertPoint(WSec->getExitBBlock()->getFirstNonPHI());
    Builder.CreateCall(SExit, { SID }, {});

    // Remove directives from section
    VPOUtils::stripDirectives(WSec);
  }
  return true;
}