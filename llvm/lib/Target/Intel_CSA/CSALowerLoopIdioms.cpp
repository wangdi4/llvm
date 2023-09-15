//===- CSALowerLoopIdioms.cpp - --------------------------------*- C++ -*--===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// \file
// Lower memset, memcpy, and memmov intrinsics into individual instructions or
// loops.
//
// This pass was originally based on NVPTXLowerAggrCopies, but has since
// diverged to produce code that is better suited for CSA.
//
// See docs/Intel/CSA/CSALowerLoopIdioms.rst for more in-depth information.
//
//===----------------------------------------------------------------------===//

#include "CSALowerLoopIdioms.h"
#include "CSATargetMachine.h"
#include "llvm/CodeGen/StackProtector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <algorithm>

#define DEBUG_TYPE "csa-lower-loop-idioms"

using namespace llvm;

static cl::opt<unsigned> MaxStoresPerMemIntr{
  "csa-max-stores-per-memintr", cl::Hidden,
  cl::desc("CSA Specific: Maximum number of stores to use per "
           "memcpy/memmove/memset call expansion"),
  cl::init(4)};

namespace {

struct CSALowerLoopIdioms : public FunctionPass {
  static char ID;

  CSALowerLoopIdioms() : FunctionPass(ID) {
    initializeCSALowerLoopIdiomsPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<StackProtector>();
  }

  bool runOnFunction(Function &F) override;

