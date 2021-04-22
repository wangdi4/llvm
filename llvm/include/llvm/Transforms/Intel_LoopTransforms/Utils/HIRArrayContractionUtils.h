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
  // Allocate storage for new array
  // [Note]
  // - only support stack allocation at the moment.
  //   (May add heap support in future when needed.)
  static bool
  allocateStorage(RegDDRef *Ref, /* Ref to contract */
                  HLRegion &Reg, /* The relevant region */
                  SmallVectorImpl<unsigned>
                      &DimSizeVec, /* Dim Sizes of after-contract type */
                  Type *RootTy,
                  RegDDRef *&AfterContractRef, /* Existing contracted ref */
                  unsigned &AllocaBlobIndex);  /* Output: alloca blob index */

  // Contract a given memref, obtain its contracted counterpart memref
  static void
  contract(RegDDRef *Ref,
           SmallSet<unsigned, 4> &PreservedDims,  /* Input: Dims preserved */
           SmallSet<unsigned, 4> &ToContractDims, /* Input: Dims to contract */
           unsigned AllocaBlobIndex,              /* Input: Alloca blob index */
           RegDDRef *&ContractedRef /* Output: */);

  static void addSBToLoopnestLiveIn(HLLoop *Lp, const unsigned SB);

public:
  // If \p AfterContractRef is null, new alloca is created for contraction
  // otherwise alloca information is extracted from this ref.
  static bool contractMemRef(
      RegDDRef *ToContractRef,               /* Input: Ref to contract */
      SmallSet<unsigned, 4> &PreservedDims,  /* Input: Dims preserved */
      SmallSet<unsigned, 4> &ToContractDims, /* Input: Dims to contract */
      HLRegion &Reg,                         /* Input: the region */
      RegDDRef *&AfterContractRef            /* Output: Ref after contraction */
  );
};

} // namespace arraycontractionutils
} // namespace loopopt
} // namespace llvm
#endif
