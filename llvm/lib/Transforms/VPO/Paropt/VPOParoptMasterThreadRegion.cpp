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
#include "llvm/IR/Assumptions.h"
#include "llvm/Support/ModRef.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-target-spirv"

static cl::opt<bool> ExcludeGlobalFenceFromBarriers(
    "vpo-paropt-exclude-global-fence-from-workgroup-barriers", cl::Hidden,
    cl::init(false), cl::ZeroOrMore,
    cl::desc("Exclude global fence when adding workgroup barriers after "
             "parallel regions"));

static cl::opt<bool> MasterThreadRegionExpansion(
    "vpo-paropt-master-thread-region-expansion", cl::Hidden, cl::init(true),
    cl::ZeroOrMore,
    cl::desc(
        "This can be set to false to use the old master thread guarding "
        "strategy which only guards specific instructions with side-effects "
        "instead of the new master thread region expansion strategy. This is "
        "intended to help with analyzing possible performance differences."));

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
             .getWithoutLoc(IRMemLocation::ArgMem)
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

/// Returns a list of alloca base pointers for thread-local memory written by
/// \p I.
///
/// Returns nullopt if any thread-local memory written by \p I does not have an
/// alloca base pointer.
static std::optional<SmallVector<AllocaInst *, 1>>
getPossiblePrivateAllocasStoredToBy(Instruction *I) {
  SmallVector<AllocaInst *, 1> AllocasStored;
  for (Value *const StoredValue :
       getPossiblePrivatePointersStoredToBy(I).PrivatePointersStored) {
    if (auto *const StoredAlloca = dyn_cast<AllocaInst>(StoredValue)) {
      AllocasStored.push_back(StoredAlloca);
    } else {
      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": No alloca for private pointer:"
                        << *StoredValue << "\n");
      return {};
    }
  }
  return AllocasStored;
}

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
        "_f90_dope_vector_init", "_f90_dope_vector_init2",
        "_f90_firstprivate_copy", "_f90_dope_vector_size",
        "_f90_lastprivate_copy",
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

/// Determines if a store to \p AllocaVal is allowed in this region based on all
/// of its uses.
///
/// \p RequiresInst is called for every load of the alloca.
template <typename RequiresInstCallback>
static bool storeToAllocaAllowed(Value *AllocaVal,
                                 const RequiresInstCallback &RequiresInst) {
  const auto UserAllowed = [AllocaVal, &RequiresInst](User *AllocaUser) {
    // Only instruction users are allowed for now.
    auto *const UserInst = dyn_cast<Instruction>(AllocaUser);
    if (!UserInst) {
      LLVM_DEBUG(dbgs() << __FUNCTION__
                        << ": Non-instruction user:" << *AllocaUser << "\n");
      return false;
    }

    // Casts and GEPs should have their users added to the region as needed.
    if (UserInst->isCast() || isa<GetElementPtrInst>(UserInst))
      return storeToAllocaAllowed(UserInst, RequiresInst);

    // Debug and pseudo-instructions can be ignored.
    if (UserInst->isDebugOrPseudoInst())
      return true;

    // Loads of this value must be included in the region.
    if (auto *const Load = dyn_cast<LoadInst>(UserInst))
      return RequiresInst(Load);

    // Stores to this value can be ignored, but stores of this value need to
    // happen on the master thread. This ensures that other threads can't
    // sneakily get around the requirement that values stored in the master
    // thread region are loaded in the master thread region by storing the
    // pointer and reading from it elsewhere.
    if (auto *const Store = dyn_cast<StoreInst>(UserInst))
      return Store->getValueOperand() != AllocaVal || RequiresInst(Store);

    // Similar to loads and stores, calls need to be included in the region if
    // they read from the alloca or if they might capture its pointer value.
    if (auto *const Call = dyn_cast<CallInst>(UserInst)) {
      const bool CanReadArgMem =
          isRefSet(Call->getMemoryEffects().getModRef(IRMemLocation::ArgMem));
      for (const auto Arg : enumerate(Call->args()))
        if (Arg.value() == AllocaVal)
          if ((CanReadArgMem && !Call->onlyWritesMemory(Arg.index())) ||
              !Call->doesNotCapture(Arg.index()))
            if (!RequiresInst(Call))
              return false;
      return true;
    }

    // Any other instructions are not allowed.
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Unknown instruction:" << *UserInst
                      << "\n");
    return false;
  };

  return all_of(AllocaVal->users(), UserAllowed);
}

