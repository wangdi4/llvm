//===---------------- DTransAnalysis.cpp - DTrans Analysis ----------------===//
//
// Copyright (C) 2017-2022 Intel Corporation. All rights reserved.
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
#include "Intel_DTrans/Analysis/DTransAllocAnalyzer.h"
#include "Intel_DTrans/Analysis/DTransAnnotator.h"
#include "Intel_DTrans/Analysis/DTransArraysWithConstant.h"
#include "Intel_DTrans/Analysis/DTransBadCastingAnalyzer.h"
#include "Intel_DTrans/Analysis/DTransImmutableAnalysis.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/ADT/EquivalenceClasses.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/Intel_DopeVectorAnalysis.h"
#include "llvm/Analysis/Intel_LangRules.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/AbstractCallSite.h"
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <map>
#include <queue>
#include <set>

using namespace llvm;
using namespace dvanalysis;
using namespace llvm::PatternMatch;

#define DEBUG_TYPE "dtransanalysis"

// Debug type for basic local pointer analysis output.
#define DTRANS_LPA "dtrans-lpa"

// Debug type for verbose local pointer analysis output.
#define DTRANS_LPA_VERBOSE "dtrans-lpa-verbose"

// Debug type for outputting results of local pointer analysis
// intermixed with an IR dump to annotate the types resolved as
// the pointer usage type by the local pointer analyzer.
#define DTRANS_LPA_RESULTS "dtrans-lpa-results"

// Debug type for verbose call graph computations.
#define DTRANS_CG "dtrans-cg"

// Debug type for verbose partial pointer load/store analysis output.
#define DTRANS_PARTIALPTR "dtrans-partialptr"

// Debug type for verbose field single value analysis output.
#define DTRANS_FSV "dtrans-fsv"

// Debug type for verbose field single alloc function analysis output.
#define DTRANS_FSAF "dtrans-fsaf"

// Debug type for verbose C-rule compatibility testing
#define DTRANS_CRC "dtrans-crc"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static cl::opt<bool> DTransPrintAllocations("dtrans-print-allocations",
                                            cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

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

// Enable merging padded structures with base structures even if the
// safety checks didn't pass. This option is for testing purposes and
// must remain turned off.
static cl::opt<bool> DTransTestPaddedStructs("dtrans-test-padded-structs",
                                              cl::init(false),
                                              cl::ReallyHidden);

// Enable the analysis process for arrays with constant entries
static cl::opt<bool> DTransArraysWithConstEntries(
    "dtrans-arrays-with-const-entries", cl::init(true), cl::ReallyHidden);

