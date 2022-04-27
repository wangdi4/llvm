//===--------------------DTransInfoAdapter.h--------------------------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
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
    dtrans::TypeInfo *TI = DTInfo.getTypeInfo(STy);
    assert(TI && "visitModule() should create all TypeInfo objects");
    return cast<dtrans::StructInfo>(TI);
  }

  bool testSafetyData(dtrans::TypeInfo *TyInfo, dtrans::Transform Transform) {
    return DTInfo.testSafetyData(TyInfo, Transform);
  }

  dtrans::FieldInfo &getFieldInfo(StructType *STy, unsigned Idx) {
    auto *StInfo = cast<dtrans::StructInfo>(DTInfo.getTypeInfo(STy));
    dtrans::FieldInfo &FI = StInfo->getField(Idx);
    return FI;
  }

  bool isFieldPtrToPtr(dtrans::StructInfo &StInfo, unsigned Idx) {
    dtrans::FieldInfo &FI = StInfo.getField(Idx);
    llvm::Type *Ty = FI.getLLVMType();
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

  bool isPtrToStruct(Value *V) const {
    auto PTy = dyn_cast<PointerType>(V->getType());
    if (!PTy)
      return false;
    return isa<StructType>(PTy->getElementType());
  }

  bool isFunctionPtr(StructType *STy, unsigned Idx) {
    if (Idx >= STy->getNumElements())
      return false;
    auto PTy = dyn_cast<PointerType>(STy->getElementType(Idx));
    if (!PTy)
      return false;
    return isa<FunctionType>(PTy->getElementType());
  }

  StructType *getPtrToStructElementType(Value *V) {
    auto PTy = dyn_cast<PointerType>(V->getType());
    if (!PTy)
      return nullptr;
    return dyn_cast<StructType>(PTy->getPointerElementType());
  }

  bool isPtrToStructWithI8StarFieldAt(Value *V, unsigned StructIndex) {
    auto PTy = dyn_cast<PointerType>(V->getType());
    if (!PTy)
      return false;
    auto STy = dyn_cast<StructType>(PTy->getElementType());
    if (!STy || StructIndex >= STy->getNumElements())
      return false;
    auto SFTy = STy->getTypeAtIndex(StructIndex);
    if (!SFTy || !SFTy->isPointerTy())
      return false;
    return SFTy->getPointerElementType()->isIntegerTy(8);
  }

  iterator_range<DTransAnalysisInfo::type_info_iterator> type_info_entries() {
    return DTInfo.type_info_entries();
  }

  dtrans::TypeInfo *getFieldTypeInfo(dtrans::FieldInfo &FI) {
    return DTInfo.getTypeInfo(FI.getLLVMType());
  }

  std::pair<llvm::StructType *, uint64_t> getStructField(GEPOperator *GEP) {
    return DTInfo.getStructField(GEP);
  }

  bool requiresBadCastValidation(SetVector<Function *> &Funcs,
                                 unsigned &ArgumentIndex,
                                 unsigned &StructIndex) {
    return DTInfo.requiresBadCastValidation(Funcs, ArgumentIndex, StructIndex);
  }

  bool hasSupportedPaddedMallocPtrType(Value *V) {
    auto PTy = dyn_cast<PointerType>(V->getType());
    if (!PTy)
      return false;
    Type *PETy = PTy->getPointerElementType();
    return PETy && (PETy->isIntegerTy() || PETy->isFloatingPointTy());
  }

  bool hasSupportedPaddedMallocPtrTypeForFncReturn(Function *F) {
    auto PTy = dyn_cast<PointerType>(F->getReturnType());
    if (!PTy)
      return false;
    Type *PETy = PTy->getPointerElementType();
    return PETy && (PETy->isIntegerTy() || PETy->isFloatingPointTy());
  }

  bool hasSupportedPaddedMallocPtrType(dtrans::FieldInfo &FI) {
    auto PTy = dyn_cast<PointerType>(FI.getLLVMType());
    if (!PTy)
      return false;
    Type *PETy = PTy->getPointerElementType();
    return PETy && (PETy->isIntegerTy() || PETy->isFloatingPointTy());
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
