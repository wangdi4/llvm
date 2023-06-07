#if INTEL_COLLAB // -*- C++ -*-
//===--- VPOParoptMasterThreadRegion.h - Master Thread Regions -*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines interfaces for MasterThreadRegion and
/// MasterThreadRegionFinder, which are used for SPIR-V offloading to identify
/// and transform code regions outside of parallel regions that should run only
/// on the master thread.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_MASTER_THREAD_REGION
#define LLVM_TRANSFORMS_VPO_PAROPT_MASTER_THREAD_REGION

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"

#include <optional>

namespace llvm {

class AllocaInst;
class BasicBlock;
class CallInst;
class DominatorTree;
class DomTreeUpdater;
class Instruction;
class LoopInfo;
class PostDominatorTree;
class Value;

namespace vpo {

class WRegionNode;

/// Returns true for instructions that must be guarded under master thread
/// checks by VPOParoptTransform::guardSideEffectStatements().
///
/// This is generally true for all instructions with side-effects, but there are
/// some special cases where this will still return false:
///
/// * \p I is a call instruction that is a special call.
/// * \p I is an intrinsic call, unless it's a memcpy where the destination
///   operand is not allocated locally in the thread.
/// * \p I is a store with a store address that is an alloca instruction
///   allocating locally in the thread.
/// * \p I has already been marked as being executed under its own master thread
///   check.
bool needsMasterThreadGuard(Instruction *I);

/// Inserts a work group barrier before \p InsertPt.
///
/// If \p GlobalFence is set, the barrier will have both global and local memory
/// fences. If it is not set, the barrier will only have a local memory fence.
CallInst *insertWorkGroupBarrier(Instruction *InsertPt, bool GlobalFence);

/// A single-entry/single-exit code region which should be executed on the
/// master thread.
///
/// A region contains all instructions reachable from Start where the path to
/// that instruction from Start does not include End. To be a valid region for
/// inserting master thread guards, these conditions must be met:
///
/// * The region must be a single-entry/single-exit region, so there should also
///   not be any instructions not in the region where End is reachable through a
///   path that doesn't include Start. This means that successors of all basic
///   blocks in the region except for End's basic block must also be in the
///   region and not be Start's basic block, and that the predecessors of all
///   basic blocks in the region except for Start's basic block must also be in
///   the region and not be End's basic block.
/// * The region must be entirely within the target region.
/// * The region must not contain any basic blocks inside a parallel region.
/// * Uses of any instruction in the region must also be in the region.
/// * If any instruction is a store to an alloca'd value, any loads of that
///   value must be in the region.
/// * No other instructions with thread-local side-effects (which don't have
///   non-thread-local side-effects) are allowed. Instructions like barriers
///   that require convergent control flow are also not allowed.
class MasterThreadRegion {

  /// The start of the region.
  ///
  /// All instructions in the region should be dominated by this one. This
  /// instruction is contained in the region.
  Instruction *Start;

  /// The end of the region.
  ///
  /// All instructions in the region should be strictly post-dominated by this
  /// one. This instruction is not contained in the region; its immediate
  /// predecessor (if present) is the last instruction in the region.
  Instruction *End;

  /// Whether this region is in a critical section.
  ///
  /// If so, it should not have barriers inserted around it.
  bool Critical;

  /// All basic blocks fully enclosed by this region.
  SmallPtrSet<BasicBlock *, 4> EnclosedBBs;

  /// Thread-local allocas that are referenced by stores in this region.
  SmallPtrSet<const AllocaInst *, 2> StoredToAllocas;

  friend class MasterThreadRegionFinder;

  /// Creates an initial one-instruction master thread region.
  MasterThreadRegion(Instruction *Inst, bool Critical);

public:
  /// The start instruction of this region.
  Instruction *getStart() const { return Start; }

  /// The end instruction of this region.
  Instruction *getEnd() const { return End; }

  /// Returns true if \p I is contained in the region.
  bool contains(const Instruction *I) const;

  /// Attempts to add \p Alloca as a new stored-to alloca for this region.
  ///
  /// Returns false if \p Alloca already has stores in this region; in this case
  /// it doesn't need to have all of its uses checked again.
  bool newStoredToAlloca(const AllocaInst *Alloca) {
    return StoredToAllocas.insert(Alloca).second;
  }

  /// Inserts broadcasts of any thread-local values used outside of this region.
  ///
  /// This ensures that all values used outside the region are visible on all
  /// threads, and should be called before insertBarriers.
  /// \p TargetDirectiveBegin is the begin directive for the target region this
  /// master thread region is in, which is used to find the right insertion
  /// point for new workgroup-local variables needed to broadcast values.
  void insertBroadcasts(Instruction *TargetDirectiveBegin);

