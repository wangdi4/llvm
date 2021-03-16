//===--- HIRArrayContractionUtils.h -------------------------*- C++ -*---===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIR_ARRAY_CONTRACTION_UTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIR_ARRAY_CONTRACTION_UTILS_H

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/Pass.h"

namespace llvm {
namespace loopopt {

class DDGraph;
class HIRDDAnalysis;
class HIRLoopStatistics;

namespace arraycontractionutils {

// -----------------------------------------------------------------------
// HIRArrayContractionUtil is a utility for array contraction related APIs.
// Its main goal is to contract individual memref(s) from high-dimension to
// low-dimension.
//
// -----------------------------------------------------------------------
// Example of contracting a single high-dimension memref into its equivalent
// low-dimension counter part.
//
// [BEFORE]
//  ..
//    A[1][2][i][j][k] = . // A[] is a 5-dimension memref
//  ..
//
// When information is provided to specify that dimensions 1,2,and 3 are to
// be contracted, the memref will become:
//
// [AFTER]
//  ..
//    AA[1][2] = . // AA[] is a 2-dimension memref after contration from A[]
//  ..
//
// [Note]
// - A is a 5-dimension array and AA is a 2-dimension array.
//   The mapping between A and AA is maintained internally by the utility.
//
// - The contraction API will figure out the size of the contracted array (AA)
//   by examining the size of the original array (A) and the dimensions remained
//   after contraction (in AA).
//
// - Array A an AA are consistent in global/local scope.
//   If A is a global array, its contracted counterpart (AA) will also be a
//   global array.
//   If A is a local array, its contracted counterpart (AA) will also be a local
//   array.
//   Currently, only local array is supported.
//
// -----------------------------------------------------------------------

class HIRArrayContractionUtil {
  HIRArrayContractionUtil() = delete;

  // ** 2-way mapping of refs before and after contraction **
  // forward map: BeforeContractRef -> AfterContractRef
  static DenseMap<RegDDRef *, RegDDRef *> Pre2PostMap;
  // reverse map: AfterContractRef -> BeforeContractRef
  static DenseMap<RegDDRef *, RegDDRef *> Post2PreMap;

  // Storage allocation record mapping:
  // (MemRef's BaseCE Blob Index, ContractDims) -> AllocaInst
  static DenseMap<std::pair<unsigned, unsigned>, HLInst *>
      StorageBlobIdxAllocaMap;

  // Map: contracted memref's BasePtrBlobIndex -> Symbase
  static DenseMap<unsigned, unsigned> ContractedRefBlob2SB;

  // Check sanity of a given MemRef
  // Items to check:
  // - MemRef: ArrayType RegDDRef
  // - ToContractDims: non empty, need to be consecutive
  // - PreservedDims: non empty, anything not in ToContractDims
  // - ToContractDims and PreservedDims: mutually exclusive, combined size
  //   matches ref's #dims
  //
  static bool checkSanity(
      RegDDRef *Ref,                         /* Inout: Ref to check sanity */
      SmallSet<unsigned, 4> &PreservedDims,  /* Input: Dims preserved */
      SmallSet<unsigned, 4> &ToContractDims, /* Input: Dims to contract */
      SmallVectorImpl<unsigned>
          &DimSizeVec, /* Output: Dimension Sizes of after-contract type */
      Type *&RootTy /* Output : ArrayType's Root type */);

  // Check if new storage has been allocated for the Ref
  static bool isStorageAllocated(
      RegDDRef *Ref,                         /* Input: Ref to contract */
      SmallSet<unsigned, 4> &PreservedDims,  /* Input: Dims to preserve */
      SmallSet<unsigned, 4> &ToContractDims, /* Input: Dims to contract */
      HLInst *&AllocaInst /* Output: existing or new allocated storage */
  );

  // Allocate storage for new array
  // [Note]
  // - only support stack allocation at the moment.
  //   (May add heap support in future when needed.)
  static bool
  allocateStorage(RegDDRef *Ref,                         /* Ref to contract */
                  SmallSet<unsigned, 4> &PreservedDims,  /* Dims preserved */
                  SmallSet<unsigned, 4> &ToContractDims, /* Dims to contract */
                  HLRegion &Reg, /* The relevant region */
                  SmallVectorImpl<unsigned>
                      &DimSizeVec, /* Dim Sizes of after-contract type */
                  Type *&RootTy,
                  HLInst *&AllocaInst /* Output: Alloca created */
  );

