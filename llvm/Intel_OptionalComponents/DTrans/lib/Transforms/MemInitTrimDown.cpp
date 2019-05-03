//===---------------- MemInitTrimDown.cpp - DTransMemInitTrimDownPass -----===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Initial Memory Allocation Trim Down
// optimization pass.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/MemInitTrimDown.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/MemInitTrimDownInfoImpl.h"

#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

using namespace llvm;

#define DTRANS_MEMINITTRIMDOWN "dtrans-meminittrimdown"

namespace {

class DTransMemInitTrimDownWrapper : public ModulePass {
private:
  dtrans::MemInitTrimDownPass Impl;

public:
  static char ID;

  DTransMemInitTrimDownWrapper() : ModulePass(ID) {
    initializeDTransMemInitTrimDownWrapperPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    DTransAnalysisWrapper &DTAnalysisWrapper =
        getAnalysis<DTransAnalysisWrapper>();
    DTransAnalysisInfo &DTInfo = DTAnalysisWrapper.getDTransInfo(M);

    dtrans::MemInitDominatorTreeType GetDT =
        [this](Function &F) -> DominatorTree & {
      return this->getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    };

    bool Changed = Impl.runImpl(
        M, DTInfo, getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(),
        getAnalysis<WholeProgramWrapperPass>().getResult(), GetDT);
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<DTransAnalysisWrapper>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // end anonymous namespace

char DTransMemInitTrimDownWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransMemInitTrimDownWrapper, "dtrans-meminittrimdown",
                      "DTrans Mem Init Trim Down", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransMemInitTrimDownWrapper, "dtrans-meminittrimdown",
                    "DTrans Mem Init Trim Down", false, false)

ModulePass *llvm::createDTransMemInitTrimDownWrapperPass() {
  return new DTransMemInitTrimDownWrapper();
}

namespace llvm {

namespace dtrans {

// Member functions of vector class are classified based on functionality.
enum FunctionKind {
  Constructor,       // Constructor
  CopyConstructor,   // Copy-constructor
  Destructor,        // Destructor
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
  ClassInfo(const DataLayout &DL, DTransAnalysisInfo &DTInfo,
            TargetLibraryInfo &TLI, MemInitDominatorTreeType GetDT,
            MemInitCandidateInfo *MICInfo, int32_t FieldIdx)
      : DL(DL), DTInfo(DTInfo), TLI(TLI), GetDT(GetDT), MICInfo(MICInfo),
        FieldIdx(FieldIdx){};

  // Analyze each member function to detect functionality.
  bool analyzeClassFunctions();

private:
  const DataLayout &DL;
  DTransAnalysisInfo &DTInfo;
  TargetLibraryInfo &TLI;
  MemInitDominatorTreeType GetDT;

  // Candidate struct info.
  MemInitCandidateInfo *MICInfo;

  // Field position of potential vector class in candidate struct.
  int32_t FieldIdx;

  // Index of "size" field in vector class.
  int32_t SizeField = -1;

  // Index of "capacity" field in vector class.
  int32_t CapacityField = -1;

  // This is the index where pointer of MemoryInterface is saved.
  int32_t MemIntField = -1;

  // This is the index where pointer of actual array is saved.
  int32_t ArrayField = -1;

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

