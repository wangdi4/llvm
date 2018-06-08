//===---------------- DTransAnalysis.cpp - DTrans Analysis ----------------===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This file does DTrans analysis.
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <map>
#include <set>

using namespace llvm;
using namespace llvm::PatternMatch;

#define DEBUG_TYPE "dtransanalysis"

// Debug type for verbose local pointer analysis output.
#define LPA_VERBOSE "dtrans-lpa-verbose"

// Debug type for verbose partial pointer load/store analysis output.
#define DTRANS_PARTIALPTR "dtrans-partialptr"

static cl::opt<bool> DTransPrintAllocations("dtrans-print-allocations",
                                            cl::ReallyHidden);

static cl::opt<bool> DTransPrintAnalyzedTypes("dtrans-print-types",
                                              cl::ReallyHidden);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// Prints information that is saved during analysis about specific function
/// calls (malloc, free, memset, etc) that may be useful to the transformations.
static cl::opt<bool> DTransPrintAnalyzedCalls("dtrans-print-callinfo",
                                              cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

//
// An option that indicates that a pointer to a struct could access
// somewhere beyond the boundaries of that struct:
//
// For example:
//
// %struct.A = type { i32, i32 }
// %struct.B = type { i16, i16, i16, i16 }
// %struct.C = type { %struct.A, %struct.B }
//
// define void @foo(%struct.A* nocapture) local_unnamed_addr #0 {
//   %2 = getelementptr inbounds %struct.A, %struct.A* %0, i64 1, i32 1
//   store i32 -1, i32* %2, align 4, !tbaa !2
//   ret void
// }
//
// define void @bar(%struct.C* nocapture) local_unnamed_addr #0 {
//   %2 = getelementptr inbounds %struct.C, %struct.C* %0, i64 0, i32 0
//   tail call void @foo(%struct.A* %2)
//   ret void
// }
//
// Here the getelementptr in @foo is accessing beyond the end of the inner
// %struct.A within %struct.C.
//
// With respect to dtransanalysis, having -dtrans-outofboundsok=true will
// cause safety checks to be propagated from outer structs to inner structs.
// So, in the above example, if -dtrans-outofboundsok=false, 'Field address
// taken' will be true only for %structC. But if -dtrans-outofboundsok=true,
// it will also be true for %struct.A and %struct.B.
//
static cl::opt<bool> DTransOutOfBoundsOK("dtrans-outofboundsok", cl::init(true),
                                         cl::ReallyHidden);

// Use the C language compatibility rule to determine if two aggregate types
// are compatible. This is used by the analysis of AddressTaken safety checks.
// If the actual argument of a call is a pointer to an aggregate with type T,
// and there is no type U distinct from T which is compatible with T, then
// we know that the types of the formal and actual arguments must be identical.
// So, in this case, we will not need to report an AddressTaken safety check
// for a potential mismatch between formal and actual arguments.
//
static cl::opt<bool> DTransUseCRuleCompat("dtrans-usecrulecompat",
                                          cl::init(false), cl::ReallyHidden);
namespace {

// FIXME: Find a better home for this very generic utility function.
inline bool isValueInt8PtrType(Value *V) {
  return (V->getType() == llvm::Type::getInt8PtrTy(V->getContext()));
}

// There is a very specific pattern that we need to be able to identify
// where we have pointer-to-pointer values and the pointers being pointed
// to are swapped by copying partial chunks of the pointer (either i8 or i32)
// We don't want to track the loads, stores or any associated bitcasts as
// potential safety violations in this case.
//
// The pattern we want to match looks like this for i32 swaps:
//
//   Block1:
//     %Cast1 = bitcast i8* %PtrToPtr to i32*
//     %Cast2 = bitcast i8* %OtherPtrToPtr to i32*
//     br label %Block2
//
//   Block2:
//     %Count = phi i64 [ 2, %Block1 ], [ %NextCount, %Block2 ]
//     %HalfPtr1 = phi i32* [ %Cast1, %Block1 ], [ %NextHalf1, %Block2 ]
//     %HalfPtr2 = phi i32* [ %Cast2, %Block1 ], [ %NextHalf2, %Block2 ]
//     %HalfVal1 = load i32, i32* %HalfPtr1
//     %HalfVal2 = load i32, i32* %HalfPtr2
//     %NextHalf1 = getelementptr inbounds i32, i32* %HalfPtr1, i64 1
//     store i32 %HalfVal2, i32* %HalfPtr1
//     %NextHalf2 = getelementptr inbounds i32, i32* %HalfPtr2, i64 1
//     store i32 %HalfVal1, i32* %HalfPtr2
//     %NextCount = add nsw i64 %Count, -1
//     %Cmp = icmp sgt i64 %Count, 1
//     br i1 %Cmp, label %Block2, label %ExitBlock
//
// For i8 swaps, it looks similar without the bitcasts in Block1 and with
// a count constant of 8 rather than two.
//
// (Note: Sometimes we can't identify the incoming count value for the
//  32-bit case is not constant.)
//
// Even though the treatment of the two values is symmetric, we need to
// check them both together because we need to be sure that the partial-values
// are being written to adjacent memory locations.
//
// Here we attempt to match the pattern starting with one of the load
// instructions. For the pattern to match, the following conditions must be met.
//
//   1. That pointer operand of the load must be a PHI node with two incoming
//      values.
//   2. One of the incoming values must be from the block containing the
//      PHI, which loops back on itself.
//   3. That incoming value must be a GEP which increments the PHI pointer.
//   4. The PHI node must have three users, a load, a store, and a GEP.
//   5. The store must be storing a value loaded from the "partner PHI"
//   5. The load must have a single user, a store in the same block.
//   6. The load must be stored to the "partner PHI"
//   7. The GEP must be the other incoming value to the PHI.
//   8. The block containing this code must loop back on itself based on
//      an count value which is decremented each time the block executes.
//
bool isPartialPtrLoad(LoadInst *Load) {
  // Since everything here needs to be checked twice, we'll implement the
  // checks as lambdas.
  auto verifyPHI = [](Value *V, BasicBlock *LoopBB) {
    auto *PN = dyn_cast<PHINode>(V);
    if (!PN)
      return false;

    // The PHI must have two incoming values. We know one is the
    // value we're here to check. We'll check the other below.
    if (PN->getNumIncomingValues() != 2)
      return false;

    // The block containing the PHI must loop back on itself and the incoming
    // value from that block must be a GEP that increments the PHI pointer.
    Value *SelfInVal = nullptr;
    if (PN->getIncomingBlock(0) == LoopBB)
      SelfInVal = PN->getIncomingValue(0);
    else if (PN->getIncomingBlock(1) == LoopBB)
      SelfInVal = PN->getIncomingValue(1);
    if (!SelfInVal)
      return false;
    auto *GEP = dyn_cast<GetElementPtrInst>(SelfInVal);
    if (!GEP || GEP->getNumIndices() != 1 || !GEP->hasAllConstantIndices())
      return false;
    auto *Idx = dyn_cast<ConstantInt>(*GEP->idx_begin());
    if (!Idx || !Idx->isOne())
      return false;

    return true;
  };

  auto verifyBlockIsLoop = [](PHINode *PN) {
    // Verify that the PHI node is used in a block that loops back on itself
    // based on a counter value.
    auto *LoopBB = PN->getParent();
    auto *Branch = dyn_cast<BranchInst>(LoopBB->getTerminator());
    if (!Branch || !Branch->isConditional()) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. No conditional branch!\n");
      return false;
    }
    // This could be much more general, but it meets our current needs.
    auto *Condition = dyn_cast<ICmpInst>(Branch->getCondition());
    if (!Condition) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. Branch condition is not icmp!\n");
      return false;
    }
    ICmpInst::Predicate Pred;
    Instruction *Base;
    // The condition should be a comparison based on a PHI node.
    if (!match(Condition,
               m_ICmp(Pred, m_Instruction(Base), m_SpecificInt(1)))) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. icmp not using constant int!\n");
      return false;
    }
    if (Pred != CmpInst::Predicate::ICMP_SGT) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. icmp predicate isn't sgt!\n");
      return false;
    }
    auto *BasePHI = dyn_cast<PHINode>(Base);
    if (!BasePHI || BasePHI->getParent() != LoopBB) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. Branch condition isn't PHI!\n");
      return false;
    }
    // The incoming value from the root block must be constant.
    Value *OtherInVal;
    if (BasePHI->getIncomingBlock(0) == LoopBB)
      OtherInVal = BasePHI->getIncomingValue(0);
    else
      OtherInVal = BasePHI->getIncomingValue(1);
    // The other incoming (from the loop block) must be a decrement of
    // the BasePHI.
    if (!(match(OtherInVal, m_Add(m_Specific(BasePHI), m_SpecificInt(-1))) ||
          match(OtherInVal, m_Add(m_SpecificInt(-1), m_Specific(BasePHI))))) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. PHI decrement not matched!\n");
      return false;
    }
    return true;
  };

  auto matchPHIUsers = [](PHINode *PN, LoadInst *&LoadUser,
                          StoreInst *&StoreUser, GetElementPtrInst *&GEPUser) {
    // The PHI must have three users.
    if (!PN->hasNUses(3)) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. PHI doesn't have three users!\n");
      return false;
    }

    LoadUser = nullptr;
    StoreUser = nullptr;
    GEPUser = nullptr;
    for (auto *U : PN->users()) {
      if (!LoadUser)
        LoadUser = dyn_cast<LoadInst>(U);
      if (!StoreUser)
        StoreUser = dyn_cast<StoreInst>(U);
      if (!GEPUser)
        GEPUser = dyn_cast<GetElementPtrInst>(U);
    }
    if (!LoadUser || !StoreUser || !GEPUser) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. PHI users don't match!\n");
      return false;
    }

    // The GEP must have a single use which is the PHI.
    if (!GEPUser->hasOneUse() || (*GEPUser->user_begin() != PN)) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. "
                             << "GEP isn't uniquely used by PHI!\n");
      return false;
    }

    // The load user must have a single use. We'll check that elsewhere.
    if (!LoadUser->hasOneUse()) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. "
                             << "Secondary load isn't single use!\n");
      return false;
    }

    // The phi must be the target of the store, not the value stored.
    if (StoreUser->getPointerOperand() != PN) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. "
                             << "Store doesn't write to PHI pointer!\n");
      return false;
    }

    return true;
  };

  auto verifyLoadUsage = [](LoadInst *Load, Value *ExpectedDest) {
    // We've already verified that the load user is only used once.
    // That use must be a store instruction
    auto *Store = dyn_cast<StoreInst>(*Load->user_begin());
    if (!Store) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. "
                             << "Loaded value isn't used by store!\n");
      return false;
    }

    // The loaded value must be the stored value, not the destination
    // of the store.
    if (Store->getValueOperand() != Load) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. Loaded value isn't stored!\n");
      return false;
    }

    // Check that the destination of the store is what we expect.
    if (Store->getPointerOperand() != ExpectedDest) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. Unexpected store destination!\n");
      return false;
    }

    return true;
  };

  //////////////////////////////////////////////
  // The actual implementation begins here.
  //////////////////////////////////////////////

  // If we're not loading from a PHI node pointer, the whole this is a
  // non-starter.
  auto *PrimaryPHI = dyn_cast<PHINode>(Load->getPointerOperand());
  if (!PrimaryPHI)
    return false;

  auto *LoopBB = PrimaryPHI->getParent();
  if (!verifyPHI(PrimaryPHI, LoopBB))
    return false;

  // To reduce noise, don't report that we're even checking for the
  // partial pointer pattern until we see that the load is from a suitable PHI.
  DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                  dbgs() << "dtrans: Check for partial pointer load/store "
                         << "idiom starting at: " << *Load << "\n");

  // Make sure the block loops back on itself.
  if (!verifyBlockIsLoop(PrimaryPHI))
    return false;

  // Try to match the PHI users as a load, a store, and a GEP.
  LoadInst *LoadUser;
  StoreInst *StoreUser;
  GetElementPtrInst *GEPUser;
  if (!matchPHIUsers(PrimaryPHI, LoadUser, StoreUser, GEPUser))
    return false;

  // The value stored should trace back to our partner value as such:
  //   PartnerVal = bitcast
  //   PartnerPHI = phi [PartnerVal....
  //   StoredVal = load i32, i32* PartnerPHI
  auto *ValStored = StoreUser->getValueOperand();
  auto *PartnerLoad = dyn_cast<LoadInst>(ValStored);
  if (!PartnerLoad) {
    DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                    dbgs() << "Not matched. Can't find partner load!\n");
    return false;
  }
  auto *PartnerPHI = dyn_cast<PHINode>(PartnerLoad->getPointerOperand());
  if (!PartnerPHI || PartnerPHI->getParent() != PrimaryPHI->getParent()) {
    DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                    dbgs() << "Not matched. Can't find partner PHI!\n");
    return false;
  }

  if (!verifyPHI(PartnerPHI, LoopBB)) {
    DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                    dbgs() << "Not matched. Partner PHI does match idiom!\n");
    return false;
  }

  // Check that the value loaded from the PHI pointer is stored in the
  // same place that the partner load was loaded from.
  if (!verifyLoadUsage(LoadUser, PartnerPHI))
    return false;

  StoreInst *PartnerStore;
  GetElementPtrInst *PartnerGEP;
  if (!matchPHIUsers(PartnerPHI, PartnerLoad, PartnerStore, PartnerGEP))
    return false;

  DEBUG_WITH_TYPE(DTRANS_PARTIALPTR, dbgs() << "Idiom matched.\n");
  return true;
}

// This checks the same pattern as above, but starts from one of the store
// instructions, finds the associated load and then calls the function above.
bool isPartialPtrStore(StoreInst *Store) {
  // We're peeking ahead a bit here in checking for three users.
  // We'll check that again in isPartPointerLoad() but checking it here
  // avoids potentially wasteful loops over the PHI's users if it cann't
  // match.
  auto *PN = dyn_cast<PHINode>(Store->getPointerOperand());
  if (!PN || !PN->hasNUses(3))
    return false;

  LoadInst *Load = nullptr;
  for (auto *U : PN->users()) {
    Load = dyn_cast<LoadInst>(U);
    if (Load)
      break;
  }
  if (!Load)
    return false;

  return isPartialPtrLoad(Load);
}

/// Information describing type alias information for temporary values used
/// within a function.
///
/// This class is used within the DTransAnalysis to track the types of data
/// that a value may point to, independent of the type of the value.
/// For example, consider the following line of IR:
///
///   %t = bitcast %struct.S* %ps to i8*
///
/// The type of %t is i8*, but because the value was created using a bitcast
/// we know that it points to a memory block that is treated as %struct.S
/// elsewhere in the IR.
///
/// This class is also used to track information about whether or not a value
/// points to an element within an aggregate type.  If a pointer value is
/// obtained using a GetElementPtr instruction, that pointer is a pointer to an
/// element in the base object.  In addition, if the pointer is subsequently
/// cast to another type, the bitcast value is also tracked as a pointer to the
/// same element.  For instance,
///
///   %struct.S = type { i32, i32 }
///   ...
///   %pb = getelementptr %struct.S, %struct.S* %ps, i64 0, i32 1
///   %tb = bitcast i32* %pb to i8*
///
/// In this case, %pb and %tb are both tracked as pointers to the second
/// element of the %struct.S structure.
class LocalPointerInfo {
public:
  typedef std::set<std::pair<llvm::Type *, size_t>> ElementPointeeSet;
  typedef std::set<std::pair<llvm::Type *, size_t>> &ElementPointeeSetRef;
  typedef SmallPtrSet<llvm::Type *, 3> PointerTypeAliasSet;
  typedef SmallPtrSetImpl<llvm::Type *> &PointerTypeAliasSetRef;

  LocalPointerInfo()
      : AnalysisState(LPIS_NotAnalyzed), AliasesToAggregatePointer(false),
        IsPartialPtrLoadStore(false) {}

  void setAnalyzed() { AnalysisState = LPIS_AnalysisComplete; }
  bool getAnalyzed() { return AnalysisState == LPIS_AnalysisComplete; }

  bool canAliasToAggregatePointer() {
    assert(getAnalyzed() &&
           "canAliasToAggregatePointer called for incomplete LPI!");
    return AliasesToAggregatePointer;
  }

  bool isPartialAnalysis() { return AnalysisState == LPIS_PartiallyAnalyzed; }
  void setPartialAnalysis(bool b) {
    assert(AnalysisState != LPIS_AnalysisComplete &&
           "Regression in analysis state!");
    AnalysisState = LPIS_PartiallyAnalyzed;
  }

  void addPointerTypeAlias(llvm::Type *T) {
    // This should only be called for integers that are returned by PtrToInt
    // instructions or actual pointers.
    assert(T->isPointerTy() || T->isIntegerTy());
    // Check to see if this is a pointer (at any level of indirection) to
    // an aggregate type. That will make it faster later to tell if a value
    // is interesting or not.
    Type *BaseTy = T;
    while (BaseTy->isPointerTy())
      BaseTy = BaseTy->getPointerElementType();
    if (BaseTy->isAggregateType())
      AliasesToAggregatePointer = true;
    // Save this alias.
    PointerTypeAliases.insert(T);
  }

  // If a pointer is pointing to an element of an aggregate, we want to track
  // that information.
  void addElementPointee(llvm::Type *Base, size_t ElemIdx) {
    ElementPointees.insert(std::make_pair(Base, ElemIdx));
  }

  bool typesCompatible(llvm::Type *T1, llvm::Type *T2) {
    if (T1 == T2)
      return true;

    if (T1 == nullptr || T2 == nullptr)
      return false;

    if (T1->isPointerTy() && T2->isPointerTy()) {
      // Consider a pointer to type T and a pointer to an array of type T as
      // compatible types.
      if (T1->getPointerElementType()->isArrayTy())
        return (T2->getPointerElementType() ==
                T1->getPointerElementType()->getArrayElementType());

      if (T2->getPointerElementType()->isArrayTy())
        return (T1->getPointerElementType() ==
                T2->getPointerElementType()->getArrayElementType());
      else
        return typesCompatible(T1->getPointerElementType(),
                               T2->getPointerElementType());
    }

    if (T1->isArrayTy() && T2->isArrayTy() &&
        (T1->getArrayNumElements() == T2->getArrayNumElements()))
      return typesCompatible(T1->getArrayElementType(),
                             T2->getArrayElementType());
    return false;
  }

  bool canPointToType(llvm::Type *T) {
    for (auto *AliasTy : PointerTypeAliases)
      if (AliasTy->isPointerTy() &&
          typesCompatible(AliasTy->getPointerElementType(), T))
        return true;
    return false;
  }

  bool pointsToSomeElement() { return ElementPointees.size() > 0; }

  bool pointsToMultipleAggregateTypes() {
    if (!AliasesToAggregatePointer)
      return false;
    int NumAliased =
        std::count_if(PointerTypeAliases.begin(), PointerTypeAliases.end(),
                      [](llvm::Type *T) {
                        if (!T->isPointerTy())
                          return false;
                        return T->getPointerElementType()->isAggregateType();
                      });
    // AliasesToAggregatePointer should be set as aliases are added, but
    // since we have the information here we can assert to make sure things
    // are working as expected.
    assert((NumAliased < 1) || AliasesToAggregatePointer);
    return NumAliased > 1;
  }

  llvm::Type *getDominantAggregateTy() {
    // TODO: Compute this as aliases are added.
    if (!AliasesToAggregatePointer)
      return nullptr;
    llvm::Type *DomTy = nullptr;
    for (auto *AliasTy : PointerTypeAliases) {
      llvm::Type *BaseTy = AliasTy;
      while (BaseTy->isPointerTy())
        BaseTy = BaseTy->getPointerElementType();
      if (!BaseTy->isAggregateType())
        continue;
      if (!DomTy) {
        DomTy = AliasTy;
        continue;
      }
      // If this type can be an element zero access of DomTy,
      // DomTy is still dominant.
      if (dtrans::isElementZeroAccess(DomTy, AliasTy))
        continue;
      // If what we previously thought was the dominant type can be
      // an element zero access of the current alias, the current
      // alias becomes dominant.
      if (dtrans::isElementZeroAccess(AliasTy, DomTy)) {
        DomTy = AliasTy;
        continue;
      }
      // Otherwise, there are conflicting aliases and nothing can be dominant.
      return nullptr;
    }
    return DomTy;
  }

  // If we detect that a bitcast is the beginning of a partial pointer
  // load/store idiom, we set this flag so the analysis knows to handle
  // the bitcast differently.
  void setPartialPtrLoadStore() { IsPartialPtrLoadStore = true; }
  bool isPartialPtrLoadStore() { return IsPartialPtrLoadStore; }

  bool isPtrToPtr() {
    llvm::Type *DomTy = getDominantAggregateTy();
    if (!DomTy)
      return false;
    if (!DomTy->isPointerTy())
      return false;
    if (!DomTy->getPointerElementType()->isPointerTy())
      return false;
    return true;
  }

  PointerTypeAliasSetRef getPointerTypeAliasSet() { return PointerTypeAliases; }
  ElementPointeeSetRef getElementPointeeSet() { return ElementPointees; }

