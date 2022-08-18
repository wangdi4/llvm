//===---MemoryBuiltinsExtras.cpp -Extend MemoryBuiltins.h functionality----===//
//
// Copyright (C) 2018-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file provides convenience functions to DTrans to extend the
// functionality of "llvm/Analysis/MemoryBuiltins.h" for DTrans to analyze
// calls to functions that allocate or free memory.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/MemoryBuiltinsExtras.h"

#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/TargetLibraryInfo.h"

#define DEBUG_TYPE "dtransanalysis"

namespace llvm {
namespace dtrans {

StringRef AllocKindName(AllocKind Kind) {
  switch (Kind) {
  case AK_NotAlloc:
    return "NotAlloc";
  case AK_Malloc:
    return "Malloc";
  case AK_Calloc:
    return "Calloc";
  case AK_Realloc:
    return "Realloc";
  case AK_UserMalloc:
    return "UserMalloc";
  case AK_UserMalloc0:
    return "UserMalloc0";
  case AK_UserMallocThis:
    return "UserMallocThis";
  case AK_New:
    return "new/new[]";
  }
  llvm_unreachable("Unexpected continuation past AllocKind switch.");
}

StringRef FreeKindName(FreeKind Kind) {
  switch (Kind) {
  case FK_NotFree:
    return "NotFree";
  case FK_Free:
    return "Free";
  case FK_UserFree:
    return "UserFree";
  case FK_UserFree0:
    return "UserFree0";
  case FK_UserFreeThis:
    return "UserFreeThis";
  case FK_Delete:
    return "delete/delete[]";
  }
  llvm_unreachable("Unexpected continuation past FreeKind switch.");
}

bool isUserAllocKind(AllocKind Kind) {
  return Kind == AK_UserMalloc || Kind == AK_UserMalloc0 ||
         Kind == AK_UserMallocThis;
}

bool isUserFreeKind(FreeKind Kind) {
  return Kind == FK_UserFree || Kind == FK_UserFree0 || Kind == FK_UserFreeThis;
}

AllocKind getAllocFnKind(const CallBase *Call, const TargetLibraryInfo &TLI) {
  // Returns non-null, so C++ function.
  if (isNewLikeFn(Call, &TLI))
    return AK_New;
  if (isMallocLikeFn(Call, &TLI))
    // if C++ and could return null, then there should be more than one
    // argument.
    return Call->arg_size() == 1 ? AK_Malloc : AK_New;
  if (isCallocLikeFn(Call, &TLI))
    return AK_Calloc;
  if (getReallocatedOperand(Call, &TLI))
    return AK_Realloc;
  return AK_NotAlloc;
}

static void getAllocSizeArgsImpl(AllocKind Kind, const CallBase *Call,
                                 unsigned &AllocSizeInd,
                                 unsigned &AllocCountInd,
                                 const TargetLibraryInfo &TLI) {
  assert(Kind != AK_NotAlloc &&
         "Unexpected alloc kind passed to getAllocSizeArgs");
  switch (Kind) {
  case AK_UserMalloc:
    AllocSizeInd = 0;
    AllocCountInd = -1U;
    return;
  case AK_UserMalloc0:
    AllocSizeInd = 0;
    AllocCountInd = -1U;
    return;
  case AK_UserMallocThis:
    AllocSizeInd = 1;
    AllocCountInd = -1U;
    return;
  case AK_New:
    AllocSizeInd = 0;
    AllocCountInd = -1U;
    return;
  case AK_Calloc:
  case AK_Malloc:
  case AK_Realloc: {
    /// All functions except calloc return -1 as a second argument.
    auto Inds = getAllocSizeArgumentIndices(Call, &TLI);
    if (Inds.second == -1U) {
      AllocSizeInd = Inds.first;
      AllocCountInd = -1U;
    } else {
      assert(Kind == AK_Calloc && "Only calloc has two size arguments");
      AllocCountInd = Inds.first;
      AllocSizeInd = Inds.second;
    }
    break;
  }
  default:
    llvm_unreachable("Unexpected alloc kind passed to getAllocSizeArgs");
  }
}

void getAllocSizeArgs(AllocKind Kind, const CallBase *Call,
                      unsigned &AllocSizeInd, unsigned &AllocCountInd,
                      const TargetLibraryInfo &TLI) {
  getAllocSizeArgsImpl(Kind, Call, AllocSizeInd, AllocCountInd, TLI);
  LLVM_DEBUG({
    bool Direct = true;
    if (!dtrans::getCalledFunction(*Call))
      Direct = false;

    dbgs() << "AllocSizeArgs: [Kind=" << AllocKindName(Kind)
           << " Direct=" << Direct << " ArgsCount=" << Call->arg_size()
           << " SizeInd=" << AllocSizeInd << " CountInd=" << AllocCountInd
           << " :" << *Call << "\n";
  });

  if (AllocSizeInd != -1U)
    assert(Call->getArgOperand(AllocSizeInd)->getType()->isIntegerTy() &&
           "Size argument should be an integer");
}

// Should be kept in sync with DTransInstVisitor::DTanalyzeAllocationCall.
void collectSpecialAllocArgs(AllocKind Kind, const CallBase *Call,
                             SmallPtrSet<const Value *, 3> &OutputSet,
                             const TargetLibraryInfo &TLI) {

  unsigned AllocSizeInd = -1U;
  unsigned AllocCountInd = -1U;
  getAllocSizeArgs(Kind, Call, AllocSizeInd, AllocCountInd, TLI);
  if (AllocSizeInd < Call->arg_size())
    OutputSet.insert(Call->getArgOperand(AllocSizeInd));
  if (AllocCountInd < Call->arg_size())
    OutputSet.insert(Call->getArgOperand(AllocCountInd));

  if (Kind == AK_Realloc)
    OutputSet.insert(Call->getArgOperand(0));
}

bool isFreeFn(const CallBase *Call, const TargetLibraryInfo &TLI) {
  return getFreedOperand(Call, &TLI, false);
}

bool isDeleteFn(const CallBase *Call, const TargetLibraryInfo &TLI) {
  return isDeleteCall(Call, &TLI, false);
}

static void getFreePtrArgImpl(FreeKind Kind, const CallBase *Call,
                              unsigned &PtrArgInd,
                              const TargetLibraryInfo &TLI) {
  assert(Kind != FK_NotFree && "Unexpected free kind passed to getFreePtrArg");

  if (!dtrans::getCalledFunction(*Call)) {
    assert(Kind == FK_UserFreeThis &&
           "Indirect call only supports \"this\" and \"ptr\" argument");
    PtrArgInd = 1;
    return;
  }

  if (Kind == FK_UserFreeThis) {
    PtrArgInd = 1;
    return;
  }

  PtrArgInd = 0;
}

void getFreePtrArg(FreeKind Kind, const CallBase *Call, unsigned &PtrArgInd,
                   const TargetLibraryInfo &TLI) {
  getFreePtrArgImpl(Kind, Call, PtrArgInd, TLI);
  LLVM_DEBUG({
    bool Direct = true;
    if (!dtrans::getCalledFunction(*Call))
      Direct = false;

    dbgs() << "FreeSizeArgs: [Kind=" << FreeKindName(Kind)
           << " Direct=" << Direct << " ArgsCount=" << Call->arg_size()
           << " ArgInd=" << PtrArgInd << " :" << *Call << "\n";
  });

  if (PtrArgInd != -1U)
    assert(Call->getArgOperand(PtrArgInd)->getType()->isPointerTy() &&
           "Free argument should be a pointer");
}

void collectSpecialFreeArgs(FreeKind Kind, const CallBase *Call,
                            SmallPtrSetImpl<const Value *> &OutputSet,
                            const TargetLibraryInfo &TLI) {
  unsigned PtrArgInd = -1U;
  getFreePtrArg(Kind, Call, PtrArgInd, TLI);

  if (PtrArgInd < Call->arg_size())
    OutputSet.insert(Call->getArgOperand(PtrArgInd));
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
static bool mallocBasedGEPChain(GetElementPtrInst *GV, GetElementPtrInst **GBV,
                                dtrans::AllocKind *Kind, CallBase **GCI,
                                const TargetLibraryInfo &TLI) {
  GetElementPtrInst *V;
  for (V = GV; isa<GetElementPtrInst>(V->getPointerOperand());
       V = cast<GetElementPtrInst>(V->getPointerOperand())) {
    if (!V->getSourceElementType()->isIntegerTy(8))
      return false;
  }
  if (!V->getSourceElementType()->isIntegerTy(8))
    return false;

  Value *BasePtr = V->getPointerOperand();
  if (auto *Call = dyn_cast<CallBase>(BasePtr)) {
    *Kind = dtrans::getAllocFnKind(Call, TLI);
    if (*Kind != dtrans::AK_Malloc && *Kind != dtrans::AK_New)
      return false;
    *GBV = V;
    *GCI = Call;
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
static bool mallocOffset(Value *V, int64_t *offset) {
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
        } else {
          return false;
        }
      } else {
        return false;
      }
    } else if (auto SI = dyn_cast<SExtInst>(V)) {
      V = SI->getOperand(0);
    } else {
      return false;
    }
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
// Here 'V' is operand 1 of the byte-flattened GEP 'GV'.
//
// If we return true, we set '*Result' to the value of the upper bound.
//
// Here is a simple example:
//
//  %6 = getelementptr inbounds i8, i8* %5, i64 15
//  %7 = getelementptr inbounds i8, i8* %6, i64 8
//  %8 = getelementptr inbounds i8, i8* %7, i64 4
//  %9 = getelementptr inbounds i8, i8* %8, i64 -11
//
// If 'V' here is -11 and 'GBV' is %6, then the most that %9 can be less
// than %8 is 11. So the value returned in '*Result' is 11.
//
// Here is a more complex example:
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
// than %8 is 15. So the value returned in '*Result' is 15.
//
// The sequence of GEPs starting with 'GBV' are:
//
//  %6 = getelementptr inbounds i8, i8* %5, i64 15
//  %7 = getelementptr inbounds i8, i8* %6, i64 8
//  %8 = getelementptr inbounds i8, i8* %7, i64 4
//
// The offset computed is 15+8+4 == 27, which should be equal to 'Offset'.
//
// NOTE: mallocLimit() is a bit of a pattern match, albeit for a few very
// important cases.
//
static bool mallocLimit(GetElementPtrInst *GBV, GetElementPtrInst *GV,
                        int64_t Offset, int64_t *Result) {
  int64_t Limit = 0;
  Value *V = GV->getOperand(1);
  auto CI0 = dyn_cast<ConstantInt>(V);
  Value *NewGEP = nullptr;
  if (CI0) {
    int64_t Result = CI0->getSExtValue();
    if (Result >= 0)
      return false;
    Limit = -Result;
    NewGEP = GV->getPointerOperand();
  } else {
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
    NewGEP = PI->getOperand(0);
  }
  int64_t LocalOffset = 0;
  auto *Int8Ty = llvm::Type::getInt8Ty(V->getContext());
  GetElementPtrInst *LastGEP = nullptr;
  while (auto GEP = dyn_cast<GetElementPtrInst>(NewGEP)) {
    LastGEP = GEP;
    if (GEP->getSourceElementType() != Int8Ty)
      return false;
    NewGEP = GEP->getPointerOperand();
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

bool analyzeGEPAsAllocationResult(GetElementPtrInst *GEP,
                                  const TargetLibraryInfo &TLI, CallBase **Call,
                                  dtrans::AllocKind *Kind) {
  int64_t Limit, Offset;
  GetElementPtrInst *GBV;
  if (!mallocBasedGEPChain(GEP, &GBV, Kind, Call, TLI))
    return false;
  if (!mallocOffset((*Call)->getArgOperand(0), &Offset))
    return false;
  if (!mallocLimit(GBV, GEP, Offset, &Limit))
    return false;
  if (Offset < Limit)
    return false;

  return true;
}

} // end namespace dtrans
} // end namespace llvm