  void collectElementDataTypes();
  bool checkDominatorInfo(Instruction *, Instruction *);
  FunctionKind categorizeFunctionUsingSignature(Function *, StructType *);
  bool isAccessingFieldOfArgClass(const GetElementPtrInst *, Argument *,
                                  int32_t *);
  bool isAccessingVTableFieldInBaseClass(const GetElementPtrInst *, Argument *);
  bool checkFieldOfArgClassLoad(const Value *, Argument *, int32_t);
  bool checkMemInterfacePointer(Value *, Argument *);
  bool checkAllocSizeOfArray(const Value *, Argument *);
  bool checkAllocCall(Value *, Argument *);
  bool checkVtableLoadOfMemInt(Value *, Argument *);
  bool isIndirectCallCheck(Value *, Argument *);
  bool checkAllocatedArrayPtr(Value *, Argument *);
  bool checkBBControlledUnderCapacityVal(BasicBlock *, Argument *);
  bool checkMemset(SmallPtrSet<Value *, 4>, Argument *);
  bool checkAllInstsProcessed(Function *,
                              SmallPtrSetImpl<const Instruction *> &);
  FunctionKind recognizeConstructor(Function *);
  FunctionKind recognizeDerivedConstructor(Function *, Type *, Type *);
};

// Element of array can be a struct but member functions like SetElem,
// GetElem etc are defined based on individual fields of the struct.
// This case usually occurs when SOAToAOS transformation is applied
// on multiple vector class fields. So, treat all fields of struct
// as data elements of vector class if array element is struct type.
// This routine collects actual data elements of array.
void ClassInfo::collectElementDataTypes() {
  Type *ElemTy = MICInfo->getFieldElemTy(FieldIdx);
  if (auto *STy = dyn_cast<StructType>(ElemTy))
    for (auto *ETy : STy->elements())
      ElemDataTypes.insert(ETy->getPointerTo());
  else
    ElemDataTypes.insert(ElemTy);
}

// Returns true if D dominates U.
bool ClassInfo::checkDominatorInfo(Instruction *D, Instruction *U) {
  DominatorTree &DT = (GetDT)(*D->getParent()->getParent());
  if (DT.dominates(D, U))
    return true;
  return false;
}

// Categorize given function F using return type and signature of the
// function. ClassTy is the type of first argument of F.
//
// CopyConstructor: No return type. Two arguments of same class type.
//
// Constructor: No return type. Only one argument is class type.
//              Pointer of MemInterface should be passed to initialize.
//
// Destructor: No return type. Only one class type argument.
//
// GetElem: Element type is returned. One class type argument and one
//          integer argument to indicate position.
//
// GetSizeOrCapacity: Integer type is returned. Only one class type argument.
//
// SetElem: No return type. Only one class type argument and at least one
//          element type as argument. One integer argument to indicate
//          position.
//
// AddElem: No return type. Only one class type argument and at least one
//          element type as argument. No integer argument.
//
// Resize: No return type. Only one class type argument and one integer
//         argument.
//
FunctionKind ClassInfo::categorizeFunctionUsingSignature(Function *F,
                                                         StructType *ClassTy) {
  bool NoReturn = false;
  bool ElemReturn = false;
  bool IntReturn = false;
  int32_t MemInterfaceArgs = 0;
  int32_t ClassArgs = 0;
  int32_t ElemArgs = 0;
  int32_t IntArgs = 0;
  StructType *MemInterTy = MICInfo->getMemInterfaceType();

  // Analyze return type here.
  Type *RTy = F->getReturnType();
  switch (RTy->getTypeID()) {
  default:
    return UnKnown;

  case Type::VoidTyID:
    NoReturn = true;
    break;

  case Type::PointerTyID:
    if (ElemDataTypes.count(RTy))
      ElemReturn = true;
    else
      return UnKnown;
    break;

  case Type::IntegerTyID:
    IntReturn = true;
    break;
  }

  // Analyze type of each argument.
  for (auto &Arg : F->args()) {
    Type *ArgTy = Arg.getType();
    switch (ArgTy->getTypeID()) {
    default:
      return UnKnown;

    case Type::PointerTyID: {
      auto *PTy = cast<PointerType>(ArgTy);
      if (PTy->getElementType() == MemInterTy)
        MemInterfaceArgs++;
      else if (PTy->getElementType() == ClassTy)
        ClassArgs++;
      else if (ElemDataTypes.count(PTy))
        ElemArgs++;
      else
        return UnKnown;
      break;
    }

    case Type::IntegerTyID:
      IntArgs++;
      break;
    }
  }

  // Categorize function based on return and argument types.
  auto ArgsSize = F->arg_size();
  if (NoReturn && ArgsSize == 2 && ClassArgs == 2)
    return CopyConstructor;
  else if (NoReturn && MemInterfaceArgs == 1 && ClassArgs == 1)
    return Constructor;
  else if (NoReturn && ClassArgs == 1 && ArgsSize == 1)
    return Destructor;
  else if (ElemReturn && ClassArgs == 1 && IntArgs == 1 && ArgsSize == 2)
    return GetElem;
  else if (IntReturn && ClassArgs == 1 && ArgsSize == 1)
    return GetSizeOrCapacity;
  else if (NoReturn && ClassArgs == 1 && ElemArgs >= 1 && IntArgs == 1)
    return SetElem;
  else if (NoReturn && ClassArgs == 1 && ElemArgs >= 1 && IntArgs == 0)
    return AppendElem;
  else if (NoReturn && ClassArgs == 1 && IntArgs == 1 && ArgsSize == 2)
    return Resize;
  return UnKnown;
}

// Helper routine to check if GEP is accessing an element from Obj pointer
// that points to Base class. If GEP is in expected format, Idx is updated
// with the field that is accessed.
// Ex:
// getelementptr %"class.Base", %"class.Base"* %Obj, i64 0, i32 1
//
bool ClassInfo::isAccessingFieldOfArgClass(const GetElementPtrInst *GEP,
                                           Argument *Obj, int32_t *Idx) {
  if (GEP->getOperand(0) != Obj)
    return false;
  assert(isa<StructType>(GEP->getSourceElementType()) && "Expected StructType");
  if (GEP->getNumIndices() != 2)
    return false;
  if (!cast<Constant>(GEP->getOperand(1))->isZeroValue())
    return false;
  *Idx = cast<ConstantInt>(GEP->getOperand(2))->getLimitedValue();
  return true;
}

// Helper routine to check if GEP is accessing element at zero index in
// base class from Obj, which is expected to be a pointer that points to
// derived class, and accessing element is pointer to VTable.
// Ex:
// getelementptr %"class.Ref", %"class.Ref"* %Obj, i64 0, i32 0, i32 0
//
bool ClassInfo::isAccessingVTableFieldInBaseClass(const GetElementPtrInst *GEP,
                                                  Argument *Obj) {
  if (GEP->getOperand(0) != Obj)
    return false;
  assert(isa<StructType>(GEP->getSourceElementType()) && "Expected StructType");
  if (GEP->getNumIndices() != 3)
    return false;
  if (!GEP->hasAllZeroIndices())
    return false;
  auto *GEPTy = GEP->getResultElementType();
  if (!isPtrToVFTable(GEPTy))
    return false;
  return true;
}

// Returns true if V is loading a field at index FI using Obj pointer
// that points to class type.
// Ex:
//       %7 = getelementptr %"C", %"c"* Obj, i64 0, i32 FI
//   V:  %25 = load i32, i32* %7
bool ClassInfo::checkFieldOfArgClassLoad(const Value *V, Argument *Obj,
                                         int32_t FI) {
  auto *LI = dyn_cast<LoadInst>(V);
  if (!LI)
    return false;
  auto *GEP = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
  if (!GEP)
    return false;
  int32_t Idx;
  if (!isAccessingFieldOfArgClass(GEP, Obj, &Idx))
    return false;
  if (Idx != FI)
    return false;
  Visited.insert(LI);
  return true;
}

// Returns true if V is pointer to MemInterface. Two possibilities
//
// 1. MemInterface pointer argument that is stored to MemInterface field.
// 2. Loading value from the MemInterface field.
//
bool ClassInfo::checkMemInterfacePointer(Value *V, Argument *Obj) {
  if (auto *Arg = dyn_cast<Argument>(V))
    if (auto *PTy = dyn_cast<PointerType>(Arg->getType()))
      if (PTy->getElementType() == MICInfo->getMemInterfaceType())
        return true;

  assert(MemIntField != -1 && "Expected valid MemIntField");
  if (checkFieldOfArgClassLoad(V, Obj, MemIntField))
    return true;
  return false;
}

// Returns true if V represents allocation size of array.
//
// 1. V is constant: This can occur when Capacity is constant.
//       V == Capacity * Element_Size
//
// 2. V is non-constant: Try to find if it is computing allocation
//    size of array.
//
//   Ex:
//           %25 = load i32, i32* %7  // Load Capacity field or get
//                                    // Capacity value from argument that
//                                    // is stored in Capacity field.
//           %26 = zext i32 %25 to i64
//           %27 = shl i64 %26, 3
//     V:    %28 = mul i64 %27, 2
//
bool ClassInfo::checkAllocSizeOfArray(const Value *V, Argument *ThisObj) {
  Type *ElemTy = MICInfo->getFieldElemTy(FieldIdx);
  int64_t ElemSize = DL.getTypeAllocSize(ElemTy);
  assert(CapacityInitInst && " Expected valid CapacityInitInst");
  Value *StoredOp = CapacityInitInst->getValueOperand();

  // Case 1: V is constant.
  if (auto *ConstVal = dyn_cast<ConstantInt>(V)) {
    int64_t ArrSize = ConstVal->getLimitedValue();
    auto *StoredCapacityVal = dyn_cast<ConstantInt>(StoredOp);
    if (!StoredCapacityVal)
      return false;
    int64_t StoredCapacityConst = StoredCapacityVal->getLimitedValue();
    if (ElemSize * StoredCapacityConst == ArrSize)
      return true;
    return false;
  }

  // Case 2: V is non-constant
  int64_t Multiplier = 1;
  // Compute multiplier by checking Shl and Mul instructions.
  while (auto *BO = dyn_cast<BinaryOperator>(V)) {
    auto OpC = BO->getOpcode();
    if (OpC == Instruction::Shl) {
      auto *ConstVal = dyn_cast<ConstantInt>(BO->getOperand(1));
      if (!ConstVal)
        return false;
      Multiplier = Multiplier * ((int64_t)1 << ConstVal->getLimitedValue());
      Visited.insert(BO);
    } else if (OpC == Instruction::Mul) {
      auto *ConstVal = dyn_cast<ConstantInt>(BO->getOperand(1));
      if (!ConstVal)
        return false;
      Multiplier = Multiplier * ConstVal->getLimitedValue();
      Visited.insert(BO);
    } else {
      break;
    }
    V = BO->getOperand(0);
  }
  // Skip ZExt instruction.
  if (auto *Zext = dyn_cast<ZExtInst>(V)) {
    Visited.insert(Zext);
    V = Zext->getOperand(0);
  }
  // Check if V represents value of Capacity.
  if (V == StoredOp || checkFieldOfArgClassLoad(V, ThisObj, CapacityField))
    return true;
  return false;
}

// Returns true if BB is controlled under "Capacity != 0" condition.
//
//  Ex:
//      %28 = icmp eq i32 %Capacity, 0
//      br i1 %28, lable %some_bb, label %BB
//
//   BB:
//      ...
//
bool ClassInfo::checkBBControlledUnderCapacityVal(BasicBlock *BB,
                                                  Argument *ThisObj) {
  // Entry BB
  if (BB->hasNPredecessors(0))
    return true;
  BasicBlock *Pred = BB->getSinglePredecessor();
  if (!Pred)
    return false;
  auto *BI = dyn_cast<BranchInst>(Pred->getTerminator());
  if (!BI)
    return false;

  // Returns true if BB is controlled under no immediate conditional branch.
  if (!BI->isConditional())
    return true;

  Value *BrCond = BI->getCondition();
  ICmpInst *IC = dyn_cast<ICmpInst>(BrCond);
  if (!IC)
    return false;
  // Checking only ICmpInst::ICMP_EQ for simple implementation.
  if (IC->getPredicate() != ICmpInst::ICMP_EQ)
    return false;
  Value *RValue = IC->getOperand(1);
  if (!isa<ConstantInt>(RValue) || !cast<ConstantInt>(RValue)->isZeroValue())
    return false;
  Value *LValue = IC->getOperand(0);
  Value *StoredOp = CapacityInitInst->getValueOperand();
  // Check if LValue represents Capacity value.
  if (StoredOp != LValue &&
      !checkFieldOfArgClassLoad(LValue, ThisObj, CapacityField))
    return false;
  // Makes sure BB is the else part.
  if (BI->getSuccessor(1) != BB)
    return false;
  Visited.insert(BI);
  Visited.insert(IC);
  return true;
}

// Returns true if any pointer in ArrayPtrAliases is initialized with
// valid memset.
//
// Ex:
//     %25 = phi i8* [ %21, %20 ], [ %23, %22 ]
//     %27 = bitcast i8* %25 to i16**
//     ...
//     call void @llvm.memset.p0i8.i64(i8* %25, i8 0, i64 ArrSize, i1 false)
//
bool ClassInfo::checkMemset(SmallPtrSet<Value *, 4> ArrayPtrAliases,
                            Argument *ThisObj) {
  int32_t MemsetCalled = 0;
  IntrinsicInst *MemsetI;
  for (auto *APtr : ArrayPtrAliases)
    for (auto *U : APtr->users()) {
      MemSetInst *II = dyn_cast<MemSetInst>(U);
      if (!II)
        continue;
      if (II->getArgOperand(0) != APtr)
        return false;
      Value *Val1 = II->getArgOperand(1);
      if (!isa<ConstantInt>(Val1) || !cast<ConstantInt>(Val1)->isZeroValue())
        return false;
      // Check 3rd argument is size of element array.
      if (checkAllocSizeOfArray(II->getArgOperand(2), ThisObj))
        MemsetCalled++;
      MemsetI = II;
    }

  if (MemsetCalled != 1)
    return false;
  assert(MemsetI && "Expected valid memset instruction");
  // Check if it is controlled under "Capacity != 0" condition.
  if (!checkBBControlledUnderCapacityVal(MemsetI->getParent(), ThisObj))
    return false;
  Visited.insert(MemsetI);
  return true;
}

// Returns true if Val is allocation call that allocates memory
// for element array.
bool ClassInfo::checkAllocCall(Value *Val, Argument *ThisObj) {
  auto *AllocCall = dyn_cast<CallInst>(Val->stripPointerCasts());
  if (!AllocCall)
    return false;
  auto *CallInfo = DTInfo.getCallInfo(AllocCall);
  if (!CallInfo)
    return false;
  if (CallInfo->getCallInfoKind() != dtrans::CallInfo::CIK_Alloc)
    return false;
  auto AKind = cast<AllocCallInfo>(CallInfo)->getAllocKind();
  if (AKind != AK_Malloc && AKind != AK_New && AKind != AK_UserMalloc)
    return false;

  SmallPtrSet<const Value *, 3> Args;
  collectSpecialAllocArgs(AKind, AllocCall, Args, TLI);
  assert(Args.size() == 1 && "Unexpected allocation function");
  const Value *SizeArg = *Args.begin();

  // Check size should be equal to element array size.
  if (!checkAllocSizeOfArray(SizeArg, ThisObj))
    return false;
  Visited.insert(AllocCall);
  return true;
}

// Returns true if Value is Vtable load from MemInterface pointer.
//
// Ex:
//        store %"MemInt"* %3, %"MemInt"** %10 // Store to MemInt field.
//        %13 = bitcast %"MemInt"* %3 to i8* (%"MemInt"*, i64)***
//        %14 = load i8* (%"MemInt"*, i64)**, i8* (%"MemInt"*, i64)*** %13
//        %15 = GEP i8* (%"MemInt"*, i64)*, i8* (%"MemInt"*, i64)** %14, i64 2
// Value: %16 = load i8* (%"MemInt"*, i64)*, i8* (%"MemInt"*, i64)** %15
//
bool ClassInfo::checkVtableLoadOfMemInt(Value *Value, Argument *ThisObj) {
  auto *LI = dyn_cast<LoadInst>(Value);
  if (!LI)
    return false;
  auto *PTy = dyn_cast<PointerType>(LI->getType());
  if (!PTy || !PTy->getElementType()->isFunctionTy())
    return false;
  auto *GEP = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
  if (!GEP)
    return false;
  // Just fixing offset.
  if (GEP->getNumIndices() != 1)
    return false;
  auto *PO = GEP->getPointerOperand();
  auto *VTLoad = dyn_cast<LoadInst>(PO);
  if (!VTLoad)
    return false;
  auto *MemI = VTLoad->getPointerOperand();
  if (auto *BC = dyn_cast<BitCastInst>(MemI)) {
    Visited.insert(BC);
    MemI = BC->getOperand(0);
  }
  if (!checkMemInterfacePointer(MemI, ThisObj))
    return false;
  Visited.insert(LI);
  Visited.insert(GEP);
  Visited.insert(VTLoad);
  return true;
}

// Returns true if BrCond is an indirect call convert check.
//
// Ex:
//   %17 = bitcast i8* (%"MemInt"*, i64)* %16 to i8* // Prove %16 is load
//                                                   // from Vtable of MemInt
//   %18 = bitcast i8* (%"MemInt"*, i64)* @_foo to i8*
//   %19 = icmp eq i8* %17, %18
//   br i1 %19, label %20, label %22
//
bool ClassInfo::isIndirectCallCheck(Value *BrCond, Argument *ThisObj) {
  ICmpInst *IC = dyn_cast<ICmpInst>(BrCond);
  if (!IC)
    return false;
  if (IC->getPredicate() != ICmpInst::ICMP_EQ)
    return false;
  Value *RValue = IC->getOperand(1);
  Value *LValue = IC->getOperand(0);
  if (auto *BC = dyn_cast<BitCastInst>(RValue)) {
    Visited.insert(BC);
    RValue = BC->getOperand(0);
  }
  if (auto *BC = dyn_cast<BitCastInst>(LValue)) {
    Visited.insert(BC);
    LValue = BC->getOperand(0);
  }
  if (!isa<Function>(RValue->stripPointerCasts()))
    return false;
  if (!checkVtableLoadOfMemInt(LValue, ThisObj))
    return false;
  Visited.insert(IC);
  return true;
}

// Returns true if ValOp is return value of allocation call.
//
// Ex:
//    br i1 %19, label %20, label %22
//
//  20:
//    %21 = tail call alloc1(Element_array_Size);
//    br label %24
//
//  22:
//    %23 = tail call alloc2(Element_array_Size);
//    br label %24
//
//  24:                                               ; preds = %22, %20
//    %25 = phi i8* [ %21, %20 ], [ %23, %22 ]
//     br label %26
//
//   26:                                               ; preds = %24
// ValOp: %27 = bitcast i8* %25 to i16**
//
//
bool ClassInfo::checkAllocatedArrayPtr(Value *ValOp, Argument *ThisObj) {

  // Collection of allocated pointer aliases.
  SmallPtrSet<Value *, 4> ArrayPtrAliases;
  ArrayPtrAliases.insert(ValOp);
  if (auto *BC = dyn_cast<BitCastInst>(ValOp)) {
    Visited.insert(BC);
    ValOp = BC->getOperand(0);
    ArrayPtrAliases.insert(ValOp);
  }
  // Just return true if it is alloc call.
  if (checkAllocCall(ValOp, ThisObj) && checkMemset(ArrayPtrAliases, ThisObj))
    return true;
  auto *Phi = dyn_cast<PHINode>(ValOp);
  // If it is PHI node, check all inputs are coming from allocation
  // calls. For now, it supports only diamond CFG, which is created
  // due to indirect call conv.
  if (!Phi || Phi->getNumIncomingValues() != 2)
    return false;
  BasicBlock *Pred0 = Phi->getIncomingBlock(0);
  BasicBlock *Pred1 = Phi->getIncomingBlock(1);
  BasicBlock *Pred0Pred = Pred0->getSinglePredecessor();
  BasicBlock *Pred1Pred = Pred1->getSinglePredecessor();
  BasicBlock *Pred0Succ = Pred0->getSingleSuccessor();
  BasicBlock *Pred1Succ = Pred1->getSingleSuccessor();
  if (!Pred0Pred || !Pred1Pred || Pred0Pred != Pred1Pred)
    return false;
  if (!Pred0Succ || !Pred1Succ || Pred0Succ != Pred1Succ)
    return false;
  auto *BI = dyn_cast<BranchInst>(Pred0Pred->getTerminator());
  if (!BI || !BI->isConditional())
    return false;
  Value *BrCond = BI->getCondition();
  if (!isIndirectCallCheck(BrCond, ThisObj))
    return false;
  // Check all inputs of Phi are valid allocated pointers.
  int32_t AllocFound = false;
  for (auto I = 0; I < 2; I++) {
    Value *Val = Phi->getIncomingValue(I);
    auto *AllocCall = dyn_cast<CallInst>(Val->stripPointerCasts());
    if (!AllocCall)
      return false;
    // Ignore if it is a dummy allocated call.
    if (isDummyFuncWithThisAndIntArgs(AllocCall, TLI)) {
      Visited.insert(AllocCall);
      continue;
    }
    if (!checkAllocCall(Val, ThisObj))
      return false;
    AllocFound = true;
  }
  if (!AllocFound)
    return false;
  // Makes sure allocated array is initialized with memset.
  if (!checkMemset(ArrayPtrAliases, ThisObj))
    return false;

  // Mark all instructions as processed.
  auto *BI0 = dyn_cast<BranchInst>(Pred0->getTerminator());
  if (BI0)
    Visited.insert(BI0);
  auto *BI1 = dyn_cast<BranchInst>(Pred1->getTerminator());
  if (BI1)
    Visited.insert(BI1);
  Visited.insert(Phi);
  Visited.insert(BI);
  return true;
}

// Return false if any instruction of F is not found in Processed.
//
bool ClassInfo::checkAllInstsProcessed(
    Function *F, SmallPtrSetImpl<const Instruction *> &Processed) {
  for (auto &I : instructions(F)) {
    if (Processed.count(&I))
      continue;
    // Ignore ReturnInst with no operands.
    if (isa<ReturnInst>(&I) && I.getNumOperands() == 0)
      continue;

    // TODO: For now, ignore unconditional BranchInst. Need to find
    // a better way to handle them.
    if (isa<BranchInst>(&I) && cast<BranchInst>(&I)->isUnconditional())
      continue;

    // Return false if any instruction is not found in Processed list.
    return false;
  }
  return true;
}

// Recognize construction of derived class, which has only base class
// object as one field. This constructor calls constructor of base class
// to initialize fields of the base class object.
// Ex:
//  define void Derived(%"D"*, i32, i1, %"MemInt"*) {
//    %5 = getelementptr %"C", %"C"* %0, i64 0, i32 0
//    call Base(%"B"* %5, i32 %1, i1 true, %"MemInt"* %3)
//    %6 = GEP %"D", %"D"* %0, i64 0, i32 0, i32 0
//    store Vtable, i32 (...)*** %6
//    ret void
//  }
//
FunctionKind ClassInfo::recognizeDerivedConstructor(Function *F, Type *DTy,
                                                    Type *BTy) {
  unsigned ConstructorCall = 0;
  SmallVector<GetElementPtrInst *, 2> GEPList;
  Argument *ThisObj = &*F->arg_begin();
  SmallPtrSet<const Instruction *, 8> ProcessedInsts;

  // Collect all uses of This pointer. Expected only GEP as user.
  for (auto *U : ThisObj->users())
    if (auto *GEPI = dyn_cast<GetElementPtrInst>(U))
      GEPList.push_back(GEPI);
    else
      return UnKnown;

  // Analyze how these GEP instructions are used.
  // Case 1: Get address of base object and pass it as first argument
  //         to the base class constructor.
  //
  // Case 2: Store Vtable into 1st field of Base class object.
  //
  for (auto *GEP : GEPList) {
    int32_t Idx;

    ProcessedInsts.insert(GEP);
    auto *GEPTy = GEP->getResultElementType();
    if (isAccessingFieldOfArgClass(GEP, ThisObj, &Idx)) {
      // Case 1: Calling Base constructor call
      if (GEPTy != BTy || Idx != 0)
        return UnKnown;
      if (!GEP->hasOneUse())
        return UnKnown;
      auto *CI = dyn_cast<CallInst>(*GEP->user_begin());
      if (!CI)
        return UnKnown;

      unsigned Count = 0;
      for (const Value *AVal : CI->args()) {
        if (Count == 0) {
          // Makes sure base object is passed as 1st argument.
          if (AVal != GEP)
            return UnKnown;
          Count++;
          continue;
        }
        Count++;
        // Makes sure args of base call constructors are either argument or
        // constant.
        if (!isa<Argument>(AVal) && !isa<ConstantInt>(AVal))
          return UnKnown;
      }
      auto *Callee = CI->getCalledFunction();
      if (!Callee)
        return UnKnown;
      // Now, recognize base class constructor.
      FunctionKind FKind = recognizeConstructor(Callee);
      if (FKind != Constructor)
        return UnKnown;
      ConstructorCall++;
      ProcessedInsts.insert(CI);
    } else if (isAccessingVTableFieldInBaseClass(GEP, ThisObj)) {
      // Case 2: Storing VTable
      if (!GEP->hasOneUse())
        return UnKnown;
      auto *SI = dyn_cast<StoreInst>(*GEP->user_begin());
      if (!SI)
        return UnKnown;
      // Not checking what value is stored since it will not be used anywhere.
      if (SI->getPointerOperand() != GEP)
        return UnKnown;
      ProcessedInsts.insert(SI);
    } else {
      return UnKnown;
    }
  }
  // Check all instructions are processed. Make Vtable store
  // as optional.
  if (ConstructorCall != 1 || !checkAllInstsProcessed(F, ProcessedInsts))
    return UnKnown;
  return Constructor;
}

// Analyze constructor and find fields that represent Capacity, Size,
// Array, MemInterface etc based on type of field and value that is
// assigned.
//  Size field (i32): Zero is assigned.
//  Capacity field (i32): Non-Zero is assigned.
//  Flag field (i8)
//  MemInt field (MemInterface type):
//  Vtable field (Pointer to Vtable)
//  Array field (Pointer to Element field)
//
// Step 1: Collect all uses of This pointer.
// Step 2: First, process all GEPs that belong to non-array-pointer fields.
//         Detect Size, Capacity fields. Most uses of these GEPs are simple
//         store instructions.
// Step 3: Using the information that collected in Step 2, analyze the use
//         of GEP that belong to array pointer. This covers all related
//         things like memory allocation, memset etc.
//
// Ex:
//   Base(int Capacity, MemInt* mem, bool  Flag) :
//       Flag(Flag), Size(0), Capacity(Capacity), ArrayPtr(0), MemInt(mem)
//   {
//      ArrayPtr = (Elem*) allocate(Capacity * sizeof(Elem));
//      memset(ArrayPtr, 0, Capacity * sizeof(Elem));
//   }
//
FunctionKind ClassInfo::recognizeConstructor(Function *F) {

  // Process all GEPs that belong to non-array-pointer fields. GEPList
  // represents all uses of This pointer.
  auto AnalyzeNonArrayPtrFields =
      [this](SmallVector<GetElementPtrInst *, 8> &GEPList, Argument *ThisObj) {
        unsigned MemIntAssign = 0;
        unsigned FlagFieldAssign = 0;
        unsigned SizeFieldAssign = 0;
        unsigned CapacityFieldAssign = 0;

        for (auto *GEP : GEPList) {
          int32_t Idx;
          auto *GEPTy = GEP->getResultElementType();
          if (!isAccessingFieldOfArgClass(GEP, ThisObj, &Idx))
            return false;

          // Ignore array pointer field.
          if (GEPTy->isPointerTy() &&
              cast<PointerType>(GEPTy)->getElementType() ==
                  MICInfo->getFieldElemTy(FieldIdx))
            continue;

          Visited.insert(GEP);
          for (auto *U : GEP->users()) {
            // We are interested only store instructions during this step.
            auto *SI = dyn_cast<StoreInst>(U);
            if (!SI)
              continue;

            if (SI->getPointerOperand() != GEP)
              return false;

            Value *ValOp = SI->getValueOperand();
            if (GEPTy->isIntegerTy(8)) {
              // Flag is assigned here.
              FlagFieldAssign++;
            } else if (GEPTy->isIntegerTy(32)) {
              if (!isa<ConstantInt>(ValOp) ||
                  !cast<ConstantInt>(ValOp)->isZero()) {
                // Non-zero assignment is considered as Capacity.
                CapacityFieldAssign++;
                CapacityField = Idx;
                CapacityInitInst = SI;
              } else {
                // Zero assignment is considered as Size.
                SizeFieldAssign++;
                SizeField = Idx;
              }
            } else if (GEPTy->isPointerTy()) {
              auto *PTy = cast<PointerType>(GEPTy);
              auto *PtrElemTy = PTy->getElementType();
              if (PtrElemTy == MICInfo->getMemInterfaceType()) {
                // MemInt field should be assigned with argument.
                if (!isa<Argument>(ValOp))
                  continue;
                MemIntAssign++;
                MemIntField = Idx;
              } else if (isPtrToVFTable(GEPTy)) {
                // If it has Vtable, it should be at index zero.
                if (Idx != 0)
                  return false;
              } else {
                // No other pointer types allowed.
                return false;
              }
            } else {
              // No other types allowed.
              return false;
            }
            Visited.insert(SI);
          }
        }

        // Expected all non-array fields are assigned. VTable field is optional.
        if (MemIntAssign != 1 || FlagFieldAssign != 1 ||
            CapacityFieldAssign != 1 || SizeFieldAssign != 1)
          return false;

        return true;
      };

  // Analyze GEP related to array pointer field. This also verifies that
  // memory is allocated and initialized properly for array pointer field.
  // This allows two assignments to array pointer field. One is simple
  // nullptr assignment and other is actual allocated memory allocation
  // assignment.
  auto AnalyzeArrayPtrFields =
      [this](SmallVector<GetElementPtrInst *, 8> &GEPList, Argument *ThisObj) {
        StoreInst *NullPtrStore = nullptr;
        StoreInst *AllocPtrStore = nullptr;

        for (auto *GEP : GEPList) {
          int32_t Idx;
          auto *GEPTy = GEP->getResultElementType();

          // Skip non-array pointer fields here.
          if (!isa<PointerType>(GEPTy))
            continue;
          auto *PTy = cast<PointerType>(GEPTy);
          auto *PtrElemTy = PTy->getElementType();
          if (PtrElemTy == MICInfo->getMemInterfaceType() ||
              isPtrToVFTable(GEPTy))
            continue;

          if (!isAccessingFieldOfArgClass(GEP, ThisObj, &Idx))
            return false;
          Visited.insert(GEP);
          for (auto *U : GEP->users()) {
            // Something wrong if it is not array pointer field.
            if (PtrElemTy != MICInfo->getFieldElemTy(FieldIdx))
              return false;
            if (!isa<StoreInst>(U))
              continue;
            auto *SI = cast<StoreInst>(U);
            Value *ValOp = SI->getValueOperand();
            if (SI->getPointerOperand() != GEP)
              return false;
            if (isa<Constant>(ValOp) && cast<Constant>(ValOp)->isNullValue()) {
              // nullptr assignment to array pointer field.
              if (NullPtrStore)
                return false;
              NullPtrStore = SI;
              ArrayField = Idx;
            } else if (checkAllocatedArrayPtr(ValOp, ThisObj)) {
              // Memory allocation assignment to array pointer field.
              if (AllocPtrStore)
                return false;
              AllocPtrStore = SI;
            } else {
              return false;
            }
          }
        }
        // Making NullPtrStore as optional.
        if (!AllocPtrStore)
          return false;
        // Makes sure AllocPtrStore is executed later.
        if (NullPtrStore && !checkDominatorInfo(NullPtrStore, AllocPtrStore))
          return false;
        Visited.insert(NullPtrStore);
        Visited.insert(AllocPtrStore);
        return true;
      };

  // Clear all processed instructions first.
  Visited.clear();
  Argument *ThisObj = &*F->arg_begin();
  SmallVector<GetElementPtrInst *, 8> GEPList;

  // Collect all uses of This pointer. Expected all uses of This pointer
  // are in GEPs.
  for (auto *U : ThisObj->users())
    if (auto *GEPI = dyn_cast<GetElementPtrInst>(U))
      GEPList.push_back(GEPI);
    else
      return UnKnown;

  // Analyze non-array pointer fields.
  if (!AnalyzeNonArrayPtrFields(GEPList, ThisObj)) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                    { dbgs() << "  Failed: Non-pointer field analysis.\n"; });
    return UnKnown;
  }