  void merge(LocalPointerInfo &Other) {
    // This routine is called during analysis, so don't change AnalysisState.
    AliasesToAggregatePointer |= Other.AliasesToAggregatePointer;
    for (auto *Ty : Other.PointerTypeAliases)
      PointerTypeAliases.insert(Ty);
    for (auto &Pair : Other.ElementPointees)
      ElementPointees.insert(Pair);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD
  void dump() {
    dbgs() << "LocalPointerInfo:\n";
    if (PointerTypeAliases.empty()) {
      dbgs() << "  No aliased types.\n";
    } else {
      dbgs() << "  Aliased types:\n";
      for (auto *Ty : PointerTypeAliases)
        dbgs() << "    " << *Ty << "\n";
    }
    if (ElementPointees.empty()) {
      dbgs() << "  No element pointees.\n";
    } else {
      dbgs() << "  Element pointees:\n";
      for (auto &PointeePair : ElementPointees)
        dbgs() << "    " << *PointeePair.first << " @ " << PointeePair.second
               << "\n";
    }
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  enum LPIState {
    LPIS_NotAnalyzed,
    LPIS_PartiallyAnalyzed,
    LPIS_AnalysisComplete
  };

  LPIState AnalysisState;
  bool AliasesToAggregatePointer;
  PointerTypeAliasSet PointerTypeAliases;
  ElementPointeeSet ElementPointees;
  bool IsPartialPtrLoadStore;
};

// Class to analyze and identify functions that are post-dominated by
// a call to malloc() or free(). Those post-dominated by malloc() will
// yield true for isMallocPostDom().  Those post-dominated by free()
// will yield true for isFreePostDom().
//
// Functions whose prototype match malloc() and free() need not have their
// return value strictly post-dominated by malloc() or free(). Exceptions
// are made for "skip cases", like the following:
//
// (1) Function calling malloc() does nothing if size argument is 0:
//
//   extern void *mymalloc(int size) {
//     if (size != 0)
//       return malloc(size);
//     return nullptr;
//   }
//
// (2) Function returns immediately after call to malloc() if malloc()
//     returns a nullptr:
//
//   extern void *mymalloc(int size) {
//     char *ptr = malloc(size);
//     if (ptr == nullptr) {
//       printf("Warning!\n");
//       return nullptr;
//     }
//     return ptr;
//   }
//
// (3) Function calling free() does nothing if argument is nullptr:
//
//   extern void myfree(void *ptr) {
//     if (ptr != 0)
//       free(ptr);
//   }
//
// In cases (1) and (2), we will still return true for isMallocPostDom(),
// even through the call to malloc() does not post-dominate all returns.
// Similarly, in case (3), we wll still return true for isFreePostDom()
// even though not all paths to the return are post-dominated by a call
// to free().
//
class DTransAllocAnalyzer {
public:
  DTransAllocAnalyzer(const TargetLibraryInfo &TLI) : TLI(TLI) {}
  bool isMallocPostDom(Function *F);
  bool isFreePostDom(Function *F);

private:
  // An enum recording the current status of a function. The status is
  // updated each time we need to know if the function is isMallocPostDom()
  // or isFreePostDom(), until the function is determined to be one or
  // the other or neither.
  enum AllocStatus {
    AKS_Unknown,
    AKS_Malloc,
    AKS_Free,
    AKS_NotMalloc,
    AKS_NotFree,
    AKS_NotMallocFree
  };
  // Mapping for the AllocStatus of each Function we have queried.
  std::map<Function *, AllocStatus> LocalMap;
  // A set to hold visited BasicBlocks.  This is a temporary set used
  // while we are determining the AllocStatus of a Function.
  SmallPtrSet<BasicBlock *,20> VisitedBlocks;
  // A set to hold the BasicBlocks which do not need to be post-dominated
  // by malloc() to be considered isMallocPostDom() or free to be considered
  // isFreePostDom().
  SmallPtrSet<BasicBlock *, 4> SkipTestBlocks;
  // Needed to detrmine if a function is malloc()
  const TargetLibraryInfo &TLI;

  bool isSkipTestBlock(BasicBlock *BB) const;
  bool isVisitedBlock(BasicBlock *BB) const;
  int skipTestSuccessor(BranchInst *BI) const;
  void visitAndSetSkipTestSuccessors(BasicBlock *BB);
  void visitAndResetSkipTestSuccessors(BasicBlock *BB);
  void visitNullPtrBlocks(Function *F);

  bool mallocBasedGEPChain(GetElementPtrInst *GV, GetElementPtrInst **GBV,
                           CallInst **GCI) const;
  bool mallocOffset(Value *V, int64_t *offset) const;
  bool mallocLimit(GetElementPtrInst *GBV, Value *V, int64_t *Result) const;
  bool returnValueIsMallocAddress(Value *RV, BasicBlock *BB);
  bool analyzeForMallocStatus(Function *F);

  bool hasFreeCall(BasicBlock *BB) const;
  bool isPostDominatedByFreeCall(BasicBlock *BB);
  bool analyzeForFreeStatus(Function *F);
};

//
// Return true if 'BB' is a skip test block, e.g. a BasicBlock which does
// not need to be post-dominated by malloc() to be isMallocPostDom(), or
// post-dominated by free() to be isFreePostDom().
//
bool DTransAllocAnalyzer::isSkipTestBlock(BasicBlock *BB) const {
  return SkipTestBlocks.find(BB) != SkipTestBlocks.end();
}

//
// Return true if 'BB' has been visited.
//
bool DTransAllocAnalyzer::isVisitedBlock(BasicBlock *BB) const {
  return VisitedBlocks.find(BB) != VisitedBlocks.end();
}

//
// Return true if 'F' is post-dominated by a call to malloc() on all paths
// that do not include skip blocks.
//
bool DTransAllocAnalyzer::isMallocPostDom(Function *F) {
  AllocStatus AS = LocalMap[F];
  switch (AS) {
  case AKS_Malloc:
    return true;
  case AKS_Free:
  case AKS_NotMalloc:
  case AKS_NotMallocFree:
    return false;
  case AKS_NotFree:
    if (analyzeForMallocStatus(F)) {
      LocalMap[F] = AKS_Malloc;
      return true;
    }
    LocalMap[F] = AKS_NotMallocFree;
    return false;
  case AKS_Unknown:
    if (analyzeForMallocStatus(F)) {
      LocalMap[F] = AKS_Malloc;
      return true;
    }
    LocalMap[F] = AKS_NotMalloc;
    return false;
  }
  return false;
}

//
// Return true if 'F' is post-dominated by a call to free() on all paths
// that do not include skip blocks.
//
bool DTransAllocAnalyzer::isFreePostDom(Function *F) {
  AllocStatus AS = LocalMap[F];
  switch (AS) {
  case AKS_Free:
    return true;
  case AKS_Malloc:
  case AKS_NotFree:
  case AKS_NotMallocFree:
    return false;
  case AKS_NotMalloc:
    if (analyzeForFreeStatus(F)) {
      LocalMap[F] = AKS_Free;
      return true;
    }
    LocalMap[F] = AKS_NotMallocFree;
    return false;
  case AKS_Unknown:
    if (analyzeForFreeStatus(F)) {
      LocalMap[F] = AKS_Free;
      return true;
    }
    LocalMap[F] = AKS_NotFree;
    return false;
  }
  return false;
}

//
// Return:
//  0 if the 0th operand of the 'BranchInst' is the successor which will
//    be taken if the skip test condition is satisfied.
//  1 if the 1st operand of the 'BranchInst' is the successor which will
//    be taken if the skip test condition is satisfied.
// -1 otherwise
//
// For example:
//   If the skip test is "ptr == nullptr", we will return 0.
//   If the skip test is "ptr != nullptr", we will return 1.
//
int DTransAllocAnalyzer::skipTestSuccessor(BranchInst *BI) const {
  if (!BI || BI->isUnconditional())
    return -1;
  if (BI->getNumSuccessors() != 2)
    return -1;
  auto *CI = dyn_cast<Constant>(BI->getCondition());
  if (CI)
    return CI->isNullValue() ? 0 : 1;
  auto *ICI = dyn_cast<ICmpInst>(BI->getCondition());
  if (ICI == nullptr || !ICI->isEquality())
    return -1;
  Value *V = nullptr;
  if (isa<ConstantPointerNull>(ICI->getOperand(0)))
    V = ICI->getOperand(1);
  else if (isa<ConstantPointerNull>(ICI->getOperand(1)))
    V = ICI->getOperand(0);
  if (V == nullptr)
    return -1;
  if (isa<Argument>(V))
    return ICI->getPredicate() == ICmpInst::ICMP_EQ ? 0 : 1;
  if (auto CCI = dyn_cast<CallInst>(V))
    if (dtrans::getAllocFnKind(CCI->getCalledFunction(), TLI) ==
        dtrans::AK_Malloc)
      return ICI->getPredicate() == ICmpInst::ICMP_EQ ? 0 : 1;
  return -1;
}

//
// If 'BB' is not already a skip test block, mark it and all of its
// successors (and their successors, etc.) as skip test blocks.
//
void DTransAllocAnalyzer::visitAndSetSkipTestSuccessors(BasicBlock *BB) {
  if (BB == nullptr)
    return;
  auto it = SkipTestBlocks.find(BB);
  if (it != SkipTestBlocks.end())
    return;
  SkipTestBlocks.insert(BB);
  for (auto BBS : successors(BB))
    visitAndSetSkipTestSuccessors(BBS);
}

//
// If 'BB' is not already a visited block, mark it and all of its
// successors (and their successors, etc.) as not being skip test blocks.
//
void DTransAllocAnalyzer::visitAndResetSkipTestSuccessors(BasicBlock *BB) {
  if (BB == nullptr)
    return;
  if (!VisitedBlocks.insert(BB).second)
    return;
  auto jt = SkipTestBlocks.find(BB);
  if (jt != SkipTestBlocks.end())
    SkipTestBlocks.erase(*jt);
  for (auto BBS : successors(BB))
    visitAndResetSkipTestSuccessors(BBS);
}

//
// Mark as skip test blocks for 'F', all those blocks which include a skip
// test, and are on a path starting with the skip test successor of that
// block, but are not on some other path which is not a successor of a
// skip test block.
//
void DTransAllocAnalyzer::visitNullPtrBlocks(Function *F) {
  SmallPtrSet<BasicBlock *, 4> SkipBlockSet;
  SmallPtrSet<BasicBlock *, 20> NoSkipBlockSet;
  SkipTestBlocks.clear();
  VisitedBlocks.clear();
  for (BasicBlock &BB : *F)
    if (auto BI = dyn_cast<BranchInst>(BB.getTerminator())) {
      int rv = skipTestSuccessor(BI);
      if (rv >= 0) {
        SkipBlockSet.insert(BI->getParent());
        SkipBlockSet.insert(BI->getSuccessor(rv));
        NoSkipBlockSet.insert(BI->getSuccessor(1 - rv));
      }
    }
  for (auto *SBB: SkipBlockSet)
    visitAndSetSkipTestSuccessors(SBB);
  for (auto *NSBB: NoSkipBlockSet)
    visitAndResetSkipTestSuccessors(NSBB);
}

//
// Return true if 'GV' is the root of a malloc based GEP chain. This
// means that if we keep following the pointer operand for a series of
// GEP instructions, we will eventually get to a malloc() call.
//
// For example:
//   %5 = tail call noalias i8* @malloc(i64 %4)
//   %8 = getelementptr inbounds i8, i8* %5, i64 27
//   %12 = getelementptr inbounds i8, i8* %8, i64 %11
// Here %12 is the root of a malloc based GEP chain.
//
// In the case that we return true, we set '*GBV' to the GEP immediately
// preceding the call to malloc (in this example %8) and we set '*GCI'
// to the call to malloc (in this example %5).
//
bool DTransAllocAnalyzer::mallocBasedGEPChain(GetElementPtrInst *GV,
                                              GetElementPtrInst **GBV,
                                              CallInst **GCI) const {
  GetElementPtrInst *V;
  for (V = GV; isa<GetElementPtrInst>(V->getPointerOperand());
       V = dyn_cast<GetElementPtrInst>(V->getPointerOperand())) {
    if (!V->getSourceElementType()->isIntegerTy(8))
      return false;
  }
  if (!V->getSourceElementType()->isIntegerTy(8))
    return false;
  auto CI = dyn_cast<CallInst>(V->getPointerOperand());
  if (!CI)
    return false;
  if (dtrans::getAllocFnKind(CI->getCalledFunction(), TLI) != dtrans::AK_Malloc)
    return false;
  *GBV = V;
  *GCI = CI;
  return true;
}

//
// Return 'true' if the function calls malloc() with a value equal to
// its own argument plus some offset.
//
// For example:
//   %2 = add nsw i32 %0, 15
//   %3 = sext i32 %2 to i64
//   %4 = add nsw i64 %3, 12
//   %5 = tail call noalias i8* @malloc(i64 %4)
//
// Here, assuming %0 is the function argument, malloc is called with a
// value of %0 + 27.
//
// When we return true, we set '*offset' to the offset (which in this
// example is 27).
//
bool DTransAllocAnalyzer::mallocOffset(Value *V, int64_t *offset) const {
  int64_t Result = 0;
  while (!isa<Argument>(V)) {
    if (auto BI = dyn_cast<BinaryOperator>(V)) {
      if (BI->getOpcode() == Instruction::Add) {
        if (auto CI = dyn_cast<ConstantInt>(BI->getOperand(0))) {
          V = BI->getOperand(1);
          Result += CI->getSExtValue();
        } else if (auto CI = dyn_cast<ConstantInt>(BI->getOperand(1))) {
          V = BI->getOperand(0);
          Result += CI->getSExtValue();
        } else
          return false;
      }
    } else if (auto SI = dyn_cast<SExtInst>(V))
      V = SI->getOperand(0);
    else
      return false;
  }
  *offset = Result;
  return true;
}

//
// Return true if 'Value' is 2^n-1 for some n.
//
static bool isLowerBitMask(int64_t Value) {
  while (Value & 1)
    Value >>= 1;
  return Value == 0;
}

//
// Return true if we can find an upper bound for the amount 'V' is less than
// the address computed by the 'GBV'.
//
// For example:
//
//  %8 = getelementptr inbounds i8, i8* %5, i64 27
//  %9 = ptrtoint i8* %8 to i64
//  %10 = and i64 %9, 15
//  %11 = sub nsw i64 0, %10
//  %12 = getelementptr inbounds i8, i8* %8, i64 %11
//
// If 'V' here is %11 and 'GBV' is %8, then the most that %12 can be less
// than %8 is 15.
//
// If we return true, we set '*Result' to the value of the upper bound.
//
// NOTE: mallocLimit() is a bit of a pattern match, albeit for a very
// important case.
//
bool DTransAllocAnalyzer::mallocLimit(GetElementPtrInst *GBV, Value *V,
                                      int64_t *Result) const {
  auto BIS = dyn_cast<BinaryOperator>(V);
  if (!BIS || BIS->getOpcode() != Instruction::Sub)
    return false;
  auto CI = dyn_cast<ConstantInt>(BIS->getOperand(0));
  if (!CI || !CI->isZero())
    return false;
  auto BIA = dyn_cast<BinaryOperator>(BIS->getOperand(1));
  if (!BIA || BIA->getOpcode() != Instruction::And)
    return false;
  Value *W = nullptr;
  int64_t Limit = 0;
  if ((CI = dyn_cast<ConstantInt>(BIA->getOperand(0)))) {
    W = BIA->getOperand(1);
    Limit = CI->getSExtValue();
  } else if ((CI = dyn_cast<ConstantInt>(BIA->getOperand(1)))) {
    W = BIA->getOperand(0);
    Limit = CI->getSExtValue();
  } else
    return false;
  if (!isLowerBitMask(Limit))
    return false;
  auto PI = dyn_cast<PtrToIntInst>(W);
  if (!PI)
    return false;
  auto GEP = dyn_cast<GetElementPtrInst>(PI->getOperand(0));
  if (GEP != GBV)
    return false;
  *Result = Limit;
  return true;
}

//
// Return true if 'RV' is a return value post-dominated by a call to
// malloc(). 'BB' is the BasicBlock containing 'RV'.
//
// NOTE: We ensure that all return values derived from calls to malloc()
// point to some address in the memeory that was allocated.
//
bool DTransAllocAnalyzer::returnValueIsMallocAddress(Value *RV,
                                                     BasicBlock *BB) {
  if (isVisitedBlock(BB))
    return false;
  VisitedBlocks.insert(BB);
  if (auto *CI = dyn_cast<CallInst>(RV))
    return dtrans::getAllocFnKind(CI->getCalledFunction(), TLI) ==
           dtrans::AK_Malloc;
  if (auto *PI = dyn_cast<PHINode>(RV)) {
    bool rv = false;
    for (unsigned I = 0; I < PI->getNumIncomingValues(); ++I) {
      Value *V = PI->getIncomingValue(I);
      BasicBlock *PB = PI->getIncomingBlock(I);
      bool NullValue = isa<ConstantPointerNull>(V);
      bool IsSkipTestBlock = isSkipTestBlock(PB);
      if ((NullValue && !IsSkipTestBlock) || (!NullValue && IsSkipTestBlock))
        return false;
      if (!NullValue && !IsSkipTestBlock && !returnValueIsMallocAddress(V, PB))
        return false;
      rv = true;
    }
    return rv;
  }
  if (auto *GV = dyn_cast<GetElementPtrInst>(RV)) {
    int64_t Limit, Offset;
    GetElementPtrInst *GBV;
    CallInst *CI;
    if (!mallocBasedGEPChain(GV, &GBV, &CI))
      return false;
    if (!mallocOffset(CI->getOperand(0), &Offset))
      return false;
    if (!mallocLimit(GBV, GV->getOperand(1), &Limit))
      return false;
    return Offset >= Limit;
  }
  return false;
}

//
// Return true if 'F' is isMallocPostDom().
//
bool DTransAllocAnalyzer::analyzeForMallocStatus(Function *F) {
  if (F == nullptr)
    return false;
  LLVM_DEBUG(dbgs() << "Analyzing for MallocPostDom " << F->getName() << "\n");
  // Make sure that VisitedBlocks and SkipTestBlocks are clear before
  // visitNullPtrBlocks() is called. SkipTestBlocks are valid until we
  // return from this function.  VisitedBlocks can be cleared immediately
  // after visitNullPtrBlocks() is run.
  VisitedBlocks.clear();
  SkipTestBlocks.clear();
  if (std::distance(F->arg_begin(), F->arg_end()) != 1 ||
      !F->arg_begin()->getType()->isIntegerTy()) {
    return false;
  }
  visitNullPtrBlocks(F);
  VisitedBlocks.clear();
  bool rv = false;
  for (BasicBlock &BB : *F)
    if (auto RI = dyn_cast<ReturnInst>(BB.getTerminator())) {
      Value *RV = RI->getReturnValue();
      if (RV == nullptr) {
        LLVM_DEBUG(dbgs() << "Not MallocPostDom " << F->getName()
                          << " Return is nullptr\n");
        return false;
      }
      if (!returnValueIsMallocAddress(RV, &BB)) {
        LLVM_DEBUG(dbgs() << "Not MallocPostDom " << F->getName()
                          << " Return is not malloc address\n");
        return false;
      }
      rv = true;
    }
  if (rv)
    LLVM_DEBUG(dbgs() << "Is MallocPostDom " << F->getName() << "\n");
  else
    LLVM_DEBUG(dbgs() << "Not MallocPostDom " << F->getName()
                      << " No malloc address returned\n");
  return rv;
}

//
// Return true if 'BB' has a call to free().
//
bool DTransAllocAnalyzer::hasFreeCall(BasicBlock *BB) const {
  for (auto BI = BB->rbegin(), BE = BB->rend(); BI != BE;) {
    Instruction *I = &*BI++;
    if (auto *CI = dyn_cast<CallInst>(I))
      if (dtrans::isFreeFn(CI->getCalledFunction(), TLI))
        return true;
  }
  return false;
}

//
// Return true if 'BB' contains or is dominated by a call to free()
// on all predecessors.
//
bool DTransAllocAnalyzer::isPostDominatedByFreeCall(BasicBlock *BB) {
  bool rv = false;
  if (isVisitedBlock(BB))
    return false;
  VisitedBlocks.insert(BB);
  bool IsSkipTestBlock = isSkipTestBlock(BB);
  if (IsSkipTestBlock || hasFreeCall(BB))
    return true;
  for (BasicBlock *PB : predecessors(BB)) {
    if (!isPostDominatedByFreeCall(PB))
      return false;
    rv = true;
  }
  return rv;
}

//
// Return true if 'F' is isFreePostDom().
//
bool DTransAllocAnalyzer::analyzeForFreeStatus(Function *F) {
  if (F == nullptr)
    return false;
  // Make sure that VisitedBlocks and SkipTestBlocks are clear before
  // visitNullPtrBlocks() is called. SkipTestBlocks are valid until we
  // return from this function.  VisitedBlocks can be cleared immediately
  // after visitNullPtrBlocks() is run.
  VisitedBlocks.clear();
  SkipTestBlocks.clear();
  if (std::distance(F->arg_begin(), F->arg_end()) == 1 &&
      F->arg_begin()->getType()->isPointerTy()) {
    visitNullPtrBlocks(F);
    VisitedBlocks.clear();
  }
  LLVM_DEBUG(dbgs() << "Analyzing for FreePostDom " << F->getName() << "\n");
  bool rv = false;
  for (BasicBlock &BB : *F)
    if (auto Ret = dyn_cast<ReturnInst>(BB.getTerminator())) {
      if (Ret->getReturnValue() != nullptr) {
        LLVM_DEBUG(dbgs() << "Not FreePostDom " << F->getName()
                          << " Return is not nullptr\n");
        return false;
      }
      if (!isPostDominatedByFreeCall(&BB)) {
        LLVM_DEBUG(dbgs() << "Not FreePostDom " << F->getName()
                          << " Return is not post-dominated by call to free\n");
        return false;
      }
      rv = true;
    }
  if (rv)
    LLVM_DEBUG(dbgs() << "Is FreePostDom " << F->getName() << "\n");
  else
    LLVM_DEBUG(dbgs() << "Not FreePostDom " << F->getName()
                      << " No return post-dominated by free\n");
  return rv;
}

// End of member functions for class DTransAllocAnalyzer

class LocalPointerAnalyzer {
public:
  LocalPointerAnalyzer(const DataLayout &DL, const TargetLibraryInfo &TLI,
                       DTransAllocAnalyzer &DTAA)
      : DL(DL), TLI(TLI), DTAA(DTAA) {}

  LocalPointerInfo &getLocalPointerInfo(Value *V) {
    LocalPointerInfo &Info = LocalMap[V];
    if (!Info.getAnalyzed())
      analyzeValue(V);
    assert(Info.getAnalyzed() && "Local pointer analysis failed.");
    return Info;
  }

  // This utility function is used by both the local pointer analysis and
  // in the main DTransAnalysisInfo class.
  bool isPossiblePtrValue(Value *V) {
    // If the value is a pointer or the result of a pointer-to-int cast
    // it definitely is a pointer.
    if (V->getType()->isPointerTy() || isa<PtrToIntOperator>(V))
      return true;

    // If the value is not a pointer and is not a pointer-sized integer, it
    // is definitely not a value we will track as a pointer.
    if (V->getType() !=
        llvm::Type::getIntNTy(V->getContext(), DL.getPointerSizeInBits()))
      return false;

    // If it is a pointer-sized integer, we need may need to analyze it if
    // it is the result of a load, select or PHI node.
    if (isa<LoadInst>(V) || isa<SelectInst>(V) || isa<PHINode>(V))
      return true;

    // Otherwise, we don't need to analyze it as a pointer.
    return false;
  }

private:
  const DataLayout &DL;
  const TargetLibraryInfo &TLI;
  DTransAllocAnalyzer &DTAA;
  // We cannot use DenseMap or ValueMap here because we are inserting values
  // during recursive calls to analyzeValue() and with a DenseMap or ValueMap
  // that would cause the LocalPointerInfo to be copied and our local
  // reference to it to be invalidated.
  std::map<Value *, LocalPointerInfo> LocalMap;

  void analyzeValue(Value *V) {
    // If we've already analyzed this value, there is no need to
    // repeat the work.
    LocalPointerInfo &Info = LocalMap[V];
    if (Info.getAnalyzed())
      return;

    // If this isn't either a pointer or derived from a pointer, we don't need
    // to do any analysis.
    if (!isPossiblePtrValue(V)) {
      Info.setAnalyzed();
      return;
    }

    // Build a stack of unresolved dependent values that must be analyzed
    // before we can complete the analysis of this value.
    SmallVector<Value *, 16> DependentVals;
    DependentVals.push_back(V);
    populateDependencyStack(V, DependentVals);

    // This first line is intentionally left non-verbose.
    LLVM_DEBUG(dbgs() << "analyzeValue " << *V << "\n");
    DEBUG_WITH_TYPE(LPA_VERBOSE, dumpDependencyStack(DependentVals));

    // Now attempt to analyze each of these values. Some may be left in a
    // partially analyzed state, but they will be fully resolved when
    // their complete info is needed.
    while (!DependentVals.empty()) {
      Value *Dep = DependentVals.back();
      DependentVals.pop_back();
      LocalPointerInfo &DepInfo = LocalMap[Dep];
      // If we have complete results for this value, don't repeat the analysis.
      if (DepInfo.getAnalyzed()) {
        DEBUG_WITH_TYPE(LPA_VERBOSE,
                        dbgs() << "  Already analyzed: " << *Dep << "\n");
        continue;
      }
      analyzeValueImpl(Dep, DepInfo);
    }

    DEBUG_WITH_TYPE(LPA_VERBOSE, {
      if (Info.isPartialAnalysis())
        dbgs() << " Analysis completed but was reported as partial.\n";
    });
    // These are intentionally left non-verbose.
    LLVM_DEBUG(Info.dump());
    LLVM_DEBUG(dbgs() << "\n");

    // Some of the dependencies may have reported partial analysis, but
    // by the time we get here everything will have been complete for
    // this value.
    Info.setAnalyzed();
  }

  void analyzeValueImpl(Value *V, LocalPointerInfo &Info) {
    // If this isn't either a pointer or derived from a pointer, we don't need
    // to do any analysis.
    if (!isPossiblePtrValue(V)) {
      Info.setAnalyzed();
      return;
    }

    // If the value is an i8* function argument, look ahead to its uses to
    // infer its alias set.
    if (isa<Argument>(V) && isValueInt8PtrType(V)) {
      analyzePossibleVoidPtrArgument(V, Info);
      return;
    }

    if (isPartialPtrBitCast(V)) {
      LLVM_DEBUG(dbgs() << "Partial pointer bitcast detected: " << *V << "\n");
      Info.setPartialPtrLoadStore();
      Info.setAnalyzed();
      return;
    }

    // If this value is derived from another local value, follow the
    // collect info from the source operand.
    if (isDerivedValue(V))
      collectSourceOperandInfo(V, Info);

    // If this value has a pointer type, add the type of the value.
    // An example of a value that would be used here but not have a pointer
    // type is the result of a PtrToInt instruction.
    llvm::Type *VTy = V->getType();
    if (isa<PointerType>(VTy))
      Info.addPointerTypeAlias(VTy);

    if (auto *CI = getCallInstIfAlloc(V)) {
      // If the value we're analyzing is a call to an allocation function
      // we need to look for bitcast users so that we can proactively assign
      // the type to which the value will be cast as an alias.
      analyzeAllocationCallAliases(CI, Info);
    } else if (auto *GEP = dyn_cast<GEPOperator>(V)) {
      // If this is a GetElementPtr, figure out what element it is
      // accessing.
      analyzeElementAccess(GEP, Info);
    } else if (auto *Load = dyn_cast<LoadInst>(V)) {
      // If the value being analyzed is a load instruction the loaded value
      // may inherit some alias information from the load's pointer operand.
      analyzeLoadInstruction(Load, Info);
    }

    // If there were no unresolved dependencies, mark the info as analyzed.
    if (!Info.isPartialAnalysis())
      Info.setAnalyzed();
  }

  // Here we're looking for a bitcast to i32* that is passed into another block
  // where it is used as part of the partial pointer load/store idiom. The
  // idiom is generalized to handle 8-bit and 32-bit variants so here we're
  // just checking that the bitcast might feed the pattern, then we call a
  // helper function to do the rest of the check.
  bool isPartialPtrBitCast(Value *V) {
    llvm::Type *HalfPtrSizeIntPtrTy = llvm::Type::getIntNPtrTy(
        V->getContext(), DL.getPointerSizeInBits() / 2);

    auto *Cast = dyn_cast<BitCastInst>(V);
    if (!Cast || Cast->getType() != HalfPtrSizeIntPtrTy || !Cast->hasOneUse())
      return false;

    // We're peeking ahead a bit here in checking for three users.
    // We'll check that again in isPartPointerLoad() but checking it here
    // avoids potentially wasteful loops over the PHI's users if it cann't
    // match.
    auto *PN = dyn_cast<PHINode>(*Cast->user_begin());
    if (!PN || !PN->hasNUses(3))
      return false;

    LoadInst *Load = nullptr;
    for (auto *U : PN->users()) {
      Load = dyn_cast<LoadInst>(U);
      if (Load)
        break;
    }
    if (!Load)
      return false;

    return isPartialPtrLoad(Load);
  }

  CallInst *getCallInstIfAlloc(Value *V) {
    auto *CI = dyn_cast<CallInst>(V);
    if (!CI)
      return nullptr;
    Function *Callee = CI->getCalledFunction();
    dtrans::AllocKind Kind = dtrans::getAllocFnKind(Callee, TLI);
    if (Kind == dtrans::AK_NotAlloc && DTAA.isMallocPostDom(Callee))
      Kind = dtrans::AK_Malloc;
    if (Kind != dtrans::AK_NotAlloc)
      return CI;
    return nullptr;
  }

  // This helper function pushes a value on the back of the dependency stack
  // and returns true if this is the first occurance of the value on the stack
  // or false if it was present before the call. The value is pushed onto the
  // stack in either case.
  bool addDependency(Value *DV, SmallVectorImpl<Value *> &DependentVals) {
    auto REnd = DependentVals.rend();
    auto It = std::find(DependentVals.rbegin(), REnd, DV);
    DependentVals.push_back(DV);
    return (It == REnd);
  }

  // This routine adds the values on which \p V depends to the \p DependentVals
  // stack, calling itself recursively to add additional dependencies as
  // necessary.
  //
  // In order to handle cyclic dependencies, some values may be added to the
  // stack multiple times. If we are able to fully analyze a value, any
  // subsequent attempts to analyze the value will return a cached result.
  // However, we may only get partial results at some point which may be
  // improved by later re-analysis.
  //
  // In order to avoid infinite recursion, we do not traverse secondary
  // dependencies for values that have been added to the stack more than
  // once. Because we only need complete results for the initial value at
  // the base of the dependency stack, it is sufficient to have these
  // secondary dependencies appear just once, as far down in the stack as
  // possible.
  void populateDependencyStack(Value *V,
                               SmallVectorImpl<Value *> &DependentVals) {
    // If this isn't either a pointer or derived from a pointer, we don't need
    // to do any analysis.
    if (!isPossiblePtrValue(V))
      return;

    // BitCast and PtrToInt can be a ConstExpr acting on a global, so we
    // need to check the operator form. The instruction form of these
    // would be covered by CastInst, but that also handles IntToPtr which
    // does not have a ConstExpr equivalent.
    if (isa<BitCastOperator>(V) || isa<PtrToIntOperator>(V) ||
        isa<CastInst>(V)) {
      Value *Src = cast<User>(V)->getOperand(0);
      if (addDependency(Src, DependentVals))
        populateDependencyStack(Src, DependentVals);
      return;
    }

    // For PHI nodes we need to add all of the non-self incoming values.
    // The values themselves should all be analyzed after the dependencies for
    // all of the incoming values have been handled. This resolves as many
    // partial results as possible.
    if (auto *PN = dyn_cast<PHINode>(V)) {
      // We use a set here to avoid adding the same value twice if there are
      // duplicate incoming values. As we add these values to the dependency
      // stack we'll remove them from the set if they were previously on
      // the stack.
      SmallPtrSet<Value *, 4> NewDeps;
      for (Value *InVal : PN->incoming_values())
        if (InVal != PN)
          NewDeps.insert(InVal);
      auto It = NewDeps.begin();
      auto End = NewDeps.end();
      while (It != End) {
        Value *Dep = *It;
        ++It;
        if (!addDependency(Dep, DependentVals))
          NewDeps.erase(Dep);
      }
      for (Value *Dep : NewDeps)
        populateDependencyStack(Dep, DependentVals);
      return;
    }

    // For select nodes, we add both the true and false values.
    if (auto *Sel = dyn_cast<SelectInst>(V)) {
      Value *TV = Sel->getTrueValue();
      Value *FV = Sel->getFalseValue();
      bool TrueWasNew = addDependency(TV, DependentVals);
      bool FalseWasNew = addDependency(FV, DependentVals);
      if (TrueWasNew)
        populateDependencyStack(TV, DependentVals);
      if (FalseWasNew)
        populateDependencyStack(FV, DependentVals);
      return;
    }

    // GEPs only depend on the analysis of other values if they are in their
    // byte-flattened form.
    if (auto *GEP = dyn_cast<GEPOperator>(V)) {
      Value *BasePtr = GEP->getPointerOperand();
      if (BasePtr->getType() == llvm::Type::getInt8PtrTy(GEP->getContext())) {
        if (addDependency(BasePtr, DependentVals))
          populateDependencyStack(BasePtr, DependentVals);
      }
      return;
    }

    // Load instructions depend on the analysis of their pointer operand.
    if (auto *LI = dyn_cast<LoadInst>(V)) {
      Value *SrcPtr = LI->getPointerOperand();
      if (addDependency(SrcPtr, DependentVals))
        populateDependencyStack(SrcPtr, DependentVals);
      return;
    }

    // Allocation calls have a non-trivial dependency on their uses.
    // We have a helper function to handle this case.
    if (auto *CI = getCallInstIfAlloc(V)) {
      SmallPtrSet<User *, 8> VisitedUsers;
      addAllocUsesToDependencyStack(CI, DependentVals, VisitedUsers);
    }
  }

  void addAllocUsesToDependencyStack(Value *V,
                                     SmallVectorImpl<Value *> &DependentVals,
                                     SmallPtrSetImpl<User *> &VisitedUsers) {
    for (auto *U : V->users()) {
      // Don't re-visit users we've already seen.
      if (!VisitedUsers.insert(U).second)
        continue;

      // Although BitCast instructions are fundamental to the analysis
      // of allocation calls, we don't need to track their dependencies
      // because we're looking in the opposite direction of the use-chain
      // here.
      //
      // However, we need to follow the uses of the bitcast to see if this
      // value is stored somewhere.
      if (isa<CastInst>(U))
        addAllocUsesToDependencyStack(U, DependentVals, VisitedUsers);

      // We do need to follow the uses of PHI nodes and selects, because
      // they might lead to a store instruction, which does introduce a
      // dependency.
      if (isa<PHINode>(U) || isa<SelectInst>(U))
        addAllocUsesToDependencyStack(U, DependentVals, VisitedUsers);

      // Finally, if the allocated value is passed to a store instruction
      // we need to record a dependency on the pointer value because
      // the allocated type may be inferred from the alias information
      // for this pointer.
      if (auto *SI = dyn_cast<StoreInst>(U)) {
        Value *PtrOp = SI->getPointerOperand();
        Value *ValOp = SI->getValueOperand();
        // If the value we're analyzing is the pointer operand we will be
        // inferring the allocated type from the value written to this
        // location. If the value we're analyzing is the value operand we will
        // be inferring the allocated type from the pointer operand.
        Value *InferenceOp = (PtrOp == V) ? ValOp : PtrOp;
        if (addDependency(InferenceOp, DependentVals))
          populateDependencyStack(InferenceOp, DependentVals);
      }
    }
  }

  void dumpDependencyStack(SmallVectorImpl<Value *> &DependentVals) {
    dbgs() << "  DependentVals:\n";
    for (auto *V : DependentVals)
      dbgs() << "    " << *V << "\n";
    dbgs() << "\n";
  }

  // This helper function is here to avoid repeating the check for an
  // incomplete result.
  inline void mergeOperandInfo(Value *Op, LocalPointerInfo &TargetInfo) {
    LocalPointerInfo &OperandInfo = LocalMap[Op];
    TargetInfo.merge(OperandInfo);
    if (!OperandInfo.getAnalyzed()) {
      TargetInfo.setPartialAnalysis(true);
      DEBUG_WITH_TYPE(LPA_VERBOSE, dbgs() << "Incomplete analysis merged from "
                                          << *Op << "\n");
    }
  }

  void collectSourceOperandInfo(Value *V, LocalPointerInfo &Info) {
    // In each case, the call to analyzeValue performs a check which will
    // prevent infinite recursion if we track back to a block we've
    // already visited.
    if (auto *PN = dyn_cast<PHINode>(V)) {
      for (Value *InVal : PN->incoming_values())
        mergeOperandInfo(InVal, Info);
      return;
    }
    if (auto *Sel = dyn_cast<SelectInst>(V)) {
      mergeOperandInfo(Sel->getTrueValue(), Info);
      mergeOperandInfo(Sel->getFalseValue(), Info);
      return;
    }
    if (isa<CastInst>(V) || isa<BitCastOperator>(V) ||
        isa<PtrToIntOperator>(V)) {
      Value *SrcVal = cast<User>(V)->getOperand(0);
      LocalPointerInfo &SrcLPI = LocalMap[SrcVal];
      // If the incoming analysis was incomplete, what we do below won't be
      // complete, but the partial analysis may be necessary so we note the
      // incompleteness and continue.
      if (!SrcLPI.getAnalyzed()) {
        Info.setPartialAnalysis(true);
        DEBUG_WITH_TYPE(LPA_VERBOSE,
                        dbgs() << "Incomplete analysis collected from "
                               << *SrcVal << "\n");
      }
      // If the bitcast is part of an idiom where pointer values are copied
      // in smaller chunks, don't treat it like other bitcasts.
      if (isPartialPtrBitCast(V)) {
        LLVM_DEBUG(dbgs() << "Partial pointer bitcast detected: " << *V
                          << "\n");
        Info.setPartialPtrLoadStore();
        // Even if the input was partial, this is all we needed to know.
        Info.setAnalyzed();
        return;
      }
      // If this is a bitcast that would be a valid way to access element
      // zero of any type known to be aliased by SrcVal, then record this
      // as an element access rather than merging the incoming value's aliases.
      if (auto *BC = dyn_cast<BitCastOperator>(V)) {
        // FIXME: This has the potential to miss the case where the bitcast
        //        is accessing element zero of a type that is aliased through
        //        some value that we were not able to analyze (because of a
        //        cycle of PHI nodes). The result would be that we'd report
        //        a mismatched element access. However, I don't believe this
        //        unhandled case will actually happen without an overloaded
        //        alias set.
        // It's possible that the source value aliases multiple pointers that
        // meet the element zero idiom. If it does, we want to know about all
        // of them.
        bool IsElementZeroAccess = false;
        for (auto *AliasTy : SrcLPI.getPointerTypeAliasSet()) {
          llvm::Type *AccessedTy = nullptr;
          if (dtrans::isElementZeroAccess(AliasTy, BC->getDestTy(),
                                          &AccessedTy)) {
            Info.addElementPointee(AccessedTy->getPointerElementType(), 0);
            IsElementZeroAccess = true;
          }
        }
        // If this is an element zero access, don't merge the source info.
        if (IsElementZeroAccess)
          return;
      }
      Info.merge(SrcLPI);
      return;
    }
    // The caller should have checked isDerivedValue() before calling this
    // function, and the above cases should cover all possible derived
    // values.
    llvm_unreachable("Unexpected class for derived value!");
  }

  void analyzeElementAccess(GEPOperator *GEP, LocalPointerInfo &Info) {
    auto *Int8PtrTy = llvm::Type::getInt8PtrTy(GEP->getContext());

    // If the base pointer is an i8* we need to analyze this as a
    // byte-flattened GEP unless the base pointer is a pointer-to-pointer.
    Value *BasePointer = GEP->getPointerOperand();
    LocalPointerInfo &BaseLPI = LocalMap[BasePointer];
    if ((BasePointer->getType() == Int8PtrTy) && !BaseLPI.isPtrToPtr()) {
      analyzeByteFlattenedGEPAccess(GEP, Info);
      return;
    }

    // A GEP with only one index argument is a special case where a pointer
    // is being used as an array. That doesn't get us a pointer to an element
    // within an aggregate type.
    if (GEP->getNumIndices() == 1) {
      // If the incoming analysis was incomplete, what we do below won't be
      // complete, but the partial analysis may be necessary so we note the
      // incompleteness and continue.
      if (!BaseLPI.getAnalyzed()) {
        Info.setPartialAnalysis(true);
        DEBUG_WITH_TYPE(LPA_VERBOSE, dbgs()
                                         << "Incomplete analysis derived from "
                                         << *BasePointer << "\n");
      }
      Info.merge(BaseLPI);
      return;
    }

    // There's an odd case where LLVM's constant folder will transform
    // a bitcast into a GEP if the first element of the structure at
    // any level of nesting matches the type of the bitcast. Normally
    // this is good, but if the first element is an i8 (or a fixed array
    // of i8, or a nested structure whose first element is an i8, etc.)
    // then this folding can lead to a misleading GEP where the code
    // was actually just trying to obtain a void pointer to the structure.
    //
    // For that reason, if this GEP returns an i8* and all but its first
    // index arguments are zero, we want to include the base structure
    // type in the alias set. The first index argument does not index
    // into the structure but offsets from it (as a dynamic array) so
    // this case applies even if the first index is non-zero.
    if (GEP->getType() == Int8PtrTy) {
      bool IsBitCastEquivalent = true;
      for (unsigned i = 1; i < GEP->getNumIndices(); ++i)
        // The +1 here is because the first operand of a GEP is not an index.
        if (ConstantInt *CI = dyn_cast<ConstantInt>(GEP->getOperand(i + 1)))
          if (!CI->isZero())
            IsBitCastEquivalent = false;
      if (IsBitCastEquivalent)
        Info.addPointerTypeAlias(BasePointer->getType());
    }

    // Find the type of the type of the last composite type being
    // indexed by this GEP.
    SmallVector<Value *, 4> Ops(GEP->idx_begin(), GEP->idx_end() - 1);
    Type *IndexedTy =
        GetElementPtrInst::getIndexedType(GEP->getSourceElementType(), Ops);

    // We should be able to get the indexed type for any valid GEP instruction.
    assert(IndexedTy);

    // Get the last index argument. If we can't determine its value,
    // we can't handle this GEP.
    // FIXME: With SCEV we might be able to handle some non-constant cases.
    // FIXME: Handle arrays with non-constant indices.
    auto *LastArg =
        dyn_cast<ConstantInt>(GEP->getOperand(GEP->getNumOperands() - 1));
    if (!LastArg)
      return;
    uint64_t Idx = LastArg->getLimitedValue();

    // Add this information to the local pointer information for the GEP.
    Info.addElementPointee(IndexedTy, Idx);

    return;
  }

  // This method determines the real aggregate type and element index being
  // accessed by an i8* based GEP instruction.
  //
  // The case we're looking for has this basic form:
  //
  //   %pMem = call i8* @malloc(i64 64)
  //   %pOffset = getelementptr i8, i8* %pMem, i64 32
  //   %pElement = bitcast i8* %pOffset to %struct.Elem
  //   %pStruct = bitcast i8* pMem to %struct.S
  //
  // where %struct.S is a structure that has an element of type %struct.Elem
  // at offset 32 (as determined by DataLayout).
  bool analyzeByteFlattenedGEPAccess(GEPOperator *GEP, LocalPointerInfo &Info) {
    Value *BasePointer = GEP->getPointerOperand();
    // The caller should have checked this.
    assert(BasePointer->getType() ==
           llvm::Type::getInt8PtrTy(GEP->getContext()));

    // Check for types that the base pointer is known to alias.
    LocalPointerInfo &BaseLPI = LocalMap[BasePointer];
    // If the incoming analysis was incomplete, what we do below won't be
    // complete, but the partial analysis may be necessary so we note the
    // incompleteness and continue.
    if (!BaseLPI.getAnalyzed()) {
      Info.setPartialAnalysis(true);
      DEBUG_WITH_TYPE(LPA_VERBOSE, dbgs() << "Incomplete analysis derived from "
                                          << *BasePointer << "\n");
    }

    // If we can't compute a constant offset, we won't be able to
    // figure out which element is being accessed.
    unsigned BitWidth = DL.getPointerSizeInBits();
    APInt APOffset(BitWidth, 0);
    if (!GEP->accumulateConstantOffset(DL, APOffset))
      return false;
    uint64_t Offset = APOffset.getLimitedValue();

    bool HasPtrToPtrAlias = false;
    for (auto *AliasTy : BaseLPI.getPointerTypeAliasSet()) {
      if (!AliasTy->isPointerTy())
        continue;
      // If this value aliases a pointer to a pointer, this isn't a
      // byte-flattened GEP after all. It's indexing the pointer-to-pointer
      // as a dynamic array.
      if (AliasTy->getPointerElementType()->isPointerTy()) {
        Info.addPointerTypeAlias(AliasTy);
        HasPtrToPtrAlias = true;
        continue;
      }
      if (!HasPtrToPtrAlias &&
          analyzePossibleOffsetAggregateAccess(
              GEP, AliasTy->getPointerElementType(), Offset, Info))
        return true;
    }
    if (HasPtrToPtrAlias)
      return true;
    // If none of the aliased types was a match, we can't identify any field
    // that the GEP is trying to access.
    return false;
  }

  bool analyzePossibleOffsetAggregateAccess(GEPOperator *GEP,
                                            llvm::Type *AggregateTy,
                                            uint64_t Offset,
                                            LocalPointerInfo &Info) {
    if (!AggregateTy->isAggregateType())
      return false;

    if (auto *StructTy = dyn_cast<StructType>(AggregateTy))
      return analyzePossibleOffsetStructureAccess(GEP, StructTy, Offset, Info);
    else
      return analyzePossibleOffsetArrayAccess(GEP, cast<ArrayType>(AggregateTy),
                                              Offset, Info);
  }

  bool analyzePossibleOffsetStructureAccess(GEPOperator *GEP,
                                            llvm::StructType *StructTy,
                                            uint64_t Offset,
                                            LocalPointerInfo &Info) {
    auto *SL = DL.getStructLayout(StructTy);
    // If the offset is as large or larger than the structure,
    // this isn't a match.
    if (Offset >= SL->getSizeInBytes())
      return false;
    // See which element in the structure would contain this offset.
    unsigned IdxAtOffset = SL->getElementContainingOffset(Offset);

    // If the containing element is not at that offset, this is not a match.
    uint64_t ElementOffset = SL->getElementOffset(IdxAtOffset);
    if (ElementOffset != Offset) {
      // If the element at that offset is a struct, we may be accessing
      // an element within the nested aggregate type.
      return analyzePossibleOffsetAggregateAccess(
          GEP, StructTy->getElementType(IdxAtOffset), Offset - ElementOffset,
          Info);
    }

    // Otherwise, this is a match.
    // Save the element address usage in the returned value's
    // local pointer info.
    Info.addElementPointee(StructTy, IdxAtOffset);

    // If this element is a type of interest, mark that as an expected
    // type alias for this pointer.
    llvm::Type *ElementTy = StructTy->getElementType(IdxAtOffset);
    Info.addPointerTypeAlias(ElementTy->getPointerTo());
    return true;
  }

  bool analyzePossibleOffsetArrayAccess(GEPOperator *GEP,
                                        llvm::ArrayType *ArrayTy,
                                        uint64_t Offset,
                                        LocalPointerInfo &Info) {
    llvm::Type *ElemTy = ArrayTy->getElementType();
    uint64_t ElementSize = DL.getTypeAllocSize(ElemTy);
    uint64_t NewOffset = Offset % ElementSize;
    if (NewOffset == 0) {
      // The offset is an exact multiple of the element size. This
      // is a match for the element access.
      Info.addElementPointee(ArrayTy, Offset / ElementSize);

      // If this element is a type of interest, mark that as an expected
      // type alias for this pointer.
      Info.addPointerTypeAlias(ElemTy->getPointerTo());
      return true;
    }

    // Otherwise, we may be accessing a sub-element within a nested aggregate.
    return analyzePossibleOffsetAggregateAccess(GEP, ElemTy, NewOffset, Info);
  }

  void analyzeLoadInstruction(LoadInst *Load, LocalPointerInfo &Info) {
    // If the pointer operand aliases any pointers-to-pointers, the loaded
    // value will be considered to alias to the pointed-to pointer type.
    Value *Src = Load->getPointerOperand();
    // We should have at least attempted to analyze this value already.
    // If not, there is a problem in populateDependencyStack().
    assert(LocalMap.count(Src) && "Load pointer operand missing from map!");
    LocalPointerInfo &SrcLPI = LocalMap[Src];
    for (auto *AliasTy : SrcLPI.getPointerTypeAliasSet())
      if (AliasTy->isPointerTy() &&
          AliasTy->getPointerElementType()->isPointerTy())
        Info.addPointerTypeAlias(AliasTy->getPointerElementType());
  }

  bool isDerivedValue(Value *V) {
    // TODO: Consider whether it will be necessary to handle llvm::MemoryAccess.

    // These value types transform other values into a new temporary.
    // GetElementPtr isn't in this list because the pointer it returns is
    // referring to a different logical object (a field) than the input
    // value, even though it points to the same block of memory.
    // The GetElementPtr case will be handled elsewhere.
    if (isa<CastInst>(V) || isa<PHINode>(V) || isa<SelectInst>(V) ||
        isa<BitCastOperator>(V) || isa<PtrToIntOperator>(V))
      return true;

    // This assert is here to catch cases that I haven't thought about.
    assert(isa<GlobalVariable>(V) || isa<Argument>(V) || isa<AllocaInst>(V) ||
           isa<LoadInst>(V) || isa<CallInst>(V) || isa<GetElementPtrInst>(V) ||
           isa<Constant>(V) || isa<GEPOperator>(V));

    return false;
  }

  // Find any type to which the return value of an allocation call will be
  // bitcast and, unless it looks like an element zero access, add that type
  // as an alias of the allocated pointer.
  void analyzeAllocationCallAliases(CallInst *CI, LocalPointerInfo &Info) {
    LLVM_DEBUG(dbgs() << "dtrans: Analyzing allocation call.\n  " << *CI
                      << "\n");
    SmallPtrSet<llvm::PointerType *, 4> CastTypes;
    SmallPtrSet<Value *, 4> VisitedUsers;
    bool IsPartial = false;
    collectAllocatedPtrBitcasts(CI, CastTypes, VisitedUsers, IsPartial);
    if (IsPartial)
      Info.setPartialAnalysis(true);
    // Eliminate casts that access element zero in other known types.
    // This is an N^2 algorithm, but N will generally be very small.
    if (CastTypes.size() > 1) {
      SmallPtrSet<llvm::PointerType *, 4> TypesToRemove;
      for (auto *Ty1 : CastTypes) {
        if (!Ty1->getPointerElementType()->isAggregateType())
          continue;
        for (auto *Ty2 : CastTypes)
          if (dtrans::isElementZeroAccess(Ty1, Ty2))
            TypesToRemove.insert(Ty2);
      }
      for (auto *Ty : TypesToRemove)
        CastTypes.erase(Ty);
    }

    for (auto *Ty : CastTypes)
      Info.addPointerTypeAlias(Ty);
  }

  // To identify the type of allocated memory, we look for bitcast users of
  // the returned value. If one of the users is a PHI node or a select
  // instruction, we need to also look at the users of that instruction to
  // handle cases like this:
  //
  //  entry:
  //    %origS = bitcast %struct.S* %p to i8*
  //    %isNull = icmp eq %struct.S* %p, null
  //    br i1 %flag, label %new, label %end
  //  new:
  //    %newS = call i8* @malloc(i64 16)
  //    br label %end
  //  end:
  //    %tmp = phi i8* [%origS, %entry], [%newS, %new]
  //    %val = bitcast i8* tmp to %struct.S*
  //    ...
  //
  // In such a case, this routine will recursively call itself.
  void collectAllocatedPtrBitcasts(
      Instruction *I, SmallPtrSetImpl<llvm::PointerType *> &CastTypes,
      SmallPtrSetImpl<Value *> &VisitedUsers, bool &IsPartial) {
    for (auto *U : I->users()) {
      // If we've already visited this user, don't visit again.
      // This prevents infinite loops as we follow the sub-users of PHI nodes
      // and select instructions.
      if (!VisitedUsers.insert(U).second)
        continue;
      // If the user is a bitcast, that's what we're looking for.
      if (auto *BI = dyn_cast<BitCastInst>(U)) {
        // This must be a cast to another pointer type. Otherwise, the cast
        // would be done with the PtrToInt instruction.
        auto PtrTy = cast<PointerType>(BI->getType());

        LLVM_DEBUG(dbgs() << "  Associated bitcast: " << *BI << "\n");

        // Save the type information.
        CastTypes.insert(PtrTy);

        // Follow the uses of the bitcast as it may be stored in a location
        // from which we can infer more information.
        collectAllocatedPtrBitcasts(BI, CastTypes, VisitedUsers, IsPartial);
        continue;
      }
      // If the user is a PHI node or a select instruction, we need to follow
      // the users of that instruction.
      if (isa<PHINode>(U) || isa<SelectInst>(U))
        collectAllocatedPtrBitcasts(cast<Instruction>(U), CastTypes,
                                    VisitedUsers, IsPartial);

      // If the user is a store instruction, treat the alias types of the
      // destination pointer as implicit casts.
      if (auto *Store = dyn_cast<StoreInst>(U))
        inferAllocatedTypesFromStoreInst(I, Store, CastTypes, IsPartial);
    }
  }

  // Sometime an allocated pointer will be directly stored to a memory
  // location. In such a case, the type of memory that was allocated can be
  // inferred from the type of the destination pointer. A typical example
  // might look like this:
  //
  //   %dest = bitcast %struct.A** %val to %i8**
  //   %p = call i8* @malloc(i64 64)
  //   store i8* p, i8** %dest
  //
  // In this case, we can infer that the memory allocated is a %struct.A
  // object because its pointer (effectively %struct.A*) is stored in a
  // location that aliases to %struct.A**. This often happens when an
  // allocated pointer is stored in a structure field.
  //
  // There are also cases where we need to infer the allocated type from
  // what is stored there. For example:
  //
  //   %t1 = call i8* @malloc(i64 8)
  //   %t2 = call i8* @calloc(i64 %N, i64 8)
  //   %t3 = bitcast i8* %t1 to i8**
  //   %t4 = bitcast i8* %t2 to %struct.A**
  //   store i8* %t2, i8** %t3
  //
  // In this case, because we see a %struct.A** value being stored to the
  // memory allocated at %t1 we can infer that the %t1 and %t3 values must
  // alias to %struct.A***.
  void
  inferAllocatedTypesFromStoreInst(Instruction *ValOfInterest, StoreInst *Store,
                                   SmallPtrSetImpl<llvm::PointerType *> &Types,
                                   bool &IsPartial) {
    // Get the local pointer info for the destination address.
    Value *DestPtr = Store->getPointerOperand();
    Value *StoredVal = Store->getValueOperand();

    // If the value of interest is the value being stored, we infer the type
    // from the aliases of the pointer value where it is being stored.
    if (ValOfInterest == StoredVal) {
      // For each type aliased by the destination, if the type is a pointer
      // add the type that it points to to the Types set.
      LocalPointerInfo &DestInfo = LocalMap[DestPtr];
      if (!DestInfo.getAnalyzed())
        IsPartial = true;
      for (auto *AliasTy : DestInfo.getPointerTypeAliasSet())
        if (AliasTy->isPointerTy() &&
            AliasTy->getPointerElementType()->isPointerTy())
          Types.insert(cast<PointerType>(AliasTy->getPointerElementType()));
      return;
    }

    // Otherwise, the value of interest must be the destination pointer and
    // we must infer its type from what is being stored there.
    assert(ValOfInterest == DestPtr &&
           "Can't infer type from unused value of interest.");
    // For each type aliased by the stored value, add a pointer to that type
    // to the Types set for the destination.
    LocalPointerInfo StoredLPI = LocalMap[StoredVal];
    if (!StoredLPI.getAnalyzed())
      IsPartial = true;
    for (auto *AliasTy : StoredLPI.getPointerTypeAliasSet())
      Types.insert(AliasTy->getPointerTo());
  }

  // We need to handle i8* function arguments kind of backward.
  // Because we don't know within the function what aliases the
  // value passed in might have had, we look at the way the
  // argument is used in the function and infer from that what
  // its aliases need to be. When the function is called, we will
  // compare the actual alias set of the values passed to the
  // function against the inferred alias set and report any
  // mismatches there.
  void analyzePossibleVoidPtrArgument(Value *V, LocalPointerInfo &Info) {
    DEBUG_WITH_TYPE(LPA_VERBOSE,
                    dbgs() << "dtrans: Analyzing function argument.\n");
    SmallPtrSet<llvm::PointerType *, 4> CastTypes;
    SmallPtrSet<Value *, 4> VisitedUsers;
    inferAliasedTypesFromUses(V, CastTypes, VisitedUsers);
    // Eliminate casts that access element zero in other known types.
    // This is an N^2 algorithm, but N will generally be very small.
    if (CastTypes.size() > 1) {
      SmallPtrSet<llvm::PointerType *, 4> TypesToRemove;
      for (auto *Ty1 : CastTypes) {
        if (!Ty1->getPointerElementType()->isAggregateType())
          continue;
        for (auto *Ty2 : CastTypes)
          if (dtrans::isElementZeroAccess(Ty1, Ty2))
            TypesToRemove.insert(Ty2);
      }
      for (auto *Ty : TypesToRemove)
        CastTypes.erase(Ty);
    }

    for (auto *Ty : CastTypes)
      Info.addPointerTypeAlias(Ty);
    Info.setAnalyzed();
  }

  // FIXME: This is doing nearly the same thing as collectAllocatedPtrBitcasts.
  //        Eventually they should be merged. Right now, we need to tolerate
  //        the duplication for the sake of meeting schedule constraints.
  // The notable differences between this function and
  // collectAllocatedPtrBitcasts are that this function does not attempt to
  // infer alias information from loads and stores (which would require
  // building a dependency stack, and unlike collectAllocatedPtrBitcasts this
  // function does attempt to infer alias types from called functions.
  void inferAliasedTypesFromUses(Value *V,
                                 SmallPtrSetImpl<PointerType *> &CastTypes,
                                 SmallPtrSetImpl<Value *> &VisitedUsers) {
    for (auto *U : V->users()) {
      // If we've already visited this user, don't visit again.
      // This prevents infinite loops as we follow the sub-users of PHI nodes
      // and select instructions.
      if (!VisitedUsers.insert(U).second)
        continue;
      // If the user is a cast, that's what we're looking for.
      if (auto *Cast = dyn_cast<CastInst>(U)) {
        if (isPartialPtrBitCast(U)) {
          LLVM_DEBUG(dbgs() << "Found partial pointer bitcast: " << *U << "\n");
          continue;
        }
        // We want to follow the uses through PointerToInt casts, but they
        // don't tell us anything about aliases. The dyn_cast above catches
        // PtrToInt, IntToPtr, and BitCast. If the result is a pointer
        // type, we want to add it to the alias set.
        if (auto *PtrTy = dyn_cast<PointerType>(Cast->getType())) {
          DEBUG_WITH_TYPE(LPA_VERBOSE,
                          dbgs() << "  Argument cast: " << *Cast << "\n");
          // Save the type information.
          CastTypes.insert(PtrTy);
        }

        // Follow the uses of the cast to see what other types are used.
        inferAliasedTypesFromUses(Cast, CastTypes, VisitedUsers);
        continue;
      }
      // If the user is a PHI node or a select instruction, we need to follow
      // the users of that instruction.
      if (isa<PHINode>(U) || isa<SelectInst>(U)) {
        inferAliasedTypesFromUses(U, CastTypes, VisitedUsers);
        continue;
      }
      // If the user is a call instruction and the value we're currently
      // following is an i8* value, follow the uses of the argument within
      // the called function. (Note: while it may seem counter-intuitive
      // to be able to cross function boundaries like this, the VisitedUsers
      // set will continue to work as expected.)
      if (auto *CI = dyn_cast<CallInst>(U)) {
        if (isValueInt8PtrType(V)) {
          DEBUG_WITH_TYPE(LPA_VERBOSE,
                          dbgs() << "Analyzing use in call instruction: " << *CI
                                 << "\n");
          // Check all the arguments of the call as our value may be used more
          // than once.
          unsigned NumArgs = CI->getNumArgOperands();
          if (Function *F = CI->getCalledFunction()) {
            for (unsigned ArgNo = 0; ArgNo < NumArgs; ++ArgNo) {
              if (CI->getArgOperand(ArgNo) == V) {
                DEBUG_WITH_TYPE(LPA_VERBOSE,
                                dbgs()
                                    << "Analyzing function argument: "
                                    << F->getName() << " @ " << ArgNo << "\n");
                Argument *Arg = F->arg_begin();
                std::advance(Arg, ArgNo);
                inferAliasedTypesFromUses(Arg, CastTypes, VisitedUsers);
              }
            }
          } else {
            // This happens with an indirect function call or a call to
            // a function that has been bitcast to a different type.
            Value *Callee = CI->getCalledValue();
            if (auto *Cast = dyn_cast<BitCastOperator>(Callee)) {
              F = dyn_cast<Function>(Cast->getOperand(0));
              for (unsigned ArgNo = 0; ArgNo < NumArgs; ++ArgNo) {
                if (CI->getArgOperand(ArgNo) == V) {
                  DEBUG_WITH_TYPE(LPA_VERBOSE,
                                  dbgs() << "Analyzing function argument: "
                                         << F->getName() << " @ " << ArgNo
                                         << "\n");
                  Argument *Arg = F->arg_begin();
                  std::advance(Arg, ArgNo);
                  // If the argument is still an i8* after the function bitcast
                  // follow its uses. Otherwise, infer the type directly from
                  // the argument type.
                  if (isValueInt8PtrType(Arg))
                    inferAliasedTypesFromUses(Arg, CastTypes, VisitedUsers);
                  else if (auto *ArgPtrTy =
                               dyn_cast<PointerType>(Arg->getType()))
                    CastTypes.insert(ArgPtrTy);
                }
              }
            } else {
              DEBUG_WITH_TYPE(LPA_VERBOSE,
                              dbgs() << "Unable to get called function!\n");
            }
          }
        }
        continue;
      }
    }
  }
};

class DTransInstVisitor : public InstVisitor<DTransInstVisitor> {
public:
  DTransInstVisitor(LLVMContext &Context, DTransAnalysisInfo &Info,
                    const DataLayout &DL, const TargetLibraryInfo &TLI,
                    DTransAllocAnalyzer &DTAA)
      : DTInfo(Info), DL(DL), TLI(TLI), LPA(DL, TLI, DTAA), DTAA(DTAA) {
    // Save pointers to some commonly referenced types.
    Int8PtrTy = llvm::Type::getInt8PtrTy(Context);
    PtrSizeIntTy = llvm::Type::getIntNTy(Context, DL.getPointerSizeInBits());
    PtrSizeIntPtrTy =
        llvm::Type::getIntNPtrTy(Context, DL.getPointerSizeInBits());
  }

  void visitIntrinsicInst(IntrinsicInst &I) {
    Intrinsic::ID Intrin = I.getIntrinsicID();
    switch (Intrin) {
    default:
      break;

    // The following intrinsics do not affect the safety checks of
    // the DTrans analysis for any of their arguments.
    case Intrinsic::dbg_declare:
    case Intrinsic::dbg_value:
    case Intrinsic::lifetime_end:
    case Intrinsic::lifetime_start:
      return;

    case Intrinsic::memset:
      analyzeMemset(I);
      return;

    case Intrinsic::memcpy:
      analyzeMemcpyOrMemmove(I, dtrans::MemfuncCallInfo::MK_Memcpy);
      return;
    case Intrinsic::memmove:
      analyzeMemcpyOrMemmove(I, dtrans::MemfuncCallInfo::MK_Memmove);
      return;
    }

    // The intrinsic was not handled, mark all parameters as unhandled uses.
    for (Value *Arg : I.arg_operands()) {
      if (isValueOfInterest(Arg)) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Unhandled use --  IntrinsicInst: "
                          << I << " value passed as argument.\n"
                          << "  " << *Arg << "\n");
        setValueTypeInfoSafetyData(Arg, dtrans::UnhandledUse);
      }
    }
  }