/// Determines if \p I is allowed to be contained in \p Region.
///
/// If \p I is only allowed if certain other instructions are also contained in
/// the same region, \p RequiresInst (a callback with signature
/// bool RequiresInst(Instruction*)) will be called for each of those
/// instructions and can return false if adding that instruction fails
/// immediately.
template <typename RequiresInstCallback>
static bool
allowedInMasterThreadRegion(MasterThreadRegion &Region, Instruction *I,
                            const RequiresInstCallback &RequiresInst) {

  // Instructions that are already guarded don't need to be checked any further.
  if (isGuardedByThreadCheck(I))
    return true;

  // Barrier calls are not allowed in master thread regions because the
  // non-convergent control flow makes them hang.
  if (auto *const CallI = dyn_cast<CallInst>(I)) {
    static const StringSet<> BarrierCalls{"_Z22__spirv_ControlBarrieriii",
                                          "__kmpc_barrier"};
    const Value *const CalledF = CallI->getCalledOperand()->stripPointerCasts();
    assert(CalledF && "Called Function not found.");
    if (BarrierCalls.contains(CalledF->getName())) {
      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Barrier not allowed:" << *CallI
                        << "\n");
      return false;
    }
  }

  // Instructions that store to thread-local memory are only allowed if alloca
  // base pointers can be found for all thread-local stores, all uses of the
  // alloca base pointers are simple uses that can be tracked here, and any
  // loads are also in this region.
  SmallVector<AllocaInst *> AllocasStored;
  if (const auto AllocasStored = getPossiblePrivateAllocasStoredToBy(I)) {
    for (AllocaInst *const StoreAlloca : *AllocasStored)
      if (Region.newStoredToAlloca(StoreAlloca) &&
          !storeToAllocaAllowed(StoreAlloca, RequiresInst))
        return false;
  } else {
    return false;
  }

  // At this point, all instructions are allowed as long as their
  // non-debug/pseudo-inst users are also in the region.
  for (User *const IUser : I->users()) {
    if (auto *const UserInst = dyn_cast<Instruction>(IUser)) {
      if (!UserInst->isDebugOrPseudoInst() && !RequiresInst(UserInst))
        return false;
    } else {
      return false;
    }
  }
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

  // All barriers that we add here are "aligned", meaning that all threads in
  // the team will reach them in the same order. This special annotation makes
  // it possible for OpenMPOpt to optimize them.
  addAssumptions(*CI, {"ompx_aligned_barrier"});

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
                                     DomTreeUpdater &DTU, LoopInfo &LI,
                                     WRegionNode *W) {

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
  W->updateBBsAfterSplit(PreBB, ThenBB);

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
  // BB2: ; PreSplitEndBB
  //   ...
  //   br label %master.thread.fallthru
  //
  // master.thread.fallthru: ; ElseBB
  //   ; Region End
  //   ...
  BasicBlock *const PreSplitEndBB = End->getParent();
  BasicBlock *const ElseBB = SplitBlock(PreSplitEndBB, End, &DTU, &LI, nullptr,
                                        "master.thread.fallthru");
  W->updateBBsAfterSplit(PreSplitEndBB, ElseBB);

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
  // BB2: ; PreSplitEndBB
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

bool llvm::vpo::operator==(const MasterThreadRegion &A,
                           const MasterThreadRegion &B) {
  return A.getStart() == B.getStart() && A.getEnd() == B.getEnd();
}

#ifndef NDEBUG
void MasterThreadRegionFinder::assertStructuralInvariants(
    MasterThreadRegion &Region) const {
  assert(Region.Start != Region.End);
  const BasicBlock *const StartBB = Region.Start->getParent();
  const BasicBlock *const EndBB = Region.End->getParent();
  assert(!ParBBSet.contains(StartBB));
  assert(!ParBBSet.contains(EndBB));
  assert(!Region.EnclosedBBs.contains(StartBB));
  assert(!Region.EnclosedBBs.contains(EndBB));
  assert(DT.dominates(StartBB, EndBB));
  assert(PDT.dominates(EndBB, StartBB));
  if (StartBB != EndBB || !Region.Start->comesBefore(Region.End)) {
    for (const BasicBlock *const Succ : successors(StartBB))
      assert(Succ == EndBB || Region.EnclosedBBs.contains(Succ));
    for (const BasicBlock *const Pred : predecessors(EndBB))
      assert(Pred == StartBB || Region.EnclosedBBs.contains(Pred));
  }
  for (const BasicBlock *const Enclosed : Region.EnclosedBBs) {
    assert(!ParBBSet.contains(Enclosed));
    assert(DT.dominates(StartBB, Enclosed));
    assert(PDT.dominates(EndBB, Enclosed));
    for (const BasicBlock *const Succ : successors(Enclosed))
      assert(Succ == EndBB || Region.EnclosedBBs.contains(Succ));
    for (const BasicBlock *const Pred : predecessors(Enclosed))
      assert(Pred == StartBB || Region.EnclosedBBs.contains(Pred));
  }
}
#endif // NDEBUG

/// Determines \p BB's immediate dominator.
///
/// For our usage \p BB should be reachable and not the entry node.
static BasicBlock *getImmediateDominator(const DominatorTree &DT,
                                         BasicBlock *BB) {
  const DomTreeNode *const Node = DT.getNode(BB);
  assert(Node && "BB isn't reachable?");
  const DomTreeNode *const DomNode = Node->getIDom();
  assert(DomNode && DomNode->getBlock() && "BB has no dominator?");
  return DomNode->getBlock();
}

/// Determines \p BB's immediate post-dominator.
///
/// There may be no post-dominator (if \p BB is in an infinite loop); in that
/// case this returns nullptr.
static BasicBlock *getImmediatePostDominator(const PostDominatorTree &PDT,
                                             BasicBlock *BB) {
  const DomTreeNode *const Node = PDT.getNode(BB);
  assert(Node && "BB isn't in the post-dominator tree?");
  const DomTreeNode *const DomNode = Node->getIDom();
  assert(DomNode && "BB has no post-dominator node?");
  return DomNode->getBlock();
}

bool MasterThreadRegionFinder::expandUntilStructurallyValid(
    MasterThreadRegion &Region, Instruction *NewStart,
    Instruction *NewEnd) const {

#ifndef NDEBUG
  // Double-check that the incoming region is structurally valid and that it is
  // dominated/post-dominated by the new start/end. Note that
  // DominatorTree::dominates determines *strict* dominance when called with two
  // Instructions, unlike the BasicBlock overload or
  // PostDominatorTree::dominates.
  assertStructuralInvariants(Region);
  assert(NewStart == Region.Start || DT.dominates(NewStart, Region.Start));
  assert(PDT.dominates(NewEnd, Region.End));
#endif // NDEBUG

  // Update the region start/end, while keeping track of the old start/end basic
  // blocks.
  BasicBlock *const OrigStartBB = Region.Start->getParent();
  BasicBlock *const OrigEndBB = Region.End->getParent();
  Region.Start = NewStart;
  Region.End = NewEnd;

  // If the region starts and ends in the same basic block and Start precedes
  // End, the region is trivially valid. These types of regions are assumed to
  // not already be inside a parallel region.
  BasicBlock *StartBB = Region.Start->getParent();
  BasicBlock *EndBB = Region.End->getParent();
  if (StartBB == EndBB && Region.Start->comesBefore(Region.End)) {
    assert(!ParBBSet.contains(StartBB));
    return true;
  }

  // We need to ensure that StartBB strictly dominates EndBB and EndBB strictly
  // post-dominates StartBB. We need to keep these updated every time they're
  // changed to make sure the dominance/post-dominance conditions still hold,
  // and this may take several iterations because fixing one condition might
  // break the other. For instance, if we have this CFG (all edges pointing
  // down):
  //     E
  //    /|
  //   C |
  //  /|\ \
  // A | | |
  //  \| | |
  //   B | |
  //    \|/
  //     D
  // Starting with a non-valid region from A to B, we need to move the start to
  // C so that it dominates B. The start (C) is no longer post-dominated by B,
  // so we need to move the end to D. The end (D) is no longer dominated by C,
  // so we need to move the start to E. This finally satisfies both dominance
  // and post-dominance conditions.
  const auto EnforceDominance = [&]() {
    for (;;) {
      if (!DT.dominates(StartBB, EndBB)) {
        StartBB = DT.findNearestCommonDominator(StartBB, EndBB);
        if (StartBB == EndBB)
          StartBB = getImmediateDominator(DT, StartBB);
        assert(StartBB->getTerminator());
        Region.Start = StartBB->getTerminator();
        LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Expanded region start to"
                          << *Region.Start << "\n");
      }
      if (!PDT.dominates(EndBB, StartBB)) {
        EndBB = PDT.findNearestCommonDominator(EndBB, StartBB);
        if (EndBB == StartBB)
          EndBB = getImmediatePostDominator(PDT, EndBB);
        if (!EndBB) {
          LLVM_DEBUG(dbgs() << __FUNCTION__
                            << ": Could not find common post-dominator\n");
          return false;
        }
        assert(EndBB->getFirstNonPHIOrDbgOrLifetime());
        Region.End = EndBB->getFirstNonPHIOrDbgOrLifetime();
        LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Expanded region end to"
                          << *Region.End << "\n");
        continue;
      }
      if (ParBBSet.contains(StartBB)) {
        LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Reached parallel start block "
                          << StartBB->getNameOrAsOperand() << "\n");
        return false;
      }
      if (ParBBSet.contains(EndBB)) {
        LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Reached parallel end block "
                          << EndBB->getNameOrAsOperand() << "\n");
        return false;
      }
      return true;
    }
  };
  if (!EnforceDominance())
    return false;

  // Start checking block predecessors/successors at the new and previous start
  // and end blocks and continue until all blocks in a valid region have been
  // checked.
  SmallVector<BasicBlock *> ToCheck{StartBB, EndBB, OrigStartBB, OrigEndBB};
  while (!ToCheck.empty()) {
    BasicBlock *const BB = ToCheck.pop_back_val();
    assert(DT.dominates(StartBB, BB));
    assert(PDT.dominates(EndBB, BB));

    // If BB is not the end block, check its successors.
    if (BB != EndBB) {
      for (BasicBlock *const Succ : successors(BB)) {

        // All blocks pushed to ToCheck should be post-dominated by EndBB, so
        // (except for EndBB) their successors should also be post-dominated by
        // EndBB.
        assert(PDT.dominates(EndBB, Succ));

        // Don't visit EndBB again.
        if (Succ == EndBB)
          continue;

        // Also don't visit this block again if it's already recorded as
        // enclosed.
        if (Region.EnclosedBBs.contains(Succ))
          continue;

        // Region expansion failed if this block is in a parallel region.
        if (ParBBSet.contains(Succ)) {
          LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Reached parallel successor "
                            << Succ->getNameOrAsOperand() << "\n");
          return false;
        }

        // This block is allowed in the region; mark it as enclosed and add it
        // to ToCheck so its successors/predecessors are also checked.
        Region.EnclosedBBs.insert(Succ);
        ToCheck.push_back(Succ);

        // If this successor is not strictly dominated by the start block, we
        // have an extra edge out of the region. To contain it, the start block
        // needs to be updated to strictly dominate the successor.
        if (!DT.properlyDominates(StartBB, Succ)) {
          BasicBlock *const OldStartBB = StartBB;
          BasicBlock *const OldEndBB = EndBB;
          StartBB = DT.findNearestCommonDominator(StartBB, Succ);
          if (Succ == StartBB)
            StartBB = getImmediateDominator(DT, StartBB);
          LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Expanded region start to "
                            << StartBB->getNameOrAsOperand() << "\n");
          assert(StartBB->getTerminator());
          Region.Start = StartBB->getTerminator();
          if (!EnforceDominance())
            return false;
          ToCheck.push_back(OldStartBB);
          ToCheck.push_back(StartBB);
          if (EndBB != OldEndBB) {
            ToCheck.push_back(OldEndBB);
            ToCheck.push_back(EndBB);
          }
        }
      }
    }

    // If BB is not the start block, check its predecessors. All blocks pushed
    // to ToCheck should be dominated by StartBB, so (except for StartBB) their
    // predecessors should also be dominated by StartBB. The predecessors still
    // need to be checked to make sure they're dominated by EndBB.
    if (BB != StartBB) {
      for (BasicBlock *const Pred : predecessors(BB)) {

        // All blocks pushed to ToCheck should be dominated by StartBB, so
        // (except for StartBB) their predecessors should also be dominated by
        // StartBB.
        assert(DT.dominates(StartBB, Pred));

        // Don't visit StartBB again.
        if (Pred == StartBB)
          continue;

        // Also don't visit this block again if it's already recorded as
        // enclosed.
        if (Region.EnclosedBBs.contains(Pred))
          continue;

        // Region expansion failed if this block is in a parallel region.
        if (ParBBSet.contains(Pred)) {
          LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Reached parallel predecessor "
                            << Pred->getNameOrAsOperand() << "\n");
          return false;
        }

        // This block is allowed in the region; mark it as enclosed and add it
        // to ToCheck so its successors/predecessors are also checked.
        Region.EnclosedBBs.insert(Pred);
        ToCheck.push_back(Pred);

        // If this predecessor is not strictly post-dominated by the end block,
        // we have an extra edge into the region. To contain it, the end block
        // needs to be updated to strictly post-dominate the predecessor. If
        // there is no post-dominator, region expansion failed.
        if (!PDT.properlyDominates(EndBB, Pred)) {
          BasicBlock *const OldStartBB = StartBB;
          BasicBlock *const OldEndBB = EndBB;
          EndBB = PDT.findNearestCommonDominator(EndBB, Pred);
          if (!EndBB) {
            LLVM_DEBUG(dbgs() << __FUNCTION__
                              << ": Could not find common post-dominator\n");
            return false;
          }
          if (Pred == EndBB)
            EndBB = getImmediatePostDominator(PDT, EndBB);
          if (!EndBB) {
            LLVM_DEBUG(dbgs()
                       << __FUNCTION__ << ": Could not find post-dominator\n");
            return false;
          }
          LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Expanded region end to "
                            << EndBB->getNameOrAsOperand() << "\n");
          assert(EndBB->getFirstNonPHIOrDbgOrLifetime());
          Region.End = EndBB->getFirstNonPHIOrDbgOrLifetime();
          if (!EnforceDominance())
            return false;
          if (StartBB != OldStartBB) {
            ToCheck.push_back(OldStartBB);
            ToCheck.push_back(StartBB);
          }
          ToCheck.push_back(OldEndBB);
          ToCheck.push_back(EndBB);
        }
      }
    }
  }

