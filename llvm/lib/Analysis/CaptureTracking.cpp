//===--- CaptureTracking.cpp - Determine whether a pointer is captured ----===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains routines that help determine which pointers are captured.
// A pointer value is captured if the function makes a copy of any part of the
// pointer that outlives the call.  Not being captured means, more or less, that
// the pointer is only dereferenced and not stored in a global.  Returning part
// of the pointer as the function return value may or may not count as capturing
// the pointer, depending on the context.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/CaptureTracking.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

#define DEBUG_TYPE "capture-tracking"

STATISTIC(NumCaptured,          "Number of pointers maybe captured");
STATISTIC(NumNotCaptured,       "Number of pointers not captured");
STATISTIC(NumCapturedBefore,    "Number of pointers maybe captured before");
STATISTIC(NumNotCapturedBefore, "Number of pointers not captured before");

/// The default value for MaxUsesToExplore argument. It's relatively small to
/// keep the cost of analysis reasonable for clients like BasicAliasAnalysis,
/// where the results can't be cached.
/// TODO: we should probably introduce a caching CaptureTracking analysis and
/// use it where possible. The caching version can use much higher limit or
/// don't have this cap at all.
#if INTEL_CUSTOMIZATION
// Setting limit back to 20, after reverting back to previous algorithm before
// D126236.
// TODO: Opaque pointers increases the number of uses. We may need to increase
// this default, and/or the override value (80) from the AA passes.
static cl::opt<unsigned>
    DefaultMaxUsesToExplore("capture-tracking-max-uses-to-explore", cl::Hidden,
                            cl::desc("Maximal number of uses to explore."),
                            cl::init(20));
#endif // INTEL_CUSTOMIZATION

unsigned llvm::getDefaultMaxUsesToExploreForCaptureTracking() {
  return DefaultMaxUsesToExplore;
}

CaptureTracker::~CaptureTracker() = default;

bool CaptureTracker::shouldExplore(const Use *U) { return true; }

bool CaptureTracker::isDereferenceableOrNull(Value *O, const DataLayout &DL) {
  // We want comparisons to null pointers to not be considered capturing,
  // but need to guard against cases like gep(p, -ptrtoint(p2)) == null,
  // which are equivalent to p == p2 and would capture the pointer.
  //
  // A dereferenceable pointer is a case where this is known to be safe,
  // because the pointer resulting from such a construction would not be
  // dereferenceable.
  //
  // It is not sufficient to check for inbounds GEP here, because GEP with
  // zero offset is always inbounds.

#if INTEL_CUSTOMIZATION
  // The same works for llvm.intel.subscript.
  // FIXME: GEPs that are not inbounds and are subjects to be
  // converted into llvm.intel.subscript should be conservative and
  // refrain from returning NotCaptured.
  if (isa<SubscriptInst>(O))
    return true;
#endif // INTEL_CUSTOMIZATION

  bool CanBeNull, CanBeFreed;
  return O->getPointerDereferenceableBytes(DL, CanBeNull, CanBeFreed);
}

namespace {
  struct SimpleCaptureTracker : public CaptureTracker {
    explicit SimpleCaptureTracker(
#if INTEL_CUSTOMIZATION
        bool ReturnCaptures, bool IgnoreFlag)
        : ReturnCaptures(ReturnCaptures),
          IgnoreNoAliasArgStCaptured(IgnoreFlag) {
    }
#endif // INTEL_CUSTOMIZATION

    void tooManyUses() override {
      LLVM_DEBUG(dbgs() << "Captured due to too many uses\n");
      Captured = true;
    }

    bool captured(const Use *U) override {
      if (isa<ReturnInst>(U->getUser()) && !ReturnCaptures)
        return false;
#if INTEL_CUSTOMIZATION
      Instruction *I = cast<Instruction>(U->getUser());
      if (I->getOpcode() == Instruction::Store) {
        if (IgnoreNoAliasArgStCaptured) {
          Value *V2 = I->getOperand(1);
          V2 = V2->stripPointerCasts();
          if (V2 && isNoAliasOrByValArgument(V2)) {
            return false;
          }
        }
      }
#endif // INTEL_CUSTOMIZATION

      LLVM_DEBUG(dbgs() << "Captured by: " << *U->getUser() << "\n");

      Captured = true;
      return true;
    }

    bool ReturnCaptures;