  // See typesMayBeCRuleCompatible() immediately below for explanation of
  // this function.
  static bool typesMayBeCRuleCompatibleX(llvm::Type *T1, llvm::Type *T2,
                                         SmallPtrSetImpl<llvm::Type *> &Tstack) {

    // Enum indicating that on the particular predicate being compared for
    // T1 and T2, the types have the opposite value of the predicate
    // (TME_OPPOSITE), the types both have the predicate (TME_YES), or
    // neither type has the predicate (TME_YES).
    enum TME { TME_OPPOSITE, TME_YES, TME_NO };

    // Lambda to match the result of testing the same predicate for T1 and T2
    auto boolT = [](bool B1, bool B2) -> TME {
      return (B1 && B2) ? TME_YES : (!B1 && !B2) ? TME_NO : TME_OPPOSITE;
    };

    // Typedef for const pointer to member function which returns a bool
    typedef bool (llvm::Type::*MFP)() const;
    // Lambda to compare an predicate indicated by Fp for T1 and T2
    auto typeTest = [&boolT](llvm::Type *T1, llvm::Type *T2, MFP Fp) -> TME {
      return boolT((T1->*Fp)(), (T2->*Fp)());
    };

    // An array of predicate conditions for which T1 and T2 will be tested
    // for compatibility. The predicate "isIntegerTy" and "isFloatingPointTy"
    // are base properties that cannot be further refined.  Testing for the
    // predicate "isFunctionTy" can be refined if we want to sharpen the
    // results of typesMayBeCRuleCompatible().
    MFP F1Array[] = {&llvm::Type::isIntegerTy, &llvm::Type::isFloatingPointTy,
                     &llvm::Type::isFunctionTy};

    // A Type is always compatible with itself.
    if (T1 == T2)
      return true;

    // Test some fundamental and complicated predicates. Return false if they
    // don't match, true if they do.
    for (auto Fx : F1Array) {
      TME R = typeTest(T1, T2, Fx);
      if (R == TME_OPPOSITE)
        return false;
      if (R == TME_YES)
        return true;
    }

    // Two pointer Types are compatible if their element types are compatible.
    TME R = typeTest(T1, T2, &llvm::Type::isPointerTy);
    if (R == TME_OPPOSITE)
      return false;
    if (R == TME_YES) {
      //
      // Pointers to StructTypes are a tricky case, as the definition could
      // be recursive.  This could be resolved by struct TAGs, but these are
      // not structly preserved in LLVM. We could try to derive them from
      // the StructType's name, but in LLVM even anonymous types get a name.
      // (albeit a recognizable one because it is either %struct.anon or
      // of the form %struct.anon.*).
      //
      // So we keep a stack of pointers to Types that we have already seen,
      // and give up whenever we encounter one again. This is conservative
      // and could be improved.
      //
      auto *T3 = T1->getPointerElementType();
      auto *T4 = T2->getPointerElementType();
      TME S = typeTest(T3, T4, &llvm::Type::isStructTy);
      if (S == TME_OPPOSITE)
        return false;
      if (S == TME_YES) {
        if (T3->getStructNumElements() != T4->getStructNumElements())
          return false;
        if (Tstack.find(T3) != Tstack.end())
          return true;
        if (Tstack.find(T4) != Tstack.end())
          return true;
        Tstack.insert(T3);
        Tstack.insert(T4);
      }
      return typesMayBeCRuleCompatibleX(T1->getPointerElementType(),
                                        T2->getPointerElementType(), Tstack);
    }

    // Two array Types are compatible if they have the same number of elements
    // and their element types are compatible.
    R = typeTest(T1, T2, &llvm::Type::isArrayTy);
    if (R == TME_OPPOSITE)
      return false;
    if (R == TME_YES) {
      if (T1->getArrayNumElements() != T2->getArrayNumElements())
        return false;
      return typesMayBeCRuleCompatibleX(T1->getArrayElementType(),
                                        T2->getArrayElementType(), Tstack);
    }

    // Two struct Types are compatible if they have the same number of
    // elements, and corresponding elements are compatible with one another.
    R = typeTest(T1, T2, &llvm::Type::isStructTy);
    if (R == TME_OPPOSITE)
      return false;
    if (R == TME_YES) {
      if (T1->getStructNumElements() != T2->getStructNumElements())
        return false;
      for (unsigned I = 0; I < T1->getStructNumElements(); ++I)
        if (!typesMayBeCRuleCompatibleX(T1->getStructElementType(I),
                                        T2->getStructElementType(I), Tstack))
          return false;
      return true;
    }

    // None of the rules cited above were able to determine the Types are
    // not compatible with one another. Conservatively assume they are.
    return true;
  }

