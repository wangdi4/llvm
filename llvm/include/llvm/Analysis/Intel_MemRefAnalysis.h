//==- Intel_MemRefAnalysis.h-Declares data-structures for MemRef analyses--===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares classes for MemRef Analysis and an interface class used by
// LoadCoalescing.
//
// TODO: Improve the BasicBlockMemRefAnalysis class to not expose internal
// data-structures and provide the client classes with well-defined interfaces
// and helpers. This would include more getter/setters, iterators for scanning
// buckets and their members etc.
// TODO: Make the BasicBlockMemRefAnalysis class more extensible. This would
// make it useful for storing information about both Loads and Stores at the
// same time.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_MEM_REF_ANALYSIS_H
#define LLVM_ANALYSIS_INTEL_MEM_REF_ANALYSIS_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Casting.h"

namespace llvm {

class Value;
class LoadInst;
class StoreInst;
class BasicBlock;
class DataLayout;
class ScalarEvolution;

namespace vpmemrefanalysis {

const short DefaultBucketSize = 8;
const short DefaultNumBuckets = 4;

class MemRefBucketMember;
template <typename MemInstType> class MemRefBucket {

  friend class BasicBlockMemRefAnalysis;

  using OffsetTy = int64_t;

  /// Class for representing the memory reference that occurs in a memory
  /// reference instructions, i.e., either a load or a store instruction and
  /// which is at a fixed offset from the bucket header.
  class MemRefBucketMember {
  public:
    MemRefBucketMember() = delete;
    MemRefBucketMember(MemInstType *LoadStoreInstruction, OffsetTy Offset)
        : LSInstruction(LoadStoreInstruction),
          BaseExprValue(LoadStoreInstruction->getPointerOperand()),
          Offset(Offset), NextMemRefDistance((OffsetTy)0) {}
    ~MemRefBucketMember() {}

    /// Integer Offset from the pointer stored in MemRefBucketHeader
    OffsetTy getOffset() const { return Offset; }

    /// Return the underlying value corresponding to the MemRef
    Value *getValue() const { return BaseExprValue; }

    /// Return the underlying Instruction corresponding to the MemRef
    MemInstType *getInstruction() const { return LSInstruction; }

  private:
    /// The actual MemRef instruction
    MemInstType *LSInstruction;

    /// Stores the base expression i.e., underlying pointer
    Value *BaseExprValue;

    /// Offset from the base pointer
    OffsetTy Offset;

    /// Offset from the current MemRef
    OffsetTy NextMemRefDistance;
  };

  using MemRefBucketMembers =
      SmallVector<MemRefBucketMember, DefaultBucketSize>;

private:
  MemInstType *BucketHeader;
  MemRefBucketMembers BucketMembers;

public:
  MemInstType *getBucketHeader() { return BucketHeader; }
  MemRefBucketMembers const &getMembers() const { return BucketMembers; }
  void add(const MemRefBucketMember &BM) { BucketMembers.push_back(BM); }
  unsigned size() const { return BucketMembers.size(); }
  MemRefBucket() = delete;
  MemRefBucket(MemInstType *LSI, OffsetTy Offset) : BucketHeader(LSI) {
    BucketMembers.push_back(MemRefBucketMember(LSI, Offset));
  };
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump();
#endif
};

// This class acts as an interface to the MemRef bucketing mechanism and is
// responsible for the analysis of loads for the purpose of LoadCoalescing
class BasicBlockMemRefAnalysis {

public:
  // Type Aliases
  using OffsetTy = MemRefBucket<LoadInst>::OffsetTy;
  using OffsetResultTy = std::tuple<bool, OffsetTy>;

  using LoadBucketMembers =
      SmallVector<MemRefBucket<LoadInst>::MemRefBucketMember,
                  DefaultBucketSize>;

  template <typename MemInstType>
  using BBBucketsTy = SmallVector<MemRefBucket<MemInstType>, DefaultNumBuckets>;

  using BBLoadBucketsTy = BBBucketsTy<LoadInst>;

  /// This function works on Basic Block and gathers Memory instructions
  void populateBasicBlockMemRefBuckets(BasicBlock *BB,
                                       bool AllowScalars = false);

  /// Returns the Load-Buckets to the clients.
  BBLoadBucketsTy const &getLoadBuckets() { return BBLoadBuckets; }
  const DataLayout &DL;
  BasicBlockMemRefAnalysis(ScalarEvolution &SE, const DataLayout &DL)
    : DL(DL), SE(SE) {}

private:
  ScalarEvolution &SE;

  BBLoadBucketsTy BBLoadBuckets;

  template <typename MemInstType>
  OffsetResultTy isConstantOffset(MemInstType *A, MemInstType *B);

  void sortBucketsByOffsetFromBasePointer();

  template <typename MemInstType> void insertIntoBucket(MemInstType *I);
};

} // namespace vpmemrefanalysis
} // namespace llvm

#endif // LLVM_ANALYSIS_INTEL_MEM_REF_ANALYSIS_H
