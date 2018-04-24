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

#define DEBUG_TYPE "dtransanalysis"

static cl::opt<bool> DTransPrintAllocations("dtrans-print-allocations",
                                            cl::ReallyHidden);

static cl::opt<bool> DTransPrintAnalyzedTypes("dtrans-print-types",
                                              cl::ReallyHidden);

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

namespace {

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
      : HasBeenAnalyzed(false), AliasesToAggregatePointer(false) {}

  void setAnalyzed() { HasBeenAnalyzed = true; }
  bool getAnalyzed() { return HasBeenAnalyzed; }

  bool canAliasToAggregatePointer() {
    assert(HasBeenAnalyzed);
    return AliasesToAggregatePointer;
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

  bool canPointToType(llvm::Type *T) {
    for (auto *AliasTy : PointerTypeAliases)
      if (AliasTy->isPointerTy() && AliasTy->getPointerElementType() == T)
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

  PointerTypeAliasSetRef getPointerTypeAliasSet() { return PointerTypeAliases; }
  ElementPointeeSetRef getElementPointeeSet() { return ElementPointees; }

  void merge(LocalPointerInfo &Other) {
    // This routine is called during analysis, so don't change HasBeenAnalyzed.
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
  bool HasBeenAnalyzed;
  bool AliasesToAggregatePointer;
  PointerTypeAliasSet PointerTypeAliases;
  ElementPointeeSet ElementPointees;
};

//
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
  std::set<BasicBlock *> VisitedBlocks;
  // A set to hold the BasicBlocks which do not need to be post-dominated
  // by malloc() to be considered isMallocPostDom() or free to be considered
  // isFreePostDom().
  std::set<BasicBlock *> SkipTestBlocks;
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
  auto it = VisitedBlocks.find(BB);
  if (it != VisitedBlocks.end())
    return;
  VisitedBlocks.insert(BB);
  it = SkipTestBlocks.find(BB);
  if (it != SkipTestBlocks.end())
    SkipTestBlocks.erase(it);
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
  std::set<BasicBlock *> SkipBlockSet;
  std::set<BasicBlock *> NoSkipBlockSet;
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
  std::set<BasicBlock *>::const_iterator it, ie;
  for (it = SkipBlockSet.begin(), ie = SkipBlockSet.end(); it != ie; ++it)
    visitAndSetSkipTestSuccessors(*it);
  for (it = NoSkipBlockSet.begin(), ie = NoSkipBlockSet.end(); it != ie; ++it)
    visitAndResetSkipTestSuccessors(*it);
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
  if (dtrans::getAllocFnKind(CI->getCalledFunction(), TLI) !=
      dtrans::AK_Malloc)
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
  DEBUG(dbgs() << "Analyzing for MallocPostDom " << F->getName() << "\n");
  VisitedBlocks.clear();
  if (std::distance(F->arg_begin(), F->arg_end()) == 1 &&
      F->arg_begin()->getType()->isIntegerTy()) {
    visitNullPtrBlocks(F);
    VisitedBlocks.clear();
  }
  bool rv = false;
  for (BasicBlock &BB : *F)
    if (auto RI = dyn_cast<ReturnInst>(BB.getTerminator())) {
      Value *RV = RI->getReturnValue();
      if (RV == nullptr) {
        DEBUG(dbgs() << "Not MallocPostDom " << F->getName()
                     << " Return is nullptr\n");
        return false;
      }
      if (!returnValueIsMallocAddress(RV, &BB)) {
        DEBUG(dbgs() << "Not MallocPostDom " << F->getName()
                     << " Return is not malloc address\n");
        return false;
      }
      rv = true;
    }
  if (rv)
    DEBUG(dbgs() << "Is MallocPostDom " << F->getName() << "\n");
  else
    DEBUG(dbgs() << "Not MallocPostDom " << F->getName()
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
  VisitedBlocks.clear();
  if (std::distance(F->arg_begin(), F->arg_end()) == 1 &&
      F->arg_begin()->getType()->isPointerTy()) {
    visitNullPtrBlocks(F);
    VisitedBlocks.clear();
  }
  DEBUG(dbgs() << "Analyzing for FreePostDom " << F->getName() << "\n");
  bool rv = false;
  for (BasicBlock &BB : *F)
    if (auto Ret = dyn_cast<ReturnInst>(BB.getTerminator())) {
      if (Ret->getReturnValue() != nullptr) {
        DEBUG(dbgs() << "Not FreePostDom " << F->getName()
                     << " Return is not nullptr\n");
        return false;
      }
      if (!isPostDominatedByFreeCall(&BB)) {
        DEBUG(dbgs() << "Not FreePostDom " << F->getName()
                     << " Return is not post-dominated by call to free\n");
        return false;
      }
      rv = true;
    }
  if (rv)
    DEBUG(dbgs() << "Is FreePostDom " << F->getName() << "\n");
  else
    DEBUG(dbgs() << "Not FreePostDom " << F->getName()
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
    // If we don't already have an entry for this pointer, do some analysis.
    if (!LocalMap.count(V))
      analyzeValue(V);
    // Now the information we want will be in the map.
    LocalPointerInfo &Info = LocalMap[V];
    assert((Info.getAnalyzed() || InProgressValues.count(V)) &&
           "Local pointer analysis failed.");
    return Info;
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
  SmallPtrSet<Value *, 8> InProgressValues;

  // These two maps are needed to manage cyclic dependencies in the
  // analysis chain. DependentValues maps a Value to the set of other
  // values that require the final alias set from the analysis of the key
  // value. PendingValues maps a value to the set of other Values on
  // which it is waiting.
  DenseMap<Value *, SmallPtrSet<Value *, 8>> DependentValues;
  DenseMap<Value *, SmallPtrSet<Value *, 8>> PendingValues;

  void analyzeValue(Value *V) {
    // If we've already analyzed this value, there is no need to
    // repeat the work.
    LocalPointerInfo &Info = LocalMap[V];
    if (Info.getAnalyzed())
      return;

    // If this isn't either a pointer, the result of a ptrtoint, or the load
    // of a pointer-sized integer we don't need to do any analysis.
    if (!V->getType()->isPointerTy() && !isa<PtrToIntInst>(V) &&
        !(isa<LoadInst>(V) &&
          (V->getType() == llvm::Type::getIntNTy(V->getContext(),
                                                 DL.getPointerSizeInBits())))) {
      Info.setAnalyzed();
      InProgressValues.erase(V);
      return;
    }

    // If we're already working on this value (for instance, tracing the
    // incoming values of a PHI node), don't go any further.
    if (!InProgressValues.insert(V).second) {
      DEBUG(dbgs() << "  InProgress skip analyzing " << *V << "\n");
      return;
    }

    DEBUG(dbgs() << "Analyzing " << *V << "\n");

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

    // If the value we're analyzing is a call to an allocation function
    // we need to look for bitcast users so that we can proactively assign
    // the type to which the value will be cast as an alias.
    if (auto *CI = dyn_cast<CallInst>(V)) {
      Function *Callee = CI->getCalledFunction();
      dtrans::AllocKind Kind = dtrans::getAllocFnKind(Callee, TLI);
      if (Kind == dtrans::AK_NotAlloc && DTAA.isMallocPostDom(Callee))
        Kind = dtrans::AK_Malloc;
      if (Kind != dtrans::AK_NotAlloc)
        analyzeAllocationCallAliases(CI, Info);
    }

    // If this is a GetElementPtr, figure out what element it is
    // accessing.
    if (auto *GEP = dyn_cast<GEPOperator>(V))
      analyzeElementAccess(GEP, Info);

    // If the value being analyzed is a load instruction the loaded value
    // may inherit some alias information from the load's pointer operand.
    if (auto *Load = dyn_cast<LoadInst>(V))
      analyzeLoadInstruction(Load, Info);

    // Mark the info as analyzed.
    Info.setAnalyzed();

    // If any other values were dependent on the result of the current
    // analysis, merge these results now.
    if (DependentValues.count(V)) {
      for (Value *Dependent : DependentValues[V]) {
        DEBUG(dbgs() << "  Merge analysis for " << V->getName() << " into "
                     << Dependent->getName() << "\n");
        LocalPointerInfo &DepLPI = LocalMap[Dependent];
        DepLPI.merge(Info);
        PendingValues[Dependent].erase(V);
      }
      DependentValues[V].clear();
    }

    // Erase this value from the in-progress set.
    // The 'analyzed' flag will be sufficient to prevent future re-analysis.
    InProgressValues.erase(V);
  }

  // This routine is called to attempt to get local pointer info for one
  // value while analyzing another. If the value has been previously
  // analyzed, a reference to its LocalPointerInfo will be returned.
  // If the value has not been analyzed and is not in the 'InProgressValues'
  // set, we will attempt to analyze it. If the analysis for this value is
  // already in progress (as can happen with a cycle containing PHI nodes)
  // the currently available (though incomplete) LocalPointerInfo will be
  // returned and an entry will be created in the dependent value map
  // indicating that the Dependent value has an unresolved dependency on
  // value V. When analysis of V is complete, these results will be merged
  // with the LocalPointerInfo for the Dependent value.
  LocalPointerInfo &tryGetLocalPointerInfo(Value *V, Value *Dependent) {
    // Helper lambda
    auto recordSecondaryDependencies = [this](Value *V, Value *Dependent) {
      if (PendingValues.count(V)) {
        for (Value *Pending : PendingValues[V]) {
          // Don't make values dependent on themselves.
          if (Pending == Dependent)
            continue;
          DEBUG(dbgs() << "  Recording secondary dependency of "
                       << Dependent->getName() << " on " << Pending->getName()
                       << "\n");
          DependentValues[Pending].insert(Dependent);
          PendingValues[Dependent].insert(Pending);
        }
      }
    };

    LocalPointerInfo &Info = LocalMap[V];
    if (Info.getAnalyzed()) {
      recordSecondaryDependencies(V, Dependent);
      return Info;
    }

    // If analysis of this value is currently in progress, add an entry
    // to the pending values map so that its analysis can be appended to
    // the Dependent value when it is complete.
    if (InProgressValues.count(V)) {
      DEBUG(dbgs() << "  Recording dependency of " << Dependent->getName()
                   << " on " << V->getName() << "\n");
      DependentValues[V].insert(Dependent);
      PendingValues[Dependent].insert(V);
      return Info;
    }

    // Otherwise, we can attempt analysis.
    analyzeValue(V);

    // It is still possible that the analysis of V could not be completed
    // because it depends on a value that was pending. If so, mark our
    // Dependent value as also being dependent on the results for which
    // the analysis of V is waiting.
    recordSecondaryDependencies(V, Dependent);

    return Info;
  }

  void collectSourceOperandInfo(Value *V, LocalPointerInfo &Info) {
    // In each case, the call to analyzeValue performs a check which will
    // prevent infinite recursion if we track back to a block we've
    // already visited.
    if (auto *PN = dyn_cast<PHINode>(V)) {
      for (Value *InVal : PN->incoming_values()) {
        LocalPointerInfo &InLPI = tryGetLocalPointerInfo(InVal, V);
        Info.merge(InLPI);
      }
      return;
    }
    if (auto *Sel = dyn_cast<SelectInst>(V)) {
      Value *TV = Sel->getTrueValue();
      LocalPointerInfo &TrueLPI = tryGetLocalPointerInfo(TV, V);
      Info.merge(TrueLPI);
      Value *FV = Sel->getFalseValue();
      LocalPointerInfo &FalseLPI = tryGetLocalPointerInfo(FV, V);
      Info.merge(FalseLPI);
      return;
    }
    if (isa<CastInst>(V) || isa<BitCastOperator>(V) ||
        isa<PtrToIntOperator>(V)) {
      Value *SrcVal = cast<User>(V)->getOperand(0);
      LocalPointerInfo &SrcLPI = tryGetLocalPointerInfo(SrcVal, V);
      // If this is a bitcast that would be a valid way to access element
      // zero of any type known to be aliased by SrcVal, then record this
      // as an element access rather than merging the incoming value's aliases.
      if (auto *BC = dyn_cast<BitCastInst>(V)) {
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

  bool analyzeElementAccess(GEPOperator *GEP, LocalPointerInfo &Info) {
    auto *Int8PtrTy = llvm::Type::getInt8PtrTy(GEP->getContext());

    // If the base pointer is an i8* we need to analyze this as a
    // byte-flattened GEP.
    Value *BasePointer = GEP->getPointerOperand();
    if (BasePointer->getType() == Int8PtrTy)
      return analyzeByteFlattenedGEPAccess(GEP, Info);

    // A GEP with only one index argument is a special case where a pointer
    // is being used as an array. That doesn't get us a pointer to an element
    // within an aggregate type.
    if (GEP->getNumIndices() == 1)
      return false;

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
      return false;
    uint64_t Idx = LastArg->getLimitedValue();

    // Add this information to the local pointer information for the GEP.
    Info.addElementPointee(IndexedTy, Idx);

    return true;
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

    // If we can't compute a constant offset, we won't be able to
    // figure out which element is being accessed.
    unsigned BitWidth = DL.getPointerSizeInBits();
    APInt APOffset(BitWidth, 0);
    if (!GEP->accumulateConstantOffset(DL, APOffset))
      return false;
    uint64_t Offset = APOffset.getLimitedValue();

    // Check for types that the base pointer is known to alias.
    LocalPointerInfo &BaseLPI = getLocalPointerInfo(BasePointer);
    for (auto *AliasTy : BaseLPI.getPointerTypeAliasSet()) {
      if (!AliasTy->isPointerTy())
        continue;
      if (analyzePossibleOffsetAggregateAccess(
              GEP, AliasTy->getPointerElementType(), Offset, Info))
        return true;
    }
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
    LocalPointerInfo &SrcLPI = getLocalPointerInfo(Src);
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
    DEBUG(dbgs() << "dtrans: Analyzing allocation call.\n  " << *CI << "\n");
    SmallPtrSet<llvm::PointerType *, 4> CastTypes;
    SmallPtrSet<Value *, 4> VisitedUsers;
    collectAllocatedPtrBitcasts(CI, CastTypes, VisitedUsers);
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
  void
  collectAllocatedPtrBitcasts(Instruction *I,
                              SmallPtrSetImpl<llvm::PointerType *> &CastTypes,
                              SmallPtrSetImpl<Value *> &VisitedUsers) {
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

        DEBUG(dbgs() << "  Associated bitcast: " << *BI << "\n");

        // Save the type information.
        CastTypes.insert(PtrTy);
        continue;
      }
      // If the user is a PHI node or a select instruction, we need to follow
      // the users of that instruction.
      if (isa<PHINode>(U) || isa<SelectInst>(U))
        collectAllocatedPtrBitcasts(cast<Instruction>(U), CastTypes,
                                    VisitedUsers);
      // If the user is a store instruction, treat the alias types of the
      // destination pointer as implicit casts.
      if (auto *Store = dyn_cast<StoreInst>(U))
        inferAllocatedTypesFromStoreInst(Store, CastTypes);
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
  void inferAllocatedTypesFromStoreInst(
      StoreInst *Store, SmallPtrSetImpl<llvm::PointerType *> &Types) {
    // Get the local pointer info for the destination address.
    Value *DestPtr = Store->getPointerOperand();
    analyzeValue(DestPtr);
    LocalPointerInfo &DestInfo = LocalMap[DestPtr];

    // For each type aliased by the destination, if the type is a pointer
    // add the type that it points to to the Types set.
    for (auto *AliasTy : DestInfo.getPointerTypeAliasSet())
      if (AliasTy->isPointerTy() &&
          AliasTy->getPointerElementType()->isPointerTy())
        Types.insert(cast<PointerType>(AliasTy->getPointerElementType()));
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
    case Intrinsic::memmove:
      analyzeMemcpyOrMemmove(I);
      return;
    }

    // The intrinsic was not handled, mark all parameters as unhandled uses.
    for (Value *Arg : I.arg_operands()) {
      if (isValueOfInterest(Arg)) {
        DEBUG(dbgs() << "dtrans-safety: Unhandled use --  IntrinsicInst: " << I
                     << " value passed as argument.\n"
                     << "  " << *Arg << "\n");
        setValueTypeInfoSafetyData(Arg, dtrans::UnhandledUse);
      }
    }
  }

  void visitCallInst(CallInst &CI) {
    Function *F = CI.getCalledFunction();

    // If the called function is a known allocation function, we need to
    // analyze the allocation.
    dtrans::AllocKind Kind = dtrans::getAllocFnKind(F, TLI);
    if (Kind == dtrans::AK_NotAlloc && DTAA.isMallocPostDom(F))
      Kind = dtrans::AK_Malloc;
    if (Kind != dtrans::AK_NotAlloc) {
      analyzeAllocationCall(CI, Kind);
      return;
    }

    // If this is a call to the "free" lib function, we don't need
    // to analyze the argument.
    if (dtrans::isFreeFn(F, TLI) || DTAA.isFreePostDom(F))
      return;

    // For all other calls, if a pointer to an aggregate type is passed as an
    // argument to a function in a form other than its dominant type, the
    // address has escaped. Also, if a pointer to a field is passed as an
    // argument to a function, the address of the field has escaped.
    // FIXME: Try to resolve indirect calls.
    bool IsFnLocal = F ? !F->isDeclaration() : false;
    for (Value *Arg : CI.arg_operands()) {
      if (!isValueOfInterest(Arg))
        continue;

      LocalPointerInfo &LPI = LPA.getLocalPointerInfo(Arg);

      if (LPI.pointsToSomeElement()) {
        DEBUG(dbgs() << "dtrans-safety: Field address taken -- "
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
      // worry about the types individually.
      auto *ArgTy = Arg->getType();
      for (auto *AliasTy : LPI.getPointerTypeAliasSet()) {
        if (!DTInfo.isTypeOfInterest(AliasTy))
          continue;
        if (IsFnLocal && (AliasTy == ArgTy))
          continue;
        DEBUG(dbgs() << "dtrans-safety: Address taken -- "
                     << "pointer to aggregate passed as argument:\n"
                     << "  " << CI << "\n  " << *Arg << "\n");

        setBaseTypeInfoSafetyData(AliasTy, dtrans::AddressTaken);
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
        DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
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
      DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
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
          DEBUG(dbgs() << "dtrans-safety: Whole structure reference:\n"
                       << "  " << I << "\n");
          setBaseTypeInfoSafetyData(AliasTy, dtrans::WholeStructureReference);
        } else {
          DEBUG(dbgs() << "dtrans-safety: Mismatched element access -- "
                       << "  bad type for element zero load:\n"
                       << "  " << I << "\n");
          setBaseTypeInfoSafetyData(AliasTy, dtrans::MismatchedElementAccess);
        }
      }
      // We expect that all cases will either find a pointer to a pointer alias
      // or will fall through to the base type handling. If neither of those
      // happen, we can't handle this load.
      if (!SourceIsPtrToPtr && !BaseTypeFound) {
        DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
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
          DEBUG(dbgs() << "dtrans-safety: Unsafe pointer store:\n");
          if (I != nullptr)
            DEBUG(dbgs() << "  " << *I << "\n");
          else
            DEBUG(dbgs() << " " << *ValOperand << " -> " << *PtrOperand
                         << " \n");
          setValueTypeInfoSafetyData(ValOperand, dtrans::UnsafePointerStore);
          setValueTypeInfoSafetyData(PtrOperand, dtrans::UnsafePointerStore);
        }
      }
    } else if (PtrLPI.canAliasToAggregatePointer()) {
      // If we get here the value operand is not a pointer to an aggregate
      // type, but the pointer operand is. Unless the value operand is a
      // null pointer, this is a bad store.
      if (!isa<ConstantPointerNull>(ValOperand)) {
        DEBUG(dbgs() << "dtrans-safety: Unsafe pointer store:\n");
        if (I != nullptr)
          DEBUG(dbgs() << "  " << *I << "\n");
        else
          DEBUG(dbgs() << " " << *ValOperand << " -> " << *PtrOperand << " \n");
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
      DEBUG(dbgs() << "dtrans-safety: Whole structure reference:\n"
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
        DEBUG(dbgs() << "dtrans-safety: Field address taken:\n"
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
      DEBUG(dbgs() << "dtrans-safety: Ambiguous GEP:\n"
                   << "  " << I << "\n");
      DEBUG(SrcLPI.dump());
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
      if (isInt8Ptr(Src) || I.getNumIndices() != 1) {
        DEBUG(dbgs() << "dtrans-safety: Bad pointer manipulation:\n"
                     << "  " << I << "\n");
        setAllAliasedTypeSafetyData(SrcLPI, dtrans::BadPtrManipulation);
      }
    }

    // Otherwise, the GEP instruction is safe.
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
      DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
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
      DEBUG(dbgs() << "dtrans-safety: Whole structure reference -- "
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
      DEBUG(dbgs() << "dtrans-safety: Address taken -- "
                   << "Non-typed address is returned by function: "
                   << I.getParent()->getParent()->getName() << "\n");
      setBaseTypeInfoSafetyData(AliasTy, dtrans::AddressTaken);
    }

    // If the value returned is a pointer to an element within an aggregate
    // type we also need to note that since it can have implications on
    // field access tracking.
    if (LPI.pointsToSomeElement()) {
      for (auto PointeePair : LPI.getElementPointeeSet()) {
        DEBUG(dbgs() << "dtrans-safety: Field address taken -- "
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

    DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
                 << "Type used by a stack variable:\n"
                 << "  " << I << "\n");
    // TODO: Set specific safety info.
    setBaseTypeInfoSafetyData(Ty, dtrans::UnhandledUse);
  }

  void visitBinaryOperator(BinaryOperator &I) {
    // Binary operator analysis will be implemented as needed.
    // For now, unimplemented operators will cause values to be marked
    // as unhandled use.
    switch (I.getOpcode()) {
    case Instruction::Sub:
      analyzeSub(I);
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
      DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
                   << "Type returned by an unmodeled instruction:\n"
                   << "  " << I << "\n");
      setBaseTypeInfoSafetyData(Ty, dtrans::UnhandledUse);
    }

    for (Value *Arg : I.operands()) {
      if (isValueOfInterest(Arg)) {
        DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
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
          DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
                       << "Unexpected global variable usage:\n"
                       << "  " << GV << "\n");
          setBaseTypeInfoSafetyData(GVTy, dtrans::UnhandledUse);
          continue;
        }
        if (GVElemTy->isPointerTy()) {
          DEBUG(dbgs() << "dtrans-safety: Global pointer\n"
                       << "  " << GV << "\n");
          setBaseTypeInfoSafetyData(GVTy, dtrans::GlobalPtr);
          Constant *Initializer = GV.getInitializer();
          analyzeUnsafePointerStores(nullptr, Initializer, &GV);
        } else {
          DEBUG(dbgs() << "dtrans-safety: Global instance\n"
                       << "  " << GV << "\n");
          setBaseTypeInfoSafetyData(GVTy, dtrans::GlobalInstance);
          // The local linkage check should guarantee a unique and definitive
          // initializer.
          assert(GV.hasUniqueInitializer() && GV.hasDefinitiveInitializer() &&
                 !GV.isExternallyInitialized());
          Constant *Initializer = GV.getInitializer();
          if (!isa<ConstantAggregateZero>(Initializer) &&
              !isa<UndefValue>(Initializer)) {
            DEBUG(dbgs() << "dtrans-safety: Has initializer list\n"
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
    if (V->getType()->isPointerTy() ||
        ((isa<PtrToIntInst>(V) || isa<PtrToIntOperator>(V)) &&
         isPtrSizeInt(V))) {
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

  // Analyze a structure definition, independent of its use in any
  // instruction. This checks for basic issues like structure nesting
  // and empty structures.
  void analyzeStructureType(llvm::StructType *Ty) {
    // Add this to our type info list.
    dtrans::TypeInfo *TI = DTInfo.getOrCreateTypeInfo(Ty);

    // Check to see if this structure is known to be a system type.
    if (dtrans::isSystemObjectType(Ty)) {
      DEBUG(dbgs() << "dtrans-safety: System object:\n  " << *Ty << "\n");
      TI->setSafetyData(dtrans::SystemObject);
    }

    // Get the number of fields in the structure.
    unsigned NumElements = Ty->getNumElements();

    // If the structure is empty, we can't need to try to optimize it.
    if (NumElements == 0) {
      DEBUG(dbgs() << "dtrans-safety: No fields in structure:\n  " << *Ty
                   << "\n");
      TI->setSafetyData(dtrans::NoFieldsInStruct);
      return;
    }

    // I think opaque structures will report having zero fields.
    assert(!Ty->isOpaque());

    // It isn't clear to me under what circumstances a type will be reported
    // as unsized, but if one is we definitely can't do anything with it.
    if (!Ty->isSized()) {
      DEBUG(dbgs() << "dtrans-safety: Unhandled use -- non-sized structure\n  "
                   << *Ty << "\n");
      TI->setSafetyData(dtrans::UnhandledUse);
    }

    // Walk the fields in the structure looking for nested structures.
    for (llvm::Type *ElementTy : Ty->elements()) {
      // If the element is a non-pointer structure, set the nested type
      // safety conditions.
      if (ElementTy->isStructTy()) {
        DEBUG(dbgs() << "dtrans-safety: Nested structure\n"
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
          DEBUG(dbgs() << "dtrans-safety: Nested structure\n"
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
    DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
                 << "unsafe cast of aliased pointer:\n"
                 << "  " << I << "\n");
    setValueTypeInfoSafetyData(I.getOperand(0), dtrans::BadCasting);
    if (DTInfo.isTypeOfInterest(DestTy))
      setValueTypeInfoSafetyData(&I, dtrans::BadCasting);
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
        DEBUG(dbgs() << "dtrans-fsv: " << *(StInfo->getLLVMType()) << " ["
                     << Count << "] ");
        DEBUG(NV->printAsOperand(dbgs()));
        FI.processNewSingleValue(NV);
        DEBUG(dbgs() << (FI.isMultipleValue() ? " <MULTIPLE>\n" : "\n"));
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
        DEBUG(dbgs() << "dtrans-safety: Bad casting -- "
                     << "allocation cast to multiple types:\n  " << CI << "\n");
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
        DEBUG(dbgs() << "dtrans-safety: Volatile data:\n");
        DEBUG(dbgs() << "  " << I << "\n");
        setBaseTypeInfoSafetyData(ParentTy, dtrans::VolatileData);
      }

      if (auto *CompTy = cast<CompositeType>(ParentTy)) {
        llvm::Type *FieldTy = CompTy->getTypeAtIndex(PointeePair.second);

        // If this field is an aggregate, and this is not a nested
        // element zero access, mark this as a whole structure reference.
        if (FieldTy->isAggregateType() && FieldTy == ValTy) {
          DEBUG(dbgs() << "dtrans-safety: Whole structure reference:\n");
          DEBUG(dbgs() << "  " << I << "\n");
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
          DEBUG(dbgs() << "dtrans-safety: Mismatched element access:\n");
          DEBUG(dbgs() << "  " << I << "\n");
          setBaseTypeInfoSafetyData(ParentTy, dtrans::MismatchedElementAccess);
        }

        if (ParentTy->isStructTy()) {
          auto *ParentStInfo =
              cast<dtrans::StructInfo>(DTInfo.getOrCreateTypeInfo(ParentTy));
          dtrans::FieldInfo &FI = ParentStInfo->getField(PointeePair.second);
          if (IsLoad)
            FI.setRead(true);
          else {
            DEBUG(dbgs() << "dtrans-fsv: " << *(ParentStInfo->getLLVMType())
                         << " [" << PointeePair.second << "] ");
            if (auto *ConstVal = dyn_cast<llvm::Constant>(WriteVal)) {
              DEBUG(ConstVal->printAsOperand(dbgs()));
              FI.processNewSingleValue(ConstVal);
              DEBUG(dbgs() << (FI.isMultipleValue() ? " <MULTIPLE>\n" : "\n"));
            } else {
              DEBUG(dbgs() << "<MULTIPLE>\n");
              FI.setMultipleValue();
            }
            FI.setWritten(true);
          }
        }
        // TODO: Track array element access?
      } else {
        // Otherwise, the parent type is either a vector or a pointer (which
        // would have been indexed as an array).
        // TODO: Handle this case.
        DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
                        "unimplemented load/store:\n");
        DEBUG(dbgs() << "  " << I << "\n");
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
      DEBUG(dbgs() << "dtrans-safety: Unsafe pointer merge -- "
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
        DEBUG(dbgs() << "dtrans-safety: Unsafe pointer merge -- "
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
    if (isValueMultipleOfSize(AllocSizeVal, ElementSize) ||
        isValueMultipleOfSize(AllocCountVal, ElementSize))
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
      if ((isValueMultipleOfSize(AllocSizeVal, ArrElementSize) &&
           isValueMultipleOfSize(AllocCountVal, NumArrElements)) ||
          (isValueMultipleOfSize(AllocCountVal, ArrElementSize) &&
           isValueMultipleOfSize(AllocSizeVal, NumArrElements)))
        return;
    }

    // Otherwise, we must assume the size arguments are not acceptable.
    DEBUG(dbgs() << "dtrans-safety: Bad alloc size:\n"
                 << "  " << CI << "\n");
    setBaseTypeInfoSafetyData(Ty, dtrans::BadAllocSizeArg);
  }

  // This helper function checks a value to see if it is either (a) a constant
  // whose value is a multiple of the specified size, or (b) an integer
  // multiplication operator where either operand is a constant multiple of the
  // specified size.
  bool isValueMultipleOfSize(Value *Val, uint64_t Size) {
    if (!Val)
      return false;

    // Is it a constant?
    if (auto *ConstVal = dyn_cast<ConstantInt>(Val)) {
      uint64_t ConstSize = ConstVal->getLimitedValue();
      return ((ConstSize % Size) == 0);
    }
    // Is it a mul?
    Value *LHS;
    Value *RHS;
    if (PatternMatch::match(Val,
                            PatternMatch::m_Mul(PatternMatch::m_Value(LHS),
                                                PatternMatch::m_Value(RHS)))) {
      return (isValueMultipleOfSize(LHS, Size) ||
              isValueMultipleOfSize(RHS, Size));
    }
    // Otherwise, it's not what we needed.
    return false;
  }

  // This helper function checks if a value is a constant integer equal to
  // 'Size'.
  bool isValueEqualToSize(Value *Val, uint64_t Size) {
    if (!Val)
      return false;

    if (auto *ConstVal = dyn_cast<ConstantInt>(Val)) {
      uint64_t ConstSize = ConstVal->getLimitedValue();
      return ConstSize == Size;
    }

    return false;
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
    DEBUG(dbgs() << "dtrans: Analyzing memset call:\n  " << I << "\n");

    assert(I.getNumArgOperands() >= 2);
    auto *DestArg = I.getArgOperand(0);
    Value *SetSize = I.getArgOperand(2);

    // A memset of 0 bytes will not affect the safety of any data structure.
    if (isValueEqualToSize(SetSize, 0))
      return;

    if (!isValueOfInterest(DestArg))
      return;

    LocalPointerInfo &DstLPI = LPA.getLocalPointerInfo(DestArg);
    auto *DestParentTy = DstLPI.getDominantAggregateTy();
    if (!DestParentTy) {
      if (isAliasSetOverloaded(DstLPI.getPointerTypeAliasSet())) {
        // Cannot do anything for an aliased type. Declare them
        // to be unsafe.
        DEBUG(dbgs() << "dtrans-safety: Ambiguous pointer target -- "
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
        // Try to determine if a set of fields in a structure is being written.
        unsigned int FirstField = 0;
        unsigned int LastField = 0;
        if (analyzePartialStructUse(StructTy, FieldNum, SetSize, &FirstField,
                                    &LastField)) {
          auto *ParentTI = DTInfo.getOrCreateTypeInfo(StructTy);

          // If not all members of the structure were set, mark it as
          // a partial write.
          if (!(FirstField == 0 &&
                LastField == (StructTy->getNumElements() - 1))) {
            DEBUG(dbgs() << "dtrans-safety: Memfunc partial write -- "
                         << "size is a subset of fields:\n"
                         << "  " << I << "\n");

            ParentTI->setSafetyData(dtrans::MemFuncPartialWrite);
          }
          markStructFieldsWritten(ParentTI, FirstField, LastField);
          // Conservatively mark all fields as having been written by the
          // memset.  We can improve this analysis later.
          markAllFieldsMultipleValue(ParentTI);
        } else {
          // The size could not be matched to the fields of the structure.
          DEBUG(dbgs() << "dtrans-safety: Bad memfunc size -- "
                       << "size does not equal member field type(s) size:\n"
                       << "  " << I << "\n");

          setBaseTypeInfoSafetyData(StructTy, dtrans::BadMemFuncSize);
        }

        return;
      }

      // The pointer to member was not able to be analyzed. It could be a member
      // of an array type, or the element pointee set contained multiple
      // entries.
      if (DstLPI.getElementPointeeSet().size() != 1) {
        DEBUG(dbgs() << "dtrans-safety: Ambiguous pointer target -- "
                     << "Pointer to member with multiple potential target "
                        "members:\n"
                     << "  " << I << "\n");

        setValueTypeInfoSafetyData(DestArg, dtrans::AmbiguousPointerTarget);
      } else {
        // This could be extended in the future to handle the case that it was
        // member of an array.
        DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
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
    if (isValueMultipleOfSize(SetSize, ElementSize)) {
      // It is a safe use. Mark all the fields as being written.
      markAllFieldsWritten(ParentTI);
      return;
    }

    // Consider the case where a portion of a structure is being set, starting
    // from the 1st field.
    if (DestPointeeTy->isStructTy()) {
      StructType *StructTy = cast<StructType>(DestPointeeTy);
      unsigned int FirstField = 0;
      unsigned int LastField = 0;
      if (analyzePartialStructUse(StructTy, 0, SetSize, &FirstField,
                                  &LastField)) {

        // It's possible the write covered all the fields, but excluded any
        // padding after the last element, so check whether it was a partial
        // write or not.
        if (!(FirstField == 0 &&
              LastField == (StructTy->getNumElements() - 1))) {
          DEBUG(dbgs() << "dtrans-safety: Memfunc partial write -- "
                       << "size is a subset of fields:\n"
                       << "  " << I << "\n");

          ParentTI->setSafetyData(dtrans::MemFuncPartialWrite);
        }

        markStructFieldsWritten(ParentTI, FirstField, LastField);
        return;
      }
    }

    DEBUG(dbgs() << "dtrans-safety: Bad memfunc size -- "
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
  void analyzeMemcpyOrMemmove(IntrinsicInst &I) {
    DEBUG(dbgs() << "dtrans: Analyzing memcpy/memmove call:\n  " << I << "\n");
    assert(I.getNumArgOperands() >= 2);

    auto *DestArg = I.getArgOperand(0);
    auto *SrcArg = I.getArgOperand(1);
    Value *SetSize = I.getArgOperand(2);

    // A memcpy/memmove of 0 bytes will not affect the safety of any data
    // structure.
    if (isValueEqualToSize(SetSize, 0))
      return;

    bool DestOfInterest = isValueOfInterest(DestArg);
    bool SrcOfInterest = isValueOfInterest(SrcArg);
    if (!DestOfInterest && !SrcOfInterest) {
      return;
    } else if (!SrcOfInterest || !DestOfInterest) {
      DEBUG(dbgs() << "dtrans-safety: Bad memfunc manipulation --  "
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
        DEBUG(dbgs() << "dtrans-safety: Ambiguous pointer target -- "
                     << "Aliased type for destination operand:\n"
                     << "  " << I << "\n");

        setAllAliasedTypeSafetyData(DstLPI, dtrans::AmbiguousPointerTarget);
        setAllAliasedTypeSafetyData(SrcLPI, dtrans::BadMemFuncManipulation);
        return;
      } else if (!SrcParentTy &&
                 isAliasSetOverloaded(SrcLPI.getPointerTypeAliasSet())) {
        DEBUG(dbgs() << "dtrans-safety: Bad memfunc manipulation -- "
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
      DEBUG(dbgs() << "dtrans-safety: Bad memfunc manipulation -- "
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
          DEBUG(dbgs() << "dtrans-safety: Ambiguous pointer target -- "
                       << "Pointer to member with multiple potential target "
                          "members:\n"
                       << "  " << I << "\n");

          setValueTypeInfoSafetyData(DestArg, dtrans::AmbiguousPointerTarget);
          setValueTypeInfoSafetyData(SrcArg, dtrans::AmbiguousPointerTarget);
        } else {
          // This could be extended in the future to handle the case that it was
          // member of an array.
          DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
                       << "Pointer to member of array type:\n"
                       << "  " << I << "\n");

          setValueTypeInfoSafetyData(DestArg, dtrans::UnhandledUse);
          setValueTypeInfoSafetyData(SrcArg, dtrans::UnhandledUse);
        }

        return;
      }

      bool SrcSimple =
          isSimpleStructureMember(SrcLPI, &SrcStructTy, &SrcFieldNum);

      // It is probably safe to copy from one set of fields to a different set
      // of fields in the structure, if the data types for each source and
      // destination element match, but to keep things simple for the
      // transformations, we will currently require the same source and
      // destination types and fields when processing the pointer to member
      // case.
      if (DstSimple == SrcSimple && DstStructTy == SrcStructTy &&
          DstFieldNum == SrcFieldNum) {
        unsigned int FirstField = 0;
        unsigned int LastField = 0;
        if (analyzePartialStructUse(DstStructTy, DstFieldNum, SetSize,
                                    &FirstField, &LastField)) {
          auto *ParentTI = DTInfo.getOrCreateTypeInfo(DstStructTy);

          // If the not all members of the structure were set, mark it as
          // a partial write.
          if (!(FirstField == 0 &&
                LastField == (DstStructTy->getNumElements() - 1))) {
            DEBUG(dbgs() << "dtrans-safety: Memfunc partial write -- "
                         << "size is a subset of fields:\n"
                         << "  " << I << "\n");

            ParentTI->setSafetyData(dtrans::MemFuncPartialWrite);
          }
          markStructFieldsWritten(ParentTI, FirstField, LastField);
        } else {
          // The size could not be matched to the fields of the structure.
          DEBUG(dbgs() << "dtrans-safety: Bad memfunc size -- "
                       << "size does not equal member field type(s) size:\n"
                       << "  " << I << "\n");

          setBaseTypeInfoSafetyData(DstStructTy, dtrans::BadMemFuncSize);
        }
      } else {
        DEBUG(dbgs() << "dtrans-safety: Bad memfunc manipulation -- "
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
    if (isValueMultipleOfSize(SetSize, ElementSize)) {
      // It is a safe use. Mark all the fields as being written.
      auto *ParentTI = DTInfo.getOrCreateTypeInfo(DestPointeeTy);
      markAllFieldsWritten(ParentTI);
      return;
    }

    // Consider the case where a portion of a structure is being set, starting
    // from the 1st field.
    if (DestPointeeTy->isStructTy()) {
      StructType *StructTy = cast<StructType>(DestPointeeTy);
      unsigned int FirstField = 0;
      unsigned int LastField = 0;
      if (analyzePartialStructUse(StructTy, 0, SetSize, &FirstField,
                                  &LastField)) {
        auto *ParentTI = DTInfo.getOrCreateTypeInfo(DestPointeeTy);
        // It's possible the write covered all the fields, but excluded any
        // padding after the last element, so check whether it was a partial
        // write or not.
        if (!(FirstField == 0 &&
              LastField == (StructTy->getNumElements() - 1))) {
          DEBUG(dbgs() << "dtrans-safety: Memfunc partial write -- "
                       << "size is a subset of fields:\n"
                       << "  " << I << "\n");

          ParentTI->setSafetyData(dtrans::MemFuncPartialWrite);
        }
        markStructFieldsWritten(ParentTI, FirstField, LastField);
        return;
      }
    }

    DEBUG(dbgs() << "dtrans-safety: Bad memfunc size -- "
                 << "size is not a multiple of type size:\n"
                 << "  " << I << "\n");

    setValueTypeInfoSafetyData(DestArg, dtrans::BadMemFuncSize);
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

  // Wrapper function for analyzing structure field access which prepares
  // parameters for that function.
  bool analyzePartialStructUse(StructType *StructTy, size_t FieldNum,
                               const Value *AccessSizeVal,
                               unsigned int *FirstField,
                               unsigned int *LastField) {
    if (!StructTy)
      return false;

    if (!AccessSizeVal)
      return false;

    auto *AccessSizeCI = dyn_cast<ConstantInt>(AccessSizeVal);
    if (!AccessSizeCI)
      return false;

    uint64_t AccessSize = AccessSizeCI->getLimitedValue();
    assert(FieldNum < StructTy->getNumElements());

    return analyzeStructFieldAccess(StructTy, FieldNum, AccessSize, FirstField,
                                    LastField);
  }

  // Helper to analyze a pointer-to-member usage to determine if only a
  // specific subset of the structure fields of \p StructTy, starting from \p
  // FieldNum and extending by \p AccessSize bytes of the structure are
  // touched.
  //
  // Return 'true' if it can be resolved to precisely match one or more
  // adjacent fields starting with the field number identified in the 'LPI'.
  // If so, also set the starting index into 'FirstField' and the ending index
  // of affected fields into 'LastField'. Otherwise, return 'false'.
  bool analyzeStructFieldAccess(StructType *StructTy, size_t FieldNum,
                                uint64_t AccessSize, unsigned int *FirstField,
                                unsigned int *LastField) {
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

    *FirstField = FieldNum;
    *LastField = LF;
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
        DEBUG(dbgs() << "dtrans-fsv: " << *(StInfo->getLLVMType()) << " [" << I
                     << "] ");
        if (ConstVal->getType() == FieldTy) {
          DEBUG(ConstVal->printAsOperand(dbgs()));
          FI.processNewSingleValue(ConstVal);
          DEBUG(dbgs() << (FI.isMultipleValue() ? " <MULTIPLE>\n" : "\n"));
        } else {
          DEBUG(dbgs() << "<MULTIPLE>\n");
          FI.setMultipleValue();
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
        DEBUG(dbgs() << "dtrans-fsv: " << *(StInfo->getLLVMType()) << " ["
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
 
    if (!pointerAliasSetsAreEqual(LHSLPI.getPointerTypeAliasSet(),
                                  RHSLPI.getPointerTypeAliasSet())) {
      DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
                   << "sub instruction operands do not match:\n"
                   << "  " << I << "\n");
      setValueTypeInfoSafetyData(I.getOperand(0), dtrans::UnhandledUse);
      setValueTypeInfoSafetyData(I.getOperand(1), dtrans::UnhandledUse);
    }

    if (LHSLPI.pointsToSomeElement()) {
      DEBUG(dbgs() << "dtrans-safety: Bad pointer manipulation -- "
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
      DEBUG(dbgs() << "dtrans-safety: Bad pointer manipulation -- "
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
        DEBUG(dbgs() << "dtrans-safety: Unhandled use -- "
                     << "Type used by an unmodeled binary operator:\n"
                     << "  " << I << "\n");
        setValueTypeInfoSafetyData(Arg, dtrans::UnhandledUse);
      }
    }
  }

  // In many cases we need to set safety data based on a value that
  // was derived from a pointer to a type of interest, via a bitcast
  // or a ptrtoint cast. In those cases, this function is called to
  // propogate safety data to the interesting type.
  void setValueTypeInfoSafetyData(Value *V, dtrans::SafetyData Data) {
    // In some cases this function might have been called for multiple
    // operands, not all of which we are actually tracking.
    if (!isValueOfInterest(V))
      return;

    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(V);
    setAllAliasedTypeSafetyData(LPI, Data);

    // If the value is a pointer to an element in some aggregate type
    // set the safety info for that type also.
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
      BaseTy = cast<PointerType>(BaseTy)->getElementType();
    dtrans::TypeInfo *TI = DTInfo.getOrCreateTypeInfo(BaseTy);
    TI->setSafetyData(Data);
    if (!isCascadingSafetyCondition(Data))
      return;
    // Propagate this condition to any nested types.
    if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI)) {
      for (dtrans::FieldInfo &FI : StInfo->getFields()) {
        llvm::Type *FieldTy = FI.getLLVMType();
        // Propagate the safety condition if this field is an instance of
        // a type of interest, but not if it is merely a pointer to such
        // a type. Call setBaseTypeInfoSafetyData to handle additional levels
        // of nesting.
        if (!FieldTy->isPointerTy() && DTInfo.isTypeOfInterest(FieldTy))
          setBaseTypeInfoSafetyData(FieldTy, Data);
      }
    } else if (auto *ArrInfo = dyn_cast<dtrans::ArrayInfo>(TI)) {
      ArrInfo->setSafetyData(Data);
      llvm::Type *ElementTy = BaseTy->getArrayElementType();
      // Propagate the safety condition if this field is an instance of
      // a type of interest, but not if it is merely a pointer to such
      // a type. Call setBaseTypeInfoSafetyData to handle additional levels
      // of nesting.
      if (!ElementTy->isPointerTy() && DTInfo.isTypeOfInterest(ElementTy))
        setBaseTypeInfoSafetyData(ElementTy, Data);
    }
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

DTransAnalysisInfo::DTransAnalysisInfo() {}

DTransAnalysisInfo::~DTransAnalysisInfo() {
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
    default:
      llvm_unreachable("Missing cast for appropriate TypeInfo destruction");
    }
  }
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
  }

  // Set all aggregate fields conservatively as MultipleValue for now.
  for (auto *TI : type_info_entries()) {
    if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI))
      for (unsigned I = 0, E = StInfo->getNumFields(); I != E; ++I)
        if (StInfo->getField(I).getLLVMType()->isAggregateType())
          StInfo->getField(I).setMultipleValue();
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

  return false;
}

void DTransAnalysisInfo::printStructInfo(dtrans::StructInfo *SI) {
  outs() << "DTRANS_StructInfo:\n";
  outs() << "  LLVMType: " << *(SI->getLLVMType()) << "\n";
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
}

void DTransAnalysisInfo::reset() {
  // TODO: Release resources.
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
