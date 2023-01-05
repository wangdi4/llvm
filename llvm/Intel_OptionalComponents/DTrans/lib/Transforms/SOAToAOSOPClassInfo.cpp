//===--------- SOAToAOSOPClassInfo.cpp - SOAToAOSOP Class Info Analysis ---===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
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

#include "SOAToAOSOPClassInfo.h"
#include "Intel_DTrans/Analysis/DTransAllocCollector.h"
#include "Intel_DTrans/Analysis/DTransAnnotator.h"
#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/DTransCommon.h"

#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/PatternMatch.h"

using namespace llvm;
using namespace PatternMatch;
using dtrans::DTransAnnotator;

#define DTRANS_SOATOAOSOPCLASSINFO "dtrans-soatoaosopclassinfo"

namespace llvm {

namespace dtransOP {

DTransType *ClassInfo::getDTransType(const Value *V) {
  auto &PTA = DTInfo.getPtrTypeAnalyzer();
  auto *Info = PTA.getValueTypeInfo(V);
  if (!Info)
    return nullptr;
  DTransType *DTy = PTA.getDominantType(*Info, ValueTypeInfo::VAT_Use);
  return DTy;
}

DTransType *
ClassInfo::getResultElementDTransType(const GetElementPtrInst *GEP) {
  auto *Ty = getDTransType(GEP);
  if (!Ty || !Ty->isPointerTy())
    return nullptr;
  return Ty->getPointerElementType();
}

// Element of array can be a struct but member functions like SetElem,
// GetElem etc are defined based on individual fields of the struct.
// This case usually occurs when SOAToAOS transformation is applied
// on multiple vector class fields. So, treat all fields of struct
// as data elements of vector class if array element is struct type.
// This routine collects types of data elements and pointer to data
// elements of array.
void ClassInfo::collectElementDataTypes() {
  DTransType *ElemTy = getElementTy();
  if (auto *STy = dyn_cast<DTransStructType>(ElemTy)) {
    // TODO: For now, not supporting all possible types when element is
    // struct. Not interested any more to support struct as element since
    // MemInitTrimDown is running before SOAToAOS.
    for (unsigned I = 0, E = STy->getNumFields(); I != E; ++I) {
      DTransType *ETy = STy->getFieldType(I);
      assert(ETy && "DTransType was not initialized properly");
      ElemDataTypes.insert(
          (DTInfo.getTypeManager().getOrCreatePointerType(ETy)));
    }
  } else {
    ElemDataTypes.insert(ElemTy);
    ElemDataAddrTypes.insert(
        DTInfo.getTypeManager().getOrCreatePointerType(ElemTy));
  }
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
// Ctor/Dtor/CCtor have return types on windows.
//
// CopyConstructor: No return type or ClassTy type. Two arguments
//                  of same class type.
//
// Constructor: No return type  or ClassTy type. Only one argument is class
//              type. Pointer of MemInterface should be passed to initialize.
//
// Destructor: No return type or pointer type. Only one class type argument.
//
// GetElem: Element type or pointer to element type is returned. One class type
// argument and one integer argument to indicate position.
//
// GetSizeOrCapacity: Integer type is returned. Only one class type argument.
//
// SetElem: No return type. Only one class type argument and only one
//          element type or pointer to element type as argument. One integer
//          argument to indicate position.
//
// AddElem: No return type. Only one class type argument and only one
//          element type or pointer to element type as argument. No integer
//          argument.
//
// Resize: No return type. Only one class type argument and one integer
//         argument.
//
FunctionKind
ClassInfo::categorizeFunctionUsingSignature(Function *F,
                                            DTransStructType *ClassTy) {
  bool NoReturn = false;
  bool ThisReturn = false;
  bool PtrReturn = false;
  bool ElemReturn = false;
  bool IntReturn = false;
  int32_t MemInterfaceArgs = 0;
  int32_t ClassArgs = 0;
  int32_t ElemArgs = 0;
  int32_t IntArgs = 0;
  DTransStructType *MemInterTy = SOACInfo->getMemInterfaceType();

  DTransFunctionType *DFnTy = dyn_cast_or_null<DTransFunctionType>(
      DTInfo.getTypeMetadataReader().getDTransTypeFromMD(F));
  if (!DFnTy)
    return UnKnown;

  // Analyze return type here.
  DTransType *RTy = DFnTy->getReturnType();

  if (isa<DTransPointerType>(RTy)) {
    if (ElemDataTypes.count(RTy) || ElemDataAddrTypes.count(RTy))
      ElemReturn = true;
    else if (cast<DTransPointerType>(RTy)->getPointerElementType() == ClassTy)
      ThisReturn = true;
    else
      PtrReturn = true;
  } else if (RTy->isIntegerTy()) {
    IntReturn = true;
  } else if (RTy->getLLVMType()->isVoidTy()) {
    NoReturn = true;
  } else {
    return UnKnown;
  }

  // Analyze type of each argument.
  for (unsigned I = 0, E = DFnTy->getNumArgs(); I != E; ++I) {
    auto *ArgTy = DFnTy->getArgType(I);
    if (auto *PTy = dyn_cast<DTransPointerType>(ArgTy)) {
      if (PTy->getPointerElementType() == MemInterTy)
        MemInterfaceArgs++;
      else if (PTy->getPointerElementType() == ClassTy)
        ClassArgs++;
      else if (ElemDataTypes.count(PTy) || ElemDataAddrTypes.count(PTy))
        ElemArgs++;
    } else if (ArgTy->isIntegerTy()) {
      IntArgs++;
    } else {
      return UnKnown;
    }
  }

  // Categorize function based on return and argument types.
  auto ArgsSize = F->arg_size();
  if ((ThisReturn || NoReturn) && ArgsSize == 2 && ClassArgs == 2)
    return CopyConstructor;
  else if ((ThisReturn || NoReturn) && MemInterfaceArgs == 1 && ClassArgs == 1)
    return Constructor;
  else if ((NoReturn && ClassArgs == 1 && ArgsSize == 1) ||
           (PtrReturn && ClassArgs == 1 && ArgsSize == 2 && IntArgs == 1))
    return Destructor;
  else if (ElemReturn && ClassArgs == 1 && IntArgs == 1 && ArgsSize == 2)
    return GetElem;
  else if (IntReturn && ClassArgs == 1 && ArgsSize == 1)
    return GetSizeOrCapacity;
  else if (NoReturn && ClassArgs == 1 && ElemArgs == 1 && IntArgs == 1 &&
           ArgsSize == 3)
    return SetElem;
  else if (NoReturn && ClassArgs == 1 && ElemArgs == 1 && IntArgs == 0 &&
           ArgsSize == 2)
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
// or
//
// %b = getelementptr %"class.Base", %"class.Base"* %Obj, i64 0, i32 1
// %d = getelementptr %"class.D", %"class.D"* %b, i64 0, i32 0
//
bool ClassInfo::isAccessingFieldOfArgClass(const GetElementPtrInst *GEP,
                                           Value *Obj, int32_t *Idx) {
  // Return true if GEP is computing address of base object.
  auto IsGEPBaseObjAddr = [this](GetElementPtrInst *GEP) {
    auto *GEPTy = dyn_cast<StructType>(GEP->getSourceElementType());
    if (!GEPTy)
      return false;
    auto *GEPDTy = DTInfo.getTypeManager().getStructType(GEPTy->getName());
    if (!GEPDTy)
      return false;
    if (!SOACInfo->isCandidateFieldDerivedTy(GEPDTy) ||
        GEP->getNumIndices() != 2 || !GEP->hasAllZeroIndices())
      return false;
    return true;
  };
  Value *GEPOp = GEP->getOperand(0);

  auto *G = dyn_cast<GetElementPtrInst>(GEPOp);
  // Get Base object if it is computing address of base object.
  if (G && IsGEPBaseObjAddr(G))
    GEPOp = G->getOperand(0);
  if (GEPOp != Obj)
    return false;
  assert(isa<StructType>(GEP->getSourceElementType()) && "Expected StructType");
  if (GEP->getNumIndices() != 2 || !GEP->hasAllConstantIndices())
    return false;
  if (!cast<Constant>(GEP->getOperand(1))->isZeroValue())
    return false;
  *Idx = cast<ConstantInt>(GEP->getOperand(2))->getLimitedValue();
  if (G)
    Visited.insert(G);
  Visited.insert(GEP);
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
  auto *GEPTy = getResultElementDTransType(GEP);
  if (!isPtrToVFTable(GEPTy))
    return false;
  Visited.insert(GEP);
  return true;
}

// Returns true if V is loading a field at index FI using Obj pointer
// that points to class type.
// Ex:
//       %7 = getelementptr %"C", %"C"* Obj, i64 0, i32 FI
//   V:  %25 = load i32, i32* %7
//
//      or
//
//   V:   %47 = phi i8* [ %41, %39 ], [ %45, %61 ]
//   where %41 and %45 are both loading same field at FI.
//
bool ClassInfo::checkFieldOfArgClassLoad(const Value *V, Value *Obj,
                                         int32_t FI) {
  auto IsFieldOfArgClassLoad = [this](const Value *V, Value *Obj, int32_t FI) {
    auto *Zext = dyn_cast<ZExtInst>(V);
    if (Zext)
      V = Zext->getOperand(0);
    auto *LI = dyn_cast<LoadInst>(V);
    if (!LI)
      return false;
    const Value *LoadAddr = LI->getPointerOperand();
    auto *BC = dyn_cast<BitCastInst>(LoadAddr);
    if (BC)
      LoadAddr = BC->getOperand(0);
    auto *GEP = dyn_cast<GetElementPtrInst>(LoadAddr);
    if (!GEP)
      return false;
    int32_t Idx;
    if (!isAccessingFieldOfArgClass(GEP, Obj, &Idx))
      return false;
    if (Idx != FI)
      return false;
    Visited.insert(LI);
    Visited.insert(GEP);
    if (BC)
      Visited.insert(BC);
    if (Zext)
      Visited.insert(Zext);
    return true;
  };

  V = skipCasts(V);
  auto *PN = dyn_cast<PHINode>(V);
  if (!PN)
    return IsFieldOfArgClassLoad(V, Obj, FI);

  if (PN->getNumIncomingValues() != 2)
    return false;
  for (auto I = 0; I < 2; I++) {
    const Value *Val = skipCasts(PN->getIncomingValue(I));
    if (!IsFieldOfArgClassLoad(Val, Obj, FI))
      return false;
  }
  Visited.insert(PN);
  return true;
}

// Returns true if V is pointer to MemInterface. Two possibilities
//
// 1. MemInterface pointer argument that is stored to MemInterface field.
// 2. Loading value from the MemInterface field.
//
bool ClassInfo::checkMemInterfacePointer(Value *V, Argument *Obj) {
  if (auto *BC = dyn_cast<BitCastInst>(V)) {
    Visited.insert(BC);
    V = BC->getOperand(0);
  }
  // Ignore normalized GEPs.
  if (auto *GEP = dyn_cast<GetElementPtrInst>(V)) {
    if (GEP->getNumIndices() != 2 || !GEP->hasAllZeroIndices())
      return false;
    Visited.insert(GEP);
    V = GEP->getPointerOperand();
  }
  if (auto *Arg = dyn_cast<Argument>(V)) {
    DTransType *DTy = getDTransType(V);
    if (auto *PTy = dyn_cast_or_null<DTransPointerType>(DTy))
      if (PTy->getPointerElementType() == SOACInfo->getMemInterfaceType())
        return true;
  }

  assert(MemIntField != -1 && "Expected valid MemIntField");
  if (checkFieldOfArgClassLoad(V, Obj, MemIntField))
    return true;
  return false;
}

// Try to compute multiplier by processing Mul and Shl operators.
// MulPtr is updated with computed value. Returns the first instruction
// that is not Mul/Shl/Zext instruction. Returns nullptr if it is unable
// to process Shl/Mul instructions.
//
//           %25 = load i32, i32* %7
//           %26 = zext i32 %25 to i64
//           %27 = shl i64 %26, 3
//     V:    %28 = mul i64 %27, 2
const Value *ClassInfo::computeMultiplier(const Value *V, int64_t *MulPtr) {
  *MulPtr = 1;
  // Compute multiplier by checking Shl and Mul instructions.
  while (auto *BO = dyn_cast<BinaryOperator>(V)) {
    auto OpC = BO->getOpcode();
    if (OpC == Instruction::Shl) {
      auto *ConstVal = dyn_cast<ConstantInt>(BO->getOperand(1));
      if (!ConstVal)
        return nullptr;
      uint64_t ShiftAmount = ConstVal->getLimitedValue();
      if (ConstVal->isNegative() || ShiftAmount >= 64)
        return nullptr;
      *MulPtr = *MulPtr * ((int64_t)1 << ShiftAmount);
      Visited.insert(BO);
    } else if (OpC == Instruction::Mul) {
      auto *ConstVal = dyn_cast<ConstantInt>(BO->getOperand(1));
      if (!ConstVal)
        return nullptr;
      *MulPtr = *MulPtr * ConstVal->getLimitedValue();
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
  return V;
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
// NumOfElems: %25 = load i32, i32* %7  // Load Capacity field or get
//                                    // Capacity value from argument that
//                                    // is stored in Capacity field.
//           %26 = zext i32 %25 to i64
//           %27 = shl i64 %26, 3
//     V:    %28 = mul i64 %27, 2
//
// If NumOfElems is not nullptr, it checks that *NumOfElems is the one
// to be multiplied.
//
bool ClassInfo::checkAllocSizeOfArray(const Value *V, Value *ThisObj,
                                      Value *NumOfElems) {
  int64_t ElemSize = getElemTySize();
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
  V = computeMultiplier(V, &Multiplier);
  if (!V || Multiplier != ElemSize)
    return false;

  if (NumOfElems) {
    if (V == NumOfElems)
      return true;
    return false;
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
                                                  Value *ThisObj) {
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

  Value *LValue = checkCondition(Pred, BB);
  if (!LValue)
    return false;
  Value *StoredOp = CapacityInitInst->getValueOperand();
  // Check if LValue represents Capacity value.
  if (StoredOp != LValue &&
      !checkFieldOfArgClassLoad(LValue, ThisObj, CapacityField))
    return false;
  return true;
}

// Returns true if any pointer in ArrayPtrAliases is initialized with
// valid memset. Set MemsetInCtor if "CalledFromCtor" is true.
//
// Ex:
//     %25 = phi i8* [ %21, %20 ], [ %23, %22 ]
//     %27 = bitcast i8* %25 to i16**
//     ...
//     call void @llvm.memset.p0i8.i64(i8* %25, i8 0, i64 ArrSize, i1 false)
//
bool ClassInfo::checkMemset(SmallPtrSetImpl<Value *> &ArrayPtrAliases,
                            Value *ThisObj, bool CalledFromCtor = false) {
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
      if (checkAllocSizeOfArray(II->getArgOperand(2), ThisObj, nullptr))
        MemsetCalled++;
      MemsetI = II;
    }

  if (MemsetCalled != 1)
    return false;
  assert(MemsetI && "Expected valid memset instruction");
  if (CalledFromCtor) {
    // MemsetInCtor is used later during transformation.
    if (MemsetInCtor)
      return false;
    MemsetInCtor = MemsetI;
  }
  // Check if it is controlled under "Capacity != 0" condition.
  if (!checkBBControlledUnderCapacityVal(MemsetI->getParent(), ThisObj))
    return false;
  Visited.insert(MemsetI);
  return true;
}

// Returns true if Val is allocation call that allocates memory
// for element array. If NumOfElems is not null, it checks
// that memory is allocated for *NumOfElems of elements.
// Collect allocation calls and position of size argument in
// AllocsInCtor if CallFromCtor is true.
bool ClassInfo::checkAllocCall(Value *Val, Argument *ThisObj, Value *NumOfElems,
                               bool CallFromCtor = false) {
  auto *AllocCall = dyn_cast<CallInst>(Val->stripPointerCasts());
  if (!AllocCall)
    return false;
  auto *CallInfo = DTInfo.getCallInfo(AllocCall);
  if (!CallInfo)
    return false;
  if (CallInfo->getCallInfoKind() != dtrans::CallInfo::CIK_Alloc)
    return false;
  auto AKind = cast<dtrans::AllocCallInfo>(CallInfo)->getAllocKind();
  if (AKind != dtrans::AK_Malloc && AKind != dtrans::AK_New &&
      !isUserAllocKind(AKind))
    return false;

  SmallPtrSet<const Value *, 3> Args;
  auto &TLI = GetTLI(*AllocCall->getFunction());
  collectSpecialAllocArgs(AKind, AllocCall, Args, TLI);
  assert(Args.size() == 1 && "Unexpected allocation function");
  const Value *SizeArg = *Args.begin();

  // Check size should be equal to element array size.
  if (!checkAllocSizeOfArray(SizeArg, ThisObj, NumOfElems))
    return false;
  Visited.insert(AllocCall);
  // Allocation call and size arguments are collected in AllocsInCtor
  // to use them during transformation.
  if (CallFromCtor) {
    unsigned SizeInd = 0;
    unsigned CountInd = 0;
    getAllocSizeArgs(AKind, AllocCall, SizeInd, CountInd, TLI);
    AllocsInCtor.insert(std::make_pair(AllocCall, SizeInd));
  }
  return true;
}

// Returns true if Value is Vtable pointer of MemInterface pointer.
//
// Ex:
//        store %"MemInt"* %3, %"MemInt"** %10 // Store to MemInt field.
//        %13 = bitcast %"MemInt"* %3 to i8* (%"MemInt"*, i64)***
//        %14 = load i8* (%"MemInt"*, i64)**, i8* (%"MemInt"*, i64)*** %13
// V:     %15 = bitcast i8* (%"MemInt"*, i64)** to i8*
bool ClassInfo::checkVtablePtrOfMemInt(Value *V, Argument *ThisObj) {
  if (auto *BC = dyn_cast<BitCastInst>(V)) {
    Visited.insert(BC);
    V = BC->getOperand(0);
  }
  auto *VTLoad = dyn_cast<LoadInst>(V);
  if (!VTLoad)
    return false;
  auto *MemI = VTLoad->getPointerOperand();
  if (!checkMemInterfacePointer(MemI, ThisObj))
    return false;
  Visited.insert(VTLoad);
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
  auto *PTy = dyn_cast_or_null<DTransPointerType>(getDTransType(LI));
  if (!PTy || !PTy->getPointerElementType()->isFunctionTy())
    return false;
  auto *GEP = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
  if (!GEP)
    return false;
  // Just fixing offset.
  if (GEP->getNumIndices() != 1)
    return false;
  if (!checkVtablePtrOfMemInt(GEP->getPointerOperand(), ThisObj))
    return false;
  Visited.insert(LI);
  Visited.insert(GEP);
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
// ArrayPtrAliases will be updated with allocated pointer aliases.
// If NumOfElems is not null, it checks that memory is allocated
// for *NumOfElems of elements.
// Collect allocation calls and position of size argument in
// AllocsInCtor if CallFromCtor is true.
bool ClassInfo::checkAllocatedArrayPtr(
    Value *ValOp, Argument *ThisObj, SmallPtrSetImpl<Value *> &ArrayPtrAliases,
    Value *NumOfElems = nullptr, bool CallFromCtor = false) {

  // Collection of allocated pointer aliases.
  ArrayPtrAliases.clear();
  ArrayPtrAliases.insert(ValOp);
  if (auto *BC = dyn_cast<BitCastInst>(ValOp)) {
    Visited.insert(BC);
    ValOp = BC->getOperand(0);
    ArrayPtrAliases.insert(ValOp);
  }
  // Just return true if it is alloc call.
  if (checkAllocCall(ValOp, ThisObj, NumOfElems, CallFromCtor))
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
    if (DTransAllocCollector::isDummyFuncWithThisAndIntArgs(
            AllocCall, GetTLI(*AllocCall->getFunction()),
            DTInfo.getTypeMetadataReader())) {
      Visited.insert(AllocCall);
      Value *DSizeArg = AllocCall->getArgOperand(1);
      // Make sure size value that is passed to dummy allocation is
      // valid array size even though it is not used.
      if (!checkAllocSizeOfArray(DSizeArg, ThisObj, NumOfElems))
        return false;
      if (CallFromCtor)
        AllocsInCtor.insert(std::make_pair(AllocCall, 1));
      continue;
    }
    if (!checkAllocCall(Val, ThisObj, NumOfElems, CallFromCtor))
      return false;
    AllocFound = true;
  }
  if (!AllocFound)
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

// Returns true if V is integer argument.
// Ex:
//
//   foo(..., i32 %2, ... ) {
//
//     BB1:
//       %21 = zext i32 %2 to i64
//       ...
//     BB2:
//        %27 = zext i32 %2 to i64
//        ...
//     BB3:
//  V:    %43 = phi i64 [ %21, %BB1 ], [ %27, %BB2 ]
//
//
//   }
//
Value *ClassInfo::isIntegerArgument(Value *V) {

  // Returns true if V is simple integer argument.
  //  Ex:
  //    foo(..., i32 %2, ... ) {
  //
  // V:   %27 = zext i32 %2 to i64
  //    }
  auto IsSimpleIntArgument = [this](Value *V) -> Argument * {
    ZExtInst *Zext = dyn_cast<ZExtInst>(V);
    if (Zext)
      V = Zext->getOperand(0);
    auto *Arg = dyn_cast<Argument>(V);
    if (!Arg || !Arg->getType()->isIntegerTy(32))
      return nullptr;
    if (Zext)
      Visited.insert(Zext);
    return Arg;
  };

  auto *PN = dyn_cast<PHINode>(V);
  if (!PN)
    return IsSimpleIntArgument(V);

  if (PN->getNumIncomingValues() != 2)
    return nullptr;
  Value *SameArg = nullptr;
  for (auto I = 0; I < 2; I++) {
    Value *Val = IsSimpleIntArgument(PN->getIncomingValue(I));
    if (!Val)
      return nullptr;
    if (!SameArg)
      SameArg = Val;
    else if (SameArg != Val)
      return nullptr;
  }
  Visited.insert(PN);
  return SameArg;
}

// Returns location of the array element accessed by Val.
// It returns %AtLoc in the example. If CheckLocAsArgument is
// true, it makes sure AtLoc is integer argument.
// Ex:
//      %44 = getelementptr %"class.B", %"class.B"* %ThisObj, i64 0, i32 4
//      %45 = load i16**, i16*** %44
// Val: %46 = getelementptr i16*, i16** %45, i64 %AtLoc
//
Value *ClassInfo::isArrayElementAt(const Value *Val, Value *ThisObj,
                                   bool CheckLocAsArgument = true) {
  Value *AtLoc = nullptr;
  auto *BC2 = dyn_cast<BitCastInst>(Val);
  if (BC2)
    Val = BC2->getOperand(0);
  auto *GEP = dyn_cast<GetElementPtrInst>(Val);
  if (!GEP)
    return AtLoc;
  if (GEP->getNumIndices() != 1)
    return AtLoc;
  const Value *GEPOp = GEP->getPointerOperand();
  auto *BC = dyn_cast<BitCastInst>(GEPOp);
  if (BC)
    GEPOp = BC->getOperand(0);
  if (!checkFieldOfArgClassLoad(GEPOp, ThisObj, ArrayField))
    return AtLoc;
  if (CheckLocAsArgument)
    AtLoc = isIntegerArgument(GEP->getOperand(1));
  else
    AtLoc = GEP->getOperand(1);
  if (!AtLoc)
    return nullptr;
  // Skip ZExt instruction.
  if (auto *Zext = dyn_cast<ZExtInst>(AtLoc)) {
    Visited.insert(Zext);
    AtLoc = Zext->getOperand(0);
  }
  Visited.insert(GEP);
  if (BC)
    Visited.insert(BC);
  if (BC2)
    Visited.insert(BC2);
  return AtLoc;
}

// Return some_index if Val is "Addr + some_index"
//
// Ex:
//      %arrayidx = getelementptr i32**, i32*** %Addr, i64 %Loc
// Val: %8 = bitcast i32*** %arrayidx12 to i64*
//
Value *ClassInfo::isElementIndexAddress(const Value *Val, Value *Addr) {
  auto *BC = dyn_cast<BitCastInst>(Val);
  if (BC)
    Val = BC->getOperand(0);
  auto *GEP = dyn_cast<GetElementPtrInst>(Val);
  if (!GEP)
    return nullptr;
  if (GEP->getNumIndices() != 1)
    return nullptr;
  if (skipCasts(Addr) != skipCasts(GEP->getOperand(0)))
    return nullptr;
  Visited.insert(GEP);
  if (BC)
    Visited.insert(BC);
  return GEP->getOperand(1);
}

// Returns some_index if Val is "(Addr + some_index).some_field"
// ElemIdx is updated with field index of ElemTy that is accessed.
//
// Ex:
//      %70 = getelementptr %"E", %"E"* %Addr, i64 %Loc
//      %72 = getelementptr %"E", %"E"* %70, i64 0, i32 1
// Val: %73 = bitcast %"D"** %72 to i64*
//
Value *ClassInfo::isStructElementIndexAddress(const Value *Val,
                                              DTransType *ElemTy, Value *Addr,
                                              int *ElemIdx) {
  auto *BC = dyn_cast<BitCastInst>(Val);
  if (BC)
    Val = BC->getOperand(0);
  auto *GEP = dyn_cast<GetElementPtrInst>(Val);
  if (!GEP)
    return nullptr;
  auto *SElemType = getDTransType(GEP->getOperand(0));
  if (SElemType != ElemTy || GEP->getNumOperands() != 3 ||
      !isa<ConstantInt>(GEP->getOperand(2)) ||
      !isa<ConstantInt>(GEP->getOperand(1)) ||
      !cast<ConstantInt>(GEP->getOperand(1))->isZero())
    return nullptr;
  if (BC)
    Visited.insert(BC);
  Visited.insert(GEP);
  if (ElemIdx)
    *ElemIdx = cast<ConstantInt>(GEP->getOperand(2))->getLimitedValue();
  return isElementIndexAddress(GEP->getPointerOperand(), Addr);
}

// Returns location of the array element that is loading.
// It returns %AtLoc in the example. If CheckLocAsArgument is
// true, it makes sure AtLoc is integer argument.
//
// Ex:
//        %15 = getelementptr %"class.B", %"class.B"* %0, i64 0, i32 4
//        %16 = load i16**, i16*** %15
//        %AtLoc = zext i32 %1 to i64
//        %18 = getelementptr i16*, i16** %16, i64 %AtLoc
// Val:  %19 = load i16*, i16** %18
//
Value *ClassInfo::isArrayElementLoadAt(const Value *Val, Value *ThisObj,
                                       bool CheckLocAsArgument = true) {
  Value *AtLoc = nullptr;
  const BitCastInst *BC = nullptr;
  auto *LI = dyn_cast<LoadInst>(Val);
  if (!LI)
    return AtLoc;
  const Value *PtrOp = LI->getPointerOperand();
  BC = dyn_cast<BitCastInst>(PtrOp);
  if (BC)
    PtrOp = BC->getOperand(0);
  AtLoc = isArrayElementAt(PtrOp, ThisObj, CheckLocAsArgument);
  if (!AtLoc)
    return AtLoc;
  Visited.insert(LI);
  if (BC)
    Visited.insert(BC);
  return AtLoc;
}

// Returns location of element address in struct, which is element
// type of vector class.
// It returns %AtLoc in the example. If CheckLocAsArgument is
// true, it makes sure AtLoc is integer argument.
// If ElemIdx is not null, it updates with the field index of array
// element that is accessed.
// Ex:
//      %15 = getelementptr %"class.V", %"class.V"* %0, i64 0, i32 3
//      %16 = load %"ElemTy"*, %"ElemTy"** %15
//      %AtLoc = zext i32 %1 to i64
//      %18 = getelementptr %"ElemTy", %"ElemTy"* %16, i64 %AtLoc
// Val: %19 = getelementptr %"ElemTy", %"ElemTy"* %18, i64 0, i32 0
//
Value *ClassInfo::isArrayElementAddressAt(Value *Val, DTransType *ElemTy,
                                          Value *ThisObj,
                                          int *ElemIdx = nullptr,
                                          bool CheckLocAsArgument = true) {
  Value *AtLoc = nullptr;
  BitCastInst *BC;
  BC = dyn_cast<BitCastInst>(Val);
  if (BC)
    Val = BC->getOperand(0);
  auto *GEP = dyn_cast<GetElementPtrInst>(Val);
  if (!GEP)
    return nullptr;
  auto *SElemType = getDTransType(GEP->getOperand(0));
  if (SElemType != ElemTy || GEP->getNumOperands() != 3 ||
      !isa<ConstantInt>(GEP->getOperand(2)) ||
      !isa<ConstantInt>(GEP->getOperand(1)) ||
      !cast<ConstantInt>(GEP->getOperand(1))->isZero())
    return AtLoc;
  AtLoc =
      isArrayElementAt(GEP->getPointerOperand(), ThisObj, CheckLocAsArgument);
  if (!AtLoc)
    return AtLoc;
  if (BC)
    Visited.insert(BC);
  Visited.insert(GEP);
  if (ElemIdx)
    *ElemIdx = cast<ConstantInt>(GEP->getOperand(2))->getLimitedValue();
  return AtLoc;
}

// Returns the location of element address in struct if Val
// is loading a field of array struct element.
// It returns %Loc1 in the example. If CheckLocAsArgument is
// true, it makes sure Loc1 is integer argument.
// If ElemIdx is not null, it updates with the field index of array
// element that is accessed.
// Ex:
//      %49 = getelementptr %"E", %"E"* %Array_ThisObj, i64 %Loc1
//      %51 = getelementptr %"E", %"E"* %49, i64 0, i32 1
//      %52 = bitcast %"D"** %51 to i64*
// Val: %54 = load i64, i64* %52
Value *ClassInfo::isArrayStructElementLoadAt(Value *Val, DTransType *ElemTy,
                                             Value *ThisObj,
                                             int *ElemIdx = nullptr,
                                             bool CheckLocAsArgument = true) {
  Value *AtLoc = nullptr;
  auto *LI = dyn_cast<LoadInst>(Val);
  if (!LI)
    return AtLoc;
  Value *PtrOp = LI->getPointerOperand();
  BitCastInst *BC = dyn_cast<BitCastInst>(PtrOp);
  if (BC)
    PtrOp = BC->getOperand(0);
  AtLoc = isArrayElementAddressAt(PtrOp, ElemTy, ThisObj, ElemIdx,
                                  CheckLocAsArgument);
  if (!AtLoc)
    return AtLoc;
  Visited.insert(LI);
  if (BC)
    Visited.insert(BC);
  return AtLoc;
}

// Returns true if Val is ThisObj.
//
// Ex:
//    Val:   %65 = bitcast %"class.RefOf"* %ThisObj to i8*
//
bool ClassInfo::checkCompleteObjPtr(const Value *Val, Argument *ThisObj) {
  const Value *Op = Val;
  auto *BC = dyn_cast<BitCastInst>(Val);
  if (BC)
    Op = BC->getOperand(0);
  if (Op != ThisObj)
    return false;
  if (BC)
    Visited.insert(BC);
  return true;
}

// Checks that all instructions in BB are related to EH and ThisObj
// is accessed only to get MemIntType field.
//
// TODO: If needed, we could extend the current implementation
// to recognize the exact functionality of EH blocks.
//
bool ClassInfo::checkEHBlock(BasicBlock *BB, Argument *ThisObj) {

  // Returns true if I is "free(ThisObj)".
  auto CheckFreeCall = [this](Instruction *I, Argument *ThisObj) {
    auto *FreeI = getFreeCall(I);
    if (!FreeI)
      return false;
    const Value *Ptr = getFreeArg(FreeI);
    if (!checkCompleteObjPtr(Ptr, ThisObj))
      return false;
    return true;
  };

  // Returns true if Call is a call to LibFunc LB.
  auto IsLibFunction = [this](CallBase *Call, LibFunc LB) {
    LibFunc LibF;
    auto &TLI = GetTLI(*Call->getFunction());
    auto *F = dyn_cast_or_null<Function>(Call->getCalledFunction());
    if (!F || !TLI.getLibFunc(*F, LibF) || !TLI.has(LibF))
      return false;
    if (LibF != LB)
      return false;
    return true;
  };

  // Returns true if Call is any of the below:
  //  1. "free(ThisObj)"
  //  2. "__clang_call_terminate" or "__std_terminate"
  //  3. Other calls: Arguments should be constants or MemIntField or return
  //     value of __cxa_allocate_exception.
  //
  auto IsCallOkayInEH = [this, &CheckFreeCall,
                         &IsLibFunction](CallBase *Call, Argument *ThisObj) {
    if (IsLibFunction(Call, LibFunc_clang_call_terminate) ||
        IsLibFunction(Call, LibFunc_dunder_std_terminate) ||
        CheckFreeCall(Call, ThisObj)) {
      Visited.insert(Call);
      return true;
    }
    Visited.insert(Call);
    for (Value *Arg : Call->args()) {
      if (isa<Constant>(Arg))
        continue;
      if (checkFieldOfArgClassLoad(Arg, ThisObj, MemIntField))
        continue;
      Value *Val = Arg;
      BitCastInst *BC = dyn_cast<BitCastInst>(Val);
      if (BC) {
        Val = BC->getOperand(0);
        Visited.insert(BC);
      }
      if (auto AI = dyn_cast<AllocaInst>(Val)) {
        Visited.insert(AI);
      } else if (auto CB = dyn_cast<CallBase>(Val)) {
        if (!IsLibFunction(CB, LibFunc_cxa_allocate_exception))
          return false;
        Visited.insert(CB);
      } else {
        return false;
      }
    }
    return true;
  };

  for (auto &I : *BB) {
    switch (I.getOpcode()) {
    default:
      return false;

    // Don't mark them as visited here. These instructions will be
    // marked as visited when call instructions are processed.
    case Instruction::GetElementPtr:
    case Instruction::BitCast:
    case Instruction::Load:
    case Instruction::Br:
      continue;

    case Instruction::PHI:
    case Instruction::Resume:
    case Instruction::Unreachable:
    case Instruction::ExtractValue:
    case Instruction::InsertValue:
    case Instruction::LandingPad:
    case Instruction::CleanupPad:
    case Instruction::CleanupRet:
      Visited.insert(&I);
      continue;

    case Instruction::Call:
    case Instruction::Invoke:
      if (IsCallOkayInEH(cast<CallBase>(&I), ThisObj))
        Visited.insert(&I);
      else
        return false;
    }
  }
  return true;
}

// Returns true if BB and its successors are all related to EH.
//
//        BB:          ...
//                     Invoke %Non-Unwind  %Unwind
//
//        %Non-Unwind: ...
//                     unreachable
//
//        %Unwind:     ...
//                     resume
//
//   First, check there are no successors for %Non-Unwind and %Unwind
//   blocks. Then, check all BB, %Non-Unwind and %Unwind are blocks that
//   don't have any code that accesses to ThisObj except MemIntType field.
//
bool ClassInfo::isEHRelatedBB(BasicBlock *BB, Argument *ThisObj) {

  // Returns true if Terminator is EH related instructions.
  auto IsNoSuccTerminator = [](Instruction *I) {
    if (isa<UnreachableInst>(I) || isa<ResumeInst>(I))
      return true;
    return false;
  };

  if (!BB->hasNPredecessors(1))
    return false;
  // Return true if it is a simple block with just EH code.
  if (IsNoSuccTerminator(BB->getTerminator()) && checkEHBlock(BB, ThisObj))
    return true;
  auto *II = dyn_cast<InvokeInst>(BB->getTerminator());
  if (!II)
    return false;
  auto *ND = II->getNormalDest();
  auto *UD = II->getUnwindDest();
  // Check no successors for ND and UD.
  if (!IsNoSuccTerminator(ND->getTerminator()) ||
      !IsNoSuccTerminator(UD->getTerminator()))
    return false;
  // Makes sure no actual code in BB, ND and UD.
  if (!checkEHBlock(BB, ThisObj) || !checkEHBlock(ND, ThisObj) ||
      !checkEHBlock(UD, ThisObj))
    return false;
  return true;
}

// Returns FalseBB if BB is controlled like below
//
//       if (Any_Predicate LValue, RValue)
//           BB
//  LValue and RValue are updated with operands of ICmp.
//  Predi is updated with predicate of ICmp.
//
BasicBlock *ClassInfo::getBBControlledOps(BasicBlock *BB, Value **LValue,
                                          Value **RValue,
                                          ICmpInst::Predicate *Predi) {
  // Makes sure BB has single predecessor.
  BasicBlock *Pred = BB->getSinglePredecessor();
  if (!Pred)
    return nullptr;
  auto *BI = dyn_cast<BranchInst>(Pred->getTerminator());
  if (!BI || !BI->isConditional())
    return nullptr;
  Value *BrCond = BI->getCondition();
  ICmpInst *IC = dyn_cast<ICmpInst>(BrCond);
  if (!IC)
    return nullptr;
  if (BI->getSuccessor(0) != BB)
    return nullptr;

  *RValue = IC->getOperand(1);
  *LValue = IC->getOperand(0);
  *Predi = IC->getPredicate();
  Visited.insert(BI);
  Visited.insert(IC);
  return BI->getSuccessor(1);
}

// Returns true if BB is controlled like below
//
//       if (SizeField > AtLoc)
//           BB
//       else
//           EH_Code
//
bool ClassInfo::isControlledUnderSizeCheck(BasicBlock *BB, Argument *ThisObj,
                                           Value *AtLoc) {
  Value *LValue;
  Value *RValue;
  ICmpInst::Predicate Predi;
  BasicBlock *FBlock = getBBControlledOps(BB, &LValue, &RValue, &Predi);
  if (!FBlock || Predi != ICmpInst::ICMP_UGT)
    return false;

  if (RValue != AtLoc || !checkFieldOfArgClassLoad(LValue, ThisObj, SizeField))
    return false;

  // Checks False Block is just EH related code.
  if (!isEHRelatedBB(FBlock, ThisObj))
    return false;
  return true;
}

// Returns FreeCallInfo if I is a call to "free".
dtrans::FreeCallInfo *ClassInfo::getFreeCall(Instruction *I) {
  if (isa<CallBase>(I)) {
    auto *Info = DTInfo.getCallInfo(I);
    if (Info && Info->getCallInfoKind() == dtrans::CallInfo::CIK_Free)
      return cast<dtrans::FreeCallInfo>(Info);
  }
  return nullptr;
}

// Returns argument of given FInfo.
const Value *ClassInfo::getFreeArg(dtrans::FreeCallInfo *FInfo) {
  Instruction *CallI = FInfo->getInstruction();
  SmallPtrSet<const Value *, 3> Args;

  collectSpecialFreeArgs(FInfo->getFreeKind(), cast<CallBase>(CallI), Args,
                         GetTLI(*CallI->getFunction()));
  assert(Args.size() == 1 && "Unexpected free function");
  return *Args.begin();
}

// Returns true if
//
// 1. Argument of FInfo is not used by anyone except FInfo call. BBPtr is
//    updated with basic block of FInfo.
//
// or
//
// 2. Argument of FInfo is used in another dummy Free call. Both FInfo call
//    and the dummy free call should be controlled by same indirect
//    call condition. BBPtr is updated with basic block of indirect call
//    check.
//
const Value *ClassInfo::checkFree(dtrans::FreeCallInfo *FInfo,
                                  Argument *ThisObj, BasicBlock **BBPtr) {

  // Returns true if FCall1 and FCall2 are controlled under the same
  // indirect call condition.
  //
  //  Ex:
  //        if (indirect call check)
  //           FCall1;
  //        else
  //           FCall2;
  //
  auto AreFreeCallsUnderSameCondition =
      [this](CallBase *FCall1, const CallBase *FCall2, Argument *ThisObj) {
        const BasicBlock *TB = FCall1->getParent();
        const BasicBlock *FB = FCall2->getParent();
        const BasicBlock *BB = FB->getSinglePredecessor();
        if (!BB)
          return false;
        auto *BI = dyn_cast<BranchInst>(BB->getTerminator());
        if (!BI || !BI->isConditional())
          return false;
        if (!isIndirectCallCheck(BI->getCondition(), ThisObj))
          return false;
        if (TB != BI->getSuccessor(0) || FB != BI->getSuccessor(1))
          return false;
        Visited.insert(BI);
        return true;
      };

  const Value *PtrArg = getFreeArg(FInfo);
  Instruction *CallI = FInfo->getInstruction();
  const CallBase *DummyFreeCall = nullptr;
  BasicBlock *FreeCallBlock = CallI->getParent();
  for (auto *U : PtrArg->users()) {
    // Bitcasts are removed with opaque pointers before passing
    // argument to "free" call. Due to this, PtrArg has more users.
    if (U == CallI || isa<ReturnInst>(U) || isa<GetElementPtrInst>(U))
      continue;
    DummyFreeCall = dyn_cast<CallBase>(U);
    if (!DummyFreeCall)
      return nullptr;
    auto *CalledF = dtrans::getCalledFunction(*DummyFreeCall);
    if (!CalledF)
      return nullptr;
    // Ignore here if PtrArg is used by Destructor call.
    // Not adding DummyFreeCall to Visited here so that verification
    // of Destructor call is expected to be done at callsites.
    if (InitialFuncKind[CalledF] == Destructor)
      continue;
    if (!DTransAllocCollector::isDummyFuncWithThisAndInt8PtrArgs(
            DummyFreeCall, GetTLI(*CallI->getFunction()),
            DTInfo.getTypeMetadataReader()))
      return nullptr;
    if (!AreFreeCallsUnderSameCondition(cast<CallBase>(CallI), DummyFreeCall,
                                        ThisObj))
      return nullptr;
    FreeCallBlock = CallI->getParent()->getSinglePredecessor();
  }
  Visited.insert(CallI);
  if (DummyFreeCall)
    Visited.insert(DummyFreeCall);
  *BBPtr = FreeCallBlock;
  return PtrArg;
}

// Checks that BB has conditional branch like below and returns %RetValue
// if it matches.
//
//    BB: ...
//        %Cond = icmp eq i8 %RetValue, 0
//        br i1 %Cond, label %some_block, label %CheckBB
//
Value *ClassInfo::checkCondition(BasicBlock *BB, BasicBlock *CheckBB) {
  auto *BI = dyn_cast<BranchInst>(BB->getTerminator());
  if (!BI || !BI->isConditional())
    return nullptr;

  ICmpInst *Cond = dyn_cast<ICmpInst>(BI->getCondition());
  if (!Cond)
    return nullptr;

  ConstantInt *CmpZero = dyn_cast<ConstantInt>(Cond->getOperand(1));
  if (!CmpZero || !CmpZero->isZero())
    return nullptr;

  ICmpInst::Predicate Pred = Cond->getPredicate();
  // Checking only ICmpInst::ICMP_EQ for simple implementation.
  if (Pred != ICmpInst::ICMP_EQ || BI->getSuccessor(1) != CheckBB)
    return nullptr;
  Visited.insert(BI);
  Visited.insert(Cond);
  return Cond->getOperand(0);
}

// Returns true if BB is controlled under Flag field.
//
//   Ex:
//      Pred:
//         %cond = icmp eq i8 %FlagField, 0
//         br i1 %cond, label %some_bb, label %BB
//
//      BB:
//
//    Or
//
//      Pred:
//         %cond = icmp eq i8 1, 0
//         br i1 %cond, label %some_bb, label %BB
//
//      BB:
//
bool ClassInfo::checkBBControlledUnderFlagVal(BasicBlock *BB,
                                              Argument *ThisObj) {
  BasicBlock *Pred = BB->getSinglePredecessor();
  if (!Pred)
    return false;
  Value *LValue = checkCondition(Pred, BB);
  if (!LValue)
    return false;
  ConstantInt *FVal = dyn_cast<ConstantInt>(LValue);
  if ((FVal && FVal->isOne()) ||
      checkFieldOfArgClassLoad(LValue, ThisObj, FlagField))
    return true;
  return false;
}

// Returns true if given loop L is controlled under
// zero trip test.
//
//   Ex 1:
//    PreCondBB: ...
//              %cond = icmp eq i32 SizeField, 0
//              br i1 %cond, label %some_bb, label %PH
//
//           PH: ...
//              br label %EntryBI
//
//      EntryBI:...
//              Loop
//
//   Ex 2:
//    PreCondBB: ...
//              %cond = icmp eq i32 SizeField, 0
//              br i1 %cond, label %some_bb, label %EntryBI
//
//      EntryBI:...
//              Loop
bool ClassInfo::checkZTT(Loop *L, Value *ThisObj) {
  BasicBlock *PH = L->getLoopPreheader();
  BasicBlock *PreCondBB = nullptr;
  BasicBlock *SuccBB;
  if (PH) {
    auto *EntryBI = dyn_cast<BranchInst>(PH->getTerminator());
    if (!EntryBI || EntryBI->isConditional())
      return false;
    PreCondBB = PH->getSinglePredecessor();
    SuccBB = PH;
  } else {
    PreCondBB = L->getLoopPredecessor();
    SuccBB = L->getHeader();
  }
  if (!PreCondBB)
    return false;

  Value *Ptr = checkCondition(PreCondBB, SuccBB);
  if (!checkFieldOfArgClassLoad(Ptr, ThisObj, SizeField))
    return false;
  return true;
}

// Verifies that Ind is a loop counter and returns the Loop if it is
// like below.
//
// Ex:
//    Loop:
//         %Ind = phi  i64 [ 0, %b1 ], [ %AddI, %Latch ]
//          ...
//
//    Latch:   %AddI = add nuw nsw i64 %Ind, 1
//             %Zext = zext i32 %SizeField to i64
//             %Cond = icmp ult i64 %AddI, %Zext
//             br i1 %Cond, label %Loop, label %some_bb
//
Loop *ClassInfo::checkLoop(Value *Ind, Value *ThisObj, LoopInfo &LI) {
  auto *PN = dyn_cast<PHINode>(Ind);
  if (!PN || PN->getNumIncomingValues() != 2)
    return nullptr;
  Loop *L = LI.getLoopFor(PN->getParent());
  if (!L || L->getNumBackEdges() != 1 || !L->getSubLoops().empty() ||
      L->getParentLoop() || L->getHeader() != PN->getParent())
    return nullptr;
  BasicBlock *Latch = L->getLoopLatch();
  BasicBlock *PredBB = L->getLoopPredecessor();
  if (!PredBB)
    return nullptr;
  Value *V1 = PN->getIncomingValueForBlock(PredBB);
  Value *V2 = PN->getIncomingValueForBlock(Latch);
  ConstantInt *Init = dyn_cast<ConstantInt>(V1);
  if (!Init || !Init->isZero())
    return nullptr;
  BranchInst *BI = dyn_cast<BranchInst>(Latch->getTerminator());
  if (!BI || !BI->isConditional())
    return nullptr;

  ICmpInst *Cond = dyn_cast<ICmpInst>(BI->getCondition());
  if (!Cond)
    return nullptr;

  ICmpInst::Predicate Pred = Cond->getPredicate();
  // Allow both ICMP_ULT and ICMP_EQ predicates for now.
  if ((Pred != ICmpInst::ICMP_ULT || BI->getSuccessor(0) != L->getHeader()) &&
      (Pred != ICmpInst::ICMP_EQ || BI->getSuccessor(1) != L->getHeader()))
    return nullptr;

  Value *CmpOp0 = Cond->getOperand(0);
  Value *CmpOp1 = Cond->getOperand(1);
  if (V2 != CmpOp0)
    return nullptr;
  if (!checkFieldOfArgClassLoad(CmpOp1, ThisObj, SizeField))
    return nullptr;

  auto *AddI = dyn_cast<Instruction>(CmpOp0);
  if (!AddI || AddI->getOpcode() != Instruction::Add)
    return nullptr;
  if (AddI->getOperand(0) != PN)
    return nullptr;
  ConstantInt *Inc = dyn_cast<ConstantInt>(AddI->getOperand(1));
  if (!Inc || !Inc->isOne())
    return nullptr;
  Visited.insert(AddI);
  Visited.insert(Cond);
  Visited.insert(BI);
  Visited.insert(PN);
  return L;
}

// Returns true if Loc is induction variable of a loop that
// iterates from 0 to "size" field of "Obj" with increment 1.
Loop *ClassInfo::checkLoopWithZTT(Value *Loc, Value *Obj, LoopInfo &LI) {
  Loop *L = checkLoop(Loc, Obj, LI);
  if (!L)
    return nullptr;
  if (!checkZTT(L, Obj))
    return nullptr;
  return L;
}

// Return ReturnInst if Fn has single ReturnInst. Otherwise, return nullptr.
ReturnInst *ClassInfo::getSingleRetInst(Function *Fn) {
  ReturnInst *RI = nullptr;
  for (BasicBlock &BB : *Fn)
    if (auto *Ret = dyn_cast<ReturnInst>(BB.getTerminator())) {
      if (RI)
        return nullptr;
      else
        RI = Ret;
    }
  return RI;
}

// Collect all store and free call instructions in all basic blocks in Fn
// except basic blocks in IgnoreBBSet.
void ClassInfo::collectStoreInstsFreeCalls(
    Function *Fn, SmallPtrSetImpl<BasicBlock *> &IgnoreBBSet,
    SmallPtrSetImpl<StoreInst *> &StoresSet,
    SmallPtrSetImpl<dtrans::FreeCallInfo *> &FreeCallsSet) {
  for (auto &BB : *Fn) {
    if (IgnoreBBSet.count(&BB))
      continue;
    for (auto &I : BB)
      if (auto *II = dyn_cast<StoreInst>(&I))
        StoresSet.insert(II);
      else if (dtrans::FreeCallInfo *FInfo = getFreeCall(&I))
        FreeCallsSet.insert(FInfo);
  }
}

// Return the argument if Val is Load of an argument.
Value *ClassInfo::isLoadOfArg(Value *Val) {
  if (auto *LI = dyn_cast<LoadInst>(Val)) {
    Value *ValOp = LI->getPointerOperand();
    auto *BC = dyn_cast<BitCastInst>(ValOp);
    if (BC)
      ValOp = BC->getOperand(0);
    if (isa<Argument>(ValOp)) {
      Visited.insert(LI);
      if (BC)
        Visited.insert(BC);
      return ValOp;
    }
  }
  return nullptr;
}

// Check if ValOp is either argument or dereference of argument.
// Return the argument if ValOp is either argument or dereference of argument.
// Otherwise, it returns nullptr.
Value *ClassInfo::isValidArgumentSave(Value *ValOp) {
  if (isa<Argument>(ValOp)) {
    auto *ValType = getDTransType(ValOp);
    if (!ValType)
      return nullptr;
    if (ElemDataTypes.count(ValType))
      return ValOp;
  } else {
    // Check if it is load of argument.
    Value *ArgVal = isLoadOfArg(ValOp);
    auto *ArgValType = getDTransType(ArgVal);
    if (!ArgValType)
      return nullptr;
    if (ArgVal && isa<Argument>(ArgVal) && ElemDataAddrTypes.count(ArgValType))
      return ArgVal;
  }
  return nullptr;
}

// Skip BitCastInst/ZExtInst instructions.
const Value *ClassInfo::skipCasts(const Value *V) {
  while (isa<BitCastInst>(V) || isa<ZExtInst>(V)) {
    auto *I = cast<Instruction>(V);
    Visited.insert(I);
    V = I->getOperand(0);
  }
  return V;
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

    if (isa<DbgInfoIntrinsic>(&I))
      continue;
    // Return false if any instruction is not found in Processed list.
    return false;
  }
  return true;
}

// Process llvm.assume and llvm.type.test calls in function "F".
// Only Vtable pointer inMemInterfaceMemInit struct field of "ThisObj"
// is allowed to pass as first argument of llvm.type.test call.
//
//  Ex:
//     %mgep = getelementptr inbounds %Arr, %* %ThisObj, i64 0, i32 4
//     %mem = load %MemType*, %MemType** %mgep
//     %tmp = bitcast %MemType* %mem to i8* (%MemType*, i32)***
//     %vtable = load i8* (%MemType*, i32)**, i8* (%MemType*, i32)*** %tmp
//     %bc1 = bitcast i8* (%MemType*, i32)** %vtable to i8*
//     %tt = tail call i1 @llvm.type.test(i8* %bc1, metadata !"typeId")
//
bool ClassInfo::processAssumeCalls(Function *F, Argument *ThisObj) {
  for (auto &I : instructions(F)) {
    auto II = dyn_cast<IntrinsicInst>(&I);
    if (!II || II->getIntrinsicID() != Intrinsic::assume)
      continue;
    auto TTCall = dyn_cast<IntrinsicInst>(II->getArgOperand(0));
    if (!TTCall || TTCall->getIntrinsicID() != Intrinsic::type_test ||
        !checkVtablePtrOfMemInt(TTCall->getArgOperand(0), ThisObj)) {
      DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO,
                      { dbgs() << "  Failed: Assume intrinsic analysis.\n"; });
      return false;
    }
    Visited.insert(II);
    Visited.insert(TTCall);
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
FunctionKind ClassInfo::recognizeDerivedConstructor(Function *F,
                                                    DTransType *DTy,
                                                    DTransType *BTy) {
  unsigned ConstructorCall = 0;
  SmallVector<GetElementPtrInst *, 2> GEPList;
  Argument *ThisObj = &*F->arg_begin();
  SmallPtrSet<const Instruction *, 8> ProcessedInsts;

  // Collect all uses of This pointer. Expected only GEP as user.
  for (auto *U : ThisObj->users())
    if (auto *GEPI = dyn_cast<GetElementPtrInst>(U))
      GEPList.push_back(GEPI);
    else if (auto *RI = dyn_cast<ReturnInst>(U))
      ProcessedInsts.insert(RI);
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
    auto *GEPTy = getResultElementDTransType(GEP);
    if (!GEPTy)
      return UnKnown;
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
  if (!processAssumeCalls(F, ThisObj))
    return UnKnown;

  // Check all instructions are processed. Make Vtable store
  // as optional.
  if (ConstructorCall != 1 || !checkAllInstsProcessed(F, ProcessedInsts))
    return UnKnown;
  FinalFuncKind[F] = Constructor;
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
          auto *GEPTy = getResultElementDTransType(GEP);
          if (!GEPTy)
            return false;
          if (!isAccessingFieldOfArgClass(GEP, ThisObj, &Idx))
            return false;

          // Ignore array pointer field.
          if (GEPTy->isPointerTy() &&
              cast<DTransPointerType>(GEPTy)->getPointerElementType() ==
                  getElementTy())
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
            if (ValOp->getType()->isIntegerTy(8)) {
              // Flag is assigned here.
              FlagFieldAssign++;
              FlagField = Idx;
            } else if (ValOp->getType()->isIntegerTy(32)) {
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
              auto *PTy = cast<DTransPointerType>(GEPTy);
              auto *PtrElemTy = PTy->getPointerElementType();
              if (PtrElemTy == SOACInfo->getMemInterfaceType()) {
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
          auto *GEPTy = getResultElementDTransType(GEP);
          if (!GEPTy)
            return false;
          // Skip non-array pointer fields here.
          if (!isa<DTransPointerType>(GEPTy))
            continue;
          auto *PTy = cast<DTransPointerType>(GEPTy);
          auto *PtrElemTy = PTy->getPointerElementType();
          if (PtrElemTy == SOACInfo->getMemInterfaceType() ||
              isPtrToVFTable(GEPTy))
            continue;

          if (!isAccessingFieldOfArgClass(GEP, ThisObj, &Idx))
            return false;
          Visited.insert(GEP);
          SmallPtrSet<Value *, 4> ArrayPtrAliases;
          for (auto *U : GEP->users()) {
            // Something wrong if it is not array pointer field.
            if (PtrElemTy != getElementTy())
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
            } else if (checkAllocatedArrayPtr(ValOp, ThisObj, ArrayPtrAliases,
                                              nullptr, true)) {
              // Makes sure allocated array is initialized with memset.
              if (!checkMemset(ArrayPtrAliases, ThisObj, true))
                return false;
              // Memory allocation assignment to array pointer field.
              if (AllocPtrStore)
                return false;
              AllocPtrStore = SI;
              ArrayField = Idx;
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
    else if (auto RI = dyn_cast<ReturnInst>(U))
      Visited.insert(RI);
    else
      return UnKnown;

  // Analyze non-array pointer fields.
  if (!AnalyzeNonArrayPtrFields(GEPList, ThisObj)) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO,
                    { dbgs() << "  Failed: Non-pointer field analysis.\n"; });
    return UnKnown;
  }

  // Analyze array pointer field.
  if (!AnalyzeArrayPtrFields(GEPList, ThisObj)) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO,
                    { dbgs() << "  Failed: Pointer field analysis.\n"; });
    return UnKnown;
  }

  if (!processAssumeCalls(F, ThisObj))
    return UnKnown;

  // Makes sure all instructions are processed.
  if (!checkAllInstsProcessed(F, Visited)) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO, {
      dbgs() << " Failed: Missing to process some instructions.\n";
    });
    return UnKnown;
  }
  FinalFuncKind[F] = Constructor;
  return Constructor;
}

// Analyze Fn as GetSizeOrCapacity.
//
// Ex:
//    getSize(this) {
//      return this->Size;
//    }
//
FunctionKind ClassInfo::recognizeGetSizeOrCapacity(Function *Fn) {
  Argument *ThisObj = &*Fn->arg_begin();
  SmallPtrSet<ReturnInst *, 1> RetList;
  FunctionKind FKind = UnKnown;

  assert(Fn->getReturnType()->isIntegerTy(32) && "Unexpected return type");
  // Clear all processed instructions first.
  Visited.clear();

  // Collect Return stmts.
  ReturnInst *RI = getSingleRetInst(Fn);
  if (!RI)
    return FKind;

  Value *RetVal = RI->getReturnValue();
  // Check if RetVal is value of either Size or Capacity field.
  if (checkFieldOfArgClassLoad(RetVal, ThisObj, SizeField))
    FKind = GetSize;
  else if (checkFieldOfArgClassLoad(RetVal, ThisObj, CapacityField))
    FKind = GetCapacity;
  Visited.insert(RI);

  // Makes sure all instructions are processed.
  if (!checkAllInstsProcessed(Fn, Visited)) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO, {
      dbgs() << " Failed: Missing to process some instructions.\n";
    });
    return UnKnown;
  }
  return FKind;
}

// Analyze it as GetElem functionality.
//
// Ex:
//    getElementAt(this, int Loc) {
//      if (Loc >= Size)
//        EH_Code;
//      return this->Array[Loc];
//    }
//
//  Step 1: Check it is returning array element at Loc.
//  Step 2: Make sure it is controlled under "Loc >= Size" condition.
//
FunctionKind ClassInfo::recognizeGetElem(Function *Fn) {
  Argument *ThisObj = &*Fn->arg_begin();
  SmallPtrSet<ReturnInst *, 1> RetList;

  // Clear all processed instructions first.
  Visited.clear();

  // Collect RetInsts
  ReturnInst *RI = getSingleRetInst(Fn);
  if (!RI)
    return UnKnown;

  // Check if it is returning array element.
  DTransType *ElemTy = getElementTy();
  Value *AtLoc;
  if (isa<DTransStructType>(ElemTy)) {
    AtLoc = isArrayElementAddressAt(RI->getReturnValue(), ElemTy, ThisObj);
  } else {
    // Either data element or address of data element is allowed
    // as return value.
    auto *DTFuncTy = dyn_cast_or_null<DTransFunctionType>(
        DTInfo.getTypeMetadataReader().getDTransTypeFromMD(Fn));
    assert(DTFuncTy && "Must have type if function is being transformed");

    DTransType *RTy = DTFuncTy->getReturnType();
    if (ElemDataAddrTypes.count(RTy))
      AtLoc = isArrayElementAt(RI->getReturnValue(), ThisObj);
    else
      AtLoc = isArrayElementLoadAt(RI->getReturnValue(), ThisObj);
  }
  if (!AtLoc)
    return UnKnown;

  // Makes sure Return is controlled under "Loc >= Size" condition.
  if (!isControlledUnderSizeCheck(RI->getParent(), ThisObj, AtLoc))
    return UnKnown;
  Visited.insert(RI);

  // Makes sure all instructions are processed.
  if (!checkAllInstsProcessed(Fn, Visited)) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO, {
      dbgs() << " Failed: Missing to process some instructions.\n";
    });
    return UnKnown;
  }
  return GetElem;
}

// Analyze Fn as SetElem.
//
// Ex:
//  setElem(this, Elem, Loc)
//  {
//    if (Loc >= this->size_field)
//      EH_Code;
//
//    if (this->flag)                // This IF statement is optional.
//      delete this->array[Loc];
//
//    this->array[Loc] = Elem;
//  }
//
//  It expects mainly 2 sink points.
//  1. Set array element at given location.
//  2. Delete array element at given location before assigning new one. This
//     is controlled under flag value. This is optional.
//  3. Both 1 and 2 are controlled under "Loc >= Size_field" check.
//
FunctionKind ClassInfo::recognizeSetElem(Function *Fn) {
  // Clear all processed instructions first.
  Visited.clear();
  Argument *ThisObj = &*Fn->arg_begin();
  SmallPtrSet<StoreInst *, 1> SIList;
  SmallPtrSet<dtrans::FreeCallInfo *, 4> FreeList;
  SmallPtrSet<BasicBlock *, 16> IgnoreBBSet;

  if (DTransAnnotator::hasDTransSOAToAOSPrepareTypeAnnotation(*Fn))
    return SetElem;

  // Collect Store instructions and Free calls.
  collectStoreInstsFreeCalls(Fn, IgnoreBBSet, SIList, FreeList);

  if (SIList.size() != 1)
    return UnKnown;
  auto *SI = *SIList.begin();
  DTransType *ElemTy = getElementTy();
  Value *SetAtLoc;
  // Set array element at location.
  if (isa<DTransStructType>(ElemTy)) {
    SetAtLoc =
        isArrayElementAddressAt(SI->getPointerOperand(), ElemTy, ThisObj);
    if (!SetAtLoc || !isLoadOfArg(SI->getValueOperand()))
      return UnKnown;
  } else {
    SetAtLoc = isArrayElementAt(SI->getPointerOperand(), ThisObj);
    if (!SetAtLoc)
      return UnKnown;
    Value *ArgVal = isValidArgumentSave(SI->getValueOperand());
    if (!ArgVal)
      return UnKnown;
  }

  int32_t FCount = FreeList.size();
  BasicBlock *FreeBB = nullptr;
  if (FCount) {
    if (FCount != 1)
      return UnKnown;
    dtrans::FreeCallInfo *FInfo = *FreeList.begin();
    // Delete array element at given location.
    const Value *PtrArg = checkFree(FInfo, ThisObj, &FreeBB);
    if (!PtrArg)
      return UnKnown;
    Value *DeleteAtLoc = isArrayElementLoadAt(PtrArg, ThisObj);
    if (!DeleteAtLoc || DeleteAtLoc != SetAtLoc)
      return UnKnown;
    assert(FreeBB && "Expected valid FreeBB");
    // Check deleting is controlled under flag field.
    if (!checkBBControlledUnderFlagVal(FreeBB, ThisObj))
      return UnKnown;
  }
  BasicBlock *CheckBB;
  // Get nearest dominator of basicblock that has deleting existing element and
  // basicblock that has setting new element.
  if (FreeBB)
    CheckBB = (GetDT)(*FreeBB->getParent())
                  .findNearestCommonDominator(FreeBB, SI->getParent());
  else
    CheckBB = SI->getParent();

  // Makes sure deleting existing element and setting new element are
  // controlled under "AtLoc >= Size_field" check.
  if (!CheckBB || !isControlledUnderSizeCheck(CheckBB, ThisObj, SetAtLoc))
    return UnKnown;
  Visited.insert(SI);

  if (!processAssumeCalls(Fn, ThisObj))
    return UnKnown;

  // Makes sure all instructions are processed.
  if (!checkAllInstsProcessed(Fn, Visited)) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO, {
      dbgs() << " Failed: Missing to process some instructions.\n";
    });
    return UnKnown;
  }
  return SetElem;
}

