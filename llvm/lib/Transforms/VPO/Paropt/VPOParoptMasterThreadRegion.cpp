#if INTEL_COLLAB
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===--- VPOParoptMasterThreadRegion.cpp - Master Thread Region Support ---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements MasterThreadRegion, MasterThreadRegionFinder, and
/// related functions.
///
//===----------------------------------------------------------------------===//

#include "VPOParoptMasterThreadRegion.h"

#include "llvm/Analysis/VPO/VPOParoptConstants.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"

#include "llvm/Analysis/DomTreeUpdater.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Support/ModRef.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-target-spirv"

static cl::opt<bool> ExcludeGlobalFenceFromBarriers(
    "vpo-paropt-exclude-global-fence-from-workgroup-barriers", cl::Hidden,
    cl::init(false), cl::ZeroOrMore,
    cl::desc("Exclude global fence when adding workgroup barriers after "
             "parallel regions"));

/// Return true if the Instruction \p I has metadata indicating that it is
/// already guarded by a thread-check like `if (thread_id == xyz)` to ensure
/// that only one thread executes it.
static bool isGuardedByThreadCheck(const Instruction *I) {
  return I->hasMetadata(GuardedByThreadCheckMDStr);
}

namespace {

/// Returns the base pointer of \p Pointer if it is in the private address
/// space.
///
/// Otherwise returns nullptr.
static Value *privateBasePointer(Value *Pointer) {
  Value *RootPointer = Pointer->stripPointerCasts();
  if (cast<PointerType>(RootPointer->getType())->getAddressSpace() ==
          vpo::ADDRESS_SPACE_PRIVATE ||
      cast<PointerType>(Pointer->getType())->getAddressSpace() ==
          vpo::ADDRESS_SPACE_PRIVATE)
    return RootPointer;
  return nullptr;
}

/// The two values returned by getPossiblePrivatePointersStoredToBy.
struct PrivatePointersStoredResult {

  /// The list of thread-local pointers that could be stored to by the
  /// instruction.
  SmallVector<Value *, 1> PrivatePointersStored;

  /// Whether the instruction can only store to thread-local memory.
  bool NoNonPrivateStores;
};

/// Determines what thread-local memory could be written by \p I and whether
/// non-thread-local memory could also be stored.
static PrivatePointersStoredResult
getPossiblePrivatePointersStoredToBy(Instruction *I) {

  // Handle normal stores.
  if (auto *const StoreI = dyn_cast<StoreInst>(I)) {
    Value *StorePointer = StoreI->getPointerOperand();
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Store op::" << *StorePointer
                      << "\n");
    if (Value *const PrivateBase = privateBasePointer(StorePointer))
      return {{PrivateBase}, true};
    return {{}, false};
  }

  // Handle calls based on their attributes.
  if (auto *const CallI = dyn_cast<CallInst>(I)) {

    // Calls with these attributes are defined to only store to their first
    // operand.
    if (CallI->hasFnAttr("openmp-privatization-constructor") ||
        CallI->hasFnAttr("openmp-privatization-destructor") ||
        CallI->hasFnAttr("openmp-privatization-copyconstructor") ||
        CallI->hasFnAttr("openmp-privatization-copyassign")) {
      if (Value *const PrivateBase =
              privateBasePointer(CallI->getArgOperand(0)))
        return {{PrivateBase}, true};
      return {{}, false};
    }

    // If the call only reads from memory, there are no stores to
    // non-thread-local memory and we don't need to check for argument stores.
    if (!CallI->mayWriteToMemory())
      return {{}, true};

    // If the call could write to any non-argument memory, it might write to
    // non-thread-local memory.
    // TODO: It could also store to any thread-local memory too; we don't have a
    // good way of handling this properly yet.
    if (!CallI->getMemoryEffects()
             .getWithoutLoc(MemoryEffects::ArgMem)
             .onlyReadsMemory())
      return {{}, false};

    // Check each pointer argument to the call.
    SmallVector<Value *, 1> PrivateArgsStored;
    bool OnlyPrivateArgs = true;
    for (const auto Arg : enumerate(CallI->args())) {
      if (!Arg.value()->getType()->isPointerTy())
        continue;

      // Ignore arguments that are not written.
      if (CallI->onlyReadsMemory(Arg.index()))
        continue;

      LLVM_DEBUG({
        dbgs() << __FUNCTION__ << ": Argument " << Arg.index() << " (";
        Arg.value()->printAsOperand(dbgs());
        dbgs() << ") may be stored to by" << *CallI << "\n";
      });

      // Check if the memory this argument might write to is thread-local.
      if (Value *const PrivateBase = privateBasePointer(Arg.value()))
        PrivateArgsStored.push_back(PrivateBase);
      else
        OnlyPrivateArgs = false;
    }
    return {PrivateArgsStored, OnlyPrivateArgs};
  }