  /// Inserts barriers around this region.
  ///
  /// This ensures that memory dependencies are preserved between the master
  /// thread and all other threads at the beginning and end of the master thread
  /// region, and should be called after insertBroadcasts.
  void insertBarriers();

  /// Inserts a master thread guard for this region.
  ///
  /// This inserts the conditional to ensure code in the region is only run on
  /// the master thread, and should be called after insertBroadcasts and
  /// insertBarriers. \p MasterCheckPredicate is the condition value to use for
  /// the branch. \p DTU, \p LI, and \p W are updated to incorporate the new CFG
  /// changes.
  void insertGuard(Value *MasterCheckPredicate, DomTreeUpdater &DTU,
                   LoopInfo &LI, WRegionNode *W);
};

/// Compares two master thread regions for equality, which is true if they have
/// the same start and end instructions.
bool operator==(const MasterThreadRegion &, const MasterThreadRegion &);

/// Utility class to find optimal code regions to execute on the master thread.
class MasterThreadRegionFinder {

  /// The set of basic blocks which are in parallel regions.
  ///
  /// These must not run on the master thread.
  const SmallPtrSetImpl<BasicBlock *> &ParBBSet;

  const DominatorTree &DT;
  const PostDominatorTree &PDT;

  /// The current set of master thread regions to insert.
  SmallVector<MasterThreadRegion> Regions;

#ifndef NDEBUG
  /// Checks structural invariants of \p Region to make sure it's structurally
  /// valid.
  void assertStructuralInvariants(MasterThreadRegion &Region) const;
#endif // NDEBUG

  /// Expands \p Region until it is a structurally valid
  /// single-entry/single-exit region where the start dominates \p NewStart and
  /// the end post-dominates \p NewEnd.
  ///
  /// This does not verify that all instructions/uses/alloca loads in the new
  /// region are valid. Returns false if the expansion fails because it is not
  /// possible without including parallel region basic blocks. \p Region should
  /// be a structurally valid subregion of the new region.
  bool expandUntilStructurallyValid(MasterThreadRegion &Region,
                                    Instruction *NewStart,
                                    Instruction *NewEnd) const;

  /// Expands \p Region until it is a structurally valid region containing
  /// \p Inst.
  ///
  /// Like expandUntilStructurallyValid, this only checks structural
  /// requirements and not included instructions. Returns false if expansion
  /// fails.
  bool expandStructurallyToContain(MasterThreadRegion &Region,
                                   Instruction *Inst) const;

  /// Expands \p Region to return a fully valid MasterThreadRegion.
  ///
  /// Unlike expandUntilStructurallyValid, this will also check instructions in
  /// the region to make sure they can be included, their uses are in the
  /// region, and loads of allocas they store to are in the region. If
  /// specified, \p PrevRegion is assumed to be entirely contained by \p Region
  /// and instructions in \p PrevRegion are assumed to have already been
  /// checked. Returns nullopt if expansion fails.
  std::optional<MasterThreadRegion>
  expandUntilValid(MasterThreadRegion Region,
                   std::optional<MasterThreadRegion> PrevRegion = {}) const;

  /// Expands the start of \p Region as much as possible, and returns the
  /// expanded region.
  MasterThreadRegion expandStart(MasterThreadRegion Region) const;

  /// Expands the end of \p Region as much as possible, and returns the expanded
  /// region.
  MasterThreadRegion expandEnd(MasterThreadRegion Region) const;

public:
  MasterThreadRegionFinder(const SmallPtrSetImpl<BasicBlock *> &ParBBSet,
                           const DominatorTree &DT, PostDominatorTree &PDT)
      : ParBBSet(ParBBSet), DT(DT), PDT(PDT) {}

  /// Returns the region end if a multi-basic block region ends in \p BB, or the
  /// first non-phi/debug/lifetime instruction if none end in \p BB, or the
  /// terminator if \p BB is completely enclosed by any existing region.
  Instruction *multiBBRegionEnd(BasicBlock *BB) const;

  /// Returns the region start if a multi-basic block region starts in \p BB, or
  /// the terminator if none start in \p BB.
  Instruction *multiBBRegionStart(BasicBlock *BB) const;

  /// Finds or creates a master thread region containing \p Inst.
  ///
  /// \p Inst is assumed to not be a terminator and to not already be contained
  /// by any of the master thread regions found so far. If \p Critical is set,
  /// no barriers should be added for this instruction.
  const MasterThreadRegion &findMasterThreadRegion(Instruction *Inst,
                                                   bool Critical);

  /// Returns the final list of master thread regions.
  SmallVector<MasterThreadRegion> &&foundRegions() {
    return std::move(Regions);
  }
};

} // namespace vpo

} // namespace llvm

#endif // LLVM_TRANSFORMS_VPO_PAROPT_MASTER_THREAD_REGION
#endif // INTEL_COLLAB