// Analyze Fn as Destructor.
//
//  It expects mainly 4 sink points.
//   1. Store to virtual function pointer  (Optional)
//   2. Free all elements of array in loop. The entire loop is controlled
//      under flag field. (Optional)
//   3. Free entire Array field  (Mandatory)
//   4. Free entire object (Optional)
//
// It checks for the following pattern.
//
// Ex:
//  {
//    this->virtual_func_pointer = some_constant;
//    if (this->flag_field) {
//       for (int i = 0; i < this->size_field; i++) {
//         delete this->array[i];
//       }
//    }
//    free (this->array);  // Except this statement every thing else
//                         // is optional.
//    free(this);
//  }
//
FunctionKind ClassInfo::recognizeDestructor(Function *Fn) {
  std::function<void(BasicBlock *, SmallPtrSet<BasicBlock *, 16> &)>
      CollectAllSuccs;
  // Collects all successors of BB recursively.
  CollectAllSuccs = [&CollectAllSuccs](BasicBlock *BB,
                                       SmallPtrSet<BasicBlock *, 16> &SuccSet) {
    if (!SuccSet.insert(BB).second)
      return;
    for (auto *S : successors(BB)) {
      CollectAllSuccs(S, SuccSet);
    }
  };

  // Collect all EH related basicblocks in Fn.
  auto CollectEHBasicBlocks =
      [&CollectAllSuccs](Function *Fn,
                         SmallPtrSet<BasicBlock *, 16> &EHBasicBlocks) {
        SmallPtrSet<BasicBlock *, 8> UnwindBlocks;
        // First, collect UnwindDest blocks.
        for (BasicBlock &BB : *Fn)
          if (auto *II = dyn_cast<InvokeInst>(BB.getTerminator()))
            UnwindBlocks.insert(II->getUnwindDest());
        // Then, get all their successors.
        for (auto *BB : UnwindBlocks)
          CollectAllSuccs(BB, EHBasicBlocks);
      };

  // Check blocks in EhBlocks have just EH related code.
  auto CheckEHBlocks = [this](SmallPtrSet<BasicBlock *, 16> &EhBlocks,
                              Argument *ThisObj) {
    for (auto *BB : EhBlocks)
      if (!checkEHBlock(BB, ThisObj))
        return false;
    return true;
  };

  std::function<bool(Function * F, Function * Callee, int32_t ArgNo,
                     SmallPtrSetImpl<Function *> & ProcessedFuncs)>
      CheckAllCallSitesInStructMethods;

  // Look at all callsites of "Callee" in "F" recursively and return true if 1
  // is passed as "ArgNo"-th argument at all the callsites.
  CheckAllCallSitesInStructMethods =
      [&CheckAllCallSitesInStructMethods](
          Function *F, Function *Callee, int32_t ArgNo,
          SmallPtrSetImpl<Function *> &ProcessedFunctions) {
        if (!F || F->isDeclaration())
          return true;
        if (!ProcessedFunctions.insert(F).second)
          return true;
        for (auto &I : instructions(F))
          if (auto *CB = dyn_cast<CallBase>(&I)) {
            auto *CalledF = dtrans::getCalledFunction(*CB);
            if (!CalledF)
              return false;
            if (CalledF == Callee) {
              auto *CI = dyn_cast<ConstantInt>(CB->getArgOperand(ArgNo));
              if (!CI || !CI->isOne())
                return false;
            } else if (!CheckAllCallSitesInStructMethods(CalledF, Callee, ArgNo,
                                                         ProcessedFunctions))
              return false;
          }
        return true;
      };

  // Returns true if "Val" is always 1 in "Callee" when "Callee" is called
  // from member functions of struct and array fields.
  // Ex:
  //  foo(..., %Val, ...) {
  //    ..
  //  }
  //
  //  call foo(..., 1, ...);
  //
  auto IsAlwaysOne =
      [this, &CheckAllCallSitesInStructMethods](Value *Val, Function *Callee) {
        Argument *A = dyn_cast<Argument>(Val);
        if (!A)
          return false;
        int32_t ArgNo = A->getArgNo();
        SmallPtrSet<Function *, 32> ProcessedFunctions;
        for (auto *StructF : SOACInfo->struct_functions())
          if (!CheckAllCallSitesInStructMethods(StructF, Callee, ArgNo,
                                                ProcessedFunctions))
            return false;
        return true;
      };

  // Wrapper for destructor to handle "delete" operator for objects of user
  // defined classes with virtual destructors.
  // Returns DestructorWrapper if it is just calling actual destructor and
  // then freeing up memory.
  //
  // Ex:
  //   DtorWrapper(Obj) {
  //     ActualDtor(Obj);
  //     free(Obj);
  //     May_Have_some_EH_code
  //   }
  //
  //   or
  //
  //   DtorWrapper(Obj) {
  //     ActualDtor(Obj);
  //     if (always_true_cond)
  //       free(Obj);
  //     May_Have_some_EH_code
  //   }
  //
  auto CheckDestructorWrapper =
      [this, &CheckEHBlocks,
       &IsAlwaysOne](Function *Fn, SmallPtrSet<BasicBlock *, 16> &EHBBSet) {
        // Clear all processed instructions first.
        Visited.clear();
        Argument *ThisObj = &*Fn->arg_begin();
        CallBase *FreeCall = nullptr;
        CallBase *DtorCall = nullptr;
        for (auto &BB : *Fn) {
          if (EHBBSet.count(&BB))
            continue;
          for (auto &I : BB) {
            if (isa<DbgInfoIntrinsic>(&I))
              continue;
            if (auto RI = dyn_cast<ReturnInst>(&I)) {
              if (!RI->getNumOperands())
                continue;
              // Makes sure it is returning "This" pointer.
              if (!checkCompleteObjPtr(RI->getReturnValue(), ThisObj))
                return UnKnown;
              Visited.insert(RI);
            }
            // We are interested in only Calls.
            auto *Call = dyn_cast<CallBase>(&I);
            if (!Call)
              continue;
            auto *Callee = Call->getCalledFunction();
            // Check if Call is
            //  1. Indirect call
            //  2. Direct recursive call
            //  3. Actual destructor call (Functionality will be proved
            //                             separately)
            //  4. Argument of the call is "ThisObj"
            if (Callee && Callee != Fn &&
                InitialFuncKind.find(Callee) != InitialFuncKind.end() &&
                InitialFuncKind[Callee] == Destructor &&
                Call->getArgOperand(0) == ThisObj) {
              // Expects DtorCall is set only once.
              if (DtorCall)
                return UnKnown;
              DtorCall = Call;
            } else if (dtrans::FreeCallInfo *FInfo = getFreeCall(&I)) {
              // Check if it is free instruction.
              BasicBlock *FreeBB;
              const Value *PtrArg = checkFree(FInfo, ThisObj, &FreeBB);
              // Check it is freeing complete object. Expects FreeCall
              // is set only once.
              if (!PtrArg || !checkCompleteObjPtr(PtrArg, ThisObj) || FreeCall)
                return UnKnown;
              assert(FreeBB && "Expected valid FreeBB");
              if (FreeBB->hasNPredecessorsOrMore(2))
                return UnKnown;
              BasicBlock *Pred = FreeBB->getSinglePredecessor();
              if (Pred) {
                // If  FreeBB has single predecessor, makes sure it is executed
                // always.
                Value *LVal = checkCondition(Pred, FreeBB);
                if (!LVal)
                  return UnKnown;
                if (!IsAlwaysOne(LVal, Fn))
                  return UnKnown;
              }
              FreeCall = Call;
            } else {
              return UnKnown;
            }
          }
        }
        if (!DtorCall || !FreeCall || !checkDominatorInfo(DtorCall, FreeCall))
          return UnKnown;
        if (!CheckEHBlocks(EHBBSet, ThisObj))
          return UnKnown;
        Visited.insert(DtorCall);
        if (!checkAllInstsProcessed(Fn, Visited))
          return UnKnown;
        return DestructorWrapper;
      };

  SmallPtrSet<BasicBlock *, 16> EHBasicBlocks;
  SmallPtrSet<StoreInst *, 1> SIList;
  SmallPtrSet<dtrans::FreeCallInfo *, 4> FreeList;

  // Collect EH related BBs.
  CollectEHBasicBlocks(Fn, EHBasicBlocks);

  // First, check if it is DestructorWrapper.
  if (CheckDestructorWrapper(Fn, EHBasicBlocks) == DestructorWrapper)
    return DestructorWrapper;

  // Clear all processed instructions first.
  Visited.clear();
  Argument *ThisObj = &*Fn->arg_begin();
  // Collect All store and free calls stmts in all BBs of Fn except
  // BBs in EHBasicBlocks.
  collectStoreInstsFreeCalls(Fn, EHBasicBlocks, SIList, FreeList);

  unsigned StoreCount = 0;
  for (auto *SI : SIList) {
    // Check this is a store to virtual function pointer.
    auto *GEP = dyn_cast<GetElementPtrInst>(SI->getPointerOperand());
    if (!GEP || !isAccessingVTableFieldInBaseClass(GEP, ThisObj))
      return UnKnown;
    Visited.insert(SI);
    StoreCount++;
  }
  // Expects at most one StoreInst that stores to virtual function pointer.
  if (StoreCount > 1)
    return UnKnown;

  unsigned AllElementsFreed = 0;
  unsigned ElementArrayFreed = 0;
  unsigned ObjectFreed = 0;
  Loop *L = nullptr;
  Instruction *FlagCheckInst = nullptr;
  BasicBlock *ArrayFreeBB = nullptr;
  Instruction *CompleteObjFreeInst = nullptr;
  LoopInfo LI((GetDT)(*Fn));

  // Now, try to analyze free calls.
  for (auto *FreeI : FreeList) {
    BasicBlock *FreeBB = nullptr;
    const Value *PtrArg = checkFree(FreeI, ThisObj, &FreeBB);
    if (!PtrArg)
      return UnKnown;
    if (checkCompleteObjPtr(PtrArg, ThisObj)) {
      // Free entire object (Optional)
      ObjectFreed++;
      CompleteObjFreeInst = FreeI->getInstruction();
    } else if (checkFieldOfArgClassLoad(PtrArg, ThisObj, ArrayField)) {
      // Free entire Array field (Mandatory)
      ElementArrayFreed++;
      ArrayFreeBB = FreeBB;
    } else {
      // Free all elements of array in loop. The entire loop is controlled
      // under flag field. (Optional)

      // Check pointer, which is passed to free call, is accessing
      // array element at AtLoc.
      Value *AtLoc = isArrayElementLoadAt(PtrArg, ThisObj, false);
      if (!AtLoc)
        return UnKnown;

      // Checks that AtLoc is loop counter that iterates from 0 to SizeField.
      L = checkLoop(AtLoc, ThisObj, LI);
      if (!L)
        return UnKnown;
      // Check if the loop has ZTT.
      if (!checkZTT(L, ThisObj))
        return UnKnown;
      BasicBlock *PH = L->getLoopPreheader();
      BasicBlock *PreCondBB = nullptr;
      // Get BB that has ZTT.
      if (PH)
        PreCondBB = PH->getSinglePredecessor();
      else
        PreCondBB = L->getLoopPredecessor();
      // Check if the entire loop controlled under the flag field.
      assert(PreCondBB && "Expected ZTT Basic Block");
      if (!checkBBControlledUnderFlagVal(PreCondBB, ThisObj))
        return UnKnown;
      BasicBlock *FlagCheckBB = PreCondBB->getSinglePredecessor();
      if (!FlagCheckBB)
        return UnKnown;
      FlagCheckInst = FlagCheckBB->getTerminator();
      AllElementsFreed++;
    }
  }

  if (ElementArrayFreed != 1 || AllElementsFreed > 1 || ObjectFreed > 1)
    return UnKnown;

  if (FlagCheckInst) {
    // Check that entire Array field is freed if flag field value is zero.
    if (cast<BranchInst>(FlagCheckInst)->getSuccessor(0) != ArrayFreeBB)
      return UnKnown;
    // Check that entire Array field is freed immediately after the loop
    // exited.
    BranchInst *BI = cast<BranchInst>(L->getLoopLatch()->getTerminator());
    if (BI->getSuccessor(1) != ArrayFreeBB)
      return UnKnown;
  }
  if (CompleteObjFreeInst) {
    // Make sure entire object is freed at the end.
    Instruction *NextInst = CompleteObjFreeInst->getNextNode();
    if (!NextInst || !isa<ReturnInst>(NextInst))
      return UnKnown;
  }

  if (!CheckEHBlocks(EHBasicBlocks, ThisObj))
    return UnKnown;

  if (!processAssumeCalls(Fn, ThisObj))
    return UnKnown;

  if (!checkAllInstsProcessed(Fn, Visited)) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO, {
      dbgs() << " Failed: Missing to process some instructions.\n";
    });
    return UnKnown;
  }
  return Destructor;
}