#ifndef NDEBUG
  // The region should be structurally valid now; double-check the invariants to
  // make sure it was expanded correctly.
  assert(StartBB == Region.Start->getParent());
  assert(EndBB == Region.End->getParent());
  assertStructuralInvariants(Region);
#endif // NDEBUG
  return true;
}

/// Finds the immediate dominating non-debug instruction of \p Inst.
///
/// This can just be the predecessor instruction of \p Inst if it's not the
/// first non-phi/debug/lifetime instruction in its block; otherwise, it will be
/// the terminator of its immediate dominator basic block.
static Instruction *getImmediateDominator(const DominatorTree &DT,
                                          Instruction *Inst) {
  if (Instruction *const Predecessor = Inst->getPrevNonDebugInstruction())
    if (!isa<PHINode>(Predecessor) && !Predecessor->isLifetimeStartOrEnd())
      return Predecessor;
  BasicBlock *Dominator = getImmediateDominator(DT, Inst->getParent());
  assert(Dominator->getTerminator());
  return Dominator->getTerminator();
}

/// Finds the immediate post-dominating non-debug instruction of \p Inst.
///
/// This can just be the successor instruction of \p Inst if it's not a
/// terminator; otherwise, it will be the first non-phi/debug/lifetime in its
/// immediate post-dominator basic block. If there is no post-dominator block
/// (because \p Inst is at the end of an infinite loop or for some other reason
/// doesn't properly exit the target region), returns nullptr.
static Instruction *getImmediatePostDominator(const PostDominatorTree &PDT,
                                              Instruction *Inst) {
  if (Instruction *const Successor = Inst->getNextNonDebugInstruction())
    return Successor;
  if (BasicBlock *const PostDominator =
          getImmediatePostDominator(PDT, Inst->getParent()))
    return PostDominator->getFirstNonPHIOrDbgOrLifetime();
  return nullptr;
}

