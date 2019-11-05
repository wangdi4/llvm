//===------------- SOAToAOSClassInfo.h - SOAToAOS Class Info Analysis -----===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file is used to analyze the functionality of member functions of
// vector class.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_SOATOAOS_CLASSINFO_H
#define INTEL_DTRANS_TRANSFORMS_SOATOAOS_CLASSINFO_H

#if !INTEL_INCLUDE_DTRANS
#error SOAToAOSClassInfo.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#include "Intel_DTrans/Transforms/MemInitTrimDownInfoImpl.h"

namespace llvm {

namespace dtrans {

// Member functions of vector class are classified based on functionality.
enum FunctionKind {
  Constructor,       // Constructor
  CopyConstructor,   // Copy-constructor
  Destructor,        // Destructor
  DestructorWrapper, // DestructorWrapper
  Resize,            // Reallocating element array
  GetSize,           // Get size of element array
  GetCapacity,       // Get capacity of element array
  SetElem,           // Set element at given position
  GetElem,           // Get element at given position
  AppendElem,        // Add element to the array
  GetSizeOrCapacity, // Return either size or capacity of element array.
  UnKnown            // Unknown functionality.
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Dumper for FunctionKind.
inline raw_ostream &operator<<(raw_ostream &OS, FunctionKind FKind) {
  switch (FKind) {
  case Constructor:
    OS << "Constructor";
    break;
  case CopyConstructor:
    OS << "CopyConstructor";
    break;
  case Destructor:
    OS << "Destructor";
    break;
  case DestructorWrapper:
    OS << "DestructorWrapper";
    break;
  case Resize:
    OS << "Resize";
    break;
  case GetSize:
    OS << "GetSize";
    break;
  case GetCapacity:
    OS << "GetCapacity";
    break;
  case SetElem:
    OS << "SetElem";
    break;
  case GetElem:
    OS << "GetElem";
    break;
  case AppendElem:
    OS << "AppendElem";
    break;
  case GetSizeOrCapacity:
    OS << "GetSizeOrCapacity";
    break;
  case UnKnown:
    OS << "Unknown";
    break;
  }
  return OS;
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// The purpose of this class is to detect and prove the functionality
// of each member function of vector class.
class ClassInfo {
public:
  // Max limit for number of fields in candidate struct.
  constexpr static int MinCapacityLimit = 1;
  constexpr static int MaxCapacityLimit = 4;
  constexpr static int NewCapacityValue = 1;

  using EscapePointTy = SmallSet<std::pair<Function *, int32_t>, 2>;
  using CapacityValuesTy = SmallPtrSet<Value *, 4>;

  ClassInfo(const DataLayout &DL, DTransAnalysisInfo &DTInfo,
            MemGetTLITy GetTLI, MemInitDominatorTreeType GetDT,
            MemInitCandidateInfo *MICInfo, int32_t FieldIdx, bool RecognizeAll)
      : DL(DL), DTInfo(DTInfo), GetTLI(GetTLI), GetDT(GetDT), MICInfo(MICInfo),
        FieldIdx(FieldIdx), RecognizeAll(RecognizeAll){};

  // Analyze each member function to detect functionality.
  bool analyzeClassFunctions();

  // Returns index of "flag" field.
  int32_t getFlagField() { return FlagField; }

  // Returns CtorFunction.
  Function *getCtorFunction() { return CtorFunction; }

  // Returns CapacityInitInst.
  StoreInst *getCapacityInitInst() { return CapacityInitInst; }

  // Returns MemsetInCtor.
  CallBase *getMemsetInCtor() { return MemsetInCtor; }

  // Returns type of field element array.
  Type *getElementTy() { return MICInfo->getFieldElemTy(FieldIdx); }

  // Returns size of field element array.
  int64_t getElemTySize() {
    return DL.getTypeAllocSize(MICInfo->getFieldElemTy(FieldIdx));
  }

  // Returns FinalFuncKind for given Fn.
  FunctionKind getFinalFuncKind(Function *Fn) { return FinalFuncKind[Fn]; }

  // Returns size of AllocsInCtor.
  int32_t getAllocsInCtorSize() { return AllocsInCtor.size(); }

  // Returns field position of vector class.
  int32_t getFieldIdx() { return FieldIdx; }

  // Returns true if F is member function of candidate struct.
  bool isCandidateStructMethod(Function *F) {
    return MICInfo->isStructMethod(F);
  }

  // Returns true if F is member function of candidate field vector class.
  bool isCandidateMemberFunction(Function *F) {
    return MICInfo->isMemberFunction(F, FieldIdx);
  }

  // Returns iterator for member functions of field element class.
  using VectorMethodSetTy = SmallPtrSet<Function *, 10>;
  typedef VectorMethodSetTy::const_iterator m_const_iterator;
  inline iterator_range<m_const_iterator> field_member_functions() {
    return MICInfo->member_functions(FieldIdx);
  }

  // Iterator for AllocsInCtor set.
  using AllocsInCtorTy = SmallSet<std::pair<CallInst *, unsigned>, 2>;
  typedef AllocsInCtorTy::const_iterator a_const_iterator;
  inline iterator_range<a_const_iterator> allocs_in_ctor() {
    return make_range(AllocsInCtor.begin(), AllocsInCtor.end());
  }

  Function *getCtorWrapper();

  Function *getDtorWrapper();

private:
  const DataLayout &DL;
  DTransAnalysisInfo &DTInfo;
  MemGetTLITy GetTLI;
  MemInitDominatorTreeType GetDT;

  // Candidate struct info.
  MemInitCandidateInfo *MICInfo;

  // Field position of potential vector class in candidate struct.
  int32_t FieldIdx;

  // If true, try to analyze all member functions.
  bool RecognizeAll;

  // Index of "size" field in vector class.
  int32_t SizeField = -1;

  // Index of "capacity" field in vector class.
  int32_t CapacityField = -1;

  // This is the index where pointer of MemoryInterface is saved.
  int32_t MemIntField = -1;

  // This is the index where pointer of actual array is saved.
  int32_t ArrayField = -1;

  // Index of "flag" field in vector class.
  int32_t FlagField = -1;

  // Store instruction (in Constructor) where "capacity" field is
  // initialized.
  StoreInst *CapacityInitInst = nullptr;

  // Due to SOAToAOS transformation, element of array can be a struct.
  // If element of array is struct, each field of the struct is treated
  // as different data-element. This is used to maintain list of all
  // data elements.
  SmallPtrSet<Type *, 4> ElemDataTypes;

  // While recognizing functionality of member functions, this is used
  // to maintain all processed instructions.
  SmallPtrSet<const Instruction *, 32> Visited;

  // Mapping between member function and its actual functionality that
  // is recognized by analyzing the function.
  DenseMap<Function *, FunctionKind> FinalFuncKind;

  // Mapping between member function and its assumed functionality that
  // is categorized using signature.
  DenseMap<Function *, FunctionKind> InitialFuncKind;

  // Ctor function.
  Function *CtorFunction = nullptr;

  // MemSet instruction that resets newly allocated memory in Ctor.
  CallBase *MemsetInCtor = nullptr;

  // Alloc instructions that allocates memory for array in Ctor.
  AllocsInCtorTy AllocsInCtor;

  void collectElementDataTypes();
  bool checkDominatorInfo(Instruction *, Instruction *);
  FunctionKind categorizeFunctionUsingSignature(Function *, StructType *);
  bool isAccessingFieldOfArgClass(const GetElementPtrInst *, Value *,
                                  int32_t *);
  bool isAccessingVTableFieldInBaseClass(const GetElementPtrInst *, Argument *);
  bool checkFieldOfArgClassLoad(const Value *, Value *, int32_t);
  bool checkMemInterfacePointer(Value *, Argument *);
  const Value *computeMultiplier(const Value *, int64_t *);
  bool checkAllocSizeOfArray(const Value *, Value *, Value *);
  bool checkAllocCall(Value *, Argument *, Value *, bool);
  bool checkVtableLoadOfMemInt(Value *, Argument *);
  bool isIndirectCallCheck(Value *, Argument *);
  bool checkAllocatedArrayPtr(Value *, Argument *, SmallPtrSetImpl<Value *> &,
                              Value *, bool);
  bool checkBBControlledUnderCapacityVal(BasicBlock *, Value *);
  bool checkBBControlledUnderFlagVal(BasicBlock *, Argument *);
  bool checkMemset(SmallPtrSetImpl<Value *> &, Value *, bool);
  bool checkAllInstsProcessed(Function *,
                              SmallPtrSetImpl<const Instruction *> &);
  Value *isIntegerArgument(Value *);
  Value *isArrayElementAt(const Value *, Value *, bool);
  Value *isArrayElementLoadAt(const Value *, Value *, bool);
  Value *isArrayElementAddressAt(Value *, Type *, Value *, int *, bool);
  Value *isArrayStructElementLoadAt(Value *, Type *, Value *, int *, bool);
  Value *isElementIndexAddress(const Value *Val, Value *Addr);
  Value *isStructElementIndexAddress(const Value *, Type *, Value *, int *);
  bool checkCompleteObjPtr(const Value *, Argument *);
  bool checkEHBlock(BasicBlock *, Argument *);
  bool isEHRelatedBB(BasicBlock *, Argument *);
  BasicBlock *getBBControlledOps(BasicBlock *, Value **, Value **,
                                 ICmpInst::Predicate *Pred);
  bool isControlledUnderSizeCheck(BasicBlock *, Argument *, Value *);
  FreeCallInfo *getFreeCall(Instruction *I);
  const Value *checkFree(FreeCallInfo *, Argument *, BasicBlock **);
  const Value *getFreeArg(FreeCallInfo *);
  Value *checkCondition(BasicBlock *, BasicBlock *);
  bool checkZTT(BasicBlock *, Value *);
  Loop *checkLoop(Value *, Value *, LoopInfo &);
  Loop *checkLoopWithZTT(Value *, Value *, LoopInfo &);
  bool checkCapacityIncrement(Value *);
  ReturnInst *getSingleRetInst(Function *);
  void collectStoreInstsFreeCalls(Function *, SmallPtrSetImpl<BasicBlock *> &,
                                  SmallPtrSetImpl<StoreInst *> &,
                                  SmallPtrSetImpl<FreeCallInfo *> &);
  bool checkCapacityIncrementPattern(Value *, Argument *);
  bool isControlledUnderCapacityField(BasicBlock *, Value *, Value *);
  Value *isLoadOfArg(Value *);
  const Value *skipCasts(const Value *V);
  FunctionKind recognizeConstructor(Function *);
  FunctionKind recognizeDerivedConstructor(Function *, Type *, Type *);
  FunctionKind recognizeGetSizeOrCapacity(Function *);
  FunctionKind recognizeGetElem(Function *);
  FunctionKind recognizeSetElem(Function *);
  FunctionKind recognizeDestructor(Function *);
  FunctionKind recognizeCopyConstructor(Function *);
  FunctionKind recognizeAppendElem(Function *);
  FunctionKind recognizeResize(Function *);
};

} // namespace dtrans

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOS_CLASSINFO_H
