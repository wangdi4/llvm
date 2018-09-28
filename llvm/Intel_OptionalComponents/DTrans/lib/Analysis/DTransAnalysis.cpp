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
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
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
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <map>
#include <set>

using namespace llvm;
using namespace llvm::PatternMatch;

#define DEBUG_TYPE "dtransanalysis"

// Debug type for basic local pointer analysis output.
#define DTRANS_LPA "dtrans-lpa"

// Debug type for verbose local pointer analysis output.
#define DTRANS_LPA_VERBOSE "dtrans-lpa-verbose"

// Debug type for verbose call graph computations.
#define DTRANS_CG "dtrans-cg"

// Debug type for verbose partial pointer load/store analysis output.
#define DTRANS_PARTIALPTR "dtrans-partialptr"

// Debug type for verbose field single value analysis output.
#define DTRANS_FSV "dtrans-fsv"

// Debug type for verbose field single alloc function analysis output.
#define DTRANS_FSAF "dtrans-fsaf"

static cl::opt<bool> DTransPrintAllocations("dtrans-print-allocations",
                                            cl::ReallyHidden);

static cl::opt<bool> DTransPrintAnalyzedTypes("dtrans-print-types",
                                              cl::ReallyHidden);
// BlockFrequencyInfo is ignored while computing field frequency info
// if this flag is true.
// TODO: Disable this flag by default after doing more experiments and
// tuning.
static cl::opt<bool>
    DTransIgnoreBFI("dtrans-ignore-bfi", cl::init(true), cl::ReallyHidden,
                    cl::desc("Ignore using BFI while computing field freq"));

// This internal option is used to avoid assigning safety check violations to
// types in the list. Syntax: the list should be a sequence of records separated
// by ';'. Each record should be in the form
// 'transformation:typename(,typename)*'
// Ex.: -dtrans-nosafetychecks-list="aostosoa:type1,type2,type3;fsv:type2"
static cl::list<std::string> DTransNoSafetyChecksList(
    "dtrans-nosafetychecks-list",
    cl::desc("Suppress dtrans safety violations for aggregate types."),
    cl::ReallyHidden);

// Maximum number of callsites before we disable DTransAnalysis
static cl::opt<unsigned> DTransMaxCallsiteCount("dtrans-maxcallsitecount",
                                                cl::init(150000),
                                                cl::ReallyHidden);

// Maximum number of instructions before we disable DTransAnalysis
static cl::opt<unsigned> DTransMaxInstructionCount("dtrans-maxinstructioncount",
                                                   cl::init(1500000),
                                                   cl::ReallyHidden);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// Prints information that is saved during analysis about specific function
/// calls (malloc, free, memset, etc) that may be useful to the transformations.
static cl::opt<bool> DTransPrintAnalyzedCalls("dtrans-print-callinfo",
                                              cl::ReallyHidden);

// ';'-separated list of items:
// - <function name>
//  Direct calls to function <function name> is considered as free/delete.
// - <type name>,<offset>
//  See DTransAll::analyzeForIndirectStatus
static cl::list<std::string> DTransFreeFunctions("dtrans-free-functions",
                                                 cl::ReallyHidden);

// ';'-separated list of items:
// - <function name>
//  Direct calls to function <function name> is considered as malloc/new.
// - <type name>,<offset>
//  See DTransAll::analyzeForIndirectStatus
static cl::list<std::string> DTransMallocFunctions("dtrans-malloc-functions",
                                                   cl::ReallyHidden);

// Enables identification of values that are loaded but never used.
// See LocalPointerAnalyzer::identifyUnusedValue
static cl::opt<bool> DTransIdentifyUnusedValues("dtrans-identify-unused-values",
                                                cl::init(true),
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
    const APInt *Count;
    // The condition should be a comparison based on a PHI node.
    if (!match(Condition,
               m_ICmp(Pred, m_Instruction(Base), m_APInt(Count)))) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. icmp not using constant int!\n");
      return false;
    }
    if (Pred != CmpInst::Predicate::ICMP_SGT) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. icmp predicate isn't sgt!\n");
      return false;
    }

    // The loop count can be represented in various ways. The two handled here
    // are:
    //
    //   %Count = phi i64 [ %InitCount, %Block1 ], [ %NextCount, %Block2 ]
    //   ...
    //   %NextCount = add nsw i64 %Count, -1
    //   %Cmp = icmp sgt i64 %Count, 1
    //
    // and
    //
    //   %Count = phi i64 [ %InitCount, %Block1 ], [ %NextCount, %Block2 ]
    //   ...
    //   %NextCount = add nsw i64 %Count, -1
    //   %Cmp = icmp sgt i64 %NextCount, 0
    if (Count->isOneValue()) {
      auto *BasePHI = dyn_cast<PHINode>(Base);
      if (!BasePHI || BasePHI->getParent() != LoopBB) {
        DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                        dbgs() << "Not matched. Branch condition isn't PHI!\n");
        return false;
      }
      // The incoming value from the loop block must be a decrement of the
      // count PHI.
      Value *LoopInVal;
      if (BasePHI->getIncomingBlock(0) == LoopBB)
        LoopInVal = BasePHI->getIncomingValue(0);
      else
        LoopInVal = BasePHI->getIncomingValue(1);
      if (!(match(LoopInVal, m_Add(m_Specific(BasePHI), m_SpecificInt(-1))) ||
            match(LoopInVal, m_Add(m_SpecificInt(-1), m_Specific(BasePHI))))) {
        DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                        dbgs() << "Not matched. PHI decrement not matched!\n");
        return false;
      }
    } else {
      if (!Count->isNullValue()) {
        DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                        dbgs() << "Not matched. icmp not using 0 or 1!\n");
        return false;
      }

      // If the comparison is against zero, we expect the value being
      // compared to be a decrement of the PHI value.
      Instruction *DecBase;
      if (!(match(Base, m_Add(m_Instruction(DecBase), m_SpecificInt(-1))) ||
            match(Base, m_Add(m_SpecificInt(-1), m_Instruction(DecBase))))) {
        DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                        dbgs() << "Not matched. PHI decrement not matched!\n");
        return false;
      }
      auto *BasePHI = dyn_cast<PHINode>(DecBase);
      if (!BasePHI || BasePHI->getParent() != LoopBB) {
        DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                        dbgs() << "Not matched. Decrement input isn't PHI!\n");
        return false;
      }
      // The incoming value from the loop block must be the decrement result.
      Value *LoopInVal;
      if (BasePHI->getIncomingBlock(0) == LoopBB)
        LoopInVal = BasePHI->getIncomingValue(0);
      else
        LoopInVal = BasePHI->getIncomingValue(1);
      if (LoopInVal != Base) {
        DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                        dbgs() << "Not matched. PHI decrement not matched!\n");
        return false;
      }
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
    if (T1 == nullptr || T2 == nullptr)
      return false;

    if (T1 == T2)
      return true;

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

  bool isPtrToCharArray() {
    llvm::Type *DomTy = getDominantAggregateTy();
    if (!DomTy) {
      if (pointsToSomeElement())
        for (auto &PointeePair : getElementPointeeSet()) {
          if (auto *ArrayTy = dyn_cast<ArrayType>(PointeePair.first)) {
            llvm::Type *Int8Ty =
                llvm::Type::getIntNTy(ArrayTy->getContext(), 8);
            if (ArrayTy->getArrayElementType() == Int8Ty)
              return true;
          }
        }
      return false;
    }
    if (!DomTy->isPointerTy())
      return false;
    if (auto *ArrayTy = dyn_cast<ArrayType>(DomTy->getPointerElementType())) {
      llvm::Type *Int8Ty = llvm::Type::getIntNTy(DomTy->getContext(), 8);
      if (ArrayTy->getArrayElementType() == Int8Ty)
        return true;
    }
    return false;
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
  DTransAllocAnalyzer(const TargetLibraryInfo &TLI, const Module &M)
      : TLI(TLI) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    parseListOptions(M);
#endif
    Int8PtrTy = Type::getInt8PtrTy(M.getContext(), 0 /*AS*/);
  }
  bool isMallocPostDom(CallSite CS);
  bool isFreePostDom(CallSite CS);
  void populateAllocDeallocTable(const Module &M);

private:
  typedef PointerIntPair<StructType *, 1, bool> PtrBoolPair;
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
  std::map<const Function *, AllocStatus> LocalMap;
  // Offsets inside vtable.
  // Key is (pointer to some type, true = allocation/false = deallocation).
  std::map<PtrBoolPair, int32_t> VTableOffs;
  // A set to hold visited BasicBlocks.  This is a temporary set used
  // while we are determining the AllocStatus of a Function.
  SmallPtrSet<BasicBlock *, 20> VisitedBlocks;
  // A set to hold the BasicBlocks which do not need to be post-dominated
  // by malloc() to be considered isMallocPostDom() or free to be considered
  // isFreePostDom().
  SmallPtrSet<BasicBlock *, 4> SkipTestBlocks;
  // Needed to detrmine if a function is malloc()
  const TargetLibraryInfo &TLI;
  // Needed to check argument types
  PointerType *Int8PtrTy;

  bool isSkipTestBlock(BasicBlock *BB) const;
  bool isVisitedBlock(BasicBlock *BB) const;
  int skipTestSuccessor(BranchInst *BI) const;
  void visitAndSetSkipTestSuccessors(BasicBlock *BB);
  void visitAndResetSkipTestSuccessors(BasicBlock *BB);
  void visitNullPtrBlocks(Function *F);

  bool mallocBasedGEPChain(GetElementPtrInst *GV, GetElementPtrInst **GBV,
                           CallSite *GCI) const;
  bool mallocOffset(Value *V, int64_t *offset) const;
  bool mallocLimit(GetElementPtrInst *GBV, Value *V, int64_t Offset,
                   int64_t *Result) const;
  bool returnValueIsMallocAddress(Value *RV, BasicBlock *BB);
  bool analyzeForMallocStatus(Function *F);

  bool hasFreeCall(BasicBlock *BB) const;
  bool isPostDominatedByFreeCall(BasicBlock *BB, bool &IsFreeSeen);
  bool analyzeForFreeStatus(Function *F);

  bool analyzeForIndirectStatus(CallSite CS, bool Alloc);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void parseListOptions(const Module &M);
#endif
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
// Return true if 'CS' is post-dominated by a call to malloc() on all paths
// that do not include skip blocks.
// Trivial wrappers are important special cases.
//
bool DTransAllocAnalyzer::isMallocPostDom(CallSite CS) {
  // If the Function wasn't found, then there is a possibility
  // that it is inside a BitCast. In this case we need
  // to strip the pointer casting from the Value and then
  // access the Function.
  Function *F = dyn_cast<Function>(CS.getCalledValue()->stripPointerCasts());

  if (!F)
    // Check for allocation routine.
    return analyzeForIndirectStatus(CS, true);

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
      llvm_unreachable("populateAllocDeallocTable analysis was incomplete");
      LocalMap[F] = AKS_Malloc;
      return true;
    }
    LocalMap[F] = AKS_NotMallocFree;
    return false;
  case AKS_Unknown:
    if (analyzeForMallocStatus(F)) {
      llvm_unreachable("populateAllocDeallocTable analysis was incomplete");
      LocalMap[F] = AKS_Malloc;
      return true;
    }
    LocalMap[F] = AKS_NotMalloc;
    return false;
  }
  return false;
}

//
// Return true if 'CS' is post-dominated by a call to free() on all paths that
// do not include skip blocks.
// Trivial wrappers are important special cases.
//
bool DTransAllocAnalyzer::isFreePostDom(CallSite CS) {
  // If the Function wasn't found, then there is a possibility
  // that it is inside a BitCast. In this case we need
  // to strip the pointer casting from the Value and then
  // access the Function.
  Function *F = dyn_cast<Function>(CS.getCalledValue()->stripPointerCasts());

  if (!F)
    // Check for deallocation routine.
    return analyzeForIndirectStatus(CS, false);

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
      llvm_unreachable("populateAllocDeallocTable analysis was incomplete");
      LocalMap[F] = AKS_Free;
      return true;
    }
    LocalMap[F] = AKS_NotMallocFree;
    return false;
  case AKS_Unknown:
    if (analyzeForFreeStatus(F)) {
      llvm_unreachable("populateAllocDeallocTable analysis was incomplete");
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
  if (auto CS = CallSite(V))
    if (auto Kind = dtrans::getAllocFnKind(CS, TLI))
      if (Kind == dtrans::AK_Malloc || Kind == dtrans::AK_New)
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
  for (auto *SBB : SkipBlockSet)
    visitAndSetSkipTestSuccessors(SBB);
  for (auto *NSBB : NoSkipBlockSet)
    visitAndResetSkipTestSuccessors(NSBB);
}