bool MasterThreadRegionFinder::expandStructurallyToContain(
    MasterThreadRegion &Region, Instruction *Inst) const {

  // If Region already contains Inst, nothing needs to be done.
  if (Region.contains(Inst))
    return true;

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Expanding region to contain" << *Inst
                    << "\n");

  // We cannot split a basic block ahead of any phis, so if this is a phi the
  // region needs to contain any successor phis and also the immediate dominator
  // branch for the basic block.
  if (isa<PHINode>(Inst)) {
    assert(Inst->getNextNode());
    if (isa<PHINode>(Inst->getNextNode()))
      return expandStructurallyToContain(Region, Inst->getNextNode());

    BasicBlock *const Dominator = getImmediateDominator(DT, Inst->getParent());
    assert(Dominator->getTerminator());
    if (!expandStructurallyToContain(Region, Dominator->getTerminator()))
      return false;
  }

  // Region needs to be expanded; find a new common dominator/strict
  // post-dominator to expand to. Note that DominatorTree::dominates determines
  // *strict* dominance when called with two Instructions, unlike the BasicBlock
  // overload or PostDominatorTree::dominates, but this isn't relevant in this
  // case because Inst can't be contained in Region.
  Instruction *NewStart = Region.Start;
  Instruction *NewEnd = Region.End;
  assert(NewStart != Inst);
  if (!DT.dominates(NewStart, Inst)) {
    NewStart = DT.findNearestCommonDominator(NewStart, Inst);
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Expanding region start to"
                      << *NewStart << "\n");
  }
  if (!PDT.dominates(NewEnd, Inst) || NewEnd == Inst) {
    if (PDT.dominates(Inst, NewEnd))
      NewEnd = getImmediatePostDominator(PDT, Inst);
    else if (BasicBlock *const CommonPostDominator =
                 PDT.findNearestCommonDominator(NewEnd->getParent(),
                                                Inst->getParent()))
      NewEnd = CommonPostDominator->getFirstNonPHIOrDbgOrLifetime();
    else
      NewEnd = nullptr;
    if (!NewEnd) {
      LLVM_DEBUG(dbgs() << __FUNCTION__
                        << ": Could not find a common post-dominator\n");
      return false;
    }
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Expanding region end to" << *NewEnd
                      << "\n");
  }

  // Keep expanding region until it is structurally valid.
  return expandUntilStructurallyValid(Region, NewStart, NewEnd);
}

