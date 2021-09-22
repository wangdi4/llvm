//===-----DTransAllocCollector.cpp - Allocation/Free function analyzer-----===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file provides support for identifying user functions that wrap memory
// allocation and free calls.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/DTransAllocCollector.h"

#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/MemoryBuiltinsExtras.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"

#define DEBUG_TYPE "dtrans-alloc-collector"

namespace llvm {
namespace dtransOP {

void DTransAllocCollector::populateAllocDeallocTable(const Module &M) {
  SmallPtrSet<const Function *, 32> CheckedAsMalloc;
  SmallPtrSet<const Function *, 32> CheckedAsFree;
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

    const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
    if (dtrans::isFreeFn(Call, TLI)) {
      // Check all functions, calling free function F.
      for (auto &U : F.uses())
        if (isa<CallBase>(U.getUser())) {
          auto *FreeCand = cast<Instruction>(U.getUser())->getFunction();
          if (!CheckedAsFree.insert(FreeCand).second)
            continue;
          AllocStatus Kind = analyzeForFreeStatus(FreeCand);
          if (Kind != AKS_Unknown)
            AllocStatusMap[FreeCand] = Kind;
        }
      continue;
    }

    auto Kind = dtrans::getAllocFnKind(Call, TLI);
    if (Kind == dtrans::AK_Malloc || Kind == dtrans::AK_New) {
      // Check all functions, calling malloc/new function F.
      for (auto &U : F.uses())
        if (isa<CallBase>(U.getUser())) {
          auto *MallocCand = cast<Instruction>(U.getUser())->getFunction();
          if (!CheckedAsMalloc.insert(MallocCand).second)
            continue;
          AllocStatus Kind = analyzeForMallocStatus(MallocCand);
          if (Kind != AKS_Unknown)
            AllocStatusMap[MallocCand] = Kind;
        }
      continue;
    }
  }