    bool Captured = false;
    bool IgnoreNoAliasArgStCaptured; // INTEL
  };

  /// Only find pointer captures which happen before the given instruction. Uses
  /// the dominator tree to determine whether one instruction is before another.
  /// Only support the case where the Value is defined in the same basic block
  /// as the given instruction and the use.
  struct CapturesBefore : public CaptureTracker {

    CapturesBefore(bool ReturnCaptures, const Instruction *I,
                   const DominatorTree *DT, bool IncludeI, const LoopInfo *LI)
        : BeforeHere(I), DT(DT), ReturnCaptures(ReturnCaptures),
          IncludeI(IncludeI), LI(LI) {}

    void tooManyUses() override { Captured = true; }

    bool isSafeToPrune(Instruction *I) {
      if (BeforeHere == I)
        return !IncludeI;

      // We explore this usage only if the usage can reach "BeforeHere".
      // If use is not reachable from entry, there is no need to explore.
      if (!DT->isReachableFromEntry(I->getParent()))
        return true;

      // Check whether there is a path from I to BeforeHere.
      return !isPotentiallyReachable(I, BeforeHere, nullptr, DT, LI);
    }

    bool captured(const Use *U) override {
      Instruction *I = cast<Instruction>(U->getUser());
      if (isa<ReturnInst>(I) && !ReturnCaptures)
        return false;

      // Check isSafeToPrune() here rather than in shouldExplore() to avoid
      // an expensive reachability query for every instruction we look at.
      // Instead we only do one for actual capturing candidates.
      if (isSafeToPrune(I))
        return false;

      Captured = true;
      return true;
    }

    const Instruction *BeforeHere;
    const DominatorTree *DT;

    bool ReturnCaptures;
    bool IncludeI;

    bool Captured = false;

    const LoopInfo *LI;
  };

  /// Find the 'earliest' instruction before which the pointer is known not to
  /// be captured. Here an instruction A is considered earlier than instruction
  /// B, if A dominates B. If 2 escapes do not dominate each other, the
  /// terminator of the common dominator is chosen. If not all uses cannot be
  /// analyzed, the earliest escape is set to the first instruction in the
  /// function entry block.
  // NOTE: Users have to make sure instructions compared against the earliest
  // escape are not in a cycle.
  struct EarliestCaptures : public CaptureTracker {

    EarliestCaptures(bool ReturnCaptures, Function &F, const DominatorTree &DT)
        : DT(DT), ReturnCaptures(ReturnCaptures), F(F) {}

    void tooManyUses() override {
      Captured = true;
      EarliestCapture = &*F.getEntryBlock().begin();
    }

    bool captured(const Use *U) override {
      Instruction *I = cast<Instruction>(U->getUser());
      if (isa<ReturnInst>(I) && !ReturnCaptures)
        return false;

      if (!EarliestCapture)
        EarliestCapture = I;
      else
        EarliestCapture = DT.findNearestCommonDominator(EarliestCapture, I);
      Captured = true;

      // Return false to continue analysis; we need to see all potential
      // captures.
      return false;
    }

    Instruction *EarliestCapture = nullptr;

    const DominatorTree &DT;

    bool ReturnCaptures;

    bool Captured = false;

    Function &F;
  };
}

/// PointerMayBeCaptured - Return true if this pointer value may be captured
/// by the enclosing function (which is required to exist).  This routine can
/// be expensive, so consider caching the results.  The boolean ReturnCaptures
/// specifies whether returning the value (or part of it) from the function
/// counts as capturing it or not.  The boolean StoreCaptures specified whether
/// storing the value (or part of it) into memory anywhere automatically
/// counts as capturing it or not.
bool llvm::PointerMayBeCaptured(const Value *V, bool ReturnCaptures,
                                bool StoreCaptures,
                                bool IgnoreStoreCapturesByNoAliasArgument, // INTEL
                                unsigned MaxUsesToExplore) {
  assert(!isa<GlobalValue>(V) &&
         "It doesn't make sense to ask whether a global is captured.");

  // TODO: If StoreCaptures is not true, we could do Fancy analysis
  // to determine whether this store is not actually an escape point.
  // In that case, BasicAliasAnalysis should be updated as well to
  // take advantage of this.
  (void)StoreCaptures;

  LLVM_DEBUG(dbgs() << "Captured?: " << *V << " = ");

#if INTEL_CUSTOMIZATION
  SimpleCaptureTracker SCT(ReturnCaptures,
                           IgnoreStoreCapturesByNoAliasArgument);
#endif // INTEL_CUSTOMIZATION
  PointerMayBeCaptured(V, &SCT, MaxUsesToExplore);
  if (SCT.Captured)
    ++NumCaptured;
  else {
    ++NumNotCaptured;
    LLVM_DEBUG(dbgs() << "not captured\n");
  }
  return SCT.Captured;
}