// Analyze Fn as CopyConstructor.
//
//  It expects mainly 3 things (All are mandatory).
//   1. Except array field, all fields of 2nd argument are copied
//      to the same field of "this" pointer (1st argument).
//   2. Allocate memory with "capacity" field of either 1st or 2nd
//      argument and assign it to the "array" field of "this" pointer.
//      memset that allocated array to null.
//   3. Copy all array element from 2nd argument to 1st argument.
//
// It checks for the following pattern.
//
// Ex:
//   CConst(this, A) {
//     this->flag = A.flag;
//     this->capacity = A.capacity;
//     this->size = A.size;
//     this->base = nullptr;
//     this->memint = A.memint;
//     base = malloc(capacity * sizeof_elem)
//     memset(base, 0, capacity * sizeof_elem);
//     for (i = 0; i < size; i++)
//       base[i] = A.base[i];
//   }
FunctionKind ClassInfo::recognizeCopyConstructor(Function *Fn) {

  // Returns true if SI is doing array element copy from SrcObj to
  // DstObj in a loop like below.
  //
  // Ex:
  //  Loop:
  //     %ind = phi i64 [ 0, %ph ], [ %indvars.iv.next, %Loop ]
  //     %arrayidx = getelementptr i32**, i32*** %SrcObj, i64 %ind
  //     %9 = bitcast i32*** %arrayidx to i64*
  //     %10 = load i64, i64* %9
  //     %arrayidx16 = getelementptr i32**, i32*** %DstObj, i64 %ind
  //     %11 = bitcast i32*** %arrayidx16 to i64*
  // SI: store i64 %10, i64* %11
  //     %ind = add i64 %ind, 1
  //     %exitcond = icmp eq i64 %ind, %Size_field
  //     br i1 %exitcond, label %cleanup, label %Loop
  //
  // If type of array element is struct, insert field index that is copied
  // into ElemIdxSet.
  //
  auto CheckArrayCopyFromSrcObjToDstObj = [this](StoreInst *SI, Value *SrcObj,
                                                 Value *DstObj, LoopInfo &LI,
                                                 SmallSet<int, 2> &ElemIdxSet) {
    DTransType *ElemTy = getElementTy();
    Value *Loc;
    Value *PtrOp = SI->getPointerOperand();
    Value *ValOp = SI->getValueOperand();
    int32_t ElemIdx1;
    int32_t ElemIdx2;
    if (isa<DTransStructType>(ElemTy)) {
      // %49 = getelementptr %"E", %"E"* %Array_SrcObj, i64 %Loc1
      // %51 = getelementptr %"E", %"E"* %49, i64 0, i32 1
      // %52 = bitcast %"D"** %51 to i64*
      // %54 = load i64, i64* %52
      Value *Loc1 =
          isArrayStructElementLoadAt(ValOp, ElemTy, SrcObj, &ElemIdx1, false);
      if (!Loc1)
        return false;
      // %56 = getelementptr %"E", %"E"* %Array_DstObj, i64 %Loc2
      // %58 = getelementptr %"E", %"E"* %56, i64 0, i32 1
      // %59 = bitcast %"D"** %58 to i64*
      Value *Loc2 =
          isArrayElementAddressAt(PtrOp, ElemTy, DstObj, &ElemIdx2, false);
      if (Loc1 != Loc2 || ElemIdx1 != ElemIdx2)
        return false;
      Loc = Loc1;
      ElemIdxSet.insert(ElemIdx1);
    } else {
      // %arrayidx = getelementptr i32**, i32*** %SrcObj, i64 %ind
      // %9 = bitcast i32*** %arrayidx to i64*
      // %ValOp = load i64, i64* %9
      Value *Loc1 = isArrayElementLoadAt(ValOp, SrcObj, false);
      if (!Loc1)
        return false;

      // %arrayidx16 = getelementptr i32**, i32*** %DstObj, i64 %ind
      // %PtrOp = bitcast i32*** %arrayidx16 to i64*
      Value *Loc2 = isArrayElementAt(PtrOp, DstObj, false);
      if (Loc1 != Loc2)
        return false;
      Loc = Loc1;
    }
    // Allow size field of either DstObj or SrcObj as loop trip count.
    if (!checkLoopWithZTT(Loc, DstObj, LI) &&
        !checkLoopWithZTT(Loc, SrcObj, LI))
      return false;
    Visited.insert(SI);
    return true;
  };

  if (DTransAnnotator::hasDTransSOAToAOSPrepareTypeAnnotation(*Fn))
    return CopyConstructor;

  SmallPtrSet<StoreInst *, 8> SIList;
  ReturnInst *RetI = nullptr;
  Argument *ThisObj = Fn->arg_begin();
  Argument *SrcObj = Fn->arg_begin() + 1;
  // Collect all store instructions.
  for (auto &I : instructions(Fn))
    if (auto *SI = dyn_cast<StoreInst>(&I)) {
      SIList.insert(SI);
    } else if (auto RI = dyn_cast<ReturnInst>(&I)) {
      if (RetI)
        return UnKnown;
      RetI = RI;
    }

  int32_t MemIntAssign = 0;
  int32_t FlagFieldAssign = 0;
  int32_t SizeFieldAssign = 0;
  int32_t CapacityFieldAssign = 0;
  StoreInst *NullPtrStore = nullptr;
  StoreInst *AllocArrayStore = nullptr;
  SmallPtrSet<Value *, 4> ArrayPtrAliases;
  Visited.clear();

  if (!RetI)
    return UnKnown;
  // Makes sure it is returning "This" pointer.
  if (RetI->getNumOperands())
    if (!checkCompleteObjPtr(RetI->getReturnValue(), ThisObj))
      return UnKnown;
  Visited.insert(RetI);

  // Process all stores except array element stores.
  for (auto *SI : SIList) {
    const Value *PtrOp = skipCasts(SI->getPointerOperand());
    Value *ValOp = SI->getValueOperand();
    auto *GEP = dyn_cast<GetElementPtrInst>(PtrOp);
    int32_t Idx;
    if (!GEP || !isAccessingFieldOfArgClass(GEP, ThisObj, &Idx))
      continue;
    if (Idx != ArrayField) {
      // Ex:
      // %flag = getelementptr %A, %A* %this, i64 0, i32 0
      // %flag2 = getelementptr %A, %A* %A, i64 0, i32 0
      // %0 = load i8, i8* %flag2
      // store i8 %0, i8* %flag
      if (!checkFieldOfArgClassLoad(ValOp, SrcObj, Idx))
        return UnKnown;
      if (Idx == FlagField)
        FlagFieldAssign++;
      else if (Idx == SizeField)
        SizeFieldAssign++;
      else if (Idx == CapacityField)
        CapacityFieldAssign++;
      else if (Idx == MemIntField)
        MemIntAssign++;
      else
        return UnKnown;
    } else {
      if (isa<Constant>(ValOp) && cast<Constant>(ValOp)->isNullValue()) {
        // nullptr assignment to array pointer field.
        if (NullPtrStore)
          return UnKnown;
        NullPtrStore = SI;
      } else if (checkAllocatedArrayPtr(ValOp, SrcObj, ArrayPtrAliases)) {
        // Ex:
        //  %call = tail call noalias i8* @malloc(i64 %mul)
        //  store i8* %call, i8** %array_addr
        //  call @memset(i8* align 8 %call, i8 0, i64 %mul, i1 false)
        //
        //  Allow size of the array can either be capacity field of
        //  1st or 2nd argument.
        if (!checkMemset(ArrayPtrAliases, ThisObj) &&
            !checkMemset(ArrayPtrAliases, SrcObj))
          return UnKnown;
        if (AllocArrayStore)
          return UnKnown;
        AllocArrayStore = SI;
      } else {
        return UnKnown;
      }
    }
    Visited.insert(SI);
  }

  // Make sure all fields of vector are assigned once.
  if (MemIntAssign != 1 || FlagFieldAssign != 1 || CapacityFieldAssign != 1 ||
      SizeFieldAssign != 1 || !AllocArrayStore)
    return UnKnown;
  // Make sure actual NullPtrStore dominates allocation.
  if (NullPtrStore && !checkDominatorInfo(NullPtrStore, AllocArrayStore))
    return UnKnown;

  // Set of fields of array struct elements that are copied.
  SmallSet<int, 2> ElemIdxSet;
  LoopInfo LI((GetDT)(*Fn));
  unsigned ElemStoreCount = 0;
  // This loop handles array element stores.
  for (auto *SI : SIList) {
    if (Visited.count(SI))
      continue;
    if (!CheckArrayCopyFromSrcObjToDstObj(SI, SrcObj, ThisObj, LI, ElemIdxSet))
      return UnKnown;
    if (!checkDominatorInfo(AllocArrayStore, SI))
      return UnKnown;
    ElemStoreCount++;
  }
  DTransType *ElemTy = getElementTy();
  if (isa<DTransStructType>(ElemTy)) {
    // Check all fields of array struct element are copied.
    unsigned NumFields = cast<DTransStructType>(ElemTy)->getNumFields();
    if (ElemStoreCount != NumFields || ElemIdxSet.size() != NumFields)
      return UnKnown;
  } else {
    if (ElemStoreCount != 1)
      return UnKnown;
  }

  // In case of CopyConstructor, make sure Vtable is accessed from
  // "SrcObj".
  if (!processAssumeCalls(Fn, SrcObj))
    return UnKnown;

  if (!checkAllInstsProcessed(Fn, Visited)) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO, {
      dbgs() << " Failed: Missing to process some instructions.\n";
    });
    return UnKnown;
  }
  return CopyConstructor;
}