  // TODO: Expand the AllocStatusMap to include special functions that wrap the
  // identified functions. This could also be extended to produce a closure of
  // functions calling the identified function that also meet that criteria of
  // being a user allocation/free function, but for now it will just look for
  // some special cases.
}

dtrans::AllocKind
DTransAllocCollector::getAllocFnKind(const CallBase *Call,
                                     const TargetLibraryInfo &TLI) {
  dtrans::AllocKind Kind = dtrans::getAllocFnKind(Call, TLI);
  if (Kind != dtrans::AK_NotAlloc)
    return Kind;

  // There is no need to process indirect function calls at this time because
  // devirtualization has converted any indirect calls to direct calls in the
  // cases of interest.
  const Function *F = dtrans::getCalledFunction(*Call);
  if (!F)
    return dtrans::AK_NotAlloc;

  auto IT = AllocStatusMap.find(F);
  if (IT == AllocStatusMap.end())
    return dtrans::AK_NotAlloc;

  switch (IT->second) {
  default:
    return dtrans::AK_NotAlloc;
  case AKS_Malloc:
    return dtrans::AK_UserMalloc;
  case AKS_Malloc0:
    return dtrans::AK_UserMalloc0;
  case AKS_MallocThis:
    return dtrans::AK_UserMallocThis;
  }
  llvm_unreachable("Fully covered switch");
}

dtrans::FreeKind
DTransAllocCollector::getFreeFnKind(const CallBase *Call,
                                    const TargetLibraryInfo &TLI) {
  dtrans::FreeKind Kind =
      dtrans::isFreeFn(Call, TLI)
          ? (dtrans::isDeleteFn(Call, TLI) ? dtrans::FK_Delete
                                           : dtrans::FK_Free)
          : dtrans::FK_NotFree;
  if (Kind != dtrans::FK_NotFree)
    return Kind;

  // There is no need to process indirect function calls at this time because
  // devirtualization has converted any indirect calls to direct calls in the
  // cases of interest.
  const Function *F = dtrans::getCalledFunction(*Call);
  if (!F)
    return dtrans::FK_NotFree;

  auto IT = AllocStatusMap.find(F);
  if (IT == AllocStatusMap.end())
    return dtrans::FK_NotFree;

  switch (IT->second) {
  default:
    return dtrans::FK_NotFree;
  case AKS_Free:
    return dtrans::FK_UserFree;
  case AKS_Free0:
    return dtrans::FK_UserFree0;
  case AKS_FreeThis:
    return dtrans::FK_UserFreeThis;
  }

  llvm_unreachable("Fully covered switch");
}

// Return 'true' if Function 'F' is identified as a user allocation function
// that returns memory allocated by 'malloc' or 'new'. The function is also
// allowed to return a 'null' value. The size of the allocation should be
// derived from an integer input argument. There is no need to check whether the
// allocated memory gets used as an aggregate type within the function because
// the safety analyzer will detect a type being returned from a function
// declared as returning an i8* type which would generate an "Address taken"
// safety violation on the types aliased to the return value and value assigned
// at the callsite.
//
// For example:
//   define internal ptr @AcquireMMemory(i64 %size) {
//     %mem = tail call ptr @malloc(i64 %size)
//     %failed = icmp eq ptr %mem, null
//     br i1 %failed, label %report, label %done
//   report:
//     call void @reportError()
//     ret ptr null
//   done:
//     ret ptr %mem
//   }
//
DTransAllocCollector::AllocStatus
DTransAllocCollector::analyzeForMallocStatus(const Function *F) {
  // The signatures supported for the user allocation function detected here are
  // limited to the cases for AKS_Malloc and AKS_MallocThis. AKS_Malloc0 could
  // be supported, but is currently limited to being used in special cases.
  //   ptr @func(i64 %size)
  //   ptr @func(ptr %this, i64 %size)
  //       where the 'ptr %this' argument is not used by the function.
  //
  // The return type should be equivalent to i8* based on the DTrans metadata
  // for both cases.
  auto HasSupportedSignature = [this](const Function *F, AllocStatus *Kind) {
    auto ReturnIsI8Ptr = [](DTransFunctionType *DFTy) {
      DTransType *DRetTy = DFTy->getReturnType();
      assert(DRetTy && "Invalid DTransFunctionType");
      if (!DRetTy->isPointerTy())
        return false;

      DTransType *DPointeeTy = DRetTy->getPointerElementType();
      llvm::Type *Ty = DPointeeTy->getLLVMType();
      return Ty->isIntegerTy(8);
    };

    if (F->isVarArg())
      return false;

    DTransFunctionType *DFTy =
        dyn_cast_or_null<DTransFunctionType>(MDReader.getDTransTypeFromMD(F));
    if (!DFTy)
      return false;

    if (F->arg_size() == 1 && F->getArg(0)->getType()->isIntegerTy()) {
      if (!ReturnIsI8Ptr(DFTy))
        return false;

      *Kind = AKS_Malloc;
      return true;
    }

    if (F->arg_size() != 2)
      return false;

    DTransType *DArg0Ty = DFTy->getArgType(0);
    DTransType *DArg1Ty = DFTy->getArgType(1);
    assert(DArg0Ty && DArg1Ty && "Invalid DTransFunctionTy");
    if (!(DArg0Ty->isPointerTy() &&
          DArg0Ty->getPointerElementType()->isStructTy() &&
          DArg1Ty->isIntegerTy()))
      return false;

    const Argument *Arg0 = F->arg_begin();
    if (!Arg0->user_empty())
      return false;

    if (!ReturnIsI8Ptr(DFTy))
      return false;

    *Kind = AKS_MallocThis;
    return true;
  };

  // Return 'true' if value 'V' is derived from the result of a call to 'malloc'
  // or 'new'. Supported cases are ones where value 'V' is the result of the
  // allocation call, or a GEP that gets an address within the allocation when
  // the allocation is created with additional space to store some metadata
  // within the allocation. The size argument of the allocation will be captured
  // into the set 'AllocSize' to allow checking that the size is related to the
  // input argument of the function being evaluated.
  auto ValDerivedFromAlloc = [this](Value *V, const TargetLibraryInfo &TLI,
                                    SmallPtrSetImpl<Value *> &AllocSizes) {
    if (auto *Call = dyn_cast<CallBase>(V)) {
      dtrans::AllocKind Kind = getAllocFnKind(Call, TLI);
      if (Kind == dtrans::AK_Malloc || Kind == dtrans::AK_New) {
        unsigned AllocSizeInd = -1U;
        unsigned AllocCountInd = -1U;
        dtrans::getAllocSizeArgs(Kind, Call, AllocSizeInd, AllocCountInd, TLI);
        AllocSizes.insert(Call->getArgOperand(AllocSizeInd));
        return true;
      }
    } else if (auto *GEP = dyn_cast<GetElementPtrInst>(V)) {
      dtrans::AllocKind Kind = dtrans::AK_NotAlloc;
      CallBase *Call = nullptr;
      if (!dtrans::analyzeGEPAsAllocationResult(GEP, TLI, &Call, &Kind))
        return false;

      unsigned AllocSizeInd = -1U;
      unsigned AllocCountInd = -1U;
      dtrans::getAllocSizeArgs(Kind, Call, AllocSizeInd, AllocCountInd, TLI);
      AllocSizes.insert(Call->getArgOperand(AllocSizeInd));
      return true;
    }

    return false;
  };

  // Return 'true' is the function only returns 'null' or a pointer derived from
  // an allocation call.
  auto RetValDerivedFromAlloc =
      [&ValDerivedFromAlloc](const Function *F, const TargetLibraryInfo &TLI,
                             SmallPtrSetImpl<Value *> &AllocSizes) {
        // Collect the return values
        SmallPtrSet<Value *, 4> RetVals;
        for (const BasicBlock &BB : *F) {
          const Instruction *Terminator = BB.getTerminator();
          auto *RetI = dyn_cast<ReturnInst>(Terminator);
          if (!RetI)
            continue;

          auto *V = RetI->getReturnValue();
          if (isa<ConstantPointerNull>(V)) {
            continue;
          } else if (auto *PHI = dyn_cast<PHINode>(V)) {
            unsigned int Paths = PHI->getNumIncomingValues();
            for (unsigned int Idx = 0; Idx < Paths; ++Idx)
              RetVals.insert(PHI->getIncomingValue(Idx));
            continue;
          } else if (auto *Sel = dyn_cast<SelectInst>(V)) {
            RetVals.insert(Sel->getTrueValue());
            RetVals.insert(Sel->getFalseValue());
            continue;
          }

          RetVals.insert(V);
        }

        bool FoundNonNullValue = false;
        for (auto *I : RetVals) {
          if (isa<ConstantPointerNull>(I))
            continue;

          if (!ValDerivedFromAlloc(I, TLI, AllocSizes))
            return false;
          FoundNonNullValue = true;
        }

        return FoundNonNullValue;
      };

  // Return 'true' if the Value 'V' is derived from the Argument 'Arg'.
  // 'V' is value passed as the size of the allocation, and 'Arg' is the
  // argument for the size of the user allocation function being evaluated.
  auto ValDerivedFromArgument = [](Value *V, Argument *Arg) {
    // Allow the patterns:
    //
    //   %mem = call ptr @malloc(%v)
    //
    // or
    //
    //   %cmp = icmp eq i64 %arg, 0
    //   %v = select i1 %cmp, i64 1, i64 %arg
    //   %mem = call ptr @malloc(%v)
    //
    // or
    //
    //   %o = add nsw i32, %arg, 15
    //   (optional): %tmp = sext i32 %o to i64
    //   %v = add nsw i64 %tmp, 12
    //   %mem = call ptr @malloc(%v)
    //
    if (V == Arg)
      return true;

    ICmpInst::Predicate Pred;
    if (PatternMatch::match(
            V, PatternMatch::m_Select(
                   PatternMatch::m_ICmp(Pred, PatternMatch::m_Specific(Arg),
                                        PatternMatch::m_ZeroInt()),
                   PatternMatch::m_One(), PatternMatch::m_Specific(Arg))))
      if (Pred == CmpInst::Predicate::ICMP_EQ)
        return true;

    // Helper to check whether a simple arithmetic Value is just a sign/zero
    // extended or constant offset from 'Arg'
    std::function<bool(Value *, Argument *)> ValueIsConstantOffset =
        [&ValueIsConstantOffset](Value *Val, Argument *Arg) {
          if (Val == Arg)
            return true;

          // Ignore value extensions
          if (isa<SExtInst>(Val) || isa<ZExtInst>(Val))
            return ValueIsConstantOffset(cast<Instruction>(Val)->getOperand(0),
                                         Arg);

          // Look for "Val+C" or "C+Val" to see if Val comes from the Argument.
          Value *Op = nullptr;
          if (PatternMatch::match(
                  Val, PatternMatch::m_c_Add(PatternMatch::m_Constant(),
                                             PatternMatch::m_Value(Op))))
            if (Op == Arg)
              return true;
            else
              return ValueIsConstantOffset(Op, Arg);

          return false;
        };

    if (ValueIsConstantOffset(V, Arg))
      return true;

    return false;
  };

  assert(F && "Invalid argument");
  LLVM_DEBUG(dbgs() << "Analyzing for user allocation function: "
                    << F->getName() << "\n");
  AllocStatus AllocType = AKS_Unknown;
  if (!HasSupportedSignature(F, &AllocType)) {
    LLVM_DEBUG(dbgs() << "Not user allocation function: " << F->getName()
                      << " - Unsupported function signature\n");
    return AKS_Unknown;
  }

  const TargetLibraryInfo &TLI = GetTLI(*F);
  SmallPtrSet<Value *, 4> AllocArgs;
  if (!RetValDerivedFromAlloc(F, TLI, AllocArgs)) {
    LLVM_DEBUG(dbgs() << "Not user allocation function: " << F->getName()
                      << " - Return value not from allocation\n");
    return AKS_Unknown;
  }

  assert(AllocType == AKS_Malloc ||
         AllocType == AKS_MallocThis && "Unsupported AllocType");
  Argument *SizeArg = AllocType == AKS_Malloc ? F->getArg(0) : F->getArg(1);
  for (auto *V : AllocArgs) {
    if (!ValDerivedFromArgument(V, SizeArg)) {
      LLVM_DEBUG(
          dbgs() << "Not user allocation function: " << F->getName()
                 << " - Allocation size not derived from function argument\n");
      return AKS_Unknown;
    }
  }

  LLVM_DEBUG(dbgs() << "Identified as user allocation function: "
                    << F->getName() << "\n\n"
                    << *F << "\n");
  return AllocType;
}

DTransAllocCollector::AllocStatus
DTransAllocCollector::analyzeForFreeStatus(const Function *F) {

  // The signatures supported for the user free function are limited to the
  // cases of AKS_Free and AKS_FreeThis. AKS_Free0 could be supported, but is
  // only used for special cases currently.
  //   void @func(ptr %mem)
  //   ptr @func(ptr %mem)
  //       Later it will be checked that the return pointer is 'null'
  //   void @func(ptr %this, ptr %mem)
  //       Where the 'ptr %this' argument is not used.
  //
  // The 'ptr %mem' type should be equivalent to i8* based on the DTrans
  // metadata in all cases.
  auto HasSupportedSignature = [this](const Function *F, int *ArgNum,
                                      AllocStatus *Kind) {
    auto ArgIsI8Ptr = [](DTransFunctionType *DFuncTy, unsigned ArgNo) {
      assert(DFuncTy->getNumArgs() >= ArgNo && "Invalid argument number");
      DTransType *DArgTy = DFuncTy->getArgType(ArgNo);
      assert(DArgTy && "Invalid DTransFunctionType");
      if (!DArgTy->isPointerTy())
        return false;

      DTransType *DPointeeTy = DArgTy->getPointerElementType();
      llvm::Type *Ty = DPointeeTy->getLLVMType();
      if (!Ty->isIntegerTy(8))
        return false;

      return true;
    };

    if (F->arg_size() == 0 || F->isVarArg())
      return false;

    DTransFunctionType *DFTy =
        dyn_cast_or_null<DTransFunctionType>(MDReader.getDTransTypeFromMD(F));
    if (!DFTy)
      return false;

    DTransType *DRetTy = DFTy->getReturnType();
    assert(DRetTy && "Invalid DTransFunctionType");
    llvm::Type *RetTy = DRetTy->getLLVMType();
    if (!RetTy->isVoidTy() && !RetTy->isPointerTy())
      return false;

    if (F->arg_size() == 1) {
      if (!ArgIsI8Ptr(DFTy, 0))
        return false;

      *ArgNum = 0;
      *Kind = AKS_Free;
      return true;
    }

    if (F->arg_size() == 2) {
      const Argument *Arg0 = F->arg_begin();
      if (!Arg0->user_empty())
        return false;

      // Check for "this, i8*" signature.
      DTransType *DArgTy = DFTy->getArgType(0);
      assert(DArgTy && "Invalid DTransFunctionType");
      if (DArgTy->isPointerTy() &&
          DArgTy->getPointerElementType()->isStructTy()) {
        if (!ArgIsI8Ptr(DFTy, 1))
          return false;

        *ArgNum = 1;
        *Kind = AKS_FreeThis;
        return true;
      }
    }

    return false;
  };

  // Return 'true' if 'ArgNum' of 'F' is used by a call to free/delete.
  auto ArgumentReachesFree = [this](const Function *F, int ArgNum,
                                    const TargetLibraryInfo &TLI) {
    // Return true if we can find the Value 'V' being used in a free call.
    // There is also a special case that allows the actual location to be
    // embedded within the memory prior to the address passed into the function
    // as metadata using the following pattern:
    //   (optional) %addr = bitcast i8* %in to i8**
    //   %offset = getelementptr inbounds i8*, i8** %addr, i64 -1
    //   %metaptr = load i8*, i8** %offset
    //   tail call void @free(i8* %metaptr)
    std::function<bool(Value *, bool, bool)> ValueUsedByFree;
    ValueUsedByFree = [&](Value *V, bool GEPSeen, bool LoadSeen) {
      for (auto &U : V->uses()) {
        auto *I = dyn_cast<Instruction>(U.getUser());
        if (!I)
          continue;

        if (auto *Call = dyn_cast<CallBase>(I)) {
          dtrans::FreeKind Kind = getFreeFnKind(Call, TLI);
          if (Kind != dtrans::FK_NotFree)
            return true;
        } else if (auto *BC = dyn_cast<BitCastInst>(I)) {
          if (ValueUsedByFree(BC, GEPSeen, LoadSeen))
            return true;
        } else if (auto *GEP = dyn_cast<GetElementPtrInst>(I)) {
          if (GEPSeen || LoadSeen)
            return false;
          if (GEP->getNumIndices() != 1 ||
              !GEP->getSourceElementType()->isPointerTy() ||
              !isa<ConstantInt>(GEP->getOperand(1)))
            return false;
          int64_t Offset =
              cast<ConstantInt>(GEP->getOperand(1))->getSExtValue();
          if (Offset >= 0)
            return false;
          if (ValueUsedByFree(GEP, true, LoadSeen))
            return true;
          // Don't return false here, continue to look for other GEPs that may
          // lead to the free call.
        } else if (auto *LI = dyn_cast<LoadInst>(I)) {
          if (LoadSeen)
            return false;
          // The load is coming from either the input argument, a bitcast or a
          // GEP that offsets it. Check whether the loaded address reaches the
          // free.
          if (ValueUsedByFree(LI, GEPSeen, true))
            return true;
        }
      }
      return false;
    };

    Argument *Arg = F->getArg(ArgNum);
    if (ValueUsedByFree(Arg, /*GEPSeen=*/false, /*LoadSeen=*/false))
      return true;

    return false;
  };

  // Return 'true' if all return values from 'F' are 'null'
  auto ReturnIsNull = [](const Function *F) {
    for (const BasicBlock &BB : *F) {
      const Instruction *Terminator = BB.getTerminator();
      auto *RetI = dyn_cast<ReturnInst>(Terminator);
      if (!RetI)
        continue;

      auto *V = RetI->getReturnValue();
      if (isa<ConstantPointerNull>(V))
        continue;

      return false;
    }

    return true;
  };

  assert(F && "Invalid argument");
  LLVM_DEBUG(dbgs() << "Analyzing for user free function: " << F->getName()
                    << "\n");

  int ArgNum = -1;
  AllocStatus FreeType = AKS_Unknown;
  if (!HasSupportedSignature(F, &ArgNum, &FreeType)) {
    LLVM_DEBUG(dbgs() << "Not user free function: " << F->getName()
                      << " - Unsupported function signature\n");
    return AKS_Unknown;
  }

  if (cast<FunctionType>(F->getValueType())->getReturnType()->isPointerTy()) {
    // Only allow nullptr to be returned value to avoid treating functions
    // that perform a reallocation operation (i.e. free followed by malloc)
    // from being identified as wrapper functions for 'free'
    if (!ReturnIsNull(F)) {
      LLVM_DEBUG(dbgs() << "Not user free function: " << F->getName()
                        << " - Return value is not 'null'\n");
      return AKS_Unknown;
    }
  }

  assert(ArgNum != -1 && "HasSupportedSignature failed to set ArgNum");
  const TargetLibraryInfo &TLI = GetTLI(*F);
  if (!ArgumentReachesFree(F, ArgNum, TLI)) {
    LLVM_DEBUG(dbgs() << "Not user free function: " << F->getName()
                      << " - Argument not passed to 'free' call\n");
    return AKS_Unknown;
  }

  LLVM_DEBUG(dbgs() << "Identified as user free function: " << F->getName()
                    << "\n\n"
                    << *F << "\n");
  return FreeType;
}

} // end namespace dtransOP
} // end namespace llvm