  // Other types of instructions aren't handled yet, and should be assumed to
  // possibly store to non-thread-local memory.
  // TODO: They could also store to thread-local memory.
  return {{}, false};
}

} // namespace

bool llvm::vpo::needsMasterThreadGuard(Instruction *I) {

  // Instructions without side-effects do not need master thread guards.
  if (!I->mayThrow() && !I->mayWriteToMemory())
    return false;

  // Ignore calls to the following functions.
  static const StringSet<> IgnoreCalls {
    "_Z13get_global_idj", "_Z12get_local_idj", "_Z14get_local_sizej",
        "_Z14get_num_groupsj", "_Z12get_group_idj",
        "_Z28__spirv_GlobalInvocationId_xv",
        "_Z28__spirv_GlobalInvocationId_yv",
        "_Z28__spirv_GlobalInvocationId_zv", "_Z27__spirv_LocalInvocationId_xv",
        "_Z27__spirv_LocalInvocationId_yv", "_Z27__spirv_LocalInvocationId_zv",
        "_Z23__spirv_WorkgroupSize_xv", "_Z23__spirv_WorkgroupSize_yv",
        "_Z23__spirv_WorkgroupSize_zv", "_Z23__spirv_NumWorkgroups_xv",
        "_Z23__spirv_NumWorkgroups_yv", "_Z23__spirv_NumWorkgroups_zv",
        "_Z21__spirv_WorkgroupId_xv", "_Z21__spirv_WorkgroupId_yv",
        "_Z21__spirv_WorkgroupId_zv", "_Z22__spirv_ControlBarrieriii",
        "_Z9mem_fencej", "_Z14read_mem_fencej", "_Z15write_mem_fencej",
#if INTEL_CUSTOMIZATION
        "_f90_dope_vector_init", "_f90_firstprivate_copy",
        "_f90_dope_vector_size", "_f90_lastprivate_copy",
#endif // INTEL_CUSTOMIZATION
        "omp_get_thread_num", "__kmpc_barrier"
  };

  // Ignore instructions that are already guarded by master-thread checks,
  // e.g. atomic-free reduction implementation's global update loop
  // is executed only by the master thread of a single team
  // so no additional guarding is required.
  if (isGuardedByThreadCheck(I))
    return false;

  if (const auto *const CallI = dyn_cast<CallInst>(I)) {
    // Unprototyped function calls may result in a call of a bitcasted
    // Function.
    const auto *const CalledF = CallI->getCalledOperand()->stripPointerCasts();
    assert(CalledF != nullptr && "Called Function not found.");
    if (CalledF->hasName() &&
        IgnoreCalls.contains(std::string(CalledF->getName())))
      return false;

    // Intrinsics besides memory intrinsics cannot have non-thread-local
    // side-effects and so don't need to be guarded.
    if (isa<IntrinsicInst>(CallI) && !isa<AnyMemIntrinsic>(CallI))
      return false;
  }

  // Instructions which only write to thread-local memory do not need to be
  // guarded, unless they might throw exceptions.
  if (!I->mayThrow() &&
      getPossiblePrivatePointersStoredToBy(I).NoNonPrivateStores)
    return false;

  return true;
}