std::optional<MasterThreadRegion> MasterThreadRegionFinder::expandUntilValid(
    MasterThreadRegion Region,
    std::optional<MasterThreadRegion> PrevRegion) const {

  // If Region and PrevRegion are equivalent, we have nothing left to check.
  if (PrevRegion && Region == *PrevRegion) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Found valid region from:\n"
                      << *Region.Start << "\nto:\n"
                      << *Region.End << "\n");
    return Region;
  }

  // Otherwise, start NewRegion which will be expanded structurally until it
  // contains all the relevant uses/loads for new instructions in Region.
  MasterThreadRegion NewRegion = Region;

  // For each instruction to be checked, check whether it is allowed in a master
  // thread region if it hasn't already been checked in PrevRegion. If other
  // instructions also need to be in the region for this one to be allowed,
  // expand NewRegion to contain those.
  const auto InstructionAllowed = [this, &NewRegion,
                                   &PrevRegion](Instruction &I) {
    if (PrevRegion && PrevRegion->contains(&I))
      return true;
    const auto ExpandToContain = [this, &NewRegion](Instruction *Inst) {
      return expandStructurallyToContain(NewRegion, Inst);
    };
    return allowedInMasterThreadRegion(NewRegion, &I, ExpandToContain);
  };

  // Check all contained instructions in Region's start block.
  BasicBlock *const StartBB = Region.Start->getParent();
  BasicBlock *const EndBB = Region.End->getParent();
  const BasicBlock::iterator StartBBBegin = Region.Start->getIterator();
  BasicBlock::iterator StartBBEnd = StartBB->end();
  if (EndBB == StartBB)
    StartBBEnd = Region.End->getIterator();
  if (!std::all_of(StartBBBegin, StartBBEnd, InstructionAllowed))
    return {};

  // Check all instructions in blocks entirely contained by Region. Blocks also
  // entirely contained by PrevRegion have already been checked.
  for (BasicBlock *EnclosedBB : Region.EnclosedBBs) {
    if (PrevRegion && PrevRegion->EnclosedBBs.contains(EnclosedBB))
      continue;
    if (!all_of(*EnclosedBB, InstructionAllowed))
      return {};
  }

  // Check all contained instructions in Region's end block. This is only needed
  // for multi-block regions, because the end block will have already been
  // checked if it is also the start block.
  if (EndBB != StartBB)
    if (!std::all_of(EndBB->begin(), Region.End->getIterator(),
                     InstructionAllowed))
      return {};

  // Tail-recurse to also check any new instructions added in NewRegion.
  return expandUntilValid(NewRegion, Region);
}