  // Return true if Types T1 and T2 may be compatible by C language rules.
  // The full C language rules for Type compatibility are not implemented
  // here.  This is a conservative test.  It will return true in some cases
  // where the T1 and T2 are not compatible. Here are some examples:
  //   (1) When the names of structure fields do not match. Since the LLVM
  //       IR does not include info about the structure field names, this
  //       function must return a conservative result here.
  //   (2) When the structure tags do not match. (See the comment in the
  //       code for typesMayBeCRuleCompatibleX.) In the absence of completely
  //       reliable structure tags, we keep a stack (Tstack) of Types that
  //       we have already seen to avoid recursion.
  //   (3) Function types. This could be implemented in the future. We are
  //       not doing it now because there is no immediate need.
  static bool typesMayBeCRuleCompatible(llvm::Type *T1, llvm::Type *T2) {
    SmallPtrSet<llvm::Type *, 4> Tstack;
    return typesMayBeCRuleCompatibleX(T1, T2, Tstack);
  }

  // Return true if the Type T may have a distinct compatible Type by
  // C language rules. Before this function is executed, we must ensure
  // that all Types against which we test it have TypeInfos created for
  // them.  This is done in analyzeModule().
  bool mayHaveDistinctCompatibleCType(llvm::Type *T) {
    if (!DTInfo.isTypeOfInterest(T))
      return true;
    dtrans::TypeInfo *TIN = DTInfo.getOrCreateTypeInfo(T);
    switch (TIN->getCRuleTypeKind()) {
    case dtrans::CRT_False:
      return false;
    case dtrans::CRT_True:
      return true;
    case dtrans::CRT_Unknown:
      for (auto *TI : DTInfo.type_info_entries()) {
        llvm::Type *U = TI->getLLVMType();
        if (U != T && typesMayBeCRuleCompatible(T, U)) {
          LLVM_DEBUG(dbgs()
                     << "dtrans-crule-compat: " << *T << " (X) " << *U << "\n");
          TIN->setCRuleTypeKind(dtrans::CRT_True);
          return true;
        }
      }
      TIN->setCRuleTypeKind(dtrans::CRT_False);
      LLVM_DEBUG(dbgs() << "dtrans-crule-nocompat: " << *T << "\n");
      return false;
    }
    return true;
  }

  void visitCallInst(CallInst &CI) {
    auto *CV = CI.getCalledValue();
    Function *F = dyn_cast<Function>(CV);

    // If the Function wasn't found, then there is a possibility
    // that it is inside a BitCast. In this case we need
    // to strip the pointer casting from the Value and then
    // access the Function.
    if (!F)
      F = dyn_cast<Function>(CI.getCalledValue()->stripPointerCasts());

    // If the called function is a known allocation function, we need to
    // analyze the allocation.
    dtrans::AllocKind Kind = dtrans::getAllocFnKind(F, TLI);
    if (Kind == dtrans::AK_NotAlloc && DTAA.isMallocPostDom(F))
      Kind = dtrans::AK_UserMalloc;
    if (Kind != dtrans::AK_NotAlloc) {
      analyzeAllocationCall(CI, Kind);
      return;
    }

    // If this is a call to the "free" lib function,  the call is safe, but
    // we analyze the instruction for the purpose of capturing the argument
    // TypeInfo, which will be needed by some of the transformations when
    // rewriting allocations and frees.
    if (dtrans::isFreeFn(F, TLI)) {
      analyzeFreeCall(CI, dtrans::FK_Free);
      return;
    }

    if (DTAA.isFreePostDom(F)) {
      analyzeFreeCall(CI, dtrans::FK_UserFree);
      return;
    }

    // For all other calls, if a pointer to an aggregate type is passed as an
    // argument to a function in a form other than its dominant type, the
    // address has escaped. Also, if a pointer to a field is passed as an
    // argument to a function, the address of the field has escaped.
    // FIXME: Try to resolve indirect calls.
    bool IsFnLocal = F ? !F->isDeclaration() : false;
    unsigned NextArgNo = 0;
    for (Value *Arg : CI.arg_operands()) {
      // Keep track of the argument index we're working with.
      unsigned ArgNo = NextArgNo++;

      if (!isValueOfInterest(Arg))
        continue;

      LocalPointerInfo &LPI = LPA.getLocalPointerInfo(Arg);

      if (LPI.pointsToSomeElement()) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Field address taken -- "
                          << "pointer to element passed as argument:\n"
                          << "  " << CI << "\n  " << *Arg << "\n");
        // Selects and PHIs may have created a pointer that refers to
        // elements in multiple aggregate types. This sets the field
        // address taken condition for them all.
        for (auto &PointeePair : LPI.getElementPointeeSet()) {
          setBaseTypeInfoSafetyData(PointeePair.first,
                                    dtrans::FieldAddressTaken);
          dtrans::TypeInfo *ParentTI =
              DTInfo.getOrCreateTypeInfo(PointeePair.first);
          if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(ParentTI))
            ParentStInfo->getField(PointeePair.second).setAddressTaken();
        }
      }

      // If the argument aliases an aggregate pointer type that is not the type
      // of the argument, the address has been taken in a way we can't track.
      // If the called function is locally defined and the type of the argument
      // matches the aggregate pointer type, we will be able to analyze the use
      // when we visit that function. If more than one aggregate type is
      // aliased, we will have flagged the overloaded pointer when we visited
      // the select or PHI that created this situation, so here we just need to
      // worry about the types individually. However, if the function being
      // called is varadic and the argument is not one of the fixed parameters
      // we can't check its use from here.
      auto *ArgTy = Arg->getType();
      if (IsFnLocal && (ArgTy == Int8PtrTy) &&
          (!F->isVarArg() || (ArgNo < F->getFunctionType()->getNumParams()))) {
        // If we're calling a local function that takes an i8* operand
        // get the expected alias types from the local pointer analyzer.
        auto Param = F->arg_begin();
        std::advance(Param, ArgNo);
        LocalPointerInfo &ParamLPI = LPA.getLocalPointerInfo(Param);

        if (isAliasSetOverloaded(LPI.getPointerTypeAliasSet(),
                                 /*AllowElementZeroAccess=*/true) ||
            isAliasSetOverloaded(ParamLPI.getPointerTypeAliasSet(),
                                 /*AllowElementZeroAccess=*/true)) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Mismatched argument use -- "
                            << "overloaded alias set found at call site:\n"
                            << "  " << CI << "\n  " << *Arg << "\n");
          setAllAliasedTypeSafetyData(LPI, dtrans::MismatchedArgUse);
          setAllAliasedTypeSafetyData(ParamLPI, dtrans::MismatchedArgUse);
        } else {
          llvm::Type *DomParamTy = ParamLPI.getDominantAggregateTy();
          llvm::Type *DomArgTy = LPI.getDominantAggregateTy();
          if (DomParamTy != DomArgTy) {
            LLVM_DEBUG(dbgs() << "dtrans-safety: Mismatched argument use -- "
                              << "overloaded alias set found at call site:\n"
                              << "  " << CI << "\n  " << *Arg << "\n");
            setAllAliasedTypeSafetyData(LPI, dtrans::MismatchedArgUse);
            setAllAliasedTypeSafetyData(ParamLPI, dtrans::MismatchedArgUse);
          }
        }
      } else {
        for (auto *AliasTy : LPI.getPointerTypeAliasSet()) {
          if (!DTInfo.isTypeOfInterest(AliasTy))
            continue;
          if (IsFnLocal && (AliasTy == ArgTy))
            continue;
          // For indirect calls, Use the C language rule, if appropriate, to
          // reject cases for which the Type of the formal and actual argument
          // must match.  In such cases, there will be no "Address taken"
          //  safety violation.
          if (!F && DTransUseCRuleCompat &&
              !mayHaveDistinctCompatibleCType(AliasTy))
            continue;
          // If the first element of the dominant type of the pointer is an
          // an array of the actual argument, don't report address taken.
          if (isElementZeroArrayOfType(AliasTy, ArgTy)) {
            LLVM_DEBUG(dbgs() << "dtrans-safety: Field address taken -- "
                              << "ptr to array element passed as argument:\n"
                              << "  " << CI << "\n  " << *Arg << "\n");
            setBaseTypeInfoSafetyData(AliasTy, dtrans::FieldAddressTaken);
            dtrans::TypeInfo *ParentTI = DTInfo.getOrCreateTypeInfo(AliasTy);
            if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(ParentTI))
              ParentStInfo->getField(0).setAddressTaken();
            continue;
          }
          LLVM_DEBUG(dbgs() << "dtrans-safety: Address taken -- "
                            << "pointer to aggregate passed as argument:\n"
                            << "  " << CI << "\n  " << *Arg << "\n");
          setBaseTypeInfoSafetyData(AliasTy, dtrans::AddressTaken);
        }
      }
    }
  }

  void visitBitCastInst(BitCastInst &I) {
    llvm::Type *DestTy = I.getDestTy();
    llvm::Value *SrcVal = I.getOperand(0);

    // If the source operand is not a value of interest, we only need to
    // consider the destination type.
    if (!isValueOfInterest(SrcVal)) {
      // If the destination type is a type of interest, then the cast is
      // unsafe since we don't know anything about the source value.
      if (DTInfo.isTypeOfInterest(DestTy)) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
                          << "unknown pointer cast to type of interest:\n"
                          << "  " << I << "\n");
        setValueTypeInfoSafetyData(&I, dtrans::BadCasting);
      }
      return;
    }

    // If we get here, the source operand is a value of interest.

    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(SrcVal);

    // If the source points to multiple incompatible types, mark the cast
    // as unsafe for all aliased types.
    if (isAliasSetOverloaded(LPI.getPointerTypeAliasSet(),
                             /*AllowElementZeroAccess=*/true)) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
                        << "cast of ambiguous pointer:\n"
                        << "  " << I << "\n");
      setValueTypeInfoSafetyData(&I, dtrans::BadCasting);
      setValueTypeInfoSafetyData(SrcVal, dtrans::BadCasting);
      return;
    }

    // Get the dominant aggregate type to which the source operand points.
    // Usually the value will only alias to one aggregate type. However, if
    // the first element in an aggregate type is a nested type, the value
    // may also alias to a pointer to that type. In that case, the parent
    // type will be returned as the dominant type. The case of multiple
    // incompatible types was checked for above.
    llvm::Type *DomTy = LPI.getDominantAggregateTy();
    if (!DomTy) {
      // If a dominant aggregate type was not identified, this must be a
      // pointer to an element within some other aggregate type.
      auto ElementPointees = LPI.getElementPointeeSet();
      assert(ElementPointees.size() > 0);
      // In the case of multiple nested element zero types, there may be
      // more than one safe element pointee.
      for (auto &PointeePair : ElementPointees) {
        dtrans::TypeInfo *ParentTI =
            DTInfo.getOrCreateTypeInfo(PointeePair.first);
        llvm::Type *ElemTy;
        if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(ParentTI)) {
          assert(PointeePair.second < ParentStInfo->getNumFields());
          dtrans::FieldInfo &FI = ParentStInfo->getField(PointeePair.second);
          ElemTy = FI.getLLVMType();
        } else {
          // We only store info for structs and arrays, so this must be
          // an array. The cast asserts that.
          auto *ParentArrayInfo = cast<dtrans::ArrayInfo>(ParentTI);
          ElemTy = ParentArrayInfo->getElementLLVMType();
        }
        verifyBitCastSafety(I, ElemTy->getPointerTo(), DestTy);
      }
      return;
    }

    // If this is the start of a partial pointer load/store idiom, don't
    // report it as a safety violation.
    LocalPointerInfo &SelfLPI = LPA.getLocalPointerInfo(&I);
    if (SelfLPI.isPartialPtrLoadStore())
      return;

    // If we get here, we have identified a single dominant type to which
    // the source operand points. We call a helper function to handle this
    // case because the details are the same as the casting of a pointer
    // to an element.
    verifyBitCastSafety(I, DomTy, DestTy);
  }

  void visitLoadInst(LoadInst &I) {
    // Load instructions that read individual fields from a struct or array
    // are handled by following the address obtained via GetElementPtr.
    // Loads of a pointer to a pointer (which is generally a field access)
    // are also safe.
    //
    // What we are looking for here are loads which read a pointer to an
    // aggregate type from memory using a pointer to the pointer or loads which
    // read an element from an aggregate type.

    // The load instruction looks like this:
    //
    //   <val> = load <ty>, <ty*> <op>
    //
    // Where '<op>' is the Value representing the address from which to load
    // '<ty*>' is the type of the '<op>' value (always a pointer type) and
    // '<ty>' is the type of the value loaded (always the element type of the
    // operand pointer type.
    Value *Ptr = I.getPointerOperand();
    if (!isValueOfInterest(Ptr))
      return;

    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(Ptr);
    if (LPI.pointsToSomeElement())
      analyzeElementLoadOrStore(LPI, nullptr, I, I.getType(), I.isVolatile(),
                                /*IsLoad=*/true);
    else {
      // If the source pointer isn't a pointer to an element in an aggregate
      // type, we need to check to see if the source value is a pointer to
      // an aggregate type or a pointer to a pointer. If it is a pointer to a
      // pointer, the load is safe. If it is a pointer to an aggregate then it
      // is either a load of the entire structure or a mismatched load of
      // element zero. For very small structures (for instance a 64-bit
      // structure) the entire structure may be loaded as an integer value.
      bool SourceIsPtrToPtr = false;
      bool BaseTypeFound = false;
      for (auto *AliasTy : LPI.getPointerTypeAliasSet()) {
        // If the alias isn't a type of interest (typically i8*) we don't
        // need to check anything else.
        if (!DTInfo.isTypeOfInterest(AliasTy))
          continue;
        if (!AliasTy->isPointerTy())
          continue;
        if (AliasTy->getPointerElementType()->isPointerTy()) {
          SourceIsPtrToPtr = true;
          continue;
        }
        // If we get here, we've found an alias which is a simple pointer
        // to an aggregate type. This means we're either loading the entire
        // structure, or we're loading a mismatch of element zero.
        BaseTypeFound = true;
        if (AliasTy == Ptr->getType()) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Whole structure reference:\n"
                            << "  " << I << "\n");
          setBaseTypeInfoSafetyData(AliasTy, dtrans::WholeStructureReference);
        } else {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Mismatched element access -- "
                            << "  bad type for element zero load:\n"
                            << "  " << I << "\n");
          setBaseTypeInfoSafetyData(AliasTy, dtrans::MismatchedElementAccess);
        }
      }
      // We expect that all cases will either find a pointer to a pointer alias
      // or will fall through to the base type handling. If neither of those
      // happen, we can't handle this load.
      if (!SourceIsPtrToPtr && !BaseTypeFound) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
                          << "  Unexpected pointer for load:"
                          << "  " << I << "\n");
        setValueTypeInfoSafetyData(&I, dtrans::UnhandledUse);
      }
    }
  }

  //
  // Analyze and report dtrans::UnsafePointerStore for the StoreInst with
  // the given ValOperand and PtrOperand. If I == nullptr, we are analyzing
  // an static initialization of a pointer.
  //
  void analyzeUnsafePointerStores(StoreInst *I, Value *ValOperand,
                                  Value *PtrOperand) {
    LocalPointerInfo &ValLPI = LPA.getLocalPointerInfo(ValOperand);
    LocalPointerInfo &PtrLPI = LPA.getLocalPointerInfo(PtrOperand);

    // If the value of interest either aliases to a pointer to a type of
    // interest or points to an element within a type of interest, check to
    // make sure the pointer operand is compatible.
    if (ValLPI.canAliasToAggregatePointer()) {
      for (auto *AliasTy : ValLPI.getPointerTypeAliasSet()) {
        if (AliasTy == Int8PtrTy)
          continue;
        if (!PtrLPI.canPointToType(AliasTy)) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Unsafe pointer store:\n");
          if (I != nullptr)
            LLVM_DEBUG(dbgs() << "  " << *I << "\n");
          else
            LLVM_DEBUG(dbgs()
                       << " " << *ValOperand << " -> " << *PtrOperand << " \n");
          setValueTypeInfoSafetyData(ValOperand, dtrans::UnsafePointerStore);
          setValueTypeInfoSafetyData(PtrOperand, dtrans::UnsafePointerStore);
        }
      }
    } else if (PtrLPI.canAliasToAggregatePointer()) {
      // If we get here the value operand is not a pointer to an aggregate
      // type, but the pointer operand is. Unless the value operand is a
      // null pointer, this is a bad store.
      if (!isa<ConstantPointerNull>(ValOperand) && !isPartialPtrStore(I)) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Unsafe pointer store:\n");
        if (I != nullptr)
          LLVM_DEBUG(dbgs() << "  " << *I << "\n");
        else
          LLVM_DEBUG(dbgs()
                     << " " << *ValOperand << " -> " << *PtrOperand << " \n");
        setValueTypeInfoSafetyData(PtrOperand, dtrans::UnsafePointerStore);
      }
    }
  }

  void visitStoreInst(StoreInst &I) {
    // Store instructions that write individual fields of a struct or array
    // are handled by following the address obtained via GetElementPtr.
    // Stores of a pointer to a pointer (which is generally a field write)
    // are also safe.
    //
    // What we are looking for here are stores which write an aggregate type
    // to memory using a pointer to the aggregate.

    // The store instruction looks like this:
    //
    //   store <ty> <val>, <ty*> <ptr>
    //
    // Where '<val>' is the Value being stored, '<ty>' is the type of that
    // value, '<ptr> is the Value representing the address at which the store
    // occurs, and '<ty*>' is the type of the '<ptr>' value (always a pointer
    // type with '<ty>' as its element type).
    if (!isValueOfInterest(I.getValueOperand()) &&
        !isValueOfInterest(I.getPointerOperand()))
      return;

    Value *ValOperand = I.getValueOperand();
    Value *PtrOperand = I.getPointerOperand();

    // This handles the unlikely, but legal, case where an instance of
    // a structure is stored directly.
    if (!ValOperand->getType()->isPointerTy() &&
        DTInfo.isTypeOfInterest(ValOperand->getType())) {
      // I think this is generally true for store operations.
      assert(PtrOperand->getType() == ValOperand->getType()->getPointerTo());
      LLVM_DEBUG(dbgs() << "dtrans-safety: Whole structure reference:\n"
                        << "  " << I << "\n");
      setBaseTypeInfoSafetyData(ValOperand->getType(),
                                dtrans::WholeStructureReference);
      return;
    }

    analyzeUnsafePointerStores(&I, ValOperand, PtrOperand);

    // If the value operand is a pointer to an element within an aggregate
    // we need to mark that element as having its address taken.
    LocalPointerInfo &ValLPI = LPA.getLocalPointerInfo(ValOperand);
    if (ValLPI.pointsToSomeElement()) {
      auto ValPointees = ValLPI.getElementPointeeSet();
      for (auto PointeePair : ValPointees) {
        dtrans::TypeInfo *ParentTI =
            DTInfo.getOrCreateTypeInfo(PointeePair.first);
        LLVM_DEBUG(dbgs() << "dtrans-safety: Field address taken:\n"
                          << "  " << *(ParentTI->getLLVMType()) << " @ "
                          << PointeePair.second << "\n"
                          << "  " << I << "\n");
        ParentTI->setSafetyData(dtrans::FieldAddressTaken);
        if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(ParentTI))
          ParentStInfo->getField(PointeePair.second).setAddressTaken();
      }
    }

    // If the pointer operand is a pointer to an element within an aggregate
    // we need to mark that element as having been written and make sure the
    // value being written has the correct size.
    LocalPointerInfo &PtrLPI = LPA.getLocalPointerInfo(PtrOperand);
    if (PtrLPI.pointsToSomeElement())
      analyzeElementLoadOrStore(PtrLPI, ValOperand, I, ValOperand->getType(),
                                I.isVolatile(),
                                /*IsLoad=*/false);
  }

  void visitGetElementPtrInst(GetElementPtrInst &I) {
    // TODO: Associate the parent type of the pointer so we can properly
    //       evaluate the uses.
    Value *Src = I.getPointerOperand();
    if (!isValueOfInterest(Src))
      return;

    // If a GetElementPtr instruction is used for a pointer, which aliases
    // multiple aggregate types, we need to set safety data for each of the
    // types. This may be the result of a union, but it may also be the
    // result of some optimization which merged two element accesses.
    LocalPointerInfo &SrcLPI = LPA.getLocalPointerInfo(Src);
    if (SrcLPI.pointsToMultipleAggregateTypes()) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Ambiguous GEP:\n"
                        << "  " << I << "\n");
      LLVM_DEBUG(SrcLPI.dump());
      setAllAliasedTypeSafetyData(SrcLPI, dtrans::AmbiguousGEP);
    }

    // If the local pointer analysis did not conclude that this value
    // points to an element within a structure aliased by the source pointer
    // and the GEP is not being used with a single index argument to index
    // from a pointer as if it were an array we need to set safety data on the
    // source pointer type indicating that bad element access occurred.
    // (Note: It is intentional that we are setting safety data for the source
    //        value rather than the GEP value.)
    LocalPointerInfo &GEPLPI = LPA.getLocalPointerInfo(&I);
    if (!GEPLPI.pointsToSomeElement()) {
      // If the source is an i8* value, this is a byte-flattened GEP access
      // and we should have been able to figure out the field being accessed.
      // Otherwise, a single index element indicates a pointer is being treated
      // as an array.
      if ((isInt8Ptr(Src) && !SrcLPI.isPtrToPtr()) || I.getNumIndices() != 1) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Bad pointer manipulation:\n"
                          << "  " << I << "\n");
        setAllAliasedTypeSafetyData(SrcLPI, dtrans::BadPtrManipulation);
      }
    } else {
      auto &PointeeSet = GEPLPI.getElementPointeeSet();
      assert(
          (SrcLPI.pointsToMultipleAggregateTypes() || PointeeSet.size() == 1) &&
          "GEP with single aggregate type points to multiple elements?");
      if (PointeeSet.size() == 1 && isInt8Ptr(Src)) {
        // If the GEP is pointing to some element and it is in the
        // byte-flattened form, store this information for later reference.
        DTInfo.addByteFlattenedGEPMapping(&I, *PointeeSet.begin());
      }

      // Check the uses of this GEP element. If it is used by anything other
      // than casts, loads, and stores, treat it as address taken. We may need
      // to revisit this, but if so we'll need to do more to figure out how
      // the address is being used.
      std::function<bool(Value *)> hasNonCastLoadStoreUses =
          [&hasNonCastLoadStoreUses](Value *V) {
            for (auto *U : V->users()) {
              if (isa<LoadInst>(U) || isa<StoreInst>(U))
                continue;
              if (isa<CastInst>(U)) {
                if (hasNonCastLoadStoreUses(U))
                  return true;
                continue;
              }
              // Anything else is the "other" use we were looking for.
              LLVM_DEBUG(dbgs() << "dtrans-field-info: Complex use of GEP: "
                                << *U << "\n");
              return true;
            }
            // If we get here, all users were load, store, or cast.
            return false;
          };

      if (hasNonCastLoadStoreUses(&I)) {
        for (auto PointeePair : PointeeSet) {
          llvm::Type *ParentTy = PointeePair.first;
          if (ParentTy->isStructTy()) {
            auto *ParentStInfo =
                cast<dtrans::StructInfo>(DTInfo.getOrCreateTypeInfo(ParentTy));
            dtrans::FieldInfo &FI = ParentStInfo->getField(PointeePair.second);
            FI.setComplexUse(true);
          }
        }
      }
    }
  }

  void visitPHINode(PHINode &I) {
    // PHI Nodes in LLVM merge other values. However, they may allow pointers
    // to be merged which alias to different types. If any of the incoming
    // values may point to a type of interest that is not pointed to by one
    // or more of the other incoming values, the merge is unsafe.
    analyzeSelectOrPHI(I);
  }

  void visitSelectInst(SelectInst &I) {
    // Selects in LLVM merge other values. However, they may allow pointers
    // to be merged which alias to different types. If any of the incoming
    // values may point to a type of interest that is not pointed to by one
    // or more of the other incoming values, the merge is unsafe.
    analyzeSelectOrPHI(I);
  }

  void visitPtrToIntInst(PtrToIntInst &I) {
    // If the source value is of interest, check to see if it is being cast
    // as a pointer-sized integer. If this is anything other than a
    // pointer being cast to a pointer-sized integer, it is a bad cast.
    // Otherwise, we will analyze this instruction when it is used.
    //
    // The safe usage looks like this:
    //
    //   %ps.as.i = ptrtoint %struct.s* %ps to i64
    //   %pps.as.pi = bitcast %struct.s** %pps to i64*
    //   store i64 %ps.as.i, i64* %pps.as.pi
    //
    // We will check this more closely when we visit the store instruction.
    Value *Src = I.getPointerOperand();
    if (!isValueOfInterest(Src))
      return;
    if (I.getDestTy() != PtrSizeIntTy) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
                        << "Ptr to aggregate cast to a non-ptr-sized integer:\n"
                        << "  " << I << "\n");
      setValueTypeInfoSafetyData(Src, dtrans::BadCasting);
    }

    // The isValueOfInterest() routine analyzes all PtrToInt result values
    // when they are used. Nothing more is needed here.
  }

  void visitReturnInst(ReturnInst &I) {
    llvm::Value *RetVal = I.getReturnValue();
    if (!RetVal || !isValueOfInterest(RetVal))
      return;

    // If the value returned is an instance of a type of interest, we need to
    // record this as a whole structure reference. This is unusual but it
    // is legal in LLVM IR.
    auto *RetTy = RetVal->getType();
    if (!RetTy->isPointerTy() && DTInfo.isTypeOfInterest(RetTy)) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Whole structure reference -- "
                        << "Type is returned by function: "
                        << I.getParent()->getParent()->getName() << "\n");
      setBaseTypeInfoSafetyData(RetTy, dtrans::WholeStructureReference);
      return;
    }

    // If the value returned is a pointer to a type of interest but is
    // not typed as that pointer type (for example, it has been cast to an i8*)
    // then we must note that the address of the object has escaped our
    // analysis.
    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(RetVal);
    for (auto *AliasTy : LPI.getPointerTypeAliasSet()) {
      // If the address of an aggregate is returned but the return value has
      // the expected type, that's OK. We will track that at the call site.
      if (AliasTy == RetTy)
        continue;
      // Skip over inconsequential aliases like i8* and i64.
      if (!DTInfo.isTypeOfInterest(AliasTy))
        continue;
      // Otherwise, the address is escaping our analysis.
      LLVM_DEBUG(dbgs() << "dtrans-safety: Address taken -- "
                        << "Non-typed address is returned by function: "
                        << I.getParent()->getParent()->getName() << "\n");
      setBaseTypeInfoSafetyData(AliasTy, dtrans::AddressTaken);
    }

    // If the value returned is a pointer to an element within an aggregate
    // type we also need to note that since it can have implications on
    // field access tracking.
    if (LPI.pointsToSomeElement()) {
      for (auto PointeePair : LPI.getElementPointeeSet()) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Field address taken -- "
                          << "Address of a field is returned by function: "
                          << I.getParent()->getParent()->getName() << "\n");
        setBaseTypeInfoSafetyData(PointeePair.first, dtrans::FieldAddressTaken);
        dtrans::TypeInfo *ParentTI =
            DTInfo.getOrCreateTypeInfo(PointeePair.first);
        if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(ParentTI))
          ParentStInfo->getField(PointeePair.second).setAddressTaken();
      }
    }
  }

  void visitICmpInst(ICmpInst &I) {
    // Compare instructions are always safe. When a compare is referencing
    // a field within an aggregate type, it does so through a GEP and a load
    // with the loaded value being passed to the compare. Therefore, we
    // do not need to track field information here either.

    // It is not possible to compare aggregate types directly.
    assert(!I.getOperand(0)->getType()->isAggregateType() &&
           !I.getOperand(1)->getType()->isAggregateType() &&
           "Unexpected compare of aggregate types.");
  }

  void visitAllocaInst(AllocaInst &I) {
    llvm::Type *Ty = I.getAllocatedType();
    if (!DTInfo.isTypeOfInterest(Ty))
      return;

    if (isa<PointerType>(Ty)) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Local pointer -- "
                        << "Pointer to type used by an alloca instruction:\n"
                        << "  " << I << "\n");
      setBaseTypeInfoSafetyData(Ty, dtrans::LocalPtr);
      return;
    }

    if (isa<ArrayType>(Ty)) {
      // For arrays of arrays, we want the deepest level of element to find
      // our type of interest.
      llvm::Type *ElemTy = Ty->getArrayElementType();
      while (isa<ArrayType>(ElemTy))
        ElemTy = ElemTy->getArrayElementType();
      // Arrays of pointers, including pointers to non-pointer arrays, are
      // effectively pointers to the type for the purposes of our analysis.
      if (isa<PointerType>(ElemTy)) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Local pointer -- "
                          << "Array of pointers to type used by "
                          << "an alloca instruction:\n"
                          << "  " << I << "\n");
        setBaseTypeInfoSafetyData(ElemTy, dtrans::LocalPtr);
        return;
      }
      if (isa<VectorType>(ElemTy)) {
        LLVM_DEBUG(
            dbgs() << "dtrans-safety: Unhandled use -- "
                   << "Array of vectors allocated by an alloca instruction:\n"
                   << "  " << I << "\n");
        setBaseTypeInfoSafetyData(Ty, dtrans::UnhandledUse);
        return;
      }
      LLVM_DEBUG(dbgs() << "dtrans-safety: Local instance -- "
                        << "Array of instance of type used by "
                        << "an alloca instruction:\n"
                        << "  " << I << "\n");
      setBaseTypeInfoSafetyData(Ty, dtrans::LocalInstance);
      return;
    }

    if (isa<VectorType>(Ty)) {
      LLVM_DEBUG(
          dbgs() << "dtrans-safety: Unhandled use -- "
                 << "Vector of type instantiated by an alloca instruction:\n"
                 << "  " << I << "\n");
      setBaseTypeInfoSafetyData(Ty, dtrans::UnhandledUse);
      return;
    }

    LLVM_DEBUG(dbgs() << "dtrans-safety: Local instance -- "
                      << "Type instantiated by an alloca instruction:\n"
                      << "  " << I << "\n");
    setBaseTypeInfoSafetyData(Ty, dtrans::LocalInstance);
  }

  void visitBinaryOperator(BinaryOperator &I) {
    // Binary operator analysis will be implemented as needed.
    // For now, unimplemented operators will cause values to be marked
    // as unhandled use.
    switch (I.getOpcode()) {
    case Instruction::Sub:
      analyzeSub(I);
      break;
    case Instruction::Or:
    case Instruction::And:
    case Instruction::Xor:
      analyzeBitMask(I);
      break;
    default:
      setBinaryOperatorUnhandledUse(I);
      break;
    }
  }

  // All instructions not handled by other visit functions.
  void visitInstruction(Instruction &I) {
    // Any instruction that we haven't yet modeled should be conservatively
    // treated as though it is doing something unsafe if it either returns
    // a value with a type of interest or takes a value as an operand that
    // is of a type of interest.
    llvm::Type *Ty = I.getType();
    if (DTInfo.isTypeOfInterest(Ty)) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
                        << "Type returned by an unmodeled instruction:\n"
                        << "  " << I << "\n");
      setBaseTypeInfoSafetyData(Ty, dtrans::UnhandledUse);
    }

    for (Value *Arg : I.operands()) {
      if (isValueOfInterest(Arg)) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
                          << "Type used by an unmodeled instruction:\n"
                          << "  " << I << "\n");
        setValueTypeInfoSafetyData(Arg, dtrans::UnhandledUse);
      }
    }
  }

  void visitModule(Module &M) {
    // Module's should already be materialized by the time this pass is run.
    assert(M.isMaterialized());

    // Analyze the structure types declared in the module.
    for (StructType *Ty : M.getIdentifiedStructTypes())
      analyzeStructureType(Ty);

    // Before visiting each Function, ensure that the types of all of the
    // Function arguments which are Types of interest are in the
    // type_info_entries. This is needed so we can determine if any
    // actual argument of an indirect or external call could be subject
    // to an AddressTaken safety violation due to an actual/formal argument
    // mismatch.
    for (auto &F : M.functions())
      for (auto &Arg : F.args())
        if (DTInfo.isTypeOfInterest(Arg.getType()))
          (void)DTInfo.getOrCreateTypeInfo(Arg.getType());

    // Call the base InstVisitor routine to visit each function.
    InstVisitor<DTransInstVisitor>::visitModule(M);

    // If a pointer to an aggregate is passed as an argument to an address
    // taken external function, that function could be a target of an
    // indirect call. Mark the aggregate and any type nested in it as
    // AddressTaken.
    for (auto &F : M.functions())
      if (F.hasAddressTaken() && F.isDeclaration())
        for (auto &Arg : F.args()) {
          LocalPointerInfo &LPI = LPA.getLocalPointerInfo(&Arg);
          for (auto *AliasTy : LPI.getPointerTypeAliasSet()) {
            if (!DTInfo.isTypeOfInterest(AliasTy))
              continue;
            LLVM_DEBUG({
              dbgs() << "dtrans-safety: Address taken -- "
                     << "pointer to aggregate passed as argument:\n"
                     << " ";
              F.printAsOperand(dbgs());
              dbgs() << "\n  " << Arg << "\n";
            });
            setBaseTypeInfoSafetyData(AliasTy, dtrans::AddressTaken);
          }
        }

    // Now follow the uses of global variables (not functions or aliases).
    for (auto &GV : M.globals()) {
      // No initializer indicates a declaration. We'll see the definition
      // if it's actually used.
      if (!GV.hasInitializer())
        continue;

      // Get the type of this variable.
      llvm::Type *GVTy = GV.getType();
      llvm::Type *GVElemTy = GVTy->getPointerElementType();

      // FIXME: Should we be considering all arrays of scalars as not
      //        interesting types?
      if (GVElemTy->isArrayTy() &&
          !DTInfo.isTypeOfInterest(GVElemTy->getArrayElementType()))
        continue;

      // If this is an interesting type, analyze its uses.
      if (DTInfo.isTypeOfInterest(GVTy)) {
        // These are conservative conditions meant to restrict us to
        // global variables that are definitely handled. If this condition
        // is triggered in code we think we can optimize additional handling
        // for these cases may be necessary.
        //
        // hasLocalLinkage() indicates that the linkage is either internal or
        //   private. This should be the case for all program defined variables
        //   during LTO. The primary intention of this check is to eliminate
        //   externally accessible variables, but we're using a more general
        //   check to defer decisions about other linkage types until they
        //   are encountered.
        //
        // isThreadLocal() may be acceptable but is included here so that
        //   consideration of its implications can be deferred until it must
        //   be handled.
        //
        if (!GV.hasLocalLinkage() || GV.isThreadLocal()) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
                            << "Unexpected global variable usage:\n"
                            << "  " << GV << "\n");
          setBaseTypeInfoSafetyData(GVTy, dtrans::UnhandledUse);
          continue;
        }
        if (GVElemTy->isPointerTy()) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Global pointer\n"
                            << "  " << GV << "\n");
          setBaseTypeInfoSafetyData(GVTy, dtrans::GlobalPtr);
          Constant *Initializer = GV.getInitializer();
          analyzeUnsafePointerStores(nullptr, Initializer, &GV);
        } else {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Global instance\n"
                            << "  " << GV << "\n");
          setBaseTypeInfoSafetyData(GVTy, dtrans::GlobalInstance);
          // The local linkage check should guarantee a unique and definitive
          // initializer.
          assert(GV.hasUniqueInitializer() && GV.hasDefinitiveInitializer() &&
                 !GV.isExternallyInitialized());
          Constant *Initializer = GV.getInitializer();
          if (!isa<ConstantAggregateZero>(Initializer) &&
              !isa<UndefValue>(Initializer)) {
            LLVM_DEBUG(dbgs() << "dtrans-safety: Has initializer list\n"
                              << "  " << GV << "\n");
            setBaseTypeInfoSafetyData(GVTy, dtrans::HasInitializerList);
          }
          analyzeGlobalStructSingleValue(GVElemTy, Initializer);
        }
      }
    }
  }

  void visitFunction(Function &F) {
    // There's nothing that needs to be done here. The argument uses will be
    // analyzed as the instructions are visited, but when we are running
    // lit tests we need to be sure we have created type info for the types
    // in the test. The lit tests often use arguments as an arbitrary source
    // for values of an aggregate type.
    //
    // When we are running analysis on a complete program we will create
    // all of the necessary types as we add complete information about them
    // but in lit tests a type with few uses and no safety issues might not
    // be added to the type info map.
    if (DTransPrintAnalyzedTypes)
      for (auto &Arg : F.args())
        if (DTInfo.isTypeOfInterest(Arg.getType()))
          (void)DTInfo.getOrCreateTypeInfo(Arg.getType());

    // Call the base class to visit the instructions in the function.
    InstVisitor<DTransInstVisitor>::visitFunction(F);
  }