CallInst *llvm::vpo::insertWorkGroupBarrier(Instruction *InsertPt,
                                            bool GlobalFence) {
  LLVMContext &C = InsertPt->getContext();

  LLVM_DEBUG(dbgs() << "\n"
                    << __FUNCTION__ << ": Insert Barrier before:" << *InsertPt
                    << "\n");

  uint64_t MemSemanticsFlags =
      spirv::SequentiallyConsistent | spirv::WorkgroupMemory;
  // Experimental option is used to remove global memory fence from the
  // barrier instruction.
  if (GlobalFence && !ExcludeGlobalFenceFromBarriers)
    MemSemanticsFlags |= spirv::CrossWorkgroupMemory;

  // TODO: we only need global fences for side effect instructions
  //       inside "omp target" and outside of the enclosed regions.
  //       Moreover, it probably makes sense to guard such instructions
  //       with (get_group_id() == 0) vs (get_local_id() == 0).
  CallInst *CI = VPOParoptUtils::genCall(
      "_Z22__spirv_ControlBarrieriii", Type::getVoidTy(C),
      // The arguments are:
      //   (Scope Execution, Scope Memory, MemorySemantics Semantics)
      //
      // work_group_barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE)
      // translates into:
      // __spirv_ControlBarrier(
      //     vpo::spirv::Scope::Workgroup,
      //     vpo::spirv::Scope::Workgroup,
      //     vpo::spirv::MemorySemantics::SequentiallyConsistent |
      //     vpo::spirv::MemorySemantics::WorkgroupMemory |
      //     vpo::spirv::MemorySemantics::CrossWorkgroupMemory)
      {ConstantInt::get(Type::getInt32Ty(C), spirv::Workgroup),
       ConstantInt::get(Type::getInt32Ty(C), spirv::Workgroup),
       ConstantInt::get(Type::getInt32Ty(C), MemSemanticsFlags)},
      InsertPt);
  // __spirv_ControlBarrier() is a convergent call.
  CI->getCalledFunction()->setConvergent();
  return CI;
}

MasterThreadRegion::MasterThreadRegion(Instruction *Inst, bool Critical)
    : Start(Inst), End(Inst->getNextNode()), Critical(Critical) {}

bool MasterThreadRegion::contains(const Instruction *I) const {

  // I is contained in the region if it's in a fully enclosed basic block.
  const BasicBlock *const BB = I->getParent();
  if (EnclosedBBs.contains(BB))
    return true;

  // Otherwise, I needs to be in the same basic block as Start and/or End, after
  // Start and/or before End.
  const BasicBlock *const StartBB = Start->getParent();
  const BasicBlock *const EndBB = End->getParent();
  const BasicBlock::const_iterator StartIt = Start->getIterator();
  const BasicBlock::const_iterator EndIt = End->getIterator();
  const auto IsI = [I](const Instruction &CurI) { return &CurI == I; };
  if (BB == StartBB) {
    if (StartBB == EndBB)
      return std::any_of(StartIt, EndIt, IsI);
    return std::any_of(StartIt, StartBB->end(), IsI);
  }
  if (BB == EndBB)
    return std::any_of(EndBB->begin(), EndIt, IsI);

  return false;
}