  // Analyze array pointer field.
  if (!AnalyzeArrayPtrFields(GEPList, ThisObj)) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                    { dbgs() << "  Failed: Pointer field analysis.\n"; });
    return UnKnown;
  }

  // Makes sure all instructions are processed.
  if (!checkAllInstsProcessed(F, Visited)) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
      dbgs() << " Failed: Missing to process some instructions.\n";
    });
    return UnKnown;
  }
  return Constructor;
}

// Analyze all member functions of class.
//  Step 1: Categorize functions using signature
//  Step 2: Analyze constructors and detect Size, Capacity etc fields.
//  Step 3: Analyze remaining member function to prove the functionality.
//
bool ClassInfo::analyzeClassFunctions() {
  DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                  { dbgs() << "  Categorize functions using signature: \n"; });

  DenseMap<Function *, FunctionKind> InitialFuncKind;
  SmallPtrSet<Function *, 2> ConstructorSet;

  collectElementDataTypes();
  for (auto *Fn : MICInfo->member_functions(FieldIdx)) {
    auto *ClassTy = getClassType(Fn);
    FunctionKind FKind = categorizeFunctionUsingSignature(Fn, ClassTy);
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
      dbgs() << "       " << Fn->getName() << ":   " << FKind << "\n";
    });
    if (FKind == UnKnown) {
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                      { dbgs() << "  Failed: Unknown signature found.\n"; });
      return false;
    }
    if (FKind == Constructor && MICInfo->isCandidateFieldDerivedTy(ClassTy))
      ConstructorSet.insert(Fn);
    InitialFuncKind[Fn] = FKind;
  }
  if (ConstructorSet.size() != 1) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
      dbgs() << "  Failed: Unexpected number of constructors\n";
    });
    return false;
  }
  auto *Fn = *ConstructorSet.begin();
  auto *ClassTy = getClassType(Fn);
  Type *BaseClassTy = getMemInitSimpleBaseType(ClassTy);
  DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
    dbgs() << "  Analyzing Constructor " << Fn->getName() << "\n";
  });
  FunctionKind FKind;
  if (BaseClassTy)
    FKind = recognizeDerivedConstructor(Fn, ClassTy, BaseClassTy);
  else
    FKind = recognizeConstructor(Fn);
  if (FKind == UnKnown) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                    { dbgs() << "  Failed: Constructor not recognized\n"; });
    return false;
  }
  DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
    dbgs() << "  Passed: Constructor recognized\n";
    dbgs() << "    Capacity field: " << CapacityField << "\n";
    dbgs() << "    Size field: " << SizeField << "\n";
  });

  // TODO: More analysis needed for remaining member functions.
  return true;
}