// Analyze Fn as AppendElem.
//
//  It expects mainly 3 things (All are mandatory).
//   1. Call to Resize.
//   2. Store array elements that are passed as arguments at size
//      location.
//   3. Increment size field by 1.
//
// It checks for the following pattern.
//
// Ex:
//  append(this, elem)
//  {
//   Resize();
//   this->array[this->size] = elem;
//   this->size++;
//  }
//
FunctionKind ClassInfo::recognizeAppendElem(Function *Fn) {
  // Returns true if V is address of SizeField.
  auto IsSizeFieldAddress = [this](Value *V, Value *ThisObj) {
    auto *GEP = dyn_cast<GetElementPtrInst>(V);
    int32_t Idx;
    if (!GEP || !isAccessingFieldOfArgClass(GEP, ThisObj, &Idx) ||
        Idx != SizeField)
      return false;
    return true;
  };

  SmallPtrSet<StoreInst *, 4> SIList;
  CallInst *Call = nullptr;
  Argument *ThisObj = Fn->arg_begin();
  StoreInst *SizeIncSI = nullptr;
  SmallPtrSet<StoreInst *, 4> ArrayElemStores;
  // Set of arguments that are stored as array elements.
  SmallPtrSet<Value *, 2> ArgValueSet;
  // If type of array element is struct, each field of the struct is
  // treated as independent element. This set indicates locations that
  // are appended.
  SmallSet<int, 2> ElemIdxSet;
  DTransType *ElemTy = getElementTy();
  Visited.clear();
  for (auto &I : instructions(Fn)) {
    if (isa<DbgInfoIntrinsic>(&I))
      continue;
    // Collect Resize call here.
    if (auto *CI = dyn_cast<CallInst>(&I)) {
      // Allowed only one call.
      if (Call)
        return UnKnown;
      Call = CI;
      continue;
    }
    auto *SI = dyn_cast<StoreInst>(&I);
    if (!SI)
      continue;
    Value *ValOp = SI->getValueOperand();
    Value *PtrOp = SI->getPointerOperand();
    if (IsSizeFieldAddress(PtrOp, ThisObj)) {
      Value *Op1;
      Value *Inc;
      // Check if this store represents  "size++" instruction
      // Ex:
      //   %size = getelementptr %A, %A* %this, i64 0, i32 2
      //   %1 = load i32, i32* %size
      //   %inc = add i32 %1, 1
      //   store i32 %inc, i32* %size
      if (!match(ValOp, m_Add(m_Value(Op1), m_Value(Inc))) ||
          !match(Inc, m_One()) ||
          !checkFieldOfArgClassLoad(Op1, ThisObj, SizeField) || SizeIncSI)
        return UnKnown;
      SizeIncSI = SI;
      Visited.insert(cast<Instruction>(ValOp));
    } else {
      // Check if this store represents "array[size] = elem;" instruction.
      Value *SetAtLoc;
      if (isa<DTransStructType>(ElemTy)) {
        int ElemIdx;
        // Ex:
        //   %valLoad = load %val
        //   %0 = load %array_addr
        //   %1 = load %size_addr
        //   %arrayidx = getelementptr %"E", %"E"* %0, i64 %1
        //   %elem = getelementptr  %"E", %"E"* %arrayidx, i64 0, i32 1
        //   %bc = bitcast %"D"** %elem to i64*
        //   store i64 %valLoad, i64* %bc
        SetAtLoc =
            isArrayElementAddressAt(PtrOp, ElemTy, ThisObj, &ElemIdx, false);
        if (!SetAtLoc)
          return UnKnown;
        Value *ArgVal = isLoadOfArg(ValOp);
        if (!ArgVal)
          return UnKnown;
        // Collect arguments that are being stored.
        ArgValueSet.insert(ArgVal);
        // Collect all fields of array struct element that are stored.
        ElemIdxSet.insert(ElemIdx);
      } else {
        // Ex:
        //   %0 = load float***, float**** %array_addr
        //   %1 = load i32, i32* %size_addr
        //   %arrayidx = getelementptr float**, float*** %0, i64 %1
        //   store float** %val, float*** %arrayidx, align 8, !tbaa !25
        SetAtLoc = isArrayElementAt(PtrOp, ThisObj, false);
        if (!SetAtLoc)
          return UnKnown;
        Value *ArgVal = isValidArgumentSave(ValOp);
        if (!ArgVal)
          return UnKnown;
        ArgValueSet.insert(ArgVal);
      }
      if (!checkFieldOfArgClassLoad(SetAtLoc, ThisObj, SizeField))
        return UnKnown;
      ArrayElemStores.insert(SI);
    }
    Visited.insert(SI);
  }
  if (!Call || !SizeIncSI)
    return UnKnown;
  if (isa<DTransStructType>(ElemTy)) {
    unsigned ElemCount = cast<DTransStructType>(ElemTy)->getNumFields();
    // Makes sure all fields of array struct element are stored with
    // different argument values.
    if (ArrayElemStores.size() != ElemCount ||
        ArgValueSet.size() != ElemCount || ElemIdxSet.size() != ElemCount)
      return UnKnown;
  } else {
    // It should be only one element if array element type is not struct.
    if (ArrayElemStores.size() != 1)
      return UnKnown;
  }
  // Make sure Call dominates all instructions.
  for (auto *I : Visited)
    if (!checkDominatorInfo(Call, const_cast<Instruction *>(I)))
      return UnKnown;
  // Make sure "size++" is dominated by all element store instructions.
  for (auto *I : ArrayElemStores)
    if (!checkDominatorInfo(I, SizeIncSI))
      return UnKnown;
  // Check Call is categorized as Resize. No need to recognize
  // functionality of Resize here.
  auto *Callee = Call->getCalledFunction();
  if (!Callee || InitialFuncKind.find(Callee) == InitialFuncKind.end() ||
      InitialFuncKind[Callee] != Resize)
    return UnKnown;
  Visited.insert(Call);

  if (!checkAllInstsProcessed(Fn, Visited)) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO, {
      dbgs() << " Failed: Missing to process some instructions.\n";
    });
    return UnKnown;
  }
  return AppendElem;
}

