//===--------------- ReorderFields.h - DTransReorderFieldsPass ------------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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
#include "Intel_DTrans/Transforms/DTransOptBase.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

namespace dtrans {

// The ReorderFieldTransInfo class has all the required information to do
// DTrans::ReorderField transformation for candidate structs.
class ReorderFieldTransInfo {
private:
  // Map of <Reordered_Struct, Vector of new indexes>
  DenseMap<StructType *, SmallVector<uint32_t, 8>> TransformedIndexes;

  // Map of <Reordered_Struct, Size of Reordered Struct>
  DenseMap<StructType *, uint64_t> TransformedTypeNewSizeMap;

  // Collection of module-level Inclusive Struct Types
  //
  // [Inclusive Struct Type]
  // Consider an inheritance case: class B: public A { ... }
  //
  // This is reflected on a type system as:
  // struct A {...};
  // struct B { struct A a; ...}
  //
  // Since struct B type includes a complete copy of struct A type, B is called
  // an inclusive struct type of A.
  // Any type that is directly or indirectly derived from struct A type is also
  // an inclusive struct type of A.
  //
  std::vector<StructType *> InclusiveStructTypeV;

  // Type map for InclusiveStructType
  // E.g.
  // Src Type        |   Dst Type
  // %class.cPacket  ->  %__DFR_class.cPacket
  // ...
  std::map<StructType *, StructType *> InclusiveStructTypeMap;

  // Type (un)map for InclusiveStructType
  // E.g.
  // Dst Type              |  Src Type
  // %__DFR_class.cPacket  -> %class.cPacket
  // ...
  std::map<StructType *, StructType *> InclusiveStructTypeUnmap;
  std::map<Function *, bool> InclusiveStructTypeMapped;

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

  // Return true if there is any type that has been mapped for its
  // reordered-type's size
  bool hasAnyTypeTransformed(void) {
    return (!TransformedTypeNewSizeMap.empty());
  }

  // Return new size of \p StructT after reordering fields.
  uint64_t getTransformedTypeNewSize(StructType *StructT) {
    assert(TransformedTypeNewSizeMap.count(StructT) == 1 &&
        "Struct is not in the transformed list");
    return TransformedTypeNewSizeMap[StructT];
  }

  // Return true if StructT is a struct type with field reordering.
  //
  // TransformedTypeNewSizeMap is a map: StructType -> uint64_t.
  // Since a struct type can only have 1 size,
  // TransformedTypeNewSizeMap.count(Ty) == 1 means that Type ty has a valid
  // entry in the map.
  //
  bool hasTransformedTypeNewSize(StructType *StructT) {
    return (TransformedTypeNewSizeMap.count(StructT) == 1);
  }

  // Return new index of a field of \p StructT at \p OldIdx index
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

  std::vector<StructType *> &getInclusiveStructTypes() {
    return InclusiveStructTypeV;
  }

  // Return true if InclusiveStructType vector is not empty
  bool hasInclusiveStructType() { return !InclusiveStructTypeV.empty(); }

  // Return true if a given StructType* is an Inclusive Struct Type
  bool hasInclusiveStructType(StructType *StTy) {
    return std::find(InclusiveStructTypeV.begin(), InclusiveStructTypeV.end(),
                     StTy)
        != InclusiveStructTypeV.end();
  }

  unsigned getInclusiveStructTypeSize() { return InclusiveStructTypeV.size(); }

  // Return true if InclusiveStructTypeMap has a valid entry for OrigTy*
  bool hasInclusiveStructOrigType(StructType *OrigTy) {
    return (InclusiveStructTypeMap.count(OrigTy) == 1);
  }

  // Return true if InclusiveStructTypeUnmap has a valid entry for ReorderedTy*
  bool hasInclusiveStructReorderedType(StructType *ReorderedTy) {
    return (InclusiveStructTypeUnmap.count(ReorderedTy) == 1);
  }

  bool isInclusiveStructTypeMapped(Function *F) { return InclusiveStructTypeMapped[F]; }

  // map Inclusive Struct Type(s) both ways
  bool doInclusiveStructTypeMap(DTransTypeRemapper *TypeRemapper, Function *F);

  // Map: OrigTy -> ReorderTy
  StructType *mapInclusiveStructType(StructType *OrigTy) {
    return InclusiveStructTypeMap[OrigTy];
  }

  // (Un)Map: ReorderTy -> OrigTy
  StructType *unmapInclusiveStructType(StructType *ReorderTy) {
    return InclusiveStructTypeUnmap[ReorderTy];
  }

  // Given a valid InclusiveStructType, obtain its base StructType that is
  // a valid DTrans's Field-Reorder (DFR) candidate.
  StructType *getDFRCandidateStructType(StructType *OrigInclusiveStructTy) {
    // Expect OrigInclusiveStructTy to be a valid Inclusive Struct Type
    if (!hasInclusiveStructType(OrigInclusiveStructTy))
      return nullptr;

    auto &TypesMap = getTypesMap();
    StructType *StructTy = OrigInclusiveStructTy;

    while (true) {
      StructType *CandidateStTy =
          dyn_cast<StructType>(StructTy->getElementType(0));
      if (!CandidateStTy)
        break;

      if (TypesMap.find(CandidateStTy) != TypesMap.end())
        return CandidateStTy;

      StructTy = CandidateStTy;

    }

    return nullptr;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dumpMappedIndex() const;
  void dumpMappedSize() const;
  void dumpInclusiveStructTypes() const;
  void dump() const;
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
};

/// Pass to perform DTrans::ReorderField optimization.
class ReorderFieldsPass : public PassInfoMixin<dtrans::ReorderFieldsPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool
  runImpl(Module &M, DTransAnalysisInfo &Info,
          std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
          WholeProgramInfo &WPInfo);

private:
  // Collection of suitable StructInfo* types for field reordering
  SmallVector<dtrans::StructInfo *, 4> CandidateTypeV;

  // Reorder-field transformation Information
  ReorderFieldTransInfo RTI;

  bool doCollection(DTransAnalysisInfo &DTInfo, const DataLayout &DL,
                    const Module &M);

  bool isLegal(TypeInfo *TI, DTransAnalysisInfo &DTInfo);
  bool isApplicable(TypeInfo *TI, const DataLayout &DL);
  bool isProfitable(TypeInfo *TI, const DataLayout &DL);

public:
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const {
    dumpCandidateTypes();
    dumpRTI();
  }

  void dumpCandidateTypes() const {
    dbgs() << "Total Candidates: " << CandidateTypeV.size() << "\n";
    unsigned Count = 0;
    for (auto *StInfo : CandidateTypeV) {
      dbgs() << Count++ << " : ";
      StInfo->getLLVMType()->dump();
    }
  }

  void dumpRTI() const { RTI.dump(); }
#endif //#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

};

} // namespace dtrans

ModulePass *createDTransReorderFieldsWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_REORDERFIELDS_H
