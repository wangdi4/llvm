//===- MemoryLocation.cpp - Memory location descriptions -------------------==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/MemoryLocation.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Operator.h"  // INTEL

using namespace llvm;

MemoryLocation MemoryLocation::get(const LoadInst *LI) {
  AAMDNodes AATags;
  LI->getAAMetadata(AATags);
  const auto &DL = LI->getModule()->getDataLayout();

  return MemoryLocation(LI->getPointerOperand(),
                        DL.getTypeStoreSize(LI->getType()), AATags);
}

MemoryLocation MemoryLocation::get(const StoreInst *SI) {
  AAMDNodes AATags;
  SI->getAAMetadata(AATags);
  const auto &DL = SI->getModule()->getDataLayout();

  return MemoryLocation(SI->getPointerOperand(),
                        DL.getTypeStoreSize(SI->getValueOperand()->getType()),
                        AATags);
}

MemoryLocation MemoryLocation::get(const VAArgInst *VI) {
  AAMDNodes AATags;
  VI->getAAMetadata(AATags);

  return MemoryLocation(VI->getPointerOperand(), UnknownSize, AATags);
}

MemoryLocation MemoryLocation::get(const AtomicCmpXchgInst *CXI) {
  AAMDNodes AATags;
  CXI->getAAMetadata(AATags);
  const auto &DL = CXI->getModule()->getDataLayout();

  return MemoryLocation(
      CXI->getPointerOperand(),
      DL.getTypeStoreSize(CXI->getCompareOperand()->getType()), AATags);
}

MemoryLocation MemoryLocation::get(const AtomicRMWInst *RMWI) {
  AAMDNodes AATags;
  RMWI->getAAMetadata(AATags);
  const auto &DL = RMWI->getModule()->getDataLayout();

  return MemoryLocation(RMWI->getPointerOperand(),
                        DL.getTypeStoreSize(RMWI->getValOperand()->getType()),
                        AATags);
}

MemoryLocation MemoryLocation::getForSource(const MemTransferInst *MTI) {
  uint64_t Size = UnknownSize;
  if (ConstantInt *C = dyn_cast<ConstantInt>(MTI->getLength()))
    Size = C->getValue().getZExtValue();

  // memcpy/memmove can have AA tags. For memcpy, they apply
  // to both the source and the destination.
  AAMDNodes AATags;
  MTI->getAAMetadata(AATags);

  return MemoryLocation(MTI->getRawSource(), Size, AATags);
}

MemoryLocation MemoryLocation::getForDest(const MemIntrinsic *MTI) {
  uint64_t Size = UnknownSize;
  if (ConstantInt *C = dyn_cast<ConstantInt>(MTI->getLength()))
    Size = C->getValue().getZExtValue();

  // memcpy/memmove can have AA tags. For memcpy, they apply
  // to both the source and the destination.
  AAMDNodes AATags;
  MTI->getAAMetadata(AATags);

  return MemoryLocation(MTI->getRawDest(), Size, AATags);
}

MemoryLocation MemoryLocation::getForArgument(ImmutableCallSite CS,
                                              unsigned ArgIdx,
                                              const TargetLibraryInfo &TLI) {
  AAMDNodes AATags;
  CS->getAAMetadata(AATags);
  const Value *Arg = CS.getArgument(ArgIdx);

  // We may be able to produce an exact size for known intrinsics.
  if (const IntrinsicInst *II = dyn_cast<IntrinsicInst>(CS.getInstruction())) {
    const DataLayout &DL = II->getModule()->getDataLayout();

    switch (II->getIntrinsicID()) {
    default:
      break;
    case Intrinsic::memset:
    case Intrinsic::memcpy:
    case Intrinsic::memmove:
      assert((ArgIdx == 0 || ArgIdx == 1) &&
             "Invalid argument index for memory intrinsic");
      if (ConstantInt *LenCI = dyn_cast<ConstantInt>(II->getArgOperand(2)))
        return MemoryLocation(Arg, LenCI->getZExtValue(), AATags);
      break;

    case Intrinsic::lifetime_start:
    case Intrinsic::lifetime_end:
    case Intrinsic::invariant_start:
      assert(ArgIdx == 1 && "Invalid argument index");
      return MemoryLocation(
          Arg, cast<ConstantInt>(II->getArgOperand(0))->getZExtValue(), AATags);

    case Intrinsic::invariant_end:
      assert(ArgIdx == 2 && "Invalid argument index");
      return MemoryLocation(
          Arg, cast<ConstantInt>(II->getArgOperand(1))->getZExtValue(), AATags);

    case Intrinsic::arm_neon_vld1:
      assert(ArgIdx == 0 && "Invalid argument index");
      // LLVM's vld1 and vst1 intrinsics currently only support a single
      // vector register.
      return MemoryLocation(Arg, DL.getTypeStoreSize(II->getType()), AATags);

    case Intrinsic::arm_neon_vst1:
      assert(ArgIdx == 0 && "Invalid argument index");
      return MemoryLocation(
          Arg, DL.getTypeStoreSize(II->getArgOperand(1)->getType()), AATags);
    }
  }

  // We can bound the aliasing properties of memset_pattern16 just as we can
  // for memcpy/memset.  This is particularly important because the
  // LoopIdiomRecognizer likes to turn loops into calls to memset_pattern16
  // whenever possible.
  LibFunc::Func F;
  if (CS.getCalledFunction() && TLI.getLibFunc(*CS.getCalledFunction(), F) &&
      F == LibFunc::memset_pattern16 && TLI.has(F)) {
    assert((ArgIdx == 0 || ArgIdx == 1) &&
           "Invalid argument index for memset_pattern16");
    if (ArgIdx == 1)
      return MemoryLocation(Arg, 16, AATags);
    if (const ConstantInt *LenCI = dyn_cast<ConstantInt>(CS.getArgument(2)))
      return MemoryLocation(Arg, LenCI->getZExtValue(), AATags);
  }
  // FIXME: Handle memset_pattern4 and memset_pattern8 also.

  return MemoryLocation(CS.getArgument(ArgIdx), UnknownSize, AATags);
}