class MemInitTrimDownImpl {

public:
  MemInitTrimDownImpl(Module &M, const DataLayout &DL,
                      DTransAnalysisInfo &DTInfo, TargetLibraryInfo &TLI,
                      MemInitDominatorTreeType GetDT)
      : M(M), DL(DL), DTInfo(DTInfo), TLI(TLI), GetDT(GetDT){};

  ~MemInitTrimDownImpl() {
    for (auto *CInfo : Candidates)
      delete CInfo;
    for (auto *ClassInfo : ClassInfoSet)
      delete ClassInfo;
  }
  bool run(void);

private:
  Module &M;
  const DataLayout &DL;
  DTransAnalysisInfo &DTInfo;
  TargetLibraryInfo &TLI;
  MemInitDominatorTreeType GetDT;

  constexpr static int MaxNumCandidates = 1;
  SmallVector<MemInitCandidateInfo *, MaxNumCandidates> Candidates;

  // Collection of ClassInfo. It will used for more analysis and
  // transformation.
  SmallPtrSet<ClassInfo *, 4> ClassInfoSet;

  bool gatherCandidateInfo(void);
  bool analyzeCandidate(MemInitCandidateInfo *);
};

// Analyze functionality of each member function of candidate
// field classes to prove that the classes are vector classes.
bool MemInitTrimDownImpl::analyzeCandidate(MemInitCandidateInfo *Cand) {
  for (auto Loc : Cand->candidate_fields()) {
    std::unique_ptr<ClassInfo> ClassI(
        new ClassInfo(DL, DTInfo, TLI, GetDT, Cand, Loc));
    if (!ClassI->analyzeClassFunctions())
      return false;
    ClassInfoSet.insert(ClassI.release());
  }
  return true;
}