using GetTLIFnType = std::function<const TargetLibraryInfo &(const Function &)>;

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
// There are a couple of forms of the comparison instruction supported for this
// pattern. In the form shown above, the current loop iteration %Count value
// is being compared, but a form that compares the %NextCount variable as
// greater than 0 is also supported. In both cases, we can also support an
// unsigned greater than comparison type.
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
    if (!match(Condition, m_ICmp(Pred, m_Instruction(Base), m_APInt(Count)))) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. icmp not using constant int!\n");
      return false;
    }
    if (Pred != CmpInst::Predicate::ICMP_SGT &&
        Pred != CmpInst::Predicate::ICMP_UGT) {
      DEBUG_WITH_TYPE(DTRANS_PARTIALPTR,
                      dbgs() << "Not matched. icmp predicate isn't sgt/ugt!\n");
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
      unsigned Bitwidth = LoopInVal->getType()->getScalarSizeInBits();
      if (!(match(LoopInVal, m_Add(m_Specific(BasePHI),
                                   m_SpecificInt(APInt(Bitwidth, -1)))) ||
            match(LoopInVal, m_Add(m_SpecificInt(APInt(Bitwidth, -1)),
                                   m_Specific(BasePHI))))) {
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
  // Used to describe the field and offset into an array or structure.
  struct PointeeLoc {
    enum PointeeLocKind { PLK_Field, PLK_Offset };
    PointeeLocKind Kind;
    union {
      size_t ElementNum;
      size_t ByteOffset;
    } Loc;

    PointeeLoc(PointeeLocKind Kind, size_t Val) : Kind(Kind) {
      if (Kind == PLK_Field)
        Loc.ElementNum = Val;
      else
        Loc.ByteOffset = Val;
    }
    PointeeLocKind getKind() const { return Kind; }
    size_t getElementNum() const { return Loc.ElementNum; }
    size_t getByteOffset() const { return Loc.ByteOffset; }
  };

  typedef std::pair<llvm::Type *, PointeeLoc> TypeAndPointeeLocPair;
  typedef std::set<std::pair<llvm::Type *, PointeeLoc>> ElementPointeeSet;
  typedef std::set<std::pair<llvm::Type *, PointeeLoc>> &ElementPointeeSetRef;
  typedef SmallPtrSet<llvm::Type *, 3> PointerTypeAliasSet;
  typedef SmallPtrSetImpl<llvm::Type *> &PointerTypeAliasSetRef;

  LocalPointerInfo()
      : AnalysisState(LPIS_NotAnalyzed), AliasesToAggregatePointer(false),
        IsPartialPtrLoadStore(false) {}

  LocalPointerInfo(const LocalPointerInfo &) = delete;
  LocalPointerInfo &operator=(const LocalPointerInfo &) = delete;

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
  // that information. Track the element being accessed by index.
  void addElementPointee(llvm::Type *Base, size_t ElemIdx) {
    PointeeLoc Loc(PointeeLoc::PLK_Field, ElemIdx);
    ElementPointees.insert(std::make_pair(Base, Loc));
  }

  // If the pointer is pointing into an aggregate, but does not correspond to
  // the starting location of an element, we want to track the byte offset. This
  // is used for the case of a memintrinsic that starts on a byte that is
  // padding between elements.
  void addElementPointeeByOffset(llvm::Type *Base, size_t ByteOffset) {
    PointeeLoc Loc(PointeeLoc::PLK_Offset, ByteOffset);
    ElementPointees.insert(std::make_pair(Base, Loc));
  }

  void addElementZeroAlias(llvm::Type *Ty) {
    ElementZeroAliases.insert(Ty);
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
    bool HaveMultipleAliases = false;
    bool DomTyIsElementZeroAccess = false;
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
      HaveMultipleAliases = true;
      // If this type can be an element zero access of DomTy,
      // DomTy is still dominant.
      if (dtrans::isElementZeroAccess(DomTy, AliasTy)) {
        DomTyIsElementZeroAccess = true;
        continue;
      }
      // If what we previously thought was the dominant type can be
      // an element zero access of the current alias, the current
      // alias becomes dominant.
      if (dtrans::isElementZeroAccess(AliasTy, DomTy)) {
        DomTyIsElementZeroAccess = true;
        DomTy = AliasTy;
        continue;
      }

      // Check whether the pending dominant type, DomTy, is a pointer-to-pointer
      // type that may be equivalent to the alias type, AliasTy, based on the
      // element zero types.
      //
      // For example, given the types:
      //   %struct.outer = type { %struct.middle* }
      //   %struct.middle = type { %struct.inner }
      //   %struct.inner = type { i64 }
      //
      // With an the input alias set of:
      //   %struct.outer*, %struct.middle**, and %struct.inner**
      //
      // The types %struct.middle** and %struct.inner** do not dominate each
      // other, but can be used interchangeably.
      //
      // The above checks will choose %struct.outer* as being the dominant type
      // of %struct.middle** and of %struct.inner**, if %struct.outer* is
      // considered as the pending dominant type prior to testing against the
      // other two types because the rule within the isElementZeroAccess that
      // allows for a pointer-to-pointer type to succeed when the element zero
      // type is a pointer type. However, we need to account for these aliases
      // being evaluated in an arbitrary order. If the order of evaluation was
      // chosen to test %struct.middle** and %struct.inner** as the types to
      // evaluate as potential element zero access types, then neither will
      // succeed because they are both pointer-to-pointer types. Here, we try to
      // see whether removing one level of type dereferencing indicates that one
      // of them is an element zero type of the other to determine whether the
      // search for an element zero dominant type should continue. We only need
      // to consider one level of dereferencing because isElementZeroAccess only
      // permits the zeroth element to be an aggregate of one level pointer
      // type.
      if (!(DomTy->isPointerTy() &&
            DomTy->getPointerElementType()->isPointerTy() &&
            AliasTy->isPointerTy() &&
            AliasTy->getPointerElementType()->isPointerTy()))
        return nullptr;

      llvm::Type *DomDerefTy = DomTy->getPointerElementType();
      llvm::Type *AliasDerefTy = AliasTy->getPointerElementType();
      if (dtrans::isElementZeroAccess(DomDerefTy, AliasDerefTy) ||
          dtrans::isElementZeroAccess(AliasDerefTy, DomDerefTy))
        continue;

      // Otherwise, there are conflicting aliases and nothing can be dominant.
      return nullptr;
    }
    // If there was only one potential dominant type, return it.
    // Otherwise, the dominant type must be an element zero accessible type from
    // all the other types.
    if (!HaveMultipleAliases)
      return DomTy;
    if (!DomTyIsElementZeroAccess)
      return nullptr;

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
  PointerTypeAliasSetRef getElementZeroAliasSet() { return ElementZeroAliases; }

  void merge(const LocalPointerInfo &Other) {
    // This routine is called during analysis, so don't change AnalysisState.
    AliasesToAggregatePointer |= Other.AliasesToAggregatePointer;
    for (auto *Ty : Other.PointerTypeAliases)
      PointerTypeAliases.insert(Ty);
    for (auto &Pair : Other.ElementPointees)
      ElementPointees.insert(Pair);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD
  void dump() { print(dbgs()); }

  // Print the pointer aliases and element pointee accesses.
  // The optional \p Indent value cause each line emitted to
  // be indented by some amount.
  void print(raw_ostream &OS, unsigned Indent = 0, const char *Prefix = "") {
    // Create an output string for the type to allow
    // output of a set of strings in lexical order.
    auto TypeToString = [Indent, Prefix](Type *Ty) {
      std::string OutputVal;
      raw_string_ostream StringStream(OutputVal);

      StringStream << Prefix;
      StringStream.indent(Indent + 4);
      StringStream << *Ty;
      StringStream.flush();
      return OutputVal;
    };

    // Create an output string for the element pointee pair to allow
    // output of a set of strings in lexical order.
    auto PointeePairToString =
        [Indent,
         Prefix](const LocalPointerInfo::TypeAndPointeeLocPair &PointeePair) {
          std::string OutputVal;
          raw_string_ostream StringStream(OutputVal);

          StringStream << Prefix;
          StringStream.indent(Indent + 4);
          if (auto *StTy = dyn_cast<llvm::StructType>(PointeePair.first))
            if (StTy->hasName())
              StringStream << "%" << StTy->getName();
            else
              StringStream << *StTy;
          else
            StringStream << *PointeePair.first;
          StringStream << " @ ";
          if (PointeePair.second.getKind() == PointeeLoc::PLK_Offset)
            StringStream << "not-field ByteOffset: "
                         << PointeePair.second.getByteOffset();
          else
            StringStream << std::to_string(PointeePair.second.getElementNum());

          StringStream.flush();
          return OutputVal;
        };

    OS << Prefix;
    OS.indent(Indent);
    OS << "LocalPointerInfo:\n";
    if (PointerTypeAliases.empty()) {
      OS << Prefix;
      OS.indent(Indent);
      OS << "  No aliased types.\n";
    } else {
      OS << Prefix;
      OS.indent(Indent);
      OS << "  Aliased types:\n";
      dtrans::printCollectionSorted(OS, PointerTypeAliases.begin(),
                                    PointerTypeAliases.end(), "\n",
                                    TypeToString);
      OS << "\n";
    }

    if (ElementPointees.empty()) {
      OS << Prefix;
      OS.indent(Indent);
      OS << "  No element pointees.\n";
    } else {
      OS << Prefix;
      OS.indent(Indent);
      OS << "  Element pointees:\n";
      dtrans::printCollectionSorted(OS, ElementPointees.begin(),
                                    ElementPointees.end(), "\n",
                                    PointeePairToString);
      OS << "\n";
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

  // When the first element of a structure is an i8 (or array of i8 types), or
  // ptr-to-ptr type, a bitcast of a pointer to the structure to an i8* captures
  // the result in the ElementPointee set as being element 0 of the innermost
  // nested type of the aggregate. However, the result of the bitcast could also
  // be used for a byte flattened GEP. This list is to maintain the set of other
  // possible aggregate types the address is equivalent to for being able to
  // determine the field if the bitcast result ends up being used for a
  // byte-flattened GEP.
  //
  // For example:
  //   %class.C = type { i32 (...)**, i32 }
  //   %class.B = type{ %class.C }
  //   %bc = bitcast %class.B* to i8*
  //
  // %bc could be being used for the vtable address in %class.C, or for a
  // byte-flatted GEP.
  // ElementPointees will record the result of %bc as %class.C @ 0.
  // ElementZeroAliases will record the result of %bc as coming from %class.B.
  PointerTypeAliasSet ElementZeroAliases;

  bool IsPartialPtrLoadStore;
};

// Comparator for PointeeLoc to enable using type within std::set.
bool operator<(const LocalPointerInfo::PointeeLoc &A,
               const LocalPointerInfo::PointeeLoc &B) {
  if (A.Kind == B.Kind)
    return A.Kind == LocalPointerInfo::PointeeLoc::PLK_Field
               ? A.getElementNum() < B.getElementNum()
               : A.getByteOffset() < B.getByteOffset();

  // Treat field elements as sorting before non-field elements
  if (A.Kind == LocalPointerInfo::PointeeLoc::PLK_Field)
    return true;

  return false;
}

// End of member functions for class DTransAllocAnalyzer

class LocalPointerAnalyzer {
public:
  LocalPointerAnalyzer(const DataLayout &DL, GetTLIFnType GetTLI,
                       DTransAnalysisInfo &DTAI,
                       dtrans::DTransAllocAnalyzer &DTAA)
      : DL(DL), GetTLI(GetTLI), DTAI(DTAI), DTAA(DTAA) {}

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

    // A subtraction instruction with pointer-sized integers may also need to be
    // treated as a possible pointer value to handle the case where one operand
    // is from a PtrToInt, and the other is a constant integer.
    if (auto *BinOp = dyn_cast<BinaryOperator>(V))
      if (BinOp->getOpcode() == Instruction::Sub)
        return true;

    // Otherwise, we don't need to analyze it as a pointer.
    return false;
  }

private:
  const DataLayout &DL;
  GetTLIFnType GetTLI;
  DTransAnalysisInfo &DTAI;
  dtrans::DTransAllocAnalyzer &DTAA;
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
      if (auto TyFromMD =
              dtrans::DTransAnnotator::lookupDTransTypeAnnotation(*I)) {
        llvm::Type *Ty = TyFromMD.value().first;
        unsigned Level = TyFromMD.value().second;
        while (Level--)
          Ty = Ty->getPointerTo();
        Info.addPointerTypeAlias(Ty);
      }

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
        DEBUG_WITH_TYPE(DTRANS_LPA_VERBOSE, {
          dbgs() << "  Already analyzed: ";
          if (isa<Function>(Dep))
            Dep->printAsOperand(dbgs());
          else
            dbgs() << *Dep;
          dbgs() << "\n";
        });
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

    if (CallBase *Call = getCallIfAlloc(V)) {
      // If the value we're analyzing is a call to an allocation function
      // we need to look for bitcast users so that we can proactively assign
      // the type to which the value will be cast as an alias.
      analyzeAllocationCallAliases(Call, Info);
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
    } else if (auto *BC = dyn_cast<BitCastInst>(V)){
      // If the value is a BitCast then we need to check if it used to load
      // a zero element
      analyzeBitCastInstruction(BC, Info);
    } else if (isa<ExtractValueInst>(V)) {
      // FIXME: Also analyze extract value instructions.
    }
    else if (isa<ExtractElementInst>(V)) {
      // FIXME: Also analyze extract element instructions when an instruction
      // visitor is created for them.
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

  CallBase *getCallIfAlloc(Value *V) {
    if (auto *Call = dyn_cast<CallBase>(V)) {
      const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
      if (dtrans::getAllocFnKind(Call, TLI) != dtrans::AK_NotAlloc ||
          DTAA.isMallocPostDom(Call))
        return Call;
    }
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

    if (auto *BinOp = dyn_cast<BinaryOperator>(V)) {
      if (BinOp->getOpcode() == Instruction::Sub) {
        Value *LeftOp = BinOp->getOperand(0);
        Value *RightOp = BinOp->getOperand(1);
        if (addDependency(LeftOp, DependentVals))
          populateDependencyStack(LeftOp, DependentVals);
        if (addDependency(RightOp, DependentVals))
          populateDependencyStack(RightOp, DependentVals);
        return;
      }
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
    if (CallBase *Call = getCallIfAlloc(V)) {
      SmallPtrSet<User *, 8> VisitedUsers;
      addAllocUsesToDependencyStack(Call, DependentVals, VisitedUsers);
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
    for (auto *V : DependentVals) {
      dbgs() << "    ";
      if (isa<Function>(V))
        V->printAsOperand(dbgs());
      else
        dbgs() << *V;
      dbgs() << "\n";
    }
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

            // Capture the original type to allow for evaluating the pointer as
            // a potential byte-flattened GEP.
            // For example:
            //   struct.test = type { [256 x i8], i64 }
            //   %y = bitcast %struct.test* %x to i8*
            // This will have captured [258 x i8] @ 0 as the element pointee,
            // and i8* as the aliased type. However, %y could be get used as a
            // byte-flattened GEP, capture the aggregate type for use during
            // byte-flattened GEP analysis.
            Info.addElementZeroAlias(AliasTy->getPointerElementType());

            IsElementZeroAccess = true;
            // If the bitcast is to an i8** and element zero of the accessed
            // type is a pointer, we need to add the type of that pointer
            // to the destination value's alias set.
            auto *Int8PtrPtrTy =
                llvm::Type::getInt8PtrTy(DestTy->getContext())->getPointerTo();
            if (DestTy == Int8PtrPtrTy) {
              Info.addPointerTypeAlias(
                  dtrans::dtransCompositeGetTypeAtIndex(
                      AccessedTy->getPointerElementType(), 0)->getPointerTo());
            }
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
              if (PointeePair.second.getKind() ==
                  LocalPointerInfo::PointeeLoc::PLK_Field)
                Info.addElementPointee(PointeePair.first,
                                       PointeePair.second.getElementNum());
              else
                Info.addElementPointeeByOffset(
                    PointeePair.first, PointeePair.second.getByteOffset());
          }
        }
        // If this is an element zero access, don't merge the source info.
        if (IsElementZeroAccess)
          return;
      }
      Info.merge(SrcLPI);
      return;
    }
    if (auto *BinOp = dyn_cast<BinaryOperator>(V)) {
      // Treat the result of constant integer being subtracted from a PtrToInt
      // value as still carrying the pointer type. This is necessary to perform
      // checks if the subtract is subsequently used to subtract another
      // PtrToInt value.
      //   %prev = sub i64 %t2, 8
      //   %offset = sub i64 %t1, %prev
      //
      // Subtracting a PtrToInt value from a constant or another PtrToInt value
      // will not be tracked as carrying pointer type information.
      if (BinOp->getOpcode() != Instruction::Sub)
        return;
      if (!isa<ConstantInt>(BinOp->getOperand(1)))
        return;

      // Copy any type information from the source operand. Unsupported cases,
      // such as when the type is an element pointee and has a constant
      // subtracted from it will be detected when using the LPI during
      // visitBinaryOperator.
      mergeOperandInfo(BinOp->getOperand(0), Info);
      return;
    }

    // The caller should have checked isDerivedValue() before calling this
    // function, and the above cases should cover all possible derived
    // values.
    llvm_unreachable("Unexpected class for derived value!");
  }

  // Return the structure and the index that is being accessed through
  // a long GEP, and this GEP will be used to access information by
  // another byte flattened GEP. For example:
  //
  //   %tmp = getelementptr inbounds %class.TestClass, %class.TestClass* %0,
  //          i64 0, i32 2, i32 0, i32 0, i32 0, i32 0, i32 0
  //   %tmp1 = getelementptr inbounds i8, i8* %tmp, i64 16
  //
  // In this case, the GEP in %tmp is accessing field two from the structure
  // %class.TestClass, and the rest of the entries are zero element access.
  // This means the GEP %tmp1 is accessing 16 bytes after the field mentioned
  // before. Therefore, %class.TestClass@2 must be part of the local pointer
  // information for the GEP %tmp. This kind of representation can be seen
  // after many GEPs that are accessing element 0 are squeezed by another
  // pass (e.g. AggressiveInstCombine).
  std::pair<llvm::Type*, unsigned>
  collectTypeForByteFlattenedGEP(GetElementPtrInst *GEP) {
    if (!GEP)
      return std::make_pair(nullptr, 0);

    // It should be a GEP that is accessing nested structures
    unsigned NumIndices = GEP->getNumIndices();
    if (NumIndices <= 2 || !GEP->hasAllConstantIndices())
      return std::make_pair(nullptr, 0);

    // Only used for byte flattened GEPs
    for (User *U : GEP->users()) {
      auto *GEPUser = dyn_cast<GetElementPtrInst>(U);
      if (!GEPUser || GEPUser->getNumIndices() != 1)
        return std::make_pair(nullptr, 0);
    }

    llvm::Type *RetType = nullptr;
    unsigned RetIndex = 0;
    if (!(cast<ConstantInt>(GEP->getOperand(1))->isZero()))
      return std::make_pair(nullptr, 0);

    llvm::Type *CurrType = GEP->getSourceElementType();
    if (!CurrType->isStructTy() && !CurrType->isArrayTy())
      return std::make_pair(nullptr, 0);

    // All entries in the GEP are accessing nested structures or arrays, except
    // the last entry
    for (unsigned I = 2; I < NumIndices; I++) {
      ConstantInt *Idx = cast<ConstantInt>(GEP->getOperand(I));
      // The last non-zero access represents which index is being accessed
      // for a particular structure or array
      if (!Idx->isZero()) {
        RetType = CurrType;
        RetIndex = Idx->getZExtValue();
      }

      if (auto *Struct = dyn_cast<llvm::StructType>(CurrType))
        CurrType = Struct->getElementType(Idx->getZExtValue());
      else if (auto *Array = dyn_cast<llvm::ArrayType>(CurrType))
        CurrType = Array->getElementType();
      else
        return std::make_pair(nullptr, 0);
    }

    // Last entry in the GEP will be checked outside of the loop since it
    // doesn't need to be a structure or an array, but it must be 0
    ConstantInt *LastIdx = cast<ConstantInt>(GEP->getOperand(NumIndices));
    if (!LastIdx->isZero())
      return std::make_pair(nullptr, 0);

    return std::make_pair(RetType, RetIndex);
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

    // Check for the special case of the empty set index set, which just returns
    // the GEP pointer operand.
    if (GEP->getNumIndices() == 0) {
      Info.addPointerTypeAlias(BasePointer->getType());
      return;
    }

    std::pair<llvm::Type*, unsigned> TypeForByteFlattenedGEP =
        collectTypeForByteFlattenedGEP(dyn_cast<GetElementPtrInst>(GEP));
    if (TypeForByteFlattenedGEP.first &&
        TypeForByteFlattenedGEP.first->isStructTy() &&
        TypeForByteFlattenedGEP.second != 0)
      Info.addElementPointee(TypeForByteFlattenedGEP.first,
                             TypeForByteFlattenedGEP.second);

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
                        dbgs() << "Zero-sized array access detected " << *GEP
                               << "\n");
        return;
      }

      // If the last argument is not constant and out-of-bound flag is false
      // then it is safe to have non-constant array access. Use 0 index for
      // Pointee set.
      if (!DTAI.getDTransOutOfBoundsOK())
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

    // Return 'true' and update 'LocalInfo' with the elements addressed by a
    // byte flattened GEP, if all the Offsets match element boundaries.
    // Otherwise, return false.
    auto CheckIfAllOffsetsValid = [this](GEPOperator *GEP, llvm::Type *CurrType,
                                         const SmallVectorImpl<APInt> &APOffset,
                                         LocalPointerInfo &LocalInfo) {
      for (auto &APOffsetVal : APOffset)
        if (!analyzePossibleOffsetAggregateAccess(
                GEP, CurrType, APOffsetVal.getLimitedValue(), LocalInfo))
          return false;

      return true;
    };

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
        auto *CurrType = AliasTy->getPointerElementType();
        LocalPointerInfo LocalInfo;
        if (CheckIfAllOffsetsValid(GEP, CurrType, APOffset, LocalInfo)) {
          Info.merge(LocalInfo);
          return true;
        }
      }
    }

    // If a match was not found in the pointer alias set, it's still possible
    // that a pointer to an aggregate was bitcast to a i8*, which is treated as
    // an element zero access. Check if the offset is a valid offset from a type
    // the value was bitcast from to an i8*.
    for (auto *CurrType : BaseLPI.getElementZeroAliasSet()) {
      LocalPointerInfo LocalInfo;
      if (CheckIfAllOffsetsValid(GEP, CurrType, APOffset, LocalInfo)) {
        Info.merge(LocalInfo);
        return true;
      }
    }

    if (HasPtrToPtrAlias)
      return true;

    // It is possible that the byte flattened GEP is the result of squeezing
    // multiple GEPs together into one GEP with multiple indices, and then
    // the byte flattened GEP is used to access the data (two GEPs in total).
    // In that case, we are going to check if we can collect any information
    // from the pointees.
    if (isa<GetElementPtrInst>(GEP->getOperand(0))) {
      LocalPointerInfo &GEPArgInfo = LocalMap[GEP->getOperand(0)];
      if (GEPArgInfo.getAnalyzed()) {
        auto PointeesSet = GEPArgInfo.getElementPointeeSet();
        for (auto &PointeePair : PointeesSet) {
          llvm::Type *CurrTy = PointeePair.first;
          if (auto *StructTy = dyn_cast<StructType>(CurrTy)) {
            assert(PointeePair.second.getKind() !=
                   LocalPointerInfo::PointeeLoc::PLK_Offset &&
                   "Invalid use of pointee information");
            llvm::Type *ElemTy =
                StructTy->getElementType(PointeePair.second.getElementNum());
            LocalPointerInfo LocalInfo;
            if (CheckIfAllOffsetsValid(GEP, ElemTy, APOffset, LocalInfo)) {
              Info.merge(LocalInfo);
              return true;
            }
          }
        }
      }
    }

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

    // Check if the load represents the zero element of a structure.
    Type *Pointee = nullptr;
    Type *LoadedType = dtrans::getTypeForZeroElementLoaded(Load, &Pointee);
    // Store the pointer alias
    if (LoadedType && Pointee)
      Info.addPointerTypeAlias(LoadedType);

    for (auto *AliasTy : SrcLPI.getPointerTypeAliasSet())
      if (AliasTy->isPointerTy() &&
          AliasTy->getPointerElementType()->isPointerTy())
        Info.addPointerTypeAlias(AliasTy->getPointerElementType());
  }

  void analyzeBitCastInstruction(BitCastInst *BC, LocalPointerInfo &Info) {
    // If the BitCast is used to load the 0 element from a structure
    // then we need find the structure that is being accessed
    if (!BC)
      return;

    for (User *U : BC->users()) {
      if (auto *Load = dyn_cast<LoadInst>(U)) {
        // Check if the load represents the zero element of a structure.
        Type *Pointee = nullptr;
        Type *LoadedType = dtrans::getTypeForZeroElementLoaded(Load, &Pointee);
        if (LoadedType && Pointee)
          // Store the structure that is being accessed at 0
          Info.addElementPointee(Pointee, 0);
      }
    }
  }

  bool isDerivedValue(Value *V) {
    // TODO: Consider whether it will be necessary to handle llvm::MemoryAccess.

    // These value types transform other values into a new temporary.
    // GetElementPtr isn't in this list because the pointer it returns is
    // referring to a different logical object (a field) than the input
    // value, even though it points to the same block of memory.
    // The GetElementPtr case will be handled elsewhere.
    if (isa<CastInst>(V) || isa<PHINode>(V) || isa<SelectInst>(V) ||
        isa<BitCastOperator>(V) || isa<PtrToIntOperator>(V) ||
        isa<BinaryOperator>(V))
      return true;

    // This assert is here to catch cases that I haven't thought about.
    assert(isa<GlobalVariable>(V) || isa<Argument>(V) || isa<AllocaInst>(V) ||
           isa<LoadInst>(V) || isa<CallInst>(V) || isa<GetElementPtrInst>(V) ||
           isa<Constant>(V) || isa<GEPOperator>(V) || isa<InvokeInst>(V) ||
           isa<ExtractValueInst>(V) || isa<ExtractElementInst>(V) ||
           isa<FreezeInst>(V));

    // Note that ExtractValueInst, ExtractElementInst, FreezeInst, and
    // InvokeInst are not handled by the main instruction visitor, so they will
    // cause UnhandledUse safety conditions to be set. They are added to the
    // assert here to prevent it from firing while compiling programs that we do
    // not expect to be able to optimize. Additional implementation would be
    // necessary to handle these correctly.

    return false;
  }

  // Find any type to which the return value of an allocation call will be
  // bitcast and, unless it looks like an element zero access, add that type
  // as an alias of the allocated pointer.
  void analyzeAllocationCallAliases(CallBase *Call, LocalPointerInfo &Info) {
    DEBUG_WITH_TYPE(DTRANS_LPA, dbgs()
                                    << "dtrans: Analyzing allocation call.\n  "
                                    << *Call << "\n");
    SmallPtrSet<llvm::PointerType *, 4> CastTypes;
    SmallPtrSet<Value *, 4> VisitedUsers;
    bool IsPartial = false;
    collectAllocatedPtrBitcasts(Call, CastTypes, VisitedUsers, IsPartial);
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
          if (dtrans::isElementZeroAccess(Ty1, Ty2) ||
              dtrans::isVTableAccess(Ty1, Ty2))
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

      // In order to handle the case where the allocation is stored into a
      // structure member field which is a pointer, look at the member type to
      // infer the allocation type.
      //
      // For example:
      //    %struct.P1 = type{ i32*, i32, i32, i32 }
      //    %struct.P2 = type{ i32, %struct.P1* }
      //    %ptr_i8 = bitcast %struct.P1** %ptr to i8**
      //    %ptr = getelementptr % struct.P2, %struct.P2* %s2, i64 0, i32 1
      //    %mem = call noalias i8* @malloc(i64 24)
      //    store i8* %mem, i8** %ptr_i8
      //
      // In this case, %ptr_i8 will be resolved to being field member 1 of
      // %struct.P2, which means the allocation will be inferred as a pointer to
      // %struct.P1.
      //
      if (DestInfo.pointsToSomeElement()) {
        auto ElementPointees = DestInfo.getElementPointeeSet();
        for (auto &PointeePair : ElementPointees) {
          llvm::Type *Ty = PointeePair.first;
          if (auto *StructTy = dyn_cast<StructType>(Ty)) {
            assert(PointeePair.second.getKind() !=
                       LocalPointerInfo::PointeeLoc::PLK_Offset &&
                   "Unexpected use of invalid element");
            llvm::Type *ElemTy =
                StructTy->getElementType(PointeePair.second.getElementNum());
            if (auto PtrTy = dyn_cast<PointerType>(ElemTy))
              Types.insert(PtrTy);
          }
        }
      }

      return;
    }

    // Otherwise, the value of interest must be the destination pointer and
    // we must infer its type from what is being stored there.
    assert(ValOfInterest == DestPtr &&
           "Can't infer type from unused value of interest.");
    // For each type aliased by the stored value, add a pointer to that type
    // to the Types set for the destination.
    LocalPointerInfo &StoredLPI = LocalMap[StoredVal];
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
      if (auto *Call = dyn_cast<CallBase>(U)) {
        if (isValueInt8PtrType(V)) {
          DEBUG_WITH_TYPE(
              DTRANS_LPA_VERBOSE,
              dbgs() << "Analyzing use in call instruction: " << *Call << "\n");
          Function *F = dtrans::getCalledFunction(*Call);
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
            if (Call->getArgOperand(ArgNo) == V) {
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
                    const DataLayout &DL, GetTLIFnType GetTLI,
                    dtrans::DTransAllocAnalyzer &DTAA,
                    dtrans::DTransBadCastingAnalyzer &DTBCA,
                    function_ref<BlockFrequencyInfo &(Function &)> &GetBFI,
                    std::function<DominatorTree &(Function &)> GetDomTree)
      : DTInfo(Info), DL(DL), GetTLI(GetTLI), LPA(DL, GetTLI, DTInfo, DTAA),
        DTAA(DTAA), DTBCA(DTBCA), GetBFI(GetBFI), BFI(nullptr),
        GetDomTree(GetDomTree) {
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
    case Intrinsic::assume:
    case Intrinsic::type_test:
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
    for (Value *Arg : I.args()) {
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
    if ((T1->isFunctionTy() && isEmptyStructType(T2)) ||
        (T2->isFunctionTy() && isEmptyStructType(T1)))
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
      // not strictly preserved in LLVM. We could try to derive them from
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
      if ((T3->isFunctionTy() && isEmptyStructType(T4)) ||
          (T4->isFunctionTy() && isEmptyStructType(T3)))
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
    bool RV = typesMayBeCRuleCompatibleX(T1, T2, Tstack, IgnorePointees);
    DEBUG_WITH_TYPE(DTRANS_CRC, {
          dbgs() << "dtrans-crc: " << (RV ? "YES " : "NO  ");
          T1->print(dbgs(), true);
          dbgs() << " ";
          T2->print(dbgs(), true);
          dbgs()  << "\n";
    });
    return RV;
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

  // Return true if \p Call represents an indirect call, and there is a
  // an address taken external function that matches it.
  bool hasICMatch(CallBase *Call) {
    //
    // If 'F' is only used in a single BitCastOperator, return the
    // FunctionType to which it is bit cast. This can arise when a
    // function like getenv() is declared with an incomplete prototype:
    //    char *getenv();
    // but is then used in a single specific context as:
    //    char *getenv(char *);
    // In this case, due to the incomplete prototype, it will incorrectly
    // appear as varargs.
    //
    auto FindUniqueFunctionType = [](Function &F) -> FunctionType * {
      if (!F.hasOneUse())
        return nullptr;
      auto BCO = dyn_cast<BitCastOperator>(F.user_back());
      if (!BCO)
        return nullptr;
      Type *BCOTy = BCO->getDestTy();
      if (!BCOTy->isPointerTy())
        return nullptr;
      Type *BCOTyE = BCOTy->getPointerElementType();
      return dyn_cast<FunctionType>(BCOTyE);
    };

    // No point in doing this if the C language compatibility rule is not
    // enforced.
    if (!DTInfo.getDTransUseCRuleCompat())
      return true;
    // Check if this is an indirect call site.
    if (isa<Function>(Call->getCalledOperand()))
      return false;
    // Look for a matching address taken external call.
    for (auto &F : Call->getModule()->functions()) {
      if (F.hasAddressTaken() && F.isDeclaration()) {
        if (F.isVarArg()) {
          // Screen out cases of false varargs if the a unique
          // FunctionType can be determined for F.
          FunctionType *FT = nullptr;
          auto it = UniqueFunctionTypeMap.find(&F);
          if (it != UniqueFunctionTypeMap.end()) {
            FT = it->second;
          } else {
            FunctionType *FTTest = FindUniqueFunctionType(F);
            if (FTTest) {
              FT = FTTest;
              UniqueFunctionTypeMap[&F] = FTTest;
            }
          }
          if (FT) {
            // Found a unique FunctionType for F.  Use it in the
            // test for an indirect call match.
            unsigned PC = FT->getNumParams();
            if ((PC == Call->arg_size()) ||
                (FT->isVarArg() && (PC <= Call->arg_size()))) {
              unsigned I = 0;
              bool IsFunctionMatch = true;
              for (; I < PC; ++I) {
                Type *FTT = FT->getParamType(I);
                Type *CTT = Call->getArgOperand(I)->getType();
                if (!typesMayBeCRuleCompatible(FTT, CTT)) {
                  IsFunctionMatch = false;
                  break;
                }
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
            continue;
          }
        }
        // The standard test for an indirect call match.
        if ((F.arg_size() == Call->arg_size()) ||
            (F.isVarArg() && (F.arg_size() <= Call->arg_size()))) {
          unsigned I = 0;
          bool IsFunctionMatch = true;
          for (auto &Arg : F.args()) {
            Value *VCI = Call->getArgOperand(I);
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
      }
    }
    return false;
  }

  // Set MismatchedArgUse safety check if needed for direct call represented by
  // F.
  // FIXME: unify with visitCallArgument.
  void checkArgTypeMismatch(CallBase *Call, Function *F, Argument *FormalVal,
                            Value *ActualVal) {

    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(ActualVal);
    // Collect the dominant Type of the actual parameter
    Type *ActualType = LPI.getDominantAggregateTy();
    Type *FormalType = nullptr;

    if (FormalVal) {
      if (FormalVal->user_empty())
        return;
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
               << *Call;

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

    // Capture the types for further analysis that will determine whether the
    // mismatched argument use safety flag needs to be cascaded through pointers
    // contained within the types. When both FormalType and ActualType are types
    // of interest to DTrans, this determination cannot be made until usage
    // information has been collected for the complete module. If one of the
    // types is not a type of interest, the safety data will be cascaded through
    // all pointers.
    addDeferredPointerCarriedSafetyData(FormalType, ActualType,
                                        dtrans::MismatchedArgUse);
  }

  // FIXME: unify with checkArgTypeMismatch.
  void visitCallArgument(CallBase *Call, Function *F, bool HasICMatch,
                         Value *Arg, unsigned ArgNo) {

    // Return true if the formal argument will be dead in Function F
    auto IsDeadArgument = [F, ArgNo](bool IsFnLocal) {
      if (!F || !IsFnLocal)
        return false;

      if (F->isVarArg() || F->arg_size() <= ArgNo)
        return false;

      Value *FormalArg = F->getArg(ArgNo);
      return FormalArg->user_empty();
    };

    if (F && F->hasName()) {
      if (F->hasDLLExportStorageClass()) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: System object: "
                          << "argument used in a dllexport function:\n  "
                          << *Call << "\n");

       Type *ArgType = Arg->getType();
       setBaseTypeInfoSafetyDataWithCascading(ArgType,
                                              dtrans::SystemObject);
      } else {
        LibFunc TheLibFunc;
        const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
        if (TLI.getLibFunc(F->getName(), TheLibFunc) && TLI.has(TheLibFunc)) {

          LLVM_DEBUG(dbgs() << "dtrans-safety: System object: "
                            << "argument used in a library function:\n  "
                            << *Call << "\n");

         Type *ArgType = Arg->getType();
         setBaseTypeInfoSafetyDataWithCascading(ArgType,
                                                dtrans::SystemObject);
        }
      }
    }

    bool IsFnLocal = F ? !F->isDeclaration() : false;

    // Check if Arg is a pointer to a field, if so then mark the structure
    // as field address taken call.
    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(Arg);
    if (LPI.pointsToSomeElement()) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Field address taken call -- "
                        << "pointer to element passed as argument:\n"
                        << "  " << *Call << "\n  " << *Arg << "\n");
      // Selects and PHIs may have created a pointer that refers to
      // elements in multiple aggregate types. This sets the field
      // address taken condition for them all.
      for (auto PointeePair : LPI.getElementPointeeSet()) {
        auto Res = getParentStructType(PointeePair, Arg);
        dtrans::TypeInfo *ParentTI = DTInfo.getOrCreateTypeInfo(Res.first);
        size_t Idx = Res.second;
        if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(ParentTI)) {
          ParentStInfo->getField(Idx).setAddressTaken();
          setBaseTypeInfoSafetyData(Res.first, dtrans::FieldAddressTakenCall);
        }
      }
    }

    if (isa<InvokeInst>(Call)) {
      for (auto *Ty : LPI.getPointerTypeAliasSet()) {
        if (!DTInfo.isTypeOfInterest(Ty))
          continue;

        LLVM_DEBUG(dbgs() << "dtrans-safety: C++ handling -- "
                          << "pointer to struct passed as argument to "
                             "function invoke:\n "
                          << *Call << "\n " << *Arg << "\n");
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
      if (Param->user_empty())
        return;
      LocalPointerInfo &ParamLPI = LPA.getLocalPointerInfo(Param);

      if (isAliasSetOverloaded(LPI.getPointerTypeAliasSet(),
                               /*AllowElementZeroAccess=*/true) ||
          isAliasSetOverloaded(ParamLPI.getPointerTypeAliasSet(),
                               /*AllowElementZeroAccess=*/true)) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Mismatched argument use -- "
                          << "overloaded alias set found at call site:\n"
                          << "  " << *Call << "\n  " << *Arg << "\n");
        setAllAliasedTypeSafetyDataWithCascading(LPI, dtrans::MismatchedArgUse);
        setAllAliasedTypeSafetyDataWithCascading(ParamLPI,
                                                 dtrans::MismatchedArgUse);
      } else {
        llvm::Type *DomParamTy = ParamLPI.getDominantAggregateTy();
        llvm::Type *DomArgTy = LPI.getDominantAggregateTy();
        if (DomParamTy != DomArgTy) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Mismatched argument use -- "
                            << "overloaded alias set found at call site:\n"
                            << "  " << *Call << "\n  " << *Arg << "\n");
          setAllAliasedTypeSafetyDataWithCascading(LPI,
                                                   dtrans::MismatchedArgUse);
          setAllAliasedTypeSafetyDataWithCascading(ParamLPI,
                                                   dtrans::MismatchedArgUse);
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
        if (!F && DTInfo.getDTransUseCRuleCompat() && !HasICMatch &&
            !mayHaveDistinctCompatibleCType(AliasTy))
          continue;
        // If the first element of the dominant type of the pointer is an
        // an array of the actual argument, don't report address taken.
        if (isElementZeroArrayOfType(AliasTy, ArgTy)) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Field address taken call -- "
                            << "ptr to array element passed as argument:\n"
                            << "  " << *Call << "\n  " << *Arg << "\n");
          setBaseTypeInfoSafetyData(AliasTy, dtrans::FieldAddressTakenCall);
          dtrans::TypeInfo *ParentTI = DTInfo.getOrCreateTypeInfo(AliasTy);
          if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(ParentTI))
            ParentStInfo->getField(0).setAddressTaken();
          continue;
        }

        // If there is no use for the argument inside the function then we
        // don't need to worry because it will be deleted later. There is no
        // need to mark it as Address taken.
        // NOTE: Perhaps other safety checks inside visitCallArgument could
        // be relaxed in the same way.
        if (IsDeadArgument(IsFnLocal))
          continue;

        LLVM_DEBUG(dbgs() << "dtrans-safety: Address taken -- "
                          << "pointer to aggregate passed as argument:\n"
                          << "  " << *Call << "\n  " << *Arg << "\n");
        setBaseTypeInfoSafetyData(AliasTy, dtrans::AddressTaken);
      }
    }
  }

  void visitCallBase(CallBase &Call) {
    SmallPtrSet<const Value *, 3> SpecialArguments;

    // If the called function is a known allocation function, we need to
    // analyze the allocation.
    const TargetLibraryInfo &TLI = GetTLI(*Call.getFunction());
    auto AllocKind = dtrans::getAllocFnKind(&Call, TLI);
    if (AllocKind == dtrans::AK_NotAlloc)
      AllocKind = DTAA.getMallocPostDomKind(&Call);
    if (AllocKind != dtrans::AK_NotAlloc) {
      analyzeAllocationCall(&Call, AllocKind);
      collectSpecialAllocArgs(AllocKind, &Call, SpecialArguments, TLI);
    }

    // If this is a call to the "free" lib function,  the call is safe, but
    // we analyze the instruction for the purpose of capturing the argument
    // TypeInfo, which will be needed by some of the transformations when
    // rewriting allocations and frees.
    auto FreeKind = dtrans::isFreeFn(&Call, TLI)
                        ? (dtrans::isDeleteFn(&Call, TLI) ? dtrans::FK_Delete
                                                         : dtrans::FK_Free)
                        : dtrans::FK_NotFree;

    if (FreeKind == dtrans::FK_NotFree)
      FreeKind = DTAA.getFreePostDomKind(&Call);

    if (FreeKind != dtrans::FK_NotFree) {
      analyzeFreeCall(&Call, FreeKind);
      collectSpecialFreeArgs(FreeKind, &Call, SpecialArguments, TLI);
    }

    // Do not check and potentially set the mismatched arg safety condition on
    // the 'this' pointer argument of a bitcast call created by
    // devirtualization. The devirtualizer has already proven the argument type
    // is the correct type for the member function.
    if (Call.getMetadata("_Intel.Devirt.Call") && Call.arg_size() >= 1)
      SpecialArguments.insert(Call.getArgOperand(0));

    // Try to find the called function, stripping away Bitcasts or looking
    // through GlobalAlias definitions, if necessary, in order to check the
    // argument types against the parameter values.
    Function *F = dtrans::getCalledFunction(Call);

    // For all other calls, if a pointer to an aggregate type is passed as an
    // argument to a function in a form other than its dominant type, the
    // address has escaped. Also, if a pointer to a field is passed as an
    // argument to a function, the address of the field has escaped.
    // FIXME: Try to resolve indirect calls.
    bool IsFnLocal = F ? (!F->isDeclaration() &&
                          !F->hasDLLExportStorageClass()) : false;

    // Mark structures returned by non-local functions as system types.
    auto *RetTy = Call.getType();
    if (DTInfo.isTypeOfInterest(RetTy)) {
      if (AllocKind == dtrans::AK_NotAlloc) {
        if (!IsFnLocal) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: System object: "
                            << "type returned by extern function\n  " << *RetTy
                            << "\n");
          setBaseTypeInfoSafetyData(RetTy, dtrans::SystemObject);
        }
        if (isa<InvokeInst>(&Call)) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: C++ handling -- "
                            << "struct (or pointer to struct) returned from "
                               "function invoke:\n "
                            << Call << "\n");
          setBaseTypeInfoSafetyData(RetTy, dtrans::HasCppHandling);
        }
      }

      if (F && F->hasName()) {
        // We shouldn't apply any optimization on types that are used
        // by LibFuncs
        LibFunc TheLibFunc;
        if (TLI.getLibFunc(F->getName(), TheLibFunc) && TLI.has(TheLibFunc)) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: System object: "
                          << "type returned by a library function\n  " << *RetTy
                          << "\n");
          setBaseTypeInfoSafetyDataWithCascading(RetTy,
                                               dtrans::SystemObject);
        }
      }
    }

    if (SpecialArguments.size() == Call.arg_size())
      return;

    // Check for BitCast function (or aliasee)
    bool MayBeBitcast = false;
    Value *Callee = Call.getCalledOperand();
    if (auto *GA = dyn_cast<GlobalAlias>(Callee))
      Callee = GA->getAliasee();

    if (!isa<Function>(Callee))
        MayBeBitcast = true;

    if (F && MayBeBitcast) {
      // Account for layout in registers and on stack.
      if (!typesMayBeCRuleCompatible(
              Call.getCalledOperand()->getType()->getPointerElementType(),
              // Do not need to compare pointees types
              // here.
              F->getType()->getPointerElementType(), true)) {
        for (Argument &Arg : F->args()) {
          LLVM_DEBUG(
              dbgs()
              << "dtrans-safety: Mismatched argument use -- bitcast "
                 "function arguments mismatch with layout incompatibility:\n  "
              << Call);
          setValueTypeInfoSafetyDataWithCascading(&Arg,
                                                  dtrans::MismatchedArgUse);
        }

        for (Value *Arg : Call.args()) {
          LLVM_DEBUG(
              dbgs()
              << "dtrans-safety: Mismatched argument use -- bitcast "
                 "function arguments mismatch with layout incompatibility:\n  "
              << Call);
          setValueTypeInfoSafetyDataWithCascading(Arg,
                                                  dtrans::MismatchedArgUse);
        }
      } else {
        for (auto Pair : zip(F->args(), Call.args()))
          if (!SpecialArguments.count(std::get<1>(Pair)))
            checkArgTypeMismatch(&Call, F, &std::get<0>(Pair),
                                 std::get<1>(Pair));

        // Do not care about F->arg_size() > Call.arg_size(),
        // it should be marked in the true-branch, or it is UB.
        if (F->arg_size() < Call.arg_size())
          for (Value *Arg :
               make_range(Call.arg_begin() + F->arg_size(), Call.arg_end()))
            if (!SpecialArguments.count(Arg))
              checkArgTypeMismatch(&Call, F, nullptr, Arg);
      }
    }

    // Functions marked with callback metadata can take parameters and forward
    // them to another function. Check these parameters against the type
    // expected by the function the parameters will be forwarded to.
    if (F && F->hasMetadata(LLVMContext::MD_callback)) {
      // Populate the set of abstract calls that will be analyzed with the
      // parameters passed to this call.
      SmallVector<const Use *, 4> CallbackUses;
      AbstractCallSite::getCallbackUses(Call, CallbackUses);
      for (const Use *U : CallbackUses) {
        AbstractCallSite ACS(U);
        assert(ACS && ACS.isCallbackCall() && "must be a callback call");

        Function *TargetFunc = ACS.getCalledFunction();
        unsigned NumCalleeArgs = ACS.getNumArgOperands();
        for (unsigned Idx = 0; Idx < NumCalleeArgs; ++Idx) {
          // If the broker function passes the parameter through to the callee,
          // get the parameter corresponding to the target function argument and
          // check whether the types match. If the parameter is not forwarded,
          // the safety checks will be done when checking parameters passed to
          // the broker call.
          Value *Param = ACS.getCallArgOperand(Idx);
          if (Param) {
            // This argument will be processed here because it is being
            // forwarded to a call made by the callback routine. Mark it as a
            // "special argument" so that it is not processed when iterating
            // over the original function call arguments.
            SpecialArguments.insert(Param);
            if (TargetFunc && Idx < TargetFunc->arg_size()) {
              Argument *FormalVal = TargetFunc->getArg(Idx);
              checkArgTypeMismatch(&Call, TargetFunc, FormalVal, Param);
            } else if (isValueOfInterest(Param)) {
              LLVM_DEBUG(dbgs()
                         << "dtrans-safety: Unhandled use -- "
                         << "Value passed to callback function that uses an "
                            "indirect function call\n"
                         << *Param << "\n");
              setValueTypeInfoSafetyData(Param, dtrans::UnhandledUse);
            }
          }
        }
      }
    }

    // If this is an indirect call site, find out if there is a matching
    // address taken external call.  In this case, the indirect call must
    // be treated like an external call for the purpose of generating
    // the AddressTaken safety check.
    bool HasICMatch = hasICMatch(&Call);
    unsigned NextArgNo = 0;
    for (Value *Arg : Call.args()) {
      // Keep track of the argument index we're working with.
      unsigned ArgNo = NextArgNo++;

      if (!isValueOfInterest(Arg) || SpecialArguments.count(Arg))
        continue;

      visitCallArgument(&Call, F, HasICMatch, Arg, ArgNo);
    }
  }

  void visitBitCastInst(BitCastInst &I) {
    visitBitCastOperator(cast<BitCastOperator>(&I));
  }

  void visitBitCastOperator(BitCastOperator *I) {

    // Returns true if Ty is related to vector type.
    auto IsRelatedToVectorType = [](llvm::Type *Ty) {
      llvm::Type *BaseTy = Ty;

      // For pointers, see what they point to.
      while (BaseTy->isPointerTy())
        BaseTy = cast<PointerType>(BaseTy)->getElementType();

      if (BaseTy->isVectorTy())
        return true;
      return false;
    };

    llvm::Type *DestTy = I->getDestTy();
    llvm::Value *SrcVal = I->getOperand(0);

    // If the BitCast is dead then is safe to skip it
    if (isBitCastDead(I))
      return;

    // If the source operand is not a value of interest, we only need to
    // consider the destination type.
    if (!isValueOfInterest(SrcVal)) {
      // If the destination type is a type of interest, then the cast is
      // unsafe since we don't know anything about the source value.
      if (DTInfo.isTypeOfInterest(DestTy)) {
        if (DTBCA.isBitCastFromBadCastCandidate(I) ||
            DTBCA.isPotentialBitCastOfAllocStore(I)) {
          DTBCA.setSawBadCastBitCast(I);
          LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting (pending) -- "
                            << "unknown pointer cast to type of interest:\n"
                            << "  " << *I << "\n");
          setValueTypeInfoSafetyData(I, dtrans::BadCastingPending);
        } else {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
                            << "unknown pointer cast to type of interest:\n"
                            << "  " << *I << "\n");
          setValueTypeInfoSafetyData(I, dtrans::BadCasting);
        }
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
                        << "  " << *I << "\n");
      setValueTypeInfoSafetyData(I, dtrans::BadCasting);
      setValueTypeInfoSafetyData(SrcVal, dtrans::BadCasting);
      return;
    }
    // CMPLRLLVM-8956: Handle vector types conservatively for now.
    // TODO: Analysis and transformations need to be improved for
    // vector instructions and types.
    if (IsRelatedToVectorType(DestTy)) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
                        << "pointer cast to vector type:\n"
                        << "  " << *I << "\n");
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
        // CMPLRLLVM-8956: Handle vector types conservatively for now.
        // TODO: Analysis and transformations need to be improved for
        // vector instructions and types.
        if (IsRelatedToVectorType(PointeePair.first)) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
                            << "vector pointer cast to type of interest:\n"
                            << "  " << *I << "\n");
          setValueTypeInfoSafetyData(I, dtrans::BadCasting);
          return;
        }
        dtrans::TypeInfo *ParentTI =
            DTInfo.getOrCreateTypeInfo(PointeePair.first);
        llvm::Type *ElemTy;
        if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(ParentTI)) {
          assert(PointeePair.second.getKind() !=
                     LocalPointerInfo::PointeeLoc::PLK_Offset &&
                 "Unexpected use of invalid element");

          size_t ElementNum = PointeePair.second.getElementNum();
          assert(ElementNum < ParentStInfo->getNumFields());
          dtrans::FieldInfo &FI = ParentStInfo->getField(ElementNum);
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
    LocalPointerInfo &SelfLPI = LPA.getLocalPointerInfo(I);
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

        if (I.isVolatile())
          setBaseTypeInfoSafetyData(AliasTy, dtrans::VolatileData);

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

          auto *TI =
              DTInfo.getOrCreateTypeInfo(AliasTy->getPointerElementType());
          if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(TI)) {
            // Mark the 'mismatched element access' field attribute. When
            // DTransOutOfBoundsOK is enabled,  mark all the fields to be
            // consistent with how mismatched types are handled for the element
            // pointee case. Also, if the memory access extends beyond the
            // field's size, conservatively, mark all the fields.
            TypeSize LoadSize = DL.getTypeSizeInBits(I.getType());
            TypeSize FieldSize = ParentStInfo->getNumFields() != 0
                                 ? DL.getTypeSizeInBits(
                                   ParentStInfo->getField(0).getLLVMType())
                                 : TypeSize(0, false);
            if (DTInfo.getDTransOutOfBoundsOK() || LoadSize > FieldSize) {
              for (auto &FI : ParentStInfo->getFields())
                FI.setMismatchedElementAccess();
            } else if (ParentStInfo->getNumFields() != 0) {
              dtrans::FieldInfo &FI = ParentStInfo->getField(0);
              FI.setMismatchedElementAccess();
            }
          }
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

  // Return true if the the pointers to the beginning and the end of the memory
  // space are stored in the same structure. The input GEP represents a byte
  // flattened GEP that points to the end of a memory allocated spaces, Offset
  // is the offset being accessed (size of the structure), and DomAggTy is the
  // dominant aggregate type for the allocated space. This means that we found
  // a possible store to an STL structure (e.g. std::vector). For example:
  //
  //   %class.TestClass = type { i32, %"class.std::vector" }
  //   %"class.std::vector" = type { %"struct.std::_Vector_base" }
  //   %"struct.std::_Vector_base" = type { %"struct.std::_Vector_impl" }
  //   %"struct.std::_Vector_impl" = type { %class.TestClass*, %class.TestClass*,
  //                              %class.TestClass* }
  //
  //   %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0,
  //          i64 0, i32 1, i32 0, i32 0, i32 0
  //   %tmp2 = getelementptr inbounds %class.TestClass, %class.TestClass* %0,
  //           i64 0, i32 1, i32 0, i32 0, i32 1
  //
  //   %tmp3 = tail call noalias nonnull i8* @_Znwm(i64 32)
  //   %tmp4 = bitcast i8* %tmp3 to %class.TestClass*
  //
  //   %tmp5 = bitcast %class.TestClass** %tmp1 to i8**
  //   store i8* %tmp3, i8** %tmp5
  //   %tmp6 = getelementptr inbounds i8, i8* %tmp3, i64 32
  //   %tmp7 = bitcast %class.TestClass** %tmp2 to i8**
  //   store i8* %tmp6, i8** %tmp7
  //
  // The GEP in %tmp6 accesses the end of the memory space allocated at %tmp3.
  // The offset is 32, but the dominant aggregate type from %tmp3 will be
  // %class.TestClass. First, trace where %tmp6 is being stored, which is the
  // second entry of %"struct.std::_Vector_impl" (%tmp2). Then trace where
  // %tmp3 will be stored, which is the first entry of
  // %"struct.std::_Vector_impl" (%tmp1). Now we can compare %tmp1 with %tmp2,
  // and it shows that they are in the same structure, and the fields type
  // are pointers to the dominant aggregate type (%class.TestClass*). This
  // means that the pointer to the end of the memory allocated space is used
  // to trace where the information stored in %"struct.std::_Vector_impl"@0
  // ends. %"struct.std::_Vector_impl" can be seen as a possible structure
  // from the STL libraries.
  //
  // NOTE: This is conservative, we might be able to add an extra analysis
  // which proves that the dominant aggregate type for the input GEP is
  // DomAggTy.
  bool isGEPUsedForStoreInSTL(GetElementPtrInst *GEP,
                              llvm::Type *DomAggTy, uint64_t Offset) {

    // Given a Type Src, return true if traversing through the dereferences
    // it reaches InType.
    auto CheckPointerDereference = [](llvm::Type *Src, llvm::Type *InType) {
      if (Src == InType)
        return true;

      llvm::Type *CurrType = Src;
      while (auto *PtrTy = dyn_cast<PointerType>(CurrType)) {
        CurrType = PtrTy->getPointerElementType();
        if (CurrType == InType)
          return true;
      }
      return false;
    };

    // Given a Value Src, which will be the value operand of the input Store
    // instruction, return the GEP instruction that references to the memory
    // space where Src will be stored.
    auto CollectStorePosition = [this, DomAggTy, &CheckPointerDereference](
        Value *Src, StoreInst *Store) -> GetElementPtrInst* {

      if (Src != Store->getValueOperand())
        return nullptr;

      // The pointer operand is casting from a pointer to the dominant aggregate
      // type to an i8 pointer type
      BitCastInst *BC = dyn_cast<BitCastInst>(Store->getPointerOperand());
      if (!BC)
        return nullptr;

      if (!CheckPointerDereference(BC->getSrcTy(), DomAggTy))
        return nullptr;

      if (!CheckPointerDereference(BC->getDestTy(), Int8PtrTy))
        return nullptr;

      // GEP that points to the memory where Src is going to be stored
      GetElementPtrInst *StoringIntoGEP =
          dyn_cast<GetElementPtrInst>(BC->getOperand(0));

      return StoringIntoGEP;
    };

    // Return true if the GEP collects from a structure, there is no
    // prepadding and the field's type that the GEP is accessing in the
    // structure is the same as the dominant aggregate type or a pointer
    // to the dominant aggregate type.
    auto AnalyzeGEP =[this, DomAggTy](GetElementPtrInst *GEP,
                                      size_t *FieldNum,
                                      StructType **STy) -> bool {
      LocalPointerInfo &GEPLPI = LPA.getLocalPointerInfo(GEP);

      uint64_t PrePadBytes = 0;

      if (!isSimpleStructureMember(GEPLPI, STy, FieldNum, &PrePadBytes))
        return false;

      if (!STy || PrePadBytes != 0)
        return false;

      llvm::Type *DestType = (*STy)->getElementType(*FieldNum);
      if (DestType != DomAggTy && DestType->isPointerTy())
        DestType = DestType->getPointerElementType();

      return DestType == DomAggTy;
    };

    if (!GEP)
      return false;

    // For now, we are dealing with byte flattened GEP
    if (GEP->getNumIndices() != 1 || !GEP->hasAllConstantIndices())
      return false;

    if (!isInt8Ptr(GEP))
      return false;

    if (!DomAggTy || !DomAggTy->isStructTy())
      return false;

    // The offset must be the end of the pointer
    uint64_t DomAggTySize = DL.getTypeStoreSize(DomAggTy);
    if (Offset != DomAggTySize)
      return false;

    // There is only one use for the GEP and is a store
    if (!GEP->hasOneUse())
      return false;

    StoreInst *CurrStore = dyn_cast<StoreInst>(GEP->user_back());
    if (!CurrStore)
      return false;

    // Find the reference where GEP is going to be stored
    GetElementPtrInst *GEPCurrRef = CollectStorePosition(GEP, CurrStore);
    if (!GEPCurrRef || !GEPCurrRef->hasAllConstantIndices())
      return false;

    size_t CurrFieldNum = 0;
    StructType *CurrStruct = nullptr;
    if (!AnalyzeGEP(GEPCurrRef, &CurrFieldNum, &CurrStruct))
      return false;

    // All the fields in type found must be pointers to DomAggTy.
    // NOTE: This is conservative, there is a chance that a structure
    // uses the addresses to the beginning and the end of a memory
    // allocated space for some analysis (e.g. size of the vector),
    // and still has another field with another type for other purposes.
    for (unsigned I = 0, E = CurrStruct->getNumElements(); I < E; I++) {
      llvm::PointerType *FieldType =
          dyn_cast<PointerType>(CurrStruct->getElementType(I));
      if (!FieldType)
        return false;

      if (FieldType->getElementType() != DomAggTy)
        return false;
    }

    unsigned NumIndices = GEPCurrRef->getNumIndices();

    // The input GEP is accessing the end of a memory space and storing
    // it in the field of a structure. We are going to find now if
    // the pointer to the beginning of the memory space is stored in
    // the the same structure. If so, then it means that the combination
    // of stores represent a store into STL.
    Value *PointerBeginning = GEP->getOperand(0);
    for (User *U : PointerBeginning->users()) {
      if (StoreInst *PrevStore = dyn_cast<StoreInst>(U)) {

        // Find the reference to the previous field
        GetElementPtrInst *GEPPrevRef =
            CollectStorePosition(PointerBeginning, PrevStore);
        if (!GEPPrevRef)
          continue;

        // If one of the indices is not constant, then we don't know
        // where the GEP can point to
        if (!GEPPrevRef->hasAllConstantIndices())
          return false;

        if (NumIndices != GEPPrevRef->getNumIndices())
          continue;

        // Collect the structure and the field number that is
        // being accessed
        size_t PreviousNumField = 0;
        StructType *StructFound = nullptr;
        if (!AnalyzeGEP(GEPPrevRef, &PreviousNumField, &StructFound))
          continue;

        // Make sure that the types are the same
        if (StructFound != CurrStruct)
          continue;

        // All operands must match except the last one
        bool AllIndicesMatches = true;
        for (unsigned I = 0; I < NumIndices; I++) {
          if (GEPCurrRef->getOperand(I) != GEPPrevRef->getOperand(I)) {
            AllIndicesMatches = false;
            break;
          }
        }

        if (!AllIndicesMatches)
          continue;

        return true;
      }
    }

    return false;
  }

  // Helper function that checks if the Store instruction is used for storing
  // into a possible STL structure.
  bool isStoringIntoSTL(StoreInst *I) {
    if (!I)
      return false;

    // Check if the value operand of the Store instruction is a byte flattened
    // GEP.
    GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(I->getValueOperand());
    if (!GEP || GEP->getNumIndices() != 1 ||
        !GEP->hasAllConstantIndices() || !isInt8Ptr(GEP))
      return false;

    // Collect the offset
    uint64_t Offset = cast<ConstantInt>(GEP->getOperand(1))->getSExtValue();
    if (Offset == 0)
      return false;

    Value *Src = GEP->getPointerOperand();
    if (!isValueOfInterest(Src))
      return false;

    // Try to find the dominant aggregate type
    LocalPointerInfo &GEPLPI = LPA.getLocalPointerInfo(GEP);
    llvm::Type *DomAggTy = GEPLPI.getDominantAggregateTy();
    if (!DomAggTy && isInt8Ptr(Src)) {
      // If there is no dominant aggregate type, then check if it is possible
      // to collect it from GEP's pointer operand.
      LocalPointerInfo &LPI = LPA.getLocalPointerInfo(Src);
      DomAggTy = LPI.getDominantAggregateTy();
    }

    if (!DomAggTy)
      return false;

    if (DomAggTy->isPointerTy())
      DomAggTy = DomAggTy->getPointerElementType();

    // Now check if we can collect any information from the GEP
    return isGEPUsedForStoreInSTL(GEP, DomAggTy, Offset);
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
    bool StoringInZeroElement = isStoringZeroElement(I);

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
          auto GEPI = dyn_cast<GetElementPtrInst>(PtrOperand);
          if (GEPI && DTBCA.gepiMatchesCandidate(GEPI)) {
            LLVM_DEBUG(dbgs() << "dtrans-safety: Unsafe pointer store -- "
                              << " (pending) "
                              << "  Unmatch store of aliased value:\n");
            DTBCA.setSawUnsafePointerStore(I, AliasTy);
            setValueTypeInfoSafetyData(ValOperand,
                                       dtrans::UnsafePointerStorePending);
            setValueTypeInfoSafetyData(PtrOperand,
                                       dtrans::UnsafePointerStorePending);
          } else if (StoringInZeroElement) {
            LLVM_DEBUG(dbgs() << "dtrans-safety: Unsafe pointer store -- "
                              << "  Unmatch store to the zero element "
                              << " of aliased value:\n");
            setValueTypeInfoSafetyData(ValOperand,
                dtrans::UnsafePointerStoreRelatedTypes);
            setValueTypeInfoSafetyData(PtrOperand,
                dtrans::UnsafePointerStoreRelatedTypes);
          } else {
            LLVM_DEBUG(dbgs() << "dtrans-safety: Unsafe pointer store -- "
                              << "  Unmatch store of aliased value:\n");
            setValueTypeInfoSafetyData(ValOperand,
                                       dtrans::UnsafePointerStore);
            setValueTypeInfoSafetyData(PtrOperand,
                                       dtrans::UnsafePointerStore);
          }
          if (I != nullptr)
            LLVM_DEBUG(dbgs() << "  " << *I << "\n");
          else
            LLVM_DEBUG(dbgs()
                       << " " << *ValOperand << " -> " << *PtrOperand << " \n");
        }
      }
    } else if (PtrLPI.canAliasToAggregatePointer()) {
      // If we get here the value operand is not a pointer to an aggregate
      // type, but the pointer operand is. Unless (1) the value operand is a
      // null pointer, (2) the store is part of a partial pointer store idiom,
      // or (3) the value is an i8 and the aliased pointer type has an i8 at
      // element zero, this is a bad store.
      auto *ValTy = ValOperand->getType();
      bool isZeroOrNullVal = isa<ConstantPointerNull>(ValOperand);
      if (!isZeroOrNullVal) {
        // Another possible way to store null into a pointer is by assigning
        // 0 to the address of the pointer. For example:
        //
        //   %1 = getelementptr inbounds %"class.outer", %"class.outer"* %VAR,
        //        i64 0, i32 0
        //   %2 = bitcast %"class.inner1"* %1 to i64*
        //
        //   store i64 0, i64* %2
        if (auto *Const = dyn_cast<ConstantInt>(ValOperand))
          isZeroOrNullVal = Const->isZero() || Const->isNullValue();
      }

      if (!isZeroOrNullVal &&
          (!I || !isPartialPtrStore(I)) &&
          !(ValTy->isIntegerTy(8) &&
            dtrans::isElementZeroAccess(PtrLPI.getDominantAggregateTy(),
                                        ValTy->getPointerTo()))) {
        if (StoringInZeroElement) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Unsafe pointer store -- "
                            << "  Unknown store to the zero element of"
                            << " aliased ptr:\n");
          setValueTypeInfoSafetyData(PtrOperand,
              dtrans::UnsafePointerStoreRelatedTypes);
        } else if (isStoringIntoSTL(I)) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Unsafe pointer store -- "
                            << "Pointer to the end of the memory space is "
                            << "being stored into a possible STL "
                            << "structure:\n");
          setValueTypeInfoSafetyData(PtrOperand,
              dtrans::UnsafePointerStoreRelatedTypes);
        } else {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Unsafe pointer store -- "
                            << "  Unknown store to aliased ptr:\n");
          setValueTypeInfoSafetyData(PtrOperand, dtrans::UnsafePointerStore);
        }
        if (I != nullptr)
          LLVM_DEBUG(dbgs() << "  " << *I << "\n");
        else
          LLVM_DEBUG(dbgs()
                     << " " << *ValOperand << " -> " << *PtrOperand << " \n");
      }
    }
  }

  std::pair<llvm::Type *, size_t>
  getParentStructType(LocalPointerInfo::TypeAndPointeeLocPair &PointeePair,
                      Value *ValOp) {
    llvm::Type *ParentTy = PointeePair.first;
    assert((!ParentTy->isStructTy() ||
            PointeePair.second.getKind() !=
                LocalPointerInfo::PointeeLoc::PLK_Offset) &&
           "Unexpected use of invalid element");

    size_t Idx = PointeePair.second.getElementNum();
    if (PointeePair.first->isArrayTy() && Idx == 0) {
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
      if (I.isVolatile())
        setBaseTypeInfoSafetyData(ValOperand->getType(), dtrans::VolatileData);
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
        dtrans::TypeInfo *ParentTI = DTInfo.getOrCreateTypeInfo(Res.first);
        size_t Idx = Res.second;
        if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(ParentTI)) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Field address taken memory:\n"
                            << "  " << *(ParentTI->getLLVMType()) << " @ "
                            << Idx << "\n"
                            << "  " << I << "\n");
          setBaseTypeInfoSafetyData(Res.first,
                                    dtrans::FieldAddressTakenMemory);
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
    visitGetElementPtrOperator(cast<GEPOperator>(&I));
  }

  void visitGetElementPtrOperator(GEPOperator *I) {

    // Don't visit if the instruction is in a dead basic block
    if (ignoreBrokenInstruction(dyn_cast<Instruction>(I)))
      return;

    // Check if the address produced by the GEP operator is only consumed by
    // memset calls.
    auto OnlyUsedForMemset = [](GEPOperator *I) {
      if (I->users().empty())
        return false;

      for (auto *U : I->users()) {
        if (auto *MC = dyn_cast<MemSetInst>(U))
          if (MC->getDest() == I)
            continue;
        return false;
      }

      return true;
    };

    // TODO: Associate the parent type of the pointer so we can properly
    //       evaluate the uses.
    Value *Src = I->getPointerOperand();
    if (!isValueOfInterest(Src))
      return;

    // Since the GEP collapsing optimizations were turned off for DTrans, it is
    // possible to see a GEP chain which calculates final address. For safety
    // checks we need the condition to be set for all levels of nested
    // structures containing current address.
    GEPOperator *GEPO = nullptr;
    if (GEPOperator *CurrOp = dyn_cast<GEPOperator>(Src)) {
      while (auto *PrevGEP = dyn_cast<GEPOperator>(CurrOp->getPointerOperand()))
        CurrOp = PrevGEP;
      GEPO = CurrOp;
    }

    // If a GetElementPtr instruction is used for a pointer, which aliases
    // multiple aggregate types, we need to set safety data for each of the
    // types. This may be the result of a union, but it may also be the
    // result of some optimization which merged two element accesses.
    LocalPointerInfo &SrcLPI = LPA.getLocalPointerInfo(Src);
    if (SrcLPI.pointsToMultipleAggregateTypes()) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Ambiguous GEP:\n"
                        << "  " << *I << "\n");
      LLVM_DEBUG(SrcLPI.dump());
      setAllAliasedTypeSafetyData(SrcLPI, dtrans::AmbiguousGEP);
      // We set safety data on the first GEP. It will be naturally promoted to
      // subtypes.
      if (GEPO) {
        LocalPointerInfo &DeepestLPI =
            LPA.getLocalPointerInfo(GEPO->getPointerOperand());
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
    LocalPointerInfo &GEPLPI = LPA.getLocalPointerInfo(I);
    if (!GEPLPI.pointsToSomeElement()) {
      // If the source is an i8* value, this is a byte-flattened GEP access
      // and we should have been able to figure out the field being accessed.
      // Otherwise:
      //   - Zero index elements just results in the original pointer.
      //   - A single index element indicates a pointer is being treated
      //     as an array.
      if ((isInt8Ptr(Src) &&
           !(SrcLPI.isPtrToPtr() || SrcLPI.isPtrToCharArray())) ||
          I->getNumIndices() > 1) {

        // If the byte-flattened GEP access is used to access a location that is
        // between fields, and is only being used as a parameter to memset, we
        // will let the memset partial region analyzer check it.
        int64_t Offset = 0;
        if (I->getNumIndices() == 1 && I->hasAllConstantIndices())
          Offset = cast<ConstantInt>(I->getOperand(1))->getSExtValue();

        auto *DomTy = SrcLPI.getDominantAggregateTy();
        llvm::Type *DomAggTy = nullptr;
        if (DomTy && DomTy->isPointerTy()) {
          DomAggTy = DomTy->getPointerElementType();
        } else {

          // If there was no dominant type for the GEP pointer source, it may be
          // due to it being a element zero access. In this case, search for
          // a structure type in the element zero set. This is necessary because
          // a bitcast of a type that starts with a ptr-to-ptr, i8*, or array of
          // i8 elements does not have the type added to the alias set.
          for (auto *CurrType : SrcLPI.getElementZeroAliasSet()) {
            // TODO: Array aggregates are ignored for now because we don't need
            // byte-flattened GEP information about them.
            if (!CurrType->isStructTy())
              continue;
            if (DomAggTy) {
              DomAggTy = nullptr;
              break;
            }
            DomAggTy = DomTy;
          }
        }

        if (DomAggTy && DomAggTy->isStructTy() && Offset > 0 &&
            static_cast<uint64_t>(Offset) < DL.getTypeStoreSize(DomAggTy)) {
          if (OnlyUsedForMemset(I)) {
            // Record the access offset to allow the memset analyzer to check
            // it.
            GEPLPI.addElementPointeeByOffset(DomAggTy, Offset);
          } else {
            // We know "Offset" is positive from the above test. Convert it to
            // an unsigned type to avoid sign mismatch compiler warnings during
            // comparisons.
            uint64_t UOffset = static_cast<uint64_t>(Offset);
            auto *SL = DL.getStructLayout(cast<llvm::StructType>(DomAggTy));
            bool Matched = false;
            if (UOffset < SL->getSizeInBytes()) {
              unsigned IdxAtOffset = SL->getElementContainingOffset(UOffset);
              uint64_t ElementOffset = SL->getElementOffset(IdxAtOffset);
              if (UOffset == ElementOffset) {
                GEPLPI.addElementPointee(DomAggTy, IdxAtOffset);
                std::pair<llvm::Type *, size_t> Pointee(DomAggTy, IdxAtOffset);
                DTInfo.addByteFlattenedGEPMapping(I, Pointee);
                Matched = true;
              }
            }
            if (!Matched) {
              LLVM_DEBUG(dbgs() << "dtrans-safety: Bad pointer manipulation:\n"
                                << "  " << *I << "\n");
              setBaseTypeInfoSafetyData(DomAggTy, dtrans::BadPtrManipulation);
            }
          }
        } else if (isGEPUsedForStoreInSTL(
              dyn_cast<GetElementPtrInst>(I), DomAggTy,
              static_cast<uint64_t>(Offset))) {
          // NOTE: This analysis is for the case when we have byte flattened
          // GEP pointing to the end of a memory space. There is the
          // possibility that the GEP is not byte flattened and points
          // to the end. For example:
          //
          //   %class.TestClass = type { i32, %"class.std::vector" }
          //   %"class.std::vector" = type { %"struct.std::_Vector_base" }
          //   %"struct.std::_Vector_base" =
          //       type { %"struct.std::_Vector_impl" }
          //   %"struct.std::_Vector_impl" = type { %class.TestClass*,
          //                                        %class.TestClass*,
          //                                        %class.TestClass* }
          //
          //   %tmp1 = tail call noalias nonnull i8* @_Znwm(i64 24)
          //   %tmp2 = bitcast i8* %tmp1 to %class.TestClass*
          //   %tmp3 = getelementptr inbounds %class.TestClass,
          //           %class.TestClass* %tmp2, i64 1
          //
          // The GEP in %tmp3 won't produce a "Bad pointer manipulation",
          // nor "Bad pointer manipulation for related types". The reason
          // is that all checks after the conditional
          // "!GEPLPI.pointsToSomeElement()" are related to byte flattened
          // GEPs. Also, for this case, the local pointer analysis will find a
          // dominant aggregate type for %tmp3.
          LLVM_DEBUG(dbgs() << "dtrans-safety: Bad pointer manipulation "
                            << "(related types) -- "
                            << "Pointer to the end of the memory space is "
                            << "being stored into a possible STL structure:\n"
                            << "  " << I << "\n");
          setAllAliasedTypeSafetyData(SrcLPI,
              dtrans::BadPtrManipulationForRelatedTypes);
        } else {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Bad pointer manipulation:\n"
                            << "  " << *I << "\n");
          setAllAliasedTypeSafetyData(SrcLPI, dtrans::BadPtrManipulation);
          // We set safety data on the first GEP. It will be naturally promoted
          // to subtypes.
          if (GEPO) {
            LocalPointerInfo &DeepestLPI =
              LPA.getLocalPointerInfo(GEPO->getPointerOperand());
            setAllAliasedTypeSafetyData(DeepestLPI, dtrans::BadPtrManipulation);
          }
        }
      }
    } else {
      auto &PointeeSet = GEPLPI.getElementPointeeSet();
      if (PointeeSet.size() == 1 && isInt8Ptr(Src)) {
        // If the GEP is pointing to some element and it is in the
        // byte-flattened form, store this information for later reference.
        LocalPointerInfo::TypeAndPointeeLocPair Element = *PointeeSet.begin();
        assert((!Element.first->isStructTy() ||
                Element.second.getKind() !=
                    LocalPointerInfo::PointeeLoc::PLK_Offset) &&
               "Unexpected use of invalid element");
        std::pair<llvm::Type *, size_t> Pointee(Element.first,
                                                Element.second.getElementNum());
        DTInfo.addByteFlattenedGEPMapping(I, Pointee);
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

      if (hasNonCastLoadStoreUses(I)) {
        for (auto PointeePair : PointeeSet) {
          llvm::Type *ParentTy = PointeePair.first;
          if (ParentTy->isStructTy()) {
            auto *ParentStInfo =
                cast<dtrans::StructInfo>(DTInfo.getOrCreateTypeInfo(ParentTy));
            assert(PointeePair.second.getKind() !=
                       LocalPointerInfo::PointeeLoc::PLK_Offset &&
                   "Unexpected use of invalid element");
            dtrans::FieldInfo &FI =
                ParentStInfo->getField(PointeePair.second.getElementNum());
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
    visitPtrToIntOperator(cast<PtrToIntOperator>(&I));
  }

  void visitPtrToIntOperator(PtrToIntOperator *I) {
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
    Value *Src = I->getPointerOperand();
    if (!isValueOfInterest(Src))
      return;
    if (I->getType() != PtrSizeIntTy) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
                        << "Ptr to aggregate cast to a non-ptr-sized integer:\n"
                        << "  " << *I << "\n");
      setValueTypeInfoSafetyData(Src, dtrans::BadCasting);
    }

    // The isValueOfInterest() routine analyzes all PtrToInt result values
    // when they are used. Nothing more is needed here.
  }

  // TODO: Need to extend the analysis for IntToPtr instruction for more
  // accurate safety checks.
  void visitIntToPtrInst(IntToPtrInst &I) {

    // Return true if the IntToPtr instruction represents a load. For
    // example:
    //
    // %"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
    // %"struct.std::_Vector_base.13" =
    //     type { %"struct.std::_Vector_base<TestClass,
    //     std::allocator<TestClass>>::_Vector_impl.67" }
    // %"struct.std::_Vector_base<TestClass,
    //     std::allocator<TestClass>>::_Vector_impl.67" =
    //     type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
    // %class.TestClass = type {i64, i64, i64}
    //
    // %tmp = getelementptr inbounds %"class.std::vector.12",
    //                      %"class.std::vector.12"* %arg, i64 0, i32 0
    // %tmp2 = bitcast %"struct.std::_Vector_base.13"* %tmp to i64*
    // %tmp3 = load i64, i64* %tmp2
    // %tmp4 = inttoptr i64 %tmp3 to %class.TestClass*
    //
    // In the example above there is a GEP (%tmp) that goes through a BitCast
    // (%tmp2) and then is loading the innermost structure (%tmp3 and %tmp4).
    // This function will validate that %classTestClass* is an inner structure
    // of %"struct.std::_Vector_base.13".
    auto IsLoadingInnermostStructure = [&I, this]() -> bool {

      LoadInst *Load = dyn_cast<LoadInst>(I.getOperand(0));
      if (!Load)
        return false;

      BitCastInst *BC = dyn_cast<BitCastInst>(Load->getPointerOperand());
      if (!BC || !BC->hasOneUse())
        return false;

      // Check that the destination pointer in the int-to-pointer instruction
      // is the same size as the load and the source pointer in the BitCast
      llvm::Type *LoadTy = I.getSrcTy();
      llvm::Type *DestTy = I.getDestTy();
      llvm::Type *BCSourceTy = BC->getSrcTy();

      uint64_t DestSize = DL.getTypeSizeInBits(DestTy).getFixedSize();
      uint64_t LoadSize = DL.getTypeSizeInBits(LoadTy).getFixedSize();
      uint64_t BCSourceSize = DL.getTypeSizeInBits(BCSourceTy).getFixedSize();

      if (DestSize != LoadSize || LoadSize != BCSourceSize ||
          DestSize != BCSourceSize)
        return false;

      // Now make sure that the destination for the IntToPointer is a pointee
      // of the BitCast instruction
      return isBitCastUsedToAccessAnInnerStructure(cast<BitCastOperator>(BC),
                                                   DestTy);
    };

    if (!isValueOfInterest(&I))
      return;

    // It is OK if the IntToPtr instruction is used to represent a load to a
    // field in a structure
    if (IsLoadingInnermostStructure())
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
        dtrans::TypeInfo *ParentTI = DTInfo.getOrCreateTypeInfo(Res.first);
        size_t Idx = Res.second;
        if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(ParentTI)) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Field address taken return -- "
                            << "Address of a field is returned by function: "
                            << I.getParent()->getParent()->getName() << "\n");
          setBaseTypeInfoSafetyData(Res.first,
                                    dtrans::FieldAddressTakenReturn);
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

  //
  // CMPLRLLVM-12063: Throw the "bad casting" safety check if a Constant used
  // in an initializer of a GlobalVariable is being cast from a source type to
  // the declared type of the GlobalVariable or one of its fields.
  //
  // To illustrate the issue, consider this GlobalVariable definition:
  //  @T = internal unnamed_addr constant %eh.ThrowInfo
  //    { i32 0, i8* bitcast (%eh.CatchableTypeArray* @U to i8*) }
  // If the type %eh.ThrowInfo has a safety check violation like "system
  // object", this definition of @T implies that %eh.CatchableTypeArray
  // should also have the "system object" safety violation, because the
  // initializer of @T implies that the GlobalVariable @U is accessible
  // from @T. We note this as a "bad casting" because we do not
  // enough context to determine what safety violation might be on
  // %eh.ThrowInfo.
  //
  // NOTE: Andy Kaylor and Chris Chrulski prefer marking this as "unsafe
  // pointer store", but right now "unsafe pointer store" does not cascade
  // through pointer types. After the immediate release (Feb 2020), we should
  // try replacing this with "unsafe pointer store" and add "unsafe pointer
  // store" to the cases in isPointerCarriedSafetyCondition().
  //
  // NOTE: This may be a more conservative fix than necessary. In the future,
  // we can enhance this by checking whether the initializer type is compatible
  // with the field declaration type.
  //
  void visitBitCastInInitializer(Constant *Initializer) {
    auto BC = dyn_cast_or_null<BitCastOperator>(Initializer);
    if (!BC)
      return;
    setBaseTypeInfoSafetyData(BC->getSrcTy(), dtrans::BadCasting);
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
          visitBitCastInInitializer(Initializer);
          analyzeGlobalStructSingleValue(GVElemTy, Initializer);

          DEBUG_WITH_TYPE(
              DTRANS_CG,
              dbgs() << "dtrans-cg: CGraph update for Globalvalue  -- \n"
                     << "  " << GV << "\n");
          setBaseTypeCallGraph(GV.getType(), nullptr);
        }
        analyzeGlobalVariableUses(GV);
      }
    }
    // Analyze definitions of all structures in type info list.
    for (dtrans::TypeInfo *TI : DTInfo.type_info_entries())
      analyzeStructureType(TI);
  }

  // Identify the call graph information that collects the outermost type that a
  // method operates upon.
  // Note: This function needs to be run after the local pointer analyzer
  // visitor has completed because it can cause analyzeValue to be run on
  // elements during the check of "isValueOfInterest" that have not been visited
  // yet. Unfortunately, with the current implementation of the "visit"
  // functions, differences may occur when analyzeValue is called for a Value
  // prior to the corresponding "visit" function being executed.
  void collectCallGraphInfo(Module &M) {
    for (auto &F : M) {
      for (auto It = inst_begin(&F), E = inst_end(&F); It != E; ++It) {
        Instruction &I = *It;
        if (isValueOfInterest(&I)) {
          DEBUG_WITH_TYPE(
              DTRANS_CG, dbgs()
                             << "dtrans-cg: CGraph update for Instruction -- \n"
                             << "  " << I << "\n");
          setBaseTypeCallGraph(I.getType(), &F);
        }

        for (Value *Arg : I.operands()) {
          if (!isa<Constant>(Arg) && !isa<Argument>(Arg))
            continue;
          if (isValueOfInterest(Arg)) {
            DEBUG_WITH_TYPE(DTRANS_CG,
                            dbgs()
                                << "dtrans-cg: CGraph update for Operand  -- \n"
                                << "  " << *Arg << "\n");
            setBaseTypeCallGraph(Arg->getType(), &F);
          }
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
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (dtrans::DTransPrintAnalyzedTypes)
      for (auto &Arg : F.args())
        if (DTInfo.isTypeOfInterest(Arg.getType()))
          (void)DTInfo.getOrCreateTypeInfo(Arg.getType());
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    // Get BFI if available.
    BFI = (!F.isDeclaration()) ? &(GetBFI(F)) : nullptr;

    // Call the base class to visit the instructions in the function.
    InstVisitor<DTransInstVisitor>::visitFunction(F);
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

  // This is a version of setValueTypeInfoSafetyDataBase() that always cascades
  // the safety data to nested types and through pointers for all types the
  // local pointer info has associated with \p V.
  bool setValueTypeInfoSafetyDataBaseWithCascading(Value *V,
                                                   dtrans::SafetyData Data) {
    if (!isValueOfInterest(V))
      return false;

    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(V);
    setAllAliasedTypeSafetyDataWithCascading(LPI, Data);
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

  // This is a version of setValueTypeInfoSafetyData() that always cascades the
  // safety data to nested types and through pointers. This sets the safety
  // data on all types the local pointer info has associated with \p V, and to
  // members of the element pointee set.
  void setValueTypeInfoSafetyDataWithCascading(Value *V,
                                               dtrans::SafetyData Data) {

    if (!setValueTypeInfoSafetyDataBaseWithCascading(V, Data))
      return;

    // If the value is a pointer to an element in some aggregate type
    // set the safety info for that type also.
    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(V);
    auto ElementPointees = LPI.getElementPointeeSet();
    if (ElementPointees.size() > 0)
      for (auto &PointeePair : ElementPointees)
        setBaseTypeInfoSafetyDataWithCascading(PointeePair.first, Data);
  }

  // This method is used for noting certain types of bitcast operations that
  // may need to be carried through the pointer members of structure type.
  // However, it cannot be determined whether the safety data needs to be
  // propagated through the pointer fields until the complete application has
  // been examined. If the casting is happening between two types of interest,
  // record a safety check for a pointer carried safety condition that involves
  // the casting of one type to another for later processing. If there is only
  // one type of interest, the safety bit can be cascaded completely right now.
  void addDeferredPointerCarriedSafetyData(llvm::Type *CastTy1,
                                           llvm::Type *CastTy2,
                                           dtrans::SafetyData Data) {
    // TODO: Currently, this deferral of cascaded pointer carried data has
    // only been analyzed for the MismatchedArgUse safety condition. In the
    // future, other bitcast safety conditions may also be able to be handled
    // here.
    assert(Data == dtrans::MismatchedArgUse &&
           "Unsupported safety data for deferred pointer carried safety "
           "setting");

    llvm::Type *Ty1 = nullptr;
    if (CastTy1 && DTInfo.isTypeOfInterest(CastTy1))
      Ty1 = CastTy1;

    llvm::Type *Ty2 = nullptr;
    if (CastTy2 && DTInfo.isTypeOfInterest(CastTy2))
      Ty2 = CastTy2;

    if (Ty1 && Ty2)
      DeferredCastingSafetyCascades.insert(std::make_tuple(Ty1, Ty2, Data));
    else if (Ty1)
      setBaseTypeInfoSafetyDataWithCascading(Ty1, Data);
    else if (Ty2)
      setBaseTypeInfoSafetyDataWithCascading(Ty2, Data);
  }

  // Set the deferred safety checks for the types that were collected, but needed
  // the entire module to be analyzed before it could be determined which safety
  // data will be pointer carried.
  void processDeferredPointerCarriedSafetyData() {
    LLVM_DEBUG(dbgs() << "\nStart of processing deferred safety cascades\n");

    // When processing the deferred safety checks, we need to consider the
    // entire set of source/destination types involved, rather than each pair
    // individually. For example: %struct.foo could be used as %struct.foo.2 in
    // one function and %struct.foo.3 in another function, so we need to take
    // into account the information about which fields of %struct.foo.2 and
    // %struct.foo.3 are used when determining whether to cascade the safety
    // flag. To do this, first get the set of structures that are related, and
    // then analyze each pair of the set.
    EquivalenceClasses<Type *> CastTypes;
    for (auto &Elem : DeferredCastingSafetyCascades) {
      // TODO: We will need to extend the algorithm to iterate over different
      // safety types, if the we start using 'DeferredCastingSafetyCascades' for
      // more than just MismatchedArgUse.
      dtrans::SafetyData Data = std::get<2>(Elem);
      assert(Data == dtrans::MismatchedArgUse &&
             "Deferred pointer carried safety data currently only supports "
             "MismatchedArgUse");
      (void)Data;

      llvm::Type *Ty1 = std::get<0>(Elem);
      llvm::Type *Ty2 = std::get<1>(Elem);
      CastTypes.unionSets(Ty1, Ty2);
    }

    SmallVector<Type *, 16> SetLeaders;
    for (auto It = CastTypes.begin(), E = CastTypes.end(); It != E; ++It) {
      // A "Leader" in an EquivalenceClasses set is just the first entry
      // in a group of entries that have been marked as belonging together.
      // It serves as the head of a linked list.
      if (!It->isLeader())
        continue; // Skip over non-leader sets.
      SetLeaders.emplace_back(It->getData());
    }

    for (auto *LeaderTy : SetLeaders) {
      auto I = CastTypes.findValue(LeaderTy);
      auto MI = CastTypes.member_begin(I);
      auto ME = CastTypes.member_end();
      SmallVector<llvm::Type *, 4> TransitiveGroup;
      for (; MI != ME; ++MI) {
        TransitiveGroup.push_back(*MI);
      }

      size_t Len = TransitiveGroup.size();
      for (size_t I = 0; I < Len; ++I) {
        llvm::Type *Ty1 = TransitiveGroup[I];
        for (size_t J = I + 1; J < Len; ++J) {
          llvm::Type *Ty2 = TransitiveGroup[J];
          cascadeSafetyDataToMismatchedFields(Ty1, Ty2,
                                              dtrans::MismatchedArgUse);
        }
      }
    }
  }

  // This function is for setting safety data on a type when one type is
  // cast to another type, such as at a bitcast function call that coerces a
  // parameter into the type expected by the function. This type of safety data
  // needs to be cascaded through pointer members contained within structures.
  // However, there are some conditions that can allow us to resolve that
  // there is not a safety issue which requires the safety data to be
  // pointer carried. This function analyzes those conditions, and only
  // propagates the safety data when necessary.
  //
  // The following conditions do not require the safety data to be pointer
  // carried:
  //
  // 1) The same type is reached at that same offset in both the type
  // being cast from and the type being cast to.
  //
  // For example:
  //    Src: %typeA = { %typeB*, %typeC* }
  //    Dst: %typeA.1 = { %typeB.1*, %typeC* }
  //
  // The safety data will be carried to the pointer members %typeB and %typeB.1,
  // but does not need to be carried to %typeC because the same data type will
  // be used regardless of which form of %typeA is used.
  //
  // 2) The pointer is never dereferenced by the program in one of the types.
  // This typically occurs as a result of the IRMover assembling types which
  // contained opaque types resulting in the bitcast at function boundaries, but
  // don't actually exist in the program.
  //
  // For example:
  //    Src: %typeA = { %typeB*, %typeC* }
  //    Dst: %typeA.1 = { %typeB.1*, %typeC* }
  //         %typeB = { %typeD* }
  //         %typeD = opaque
  //
  //    If the %typeB.1 field of %typeA.1 is not referenced by the program,
  //    then cast of %typeA to %typeA.1 cannot cause accesses to %typeB to be
  //    mismatched.
  //
  void cascadeSafetyDataToMismatchedFields(llvm::Type *SrcTy, llvm::Type *DstTy,
                                           dtrans::SafetyData Data) {

    // Helper to check whether a pointer field is dereferenced. This can be
    // improved in the future by having the analysis keep a 'dereferenced' bit
    // in the FieldInfo structure. Currently, this is more conservative than
    // necessary because a memset/memcpy could result in the field being marked
    // read/written.
    auto IsFieldUsed = [](dtrans::FieldInfo &FI) {
      return FI.isAddressTaken() || FI.isRead() || FI.isWritten() ||
             FI.hasComplexUse();
    };

    // This lambda does the work of analyzing and propagating the safety data.
    std::function<void(llvm::Type *, llvm::Type *, dtrans::SafetyData,
                       SmallPtrSetImpl<llvm::Type *> &, unsigned)>
        CascadeToMismatchedFields =
            [this, &CascadeToMismatchedFields, &IsFieldUsed](
                llvm::Type *SrcTy, llvm::Type *DstTy, dtrans::SafetyData Data,
                SmallPtrSetImpl<llvm::Type *> &Visited,
                unsigned Depth) -> void {
      // SrcTy and DstTy are being iterated in lockstep, so only need to check
      // if a visit of the source type is underway.
      if (!Visited.insert(SrcTy).second)
        return;

      LLVM_DEBUG({
        dbgs().indent(Depth * 2);
        dbgs() << "Propagating to : " << *SrcTy << "\n";
        dbgs().indent(Depth * 2);
        dbgs() << "Propagating to : " << *DstTy << "\n";
      });

      if (SrcTy == DstTy) {
        LLVM_DEBUG(dbgs() << "Common type reached: ending propagation\n");
        return;
      }

      // If the src and dst types cannot be walked concurrently, set
      // the safety data on all elements reachable from the src and dst
      // types. This may be more conservative than necessary because some
      // fields may match each other in the src and dst types.
      if (SrcTy->getTypeID() != DstTy->getTypeID() ||
          SrcTy->getNumContainedTypes() != DstTy->getNumContainedTypes()) {
        setPointerCarriedCascadingSafetyData(SrcTy, Data);
        setPointerCarriedCascadingSafetyData(DstTy, Data);
        return;
      }

      // For pointer types, step into the object type that is pointed-to and set
      // the safety bit.
      if (auto PtrSrcTy = dyn_cast<llvm::PointerType>(SrcTy)) {
        CascadeToMismatchedFields(PtrSrcTy->getPointerElementType(),
                                  DstTy->getPointerElementType(), Data, Visited,
                                  Depth + 1);
        return;
      }

      if (!SrcTy->isAggregateType())
        return;

      // For arrays, set the safety bit on the src and dst arrays, then cascade
      // it to the element type of the array.
      if (isa<llvm::ArrayType>(SrcTy)) {
        dtrans::TypeInfo *SrcTI = DTInfo.getOrCreateTypeInfo(SrcTy);
        dtrans::TypeInfo *DstTI = DTInfo.getOrCreateTypeInfo(DstTy);
        SrcTI->setSafetyData(Data);
        DstTI->setSafetyData(Data);
        CascadeToMismatchedFields(SrcTy->getArrayElementType(),
                                  DstTy->getArrayElementType(), Data, Visited,
                                  Depth + 1);
        return;
      }

      // For struct types, walk the fields of the source and destination types
      // concurrently. If both structures contain a pointer to the same type at
      // the same offset, then there is no casting violation. Also, if one of
      // the structures never dereferences the pointer, then there not a casting
      // violation.
      assert(SrcTy->isStructTy() && "Expected struct type");
      auto SrcStTy = cast<llvm::StructType>(SrcTy);
      auto DstStTy = cast<llvm::StructType>(DstTy);

      // If one of the types is opaque, there can't be accesses to mismatched
      // fields.
      if (SrcStTy->isOpaque() || DstStTy->isOpaque())
        return;

      // Set the safety data on the structure, then walk the members to set it
      // on the nested elements and cases where it should be pointer-carried.
      dtrans::StructInfo *SrcTI =
          cast<dtrans::StructInfo>(DTInfo.getOrCreateTypeInfo(SrcTy));
      SrcTI->setSafetyData(Data);
      dtrans::StructInfo *DstTI =
          cast<dtrans::StructInfo>(DTInfo.getOrCreateTypeInfo(DstTy));
      DstTI->setSafetyData(Data);

      auto *SrcLayout = DL.getStructLayout(SrcStTy);
      auto *DstLayout = DL.getStructLayout(DstStTy);
      for (unsigned I = 0, E = SrcTy->getNumContainedTypes(); I != E; ++I) {
        // Check that field offsets are still matching up when walking,
        // otherwise remaining fields need to be marked. If a mismatch in
        // offsets is found, all elements starting from that element need to be
        // cascaded to.
        if (SrcLayout->getElementOffset(I) != DstLayout->getElementOffset(I)) {
          for (; I != E; ++I) {
            setPointerCarriedCascadingSafetyData(SrcTy->getContainedType(I),
                                                 Data);
            setPointerCarriedCascadingSafetyData(DstTy->getContainedType(I),
                                                 Data);
          }
          return;
        }

        // If the field is not used for one of the types, there is not a need to
        // cascade the safety data to the field of the other type.
        dtrans::FieldInfo &SrcFI = SrcTI->getField(I);
        dtrans::FieldInfo &DstFI = DstTI->getField(I);
        if (SrcFI.getLLVMType()->isPointerTy() &&
            DstFI.getLLVMType()->isPointerTy() &&
            (!IsFieldUsed(SrcFI) || !IsFieldUsed(DstFI)))
          continue;

        CascadeToMismatchedFields(SrcTy->getContainedType(I),
                                  DstTy->getContainedType(I), Data, Visited,
                                  Depth + 1);
      }
    };

    LLVM_DEBUG(
        dbgs() << "cascadeSafetyDataToMismatchedFields checking the pair: "
               << *SrcTy << "\nAgainst: " << *DstTy << "\n");
    SmallPtrSet<llvm::Type *, 8> Visited;
    CascadeToMismatchedFields(SrcTy, DstTy, Data, Visited, 0);
  }

  // Cascade the safety condition to all nested structures and all structures
  // reached via pointers within the type, recursively. This function differs
  // from setBaseTypeInfoSafetyData in that it will continue descending into
  // types even if the safety data is set on that type. This is necessary
  // when the deferred safety checks are being set, because that routine has not
  // exhaustively set the safety data yet, so a field may be reached that has
  // the safety data on it, but members within it have not been set yet.
  void setPointerCarriedCascadingSafetyData(llvm::Type *Ty,
                                            dtrans::SafetyData Data) {

    // Lambda that sets the safety bit, and recurses through the elements,
    // keeping track of which structures have been visited so far.
    DTransAnalysisInfo &DTInfo = this->DTInfo;
    std::function<void(llvm::Type *, dtrans::SafetyData,
                       SmallPtrSetImpl<llvm::Type *> &, unsigned Depth)>
        SetPointerCarriedRecursive = [&DTInfo, &SetPointerCarriedRecursive](
                                         llvm::Type *Ty,
                                         dtrans::SafetyData Data,
                                         SmallPtrSetImpl<llvm::Type *> &Visited,
                                         unsigned IndentLevel) -> void {
      llvm::Type *BaseTy = Ty;
      while (BaseTy->isPointerTy())
        BaseTy = BaseTy->getPointerElementType();

      if (!DTInfo.isTypeOfInterest(BaseTy))
        return;

      if (!Visited.insert(BaseTy).second)
        return;

      LLVM_DEBUG({
        dbgs().indent(IndentLevel * 2);
        dbgs() << "Cascading to: " << *BaseTy << "\n";
      });

      dtrans::TypeInfo *TI = DTInfo.getOrCreateTypeInfo(BaseTy);
      TI->setSafetyData(Data);

      // Propagate this condition to any nested types or pointer elements.
      if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI))
        for (dtrans::FieldInfo &FI : StInfo->getFields())
          SetPointerCarriedRecursive(FI.getLLVMType(), Data, Visited,
                                     IndentLevel + 1);
      else if (isa<dtrans::ArrayInfo>(TI))
        SetPointerCarriedRecursive(BaseTy->getArrayElementType(), Data, Visited,
                                   IndentLevel + 1);
    };

    // Recursively, set the safety data on every nested type and pointer type
    // reached from Ty.
    SmallPtrSet<llvm::Type *, 8> Visited;
    SetPointerCarriedRecursive(Ty, Data, Visited, 0);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Dump the local pointer analyzer results for all functions.
  void dump(Module &M) {
    // This class is used to annotate the IR dump output with
    // the information collected by the local pointer analyzer
    // for pointers, or values that may have been cast from pointers.
    class AnnotatedWriter : public AssemblyAnnotationWriter {
    private:
      LocalPointerAnalyzer &LPA;

    public:
      AnnotatedWriter(LocalPointerAnalyzer &LPA) : LPA(LPA) {}

      // Output the types pointer parameters are used as within the function.
      void emitFunctionAnnot(const Function *F,
                             formatted_raw_ostream &OS) override {
        // We don't have LPA information for parameters of function
        // declarations.
        if (F->isDeclaration())
          return;

        OS << ";  Input Parameters: " << F->getName() << "\n";
        for (auto &Arg : F->args())
          if (Arg.getType()->isPointerTy()) {
            OS << ";    Arg " << Arg.getArgNo() << ": " << Arg;
            Argument *ArgC = const_cast<Argument *>(&Arg);
            emitLPA(ArgC, OS);
          }
      }

      // Output the LPA information about the value object
      void printInfoComment(const Value &CV,
                            formatted_raw_ostream &OS) override {
        Value *V = const_cast<Value *>(&CV);
        emitLPA(V, OS);
      }

      void emitLPA(Value *V, formatted_raw_ostream &OS) {
        std::function<void(formatted_raw_ostream &, ConstantExpr *)>
            PrintConstantExpr =
                [&PrintConstantExpr, this](formatted_raw_ostream &OS,
                                           ConstantExpr *CE) -> void {
          OS << "\n;        CE: " << *CE << "\n";
          auto &LPI = LPA.getLocalPointerInfo(CE);
          LPI.print(OS, 10, ";");
          OS << ";            ";
          printDomTy(OS, LPI);

          // There may be constant expressions nested within this CE that should
          // be reported.
          for (auto *Op : CE->operand_values())
            if (auto *InnerCE = dyn_cast<ConstantExpr>(Op))
              PrintConstantExpr(OS, InnerCE);
        };

        // Check for any constant expressions being used for the instruction,
        // and report types for those, if available.
        if (auto *I = dyn_cast<Instruction>(V))
          for (auto *Op : I->operand_values())
            if (auto *CE = dyn_cast<ConstantExpr>(Op))
              PrintConstantExpr(OS, CE);

        if (!LPA.isPossiblePtrValue(V))
          return;

        LocalPointerInfo &LPI = LPA.getLocalPointerInfo(V);
        if (V->getType()->isPointerTy() || LPI.canAliasToAggregatePointer() ||
            LPI.pointsToSomeElement()) {
          OS << "\n";
          LPI.print(OS, 4, ";");
          OS << ";      ";
          printDomTy(OS, LPI);
        }
      }

      void printDomTy(formatted_raw_ostream &OS, LocalPointerInfo &LPI) {
        auto *DomTy = LPI.getDominantAggregateTy();
        if (DomTy)
          OS << "DomTy: " << *DomTy << "\n";
        else if (LPI.canAliasToAggregatePointer())
          OS << "Ambiguous Dominant Type\n";
        else
          OS << "No Dominant Type\n";
      }
    };

    AnnotatedWriter Annotator(LPA);
    M.print(dbgs(), &Annotator);
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  DTransAnalysisInfo &DTInfo;
  const DataLayout &DL;
  GetTLIFnType GetTLI;

  // This helper class is used to track the types and aggregate elements to
  // which a local pointer value may refer. This information is created and
  // updated as needed.
  LocalPointerAnalyzer LPA;
  dtrans::DTransAllocAnalyzer &DTAA;
  dtrans::DTransBadCastingAnalyzer &DTBCA;
  function_ref<BlockFrequencyInfo &(Function &)> &GetBFI;
  // Set this in visitFunction before visiting instructions in the function.
  BlockFrequencyInfo *BFI;

  // Collect the dominator tree
  std::function<DominatorTree &(Function &)> GetDomTree;

  // A collection for safety data that needs to be set which may also require
  // cascading to pointer types that are reachable from a type. These are
  // deferred because it is not known whether the cascading is needed until it
  // is determined whether the pointers fields get used or not, which requires
  // deferring this safety data until the entire IR has been analyzed.
  //
  // The cascading algorithm needs to examine the members of the type being cast
  // from along with the type being cast to, so this data structure will store
  // the type being cast from as the first member of the tuple, the type being
  // cast to as the second member, along with the safety type.
  //
  // A SetVector is used here, instead of just a SmallSet so that traces get
  // emitted in a consistent order when the elements are processed while walking
  // the set.
  using DeferredInfo =
      std::tuple<llvm::Type *, llvm::Type *, dtrans::SafetyData>;
  SetVector<DeferredInfo, std::vector<DeferredInfo>, SmallSet<DeferredInfo, 16>>
      DeferredCastingSafetyCascades;
  // A map to store a unique FunctionType of a Function with an incomplete
  // prototype declaration. For example, getenv() could be declared as:
  //    char *getenv();
  // but then used in a single specific context as:
  //    char *getenv(char *);
  // So, in the UniqueFunctionTypeMap, getenv() would be mapped to:
  //    i8* (i8 *)*
  DenseMap<Function *, FunctionType *> UniqueFunctionTypeMap;

  // We need these types often enough that it's worth keeping them around.
  llvm::Type *Int8PtrTy;
  llvm::Type *PtrSizeIntTy;
  llvm::Type *PtrSizeIntPtrTy;

  // Helper function to identify if an instruction is in broken SSA
  // form but it is allowed to ignore it. Return true if the instruction
  // is in broken SSA form and is in a dead block.
  bool ignoreBrokenInstruction(Instruction *I) {
    if (!I)
      return false;

    Value *ValInst = cast<Value>(I);
    bool CheckTree = false;

    for (Value *OperandI : I->operands()) {
      // Broken SSA form found
      if (OperandI == ValInst) {
        CheckTree = true;
        break;
      }
    }

    if (!CheckTree)
      return false;

    BasicBlock *BB = I->getParent();
    Function *F = I->getFunction();
    DominatorTree &DT = GetDomTree(*F);
    // Check if the basic block is a dead block
    return !DT.isReachableFromEntry(BB);
  }

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
          auto *Call = cast<CallBase>(&Inst);
          Function *Fn = Call->getCalledFunction();
          if (Fn && Fn->isIntrinsic() &&
              Fn->getIntrinsicID() == Intrinsic::vastart) {
            Value *Arg = Call->getArgOperand(0);
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

    // This happens with opaque structures and zero-element arrays.
    if (!dtrans::dtransCompositeIndexValid(BaseTy, 0u))
      return false;

    auto *ElementZeroTy = dtrans::dtransCompositeGetTypeAtIndex(BaseTy, 0u);
    if (!ElementZeroTy->isArrayTy())
      return false;

    return ElementZeroTy->getArrayElementType() ==
           ElementTy->getPointerElementType();
  }

  // When a global variable is used in a ConstantExpr, the resulting value
  // appears directly as an operand to instructions and is never seen by
  // the instruction visitor. To handle these uses, we need to visit them
  // from the GlobalVariable's users list. Because ConstantExprs can be
  // chained, we call a helper function to do the analysis.
  void analyzeGlobalVariableUses(GlobalVariable &GV) {
    for (auto *U : GV.users())
      if (auto *CE = dyn_cast<ConstantExpr>(U))
        analyzeConstantExpr(CE);
  }

  // Determine the type of operation being performed by a ConstantExpr and
  // dispatch it appropriately. Currently only BitCast, GEP and PtrToInt are
  // handled. Other possibilities are marked as unhandled uses. This can be
  // updated as needed.
  void analyzeConstantExpr(ConstantExpr *CE) {
    if (auto *BOp = dyn_cast<BitCastOperator>(CE)) {
      visitBitCastOperator(BOp);
    } else if (auto *GEPOp = dyn_cast<GEPOperator>(CE)) {
      visitGetElementPtrOperator(GEPOp);
    } else if (auto *PtrToIntOp = dyn_cast<PtrToIntOperator>(CE)) {
      visitPtrToIntOperator(PtrToIntOp);
    } else {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Unhandled use -- constant expr\n  "
                        << *CE << "\n");
      (void)setValueTypeInfoSafetyDataBase(CE, dtrans::UnhandledUse);
      for (Value *Op : CE->operands())
        (void)setValueTypeInfoSafetyDataBase(Op, dtrans::UnhandledUse);
    }

    // All cases above fall through to continue following the chain of
    // ConstantExprs.
    for (auto *U : CE->users())
      if (auto *UCE = dyn_cast<ConstantExpr>(U))
        analyzeConstantExpr(UCE);
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
    if (llvm::dvanalysis::isDopeVectorType(Ty, DL)) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Dope vector:\n  " << *Ty << "\n");
      TI->setSafetyData(dtrans::DopeVector);
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
      if (ElementTy->isArrayTy() || ElementTy->isVectorTy()) {
        // Look through multiple layers of arrays or vectors if necessary.
        llvm::Type *NestedTy = ElementTy;
        while (NestedTy->isArrayTy() || NestedTy->isVectorTy()) {
          if (NestedTy->isArrayTy())
            NestedTy = cast<ArrayType>(NestedTy)->getElementType();
          else
            NestedTy = cast<VectorType>(NestedTy)->getElementType();
        }
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

  // Return true if the input BitCast doesn't have any user or if its only
  // use is a dead argument. Else,
  // return false. For example:
  //
  // define void @bar(%struct.test01b* %p2) {
  //   ret void
  // }
  //
  // define void @foo(%struct.test01a* %p) {
  //   %p2 = bitcast %struct.test01a* %p to %struct.test01b*
  //   call void @bar(%struct.test01b* %p2)
  //   ret void
  // }
  //
  // In the example above, %p2 in @foo is a BitCast from %struct.test01a* to
  // %struct.test01b* and it is only used as an argument in the callsite for
  // @bar. The formal argument of %p2 in @bar doesn't have any user, therefore
  // it will be a dead argument. This is safe to cast.
  bool isBitCastDead(BitCastOperator *BC) {
    if (!BC)
      return false;

    if (BC->user_empty())
      return true;

    for (User *User : BC->users()) {
      CallBase *CI = dyn_cast<CallBase>(User);
      if (!CI)
        return false;

      if (CI->isIndirectCall())
        return false;

      Function *F = CI->getCalledFunction();
      if (!F || F->isDeclaration() || F->isVarArg() || F->isIntrinsic())
        return false;

      for (unsigned I = 0, E = CI->arg_size(); I < E; I++) {
        if (BC == CI->getArgOperand(I)) {
          Argument *FormalArg = F->getArg(I);
          if (!FormalArg->user_empty())
            return false;
        }
      }
    }

    return true;
  }

  // Return true if the BitCast is used to access a field in a nested
  // structure. For example:
  //
  // %"class.outer" = type { %"class.inner" }
  // %"class.inner" = type { %"class.inner2" }
  // %"class.inner2" = type { %class.TestClass*, %class.TestClass* }
  // %class.TestClass = type { i64, i64, i64 }
  //
  // Consider the following BitCast and Load instructions, where %tmp
  // is a GEP or an Argument with type %"class.outer":
  //
  // %tmp2 = bitcast %"class.outer"* %tmp to i64*
  // %tmp3 = load i64, i64* %tmp2
  // %tmp4 = inttoptr i64 %tmp3 to %class.TestClass*
  //
  // The example above shows that the casting in %tmp2 will be used
  // for loading the innermost structure (%class.TestClass). This could be
  // collected by calling the local pointers information related to %tmp2
  // and check if there is a pointee with the same type as the destination.
  bool isBitCastUsedToAccessAnInnerStructure(BitCastOperator *BC,
                                             llvm::Type *DestinationType) {
    if (!BC || !DestinationType)
      return false;

    // Given a PointerType, traverse through the element types until
    // it reaches to the actual type that is not a poiner. For example:
    //
    //   Input  -> %"class.outer"**
    //   Output -> %"class.outer"
    auto GetPtrElementType = [](PointerType *PtrTy) -> Type* {
      if (!PtrTy)
        return nullptr;

      Type *CurrType = cast<Type>(PtrTy);
      while (CurrType && CurrType->isPointerTy()) {
        auto CurrPtr = cast<PointerType>(CurrType);
        CurrType = CurrPtr->getElementType();
      }

      return CurrType;
    };

    // Return true if SrcTy is a nested structure, and traversing the
    // 0 element it reaches to DstTy. This is a simplified version
    // of dtrans::isElementZeroAccess since the Type stored in
    // the pointees of interest is a StructType.
    auto IsFieldZeroAccess = [](Type *SrcTy, Type *DstTy) -> bool {
      if (!SrcTy || !DstTy)
        return false;

      if (!SrcTy->isStructTy())
        return false;

      Type *CurrType = SrcTy;
      while (CurrType) {
        if (CurrType == DstTy)
          return true;

        if (auto *StructTy = dyn_cast<StructType>(CurrType))
          CurrType = StructTy->getElementType(0);
        else
          break;
      }
      return false;
    };

    PointerType *BCSourceTy = dyn_cast<PointerType>(BC->getSrcTy());
    PointerType *BCDestTy = dyn_cast<PointerType>(BC->getDestTy());
    if (!BCSourceTy || !BCDestTy)
      return false;

    // If source is not a pointer to a structure or destination is not
    // a pointer to an integer then return
    if (!GetPtrElementType(BCSourceTy)->isStructTy() ||
        !BCDestTy->getElementType()->isIntegerTy())
      return false;

    uint64_t BCDestSize = DL.getTypeSizeInBits(BCDestTy).getFixedSize();
    uint64_t BCSourceSize = DL.getTypeSizeInBits(BCSourceTy).getFixedSize();
    uint64_t DestinationSize =
        DL.getTypeSizeInBits(DestinationType).getFixedSize();

    // Make sure that the size of the pointers match
    if (BCSourceSize != BCDestSize || BCSourceSize != DestinationSize ||
        BCDestSize != DestinationSize)
      return false;

    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(BC);
    if (!LPI.pointsToSomeElement())
      return false;

    // If the BitCast is used to access fields in a structure then the
    // LocalPointerInfo will provide the information of which fields could be
    // accessed. For example mentioned before, the bitcast and load
    // instructions are loading the inner structures. This means that there
    // will be one pointee with the following information:
    //
    //   Parent type: %"class.inner2"
    //   Field number: 0
    //
    // This basically says that the BitCast will be used to access field 0 in
    // %"class.inner2", which is type %class.TestClass* (DestinationType)
    for (auto &PointeePair : LPI.getElementPointeeSet()) {

      if (PointeePair.second.getKind() !=
          LocalPointerInfo::PointeeLoc::PLK_Field)
        return false;

      llvm::Type *ParentTy = PointeePair.first;
      size_t ElementNum = PointeePair.second.getElementNum();

      // Check that the pointee is a structure
      llvm::StructType *ParentStruct = dyn_cast<StructType>(ParentTy);
      if (!ParentStruct)
        return false;

      // NOTE: The pointee should not necessarily need to be element 0.
      // Consider the following example:
      //
      // %"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
      // %"struct.std::_Vector_base.13" =
      //     type { %"struct.std::_Vector_base<TestClass,
      //     std::allocator<TestClass>>::_Vector_impl.67" }
      // %"struct.std::_Vector_base<TestClass,
      //     std::allocator<TestClass>>::_Vector_impl.67" =
      //     type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
      // %class.TestClass = type {i64, i64, i64}
      //
      // %tmp = getelementptr inbounds %"class.std::vector.12",
      //             %"class.std::vector.12"* %arg, i64 0, i32 0, i32 0, i32 1
      // %tmp2 = bitcast %class.TestClass** %tmp to i64*
      // %tmp3 = load i64, i64* %tmp2
      // %tmp4 = inttoptr i64 %tmp3 to %class.TestClass*
      //
      // In the example above the local pointer info for the BitCast in %tmp2
      // will produce the following pointee information:
      //
      //   Parent type:  %"struct.std::_Vector_base<TestClass,
      //                  std::allocator<TestClass>>::_Vector_impl.67" =
      //                      type { %class.TestClass*, %class.TestClass*,
      //                      %class.TestClass* }
      //   Field number: 1
      //
      // This BitCast is OK too.
      if (ElementNum >= ParentStruct->getNumElements())
        return false;

      // Check if the type of the field matches with the destination
      llvm::Type *FieldTy = ParentStruct->getElementType(ElementNum);
      if (DestinationType == FieldTy)
        continue;

      // If not, then check if it is possible to reach the destination by
      // traversing the 0 elements in the parent structure
      if (IsFieldZeroAccess(FieldTy, DestinationType))
        continue;

      // The destination doesn't match any information related to the pointee
      return false;
    }

    // The pointees in the BitCast matches the destination
    return true;
  }

  // Return true if the Bitcast is used for store. This case is looking
  // for the following:
  //
  // %1 = getelementptr inbounds %struct.test, %struct.test* %VAR,
  //      i64 0, i32 1
  // %2 = bitcast %"inner.struct"* %1 to i64*
  //
  // %3 = getelementptr inbounds %struct.test, %struct.test* %VAR2,
  //      i64 0, i32 1
  // %4 = bitcast %"inner.struct"* %3 to i64*
  // %5 = load i64, i64* %4, align 8
  //
  // store i64 %5, i64* %2
  //
  // In the above example, both bitcast instructions (%2 and %4) have the
  // same source and destination types. Also, both point to GEP instructions
  // (%1 and %3 respectively) that access the same data type.
  bool castUsedForStore(BitCastOperator *I) {

    // Return true if the size of the source matches with the size of
    // the destination.
    auto IsValidSize = [this](Type *SrcTy, Type *DestTy) {

      if (!SrcTy || !DestTy)
        return false;

      if (!SrcTy->isPointerTy() || !DestTy->isPointerTy())
        return false;

      if (DestTy != PtrSizeIntPtrTy)
        return false;

      llvm::PointerType *SrcPtr = cast<PointerType>(SrcTy);

      // Check if the source is a pointer to a structure. This checks for the
      // the following example:
      //
      // %tmp = getelementptr inbounds %struct.test, %struct.test* %VAR,
      //       i64 0, i32 1
      // %tmp2 = bitcast %"inner.struct"* %tmp to i64*
      //
      // In this case we are looking if the casting goes from a pointer to a
      // structure to i64*.
      if (SrcPtr->getElementType()->isStructTy())
        return true;

      // Else, check if the source is an i8 pointer. We want to also cover
      // the case when the casting is done to a "new" allocated space.
      // For example:
      //
      // %tmp = invoke noalias nonnull dereferenceable(8512)
      //        i8* @_Znwm(i64 8512)
      //
      // %tmp2 = getelementptr inbounds i8, i8* %tmp, i64 8440
      // %tmp3 = bitcast i8* %tmp2 to i64*
      //
      // In this case, the casting is being done to a new allocated space and
      // its type is i8*.
      return (SrcPtr->getElementType()->isIntegerTy(8));
    };

    BitCastInst *BC = dyn_cast_or_null<BitCastInst>(I);
    if (!BC)
      return false;

    if (!BC->hasOneUse())
      return false;

    StoreInst *Store = dyn_cast<StoreInst>(BC->user_back());
    if (!Store)
      return false;

    LoadInst *StoredValue = dyn_cast<LoadInst>(Store->getValueOperand());
    if (!StoredValue)
      return false;

    BitCastInst *BCLoaded = dyn_cast<BitCastInst>(StoredValue->getOperand(0));
    if (!BCLoaded)
      return false;

    llvm::Type *BCDestTy = BC->getDestTy();
    llvm::Type *BCLoadedDestTy = BCLoaded->getDestTy();

    if (BCDestTy != BCLoadedDestTy)
      return false;

    if (!IsValidSize(BC->getSrcTy(), BC->getDestTy()) ||
        !IsValidSize(BCLoaded->getSrcTy(), BCLoaded->getDestTy()))
      return false;


    // In the previous example, we could get away with comparing the bitcats
    // and the GEP instructions, but it doesn't cover other the possible cases.
    // For example:
    //
    // %tmp = invoke noalias nonnull dereferenceable(8512) i8* @_Znwm(i64 8512)
    //
    // %tmp2 = getelementptr inbounds i8, i8* %tmp, i64 8440
    // %tmp3 = bitcast i8* %tmp2 to i64*
    //
    // %tmp4 = getelementptr inbounds %class.test, %class.test* %VAR,
    //         i64 0, i32 1
    // %tmp5 = bitcast %"class.inner"* %tmp4 to i64*
    // %tmp6 = load i64, i64* %tmp5
    // store i64 %tmp6, i64* %tmp3
    //
    // In the above case, assuming that the size of %class.test is 8512 and
    // %"class.inner" is nested in %class.test, there is no GEP, just storing
    // in a "new". For this case is better to collect the local pointers'
    // information and make sure the dominant aggregate types match.
    LocalPointerInfo &LPIStoredValue = LPA.getLocalPointerInfo(BCLoaded);
    LocalPointerInfo &LPIPointer = LPA.getLocalPointerInfo(BC);

    if (!LPIPointer.pointsToSomeElement() ||
        !LPIStoredValue.pointsToSomeElement())
      return false;

    if (!LPIPointer.getDominantAggregateTy() ||
        !LPIStoredValue.getDominantAggregateTy())
      return false;

    return LPIPointer.getDominantAggregateTy() ==
        LPIStoredValue.getDominantAggregateTy();
  }

  // Verify that a bitcast from \p SrcTy to \p DestTy would be safe. The
  // caller has analyzed the bitcast instruction to determine that these
  // types need to be considered. These may not be the actual types used
  // by the bitcast instruction, but the source operand will be known to
  // be an instance of SrcTy in some way.
  void verifyBitCastSafety(BitCastOperator *I, llvm::Type *SrcTy,
                           llvm::Type *DestTy) {
    // If the types are the same, it's a safe cast.
    if (SrcTy == DestTy)
      return;

    // If DestTy is a generic equivalent of SrcTy, it's a safe cast.
    if (isGenericPtrEquivalent(SrcTy, DestTy))
      return;

    // If this can be interpreted as an element-zero access of SrcTy
    // or as a cast from a ptr-to-ptr to a type to a ptr-to-ptr to
    // element zero of that type or it's accessing the vtable, it's a
    // safe cast.
    if (dtrans::isElementZeroAccess(SrcTy, DestTy) ||
        dtrans::isPtrToPtrToElementZeroAccess(SrcTy, DestTy) ||
        dtrans::isVTableAccess(SrcTy, DestTy) ||
        (isa<BitCastInst>(I) &&
        dtrans::isBitCastLoadingZeroElement(dyn_cast<BitCastInst>(I))))
      return;

    // If element zero is an i8* and we're casting the source value as a
    // pointer to a pointer, that is an element zero access but it isn't
    // reported as such because we need to recognize the i8*->(struct**)
    // bitcast as a potential problem for the destination type.
    if (DTInfo.isTypeOfInterest(DestTy) && SrcTy->isPointerTy() &&
        DestTy->isPointerTy() &&
        DestTy->getPointerElementType()->isPointerTy() &&
        dtrans::isElementZeroI8Ptr(SrcTy->getPointerElementType())) {
      // In this case we always want to avoid setting the element pointee
      // safety data, regardless of the state of DTransOutOfBoundsOK.
      if (DTBCA.isBadCastTypeAndFieldCandidate(SrcTy, 0) ||
          DTBCA.isPotentialBitCastOfAllocStore(I)) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting (pending) -- "
                          << "unsafe cast of i8* element to specific type:\n"
                          << "  " << *I << "\n");
        DTBCA.setSawBadCastBitCast(I);
        (void)setValueTypeInfoSafetyDataBase(I, dtrans::BadCastingPending);
      } else {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
                          << "unsafe cast of i8* element to specific type:\n"
                          << "  " << *I << "\n");
        (void)setValueTypeInfoSafetyDataBase(I, dtrans::BadCasting);
      }
      return;
    }

    // Check if the bitcast is used for moving data from a source to a
    // destination with the same type.
    if (castUsedForStore(I)) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting (related types) -- "
                      << "unsafe cast of aliased pointer:\n"
                      << "  " << *I << "\n");
      if (DTInfo.getDTransOutOfBoundsOK())
        setValueTypeInfoSafetyData(I->getOperand(0),
                                  dtrans::BadCastingForRelatedTypes);
      else
        (void)setValueTypeInfoSafetyDataBase(I->getOperand(0),
            dtrans::BadCastingForRelatedTypes);
      return;
    }

    // Otherwise, it's not safe.
    LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
                      << "unsafe cast of aliased pointer:\n"
                      << "  " << *I << "\n");
    DTBCA.noteUnsafeCastOfAliasedPtr(I);
    if (DTInfo.getDTransOutOfBoundsOK())
      setValueTypeInfoSafetyData(I->getOperand(0), dtrans::BadCasting);
    else
      (void)setValueTypeInfoSafetyDataBase(I->getOperand(0),
                                           dtrans::BadCasting);

    if (DTInfo.isTypeOfInterest(DestTy)) {
      if (DTInfo.getDTransOutOfBoundsOK())
        setValueTypeInfoSafetyData(I, dtrans::BadCasting);
      else
        (void)setValueTypeInfoSafetyDataBase(I, dtrans::BadCasting);
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

  void analyzeAllocationCall(CallBase *Call, dtrans::AllocKind Kind) {

    // The LocalPointerAnalyzer will visit bitcast users to determine the
    // type of memory being allocated. This must be done in the
    // LocalPointerAnalyzer class rather than here because it is possible
    // that we will visit the bitcast first and the LocalPointerAnalyzer
    // must be able to identify the connection with the allocation call
    // in that case also.
    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(Call);

    // The list of aliased types in the LPI will be the types to which
    // this pointer is cast that were not identified as element zero
    // accesses. In almost all cases there will only be one such type.

    // If the malloc wasn't cast to a type of interest, we're finished.
    LocalPointerInfo::PointerTypeAliasSetRef &AliasSet =
        LPI.getPointerTypeAliasSet();
    if (AliasSet.empty()) {
      return;
    }

    dtrans::AllocCallInfo *ACI = DTInfo.createAllocCallInfo(Call, Kind);
    populateCallInfoFromLPI(LPI, ACI);

    // If the value is cast to multiple types, mark them all as bad casting.
    bool WasCastToMultipleTypes = LPI.pointsToMultipleAggregateTypes();

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    if (DTransPrintAllocations && WasCastToMultipleTypes)
      dbgs() << "dtrans: Detected allocation cast to multiple types.\n";
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

    // We expect to only see one type, but we loop to keep the code general.
    for (auto *Ty : AliasSet) {
      if (!DTInfo.isTypeOfInterest(Ty))
        continue;

      if (Kind == dtrans::AK_New) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: C++ handling -- "
                          << "new/new[] function call:\n  " << *Call << "\n");
        setBaseTypeInfoSafetyData(Ty, dtrans::HasCppHandling);
      }

      // If there are casts to multiple types of interest, they all get
      // handled as bad casts.
      if (WasCastToMultipleTypes) {
        LLVM_DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
                          << "allocation cast to multiple types:\n  " << *Call
                          << "\n");
        setBaseTypeInfoSafetyData(Ty, dtrans::BadCasting);
      }

      // Check the size of the allocation to make sure it's a multiple of the
      // size of the type being allocated.
      verifyAllocationSize(Call, Kind, cast<PointerType>(Ty));

      // Add this to our type info list.
      (void)DTInfo.getOrCreateTypeInfo(Ty);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      if (DTransPrintAllocations) {
        dbgs() << "dtrans: Detected allocation cast to pointer type\n";
        dbgs() << "  " << *Call << "\n";
        dbgs() << "    Detected type: " << *(Ty->getPointerElementType())
               << "\n";
      }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

      if (Kind == dtrans::AK_Calloc) {
        auto *TI = DTInfo.getOrCreateTypeInfo(Ty->getPointerElementType());
        analyzeCallocSingleValue(TI);
      }
    }
  }

  /// Record the type of object being passed to a call to "free".
  /// This is useful for transformations that need to rewrite allocation and
  /// free calls.
  void analyzeFreeCall(CallBase *Call, dtrans::FreeKind FK) {
    LLVM_DEBUG(dbgs() << "dtrans: Analyzing free call.\n  " << *Call << "\n");
    dtrans::FreeCallInfo *FCI = DTInfo.createFreeCallInfo(Call, FK);

    // If it's a standard free call, we can check for the type of the first
    // argument for the potential pointer types. If it's a user free call, there
    // is currently no guarantee to know which argument contains the TypeInfo.
    if (FK == dtrans::FK_Free || FK == dtrans::FK_Delete) {
      unsigned PtrArgInd = -1U;
      const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
      getFreePtrArg(FK, Call, PtrArgInd, TLI);
      Value *Arg = Call->getArgOperand(PtrArgInd);

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

          LLVM_DEBUG(dbgs()
                     << "dtrans-safety: C++ handling -- "
                     << "delete/delete[] function call:\n  " << *Call << "\n");
          setBaseTypeInfoSafetyData(Ty, dtrans::HasCppHandling);
        }
    } else {
      FCI->setAnalyzed(false);
    }
  }

  // Return true if the \p Call is to a suitably identified alloc
  // function.
  bool isSafeStoreForSingleAllocFunction(CallBase *Call) {
    const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
    return dtrans::getAllocFnKind(Call, TLI) != dtrans::AK_NotAlloc ||
           DTAA.isMallocPostDom(Call);
  }

  // Returns true if the loaded value is stored to the same address from which
  // it was loaded and does not escape.
  bool identifyUnusedValue(LoadInst &LI) {
    return dtrans::isLoadedValueUnused(&LI, LI.getPointerOperand());
  }

  // Return true if the BitCast operation is casting to from an
  // outer structure to an *i64, and all the pointees in the
  // local pointer information are inner structures with element
  // 0, or the outer type itself.
  // For example:
  //
  //   %"class.outer" = type { %"class.inner1" }
  //   %"class.inner1" = type { %"class.inner2", %"class.inner2"}
  //   %"class.inner2" = type { %class.TestClass*, %class.TestClass*}
  //   %class.TestClass = type { i64, i64, i64}
  //
  //   %1 = getelementptr inbounds %"class.outer", %"class.outer"* %0,
  //            i64 0, i32 0
  //   %2 = bitcast %"class.inner1"* %1 to i64*
  //   %3 = load i64, i64* %2
  //
  // In the above example, %2 is casting from %"class.inner1" to *i64.
  // The local pointer information for %2 will look as follows:
  //
  //    LocalPointerInfo:
  //      Aliased types:
  //        %class.inner1*
  //        i64*
  //      Element pointees:
  //        %class.inner2 @ 0
  //        %class.outer @ 0
  //      DomTy: %class.inner1*
  //
  // The pointees in the local pointer information are "%class.outer @ 0"
  // and "%class.inner2 @ 0". The type for the first pointee is the
  // same as the source type in the bitcast (%"class.inner1"), and the
  // second pointee represents a zero element access(%"class.inner2").
  // This means that the bitcast points to the zero element in the
  // innermost structure.
  bool isCastingToZeroElement(BitCastOperator *BC) {

    if (!BC)
      return false;

    PointerType *SrcTy = dyn_cast<PointerType>(BC->getSrcTy());
    Type *DestTy = BC->getDestTy();

    // Make sure we are collecting the correct pointer sizes
    if (!SrcTy || !SrcTy->getElementType()->isStructTy() ||
        DestTy != PtrSizeIntPtrTy)
      return false;

    // Collect the local pointer information
    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(BC);
    Type *LPIDominatType = LPI.getDominantAggregateTy();

    if (!LPIDominatType || !LPI.pointsToSomeElement())
      return false;

    for (auto &PointeePair : LPI.getElementPointeeSet()) {
      llvm::Type *ParentTy = PointeePair.first;

      if (!dtrans::dtransIsCompositeType(ParentTy))
        return false;

      assert((!PointeePair.first->isStructTy() ||
              PointeePair.second.getKind() !=
                  LocalPointerInfo::PointeeLoc::PLK_Offset) &&
             "Unexpected use of invalid element");

      size_t ElementNum = PointeePair.second.getElementNum();
      llvm::Type *FieldTy = dtrans::dtransCompositeGetTypeAtIndex(ParentTy,
                                                                  ElementNum);

      // If the field is the same as the source type, then is OK
      if (FieldTy->getPointerTo() == SrcTy)
        continue;

      // If the element number is not 0, then return false
      if (ElementNum != 0)
        return false;

      // Check that all pointees that have 0 as element number are zero
      // element access of the source.
      if (dtrans::isElementZeroAccess(SrcTy, FieldTy->getPointerTo()))
        continue;

      if (dtrans::isPtrToPtrToElementZeroAccess(SrcTy, FieldTy->getPointerTo()))
        continue;
    }
    return true;
  }

  // Return true if the load instruction is loading a zero element.
  // For example:
  //
  //   %"class.outer" = type { %"class.inner1" }
  //   %"class.inner1" = type { %"class.inner2", %"class.inner2"}
  //   %"class.inner2" = type { %class.TestClass*, %class.TestClass*}
  //   %class.TestClass = type { i64, i64, i64}
  //
  //   %1 = getelementptr inbounds %"class.outer", %"class.outer"* %0,
  //            i64 0, i32 0
  //   %2 = bitcast %"class.inner1"* %1 to i64*
  //   %3 = load i64, i64* %2
  //
  // In the above example, %3 is a load instruction that is loading the
  // zero element from %"class.inner2". This function will collect the
  // BitCast in %2 and will check if the pointees in the local pointer
  // information points to the zero element in the inner structures.
  bool isLoadingZeroElement(LoadInst *Load) {
    if (!Load)
      return false;

    BitCastOperator *BC = dyn_cast<BitCastOperator>(Load->getOperand(0));
    if (!BC)
      return false;

    return isCastingToZeroElement(BC);
  }

  // Return true if the store instruction is used for storing into the
  // zero element. This case is looking for the following:
  //
  //   %"class.outer" = type { %"class.inner1" }
  //   %"class.inner1" = type { %"class.inner2", %"class.inner2"}
  //   %"class.inner2" = type { %class.TestClass*, %class.TestClass*}
  //   %class.TestClass = type { i64, i64, i64}
  //
  //   %1 = getelementptr inbounds %"class.outer", %"class.outer"* %VAR,
  //        i64 0, i32 0
  //   %2 = bitcast %"class.inner1"* %1 to i64*
  //
  //   %3 = getelementptr inbounds %"class.outer", %"class.outer"* %VAR2,
  //        i64 0, i32 0
  //   %4 = bitcast %"class.inner1"* %3 to i64*
  //   %5 = load i64, i64* %4, align 8
  //
  //   store i64 %5, i64* %2
  //
  // In the above example, both bitcast instructions (%2 and %4) have the
  // same source and destination types. Also, both point to GEP instructions
  // (%1 and %3 respectively) that access the same data type.
  bool isStoringZeroElement(StoreInst *Store) {
    if (!Store)
      return false;

    Value *ValueOperand = Store->getValueOperand();
    Value *PtrOperand = Store->getPointerOperand();

    BitCastOperator *BCPtr = dyn_cast<BitCastOperator>(PtrOperand);
    if (!BCPtr)
      return false;

    // In case the value operand is 0 or null, it means that the data is being
    // initialized or removed. For example:
    //
    //   %1 = getelementptr inbounds %"class.outer", %"class.outer"* %VAR,
    //        i64 0, i32 0
    //   %2 = bitcast %"class.inner1"* %1 to i64*
    //
    // store i64 0, i64* %2
    if (auto *Const = dyn_cast<ConstantInt>(ValueOperand)) {

      if (!Const->isZero() && !Const->isNullValue())
        return false;

      // If the Value operand is a 0 or null, then make sure that
      // this value is being stored into a zero element
      return isCastingToZeroElement(BCPtr);
    }

    // Else, check if we are loading and storing into the same type
    return castUsedForStore(BCPtr);
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

    // Analyze fields which are pointers to allocated arrays. Here, 'I' is
    // an instruction which either loads or stores the field which points
    // to the allocated array, and 'FI' is the field info that will store
    // the result.  Because the means for storing into the allocated array
    // fields are not exhaustively analyzed, for now, we always mark such
    // field values as 'incomplete'.
    std::function<void(dtrans::FieldInfo *FI, Instruction *I)>
        AnalyzeIndirectArrays = [&AnalyzeIndirectArrays](dtrans::FieldInfo *FI,
                                                         Instruction *I) {
      if (!I)
        return;
      for (User *U : I->users()) {
        auto BCI = dyn_cast<BitCastInst>(U);
        if (BCI) {
          AnalyzeIndirectArrays(FI, BCI);
          continue;
        }
        auto GEPI = dyn_cast<GetElementPtrInst>(U);
        if (!GEPI || GEPI->getPointerOperand() != I ||
            GEPI->getNumIndices() != 1)
          continue;
        for (User *V : GEPI->users()) {
          auto SI = dyn_cast<StoreInst>(V);
          if (!SI)
            continue;
          auto CI = dyn_cast<Constant>(SI->getValueOperand());
          if (CI)
            FI->processNewSingleIAValue(CI);
        }
      }
    };

    // FSV and FSAF checks for "ConstVal" that is being assigned to a field
    // corresponding to "FI".
    auto CheckWriteValue = [](Constant *ConstVal, dtrans::FieldInfo &FI,
                              size_t ElementNum,
                              dtrans::StructInfo *ParentStInfo) {
      if (FI.processNewSingleValue(ConstVal)) {
        DEBUG_WITH_TYPE(DTRANS_FSV, {
          dbgs() << "dtrans-fsv: " << *(ParentStInfo->getLLVMType()) << " ["
                 << ElementNum << "] New value: ";
          ConstVal->printAsOperand(dbgs());
          dbgs() << "\n";
        });
      }
      if (!isa<ConstantPointerNull>(ConstVal)) {
        DEBUG_WITH_TYPE(DTRANS_FSAF, {
          if (!FI.isBottomAllocFunction())
            dbgs() << "dtrans-fsaf: " << *(ParentStInfo->getLLVMType()) << " ["
                   << ElementNum << "] <BOTTOM>\n";
        });
        FI.setBottomAllocFunction();
      }
    };

    // Returns true if "WriteVal" is SelectInst and both true and false values
    // are constants.
    auto IsConstantResultSelectInst = [](Value *WriteVal) {
      auto *SI = dyn_cast<SelectInst>(WriteVal);
      if (!SI || !isa<Constant>(SI->getTrueValue()) ||
          !isa<Constant>(SI->getFalseValue()))
        return false;
      return true;
    };

    // Update LoadInfoMap and StoreInfoMap if the instruction I is accessing
    // a structure element.
    auto &PointeeSet = PtrInfo.getElementPointeeSet();
    if (PointeeSet.size() == 1 && PointeeSet.begin()->first->isStructTy()) {
      LocalPointerInfo::TypeAndPointeeLocPair Element = *PointeeSet.begin();
      assert(Element.second.getKind() !=
                 LocalPointerInfo::PointeeLoc::PLK_Offset &&
             "Unexpected use of invalid element");
      std::pair<llvm::Type *, size_t> Pointee(Element.first,
                                              Element.second.getElementNum());

      if (IsLoad)
        DTInfo.addLoadMapping(cast<LoadInst>(&I), Pointee);
      else
        DTInfo.addStoreMapping(cast<StoreInst>(&I), Pointee);
    }

    // Add I to MultiElemLoadStoreInfo if it is accessing more than one
    // struct elements.
    // TODO: Need to handle if PointeeSet has non-struct elements.
    if (PointeeSet.size() > 1 && PointeeSet.begin()->first->isStructTy())
      DTInfo.addMultiElemLoadStore(&I);

    bool LoadingOrStoringZeroElement =
        isLoadingZeroElement(dyn_cast<LoadInst>(&I)) ||
        isStoringZeroElement(dyn_cast<StoreInst>(&I));

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

      if (dtrans::dtransIsCompositeType(ParentTy)) {
        assert((!PointeePair.first->isStructTy() ||
                PointeePair.second.getKind() !=
                    LocalPointerInfo::PointeeLoc::PLK_Offset) &&
               "Unexpected use of invalid element");
        size_t ElementNum = PointeePair.second.getElementNum();
        llvm::Type *FieldTy = dtrans::dtransCompositeGetTypeAtIndex(ParentTy,
                                                                    ElementNum);

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
        //   (3) FieldTy is a pointer type that does not point to an aggregate
        //       type, and ValTy is a pointer to a pointer-sized integer.
        //   (4) ValTy matches the type of element zero of FieldTy
        //   (5) FieldTy is a pointer type and ValTy is a pointer to the type
        //       of element zero of the type pointed to by FieldTy
        //
        // Examples of the mismatched element access safety check:
        //   - Loading a field declared as an double as a i64 should trigger
        //     the safety check.
        //   - Loading a field declared as an i8* as a %struct.foo* should
        //     trigger the safety check.
        //   - However, loading a field declared as double* as an i64* should be
        //     treated as safe.
        //   - Trigger the safety check if a field is a pointer to a structure
        //     type, but used as an i64* (this may be more conservative than
        //     necessary).
        bool IsAggregateTy =
            FieldTy->isPointerTy() &&
            FieldTy->getPointerElementType()->isAggregateType();
        bool AggrTyPtrAsPtrSizedInt =
            IsAggregateTy && (ValTy == PtrSizeIntPtrTy);
        if ((FieldTy != ValTy) &&
            (((!FieldTy->isPointerTy() ||
               (ValTy != PtrSizeIntTy && ValTy != Int8PtrTy &&
                ValTy != PtrSizeIntPtrTy)) &&
              !dtrans::isElementZeroAccess(FieldTy->getPointerTo(),
                                           ValTy->getPointerTo()) &&
              !dtrans::isPtrToPtrToElementZeroAccess(FieldTy->getPointerTo(),
                                                     ValTy->getPointerTo())) ||
            AggrTyPtrAsPtrSizedInt)) {

          auto *TI = DTInfo.getOrCreateTypeInfo(ParentTy);
          if (LoadingOrStoringZeroElement) {
            LLVM_DEBUG(dbgs() << "dtrans-safety: Mismatched element access"
                              << " -- zero element access\n");
            LLVM_DEBUG(dbgs() << "  " << I << "\n");
            if (DTInfo.getDTransOutOfBoundsOK()) {
              // Assuming out of bound access, set safety issue for the entire
              // ParentTy.
              setBaseTypeInfoSafetyData(ParentTy,
                  dtrans::MismatchedElementAccessRelatedTypes);
            } else {
              // Set safety issue to the ParentTy type only.
              TI->setSafetyData(dtrans::MismatchedElementAccessRelatedTypes);
              setBaseTypeInfoSafetyData(FieldTy,
                  dtrans::MismatchedElementAccessRelatedTypes);
            }
          } else {
            LLVM_DEBUG(dbgs() << "dtrans-safety: Mismatched element access:\n");
            LLVM_DEBUG(dbgs() << "  " << I << "\n");
            if (DTInfo.getDTransOutOfBoundsOK()) {
              // Assuming out of bound access, set safety issue for the entire
              // ParentTy.
              setBaseTypeInfoSafetyData(ParentTy,
                                        dtrans::MismatchedElementAccess);
            } else {
              // Set safety issue to the ParentTy type only.
              TI->setSafetyData(dtrans::MismatchedElementAccess);
              setBaseTypeInfoSafetyData(FieldTy, dtrans::MismatchedElementAccess);
            }
          }
          if (auto *ParentStInfo = dyn_cast<dtrans::StructInfo>(TI)) {
            TypeSize ValSize = DL.getTypeSizeInBits(ValTy);
            TypeSize FieldSize = DL.getTypeSizeInBits(
                ParentStInfo->getField(ElementNum).getLLVMType());
            if (DTInfo.getDTransOutOfBoundsOK() || ValSize > FieldSize) {
              for (auto &FI : ParentStInfo->getFields())
                FI.setMismatchedElementAccess();
            } else {
              dtrans::FieldInfo &FI = ParentStInfo->getField(ElementNum);
              FI.setMismatchedElementAccess();
            }
          }
        }

        if (ParentTy->isStructTy()) {
          // If the element pointee was added as a result of a bitcast from the
          // structure type to the type of element zero, then the
          // EleemntZeroAliasSet will be non-empty.
          bool HasNonGEPAccess = !PtrInfo.getElementZeroAliasSet().empty();
          auto *ParentStInfo =
              cast<dtrans::StructInfo>(DTInfo.getOrCreateTypeInfo(ParentTy));
          assert((!PointeePair.first->isStructTy() ||
                  PointeePair.second.getKind() !=
                      LocalPointerInfo::PointeeLoc::PLK_Offset) &&
                 "Unexpected use of invalid element");
          size_t ElementNum = PointeePair.second.getElementNum();
          dtrans::FieldInfo &FI = ParentStInfo->getField(ElementNum);
          if (IsLoad) {
            FI.setRead(I);
            if (HasNonGEPAccess)
              FI.setNonGEPAccess();
            DTBCA.analyzeLoad(FI, I);
            if (!identifyUnusedValue(cast<LoadInst>(I)))
              FI.setValueUnused(false);
            accumulateFrequency(FI, I);
            AnalyzeIndirectArrays(&FI, &I);
          } else {
            if (auto *ConstVal = dyn_cast<llvm::Constant>(WriteVal)) {
              CheckWriteValue(ConstVal, FI, ElementNum, ParentStInfo);
            } else if (auto *Call = dyn_cast<CallBase>(WriteVal)) {
              DEBUG_WITH_TYPE(DTRANS_FSV, {
                if (!FI.isValueSetComplete())
                  dbgs() << "dtrans-fsv: " << *(ParentStInfo->getLLVMType())
                         << " [" << ElementNum << "] <INCOMPLETE>\n";
              });
              FI.setIncompleteValueSet();
              if (isSafeStoreForSingleAllocFunction(Call)) {
                Function *Callee = Call->getCalledFunction();
                if (FI.processNewSingleAllocFunction(Callee)) {
                  DEBUG_WITH_TYPE(DTRANS_FSAF, {
                    dbgs() << "dtrans-fsaf: " << *(ParentStInfo->getLLVMType())
                           << " [" << ElementNum << "] ";
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
                           << " [" << ElementNum << "] <BOTTOM>\n";
                });
                FI.setBottomAllocFunction();
              }
            } else if (IsConstantResultSelectInst(WriteVal)) {
              auto *SI = cast<SelectInst>(WriteVal);
              CheckWriteValue(cast<Constant>(SI->getTrueValue()), FI,
                              ElementNum, ParentStInfo);
              CheckWriteValue(cast<Constant>(SI->getFalseValue()), FI,
                              ElementNum, ParentStInfo);
            } else {
              DEBUG_WITH_TYPE(DTRANS_FSV, {
                if (!FI.isValueSetComplete())
                  dbgs() << "dtrans-fsv: " << *(ParentStInfo->getLLVMType())
                         << " [" << ElementNum << "] <INCOMPLETE>\n";
              });
              FI.setIncompleteValueSet();
              DEBUG_WITH_TYPE(DTRANS_FSAF, {
                if (!FI.isBottomAllocFunction())
                  dbgs() << "dtrans-fsaf: " << *(ParentStInfo->getLLVMType())
                         << " [" << ElementNum << "] <BOTTOM>\n";
              });
              FI.setBottomAllocFunction();
            }
            FI.setWritten(I);
            if (HasNonGEPAccess)
              FI.setNonGEPAccess();
            accumulateFrequency(FI, I);
            Value *V = cast<StoreInst>(&I)->getValueOperand();
            Instruction *II = dyn_cast<Instruction>(V);
            AnalyzeIndirectArrays(&FI, II);
            DTBCA.analyzeStore(FI, I);
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
      // If the incoming value is a result of a non-returning function,
      // then skip it.
      if (const auto *Call = dyn_cast<CallBase>(ValIn)) {
        const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
        if (dtrans::isDummyFuncWithUnreachable(Call, TLI))
          continue;
      }
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

  // Return true if the input size for a memory allocation is multiple
  // of the size's type. Else return false.
  bool isSizeMultipleOfAllocationType(CallBase *Call) {
    if (!Call)
      return false;

    // Return true if the allocation size is a subtraction whose result
    // is a multiple of the type's size. For example:
    //
    //   %class.OuterClass = type {%class.TestClass, %"class.std::vector.12"}
    //   %"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
    //   %"struct.std::_Vector_base.13" =
    //       type { %"struct.std::_Vector_base<OuterClass,
    //       std::allocator<OuterClass>>::_Vector_impl" }
    //   %"struct.std::_Vector_base<OuterClass,
    //       std::allocator<OuterClass>>::_Vector_impl" =
    //       type { %class.OuterClass*, %class.OuterClass*, %class.OuterClass* }
    //   %class.TestClass = type {i64, i64, i64}
    //
    //   %tmp = getelementptr inbounds %class.OuterClass,
    //          %class.OuterClass* %arg1, i64 0, i32 1
    //   %tmp1 = bitcast %"class.std::vector.12"* %tmp to i64*
    //   %tmp2 = load i64, i64* %tmp1
    //
    //   %tmp3 = getelementptr inbounds %class.OuterClass,
    //           %class.OuterClass* %arg1, i64 0, i32 1
    //   %tmp4 = getelementptr inbounds %"class.std::vector.12",
    //           %"class.std::vector.12"* %tmp3, i64 0, i32 0
    //   %tmp5 = getelementptr inbounds %"struct.std::_Vector_base.13",
    //           %"struct.std::_Vector_base.13"* %tmp4, i64 0, i32 0
    //   %tmp6 = getelementptr inbounds %"struct.std::_Vector_base<OuterClass,
    //           std::allocator<OuterClass>>::_Vector_impl",
    //           %"struct.std::_Vector_base<OuterClass,
    //           std::allocator<OuterClass>>::_Vector_impl"* %tmp5, i64 0, i32 1
    //   %tmp7 = bitcast %class.OuterClass** %tmp6 to i64*
    //   %tmp8 = load i64, i64* %tmp7
    //
    //   %tmp9 = sub i64 %tmp8, %tmp2
    //   %tmp10 = sdiv exact i64 %tmp9, 48
    //   %tmp11 = icmp eq i64 %tmp10, 0
    //   br i1 %tmp11, label %bbcheck, label %bbcont
    //
    //  bbcheck:
    //   %tmp12 = icmp ugt i64 %tmp10, 2167145685351216
    //   br i1 %tmp12, label %bbnew, label %unreachable
    //
    //  bbnew:
    //   %tmp13 = invoke noalias nonnull i8* @_Znwm(i64 %tmp9)
    //            to label %bb2 unwind label %lpad
    //
    //  bb2:
    //   %tmp14 = bitcast i8* %tmp13 to %class.OuterClass*
    //
    // The allocation site in %tmp13 uses %tmp9 as the allocation size, which is
    // a subtraction. The operands of %tmp9 are %tmp8 and %tmp2. The local
    // pointer information for both instructions has the same dominant
    // aggregate type (%class.OuterClass*). Also, the local pointer information
    // for %tmp13 has the same dominant aggregate type. This subtraction is
    // guarded with a division by the exact size of this dominant aggregate
    // type (%tmp10). This means that the input size in %tmp13 is a multiple of
    // the type that is being allocated.
    auto CheckSubtraction = [this, Call](BinaryOperator *Sub) -> bool {
      if (!Sub)
        return false;

      if(Sub->getOpcode() != Instruction::Sub)
        return false;

      if (!isValueOfInterest(Sub->getOperand(0)) &&
          !isValueOfInterest(Sub->getOperand(1)))
        return false;

      LocalPointerInfo &LeftLPI = LPA.getLocalPointerInfo(Sub->getOperand(0));
      LocalPointerInfo &RightLPI = LPA.getLocalPointerInfo(Sub->getOperand(1));

      // If we have a pointer to an element in the subtraction then don't
      // continue.
      if (LeftLPI.pointsToSomeElement() || RightLPI.pointsToSomeElement())
        return false;

      Type *LeftDomType = LeftLPI.getDominantAggregateTy();
      Type *RightDomType = RightLPI.getDominantAggregateTy();
      if (!LeftDomType || !LeftDomType->isPointerTy() ||
          !RightDomType || !RightDomType->isPointerTy() ||
          LeftDomType != RightDomType)
        return false;

      LocalPointerInfo &CallLPI = LPA.getLocalPointerInfo(Call);
      Type *CallDomType = CallLPI.getDominantAggregateTy();
      if (!CallDomType || CallDomType != LeftDomType)
        return false;

      llvm::Type *ElementTy = LeftDomType->getPointerElementType();
      if (ElementTy->isPointerTy())
        return false;

      uint64_t ElementSize = DL.getTypeAllocSize(ElementTy);
      return subUsedForAllocation(Sub, ElementSize);
    };

    Value *CallSize = nullptr;

    // This catches the case when the function for memory allocation
    // uses one argument
    if (Call->arg_size() == 1)
      CallSize = Call->getArgOperand(0);

    if (!CallSize)
      return false;

    // NOTE: For now we are going to catch a subtraction, but there are
    // other possible ways to get a multiple of the size. Also, the
    // "bad pointer manipulation (related type)" safety check will be set
    // when the subtract instruction is evaluated. This is needed to prevent
    // transformations that adjust the allocation size when "bad alloc size"
    // isn't set and the operation is not a multiplication.
    if (auto *BC = dyn_cast<BinaryOperator>(CallSize))
      return CheckSubtraction(BC);

    return false;
  }

  /// Given a call instruction that has been determined to be an allocation
  /// of the specified aggregate type, check the size arguments to verify
  /// that the allocation is a multiple of the type size.
  void verifyAllocationSize(CallBase *Call, dtrans::AllocKind Kind,
                            llvm::PointerType *Ty) {
    // The type may be a pointer to an aggregate or a pointer to a pointer.
    // In either case, it is the type that was used as a bitcast for the
    // return value of an allocation call. So if it is a pointer to a pointer
    // then the allocated buffer is a buffer that will container a pointer
    // or array of pointers. Therefore, we do not want to trace all the way
    // to the base type. If Ty is a pointer-to-pointer type then we can return
    // early because we can reason about the size of a pointer without finding
    // the size in the IR.
    if (Ty->getElementType()->isPointerTy())
      return;

    // The size returned by DL.getTypeAllocSize() includes padding, both
    // within the type and between successive elements of the same type
    // if multiple elements are being allocated.
    uint64_t ElementSize = DL.getTypeAllocSize(Ty->getElementType());
    unsigned AllocSizeInd = 0;
    unsigned AllocCountInd = 0;
    const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
    getAllocSizeArgs(Kind, Call, AllocSizeInd, AllocCountInd, TLI);

    auto *AllocSizeVal = Call->getArgOperand(AllocSizeInd);
    auto *AllocCountVal =
        AllocCountInd != -1U ? Call->getArgOperand(AllocCountInd) : nullptr;
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
    dtrans::TypeInfo *TI = DTInfo.getOrCreateTypeInfo(Ty->getElementType());
    bool EndsInZeroSizedArray = TI ? TI->hasZeroSizedArrayAsLastField() : false;
    if (!dtrans::isValueConstant(AllocSizeVal, &Res) &&
        dtrans::traceNonConstantValue(AllocSizeVal, ElementSize,
                                      EndsInZeroSizedArray)) {
      setBaseTypeInfoSafetyData(Ty, dtrans::ComplexAllocSize);
      return;
    }

    // Check if the allocation size is not constant, but we can prove that the
    // variable is a multiple of the type's size
    if (isSizeMultipleOfAllocationType(Call))
      return;

    // Otherwise, we must assume the size arguments are not acceptable.
    LLVM_DEBUG(dbgs() << "dtrans-safety: Bad alloc size:\n"
                      << "  " << *Call << "\n");
    setBaseTypeInfoSafetyData(Ty, dtrans::BadAllocSizeArg);
  }

  // Mark fields designated by \p Info pointer and the memory \p Size to have
  // multiple values. IsNullValue indicates that the null value is written.
  void markPointerWrittenWithMultipleValue(LocalPointerInfo &Info, Value *Size,
                                           bool IsNullValue = false) {
    StructType *STy = nullptr;
    size_t FieldNum = 0;
    uint64_t PrePadBytes = 0;

    // Identify the structure type that is pointed to by Info.
    if (!Info.pointsToSomeElement() ||
        !isSimpleStructureMember(Info, &STy, &FieldNum, &PrePadBytes)) {

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

    // If the pointee type is not a sized structure we have no interest in it.
    if (!STy || !STy->isSized())
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
    bool InBounds = true;
    uint64_t WriteBound = 0;
    if (FieldNum >= STy->getNumElements()) {
      InBounds = false;
    } else {
      WriteBound = SL->getElementOffset(FieldNum) + WriteSize;
      if (WriteBound > StructSize)
        InBounds = false;
    }

    if (!InBounds) {
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
        DEBUG_WITH_TYPE(DTRANS_FSAF, {
          if (!FInfo.isBottomAllocFunction())
            dbgs() << "dtrans-fsaf: " << *(SInfo->getLLVMType()) << " ["
                   << FieldNum << "] <BOTTOM>\n";
        });
        FInfo.setBottomAllocFunction();
        FInfo.setIncompleteValueSet();
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
      uint64_t PrePadBytes = 0;

      // Check if the pointer-to-member is a member of structure that can
      // be analyzed.
      if (isSimpleStructureMember(DstLPI, &StructTy, &FieldNum, &PrePadBytes)) {

        // Pass 'false' for IsValuePreservingWrite to conservatively mark all
        // fields as being multiple value from the memset.  We can improve this
        // analysis later by analyzing the value being set.
        dtrans::MemfuncRegion RegionDesc;

        if (analyzeMemfuncStructureMemberParam(
                I, StructTy, FieldNum, PrePadBytes, SetSize, RegionDesc))
          createMemsetCallInfo(I, StructTy, RegionDesc);

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

    // If DestParentTy was null, we should have either hit the overloaded
    // alias set early return, the (!DestParentTy && !pointsToSomeElement)
    // early return or the pointsToSomeElement early return above.
    assert(DestParentTy && "Unexpected null parent type!");

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
      createMemsetCallInfo(I, DestPointeeTy, RegionDesc);

      return;
    }

    // Consider the case where a portion of a structure is being set, starting
    // from the 1st field.
    if (auto *StructTy = dyn_cast<StructType>(DestPointeeTy)) {
      dtrans::MemfuncRegion RegionDesc;
      if (analyzeMemfuncStructureMemberParam(I, StructTy, /*FieldNum=*/0,
                                             /*PrePadBytes=*/0, SetSize,
                                             RegionDesc))
        createMemsetCallInfo(I, StructTy, RegionDesc);
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
    assert(I.arg_size() >= 2);

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

      // In order for control to reach this point,
      //   1. If DestParentTy is null, DstPtrToMember must be true.
      //   2. If SrcParentTy is null, SrcPtrToMember must be true.
      //   3. DestParentTy must equal SrcParentTy, so if either is null
      //      both must be null.
      // We can only get here if either DstPtrToMember or SrcPtrToMember
      // is false, so we can assert that neither ParentTy pointer is null.
      assert(DestParentTy && SrcParentTy &&
             "Broken assumptions in analyzeMemcpyOrMemmove!");

      // Conservatively set destination pointer to unknown value.
      markPointerWrittenWithMultipleValue(DstLPI, SetSize);

      // Do not set safety issue if the memfunc call affects only one field.
      StructType *OuterStructType;
      size_t FieldNum;
      uint64_t PrePadBytes;
      uint64_t AccessSize;
      bool IsConstantSize = dtrans::isValueConstant(SetSize, &AccessSize);
      bool IsSimple =
          isSimpleStructureMember(DstPtrToMember ? DstLPI : SrcLPI,
                                  &OuterStructType, &FieldNum, &PrePadBytes);

      // Note: Currently, PrePadBytes should always be 0 if we get here, because
      // currently the local pointer info should only be capturing an access
      // betweeen fields for the memset case, but we will check it, just in
      // case.
      if (IsConstantSize && IsSimple && PrePadBytes == 0) {
        llvm::Type *ElemTy = OuterStructType->getElementType(FieldNum);
        if (DL.getTypeStoreSize(ElemTy) == AccessSize) {
          // The read/write is a single field within the structure. If that
          // element is an aggregate type, collect the info in a MemCallInfo
          // object. The MemCallInfo object is used by the transformations to
          // adjust the size parameter of the call for the type being copied.
          // we need to track that a copy is being made of the inner structure
          // type to be able to adjust the size if a field gets deleted from it.
          //
          // For example:
          //   %struct.test01a = type { i32, i32, i32, i32, i32 }
          //   %struct.test01b = type { i32, %struct.test01a }
          //
          // A pointer of type %struct.test01a may be copied to/from the field
          // member of %struct.test01b. If we reach this point, we know that one
          // of the pointers is the element pointee, and the other is an alias
          // of %struct.test01a. Set up the MemCallInfo object to record that
          // the type of the field is being completely copied.
          dtrans::MemfuncRegion RegionDesc;
          RegionDesc.IsCompleteAggregate = true;
          createMemcpyOrMemmoveCallInfo(I, ElemTy, Kind, RegionDesc,
                                        RegionDesc);
          auto *ElemInfo = DTInfo.getOrCreateTypeInfo(ElemTy);
          markAllFieldsWritten(ElemInfo, I);
          return;
        }
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
      uint64_t DstPrePadBytes = 0;
      uint64_t SrcPrePadBytes = 0;

      // Check if the pointer-to-member is a member of structure that can
      // be analyzed.
      bool DstSimple = isSimpleStructureMember(DstLPI, &DstStructTy,
                                               &DstFieldNum, &DstPrePadBytes);

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

      bool SrcSimple = isSimpleStructureMember(SrcLPI, &SrcStructTy,
                                               &SrcFieldNum, &SrcPrePadBytes);
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
      if (DstStructTy == SrcStructTy && DstFieldNum == SrcFieldNum &&
          DstPrePadBytes == SrcPrePadBytes) {
        // The structures for the source and destination match, so we only need
        // to populate a RegionDesc structure for the destination.
        dtrans::MemfuncRegion RegionDesc;
        if (analyzeMemfuncStructureMemberParam(I, DstStructTy, DstFieldNum,
                                               DstPrePadBytes, SetSize,
                                               RegionDesc)) {
          createMemcpyOrMemmoveCallInfo(I, DstStructTy, Kind, RegionDesc,
                                        RegionDesc);

          auto *SrcParentTI = DTInfo.getOrCreateTypeInfo(SrcStructTy);
          auto *SrcParentStructTI = cast<dtrans::StructInfo>(SrcParentTI);
          addFieldReaders(SrcParentStructTI, RegionDesc.FirstField,
                          RegionDesc.LastField, I);
        } else {
          markPointerWrittenWithMultipleValue(DstLPI, SetSize);

          // In this case, the structure would have been set as BadMemFuncSize,
          // so we do not need to set the readers. All fields will be set to
          // BOTTOM for ModRef info during analysis.
        }
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

    // Far above we have checks that would have marked a safety condition and
    // returned if DestParentTy is null and DestPtrToMember is false. The block
    // immediately above returns in all cases where DestPtrToMember is true,
    // so at this point we can be sure that DestParentTy is not null.
    assert(DestParentTy && "Unexpected null DestParentTy");

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
      auto *SrcPointeeTy = DestParentTy->getPointerElementType();
      auto *SrcParentTI = DTInfo.getOrCreateTypeInfo(SrcPointeeTy);

      markAllFieldsWritten(ParentTI, I);

      if (auto *SrcParentStructTI = dyn_cast<dtrans::StructInfo>(SrcParentTI)) {
        // For memcpy/memmove, we do not mark the fields as read in order to
        // allow for field deletion to identify the field as potentially
        // unneeded. However, for analyzing ModRef information of the field, we
        // need to collect the function as a reader.
        unsigned LastFieldNum = SrcParentStructTI->getNumFields() - 1;
        addFieldReaders(SrcParentStructTI, 0, LastFieldNum, I);
      }
      // The copy/move is the complete aggregate of the source and destination,
      // which are the same types/
      dtrans::MemfuncRegion RegionDesc;
      RegionDesc.IsCompleteAggregate = true;
      createMemcpyOrMemmoveCallInfo(I, DestPointeeTy, Kind, RegionDesc,
                                    RegionDesc);
      return;
    }

    // Consider the case where a portion of a structure is being set, starting
    // from the 1st field.
    if (auto *StructTy = dyn_cast<StructType>(DestPointeeTy)) {
      dtrans::MemfuncRegion RegionDesc;
      llvm::Type *SrcPointeeTy = SrcParentTy->getPointerElementType();
      auto *SrcParentTI = DTInfo.getOrCreateTypeInfo(SrcPointeeTy);
      auto SrcParentStructTI = cast<dtrans::StructInfo>(SrcParentTI);

      if (analyzeMemfuncStructureMemberParam(I, StructTy, /*FieldNum=*/0,
                                             /*PrePadBytes=*/0, SetSize,
                                             RegionDesc)) {
        createMemcpyOrMemmoveCallInfo(I, StructTy, Kind, RegionDesc,
                                      RegionDesc);

        addFieldReaders(SrcParentStructTI, RegionDesc.FirstField,
                        RegionDesc.LastField, I);
      } else {
        markPointerWrittenWithMultipleValue(DstLPI, SetSize);

        // In this case, the structure would have been set as BadMemFuncSize,
        // so we do not need to set the readers. All fields will be set to
        // BOTTOM for ModRef info during analysis.
      }

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
      CI->setAliasesToAggregateType(true);
      LocalPointerInfo::PointerTypeAliasSetRef &AliasSet =
          LPI.getPointerTypeAliasSet();
      for (auto *Ty : AliasSet)
        if (DTInfo.isTypeOfInterest(Ty)) {
          assert(Ty->isPointerTy() && "Expected pointer type");
          CI->addElemType(Ty->getPointerElementType());
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
  void createMemsetCallInfo(Instruction &I, llvm::Type *ElemTy,
                            dtrans::MemfuncRegion &RegionDesc) {
    dtrans::MemfuncCallInfo *MCI = DTInfo.createMemfuncCallInfo(
        &I, dtrans::MemfuncCallInfo::MK_Memset, RegionDesc);
    MCI->setAliasesToAggregateType(true);
    MCI->setAnalyzed(true);
    MCI->addElemType(ElemTy);

    if (!RegionDesc.IsCompleteAggregate)
      markFieldsComplexUse(ElemTy, RegionDesc.FirstField, RegionDesc.LastField);
  }

  // Create a MemfuncCallInfo object that will store the details about a safe
  // memcpy/memmove call.
  void createMemcpyOrMemmoveCallInfo(Instruction &I, llvm::Type *ElemTy,
                                     dtrans::MemfuncCallInfo::MemfuncKind Kind,
                                     dtrans::MemfuncRegion &RegionDescDest,
                                     dtrans::MemfuncRegion &RegionDescSrc) {
    dtrans::MemfuncCallInfo *MCI =
        DTInfo.createMemfuncCallInfo(&I, Kind, RegionDescDest, RegionDescSrc);
    MCI->setAliasesToAggregateType(true);
    MCI->setAnalyzed(true);
    MCI->addElemType(ElemTy);

    if (!RegionDescDest.IsCompleteAggregate)
      markFieldsComplexUse(ElemTy, RegionDescDest.FirstField,
                           RegionDescDest.LastField);
  }

  // Helper function for retrieving information when the \p LPI argument refers
  // to a pointer-to-member element. This function checks that the
  // pointer to member is a referencing a single member from a single structure.
  // If so, it returns 'true'. Otherwise, return 'false'.
  // When returning 'true', the following output parameters are set:
  // \p StructTy    - The structure type in the \p StructTy.
  // \p FieldNum    - Field number of the first complete field that may be
  //                  accessed.
  // \p PrePadBytes - Number of padding bytes prior to \p FieldNum
  //                  accessed.
  bool isSimpleStructureMember(LocalPointerInfo &LPI, StructType **StructTy,
                               size_t *FieldNum, uint64_t *PrePadBytes) {
    assert(LPI.pointsToSomeElement());

    *StructTy = nullptr;
    *FieldNum = SIZE_MAX;
    *PrePadBytes = 0;

    if (!LPI.pointsToSomeElement())
      return false;

    auto &ElementPointees = LPI.getElementPointeeSet();
    if (ElementPointees.size() != 1)
      return false;

    auto &PointeePair = *(ElementPointees.begin());
    // Check whether the address is to a location that is not the start of a
    // field.
    if (PointeePair.second.getKind() ==
        LocalPointerInfo::PointeeLoc::PLK_Offset) {
      Type *Ty = PointeePair.first;
      if (auto *StTy = dyn_cast<llvm::StructType>(Ty)) {
        auto *SL = DL.getStructLayout(StTy);
        uint64_t AccessOffset = PointeePair.second.getByteOffset();
        if (AccessOffset < SL->getSizeInBytes()) {
          uint64_t Elem = SL->getElementContainingOffset(AccessOffset);
          uint64_t FieldStart = SL->getElementOffset(Elem);

          // getElementContainingOffset returns the field member prior to any
          // pad bytes if the offset falls into a padding region.
          if (AccessOffset > FieldStart) {
            // Make sure the offset is beyond the end of the prior field member.
            uint64_t FieldSize =
                DL.getTypeStoreSize(StTy->getElementType(Elem));
            if (AccessOffset < FieldStart + FieldSize)
              return false;

            // Advance to the next field, if possible.
            ++Elem;
            if (Elem == StTy->getNumElements())
              return false;

            FieldStart = SL->getElementOffset(Elem);
          }
          *StructTy = StTy;
          *FieldNum = Elem;
          *PrePadBytes = FieldStart - AccessOffset;
          return true;
        }
      }
      return false;
    }

    // If the element is the first element of the structure, it is necessary
    // to check for the possibility that it is a structure that started with
    // a character array to find the underlying structure type.
    // For example:
    //   %struct.test08 = type { [200 x i8], [200 x i8], i64 }
    //   call void @llvm.memset.p0i8.i64(i8* getelementptr(
    //        %struct.test08, %struct.test08* @test08var, i64 0, i32 0, i64 0),
    //       i8 0, i64 200, i32 4, i1 false)
    size_t ElementNum = PointeePair.second.getElementNum();
    if (ElementNum == 0) {
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

    // It's not the special case, so just get the type and field number if
    // it's a struct.
    Type *Ty = PointeePair.first;
    if (Ty->isStructTy()) {
      *StructTy = cast<StructType>(Ty);
      *FieldNum = ElementNum;
      return true;
    }

    return false;
  }

  // Analyze a structure pointer that is passed to memfunc call, possibly using
  // a pointer to one of the fields within the structure to determine which
  // fields are modified, and whether it is a safe usage. Return 'true' if safe
  // usage, and populate the \p RegionDesc with the results.
  bool analyzeMemfuncStructureMemberParam(Instruction &I, StructType *StructTy,
                                          size_t FieldNum, uint64_t PrePadBytes,
                                          Value *SetSize,
                                          dtrans::MemfuncRegion &RegionDesc) {
    auto *ParentTI = DTInfo.getOrCreateTypeInfo(StructTy);

    // Try to determine if a set of fields in a structure is being written.
    if (dtrans::analyzePartialStructUse(DL, StructTy, FieldNum, PrePadBytes,
                                        SetSize, &RegionDesc)) {
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
    } else if (FieldNum == 0 && PrePadBytes == 0 &&
               dtrans::analyzePartialAccessNestedStructures(DL, StructTy,
                                                            SetSize)) {
      LLVM_DEBUG(dbgs() << "dtrans-safety: Memfunc partial write "
                        << "(nested structure) -- size covers a set of fields"
                        << " in the inner structures:\n  " << I << "\n");

      setBaseTypeInfoSafetyData(StructTy,
                                dtrans::MemFuncNestedStructsPartialWrite);
      return false;
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
        FI.setWritten(I);
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
      FI.setWritten(I);
      accumulateFrequency(FI, I);
      auto *ComponentTI = DTInfo.getTypeInfo(FI.getLLVMType());
      markAllFieldsWritten(ComponentTI, I);
    }

    return;
  }

  // Update the fields of the structure \p StInfo from \p FirstField to \p
  // LastField to indicate that a read occurs via a memset, memcpy, or memmove
  // instruction by the function containing Instruction \p I.
  void addFieldReaders(dtrans::StructInfo *StInfo, unsigned int FirstField,
                       unsigned int LastField, Instruction &I) {
    assert(LastField >= FirstField && LastField < StInfo->getNumFields() &&
           "addFieldReaders with invalid field index");

    Function *F = I.getFunction();
    for (unsigned int Idx = FirstField; Idx <= LastField; ++Idx) {
      auto &FI = StInfo->getField(Idx);
      FI.addReader(F);

      dtrans::TypeInfo *FieldInfo = DTInfo.getTypeInfo(FI.getLLVMType());
      if (auto *FieldStInfo = dyn_cast<dtrans::StructInfo>(FieldInfo))
        addFieldReaders(FieldStInfo, 0, FieldStInfo->getNumFields() - 1, I);
    }
  }

  //
  // Update the "single value" info for GVElemTy, given that it has the
  // indicated Init. Also, update the safety checks, if needed.
  //
  void analyzeGlobalStructSingleValue(llvm::Type *GVElemTy,
                                      llvm::Constant *Init) {
    if (auto *StTy = dyn_cast<StructType>(GVElemTy)) {
      auto *StInfo = cast<dtrans::StructInfo>(DTInfo.getOrCreateTypeInfo(StTy));
      for (unsigned I = 0, E = StInfo->getNumFields(); I != E; ++I) {
        llvm::Type *FieldTy = StTy->getTypeAtIndex(I);
        llvm::Constant *ConstVal = Init ? Init->getAggregateElement(I) :
                                   nullptr;
        visitBitCastInInitializer(ConstVal);
        dtrans::FieldInfo &FI = StInfo->getField(I);
        analyzeGlobalStructSingleValue(FieldTy, ConstVal);
        DEBUG_WITH_TYPE(DTRANS_FSV,
                        dbgs() << "dtrans-fsv: " << *(StInfo->getLLVMType())
                               << " [" << I << "] ");
        if (ConstVal && (ConstVal->getType() == FieldTy)) {
          DEBUG_WITH_TYPE(DTRANS_FSV, ConstVal->printAsOperand(dbgs()));
          FI.processNewSingleValue(ConstVal);
          DEBUG_WITH_TYPE(
              DTRANS_FSV,
              dbgs() << (FI.isMultipleValue() ? " <MULTIPLE>\n" : "\n"));
        } else {
          DEBUG_WITH_TYPE(DTRANS_FSV, dbgs() << "<INCOMPLETE>\n");
          FI.setIncompleteValueSet();
        }
        if (!isa<ConstantPointerNull>(ConstVal)) {
          DEBUG_WITH_TYPE(DTRANS_FSAF, {
            if (!FI.isBottomAllocFunction())
              dbgs() << "dtrans-fsaf: " << *(StInfo->getLLVMType()) << " ["
                     << I << "] <BOTTOM>\n";
          });
          FI.setBottomAllocFunction();
        }
      }
    } else if (auto *ArTy = dyn_cast<ArrayType>(GVElemTy)) {
      auto *ArInfo = cast<dtrans::ArrayInfo>(DTInfo.getOrCreateTypeInfo(ArTy));
      auto *ComponentTI = ArInfo->getElementDTransInfo();
      llvm::Type *ComponentTy = ComponentTI->getLLVMType();
      for (unsigned I = 0, E = ArInfo->getNumElements(); I != E; ++I) {
        llvm::Constant *ConstVal = Init ? Init->getAggregateElement(I) :
                                   nullptr;
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
                               << " [" << Count << "] <INCOMPLETE>\n");
        if (IsNullValue)
          FI.processNewSingleValue(Constant::getNullValue(FI.getLLVMType()));
        else {
          DEBUG_WITH_TYPE(DTRANS_FSAF, {
            if (!FI.isBottomAllocFunction())
              dbgs() << "dtrans-fsaf: " << *(StInfo->getLLVMType()) << " ["
                     << Count << "] <BOTTOM>\n";
          });
          FI.setBottomAllocFunction();
          FI.setIncompleteValueSet();
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

    Value *LeftOp = I.getOperand(0);
    Value *RightOp = I.getOperand(1);
    // If neither operand is of interest, we can ignore this instruction.
    if (!isValueOfInterest(LeftOp) && !isValueOfInterest(RightOp))
      return;

    // Check if the RHS can be found as part of a subtraction chain to support
    // some simple cases where reassociation was done on the subtract chain
    // when a constant integer is involved.
    // For example:
    //   %tmp = sub i64 %t1, 8
    //   %offset = sub i64 %tmp, %t2
    //
    // TODO: This could be extended to also support:
    //   %tmp = sub i64 %t2, 8
    //   %offset = sub i64 %t1, %tmp
    //
    // Note: This should only be allowed for ptr-to-ptr types because the
    // transformations do not support modifying a constant operand that is
    // part of the subtraction chain. Those cases will be marked as
    // "Unhandled use" because the subtract does not feed a divide
    // operation.
    LocalPointerInfo &LHSLPI = LPA.getLocalPointerInfo(LeftOp);
    if (!isValueOfInterest(RightOp) && isa<ConstantInt>(I.getOperand(1)) &&
        LHSLPI.isPtrToPtr() && I.hasOneUse())
      if (auto *U = dyn_cast<BinaryOperator>(*I.user_begin()))
        if (U->getOpcode() == Instruction::Sub && U->getOperand(0) == &I)
          RightOp = U->getOperand(1);

    LocalPointerInfo &RHSLPI = LPA.getLocalPointerInfo(RightOp);
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
      setValueTypeInfoSafetyData(LeftOp, dtrans::UnhandledUse);
      setValueTypeInfoSafetyData(RightOp, dtrans::UnhandledUse);
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
        if (subUsedForAllocation(&I, ElementSize)) {
          LLVM_DEBUG(dbgs() << "dtrans-safety: Bad pointer manipulation "
                            << "(related types) -- "
                            << "Pointer subtraction result used to allocate "
                            << "a new space:\n"
                            << "  " << I << "\n");
          setAllAliasedTypeSafetyData(LHSLPI,
              dtrans::BadPtrManipulationForRelatedTypes);
        }
        else if (hasNonDivBySizeUses(&I, ElementSize)) {
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

  // Return true if the binary operator is a division between the input
  // Value and the input Size. The parameter IsExact is used to store if
  // the operation is marked as "exact".
  bool isValidDivision(BinaryOperator *BinOp, Value *V,
                       uint64_t Size, bool *IsExact) {
    *IsExact = false;
    if (!BinOp || !V)
      return false;

    if (BinOp->getOpcode() != Instruction::SDiv &&
        BinOp->getOpcode() != Instruction::UDiv)
      return false;

    if (BinOp->getOperand(0) != V)
      return false;

    if (auto *Inst = dyn_cast<Instruction>(BinOp))
      *IsExact = Inst->isExact();

    return dtrans::isValueMultipleOfSize(BinOp->getOperand(1), Size);
  }

  // Return true if the input Call is a call to "new" and the type of the
  // values used to compute the size matches with the result type of "new".
  bool isValidCallToNew(CallBase *Call, Value *V) {

    // Return V cast as an Instruction if V is a Load instruction
    // or V is a PtrToInt instruction, else return nullptr
    auto GetOperandInstruction = [](Value *V) -> Instruction * {
      if (isa<LoadInst>(V) || isa<PtrToIntInst>(V))
        return cast<Instruction>(V);
      return nullptr;
    };

    // Return the dominant aggregate type for the input Value
    auto CollectDominantAggregateType = [this, GetOperandInstruction]
        (Value *V) -> Type * {
      // If V is a binary operator then the dominant aggregate type
      // of the operands must match
      if (auto *BinOp = dyn_cast<BinaryOperator>(V)) {

        Instruction *InstA = GetOperandInstruction(BinOp->getOperand(0));
        Instruction *InstB = GetOperandInstruction(BinOp->getOperand(1));
        if (!InstA || !InstB)
          return nullptr;

        LocalPointerInfo &LPIInstA = LPA.getLocalPointerInfo(InstA);
        LocalPointerInfo &LPIInstB = LPA.getLocalPointerInfo(InstB);

        Type *InstADominantType = LPIInstA.getDominantAggregateTy();
        Type *InstBDominantType = LPIInstB.getDominantAggregateTy();

        if (!InstADominantType || !InstBDominantType)
          return nullptr;

        if (InstADominantType != InstBDominantType)
          return nullptr;

        return InstADominantType;
      }

      return nullptr;
    };

    if (!Call || !V || Call->isIndirectCall())
      return false;

    Value *Arg  = nullptr;
    Function *CalledFunction = Call->getCalledFunction();
    LibFunc TheLibFunc;
    const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
    if (TLI.getLibFunc(CalledFunction->getName(), TheLibFunc) &&
        TLI.has(TheLibFunc)) {

      // NOTE: We might want to add other forms of "new" and "alloc"
      switch (TheLibFunc) {
        case LibFunc_Znwm:
          Arg = Call->getArgOperand(0);
          break;
        default:
          return false;
      }
    }

    if (!Arg || Arg != V)
      return false;

    // Now we are going to check that the types for the allocated space
    // matches the types used for the subtraction
    LocalPointerInfo &LPICall = LPA.getLocalPointerInfo(Call);
    Type *CallDominantType = LPICall.getDominantAggregateTy();
    if (!CallDominantType)
      return false;

    Type *ValueDominantAggregateType = CollectDominantAggregateType(V);
    if (!ValueDominantAggregateType)
      return false;

    return ValueDominantAggregateType == CallDominantType;
  }

  // Return true if the input Value is used to allocate a memory space,
  // and we know that the allocated size is a multiple of the input Size.
  // For example:
  //
  //   %class.OuterClass = type {%class.TestClass, %"class.std::vector.12"}
  //   %"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
  //   %"struct.std::_Vector_base.13" =
  //       type { %"struct.std::_Vector_base<OuterClass,
  //       std::allocator<OuterClass>>::_Vector_impl" }
  //   %"struct.std::_Vector_base<OuterClass,
  //       std::allocator<OuterClass>>::_Vector_impl" =
  //       type { %class.OuterClass*, %class.OuterClass*, %class.OuterClass* }
  //   %class.TestClass = type {i64, i64, i64}
  //
  //   %tmp = getelementptr inbounds %class.OuterClass,
  //          %class.OuterClass* %arg1, i64 0, i32 1
  //   %tmp1 = bitcast %"class.std::vector.12"* %tmp to i64*
  //   %tmp2 = load i64, i64* %tmp1
  //
  //   %tmp3 = getelementptr inbounds %class.OuterClass,
  //           %class.OuterClass* %arg1, i64 0, i32 1
  //   %tmp4 = getelementptr inbounds %"class.std::vector.12",
  //           %"class.std::vector.12"* %tmp3, i64 0, i32 0
  //   %tmp5 = getelementptr inbounds %"struct.std::_Vector_base.13",
  //           %"struct.std::_Vector_base.13"* %tmp4, i64 0, i32 0
  //   %tmp6 = getelementptr inbounds %"struct.std::_Vector_base<OuterClass,
  //           std::allocator<OuterClass>>::_Vector_impl",
  //           %"struct.std::_Vector_base<OuterClass,
  //           std::allocator<OuterClass>>::_Vector_impl"* %tmp5, i64 0, i32 1
  //   %tmp7 = bitcast %class.OuterClass** %tmp6 to i64*
  //   %tmp8 = load i64, i64* %tmp7
  //
  //   %tmp9 = sub i64 %tmp8, %tmp2
  //   %tmp10 = sdiv exact i64 %tmp9, 48
  //   %tmp11 = icmp eq i64 %tmp10, 0
  //   br i1 %tmp11, label %bbcheck, label %bbcont
  //
  //  bbcheck:
  //   %tmp12 = icmp ugt i64 %tmp10, 2167145685351216
  //   br i1 %tmp12, label %bbnew, label %unreachable
  //
  //  bbnew:
  //   %tmp13 = invoke noalias nonnull i8* @_Znwm(i64 %tmp9)
  //            to label %bb2 unwind label %lpad
  //
  //  bb2:
  //   %tmp14 = bitcast i8* %tmp13 to %class.OuterClass*
  //
  // In this example, the subtraction in %tmp9 is used as an argument for the
  // call to "new" (@_Znwm) in %tmp13. From the division in %tmp10 we know that
  // the result of the subtraction is a multiple of %class.OuterClass size.
  // Also, using the local pointer information for the operands of the
  // subtraction (%tmp8 and %tmp2), we can compare with the result type
  // of the "new" (%tmp13). This is how we can ensure that the size of
  // the new allocated space is matches with the destination of bitcast.
  bool subUsedForAllocation(Value *V, uint64_t Size) {
    if (!V)
      return false;

    bool DivisionFound = false;
    bool AllocSiteFound = false;
    for (User *U : V->users()) {
      // Find the division
      if (auto *BinOp = dyn_cast<BinaryOperator>(U)) {
        if (!DivisionFound) {
          bool IsExact = false;
          if (isValidDivision(BinOp, V, Size, &IsExact) && IsExact) {

            // Check that the division is guarded
            bool ZeroFound = false;
            bool PositiveFound = false;
            for (User * DivU : BinOp->users()) {
              // Check if it is comparing against positive numbers.
              // These instructions can be used to ensure limits.
              if (auto *ICmp = dyn_cast<ICmpInst>(DivU)) {
                ConstantInt *Const = nullptr;
                if (BinOp == ICmp->getOperand(0))
                  Const = dyn_cast<ConstantInt>(ICmp->getOperand(1));
                else
                  Const = dyn_cast<ConstantInt>(ICmp->getOperand(0));

                if (!Const)
                  return false;

                if (Const->isZero())
                  ZeroFound = true;
                else if (!Const->isNegative())
                  PositiveFound = true;
                else
                  return false;
              }
            }
            DivisionFound = ZeroFound && PositiveFound;
          } else {
            return false;
          }
        } else {
          return false;
        }
      }

      // Check the callsite for "new"
      else if (auto *Call = dyn_cast<CallBase>(U)) {
        if (!AllocSiteFound) {
          if (isValidCallToNew(Call, V))
            AllocSiteFound = true;
          else
            return false;
        } else {
          return false;
        }
      }

      // Else, the input Value is used for something we don't know.
      else {
        return false;
      }
    }

    // Return if the division was found and the call to new was found.
    return DivisionFound && AllocSiteFound;
  }

  bool hasNonDivBySizeUses(Value *V, uint64_t Size) {
    for (auto *U : V->users()) {
      if (auto *BinOp = dyn_cast<BinaryOperator>(U)) {
        bool IsExact = false;
        if (isValidDivision(BinOp, V, Size, &IsExact))
          continue;
        return true;
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

  // Given LocalPointerInfo for a value, set the specified safety data
  // for the base type of every type which is known to alias to the value.
  void setAllAliasedTypeSafetyData(LocalPointerInfo &LPI,
                                   dtrans::SafetyData Data) {
    for (auto *Ty : LPI.getPointerTypeAliasSet())
      if (DTInfo.isTypeOfInterest(Ty))
        setBaseTypeInfoSafetyData(Ty, Data);
  }

  // This is a version of setAllAliasedTypeSafetyData that always cascades the
  // safety data to nested types and through pointers.
  void setAllAliasedTypeSafetyDataWithCascading(LocalPointerInfo &LPI,
                                                dtrans::SafetyData Data) {
    for (auto *Ty : LPI.getPointerTypeAliasSet())
      if (DTInfo.isTypeOfInterest(Ty))
        setBaseTypeInfoSafetyDataWithCascading(Ty, Data);
  }

  // Return true if 'Data' should be propagated down to all types nested
  // within some type for which the safety condition was found to hold.
  // The motivation for this propagation is that a user may access outside
  // the bounds of a structure. This is strictly not allowed in C/C++, but
  // is allowed under the definition of LLVM IR.
  bool isCascadingSafetyCondition(dtrans::SafetyData Data) {
    if (DTInfo.getDTransOutOfBoundsOK())
      return true;
    switch (Data) {
    // We can add additional cases here to reduce the conservative behavior
    // as needs dictate.
    case dtrans::FieldAddressTakenMemory:
    case dtrans::FieldAddressTakenCall:
    case dtrans::FieldAddressTakenReturn:
    case dtrans::HasZeroSizedArray:
      return false;
    }
    return true;
  }

  // Return true if \p Data is a safety condition that should be cascaded
  // across pointer field members within a structure. Most safety conditions
  // detected on a structure do not apply to child structures that are only
  // referenced by pointer fields (as opposed to fully nested structures)
  // because changing the layout of the parent structure does not affect
  // the type pointed to. However, some safety conditions relating to how
  // a structure can be accessed in ways that we can't fully analyze must
  // be transferred to contained pointers.
  bool isPointerCarriedSafetyCondition(dtrans::SafetyData Data) {
    switch (Data) {
    case dtrans::AddressTaken:
    case dtrans::AmbiguousPointerTarget:
    case dtrans::BadCasting:
    case dtrans::BadCastingPending:
    case dtrans::BadMemFuncManipulation:
    case dtrans::BadCastingForRelatedTypes:
    case dtrans::SystemObject:
    case dtrans::UnsafePtrMerge:
    case dtrans::UnhandledUse:

      // TODO: UnsafePointerStore and UnsafePointerStoreConditional should be
      // pointer carried because they can have the effect of changing the type
      // of a pointer, similar to a bitcast but done via a memory location.
      // Currently, these are inhibited because more analysis is necessary on
      // mitigating the impact this has on one of the benchmarks.
      //
      // case dtrans::UnsafePointerStore:
      // case dtrans::UnsafePointerStoreConditional:

      return true;

      // All forms of FielAddressTaken are treated as a pointer carried
      // condition based on how out of bounds field accesses is set because the
      // access is not permitted under the C/C++ rules, but is allowed within
      // the definition of llvm IR. If an out of bounds access is permitted,
      // then it would be possible to access elements of pointed-to objects,
      // as well, in methods that DTrans would not be able to analyze.
    case dtrans::FieldAddressTakenMemory:
    case dtrans::FieldAddressTakenCall:
    case dtrans::FieldAddressTakenReturn:
      return DTInfo.getDTransOutOfBoundsOK();

    default:
      return false;
    }
    llvm_unreachable("Fully covered switch isn't fully covered?");
  }

  // This is a helper function that retrieves the aggregate type through
  // zero or more layers of indirection and sets the specified safety data
  // for that type.
  void setBaseTypeInfoSafetyData(llvm::Type *Ty, dtrans::SafetyData Data) {
    setBaseTypeInfoSafetyData(Ty, Data, isCascadingSafetyCondition(Data),
                              isPointerCarriedSafetyCondition(Data));
  }

  // This is a version of setBaseTypeInfoSafetyData that always cascades the
  // safety data to nested types and through pointers.
  void setBaseTypeInfoSafetyDataWithCascading(llvm::Type *Ty,
                                              dtrans::SafetyData Data) {
    setBaseTypeInfoSafetyData(Ty, Data, /*IsCascading=*/true,
                              /*IsPointerCarried=*/true);
  }

  // SubGraph: A struct type is considered as enclosing type of another
  // struct if all references of another structure are only reachable
  // from member functions of the enclosing type. 'SubGraph' is used
  // to represent enclosing type for struct in StructInfo.
  //
  // Lattice of properties of SubGraph:
  //  {bottom = <nullptr, false>, <Type*, false>, top = <nullptr, true>}
  // This function performs 'join' operation of the lattice.
  // “Bottom” refers to unanalyzed case (initial value) and “top” refers
  // to worst case (no enclosing type).
  //
  // updateSubGraphNode is called when any reference to "ThisTy" struct is
  // noticed in "F".
  //
  void updateSubGraphNode(Function *F, StructType *ThisTy) {
    auto *TI = cast<dtrans::StructInfo>(DTInfo.getTypeInfo(ThisTy));
    auto &CG = TI->getCallSubGraph();

    // If we could not approximate CallGraph by methods of some class,
    // no need to analyze further.
    if (CG.isTop())
      return;

    // If reference to ThisTy is encountered in global scope or
    // inside function, which does not look like class method,
    // then mark CallGraph approximation as 'top' or 'failed' approximation.
    if (!F || F->arg_size() < 1) {
      TI->setCallGraphTop();
      return;
    }
    // Candidate for 'this' pointer;
    auto *Ty = F->arg_begin()->getType();
    if (!isa<PointerType>(Ty)) {
      TI->setCallGraphTop();
      return;
    }
    auto *StTy = dyn_cast<StructType>(Ty->getPointerElementType());
    if (!StTy) {
      TI->setCallGraphTop();
      return;
    }

    // Ty >= ThisTy
    //
    // Check if ThisTy is reachable from Ty by recursion to
    // structure's elements and following pointers.
    std::function<bool(Type *, StructType *, int)> findSubType =
        [&findSubType](Type *Ty, StructType *ThisTy, int Depth) -> bool {
      Depth--;
      if (Depth <= 0)
        return false;
      switch (Ty->getTypeID()) {
      default:
        return false;
      case Type::StructTyID: {
        auto *STy = cast<StructType>(Ty);
        if (STy == ThisTy)
          return true;
        for (auto *FTy : STy->elements())
          if (findSubType(FTy, ThisTy, Depth))
            return true;
        return false;
      }
      case Type::ArrayTyID:
        if (findSubType(Ty->getArrayElementType(), ThisTy, Depth))
          return true;
        return false;
      case Type::PointerTyID:
        if (findSubType(Ty->getPointerElementType(), ThisTy, Depth))
          return true;
        return false;
      }
      llvm_unreachable("Non-exhaustive switch statement");
    };

    // !(StTy >= ThisTy)
    // If ThisTy is not reachable from 'this' argument,
    // then mark as 'top'
    if (!findSubType(StTy, ThisTy, 5)) {
      TI->setCallGraphTop();
      return;
    }

    // Compute `join`.
    // If cannot find least common approximation to old and new approximation,
    // then mark as 'top'.
    if (CG.isBottom()) {
      TI->setCallGraphEnclosingType(StTy);
    } else if (findSubType(StTy, CG.getEnclosingType(), 5)) {
      TI->setCallGraphEnclosingType(StTy);
    } else if (!findSubType(CG.getEnclosingType(), StTy, 5)) {
      TI->setCallGraphTop();
      return;
    }
    // else do nothing
  }

  // This is a helper function that retrieves the aggregate type through
  // zero or more layers of indirection and sets the specified safety data
  // for that type.
  //
  // When \p IsCascading is set, the safety data will also
  // be set for fields nested (and possibly pointers) within a structure type.
  //
  // When \p IsPointerCarried is set, the safety data will also be cascaded to
  // the types referenced via the pointer. This parameter is only used, when
  // \p IsCascading is set to true.
  //
  // Note, the descending into types for cascading of the safety data stops when
  // a type is encountered that already contains the safety data value.
  void setBaseTypeInfoSafetyData(llvm::Type *Ty, dtrans::SafetyData Data,
                                 bool IsCascading, bool IsPointerCarried) {
    llvm::Type *BaseTy = Ty;
    while (BaseTy->isPointerTy())
      BaseTy = BaseTy->getPointerElementType();
    dtrans::TypeInfo *TI = DTInfo.getOrCreateTypeInfo(BaseTy);
    TI->setSafetyData(Data);
    if (!IsCascading)
      return;

    // This lambda encapsulates the logic for propagating safety conditions to
    // structure field or array element types. If the field or element is an
    // instance of a type of interest, and not if it is merely a pointer to
    // such a type, the condition is propagated. If the field is a pointer,
    // we call a helper function to see if this is a condition which requires
    // propagation through pointer fields. Propagation is done via a recursive
    // call to setBaseTypeInfoSafetyData in order to handle additional levels
    // of nesting.
    auto maybePropagateSafetyCondition = [this, BaseTy](llvm::Type *FieldTy,
                                                        dtrans::SafetyData Data,
                                                        bool IsCascading,
                                                        bool IsPointerCarried) {
      // If FieldTy is not a type of interest, there's no need to propagate.
      if (!DTInfo.isTypeOfInterest(FieldTy))
        return;
      // If the field is an instance of the type, propagate the condition.
      if (!FieldTy->isPointerTy()) {
        setBaseTypeInfoSafetyData(FieldTy, Data, IsCascading, IsPointerCarried);
      } else if (IsPointerCarried) {
        // In some cases we need to propagate the condition even to fields
        // that are pointers to structures, but in order to avoid infinite
        // loops in the case where two structures each have pointers to the
        // other we need to avoid doing this for structures that already have
        // the condition set.
        llvm::Type *FieldBaseTy = FieldTy;
        while (FieldBaseTy->isPointerTy())
          FieldBaseTy = FieldBaseTy->getPointerElementType();
        dtrans::TypeInfo *FieldTI = DTInfo.getOrCreateTypeInfo(FieldBaseTy);
        if (!FieldTI->testSafetyData(Data)) {
          (void)BaseTy;
          LLVM_DEBUG({
            dbgs()
                << "dtrans-safety: Cascading pointer carried safety condition: "
                << "From: " << *BaseTy << " To: " << *FieldBaseTy
                << " :: " << dtrans::getSafetyDataName(Data) << "\n";
          });

          setBaseTypeInfoSafetyData(FieldBaseTy, Data, IsCascading,
                                    IsPointerCarried);
        }
      }
    };

    // Propagate this condition to any nested types.
    if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI))
      for (dtrans::FieldInfo &FI : StInfo->getFields())
        maybePropagateSafetyCondition(FI.getLLVMType(), Data, IsCascading,
                                      IsPointerCarried);
    else if (isa<dtrans::ArrayInfo>(TI))
      maybePropagateSafetyCondition(BaseTy->getArrayElementType(), Data,
                                    IsCascading, IsPointerCarried);
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
        [this, &DT, F, &Propagate](llvm::Type *Ty) -> void {
      if (!DT.isTypeOfInterest(Ty))
        return;
      if (auto *STy = dyn_cast<StructType>(Ty)) {
        updateSubGraphNode(F, STy);
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

    // Creating the ElementInfo may have created the needed type during a
    // recursive call because the type was a field in a type referenced
    // by the array, so check for the type again before creating a new object.
    // For example:
    //   Ty: [5 x %struct.foo*]
    //       %struct.foo = type { [5 x %struct.foo*] }
    auto TI = getTypeInfo(Ty);
    if (TI)
      return TI;

    DTransTy =
        new dtrans::ArrayInfo(Ty, ElementInfo, Ty->getArrayNumElements());
  } else if (Ty->isStructTy()) {
    llvm::StructType *STy = cast<StructType>(Ty);
    SmallVector<dtrans::AbstractType, 16> FieldTypes;
    for (llvm::Type *FieldTy : STy->elements()) {
      FieldTypes.push_back(FieldTy);
      // Create a DTrans type for the field, in case it is an aggregate.
      (void)getOrCreateTypeInfo(FieldTy);
    }

    // Creating the TypeInfo objects for the fields may have resulted
    // in the StructInfo for this type being created because the type
    // may be reachable from one of the fields, so check for the type again
    // before creating a new object. For example:
    //   Ty: %struct.foo = type { i32, %struct.foo* }
    auto TI = getTypeInfo(Ty);
    if (TI)
      return TI;

    DTransTy = new dtrans::StructInfo(Ty, FieldTypes);
  } else {
    assert(!Ty->isAggregateType() &&
           "DTransAnalysisInfo::getOrCreateTypeInfo unexpected aggregate type");
    DTransTy = new dtrans::NonAggregateTypeInfo(Ty);
  }

  TypeInfoMap[Ty] = DTransTy;
  auto RelatedTypeIT = RelatedTypesMap.find(Ty);

  // Set the related type
  if (RelatedTypeIT != RelatedTypesMap.end()) {
    llvm::Type *RelatedType = RelatedTypesMap[Ty];
    dtrans::StructInfo *CurrInfo =  cast<dtrans::StructInfo>(DTransTy);
    dtrans::StructInfo *RelatedInfo = nullptr;
    auto TypeInfoIT = TypeInfoMap.find(RelatedType);
    if (TypeInfoIT != TypeInfoMap.end())
      RelatedInfo = cast<dtrans::StructInfo>(TypeInfoMap[RelatedType]);
    else
      RelatedInfo = cast<dtrans::StructInfo>(
        getOrCreateTypeInfo(RelatedTypesMap[Ty]));

    CurrInfo->setRelatedType(RelatedInfo);

    // If Ty is the padded type, then set the last field as
    // padded field.
    int64_t CurrNumFields = CurrInfo->getNumFields();
    int64_t RelatedNumFields = RelatedInfo->getNumFields();
    if ((CurrNumFields - RelatedNumFields) == 1)
      CurrInfo->getField(CurrNumFields - 1).setPaddedField();
  }

  return DTransTy;
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
    GEPOperator *GEP, std::pair<llvm::Type *, size_t> Pointee) {
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

void DTransAnalysisInfo::addMultiElemLoadStore(Instruction *I) {
  MultiElemLoadStoreInfo.insert(I);
}

bool DTransAnalysisInfo::isMultiElemLoadStore(Instruction *I) {
  if (MultiElemLoadStoreInfo.count(I))
    return true;
  return false;
}

std::pair<llvm::Type *, size_t>
DTransAnalysisInfo::getByteFlattenedGEPElement(GEPOperator *GEP) {
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
                         const Instruction *, unsigned, dtrans::CallInfo *>>
      Entries;

  // To get some consistency in the printing order, populate a tuple
  // that can be sorted, then output the sorted list.
  for (auto CIVec : call_info_entries())
    for (auto &E : enumerate(CIVec)) {
      auto *CI = E.value();
      Instruction *I = CI->getInstruction();
      Entries.push_back(std::make_tuple(I->getFunction()->getName(),
                                        CI->getCallInfoKind(), I, E.index(),
                                        CI));
    }

  std::sort(Entries.begin(), Entries.end());
  for (auto &Entry : Entries) {
    OS << "Function: " << std::get<0>(Entry) << "\n";
    OS << "Instruction: " << *std::get<2>(Entry) << "\n";
    std::get<4>(Entry)->print(OS);
    OS << "\n";
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

INITIALIZE_PASS_BEGIN(DTransAnalysisWrapper, "dtransanalysis",
                      "Data transformation analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(BlockFrequencyInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DTransImmutableAnalysisWrapper)
INITIALIZE_PASS_END(DTransAnalysisWrapper, "dtransanalysis",
                    "Data transformation analysis", false, true)

char DTransAnalysisWrapper::ID = 0;

ModulePass *llvm::createDTransAnalysisWrapperPass() {
  return new DTransAnalysisWrapper();
}

DTransAnalysisWrapper::DTransAnalysisWrapper()
    : ModulePass(ID), Invalidated(true) {
  initializeDTransAnalysisWrapperPass(*PassRegistry::getPassRegistry());
}

bool DTransAnalysisWrapper::doFinalization(Module &M) {
  Result.reset();
  Invalidated = true;
  return false;
}

DTransAnalysisInfo &DTransAnalysisWrapper::getDTransInfo(Module &M) {
  // Rerun the analysis, if the prior transformation has marked it as
  // invalidated.
  if (Invalidated) {
    Result.reset();
    runOnModule(M);
  } else {
    // Check that the previous transforms did not change types and forget
    // to call setInvalidated.
    assert(verifyValid(M) &&
           "Types changed, but DTrans analysis not marked as invalidated");
  }

  return Result;
}

#if !defined(NDEBUG)
// This is for verifying that a transformation did not forget to call
// setInvalidated after making types changes. Because we require the
// transformation to replace types with new types, this checks to make sure that
// there aren't any new top-level structure types that the analysis does not
// know about. This should also prevent the getDTransInfo from being called
// with a different Module than the one that was collected for. Returns 'true',
// if everything appears valid.
bool DTransAnalysisWrapper::verifyValid(Module &M) {
  if (!Result.useDTransAnalysis())
    return true;

  for (auto *Ty : M.getIdentifiedStructTypes())
    if (!Result.getTypeInfo(Ty)) {
      dbgs() << "No DTrans TypeInfo for struct type:" << *Ty << "\n";
      return false;
    }

  return true;
}
#endif // !defined(NDEBUG)

bool DTransAnalysisWrapper::runOnModule(Module &M) {

  auto GetBFI = [this](Function &F) -> BlockFrequencyInfo & {
    return this->getAnalysis<BlockFrequencyInfoWrapperPass>(F).getBFI();
  };
  auto GetTLI = [this](const Function &F) -> TargetLibraryInfo & {
    return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
  };

  auto &DTImmutInfo = getAnalysis<DTransImmutableAnalysisWrapper>().getResult();

  auto GetDomTree = [this](Function &F) -> DominatorTree & {
    return this->getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
  };

  Invalidated = false;
  return Result.analyzeModule(
      M, GetTLI, getAnalysis<WholeProgramWrapperPass>().getResult(), GetBFI,
      DTImmutInfo, GetDomTree);
}

DTransAnalysisInfo::DTransAnalysisInfo()
    : MaxTotalFrequency(0), FunctionCount(0), CallsiteCount(0),
      InstructionCount(0), DTransAnalysisRan(false) {}

// Value map has a deleted move constructor, so we need a non-default
// implementation of ours.
DTransAnalysisInfo::DTransAnalysisInfo(DTransAnalysisInfo &&Other)
    : TypeInfoMap(std::move(Other.TypeInfoMap)), CIM(std::move(Other.CIM)) {
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
  MultiElemLoadStoreInfo.insert(Other.MultiElemLoadStoreInfo.begin(),
                                Other.MultiElemLoadStoreInfo.end());
  FunctionsRequireBadCastValidation.insert(
      Other.FunctionsRequireBadCastValidation.begin(),
      Other.FunctionsRequireBadCastValidation.end());
  MaxTotalFrequency = Other.MaxTotalFrequency;
  FunctionCount = Other.FunctionCount;
  CallsiteCount = Other.CallsiteCount;
  InstructionCount = Other.InstructionCount;
  DTransAnalysisRan = Other.DTransAnalysisRan;
  SawFortran = Other.SawFortran;
}

DTransAnalysisInfo::~DTransAnalysisInfo() { reset(); }

DTransAnalysisInfo &DTransAnalysisInfo::operator=(DTransAnalysisInfo &&Other) {
  reset();
  TypeInfoMap = std::move(Other.TypeInfoMap);
  CIM = std::move(Other.CIM);
  PtrSubInfoMap.insert(Other.PtrSubInfoMap.begin(), Other.PtrSubInfoMap.end());
  ByteFlattenedGEPInfoMap.insert(Other.ByteFlattenedGEPInfoMap.begin(),
                                 Other.ByteFlattenedGEPInfoMap.end());
  StoreInfoMap.insert(Other.StoreInfoMap.begin(), Other.StoreInfoMap.end());
  LoadInfoMap.insert(Other.LoadInfoMap.begin(), Other.LoadInfoMap.end());
  GenericStoreInfoMap.insert(Other.GenericStoreInfoMap.begin(),
                             Other.GenericStoreInfoMap.end());
  GenericLoadInfoMap.insert(Other.GenericLoadInfoMap.begin(),
                            Other.GenericLoadInfoMap.end());
  MultiElemLoadStoreInfo.insert(Other.MultiElemLoadStoreInfo.begin(),
                                Other.MultiElemLoadStoreInfo.end());
  FunctionsRequireBadCastValidation.insert(
      Other.FunctionsRequireBadCastValidation.begin(),
      Other.FunctionsRequireBadCastValidation.end());
  MaxTotalFrequency = Other.MaxTotalFrequency;
  FunctionCount = Other.FunctionCount;
  CallsiteCount = Other.CallsiteCount;
  InstructionCount = Other.InstructionCount;
  DTransAnalysisRan = Other.DTransAnalysisRan;
  IgnoreTypeMap = std::move(Other.IgnoreTypeMap);
  SawFortran = Other.SawFortran;
  return *this;
}

void DTransAnalysisInfo::reset() {
  CIM.reset();

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
  MultiElemLoadStoreInfo.clear();
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
        else if (TransformationAndTypes.first == "reusefield")
          TransName = dtrans::DT_ReuseField;
        else if (TransformationAndTypes.first == "reusefieldptr")
          TransName = dtrans::DT_ReuseFieldPtr;
        else if (TransformationAndTypes.first == "reusefieldptrofptr")
          TransName = dtrans::DT_ReuseFieldPtrOfPtr;
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
        else if (TransformationAndTypes.first == "meminittrimdown")
          TransName = dtrans::DT_MemInitTrimDown;
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

  dtrans::SafetyData Conditions =
      dtrans::getConditionsForTransform(Transform, getDTransOutOfBoundsOK());
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
              cast<dtrans::StructInfo>(TyInfo)->setIgnoredFor(Transform);
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

// Set 'SawFortran' if there is a Fortran function. This will disable
// settings like DTransOutOfBoundsOK.
void DTransAnalysisInfo::checkLanguages(Module &M) {
  for (auto &F : M.functions())
    if (F.isFortran()) {
      SawFortran = true;
      break;
    }
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
  return getLangRuleOutOfBoundsOK();
}

bool DTransAnalysisInfo::getDTransUseCRuleCompat() {
  return !SawFortran && dtrans::DTransUseCRuleCompat;
}

bool DTransAnalysisInfo::requiresBadCastValidation(
    SetVector<Function *> &Func, unsigned &ArgumentIndex,
    unsigned &StructIndex) const {
  Func.clear();
  Func.insert(FunctionsRequireBadCastValidation.begin(),
              FunctionsRequireBadCastValidation.end());

  ArgumentIndex = dtrans::DTransBadCastingAnalyzer::VoidArgumentIndex;
  StructIndex = dtrans::DTransBadCastingAnalyzer::CandidateVoidField;

  return !Func.empty();
}

// Build the related types map. This map will store the relationship between
// a padded structure and a base structure.
void DTransAnalysisInfo::buildRelatedTypesMap(Module &M) {

  SetVector<llvm::StructType *> TypesCollected;
  dtrans::collectAllStructTypes(M, TypesCollected);

  for (StructType *STy : TypesCollected) {
    llvm::Type *Ty = cast<llvm::Type>(STy);

    if (RelatedTypesMap.count(Ty) > 0)
      continue;

    llvm::Type *RelatedType = dtrans::collectRelatedType(Ty, M);
    if (!RelatedType)
      continue;

    RelatedTypesMap[RelatedType] = Ty;
    RelatedTypesMap[Ty] = RelatedType;
  }
}

bool DTransAnalysisInfo::analyzeModule(
    Module &M, GetTLIFnType GetTLI, WholeProgramInfo &WPInfo,
    function_ref<BlockFrequencyInfo &(Function &)> GetBFI,
    DTransImmutableInfo &DTImmutInfo,
    std::function<DominatorTree &(Function &)> GetDomTree) {
  LLVM_DEBUG(dbgs() << "Running DTransAnalysisInfo::analyzeModule\n");
  if (dtrans::shouldRunOpaquePointerPasses(M)) {
    LLVM_DEBUG(
        dbgs() << "dtrans: Pointers are opaque or opaque passes requested ... "
                      << "DTransAnalysis didn't run\n");
    return false;
  }
  if (!WPInfo.isWholeProgramSafe()) {
    LLVM_DEBUG(dbgs() << "dtrans: Whole Program not safe ... "
                      << "DTransAnalysis didn't run\n");
    return false;
  }

  setCallGraphStats(M);
  buildRelatedTypesMap(M);

  if (!shouldComputeDTransAnalysis())
    return false;
  dtrans::DTransAllocAnalyzer DTAA(GetTLI, M);
  dtrans::DTransBadCastingAnalyzer DTBCA(*this, DTAA, GetTLI, M);
  DTAA.populateAllocDeallocTable(M);

  DTransInstVisitor Visitor(M.getContext(), *this, M.getDataLayout(), GetTLI,
                            DTAA, DTBCA, GetBFI, GetDomTree);
  parseIgnoreList();

  checkLanguages(M);
  DTBCA.analyzeBeforeVisit();
  Visitor.visit(M);
  Visitor.collectCallGraphInfo(M);
  DEBUG_WITH_TYPE(DTRANS_LPA_RESULTS, Visitor.dump(M));
  Visitor.processDeferredPointerCarriedSafetyData();
  DTBCA.analyzeAfterVisit();

  // Record functions require bad cast validation.
  DTBCA.getConditionalFunctions(FunctionsRequireBadCastValidation);

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

  // Go through the multiple StructInfo and check if there is a safety
  // violation that makes the padded field dirty. Also, this is the moment
  // where we are going to set which structures are base and padded structures.
  //
  // NOTE: A dirty padded field means that we don't have enough information
  // to make sure if the field is not being modified.
  for (auto *TI : type_info_entries()) {
    if (auto *STInfo = dyn_cast<dtrans::StructInfo>(TI)) {

      // Return true if the input StructInfo has a padded field and
      // we found any safety violation for that field.
      auto HasInvalidPaddedField = [](dtrans::StructInfo *StrInfo) {
        if (!StrInfo)
          return true;

        if (!StrInfo->hasPaddedField())
          return false;

        size_t NumFields = StrInfo->getNumFields();
        auto &PaddedField = StrInfo->getField(NumFields - 1);

        if (!PaddedField.isPaddedField())
          return false;

        // These are the conditions we use to invalidate the padded field.
        // Perhaps some of them could be relaxed like address taken and
        // complex use.
        if (PaddedField.isRead() || PaddedField.isWritten() ||
            PaddedField.hasComplexUse() || PaddedField.isAddressTaken() ||
            !PaddedField.isNoValue() || !PaddedField.isTopAllocFunction())
          return true;

        return false;
      };

      // Return true if the input structures are padded and base structures.
      auto ArePaddedAndBaseStructures = [](dtrans::StructInfo *PaddedStruct,
          dtrans::StructInfo *BaseStruct) -> bool {

        assert((PaddedStruct->getRelatedType() == BaseStruct &&
                BaseStruct->getRelatedType() == PaddedStruct) &&
                "Incorrect related types set");

        unsigned NumFieldsPadded = PaddedStruct->getNumFields();
        unsigned NumFieldsBase = BaseStruct->getNumFields();

        if (NumFieldsPadded - NumFieldsBase != 1)
          return false;

        // We know that there will be at least one field in the padded
        // structures
        auto &LastFieldPadded = PaddedStruct->getField(NumFieldsPadded - 1);
        if (!LastFieldPadded.isPaddedField())
          return false;

        // If the base structure is not empty then the last field shouldn't
        // be marked for ABI padding
        if (NumFieldsBase > 0) {
          auto &LastFieldBase = BaseStruct->getField(NumFieldsBase - 1);
          if (LastFieldBase.isPaddedField())
            return false;
        }

        return true;
      };

      dtrans::StructInfo *RelatedTypeInfo = STInfo->getRelatedType();
      if (!RelatedTypeInfo)
        continue;

      const dtrans::SafetyData ABIPaddingSet =
          dtrans::StructCouldHaveABIPadding |
          dtrans::StructCouldBeBaseABIPadding;

      // Skip those the structures that were analyzed already
      if (STInfo->testSafetyData(ABIPaddingSet))
        continue;

      // Identify the base and padded structures
      dtrans::StructInfo *BaseStruct = nullptr;
      dtrans::StructInfo *PaddedStruct = nullptr;

      if (ArePaddedAndBaseStructures(STInfo, RelatedTypeInfo)) {
        PaddedStruct = STInfo;
        BaseStruct = RelatedTypeInfo;
      }
      else if (ArePaddedAndBaseStructures(RelatedTypeInfo, STInfo)) {
        PaddedStruct = RelatedTypeInfo;
        BaseStruct = STInfo;
      } else {
        llvm_unreachable("Incorrect base and padded structures set");
      }

      // Check if the padded field has any safety violation that
      // could break the relationship.
      bool BadSafetyData = HasInvalidPaddedField(PaddedStruct);

      if (BadSafetyData) {
        size_t NumFields = PaddedStruct->getNumFields();
        auto &Field = PaddedStruct->getField(NumFields - 1);
        Field.invalidatePaddedField();
      }

      // DTransTestPaddedStructs is used for testing purposes.
      if (!BadSafetyData || DTransTestPaddedStructs) {
        PaddedStruct->setSafetyData(dtrans::StructCouldHaveABIPadding);
        BaseStruct->setSafetyData(dtrans::StructCouldBeBaseABIPadding);
      } else {
        // If the safety data fails then break the relationship between
        // padded and base.
        PaddedStruct->unsetRelatedType();
        BaseStruct->unsetRelatedType();
      }
    }
  }

  // Invalidate the fields for which the corresponding types do not pass
  // the SafetyData checks.
  for (auto *TI : type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    bool IsInBounds = !getDTransOutOfBoundsOK();
    if (StInfo) {
      bool SD_FSV = testSafetyData(TI, dtrans::DT_FieldSingleValue);
      bool SD_FSAF = testSafetyData(TI, dtrans::DT_FieldSingleAllocFunction);
      for (unsigned I = 0, E = StInfo->getNumFields(); I != E; ++I) {

        // If we proved that the field is a clean padded field, then don't
        // set it as isBottomAllocFunction(). We know that it won't be used.
        if (StInfo->getField(I).isCleanPaddedField())
          continue;

        // Mark the field as 'incomplete' if safety conditions are not met.
        // In case of DTransOutOfBoundsOK == false we change to 'incomplete'
        // only those fields that are marked as address taken (if any).
        if (SD_FSV || (IsInBounds && StInfo->getField(I).isAddressTaken()))
          StInfo->getField(I).setIncompleteValueSet();
        // Mark the field as 'Bottom alloc function' if safety conditions are
        // not met. In case of DTransOutOfBoundsOK == false we set 'Bottom alloc
        // function' only to the fields marked as address taken (if any).
        if (SD_FSAF || (IsInBounds && StInfo->getField(I).isAddressTaken())) {
          DEBUG_WITH_TYPE(DTRANS_FSAF, {
            if (!StInfo->getField(I).isBottomAllocFunction())
              dbgs() << "dtrans-fsaf: " << *(StInfo->getLLVMType()) << " ["
                     << I << "] <BOTTOM>\n";
          });
          StInfo->getField(I).setBottomAllocFunction();
        }
      }
    }
  }

  // Set all aggregate fields conservatively as MultipleValue and
  // BottomAllocFunction for now.
  for (auto *TI : type_info_entries()) {
    if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI))
      for (unsigned I = 0, E = StInfo->getNumFields(); I != E; ++I) {
        // If we proved that the field is a clean padded field, then don't
        // set it as isBottomAllocFunction(). We know that it won't be used.
        if (StInfo->getField(I).isCleanPaddedField())
          continue;

        if (StInfo->getField(I).getLLVMType()->isAggregateType()) {
          StInfo->getField(I).setIncompleteValueSet();
          DEBUG_WITH_TYPE(DTRANS_FSAF, {
            if (!StInfo->getField(I).isBottomAllocFunction())
              dbgs() << "dtrans-fsaf: " << *(StInfo->getLLVMType()) << " ["
                     << I << "] <BOTTOM>\n";
          });
          StInfo->getField(I).setBottomAllocFunction();
        }
      }
  }

  if (DTransArraysWithConstEntries) {
    // Analyze the arrays with constant information
    dtrans::DtransArraysWithConstant DTransArraysWithConstant;
    DTransArraysWithConstant.runArraysWithConstAnalysis(M, this);
  }

  DTransAnalysisRan = true;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (dtrans::DTransPrintAnalyzedTypes) {
    // Prints the list of transformations for which the safety data will be
    // ignored on the structure 'SI' based on the command line option.
    auto &IgnoreTypeMap = this->IgnoreTypeMap;
    std::function<void(raw_ostream &, const dtrans::StructInfo *)>
        PrintIgnoreListForStructure = [&IgnoreTypeMap](
                                          raw_ostream &OS,
                                          const dtrans::StructInfo *SI) {
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
            OS << "  (will be ignored in" << Output << ")\n";
          }
        };

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

    dbgs() << "================================\n";
    dbgs() << " DTRANS Analysis Types Created\n";
    dbgs() << "================================\n\n";
    for (auto TI : TypeInfoEntries) {
      if (auto *AI = dyn_cast<dtrans::ArrayInfo>(TI)) {
        AI->print(dbgs());
      } else if (auto *SI = dyn_cast<dtrans::StructInfo>(TI)) {
        SI->print(dbgs(), &PrintIgnoreListForStructure);
      }
    }
    dbgs() << "\n MaxTotalFrequency: " << getMaxTotalFrequency() << "\n\n";
    dbgs().flush();
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // Copy type info which can be passed to downstream passes without worrying
  // about invalidation into the immutable pass.
  for (auto *TypeInfo : type_info_entries()) {
    if (auto *StructInfo = dyn_cast<dtrans::StructInfo>(TypeInfo))
      for (unsigned I = 0, E = StructInfo->getNumFields(); I != E; ++I) {
        DTImmutInfo.addStructFieldInfo(
            cast<StructType>(StructInfo->getLLVMType()), I,
            StructInfo->getField(I).values(),
            StructInfo->getField(I).iavalues(),
            StructInfo->getField(I).getArrayConstantEntries());
      }
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (dtrans::DTransPrintAnalyzedCalls) {
    printCallInfo(dbgs());
    dbgs().flush();
  }

  if (dtrans::DTransPrintImmutableAnalyzedTypes)
    DTImmutInfo.print(dbgs());
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  return false;
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
    auto StructField = getByteFlattenedGEPElement(GEP);
    auto StructTy = dyn_cast_or_null<StructType>(StructField.first);
    if (!StructTy)
      return std::make_pair(nullptr, 0);

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
    Value *FP, std::vector<Value *> &Targets, CallBase *, bool) {
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

void DTransAnalysisWrapper::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<BlockFrequencyInfoWrapperPass>();
  AU.addRequired<WholeProgramWrapperPass>();
  AU.addRequired<DTransImmutableAnalysisWrapper>();
}

char DTransAnalysis::PassID;

// Provide a definition for the static class member used to identify passes.
AnalysisKey DTransAnalysis::Key;

bool DTransAnalysisInfo::invalidate(Module &M, const PreservedAnalyses &PA,
                                ModuleAnalysisManager::Invalidator &Inv) {
  auto PAC = PA.getChecker<DTransAnalysis>();
  return !PAC.preserved();
}

DTransAnalysisInfo DTransAnalysis::run(Module &M, AnalysisManager<Module> &AM) {
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetBFI = [&FAM](Function &F) -> BlockFrequencyInfo & {
    return FAM.getResult<BlockFrequencyAnalysis>(F);
  };
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function*>(&F)));
  };
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &DTImmutInfo = AM.getResult<DTransImmutableAnalysis>(M);

  auto GetDomTree = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };

  DTransAnalysisInfo DTResult;
  DTResult.analyzeModule(M, GetTLI, WPInfo, GetBFI, DTImmutInfo, GetDomTree);
  return DTResult;
}
