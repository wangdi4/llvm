//===--------------------DTransInfoAdapter.h--------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//
//
// This file contains adapter classes that enable a class to work with either
// the dtrans::DTransAnalysisInfo class or the dtransOP::DTransSafetyInfo class
// using a unified interface. The purpose of this is to enable a transition of
// some of the DTrans code that just needs the results of the DTrans data
// collection without needing to look at specific pointer types.
//
//===---------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error DTransInfoAdapter.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSINFOADAPTER_H
#define INTEL_DTRANS_ANALYSIS_DTRANSINFOADAPTER_H

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"

namespace llvm {
namespace dtransOP {

// Class to provide an interface that wraps the DTransSafetyInfo class.
class DTransSafetyInfoAdapter {
public:
  DTransSafetyInfoAdapter(DTransSafetyInfo &DTInfo) : DTInfo(DTInfo) {}

  dtrans::StructInfo *getStructTypeInfo(llvm::StructType *STy) const {
    return DTInfo.getStructInfo(STy);
  }

  bool testSafetyData(dtrans::TypeInfo *TyInfo, dtrans::Transform Transform) {
    return DTInfo.testSafetyData(TyInfo, Transform);
  }

  dtrans::FieldInfo &getFieldInfo(StructType *STy, unsigned Idx) {
    dtrans::StructInfo *StInfo = DTInfo.getStructInfo(STy);
    dtrans::FieldInfo &FI = StInfo->getField(Idx);
    return FI;
  }

  bool isFieldPtrToPtr(dtrans::StructInfo &StInfo, unsigned Idx) {
    dtrans::FieldInfo &FI = StInfo.getField(Idx);
    DTransType *Ty = FI.getDTransType();
    return Ty->isPointerTy() && Ty->getPointerElementType()->isPointerTy();
  }

  uint64_t getMaxTotalFrequency() const {
    return DTInfo.getMaxTotalFrequency();
  }

  std::pair<llvm::Type *, size_t>
  getByteFlattenedGEPElement(GetElementPtrInst *GEP) {
    return getByteFlattenedGEPElement(cast<GEPOperator>(GEP));
  }

  std::pair<llvm::Type *, size_t> getByteFlattenedGEPElement(GEPOperator *GEP) {
    std::pair<DTransType *, size_t> Element =
        DTInfo.getByteFlattenedGEPElement(GEP);
    if (Element.first)
      return {Element.first->getLLVMType(), Element.second};
    return {nullptr, 0};
  }

  llvm::Type *getResolvedPtrSubType(BinaryOperator *BinOp) {
    DTransType *DTy = DTInfo.getResolvedPtrSubType(BinOp);
    if (DTy)
      return DTy->getLLVMType();
    return nullptr;
  }

  std::pair<llvm::Type *, size_t> getLoadElement(LoadInst *LI) {
    return DTInfo.getLoadElement(LI);
  }

  std::pair<llvm::Type *, size_t> getStoreElement(StoreInst *SI) {
    return DTInfo.getStoreElement(SI);
  }

  std::pair<dtrans::StructInfo *, uint64_t> getInfoFromLoad(LoadInst *LI) {
    return DTInfo.getInfoFromLoad(LI);
  }

  bool isMultiElemLoadStore(Instruction *I) {
    return DTInfo.isMultiElemLoadStore(I);
  }

  dtrans::CallInfo *getCallInfo(const Instruction *I) const {
    return DTInfo.getCallInfo(I);
  }

  const dtrans::CallInfoVec *getCallInfoVec(const Instruction *I) const {
    return DTInfo.getCallInfoVec(I);
  }

  dtrans::CallInfoVec *getCallInfoVec(const Instruction *I) {
    return DTInfo.getCallInfoVec(I);
  }

  bool isReadOnlyFieldAccess(LoadInst *LI) const {
    return DTInfo.isReadOnlyFieldAccess(LI);
  }

  bool isPtrToStruct(Value *V) const { return DTInfo.isPtrToStruct(V); }

  bool isFunctionPtr(StructType *STy, unsigned Idx) {
    return DTInfo.isFunctionPtr(STy, Idx);
  }

  StructType *getPtrToStructElementType(Value *V) {
    return DTInfo.getPtrToStructElementType(V);
  }

  bool isPtrToStructWithI8StarFieldAt(Value *V, unsigned StructIndex) {
    return DTInfo.isPtrToStructWithI8StarFieldAt(V, StructIndex);
  }

  iterator_range<DTransSafetyInfo::type_info_iterator> type_info_entries() {
    return DTInfo.type_info_entries();
  }

  dtrans::TypeInfo *getFieldTypeInfo(dtrans::FieldInfo &FI) {
    return DTInfo.getTypeInfo(FI.getDTransType());
  }

  std::pair<llvm::StructType *, uint64_t> getStructField(GEPOperator *GEP) {
    return DTInfo.getStructField(GEP);
  }

  bool requiresBadCastValidation(SetVector<Function *> &Funcs,
                                 unsigned &ArgumentIndex,
                                 unsigned &StructIndex) {
    // TODO: Implementation required here.
    return true;
  }

  bool hasSupportedPaddedMallocPtrType(Value *V) {
    return DTInfo.isPtrToIntOrFloat(V);
  }

  bool hasSupportedPaddedMallocPtrTypeForFncReturn(Function *F) {
    return DTInfo.hasPtrToIntOrFloatReturnType(F);
  }

  bool hasSupportedPaddedMallocPtrType(dtrans::FieldInfo &FI) {
    return DTInfo.isPtrToIntOrFloat(FI);
  }

private:
  DTransSafetyInfo &DTInfo;
};

} // end namespace dtransOP
} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSINFOADAPTER_H