private:
  DTransAnalysisInfo &DTInfo;
  const DataLayout &DL;
  const TargetLibraryInfo &TLI;

  // This helper class is used to track the types and aggregate elements to
  // which a local pointer value may refer. This information is created and
  // updated as needed.
  LocalPointerAnalyzer LPA;
  DTransAllocAnalyzer &DTAA;

  // We need these types often enough that it's worth keeping them around.
  llvm::Type *Int8PtrTy;
  llvm::Type *PtrSizeIntTy;
  llvm::Type *PtrSizeIntPtrTy;

  // There are frequent cases where a pointer to an aggregate type is
  // cast to either i8*, i64* or i64 and we need to look at the uses of
  // the cast value to determine whether or not it is being used in a way
  // that presents safety issues for the original type. This call allows
  // us to identify those cast values.
  //
  // This test current only supports immediate uses of cast values and
  // immediate uses of PHI or select users of those values. We may eventually
  // want to consider making a preliminary pass over entire functions to
  // identify all values that are indirectly dependent on a type of interest.
  bool isValueOfInterest(Value *V) {
    // If the type of the value is directly interesting, the answer is easy.
    if (DTInfo.isTypeOfInterest(V->getType()))
      return true;
    // Any pointer is potentially an interesting value. It may point to an
    // element within an aggregate type or it may be a bitcast copy of a
    // pointer to an aggregate type. Pointer-sized integers that are the
    // results of pointer-to-int casts may also alias to interesting values.
    // The local pointer info for the value will tell us if any of these
    // conditions are met.
    if (LPA.isPossiblePtrValue(V)) {
      LocalPointerInfo &LPI = LPA.getLocalPointerInfo(V);
      return (LPI.pointsToSomeElement() || LPI.canAliasToAggregatePointer());
    }
    return false;
  }

  inline bool isInt8Ptr(Value *V) { return (V->getType() == Int8PtrTy); }

  inline bool isPtrSizeIntPtr(Value *V) {
    return (V->getType() == PtrSizeIntPtrTy);
  }

  inline bool isPtrSizeInt(Value *V) { return (V->getType() == PtrSizeIntTy); }

  // Given two types if both types are pointers to pointers, unwind the
  // indirection to see if there is some level at which a pointer within
  // ATy is safely equivalent to an i8* or a pointer-sized integer in BTy.
  //
  // For instance, the following are all a safe bitcasts:
  //
  //   (1) %p1 = bitcast %struct.S** to i64*
  //   (2) %p2 = bitcast %struct.S*** to i8**
  //   (3) %p3 = bitcast %struct.S*** to i8***
  //
  // (1) is safe because after unwinding one level of indirection %struct.S*
  //     can safely be cast to a pointer-sized integer (i64).
  //
  // (2) is safe because after unwinding one level of indirection %struct.S**
  //     can be safely cast to i8*
  //
  // (3) is safe because after unwinding two levels of indirection %struct.S*
  //     can safely be cast to i8*.
  //
  // We need the equivalence to be directional. For instance, any
  // %struct.A* can be cast to an i8* but not just any i8* can be cast to a
  // %struct.A*. In cases where the generic pointer can be either type,
  // this function must be called twice, with the arguments reversed the second
  // time.
  bool isGenericPtrEquivalent(llvm::Type *ATy, llvm::Type *BTy) {
    auto *TempATy = ATy;
    auto *TempBTy = BTy;
    while (TempATy->isPointerTy() && TempBTy->isPointerTy()) {
      if (TempBTy == Int8PtrTy)
        return true;

      auto *NextATy = TempATy->getPointerElementType();
      auto *NextBTy = TempBTy->getPointerElementType();

      if (TempBTy == PtrSizeIntPtrTy && NextATy->isPointerTy())
        return true;

      TempATy = NextATy;
      TempBTy = NextBTy;
    }

    return false;
  }

  // Check to see if the value aliases to incompatible pointer types.
  // The alias set may contain a pointer-sized integer, which is compatible
  // with any pointer (via PtrToInt). The i8* type is also compatible with
  // any pointer type. Finally, any pointer-to-pointer is compatible with
  // a pointer to a pointer-sized integer (i.e. i64*).
  //
  // Ideally, we'd move this logic into LocalPointerInfo so that it could be
  // determined as aliases are added. However, the generic pointer check
  // requires access to the DataLayout which isn't easily available in the
  // LocalPointerInfo. So for now we're leaving it here.
  //
  // In practice, the maximum number of compatible aliases is 2*N+1 where N is
  // the number of levels of indirection on the dominant pointer type. In
  // practice there will almost always be three or fewer aliases, so this code
  // should be trivially fast.
  bool isAliasSetOverloaded(LocalPointerInfo::PointerTypeAliasSetRef Aliases,
                            bool AllowElementZeroAccess = false) {
    auto AliasEnd = Aliases.end();
    for (auto AliasIt = Aliases.begin(); AliasIt != AliasEnd; ++AliasIt) {
      auto *AliasTy = *AliasIt;
      if (!AliasTy->isPointerTy() || (AliasTy == Int8PtrTy))
        continue;

      for (auto OtherAliasIt = std::next(AliasIt); OtherAliasIt != AliasEnd;
           ++OtherAliasIt) {
        auto *OtherAliasTy = *OtherAliasIt;
        if (!OtherAliasTy->isPointerTy() || (OtherAliasTy == Int8PtrTy))
          continue;
        if (isGenericPtrEquivalent(AliasTy, OtherAliasTy) ||
            isGenericPtrEquivalent(OtherAliasTy, AliasTy))
          continue;
        if (AllowElementZeroAccess &&
            (dtrans::isElementZeroAccess(AliasTy, OtherAliasTy) ||
             dtrans::isElementZeroAccess(OtherAliasTy, AliasTy)))
          continue;
        return true;
      }
    }
    return false;
  }

  // This is a helper function that determines whether \p ParentTy points to
  // a type whose first element is a fixed size array of the type pointed to
  // by \p ElementTy.
  bool isElementZeroArrayOfType(llvm::Type *ParentTy, llvm::Type *ElementTy) {
    // This may be called with a null type pointer.
    if (!ParentTy || !ParentTy->isPointerTy() || !ElementTy->isPointerTy())
      return false;

    auto *BaseTy = ParentTy->getPointerElementType();
    if (!BaseTy || (!BaseTy->isStructTy() && !BaseTy->isArrayTy()))
      return false;

    auto *ElementZeroTy = cast<CompositeType>(BaseTy)->getTypeAtIndex(0u);
    if (!ElementZeroTy->isArrayTy())
      return false;

    return ElementZeroTy->getArrayElementType() ==
           ElementTy->getPointerElementType();
  }

  // Analyze a structure definition, independent of its use in any
  // instruction. This checks for basic issues like structure nesting
  // and empty structures.
  void analyzeStructureType(llvm::StructType *Ty) {
    // Add this to our type info list.
    dtrans::TypeInfo *TI = DTInfo.getOrCreateTypeInfo(Ty);

    // Check to see if this structure is known to be a system type.
    if (dtrans::isSystemObjectType(Ty)) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: System object:\n  " << *Ty << "\n");
      TI->setSafetyData(dtrans::SystemObject);
    }

    // Get the number of fields in the structure.
    unsigned NumElements = Ty->getNumElements();

    // If the structure is empty, we can't need to try to optimize it.
    if (NumElements == 0) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: No fields in structure:\n  " << *Ty
                        << "\n");
      TI->setSafetyData(dtrans::NoFieldsInStruct);
      return;
    }

    // I think opaque structures will report having zero fields.
    assert(!Ty->isOpaque());

    // It isn't clear to me under what circumstances a type will be reported
    // as unsized, but if one is we definitely can't do anything with it.
    if (!Ty->isSized()) {
      LLVM_DEBUG(
          dbgs() << "dtrans-safety: Unhandled use -- non-sized structure\n  "
                 << *Ty << "\n");
      TI->setSafetyData(dtrans::UnhandledUse);
    }

    // Walk the fields in the structure looking for nested structures.
    for (llvm::Type *ElementTy : Ty->elements()) {
      // If the element is a non-pointer structure, set the nested type
      // safety conditions.
      if (ElementTy->isStructTy()) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Nested structure\n"
                          << "  parent: " << *Ty << "\n"
                          << "  child : " << *ElementTy << "\n");
        TI->setSafetyData(dtrans::ContainsNestedStruct);
        DTInfo.getOrCreateTypeInfo(ElementTy)->setSafetyData(
            dtrans::NestedStruct);
        continue;
      }
      // If one of the fields is a vector or array, check for a contained
      // structure type.
      if (auto *SeqTy = dyn_cast<SequentialType>(ElementTy)) {
        // Look through multiple layers of arrays or vectors if necessary.
        llvm::Type *NestedTy = SeqTy;
        while (isa<SequentialType>(NestedTy))
          NestedTy = NestedTy->getSequentialElementType();
        // If this was an array/vector of structures, set the nested type
        // safety conditions.
        if (NestedTy->isStructTy()) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Nested structure\n"
                            << "  parent: " << *Ty << "\n"
                            << "  child : " << *NestedTy << "\n");
          TI->setSafetyData(dtrans::ContainsNestedStruct);
          DTInfo.getOrCreateTypeInfo(NestedTy)->setSafetyData(
              dtrans::NestedStruct);
        }
      }
    }
  }

  // Verify that a bitcast from \p SrcTy to \p DestTy would be safe. The
  // caller has analyzed the bitcast instruction to determine that these
  // types need to be considered. These may not be the actual types used
  // by the bitcast instruction, but the source operand will be known to
  // be an instance of SrcTy in some way.
  void verifyBitCastSafety(BitCastInst &I, llvm::Type *SrcTy,
                           llvm::Type *DestTy) {
    // If the types are the same, it's a safe cast.
    if (SrcTy == DestTy)
      return;

    // If DestTy is a generic equivalent of SrcTy, it's a safe cast.
    if (isGenericPtrEquivalent(SrcTy, DestTy))
      return;

    // If this can be interpreted as an element-zero access of SrcTy,
    // it's a safe cast.
    if (dtrans::isElementZeroAccess(SrcTy, DestTy))
      return;

    // Otherwise, it's not safe.
    LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
                      << "unsafe cast of aliased pointer:\n"
                      << "  " << I << "\n");
    if (DTransOutOfBoundsOK)
      setValueTypeInfoSafetyData(I.getOperand(0), dtrans::BadCasting);
    else
      (void)setValueTypeInfoSafetyDataBase(I.getOperand(0), dtrans::BadCasting);

    if (DTInfo.isTypeOfInterest(DestTy)) {
      if (DTransOutOfBoundsOK)
        setValueTypeInfoSafetyData(&I, dtrans::BadCasting);
      else
        (void)setValueTypeInfoSafetyDataBase(&I, dtrans::BadCasting);
    }
  }

  //
  // Indicate that all fields of TI may have a zero value because calloc
  // was used to allocate a structure of this type.
  //
  void analyzeCallocSingleValue(dtrans::TypeInfo *TI) {
    if (TI == nullptr)
      return;
    if (!TI->getLLVMType()->isAggregateType())
      return;
    if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI)) {
      int Count = 0;
      for (auto &FI : StInfo->getFields()) {
        Constant *NV = llvm::Constant::getNullValue(FI.getLLVMType());
        LLVM_DEBUG(dbgs() << "dtrans-fsv: " << *(StInfo->getLLVMType()) << " ["
                          << Count << "] ");
        LLVM_DEBUG(NV->printAsOperand(dbgs()));
        FI.processNewSingleValue(NV);
        LLVM_DEBUG(dbgs() << (FI.isMultipleValue() ? " <MULTIPLE>\n" : "\n"));
        auto *ComponentTI = DTInfo.getTypeInfo(FI.getLLVMType());
        analyzeCallocSingleValue(ComponentTI);
        ++Count;
      }
    } else if (auto *AInfo = dyn_cast<dtrans::ArrayInfo>(TI)) {
      auto *ComponentTI = AInfo->getElementDTransInfo();
      analyzeCallocSingleValue(ComponentTI);
    }
  }

  void analyzeAllocationCall(CallInst &CI, dtrans::AllocKind Kind) {

    // The LocalPointerAnalyzer will visit bitcast users to determine the
    // type of memory being allocated. This must be done in the
    // LocalPointerAnalyzer class rather than here because it is possible
    // that we will visit the bitcast first and the LocalPointerAnalyzer
    // must be able to identify the connection with the allocation call
    // in that case also.
    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(&CI);

    // The list of aliased types in the LPI will be the types to which
    // this pointer is cast that were not identified as element zero
    // accesses. In almost all cases there will only be one such type.

    // If the malloc wasn't cast to a type of interest, we're finished.
    LocalPointerInfo::PointerTypeAliasSetRef &AliasSet =
        LPI.getPointerTypeAliasSet();
    if (AliasSet.empty()) {
      return;
    }

    dtrans::AllocCallInfo *ACI = DTInfo.createAllocCallInfo(&CI, Kind);
    populateCallInfoFromLPI(LPI, ACI);

    // If the value is cast to multiple types, mark them all as bad casting.
    bool WasCastToMultipleTypes = LPI.pointsToMultipleAggregateTypes();

    if (DTransPrintAllocations && WasCastToMultipleTypes)
      outs() << "dtrans: Detected allocation cast to multiple types.\n";

    // We expect to only see one type, but we loop to keep the code general.
    for (auto *Ty : AliasSet) {
      if (!DTInfo.isTypeOfInterest(Ty))
        continue;

      // If there are casts to multiple types of interest, they all get
      // handled as bad casts.
      if (WasCastToMultipleTypes) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
                          << "allocation cast to multiple types:\n  " << CI
                          << "\n");
        setBaseTypeInfoSafetyData(Ty, dtrans::BadCasting);
      }

      // Check the size of the allocation to make sure it's a multiple of the
      // size of the type being allocated.
      verifyAllocationSize(CI, Kind, cast<PointerType>(Ty));

      // Add this to our type info list.
      (void)DTInfo.getOrCreateTypeInfo(Ty);

      if (DTransPrintAllocations) {
        outs() << "dtrans: Detected allocation cast to pointer type\n";
        outs() << "  " << CI << "\n";
        outs() << "    Detected type: " << *(Ty->getPointerElementType())
               << "\n";
      }

      if (Kind == dtrans::AK_Calloc) {
        auto *TI = DTInfo.getOrCreateTypeInfo(Ty->getPointerElementType());
        analyzeCallocSingleValue(TI);
      }
    }
  }

  /// Record the type of object being passed to a call to "free".
  /// This is useful for transformations that need to rewrite allocation and
  /// free calls.
  void analyzeFreeCall(CallInst &CI, dtrans::FreeKind FK) {
    LLVM_DEBUG(dbgs() << "dtrans: Analyzing free call.\n  " << CI << "\n");
    dtrans::FreeCallInfo *FCI = DTInfo.createFreeCallInfo(&CI, FK);

    // If it's a standard free call, we can check for the type of the first
    // argument for the potential pointer types. If it's a user free call, there
    // is currently no guarantee to know which argument contains the TypeInfo.
    if (FK == dtrans::FK_Free) {
      Value *Arg = CI.getOperand(0);
      LocalPointerInfo &LPI = LPA.getLocalPointerInfo(Arg);
      LocalPointerInfo::PointerTypeAliasSetRef &AliasSet =
          LPI.getPointerTypeAliasSet();
      if (AliasSet.empty()) {
        return;
      }
      populateCallInfoFromLPI(LPI, FCI);
    } else {
      FCI->setAnalyzed(false);
    }
  }

  // Return true if the Instruction *I is not used in some way that inhibits
  // marking it as a single alloc function.  Right now, we allow the
  // following cases:
  //   (1) It can be used in a test against a nullptr.
  //   (2) It can be passed to "free" or something equivalent.
  //   (3) It can be passed to a "mem" intrinsic.
  //   (4) It can be passed down to a load.
  //   (5) It is used by a GEP instruction whose uses are covered by (1-5)
  // The point here is to determine whether the pointer being assigned to
  // a field can escape.  If it does, the memory could be, for example, be
  // realloc'ed and it might not have the same properties that it had
  // when it was returned from the single alloc function.
  bool isSafeLoadForSingleAllocFunction(Instruction *I) {
    for (auto *U : I->users()) {
      if (auto ICI = dyn_cast<ICmpInst>(U)) {
        Value *V = nullptr;
        if (isa<ConstantPointerNull>(ICI->getOperand(0)))
          V = ICI->getOperand(1);
        else if (isa<ConstantPointerNull>(ICI->getOperand(1)))
          V = ICI->getOperand(0);
        if (V == nullptr)
          return false;
      } else if (auto CI = dyn_cast<CallInst>(U)) {
        Function *F = CI->getCalledFunction();
        if (F == nullptr)
          return false;
        if (dtrans::isFreeFn(F, TLI) || DTAA.isFreePostDom(F))
          return true;
        if (auto II = dyn_cast<IntrinsicInst>(U)) {
          Intrinsic::ID Intrin = II->getIntrinsicID();
          switch (Intrin) {
          case Intrinsic::memset:
          case Intrinsic::memcpy:
          case Intrinsic::memmove:
            return true;
          default:
            break;
          }
          return false;
        }
        return false;
      } else if (auto GEPI = dyn_cast<GetElementPtrInst>(U)) {
        return GEPI->getPointerOperand() == I &&
               isSafeLoadForSingleAllocFunction(GEPI);
      } else if (!isa<LoadInst>(U))
        return false;
    }
    return true;
  }

  // Return true if the CallInst *CI is to a suitably identified alloc
  // function.
  bool isSafeStoreForSingleAllocFunction(CallInst *CI) {
    Function *F = CI->getCalledFunction();
    if (F == nullptr)
      return false;
    dtrans::AllocKind Kind = dtrans::getAllocFnKind(F, TLI);
    if (Kind == dtrans::AK_NotAlloc && DTAA.isMallocPostDom(F))
      Kind = dtrans::AK_UserMalloc;
    return Kind != dtrans::AK_NotAlloc;
  }

  // The element access analysis for load and store instructions are nearly
  // identical, so we use this helper function to perform the task for both.
  // For both loads and stores the PtrInfo argument refers to the address that
  // is being accessed. For load instructions, the WriteVal argument is
  // nullptr, while for store instructions, the Val argument is the value
  // operand of the store instruction. For load instructions, the ValTy
  // argument is the type of the value being loaded (i.e. the type of the value
  // returned by the load instruction). For store instructions, the ValTy
  // argument is the type of the value operand to the store instruction.
  void analyzeElementLoadOrStore(LocalPointerInfo &PtrInfo, Value *WriteVal,
                                 Instruction &I, llvm::Type *ValTy,
                                 bool IsVolatile, bool IsLoad) {
    // There will generally only be one ElementPointee in code that is safe for
    // dtrans to operate on, but I'm using a for-loop here to keep the
    // analysis as general as possible.
    //
    // TODO: Track read/write frequency.
    for (auto &PointeePair : PtrInfo.getElementPointeeSet()) {
      llvm::Type *ParentTy = PointeePair.first;
      if (IsVolatile) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Volatile data:\n");
        LLVM_DEBUG(dbgs() << "  " << I << "\n");
        setBaseTypeInfoSafetyData(ParentTy, dtrans::VolatileData);
      }

      if (auto *CompTy = cast<CompositeType>(ParentTy)) {
        llvm::Type *FieldTy = CompTy->getTypeAtIndex(PointeePair.second);

        // If this field is an aggregate, and this is not a nested
        // element zero access, mark this as a whole structure reference.
        if (FieldTy->isAggregateType() && FieldTy == ValTy) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Whole structure reference:\n");
          LLVM_DEBUG(dbgs() << "  " << I << "\n");
          setBaseTypeInfoSafetyData(FieldTy, dtrans::WholeStructureReference);
        }

        // The value type must be the same as the field type or the field must
        // be a pointer and the value a pointer-sized integer or an i8*.
        if ((FieldTy != ValTy) &&
            (!FieldTy->isPointerTy() ||
             (ValTy != PtrSizeIntTy && ValTy != Int8PtrTy))) {
          // The local pointer analyzer should have directed us to the
          // correct level of nesting.
          assert(!dtrans::isElementZeroAccess(FieldTy->getPointerTo(),
                                              ValTy->getPointerTo()));
          LLVM_DEBUG(dbgs() << "dtrans-safety: Mismatched element access:\n");
          LLVM_DEBUG(dbgs() << "  " << I << "\n");
          setBaseTypeInfoSafetyData(ParentTy, dtrans::MismatchedElementAccess);
        }

        if (ParentTy->isStructTy()) {
          auto *ParentStInfo =
              cast<dtrans::StructInfo>(DTInfo.getOrCreateTypeInfo(ParentTy));
          dtrans::FieldInfo &FI = ParentStInfo->getField(PointeePair.second);
          if (IsLoad) {
            FI.setRead(true);
            if (!isSafeLoadForSingleAllocFunction(&I)) {
              if (!FI.isBottomAllocFunction())
                LLVM_DEBUG(dbgs()
                           << "dtrans-fsaf: " << *(ParentStInfo->getLLVMType())
                           << " [" << PointeePair.second << "] <BOTTOM>\n");
              FI.setBottomAllocFunction();
            }
          } else {
            if (auto *ConstVal = dyn_cast<llvm::Constant>(WriteVal)) {
              if (FI.processNewSingleValue(ConstVal)) {
                LLVM_DEBUG({
                  dbgs() << "dtrans-fsv: " << *(ParentStInfo->getLLVMType());
                  if (FI.isSingleValue())
                    ConstVal->printAsOperand(dbgs());
                  else
                    dbgs() << "<MULTIPLE>";
                  dbgs() << "\n";
                });
              }
              if (!isa<ConstantPointerNull>(WriteVal)) {
                if (!FI.isBottomAllocFunction())
                  LLVM_DEBUG(dbgs() << "dtrans-fsaf: "
                                    << *(ParentStInfo->getLLVMType()) << " ["
                                    << PointeePair.second << "] <BOTTOM>\n");
                FI.setBottomAllocFunction();
              }
            } else if (auto *CI = dyn_cast<CallInst>(WriteVal)) {
              if (!FI.isMultipleValue())
                LLVM_DEBUG(dbgs()
                           << "dtrans-fsv: " << *(ParentStInfo->getLLVMType())
                           << " [" << PointeePair.second << "] <MULTIPLE>\n");
              FI.setMultipleValue();
              if (isSafeStoreForSingleAllocFunction(CI)) {
                Function *Callee = CI->getCalledFunction();
                if (FI.processNewSingleAllocFunction(Callee)) {
                  LLVM_DEBUG({
                    dbgs() << "dtrans-fsaf: " << *(ParentStInfo->getLLVMType())
                           << " [" << PointeePair.second << "] ";
                    if (FI.isSingleAllocFunction())
                      Callee->printAsOperand(dbgs());
                    else
                      dbgs() << "<BOTTOM>";
                    dbgs() << "\n";
                  });
                }
              } else {
                if (!FI.isBottomAllocFunction())
                  LLVM_DEBUG(dbgs() << "dtrans-fsaf: "
                                    << *(ParentStInfo->getLLVMType()) << " ["
                                    << PointeePair.second << "] <BOTTOM>\n");
                FI.setBottomAllocFunction();
              }
            } else {
              if (!FI.isMultipleValue())
                LLVM_DEBUG(dbgs()
                           << "dtrans-fsv: " << *(ParentStInfo->getLLVMType())
                           << " [" << PointeePair.second << "] <MULTIPLE>\n");
              FI.setMultipleValue();
              if (!FI.isBottomAllocFunction())
                LLVM_DEBUG(dbgs()
                           << "dtrans-fsaf: " << *(ParentStInfo->getLLVMType())
                           << " [" << PointeePair.second << "] <BOTTOM>\n");
              FI.setBottomAllocFunction();
            }
            FI.setWritten(true);
          }
        }
        // TODO: Track array element access?
      } else {
        // Otherwise, the parent type is either a vector or a pointer (which
        // would have been indexed as an array).
        // TODO: Handle this case.
        LLVM_DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
                             "unimplemented load/store:\n");
        LLVM_DEBUG(dbgs() << "  " << I << "\n");
        setBaseTypeInfoSafetyData(ParentTy, dtrans::UnhandledUse);
      }
    }
  }

  /// \brief Check for overloaded alias sets being introduced by a PHI node
  ///        or select instruction.
  ///
  /// When multiple pointers are merged using either a PHI node or a select
  /// instruction the type alias sets for the pointers are merged by our
  /// local pointer analyzer. If the new alias set is overloaded (that is,
  /// incompatible types are aliased) we want to be able to identify the
  /// spot where that happened. This will typically be caused by some
  /// transformation that we may be able to undo. An example of such a case
  /// might look like this:
  ///
  ///   Block_A:
  ///     %p1 = call i8* @malloc(i64 128)
  ///     %p_A = bitcast i8* %p1 to %struct.A
  ///     br label %Block_C
  ///
  ///   Block_B:
  ///     %p2 = call i8* @malloc(i64 32)
  ///     %p_B = bitcast i8* %p2 to %struct.B
  ///     br label %Block_C
  ///
  ///   Block_C:
  ///     %p_merged = phi i8* [%p1, %Block_A], [%p2, %Block_B]
  ///     br label %Block_D
  ///
  /// In this case, %p_merged may be a pointer to either %struct.A or
  /// %struct.B. We will be unable to optimize either of those structures
  /// unless we are able to rewrite this IR in some way that eliminates this
  /// PHI node.
  void analyzeSelectOrPHI(Instruction &I) {
    // This routine should only ever be called for PHI nodes or select
    // instructions.
    assert(isa<SelectInst>(&I) || isa<PHINode>(&I));

    // Local pointer analysis will already know if none of the incoming
    // values are of interest, so we can avoid the checks below.
    if (!isValueOfInterest(&I))
      return;

    // The local pointer analyzer will handle merging all relevant information
    // for this instruction. If the value resulting from the merge has an
    // overloaded alias set, the instruction and all of its aliased types
    // should be marked as an unsafe merge.
    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(&I);
    if (isAliasSetOverloaded(LPI.getPointerTypeAliasSet())) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Unsafe pointer merge -- "
                        << "overloaded merge result\n"
                        << "  " << I << "\n");
      setValueTypeInfoSafetyData(&I, dtrans::UnsafePtrMerge);
      return;
    }

    // Get the aggregate type to which the result points. If this call returns
    // null, it is either because the value is overloaded (which we checked
    // above) or it is a pointer to some element. It is common to merge
    // pointers to fields with other pointers to the same type of element.
    // If there are problems with the field access, they will appear in
    // later analysis. The merge itself is safe.
    llvm::Type *DomTy = LPI.getDominantAggregateTy();
    if (!DomTy)
      return;

    // Build a set of all the incoming values for this instruction.
    SmallVector<Value *, 4> IncomingVals;
    if (auto *Sel = dyn_cast<SelectInst>(&I)) {
      IncomingVals.push_back(Sel->getTrueValue());
      IncomingVals.push_back(Sel->getFalseValue());
    } else {
      auto *PHI = cast<PHINode>(&I);
      for (Value *Val : PHI->incoming_values())
        IncomingVals.push_back(Val);
    }

    // If the result of the merge points to some aggregate type, each of
    // the incoming values must also be capable of pointing to that type
    // otherwise the merge is unsafe. For example, if a pointer to a structure
    // is bitcast to an i8* and then merged with some arbitrary i8* value
    // that is not otherwise known to point to that structure type, the merge
    // is unsafe because the memory pointed to by the arbitrary i8* value
    // cannot be transformed in any meaningful way.
    for (auto *ValIn : IncomingVals) {
      // Null pointers can be safely merged since they don't actually point
      // to a memory buffer that can be used in any way.
      if (isa<ConstantPointerNull>(ValIn))
        continue;
      LocalPointerInfo &ValLPI = LPA.getLocalPointerInfo(ValIn);
      if (!ValLPI.canPointToType(DomTy->getPointerElementType())) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Unsafe pointer merge -- "
                          << "incompatible incoming value\n"
                          << "  " << I << "\n");
        setValueTypeInfoSafetyData(&I, dtrans::UnsafePtrMerge);
        return;
      }
    }

    // Otherwise, the merge is safe.
  }

  /// Given a call instruction that has been determined to be an allocation
  /// of the specified aggregate type, check the size arguments to verify
  /// that the allocation is a multiple of the type size.
  void verifyAllocationSize(CallInst &CI, dtrans::AllocKind Kind,
                            llvm::PointerType *Ty) {
    // The type may be a pointer to an aggregate or a pointer to a pointer.
    // In either case, it is the type that was used as a bitcast for the
    // return value of an allocation call. So if it is a pointer to a pointer
    // then the allocated buffer is a buffer that will container a pointer
    // or array of pointers. Therefore, we do not want to trace all the way
    // to the base type. If Ty is a pointer-to-pointer type then we do
    // actually want to use the size of a pointer as the element size.
    //
    // The size returned by DL.getTypeAllocSize() includes padding, both
    // within the type and between successive elements of the same type
    // if multiple elements are being allocated.
    uint64_t ElementSize = DL.getTypeAllocSize(Ty->getElementType());
    Value *AllocSizeVal;
    Value *AllocCountVal;
    getAllocSizeArgs(Kind, &CI, AllocSizeVal, AllocCountVal);

    // If either AllocSizeVal or AllocCountVal can be proven to be a multiple
    // of the element size, the size arguments are acceptable.
    if (dtrans::isValueMultipleOfSize(AllocSizeVal, ElementSize) ||
        dtrans::isValueMultipleOfSize(AllocCountVal, ElementSize))
      return;

    // If the allocation is cast as a pointer to a fixed size array, and
    // one argument is a multiple of the array's element size and the other
    // is a multiple of the number of elements in the array, the size arguments
    // are acceptable.
    if (Ty->getElementType()->isArrayTy() && (AllocCountVal != nullptr)) {
      llvm::Type *PointeeTy = Ty->getElementType();
      uint64_t NumArrElements = PointeeTy->getArrayNumElements();
      uint64_t ArrElementSize =
          DL.getTypeAllocSize(PointeeTy->getArrayElementType());
      if ((dtrans::isValueMultipleOfSize(AllocSizeVal, ArrElementSize) &&
           dtrans::isValueMultipleOfSize(AllocCountVal, NumArrElements)) ||
          (dtrans::isValueMultipleOfSize(AllocCountVal, ArrElementSize) &&
           dtrans::isValueMultipleOfSize(AllocSizeVal, NumArrElements)))
        return;
    }

    // Otherwise, we must assume the size arguments are not acceptable.
    LLVM_DEBUG(dbgs() << "dtrans-safety: Bad alloc size:\n"
                      << "  " << CI << "\n");
    setBaseTypeInfoSafetyData(Ty, dtrans::BadAllocSizeArg);
  }

  // Check the destination of a call to memset for safety.
  //
  // A safe call is one where it can be resolved that the operand to the
  // memset meets the following conditions:
  //   - The operand does not affect an aggregate data type.
  //  or
  //   - If the operand is not a pointer to a field within an aggregate, then
  //     the size must be a multiple of the aggregate size. (If the size is
  //     smaller than the aggregate type and a specific subset of fields is set,
  //     the memfunc partial write safety bit will be set)
  //   - If the operand is a pointer to a field within an aggregate, then
  //     the size operand must cover the size of one or more fields, in which
  //     case the memfunc partial write safety bit will be set on the containing
  //     structure.
  //  or
  //  - The size operand is 0.
  //
  // A necessary precursor for most of these rule is that the operand type
  // is able to be resolved to a unique dominant type for the pointer.
  //
  // For a safe call, the field information tracking of the aggregate type
  // will be updated to indicate the field is written to.
  void analyzeMemset(IntrinsicInst &I) {
    LLVM_DEBUG(dbgs() << "dtrans: Analyzing memset call:\n  " << I << "\n");

    assert(I.getNumArgOperands() >= 2);
    auto *DestArg = I.getArgOperand(0);
    Value *SetSize = I.getArgOperand(2);

    // A memset of 0 bytes will not affect the safety of any data structure.
    if (dtrans::isValueEqualToSize(SetSize, 0))
      return;

    if (!isValueOfInterest(DestArg))
      return;

    LocalPointerInfo &DstLPI = LPA.getLocalPointerInfo(DestArg);
    auto *DestParentTy = DstLPI.getDominantAggregateTy();
    if (!DestParentTy) {
      if (isAliasSetOverloaded(DstLPI.getPointerTypeAliasSet())) {
        // Cannot do anything for an aliased type. Declare them
        // to be unsafe.
        LLVM_DEBUG(dbgs() << "dtrans-safety: Ambiguous pointer target -- "
                          << "Aliased type, could not identify dominant type:\n"
                          << " " << I << "\n");

        setAllAliasedTypeSafetyData(DstLPI, dtrans::AmbiguousPointerTarget);
        return;
      }

      // We expect we are dealing with a pointer to scalar element member
      // when no dominant type was found, but if not, we don't handle it
      // in the code below.
      if (!DstLPI.pointsToSomeElement()) {
        setAllAliasedTypeSafetyData(DstLPI, dtrans::UnhandledUse);
        return;
      }
    }

    // Verify the size being is valid for the type of the pointer:
    //  - When an aggregate type is passed, check if the size parameter
    //    equals the aggregate size (or some multiple to allow for arrays) and
    //    mark all the fields as being written. If the size is smaller than
    //    the aggregate type, try to identify which fields are being set to
    //    mark those as written and set the safety bit to indicate only a
    //    portion of the structure is covered.
    //
    //  - When a pointer to a member of another aggregate is used, make sure
    //    the size being set is exactly the size of aggregate being set, or
    //    covers a proper subset of fields.
    //
    // NOTE: In the future, an additional safety check may be necessary here
    // to check if a non-zero value is being written to inhibit
    // transformations such as field shrinking. But this is not implemented
    // now because it will also require the rest of the analysis to perform
    // range checks of the values being stored in a field.

    if (DstLPI.pointsToSomeElement()) {
      StructType *StructTy = nullptr;
      size_t FieldNum = 0;

      // Check if the pointer-to-member is a member of structure that can
      // be analyzed.
      if (isSimpleStructureMember(DstLPI, &StructTy, &FieldNum)) {

        // Pass 'false' for IsValuePreservingWrite to conservatively mark all
        // fields as being multiple value from the memset.  We can improve this
        // analysis later by analyzing the value being set.
        dtrans::MemfuncRegion RegionDesc;
        if (analyzeMemfuncStructureMemberParam(I, StructTy, FieldNum, SetSize,
                                               /*IsValuePreservingWrite=*/false,
                                               RegionDesc))
          createMemsetCallInfo(I, StructTy->getPointerTo(), RegionDesc);

        return;
      }

      // The pointer to member was not able to be analyzed. It could be a member
      // of an array type, or the element pointee set contained multiple
      // entries.
      if (DstLPI.getElementPointeeSet().size() != 1) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Ambiguous pointer target -- "
                          << "Pointer to member with multiple potential target "
                             "members:\n"
                          << "  " << I << "\n");

        setValueTypeInfoSafetyData(DestArg, dtrans::AmbiguousPointerTarget);
      } else {
        // This could be extended in the future to handle the case that it was
        // member of an array.
        LLVM_DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
                          << "Pointer to member of array type:\n"
                          << "  " << I << "\n");

        setValueTypeInfoSafetyData(DestArg, dtrans::UnhandledUse);
      }

      return;
    }

    // The operand is not a pointer to member if we reach this point
    auto *DestPointeeTy = DestParentTy->getPointerElementType();
    uint64_t ElementSize = DL.getTypeAllocSize(DestPointeeTy);

    if (!DestPointeeTy->isAggregateType())
      return;

    // Conservatively mark all fields as having been written by the memset.
    // We can improve this analysis later.
    auto *ParentTI = DTInfo.getOrCreateTypeInfo(DestPointeeTy);
    markAllFieldsMultipleValue(ParentTI);

    // Consider the case where the complete aggregate (or an array of
    // aggregates is being set).
    if (dtrans::isValueMultipleOfSize(SetSize, ElementSize)) {
      // It is a safe use. Mark all the fields as being written.
      markAllFieldsWritten(ParentTI);

      dtrans::MemfuncRegion RegionDesc;
      RegionDesc.IsCompleteAggregate = true;
      createMemsetCallInfo(I, DestParentTy, RegionDesc);

      return;
    }

    // Consider the case where a portion of a structure is being set, starting
    // from the 1st field.
    if (auto *StructTy = dyn_cast<StructType>(DestPointeeTy)) {
      dtrans::MemfuncRegion RegionDesc;
      if (analyzeMemfuncStructureMemberParam(I, StructTy, 0, SetSize,
                                             /*IsValuePreservingWrite=*/false,
                                             RegionDesc))
        createMemsetCallInfo(I, StructTy->getPointerTo(), RegionDesc);

      return;
    }

    LLVM_DEBUG(dbgs() << "dtrans-safety: Bad memfunc size -- "
                      << "size is not a multiple of type size:\n"
                      << "  " << I << "\n");

    setValueTypeInfoSafetyData(DestArg, dtrans::BadMemFuncSize);
  }

  // Check the source and destination of a call to memcpy or memmove call
  // for safety.
  //
  // A safe call is one where it can be resolved that the operands to the
  // call meets of the following conditions:
  //   - Neither operand affects an aggregate data type.
  //   or
  //   - The source and destination data types are the same.
  //   - If the operand is not a pointer to a field within an aggregate, then
  //     the size must be a multiple of the aggregate size.
  //   - If the either the source or destination operands are pointers to a
  //     field within an aggregate, then the size operand must equal the size
  //     of the field.
  //   or
  //  - The size operand is 0.
  //
  // A necessary precursor for most of these rule is that the operand type
  // is able to be resolved to a unique dominant type for the pointer.
  //
  // For a safe call, the field information tracking of the aggregate type
  // will be updated to indicate the field is written to for the destination.
  // Fields of the source operand will not be updated to allow for fields
  // to be eliminated if they are never directly read.
  void analyzeMemcpyOrMemmove(IntrinsicInst &I,
                              dtrans::MemfuncCallInfo::MemfuncKind Kind) {
    LLVM_DEBUG(dbgs() << "dtrans: Analyzing memcpy/memmove call:\n  " << I
                      << "\n");
    assert(I.getNumArgOperands() >= 2);

    auto *DestArg = I.getArgOperand(0);
    auto *SrcArg = I.getArgOperand(1);
    Value *SetSize = I.getArgOperand(2);

    // A memcpy/memmove of 0 bytes will not affect the safety of any data
    // structure.
    if (dtrans::isValueEqualToSize(SetSize, 0))
      return;

    bool DestOfInterest = isValueOfInterest(DestArg);
    bool SrcOfInterest = isValueOfInterest(SrcArg);
    if (!DestOfInterest && !SrcOfInterest) {
      return;
    } else if (!SrcOfInterest || !DestOfInterest) {
      LLVM_DEBUG(
          dbgs() << "dtrans-safety: Bad memfunc manipulation --  "
                 << "Either source or destination operand is fundamental "
                    "pointer type:\n"
                 << "  " << I << "\n");

      setValueTypeInfoSafetyData(SrcArg, dtrans::BadMemFuncManipulation);
      setValueTypeInfoSafetyData(DestArg, dtrans::BadMemFuncManipulation);
      return;
    }

    // If we get here, both parameters are types of interest.
    LocalPointerInfo &DstLPI = LPA.getLocalPointerInfo(DestArg);
    LocalPointerInfo &SrcLPI = LPA.getLocalPointerInfo(SrcArg);
    auto *DestParentTy = DstLPI.getDominantAggregateTy();
    auto *SrcParentTy = SrcLPI.getDominantAggregateTy();

    if (!DestParentTy || !SrcParentTy) {
      if (!DestParentTy &&
          isAliasSetOverloaded(DstLPI.getPointerTypeAliasSet())) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Ambiguous pointer target -- "
                          << "Aliased type for destination operand:\n"
                          << "  " << I << "\n");

        setAllAliasedTypeSafetyData(DstLPI, dtrans::AmbiguousPointerTarget);
        setAllAliasedTypeSafetyData(SrcLPI, dtrans::BadMemFuncManipulation);
        return;
      } else if (!SrcParentTy &&
                 isAliasSetOverloaded(SrcLPI.getPointerTypeAliasSet())) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Bad memfunc manipulation -- "
                          << "Aliased type for source operand.\n"
                          << "  " << I << "\n");

        setAllAliasedTypeSafetyData(DstLPI, dtrans::BadMemFuncManipulation);
        setAllAliasedTypeSafetyData(SrcLPI, dtrans::AmbiguousPointerTarget);
        return;
      } else {
        // If the dominant type was not identified, and the pointer is not
        // an aliased type, we expect that we are dealing with a pointer
        // to a member element. assert this to be sure.
        if ((!DestParentTy && !DstLPI.pointsToSomeElement()) ||
            (!SrcParentTy && !SrcLPI.pointsToSomeElement())) {
          setAllAliasedTypeSafetyData(DstLPI, dtrans::UnhandledUse);
          setAllAliasedTypeSafetyData(SrcLPI, dtrans::UnhandledUse);
          return;
        }
      }
    }

    if (DestParentTy != SrcParentTy) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Bad memfunc manipulation -- "
                        << "Different types for source and destination:\n"
                        << "  " << I << "\n");

      setValueTypeInfoSafetyData(DestArg, dtrans::BadMemFuncManipulation);
      setValueTypeInfoSafetyData(SrcArg, dtrans::BadMemFuncManipulation);
      return;
    }

    bool DstPtrToMember = DstLPI.pointsToSomeElement();
    bool SrcPtrToMember = SrcLPI.pointsToSomeElement();

    // For simplicity, require either both elements to be pointers to members,
    // or neither element to be. This is a conservative approach, but otherwise
    // any transforms will have to deal the complexity of the types when
    // memcpy/memmove calls have to be modified.
    if (DstPtrToMember != SrcPtrToMember) {
      setValueTypeInfoSafetyData(DestArg, dtrans::BadMemFuncManipulation);
      setValueTypeInfoSafetyData(SrcArg, dtrans::BadMemFuncManipulation);
      return;
    }

    if (DstPtrToMember) {
      StructType *DstStructTy = nullptr;
      StructType *SrcStructTy = nullptr;
      size_t DstFieldNum = 0;
      size_t SrcFieldNum = 0;

      // Check if the pointer-to-member is a member of structure that can
      // be analyzed.
      bool DstSimple =
          isSimpleStructureMember(DstLPI, &DstStructTy, &DstFieldNum);

      if (!DstSimple) {
        // The pointer to member was not able to be analyzed. It could be a
        // member of an array type, or the element pointee set contained
        // multiple entries.
        if (DstLPI.getElementPointeeSet().size() != 1) {
          LLVM_DEBUG(dbgs()
                     << "dtrans-safety: Ambiguous pointer target -- "
                     << "Pointer to member with multiple potential target "
                        "members (destination operand):\n"
                     << "  " << I << "\n");

          setValueTypeInfoSafetyData(DestArg, dtrans::AmbiguousPointerTarget);
          setValueTypeInfoSafetyData(SrcArg, dtrans::AmbiguousPointerTarget);
        } else {
          // This could be extended in the future to handle the case that it was
          // member of an array.
          LLVM_DEBUG(
              dbgs()
              << "dtrans-safety: Unhandled use -- "
              << "Pointer to member of array type (destination operand):\n"
              << "  " << I << "\n");

          setValueTypeInfoSafetyData(DestArg, dtrans::UnhandledUse);
          setValueTypeInfoSafetyData(SrcArg, dtrans::UnhandledUse);
        }

        return;
      }

      bool SrcSimple =
          isSimpleStructureMember(SrcLPI, &SrcStructTy, &SrcFieldNum);
      if (!SrcSimple) {
        // The destination was found to be a member of a structure. The
        // copy/move is only supported when the source is also a simple
        // structure member.

        // The pointer to member was not able to be analyzed. It could be a
        // member of an array type, or the element pointee set contained
        // multiple entries.
        if (SrcLPI.getElementPointeeSet().size() != 1) {
          LLVM_DEBUG(dbgs()
                     << "dtrans-safety: Ambiguous pointer target -- "
                     << "Pointer to member with multiple potential target "
                        "members (source operand):\n"
                     << "  " << I << "\n");

          setValueTypeInfoSafetyData(DestArg, dtrans::AmbiguousPointerTarget);
          setValueTypeInfoSafetyData(SrcArg, dtrans::AmbiguousPointerTarget);
        } else {
          // This could be extended in the future to handle the case where an
          // array that is not within a structure is copied to an array
          // that is within a structure.
          LLVM_DEBUG(dbgs()
                     << "dtrans-safety: Unhandled use -- "
                     << "Pointer to member of array type (source operand): \n"
                     << "  " << I << "\n");

          setValueTypeInfoSafetyData(DestArg, dtrans::UnhandledUse);
          setValueTypeInfoSafetyData(SrcArg, dtrans::UnhandledUse);
        }

        return;
      }

      // It is probably safe to copy from one set of fields to a different set
      // of fields in the structure, if the data types for each source and
      // destination element match, but to keep things simple for the
      // transformations, we will currently require the same source and
      // destination types and fields when processing the pointer to member
      // case.
      if (DstStructTy == SrcStructTy && DstFieldNum == SrcFieldNum) {
        // The structures for the source and destination match, so we only need
        // to populate a RegionDesc structure for the destination.
        dtrans::MemfuncRegion RegionDesc;
        if (analyzeMemfuncStructureMemberParam(
                I, DstStructTy, DstFieldNum, SetSize,
                /*IsValuePreservingWrite=*/true, RegionDesc))
          createMemcpyOrMemmoveCallInfo(I, DstStructTy->getPointerTo(), Kind,
                                        RegionDesc, RegionDesc);

        return;
      } else {
        LLVM_DEBUG(
            dbgs() << "dtrans-safety: Bad memfunc manipulation -- "
                   << "source and destination pointer to member types or "
                      "offsets do not match:\n"
                   << "  " << I << "\n");

        setBaseTypeInfoSafetyData(DstStructTy, dtrans::BadMemFuncManipulation);
        setBaseTypeInfoSafetyData(SrcStructTy, dtrans::BadMemFuncManipulation);
      }

      return;
    }

    // The operand is not a pointer to member if we reach this point,
    // and the source and destination types are the same.
    auto *DestPointeeTy = DestParentTy->getPointerElementType();
    uint64_t ElementSize = DL.getTypeAllocSize(DestPointeeTy);

    if (!DestPointeeTy->isAggregateType())
      return;

    // Consider the case where the complete aggregate (or an array of
    // aggregates is being set.
    if (dtrans::isValueMultipleOfSize(SetSize, ElementSize)) {
      // It is a safe use. Mark all the fields as being written.
      auto *ParentTI = DTInfo.getOrCreateTypeInfo(DestPointeeTy);
      markAllFieldsWritten(ParentTI);

      // The copy/move is the complete aggregate of the source and destination,
      // which are the same types/
      dtrans::MemfuncRegion RegionDesc;
      RegionDesc.IsCompleteAggregate = true;
      createMemcpyOrMemmoveCallInfo(I, DestParentTy, Kind, RegionDesc,
                                    RegionDesc);

      return;
    }

    // Consider the case where a portion of a structure is being set, starting
    // from the 1st field.
    if (auto *StructTy = dyn_cast<StructType>(DestPointeeTy)) {
      dtrans::MemfuncRegion RegionDesc;
      if (analyzeMemfuncStructureMemberParam(I, StructTy, 0, SetSize,
                                             /*IsValuePreservingWrite=*/true,
                                             RegionDesc))
        createMemcpyOrMemmoveCallInfo(I, StructTy->getPointerTo(), Kind,
                                      RegionDesc, RegionDesc);
      return;
    }

    LLVM_DEBUG(dbgs() << "dtrans-safety: Bad memfunc size -- "
                      << "size is not a multiple of type size:\n"
                      << "  " << I << "\n");

    setValueTypeInfoSafetyData(DestArg, dtrans::BadMemFuncSize);
  }

  // This function is used to set the type information captured in the
  // LPI structure into the CallInfo structure that is going to be exposed
  // to the transforms.
  void populateCallInfoFromLPI(LocalPointerInfo &LPI, dtrans::CallInfo *CI) {
    CI->setAnalyzed(true);
    if (LPI.canAliasToAggregatePointer()) {
      CI->setAliasesToAggregatePointer(true);
      LocalPointerInfo::PointerTypeAliasSetRef &AliasSet =
          LPI.getPointerTypeAliasSet();
      for (auto *Ty : AliasSet) {
        if (DTInfo.isTypeOfInterest(Ty)) {
          CI->addType(Ty);
        }
      }
    }
  }

  // Create a MemfuncCallInfo object that will store the details about a safe
  // memset call.
  void createMemsetCallInfo(Instruction &I, llvm::Type *Ty,
                            dtrans::MemfuncRegion &RegionDesc) {
    dtrans::MemfuncCallInfo *MCI = DTInfo.createMemfuncCallInfo(
        &I, dtrans::MemfuncCallInfo::MK_Memset, RegionDesc);
    MCI->setAliasesToAggregatePointer(true);
    MCI->setAnalyzed(true);
    MCI->addType(Ty);
  }

  // Create a MemfuncCallInfo object that will store the details about a safe
  // memcpy/memmove call.
  void createMemcpyOrMemmoveCallInfo(Instruction &I, llvm::Type *Ty,
                                     dtrans::MemfuncCallInfo::MemfuncKind Kind,
                                     dtrans::MemfuncRegion &RegionDescDest,
                                     dtrans::MemfuncRegion &RegionDescSrc) {
    dtrans::MemfuncCallInfo *MCI =
        DTInfo.createMemfuncCallInfo(&I, Kind, RegionDescDest, RegionDescSrc);
    MCI->setAliasesToAggregatePointer(true);
    MCI->setAnalyzed(true);
    MCI->addType(Ty);
  }

  // Helper function for retrieving information when the \p LPI argument refers
  // to a pointer-to-member element. This function checks that the
  // pointer to member is a referencing a single member from a single structure.
  // If so, it returns 'true', and saves the structure type in the \p StructTy
  // argument, and the field number in the \p FieldNum argument. Otherwise,
  // return 'false'.
  bool isSimpleStructureMember(LocalPointerInfo &LPI, StructType **StructTy,
                               size_t *FieldNum) {
    assert(LPI.pointsToSomeElement());

    *StructTy = nullptr;
    *FieldNum = SIZE_MAX;

    auto &ElementPointees = LPI.getElementPointeeSet();
    if (ElementPointees.size() != 1)
      return false;

    auto &PointeePair = *(ElementPointees.begin());

    // If the element is the first element of the structure, it is necessary
    // to check for the possibility that it is a structure that started with
    // a character array to find the underlying structure type.
    // For example:
    //   %struct.test08 = type { [200 x i8], [200 x i8], i64 }
    //   call void @llvm.memset.p0i8.i64(i8* getelementptr(
    //        %struct.test08, %struct.test08* @test08var, i64 0, i32 0, i64 0),
    //       i8 0, i64 200, i32 4, i1 false)
    if (PointeePair.second == 0) {
      if (isAliasSetOverloaded(LPI.getPointerTypeAliasSet()))
        return false;

      for (auto *AliasTy : LPI.getPointerTypeAliasSet()) {
        if (dtrans::isElementZeroAccess(AliasTy,
                                        PointeePair.first->getPointerTo()) &&
            AliasTy->getPointerElementType()->isStructTy()) {
          *StructTy = cast<StructType>(AliasTy->getPointerElementType());
          *FieldNum = 0;
          return true;
        }
      }
    }

    // It's not the special case, so just get the type and field number if it's
    // a struct.
    Type *Ty = PointeePair.first;
    if (Ty->isStructTy()) {
      *StructTy = cast<StructType>(Ty);
      *FieldNum = PointeePair.second;
      return true;
    }

    return false;
  }

  // Analyze a structure pointer that is passed to memfunc call, possibly using
  // a pointer to one of the fields within the structure to determine which
  // fields are modified, and whether it is a safe usage. Return 'true' if safe
  // usage, and populate the \p RegionDesc with the results.
  bool analyzeMemfuncStructureMemberParam(Instruction &I, StructType *StructTy,
                                          size_t FieldNum, Value *SetSize,
                                          bool IsValuePreservingWrite,
                                          dtrans::MemfuncRegion &RegionDesc) {
    // Try to determine if a set of fields in a structure is being written.
    if (analyzePartialStructUse(StructTy, FieldNum, SetSize, &RegionDesc)) {
      auto *ParentTI = DTInfo.getOrCreateTypeInfo(StructTy);

      // If not all members of the structure were set, mark it as
      // a partial write.
      if (!RegionDesc.IsCompleteAggregate) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Memfunc partial write -- "
                          << "size is a subset of fields:\n"
                          << "  " << I << "\n");

        ParentTI->setSafetyData(dtrans::MemFuncPartialWrite);
      }
      markStructFieldsWritten(ParentTI, RegionDesc.FirstField,
                              RegionDesc.LastField);

      // A copy is considered as preserving the single value analysis info.
      // However, for a memset, it is not known whether the value changed
      // without checking the value being set.
      if (!IsValuePreservingWrite)
        markAllFieldsMultipleValue(ParentTI);
    } else {
      // The size could not be matched to the fields of the structure.
      LLVM_DEBUG(dbgs() << "dtrans-safety: Bad memfunc size -- "
                        << "size does not equal member field type(s) size:\n"
                        << "  " << I << "\n");

      setBaseTypeInfoSafetyData(StructTy, dtrans::BadMemFuncSize);
      return false;
    }

    return true;
  }

  // Wrapper function for analyzing structure field access which prepares
  // parameters for that function.
  bool analyzePartialStructUse(StructType *StructTy, size_t FieldNum,
                               const Value *AccessSizeVal,
                               dtrans::MemfuncRegion *RegionDesc) {
    if (!StructTy)
      return false;

    if (!AccessSizeVal)
      return false;

    auto *AccessSizeCI = dyn_cast<ConstantInt>(AccessSizeVal);
    if (!AccessSizeCI)
      return false;

    uint64_t AccessSize = AccessSizeCI->getLimitedValue();
    assert(FieldNum < StructTy->getNumElements());

    return analyzeStructFieldAccess(StructTy, FieldNum, AccessSize, RegionDesc);
  }

  // Helper to analyze a pointer-to-member usage to determine if only a
  // specific subset of the structure fields of \p StructTy, starting from \p
  // FieldNum and extending by \p AccessSize bytes of the structure are
  // touched.
  //
  // Return 'true' if it can be resolved to precisely match one or more
  // adjacent fields starting with the field number identified in the 'LPI'.
  // If so, also updated the RegionDesc to set the starting index into
  // 'FirstField' and the ending index of affected fields into 'LastField'.
  // Otherwise, return 'false'.
  bool analyzeStructFieldAccess(StructType *StructTy, size_t FieldNum,
                                uint64_t AccessSize,
                                dtrans::MemfuncRegion *RegionDesc) {
    uint64_t TypeSize = DL.getTypeAllocSize(StructTy);

    // If the size is larger than the base structure size, then the write
    // exceeds the bounds of a single structure, and it's an unsupported
    // use.
    if (AccessSize > TypeSize)
      return false;

    // Try to identify the range of fields being accessed based on the
    // layout of the structure.
    auto FieldTypes = StructTy->elements();
    auto *SL = DL.getStructLayout(StructTy);
    uint64_t Offset = SL->getElementOffset(FieldNum);

    uint64_t LastOffset = Offset + AccessSize - 1;
    if (LastOffset > TypeSize)
      return false;

    unsigned int LF = SL->getElementContainingOffset(LastOffset);

    // Check if the last field was completely covered. If not, we do not
    // support it. It could be safe, but could complicate transforms that need
    // to work with nested structures.
    uint64_t LastFieldStart = SL->getElementOffset(LF);
    uint64_t LastFieldSize = DL.getTypeStoreSize(FieldTypes[LF]);
    if (LastOffset < (LastFieldStart + LastFieldSize - 1))
      return false;

    RegionDesc->FirstField = FieldNum;
    RegionDesc->LastField = LF;
    if (!(FieldNum == 0 && LF == (StructTy->getNumElements() - 1)))
      RegionDesc->IsCompleteAggregate = false;
    else
      RegionDesc->IsCompleteAggregate = true;
    return true;
  }

  // Mark all the fields of the type, and fields of aggregates the type contains
  // as written.
  void markAllFieldsWritten(dtrans::TypeInfo *TI) {
    if (TI == nullptr)
      return;

    if (!TI->getLLVMType()->isAggregateType()) {
      return;
    }

    if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI)) {
      for (auto &FI : StInfo->getFields()) {
        FI.setWritten(true);
        auto *ComponentTI = DTInfo.getTypeInfo(FI.getLLVMType());
        markAllFieldsWritten(ComponentTI);
      }
    } else if (auto *AInfo = dyn_cast<dtrans::ArrayInfo>(TI)) {
      auto *ComponentTI = AInfo->getElementDTransInfo();
      markAllFieldsWritten(ComponentTI);
    }

    return;
  }

  // A specialized form of the MarkAllFieldsWritten that is used to mark a
  // subset of fields of a structure type as written. Any contained aggregates
  // within the subset are marked as completely written.
  void markStructFieldsWritten(dtrans::TypeInfo *TI, unsigned int FirstField,
                               unsigned int LastField) {
    assert(TI && TI->getLLVMType()->isStructTy() &&
           "markStructFieldsWritten requires Structure type");

    auto *StInfo = cast<dtrans::StructInfo>(TI);
    assert(LastField >= FirstField && LastField < StInfo->getNumFields() &&
           "markStructFieldsWritten with invalid field index");

    for (unsigned int Idx = FirstField; Idx <= LastField; ++Idx) {
      auto &FI = StInfo->getField(Idx);
      FI.setWritten(true);
      auto *ComponentTI = DTInfo.getTypeInfo(FI.getLLVMType());
      markAllFieldsWritten(ComponentTI);
    }

    return;
  }

  //
  // Update the "single value" info for GVElemTy, given that it has the
  // indicated Init.
  //
  void analyzeGlobalStructSingleValue(llvm::Type *GVElemTy,
                                      llvm::Constant *Init) {
    if (auto *StTy = dyn_cast<StructType>(GVElemTy)) {
      auto *StInfo = cast<dtrans::StructInfo>(DTInfo.getOrCreateTypeInfo(StTy));
      for (unsigned I = 0, E = StInfo->getNumFields(); I != E; ++I) {
        llvm::Type *FieldTy = StTy->getTypeAtIndex(I);
        llvm::Constant *ConstVal = Init->getAggregateElement(I);
        dtrans::FieldInfo &FI = StInfo->getField(I);
        analyzeGlobalStructSingleValue(FieldTy, ConstVal);
        LLVM_DEBUG(dbgs() << "dtrans-fsv: " << *(StInfo->getLLVMType()) << " ["
                          << I << "] ");
        if (ConstVal->getType() == FieldTy) {
          LLVM_DEBUG(ConstVal->printAsOperand(dbgs()));
          FI.processNewSingleValue(ConstVal);
          LLVM_DEBUG(dbgs() << (FI.isMultipleValue() ? " <MULTIPLE>\n" : "\n"));
        } else {
          LLVM_DEBUG(dbgs() << "<MULTIPLE>\n");
          FI.setMultipleValue();
        }
        if (!isa<ConstantPointerNull>(ConstVal)) {
          if (!FI.isBottomAllocFunction())
            LLVM_DEBUG(dbgs() << "dtrans-fsaf: " << *(StInfo->getLLVMType())
                              << " [" << I << "] <BOTTOM>\n");
          FI.setBottomAllocFunction();
        }
      }
    } else if (auto *ArTy = dyn_cast<ArrayType>(GVElemTy)) {
      auto *ArInfo = cast<dtrans::ArrayInfo>(DTInfo.getOrCreateTypeInfo(ArTy));
      auto *ComponentTI = ArInfo->getElementDTransInfo();
      llvm::Type *ComponentTy = ComponentTI->getLLVMType();
      for (unsigned I = 0, E = ArInfo->getNumElements(); I != E; ++I) {
        llvm::Constant *ConstVal = Init->getAggregateElement(I);
        analyzeGlobalStructSingleValue(ComponentTy, ConstVal);
      }
    }
  }

  //
  // Mark all fields of TI to have multiple values.
  //
  void markAllFieldsMultipleValue(dtrans::TypeInfo *TI) {
    if (TI == nullptr)
      return;
    if (!TI->getLLVMType()->isAggregateType())
      return;
    if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI)) {
      int Count = 0;
      for (auto &FI : StInfo->getFields()) {
        LLVM_DEBUG(dbgs() << "dtrans-fsv: " << *(StInfo->getLLVMType()) << " ["
                          << Count << "] <MULTIPLE>\n");
        FI.setMultipleValue();
        auto *ComponentTI = DTInfo.getTypeInfo(FI.getLLVMType());
        markAllFieldsMultipleValue(ComponentTI);
        ++Count;
      }
    } else if (auto *AInfo = dyn_cast<dtrans::ArrayInfo>(TI)) {
      auto *ComponentTI = AInfo->getElementDTransInfo();
      markAllFieldsMultipleValue(ComponentTI);
    }
  }

  void analyzeSub(BinaryOperator &I) {
    assert(I.getOpcode() == Instruction::Sub &&
           "analyzeSub() called with unexpected opcode");

    // If neither operand is of interest, we can ignore this instruction.
    if (!isValueOfInterest(I.getOperand(0)) &&
        !isValueOfInterest(I.getOperand(1)))
      return;

    LocalPointerInfo &LHSLPI = LPA.getLocalPointerInfo(I.getOperand(0));
    LocalPointerInfo &RHSLPI = LPA.getLocalPointerInfo(I.getOperand(1));

    if (LHSLPI.pointsToSomeElement()) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Bad pointer manipulation -- "
                        << "pointer to element used in sub instruction:\n"
                        << "  " << I << "\n");
      // Selects and PHIs may have created a pointer that refers to
      // elements in multiple aggregate types. This sets the bad pointer
      // manipulation condition for them all.
      for (auto &PointeePair : LHSLPI.getElementPointeeSet()) {
        setBaseTypeInfoSafetyData(PointeePair.first,
                                  dtrans::BadPtrManipulation);
      }
    }
    if (RHSLPI.pointsToSomeElement()) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Bad pointer manipulation -- "
                        << "pointer to element used in sub instruction:\n"
                        << "  " << I << "\n");
      // Selects and PHIs may have created a pointer that refers to
      // elements in multiple aggregate types. This sets the bad pointer
      // manipulation condition for them all.
      for (auto &PointeePair : RHSLPI.getElementPointeeSet()) {
        setBaseTypeInfoSafetyData(PointeePair.first,
                                  dtrans::BadPtrManipulation);
      }
    }

    if (!pointerAliasSetsAreEqual(LHSLPI.getPointerTypeAliasSet(),
                                  RHSLPI.getPointerTypeAliasSet())) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
                        << "sub instruction operands do not match:\n"
                        << "  " << I << "\n");
      setValueTypeInfoSafetyData(I.getOperand(0), dtrans::UnhandledUse);
      setValueTypeInfoSafetyData(I.getOperand(1), dtrans::UnhandledUse);
      return;
    }

    // A subtract instruction can safely be used to compute the index offset
    // between to pointers in a fixed array, but only if the result is
    // divided by the size of the structure. If these were pointers-to-pointers
    // that's not a concern, but if they are pointers to structures we need
    // to look for the divide.
    //
    // If DomTy is null here, either the alias set is overloaded, which will
    // have been reported at the PHI or select that created the condition or
    // one or both pointers are pointers to some element and we already set
    // the necessary safety condition above.
    if (auto *DomTy = LHSLPI.getDominantAggregateTy()) {
      // Since we won't get here unless the alias sets are equal, the
      // dominant types must match.
      assert(RHSLPI.getDominantAggregateTy() == DomTy &&
             "Unexpected dominant type mismatch");
      assert(DomTy->isPointerTy() && "Pointer sub analysis of non-pointers!");

      // If the dominant type is a pointer to a pointer, we don't need to look
      // at its uses.
      llvm::Type *ElementTy = DomTy->getPointerElementType();
      if (!ElementTy->isPointerTy()) {
        uint64_t ElementSize = DL.getTypeAllocSize(ElementTy);
        if (hasNonDivBySizeUses(&I, ElementSize)) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Bad pointer manipulation -- "
                            << "Pointer subtract result has non-div use:\n"
                            << "  " << I << "\n");
          // Both operands have the same alias set, so we only need to set the
          // safety condition once.
          setAllAliasedTypeSafetyData(LHSLPI, dtrans::BadPtrManipulation);
        } else {
          // Otherwise, add a hint in DTInfo that will make it easier for
          // optimizations to identify the type of the element whose
          // pointers were subtracted.
          DTInfo.addPtrSubMapping(&I, ElementTy);
        }
      }
    }
  }

  bool hasNonDivBySizeUses(Value *V, uint64_t Size) {
    for (auto *U : V->users()) {
      if (auto *BinOp = dyn_cast<BinaryOperator>(U)) {
        if (BinOp->getOpcode() != Instruction::SDiv &&
            BinOp->getOpcode() != Instruction::UDiv)
          return true;
        if (BinOp->getOperand(0) != V)
          return true;
        if (!dtrans::isValueMultipleOfSize(BinOp->getOperand(1), Size))
          return true;
        continue;
      }
      LLVM_DEBUG(dbgs() << "Non-div use found for pointer sub: " << *U << "\n");
      return true;
    }
    return false;
  }

  void analyzeBitMask(BinaryOperator &I) {
    assert((I.getOpcode() == Instruction::Or ||
            I.getOpcode() == Instruction::And ||
            I.getOpcode() == Instruction::Xor) &&
           "analyzeBitMask() called with unexpected opcode");

    // If neither operand is of interest, we can ignore this instruction.
    Value *Op0 = I.getOperand(0);
    Value *Op1 = I.getOperand(1);
    if (!isValueOfInterest(Op0) && !isValueOfInterest(Op1))
      return;

    // If both operands are values of interest, something is happening that
    // we didn't expect.
    if (isValueOfInterest(Op0) && isValueOfInterest(Op1)) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
                        << "bitmask instruction with unexpected operands:\n"
                        << "  " << I << "\n");
      setValueTypeInfoSafetyData(Op0, dtrans::UnhandledUse);
      setValueTypeInfoSafetyData(Op1, dtrans::UnhandledUse);
      return;
    }

    // Check to see if this instruction is part of a chain of bitmasks that
    // leads to a compare and has no other uses. If so, it's probably part of
    // some kind of alignment check. Otherwise, mark it as an unhandled use.
    if (!isBitmaskAndCompareSequenceOnly(&I)) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
                        << "bitmask instruction has unexpected uses:\n"
                        << "  " << I << "\n");
      setValueTypeInfoSafetyData(Op0, dtrans::UnhandledUse);
      setValueTypeInfoSafetyData(Op1, dtrans::UnhandledUse);
      return;
    }
  }

  bool isBitmaskAndCompareSequenceOnly(BinaryOperator *I) {
    SmallVector<Instruction *, 4> WorkList;
    WorkList.push_back(I);

    auto hasInstConstOperands = [](Instruction *I) {
      // For the pattern to be match, one operand must be an instruction
      // and the other must be a constant.
      Value *Op0 = I->getOperand(0);
      Value *Op1 = I->getOperand(1);
      return ((isa<Instruction>(Op0) && isa<ConstantInt>(Op1)) ||
              (isa<Instruction>(Op1) && isa<ConstantInt>(Op0)));
    };

    while (!WorkList.empty()) {
      Instruction *NextI = WorkList.back();
      WorkList.pop_back();
      switch (NextI->getOpcode()) {
      default:
        return false;
      case Instruction::Or:
      case Instruction::And:
      case Instruction::Xor:
        if (!hasInstConstOperands(NextI))
          return false;
        for (auto *U : NextI->users()) {
          // We don't need to check the users of the ICmp.
          if (auto *Cmp = dyn_cast<ICmpInst>(U)) {
            if (!hasInstConstOperands(Cmp))
              return false;
            continue;
          }
          // We do need to check the users of binary operators.
          if (isa<BinaryOperator>(U)) {
            WorkList.push_back(cast<Instruction>(U));
            continue;
          }
          // Anything else breaks the pattern.
          return false;
        }
      }
    }

    return true;
  }

  bool pointerAliasSetsAreEqual(LocalPointerInfo::PointerTypeAliasSetRef Set1,
                                LocalPointerInfo::PointerTypeAliasSetRef Set2) {
    // If the number of aliases is not the same, the sets cannot be equal.
    if (Set1.size() != Set2.size())
      return false;

    // Check to make sure each type aliased by V1 are also aliased by V2.
    // This looks expensive, but typically we will have no more than five
    // aliased types.
    for (auto *AliasTy : Set1)
      if (!Set2.count(AliasTy))
        return false;

    // If we got here, the alias sets match.
    return true;
  }

  void setBinaryOperatorUnhandledUse(BinaryOperator &I) {
    // It isn't possible for binary operators to return pointers or
    // aggregate types.
    assert(!DTInfo.isTypeOfInterest(I.getType()) &&
           "Unexpected return type for binary operator");

    for (Value *Arg : I.operands()) {
      if (isValueOfInterest(Arg)) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
                          << "Type used by an unmodeled binary operator:\n"
                          << "  " << I << "\n");
        setValueTypeInfoSafetyData(Arg, dtrans::UnhandledUse);
      }
    }
  }

  // In many cases we need to set safety data based on a value that
  // was derived from a pointer to a type of interest, via a bitcast
  // or a ptrtoint cast. In those cases, this function is called to
  // propagate safety data to the interesting type.
  // Return false if V was not a value of interest, true otherwise.
  bool setValueTypeInfoSafetyDataBase(Value *V, dtrans::SafetyData Data) {
    // In some cases this function might have been called for multiple
    // operands, not all of which we are actually tracking.
    if (!isValueOfInterest(V))
      return false;

    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(V);
    setAllAliasedTypeSafetyData(LPI, Data);
    return true;
  }

  // Also propagate the safety data to sibling types and types nested
  // within the siblings, when appropriate.
  void setValueTypeInfoSafetyData(Value *V, dtrans::SafetyData Data) {

    if (!setValueTypeInfoSafetyDataBase(V, Data))
      return;

    // If the value is a pointer to an element in some aggregate type
    // set the safety info for that type also.
    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(V);
    auto ElementPointees = LPI.getElementPointeeSet();
    if (ElementPointees.size() > 0)
      for (auto &PointeePair : ElementPointees)
        setBaseTypeInfoSafetyData(PointeePair.first, Data);
  }

  // Given LocalPointerInfo for a value, set the specified safety data
  // for the base type of every type which is known to alias to the value.
  void setAllAliasedTypeSafetyData(LocalPointerInfo &LPI,
                                   dtrans::SafetyData Data) {
    for (auto *Ty : LPI.getPointerTypeAliasSet())
      if (DTInfo.isTypeOfInterest(Ty))
        setBaseTypeInfoSafetyData(Ty, Data);
  }

  // Return true if 'Data' should be propagated down to all types nested
  // within some type for which the safety condition was found to hold.
  // The motivation for this propagation is that a user may access outside
  // the bounds of a structure. This is strictly not allowed in C/C++, but
  // is allowed under the defintion of LLVM IR.
  bool isCascadingSafetyCondition(dtrans::SafetyData Data) {
    if (DTransOutOfBoundsOK)
      return true;
    switch (Data) {
    // We can add additional cases here to reduce the conservative behavior
    // as needs dictate.
    case dtrans::FieldAddressTaken:
      return false;
    }
    return true;
  }

  // This is a helper function that retrieves the aggregate type through
  // zero or more layers of indirection and sets the specified safety data
  // for that type.
  void setBaseTypeInfoSafetyData(llvm::Type *Ty, dtrans::SafetyData Data) {
    llvm::Type *BaseTy = Ty;
    while (BaseTy->isPointerTy())
      BaseTy = BaseTy->getPointerElementType();
    dtrans::TypeInfo *TI = DTInfo.getOrCreateTypeInfo(BaseTy);
    TI->setSafetyData(Data);
    if (!isCascadingSafetyCondition(Data))
      return;

    // This lambda encapsulates the logic for propagating safety conditions to
    // structure field or array element types. If the field or element is an
    // instance of a type of interest, and not if it is merely a pointer to
    // such a type, the condition is propagated. If the field is a pointer,
    // the condition is only propagated if it is AddressTaken. Propagation is
    // done via a recursive call to setBaseTypeInfoSafetyData in order to
    // handle additional levels of nesting.
    auto maybePropagateSafetyCondition = [this](llvm::Type *FieldTy,
                                                dtrans::SafetyData Data) {
      // If FieldTy is not a type of interest, there's no need to propagate.
      if (!DTInfo.isTypeOfInterest(FieldTy))
        return;
      // If the field is an instance of the type, propagate the condition.
      if (!FieldTy->isPointerTy()) {
        setBaseTypeInfoSafetyData(FieldTy, Data);
      } else if (Data == dtrans::AddressTaken) {
        // In the case of AddressTaken, we need to propagate the condition
        // even to fields that are pointers to structures, but in order
        // to avoid infinite loops in the case where two structures each
        // have pointers to the other we need to avoid doing this for
        // structures that already have the condition set.
        llvm::Type *FieldBaseTy = FieldTy;
        while (FieldBaseTy->isPointerTy())
          FieldBaseTy = FieldBaseTy->getPointerElementType();
        dtrans::TypeInfo *FieldTI = DTInfo.getOrCreateTypeInfo(FieldBaseTy);
        if (!FieldTI->testSafetyData(Data))
          setBaseTypeInfoSafetyData(FieldBaseTy, Data);
      }
    };

    // Propagate this condition to any nested types.
    if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI))
      for (dtrans::FieldInfo &FI : StInfo->getFields())
        maybePropagateSafetyCondition(FI.getLLVMType(), Data);
    else if (isa<dtrans::ArrayInfo>(TI))
      maybePropagateSafetyCondition(BaseTy->getArrayElementType(), Data);
  }
};

} // end anonymous namespace