// Returns true if BB is controlled like below
//
//       if (AtLoc > Capacity)
//           BB
//       else
//           END_BasicBlock_with_No_IR
//
bool ClassInfo::isControlledUnderCapacityField(BasicBlock *BB, Value *ThisObj,
                                               Value *AtLoc) {
  Value *LValue;
  Value *RValue;
  ICmpInst::Predicate Predi;
  BasicBlock *FBlock = getBBControlledOps(BB, &LValue, &RValue, &Predi);
  if (!FBlock || Predi != ICmpInst::ICMP_UGT)
    return false;
  if (LValue != AtLoc ||
      !checkFieldOfArgClassLoad(RValue, ThisObj, CapacityField))
    return false;

  // Make sure false block has just return.
  Instruction *I = FBlock->getFirstNonPHIOrDbg();
  if (!I || !isa<ReturnInst>(I))
    return false;

  return true;
}

// Returns true if Val matches the expected patterns.
//
// Pattern 1:
//      unsigned int newMax = size + inc;
//      if (newMax < capacity + capacity/2)
//         newMax = capacity + capacity/2;
//
//
// Pattern 2:
//        unsigned int newMax = size + inc;
//        unsigned int minNewMax = (unsigned int)((double)size * 1.25);
//        if (newMax < minNewMax)
//            newMax = minNewMax;
//
// Ex:
//       %0 = load i32, i32* %size
//       %add = add i32 %0, 1
//       %capacity = getelementptr %A, %A* %this, i64 0, i32 1
//       %1 = load i32, i32* %capacity
//       %cmp = icmp ugt i32 %add, %1
//       br i1 %cmp, label %NextBB, label %EndBB
//
//      NextBB:
//       %div = lshr i32 %1, 1
//       %add4 = add i32 %div, %1
//       %cmp5 = icmp ult i32 %add, %add4
//  Val: %spec.select = select i1 %cmp5, i32 %add4, i32 %add
//
//
bool ClassInfo::checkCapacityIncrementPattern(Value *Val, Argument *ThisObj) {

  // Returns %1 in the below example pattern.
  //  Ex:
  //             %1 = load i32, i32* %capacity_ThisObj
  //             %div = lshr i32 %1, 1
  // *MemGrowth: %add4 = add i32 %div, %1
  auto AllowedMemGrowthPatternOne = [this](Value *MemGrowth,
                                           Argument *ThisObj) -> Value * {
    Value *Op1;
    Value *Op2;
    Value *Inc;
    if (!match(MemGrowth, m_Add(m_Value(Op1), m_Value(Op2))))
      return nullptr;
    if (match(Op1, m_LShr(m_Specific(Op2), m_Value(Inc))) &&
        match(Inc, m_One()) &&
        checkFieldOfArgClassLoad(Op2, ThisObj, CapacityField)) {
      Visited.insert(cast<Instruction>(MemGrowth));
      Visited.insert(cast<Instruction>(Op1));
      return Op2;
    }
    if (match(Op2, m_LShr(m_Specific(Op1), m_Value(Inc))) &&
        match(Inc, m_One()) &&
        checkFieldOfArgClassLoad(Op1, ThisObj, CapacityField)) {
      Visited.insert(cast<Instruction>(MemGrowth));
      Visited.insert(cast<Instruction>(Op2));
      return Op1;
    }
    return nullptr;
  };

  // Returns %0 in the below example pattern.
  // Ex:
  //            %0 = load i32, i32* %size_ThisObj
  //            %conv = uitofp i32 %0 to double
  //            %mul = fmul double %conv, 1.250000e+00
  // MemGrowth: %conv3 = fptoui double %mul to i32
  //
  auto AllowedMemGrowthPatternTwo = [this](Value *MemGrowth,
                                           Argument *ThisObj) -> Value * {
    auto *FToU = dyn_cast<FPToUIInst>(MemGrowth);
    if (!FToU)
      return nullptr;
    Value *FOp = FToU->getOperand(0);
    Value *Op1;
    if (!match(FOp, m_FMul(m_Value(Op1), m_SpecificFP(1.25))))
      return nullptr;
    auto *UToF = dyn_cast<UIToFPInst>(Op1);
    if (!UToF)
      return nullptr;
    if (!checkFieldOfArgClassLoad(UToF->getOperand(0), ThisObj, SizeField))
      return nullptr;
    Visited.insert(FToU);
    Visited.insert(cast<Instruction>(FOp));
    Visited.insert(UToF);
    return UToF->getOperand(0);
  };

  //       %cmp5 = icmp ult i32 %add, %add4
  //  Val: %spec.select = select i1 %cmp5, i32 %add4, i32 %add
  //
  //  or
  //
  //  Val: %spec.select = i32 call intrin.umax(i32 %add4, i32 %add)
  //
  auto CheckUMax = [](Value *Val, Value **LHS, Value **RHS, Value **Cond) {
    if (match(Val, m_Intrinsic<Intrinsic::umax>(m_Value(*LHS), m_Value(*RHS))))
      return true;

    ICmpInst::Predicate Pred;
    if (match(Val, m_Select(m_Value(*Cond), m_Value(*LHS), m_Value(*RHS))) &&
        match(*Cond, m_ICmp(Pred, m_Specific(*RHS), m_Specific(*LHS))) &&
        Pred == ICmpInst::ICMP_ULT)
      return true;
    return false;
  };

  // Check patterns of RHS and LHS:
  //
  //      %0 = load i32, i32* %size
  // RHS: %add = add i32 %0, 1
  //
  // LHS: Check if memory growth is in expected pattern.
  //
  auto CheckUMaxInputs =
      [this, &AllowedMemGrowthPatternOne, AllowedMemGrowthPatternTwo](
          Instruction *I, Value *LHS, Value *RHS, Argument *ThisObj) {
        Value *Inc;
        Value *CurSize;
        //      %0 = load i32, i32* %size
        // RHS: %add = add i32 %0, 1
        if (!match(RHS, m_Add(m_Value(CurSize), m_Value(Inc))) ||
            !match(Inc, m_One()) ||
            !checkFieldOfArgClassLoad(CurSize, ThisObj, SizeField))
          return false;

        // Check if memory growth is in expected pattern.
        Value *Ptr = AllowedMemGrowthPatternOne(LHS, ThisObj);
        if (!Ptr) {
          Ptr = AllowedMemGrowthPatternTwo(LHS, ThisObj);
          if (!Ptr)
            return false;
        }
        if (!isControlledUnderCapacityField(I->getParent(), ThisObj, RHS))
          return false;
        Visited.insert(cast<Instruction>(CurSize));
        Visited.insert(cast<Instruction>(RHS));
        return true;
      };

  Value *LHS = nullptr;
  Value *RHS = nullptr;
  Value *Cond = nullptr;

  // Check if "Val" is computing UMax.
  if (!CheckUMax(Val, &LHS, &RHS, &Cond))
    return false;

  auto *SelInst = cast<Instruction>(Val);

  // Check LHS and RHS:
  //       %cmp5 = icmp ult i32 RHS, LHS
  //  Val: %spec.select = select i1 %cmp5, i32 LHS, i32 RHS
  //
  //  or
  //
  //  Val: %spec.select = i32 call intrin.umax(i32 LHS, i32 RHS)
  //
  //  or
  //
  //  Val: %spec.select = i32 call intrin.umax(i32 RHS, i32 LHS)
  //
  if (!CheckUMaxInputs(SelInst, LHS, RHS, ThisObj) &&
      (Cond || !CheckUMaxInputs(SelInst, RHS, LHS, ThisObj)))
    return false;

  Visited.insert(SelInst);
  if (Cond)
    Visited.insert(cast<Instruction>(Cond));
  return true;
}