void MasterThreadRegion::insertBroadcasts(Instruction *TargetDirectiveBegin) {

  // If this is a one-instruction region, it's possible that we were unable to
  // form a region that does not require broadcasting a value. Insert a new
  // workgroup-local variable and a store/load pair to broadcast through it.
  // The store needs to go at the end of the region and the load just outside
  // the end. This code:
  //
  // ; Region Start
  // %c = call spir_func i32 @_Z3barPi(i32 addrspace(4)* %8)
  // ; Region End
  // ; Uses of %c
  //
  // Will be replaced with:
  //
  // @c.broadcast.ptr.__local = internal addrspace(3) global i32 0
  //
  // ; Region Start
  // %c = call spir_func i32 @_Z3barPi(i32 addrspace(4)* %8)
  // store i32 %c, i32 addrspace(3)* @c.broadcast.ptr.__local
  // ; Region End
  // %c.new = load i32, i32 addrspace(3)* @c.broadcast.ptr.__local
  // ; Uses of %c.new
  if (Start->getNextNode() == End && !Start->use_empty()) {

    // Create a new workgroup-local variable for the broadcast:
    const DataLayout &DL = Start->getModule()->getDataLayout();
    MaybeAlign Alignment = std::nullopt;
    if (Start->getType()->isPointerTy()) {
      Align MinAlign = Start->getPointerAlignment(DL);
      if (MinAlign > 1)
        Alignment = MinAlign;
    }
    Value *const TeamLocalVal = VPOParoptUtils::genPrivatizationAlloca(
        Start->getType(), nullptr, Alignment, TargetDirectiveBegin, true,
        Start->getName() + ".broadcast.ptr", vpo::ADDRESS_SPACE_LOCAL);

    // Add the store and load. The region end is adjusted to include the store
    // but not the load.
    IRBuilder<> Builder(End);
    StoreInst *const StoreGuardedInstValue =
        Builder.CreateStore(Start, TeamLocalVal);
    LoadInst *const LoadSavedValue = Builder.CreateLoad(
        Start->getType(), TeamLocalVal, Start->getName() + ".new");
    End = LoadSavedValue;

    // Replace all of the original uses with the loaded value, except for the
    // one in the store.
    Start->replaceAllUsesWith(LoadSavedValue);
    StoreGuardedInstValue->replaceUsesOfWith(LoadSavedValue, Start);
  }
}

// Determines if \p Inst is a barrier instruction.
//
// Returns false if \p Inst is nullptr.
static bool isNonNullAndBarrier(const Instruction *Inst) {
  if (const auto *const Call = dyn_cast_or_null<CallInst>(Inst))
    if (Call->getCalledFunction()->getName() == "_Z22__spirv_ControlBarrieriii")
      return true;
  return false;
}

void MasterThreadRegion::insertBarriers() {

  // Skip barrier insertion if the region is marked as Critical.
  if (Critical)
    return;

  // Only insert a barrier before the region if the preceding non-debug
  // instruction is not also a barrier.
  //
  // Barrier
  // ; Region Start
  // ...
  // ; Region End
  if (!isNonNullAndBarrier(Start->getPrevNonDebugInstruction()))
    insertWorkGroupBarrier(Start, true);

  // Insert a barrier after the region unconditionally.
  //
  // Barrier
  // ; Region Start
  // ...
  // ; Region End
  // Barrier
  CallInst *const EndBarrier = insertWorkGroupBarrier(End, true);
  End = EndBarrier;
}