  StringRef getPassName() const override { return "CSA: Lower loop idioms"; }
};

char CSALowerLoopIdioms::ID = 0;

// Converts a pointer to one to an integer type of the given size. If the value
// is already the right type, just use that. This could end up producing
// redundant casts, but instcombine is run right after this pass and should
// clean all of those up.
Value *getPtrOfSize(Value *Base, unsigned Size, IRBuilder<> &IRB) {
  const auto BaseType = cast<PointerType>(Base->getType());
  const auto DestType =
    PointerType::get(IntegerType::get(Base->getContext(), Size * 8),
                     BaseType->getAddressSpace());
  if (BaseType == DestType)
    return Base;
  return IRB.CreatePointerCast(Base, DestType);
}

// Lowers a memcpy/memmove to flat loads/stores. The only difference between
// memcpy and memmove in the flat expansion is the ordering requirements, which
// will be dealt with later in memop ordering.
void expandMemTransferFlat(MemTransferInst *I, unsigned MaxGran) {
  uint64_t Length = cast<ConstantInt>(I->getLength())->getLimitedValue();

  LLVM_DEBUG(dbgs() << "Doing flat memtransfer expansion\n");

  // In order for the memmove expansion to be correct, emit all of the loads
  // first and then all of the stores.
  IRBuilder<> IRB{I};
  struct LoadRec {
    unsigned Size;
    uint64_t Index;
    Value *Val;
  };
  SmallVector<LoadRec, 4> Loaded;
  uint64_t LoadOffset = 0;
  LLVM_DEBUG(dbgs() << "  Sizes are:");
  for (unsigned CurSize = MaxGran; LoadOffset < Length; CurSize >>= 1) {
    Value *const BasePtr = getPtrOfSize(I->getSource(), CurSize, IRB);
    while (LoadOffset + CurSize <= Length) {
      LLVM_DEBUG(dbgs() << " " << CurSize);
      LoadRec LR;
      LR.Size  = CurSize;
      LR.Index = LoadOffset / CurSize;
      Value *const OffPtr =
          LoadOffset ? IRB.CreateConstInBoundsGEP1_64(BasePtr, LR.Index)
                     : BasePtr;
      LR.Val =
          IRB.CreateAlignedLoad(OffPtr, MaybeAlign(CurSize), I->isVolatile());
      Loaded.push_back(LR);
      LoadOffset += CurSize;
    }
  }
  for (const LoadRec &LR : Loaded) {
    Value *const BasePtr = getPtrOfSize(I->getDest(), LR.Size, IRB);
    Value *const OffPtr =
        LR.Index ? IRB.CreateConstInBoundsGEP1_64(BasePtr, LR.Index) : BasePtr;
    IRB.CreateAlignedStore(LR.Val, OffPtr, MaybeAlign(LR.Size),
                           I->isVolatile());
  }
  LLVM_DEBUG(dbgs() << "\n");

  // TODO: Add markings for memcpys to make sure that memory ordering knows the
  // loads and stores won't alias.
}

// Replicates a integer value RepCount times using bitshifts, where RepCount is
// a power of two.
Value *ReplicateInt(Value *V, unsigned RepCount, IRBuilder<> &IRB) {
  if (RepCount == 1)
    return V;

  assert(not(RepCount & 1) && "RepCount must be a power of two!");
  const unsigned OldWidth = cast<IntegerType>(V->getType())->getBitWidth();
  Value *const Extend     = IRB.CreateZExt(V, IRB.getIntNTy(OldWidth * 2));
  Value *const Shift =
    IRB.CreateShl(Extend, IRB.getIntN(OldWidth * 2, OldWidth), "", true);
  Value *const Or = IRB.CreateOr(Extend, Shift);
  return ReplicateInt(Or, RepCount / 2, IRB);
}

// Lowers a memset to flat stores.
void expandMemSetFlat(MemSetInst *I, unsigned MaxGran) {
  uint64_t Length = cast<ConstantInt>(I->getLength())->getLimitedValue();
  IRBuilder<> IRB{I};

  LLVM_DEBUG(dbgs() << "Doing flat memset expansion\n");

  uint64_t StoreOffset = 0;
  LLVM_DEBUG(dbgs() << "  Sizes are:");
  for (unsigned CurSize = MaxGran; StoreOffset < Length; CurSize >>= 1) {
    Value *const RepVal = ReplicateInt(I->getValue(), CurSize, IRB);

    Value *const BasePtr = getPtrOfSize(I->getDest(), CurSize, IRB);
    while (StoreOffset + CurSize <= Length) {
      LLVM_DEBUG(dbgs() << " " << CurSize);
      Value *const OffPtr =
          StoreOffset
              ? IRB.CreateConstInBoundsGEP1_64(BasePtr, StoreOffset / CurSize)
              : BasePtr;
      IRB.CreateAlignedStore(RepVal, OffPtr, MaybeAlign(CurSize),
                             I->isVolatile());
      StoreOffset += CurSize;
    }
  }
  LLVM_DEBUG(dbgs() << "\n");
}

// Determines the minimum known number of trailing zeros for a value.
unsigned minTrailingZeros(Value *V) {

  // If the value is a constant, look at trailing zeros directly.
  if (const auto CV = dyn_cast<ConstantInt>(V)) {
    return CV->getValue().countTrailingZeros();
  }

  if (const auto IV = dyn_cast<Instruction>(V)) {

    // If it's a shift, look for a constant shift amount and base the result on
    // that.
    if (IV->getOpcode() == Instruction::Shl) {
      const auto Shift = dyn_cast<ConstantInt>(IV->getOperand(1));
      if (not Shift)
        return minTrailingZeros(IV->getOperand(0));
      return minTrailingZeros(IV->getOperand(0)) + Shift->getLimitedValue();
    }

    // If it's a multiply, recurse through the operands.
    if (IV->getOpcode() == Instruction::Mul)
      return minTrailingZeros(IV->getOperand(0)) +
             minTrailingZeros(IV->getOperand(1));
  }

  // Otherwise, there might not be any trailing zeros.
  return 0;
}

// Determines the load/store size for loops. This is determined based on the
// number of known trailing zeros on the length and cannot be larger than the
// value for flat expansions.
unsigned loopGranularity(Value *Length, unsigned FlatGran) {
  const unsigned MaxPoTFactor = 1u << std::min(minTrailingZeros(Length), 3u);
  LLVM_DEBUG(dbgs() << "  Max usable PoT factor in length: " << MaxPoTFactor
                    << "\n");
  return std::min(MaxPoTFactor, FlatGran);
}

// Lower memcpy to loop.
void expandMemCpyLoop(MemCpyInst *I, unsigned FlatGran) {
  Value *const CopyLen = I->getLength();
  Type *const LenType  = CopyLen->getType();

  LLVM_DEBUG(dbgs() << "Doing loop memcpy expansion\n");

  BasicBlock *const OrigBB = I->getParent();
  BasicBlock *const NewBB  = OrigBB->splitBasicBlock(I, "split");
  BasicBlock *const LoopBB = BasicBlock::Create(I->getContext(), "memcpyloop",
                                                OrigBB->getParent(), NewBB);

  IRBuilder<> Builder(OrigBB->getTerminator());
  ReplaceInstWithInst(
    OrigBB->getTerminator(),
    BranchInst::Create(
      LoopBB, NewBB,
      Builder.CreateICmpUGT(CopyLen, ConstantInt::get(LenType, 0))));
  Builder.SetInsertPoint(OrigBB->getTerminator());

  const unsigned LoopGran = loopGranularity(CopyLen, FlatGran);
  LLVM_DEBUG(dbgs() << "  Granularity: " << LoopGran << "\n");

  // Cast pointers and scale trip count according to the loop granularity.
  Value *const ScaledSrcAddr = getPtrOfSize(I->getSource(), LoopGran, Builder);
  Value *const ScaledDstAddr = getPtrOfSize(I->getDest(), LoopGran, Builder);
  Value *const ScaledCopyLen =
    LoopGran > 1
      ? Builder.CreateLShr(
          CopyLen, ConstantInt::get(LenType, countTrailingZeros(LoopGran)), "",
          true)
      : CopyLen;

  IRBuilder<> LoopBuilder(LoopBB);
  LoopBuilder.SetCurrentDebugLocation(I->getDebugLoc());
  PHINode *const LoopIndex = LoopBuilder.CreatePHI(LenType, 0);
  LoopIndex->addIncoming(ConstantInt::get(LenType, 0), OrigBB);

  // load from ScaledSrcAddr+LoopIndex
  Type *Ty = ScaledSrcAddr->getType()->getScalarType()->getPointerElementType();
  Value *const Element = LoopBuilder.CreateAlignedLoad(
      LoopBuilder.CreateInBoundsGEP(Ty, ScaledSrcAddr, LoopIndex),
      MaybeAlign(LoopGran), I->isVolatile());
  // store at ScaledDstAddr+LoopIndex
  Ty = ScaledDstAddr->getType()->getScalarType()->getPointerElementType();
  LoopBuilder.CreateAlignedStore(
      Element, LoopBuilder.CreateInBoundsGEP(Ty, ScaledDstAddr, LoopIndex),
      MaybeAlign(LoopGran), I->isVolatile());

  // The value for LoopIndex coming from backedge is (LoopIndex + 1)
  Value *const NewIndex =
      LoopBuilder.CreateAdd(LoopIndex, ConstantInt::get(LenType, 1), "", true);
  LoopIndex->addIncoming(NewIndex, LoopBB);

  LoopBuilder.CreateCondBr(LoopBuilder.CreateICmpULT(NewIndex, ScaledCopyLen),
                           LoopBB, NewBB);

  // TODO: Stick the metadata on this loop so that memory ordering knows it's
  // parallel.
}

// Lower memmove to loop. memmove is required to correctly copy overlapping
// memory regions; therefore, it has to check the relative positions of the
// source and destination pointers and choose the copy direction accordingly.
// This can either be done with two separate loops or with one loop that has a
// non-constant stride. Since space is limited on CSA, the one-loop approach is
// likely the one that we want.
//
// The code below is an IR rendition of this C function:
//
// void* memmove(void* dst, const void* src, size_t n) {
//   unsigned char*const d = dst;
//   const unsigned char*const s = src;
//   const int backward = (s < d);
//   const size_t step = backward ? -1 : +1;
//   size_t off = backward ? n-1 : 0;
//   for (size_t i = 0; i < n; ++i) {
//     d[off] = s[off];
//     off += step;
//   }
//   return dst;
// }
void expandMemMoveLoop(MemMoveInst *I, unsigned FlatGran) {
  Value *const MoveLen = I->getLength();
  Type *const LenType  = MoveLen->getType();

  LLVM_DEBUG(dbgs() << "Doing loop memmove expansion\n");

  BasicBlock *const OrigBB = I->getParent();
  BasicBlock *const NewBB  = OrigBB->splitBasicBlock(I, "split");
  BasicBlock *const LoopBB = BasicBlock::Create(I->getContext(), "memmoveloop",
                                                OrigBB->getParent(), NewBB);

  IRBuilder<> Builder(OrigBB->getTerminator());
  ReplaceInstWithInst(
    OrigBB->getTerminator(),
    BranchInst::Create(
      LoopBB, NewBB,
      Builder.CreateICmpUGT(MoveLen, ConstantInt::get(LenType, 0))));
  Builder.SetInsertPoint(OrigBB->getTerminator());

  const unsigned LoopGran = loopGranularity(MoveLen, FlatGran);
  LLVM_DEBUG(dbgs() << "  Granularity: " << LoopGran << "\n");

  // Cast pointers and scale trip count according to the loop granularity.
  Value *const ScaledSrcAddr = getPtrOfSize(I->getSource(), LoopGran, Builder);
  Value *const ScaledDstAddr = getPtrOfSize(I->getDest(), LoopGran, Builder);
  Value *const ScaledMoveLen =
    LoopGran > 1
      ? Builder.CreateLShr(
          MoveLen, ConstantInt::get(LenType, countTrailingZeros(LoopGran)), "",
          true)
      : MoveLen;

  // Determine whether to copy backwards and choose base/steps accordingly.
  Value *const Backward = Builder.CreateICmpULT(ScaledSrcAddr, ScaledDstAddr);
  Value *const Step =
    Builder.CreateSelect(Backward, ConstantInt::getSigned(LenType, -1),
                         ConstantInt::get(LenType, 1));
  Value *const Start = Builder.CreateSelect(
    Backward, Builder.CreateSub(ScaledMoveLen, ConstantInt::get(LenType, 1)),
    ConstantInt::get(LenType, 0));

  IRBuilder<> LIRB{LoopBB};
  LIRB.SetCurrentDebugLocation(I->getDebugLoc());
  PHINode *LoopInd = LIRB.CreatePHI(LenType, 2);
  LoopInd->addIncoming(ConstantInt::get(LenType, 0), OrigBB);
  PHINode *Index = LIRB.CreatePHI(LenType, 2);
  Index->addIncoming(Start, OrigBB);

  Type *Ty = ScaledSrcAddr->getType()->getScalarType()->getPointerElementType();
  Value *const Element = LIRB.CreateAlignedLoad(
      LIRB.CreateInBoundsGEP(Ty, ScaledSrcAddr, Index),
      MaybeAlign(LoopGran), I->isVolatile());
  Ty = ScaledDstAddr->getType()->getScalarType()->getPointerElementType();
  LIRB.CreateAlignedStore(Element,
                          LIRB.CreateInBoundsGEP(Ty, ScaledDstAddr, Index),
                          MaybeAlign(LoopGran), I->isVolatile());

  Value *const NewLoopInd =
      LIRB.CreateAdd(LoopInd, ConstantInt::get(LenType, 1), "", true);
  LoopInd->addIncoming(NewLoopInd, LoopBB);
  Value *const NewIndex = LIRB.CreateAdd(Index, Step);
  Index->addIncoming(NewIndex, LoopBB);

  LIRB.CreateCondBr(LIRB.CreateICmpULT(NewLoopInd, ScaledMoveLen), LoopBB,
                    NewBB);

  // TODO: memmove loops aren't parallel, but still don't have any backwards
  // dependencies. If we get a way of marking this the loop should be marked
  // that way.
}

// Lower memset to loop.
void expandMemSetLoop(MemSetInst *I, unsigned FlatGran) {
  Value *const SetLen = I->getLength();

  LLVM_DEBUG(dbgs() << "Doing loop memset expansion\n");

  BasicBlock *const OrigBB = I->getParent();
  BasicBlock *const NewBB  = OrigBB->splitBasicBlock(I, "split");
  BasicBlock *const LoopBB = BasicBlock::Create(I->getContext(), "memsetloop",
                                                OrigBB->getParent(), NewBB);

  IRBuilder<> Builder(OrigBB->getTerminator());
  ReplaceInstWithInst(
    OrigBB->getTerminator(),
    BranchInst::Create(
      LoopBB, NewBB,
      Builder.CreateICmpUGT(SetLen, ConstantInt::get(SetLen->getType(), 0))));
  Builder.SetInsertPoint(OrigBB->getTerminator());

  const unsigned LoopGran = loopGranularity(SetLen, FlatGran);
  LLVM_DEBUG(dbgs() << "  Granularity: " << LoopGran << "\n");

  // Cast pointers, scale the trip count, and replicate the value according to
  // the loop granularity.
  Value *const ScaledDstAddr = getPtrOfSize(I->getDest(), LoopGran, Builder);
  Value *const ScaledSetLen =
    LoopGran > 1
      ? Builder.CreateLShr(
          SetLen,
          ConstantInt::get(SetLen->getType(), countTrailingZeros(LoopGran)), "",
          true)
      : SetLen;
  Value *const ScaledVal = ReplicateInt(I->getValue(), LoopGran, Builder);

  IRBuilder<> LoopBuilder(LoopBB);
  LoopBuilder.SetCurrentDebugLocation(I->getDebugLoc());
  PHINode *const LoopIndex = LoopBuilder.CreatePHI(SetLen->getType(), 0);
  LoopIndex->addIncoming(ConstantInt::get(SetLen->getType(), 0), OrigBB);

  Type *Ty = ScaledDstAddr->getType()->getScalarType()->getPointerElementType();
  LoopBuilder.CreateAlignedStore(
      ScaledVal,
      LoopBuilder.CreateInBoundsGEP(Ty, ScaledDstAddr, LoopIndex),
      MaybeAlign(LoopGran), I->isVolatile());

  Value *const NewIndex = LoopBuilder.CreateAdd(
      LoopIndex, ConstantInt::get(SetLen->getType(), 1), "", true);
  LoopIndex->addIncoming(NewIndex, LoopBB);

  LoopBuilder.CreateCondBr(LoopBuilder.CreateICmpULT(NewIndex, ScaledSetLen),
                           LoopBB, NewBB);
}

// Determines the maximum copy granularity for a flat memcpy/memmove/memset
// expansions. Currently this is chosen according to alignment.
unsigned maxFlatGranularity(MemTransferInst *I) {
  return std::max(
    1u, std::min({I->getSourceAlignment(), I->getDestAlignment(), 8u}));
}
unsigned maxFlatGranularity(MemSetInst *I) {
  return std::max(1u, std::min(I->getDestAlignment(), 8u));
}

// Determines the number of stores needed to implement a flat
// memcpy/memmove/memset of a given length/granularity.
uint64_t flatStoreCount(uint64_t Length, unsigned MaxGran) {
  return Length / MaxGran + countPopulation(Length % MaxGran);
}

// Determines whether to flat-expand an intrinsic.
bool doFlatExpand(MemIntrinsic *I, unsigned MaxGran) {
  LLVM_DEBUG(dbgs() << "  Max flat granularity: " << MaxGran << "\n");
  const auto CL = dyn_cast<ConstantInt>(I->getLength());
  if (not CL) {
    LLVM_DEBUG(dbgs() << "  Length: not statically known\n");
    return false;
  }
  uint64_t Length = CL->getLimitedValue();
  LLVM_DEBUG(dbgs() << "  Length: " << Length << "\n");
  uint64_t FlatStoreCount = flatStoreCount(Length, MaxGran);
  LLVM_DEBUG(dbgs() << "  Flat store count: " << FlatStoreCount << "\n");
  return FlatStoreCount <= MaxStoresPerMemIntr;
}

void expandMemTransfer(MemTransferInst *I) {
  LLVM_DEBUG(dbgs() << "Considering expansion for memtransfer:" << *I << "\n");

  const unsigned MaxGran = maxFlatGranularity(I);
  if (doFlatExpand(I, MaxGran))
    return expandMemTransferFlat(I, MaxGran);

  if (const auto MemCpy = dyn_cast<MemCpyInst>(I))
    return expandMemCpyLoop(MemCpy, MaxGran);
  if (const auto MemMove = dyn_cast<MemMoveInst>(I))
    return expandMemMoveLoop(MemMove, MaxGran);

  llvm_unreachable("Found unexpected MemTransferInst type");
}

void expandMemSet(MemSetInst *I) {
  LLVM_DEBUG(dbgs() << "Considering expansion for memset:" << *I << "\n");

  const unsigned MaxGran = maxFlatGranularity(I);
  if (doFlatExpand(I, MaxGran))
    return expandMemSetFlat(I, MaxGran);

  return expandMemSetLoop(I, MaxGran);
}

bool CSALowerLoopIdioms::runOnFunction(Function &F) {
  SmallVector<MemIntrinsic *, 4> MemCalls;

  // Collect all mem* calls.
  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      if (const auto IntrCall = dyn_cast<MemIntrinsic>(&I)) {
        MemCalls.push_back(IntrCall);
      }
    }
  }

  if (MemCalls.size() == 0)
    return false;

  // Transform mem* intrinsic calls.
  for (MemIntrinsic *const MemCall : MemCalls) {
    if (const auto MemTrans = dyn_cast<MemTransferInst>(MemCall)) {
      expandMemTransfer(MemTrans);
    } else if (const auto MemSet = dyn_cast<MemSetInst>(MemCall)) {
      expandMemSet(MemSet);
    } else {
      llvm_unreachable("Found unexpected MemIntrinsic type");
    }
    MemCall->eraseFromParent();
  }

  return true;
}

} // namespace

INITIALIZE_PASS(CSALowerLoopIdioms, DEBUG_TYPE,
                "Lower llvm.mem* intrinsics into loops", false, false)

FunctionPass *llvm::createLowerLoopIdioms() { return new CSALowerLoopIdioms(); }