// Analyze Fn as Resize.
//
//  It expects mainly the below things (All are mandatory).
//   1. Stores:
//      a. Set capacity field with new capacity value.
//      b. Set array field with newly allocated memory.
//      c. Copy elements from array to newly allocated memory in loop.
//   2. Free: Free array before assigning newly allocated memory
//
// It checks for the following pattern.
//
// Ex:
//    resize() {
//      if (size + 1 < capacity)
//        return;
//      newMax = Use some formula to get new capacity.
//      newList = malloc(newMax * sizeof(Elem));
//      for (unsigned int index = 0; index < size; index++)
//        newList[index] = array[index];
//      for (; index < newMax; index++)
//        newList[index] = 0;
//
//      free(array);
//      array = newList;
//      capacity = newMax;
//    }
FunctionKind ClassInfo::recognizeResize(Function *Fn) {

  // Returns the Loop if SI is copying all elements from array field of ThisObj
  // to AllocPtr location using a Loop.
  // Ex:
  //  i = 0;
  //  Loop:
  //    AllocPtr[i] = ThisObj->array[i];
  //    i++;
  //    if (i < ThisObj->size) goto Loop;
  auto IsArrayToAllocatedMemoryCopy =
      [this](StoreInst *SI, Value *ThisObj, Value *AllocPtr, LoopInfo &LI,
             SmallSet<int, 2> &ElemIdxSet) -> Loop * {
    DTransType *ElemTy = getElementTy();
    Value *Loc;
    Value *PtrOp = SI->getPointerOperand();
    Value *ValOp = SI->getValueOperand();
    if (isa<DTransStructType>(ElemTy)) {
      int32_t ElemIdx1;
      int32_t ElemIdx2;
      // %49 = getelementptr %"E", %"E"* %Array_ThisObj, i64 %Loc1
      // %51 = getelementptr %"E", %"E"* %49, i64 0, i32 1
      // %52 = bitcast %"D"** %51 to i64*
      // %54 = load i64, i64* %52
      Value *Loc1 =
          isArrayStructElementLoadAt(ValOp, ElemTy, ThisObj, &ElemIdx1, false);
      if (!Loc1)
        return nullptr;
      // %70 = getelementptr %"E", %"E"* %AllocPtr, i64 %Loc2
      // %72 = getelementptr %"E", %"E"* %70, i64 0, i32 1
      // %73 = bitcast %"D"** %72 to i64*
      Value *Loc2 =
          isStructElementIndexAddress(PtrOp, ElemTy, AllocPtr, &ElemIdx2);
      if (Loc1 != Loc2 || ElemIdx1 != ElemIdx2)
        return nullptr;
      Loc = Loc1;
      ElemIdxSet.insert(ElemIdx1);
    } else {
      // %arrayidx = getelementptr i32**, i32*** %Array_ThisObj, i64 %Loc1
      // %6 = bitcast i32*** %arrayidx to i64*
      // %7 = load i64, i64* %6
      Value *Loc1 = isArrayElementLoadAt(ValOp, ThisObj, false);
      if (!Loc1)
        return nullptr;
      // %arrayidx = getelementptr i32**, i32*** %AllocPtr, i64 %Loc2
      // %8 = bitcast i32*** %arrayidx12 to i64*
      Value *Loc2 = isElementIndexAddress(PtrOp, AllocPtr);
      if (Loc1 != Loc2)
        return nullptr;
      Loc = Loc1;
    }
    return checkLoopWithZTT(Loc, ThisObj, LI);
  };

  // Returns true if Phi is index value of L after exiting the
  // loop.
  //
  //     Pre-Loop:
  //
  //     Loop:
  //       ...
  //       %60 = add nuw nsw i64 %54, 1
  //       %61 = icmp ult i64 %60, %38
  //       br i1 %61, label %53, label %39
  //
  //     Loop_Exit:
  //       %40 = trunc i64 %60 to i32
  //
  //     BLK:
  // Phi:  %42 = phi i32 [ 0, %Pre-Loop ], [ %40, %Loop_Exit ]
  //
  auto IsValueOfIndexCountAfterLoopExit = [this](PHINode *Phi, Loop *L) {
    if (!Phi || Phi->getNumIncomingValues() != 2)
      return false;
    BasicBlock *Pred0 = Phi->getIncomingBlock(0);
    BasicBlock *Pred1 = Phi->getIncomingBlock(1);
    BasicBlock *PH = L->getLoopPreheader();
    BasicBlock *LPreHead = nullptr;
    if (PH)
      LPreHead = PH->getSinglePredecessor();
    else
      LPreHead = L->getLoopPredecessor();
    // Make sure incoming blocks of Phi are ZTT's block of the Loop and
    // exit block of the loop.
    if (Pred0 != LPreHead)
      return false;
    if (L->contains(Pred1) && !L->isLoopExiting(Pred1))
      return false;

    // Make sure one incoming value is zero.
    ConstantInt *Init = dyn_cast<ConstantInt>(Phi->getIncomingValue(0));
    if (!Init || !Init->isZero())
      return false;
    // Check that other incoming value is index value of the loop after
    // exiting the loop.
    BranchInst *BI = cast<BranchInst>(L->getLoopLatch()->getTerminator());
    ICmpInst *Cond = cast<ICmpInst>(BI->getCondition());
    Value *IncInd = Phi->getIncomingValue(1);
    // Okay to ignore Trunc instruction as IncInd is induction variable
    // of Loop which iterates from zero to size, which is i32.
    if (isa<TruncInst>(IncInd)) {
      auto *I = cast<Instruction>(IncInd);
      Visited.insert(cast<Instruction>(I));
      IncInd = I->getOperand(0);
    }
    if (Cond->getOperand(0) != IncInd)
      return false;
    Visited.insert(Phi);
    return true;
  };

  // Check the size of memset as "newMax - index + 1"
  // XorI:          %48 = xor i32 %SVal, -1
  // RemainSize:    %49 = add i32 %NewCap, %48
  //                %50 = zext i32 %49 to i64
  //                %51 = shl nuw nsw i64 %50, 3
  // MemsetSize:    %52 = add nuw nsw i64 %51, 8
  //
  // or
  //
  // XorI:          %48 = xor i32 %SVal, -1
  // RemainSize:    %49 = add i32 %NewCap, %48
  //                %50 = zext i32 %49 to i64
  //                %51 = add nuw nsw i64 %50, 1
  // MemsetSize:    %52 = shl nuw nsw i64 %51, 3
  //
  auto AllowedMemsetSizePatternOne = [this](Value *MemsetSize, Value *NewCap,
                                            Value *SVal) -> bool {
    const APInt *AddC;
    Value *ShlI = MemsetSize;
    Value *AddOp;
    bool PtrIncremented = false;
    unsigned ElemSize = getElemTySize();
    if (match(MemsetSize, m_Add(m_Value(AddOp), m_APInt(AddC))) &&
        *AddC == ElemSize) {
      PtrIncremented = true;
      ShlI = AddOp;
    }
    int64_t Multiplier = 1;
    const Value *RemainSize = computeMultiplier(ShlI, &Multiplier);
    if (!RemainSize || Multiplier != ElemSize)
      return false;
    if (!PtrIncremented) {
      Instruction *ZExtI;
      Value *RVal;
      if (!match(RemainSize, m_Add(m_Instruction(ZExtI), m_APInt(AddC))) ||
          *AddC != 1 || !match(ZExtI, m_ZExt(m_Value(RVal))))
        return false;
      Visited.insert(cast<Instruction>(RemainSize));
      Visited.insert(cast<Instruction>(ZExtI));
      RemainSize = RVal;
    }
    Instruction *XorI;
    if (!match(RemainSize, m_Add(m_Specific(NewCap), m_Instruction(XorI))))
      return false;
    if (!match(XorI, m_Xor(m_Specific(SVal), m_AllOnes())))
      return false;
    Visited.insert(cast<Instruction>(XorI));
    Visited.insert(cast<Instruction>(RemainSize));
    return true;
  };

  // Check the size of memset as "NewMax - OriginalSize":
  //             %16 = shl i32 %NewCap, 2
  //             %41 = shl i32 %SVal, 2
  // MemsetSize: %43 = sub i32 %16, %41
  //
  // or
  //
  //             %42 = sub i32 %NewCap, %SVal
  // MemsetSize: %43 = shl i32 %42, 2
  //
  auto AllowedMemsetSizePatternTwo = [this](Value *MemsetSize, Value *NewCap,
                                            Value *SVal) -> bool {
    Value *SubLeft, *SubRight;
    unsigned ElemSize = getElemTySize();
    int64_t Multiplier = 1;
    if (match(MemsetSize, m_Sub(m_Value(SubLeft), m_Value(SubRight)))) {
      const Value *OriginalSize = computeMultiplier(SubRight, &Multiplier);
      if (!OriginalSize || Multiplier != ElemSize || OriginalSize != SVal)
        return false;
      Multiplier = 1;
      const Value *IncreasedSize = computeMultiplier(SubLeft, &Multiplier);
      if (!IncreasedSize || Multiplier != ElemSize || IncreasedSize != NewCap)
        return false;
      Visited.insert(cast<Instruction>(SubLeft));
      Visited.insert(cast<Instruction>(SubRight));
      return true;
    } else {
      Multiplier = 1;
      const Value *SubSize = computeMultiplier(MemsetSize, &Multiplier);
      if (SubSize && Multiplier == ElemSize &&
          match(SubSize, m_Sub(m_Value(SubLeft), m_Value(SubRight))) &&
          SubLeft == NewCap && SubRight == SVal) {
        Visited.insert(cast<Instruction>(SubSize));
        return true;
      }
    }
    return false;
  };

  Visited.clear();
  Argument *ThisObj = &*Fn->arg_begin();
  SmallPtrSet<StoreInst *, 4> SIList;
  SmallPtrSet<dtrans::FreeCallInfo *, 1> FreeList;
  SmallPtrSet<BasicBlock *, 16> IgnoreBBSet;
  FunctionKind FKind = UnKnown;
  LoopInfo LI((GetDT)(*Fn));

  // Collect Stores and Free call.
  collectStoreInstsFreeCalls(Fn, IgnoreBBSet, SIList, FreeList);

  // Check Free Call.
  BasicBlock *FreeBB = nullptr;
  if (FreeList.size() != 1)
    return FKind;
  // Check delete array.
  const Value *FreeArg = checkFree(*FreeList.begin(), ThisObj, &FreeBB);
  if (!FreeArg || !checkFieldOfArgClassLoad(FreeArg, ThisObj, ArrayField))
    return FKind;

  // Process assignment to capacity field.
  unsigned CapacityFieldAssigns = 0;
  StoreInst *CapacityFieldStore = nullptr;
  for (auto *SI : SIList) {
    const Value *PtrOp = skipCasts(SI->getPointerOperand());
    auto *GEP = dyn_cast<GetElementPtrInst>(PtrOp);
    int32_t Idx;
    if (!GEP || !isAccessingFieldOfArgClass(GEP, ThisObj, &Idx) ||
        Idx != CapacityField)
      continue;
    if (!checkCapacityIncrementPattern(SI->getValueOperand(), ThisObj))
      return FKind;
    CapacityFieldAssigns++;
    CapacityFieldStore = SI;
    Visited.insert(CapacityFieldStore);
  }
  if (CapacityFieldAssigns != 1 || !CapacityFieldStore)
    return FKind;

  // Process assignment to array field.
  SmallPtrSet<Value *, 4> ArrayPtrAliases;
  unsigned ArrayFieldAssigns = 0;
  StoreInst *ArrayFieldStore = nullptr;
  Value *NewSize = CapacityFieldStore->getValueOperand();
  for (auto *SI : SIList) {
    const Value *PtrOp = skipCasts(SI->getPointerOperand());
    auto *GEP = dyn_cast<GetElementPtrInst>(PtrOp);
    int32_t Idx;
    if (!GEP || !isAccessingFieldOfArgClass(GEP, ThisObj, &Idx) ||
        Idx != ArrayField)
      continue;
    Value *ValOp = SI->getValueOperand();
    // Check memory is allocated properly.
    if (!checkAllocatedArrayPtr(ValOp, ThisObj, ArrayPtrAliases, NewSize))
      return FKind;
    ArrayPtrAliases.insert(ValOp);
    if (auto *BC = dyn_cast<BitCastInst>(ValOp)) {
      ValOp = BC->getOperand(0);
      ArrayPtrAliases.insert(ValOp);
    }
    ArrayFieldStore = SI;
    ArrayFieldAssigns++;
    Visited.insert(ArrayFieldStore);
  }
  if (ArrayFieldAssigns != 1 || !ArrayFieldStore)
    return FKind;

  // Check elements are copied from array to newly allocated memory in loop.
  Loop *CopyLoop = nullptr;
  Value *AllocPtr = ArrayFieldStore->getValueOperand();
  unsigned NumArrayElemStores = 0;
  SmallSet<int, 2> ElemIdxSet;
  for (auto *SI : SIList) {
    if (Visited.count(SI))
      continue;
    Loop *L =
        IsArrayToAllocatedMemoryCopy(SI, ThisObj, AllocPtr, LI, ElemIdxSet);
    if (!L)
      return UnKnown;
    if (!CopyLoop)
      CopyLoop = L;
    else if (CopyLoop != L)
      return UnKnown;
    Visited.insert(SI);
    NumArrayElemStores++;
  }
  DTransType *ElemTy = getElementTy();
  if (isa<DTransStructType>(ElemTy)) {
    unsigned ElemCount = cast<DTransStructType>(ElemTy)->getNumFields();
    // Makes sure all fields of array struct element are stored with
    // different argument values.
    if (NumArrayElemStores != ElemCount || ElemIdxSet.size() != ElemCount)
      return UnKnown;
  } else {
    // It should be only one element if array element type is not struct.
    if (NumArrayElemStores != 1)
      return UnKnown;
  }
  assert(CopyLoop && "Expected valid loop");

  // The remainder loop in above example source code is
  //  converted to memset like below in IR.
  //
  // Source code:
  //   for (; index < newMax; index++)
  //     base[index] = 0;
  //
  // IR:
  //   memset(base + index, 0, newMax - index + 1)
  //
  // Ex:
  //     %13 is NewSize
  //     %30 is pointer to newly allocated memory.
  //
  //     Pre-Loop:
  //
  //     Loop:
  //       ...
  //       %60 = add nuw nsw i64 %54, 1
  //       %61 = icmp ult i64 %60, %38
  //       br i1 %61, label %53, label %39
  //
  //     Loop_Exit:
  //       %40 = trunc i64 %60 to i32
  //
  //     BLK:
  //       %42 = phi i32 [ 0, %Pre-Loop ], [ %40, %Loop_Exit ]
  //       %43 = icmp ult i32 %42, %13
  //       br i1 %43, label %MEM_BLK, label %SOMEOTHER
  //
  //     MEM_BLK:
  //       %45 = zext i32 %42 to i64
  //       %46 = shl nuw nsw i64 %45, 3
  //       %47 = getelementptr i8, i8* %30, i64 %46
  //       %48 = xor i32 %42, -1
  //       %49 = add i32 %13, %48
  //       %50 = zext i32 %49 to i64
  //       %51 = shl nuw nsw i64 %50, 3
  //       %52 = add nuw nsw i64 %51, 8
  //       memset.p0i8.i64(i8* align 8 %47, i8 0, i64 %52, i1 false)

  MemSetInst *Memset = nullptr;

  for (auto *APtr : ArrayPtrAliases)
    for (auto *U : APtr->users()) {
      auto *GEP = dyn_cast<GetElementPtrInst>(U);
      if (!GEP || GEP->getNumIndices() != 1)
        continue;
      for (auto *UU : GEP->users()) {
        MemSetInst *II = dyn_cast<MemSetInst>(UU);
        if (!II)
          continue;
        if (II->getArgOperand(0) != GEP)
          return UnKnown;
        if (Memset)
          return UnKnown;
        Memset = II;
      }
    }
  // Memset is not mandatory.
  if (Memset) {
    auto *GEP = cast<GetElementPtrInst>(Memset->getArgOperand(0));
    // Make sure it is initializing zero values.
    Value *ZeroVal = Memset->getArgOperand(1);
    if (!isa<ConstantInt>(ZeroVal) ||
        !cast<ConstantInt>(ZeroVal)->isZeroValue())
      return UnKnown;

    Value *LValue;
    Value *RValue;
    ICmpInst::Predicate Predi;
    // Check this memset is controlled under index value of previous loop
    // and NewSize.
    //   %43 = icmp ult i32 %42, %NewSize
    //   br i1 %43, label %MEM_BLK, label %SOME
    BasicBlock *FBlock =
        getBBControlledOps(Memset->getParent(), &LValue, &RValue, &Predi);
    if (!FBlock || Predi != ICmpInst::ICMP_ULT)
      return UnKnown;
    if (NewSize != RValue)
      return UnKnown;
    // Make sure LValue is either index value of previous loop or
    // original size.
    auto *Phi = dyn_cast<PHINode>(LValue);
    if (!checkFieldOfArgClassLoad(LValue, ThisObj, SizeField) &&
        !IsValueOfIndexCountAfterLoopExit(Phi, CopyLoop))
      return UnKnown;

    // Check index value multiplied with size of element.
    //   %45 = zext i32 %LValue to i64
    //   %46 = shl nuw nsw i64 %45, 3
    //   %47 = getelementptr i8, i8* %30, i64 %46
    if (!checkAllocSizeOfArray(GEP->getOperand(1), ThisObj, LValue))
      return UnKnown;

    Value *MemsetSize = Memset->getArgOperand(2);
    if (!AllowedMemsetSizePatternOne(MemsetSize, NewSize, LValue) &&
        !AllowedMemsetSizePatternTwo(MemsetSize, NewSize, LValue))
      return UnKnown;

    Visited.insert(cast<Instruction>(MemsetSize));
    Visited.insert(Memset);
    Visited.insert(GEP);
  }

  // Array field assignment should be dominated by Free call.
  BasicBlock *PH = CopyLoop->getLoopPreheader();
  BasicBlock *LHead = nullptr;
  if (PH)
    LHead = PH->getSinglePredecessor();
  else
    LHead = CopyLoop->getLoopPredecessor();
  assert(LHead && "Expected predecessor");
  dtrans::FreeCallInfo *FInfo = *FreeList.begin();
  Instruction *FreeRelatedInst = FInfo->getInstruction();
  if (FreeBB != FreeRelatedInst->getParent())
    FreeRelatedInst = &FreeBB->front();
  Instruction *CopyRelatedInst = &LHead->front();
  if (!checkDominatorInfo(FreeRelatedInst, ArrayFieldStore))
    return UnKnown;
  // Free Call should be dominated by Array element assignment.
  if (!checkDominatorInfo(CopyRelatedInst, FreeRelatedInst))
    return UnKnown;

  if (!processAssumeCalls(Fn, ThisObj))
    return UnKnown;

  if (!checkAllInstsProcessed(Fn, Visited)) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO, {
      dbgs() << " Failed: Missing to process some instructions.\n";
    });
    return UnKnown;
  }
  return Resize;
}