/// PointerMayBeCapturedBefore - Return true if this pointer value may be
/// captured by the enclosing function (which is required to exist). If a
/// DominatorTree is provided, only captures which happen before the given
/// instruction are considered. This routine can be expensive, so consider
/// caching the results.  The boolean ReturnCaptures specifies whether
/// returning the value (or part of it) from the function counts as capturing
/// it or not.  The boolean StoreCaptures specified whether storing the value
/// (or part of it) into memory anywhere automatically counts as capturing it
/// or not.
bool llvm::PointerMayBeCapturedBefore(const Value *V, bool ReturnCaptures,
                                      bool StoreCaptures, const Instruction *I,
                                      const DominatorTree *DT, bool IncludeI,
                                      unsigned MaxUsesToExplore,
                                      const LoopInfo *LI) {
  assert(!isa<GlobalValue>(V) &&
         "It doesn't make sense to ask whether a global is captured.");

  if (!DT)
    return PointerMayBeCaptured(V, ReturnCaptures, StoreCaptures,
                                MaxUsesToExplore);

  // TODO: See comment in PointerMayBeCaptured regarding what could be done
  // with StoreCaptures.

  CapturesBefore CB(ReturnCaptures, I, DT, IncludeI, LI);
  PointerMayBeCaptured(V, &CB, MaxUsesToExplore);
  if (CB.Captured)
    ++NumCapturedBefore;
  else
    ++NumNotCapturedBefore;
  return CB.Captured;
}

Instruction *llvm::FindEarliestCapture(const Value *V, Function &F,
                                       bool ReturnCaptures, bool StoreCaptures,
                                       const DominatorTree &DT,
                                       unsigned MaxUsesToExplore) {
  assert(!isa<GlobalValue>(V) &&
         "It doesn't make sense to ask whether a global is captured.");

  EarliestCaptures CB(ReturnCaptures, F, DT);
  PointerMayBeCaptured(V, &CB, MaxUsesToExplore);
  if (CB.Captured)
    ++NumCapturedBefore;
  else
    ++NumNotCapturedBefore;
  return CB.EarliestCapture;
}