bool MemInitTrimDownImpl::gatherCandidateInfo(void) {

  DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
    dbgs() << "MemInitTrimDown transformation:";
    dbgs() << "\n";
  });
  // TODO: Consider only SOAToAOS candidates for MemInitTrimDown.
  for (TypeInfo *TI : DTInfo.type_info_entries()) {
    std::unique_ptr<MemInitCandidateInfo> CInfo(new MemInitCandidateInfo());

    auto *StInfo = dyn_cast<StructInfo>(TI);
    if (!StInfo)
      continue;

    if (!CInfo->isCandidateType(TI->getLLVMType()))
      continue;

    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
      dbgs() << "  Considering candidate: ";
      TI->getLLVMType()->print(dbgs(), true, true);
      dbgs() << "\n";
    });

    if (DTInfo.testSafetyData(StInfo, dtrans::DT_MemInitTrimDown)) {
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
        dbgs() << "  Failed: safety test for candidate struct.\n";
      });
      continue;
    }

    // TODO: Check SafetyData for candidate field array structs also.

    if (!CInfo->collectMemberFunctions(M)) {
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                      { dbgs() << "  Failed: member function collection.\n"; });
      continue;
    }

    if (Candidates.size() >= MaxNumCandidates) {
      DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
        dbgs() << "  Failed: Exceeding maximum candidate limit.\n";
      });
      return false;
    }
    Candidates.push_back(CInfo.release());
  }
  if (Candidates.empty()) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN,
                    { dbgs() << "  Failed: No candidates found.\n"; });
    return false;
  }
  DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
    dbgs() << "  Possible candidate structs: \n";
    for (auto *CInfo : Candidates)
      CInfo->printCandidateInfo();
  });
  return true;
}

