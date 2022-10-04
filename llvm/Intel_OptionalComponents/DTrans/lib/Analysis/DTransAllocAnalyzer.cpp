//===-----DTransAllocAnalyzer.cpp - Allocation/Free function analyzer------===//
//
// Copyright (C) 2018-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/DTransAllocAnalyzer.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "dtransanalysis"

using namespace llvm;

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

namespace llvm {
namespace dtrans {


DTransAllocAnalyzer::DTransAllocAnalyzer(
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
    const Module &M)
    : GetTLI(GetTLI) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  parseListOptions(M);
#endif
  // TODO: OpaquePtr: Need to replace this and all uses of it for opaque
  // pointer support because it will match all pointers in that case.
  Int8PtrTy = Type::getInt8PtrTy(M.getContext(), 0 /*AS*/);
}

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
// Return the AllocKind, if 'Call' is post-dominated by a call to malloc() on
// all paths that do not include skip blocks. Otherwise, return AK_NotAlloc.
// Trivial wrappers are important special cases.
//
AllocKind DTransAllocAnalyzer::getMallocPostDomKind(const CallBase *Call) {
  // Try to find the called function, stripping away Bitcasts or looking
  // through GlobalAlias definitions, if necessary.
  const Function *F = dtrans::getCalledFunction(*Call);

  if (!F)
    // Check for allocation routine.
    // Indirect calls only support AK_UserMallocThis.
    return analyzeForIndirectStatus(Call, true) ? AK_UserMallocThis : AK_NotAlloc;

  auto IT = LocalMap.find(F);
  if (IT == LocalMap.end())
    return AK_NotAlloc;

  switch (IT->second) {
  default:
    return AK_NotAlloc;
  case AKS_Malloc:
    return AK_UserMalloc;
  case AKS_Malloc0:
    return AK_UserMalloc0;
  case AKS_MallocThis:
    return AK_UserMallocThis;
  }
  llvm_unreachable("Fully covered switch");
}

//
// Return FreeKind, if 'Call' is post-dominated by a call to free() on all paths
// that do not include skip blocks. Otherwise, return FK_NotFree. Trivial
// wrappers are important special cases.
//
FreeKind DTransAllocAnalyzer::getFreePostDomKind(const CallBase *Call) {
  // Try to find the called function, stripping away Bitcasts or looking
  // through GlobalAlias definitions, if necessary.
  const Function *F = dtrans::getCalledFunction(*Call);

  if (!F)
    // Check for deallocation routine.
    // Indirect calls only support FK_UserFreeThis.
    return analyzeForIndirectStatus(Call, false) ? FK_UserFreeThis : FK_NotFree;

  auto IT = LocalMap.find(F);
  if (IT == LocalMap.end())
    return FK_NotFree;

  switch (IT->second) {
  default:
    return FK_NotFree;

  case AKS_Free:
    return FK_UserFree;
  case AKS_Free0:
    return FK_UserFree0;
  case AKS_FreeThis:
    return FK_UserFreeThis;
  }

  llvm_unreachable("Fully covered switch");
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
  if (auto *Call = dyn_cast<CallBase>(V))
    if (auto Kind = dtrans::getAllocFnKind(Call, GetTLI(*Call->getFunction())))
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
  if (const auto *Call = dyn_cast<CallBase>(RV)) {
    const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
    auto Kind = dtrans::getAllocFnKind(Call, TLI);
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
    const TargetLibraryInfo &TLI = GetTLI(*GV->getFunction());
    CallBase *Call = nullptr;
    dtrans::AllocKind Kind = dtrans::AK_NotAlloc;
    if (analyzeGEPAsAllocationResult(GV, TLI, &Call, &Kind))
      return true;
  }
  return false;
}

//
// Return an appropriate AllocStatus based on which argument is the allocation
// size if 'F' is isMallocPostDom(). Otherwise, return AKS_Unknown.
//
DTransAllocAnalyzer::AllocStatus
DTransAllocAnalyzer::analyzeForMallocStatus(Function *F) {
  if (F == nullptr || F->arg_size() == 0)
    return AKS_Unknown;
  LLVM_DEBUG(dbgs() << "Analyzing for MallocPostDom " << F->getName() << "\n");

  // Make sure that VisitedBlocks and SkipTestBlocks are clear before
  // visitNullPtrBlocks() is called. SkipTestBlocks are valid until we
  // return from this function.  VisitedBlocks can be cleared immediately
  // after visitNullPtrBlocks() is run.
  VisitedBlocks.clear();
  SkipTestBlocks.clear();
  // Allow user-defined allocation function with 'this' pointer as a first
  // argument.

  auto IsThisPtrAndSizeArgs = [&](Function *F) {
    if (F->arg_size() != 2)
      return false;
    if (F->arg_begin()->getNumUses() != 0)
      return false;
    auto PtrType = dyn_cast<PointerType>(F->arg_begin()->getType());
    if (!PtrType || !PtrType->getElementType()->isAggregateType())
      return false;
    Argument *Arg1 = F->arg_begin() + 1;
    if (!Arg1->getType()->isIntegerTy())
      return false;
    return true;
  };

  // Identify the type of user allocation. AKS_Malloc0 could also be
  // supported as an allowed type here, but for now that type is only
  // used for a special case when the set is expanded to include callers of the
  // user allocation functions.
  AllocStatus AllocType = AKS_Unknown;
  if (F->arg_size() == 1 && F->arg_begin()->getType()->isIntegerTy())
    AllocType = AKS_Malloc;
  else if (IsThisPtrAndSizeArgs(F))
    AllocType = AKS_MallocThis;
  else
    return AKS_Unknown;

  visitNullPtrBlocks(F);
  VisitedBlocks.clear();
  bool rv = false;
  for (BasicBlock &BB : *F)
    if (auto RI = dyn_cast<ReturnInst>(BB.getTerminator())) {
      Value *RV = RI->getReturnValue();
      if (RV == nullptr) {
        LLVM_DEBUG(dbgs() << "Not MallocPostDom " << F->getName()
                          << " Return is nullptr\n");
        return AKS_Unknown;
      }
      if (!returnValueIsMallocAddress(RV, &BB)) {
        LLVM_DEBUG(dbgs() << "Not MallocPostDom " << F->getName()
                          << " Return is not malloc address\n");
        return AKS_Unknown;
      }
      rv = true;
    }
  if (rv)
    LLVM_DEBUG(dbgs() << "Is MallocPostDom " << F->getName() << "\n");
  else
    LLVM_DEBUG(dbgs() << "Not MallocPostDom " << F->getName()
                      << " No malloc address returned\n");
  return rv ? AllocType : AKS_Unknown;
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
bool DTransAllocAnalyzer::analyzeForIndirectStatus(const CallBase *Call,
                                                   bool Malloc) {

  if (Call->arg_size() < 2)
    return false;

  // First argument is 'this' pointer.
  Type *ObjType = Call->getArgOperand(0)->getType();
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
  // TODO: OpaquePtr: Need implement method of checking argument operand type
  // without comparison to i8*.
  if (Malloc ? !Call->getArgOperand(1)->getType()->isIntegerTy(32) &&
                   !Call->getArgOperand(1)->getType()->isIntegerTy(64)
             : Call->getArgOperand(1)->getType() != Int8PtrTy)
    return false;

  // Search for definition of called Value
  auto *Callee = dyn_cast<LoadInst>(Call->getCalledOperand());
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
  if (!ObjCast || ObjCast->getOperand(0) != Call->getArgOperand(0))
    return false;

  return true;
}

//
// Return true if 'BB' has a call to free().
//
bool DTransAllocAnalyzer::hasFreeCall(BasicBlock *BB) const {
  for (auto BI = BB->rbegin(), BE = BB->rend(); BI != BE;) {
    Instruction *I = &*BI++;
    if (auto *Call = dyn_cast<CallBase>(I)) {
      const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
      if (dtrans::isFreeFn(Call, TLI))
        return true;
    }
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
    const CallBase *Call = nullptr;
    for (auto &U : F.uses())
      if (const auto *Tmp = dyn_cast<CallBase>(U.getUser())) {
        Call = Tmp;
        break;
      }

    // No calls/invokes.
    if (!Call)
      continue;

    // Deal with free.
    const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
    if (dtrans::isFreeFn(Call, TLI)) {
      // Check all functions, calling free function F.
      for (auto &U : F.uses())
        if (isa<CallBase>(U.getUser())) {
          // Function calling F.
          auto *FreeCand =
              cast<Instruction>(U.getUser())->getParent()->getParent();
          AllocStatus Kind = analyzeForFreeStatus(FreeCand);
          if (Kind != AKS_Unknown)
            LocalMap[FreeCand] = Kind;
        }
      continue;
    }

    // Deal with malloc/new
    auto Kind = dtrans::getAllocFnKind(Call, TLI);
    if (Kind == dtrans::AK_Malloc || Kind == dtrans::AK_New) {
      // Check all functions, calling malloc/new function F.
      for (auto &U : F.uses())
        if (isa<CallBase>(U.getUser())) {
          // Function calling F.
          auto *MallocCand =
              cast<Instruction>(U.getUser())->getParent()->getParent();
          AllocStatus Kind = analyzeForMallocStatus(MallocCand);
          if (Kind != AKS_Unknown)
            LocalMap[MallocCand] = Kind;
        }
      continue;
    }
  }

  // Now check if among user-defined malloc/free callers we have a special kind
  // of alloc/free functions.
  std::map<const Function *, AllocStatus> TempLocalMap;
  for (auto &IT : LocalMap) {
    if (isAllocation(IT.second)) {
      TempLocalMap[IT.first] = IT.second;
      // Check all functions, calling user malloc/new function.
      for (auto &U : IT.first->uses()) {
        if (auto *I = dyn_cast<Instruction>(U.getUser())) {
          auto *SpecialMallocCand = I->getParent()->getParent();
          if (isMallocWithStoredMMPtr(SpecialMallocCand))
            TempLocalMap[SpecialMallocCand] = AKS_Malloc0;
        }
      }
    } else if (isFree(IT.second)) {
      TempLocalMap[IT.first] = IT.second;
      // Check all functions, calling user free function.
      for (auto &U : IT.first->uses()) {
        if (auto *I = dyn_cast<Instruction>(U.getUser())) {
          auto *SpecialFreeCand = I->getParent()->getParent();
          if (isFreeWithStoredMMPtr(SpecialFreeCand))
            TempLocalMap[SpecialFreeCand] = AKS_Free0;
        }
      }
    }
  }
  std::swap(TempLocalMap, LocalMap);
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
// Return an appropriate AllocStatus based on which argument is the pointer
// to be freed if 'F' is isFreePostDom(). Otherwise, return AKS_Unknown.
//
DTransAllocAnalyzer::AllocStatus
DTransAllocAnalyzer::analyzeForFreeStatus(Function *F) {
  if (F == nullptr || F->arg_size() == 0)
    return AKS_Unknown;

  // Make sure that VisitedBlocks and SkipTestBlocks are clear before
  // visitNullPtrBlocks() is called. SkipTestBlocks are valid until we
  // return from this function.  VisitedBlocks can be cleared immediately
  // after visitNullPtrBlocks() is run.
  VisitedBlocks.clear();
  SkipTestBlocks.clear();

  AllocStatus FreeType = AKS_Unknown;
  switch (F->arg_size()) {
  default:
    return AKS_Unknown;

  case 1:
    if (!F->arg_begin()->getType()->isPointerTy())
      return AKS_Unknown;

    visitNullPtrBlocks(F);
    VisitedBlocks.clear();
    FreeType = AKS_Free;
    break;

  case 2: {
    Type *ZeroArgType = F->getArg(0)->getType();
    Type *FirstArgType = F->getArg(1)->getType();
    if (ZeroArgType->isPointerTy() &&
        ZeroArgType->getPointerElementType()->isStructTy() &&
        FirstArgType->isPointerTy())
      FreeType = AKS_FreeThis;
    else if (ZeroArgType->isPointerTy())
      FreeType = AKS_Free0;
    else
      return AKS_Unknown;
    break;
  }
  }

  LLVM_DEBUG(dbgs() << "Analyzing for FreePostDom " << F->getName() << "\n");
  bool rv = false;
  for (BasicBlock &BB : *F)
    if (auto Ret = dyn_cast<ReturnInst>(BB.getTerminator())) {
      if (Ret->getReturnValue() != nullptr) {
        LLVM_DEBUG(dbgs() << "Not FreePostDom " << F->getName()
                          << " Return is not nullptr\n");
        return AKS_Unknown;
      }
      bool SeenFree = false;
      if (!isPostDominatedByFreeCall(&BB, SeenFree)) {
        LLVM_DEBUG(dbgs() << "Not FreePostDom " << F->getName()
                          << " Return is not post-dominated by call to free\n");
        return AKS_Unknown;
      }
      rv = SeenFree;
    }
  if (rv)
    LLVM_DEBUG(dbgs() << "Is FreePostDom " << F->getName() << "\n");
  else
    LLVM_DEBUG(dbgs() << "Not FreePostDom " << F->getName()
                      << " No return post-dominated by free\n");
  return rv ? FreeType : AKS_Unknown;
}

// Check that function is a special kind of user-defined malloc which stores
// memory manager pointer.
// Ex.:
// define internal nonnull i8* @candidateFunc(i64, %class.MemoryManager*) {
//  %3 = add i64 %0, 8
//  %4 = bitcast %class.MemoryManager* %1 to
//       i8* (%class.MemoryManager*, i64)***
//  %5 = load i8* (%class.MemoryManager*, i64)**,
//            i8* (%class.MemoryManager*, i64)*** %4, align 8
//  %6 = getelementptr inbounds i8* (%class.MemoryManager*, i64)*,
//                              i8* (%class.MemoryManager*, i64)** %5, i64 2
//  %7 = load i8* (%class.MemoryManager*, i64)*,
//            i8* (%"class.MemoryManager*, i64)** %6, align 8
//  %8 = bitcast i8* (%class.MemoryManager*, i64)* %7 to i8*
//  %9 = bitcast i8* (%class.1*, i64)* @userAlloc to i8*
//  %10 = icmp eq i8* %8, %9
//  br i1 %10, label %11, label %13
//
//; <label>:11:                                     ; preds = %2
//  %12 = tail call i8* bitcast (i8* (%class.1*, i64)* @userAlloc to
//                               i8* (%class.MemoryManager*, i64)*)
//                      (%class.MemoryManager* nonnull %1, i64 %3)
//  br label %15
//
//; <label>:13:                                     ; preds = %2
//  %14 = tail call i8* bitcast (i8* (%class.2*, i64)* @dummyAlloc to
//                               i8* (%class.MemoryManager*, i64)*)
//                      (%class.MemoryManager* nonnull %1, i64 %3)
//  br label %15
//
//; <label>:15:                                     ; preds = %13, %11
//  %16 = phi i8* [ %12, %11 ], [ %14, %13 ]
//  br label %17
//
//; <label>:17:                                     ; preds = %15
//  %18 = bitcast i8* %16 to %class.MemoryManager**
//  store %class.MemoryManager* %1, %class.MemoryManager** %18, align 8
//  %19 = getelementptr inbounds i8, i8* %16, i64 8
//  ret i8* %19
//}
bool DTransAllocAnalyzer::isMallocWithStoredMMPtr(const Function *F) {

  // Return 'true' if 'V' is a malloc-like call within the 'Callee'.
  auto IsMallocCall = [this](const Function *Callee, Value *V) -> bool {
    // Check if V represents a call with the right number of arguments.
    const auto *Call = dyn_cast<CallBase>(V);
    if (!Call)
      return false;
    if (Call->arg_size() != 2)
      return false;
    // Check that the arguments of the call come from the appropriate Callee
    // arguments. We expect the size argument to be adjusted by 8.
    auto MMArg = dyn_cast<Argument>(Call->getArgOperand(0));
    if (!MMArg)
      return false;
    if (MMArg != (Callee->arg_begin() + 1))
      return false;
    auto BIA = dyn_cast<BinaryOperator>(Call->getArgOperand(1));
    if (!BIA || BIA->getOpcode() != Instruction::Add)
      return false;
    Value *W = nullptr;
    ConstantInt *CI = nullptr;
    int64_t Offset = 0;
    if ((CI = dyn_cast<ConstantInt>(BIA->getOperand(0)))) {
      W = BIA->getOperand(1);
      Offset = CI->getSExtValue();
    } else if ((CI = dyn_cast<ConstantInt>(BIA->getOperand(1)))) {
      W = BIA->getOperand(0);
      Offset = CI->getSExtValue();
    } else
      return false;
    if (Offset != 8)
      return false;
    Argument *Arg0 = dyn_cast<Argument>(W);
    if (!Arg0)
      return false;
    if (Callee->arg_begin() != Arg0)
      return false;
    // Check that Function being called is a malloc function or dummy function
    // with unreachable.
    return isUserAllocOrDummyFunc(Call);
  };

  // Check the expected types of the Callee's arguments.
  const Function *Callee = F;
  LLVM_DEBUG(dbgs() << "Analyzing for MallocWithStoredMMPtr " << F->getName()
                    << "\n");
  // Save some time only looking at Callees with a small number of
  // basic blocks.
  if (Callee->size() > 5)
    return false;
  if (Callee->arg_size() != 2)
    return false;
  // Check that we have the right number of Callee arguments and that
  // those arguments are of the right basic types.
  const Argument *Arg0 = &*(Callee->arg_begin());
  if (!Arg0->getType()->isIntegerTy())
    return false;
  const Argument *Arg1 = &*(Callee->arg_begin() + 1);
  // TODO: OpaquePtr: Need to implement a way to get the pointer type for the
  // argument when opaque pointers are in use.
  if (!Arg1->getType()->isPointerTy() ||
      !Arg1->getType()->getPointerElementType()->isStructTy())
    return false;
  // Look for a unique ReturnInst with a return value.
  const ReturnInst *RI = nullptr;
  for (auto &BB : *Callee) {
    auto TI = BB.getTerminator();
    auto RII = dyn_cast<const ReturnInst>(TI);
    if (RII) {
      if (RI)
        return false;
      if (!RII->getReturnValue())
        return false;
      RI = RII;
    }
  }
  if (!RI)
    return false;
  // Look for an adjustment by 8 bytes of the return value. This
  // moves the pointer past the place where the memory manager
  // address is stored.
  auto GEPAdj = dyn_cast<GetElementPtrInst>(RI->getReturnValue());
  if (!GEPAdj)
    return false;
  if (GEPAdj->getNumIndices() != 1)
    return false;
  auto Int8Ty = llvm::Type::getInt8Ty(GEPAdj->getContext());
  if (GEPAdj->getSourceElementType() != Int8Ty)
    return false;
  auto ConstInt = dyn_cast<ConstantInt>(GEPAdj->getOperand(1));
  if (!ConstInt)
    return false;
  if (ConstInt->getSExtValue() != 8)
    return false;
  auto GEPP = getPointerOperand(GEPAdj);
  auto PHI = dyn_cast<PHINode>(GEPP);
  unsigned MallocCallCount = 0;
  if (PHI) {
    for (unsigned I = 0; I < PHI->getNumIncomingValues(); ++I) {
      Value *V = PHI->getIncomingValue(I);
      if (!IsMallocCall(Callee, V))
        return false;
      MallocCallCount++;
    }
  } else if (IsMallocCall(Callee, GEPP))
    MallocCallCount++;
  else
    return false;
  // Check that there are no side effects, except for those produced
  // by the store of the memory manager address to the first 8 bytes
  // of the allocated memory.
  unsigned CallCount = 0;
  unsigned StoreCount = 0;
  for (auto &I : instructions(Callee)) {
    if (isa<CallInst>(&I) || isa<InvokeInst>(&I)) {
      // Skip debug intrinsics
      if (isa<DbgInfoIntrinsic>(&I))
        continue;
      // Skip llvm.type_test / llvm.assume intrinsics.
      if (isTypeTestRelatedIntrinsic(&I))
        continue;
      if (++CallCount > MallocCallCount)
        return false;
    } else {
      auto SI = dyn_cast<StoreInst>(&I);
      if (SI) {
        if (StoreCount)
          return false;
        auto Arg1 = dyn_cast<Argument>(SI->getValueOperand());
        if (!Arg1)
          return false;
        if (Callee->arg_begin() + 1 != Arg1)
          return false;
        auto PO = SI->getPointerOperand();
        auto BCI = dyn_cast<BitCastInst>(PO);
        if (BCI)
          PO = BCI->getOperand(0);
        if (PO != GEPP)
          return false;
        StoreCount++;
      }
    }
  }

  bool RV = CallCount && StoreCount;
  if (RV)
    LLVM_DEBUG(dbgs() << "Is MallocWithStoredMMPtr " << F->getName() << "\n");
  else
    LLVM_DEBUG(dbgs() << "Not MallocWithStoredMMPtr " << F->getName() << "\n");
  return RV;
}

// Check that function is a special kind of user-defined free with stored
// memory manager pointer.
// Ex.:
// define internal void @candidateFunc(i8*) {
//  %2 = icmp eq i8* %0, null
//  br i1 %2, label %18, label %3
//
//; <label>:3:                                      ; preds = %1
//  %4 = getelementptr inbounds i8, i8* %0, i64 -8
//  %5 = bitcast i8* %4 to %class.MemoryManager**
//  %6 = load %class.MemoryManager*, %class.MemoryManager** %5, align 8
//  %7 = bitcast %class.MemoryManager* %6 to
//               void (%class.MemoryManager*, i8*)***
//  %8 = load void (%class.MemoryManager*, i8*)**,
//            void (%class.MemoryManager*, i8*)*** %7, align 8
//  %9 = getelementptr inbounds void (%class.MemoryManager*, i8*)*,
//                void (%class.MemoryManager*, i8*)** %8, i64 3
//  %10 = load void (%class.MemoryManager*, i8*)*,
//             void (%class.MemoryManager*, i8*)** %9, align 8
//  %11 = bitcast void (%class.MemoryManager*, i8*)* %10 to i8*
//  %12 = bitcast void (%class.1*, i8*)* @userFree to i8*
//  %13 = icmp eq i8* %11, %12
//  br i1 %13, label %14, label %15
//
//; <label>:14:                                     ; preds = %3
//  tail call void bitcast (void (%class.1*, i8*)* @userFree to
//                          void (%class.MemoryManager*, i8*)*)
//                 (%class.MemoryManager* %6, i8* nonnull %4)
//  br label %16
//
//; <label>:15:                                     ; preds = %3
//  tail call void bitcast (void (%class.2*, i8*)* @dummyFree to
//                          void (%class.MemoryManager*, i8*)*)
//                 (%class.MemoryManager* %6, i8* nonnull %4)
//  br label %16
//
//; <label>:16:                                     ; preds = %15, %14
//  br label %17
//
//; <label>:17:                                     ; preds = %16
//  br label %18
//
//; <label>:18:                                     ; preds = %17, %1
//  ret void
//}
bool DTransAllocAnalyzer::isFreeWithStoredMMPtr(const Function *F) {
  // Return 'true' if 'BB' consists of a test to see if argument 0
  // is equal to nullptr.
  auto IsFreeSkipTestBlock = [](const BasicBlock *BB) -> bool {
    if (BB->size() != 2)
      return false;
    auto ICI = dyn_cast<ICmpInst>(BB->begin());
    if (!ICI || !ICI->isEquality())
      return false;
    Value *V = nullptr;
    if (isa<ConstantPointerNull>(ICI->getOperand(0)))
      V = ICI->getOperand(1);
    else if (isa<ConstantPointerNull>(ICI->getOperand(1)))
      V = ICI->getOperand(0);
    if (!V)
      return false;
    auto Arg0 = dyn_cast<Argument>(V);
    if (!Arg0 || Arg0->getArgNo() != 0)
      return false;
    return true;
  };

  // Return either 'BB' or a unique predecessor at the end of a chain
  // of BasicBlocks with nothing except an unconditional branch to one
  // another.
  auto RootBlock = [](const BasicBlock *BB) -> const BasicBlock * {
    auto ResultBB = BB;
    while (BB->size() == 1) {
      auto BI = dyn_cast<BranchInst>(BB->getTerminator());
      if (!BI || !BI->isUnconditional())
        return ResultBB;
      BB = BB->getSinglePredecessor();
      if (!BB)
        return ResultBB;
      ResultBB = BB;
    }
    return ResultBB;
  };

  // Return 'true' if 'V' is a free-like call within the 'Callee'.
  auto IsFreeCall = [this](const Function *Callee,
                           const Instruction *I) -> bool {
    // Check if 'I' represents a call with the right number of arguments.
    const auto *Call = dyn_cast<CallBase>(I);
    if (!Call)
      return false;
    if (Call->arg_size() != 2)
      return false;
    // The zeroth argument should load an I8* value.
    auto LI = dyn_cast<LoadInst>(Call->getArgOperand(0));
    if (!LI)
      return false;
    if (!LI->getType()->isPointerTy())
      return false;
    auto BCI = dyn_cast<BitCastInst>(LI->getPointerOperand());
    if (!BCI)
      return false;
    // TODO: OpaquePtr: Need to re-implement this without direct check of i8*,
    // or requiring a bitcast when opaque pointers are enabled. Perhaps this
    // will need to have pointer type analyzer resolve the type.
    if (BCI->getSrcTy() != Int8PtrTy)
      return false;
    Value *W = BCI->getOperand(0);
    if (Call->getArgOperand(1) != W)
      return false;
    auto GEPI = dyn_cast<GetElementPtrInst>(W);
    if (!GEPI)
      return false;
    if (GEPI->getPointerOperand() != Callee->arg_begin())
      return false;
    if (GEPI->getNumIndices() != 1)
      return false;
    auto *Int8Ty = llvm::Type::getInt8Ty(GEPI->getContext());
    if (GEPI->getSourceElementType() != Int8Ty)
      return false;
    // The value of the pointer to be freed is 8 bytes before the passed
    // in pointer value.
    auto ConstInt = dyn_cast<ConstantInt>(GEPI->getOperand(1));
    if (!ConstInt)
      return false;
    if (ConstInt->getSExtValue() != -8)
      return false;
    return isUserFreeOrDummyFunc(Call);
  };

  // Check the expected types of the Callee's arguments.
  const Function *Callee = F;
  LLVM_DEBUG(dbgs() << "Analyzing for FreeWithStoredMMPtr " << F->getName()
                    << "\n");
  // Save some time only looking at Callees with a small number of
  // basic blocks.
  if (Callee->size() > 7)
    return false;
  unsigned ArgSize = Callee->arg_size();
  if (ArgSize != 1 && ArgSize != 2)
    return false;
  // Check that we have the right number of Callee arguments and that
  // those arguments are of the right basic types.
  const Argument *Arg0 = &*(Callee->arg_begin());
  // TODO: OpaquePtr: Need to implement a way to get pointer type for the
  // argument when opaque pointers are in use.
  if (Arg0->getType() != Int8PtrTy)
    return false;
  if (ArgSize == 2) {
    const Argument *Arg1 = &*(Callee->arg_begin() + 1);
    // TODO: OpaquePtr: Need to implement a way to get the pointer type for the
    // argument when opaque pointers are in use.
    if (!Arg1->getType()->isPointerTy() ||
        !Arg1->getType()->getPointerElementType()->isStructTy())
      return false;
  }
  // Look for a unique ReturnInst without a return value.
  const ReturnInst *RI = nullptr;
  for (auto &BB : *Callee) {
    auto TI = BB.getTerminator();
    auto RII = dyn_cast<const ReturnInst>(TI);
    if (RII) {
      if (RI)
        return false;
      if (RII->getReturnValue())
        return false;
      RI = RII;
    }
  }
  if (!RI)
    return false;
  // Each predecessor BasicBlock of the return is either a skip test block
  // or leads to a series of calls to free-like Functions.
  for (const BasicBlock *PB : predecessors(RI->getParent())) {
    if (IsFreeSkipTestBlock(PB))
      continue;
    auto PBN = RootBlock(PB);
    for (const BasicBlock *PPB : predecessors(PBN)) {
      if (PPB->size() == 1) {
        // Expecting single invoke instruction.
        if (!IsFreeCall(Callee, &PPB->front()))
          return false;
      } else {
        // Expecting call instruction + branch instruction.
        if (PPB->size() != 2)
          return false;
        auto BI = dyn_cast<BranchInst>(PPB->getTerminator());
        if (!BI || !BI->isUnconditional())
          return false;
        if (!IsFreeCall(Callee, &PPB->front()))
          return false;
      }
    }
  }

  LLVM_DEBUG(dbgs() << "Is FreeWithStoredMMPtr " << F->getName() << "\n");
  return true;
}

// Returns true if the called function is user-defined malloc or dummy
// function.
bool DTransAllocAnalyzer::isUserAllocOrDummyFunc(const CallBase *Call) {
  const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
  return (dtrans::isDummyFuncWithThisAndIntArgs(Call, TLI) ||
          isMallocPostDom(Call));
}

// Returns true if the called function is user-defined free or dummy
// function.
bool DTransAllocAnalyzer::isUserFreeOrDummyFunc(const CallBase *Call) {
  const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
  return (dtrans::isDummyFuncWithThisAndPtrArgs(Call, TLI) ||
          isFreePostDom(Call));
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

      for (const auto &Record : ListRecords) {
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

        if (auto *Ty =
                StructType::getTypeByName(M.getContext(), RecordItem[0])) {
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

  F(DTransFreeFunctions, AKS_Free0, false, "free");
  F(DTransMallocFunctions, AKS_Malloc0, true, "malloc");
}
#endif

// End of member functions for class DTransAllocAnalyzer
} // end namespace dtrans
} // end namespace llvm