UseCaptureKind llvm::DetermineUseCaptureKind(
    const Use &U,
    function_ref<bool(Value *, const DataLayout &)> IsDereferenceableOrNull) {
  Instruction *I = dyn_cast<Instruction>(U.getUser());

  // TODO: Investigate non-instruction uses.
  if (!I)
    return UseCaptureKind::MAY_CAPTURE;

  switch (I->getOpcode()) {
  case Instruction::Call:
  case Instruction::Invoke: {
    auto *Call = cast<CallBase>(I);
    // Not captured if the callee is readonly, doesn't return a copy through
    // its return value and doesn't unwind (a readonly function can leak bits
    // by throwing an exception or not depending on the input value).
    if (Call->onlyReadsMemory() && Call->doesNotThrow() &&
        Call->getType()->isVoidTy())
      return UseCaptureKind::NO_CAPTURE;

    // The pointer is not captured if returned pointer is not captured.
    // NOTE: CaptureTracking users should not assume that only functions
    // marked with nocapture do not capture. This means that places like
    // getUnderlyingObject in ValueTracking or DecomposeGEPExpression
    // in BasicAA also need to know about this property.
    if (isIntrinsicReturningPointerAliasingArgumentWithoutCapturing(Call, true))
      return UseCaptureKind::USER_MAY_CAPTURE; // INTEL_COLLAB

#if INTEL_CUSTOMIZATION
    if (isa<AddressInst>(I)) {
      return UseCaptureKind::USER_MAY_CAPTURE;
    }
#endif // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
    if (auto *II = dyn_cast<IntrinsicInst>(I)) {
      // Calls with op bundles capture all the operands by default. Capture
      // is a strong effect, which prevents optimization across all
      // unknown stores and calls that occur after the directive. We declare
      // "nocapture" here, and then block illegal motion in BasicAA.
      if (II->getIntrinsicID() == Intrinsic::directive_region_entry)
        return UseCaptureKind::NO_CAPTURE;
    }
#endif // INTEL_COLLAB
    // Volatile operations effectively capture the memory location that they
    // load and store to.
    if (auto *MI = dyn_cast<MemIntrinsic>(Call))
      if (MI->isVolatile())
        return UseCaptureKind::MAY_CAPTURE;

    // Calling a function pointer does not in itself cause the pointer to
    // be captured.  This is a subtle point considering that (for example)
    // the callee might return its own address.  It is analogous to saying
    // that loading a value from a pointer does not cause the pointer to be
    // captured, even though the loaded value might be the pointer itself
    // (think of self-referential objects).
    if (Call->isCallee(&U))
      return UseCaptureKind::NO_CAPTURE;

    // Not captured if only passed via 'nocapture' arguments.
    if (Call->isDataOperand(&U) &&
        !Call->doesNotCapture(Call->getDataOperandNo(&U))) {
      // The parameter is not marked 'nocapture' - captured.
      return UseCaptureKind::MAY_CAPTURE;
    }
    return UseCaptureKind::NO_CAPTURE;
  }
  case Instruction::Load:
    // Volatile loads make the address observable.
    if (cast<LoadInst>(I)->isVolatile())
      return UseCaptureKind::MAY_CAPTURE;
    return UseCaptureKind::NO_CAPTURE;
  case Instruction::VAArg:
    // "va-arg" from a pointer does not cause it to be captured.
    return UseCaptureKind::NO_CAPTURE;
  case Instruction::Store:
    // Stored the pointer - conservatively assume it may be captured.
    // Volatile stores make the address observable.
    if (U.getOperandNo() == 0 || cast<StoreInst>(I)->isVolatile())
      return UseCaptureKind::MAY_CAPTURE;
    return UseCaptureKind::NO_CAPTURE;
  case Instruction::AtomicRMW: {
    // atomicrmw conceptually includes both a load and store from
    // the same location.
    // As with a store, the location being accessed is not captured,
    // but the value being stored is.
    // Volatile stores make the address observable.
    auto *ARMWI = cast<AtomicRMWInst>(I);
    if (U.getOperandNo() == 1 || ARMWI->isVolatile())
      return UseCaptureKind::MAY_CAPTURE;
    return UseCaptureKind::NO_CAPTURE;
  }
  case Instruction::AtomicCmpXchg: {
    // cmpxchg conceptually includes both a load and store from
    // the same location.
    // As with a store, the location being accessed is not captured,
    // but the value being stored is.
    // Volatile stores make the address observable.
    auto *ACXI = cast<AtomicCmpXchgInst>(I);
    if (U.getOperandNo() == 1 || U.getOperandNo() == 2 || ACXI->isVolatile())
      return UseCaptureKind::MAY_CAPTURE;
    return UseCaptureKind::NO_CAPTURE;
  }
  case Instruction::BitCast:
  case Instruction::GetElementPtr:
  case Instruction::PHI:
  case Instruction::Select:
  case Instruction::AddrSpaceCast:
    // The original value is not captured via this if the new value isn't.
    return UseCaptureKind::USER_MAY_CAPTURE; // INTEL_COLLAB
  case Instruction::ICmp: {
    unsigned Idx = U.getOperandNo();
    unsigned OtherIdx = 1 - Idx;
    if (auto *CPN = dyn_cast<ConstantPointerNull>(I->getOperand(OtherIdx))) {
      // Don't count comparisons of a no-alias return value against null as
      // captures. This allows us to ignore comparisons of malloc results
      // with null, for example.
      if (CPN->getType()->getAddressSpace() == 0)
        if (isNoAliasCall(U.get()->stripPointerCasts()))
          return UseCaptureKind::NO_CAPTURE;
      if (!I->getFunction()->nullPointerIsDefined()) {
        auto *O = I->getOperand(Idx)->stripPointerCastsSameRepresentation();
        // Comparing a dereferenceable_or_null pointer against null cannot
        // lead to pointer escapes, because if it is not null it must be a
        // valid (in-bounds) pointer.
        const DataLayout &DL = I->getModule()->getDataLayout();
        if (IsDereferenceableOrNull && IsDereferenceableOrNull(O, DL))
          return UseCaptureKind::NO_CAPTURE;
      }
    }

    // Otherwise, be conservative. There are crazy ways to capture pointers
    // using comparisons.
    return UseCaptureKind::MAY_CAPTURE;
  }
  default:
    // Something else - be conservative and say it is captured.
    return UseCaptureKind::MAY_CAPTURE;
  }
}