MasterThreadRegion
MasterThreadRegionFinder::expandStart(MasterThreadRegion Region) const {

  // Find the immediate dominator instruction of the region's start.
  Instruction *const NewStart = getImmediateDominator(DT, Region.Start);

  // If this new immediate dominator is the target region start, we're done here
  // and can't expand any further.
  if (VPOAnalysisUtils::getDirectiveID(NewStart) == DIR_OMP_TARGET) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Reached target region start"
                      << *NewStart << "\n");
    return Region;
  }

  // Otherwise, attempt to expand the region start to the new immediate
  // dominator. If this succeeds, continue expanding to the next one.
  MasterThreadRegion NewRegion = Region;
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Expanding region start to"
                    << *NewStart << "\n");
  if (!expandUntilStructurallyValid(NewRegion, NewStart, Region.End))
    return Region;
  if (const std::optional<MasterThreadRegion> ValidRegion =
          expandUntilValid(std::move(NewRegion), Region))
    return expandStart(*ValidRegion);

  // Otherwise, this is expanded as much as it can be and should be returned
  // as-is.
  return Region;
}

MasterThreadRegion
MasterThreadRegionFinder::expandEnd(MasterThreadRegion Region) const {

  // If the region already ends at a target region end, it can't be expanded
  // further.
  if (VPOAnalysisUtils::getRegionDirectiveID(Region.End) ==
      DIR_OMP_END_TARGET) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Reached target region end"
                      << *Region.End << "\n");
    return Region;
  }

  // Attempt to expand the region end to the immediate post-dominator of its
  // current end instruction.
  MasterThreadRegion NewRegion = Region;
  Instruction *NewEnd = getImmediatePostDominator(PDT, Region.End);
  if (!NewEnd) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": No post-dominator found\n");
    return Region;
  }
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Expanding region end to" << *NewEnd
                    << "\n");
  if (!expandUntilStructurallyValid(NewRegion, Region.Start, NewEnd))
    return Region;
  if (const std::optional<MasterThreadRegion> ValidRegion =
          expandUntilValid(std::move(NewRegion), Region))
    return expandEnd(*ValidRegion);
  return Region;
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