//
// Return true if 'GV' is the root of a malloc based byte-flattened GEP chain.
// This means that if we keep following the pointer operand for a series of
// byte flattened GEP instructions, we will eventually get to a malloc() call.
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
                                              CallSite *GCI) const {
  GetElementPtrInst *V;
  for (V = GV; isa<GetElementPtrInst>(V->getPointerOperand());
       V = cast<GetElementPtrInst>(V->getPointerOperand())) {
    if (!V->getSourceElementType()->isIntegerTy(8))
      return false;
  }
  if (!V->getSourceElementType()->isIntegerTy(8))
    return false;

  Value *BasePtr = V->getPointerOperand();
  if (auto CS = CallSite(BasePtr)) {
    auto Kind = dtrans::getAllocFnKind(CS, TLI);
    if (Kind != dtrans::AK_Malloc && Kind != dtrans::AK_New)
      return false;
    *GBV = V;
    *GCI = CS;
    return true;
  }
  return false;
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
// the address computed by the sequence of GEPs starting with 'GBV', and if
// the sequence of GEPs starting with 'GBV' compute an offset equal to 'Offset'.
//
// If we return true, we set '*Result' to the value of the upper bound.
//
// For example:
//
//  %6 = getelementptr inbounds i8, i8* %5, i64 15
//  %7 = getelementptr inbounds i8, i8* %6, i64 8
//  %8 = getelementptr inbounds i8, i8* %7, i64 4
//  %9 = ptrtoint i8* %8 to i64
//  %10 = and i64 %9, 15
//  %11 = sub nsw i64 0, %10
//  %12 = getelementptr inbounds i8, i8* %8, i64 %11
//
// If 'V' here is %11 and 'GBV' is %6, then the most that %12 can be less
// than %8 is 15.
//
// The sequence of GEPs starting with 'GBV' are:
//
//  %6 = getelementptr inbounds i8, i8* %5, i64 15
//  %7 = getelementptr inbounds i8, i8* %6, i64 8
//  %8 = getelementptr inbounds i8, i8* %7, i64 4
//
// The offset computed is 15+8+4 == 27, which should be equal to 'Offset'.
// The value returned in '*Result' is 15.
//
// NOTE: mallocLimit() is a bit of a pattern match, albeit for a very
// important case.
//
bool DTransAllocAnalyzer::mallocLimit(GetElementPtrInst *GBV, Value *V,
                                      int64_t Offset, int64_t *Result) const {
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
  int64_t LocalOffset = 0;
  Value *NewGEP = PI->getOperand(0);
  auto Int8PtrTy = llvm::Type::getInt8PtrTy(PI->getContext());
  GetElementPtrInst *LastGEP = nullptr;
  while (auto GEP = dyn_cast<GetElementPtrInst>(NewGEP)) {
    LastGEP = GEP;
    NewGEP = GEP->getPointerOperand();
    if (NewGEP->getType() != Int8PtrTy)
      return false;
    auto CI = dyn_cast<ConstantInt>(GEP->getOperand(1));
    if (!CI)
      return false;
    LocalOffset += CI->getSExtValue();
  }
  if (LocalOffset != Offset)
    return false;
  if (LastGEP != GBV)
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
  if (auto CS = CallSite(RV)) {
    auto Kind = dtrans::getAllocFnKind(CS, TLI);
    return Kind == dtrans::AK_Malloc || Kind == dtrans::AK_New;
  }
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
    CallSite CS;
    if (!mallocBasedGEPChain(GV, &GBV, &CS))
      return false;
    if (!mallocOffset(CS.getArgument(0), &Offset))
      return false;
    if (!mallocLimit(GBV, GV->getOperand(1), Offset, &Limit))
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
  if (F->arg_size() != 1 ||
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

//  Indirect calls through vtable. Matched sequence for 'allocate' is as
//  follows.
//      %0 = bitcast <type name>* %m to i8* (<type name>*, i64)***
//      %vtable = load i8* (<type name>*, i64)**,
//        i8* (<type name>*, i64)*** %0
//      %vfn = getelementptr inbounds i8* (<type name>*, i64)*,
//        i8* (<type name>*, i64)** %vtable, i64 <offset>
//      %1 = load i8* (<type name>*, i64)*, i8* (<type name>*, i64)** %vfn
//      %call = call i8* %1(<type name>* %m, i64 size)
//
//  Indirect calls through vtable. Matched sequence for 'deallocate' is as
//  follows.
//      %4 = bitcast <type name>* %m to void (<type name>*, i8*)***
//      %vtable1 = load void (<type name>*, i8*)**,
//          void (<type name>*, i8*)*** %4
//      %vfn2 = getelementptr inbounds void (<type name>*, i8*)*,
//          void (<type name>*, i8*)** %vtable1, i64 <offset>
//      %5 = load void (<type name>*, i8*)*, void (<type name>*, i8*)** %vfn2
//      call void %5(<type name>* %m, i8* %3)
bool DTransAllocAnalyzer::analyzeForIndirectStatus(CallSite CS, bool Malloc) {

  if (CS.getNumArgOperands() < 2)
    return false;

  // First argument is 'this' pointer.
  Type *ObjType = CS.getArgOperand(0)->getType();
  if (!isa<PointerType>(ObjType))
    return false;

  StructType *SObjType =
      dyn_cast<StructType>(cast<PointerType>(ObjType)->getElementType());

  if (!SObjType)
    return false;

  auto It = VTableOffs.find(PtrBoolPair(SObjType, Malloc));
  if (It == VTableOffs.end())
    return false;

  // Check size_t or void* argument.
  if (Malloc ? !CS.getArgOperand(1)->getType()->isIntegerTy(32) &&
                   !CS.getArgOperand(1)->getType()->isIntegerTy(64)
             : CS.getArgOperand(1)->getType() != Int8PtrTy)
    return false;

  // Search for definition of called Value
  auto *Callee = dyn_cast<LoadInst>(CS.getCalledValue());
  if (!Callee)
    return false;

  auto *VFn = dyn_cast<GetElementPtrInst>(Callee->getPointerOperand());
  int64_t IdxVal = 0;
  if (VFn) {
    if (VFn->getNumIndices() != 1)
      return false;
    if (auto *Opc = dyn_cast<ConstantInt>(*VFn->idx_begin()))
      IdxVal = Opc->getSExtValue();
    else
      return false;
  }
  if (IdxVal != It->second)
    return false;

  auto *VTable = dyn_cast<LoadInst>(VFn ? VFn->getPointerOperand()
                                        : Callee->getPointerOperand());
  if (!VTable)
    return false;

  auto *ObjCast = dyn_cast<BitCastInst>(VTable->getPointerOperand());
  if (!ObjCast || ObjCast->getOperand(0) != CS.getArgOperand(0))
    return false;

  return true;
}

//
// Return true if 'BB' has a call to free().
//
bool DTransAllocAnalyzer::hasFreeCall(BasicBlock *BB) const {
  for (auto BI = BB->rbegin(), BE = BB->rend(); BI != BE;) {
    Instruction *I = &*BI++;
    if (auto CS = CallSite(I))
      if (dtrans::isFreeFn(CS, TLI))
        return true;
  }
  return false;
}

//
// Checks all uses of 'free' and 'malloc' functions.
// Only uses in InvokeInst and CallInst are checked.
//
// This approach allows to classify all functions, which call system memory
// management routines, in advance and to avoid on-demand classification.
//
// TODO: Later it will be possible to classify user memory management functions
// calling other user management functions.
//
void DTransAllocAnalyzer::populateAllocDeallocTable(const Module &M) {
  // TODO: compute closure.
  for (auto &F : M.getFunctionList()) {
    // Find some call/invoke of F.
    ImmutableCallSite CS;
    for (auto &U : F.uses())
      if (auto Tmp = ImmutableCallSite(U.getUser())) {
        CS = Tmp;
        break;
      }

    // No calls/invokes.
    if (!CS)
      continue;

    // Deal with free.
    if (dtrans::isFreeFn(CS, TLI)) {
      // Check all functions, calling free function F.
      for (auto &U : F.uses())
        if (ImmutableCallSite(U.getUser())) {
          // Function calling F.
          auto *FreeCand =
              cast<Instruction>(U.getUser())->getParent()->getParent();
          if (analyzeForFreeStatus(FreeCand))
            LocalMap[FreeCand] = AKS_Free;
        }
      continue;
    }

    // Deal with malloc/new
    auto Kind = dtrans::getAllocFnKind(CS, TLI);
    if (Kind == dtrans::AK_Malloc || Kind == dtrans::AK_New) {
      // Check all functions, calling malloc/new function F.
      for (auto &U : F.uses())
        if (ImmutableCallSite(U.getUser())) {
          // Function calling F.
          auto *MallocCand =
              cast<Instruction>(U.getUser())->getParent()->getParent();
          if (analyzeForMallocStatus(MallocCand))
            LocalMap[MallocCand] = AKS_Malloc;
        }
      continue;
    }
  }
}

//
// Return true if 'BB' contains or is dominated by a call to free()
// on all predecessors.
//
bool DTransAllocAnalyzer::isPostDominatedByFreeCall(BasicBlock *BB,
                                                    bool &SeenFree) {
  bool rv = false;
  if (isVisitedBlock(BB))
    return false;
  VisitedBlocks.insert(BB);
  bool IsSkipTestBlock = isSkipTestBlock(BB);
  if (IsSkipTestBlock)
    return true;
  if (hasFreeCall(BB)) {
    SeenFree = true;
    return true;
  }
  for (BasicBlock *PB : predecessors(BB)) {
    if (!isPostDominatedByFreeCall(PB, SeenFree))
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
      bool SeenFree = false;
      if (!isPostDominatedByFreeCall(&BB, SeenFree)) {
        LLVM_DEBUG(dbgs() << "Not FreePostDom " << F->getName()
                          << " Return is not post-dominated by call to free\n");
        return false;
      }
      rv = SeenFree;
    }
  if (rv)
    LLVM_DEBUG(dbgs() << "Is FreePostDom " << F->getName() << "\n");
  else
    LLVM_DEBUG(dbgs() << "Not FreePostDom " << F->getName()
                      << " No return post-dominated by free\n");
  return rv;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DTransAllocAnalyzer::parseListOptions(const Module &M) {
  auto &LM = LocalMap;
  auto &VT = VTableOffs;

  auto F = [&LM, &VT, &M](cl::list<std::string> &Options, AllocStatus AKS,
                          bool Malloc, StringRef Banner) -> void {
    if (Options.empty())
      return;

    LLVM_DEBUG(dbgs() << "IPO: DTrans " << Banner << " functions\n");

    for (auto &Opt : Options) {
      if (Opt.empty())
        continue;

      LLVM_DEBUG(dbgs() << "\tList: " << Opt << "\n");

      StringRef OptRef(Opt);
      SmallVector<StringRef, 8> ListRecords;
      OptRef.split(ListRecords, ';');

      for (auto Record : ListRecords) {

        SmallVector<StringRef, 2> RecordItem;
        Record.split(RecordItem, ',');

        if (RecordItem.size() == 0)
          continue;

        if (RecordItem.size() > 2) {
          LLVM_DEBUG(dbgs() << "IPO: error 1: record <" << Record
                            << "> has a wrong format. Should be "
                               "<funcname> or <typename,offset>\n");
          continue;
        }

        if (RecordItem.size() == 1) {
          // Explicit function name case.
          if (auto *F = M.getFunction(RecordItem[0]))
            LM[F] = AKS;
          else
            LLVM_DEBUG(dbgs() << "IPO: error 1: record <" << Record
                              << "> specifies invalid function name '"
                              << RecordItem[0] << "'\n");
          continue;
        }

        // Indirect call through vptr.
        int64_t Offset = -1;
        if (RecordItem[1].getAsInteger(10, Offset)) {
          LLVM_DEBUG(dbgs() << "IPO: error 1: record <" << Record
                            << "> has a wrong format. Should be "
                               "<funcname> or <typename,offset>\n");
          continue;
        }

        if (auto *Ty = M.getTypeByName(RecordItem[0])) {
          if (Ty->element_begin() == Ty->element_end() ||
              !isa<PointerType>(Ty->getElementType(0))) {
            LLVM_DEBUG(dbgs() << "IPO: error 1: record <" << Record
                              << "> specifies type '" << Ty
                              << "', "
                                 "which does not have pointer first field\n");
            continue;
          }

          auto Key = PtrBoolPair(Ty, Malloc);
          if (VT.find(Key) != VT.end())
            LLVM_DEBUG(dbgs() << "IPO: error 1: record <" << Record
                              << "> overrides previous " << Banner
                              << " for type '" << Ty << "' \n");

          VT[Key] = Offset;
        } else
          LLVM_DEBUG(dbgs() << "IPO: error 1: record <" << Record
                            << "> specifies invalid type name '"
                            << RecordItem[0] << "'\n");
      }
    }
  };

  F(DTransFreeFunctions, AKS_Free, false, "free");
  F(DTransMallocFunctions, AKS_Malloc, true, "malloc");
}
#endif

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

    // Check for metadata used to annotate the type from one of the
    // transformations to apply to the type.
    if (auto *I = dyn_cast<Instruction>(V))
      if (auto *TyFromMD = dtrans::lookupDTransTypeAnnotation(I))
        Info.addPointerTypeAlias(TyFromMD);

    // Build a stack of unresolved dependent values that must be analyzed
    // before we can complete the analysis of this value.
    SmallVector<Value *, 16> DependentVals;
    DependentVals.push_back(V);
    populateDependencyStack(V, DependentVals);

    // This first line is intentionally left non-verbose.
    DEBUG_WITH_TYPE(DTRANS_LPA, {
      dbgs() << "analyzeValue ";
      if (isa<Function>(V))
        V->printAsOperand(dbgs());
      else
        dbgs() << *V;
      dbgs() << "\n";
    });
    DEBUG_WITH_TYPE(DTRANS_LPA_VERBOSE, dumpDependencyStack(DependentVals));

    // Now attempt to analyze each of these values. Some may be left in a
    // partially analyzed state, but they will be fully resolved when
    // their complete info is needed.
    while (!DependentVals.empty()) {
      Value *Dep = DependentVals.back();
      DependentVals.pop_back();
      LocalPointerInfo &DepInfo = LocalMap[Dep];
      // If we have complete results for this value, don't repeat the analysis.
      if (DepInfo.getAnalyzed()) {
        DEBUG_WITH_TYPE(DTRANS_LPA_VERBOSE,
                        dbgs() << "  Already analyzed: " << *Dep << "\n");
        continue;
      }
      analyzeValueImpl(Dep, DepInfo);
    }

    DEBUG_WITH_TYPE(DTRANS_LPA_VERBOSE, {
      if (Info.isPartialAnalysis())
        dbgs() << " Analysis completed but was reported as partial.\n";
    });
    // These are intentionally left non-verbose.
    DEBUG_WITH_TYPE(DTRANS_LPA, Info.dump());
    DEBUG_WITH_TYPE(DTRANS_LPA, dbgs() << "\n");

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
      DEBUG_WITH_TYPE(DTRANS_LPA, dbgs() << "Partial pointer bitcast detected: "
                                         << *V << "\n");
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

    if (auto CS = getCallSiteIfAlloc(V)) {
      // If the value we're analyzing is a call to an allocation function
      // we need to look for bitcast users so that we can proactively assign
      // the type to which the value will be cast as an alias.
      analyzeAllocationCallAliases(CS, Info);
    } else if (auto *II = dyn_cast<IntrinsicInst>(V)) {
      analyzeIntrinsic(II, Info);
    } else if (auto *GEP = dyn_cast<GEPOperator>(V)) {
      // If this is a GetElementPtr, figure out what element it is
      // accessing.
      analyzeElementAccess(GEP, Info);
    } else if (auto *Load = dyn_cast<LoadInst>(V)) {
      // If the value being analyzed is a load instruction the loaded value
      // may inherit some alias information from the load's pointer operand.
      analyzeLoadInstruction(Load, Info);
    } else if (isa<ExtractValueInst>(V)) {
      // FIXME: Also analyze extract value instructions.
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

  CallSite getCallSiteIfAlloc(Value *V) {
    if (auto CS = CallSite(V)) {
      auto Kind = dtrans::getAllocFnKind(CS, TLI);
      if (Kind == dtrans::AK_NotAlloc && DTAA.isMallocPostDom(CS))
        Kind = dtrans::AK_Malloc;
      if (Kind != dtrans::AK_NotAlloc)
        return CS;
    }
    return CallSite();
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
    if (auto CS = getCallSiteIfAlloc(V)) {
      SmallPtrSet<User *, 8> VisitedUsers;
      addAllocUsesToDependencyStack(CS.getInstruction(), DependentVals,
                                    VisitedUsers);
    }

    // FIXME: Also handle invoke and extract value instructions.
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
      DEBUG_WITH_TYPE(DTRANS_LPA_VERBOSE,
                      dbgs()
                          << "Incomplete analysis merged from " << *Op << "\n");
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
        DEBUG_WITH_TYPE(DTRANS_LPA_VERBOSE,
                        dbgs() << "Incomplete analysis collected from "
                               << *SrcVal << "\n");
      }
      // If the bitcast is part of an idiom where pointer values are copied
      // in smaller chunks, don't treat it like other bitcasts.
      if (isPartialPtrBitCast(V)) {
        DEBUG_WITH_TYPE(DTRANS_LPA,
                        dbgs() << "Partial pointer bitcast detected: " << *V
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
        auto *DestTy = BC->getDestTy();
        for (auto *AliasTy : SrcLPI.getPointerTypeAliasSet()) {
          llvm::Type *AccessedTy = nullptr;
          // If element zero is an i8* and we're casting the source value
          // as a pointer to a pointer, that is an element zero access
          // but it isn't reported as such because we need to recognize the
          // i8*->(struct**) bitcast elsewhere as a potential problem.
          if (dtrans::isElementZeroAccess(AliasTy, DestTy, &AccessedTy) ||
              (AliasTy->isPointerTy() && DestTy->isPointerTy() &&
               DestTy->getPointerElementType()->isPointerTy() &&
               dtrans::isElementZeroI8Ptr(AliasTy->getPointerElementType(),
                                          &AccessedTy))) {
            Info.addElementPointee(AccessedTy->getPointerElementType(), 0);
            // If the bitcast is to an i8** and element zero of the accessed
            // type is a pointer, we need to add the type of that pointer
            // to the destination value's alias set. If the bitcast destination
            // is not an i8** it will be the type of the accessed element
            // zero, so we don't need to check that condition. We can just
            // always add the element zero pointer type to the alias set.
            Info.addPointerTypeAlias(
                cast<CompositeType>(AccessedTy->getPointerElementType())
                    ->getTypeAtIndex(0u)
                    ->getPointerTo());
            IsElementZeroAccess = true;
          } else if (dtrans::isPtrToPtrToElementZeroAccess(AliasTy, DestTy)) {
            // If the DestTy and the AliasTy are both pointers to pointers
            // this may be an element zero access with an additional level
            // of indirection.
            // FIXME: We'll lose track of an actual element zero access if
            //        a pointer is loaded from here.
            // In this case, we don't have a way to express that the result
            // is a poointer to a pointer to an element. That's a problem.
            IsElementZeroAccess = true;
            // If the source value was a pointer to some element, we
            // need to transfer that information to the bitcast result
            // because loading from the new pointer will effectively be
            // an access of the original element.
            for (auto &PointeePair : SrcLPI.getElementPointeeSet())
              Info.addElementPointee(PointeePair.first, PointeePair.second);
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
        DEBUG_WITH_TYPE(DTRANS_LPA_VERBOSE,
                        dbgs() << "Incomplete analysis derived from "
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
    if (auto *LastArg =
            dyn_cast<ConstantInt>(GEP->getOperand(GEP->getNumOperands() - 1))) {
      uint64_t Idx = LastArg->getLimitedValue();
      // Add this information to the local pointer information for the GEP.
      Info.addElementPointee(IndexedTy, Idx);
      return;
    }

    if (auto *ArrayTy = dyn_cast<ArrayType>(IndexedTy)) {
      // Accessing variable length array elements is always 'out-of-bounds'.
      if (ArrayTy->getNumElements() == 0) {
        DEBUG_WITH_TYPE(DTRANS_LPA_VERBOSE,
                        dbgs() << "Zero-sized array access detected "
                               << *GEP << "\n");
        return;
      }

      // If the last argument is not constant and out-of-bound flag is false
      // then it is safe to have non-constant array access. Use 0 index for
      // Pointee set.
      if (!DTransOutOfBoundsOK)
        Info.addElementPointee(IndexedTy, 0);
    }

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
  // TODO: add special processing for constant offsets which are multiples of
  // structure size.
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
      DEBUG_WITH_TYPE(DTRANS_LPA_VERBOSE,
                      dbgs() << "Incomplete analysis derived from "
                             << *BasePointer << "\n");
    }

    // If we can't compute a constant offset, we won't be able to
    // figure out which element is being accessed.
    unsigned BitWidth = DL.getPointerSizeInBits();
    SmallVector<APInt, 3> APOffset;
    APInt CurrOffset(BitWidth, 0);
    // If offset is constant - put it in the list of possibe constant offsets.
    if (GEP->accumulateConstantOffset(DL, CurrOffset)) {
      APOffset.push_back(CurrOffset);
    } else if (GEP->getNumOperands() == 2) {
      // If offset comes from select instruction with two constant operands then
      // put both values in the list of possible constant offsets.
      // TODO: currently we only proccess Select. The same could work for PHI.
      Value *Cond;
      const APInt *SelT, *SelF;
      if (!match(GEP->getOperand(1),
                 m_Select(m_Value(Cond), m_APInt(SelT), m_APInt(SelF))))
        return false;
      APOffset.push_back(*SelT);
      APOffset.push_back(*SelF);
    } else {
      return false;
    }

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
      if (!HasPtrToPtrAlias) {
        // Try all possible offsets one by one.
        uint32_t Res = 0;
        auto *CurrType = AliasTy->getPointerElementType();
        LocalPointerInfo LocalInfo;
        for (auto &APOffsetVal : APOffset) {
          if (analyzePossibleOffsetAggregateAccess(
                  GEP, CurrType, APOffsetVal.getLimitedValue(), LocalInfo))
            Res++;
        }
        // If all offsets are appropriate - add those element pointees and
        // return true.
        if (Res == APOffset.size()) {
          Info.merge(LocalInfo);
          return true;
        }
      }
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
    if (!AggregateTy->isAggregateType() || !AggregateTy->isSized())
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

  void analyzeIntrinsic(IntrinsicInst *II, LocalPointerInfo &Info) {
    // The llvm.ptr.annotation intrinsic returns the value of the first
    // argument. We need to propagate the type information from that argument
    // into the result of this intrinsic call.
    if (II->getIntrinsicID() != Intrinsic::ptr_annotation)
      return;

    Value *Src = II->getOperand(0);
    LocalPointerInfo &SrcLPI = LocalMap[Src];
    if (!SrcLPI.getAnalyzed()) {
      Info.setPartialAnalysis(true);
      DEBUG_WITH_TYPE(DTRANS_LPA_VERBOSE,
                      dbgs() << "Incomplete analysis derived from " << *Src
                             << "\n");
    }

    Info.merge(SrcLPI);
  }

  void analyzeLoadInstruction(LoadInst *Load, LocalPointerInfo &Info) {
    // If the pointer operand aliases any pointers-to-pointers, the loaded
    // value will be considered to alias to the pointed-to pointer type.
    Value *Src = Load->getPointerOperand();
    LocalPointerInfo &SrcLPI = LocalMap[Src];
    // If the incoming analysis was incomplete, what we do below won't be
    // complete, but the partial analysis may be necessary so we note the
    // incompleteness and continue.
    if (!SrcLPI.getAnalyzed()) {
      Info.setPartialAnalysis(true);
      DEBUG_WITH_TYPE(DTRANS_LPA_VERBOSE,
                      dbgs() << "Incomplete analysis derived from " << *Src
                             << "\n");
    }
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
           isa<Constant>(V) || isa<GEPOperator>(V) || isa<InvokeInst>(V) ||
           isa<ExtractValueInst>(V));

    // Note that ExtractValueInst and InvokeInst are not handled by the main
    // instruction visitor, so they will cause UnhandledUse safety conditions
    // to be set. They are added to the assert here to prevent it from firing
    // while compiling programs that we do not expect to be able to optimize.
    // Additional implementation would be necessary to handle these correctly.

    return false;
  }

  // Find any type to which the return value of an allocation call will be
  // bitcast and, unless it looks like an element zero access, add that type
  // as an alias of the allocated pointer.
  void analyzeAllocationCallAliases(CallSite CS, LocalPointerInfo &Info) {
    DEBUG_WITH_TYPE(DTRANS_LPA, dbgs()
                                    << "dtrans: Analyzing allocation call.\n  "
                                    << *CS.getInstruction() << "\n");
    SmallPtrSet<llvm::PointerType *, 4> CastTypes;
    SmallPtrSet<Value *, 4> VisitedUsers;
    bool IsPartial = false;
    collectAllocatedPtrBitcasts(CS.getInstruction(), CastTypes, VisitedUsers,
                                IsPartial);
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

        DEBUG_WITH_TYPE(DTRANS_LPA,
                        dbgs() << "  Associated bitcast: " << *BI << "\n");

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
    DEBUG_WITH_TYPE(DTRANS_LPA_VERBOSE,
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
          DEBUG_WITH_TYPE(DTRANS_LPA,
                          dbgs() << "Found partial pointer bitcast: " << *U
                                 << "\n");
          continue;
        }
        // We want to follow the uses through PointerToInt casts, but they
        // don't tell us anything about aliases. The dyn_cast above catches
        // PtrToInt, IntToPtr, and BitCast. If the result is a pointer
        // type, we want to add it to the alias set.
        if (auto *PtrTy = dyn_cast<PointerType>(Cast->getType())) {
          DEBUG_WITH_TYPE(DTRANS_LPA_VERBOSE,
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
      if (auto CS = CallSite(U)) {
        if (isValueInt8PtrType(V)) {
          DEBUG_WITH_TYPE(DTRANS_LPA_VERBOSE,
                          dbgs() << "Analyzing use in call instruction: "
                                 << *CS.getInstruction() << "\n");
          Function *F =
              dyn_cast<Function>(CS.getCalledValue()->stripPointerCasts());
          if (!F) {
            DEBUG_WITH_TYPE(DTRANS_LPA_VERBOSE,
                            dbgs() << "Unable to get called function!\n");
            continue;
          }
          // Check all the arguments of the call as our value may be used more
          // than once. Use F->getNumParams() rather than CS.getNumArgs()
          // because the function may be bitcast in a way that changes its
          // argument count.
          unsigned NumArgs = F->getFunctionType()->getNumParams();
          for (unsigned ArgNo = 0; ArgNo < NumArgs; ++ArgNo) {
            if (CS.getArgOperand(ArgNo) == V) {
              DEBUG_WITH_TYPE(DTRANS_LPA_VERBOSE,
                              dbgs() << "Analyzing function argument: "
                                     << F->getName() << " @ " << ArgNo << "\n");
              Argument *Arg = F->arg_begin();
              std::advance(Arg, ArgNo);
              // If the argument is still an i8* after the function bitcast
              // follow its uses. Otherwise, infer the type directly from
              // the argument type.
              if (isValueInt8PtrType(Arg))
                inferAliasedTypesFromUses(Arg, CastTypes, VisitedUsers);
              else if (auto *ArgPtrTy = dyn_cast<PointerType>(Arg->getType()))
                CastTypes.insert(ArgPtrTy);
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
                    DTransAllocAnalyzer &DTAA,
                    function_ref<BlockFrequencyInfo &(Function &)> &GetBFI)
      : DTInfo(Info), DL(DL), TLI(TLI), LPA(DL, TLI, DTAA), DTAA(DTAA),
        GetBFI(GetBFI), BFI(nullptr) {
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
    case Intrinsic::ptr_annotation:
    case Intrinsic::var_annotation:
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
                                         SmallPtrSetImpl<llvm::Type *> &Tstack,
                                         bool IgnorePointees) {

    // Enum indicating that on the particular predicate being compared for
    // T1 and T2, the types have the opposite value of the predicate
    // (TME_OPPOSITE), the types both have the predicate (TME_YES), or
    // neither type has the predicate (TME_YES).
    enum TME { TME_OPPOSITE, TME_YES, TME_NO };

    // Lambda to match the result of testing the same predicate for T1 and T2
    auto boolT = [](bool B1, bool B2) -> TME {
      return (B1 && B2) ? TME_YES : (!B1 && !B2) ? TME_NO : TME_OPPOSITE;
    };

    // Returns true if T is a struct type with no elements.
    auto isEmptyStructType = [](llvm::Type *T) -> bool {
      return T->isStructTy() && T->getStructNumElements() == 0;
    };

    // Typedef for const pointer to member function which returns a bool
    typedef bool (llvm::Type::*MFP)() const;
    // Lambda to compare an predicate indicated by Fp for T1 and T2
    auto typeTest = [&boolT](llvm::Type *T1, llvm::Type *T2, MFP Fp) -> TME {
      return boolT((T1->*Fp)(), (T2->*Fp)());
    };

    // An array of predicate conditions for which T1 and T2 will be tested
    // for compatibility. The predicate "isIntegerTy" and "isFloatingPointTy"
    // are base properties that cannot be further refined.
    MFP F1Array[] = {&llvm::Type::isIntegerTy, &llvm::Type::isFloatingPointTy};

    // A Type is always compatible with itself.
    if (T1 == T2)
      return true;

    // CMPLRS-15252: We have seen that an empty struct type can be generated by
    // the clang front end to represent a Function type. Since this will not be
    // fixed in the clang front end, assume that an empty struct type is
    // compatible with a Function type.
    if ((T1->isFunctionTy() && isEmptyStructType(T2)) || (T2->isFunctionTy()
      && isEmptyStructType(T1)))
      return true;

    // Test some fundamental and complicated predicates. Return false if they
    // don't match, true if they do.
    for (auto Fx : F1Array) {
      TME R = typeTest(T1, T2, Fx);
      if (R == TME_OPPOSITE)
        return false;
      if (R == TME_YES)
        // FIXME: check DataLayout::getTypeStoreSize
        return true;
    }

    // Two pointer Types are compatible if their element types are compatible.
    TME R = typeTest(T1, T2, &llvm::Type::isPointerTy);
    if (R == TME_OPPOSITE)
      return false;
    if (R == TME_YES) {
      if (T1->getPointerAddressSpace() != T2->getPointerAddressSpace())
        return false;

      if (IgnorePointees)
        return true;

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

      // CMPLRS-15252: Perform the same empty struct/Function type test
      // above on the pointer element types.
      if ((T3->isFunctionTy() && isEmptyStructType(T4)) || (T4->isFunctionTy()
          && isEmptyStructType(T3)))
        return true;

      // Should contain all checks resulting in recursive calls.
      MFP CompoundChecks[] = {&llvm::Type::isArrayTy, &llvm::Type::isStructTy,
                              &llvm::Type::isFunctionTy};

      for (auto Cx : CompoundChecks) {
        TME S = typeTest(T3, T4, Cx);
        if (S == TME_OPPOSITE)
          return false;
        if (S == TME_YES) {
          if (Tstack.find(T3) != Tstack.end())
            return true;
          if (Tstack.find(T4) != Tstack.end())
            return true;
          Tstack.insert(T3);
          Tstack.insert(T4);
        }
      }

      return typesMayBeCRuleCompatibleX(T1->getPointerElementType(),
                                        T2->getPointerElementType(), Tstack,
                                        IgnorePointees);
    }

    R = typeTest(T1, T2, &llvm::Type::isFunctionTy);
    if (R == TME_OPPOSITE)
      return false;
    if (R == TME_YES) {
      auto *FT1 = cast<FunctionType>(T1);
      auto *FT2 = cast<FunctionType>(T2);
      if (!typesMayBeCRuleCompatibleX(FT1->getReturnType(),
                                      FT2->getReturnType(), Tstack,
                                      IgnorePointees))
        return false;

      // FIXME: extend to handle left over parameters and var-arg.
      for (auto Pair : zip(FT1->params(), FT2->params())) {
        auto *PT1 = std::get<0>(Pair);
        auto *PT2 = std::get<1>(Pair);
        if (!typesMayBeCRuleCompatibleX(PT1, PT2, Tstack, IgnorePointees))
          return false;
      }
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
                                        T2->getArrayElementType(), Tstack,
                                        IgnorePointees);
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
                                        T2->getStructElementType(I), Tstack,
                                        IgnorePointees))
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
  //   (3) Vector types and function types with different numbers of
  //       parameters and var arg. This could be implemented in the future.
  //       We are not doing it now because there is no immediate need.
  //
  // IgnorePointees only checks that pointers are layout compatible,
  // i.e. they are from the same address space.
  static bool typesMayBeCRuleCompatible(llvm::Type *T1, llvm::Type *T2,
                                        bool IgnorePointees = false) {
    SmallPtrSet<llvm::Type *, 4> Tstack;
    return typesMayBeCRuleCompatibleX(T1, T2, Tstack, IgnorePointees);
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

  // Return true if CS represents an indirect call site, and there is a
  // an address taken external function that matches it.
  bool hasICMatch(CallSite CS) {
    // No point in doing this if the C language compatibility rule is not
    // enforced.
    if (!DTransUseCRuleCompat)
      return true;
    // Check if this is an indirect call site.
    if (isa<Function>(CS.getCalledValue()))
      return false;
    // Look for a matching address taken external call.
    for (auto &F : CS.getInstruction()->getModule()->functions())
      if (F.hasAddressTaken() && F.isDeclaration())
        if ((F.arg_size() == CS.getNumArgOperands()) ||
            (F.isVarArg() && (F.arg_size() <= CS.getNumArgOperands()))) {
          unsigned I = 0;
          bool IsFunctionMatch = true;
          for (auto &Arg : F.args()) {
            Value *VCI = CS.getArgOperand(I);
            if (!typesMayBeCRuleCompatible(VCI->getType(), Arg.getType())) {
              IsFunctionMatch = false;
              break;
            }
            ++I;
          }
          if (IsFunctionMatch) {
            LLVM_DEBUG({
              dbgs() << "dtrans-ic-match: ";
              F.printAsOperand(dbgs());
              dbgs() << ":: " << I << "\n";
            });
            return true;
          }
        }
    return false;
  }

  // Set MismatchedArgUse safety check if needed for direct call represented by
  // F.
  // FIXME: unify with visitCallArgument.
  void checkArgTypeMismatch(CallSite CS, Function *F, Argument *FormalVal,
                            Value *ActualVal) {

    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(ActualVal);
    // Collect the dominant Type of the actual parameter
    Type *ActualType = LPI.getDominantAggregateTy();
    Type *FormalType = nullptr;

    if (FormalVal) {
      FormalType = FormalVal->getType();
      // Type match
      if (FormalType == ActualType || checkUsersType(FormalVal, ActualType))
        return;
    } else if (!ActualType || !DTInfo.isTypeOfInterest(ActualType) ||
               isVarArgSameType(F, ActualType)) {
      return;
    }

    // Print debug information if "Mismatched argument use" was set either
    // in the formal or actual Type.
    LLVM_DEBUG({
      if ((FormalType && DTInfo.isTypeOfInterest(FormalType)) ||
          (ActualType && DTInfo.isTypeOfInterest(ActualType))) {
        dbgs() << "dtrans-safety: Mismatched argument use -- bitcast "
                  "function arguments mismatch:\n  "
               << *CS.getInstruction();

        if (FormalVal) {
          dbgs() << "\n    Formal: " << *FormalVal;
          dbgs() << "  Type: " << *FormalType << "\n";
        } else
          dbgs() << "\n    Formal: ...  Type: vararg\n";
        dbgs() << "    Actual: " << *ActualVal << "\n";
        dbgs() << "  Dominant Type: ";
        if (ActualType)
          dbgs() << *ActualType;
        else
          dbgs() << "null";
        dbgs() << "\n";
      }
    });

    if (FormalType && DTInfo.isTypeOfInterest(FormalType))
      setBaseTypeInfoSafetyData(FormalType, dtrans::MismatchedArgUse);

    if (ActualType && DTInfo.isTypeOfInterest(ActualType))
      setBaseTypeInfoSafetyData(ActualType, dtrans::MismatchedArgUse);
  }

  // FIXME: unify with checkArgTypeMismatch.
  void visitCallArgument(CallSite CS, Function *F, bool HasICMatch, Value *Arg,
                         unsigned ArgNo) {

    bool IsFnLocal = F ? !F->isDeclaration() : false;

    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(Arg);
    if (LPI.pointsToSomeElement()) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Field address taken -- "
                        << "pointer to element passed as argument:\n"
                        << "  " << *CS.getInstruction() << "\n  " << *Arg
                        << "\n");
      // Selects and PHIs may have created a pointer that refers to
      // elements in multiple aggregate types. This sets the field
      // address taken condition for them all.
      for (auto PointeePair : LPI.getElementPointeeSet()) {
        auto Res = getParentStructType(PointeePair, Arg);
        dtrans::TypeInfo *ParentTI =
                              DTInfo.getOrCreateTypeInfo(Res.first);
        size_t Idx = Res.second;
        if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(ParentTI)) {
          ParentStInfo->getField(Idx).setAddressTaken();
          setBaseTypeInfoSafetyData(Res.first,
                                    dtrans::FieldAddressTaken);
        }
      }
    }

    if (CS.isInvoke()) {
      for (auto *Ty : LPI.getPointerTypeAliasSet()) {
        if (!DTInfo.isTypeOfInterest(Ty))
          continue;

        LLVM_DEBUG(dbgs() << "dtrans-safety: C++ handling -- "
                          << "pointer to struct passed as argument to "
                             "function invoke:\n "
                          << *CS.getInstruction() << "\n " << *Arg << "\n");
        setBaseTypeInfoSafetyData(Ty, dtrans::HasCppHandling);
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
    // we can't check its use from here. It's possible to cast a non-variadic
    // function to a varadic type for a call, so we always check the number of
    // parameters rather than trusting isVarArg().
    auto *ArgTy = Arg->getType();
    if (IsFnLocal && (ArgTy == Int8PtrTy) &&
        ArgNo < F->getFunctionType()->getNumParams()) {
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
                          << "  " << *CS.getInstruction() << "\n  " << *Arg
                          << "\n");
        setAllAliasedTypeSafetyData(LPI, dtrans::MismatchedArgUse);
        setAllAliasedTypeSafetyData(ParamLPI, dtrans::MismatchedArgUse);
      } else {
        llvm::Type *DomParamTy = ParamLPI.getDominantAggregateTy();
        llvm::Type *DomArgTy = LPI.getDominantAggregateTy();
        if (DomParamTy != DomArgTy) {
          LLVM_DEBUG(dbgs()
                     << "dtrans-safety: Mismatched argument use -- "
                     << "overloaded alias set found at call site:\n"
                     << "  " << *CS.getInstruction() << "\n  " << *Arg << "\n");
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
        if (!F && DTransUseCRuleCompat && !HasICMatch &&
            !mayHaveDistinctCompatibleCType(AliasTy))
          continue;
        // If the first element of the dominant type of the pointer is an
        // an array of the actual argument, don't report address taken.
        if (isElementZeroArrayOfType(AliasTy, ArgTy)) {
          LLVM_DEBUG(dbgs()
                     << "dtrans-safety: Field address taken -- "
                     << "ptr to array element passed as argument:\n"
                     << "  " << *CS.getInstruction() << "\n  " << *Arg << "\n");
          setBaseTypeInfoSafetyData(AliasTy, dtrans::FieldAddressTaken);
          dtrans::TypeInfo *ParentTI = DTInfo.getOrCreateTypeInfo(AliasTy);
          if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(ParentTI))
            ParentStInfo->getField(0).setAddressTaken();
          continue;
        }
        LLVM_DEBUG(dbgs() << "dtrans-safety: Address taken -- "
                          << "pointer to aggregate passed as argument:\n"
                          << "  " << *CS.getInstruction() << "\n  " << *Arg
                          << "\n");
        setBaseTypeInfoSafetyData(AliasTy, dtrans::AddressTaken);
      }
    }
  }

  void visitCallSite(CallSite CS) {
    SmallPtrSet<const Value *, 3> SpecialArguments;

    // If the called function is a known allocation function, we need to
    // analyze the allocation.
    auto AllocKind = dtrans::getAllocFnKind(CS, TLI);
    if (AllocKind == dtrans::AK_NotAlloc && DTAA.isMallocPostDom(CS))
      AllocKind = dtrans::AK_UserMalloc;
    if (AllocKind != dtrans::AK_NotAlloc) {
      analyzeAllocationCall(CS, AllocKind);
      collectSpecialAllocArgs(AllocKind, CS, SpecialArguments, TLI);
    }

    // If this is a call to the "free" lib function,  the call is safe, but
    // we analyze the instruction for the purpose of capturing the argument
    // TypeInfo, which will be needed by some of the transformations when
    // rewriting allocations and frees.
    auto FreeKind = dtrans::isFreeFn(CS, TLI)
                        ? (dtrans::isDeleteFn(CS, TLI) ? dtrans::FK_Delete
                                                       : dtrans::FK_Free)
                        : dtrans::FK_NotFree;

    if (FreeKind == dtrans::FK_NotFree && DTAA.isFreePostDom(CS))
      FreeKind = dtrans::FK_UserFree;

    if (FreeKind != dtrans::FK_NotFree) {
      analyzeFreeCall(CS, FreeKind);
      collectSpecialFreeArgs(FreeKind, CS, SpecialArguments, TLI);
    }

    // If the Function wasn't found, then there is a possibility
    // that it is inside a BitCast. In this case we need
    // to strip the pointer casting from the Value and then
    // access the Function.
    Function *F = dyn_cast<Function>(CS.getCalledValue()->stripPointerCasts());

    // For all other calls, if a pointer to an aggregate type is passed as an
    // argument to a function in a form other than its dominant type, the
    // address has escaped. Also, if a pointer to a field is passed as an
    // argument to a function, the address of the field has escaped.
    // FIXME: Try to resolve indirect calls.
    bool IsFnLocal = F ? !F->isDeclaration() : false;

    // Mark structures returned by non-local functions as system types.
    auto *RetTy = CS.getType();
    if (DTInfo.isTypeOfInterest(RetTy) && AllocKind == dtrans::AK_NotAlloc) {
      if (!IsFnLocal) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: System object: "
                          << "type returned by extern function\n  " << *RetTy
                          << "\n");
        setBaseTypeInfoSafetyData(RetTy, dtrans::SystemObject);
      }
      if (CS.isInvoke()) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: C++ handling -- "
                          << "struct (or pointer to struct) returned from "
                             "function invoke:\n "
                          << *CS.getInstruction() << "\n");
        setBaseTypeInfoSafetyData(RetTy, dtrans::HasCppHandling);
      }
    }

    if (SpecialArguments.size() == CS.arg_size())
      return;

    // Check for BitCast functions
    if (F && isa<ConstantExpr>(CS.getCalledValue())) {
      // Account for layout in registers and on stack.
      if (!typesMayBeCRuleCompatible(
              CS.getCalledValue()->getType()->getPointerElementType(),
              // Do not need to compare pointees types
              // here.
              F->getType()->getPointerElementType(), true)) {
        for (Argument &Arg : F->args()) {
          LLVM_DEBUG(
              dbgs()
              << "dtrans-safety: Mismatched argument use -- bitcast "
                 "function arguments mismatch with layout incompatibility:\n  "
              << *CS.getInstruction());
          setValueTypeInfoSafetyData(&Arg, dtrans::MismatchedArgUse);
        }

        for (Value *Arg : CS.args()) {
          LLVM_DEBUG(
              dbgs()
              << "dtrans-safety: Mismatched argument use -- bitcast "
                 "function arguments mismatch with layout incompatibility:\n  "
              << *CS.getInstruction());
          setValueTypeInfoSafetyData(Arg, dtrans::MismatchedArgUse);
        }
      } else {
        for (auto Pair : zip(F->args(), CS.args()))
          if (!SpecialArguments.count(std::get<1>(Pair)))
            checkArgTypeMismatch(CS, F, &std::get<0>(Pair), std::get<1>(Pair));

        // Do not care about F->arg_size() > CS.arg_size(),
        // it should be marked in the true-branch, or it is UB.
        if (F->arg_size() < CS.arg_size())
          for (Value *Arg :
               make_range(CS.arg_begin() + F->arg_size(), CS.arg_end()))
            if (!SpecialArguments.count(Arg))
              checkArgTypeMismatch(CS, F, nullptr, Arg);
      }
    }

    // If this is an indirect call site, find out if there is a matching
    // address taken external call.  In this case, the indirect call must
    // be treated like an external call for the purpose of generating
    // the AddressTaken safety check.
    bool HasICMatch = hasICMatch(CS);

    unsigned NextArgNo = 0;
    for (Value *Arg : CS.args()) {
      // Keep track of the argument index we're working with.
      unsigned ArgNo = NextArgNo++;

      if (!isValueOfInterest(Arg) || SpecialArguments.count(Arg))
        continue;

      visitCallArgument(CS, F, HasICMatch, Arg, ArgNo);
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

    // Check for possible use of pointer type loaded via pointer sized int or
    // generic i8* to expose the real type to the transforms. This is temporary,
    // until the full LocalPointerInfo is made available to the transformations.
    if (isGenericPtrType(I.getType())) {
      LocalPointerInfo &ValLPI = LPA.getLocalPointerInfo(&I);
      collectGenericLoadStoreType(I, ValLPI);
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

    // If the value of interest aliases to a pointer to a type of interest,
    // check to  make sure the pointer operand is compatible.
    if (ValLPI.canAliasToAggregatePointer()) {
      for (auto *AliasTy : ValLPI.getPointerTypeAliasSet()) {
        if (AliasTy == Int8PtrTy)
          continue;

        // If the value operand is a pointer to an aggregate type then the
        // pointer operand should either be a pointer to the value type or be a
        // pointer to pointer-sized int type. The analysis could be extended to
        // check this condition on the further levels of indirection.
        if (PtrLPI.canPointToType(PtrSizeIntTy) &&
            !PtrLPI.canAliasToAggregatePointer()) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Address taken:\n");
          if (I != nullptr)
            LLVM_DEBUG(dbgs() << "  " << *I << "\n");
          else
            LLVM_DEBUG(dbgs()
                       << " " << *ValOperand << " -> " << *PtrOperand << " \n");
          setValueTypeInfoSafetyData(ValOperand, dtrans::AddressTaken);
        } else if (!PtrLPI.canPointToType(AliasTy)) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Unsafe pointer store -- "
                            << "  Unmatch store of aliased value:\n");
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
      // type, but the pointer operand is. Unless (1) the value operand is a
      // null pointer, (2) the store is part of a partial pointer store idiom,
      // or (3) the value is an i8 and the aliased pointer type has an i8 at
      // element zero, this is a bad store.
      auto *ValTy = ValOperand->getType();
      if (!isa<ConstantPointerNull>(ValOperand) && !isPartialPtrStore(I) &&
          !(ValTy->isIntegerTy(8) &&
            dtrans::isElementZeroAccess(PtrLPI.getDominantAggregateTy(),
                                        ValTy->getPointerTo()))) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Unsafe pointer store -- "
                          << "  Unknown store to aliased ptr:\n");
        if (I != nullptr)
          LLVM_DEBUG(dbgs() << "  " << *I << "\n");
        else
          LLVM_DEBUG(dbgs()
                     << " " << *ValOperand << " -> " << *PtrOperand << " \n");
        setValueTypeInfoSafetyData(PtrOperand, dtrans::UnsafePointerStore);
      }
    }
  }

  std::pair<llvm::Type *, size_t>
  getParentStructType(std::pair<llvm::Type *, size_t> &PointeePair,
                      Value *ValOp) {
    llvm::Type *ParentTy  = PointeePair.first;
    size_t Idx = PointeePair.second;
    if (PointeePair.first->isArrayTy() && PointeePair.second == 0) {
      // Storing the address of zero array element is the same as saving
      // array address. So find a structure type containing the array.
      if (auto *GEP = dyn_cast<GEPOperator>(ValOp)) {
        if (GEP->getNumIndices() == 1) {
          if (auto *LastArg = dyn_cast<ConstantInt>(
                  GEP->getOperand(GEP->getNumOperands() - 1))) {
            // TODO: add multiple array access case here after fixing Pointee
            // index info.
            (void)LastArg;
          }
        } else if (GEP->getNumIndices() == 2) {
          if (auto *LastArg = dyn_cast<ConstantInt>(
                  GEP->getOperand(GEP->getNumOperands() - 1))) {
            Idx = LastArg->getLimitedValue();
            ParentTy = GEP->getSourceElementType();
          }
        } else {
          SmallVector<Value *, 4> Ops(GEP->idx_begin(), GEP->idx_end() - 2);
          Type *IndexedTy = GetElementPtrInst::getIndexedType(
              GEP->getSourceElementType(), Ops);
          if (IndexedTy) {
            if (auto *LastArg = dyn_cast<ConstantInt>(
                    GEP->getOperand(GEP->getNumOperands() - 2))) {
              Idx = LastArg->getLimitedValue();
              ParentTy = IndexedTy;
            }
          }
        }
      }
    }
    return std::make_pair(ParentTy, Idx);
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

    // Check for possible use of pointer type stored via pointer sized int
    // or i8* generic type to expose the real type to the transforms.
    // This is temporary, until the full LocalPointerInfo is made available
    // to the transformations.
    if (isGenericPtrType(ValOperand->getType()))
      collectGenericLoadStoreType(I, ValLPI);

    if (ValLPI.pointsToSomeElement()) {
      auto ValPointees = ValLPI.getElementPointeeSet();
      for (auto PointeePair : ValPointees) {
        auto Res = getParentStructType(PointeePair, ValOperand);
        dtrans::TypeInfo *ParentTI =
            DTInfo.getOrCreateTypeInfo(Res.first);
        size_t Idx = Res.second;
        if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(ParentTI)) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Field address taken:\n"
                            << "  " << *(ParentTI->getLLVMType()) << " @ "
                            << Idx << "\n"
                            << "  " << I << "\n");
          ParentTI->setSafetyData(dtrans::FieldAddressTaken);
          ParentStInfo->getField(Idx).setAddressTaken();
        }
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

  // Collect the aggregate type for \p ValLPI which corresponds to the Value
  // loaded or stored by \p I which is loading/storing a pointer sized integer
  // value or other generic form of the pointer. Save an association between the
  // Instruction and the actual aggregate pointer type, if there is a single
  // type, in the maps to allow queries by the transformations. This is
  // temporary until the complete LocalPointerInfo is exposed to the
  // transformations.
  void collectGenericLoadStoreType(Instruction &I, LocalPointerInfo &ValLPI) {
    if (!ValLPI.canAliasToAggregatePointer())
      return;

    // Find the actual type of aggregate pointer being loaded, if there is
    // a single type. If there is more than one type, then we will not
    // capture the information for the ptr-size-int mapping because the
    // type will not pass the safety checks for the transformations.
    llvm::Type *ActualType = nullptr;
    for (auto *AliasTy : ValLPI.getPointerTypeAliasSet()) {
      if (isGenericPtrType(AliasTy))
        continue;

      if (ActualType) {
        ActualType = nullptr;
        break;
      }

      ActualType = AliasTy;
    }

    if (!ActualType)
      return;

    if (auto *LI = dyn_cast<LoadInst>(&I))
      DTInfo.addGenericLoadMapping(LI, ActualType);
    else if (auto *SI = dyn_cast<StoreInst>(&I))
      DTInfo.addGenericStoreMapping(SI, ActualType);
    else
      llvm_unreachable("Instruction must be LoadInst or StoreInst");
  }

  // Return true if \p Ty is a type that could be a generic equivalent of
  // a pointer (at some level of indirection) to a structure type. This is
  // used when determining whether a Load/Store instruction should capture
  // the aggregate type to enable exposing the actual type being loaded
  // or stored to the transformations.
  bool isGenericPtrType(llvm::Type *Ty) {
    llvm::Type *PtrTy = nullptr;
    while (Ty->isPointerTy()) {
      PtrTy = Ty;
      Ty = Ty->getPointerElementType();
    }

    if (PtrTy == Int8PtrTy)
      return true;

    if (Ty == PtrSizeIntTy)
      return true;

    return false;
  }

  void visitGetElementPtrInst(GetElementPtrInst &I) {
    // TODO: Associate the parent type of the pointer so we can properly
    //       evaluate the uses.
    Value *Src = I.getPointerOperand();
    if (!isValueOfInterest(Src))
      return;

    // Since the GEP collapsing optimizations were turned off for DTrans, it is
    // possible to see a GEP chain which calculates final address. For safety
    // checks we need the condition to be set for all levels of nested
    // structures containing current address.
    GetElementPtrInst *GEPI = nullptr;
    if (GetElementPtrInst *CurrInst = dyn_cast<GetElementPtrInst>(Src)) {
      while (auto *PrevGEP =
                 dyn_cast<GetElementPtrInst>(CurrInst->getPointerOperand()))
        CurrInst = PrevGEP;
      GEPI = CurrInst;
    }

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
      // We set safety data on the first GEP. It will be naturally promoted to
      // subtypes.
      if (GEPI) {
        LocalPointerInfo &DeepestLPI =
            LPA.getLocalPointerInfo(GEPI->getPointerOperand());
        setAllAliasedTypeSafetyData(DeepestLPI, dtrans::AmbiguousGEP);
      }
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
      if ((isInt8Ptr(Src) &&
           !(SrcLPI.isPtrToPtr() || SrcLPI.isPtrToCharArray())) ||
          I.getNumIndices() != 1) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Bad pointer manipulation:\n"
                          << "  " << I << "\n");
        setAllAliasedTypeSafetyData(SrcLPI, dtrans::BadPtrManipulation);
        // We set safety data on the first GEP. It will be naturally promoted to
        // subtypes.
        if (GEPI) {
          LocalPointerInfo &DeepestLPI =
              LPA.getLocalPointerInfo(GEPI->getPointerOperand());
          setAllAliasedTypeSafetyData(DeepestLPI, dtrans::BadPtrManipulation);
        }
      }
    } else {
      auto &PointeeSet = GEPLPI.getElementPointeeSet();
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

  // TODO: Need to extend the analysis for IntToPtr instruction for more
  // accurate safety checks.
  void visitIntToPtrInst(IntToPtrInst &I) {
    if (!isValueOfInterest(&I))
      return;
    LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
                      << "Integer to ptr to aggregate cast:\n"
                      << "  " << I << "\n");
    setValueTypeInfoSafetyData(&I, dtrans::BadCasting);
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
        auto Res = getParentStructType(PointeePair, RetVal);
        dtrans::TypeInfo *ParentTI =
                              DTInfo.getOrCreateTypeInfo(Res.first);
        size_t Idx = Res.second;
        if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(ParentTI)) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Field address taken -- "
                            << "Address of a field is returned by function: "
                            << I.getParent()->getParent()->getName() << "\n");
          setBaseTypeInfoSafetyData(Res.first,
                                    dtrans::FieldAddressTaken);
          ParentStInfo->getField(Idx).setAddressTaken();
        }
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

    // Add the structure types declared in the module to the type info list.
    for (StructType *Ty : M.getIdentifiedStructTypes())
      (void)DTInfo.getOrCreateTypeInfo(Ty);

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

      // If this is an array of a type of interest, set a safety condition
      // on that type. The DeleteField optimization has a problem updating
      // uses of global arrays, so we'll want to disable it when that happens
      // until the problem is fixed.
      if (GVElemTy->isArrayTy() &&
          DTInfo.isTypeOfInterest(GVElemTy->getArrayElementType())) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Global array\n"
                          << "  " << GV << "\n");
        setBaseTypeInfoSafetyData(GVTy, dtrans::GlobalArray);
      }

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

          DEBUG_WITH_TYPE(
              DTRANS_CG,
              dbgs() << "dtrans-cg: CGraph update for Globalvalue  -- \n"
                     << "  " << GV << "\n");
          setBaseTypeCallGraph(GV.getType(), nullptr);
        }
      }
    }
    // Analyze definitions of all structures in type info list.
    for (dtrans::TypeInfo *TI : DTInfo.type_info_entries())
      analyzeStructureType(TI);
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

    // Get BFI if available.
    BFI = (!F.isDeclaration()) ? &(GetBFI(F)) : nullptr;

    // Call the base class to visit the instructions in the function.
    InstVisitor<DTransInstVisitor>::visitFunction(F);
  }

  void visitBasicBlock(BasicBlock &BB) {
    InstVisitor<DTransInstVisitor>::visitBasicBlock(BB);

    auto &F = *BB.getParent();
    for (auto &I : BB) {
      if (isValueOfInterest(&I)) {
        DEBUG_WITH_TYPE(DTRANS_CG,
                        dbgs()
                            << "dtrans-cg: CGraph update for Instruction -- \n"
                            << "  " << I << "\n");
        setBaseTypeCallGraph(I.getType(), &F);
      }

      for (Value *Arg : I.operands()) {
        if (!isa<Constant>(Arg) && !isa<Argument>(Arg))
          continue;
        if (isValueOfInterest(Arg)) {
          DEBUG_WITH_TYPE(
              DTRANS_CG, dbgs() << "dtrans-cg: CGraph update for Operand  -- \n"
                                << "  " << *Arg << "\n");
          setBaseTypeCallGraph(Arg->getType(), &F);
        }
      }
    }
  }

  // Accumulate field frequency of \p FI that is accessed in \p I.
  void accumulateFrequency(dtrans::FieldInfo &FI, Instruction &I) {
    // BFI may not be related to the function that we are currently
    // processing  due to cross function analysis.
    // Fix BFI if it is not the info related to current processing
    // function.
    Function *F = I.getParent()->getParent();
    if (BFI && BFI->getFunction() != F)
      BFI = &(GetBFI(*F));

    // If DTransIgnoreBFI is true, use 1 as frequency of field access.
    // Otherwise, use BasicBlock's Frequency as frequency of field access.
    uint64_t InstFreq;
    if (BFI && !DTransIgnoreBFI)
      InstFreq = BFI->getBlockFreq(I.getParent()).getFrequency();
    else
      InstFreq = 1;

    uint64_t TFreq = InstFreq + FI.getFrequency();
    // Set it to 64-bit unsigned Max value if there is overflow.
    TFreq = TFreq < InstFreq ? std::numeric_limits<uint64_t>::max() : TFreq;
    FI.setFrequency(TFreq);
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
  function_ref<BlockFrequencyInfo &(Function &)> &GetBFI;
  // Set this in visitFunction before visiting instructions in the function.
  BlockFrequencyInfo *BFI;

  // We need these types often enough that it's worth keeping them around.
  llvm::Type *Int8PtrTy;
  llvm::Type *PtrSizeIntTy;
  llvm::Type *PtrSizeIntPtrTy;

  // Return true if the dominant Type of all Users of the input Value
  // are the same as the input Type, else return false.
  bool checkUsersType(Value *Val, Type *Ty) {
    if (!Ty)
      return false;

    if (Val->user_empty())
      return false;

    for (User *User : Val->users()) {
      llvm::Value *UserVal = dyn_cast<Value>(User);
      LocalPointerInfo &LPI = LPA.getLocalPointerInfo(UserVal);
      llvm::Type *UserType = LPI.getDominantAggregateTy();

      if (Ty != UserType)
        return false;
    }

    return true;
  }

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

  // Return true if the input Function is variadic, and the input
  // Type is the only Type handled by the va_list, else return false.
  bool isVarArgSameType(Function *Fn, Type *ArgType) {

    if (!Fn->isVarArg())
      return false;

    // The Type of the structure that is holding va_list
    Type *VAListType = nullptr;

    // Lambda function that returns true if the input Value is
    // in the va_list
    auto checkIfVarArgType = [&](Value *Val) {
      if (!VAListType)
        return false;

      if (!isa<BitCastInst>(Val) && !isa<GetElementPtrInst>(Val) &&
          isa<LoadInst>(Val))
        return false;

      Instruction *NewInst = dyn_cast<Instruction>(Val);

      // Keep iterating through the operand's chain until the Type matches
      // the va_list, else break
      while (NewInst) {
        if (NewInst->getType() == VAListType)
          return true;

        Value *ValOp = NewInst->getOperand(0);
        if (ValOp) {
          LocalPointerInfo &LPI = LPA.getLocalPointerInfo(ValOp);
          Type *OpType = LPI.getDominantAggregateTy();

          if (OpType == VAListType)
            return true;

          if (isa<BitCastInst>(ValOp) || isa<GetElementPtrInst>(ValOp) ||
              isa<LoadInst>(ValOp))
            NewInst = dyn_cast<Instruction>(ValOp);
          else
            break;
        } else
          break;
      }

      return false;
    };

    for (BasicBlock &BB : *Fn) {
      for (Instruction &Inst : BB) {

        // Check the Type in case VAArgInst is available
        if (VAArgInst *VarArg = dyn_cast<VAArgInst>(&Inst)) {
          if (VarArg->getType() != ArgType)
            return false;
        }

        // Note: The IR of a variadic function is represented
        // as follow:
        //
        // C code:
        //
        //  struct test {
        //    int i;
        //  };
        //
        //  void foo(int n, ...)
        //  {
        //    va_list vl;
        //    struct test loc;
        //    va_start(vl, n);
        //    loc = va_arg(vl, struct test);
        //    va_end(vl);
        //  }
        //
        // IR:
        //
        //  %struct.__va_list_tag = type { i32, i32, i8*, i8* }
        //  %struct.test = type { i32 }
        //
        //  define void @foo( i32 %n, ...) {
        //
        //    %vl = alloca [1 x %struct.__va_list_tag], align 16
        //            *
        //            *
        //            *
        //    %arraydecay = getelementptr inbounds [1 x %struct.__va_list_tag],
        //                  [1 x %struct.__va_list_tag]* %vl, i32 0, i32 0
        //    %arraydecay1 = bitcast %struct.__va_list_tag* %arraydecay to i8*
        //    call void @llvm.va_start(i8* %arraydecay1)
        //    %arraydecay2 = getelementptr inbounds
        //                   [1 x %struct.__va_list_tag],
        //                   [1 x %struct.__va_list_tag]* %vl, i32 0, i32 0
        //            *
        //            *
        //            *
        //    %gp_offset = load i32, i32* %gp_offset_p, align 16
        //    %fits_in_gp = icmp ule i32 %gp_offset, 40
        //    br i1 %fits_in_gp, label %vaarg.in_reg, label %vaarg.in_mem
        //
        //    vaarg.in_reg:
        //      %0 = getelementptr inbounds %struct.__va_list_tag,
        //           %struct.__va_list_tag* %arraydecay2, i32 0, i32 3
        //      %reg_save_area = load i8*, i8** %0, align 16
        //      %1 = getelementptr i8, i8* %reg_save_area, i32 %gp_offset
        //      %2 = bitcast i8* %1 to %struct.test*
        //            *
        //            *
        //            *
        //      br label %vaarg.end
        //
        //    vaarg.in_mem:
        //      %overflow_arg_area_p = getelementptr inbounds
        //                             %struct.__va_list_tag,
        //                             %struct.__va_list_tag* %arraydecay2,
        //                             i32 0, i32 2
        //       %overflow_arg_area = load i8*, i8** %overflow_arg_area_p,
        //                            align 8
        //       %4 = bitcast i8* %overflow_arg_area to %struct.test*
        //            *
        //            *
        //            *
        //       br label %vaarg.end
        //
        //    vaarg.end:
        //       %vaarg.addr = phi %struct.test* [ %2, %vaarg.in_reg ],
        //                     [ %4, %vaarg.in_mem ]
        //            *
        //            *
        //            *
        //  }
        //
        // In simple words:
        //
        // The array that contains the vararg is in the structure
        // %struct.__va_list_tag. Each entry will be accessed through
        // a GetElementPrInst that can be found inside an if/else path,
        // and merges in a PHINode. This PHINode is the actual vararg.
        //
        // To find the vararg:
        //
        //   1) Collect the Type of the input parameter to llvm.va_start
        //      (%struct.__va_list_tag)
        //   2) Find a PHINode and prove that all incoming Values for the
        //      node point to %struct.__va_list_tag

        // Collect %struct.__va_list_tag from llvm.va_start
        else if ((isa<CallInst>(&Inst) || isa<InvokeInst>(&Inst)) &&
                 (!VAListType)) {
          CallSite CS = CallSite(&Inst);
          Function *Fn = CS.getCalledFunction();
          if (Fn->isIntrinsic() && Fn->getIntrinsicID() == Intrinsic::vastart) {
            Value *Arg = CS.getArgument(0);
            LocalPointerInfo &LPI = LPA.getLocalPointerInfo(Arg);
            VAListType = LPI.getDominantAggregateTy();

            // Return false if no dominant Type was found
            if (!VAListType)
              return false;
          }
        }

        // Check that the PHINode points to %struct.__va_list_tag
        else if (VAListType && isa<PHINode>(&Inst)) {
          PHINode *Phi = cast<PHINode>(&Inst);
          Type *PHIType = Phi->getType();
          if (PHIType == ArgType)
            continue;

          unsigned i;
          for (i = 0; i < Phi->getNumIncomingValues(); i++) {
            Value *IncVal = Phi->getIncomingValue(i);
            // Return false if the Type of the PHINode is not the
            // same as the input Type, and the incoming Value
            // comes from the va_list. For example:
            //
            // Consider that ArgType is %struct.test2. In the previous
            // example we can see that the PHINode's Type is %struct.test.
            // Since there is a Type mismatch, then we need to check if
            // the incoming Values of the PHINode point to
            // %struct.__va_list_tag. The function checkIfVarArgType
            // may return true, which means that the input vararg's
            // Type is different compared to the Type that is being used
            // within the Function Fn.
            if (checkIfVarArgType(IncVal))
              return false;
          }
        }
      }
    }

    return true;
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

    auto *CompositeTy = cast<CompositeType>(BaseTy);

    // This happens with opaque structures and zero-element arrays.
    if (!CompositeTy->indexValid(0u))
      return false;

    auto *ElementZeroTy = CompositeTy->getTypeAtIndex(0u);
    if (!ElementZeroTy->isArrayTy())
      return false;

    return ElementZeroTy->getArrayElementType() ==
           ElementTy->getPointerElementType();
  }

  // Analyze a structure definition, independent of its use in any
  // instruction. This checks for basic issues like structure nesting
  // and empty structures.
  void analyzeStructureType(dtrans::TypeInfo *TI) {
    auto *Ty = dyn_cast<StructType>(TI->getLLVMType());
    if (!Ty)
      return;

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

    // Check if the structure has zero-sized array in the last field.
    if (dtrans::hasZeroSizedArrayAsLastField(Ty)) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Type has zero-sized array: "
                        << "  " << *Ty << "\n");
      setBaseTypeInfoSafetyData(Ty, dtrans::HasZeroSizedArray);
    }

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
        continue;
      }
      // Fields matching this check might not actually be vtables, but
      // we will treat them as though they are.
      if (ElementTy->isPointerTy() &&
          ElementTy->getPointerElementType()->isPointerTy() &&
          ElementTy->getPointerElementType()
              ->getPointerElementType()
              ->isFunctionTy()) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Has vtable\n"
                          << "  struct:  " << *Ty << "\n"
                          << "  element: " << *ElementTy << "\n");
        TI->setSafetyData(dtrans::HasVTable);
        continue;
      }
      if (ElementTy->isPointerTy() &&
          ElementTy->getPointerElementType()->isFunctionTy()) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Has function ptr:\n  " << *Ty
                          << "\n");
        TI->setSafetyData(dtrans::HasFnPtr);
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

    // If this can be interpreted as an element-zero access of SrcTy
    // or as a cast from a ptr-to-ptr to a type to a ptr-to-ptr to
    // element zero of that type, it's a safe cast.
    if (dtrans::isElementZeroAccess(SrcTy, DestTy) ||
        dtrans::isPtrToPtrToElementZeroAccess(SrcTy, DestTy))
      return;

    // If element zero is an i8* and we're casting the source value as a
    // pointer to a pointer, that is an element zero access but it isn't
    // reported as such because we need to recognize the i8*->(struct**)
    // bitcast as a potential problem for the destination type.
    if (DTInfo.isTypeOfInterest(DestTy) && SrcTy->isPointerTy() &&
        DestTy->isPointerTy() &&
        DestTy->getPointerElementType()->isPointerTy() &&
        dtrans::isElementZeroI8Ptr(SrcTy->getPointerElementType())) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
                        << "unsafe cast of i8* element to specific type:\n"
                        << "  " << I << "\n");
      // In this case we always want to avoid setting the element pointee
      // safety data, regardless of the state of DTransOutOfBoundsOK.
      (void)setValueTypeInfoSafetyDataBase(&I, dtrans::BadCasting);
      return;
    }

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
        DEBUG_WITH_TYPE(DTRANS_FSV, {
          dbgs() << "dtrans-fsv: " << *(StInfo->getLLVMType()) << " [" << Count
                 << "] ";
          NV->printAsOperand(dbgs());
        });
        FI.processNewSingleValue(NV);
        DEBUG_WITH_TYPE(DTRANS_FSV,
                        dbgs()
                            << (FI.isMultipleValue() ? " <MULTIPLE>\n" : "\n"));
        auto *ComponentTI = DTInfo.getTypeInfo(FI.getLLVMType());
        analyzeCallocSingleValue(ComponentTI);
        ++Count;
      }
    } else if (auto *AInfo = dyn_cast<dtrans::ArrayInfo>(TI)) {
      auto *ComponentTI = AInfo->getElementDTransInfo();
      analyzeCallocSingleValue(ComponentTI);
    }
  }

  void analyzeAllocationCall(CallSite CS, dtrans::AllocKind Kind) {

    // The LocalPointerAnalyzer will visit bitcast users to determine the
    // type of memory being allocated. This must be done in the
    // LocalPointerAnalyzer class rather than here because it is possible
    // that we will visit the bitcast first and the LocalPointerAnalyzer
    // must be able to identify the connection with the allocation call
    // in that case also.
    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(CS.getInstruction());

    // The list of aliased types in the LPI will be the types to which
    // this pointer is cast that were not identified as element zero
    // accesses. In almost all cases there will only be one such type.

    // If the malloc wasn't cast to a type of interest, we're finished.
    LocalPointerInfo::PointerTypeAliasSetRef &AliasSet =
        LPI.getPointerTypeAliasSet();
    if (AliasSet.empty()) {
      return;
    }

    dtrans::AllocCallInfo *ACI =
        DTInfo.createAllocCallInfo(CS.getInstruction(), Kind);
    populateCallInfoFromLPI(LPI, ACI);

    // If the value is cast to multiple types, mark them all as bad casting.
    bool WasCastToMultipleTypes = LPI.pointsToMultipleAggregateTypes();

    if (DTransPrintAllocations && WasCastToMultipleTypes)
      outs() << "dtrans: Detected allocation cast to multiple types.\n";

    // We expect to only see one type, but we loop to keep the code general.
    for (auto *Ty : AliasSet) {
      if (!DTInfo.isTypeOfInterest(Ty))
        continue;

      if (Kind == dtrans::AK_New) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: C++ handling -- "
                          << "new/new[] function call:\n  "
                          << *CS.getInstruction() << "\n");
        setBaseTypeInfoSafetyData(Ty, dtrans::HasCppHandling);
      }

      // If there are casts to multiple types of interest, they all get
      // handled as bad casts.
      if (WasCastToMultipleTypes) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
                          << "allocation cast to multiple types:\n  "
                          << *CS.getInstruction() << "\n");
        setBaseTypeInfoSafetyData(Ty, dtrans::BadCasting);
      }

      // Check the size of the allocation to make sure it's a multiple of the
      // size of the type being allocated.
      verifyAllocationSize(CS, Kind, cast<PointerType>(Ty));

      // Add this to our type info list.
      (void)DTInfo.getOrCreateTypeInfo(Ty);

      if (DTransPrintAllocations) {
        outs() << "dtrans: Detected allocation cast to pointer type\n";
        outs() << "  " << *CS.getInstruction() << "\n";
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
  void analyzeFreeCall(CallSite CS, dtrans::FreeKind FK) {
    LLVM_DEBUG(dbgs() << "dtrans: Analyzing free call.\n  "
                      << *CS.getInstruction() << "\n");
    dtrans::FreeCallInfo *FCI =
        DTInfo.createFreeCallInfo(CS.getInstruction(), FK);

    // If it's a standard free call, we can check for the type of the first
    // argument for the potential pointer types. If it's a user free call, there
    // is currently no guarantee to know which argument contains the TypeInfo.
    if (FK == dtrans::FK_Free || FK == dtrans::FK_Delete) {
      unsigned PtrArgInd = -1U;
      getFreePtrArg(FK, CS, PtrArgInd, TLI);
      Value *Arg = CS.getArgument(PtrArgInd);

      LocalPointerInfo &LPI = LPA.getLocalPointerInfo(Arg);
      LocalPointerInfo::PointerTypeAliasSetRef &AliasSet =
          LPI.getPointerTypeAliasSet();
      if (AliasSet.empty()) {
        return;
      }
      populateCallInfoFromLPI(LPI, FCI);
      if (FK == dtrans::FK_Delete)
        for (auto *Ty : AliasSet) {
          if (!DTInfo.isTypeOfInterest(Ty))
            continue;

          LLVM_DEBUG(dbgs() << "dtrans-safety: C++ handling -- "
                            << "delete/delete[] function call:\n  "
                            << *CS.getInstruction() << "\n");
          setBaseTypeInfoSafetyData(Ty, dtrans::HasCppHandling);
        }
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
      } else if (auto CS = CallSite(U)) {
        if (dtrans::isFreeFn(CS, TLI) || DTAA.isFreePostDom(CS))
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

  // Return true if the CallSite CS is to a suitably identified alloc
  // function.
  bool isSafeStoreForSingleAllocFunction(CallSite CS) {
    auto Kind = dtrans::getAllocFnKind(CS, TLI);
    if (Kind == dtrans::AK_NotAlloc && DTAA.isMallocPostDom(CS))
      Kind = dtrans::AK_UserMalloc;
    return Kind != dtrans::AK_NotAlloc;
  }

  static bool isLoadedValueUnused(Value *V, Value *LoadAddr,
                                  SmallPtrSetImpl<Value *> &UsedValues) {
    for (auto U : V->users()) {
      // If we've seen this user before, assume its path is OK.
      if (!UsedValues.insert(U).second)
        continue;

      // If the user is a call or invoke, the value escapes.
      // If needed this can be extended for pure functions.
      if (CallSite(U))
        return false;

      // If the value is used by a terminator, it's used.
      if (isa<TerminatorInst>(U))
        return false;

      // If the user is a store, check the target address.
      if (auto *SI = dyn_cast<StoreInst>(U)) {
        // If it is volatile or it doesn't match the load address, the value is
        // used.
        if (SI->isVolatile() || SI->getPointerOperand() != LoadAddr)
          return false;

        continue;
      }

      // If load is volatile, the value is used.
      if (auto *LI = dyn_cast<LoadInst>(U)) {
        if (LI->isVolatile())
          return false;
      }

      // Follow the users of any other user
      if (!isLoadedValueUnused(U, LoadAddr, UsedValues))
        return false;
    }

    // If the value has no users, this path is unused.
    return true;
  }

  // Returns true if the loaded value is stored to the same address from which
  // it was loaded and does not escape.
  bool identifyUnusedValue(LoadInst &LI) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (!DTransIdentifyUnusedValues)
      return false;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    SmallPtrSet<Value *, 4> UsedValues;
    return isLoadedValueUnused(&LI, LI.getPointerOperand(), UsedValues);
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

    // Update LoadInfoMap and StoreInfoMap if the instruction I is accessing
    // a structure element.
    auto &PointeeSet = PtrInfo.getElementPointeeSet();
    if (PointeeSet.size() == 1 && PointeeSet.begin()->first->isStructTy()) {
      if (IsLoad)
        DTInfo.addLoadMapping(cast<LoadInst>(&I), *PointeeSet.begin());
      else
        DTInfo.addStoreMapping(cast<StoreInst>(&I), *PointeeSet.begin());
    }

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

        // The value type must be the same as the field type, unless one
        // of the following conditions is met:
        //
        //   (1) FieldTy is a pointer type and ValTy is pointer-sized integer
        //   (2) FieldTy is a pointer type and ValTy is i8*
        //   (3) ValTy matches the type of element zero of FieldTy
        //   (4) FieldTy is a pointer type and ValTy is a pointer to the type
        //       of element zero of the type pointed to by FieldTy
        if ((FieldTy != ValTy) &&
            (!FieldTy->isPointerTy() ||
             (ValTy != PtrSizeIntTy && ValTy != Int8PtrTy)) &&
            !dtrans::isElementZeroAccess(FieldTy->getPointerTo(),
                                         ValTy->getPointerTo()) &&
            !dtrans::isPtrToPtrToElementZeroAccess(FieldTy->getPointerTo(),
                                                   ValTy->getPointerTo())) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Mismatched element access:\n");
          LLVM_DEBUG(dbgs() << "  " << I << "\n");

          if (DTransOutOfBoundsOK) {
            // Assuming out of bound access, set safety issue for the entire
            // ParentTy.
            setBaseTypeInfoSafetyData(ParentTy,
                                      dtrans::MismatchedElementAccess);
          } else {
            // Set safety issue to the ParentTy and to the impacted field type
            // only.
            DTInfo.getOrCreateTypeInfo(ParentTy)->setSafetyData(
                dtrans::MismatchedElementAccess);
            setBaseTypeInfoSafetyData(FieldTy, dtrans::MismatchedElementAccess);
          }
        }

        if (ParentTy->isStructTy()) {
          auto *ParentStInfo =
              cast<dtrans::StructInfo>(DTInfo.getOrCreateTypeInfo(ParentTy));
          dtrans::FieldInfo &FI = ParentStInfo->getField(PointeePair.second);
          if (IsLoad) {
            FI.setRead(true);
            if (!identifyUnusedValue(cast<LoadInst>(I)))
              FI.setValueUnused(false);
            accumulateFrequency(FI, I);
            if (!isSafeLoadForSingleAllocFunction(&I)) {
              DEBUG_WITH_TYPE(DTRANS_FSAF, {
                if (!FI.isBottomAllocFunction())
                  dbgs() << "dtrans-fsaf: " << *(ParentStInfo->getLLVMType())
                         << " [" << PointeePair.second << "] <BOTTOM>\n";
              });
              FI.setBottomAllocFunction();
            }
          } else {
            if (auto *ConstVal = dyn_cast<llvm::Constant>(WriteVal)) {
              if (FI.processNewSingleValue(ConstVal)) {
                DEBUG_WITH_TYPE(DTRANS_FSV, {
                  dbgs() << "dtrans-fsv: " << *(ParentStInfo->getLLVMType())
                         << " [" << PointeePair.second << "] New value: ";
                  ConstVal->printAsOperand(dbgs());
                  dbgs() << "\n";
                });
              }
              if (!isa<ConstantPointerNull>(WriteVal)) {
                DEBUG_WITH_TYPE(DTRANS_FSAF, {
                  if (!FI.isBottomAllocFunction())
                    dbgs() << "dtrans-fsaf: " << *(ParentStInfo->getLLVMType())
                           << " [" << PointeePair.second << "] <BOTTOM>\n";
                });
                FI.setBottomAllocFunction();
              }
            } else if (auto CS = CallSite(WriteVal)) {
              DEBUG_WITH_TYPE(DTRANS_FSV, {
                if (!FI.isMultipleValue())
                  dbgs() << "dtrans-fsv: " << *(ParentStInfo->getLLVMType())
                         << " [" << PointeePair.second << "] <MULTIPLE>\n";
              });
              FI.setMultipleValue();
              if (isSafeStoreForSingleAllocFunction(CS)) {
                Function *Callee = CS.getCalledFunction();
                if (FI.processNewSingleAllocFunction(Callee)) {
                  DEBUG_WITH_TYPE(DTRANS_FSAF, {
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
                DEBUG_WITH_TYPE(DTRANS_FSAF, {
                  if (!FI.isBottomAllocFunction())
                    dbgs() << "dtrans-fsaf: " << *(ParentStInfo->getLLVMType())
                           << " [" << PointeePair.second << "] <BOTTOM>\n";
                });
                FI.setBottomAllocFunction();
              }
            } else {
              DEBUG_WITH_TYPE(DTRANS_FSV, {
                if (!FI.isMultipleValue())
                  dbgs() << "dtrans-fsv: " << *(ParentStInfo->getLLVMType())
                         << " [" << PointeePair.second << "] <MULTIPLE>\n";
              });
              FI.setMultipleValue();
              DEBUG_WITH_TYPE(DTRANS_FSAF, {
                if (!FI.isBottomAllocFunction())
                  dbgs() << "dtrans-fsaf: " << *(ParentStInfo->getLLVMType())
                         << " [" << PointeePair.second << "] <BOTTOM>\n";
              });
              FI.setBottomAllocFunction();
            }
            FI.setWritten(true);
            accumulateFrequency(FI, I);
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
  void verifyAllocationSize(CallSite CS, dtrans::AllocKind Kind,
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
    unsigned AllocSizeInd = 0;
    unsigned AllocCountInd = 0;
    getAllocSizeArgs(Kind, CS, AllocSizeInd, AllocCountInd, TLI);

    auto *AllocSizeVal = CS.getArgOperand(AllocSizeInd);
    auto *AllocCountVal =
        AllocCountInd != -1U ? CS.getArgOperand(AllocCountInd) : nullptr;
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

    // If allocation size is not constant we can try tracing it back to the
    // constant
    uint64_t Res;
    if (!dtrans::isValueConstant(AllocSizeVal, &Res) &&
        traceNonConstantValue(Ty, AllocSizeVal, ElementSize))
      return;

    // Otherwise, we must assume the size arguments are not acceptable.
    LLVM_DEBUG(dbgs() << "dtrans-safety: Bad alloc size:\n"
                      << "  " << *CS.getInstruction() << "\n");
    setBaseTypeInfoSafetyData(Ty, dtrans::BadAllocSizeArg);
  }

  // Trace back instruction sequence corresponding to the following code:
  //     foo (..., int n, ...) {
  //         struct s *s_ptr = malloc(c1 + c2 * n);
  //     }
  // Returns false if it cannot trace \p InVal back to constants and calculate
  // the size.
  bool traceNonConstantValue(llvm::PointerType *Ty, Value *InVal,
                             uint64_t ElementSize) {
    if (!InVal)
      return false;

    Value *AddOp, *ShlOp;
    ConstantInt *AddC, *ShlC = nullptr, *MulC = nullptr;

    // Match alloc size with the add with the constant operand.
    if (!match(InVal, m_OneUse(m_Add(m_ConstantInt(AddC), m_Value(AddOp)))) &&
        !match(InVal, m_OneUse(m_Add(m_Value(AddOp), m_ConstantInt(AddC)))))
      return false;

    // Second add operand with the shl or mul with the constant operand.
    if (!match(AddOp, m_Shl(m_Value(ShlOp), m_ConstantInt(ShlC))) &&
        !match(AddOp, m_Mul(m_Value(ShlOp), m_ConstantInt(MulC))) &&
        !match(AddOp, m_Mul(m_ConstantInt(MulC), m_Value(ShlOp))))
      return false;

    // Second operand of the shl or mul expected to be function argument.
    Argument *FormalArg = dyn_cast<Argument>(ShlOp);
    if (!FormalArg)
      return false;

    // Now we need to look into each call site and find all constant values
    // for the corresponding argument. If not all actual arguments are constant,
    // return false.
    Function *Callee = FormalArg->getParent();
    unsigned FormalArgNo = FormalArg->getArgNo();

    SmallVector<ConstantInt *, 8> ActualArgs;
    if (!findAllArgValues(Callee, Callee, FormalArgNo, ActualArgs))
      return false;

    // Check if the structure has zero-sized array in the last field. It means
    // that allocation size could be greater or equal to the structure size.
    dtrans::TypeInfo *TI = DTInfo.getOrCreateTypeInfo(Ty->getElementType());
    bool HasZeroSizedArray = TI ? TI->hasZeroSizedArrayAsLastField() : false;

    // Now iterate through all constants to verify that the allocation size was
    // correct.
    bool Verified = true;
    for (auto *Const : ActualArgs) {
      uint64_t ArgConst = Const->getLimitedValue();
      uint64_t Res = ShlC ? (ArgConst << ShlC->getLimitedValue())
                          : (ArgConst * MulC->getLimitedValue());
      Res += AddC->getLimitedValue();

      if (!HasZeroSizedArray)
        Verified &= ((Res % ElementSize) == 0);
      else
        Verified &= (Res > ElementSize);
    }

    return Verified;
  }

  // Trace call sites to collect all constant actual parameters corresponding to
  // \p FormalArgNo.
  bool findAllArgValues(Function *F, Value *V, unsigned FormalArgNo,
                        SmallVector<ConstantInt *, 8> &ActualArgs) {
    for (Use &U : V->uses()) {
      Value *Inst = U.getUser();
      // In case of function cast operator do one more step.
      if (BitCastOperator *BC = dyn_cast<BitCastOperator>(Inst)) {
        if (!findAllArgValues(F, BC, FormalArgNo, ActualArgs))
          return false;
        continue;
      }

      CallSite CS(Inst);
      // Must be a direct call.
      if (CS.getInstruction() == nullptr)
        return false;
      // A called function should be F.
      if (!CS.isCallee(&U))
        if (U->stripPointerCasts() != F)
          return false;

      ConstantInt *ArgC =
          dyn_cast_or_null<ConstantInt>(CS.getArgument(FormalArgNo));
      if (!ArgC)
        return false;

      ActualArgs.push_back(ArgC);
    }

    return true;
  }

  // Mark fields designated by \p Info pointer and the memory \p Size to have
  // multiple values. IsNullValue indicates that the null value is written.
  void markPointerWrittenWithMultipleValue(LocalPointerInfo &Info, Value *Size,
                                           bool IsNullValue = false) {
    StructType *STy = nullptr;
    size_t FieldNum;

    // Identify the structure type that is pointed to by Info.
    if (!Info.pointsToSomeElement() ||
        !isSimpleStructureMember(Info, &STy, &FieldNum)) {

      FieldNum = 0;

      // Info doesn't point to a structure field, check if it points to a
      // structure or to an array of structures.

      // In case of multiple aliasing aggregate types, when it's not possible to
      // determine a dominant type, the caller should set AmbiguousPointerTarget
      // safety issue.
      auto *DTy = Info.getDominantAggregateTy();
      if (!DTy)
        return;

      // TODO: this is probably always a pointer type.
      if (DTy->isPointerTy())
        DTy = DTy->getPointerElementType();

      if (!DTy->isAggregateType())
        return;

      // Get to the first non-array type.
      while (auto *ATy = dyn_cast<ArrayType>(DTy))
        DTy = ATy->getElementType();

      STy = dyn_cast<StructType>(DTy);
    }

    // If the pointee type is not a structure we have no interest in it.
    if (!STy)
      return;

    auto *SL = DL.getStructLayout(STy);
    auto StructSize = SL->getSizeInBytes();

    // Identify a size of the memory operation.
    uint64_t WriteSize;
    if (!dtrans::isValueConstant(Size, &WriteSize)) {
      if (dtrans::isValueMultipleOfSize(Size, StructSize)) {
        WriteSize = StructSize;
      } else {
        markAllFieldsMultipleValue(DTInfo.getOrCreateTypeInfo(STy));
        return;
      }
    }

    auto *SInfo = cast<dtrans::StructInfo>(DTInfo.getOrCreateTypeInfo(STy));

    auto WriteBound = SL->getElementOffset(FieldNum) + WriteSize;
    if (WriteBound > StructSize) {
      // Memory operation writes outside of the identified type
      markAllFieldsMultipleValue(SInfo);
      return;
    }

    // Mark every field touched by the memory operation.
    auto LastField = SL->getElementContainingOffset(WriteBound - 1);

    uint64_t LastFieldStart = SL->getElementOffset(LastField);
    uint64_t LastFieldSize =
        DL.getTypeStoreSize(STy->getElementType(LastField));
    bool LastFieldPartialAccess =
        (WriteBound < (LastFieldStart + LastFieldSize - 1));

    for (; FieldNum <= LastField; ++FieldNum) {
      auto &FInfo = SInfo->getField(FieldNum);
      if (IsNullValue && (FieldNum != LastField || !LastFieldPartialAccess)) {
        // If setting a null value and the last field is not accessed
        // partially.
        FInfo.processNewSingleValue(
            Constant::getNullValue(FInfo.getLLVMType()));
        markAllFieldsMultipleValue(DTInfo.getTypeInfo(FInfo.getLLVMType()),
                                   true);
      } else {
        LLVM_DEBUG(dbgs() << "dtrans-fsv: " << *(SInfo->getLLVMType()) << " ["
                          << FieldNum << "] <MULTIPLE>\n");

        FInfo.setBottomAllocFunction();
        FInfo.setMultipleValue();
        markAllFieldsMultipleValue(DTInfo.getTypeInfo(FInfo.getLLVMType()));
      }
    }
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

    MemSetInst &Inst = cast<MemSetInst>(I);
    auto *DestArg = Inst.getRawDest();
    Value *SetSize = Inst.getLength();

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

    // Identify if the memset value is zero, propagate zero value to the fields.
    bool IsNullValue = dtrans::isValueEqualToSize(Inst.getValue(), 0);
    markPointerWrittenWithMultipleValue(DstLPI, SetSize, IsNullValue);

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

    auto *ParentTI = DTInfo.getOrCreateTypeInfo(DestPointeeTy);

    // Consider the case where the complete aggregate (or an array of
    // aggregates is being set).
    if (dtrans::isValueMultipleOfSize(SetSize, ElementSize)) {
      // It is a safe use. Mark all the fields as being written.
      markAllFieldsWritten(ParentTI, I);

      dtrans::MemfuncRegion RegionDesc;
      RegionDesc.IsCompleteAggregate = true;
      createMemsetCallInfo(I, cast<PointerType>(DestParentTy), RegionDesc);

      return;
    }

    // Consider the case where a portion of a structure is being set, starting
    // from the 1st field.
    if (auto *StructTy = dyn_cast<StructType>(DestPointeeTy)) {
      dtrans::MemfuncRegion RegionDesc;
      if (analyzeMemfuncStructureMemberParam(I, StructTy, 0, SetSize,
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

    LocalPointerInfo &DstLPI = LPA.getLocalPointerInfo(DestArg);
    LocalPointerInfo &SrcLPI = LPA.getLocalPointerInfo(SrcArg);
    auto *DestParentTy = DstLPI.getDominantAggregateTy();
    auto *SrcParentTy = SrcLPI.getDominantAggregateTy();

    if (!DestOfInterest && !SrcOfInterest) {
      return;
    }

    if (!SrcOfInterest || !DestOfInterest) {
      LLVM_DEBUG(
          dbgs() << "dtrans-safety: Bad memfunc manipulation --  "
                 << "Either source or destination operand is fundamental "
                    "pointer type:\n"
                 << "  " << I << "\n");

      setValueTypeInfoSafetyData(SrcArg, dtrans::BadMemFuncManipulation);
      setValueTypeInfoSafetyData(DestArg, dtrans::BadMemFuncManipulation);
      markPointerWrittenWithMultipleValue(DstLPI, SetSize);
      return;
    }

    // If we get here, both parameters are types of interest.
    if (!DestParentTy || !SrcParentTy) {
      if (!DestParentTy &&
          isAliasSetOverloaded(DstLPI.getPointerTypeAliasSet())) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Ambiguous pointer target -- "
                          << "Aliased type for destination operand:\n"
                          << "  " << I << "\n");

        setAllAliasedTypeSafetyData(DstLPI, dtrans::AmbiguousPointerTarget);
        setAllAliasedTypeSafetyData(SrcLPI, dtrans::BadMemFuncManipulation);
        return;
      }

      if (!SrcParentTy &&
          isAliasSetOverloaded(SrcLPI.getPointerTypeAliasSet())) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Bad memfunc manipulation -- "
                          << "Aliased type for source operand.\n"
                          << "  " << I << "\n");

        setAllAliasedTypeSafetyData(DstLPI, dtrans::BadMemFuncManipulation);
        setAllAliasedTypeSafetyData(SrcLPI, dtrans::AmbiguousPointerTarget);
        markPointerWrittenWithMultipleValue(DstLPI, SetSize);
        return;
      }

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

    if (DestParentTy != SrcParentTy) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Bad memfunc manipulation -- "
                        << "Different types for source and destination:\n"
                        << "  " << I << "\n");

      setValueTypeInfoSafetyData(DestArg, dtrans::BadMemFuncManipulation);
      setValueTypeInfoSafetyData(SrcArg, dtrans::BadMemFuncManipulation);
      markPointerWrittenWithMultipleValue(DstLPI, SetSize);
      return;
    }

    bool DstPtrToMember = DstLPI.pointsToSomeElement();
    bool SrcPtrToMember = SrcLPI.pointsToSomeElement();

    // For simplicity, require either both elements to be pointers to members,
    // or neither element to be. This is a conservative approach, but otherwise
    // any transforms will have to deal the complexity of the types when
    // memcpy/memmove calls have to be modified.
    if (DstPtrToMember != SrcPtrToMember) {
      // Conservatively set destination pointer to unknown value.
      markPointerWrittenWithMultipleValue(DstLPI, SetSize);

      // Do not set safety issue if the memfunc call affects only one field.
      StructType *StructType;
      size_t FieldNum;
      uint64_t AccessSize;
      bool IsConstantSize = dtrans::isValueConstant(SetSize, &AccessSize);
      bool IsSimple = isSimpleStructureMember(DstPtrToMember ? DstLPI : SrcLPI,
                                              &StructType, &FieldNum);

      if (IsConstantSize && IsSimple &&
          (DL.getTypeStoreSize(StructType->getElementType(FieldNum)) ==
           AccessSize)) {

        // If write to a structure field.
        dtrans::MemfuncRegion RegionDesc;
        RegionDesc.IsCompleteAggregate = false;
        RegionDesc.FirstField = FieldNum;
        RegionDesc.LastField = FieldNum;

        // Create memfunc info for a single field access.
        createMemcpyOrMemmoveCallInfo(
            I, (DstPtrToMember ? DestParentTy : SrcParentTy)->getPointerTo(),
            Kind, RegionDesc, RegionDesc);

        auto *StructInfo =
            cast<dtrans::StructInfo>(DTInfo.getOrCreateTypeInfo(StructType));
        auto &FieldInfo = StructInfo->getField(FieldNum);

        if (DstPtrToMember) {
          FieldInfo.setWritten(true);
        } else {
          assert(SrcPtrToMember);
          FieldInfo.setRead(true);
        }

        return;
      }

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
        if (analyzeMemfuncStructureMemberParam(I, DstStructTy, DstFieldNum,
                                               SetSize, RegionDesc))
          createMemcpyOrMemmoveCallInfo(I, DstStructTy->getPointerTo(), Kind,
                                        RegionDesc, RegionDesc);
        else
          markPointerWrittenWithMultipleValue(DstLPI, SetSize);

        return;
      } else {
        LLVM_DEBUG(
            dbgs() << "dtrans-safety: Bad memfunc manipulation -- "
                   << "source and destination pointer to member types or "
                      "offsets do not match:\n"
                   << "  " << I << "\n");

        setBaseTypeInfoSafetyData(DstStructTy, dtrans::BadMemFuncManipulation);
        setBaseTypeInfoSafetyData(SrcStructTy, dtrans::BadMemFuncManipulation);
        markPointerWrittenWithMultipleValue(DstLPI, SetSize);
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
      markAllFieldsWritten(ParentTI, I);

      // The copy/move is the complete aggregate of the source and destination,
      // which are the same types/
      dtrans::MemfuncRegion RegionDesc;
      RegionDesc.IsCompleteAggregate = true;
      createMemcpyOrMemmoveCallInfo(I, cast<PointerType>(DestParentTy), Kind,
                                    RegionDesc, RegionDesc);

      return;
    }

    // Consider the case where a portion of a structure is being set, starting
    // from the 1st field.
    if (auto *StructTy = dyn_cast<StructType>(DestPointeeTy)) {
      dtrans::MemfuncRegion RegionDesc;
      if (analyzeMemfuncStructureMemberParam(I, StructTy, 0, SetSize,
                                             RegionDesc))
        createMemcpyOrMemmoveCallInfo(I, StructTy->getPointerTo(), Kind,
                                      RegionDesc, RegionDesc);
      else
        markPointerWrittenWithMultipleValue(DstLPI, SetSize);

      return;
    }

    LLVM_DEBUG(dbgs() << "dtrans-safety: Bad memfunc size -- "
                      << "size is not a multiple of type size:\n"
                      << "  " << I << "\n");

    setValueTypeInfoSafetyData(DestArg, dtrans::BadMemFuncSize);
    markPointerWrittenWithMultipleValue(DstLPI, SetSize);
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

  void markFieldsComplexUse(llvm::Type *Ty, unsigned First, unsigned Last) {
    if (auto *StInfo =
            dyn_cast<dtrans::StructInfo>(DTInfo.getOrCreateTypeInfo(Ty)))
      for (auto I = First, E = Last + 1; I != E; ++I)
        StInfo->getField(I).setComplexUse(true);
  }

  // Create a MemfuncCallInfo object that will store the details about a safe
  // memset call.
  void createMemsetCallInfo(Instruction &I, llvm::PointerType *Ty,
                            dtrans::MemfuncRegion &RegionDesc) {
    dtrans::MemfuncCallInfo *MCI = DTInfo.createMemfuncCallInfo(
        &I, dtrans::MemfuncCallInfo::MK_Memset, RegionDesc);
    MCI->setAliasesToAggregatePointer(true);
    MCI->setAnalyzed(true);
    MCI->addType(Ty);

    if (!RegionDesc.IsCompleteAggregate)
      markFieldsComplexUse(Ty->getElementType(), RegionDesc.FirstField,
                           RegionDesc.LastField);
  }

  // Create a MemfuncCallInfo object that will store the details about a safe
  // memcpy/memmove call.
  void createMemcpyOrMemmoveCallInfo(Instruction &I, llvm::PointerType *Ty,
                                     dtrans::MemfuncCallInfo::MemfuncKind Kind,
                                     dtrans::MemfuncRegion &RegionDescDest,
                                     dtrans::MemfuncRegion &RegionDescSrc) {
    dtrans::MemfuncCallInfo *MCI =
        DTInfo.createMemfuncCallInfo(&I, Kind, RegionDescDest, RegionDescSrc);
    MCI->setAliasesToAggregatePointer(true);
    MCI->setAnalyzed(true);
    MCI->addType(Ty);

    if (!RegionDescDest.IsCompleteAggregate)
      markFieldsComplexUse(Ty->getElementType(), RegionDescDest.FirstField,
                           RegionDescDest.LastField);
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
                                          dtrans::MemfuncRegion &RegionDesc) {
    auto *ParentTI = DTInfo.getOrCreateTypeInfo(StructTy);

    // Try to determine if a set of fields in a structure is being written.
    if (analyzePartialStructUse(StructTy, FieldNum, SetSize, &RegionDesc)) {
      // If not all members of the structure were set, mark it as
      // a partial write.
      if (!RegionDesc.IsCompleteAggregate) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Memfunc partial write -- "
                          << "size is a subset of fields:\n"
                          << "  " << I << "\n");

        ParentTI->setSafetyData(dtrans::MemFuncPartialWrite);
      }
      markStructFieldsWritten(ParentTI, RegionDesc.FirstField,
                              RegionDesc.LastField, I);
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
  void markAllFieldsWritten(dtrans::TypeInfo *TI, Instruction &I) {
    if (TI == nullptr)
      return;

    if (!TI->getLLVMType()->isAggregateType()) {
      return;
    }

    if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI)) {
      for (auto &FI : StInfo->getFields()) {
        FI.setWritten(true);
        accumulateFrequency(FI, I);
        auto *ComponentTI = DTInfo.getTypeInfo(FI.getLLVMType());
        markAllFieldsWritten(ComponentTI, I);
      }
    } else if (auto *AInfo = dyn_cast<dtrans::ArrayInfo>(TI)) {
      auto *ComponentTI = AInfo->getElementDTransInfo();
      markAllFieldsWritten(ComponentTI, I);
    }

    return;
  }

  // A specialized form of the MarkAllFieldsWritten that is used to mark a
  // subset of fields of a structure type as written. Any contained aggregates
  // within the subset are marked as completely written.
  void markStructFieldsWritten(dtrans::TypeInfo *TI, unsigned int FirstField,
                               unsigned int LastField, Instruction &I) {
    assert(TI && TI->getLLVMType()->isStructTy() &&
           "markStructFieldsWritten requires Structure type");

    auto *StInfo = cast<dtrans::StructInfo>(TI);
    assert(LastField >= FirstField && LastField < StInfo->getNumFields() &&
           "markStructFieldsWritten with invalid field index");

    for (unsigned int Idx = FirstField; Idx <= LastField; ++Idx) {
      auto &FI = StInfo->getField(Idx);
      FI.setWritten(true);
      accumulateFrequency(FI, I);
      auto *ComponentTI = DTInfo.getTypeInfo(FI.getLLVMType());
      markAllFieldsWritten(ComponentTI, I);
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
        DEBUG_WITH_TYPE(DTRANS_FSV,
                        dbgs() << "dtrans-fsv: " << *(StInfo->getLLVMType())
                               << " [" << I << "] ");
        if (ConstVal->getType() == FieldTy) {
          DEBUG_WITH_TYPE(DTRANS_FSV, ConstVal->printAsOperand(dbgs()));
          FI.processNewSingleValue(ConstVal);
          DEBUG_WITH_TYPE(
              DTRANS_FSV,
              dbgs() << (FI.isMultipleValue() ? " <MULTIPLE>\n" : "\n"));
        } else {
          DEBUG_WITH_TYPE(DTRANS_FSV, dbgs() << "<MULTIPLE>\n");
          FI.setMultipleValue();
        }
        if (!isa<ConstantPointerNull>(ConstVal)) {
          DEBUG_WITH_TYPE(DTRANS_FSAF, {
            if (!FI.isBottomAllocFunction())
              dbgs() << "dtrans-fsaf: " << *(StInfo->getLLVMType()) << " [" << I
                     << "] <BOTTOM>\n";
          });
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
  // Mark all fields of TI to have multiple or null values, depending on the
  // IsNullValue flag. It's used in the memset() analysis to handle null value
  // case.
  //
  void markAllFieldsMultipleValue(dtrans::TypeInfo *TI,
                                  bool IsNullValue = false) {
    if (TI == nullptr)
      return;
    if (!TI->getLLVMType()->isAggregateType())
      return;
    if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI)) {
      int Count = 0;
      for (auto &FI : StInfo->getFields()) {
        DEBUG_WITH_TYPE(DTRANS_FSV,
                        dbgs() << "dtrans-fsv: " << *(StInfo->getLLVMType())
                               << " [" << Count << "] <MULTIPLE>\n");
        if (IsNullValue)
          FI.processNewSingleValue(Constant::getNullValue(FI.getLLVMType()));
        else {
          FI.setBottomAllocFunction();
          FI.setMultipleValue();
        }

        auto *ComponentTI = DTInfo.getTypeInfo(FI.getLLVMType());
        markAllFieldsMultipleValue(ComponentTI, IsNullValue);
        ++Count;
      }
    } else if (auto *AInfo = dyn_cast<dtrans::ArrayInfo>(TI)) {
      auto *ComponentTI = AInfo->getElementDTransInfo();
      markAllFieldsMultipleValue(ComponentTI, IsNullValue);
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
    case dtrans::HasZeroSizedArray:
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

  void setBaseTypeCallGraph(llvm::Type *Ty, Function *F) {
    // Strip only outermost Pointer type constructors to account
    // for load/stores/calls, etc. instructions.
    llvm::Type *BaseTy = Ty;
    while (BaseTy->isPointerTy())
      BaseTy = BaseTy->getPointerElementType();

    // This lambda encapsulates the logic for propagating CallGraph info
    // to structure fields and array elements to account for implicit types
    // in GEP. Pointer types should be handled indirectly through
    // load/stores/calls, etc.
    auto &DT = DTInfo;
    std::function<void(llvm::Type *)> Propagate =
        [&DT, F, &Propagate](llvm::Type *Ty) -> void {
      if (!DT.isTypeOfInterest(Ty))
        return;
      if (auto *STy = dyn_cast<StructType>(Ty)) {
        cast<dtrans::StructInfo>(DT.getOrCreateTypeInfo(STy))
            ->insertCallGraphNode(F);
        for (auto FTy : STy->elements())
          Propagate(FTy);
      } else if (auto *ATy = dyn_cast<ArrayType>(Ty))
        Propagate(ATy->getElementType());
    };

    Propagate(BaseTy);
  }
};

} // end anonymous namespace

// Computes total frequency of all fields and sets TotalFrequency of
// \p StInfo.
void DTransAnalysisInfo::computeStructFrequency(dtrans::StructInfo *StInfo) {
  uint64_t StructFreq = 0;
  for (dtrans::FieldInfo &FI : StInfo->getFields()) {
    uint64_t TFreq = StructFreq + FI.getFrequency();
    // Check for overflow.
    if (TFreq < StructFreq) {
      StructFreq = std::numeric_limits<uint64_t>::max();
      break;
    }
    StructFreq = TFreq;
  }
  StInfo->setTotalFrequency(StructFreq);
}

// Return true if we are interested in tracking values of the specified type.
//
// For pointer types, we peel off the pointers to find the base type. We
// are primarily interested in named structures. We also include literal
// structures and arrays that have at least one element that is a type of
// interest (which is to see that they contain a named structure at some level
// of nesting).
bool DTransAnalysisInfo::isTypeOfInterest(llvm::Type *Ty) {
  llvm::Type *BaseTy = Ty;

  // For pointers, see what they point to.
  while (BaseTy->isPointerTy())
    BaseTy = cast<PointerType>(BaseTy)->getElementType();

  if (!BaseTy->isAggregateType() || !BaseTy->isSized())
    return false;

  // Skip literal structures unless one of their elements is a type of interest.
  if (auto *StTy = dyn_cast<StructType>(BaseTy)) {
    if (StTy->isLiteral()) {
      bool HasElementOfInterest = false;
      for (auto *ElemTy : StTy->elements()) {
        if (isTypeOfInterest(ElemTy)) {
          HasElementOfInterest = true;
          break;
        }
      }
      return HasElementOfInterest;
    }
    // Non-literal structs are always of interest.
    return true;
  }

  // Based on isAggregateType and this not being a StructType, it must be
  // an array type.
  assert(BaseTy->isArrayTy() && "Unexpected aggregate type");

  // Skip arrays whose elements are not types of interest.
  return isTypeOfInterest(BaseTy->getArrayElementType());
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
    llvm::StructType *STy = cast<StructType>(Ty);
    SmallVector<llvm::Type *, 16> FieldTypes;
    for (llvm::Type *FieldTy : STy->elements()) {
      FieldTypes.push_back(FieldTy);
      // Create a DTrans type for the field, in case it is an aggregate.
      (void)getOrCreateTypeInfo(FieldTy);
    }
    DTransTy = new dtrans::StructInfo(Ty, FieldTypes, 0);
  } else {
    assert(!Ty->isAggregateType() &&
           "DTransAnalysisInfo::getOrCreateTypeInfo unexpected aggregate type");
    DTransTy = new dtrans::NonAggregateTypeInfo(Ty);
  }

  TypeInfoMap[Ty] = DTransTy;
  return DTransTy;
}

dtrans::CallInfo *
DTransAnalysisInfo::getCallInfo(const llvm::Instruction *I) const {
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

void DTransAnalysisInfo::addLoadMapping(
    LoadInst *LdInst, std::pair<llvm::Type *, size_t> Pointee) {
  LoadInfoMap[LdInst] = Pointee;
}

void DTransAnalysisInfo::addStoreMapping(
    StoreInst *StInst, std::pair<llvm::Type *, size_t> Pointee) {
  StoreInfoMap[StInst] = Pointee;
}

std::pair<llvm::Type *, size_t>
DTransAnalysisInfo::getStoreElement(StoreInst *StInst) {
  auto It = StoreInfoMap.find(StInst);
  if (It == StoreInfoMap.end())
    return std::make_pair(nullptr, 0);
  return It->second;
}

std::pair<llvm::Type *, size_t>
DTransAnalysisInfo::getLoadElement(LoadInst *LdInst) {
  auto It = LoadInfoMap.find(LdInst);
  if (It == LoadInfoMap.end())
    return std::make_pair(nullptr, 0);
  return It->second;
}

void DTransAnalysisInfo::addGenericLoadMapping(LoadInst *LI, llvm::Type *Ty) {
  GenericLoadInfoMap[LI] = Ty;
}

void DTransAnalysisInfo::addGenericStoreMapping(StoreInst *SI, llvm::Type *Ty) {
  GenericStoreInfoMap[SI] = Ty;
}

llvm::Type *DTransAnalysisInfo::getGenericLoadType(LoadInst *LI) {
  auto It = GenericLoadInfoMap.find(LI);
  if (It == GenericLoadInfoMap.end())
    return nullptr;
  return It->second;
}

llvm::Type *DTransAnalysisInfo::getGenericStoreType(StoreInst *SI) {
  auto It = GenericStoreInfoMap.find(SI);
  if (It == GenericStoreInfoMap.end())
    return nullptr;
  return It->second;
}

std::pair<llvm::Type *, size_t>
DTransAnalysisInfo::getByteFlattenedGEPElement(GetElementPtrInst *GEP) {
  auto It = ByteFlattenedGEPInfoMap.find(GEP);
  if (It == ByteFlattenedGEPInfoMap.end())
    return std::make_pair(nullptr, 0);
  return It->second;
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
INITIALIZE_PASS_DEPENDENCY(BlockFrequencyInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
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

  auto GetBFI = [this](Function &F) -> BlockFrequencyInfo & {
    return this->getAnalysis<BlockFrequencyInfoWrapperPass>(F).getBFI();
  };

  return Result.analyzeModule(
      M, getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(),
      getAnalysis<WholeProgramWrapperPass>().getResult(), GetBFI);
}

DTransAnalysisInfo::DTransAnalysisInfo()
    : MaxTotalFrequency(0), FunctionCount(0), CallsiteCount(0),
      InstructionCount(0), DTransAnalysisRan(false) {}

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
  StoreInfoMap.insert(Other.StoreInfoMap.begin(), Other.StoreInfoMap.end());
  LoadInfoMap.insert(Other.LoadInfoMap.begin(), Other.LoadInfoMap.end());
  GenericStoreInfoMap.insert(Other.GenericStoreInfoMap.begin(),
                             Other.GenericStoreInfoMap.end());
  GenericLoadInfoMap.insert(Other.GenericLoadInfoMap.begin(),
                            Other.GenericLoadInfoMap.end());
  MaxTotalFrequency = Other.MaxTotalFrequency;
  FunctionCount = Other.FunctionCount;
  CallsiteCount = Other.CallsiteCount;
  InstructionCount = Other.InstructionCount;
  DTransAnalysisRan = Other.DTransAnalysisRan;
}

DTransAnalysisInfo::~DTransAnalysisInfo() { reset(); }

DTransAnalysisInfo &DTransAnalysisInfo::operator=(DTransAnalysisInfo &&Other) {
  reset();
  TypeInfoMap = std::move(Other.TypeInfoMap);
  CallInfoMap = std::move(Other.CallInfoMap);
  PtrSubInfoMap.insert(Other.PtrSubInfoMap.begin(), Other.PtrSubInfoMap.end());
  ByteFlattenedGEPInfoMap.insert(Other.ByteFlattenedGEPInfoMap.begin(),
                                 Other.ByteFlattenedGEPInfoMap.end());
  StoreInfoMap.insert(Other.StoreInfoMap.begin(), Other.StoreInfoMap.end());
  LoadInfoMap.insert(Other.LoadInfoMap.begin(), Other.LoadInfoMap.end());
  GenericStoreInfoMap.insert(Other.GenericStoreInfoMap.begin(),
                             Other.GenericStoreInfoMap.end());
  GenericLoadInfoMap.insert(Other.GenericLoadInfoMap.begin(),
                            Other.GenericLoadInfoMap.end());
  MaxTotalFrequency = Other.MaxTotalFrequency;
  FunctionCount = Other.FunctionCount;
  CallsiteCount = Other.CallsiteCount;
  InstructionCount = Other.InstructionCount;
  DTransAnalysisRan = Other.DTransAnalysisRan;
  IgnoreTypeMap = std::move(Other.IgnoreTypeMap);
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
  PtrSubInfoMap.clear();
  ByteFlattenedGEPInfoMap.clear();
  StoreInfoMap.clear();
  LoadInfoMap.clear();
  GenericStoreInfoMap.clear();
  GenericLoadInfoMap.clear();
  TypeInfoMap.clear();
  IgnoreTypeMap.clear();
}

// Parse 'dtrans-nosafetychecks-list' option and collect a map of
// transformations to the list of type names to be ignored.
// Syntax: -dtrans-nosafetychecks-list="record(;record)*"
//                     record := transform_name:type_name(,type_name)*
void DTransAnalysisInfo::parseIgnoreList() {
  if (!DTransNoSafetyChecksList.empty()) {
    LLVM_DEBUG(dbgs() << "\ndtrans-nosafetychecks-list: ");
    for (auto &List : DTransNoSafetyChecksList) {
      StringRef IgnoreList(List);
      if (IgnoreList.empty()) {
        continue;
      }
      SmallVector<StringRef, 20> IgnoreListElements;
      IgnoreList.split(IgnoreListElements, ";");
      for (auto Element : IgnoreListElements) {
        std::pair<StringRef, StringRef> TransformationAndTypes =
            Element.split(":");
        if (TransformationAndTypes.first.empty() ||
            TransformationAndTypes.second.empty()) {
          LLVM_DEBUG(dbgs() << "\n\tSkipping \'" << Element
                            << "\': transform name or types list is missing");
          continue;
        }
        dtrans::Transform TransName;
        if (TransformationAndTypes.first == "fsv")
          TransName = dtrans::DT_FieldSingleValue;
        else if (TransformationAndTypes.first == "fsaf")
          TransName = dtrans::DT_FieldSingleAllocFunction;
        else if (TransformationAndTypes.first == "reorderfields")
          TransName = dtrans::DT_ReorderFields;
        else if (TransformationAndTypes.first == "deletefield")
          TransName = dtrans::DT_DeleteField;
        else if (TransformationAndTypes.first == "aostosoa")
          TransName = dtrans::DT_AOSToSOA;
        else if (TransformationAndTypes.first == "aostosoadependent")
          TransName = dtrans::DT_AOSToSOADependent;
        else if (TransformationAndTypes.first == "aostosoadependentindex32")
          TransName = dtrans::DT_AOSToSOADependentIndex32;
        else if (TransformationAndTypes.first == "elimrofieldaccess")
          TransName = dtrans::DT_ElimROFieldAccess;
        else if (TransformationAndTypes.first == "dynclone")
          TransName = dtrans::DT_DynClone;
        else if (TransformationAndTypes.first == "soatoaos")
          TransName = dtrans::DT_SOAToAOS;
        else {
          LLVM_DEBUG(dbgs() << "\n\tSkipping \'" << Element
                            << "\': bad transformation name");
          continue;
        }
        LLVM_DEBUG(dbgs() << "\n\tAdding   \'" << Element << "\' ");
        SmallVector<StringRef, 20> IgnoreTypes;
        TransformationAndTypes.second.split(IgnoreTypes, ",");
        for (auto TypeName : IgnoreTypes)
          IgnoreTypeMap[TransName].insert(TypeName);
      }
    }
    LLVM_DEBUG(dbgs() << "\n");
  }
}

// Returns true if type has a safety violation and it's not in the ignore
// list.
bool DTransAnalysisInfo::testSafetyData(dtrans::TypeInfo *TyInfo,
                                        dtrans::Transform Transform) {
  assert(!(Transform & ~dtrans::DT_Legal) && "Illegal transform");

  dtrans::SafetyData Conditions = dtrans::getConditionsForTransform(Transform);
  bool checkFailed = TyInfo->testSafetyData(Conditions);

  // If there were no safety check violations, then no need to check ignore
  // list.
  if (!checkFailed)
    return false;

  if (!IgnoreTypeMap[Transform].empty())
    if (llvm::Type *Ty = TyInfo->getLLVMType())
      if (Ty->isStructTy()) {
        StringRef Name = dtrans::getStructName(Ty);
        // Cut the "{dtrans_opt_prefix}struct." from the LLVM type name.
        std::pair<StringRef, StringRef> StructPrefixAndName = Name.split('.');
        if (!StructPrefixAndName.second.empty())
          if (IgnoreTypeMap[Transform].find(StructPrefixAndName.second) !=
              IgnoreTypeMap[Transform].end())
            if (checkFailed) {
              // The type is in the ignore list and indeed violated safety
              // conditions. So print a note, discard the check and return
              // 'false'.
              checkFailed = false;
              dyn_cast<dtrans::StructInfo>(TyInfo)->setIgnoredFor(Transform);
              LLVM_DEBUG(dbgs() << "dtrans-"
                                << dtrans::getStringForTransform(Transform)
                                << ": ignoring " << dtrans::getStructName(Ty)
                                << " by user demand\n");
            }
      }
  return checkFailed;
}

// Count up the number of functions, callsites, and instructions to see
// if the call graph is too large for DtransAnalysis
void DTransAnalysisInfo::setCallGraphStats(Module &M) {
  FunctionCount = 0;
  CallsiteCount = 0;
  InstructionCount = 0;
  for (auto &F : M.functions()) {
    FunctionCount++;
    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        if (isa<DbgInfoIntrinsic>(I) || isa<FakeloadInst>(I) ||
            isa<VarAnnotIntrinsic>(I))
          continue;
        InstructionCount++;
        if (isa<CallInst>(&I) || isa<InvokeInst>(&I))
          CallsiteCount++;
      }
    }
  }
  LLVM_DEBUG(dbgs() << "Call Graph Stats:\n"
                    << "  Functions:    " << FunctionCount
                    << "  Callsites:    " << CallsiteCount
                    << "  Instructions: " << InstructionCount << "\n");
}

// Return true if we should run DTransAnalysis.
// Right now, we just disable DTransAnalysis if the call graph is too large.
bool DTransAnalysisInfo::shouldComputeDTransAnalysis(void) const {
  if (CallsiteCount > DTransMaxCallsiteCount ||
      InstructionCount > DTransMaxInstructionCount) {
    LLVM_DEBUG(dbgs() << "dtrans: Call graph too large ..."
                      << "DTransAnalysis didn't run\n");
    return false;
  }
  return true;
}

// Return true if DTransAnalysis has been run and can be used in
// transformations.
bool DTransAnalysisInfo::useDTransAnalysis(void) const {
  return DTransAnalysisRan;
}

bool DTransAnalysisInfo::getDTransOutOfBoundsOK() {
  return DTransOutOfBoundsOK;
}

bool DTransAnalysisInfo::analyzeModule(
    Module &M, TargetLibraryInfo &TLI, WholeProgramInfo &WPInfo,
    function_ref<BlockFrequencyInfo &(Function &)> GetBFI) {

  if (!WPInfo.isWholeProgramSafe()) {
    LLVM_DEBUG(dbgs() << "dtrans: Whole Program not safe ... "
                      << "DTransAnalysis didn't run\n");
    return false;
  }

  setCallGraphStats(M);

  if (!shouldComputeDTransAnalysis())
    return false;

  DTransAllocAnalyzer DTAA(TLI, M);
  DTAA.populateAllocDeallocTable(M);

  DTransInstVisitor Visitor(M.getContext(), *this, M.getDataLayout(), TLI, DTAA,
                            GetBFI);
  parseIgnoreList();

  Visitor.visit(M);

  // Computes TotalFrequency for each StructInfo and MaxTotalFrequency.
  uint64_t MaxTFrequency = 0;
  for (auto *TI : type_info_entries()) {
    if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI)) {
      computeStructFrequency(StInfo);
      // Trying to find MaxTotalFrequency here.
      MaxTFrequency = std::max(MaxTFrequency, StInfo->getTotalFrequency());
    }
  }
  setMaxTotalFrequency(MaxTFrequency);

  // Invalidate the fields for which the corresponding types do not pass
  // the SafetyData checks.
  for (auto *TI : type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (StInfo && testSafetyData(TI, dtrans::DT_FieldSingleValue))
      for (unsigned I = 0, E = StInfo->getNumFields(); I != E; ++I)
        StInfo->getField(I).setMultipleValue();
    if (StInfo && testSafetyData(TI, dtrans::DT_FieldSingleAllocFunction))
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

  DTransAnalysisRan = true;

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
    outs() << "\n MaxTotalFrequency: " << getMaxTotalFrequency() << "\n\n";
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
  printIgnoreTransListForStructure(SI);
  outs() << "  Number of fields: " << SI->getNumFields() << "\n";
  unsigned Number = 0;
  for (auto &Field : SI->getFields()) {
    outs() << format_decimal(Number++, 3) << ")";
    printFieldInfo(Field, SI->getIgnoredFor());
  }
  outs() << "  Total Frequency: " << SI->getTotalFrequency() << "\n";
  auto &CG = SI->getCallSubGraph();
  outs() << "  Call graph: "
         << (CG.isBottom() ? "bottom\n" : (CG.isTop() ? "top\n" : ""));
  if (!CG.isBottom() && !CG.isTop()) {
    outs() << "enclosing type: " << CG.getEnclosingType()->getName() << "\n";
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

void DTransAnalysisInfo::printFieldInfo(dtrans::FieldInfo &Field,
                                        dtrans::Transform IgnoredInTransform) {
  outs() << "Field LLVM Type: " << *(Field.getLLVMType()) << "\n";
  outs() << "    Field info:";

  if (Field.isRead())
    outs() << " Read";

  if (Field.isWritten())
    outs() << " Written";

  if (Field.isValueUnused())
    outs() << " UnusedValue";

  if (Field.hasComplexUse())
    outs() << " ComplexUse";

  if (Field.isAddressTaken())
    outs() << " AddressTaken";

  outs() << "\n";
  outs() << "    Frequency: " << Field.getFrequency();
  outs() << "\n";

  if (Field.isNoValue())
    outs() << "    No Value";
  else if (Field.isSingleValue()) {
    outs() << "    Single Value: ";
    Field.getSingleValue()->printAsOperand(outs());
  } else if (Field.isMultipleValue()) {
    outs() << "    Multiple Value: [ ";
    auto FirstI = Field.values().begin();
    for (auto I = FirstI, E = Field.values().end(); I != E; ++I) {
      if (I != FirstI) {
        outs() << ", ";
      }

      (*I)->printAsOperand(outs(), false);
    }
    outs() << " ] <" << (Field.isValueSetComplete() ? "complete" : "incomplete")
           << ">";
  }
  if (IgnoredInTransform & dtrans::DT_FieldSingleValue)
    outs() << " (ignored)";
  outs() << "\n";

  if (Field.isTopAllocFunction())
    outs() << "    Top Alloc Function";
  else if (Field.isSingleAllocFunction()) {
    outs() << "    Single Alloc Function: ";
    Field.getSingleAllocFunction()->printAsOperand(outs());
  } else if (Field.isBottomAllocFunction())
    outs() << "    Bottom Alloc Function";
  if (IgnoredInTransform & dtrans::DT_FieldSingleAllocFunction)
    outs() << " (ignored)";
  outs() << "\n";
}

// Interface routine to check if the field that is supposed to be loaded in the
// instruction is only read and its parent structure has no safety data
// violations.
//
bool DTransAnalysisInfo::isReadOnlyFieldAccess(LoadInst *Load) {
  std::pair<dtrans::StructInfo *, uint64_t> Res = getInfoFromLoad(Load);
  if (!Res.first)
    return false;

  if (testSafetyData(Res.first, dtrans::DT_ElimROFieldAccess))
    return false;

  dtrans::FieldInfo &FI = Res.first->getField(Res.second);
  return FI.isRead() && !FI.isWritten();
}

// A helper routine to retrieve structure type - field index pair from a
// GEPOperator.
std::pair<llvm::StructType *, uint64_t>
DTransAnalysisInfo::getStructField(GEPOperator *GEP) {
  if (!GEP || !GEP->hasAllConstantIndices())
    return std::make_pair(nullptr, 0);

  if (GEP->getNumIndices() == 1) {
    auto *GEPI = dyn_cast<GetElementPtrInst>(GEP);
    if (!GEPI)
      return std::make_pair(nullptr, 0);

    auto StructField = getByteFlattenedGEPElement(GEPI);
    auto StructTy = dyn_cast_or_null<StructType>(StructField.first);
    if (!StructTy)
      std::make_pair(nullptr, 0);

    return std::make_pair(StructTy, StructField.second);
  }

  auto StructTy = dyn_cast<StructType>(GEP->getSourceElementType());
  if (!StructTy)
    return std::make_pair(nullptr, 0);

  if (!cast<ConstantInt>(GEP->getOperand(1))->isZeroValue())
    return std::make_pair(nullptr, 0);

  uint64_t FieldIndex = 0;
  for (unsigned NI = 2; NI <= GEP->getNumIndices(); ++NI) {
    auto IndexConst = cast<ConstantInt>(GEP->getOperand(NI));
    FieldIndex = IndexConst->getLimitedValue();
    if (FieldIndex >= StructTy->getNumElements())
      return std::make_pair(nullptr, 0);
    if (NI == GEP->getNumIndices())
      break;
    auto *Ty = StructTy->getElementType(FieldIndex);
    auto *NewStructTy = dyn_cast<StructType>(Ty);
    if (!NewStructTy)
      return std::make_pair(nullptr, 0);
    StructTy = NewStructTy;
  }
  return std::make_pair(StructTy, FieldIndex);
}

// A helper routine to get a DTrans structure type and field index from the
// GEP instruction which is a pointer argument of the \p Load in the
// parameters.
//
std::pair<dtrans::StructInfo *, uint64_t>
DTransAnalysisInfo::getInfoFromLoad(LoadInst *Load) {
  if (!Load)
    return std::make_pair(nullptr, 0);

  auto GEP = dyn_cast<GEPOperator>(Load->getPointerOperand());
  auto StructField = getStructField(GEP);
  if (!StructField.first)
    return std::make_pair(nullptr, 0);

  dtrans::TypeInfo *TI = getTypeInfo(StructField.first);
  if (!TI)
    return std::make_pair(nullptr, 0);

  auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
  if (!StInfo)
    return std::make_pair(nullptr, 0);

  return std::make_pair(StInfo, StructField.second);
}

bool DTransAnalysisInfo::GetFuncPointerPossibleTargets(
    llvm::Value *FP, std::vector<llvm::Value *> &Targets, llvm::CallSite,
    bool) {
  // TODO: add a support for PHI instruction with function pointer type.

  Targets.clear();
  LLVM_DEBUG({
    dbgs() << "FSV ICS: Analyzing";
    FP->dump();
  });
  auto LI = dyn_cast<LoadInst>(FP);
  std::pair<dtrans::StructInfo *, uint64_t> Res = getInfoFromLoad(LI);
  if (!Res.first) {
    LLVM_DEBUG(dbgs() << "FSV ICS: INCOMPLETE\n"
                      << "Inst " << *FP << "\n"
                      << "Target List is NULL\n");
    return false;
  }
  dtrans::FieldInfo &FI = Res.first->getField(Res.second);
  bool IsIncomplete = !FI.isValueSetComplete();
  for (auto *C : FI.values()) {
    if (auto F = dyn_cast<Function>(C))
      Targets.push_back(F);
    else if (!C->isZeroValue())
      IsIncomplete = true;
  }
  LLVM_DEBUG({
    dbgs() << "FSV ICS: " << (!IsIncomplete ? "COMPLETE\n" : "INCOMPLETE\n")
           << "Load " << *LI << "\n";
    if (Targets.empty())
      dbgs() << "Target List is NULL\n";
    else {
      dbgs() << "Target List:\n ";
      for (auto *F : Targets)
        dbgs() << "  " << F->getName() << "\n";
    }
  });
  return !IsIncomplete;
}

void DTransAnalysisInfo::printIgnoreTransListForStructure(
    dtrans::StructInfo *SI) {
  std::string Output;
  StringRef Name = dtrans::getStructName(SI->getLLVMType());
  // Cut the "{dtrans_opt_prefix}struct." from the LLVM type name.
  std::pair<StringRef, StringRef> StructPrefixAndName = Name.split('.');
  if (StructPrefixAndName.second.empty())
    return;

  for (dtrans::Transform Tr = dtrans::DT_First; Tr < dtrans::DT_Last;
       Tr <<= 1) {
    if (IgnoreTypeMap[Tr].find(StructPrefixAndName.second) !=
        IgnoreTypeMap[Tr].end()) {
      Output += " ";
      Output += dtrans::getStringForTransform(Tr);
    }
  }
  if (!Output.empty()) {
    outs() << "  (will be ignored in" << Output << ")\n";
  }
}

void DTransAnalysisWrapper::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<BlockFrequencyInfoWrapperPass>();
  AU.addRequired<WholeProgramWrapperPass>();
}

char DTransAnalysis::PassID;

// Provide a definition for the static class member used to identify passes.
AnalysisKey DTransAnalysis::Key;

DTransAnalysisInfo DTransAnalysis::run(Module &M, AnalysisManager<Module> &AM) {
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetBFI = [&FAM](Function &F) -> BlockFrequencyInfo & {
    return FAM.getResult<BlockFrequencyAnalysis>(F);
  };
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  DTransAnalysisInfo DTResult;
  DTResult.analyzeModule(M, AM.getResult<TargetLibraryAnalysis>(M), WPInfo,
                         GetBFI);
  return DTResult;
}
