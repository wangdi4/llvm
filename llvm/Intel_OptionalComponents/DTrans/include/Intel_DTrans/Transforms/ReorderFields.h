//===--------------- ReorderFields.h - DTransReorderFieldsPass ------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans reorder fields optimization pass.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_REORDERFIELDS_H
#define INTEL_DTRANS_TRANSFORMS_REORDERFIELDS_H

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

namespace dtrans {

// This has all the required information to do IR transformations for
// reordered structs.
class ReorderTransInfo {
public:
  // Set new size of \p StructT after reordering fields.
  void setTransformedTypeNewSize(StructType *StructT, uint64_t NewSize) {
    TransformedTypeNewSizeMap[StructT] = NewSize;
  }

  // Set \p NewIdx as new index for a field of \p StructT at \p OldIdx.
  void setTransformedIndexes(StructType *StructT,
                             std::vector<uint32_t> &IdxVec) {
    for (auto Idx : IdxVec)
      TransformedIndexes[StructT].push_back(Idx);
  }

  // Returns true if fields are reordered for any type.
  bool hasAnyTypeTransformed(void) {
    return (!TransformedTypeNewSizeMap.empty());
  }

  // Returns new size of \p StructT after reordering fields.
  uint64_t getTransformedTypeNewSize(StructType *StructT) {
    assert(TransformedTypeNewSizeMap.count(StructT) == 1 &&
           "Struct is not in the transformed list");
    return TransformedTypeNewSizeMap[StructT];
  }

  // Returns new index of a field of \p StructT at \p OldIdx index
  // after reordering fields.
  uint32_t getTransformedIndex(StructType *StructT, uint32_t OldIdx) {
    auto Itr = TransformedIndexes.find(StructT);
    assert(Itr != TransformedIndexes.end() &&
           "Struct type is not in the transformed list");
    return Itr->second[OldIdx];
  }

  DenseMap<StructType *, uint64_t> &getTypesMap() {
    return TransformedTypeNewSizeMap;
  }

private:
  // Map of <Reordered_Struct, Vector of new indexes>
  DenseMap<StructType *, SmallVector<uint32_t, 8>> TransformedIndexes;

  // Map of <Reordered_Struct, New Size of Ordered Struct>
  DenseMap<StructType *, uint64_t> TransformedTypeNewSizeMap;
};

/// Pass to perform DTrans optimizations.
class ReorderFieldsPass : public PassInfoMixin<dtrans::ReorderFieldsPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool runImpl(Module &M, DTransAnalysisInfo &Info,
               const TargetLibraryInfo &TLI, WholeProgramInfo &WPInfo);

private:
  // The pointers in this vector are owned by the DTransAnalysisInfo.
  SmallVector<dtrans::StructInfo *, 4> CandidateTypes;

  // This has all the info required to do IR transform.
  ReorderTransInfo RTI;

  bool gatherCandidateTypes(DTransAnalysisInfo &DTInfo, const DataLayout &DL);
  bool doesTypeMeetReorderRestrictions(StructType *StructT);
  bool isCandidateType(StructType *StructT, const DataLayout &DL);
  bool isCandidateTypeHasEnoughPadding(StructType *StructT,
                                       const DataLayout &DL);
  void collectReorderTransInfoIfProfitable(TypeInfo *TI, const DataLayout &DL);
};

} // namespace dtrans

ModulePass *createDTransReorderFieldsWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_REORDERFIELDS_H
