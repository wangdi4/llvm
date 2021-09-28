//===--------------------DTransInfoAdapter.h--------------------------===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
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

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"

namespace llvm {
namespace dtrans {

// Class to provide an interface that wraps the DTransAnalysisInfo class.
class DTransAnalysisInfoAdapter {
public:
  DTransAnalysisInfoAdapter(DTransAnalysisInfo &DTInfo) : DTInfo(DTInfo) {}

  dtrans::StructInfo *getStructTypeInfo(llvm::StructType *STy) const {
    return cast<dtrans::StructInfo>(DTInfo.getTypeInfo(STy));
  }

  bool testSafetyData(dtrans::TypeInfo *TyInfo, dtrans::Transform Transform) {
    return DTInfo.testSafetyData(TyInfo, Transform);
  }

  dtrans::FieldInfo getFieldInfo(StructType *STy, unsigned Idx) {
    auto *StInfo = cast<dtrans::StructInfo>(DTInfo.getTypeInfo(STy));
    dtrans::FieldInfo &FI = StInfo->getField(Idx);
    return FI;
  }

  uint64_t getMaxTotalFrequency() const {
    return DTInfo.getMaxTotalFrequency();
  }

  std::pair<llvm::Type *, size_t>
  getByteFlattenedGEPElement(GetElementPtrInst *GEP) {
    return getByteFlattenedGEPElement(cast<GEPOperator>(GEP));
  }

  std::pair<llvm::Type *, size_t> getByteFlattenedGEPElement(GEPOperator *GEP) {
    return DTInfo.getByteFlattenedGEPElement(GEP);
  }

  llvm::Type *getResolvedPtrSubType(BinaryOperator *BinOp) {
    return DTInfo.getResolvedPtrSubType(BinOp);
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

  bool isReadOnlyFieldAccess(LoadInst *LI) const {
    return DTInfo.isReadOnlyFieldAccess(LI);
  } 

  bool isPtrToStruct(Argument *A) const {
    auto PTy = dyn_cast<PointerType>(A->getType());
    if (!PTy)
      return false;
    return isa<StructType>(PTy->getElementType());
  }

  bool isFunctionPtr(StructType *STy, unsigned Idx) {
    auto PTy = dyn_cast<PointerType>(STy->getElementType(Idx));
    if (!PTy)
      return false;
    return isa<FunctionType>(PTy->getElementType());
  } 

private:
  DTransAnalysisInfo &DTInfo;
};
} // end namespace dtrans

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

  dtrans::FieldInfo getFieldInfo(StructType *STy, unsigned Idx) {
    dtrans::StructInfo *StInfo = DTInfo.getStructInfo(STy);
    dtrans::FieldInfo &FI = StInfo->getField(Idx);
    return FI;
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
  
  bool isReadOnlyFieldAccess(LoadInst *LI) const {
    return DTInfo.isReadOnlyFieldAccess(LI);
  } 

  bool isPtrToStruct(Argument *A) const {
    return DTInfo.isPtrToStruct(A); 
  }

  bool isFunctionPtr(StructType *STy, unsigned Idx) {
    return DTInfo.isFunctionPtr(STy, Idx);
  } 

private:
  DTransSafetyInfo &DTInfo;
};

} // end namespace dtransOP
} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSINFOADAPTER_H