void llvm::PointerMayBeCaptured(const Value *V, CaptureTracker *Tracker,
                                unsigned MaxUsesToExplore) {
#if INTEL_CUSTOMIZATION
  // xmain allows noalias on vector-of-pointers, need to adjust this assert
  // accordingly.
  assert(V->getType()->getScalarType()->isPointerTy()
         && "Capture is for pointers only!");
#endif // INTEL_CUSTOMIZATION
  if (MaxUsesToExplore == 0)
    MaxUsesToExplore = DefaultMaxUsesToExplore;

  SmallVector<const Use *, 20> Worklist;
  Worklist.reserve(getDefaultMaxUsesToExploreForCaptureTracking());
  SmallSet<const Use *, 20> Visited;

  auto AddUses = [&](const Value *V) {
#if INTEL_CUSTOMIZATION
    // llorg is counting the total visited values from multiple calls to
    // AddUses (D126236). FORTRAN programs like 554.roms have very large
    // use-def chains and may need a limit >1500 for this new algorithm.
    // We revert back to the old algorithm (count the uses of the given value).
    // Also, in xmain, the client of CaptureTracking may pass in a MaxUses
    // limit, which must be recalibrated if the limit algorithm is changed.
    unsigned Count = 0;
#endif // INTEL_CUSTOMIZATION
    for (const Use &U : V->uses()) {
      // If there are lots of uses, conservatively say that the value
      // is captured to avoid taking too much compile time.
#if INTEL_CUSTOMIZATION
      if (Count++ >= MaxUsesToExplore) {
#endif // INTEL_CUSTOMIZATION
        Tracker->tooManyUses();
        return false;
      }
      if (!Visited.insert(&U).second)
        continue;
      if (!Tracker->shouldExplore(&U))
        continue;
      Worklist.push_back(&U);
    }
    return true;
  };
  if (!AddUses(V))
    return;

  auto IsDereferenceableOrNull = [Tracker](Value *V, const DataLayout &DL) {
    return Tracker->isDereferenceableOrNull(V, DL);
  };
  while (!Worklist.empty()) {
    const Use *U = Worklist.pop_back_val();
    switch (DetermineUseCaptureKind(*U, IsDereferenceableOrNull)) {
    case UseCaptureKind::NO_CAPTURE:
      continue;
    case UseCaptureKind::MAY_CAPTURE:
      if (Tracker->captured(U))
        return;
      continue;
    case UseCaptureKind::USER_MAY_CAPTURE: // INTEL_COLLAB
      if (!AddUses(U->getUser()))
        return;
      continue;
    }
  }

  // All uses examined.
}

#if INTEL_CUSTOMIZATION
// Intel: added PtrCaptureMaxUses and DL parameters.
bool llvm::isNonEscapingLocalObject(
    const Value *V, unsigned PtrCaptureMaxUses, const DataLayout &DL,
    SmallDenseMap<const Value *, bool, 8> *IsCapturedCache) {
#endif // INTEL_CUSTOMIZATION
  SmallDenseMap<const Value *, bool, 8>::iterator CacheIt;
  if (IsCapturedCache) {
    bool Inserted;
    std::tie(CacheIt, Inserted) = IsCapturedCache->insert({V, false});
    if (!Inserted)
      // Found cached result, return it!
      return CacheIt->second;
  }

  // If this is an identified function-local object, check to see if it escapes.
  if (isIdentifiedFunctionLocal(V)) {
    // Set StoreCaptures to True so that we can assume in our callers that the
    // pointer is not the result of a load instruction. Currently
    // PointerMayBeCaptured doesn't have any special analysis for the
    // StoreCaptures=false case; if it did, our callers could be refined to be
    // more precise.
#if INTEL_CUSTOMIZATION
    auto Ret = !PointerMayBeCaptured(V, false, /*StoreCaptures=*/true,
                                     /*IgnoreNoAliasArgStCaptures=*/false,
                                     PtrCaptureMaxUses);
#endif // INTEL_CUSTOMIZATION
    if (IsCapturedCache)
      CacheIt->second = Ret;
    return Ret;
  }

  return false;
}