  // Contract a given memref, obtain its contracted counterpart memref
  static bool
  contract(RegDDRef *Ref,
           SmallSet<unsigned, 4> &PreservedDims,  /* Input: Dims preserved */
           SmallSet<unsigned, 4> &ToContractDims, /* Input: Dims to contract */
           HLInst *AllocInst,                     /* Input: Alloca created */
           RegDDRef *&ContractedRef /* Output: */);

  // Search StorageAllocMap for a given before-contract ref,
  // return the relevant HLAllocInst that allocates storage.
  static HLInst *getHLAllocaInst(RegDDRef *ToContractRef,
                                 SmallSet<unsigned, 4> &ToContractDims) {
    return StorageBlobIdxAllocaMap[std::make_pair(
        ToContractRef->getBasePtrBlobIndex(), ToContractDims.size())];
  }

  static void addSBToLoopnestLiveIn(HLLoop *Lp, const unsigned SB);

public:
  static bool contractMemRef(
      RegDDRef *ToContractRef,               /* Input: Ref to contract */
      SmallSet<unsigned, 4> &PreservedDims,  /* Input: Dims preserved */
      SmallSet<unsigned, 4> &ToContractDims, /* Input: Dims to contract */
      HLRegion &Reg,                         /* Input: the region */
      RegDDRef *&AfterContractRef,           /* Output: Ref after contraction */
      unsigned &AfterContractSB /* Output: Symbase of the contracted ref */
  );

  // Give a BeforeContract Ref, find its matching after-contract Ref.
  static RegDDRef *getAfterContractRef(RegDDRef *BeforeContractRef) {
    return Pre2PostMap[BeforeContractRef];
  }

  // Give a AfterContract Ref, find its corresponding before-contract Ref.
  static RegDDRef *getBeforeContractRef(RegDDRef *AfterContractRef) {
    return Post2PreMap[AfterContractRef];
  }

  // For a given AfterContractMemRef:
  // -if its BasePtrBlobIndex is already recorded, return true with its Symbase.
  // -otherwise, create a new symbase, record it and return false;
  static bool getOrCreateRefSB(RegDDRef *Ref, DDRefUtils &DDRU,
                               unsigned &SymBase);

  // Debug Printers:
  void printPre2PostMap(formatted_raw_ostream &FOS);
  void printPost2PreMap(formatted_raw_ostream &FOS);
  void printStorageAllocaMap(formatted_raw_ostream &FOS);
  void printContractRefBlob2SBMap(formatted_raw_ostream &FOS);

#ifndef NDEBUG
  LLVM_DUMP_METHOD void dumpPre2PostMap(void) {
    formatted_raw_ostream FOS(dbgs());
    printPre2PostMap(FOS);
  }

  LLVM_DUMP_METHOD void dumpPost2PreMap(void) {
    formatted_raw_ostream FOS(dbgs());
    printPost2PreMap(FOS);
  }

  LLVM_DUMP_METHOD void dumpStoragAllocaMap(void) {
    formatted_raw_ostream FOS(dbgs());
    printStorageAllocaMap(FOS);
  }

  LLVM_DUMP_METHOD void dumpContractRefBlob2SBMap(void) {
    formatted_raw_ostream FOS(dbgs());
    printContractRefBlob2SBMap(FOS);
  }

  LLVM_DUMP_METHOD void dump(bool PrintPre2PostMap = true,
                             bool PrintPost2PreMap = false,
                             bool PrintStorageAllocaMap = true,
                             bool PrintContractRefBlob2SBMap = true) {
    formatted_raw_ostream FOS(dbgs());

    if (PrintPre2PostMap) {
      printPre2PostMap(FOS);
    }

    if (PrintPost2PreMap) {
      printPost2PreMap(FOS);
    }

    if (PrintStorageAllocaMap) {
      printStorageAllocaMap(FOS);
    }

    if (PrintContractRefBlob2SBMap) {
      printContractRefBlob2SBMap(FOS);
    }
  }
#endif
};

} // namespace arraycontractionutils
} // namespace loopopt
} // namespace llvm
#endif