#ifndef NDEBUG
  // This function shouldn't need to be called on any instruction already in a
  // master thread region.
  for (const MasterThreadRegion &Region : Regions)
    assert(!Region.contains(Inst));
#endif // NDEBUG

  // Start with a master thread region containing only Inst.
  assert(Inst->getNextNode());
  MasterThreadRegion Region(Inst, Critical);
  LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Starting new master thread region at"
                    << *Inst << "\n");

  // If the option is set to false, use the old guarding strategy where regions
  // are only expanded to include immediate successor side-effect (and debug)
  // instructions and only if none of the instructions have uses.
  if (!MasterThreadRegionExpansion) {
    if (Inst->use_empty()) {
      Instruction *NewLastInst;
      while ((NewLastInst =
                  Region.End->getPrevNode()->getNextNonDebugInstruction()) &&
             NewLastInst->use_empty() && needsMasterThreadGuard(NewLastInst) &&
             NewLastInst->getNextNode())
        Region.End = NewLastInst->getNextNode();
    }
  }

  // Otherwise, use the new region expansion strategy.
  else {

    // Attempt expansion to make sure the initial region is valid. If this
    // fails, the one-instruction region will have to be added as-is and
    // generated with a broadcast.
    if (std::optional<MasterThreadRegion> ValidRegion =
            expandUntilValid(Region)) {
      Region = std::move(*ValidRegion);
    } else {
      Regions.push_back(std::move(Region));
      return Regions.back();
    }

    // Keep expanding the region as far as possible in both directions.
    Region = expandStart(std::move(Region));
    Region = expandEnd(std::move(Region));
  }

  // Add the completed region to the list and return it.
  Regions.push_back(std::move(Region));
  return Regions.back();
}

#endif // INTEL_COLLAB