bool MemInitTrimDownImpl::run(void) {

  if (!gatherCandidateInfo())
    return false;

  // Analyze member functions of candidate classes to prove
  // that functionality match with usual vector class.
  SmallVector<MemInitCandidateInfo *, MaxNumCandidates> ValidCandidates;
  for (auto *Cand : Candidates)
    if (analyzeCandidate(Cand))
      ValidCandidates.push_back(Cand);
  std::swap(Candidates, ValidCandidates);

  if (Candidates.empty()) {
    DEBUG_WITH_TYPE(DTRANS_MEMINITTRIMDOWN, {
      dbgs() << "  Failed: No candidates after functionality analysis.\n";
    });
    return false;
  }
  // TODO: More analysis and transformation code needs to be added here.
  return false;
}

bool MemInitTrimDownPass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                                  TargetLibraryInfo &TLI,
                                  WholeProgramInfo &WPInfo,
                                  dtrans::MemInitDominatorTreeType &GetDT) {

  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2))
    return false;

  if (!DTInfo.useDTransAnalysis())
    return false;

  auto &DL = M.getDataLayout();

  MemInitTrimDownImpl MemInitTrimDownI(M, DL, DTInfo, TLI, GetDT);
  return MemInitTrimDownI.run();
}

PreservedAnalyses MemInitTrimDownPass::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  MemInitDominatorTreeType GetDT = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };

  if (!runImpl(M, DTransInfo, AM.getResult<TargetLibraryAnalysis>(M), WPInfo,
               GetDT))
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<DTransAnalysis>();
  return PA;
}

} // namespace dtrans

} // namespace llvm