// Return true if we are interested in tracking values of the specified type.
//
// For now, let's limit this to aggregates and various levels of indirection
// to aggregates. At some point we may also be interested in pointers to
// scalars.
bool DTransAnalysisInfo::isTypeOfInterest(llvm::Type *Ty) {
  llvm::Type *BaseTy = Ty;

  // For pointers, see what they point to.
  while (BaseTy->isPointerTy())
    BaseTy = cast<PointerType>(BaseTy)->getElementType();

  return BaseTy->isAggregateType();
}

dtrans::TypeInfo *DTransAnalysisInfo::getTypeInfo(llvm::Type *Ty) const {
  // If we have this type in our map, return it.
  auto IT = TypeInfoMap.find(Ty);
  if (IT != TypeInfoMap.end())
    return IT->second;
  // If not, return nullptr.
  return nullptr;
}

dtrans::TypeInfo *
DTransAnalysisInfo::getOrCreateTypeInfoForArray(llvm::Type *Ty,
                                                uint64_t NumElements) {
  // We saw an allocation that was cast to a pointer, but we want to treat it
  // as an array. To do that, we'll create an LLVM array type to describe it.
  llvm::ArrayType *ArrTy = llvm::ArrayType::get(Ty, NumElements);
  return getOrCreateTypeInfo(ArrTy);
}

