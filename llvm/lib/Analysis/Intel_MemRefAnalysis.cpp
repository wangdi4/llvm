//===--- Intel_MemRefAnalysis.cpp-Defines data-sturctures for MemRef analyses=//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_MemRefAnalysis.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Debug.h"
#include <tuple>

#define DEBUG_TYPE "load-coalescing"

using namespace llvm;
using namespace llvm::vpmemrefanalysis;

using OffsetTy = llvm::vpmemrefanalysis::BasicBlockMemRefAnalysis::OffsetTy;
using OffsetResultTy =
    llvm::vpmemrefanalysis::BasicBlockMemRefAnalysis::OffsetResultTy;

template <typename MemInstType>
OffsetResultTy BasicBlockMemRefAnalysis::isConstantOffset(MemInstType *A,
                                                          MemInstType *B) {
  assert(A && B && "Operands of isConstantOffset cannot be nullptr");
  Value *PtrA = A->getPointerOperand();
  Value *PtrB = B->getPointerOperand();
  if (PtrA->getType()->getPointerAddressSpace() !=
             PtrB->getType()->getPointerAddressSpace())
    return std::make_tuple(false, -1);

  const SCEV *Scev0 = SE.getSCEV(PtrA);
  const SCEV *Scev = SE.getSCEV(PtrB);
  const auto *Diff =
      dyn_cast<SCEVConstant>(SE.getMinusSCEV(Scev, Scev0, SCEV::FlagNSW));
  if (!Diff) {
    return std::make_tuple(false, -1);
  } else {
    // TODO: Use an assertion to verify that Offset does not overflow when
    // sign-extension cannot fit in int64
    OffsetTy Offset = Diff->getAPInt().getSExtValue();
    return std::make_tuple(true, Offset);
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
template <typename MemInstType> void MemRefBucket<MemInstType>::dump() {
  for (const auto &BucketMember : BucketMembers) {
    const auto Member = BucketMember.getInstruction();
    dbgs() << "\tMEMBER->"
           << "(" << Member << ")-->" << *Member << "OFFSET = ("
           << BucketMember.getOffset() << ")"
           << "\n";
  }
}
#endif

void BasicBlockMemRefAnalysis::sortBucketsByOffsetFromBasePointer() {
  for (auto &BucketsIter : BBLoadBuckets) {
    auto &BMembers = BucketsIter.BucketMembers;
    std::sort(BMembers.begin(), BMembers.end(),
              [](const MemRefBucket<LoadInst>::MemRefBucketMember &A,
                 const MemRefBucket<LoadInst>::MemRefBucketMember &B) {
                return A.getOffset() < B.getOffset();
              });
  }
  // A similar method would come in when we have active Stores buckets.
}

template <typename MemInstType>
void BasicBlockMemRefAnalysis::insertIntoBucket(MemInstType *I) {
  assert(isa<LoadInst>(I) && "Expect input argument to be of LoadInst type");
  bool foundBucket = false;
  for (auto &BucketsIter : BBLoadBuckets) {
    MemInstType *BucketHeader = BucketsIter.getBucketHeader();
    assert(BucketHeader && "BucketHeader is nullptr");
    bool isConstOff = false;
    OffsetTy Offset = 0;
    std::tie(isConstOff, Offset) =
        isConstantOffset<MemInstType>(BucketHeader, cast<MemInstType>(I));

    if (isConstOff) {
      typename MemRefBucket<MemInstType>::MemRefBucketMember MemRef(I, Offset);
      BucketsIter.add(MemRef);
      foundBucket = true;
      break;
    }
  }

  if (BBLoadBuckets.empty() || !foundBucket) {
    // We either have a non-empty BBMemRefBuckets, but no appropriate header
    // that matches this mem-ref or the very first ref that we are trying to
    // bucket.Just create a new bucket. Insert the instruction as a key as well
    // as the first member in the bucket. This makes sorting of the bucket
    // easier. In the later step, we can swap the key with the lowest offset
    // member if needed.
    BBLoadBuckets.push_back(MemRefBucket<MemInstType>(I, (OffsetTy)0));
  }
}

void BasicBlockMemRefAnalysis::populateBasicBlockMemRefBuckets(
    BasicBlock *BB, bool AllowScalars) {

  MapVector<Value *, SmallVector<LoadInst *, 8>> Loads;

  // Scan the basic-block for loads and bucket the loads based on the
  // base-pointer as the bucket header

  BBLoadBuckets.clear();
  for (Instruction &Instr : *BB) {
    Instruction *I = &Instr;
    LoadInst *LI = dyn_cast<LoadInst>(I);
    if (LI && (AllowScalars || isa<VectorType>(LI->getType())) &&
        LI->isSimple())
      insertIntoBucket<LoadInst>(LI);
  }

  // Sort all the Loads
  sortBucketsByOffsetFromBasePointer();
}

template OffsetResultTy
BasicBlockMemRefAnalysis::isConstantOffset<LoadInst>(LoadInst *A, LoadInst *B);