// Analyze all member functions of class.
//  Step 1: Categorize functions using signature
//  Step 2: Analyze constructors and detect Size, Capacity etc fields.
//  Step 3: Analyze remaining member function to prove the functionality.
//
bool ClassInfo::analyzeClassFunctions() {

  // Try to recognize functionality of Fn based on initial FunctionKind.
  auto RecognizeFunctionality = [this](Function *Fn) {
    FunctionKind FKind = UnKnown;

    // Check if the function is already analyzed.
    auto It = FinalFuncKind.find(Fn);
    if (It != FinalFuncKind.end())
      return It->second != UnKnown;

    DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO, {
      dbgs() << "  Analyzing functionality of " << Fn->getName() << "...\n";
    });
    FunctionKind InitKind = InitialFuncKind[Fn];
    switch (InitKind) {
    case CopyConstructor:
      FKind = recognizeCopyConstructor(Fn);
      break;
    case AppendElem:
      FKind = recognizeAppendElem(Fn);
      break;
    case Resize:
      FKind = recognizeResize(Fn);
      break;
    case Destructor:
      FKind = recognizeDestructor(Fn);
      break;
    case SetElem:
      FKind = recognizeSetElem(Fn);
      break;
    case GetElem:
      FKind = recognizeGetElem(Fn);
      break;
    case GetSizeOrCapacity:
      FKind = recognizeGetSizeOrCapacity(Fn);
      break;

    case GetCapacity:
    case GetSize:
    case Constructor:
    case DestructorWrapper:
    case UnKnown:
      break;
    }
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO, {
      dbgs() << "  Functionality of " << Fn->getName() << ": ";
    });
    // Record FKind.
    FinalFuncKind[Fn] = FKind;
    if (FKind == UnKnown) {
      DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO, {
        dbgs() << "Failed to recognize as " << InitKind << "\n";
      });
      return false;
    }
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO,
                    { dbgs() << "Recognized as " << FKind << "\n"; });
    return true;
  };

  DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO,
                  { dbgs() << "  Categorize functions using signature: \n"; });

  SmallPtrSet<Function *, 2> ConstructorSet;

  collectElementDataTypes();
  for (auto *Fn : field_member_functions()) {
    auto *ClassTy = getClassType(Fn, DTInfo.getTypeMetadataReader());
    FunctionKind FKind = categorizeFunctionUsingSignature(Fn, ClassTy);
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO, {
      dbgs() << "       " << Fn->getName() << ":   " << FKind << "\n";
    });
    if (FKind == UnKnown) {
      DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO,
                      { dbgs() << "  Failed: Unknown signature found.\n"; });
      return false;
    }
    if (FKind == Constructor && SOACInfo->isCandidateFieldDerivedTy(ClassTy))
      ConstructorSet.insert(Fn);
    InitialFuncKind[Fn] = FKind;
  }
  if (ConstructorSet.size() != 1) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO, {
      dbgs() << "  Failed: Unexpected number of constructors\n";
    });
    return false;
  }
  CtorFunction = *ConstructorSet.begin();
  if (CtorFunction->hasAddressTaken()) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO,
                    { dbgs() << "  Failed: Constructor address taken\n"; });
    return false;
  }
  auto *ClassTy = getClassType(CtorFunction, DTInfo.getTypeMetadataReader());
  DTransType *BaseClassTy = getSOASimpleBaseType(ClassTy);
  DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO, {
    dbgs() << "  Analyzing Constructor " << CtorFunction->getName() << "\n";
  });
  FunctionKind FKind;
  if (BaseClassTy)
    FKind = recognizeDerivedConstructor(CtorFunction, ClassTy, BaseClassTy);
  else
    FKind = recognizeConstructor(CtorFunction);
  if (FKind == UnKnown) {
    DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO,
                    { dbgs() << "  Failed: Constructor not recognized\n"; });
    return false;
  }
  DEBUG_WITH_TYPE(DTRANS_SOATOAOSOPCLASSINFO, {
    dbgs() << "  Passed: Constructor recognized\n";
    dbgs() << "    Capacity field: " << CapacityField << "\n";
    dbgs() << "    Size field: " << SizeField << "\n";
  });

  // Analyze remaining member functions.
  for (auto *Fn : field_member_functions())
    if (!RecognizeFunctionality(Fn)) {
      if (!RecognizeAll)
        return false;
    }

  return true;
}