dtrans::TypeInfo *DTransAnalysisInfo::getOrCreateTypeInfo(llvm::Type *Ty) {
  // If we already have this type in our map, just return it.
  auto TI = getTypeInfo(Ty);
  if (TI)
    return TI;

  // Create the dtrans type info for this type and any sub-types.
  dtrans::TypeInfo *DTransTy;
  if (Ty->isPointerTy()) {
    // For pointer types, we want to record the pointer type info
    // and then record what it points to. We must add the pointer to the
    // map early in this case to avoid infinite recursion.
    DTransTy = new dtrans::PointerInfo(Ty);
    TypeInfoMap[Ty] = DTransTy;
    (void)getOrCreateTypeInfo(cast<PointerType>(Ty)->getElementType());
    return DTransTy;
  } else if (Ty->isArrayTy()) {
    dtrans::TypeInfo *ElementInfo =
        getOrCreateTypeInfo(Ty->getArrayElementType());
    DTransTy =
        new dtrans::ArrayInfo(Ty, ElementInfo, Ty->getArrayNumElements());
  } else if (Ty->isStructTy()) {
    SmallVector<llvm::Type *, 16> FieldTypes;
    for (llvm::Type *FieldTy : cast<StructType>(Ty)->elements()) {
      FieldTypes.push_back(FieldTy);
      // Create a DTrans type for the field, in case it is an aggregate.
      (void)getOrCreateTypeInfo(FieldTy);
    }
    DTransTy = new dtrans::StructInfo(Ty, FieldTypes);
  } else {
    assert(!Ty->isAggregateType() &&
           "DTransAnalysisInfo::getOrCreateTypeInfo unexpected aggregate type");
    DTransTy = new dtrans::NonAggregateTypeInfo(Ty);
  }

  TypeInfoMap[Ty] = DTransTy;
  return DTransTy;
}