#if INTEL_CUSTOMIZATION
// A memory location \p Loc can be considered resolved when:
//    1) The location pointer is non-null(means a known location).
//    2) Or, the location is known to be not alias with any other pointers,
//       in that case location-pointer can be null with size zero.
static bool isMemLocResolved(const MemoryLocation &Loc) {
  return Loc.Ptr != nullptr || Loc.Size == 0;
}

static void getMemLocsForPtrVec(const Value *PtrVec,
                                SmallVectorImpl<MemoryLocation> &Results,
                                bool &Resolved, int Depth) {
  assert(PtrVec->getType()->isVectorTy());

  // Stop if search reaches depth or we already have what we need.
  if (Depth == 0 || Resolved)
    return;

  // Mark resolved if all memory locations have been resolved.
  if (all_of(Results, [](MemoryLocation &m) { return isMemLocResolved(m); })) {
    Resolved = true;
    return;
  }

  // Constant pointer vector.
  unsigned NumElts = Results.size();
  if (const auto *CV = dyn_cast<Constant>(PtrVec)) {
    // If this vector is an undef value, we can assume that none of its
    // elements aliases with any pointer.
    if (isa<UndefValue>(CV)) {
      for (auto &R : Results)
        if (!isMemLocResolved(R))
          // Set this memory location's size to zero, as zero-sized memory
          // location does not alias with any pointer.
          R.Size = 0;
      Resolved = true;
      return;
    }
    // Vector of pointer constants, check each pointer constant.
    for (unsigned i = 0; i < NumElts; i++) {
      if (isMemLocResolved(Results[i]))
        continue;
      // At ith lane, set the memory location to be the memory location
      // of ith pointer constant.
      Results[i] = MemoryLocation(CV->getOperand(i));
    }
    Resolved = true;
    return;
  }

  const auto *Op = dyn_cast<Operator>(PtrVec);
  switch (Op->getOpcode()) {
  // Currently only examine the following instructions for simplicity
  // and safety.
  default:
    break;
  case Instruction::GetElementPtr: {
    // Examine the pointers to the underlying objects, and use the underlying
    // objects' memory locations. This will make the reuslts more conservative
    // by adding false positives, but will not give false negatives.
    Value *BasePtr = Op->getOperand(0);
    if (BasePtr->getType()->isVectorTy())
      getMemLocsForPtrVec(BasePtr, Results, Resolved, Depth - 1);
    else
      Results.assign(NumElts, MemoryLocation(BasePtr));
    }
    break;
  case Instruction::InsertElement:
    // This is the major source of vector elements. If the insert index is a
    // constant int within the range of vector, we can get the memory location
    // of the inserted element for that lane. If the index is a variable, set
    // all memlocs as unknown (Ptr = nullptr, Size = UnknownSize) as it could
    // be inserted to any lane.
    // Otherwise, continue to examine the source operand.
    if (auto *IndexOp = dyn_cast<ConstantInt>(Op->getOperand(2))) {
      int64_t Idx = IndexOp->getSExtValue();
      if (Idx >= 0 && Idx < NumElts && !isMemLocResolved(Results[Idx]))
        Results[Idx] = MemoryLocation(Op->getOperand(1));
    } else {
      Results.assign(NumElts, MemoryLocation());
      Resolved = true;
      return;
    }
    getMemLocsForPtrVec(Op->getOperand(0), Results, Resolved, Depth - 1);
    break;
  }
}

void MemoryLocation::getForPtrVec(const Value *PtrVec,
                                  SmallVectorImpl<MemoryLocation> &Results,
                                  int Depth) {
  bool Resolved = false;
  getMemLocsForPtrVec(PtrVec, Results, Resolved, Depth);
}
#endif // INTEL_CUSTOMIZATION