void MasterThreadRegion::insertGuard(Value *MasterCheckPredicate,
                                     DomTreeUpdater &DTU, LoopInfo &LI) {

  LLVM_DEBUG({
    const Instruction &Last = End->getPrevNode() ? *End->getPrevNode() : *End;
    dbgs() << "\n"
           << __FUNCTION__ << ": Guarding instructions from:\n"
           << *Start << "\nto:\n"
           << Last << "\n";
  });

  // This is a generalization of the previous guard insertion to support master
  // thread regions spanning multiple basic blocks, such as this one:
  //
  // BB1: ; PreBB
  //   ...
  //   ; Region Start
  //   ...
  //   br label %BB2
  //
  // BB2:
  //   ...
  //   ; Region End
  //   ...
  BasicBlock *const PreBB = Start->getParent();

  // Start by splitting at the region start:
  //
  // BB1: ; PreBB
  //   ...
  //   br label %master.thread.code
  //
  // master.thread.code: ; ThenBB
  //   ; Region Start
  //   ...
  //   br label %BB2
  //
  // BB2:
  //   ...
  //   ; Region End
  //   ...
  BasicBlock *const ThenBB =
      SplitBlock(PreBB, Start, &DTU, &LI, nullptr, "master.thread.code");

  // Then split at the region end:
  //
  // BB1: ; PreBB
  //   ...
  //   br label %master.thread.code
  //
  // master.thread.code: ; ThenBB
  //   ; Region Start
  //   ...
  //   br label %BB2
  //
  // BB2:
  //   ...
  //   br label %master.thread.fallthru
  //
  // master.thread.fallthru: ; ElseBB
  //   ; Region End
  //   ...
  BasicBlock *const ElseBB = SplitBlock(End->getParent(), End, &DTU, &LI,
                                        nullptr, "master.thread.fallthru");

  // And insert a conditional branch to complete the guard:
  //
  // BB1: ; PreBB
  //   ...
  //   br i1 %is.master.thread, label %master.thread.code, label
  //     %master.thread.fallthru
  //
  // master.thread.code: ; ThenBB
  //   ; Region Start
  //   ...
  //   br label %BB2
  //
  // BB2:
  //   ...
  //   br label %master.thread.fallthru
  //
  // master.thread.fallthru: ; ElseBB
  //   ; Region End
  //   ...
  BranchInst *const GuardBranch =
      BranchInst::Create(ThenBB, ElseBB, MasterCheckPredicate);
  ReplaceInstWithInst(PreBB->getTerminator(), GuardBranch);
  DTU.applyUpdates(
      DominatorTree::UpdateType(DominatorTree::Insert, PreBB, ElseBB));
}

Instruction *MasterThreadRegionFinder::multiBBRegionEnd(BasicBlock *BB) const {
  for (const MasterThreadRegion &Region : Regions) {
    if (Region.End->getParent() == BB && Region.Start->getParent() != BB)
      return Region.End;
    if (Region.EnclosedBBs.contains(BB))
      return BB->getTerminator();
  }
  return BB->getFirstNonPHIOrDbgOrLifetime();
}

Instruction *
MasterThreadRegionFinder::multiBBRegionStart(BasicBlock *BB) const {
  for (const MasterThreadRegion &Region : Regions)
    if (Region.Start->getParent() == BB && Region.End->getParent() != BB)
      return Region.Start;
  return BB->getTerminator();
}

const MasterThreadRegion &
MasterThreadRegionFinder::findMasterThreadRegion(Instruction *Inst,
                                                 bool Critical) {

  LLVM_DEBUG({
    // This function shouldn't need to be called on any instruction already in a
    // master thread region.
    for (const MasterThreadRegion &Region : Regions)
      assert(!Region.contains(Inst));
  });

  // Start with a master thread region containing only Inst.
  assert(Inst->getNextNode());
  MasterThreadRegion Region(Inst, Critical);

  // TODO: Full region expansion will happen here when implemented. For now,
  // regions are only expanded to include immediate successor side-effect (and
  // debug) instructions and only if none of the instructions have uses. This
  // preserves the current master thread guarding behavior.
  (void)ParBBSet;
  (void)DT;
  if (Inst->use_empty()) {
    Instruction *NewLastInst;
    while ((NewLastInst =
                Region.End->getPrevNode()->getNextNonDebugInstruction()) &&
           NewLastInst->use_empty() && needsMasterThreadGuard(NewLastInst) &&
           NewLastInst->getNextNode())
      Region.End = NewLastInst->getNextNode();
  }

  // Add the completed region to the list and return it.
  Regions.push_back(std::move(Region));
  return Regions.back();
}

#endif // INTEL_COLLAB