dtrans::CallInfo *DTransAnalysisInfo::getCallInfo(llvm::Instruction *I) {
  auto Entry = CallInfoMap.find(I);
  if (Entry == CallInfoMap.end())
    return nullptr;

  return Entry->second;
}

void DTransAnalysisInfo::addCallInfo(Instruction *I, dtrans::CallInfo *CI) {
  assert(getCallInfo(I) == nullptr &&
         "An instruction is only allowed a single CallInfo mapping");
  CallInfoMap[I] = CI;
}

dtrans::AllocCallInfo *
DTransAnalysisInfo::createAllocCallInfo(Instruction *I, dtrans::AllocKind AK) {
  dtrans::AllocCallInfo *Info = new dtrans::AllocCallInfo(I, AK);
  addCallInfo(I, Info);
  return Info;
}

dtrans::FreeCallInfo *
DTransAnalysisInfo::createFreeCallInfo(Instruction *I, dtrans::FreeKind FK) {
  dtrans::FreeCallInfo *Info = new dtrans::FreeCallInfo(I, FK);
  addCallInfo(I, Info);
  return Info;
}

dtrans::MemfuncCallInfo *DTransAnalysisInfo::createMemfuncCallInfo(
    Instruction *I, dtrans::MemfuncCallInfo::MemfuncKind MK,
    dtrans::MemfuncRegion &MR) {
  dtrans::MemfuncCallInfo *Info = new dtrans::MemfuncCallInfo(I, MK, MR);
  addCallInfo(I, Info);
  return Info;
}

dtrans::MemfuncCallInfo *DTransAnalysisInfo::createMemfuncCallInfo(
    Instruction *I, dtrans::MemfuncCallInfo::MemfuncKind MK,
    dtrans::MemfuncRegion &MR1, dtrans::MemfuncRegion &MR2) {
  dtrans::MemfuncCallInfo *Info = new dtrans::MemfuncCallInfo(I, MK, MR1, MR2);
  addCallInfo(I, Info);
  return Info;
}

void DTransAnalysisInfo::deleteCallInfo(Instruction *I) {
  dtrans::CallInfo *Info = getCallInfo(I);
  if (!Info)
    return;

  CallInfoMap.erase(I);
  delete Info;
}

void DTransAnalysisInfo::replaceCallInfoInstruction(dtrans::CallInfo *Info,
                                                    Instruction *NewI) {
  CallInfoMap.erase(Info->getInstruction());
  addCallInfo(NewI, Info);
  Info->setInstruction(NewI);
}

void DTransAnalysisInfo::addPtrSubMapping(llvm::BinaryOperator *BinOp,
                                          llvm::Type *Ty) {
  PtrSubInfoMap[BinOp] = Ty;
}

llvm::Type *DTransAnalysisInfo::getResolvedPtrSubType(BinaryOperator *BinOp) {
  auto It = PtrSubInfoMap.find(BinOp);
  if (It == PtrSubInfoMap.end())
    return nullptr;
  return It->second;
}

void DTransAnalysisInfo::addByteFlattenedGEPMapping(
    GetElementPtrInst *GEP, std::pair<llvm::Type *, size_t> Pointee) {
  ByteFlattenedGEPInfoMap[GEP] = Pointee;
}

std::pair<llvm::Type *, size_t>
DTransAnalysisInfo::getByteFlattenedGEPElement(GetElementPtrInst *GEP) {
  auto It = ByteFlattenedGEPInfoMap.find(GEP);
  if (It == ByteFlattenedGEPInfoMap.end())
    return std::make_pair(nullptr, 0);
  return It->second;
}

// Set the size used to increase the memory allocation for padded malloc
// and the interface generated by the optimization
void DTransAnalysisInfo::setPaddedMallocInfo(unsigned Size, Function *Fn) {
  PaddedMallocSize = Size;
  PaddedMallocInterface = Fn;
}

// Return the size used in the padded malloc optimizaton
unsigned DTransAnalysisInfo::getPaddedMallocSize() {
  return PaddedMallocSize;
}

// Return the interface generated by padded malloc function
llvm::Function *DTransAnalysisInfo::getPaddedMallocInterface() {
  return PaddedMallocInterface;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// Print the cached call info data, the type of call, and the function
/// making the call. This function is just for generating traces for testing.
void DTransAnalysisInfo::printCallInfo(raw_ostream &OS) {

  std::vector<std::tuple<StringRef, dtrans::CallInfo::CallInfoKind,
                         const Instruction *, dtrans::CallInfo *>>
      Entries;

  // To get some consistency in the printing order, populate a tuple
  // that can be sorted, then output the sorted list.
  for (auto &Entry : call_info_entries()) {
    Instruction *I = Entry->getInstruction();
    Entries.push_back(std::make_tuple(I->getParent()->getParent()->getName(),
                                      Entry->getCallInfoKind(), I, Entry));
  }

  std::sort(Entries.begin(), Entries.end());
  for (auto &Entry : Entries) {
    OS << "Function: " << std::get<0>(Entry) << "\n";
    OS << "Instruction: " << *std::get<2>(Entry) << "\n";
    std::get<3>(Entry)->print(OS);
    OS << "\n";
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Helper to invoke the right destructor object for destroying a CallInfo
// type object.
void DTransAnalysisInfo::destructCallInfo(dtrans::CallInfo *Info) {
  if (!Info)
    return;

  switch (Info->getCallInfoKind()) {
  case dtrans::CallInfo::CIK_Alloc:
    delete cast<dtrans::AllocCallInfo>(Info);
    break;
  case dtrans::CallInfo::CIK_Free:
    delete cast<dtrans::FreeCallInfo>(Info);
    break;
  case dtrans::CallInfo::CIK_Memfunc:
    delete cast<dtrans::MemfuncCallInfo>(Info);
    break;
  }
}

INITIALIZE_PASS_BEGIN(DTransAnalysisWrapper, "dtransanalysis",
                      "Data transformation analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(DTransAnalysisWrapper, "dtransanalysis",
                    "Data transformation analysis", false, true)

char DTransAnalysisWrapper::ID = 0;

ModulePass *llvm::createDTransAnalysisWrapperPass() {
  return new DTransAnalysisWrapper();
}

DTransAnalysisWrapper::DTransAnalysisWrapper() : ModulePass(ID) {
  initializeDTransAnalysisWrapperPass(*PassRegistry::getPassRegistry());
}

bool DTransAnalysisWrapper::doFinalization(Module &M) {
  Result.reset();
  return false;
}

bool DTransAnalysisWrapper::runOnModule(Module &M) {
  return Result.analyzeModule(
      M, getAnalysis<TargetLibraryInfoWrapperPass>().getTLI());
}

DTransAnalysisInfo::DTransAnalysisInfo() :
  PaddedMallocSize(0), PaddedMallocInterface(nullptr) {}

// Value map has a deleted move constructor, so we need a non-default
// implementation of ours.
DTransAnalysisInfo::DTransAnalysisInfo(DTransAnalysisInfo &&Other)
    : TypeInfoMap(std::move(Other.TypeInfoMap)),
      CallInfoMap(std::move(Other.CallInfoMap)) {
  // These two maps don't actually own any pointers, so it's OK to just copy
  // them.
  PtrSubInfoMap.insert(Other.PtrSubInfoMap.begin(), Other.PtrSubInfoMap.end());
  ByteFlattenedGEPInfoMap.insert(Other.ByteFlattenedGEPInfoMap.begin(),
                                 Other.ByteFlattenedGEPInfoMap.end());
}

DTransAnalysisInfo::~DTransAnalysisInfo() { reset(); }

DTransAnalysisInfo &DTransAnalysisInfo::operator=(DTransAnalysisInfo &&Other) {
  reset();
  TypeInfoMap = std::move(Other.TypeInfoMap);
  CallInfoMap = std::move(Other.CallInfoMap);
  PaddedMallocSize = Other.getPaddedMallocSize();
  PaddedMallocInterface = Other.getPaddedMallocInterface();
  return *this;
}

void DTransAnalysisInfo::reset() {
  // DTransAnalysisInfo owns the CallInfo pointers in the CallInfoMap.
  for (auto Info : CallInfoMap)
    destructCallInfo(Info.second);
  CallInfoMap.clear();

  // DTransAnalysisInfo owns the TypeInfo pointers in the TypeInfoMap.
  for (auto Entry : TypeInfoMap) {
    switch (Entry.second->getTypeInfoKind()) {
    case dtrans::TypeInfo::NonAggregateInfo:
      delete cast<dtrans::NonAggregateTypeInfo>(Entry.second);
      break;
    case dtrans::TypeInfo::PtrInfo:
      delete cast<dtrans::PointerInfo>(Entry.second);
      break;
    case dtrans::TypeInfo::StructInfo:
      delete cast<dtrans::StructInfo>(Entry.second);
      break;
    case dtrans::TypeInfo::ArrayInfo:
      delete cast<dtrans::ArrayInfo>(Entry.second);
      break;
    }
  }
  TypeInfoMap.clear();
}

bool DTransAnalysisInfo::analyzeModule(Module &M, TargetLibraryInfo &TLI) {
  DTransAllocAnalyzer DTAA(TLI);
  DTransInstVisitor Visitor(M.getContext(), *this, M.getDataLayout(), TLI,
                            DTAA);
  Visitor.visit(M);

  // Invalidate the fields for which the corresponding types do not pass
  // the SafetyData checks.
  for (auto *TI : type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (StInfo && StInfo->testSafetyData(dtrans::SDFieldSingleValue))
      for (unsigned I = 0, E = StInfo->getNumFields(); I != E; ++I)
        StInfo->getField(I).setMultipleValue();
    if (StInfo && StInfo->testSafetyData(dtrans::SDSingleAllocFunction))
      for (unsigned I = 0, E = StInfo->getNumFields(); I != E; ++I)
        StInfo->getField(I).setBottomAllocFunction();
  }

  // Set all aggregate fields conservatively as MultipleValue and
  // BottomAllocFunction for now.
  for (auto *TI : type_info_entries()) {
    if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI))
      for (unsigned I = 0, E = StInfo->getNumFields(); I != E; ++I)
        if (StInfo->getField(I).getLLVMType()->isAggregateType()) {
          StInfo->getField(I).setMultipleValue();
          StInfo->getField(I).setBottomAllocFunction();
        }
  }

  if (DTransPrintAnalyzedTypes) {
    // This is really ugly, but it is only used during testing.
    // The type infos are stored in a map with pointer keys, and so the
    // order is non-deterministic. This copies them into a vector and sorts
    // them so that the order in which they are printed is deterministic.
    // By using the type_info_entries() method instead of accessing
    // the TypeInfoMap directly, this also tests the type_info_iterator.
    std::vector<dtrans::TypeInfo *> TypeInfoEntries;
    for (auto *TI : type_info_entries())
      if (isa<dtrans::ArrayInfo>(TI) || isa<dtrans::StructInfo>(TI))
        TypeInfoEntries.push_back(TI);

    std::sort(TypeInfoEntries.begin(), TypeInfoEntries.end(),
              [](dtrans::TypeInfo *A, dtrans::TypeInfo *B) {
                std::string TypeStrA;
                llvm::raw_string_ostream RSO_A(TypeStrA);
                A->getLLVMType()->print(RSO_A);
                std::string TypeStrB;
                llvm::raw_string_ostream RSO_B(TypeStrB);
                B->getLLVMType()->print(RSO_B);
                return RSO_A.str().compare(RSO_B.str()) < 0;
              });

    outs() << "================================\n";
    outs() << " DTRANS Analysis Types Created\n";
    outs() << "================================\n\n";
    for (auto TI : TypeInfoEntries) {
      if (auto *AI = dyn_cast<dtrans::ArrayInfo>(TI)) {
        printArrayInfo(AI);
      } else if (auto *SI = dyn_cast<dtrans::StructInfo>(TI)) {
        printStructInfo(SI);
      }
    }
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (DTransPrintAnalyzedCalls)
    printCallInfo(outs());
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  return false;
}

void DTransAnalysisInfo::printStructInfo(dtrans::StructInfo *SI) {
  outs() << "DTRANS_StructInfo:\n";
  outs() << "  LLVMType: " << *(SI->getLLVMType()) << "\n";
  llvm::StructType *S = cast<llvm::StructType>(SI->getLLVMType());
  if (S->hasName())
    outs() << "  Name: " << S->getName() << "\n";
  if (SI->getCRuleTypeKind() != dtrans::CRT_Unknown) {
    outs() << "  CRuleTypeKind: ";
    outs() << dtrans::CRuleTypeKindName(SI->getCRuleTypeKind()) << "\n";
  }
  outs() << "  Number of fields: " << SI->getNumFields() << "\n";
  for (auto &Field : SI->getFields()) {
    printFieldInfo(Field);
  }
  SI->printSafetyData();
  outs() << "\n";
}

void DTransAnalysisInfo::printArrayInfo(dtrans::ArrayInfo *AI) {
  outs() << "DTRANS_ArrayInfo:\n";
  outs() << "  LLVMType: " << *(AI->getLLVMType()) << "\n";
  if (AI->getCRuleTypeKind() != dtrans::CRT_Unknown) {
    outs() << "  CRuleTypeKind: ";
    outs() << dtrans::CRuleTypeKindName(AI->getCRuleTypeKind()) << "\n";
  }
  outs() << "  Number of elements: " << AI->getNumElements() << "\n";
  outs() << "  Element LLVM Type: " << *(AI->getElementLLVMType()) << "\n";
  AI->printSafetyData();
  outs() << "\n";
}

void DTransAnalysisInfo::printFieldInfo(dtrans::FieldInfo &Field) {
  outs() << "  Field LLVM Type: " << *(Field.getLLVMType()) << "\n";
  outs() << "    Field info:";
  if (Field.isRead())
    outs() << " Read";
  if (Field.isWritten())
    outs() << " Written";
  if (Field.hasComplexUse())
    outs() << " ComplexUse";
  if (Field.isAddressTaken())
    outs() << " AddressTaken";
  outs() << "\n";
  if (Field.isNoValue())
    outs() << "    No Value";
  else if (Field.isSingleValue()) {
    outs() << "    Single Value: ";
    Field.getSingleValue()->printAsOperand(outs());
  } else if (Field.isMultipleValue())
    outs() << "    Multiple Value";
  outs() << "\n";
  if (Field.isTopAllocFunction())
    outs() << "    Top Alloc Function";
  else if (Field.isSingleAllocFunction()) {
    outs() << "    Single Alloc Function: ";
    Field.getSingleAllocFunction()->printAsOperand(outs());
  } else if (Field.isBottomAllocFunction())
    outs() << "    Bottom Alloc Function";
  outs() << "\n";
}

bool DTransAnalysisInfo::GetFuncPointerPossibleTargets(
    llvm::Value *FP, std::vector<llvm::Value *> &Targets, llvm::CallSite,
    bool) {
  Targets.clear();
  LLVM_DEBUG({
    dbgs() << "FSV ICS: Analyzing";
    FP->dump();
  });
  auto LI = dyn_cast<LoadInst>(FP);
  llvm::Type *Ty = nullptr;
  ConstantInt *CZ = nullptr;
  ConstantInt *CI = nullptr;
  if (!LI)
    return false;
  auto *GEPI = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
  GEPOperator *GEPO = nullptr;
  if (GEPI) {
    if (GEPI->getNumIndices() != 2 || !GEPI->hasAllConstantIndices())
      return false;
    CZ = cast<ConstantInt>(GEPI->getOperand(1));
    CI = cast<ConstantInt>(GEPI->getOperand(2));
    Ty = GEPI->getSourceElementType();
  } else {
    GEPO = dyn_cast<GEPOperator>(LI->getPointerOperand());
    if (!GEPO)
      return false;
    if (GEPO->getNumIndices() != 2 || !GEPO->hasAllConstantIndices())
      return false;
    CZ = cast<ConstantInt>(GEPO->getOperand(1));
    CI = cast<ConstantInt>(GEPO->getOperand(2));
    Ty = GEPO->getSourceElementType();
  }
  if (!Ty->isStructTy())
    return false;
  if (!CZ->isZeroValue())
    return false;
  dtrans::TypeInfo *TI = getTypeInfo(Ty);
  if (!TI)
    return false;
  auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
  if (!StInfo)
    return false;
  uint64_t Index = CI->getLimitedValue();
  if (Index >= StInfo->getNumFields())
    return false;
  dtrans::FieldInfo FI = StInfo->getField(Index);
  auto F = dyn_cast_or_null<Function>(FI.getSingleValue());
  if (!F)
    return false;
  Targets.push_back(F);
  LLVM_DEBUG({
    dbgs() << "FSV ICS: Specialized TO " << F->getName() << " GEP ";
    if (GEPI)
      GEPI->dump();
    else
      GEPO->dump();
  });
  return true;
}

void DTransAnalysisWrapper::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
}

char DTransAnalysis::PassID;

// Provide a definition for the static class member used to identify passes.
AnalysisKey DTransAnalysis::Key;

DTransAnalysisInfo DTransAnalysis::run(Module &M, AnalysisManager<Module> &AM) {
  DTransAnalysisInfo DTResult;
  DTResult.analyzeModule(M, AM.getResult<TargetLibraryAnalysis>(M));
  return DTResult;
}