// Returns constructor Wrapper if there is only one.
// Otherwise, returns nullptr.
Function *ClassInfo::getCtorWrapper() {
  Function *CtorWrapper = nullptr;

  for (auto *F : field_member_functions()) {
    FunctionKind FKind = getFinalFuncKind(F);
    if (FKind == Constructor) {
      auto *ClassTy = getClassType(F, DTInfo.getTypeMetadataReader());
      auto *BaseClassTy = getSOASimpleBaseType(ClassTy);
      if (BaseClassTy) {
        if (CtorWrapper)
          return nullptr;
        CtorWrapper = F;
      }
    }
  }
  return CtorWrapper;
}

// Returns member function of type "FKind" if there is only one.
// Otherwise, returns nullptr.
Function *ClassInfo::getSingleMemberFunction(FunctionKind FKind) {
  Function *Func = nullptr;

  for (auto *F : field_member_functions())
    if (getFinalFuncKind(F) == FKind) {
      if (Func)
        return nullptr;
      Func = F;
    }
  return Func;
}

// Returns store instruction that saves flag field.
StoreInst *ClassInfo::getFlagFieldStoreInstInCtor() {
  StoreInst *SI = nullptr;

  Function *CtorF = getCtorFunction();
  int32_t FFI = getFlagField();
  for (Instruction &I : instructions(*CtorF)) {
    auto *G = dyn_cast<GetElementPtrInst>(&I);

    if (!G || !isa<StructType>(G->getSourceElementType()) ||
        G->getNumIndices() != 2)
      continue;
    // Makes sure the field is accessed from "this" pointer.
    if (G->getPointerOperand() != CtorF->getArg(0))
      continue;
    int32_t GIdx = cast<ConstantInt>(G->getOperand(2))->getLimitedValue();
    if (GIdx != FFI)
      continue;

    if (!G->hasOneUse())
      return nullptr;
    if (SI)
      return nullptr;
    SI = dyn_cast<StoreInst>(G->user_back());
    if (!SI || SI->getPointerOperand() != G)
      return nullptr;
  }
  return SI;
}

} // namespace dtransOP

} // namespace llvm
