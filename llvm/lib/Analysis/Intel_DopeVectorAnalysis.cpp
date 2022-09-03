//===------- Intel_DopeVectorAnalysis.cpp ----------------------- -*------===//
//
// Copyright (C) 2019-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_DopeVectorAnalysis.h"
#include "llvm/Analysis/CallGraph.h"
#if INTEL_FEATURE_SW_ADVANCED
#include "llvm/Analysis/Intel_LangRules.h"
#endif // INTEL_FEATURE_SW_ADVANCED
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "dopevector-analysis"

#if INTEL_FEATURE_SW_ADVANCED
// If true then we are going to relax some conditions to enable a more
// aggressive DVCP.
static cl::opt<bool> EnableAgressiveDVCPOpt("intel-dvcp-aggressive",
                                              cl::init(true),
                                              cl::ReallyHidden);

static cl::opt<bool> CheckOutOfBoundsOK("dva-check-dtrans-outofboundsok",
                                        cl::init(true),
                                        cl::ReallyHidden);
#endif // INTEL_FEATURE_SW_ADVANCED

namespace llvm {

namespace dvanalysis {

extern bool isDopeVectorType(const Type *Ty, const DataLayout &DL,
                             uint32_t *ArrayRank,
                             Type **ElementType) {
  const unsigned int DVFieldCount = 7;
  const unsigned int PerDimensionCount = 3;
  uint32_t ArRank = 0;

  // Helper to check that all types contained in the structure in the range
  // of (Begin, End) are of type \p TargType
  auto ContainedTypesMatch = [](const StructType *StTy,
                                const llvm::Type *TargType,
                                unsigned int Begin, unsigned int End) {
    for (unsigned Idx = Begin; Idx < End; ++Idx)
      if (StTy->getContainedType(Idx) != TargType)
        return false;

    return true;
  };

  auto *StTy = dyn_cast<StructType>(Ty);
  if (!StTy)
    return false;

  unsigned ContainedCount = StTy->getNumContainedTypes();
  if (ContainedCount != DVFieldCount)
    return false;

  llvm::Type *FirstType = StTy->getContainedType(0);
  if (!FirstType->isPointerTy())
    return false;

  // All fields are "long" type?
  llvm::Type *LongType =
      Type::getIntNTy(Ty->getContext(), DL.getPointerSizeInBits());
  if (!ContainedTypesMatch(StTy, LongType, 1U, ContainedCount - 1))
    return false;

  // Array of structures for each rank?
  llvm::Type *LastType = StTy->getContainedType(ContainedCount - 1);
  auto *ArType = dyn_cast<ArrayType>(LastType);
  if (!ArType)
    return false;
  ArRank = ArType->getArrayNumElements();

  // Structure for extent, stride, and lower bound?
  llvm::Type *ElemTy = ArType->getArrayElementType();
  auto *StElemTy = dyn_cast<StructType>(ElemTy);
  if (!StElemTy)
    return false;
  if (StElemTy->getNumContainedTypes() != PerDimensionCount)
    return false;
  if (!ContainedTypesMatch(StElemTy, LongType, 0U, PerDimensionCount))
    return false;

  *ArrayRank = ArRank;

  // TODO: Collecting the type for the pointer address is only available
  // for typed pointers. This needs to be updated for opaque pointers, if the
  // information is required.
  if (ElementType) {
    *ElementType = nullptr;
    if (Ty->getContext().supportsTypedPointers())
      *ElementType = FirstType->getNonOpaquePointerElementType();
  }

  return true;
}

extern bool isDopeVectorType(const Type *Ty, const DataLayout &DL) {
  uint32_t ArrayRank = 0;
  return isDopeVectorType(Ty, DL, &ArrayRank, nullptr);
}

extern bool isUplevelVarType(Type *Ty) {
  //
  // CMPLRLLVM-30087: Extend this to handle an optional function name
  // and the embedded string "uplevel_nested_type".
  //
  // For now, just check the type of the variable as being named
  //     "%[FUNCTION_NAME.]uplevel_[nested_]type[.SUFFIX]"
  // In the future, the front-end should provide some metadata indicator that
  // a variable is an uplevel.
  //
  auto *StTy = dyn_cast<StructType>(Ty);
  if (!StTy || !StTy->hasName())
    return false;
  StringRef MatchString = "";
  StringRef TypeName = StTy->getName();
  if (TypeName.contains("uplevel_type"))
    MatchString = "uplevel_type";
  else if (TypeName.contains("uplevel_nested_type"))
    MatchString = "uplevel_nested_type";
  else
    return false;
  if (!TypeName.startswith(MatchString)) {
    size_t DropCount = TypeName.find('.');
    if (DropCount == StringLiteral::npos)
      return false;
    TypeName = TypeName.drop_front(DropCount + 1);
  }
  // Strip a '.' and any characters that follow it from the name.
  TypeName = TypeName.take_until([](char C) { return C == '.'; });
  if (TypeName != MatchString)
    return false;
  return true;
}

extern bool isValidUseOfSubscriptCall(const SubscriptInst &Subs,
                                      const Value &Base,
                                      uint32_t ArrayRank, uint32_t Rank,
                                      bool CheckForTranspose,
                                      Optional<uint64_t> LowerBound,
                                      Optional<uint64_t> Stride) {
  LLVM_DEBUG({
    dbgs().indent((ArrayRank - Rank) * 2 + 4);
    dbgs() << "Checking call: " << Subs << "\n";
  });

  if (Subs.getArgOperand(PtrOpNum) != &Base)
    return false;

  if (CheckForTranspose) {
    unsigned RankParam = Subs.getRank();
    if (RankParam != Rank)
      return false;
  }

  if (LowerBound) {
    auto LBVal = dyn_cast<ConstantInt>(Subs.getLowerBound());
    if (!LBVal || LBVal->getLimitedValue() != *LowerBound)
      return false;
  }

  if (Stride) {
    auto StrideVal = dyn_cast<ConstantInt>(Subs.getStride());
    if (!StrideVal || StrideVal->getLimitedValue() != *Stride)
      return false;
  }

  return true;
}

// Helper function to check whether \p V is a GEP that corresponds to a field
// within an uplevel type.
static bool isFieldInUplevelTypeVar(Value *V) {
  auto *GEP = dyn_cast<GetElementPtrInst>(V);
  if (!GEP)
    return false;
  return isUplevelVarType(GEP->getSourceElementType());
}

Optional<uint64_t> getConstGEPIndex(const GEPOperator &GEP,
                                    unsigned int OpNum) {
  auto FieldIndex = dyn_cast<ConstantInt>(GEP.getOperand(OpNum));
  if (FieldIndex)
    return Optional<uint64_t>(FieldIndex->getLimitedValue());
  return None;
}

Optional<unsigned int> getArgumentPosition(const CallBase &CI,
                                           const Value *Val) {
  Optional<unsigned int> Pos;
  unsigned int ArgCount = CI.arg_size();
  for (unsigned int ArgNum = 0; ArgNum < ArgCount; ++ArgNum)
    if (CI.getArgOperand(ArgNum) == Val) {
      if (Pos)
        return None;

      Pos = ArgNum;
    }

  return Pos;
}

// Return true if the input CallBase is a call to a function that allocates
// memory (e.g. for_alloc_allocate_handle)
static bool isCallToAllocFunction(CallBase *Call,
    std::function<const TargetLibraryInfo &(Function &F)> &GetTLI) {

  if (!Call || !Call->getCalledFunction())
    return false;

  Function *F = Call->getCalledFunction();
  Function *Caller = Call->getFunction();

  LibFunc TheLibFunc;
  const TargetLibraryInfo &TLI = GetTLI(*const_cast<Function *>(Caller));
  if (TLI.getLibFunc(F->getName(), TheLibFunc) && TLI.has(TheLibFunc)) {
    // TODO: We may want to expand this in the future for other forms of
    // alloc
    switch (TheLibFunc) {
      case LibFunc_for_allocate_handle:
      case LibFunc_for_alloc_allocatable_handle:
        return true;
      default:
        return false;
    }
  }

  return false;
}

// Return true if the input CallBase is a call to a function that deallocates
// memory (e.g. for_dealloc_allocatable_handle)
static bool isCallToDeallocFunction(CallBase *Call,
    std::function<const TargetLibraryInfo &(Function &F)> &GetTLI) {

  if (!Call || !Call->getCalledFunction())
    return false;

  Function *F = Call->getCalledFunction();
  Function *Caller = Call->getFunction();

  LibFunc TheLibFunc;
  const TargetLibraryInfo &TLI = GetTLI(*const_cast<Function *>(Caller));
  if (TLI.getLibFunc(F->getName(), TheLibFunc) && TLI.has(TheLibFunc)) {
    // TODO: We may want to expand this in the future for other forms of
    // dealloc
    switch (TheLibFunc) {
      case LibFunc_for_dealloc_allocatable_handle:
        return true;
      default:
        return false;
    }
  }

  return false;
}

// Return true if the input BitCastOperator is casting the dope vector's
// pointer address to initialize it as null (0).
static bool bitCastUsedForInit(BitCastInst *BI, Value *DVObject) {
  if (!BI || !DVObject)
    return false;

  if (!BI->hasOneUser())
    return false;

  Value *BCOperand = BI->getOperand(0);
  if (BCOperand != DVObject)
    return false;

  StoreInst *SI = dyn_cast<StoreInst>(BI->user_back());
  if (!SI)
    return false;

  ConstantInt *ZeroInit = dyn_cast<ConstantInt>(SI->getValueOperand());
  return ZeroInit && ZeroInit->isZero();
}

#if INTEL_FEATURE_SW_ADVANCED
// Given a Value and the TargetLibraryInfo, check if it is a BitCast
// and is only used for data allocation. Return the call to the data
// alloc function.
static CallBase *bitCastUsedForAllocationOnly(Value *Val,
    std::function<const TargetLibraryInfo &(Function &F)> &GetTLI) {

  if (!Val)
    return nullptr;

  auto *BC = dyn_cast<BitCastOperator>(Val);
  if (!BC)
    return nullptr;

  CallBase *Call = nullptr;

  if (!BC->hasOneUser())
    return nullptr;
  Call = dyn_cast<CallBase>(BC->user_back());
  if (!Call || !isCallToAllocFunction(Call, GetTLI))
    return nullptr;
  return Call;
}
#endif // INTEL_FEATURE_SW_ADVANCED

// Given a Value and the TargetLibraryInfo, check if it is a BitCast
// and is only used for data allocation and deallocation. Return the
// call to the data alloc function.
static CallBase *bitCastUsedForAllocation(Value *Val,
    std::function<const TargetLibraryInfo &(Function &F)> &GetTLI) {

  if (!Val)
    return nullptr;

  auto *BC = dyn_cast<BitCastOperator>(Val);
  if (!BC)
    return nullptr;

  CallBase *Call = nullptr;

  for (auto *U : BC->users()) {
    if (auto *LI = dyn_cast<LoadInst>(U)) {
      // Check if the BitCast is used for loading the pointer to call
      // dealloc
      if (!LI->hasOneUser())
        return nullptr;

      auto *DeAllocCall = dyn_cast<CallBase>(LI->user_back());
      if (!DeAllocCall || !isCallToDeallocFunction(DeAllocCall, GetTLI))
        return nullptr;
    } else if (auto *TempCall = dyn_cast<CallBase>(U)) {
      // Else the BitCast is used for alloc and should be one alloc call
      if (Call || !isCallToAllocFunction(TempCall, GetTLI))
        return nullptr;

      Call = TempCall;
    } else {
      // Anything else is not valid
      return nullptr;
    }
  }

  return Call;
}

// Return true if the input store instruction copies the dope vector
// field from SrcPtr to the same field in another dope vector.
static bool IsCopyFromDVField(StoreInst *SI, Value *SrcPtr) {
  if (!SrcPtr)
    return false;

  auto *GEPPtr = dyn_cast<GEPOperator>(SI->getPointerOperand());
  if (!GEPPtr)
    return false;

  LoadInst *LoadPtr = dyn_cast<LoadInst>(SI->getValueOperand());
  if (!LoadPtr)
    return false;

  auto *GEPLoaded = dyn_cast<GEPOperator>(LoadPtr);
  if (!GEPLoaded)
    return false;

  if (GEPLoaded->getOperand(0) != SrcPtr)
    return false;

  auto SrcDVFieldType =
      DopeVectorAnalyzer::identifyDopeVectorField(*GEPLoaded);
  auto DestDVFieldType =
      DopeVectorAnalyzer::identifyDopeVectorField(*GEPPtr);

  return SrcDVFieldType == DestDVFieldType;
}

// Return true if the input value is:
//   * A store instruction that writes into the input Pointer
//   * A load instruction
// Else return false. This function also updates if the dope vector field has
// been read or written.
bool DopeVectorFieldUse::analyzeLoadOrStoreInstruction(Value *V,
                                                       Value *Pointer,
                                                       bool IsNotForDVCP) {
  if (!V)
    return false;

  if (auto *SI = dyn_cast<StoreInst>(V)) {
    // Make sure the store is to the field address, and that it's not the
    // field address being stored somewhere.
    if (SI->getValueOperand() != Pointer) {
      Stores.insert(SI);
      bool IsWrittenWithNull = false;
      if (auto CV = dyn_cast<Constant>(SI->getValueOperand())) {
        if (CV->isNullValue())
          IsWrittenWithNull = true;
      }
      if (!IsWritten && IsWrittenWithNull)
        IsOnlyWrittenWithNull = true;
      else if (!IsWrittenWithNull)
        IsOnlyWrittenWithNull = false;
      IsWritten = true;
    } else {
      return false;
    }
  } else if (auto *LI = dyn_cast<LoadInst>(V)) {
    Loads.insert(LI);
    if (IsNotForDVCP)
      NotForDVCPLoads.insert(LI);
    IsRead = true;
  } else {
    return false;
  }

  return true;
}

// Traverse through the users of a field address and check where they are used
// for loading or storing data.
void DopeVectorFieldUse::analyzeUses() {
  if (IsBottom)
    return;

  if (FieldAddr.empty())
    return;

  for (auto *FAddr : FieldAddr) {
    bool IsNotForDVCP = NotForDVCPFieldAddr.contains(FAddr);
    for (auto *U : FAddr->users())
      if (!analyzeLoadOrStoreInstruction(U, FAddr, IsNotForDVCP)) {
          IsBottom = true;
          break;
      }
  }
}

// Store the input subscript instruction if:
//  * The lower bound, stride and index are constants
//  * The index is in the limit of the dope vector rank (DVRank)
//  * The current dope vector field is the extent, stride or lowerbound
//
// This function basically collects the subscript instructions that are
// used to access the extent, stride and lower bound fields of a dope
// vector. All the subscripts will be analyzed in
// DopeVectorFieldUse::analyzeSubscriptUses().
void DopeVectorFieldUse::collectSubscriptInformation(SubscriptInst *SI,
    DopeVectorFieldType FieldType, unsigned long DVRank) {

  if (!SI) {
    IsBottom = true;
    return;
  }

  // This function is only for extent, stride and lowerbound
  assert(FieldType >= DopeVectorFieldType::DV_ExtentBase &&
         "Trying to add subscript information for a dope vector "
         "field that is not an extent, stride or lower bound");

  // All entries in the subscript should be constant
  if (!isa<ConstantInt>(SI->getLowerBound()) ||
      !isa<ConstantInt>(SI->getStride()) ||
      !isa<ConstantInt>(SI->getIndex())) {
    IsBottom = true;
    return;
  }

  // Collect the array entry and make sure it is within the bounds of DVRank
  ConstantInt *Index = cast<ConstantInt>(SI->getIndex());
  uint64_t ArrayEntry = Index->getZExtValue();
  if (ArrayEntry >= DVRank) {
    IsBottom = true;
    return;
  }

  Subscripts.insert(SI);
}

// Traverse through the users of a subscript instruction and check if they are
// load, store or PHINode instructions. The goal of this function is to find
// where the data for the extent, stride or lower bound is loaded and stored.
// Any unsupported use will set the dope vector field as bottom.
void DopeVectorFieldUse::analyzeSubscriptsUses() {

  // Return true if all the incoming values in the PHINode are subscript
  // instructions, they have the same rank, stide, lower bound, the pointer
  // operand as SI, and they are in the Subscripts set.
  auto AnalyzePHINode = [&, this](PHINode *PHI, SubscriptInst *SI) -> bool {
    for (unsigned int I = 0, E = PHI->getNumIncomingValues(); I < E; I++) {
      SubscriptInst *PHISI = dyn_cast<SubscriptInst>(PHI->getIncomingValue(I));
      if (!PHISI)
        return false;

      if (PHISI == SI)
        continue;

      if (PHISI->getRank() != SI->getRank() ||
          PHISI->getStride() != SI->getStride() ||
          PHISI->getLowerBound() != SI->getLowerBound() ||
          PHISI->getIndex() != SI->getIndex())
        return false;

      if (Subscripts.count(PHISI) == 0)
        return false;
    }

    // PHINodes should be used for loading data
    for (auto *U : PHI->users()) {
      if (auto *LI = dyn_cast<LoadInst>(U)) {
        Loads.insert(LI);
        IsRead = true;
      } else {
        return false;
      }
    }

    return true;
  };

  if (IsBottom)
    return;

  if (FieldAddr.empty() || Subscripts.empty())
    return;

  for (auto *SI : Subscripts) {
    // The operand of the subscript instruction should be the same as the
    // field address.
    if (FieldAddr.count(SI->getPointerOperand()) == 0) {
      IsBottom = true;
      return;
    }
    bool IsNotForDVCP = NotForDVCPFieldAddr.count(SI->getPointerOperand());
    // Traverse through the users of the subscript instruction and check for
    // Load, Store and PHINodes.
    for (auto *U : SI->users()) {
      if (isa<StoreInst>(U) || isa<LoadInst>(U)) {
        // The subscript should be used for load or store
        if(!analyzeLoadOrStoreInstruction(U, SI, IsNotForDVCP)) {
          IsBottom = true;
          break;
        }
      } else if (auto *PHI = dyn_cast<PHINode>(U)) {
        // There is a chance that a subscript can be used in a PHINode. This
        // happens when the input code checks if the data is allocated takes
        // one path, else takes another path. If this is the case, then all
        // the incoming values of the PHINode should be subscript instructions
        // with the same rank, lower bound, pointer operand and index.
        if (!AnalyzePHINode(PHI, SI)) {
          IsBottom = true;
          break;
        }
      } else {
        IsBottom = true;
        break;
      }
    }
  }
}

// Traverse through the collected store instructions and identify the possible
// constant value for the current dope vector field.
void DopeVectorFieldUse::identifyConstantValue() {
  if (!getIsSingleValue())
    return;

  StoreInst *SI = *Stores.begin();
  ConstantInt *Const = dyn_cast<ConstantInt>(SI->getValueOperand());
  if (!Const)
    return;

  ConstantValue = Const;
}

// Turn on the property of storing multiple pointer accesses
void DopeVectorFieldUse::setAllowMultipleFieldAddresses() {

  // First check that no data was collected
  if (IsBottom || IsRead || IsWritten)
    return;

  if (!Loads.empty() || !Subscripts.empty() ||
      !Stores.empty() || !FieldAddr.empty() || ConstantValue)
    return;

  AllowMultipleFieldAddresses = true;
}

bool DopeVectorFieldUse::matches(const DopeVectorFieldUse& Other) const {


  //
  // Return 'true' if after checking the stores() of 'DVFU', 'SIV' could
  // have multiple values. If not, set 'SIV' to the current single value.
  //
  auto CouldHaveMultipleValues = [](const DopeVectorFieldUse &DVFU,
                                    Optional<uint64_t> &SIV) -> bool {
    for (StoreInst *SI : DVFU.stores()) {
       auto CI = dyn_cast<ConstantInt>(SI->getValueOperand());
       if (!CI)
         return true;
       if (CI->isZero())
         continue;
       if (!SIV)
         SIV = CI->getZExtValue();
       else if (SIV && (*SIV != CI->getZExtValue()))
         return true;
    }
    return false;
  };

  if (getIsBottom() != Other.getIsBottom())
    return false;
  if (AllowMultipleFieldAddresses != Other.AllowMultipleFieldAddresses)
    return false;
  if ((!AllowMultipleFieldAddresses || !Other.AllowMultipleFieldAddresses) &&
      (FieldAddr.size() + Other.FieldAddr.size() > 1))
    return false;
  if (RequiresSingleNonNullValue != Other.RequiresSingleNonNullValue)
    return false;
  //
  // NOTE: This test must be done here, because we need to determine while
  // merging the NestedDopeVectorInfos for a single field in a structure
  // whether there is still only one written non-null value.
  //
  if (RequiresSingleNonNullValue) {
    Optional<uint64_t> SIV = None;
    if (CouldHaveMultipleValues(*this, SIV))
      return false;
    if (CouldHaveMultipleValues(Other, SIV))
      return false;
  }
  if (!getIsWritten() || !Other.getIsWritten())
    return true;
  if (!getConstantValue() || !Other.getConstantValue())
    return true;
  auto CV0 = getConstantValue()->getZExtValue();
  auto CV1 = Other.getConstantValue()->getZExtValue();
  if (CV0 != CV1)
    return false;
  return true;
}

void DopeVectorFieldUse::merge(const DopeVectorFieldUse& Other) {
  FieldAddr.insert(Other.FieldAddr.begin(), Other.FieldAddr.end());
  Loads.insert(Other.Loads.begin(), Other.Loads.end());
  NotForDVCPLoads.insert(Other.NotForDVCPLoads.begin(),
    Other.NotForDVCPLoads.end());
  Stores.insert(Other.Stores.begin(), Other.Stores.end());
  Subscripts.insert(Other.Subscripts.begin(), Other.Subscripts.end());
  if (Other.getIsRead())
    IsRead = true;
  if (Other.getIsWritten()) {
    IsWritten = true;
    identifyConstantValue();
  }
}

// Copy the load instructions from the input field that is in another dope vector
// to the current field. The difference between the merge is that the copy will
// only happens if the constant are the same, or if the constant is given by
// loading the dope vector field that is being copied.
void DopeVectorFieldUse::collectFromCopy(const DopeVectorFieldUse& CopyDVField) {
  if (CopyDVField.getIsBottom() || getIsBottom())
    return;

  if (!CopyDVField.getIsSingleValue())
    return;

  if (!getConstantValue())
    return;

  bool MergeAllowed = false;
  if (CopyDVField.getConstantValue()) {
    MergeAllowed =
      getConstantValue()->getZExtValue() ==
      CopyDVField.getConstantValue()->getZExtValue();
  } else {
    LoadInst *LI = dyn_cast<LoadInst>(CopyDVField.getSingleValue());
    if (!LI)
      return;

    for (auto *LoadI : Loads) {
      if (LoadI == LI) {
        MergeAllowed = true;
        break;
      }
    }
  }

  if (MergeAllowed) {
    Loads.insert(CopyDVField.Loads.begin(), CopyDVField.Loads.end());
    NotForDVCPLoads.insert(CopyDVField.NotForDVCPLoads.begin(),
                           CopyDVField.NotForDVCPLoads.end());
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DopeVectorFieldUse::dump() const { print(dbgs()); }
void DopeVectorFieldUse::print(raw_ostream &OS, const Twine &Header) const {
  OS << Header;
  print(OS);
}

void DopeVectorFieldUse::print(raw_ostream &OS) const {
  if (FieldAddr.empty()) {
    OS << "  Not set\n";
    return;
  }

  for (auto *FAddr : FieldAddr)
    OS << *FAddr << "\n";

  OS << "  Analysis :";
  OS << (IsBottom ? " BOTTOM" : "");
  OS << (IsRead ? " READ" : "");
  OS << (IsWritten ? " WRITTEN" : "");
  OS << "\n";

  OS << "  Stores   : " << Stores.size() << "\n";
  for (auto *V : Stores)
    OS << "    " << *V << "\n";

  OS << "  Loads    : " << Loads.size() << "\n";
  for (auto *V : Loads)
    OS << "    " << *V << "\n";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

void DopeVectorAnalyzer::setInvalid() {
  LLVM_DEBUG(dbgs() << "  DV-Invalid: " << *DVObject << "\n");
  IsValid = false;
}

bool DopeVectorAnalyzer::checkMayBeModified() const {
  if (!IsValid)
    return true;

  if (PtrAddr.getIsBottom() || ElementSizeAddr.getIsBottom() ||
      CodimAddr.getIsBottom() || FlagsAddr.getIsBottom() ||
      DimensionsAddr.getIsBottom())
    return true;

  if (PtrAddr.getIsWritten() || ElementSizeAddr.getIsWritten() ||
      CodimAddr.getIsWritten() || FlagsAddr.getIsWritten() ||
      DimensionsAddr.getIsWritten())
    return true;

  for (const auto &Field : LowerBoundAddr)
    if (Field.getIsBottom() || Field.getIsWritten())
      return true;

  for (const auto &Field : StrideAddr)
    if (Field.getIsBottom() || Field.getIsWritten())
      return true;

  for (const auto &Field : ExtentAddr)
    if (Field.getIsBottom() || Field.getIsWritten())
      return true;

  return false;
}

bool
DopeVectorAnalyzer::checkArrayPointerUses(SubscriptInstSet *SubscriptCalls) {
  // Get a set of Value objects that hold the address of the array pointer.
  SmallPtrSet<Value *, 8> ArrayPtrValues;
  const DopeVectorFieldUse &PtrAddr = getPtrAddrField();
  if (!getAllValuesHoldingFieldValue(PtrAddr, ArrayPtrValues)) {
    LLVM_DEBUG(dbgs() << "Unsupported use of array pointer address:\n");
    return false;
  }

  // Now check all uses of the address to be sure they are only used to move
  // the address to another var (Select or PhiNode), or are used in a
  // subscript intrinsic call.
  for (auto *ArrPtr : ArrayPtrValues) {
    LLVM_DEBUG(dbgs() << "  Uses: " << *ArrPtr << "\n");
    for (auto *PtrUser : ArrPtr->users()) {
      if (isa<SelectInst>(PtrUser) || isa<PHINode>(PtrUser)) {
        continue;
      } else if (auto *Subs = dyn_cast<SubscriptInst>(PtrUser)) {
        uint32_t ArrayRank = getRank();
        if (!isValidUseOfSubscriptCall(*Subs, *ArrPtr, ArrayRank,
                                       ArrayRank - 1, SubscriptCalls)) {
          LLVM_DEBUG(dbgs() << "Array address: " << *ArrPtr
                            << " not in subscript call: " << *Subs << "\n");
          return false;
        }
        if (SubscriptCalls)
          SubscriptCalls->insert(Subs);
      } else {
        LLVM_DEBUG(dbgs() << "Unsupported use of array pointer address:\n"
                          << *PtrUser << "\n");
        return false;
      }
    }
  }

  return true;
}

// Forward declaration
static bool analyzeUplevelCallArg(uint32_t ArrayRank,
                                  SubscriptInstSet *SubscriptCalls, Function &F,
                                  uint64_t ArgPos, uint64_t FieldNum,
                                  SmallPtrSetImpl<const Function *> &Visited);

// This checks the uses of an uplevel variable for safety. Safe uses are:
// - If \p DVObject is non-null, we are analyzing the function that
//   initialized the uplevel var. In this case the dope vector member of the
//   uplevel can be written. Otherwise, writes are not allowed.
// - If the dope vector object is loaded from the uplevel variable, the uses
//   of the dope vector are checked to ensure the dope vector fields are not
//   modified.
// - If the uplevel variable is passed in a function call, a recursive call
//   will be made to this routine to check the usage of the uplevel in the
//   called function.
// Here 'ArrayRank' is the number of dimensions of the array represented
// by the dope vector, and 'SubscriptCalls', if not nullptr, is the set of
// subscript calls that reference the dope vector.
// 'Visited' is to avoid recursive re-entry into this routine when checking
// function calls made by 'F' that take the 'Uplevel' as a parameter.
static bool analyzeUplevelVar(uint32_t ArrayRank,
                              SubscriptInstSet *SubscriptCalls,
                              const Function &F, UplevelDVField &Uplevel,
                              Value *DVObject,
                              SmallPtrSetImpl<const Function *> &Visited) {
  if (!Visited.insert(&F).second)
    return true;

  Value *Var = Uplevel.first;
  uint64_t FieldNum = Uplevel.second;

  LLVM_DEBUG(dbgs() << "\nChecking use of uplevel variable in function: "
                    << F.getName() << " Field: " << FieldNum << "\n");

  // If the function makes use of the uplevel, then we expect there should be
  // an Instruction that is a GEP which gets the address of the DV field from
  // the uplevel variable. Collect all these GEPs into this vector for
  // analysis.
  SmallVector<GetElementPtrInst *, 4> DVFieldAddresses;

  // The uplevel variable may be passed to another function, collect the set
  // of {Function*, argument pos} pairs for functions that take this uplevel
  // as a parameter.
  FuncArgPosPairSet FuncsWithUplevelParams;

  for (auto *U : Var->users()) {
    auto *I = dyn_cast<Instruction>(U);
    assert(I && "Expected instruction\n");

    LLVM_DEBUG(dbgs() << "Uplevel var use: " << *I << "\n");

    if (auto *GEP = dyn_cast<GetElementPtrInst>(I)) {
      if (GEP->getNumIndices() == 2) {
        auto Idx0 = getConstGEPIndex(*(cast<GEPOperator>(GEP)), 1);
        auto Idx1 = getConstGEPIndex(*(cast<GEPOperator>(GEP)), 2);
        if (Idx0 && Idx1 && *Idx0 == 0) {
          // Ignore uses of other uplevel fields.
          if (*Idx1 != FieldNum)
            continue;

          DVFieldAddresses.push_back(GEP);
          continue;
        }
      }
      LLVM_DEBUG(dbgs() << "Unsupported usage of uplevel var:\n"
                        << *I << "\n");
      return false;
    } else if (auto *CI = dyn_cast<CallInst>(I)) {
      Function *F = CI->getCalledFunction();
      if (!F) {
        LLVM_DEBUG(dbgs() << "Uplevel var passed in indirect function call:\n"
                          << *CI << "\n");
        return false;
      }
      Optional<unsigned int> ArgPos = getArgumentPosition(*CI, Var);
      if (!ArgPos) {
        LLVM_DEBUG(dbgs() << "Uplevel var argument not unique in call:\n"
                          << *CI << "\n");
        return false;
      }
      FuncsWithUplevelParams.insert(FuncArgPosPair(F, *ArgPos));
    } else {
      LLVM_DEBUG(dbgs() << "Unsupported usage of uplevel var:\n"
                        << *I << "\n");

      return false;
    }
  }

  // Check the usage for all the GEPs that get the address of the dope vector
  // variable.
  // If the dope vector pointer field is loaded, check that all uses of the
  // dope vector are safe. If the dope vector pointer field is stored, check
  // that it is the write we expected that is initializing the uplevel.
  for (auto *DVFieldAddr : DVFieldAddresses)
    for (auto *U : DVFieldAddr->users()) {
      auto *I = dyn_cast<Instruction>(U);
      assert(I && "Expected instruction\n");

      if (auto *LI = dyn_cast<LoadInst>(I)) {
        DopeVectorAnalyzer DVA(LI);
        DVA.analyze(false);
        if (!DVA.analyzeDopeVectorUseInFunction(F, SubscriptCalls))
          return false;
      } else if (auto *SI = dyn_cast<StoreInst>(I)) {
        // The only store we expect to the DV field is the dope vector object
        // currently being analyzed.
        if (!DVObject || SI->getValueOperand() != DVObject) {
           LLVM_DEBUG(dbgs()
                   << "Store into uplevel var dope vector field no allowed\n");
           return false;
        }
      } else {
        return false;
      }
    }

  // Check all the functions that take the uplevel variable.
  for (auto &FuncArg : FuncsWithUplevelParams)
    if (!analyzeUplevelCallArg(ArrayRank, SubscriptCalls, *FuncArg.first,
                               FuncArg.second, FieldNum, Visited))
      return false;

  return true;
}

// Check a called function for usage of the uplevel variable for safety.
// Here 'ArrayRank' is the number of dimensions of the array represented
// by the dope vector, and 'SubscriptCalls', if not nullptr, is the set of
// subscript calls that reference the dope vector.
static bool analyzeUplevelCallArg(uint32_t ArrayRank,
                                  SubscriptInstSet *SubscriptCalls, Function &F,
                                  uint64_t ArgPos, uint64_t FieldNum,
                                  SmallPtrSetImpl<const Function *> &Visited) {
  if (F.isDeclaration())
    return false;

  assert(ArgPos < F.arg_size() && "Invalid argument position");
  auto Args = F.arg_begin();
  std::advance(Args, ArgPos);
  Argument *FormalArg = &(*Args);

  // Check the called function for its use of the uplevel passed in. We do
  // not allow the called function to store a new dope vector into the field,
  // so pass 'nullptr' for the DVObject.
  UplevelDVField LocalUplevel(FormalArg, FieldNum);
  if (!analyzeUplevelVar(ArrayRank, SubscriptCalls, F, LocalUplevel, nullptr,
                         Visited))
    return false;

  return true;
}

bool DopeVectorAnalyzer::getAllValuesHoldingFieldValue(
                                   const DopeVectorFieldUse &Field,
                                   SmallPtrSetImpl<Value *> &ValueSet) const {
  // Prime a worklist with all the direct loads of the field.
  SmallVector<Value *, 16> Worklist;
  llvm::copy(Field.loads(), std::back_inserter(Worklist));

  // Populate the set of objects containing the value loaded.
  while (!Worklist.empty()) {
    Value *V = Worklist.back();
    Worklist.pop_back();
    if (!ValueSet.insert(V).second)
      continue;

    for (auto *U : V->users())
      if ((isa<SelectInst>(U) || isa<PHINode>(U)) && !ValueSet.count(U))
        Worklist.push_back(U);
  }

  // Verify all the source nodes for PHI nodes and select instructions
  // originate from the field load (or another PHI/select).
  SmallVector<Value *, 4> IncomingVals;
  for (auto *V : ValueSet) {
    IncomingVals.clear();
    if (auto *Sel = dyn_cast<SelectInst>(V)) {
      IncomingVals.push_back(Sel->getTrueValue());
      IncomingVals.push_back(Sel->getFalseValue());
    } else if (auto *PHI = dyn_cast<PHINode>(V)) {
      for (Value *Val : PHI->incoming_values())
        IncomingVals.push_back(Val);
    }

    for (auto *ValIn : IncomingVals)
      if (!ValueSet.count(ValIn)) {
        LLVM_DEBUG(dbgs() << "Failed during check of:\n"
                          << *V
                          << "\nExpected PHI/select source to also be "
                             "in field value set: "
                          << *ValIn << "\n");
        return false;
      }
  }

  return true;
}

void DopeVectorAnalyzer::analyze(bool ForCreation, bool IsLocalDV) {
  LLVM_DEBUG(dbgs() << "\nChecking "
                    << (ForCreation ? "construction" : "use")
                    << " of dope vector: " << *DVObject << "\n");

  // Assume valid, until proven otherwise.
  IsValid = true;

  GetElementPtrInst *PerDimensionBase = nullptr;
  GetElementPtrInst *ExtentBase = nullptr;
  GetElementPtrInst *StrideBase = nullptr;
  GetElementPtrInst *LowerBoundBase = nullptr;

  bool AllocSiteFound = false;
  bool PtrAddressInitFound = false;
  for (auto *DVUser : DVObject->users()) {
    LLVM_DEBUG(dbgs() << "  DV user: " << *DVUser << "\n");
    if (auto *GEP = dyn_cast<GetElementPtrInst>(DVUser)) {
      // Find which of the fields this GEP is the address of.
      // Note: We expect the field addresses to only be seen at most one
      // time for each field, otherwise we do not support it.
      DopeVectorFieldType DVFieldType =
          identifyDopeVectorField(*(cast<GEPOperator>(GEP)));
      switch (DVFieldType) {
      default:
        setInvalid();
        return;

      case DopeVectorFieldType::DV_ArrayPtr:
        PtrAddr.addFieldAddr(GEP);
        break;
      case DopeVectorFieldType::DV_ElementSize:
        ElementSizeAddr.addFieldAddr(GEP);
        break;
      case DopeVectorFieldType::DV_Codim:
        CodimAddr.addFieldAddr(GEP);
        break;
      case DopeVectorFieldType::DV_Flags:
        FlagsAddr.addFieldAddr(GEP);
        break;
      case DopeVectorFieldType::DV_Dimensions:
        DimensionsAddr.addFieldAddr(GEP);
        break;
      case DV_Reserved:
        // Ignore uses of reserved
        break;

        // The following fields require additional forward looking analysis to
        // get to the actual address-of objects.
      case DopeVectorFieldType::DV_PerDimensionArray:
        if (PerDimensionBase) {
          setInvalid();
          return;
        }
        PerDimensionBase = GEP;
        break;
      case DopeVectorFieldType::DV_LowerBoundBase:
        if (LowerBoundBase) {
          setInvalid();
          return;
        }
        LowerBoundBase = GEP;
        break;
      case DopeVectorFieldType::DV_ExtentBase:
        if (ExtentBase) {
          setInvalid();
          return;
        }
        ExtentBase = GEP;
        break;

      case DopeVectorFieldType::DV_StrideBase:
        if (StrideBase) {
          setInvalid();
          return;
        }
        StrideBase = GEP;
        break;
      }
    } else if (const auto *CI = dyn_cast<CallInst>(DVUser)) {
      if (IsLocalDV) {
        LLVM_DEBUG(dbgs() << "Dope vector used in call when it was expected "
                          << "to be local to the function: " << *CI << "\n");
        setInvalid();
        return;
      }


      Function *F = CI->getCalledFunction();
      if (!F) {
        LLVM_DEBUG(dbgs() << "Dope vector passed in indirect function call:\n"
                          << *CI << "\n");
        setInvalid();
        return;
      }

      Optional<unsigned int> ArgPos = getArgumentPosition(*CI, DVObject);
      if (!ArgPos) {
        LLVM_DEBUG(dbgs() << "Dope vector argument not unique in call:\n"
                          << *CI << "\n");
        setInvalid();
        return;
      }

      // Save the function for later analysis.
      FuncsWithDVParam.insert({F, *ArgPos});
    } else if (auto *SI = dyn_cast<StoreInst>(DVUser)) {
      // Check if the store is saving the dope vector object into an uplevel
      // var. Save the variable and field number for later analysis. (The
      // dope vector should only ever need to be stored to a single uplevel,
      // but make sure we didn't see one yet.)
      if (SI->getValueOperand() == DVObject && !IsLocalDV) {
        Value *PtrOp = SI->getPointerOperand();
        if (isFieldInUplevelTypeVar(PtrOp) && Uplevel.first == nullptr) {
          LLVM_DEBUG(dbgs() << "Dope vector needs uplevel analysis: "
                            << *SI << "\n");
          auto PtrGEP = cast<GEPOperator>(PtrOp);
          auto Idx0 = getConstGEPIndex(*PtrGEP, 1);
          auto Idx1 = getConstGEPIndex(*PtrGEP, 2);
          if (Idx0 && Idx1 && *Idx0 == 0) {
            Uplevel = UplevelDVField(PtrGEP->getPointerOperand(), *Idx1);
            continue;
          }
        }
      }

      LLVM_DEBUG(dbgs() << "Unsupported StoreInst using dope vector object\n"
                        << *DVUser << "\n");
      setInvalid();
      return;
    } else if (auto *BI = dyn_cast<BitCastInst>(DVUser)) {
      if (!IsLocalDV || !GetTLI) {
        LLVM_DEBUG(dbgs() << "Unsupported use of dope vector object\n"
                        << *DVUser << "\n");
        setInvalid();
        return;
      }

      bool BCForAlloc = bitCastUsedForAllocation(BI, *GetTLI);
      bool BCForInit = bitCastUsedForInit(BI, DVObject);

      if (!BCForAlloc && !BCForInit) {
        LLVM_DEBUG(dbgs() << "Unsupported BitCastInst using dope vector "
                          << "object\n" << *DVUser << "\n");
        setInvalid();
        return;
      }

      if (BCForAlloc) {
        if (AllocSiteFound) {
          LLVM_DEBUG(dbgs() << "Multiple allocations for dope vector "
                            << "object\n" << *DVUser << "\n");
          setInvalid();
          return;
        }

        AllocSiteFound = true;
      }

      if (BCForInit) {
        if (PtrAddressInitFound) {
          LLVM_DEBUG(dbgs() << "Multiple initializations for dope vector "
                            << "object\n" << *DVUser << "\n");
          setInvalid();
          return;
        }

        PtrAddressInitFound = true;
      }
    } else {
      LLVM_DEBUG(dbgs() << "Unsupported use of dope vector object\n"
                        << *DVUser << "\n");
      setInvalid();
      return;
    }
  }

  // We expect either the per-dimension base or base addresses of the
  // individual components, not both.
  if (PerDimensionBase) {
    if (ExtentBase || StrideBase || LowerBoundBase) {
      setInvalid();
      return;
    }

    std::pair<GetElementPtrInst *, FindResult> Result;
    Result = findPerDimensionArrayFieldGEP(*PerDimensionBase, DVR_Extent);
    if (Result.second == FR_Valid)
      ExtentBase = Result.first;
    Result = findPerDimensionArrayFieldGEP(*PerDimensionBase, DVR_Stride);
    if (Result.second == FR_Valid)
      StrideBase = Result.first;
    Result = findPerDimensionArrayFieldGEP(*PerDimensionBase, DVR_LowerBound);
    if (Result.second == FR_Valid)
      LowerBoundBase = Result.first;
  }

  // If the dope vector is local to the function, then check if the allocation
  // site was found.
  if (IsLocalDV && !AllocSiteFound) {
    LLVM_DEBUG(dbgs() << "Allocation site not found for local dope vector "
                      << "object\n" << *DVObject << "\n");
    setInvalid();
    return;
  }

  // Check the uses of the fields to make sure there are no unsupported uses,
  // and collect the loads and stores. For the PtrAddr field, we will need to
  // later analyze all the reads that get the address of the array to ensure
  // the address does not escape the module. For the dope vector strides, we
  // will need to analyze all the writes to the field to be sure the expected
  // value is being stored. For other fields, we may not need to collect all
  // the loads and stores, but for now, collect them all.
  PtrAddr.analyzeUses();
  ElementSizeAddr.analyzeUses();
  CodimAddr.analyzeUses();
  FlagsAddr.analyzeUses();
  DimensionsAddr.analyzeUses();

  // During dope vector creation, we expect to be see all the fields being set
  // up.
  if (ForCreation) {
    if (!PtrAddr.hasFieldAddr() || !ElementSizeAddr.hasFieldAddr() ||
        !CodimAddr.hasFieldAddr() || !FlagsAddr.hasFieldAddr() ||
        !DimensionsAddr.hasFieldAddr()) {
      LLVM_DEBUG(dbgs() << "Unsupported use of dope vector object: "
                           "Could not find addresses for all fields.\n");
      setInvalid();
      return;
    }
  }
  // Verify all the uses of the fields present were successfully analyzed.
  if (PtrAddr.getIsBottom() || ElementSizeAddr.getIsBottom() ||
      CodimAddr.getIsBottom() || FlagsAddr.getIsBottom() ||
      DimensionsAddr.getIsBottom()) {
    LLVM_DEBUG(dbgs() << "Unsupported use of dope vector object: Could "
                         "not analyze all fields.\n");

    setInvalid();
    return;
  }

  // If a field was found that corresponds to the Extent, Stride or
  // LowerBounds fields, reserve space for all of them, then collect all the
  // loads/stores that use those fields.
  if (ExtentBase || StrideBase || LowerBoundBase) {
    ExtentAddr.resize(Rank);
    StrideAddr.resize(Rank);
    LowerBoundAddr.resize(Rank);
    for (unsigned Dim = 0; Dim < Rank; ++Dim) {
      if (ExtentBase) {
        Value *Ptr = findPerDimensionArrayFieldPtr(*ExtentBase, Dim);
        if (Ptr) {
          DopeVectorFieldUse &ExtentField = ExtentAddr[Dim];
          ExtentField.addFieldAddr(Ptr);
          ExtentField.analyzeUses();
          if (ExtentField.getIsBottom()) {
            setInvalid();
            return;
          }
        }
      }
      if (StrideBase) {
        Value *Ptr = findPerDimensionArrayFieldPtr(*StrideBase, Dim);
        if (Ptr) {
          DopeVectorFieldUse &StrideField = StrideAddr[Dim];
          StrideField.addFieldAddr(Ptr);
          StrideField.analyzeUses();
          if (StrideField.getIsBottom()) {
            setInvalid();
            return;
          }
        }
      }
      if (LowerBoundBase) {
        Value *Ptr = findPerDimensionArrayFieldPtr(*LowerBoundBase, Dim);
        if (Ptr) {
          DopeVectorFieldUse &LBField = LowerBoundAddr[Dim];
          LBField.addFieldAddr(Ptr);
          LBField.analyzeUses();
          if (LBField.getIsBottom()) {
            setInvalid();
            return;
          }
        }
      }

      // For dope vector creation, we expect to find writes for all the fields
      if (ForCreation) {
        if (!ExtentAddr[Dim].hasFieldAddr() ||
            !StrideAddr[Dim].hasFieldAddr() ||
            !LowerBoundAddr[Dim].hasFieldAddr()) {
          LLVM_DEBUG(dbgs() << "Unsupported use of dope vector object: Could "
                               "not find addresses for all ranks.\n");
          setInvalid();
          return;
        }

        if (!ExtentAddr[Dim].getIsWritten() ||
            !StrideAddr[Dim].getIsWritten() ||
            !LowerBoundAddr[Dim].getIsWritten()) {
          LLVM_DEBUG(dbgs() << "Unsupported use of dope vector object: "
                               "Could not find writes for all ranks.\n");
          setInvalid();
          return;
        }
      }

      // If the dope vector is used only for the local function then all
      // the analysis process has been done and we can safely identify the
      // constant values that will be propagated.
      if (IsLocalDV) {
        ExtentAddr[Dim].identifyConstantValue();
        StrideAddr[Dim].identifyConstantValue();
        LowerBoundAddr[Dim].identifyConstantValue();
      }
    }
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DopeVectorAnalyzer::dump() const { print(dbgs()); }

void DopeVectorAnalyzer::print(raw_ostream &OS) const {
  OS << "DopeVectorAnalyzer: " << *DVObject << "\n";
  OS << "IsValid: " << (IsValid ? "true" : "false") << "\n";

  PtrAddr.print(OS, "PtrAddr:");
  ElementSizeAddr.print(OS, "ElementSize:");
  CodimAddr.print(OS, "Codim:");
  FlagsAddr.print(OS, "Flags:");
  DimensionsAddr.print(OS, "Dimensions:");
  for (unsigned Dim = 0; Dim < LowerBoundAddr.size(); ++Dim) {
    std::string DimStr = std::to_string(Dim);
    LowerBoundAddr[Dim].print(OS, "LowerBound" + DimStr);
  }
  for (unsigned Dim = 0; Dim < StrideAddr.size(); ++Dim) {
    std::string DimStr = std::to_string(Dim);
    StrideAddr[Dim].print(OS, "Stride" + DimStr);
  }

  for (unsigned Dim = 0; Dim < ExtentAddr.size(); ++Dim) {
    std::string DimStr = std::to_string(Dim);
    ExtentAddr[Dim].print(OS, "Extent" + DimStr);
  }
  OS << "\n";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

DopeVectorFieldType
DopeVectorAnalyzer::identifyDopeVectorField(const GEPOperator &GEP,
                                            uint64_t DopeVectorIndex) {
  assert(GEP.getSourceElementType()->isStructTy() && "Expected struct type");

  // Array index should always be zero.
  auto ArrayIdx = getConstGEPIndex(GEP, 1);
  if (!ArrayIdx || *ArrayIdx != 0)
    return DopeVectorFieldType::DV_Invalid;

  // If the dope vector index specified by the user is 1 then return
  // DopeVectorFieldType::DV_Invalid. Index 0 is reserved for the array
  // index and it's value should be 0.
  if (DopeVectorIndex == 1)
    return DopeVectorFieldType::DV_Invalid;

  // If DopeVectorIndex is greater than 0, then we need to compute the offset
  // of the rest of indices that access the fields of the dope vector.
  uint64_t DopeVectorOffSet = 0;
  if (DopeVectorIndex > 0)
    DopeVectorOffSet = DopeVectorIndex - 1;

  unsigned NumIndices = GEP.getNumIndices();
  if (NumIndices < 2 + DopeVectorOffSet || NumIndices > 4 + DopeVectorOffSet)
    return DopeVectorFieldType::DV_Invalid;

  // The address for the first 6 fields of the dope vector are accessed
  // directly with a GEP of the form:
  //     %field4 = getelementptr
  //               { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] },
  //               { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }*
  //               %"var$08", i64 0, i32 4
  if (NumIndices == 2 + DopeVectorOffSet) {
    auto FieldIdx = getConstGEPIndex(GEP, 2 + DopeVectorOffSet);
    assert(FieldIdx &&
           "Field index should always be constant for struct type");
    assert(FieldIdx
        <= static_cast<uint64_t>(DopeVectorFieldType::DV_PerDimensionArray) &&
        "expected dope vector to have a maximum of 7 fields");
    return static_cast<DopeVectorFieldType>(*FieldIdx);
  }

  // The per-dimension array elements may be accessed using either of the
  // following forms:
  //   %16 = getelementptr
  //         { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] },
  //         { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %2,
  //         i64 0, i32 6, i64 0
  //
  // or:
  //
  //   %14 = getelementptr { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64,
  //   i64 }] },
  //         { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %3,
  //         i64 0, i32 6, i64 0, i32 1
  //
  // For the first form, another GEP will follow to get the index from the
  // per-array dimension. For the second form, the field may be passed
  // directly to a subscript intrinsic.
  if (NumIndices == 3 + DopeVectorOffSet) {
    auto FieldIdx = getConstGEPIndex(GEP, 2 + DopeVectorOffSet);
    if (FieldIdx != static_cast<uint64_t>(DV_PerDimensionArray))
      return DopeVectorFieldType::DV_Invalid;

    // We only expect the GEP to use 0 for last index which corresponds to the
    // per-dimension array base, and then be followed by another GEP to get
    // the specific structure element.
    auto SubIdx = getConstGEPIndex(GEP, 3 + DopeVectorOffSet);
    assert(SubIdx && "Field index should always be constant for struct type");
    if (*SubIdx != 0)
      return DopeVectorFieldType::DV_Invalid;
    return DopeVectorFieldType::DV_PerDimensionArray;
  }

  assert(NumIndices == 4 + DopeVectorOffSet && "Only expected case 4 to be left");

  // The second form of access directly gets the address of the Lower Bound,
  // Stride or Extent field of the first array element.
  auto SubIdx = getConstGEPIndex(GEP, 4 + DopeVectorOffSet);
  assert(SubIdx && "Field index should always be constant for struct type");
  switch (*SubIdx) {
  default:
    return DopeVectorFieldType::DV_Invalid;
  case 0:
    return DopeVectorFieldType::DV_ExtentBase;
  case 1:
    return DopeVectorFieldType::DV_StrideBase;
  case 2:
    return DopeVectorFieldType::DV_LowerBoundBase;
  }
  return DopeVectorFieldType::DV_Invalid;
}

std::pair<GetElementPtrInst *, DopeVectorAnalyzer::FindResult>
DopeVectorAnalyzer::findPerDimensionArrayFieldGEP(GetElementPtrInst &GEP,
                              DopeVectorRankFields RankFieldType) {
  std::pair<GetElementPtrInst *, FindResult> InvalidResult = {nullptr,
                                                              FR_Invalid};
  unsigned int FieldNum;
  switch (RankFieldType) {
  case DVR_Extent:
    FieldNum = 0;
    break;
  case DVR_Stride:
    FieldNum = 1;
    break;
  case DVR_LowerBound:
    FieldNum = 2;
    break;
  }

  // Find the GEP that corresponds to the per-dimension element wanted. There
  // should only be one, if there are more, we do not support it.
  GetElementPtrInst *FieldGEP = nullptr;
  for (auto *U : GEP.users()) {
    if (auto *GEPU = dyn_cast<GetElementPtrInst>(U)) {
      if (GEPU->getNumIndices() != 2)
        return InvalidResult;

      auto ArIdx = getConstGEPIndex(*(cast<GEPOperator>(GEPU)), 1);
      if (!ArIdx || *ArIdx != 0)
        return InvalidResult;

      // Check that there is only one instance of field being searched for.
      auto FieldIdx = getConstGEPIndex(*(cast<GEPOperator>(GEPU)), 2);
      assert(FieldIdx && "Field index of struct must be constant");
      if (*FieldIdx == FieldNum) {
        if (FieldGEP)
          return InvalidResult;
        FieldGEP = GEPU;
      }
    } else {
      return InvalidResult;
    }
  }

  // No instances using field. Return a constructed value that holds a
  // nullptr, as a valid analysis result.
  if (!FieldGEP)
    return {nullptr, FR_Valid};

  return {FieldGEP, FR_Valid};
}

Value *
DopeVectorAnalyzer::findPerDimensionArrayFieldPtr(GetElementPtrInst &FieldGEP,
                                                  unsigned Dimension) {
  // Find the address element
  Instruction *Addr = nullptr;
  for (auto *U : FieldGEP.users()) {
    if (auto *Subs = dyn_cast<SubscriptInst>(U)) {
      auto *IdxVal = dyn_cast<ConstantInt>(Subs->getIndex());
      if (!IdxVal)
        return nullptr;
      if (IdxVal->getLimitedValue() == Dimension) {
        if (Addr)
          return nullptr;
        Addr = Subs;
      }
    } else {
      return nullptr;
    }
  }

  return Addr;
}

bool
DopeVectorAnalyzer::analyzeDopeVectorUseInFunction(const Function &F,
                                                   SubscriptInstSet
                                                      *SubscriptCalls) {
  LLVM_DEBUG({
    dbgs() << "\nDope vector collection for function: " << F.getName() << "\n"
           << *getDVObject() << "\n";
    dump();
  });

  // Verify that the dope vector fields are not written.
  if (checkMayBeModified()) {
    LLVM_DEBUG(dbgs() << "Dope vector fields modified in function: "
                      << F.getName() << "\n");
    return false;
  }

  // Check that the DV object was not forwarded to another function call. We
  // could allow this by analyzing all the uses within that function,
  // but we currently do not.
  if (getNumberCalledFunctions()) {
    LLVM_DEBUG(dbgs() << "Dope vector passed to another function within: "
                      << F.getName() << "\n");
    return false;
  }

  // Check that the array pointer does not escape to another memory location.
  SubscriptInstSet LocalSubscriptCalls;
  SubscriptInstSet *LocalSubscriptCallsCheck = SubscriptCalls ?
      &LocalSubscriptCalls : nullptr;
  if (!checkArrayPointerUses(LocalSubscriptCallsCheck)) {
    LLVM_DEBUG(dbgs() << "Array pointer address may escape from: "
                      << F.getName() << "\n");
    return false;
  }

  if (SubscriptCalls && !LocalSubscriptCalls.empty()) {
    // Check the stride value used in the subscript calls.
    if (!checkSubscriptStrideValues(LocalSubscriptCalls)) {
      LLVM_DEBUG(dbgs() << "Subscript call with unsupported stride in: "
                        << F.getName() << "\n");
      return false;
    }

    // Save the set of subscript calls that use the dope vector for
    // profitability analysis.
    SubscriptCalls->insert(LocalSubscriptCalls.begin(),
      LocalSubscriptCalls.end());
  }

  // If there was a store of the dope vector into an uplevel variable, check
  // the uses of the uplevel variable.
  UplevelDVField Uplevel = getUplevelVar();
  SmallPtrSet<const Function *, 16> Visited;
  if (Uplevel.first && !analyzeUplevelVar(getRank(), SubscriptCalls, F, Uplevel,
                                          getDVObject(), Visited))
    return false;
  return true;
}

bool
DopeVectorAnalyzer::checkSubscriptStrideValues(const SubscriptInstSet
                                                    &SubscriptCalls) {
  SmallVector<SmallPtrSet<Value *, 4>, FortranMaxRank> StrideLoads;

  // Function to check the stride value used in an instruction that is a
  // subscript call or holds the result of a subscript call. This is to
  // verify that the value used for the stride was identified as one of the
  // expected values when information was collected for the dope vector.
  //
  // If the instruction is a subscript call, then verify that the value used for
  // the stride operand is a member of the \p StrideLoads set. If the subscript
  // call is not the final Dimension, then recurse to check the stride value
  // for the next level subscript call.
  // If the instruction is a PHINode or Select instruction that holds the result
  // of a subscript call, then check all the users to find where the subscript
  // result is used for next dimension.
  std::function<bool(const SmallVectorImpl<SmallPtrSet<Value *, 4>> &,
                     const Instruction &, uint32_t,
                     SmallPtrSetImpl<const Instruction *> &)>
      CheckCall =
          [&CheckCall](
              const SmallVectorImpl<SmallPtrSet<Value *, 4>> &StrideLoads,
              const Instruction &I, uint32_t Dimension,
              SmallPtrSetImpl<const Instruction *> &Visited) -> bool {
    if (!Visited.insert(&I).second)
      return true;

    if (auto *Subs = dyn_cast<SubscriptInst>(&I)) {
      Value *StrideOp = Subs->getStride();
      if (!StrideLoads[Dimension].count(StrideOp))
        return false;

      if (Dimension == 0)
        return true;
    }

    for (auto *UU : I.users())
      if (auto *Subs2 = dyn_cast<SubscriptInst>(UU)) {
        if (!CheckCall(StrideLoads, *Subs2, Dimension - 1, Visited))
          return false;
      } else if (isa<SelectInst>(UU) || isa<PHINode>(UU)) {
        const Instruction *I2 = dyn_cast<Instruction>(UU);
        if (!CheckCall(StrideLoads, *I2, Dimension, Visited))
          return false;
      } else {
        return false;
      }

    return true;
  };

  // For each dimension of the variable, get the set of objects that hold the
  // value for the stride loaded from the dope vector object.
  for (unsigned Dim = 0; Dim < getRank(); ++Dim) {
    if (!hasStrideField(Dim))
      return false;

    const DopeVectorFieldUse &StrideField = getStrideField(Dim);
    StrideLoads.push_back(SmallPtrSet<Value *, 4>());
    auto &LoadSet = StrideLoads.back();
    bool Valid = getAllValuesHoldingFieldValue(StrideField, LoadSet);
    if (!Valid)
      return false;
  }

  // Check all the subscript calls to ensure the stride value comes from the
  // dope vector.
  SmallPtrSet<const Instruction*, 16> Visited;
  for (auto *Subs : SubscriptCalls)
    if (!CheckCall(StrideLoads, *Subs, getRank() - 1, Visited))
      return false;

  return true;
}

// Given an Value and LLVM Type that was already identified as dope vector then
// construct a DopeVectorInfo.
DopeVectorInfo::DopeVectorInfo(Value *DVObject, Type *DVType,
                               bool AllowMultipleFieldAddresses,
                               bool IsCopyDopeVector) :
    DVObject(DVObject), IsCopyDopeVector(IsCopyDopeVector) {

  assert(DVType->isStructTy() && DVType->getStructNumElements() == 7 &&
         DVType->getContainedType(DopeVectorFieldType::DV_PerDimensionArray)
               ->isArrayTy() && "Invalid type for dope vector object");
  // The rank of the dope vector can be determined using the array length of
  // array that is the last field of the dope vector.
  Rank = DVType->getContainedType(DopeVectorFieldType::DV_PerDimensionArray)
               ->getArrayNumElements();

  AnalysisRes = DopeVectorInfo::AnalysisResult::AR_Top;
  LLVMDVType = cast<StructType>(DVType);
  ConstantsPropagated = false;

  ExtentAddr.resize(Rank);
  StrideAddr.resize(Rank);
  LowerBoundAddr.resize(Rank);
  ElementSizeAddr.setRequiresSingleNonNullValue();
  CodimAddr.setRequiresSingleNonNullValue();

  if (AllowMultipleFieldAddresses) {
    PtrAddr.setAllowMultipleFieldAddresses();
    ElementSizeAddr.setAllowMultipleFieldAddresses();
    CodimAddr.setAllowMultipleFieldAddresses();
    FlagsAddr.setAllowMultipleFieldAddresses();
    DimensionsAddr.setAllowMultipleFieldAddresses();

    for (unsigned long I = 0; I < Rank; I++) {
      ExtentAddr[I].setAllowMultipleFieldAddresses();
      StrideAddr[I].setAllowMultipleFieldAddresses();
      LowerBoundAddr[I].setAllowMultipleFieldAddresses();
    }
  }
}

// Given a dope vector field type, return the DopeVectorFieldUse corresponding
// to that field. If the field is DV_ExtentBase, DV_StrideBase or
// DV_LowerBoundBase then the ArrayEntry is needed. If the field is
// DV_Reserved, then return nullptr.
DopeVectorFieldUse* DopeVectorInfo::getDopeVectorField(
    DopeVectorFieldType DVFieldType, uint64_t ArrayEntry) {

  // Return the DopeVectorField use corresponding to the array entry
  auto GetArrayEntry = [&, ArrayEntry](SmallVectorImpl<DopeVectorFieldUse>
                                       &DVFieldArray) -> DopeVectorFieldUse* {

    assert(ArrayEntry < DVFieldArray.size() && "Trying to access an out of "
        "bounds entry in the per dimension array");

    return &(DVFieldArray[ArrayEntry]);
  };

  switch(DVFieldType) {
    case DopeVectorFieldType::DV_ArrayPtr:
      return &PtrAddr;
    case DopeVectorFieldType::DV_ElementSize:
      return &ElementSizeAddr;
    case DopeVectorFieldType::DV_Codim:
      return &CodimAddr;
    case DopeVectorFieldType::DV_Flags:
      return &FlagsAddr;
    case DopeVectorFieldType::DV_Dimensions:
      return &DimensionsAddr;
    case DopeVectorFieldType::DV_Reserved:
      return nullptr;
    case DopeVectorFieldType::DV_PerDimensionArray:
      return &DimensionsAddr;
    case DopeVectorFieldType::DV_ExtentBase:
      return GetArrayEntry(ExtentAddr);
    case DopeVectorFieldType::DV_StrideBase:
      return GetArrayEntry(StrideAddr);
    case DopeVectorFieldType::DV_LowerBoundBase:
      return GetArrayEntry(LowerBoundAddr);
    default:
      llvm_unreachable("Trying to collect field from the dope "
                       "vector that is out of bounds");
  }

  return nullptr;
}

// This function checks if the information for all the fields in the dope
// vector was collected correctly. If CopyFromPtr is provided then we are
// going to check if a write to a field that should not be written is
// a copy. For example, assume that %DV1 and %DV2 are two dope vectors,
// then a field copy will look as follow:
//
//   %1 = getelementptr inbounds %"__DTRT_QNCA_a0$double*$rank3$",
//           %"__DTRT_QNCA_a0$double*$rank3$"* %DV1, i64 0, i32 0
//   %2 = load double*, double** %1
//   %3 = getelementptr inbounds %"__DTRT_QNCA_a0$double*$rank3$",
//           %"__DTRT_QNCA_a0$double*$rank3$"* %DV2, i64 0, i32 0
//   store double* %2, double** %3
//
// If we are validating %DV2 then CopyFromPtr will be %DV1, since %DV2 is
// copying information from it.
void DopeVectorInfo::validateDopeVector(Value *CopyFromPtr) {

  // We are going to follow the same principle as global constant propagation.
  // If there is only one store instruction and the global variable is loaded
  // anywhere, then the global variable should be initialized first (store
  // must happen before the load). Else, if the store instruction wasn't
  // executed it means that the user code didn't properly initialize the
  // global variable. In this case the load instruction is loading garbage,
  // which is an undefined behavior.
  //
  // In this case, the user is accessing an array, therefore it must be
  // allocated before accessing any information, if not this is undefined
  // behavior. The dope vector is used to access information in the array
  // therefore the information for the dope vector must be initialized
  // first and the array it must be allocated. If there is one store
  // instruction for a dope vector field and that store instruction will
  // always execute if the call to allocate the array executes, then it means
  // that the dope vector field will be initialized when the array is
  // allocated. Any load to the dope vector field will use initialized data
  // because the array was allocated (store must happen before the load).
  // If there is an use of the dope vector field without allocating the array
  // first then there is an access to an unallocated array, which is
  // undefined behavior.
  auto StoreHappensWithAllocation =
      [this](DopeVectorFieldUse &Field) -> bool {
    if (!Field.getIsSingleValue())
      return false;

    // Trying to store into a dope vector without alloc-site.
    // NOTE: We may want to check this condition in the future. There is a
    // chance that a global dope vector is a copy of another global dope
    // vector. In that case it won't have an allocation site, but a
    // combination of load and store instructions that are handling pointers.
    // In that case we need to prove that both dope vectors point to the same
    // data and the information is constant across the whole program.
    if (AllocSites.empty())
      return false;

    StoreInst *SI = *Field.stores().begin();
    Instruction *AllocSiteInst = cast<Instruction>(AllocSites[0]);

    // If the store instruction is in the same function as the alloc-site,
    // then it means that both instructions might execute. A store before
    // an allocation is an undefined behavior, therefore all we need to
    // prove is that there is only one store and one alloc-site.
    //
    // NOTE-1: This is conservative. We may need to extend this in the future
    // to address the following case:
    //
    //   1) Function "foo" can store to information to the dope vector fields,
    //      then it calls "bar" which allocates the array. If there is no
    //      branching between the store instructions and the call to "bar",
    //      nor between the entry block in "bar" and the call to the alloc
    //      function, then the store instruction and the alloc site will
    //      always execute (foo -> bar -> alloc).
    //
    // NOTE-2: At this point, we are expecting only one alloc site per
    // nested dope vector.  After merging, there may be multiple alloc sites.
    if ((AllocSites.size() != 1) ||
       (SI->getFunction() != AllocSiteInst->getFunction()))
      return false;

    return true;
  };

  // Invalidate the data collected if at least one field is Bottom,
  // if writing or reading into the field is not allowed, or if
  // it didn't pass the store-alloc test.
  auto ValidateDopeVectorField =
      [&StoreHappensWithAllocation, this](DopeVectorFieldUse &Field,
                                          bool ComputeConstant,
                                          bool AnyWriteAllowed,
                                          bool NullWriteAllowed,
                                          bool ReadAllowed,
                                          Value *CopyFromPtr) -> bool {
    if (Field.getIsBottom())
      return false;

    if (!AnyWriteAllowed && Field.getIsWritten() &&
        !Field.getIsOnlyWrittenWithNull() && !IsCopyDopeVector &&
        !Field.getIsSingleValue() &&
        !IsCopyFromDVField(*Field.stores().begin(), CopyFromPtr)) {
      AnalysisRes = DopeVectorInfo::AnalysisResult::AR_WriteIllegality;
      return false;
    }

    if (!NullWriteAllowed && Field.getIsOnlyWrittenWithNull()) {
      AnalysisRes = DopeVectorInfo::AnalysisResult::AR_WriteIllegality;
      return false;
    }

    if (!ReadAllowed && Field.getIsRead()) {
      AnalysisRes = DopeVectorInfo::AnalysisResult::AR_ReadIllegality;
      return false;
    }

    if (ComputeConstant)
      Field.identifyConstantValue();

    // If we found the constant, then we need to make sure that the store
    // executes every time the memory is allocated.
    if (Field.getConstantValue() &&
        !StoreHappensWithAllocation(Field)) {
      AnalysisRes = DopeVectorInfo::AnalysisResult::AR_AllocStoreIllegality;
      return false;
    }

    return true;
  };

  if (AnalysisRes == DopeVectorInfo::AnalysisResult::AR_Invalid)
    return;

  bool PassValidation = true;

  // Pointer address should not be written, only allocated and read
  PassValidation &= ValidateDopeVectorField(PtrAddr,
                                            false /* ComputeConstant */,
                                            false /* AnyWriteAllowed */,
                                            true /* NullWriteAllowed */,
                                            true /* ReadAllowed */,
                                            CopyFromPtr);

  // The element size and co-dimension should have a single non-null
  // value that is written and can also be read.
  // The flags and dimensions should be written but not read.
  // These fields is for storing information that the compiler can use
  // for analysis.
  PassValidation &= ValidateDopeVectorField(ElementSizeAddr,
                                            true /* ComputeConstant */,
                                            true /* AnyWriteAllowed */,
                                            true /* NullWriteAllowed */,
                                            true /* ReadAllowed */,
                                            CopyFromPtr);

  PassValidation &= ValidateDopeVectorField(CodimAddr,
                                            true /* ComputeConstant */,
                                            true /* AnyWriteAllowed */,
                                            true /* NullWriteAllowed */,
                                            true /* ReadAllowed */,
                                            CopyFromPtr);

  // NOTE: The FE can generate a load to the flags field, then do some
  // operations and followed by a store to the same field. In summary,
  // there will be a read before a write for this field. We are not
  // going to compute any constant information related to the flags field
  // until this issue is resolved. On the other hand, the field should be
  // only used by load and/or store instructions. Any other use should
  // invalidate the information for the current dope vector.
  PassValidation &= ValidateDopeVectorField(FlagsAddr,
                                            false /* ComputeConstant */,
                                            true /* AnyWriteAllowed */,
                                            true /* NullWriteAllowed */,
                                            true /* ReadAllowed */,
                                            CopyFromPtr);

  PassValidation &= ValidateDopeVectorField(DimensionsAddr,
                                            true /* ComputeConstant */,
                                            true /* AnyWriteAllowed */,
                                            true /* NullWriteAllowed */,
                                            false /* ReadAllowed */,
                                            CopyFromPtr);

  // This is the actual information of the dope vector. The extent,
  // stride and lower bounds fields can be read and written. If the
  // array pointer is not read, then none of these fields should be
  // read.
  // NOTE: There is a chance to extend this analysis for a possible
  // dead global dope vector elimination (CMPLRLLVM-28117).
  bool ReadAllowed = PtrAddr.getIsRead();
  for (uint64_t I = 0; I < Rank; I++) {
    PassValidation &= ValidateDopeVectorField(ExtentAddr[I],
                                              true /* ComputeConstant */,
                                              true /* AnyWriteAllowed */,
                                              true /* NullWriteAllowed */,
                                              ReadAllowed,
                                              CopyFromPtr);

    PassValidation &= ValidateDopeVectorField(StrideAddr[I],
                                              true /* ComputeConstant */,
                                              true /* AnyWriteAllowed */,
                                              true /* NullWriteAllowed */,
                                              ReadAllowed,
                                              CopyFromPtr);

    PassValidation &= ValidateDopeVectorField(LowerBoundAddr[I],
                                              true /* ComputeConstant */,
                                              true /* AnyWriteAllowed */,
                                              true /* NullWriteAllowed */,
                                              ReadAllowed,
                                              CopyFromPtr);
  }

  if (PassValidation)
    AnalysisRes = DopeVectorInfo::AnalysisResult::AR_Pass;
  else if (AnalysisRes == DopeVectorInfo::AnalysisResult::AR_Top)
    AnalysisRes = DopeVectorInfo::AnalysisResult::AR_Invalid;
}

void DopeVectorInfo::validateAllocSite() {
  if (getAnalysisResult() != DopeVectorInfo::AnalysisResult::AR_Pass)
    return;
  if (!AllocSites.empty())
    return;
  AnalysisRes = DopeVectorInfo::AnalysisResult::AR_NoAllocSite;
}

void DopeVectorInfo::validateSingleNonNullValue(DopeVectorFieldType DVFT) {
  if (getAnalysisResult() != DopeVectorInfo::AnalysisResult::AR_Pass)
    return;
  DopeVectorFieldUse *DVFU = getDopeVectorField(DVFT);
  assert(DVFU->getRequiresSingleNonNullValue() &&
    "RequiresSingleNonNullValue expected");
  if (!DVFU->getIsSingleNonNullValue())
    AnalysisRes = DopeVectorInfo::AnalysisResult::AR_NoSingleNonNullValue;
}

// Return true if all pointers that access the dope vector fields were
// collected correctly by tracing the users of V, else return false. V
// represents a pointer to a nested dope vector (could come from a BitCast,
// GEP or an Argument). If AllowCheckForAllocSite is true then allow to
// trace the BitCast instructions as allocation sites, else any BitCast
// found is treated as an illegal access and the function will return false.
bool GlobalDopeVector::collectNestedDopeVectorFieldAddress(
    NestedDopeVectorInfo *NestedDV, Value *V,
    std::function<const TargetLibraryInfo &(Function &F)> &GetTLI,
    SetVector<Value *> &ValueChecked, const DataLayout &DL, bool ForDVCP,
    bool AllowCheckForAllocSite) {

  // Return true if the users of the input GEP are subscript instructions.
  // The subscript instructions represents the access to the per dimension
  // array entry. This function is only used to collect data for the
  // extent, stride and lower bound.
  auto CollectAccessForExtentStrideAndLB = [NestedDV](GEPOperator *GEP,
      DopeVectorFieldType DVFieldType) -> bool {

    assert(GEP && "Trying to collect data for extent, stride or lower "
                 "bound without providing the access");
    assert(DVFieldType > DopeVectorFieldType::DV_PerDimensionArray &&
           DVFieldType < DopeVectorFieldType::DV_Invalid &&
           "Trying to collect data from extent, stride or lower bound "
           "without providing the proper field type");

    // All users should be array subscripts
    for (auto *ArrayAccess : GEP->users()) {
      auto *SI = dyn_cast<SubscriptInst>(ArrayAccess);
      if (!SI)
        return false;

      if (!isa<ConstantInt>(SI->getStride()) ||
          !isa<ConstantInt>(SI->getLowerBound()) ||
          !isa<ConstantInt>(SI->getIndex()))
        return false;

      // The index should be less than the rank
      ConstantInt *SIIndex = cast<ConstantInt>(SI->getIndex());
      unsigned long SubScriptIndex = SIIndex->getZExtValue();
      if (SubScriptIndex >= NestedDV->getRank())
        return false;

      auto *DVField = NestedDV->getDopeVectorField(DVFieldType,
                                                   SubScriptIndex);
      if (DVField->getIsBottom())
        return false;

      DVField->addFieldAddr(SI, NestedDV->getNotForDVCP());
    }
    return true;
  };

  // If the input GEP is accessing the per dimension array field (field 6)
  // then we need to go through the users and collect the extent, stride
  // and lower boound accesses.
  auto CollectAccessForPerDimensionArray =
      [&CollectAccessForExtentStrideAndLB](GEPOperator *GEP) -> bool {

    for (auto *DimUser : GEP->users()) {

      // The users of the GEP will be another GEP accessing the
      // extent, stride or lower bound
      auto GEPUser = dyn_cast<GEPOperator>(DimUser);
      if (!GEPUser)
        return false;

      // This case handles when the GEP has 2 indices, more than 2
      // should be handled to a direct access of the extent, stride
      // or lower bound
      if (!GEPUser->hasAllConstantIndices() ||
        GEPUser->getNumIndices() != 2)
        return false;

      // Array index should be 0
      auto ArrayIdx = getConstGEPIndex(*GEPUser, 1);
      if (!ArrayIdx || *ArrayIdx != 0)
        return false;

      // Identify if GEPUser is accessing the extent, stride or
      // lower bound
      auto Idx = getConstGEPIndex(*GEPUser, 2);
      if (!Idx)
        return false;

      // We could add this as part of the switch-and-case below, but clang
      // doesn't like adding the default case when all cases in the enum
      // are covered.
      if (*Idx >
          (uint64_t)DopeVectorAnalyzer::DopeVectorRankFields::DVR_LowerBound)
        return false;

      DopeVectorFieldType DVFieldType;
      switch((DopeVectorAnalyzer::DopeVectorRankFields)*Idx) {
        case DopeVectorAnalyzer::DopeVectorRankFields::DVR_Extent:
          DVFieldType = DopeVectorFieldType::DV_ExtentBase;
          break;
        case DopeVectorAnalyzer::DopeVectorRankFields::DVR_Stride:
          DVFieldType = DopeVectorFieldType::DV_StrideBase;
          break;
        case DopeVectorAnalyzer::DopeVectorRankFields::DVR_LowerBound:
          DVFieldType = DopeVectorFieldType::DV_LowerBoundBase;
          break;
      }

      // Collect the uses of the extent, stride or lower bound
      if (!CollectAccessForExtentStrideAndLB(GEPUser, DVFieldType))
        return false;
    }
    return true;
  };

  //
  // Return 'true' if 'F' which is called from 'CB' passing down a dope vector
  // argument 'ArgNo' is also called from other callsites which could pass
  // down a different dope vector at that argument. In this case, we cannot
  // use this dope vector analysis to determine constants that argument
  // within 'F', but the escape analysis for transpose is still valid.
  //
  auto HasBadSideCalls = [](CallBase *CB, Function *F,
                            uint64_t ArgNo) -> bool {
    for (User *U : F->users()) {
      if (auto LCB = dyn_cast<CallBase>(U)) {
        if (LCB == CB)
          continue;
        if (LCB->getArgOperand(ArgNo) == CB->getArgOperand(ArgNo))
          continue;
        return true;
      } else {
        return true;
      }
    }
    return false;
  };

  // Return true if we can collect the dope vector field accesses
  // in the input call. This function will find which argument is
  // Val in the input Call, check that the argument attributes are
  // set correctly and recurse collectNestedDopeVectorFieldAddress
  // where the argument now represents a pointer to the nested dope
  // vector.
  //
  // NOTE: AllowCheckForAllocSite will be false in this case since
  // a function should not allocate a dope vector that is passed as
  // pointer. This conservative, we may want to relax this in the future.
  auto CollectAccessFromCall = [&HasBadSideCalls, this](
      CallBase *Call, Value *Val,
      std::function<const TargetLibraryInfo &(Function &F)> &GetTLI,
      const DataLayout &DL, NestedDopeVectorInfo *NestedDV, bool ForDVCP,
      SetVector<Value *> &ValueChecked) -> bool {

    // Indirect calls or declarations aren't allowed
    // NOTE: In case of declarations, we may be able to mark the
    // libfuncs with some attributes that won't stop the data
    // collection.
    if (Call->isIndirectCall())
      return false;

    auto F = dyn_cast<Function>(Call->getCalledOperand()->stripPointerCasts());
    if (!F || F->isDeclaration())
      return false;

    // Find the actual argument
    uint64_t NumArgs = Call->arg_size();
    uint64_t ArgNo = 0;
    bool ArgFound = false;
    for (uint64_t I = 0; I < NumArgs; I++) {
      if (Call->getArgOperand(I) == Val) {

        if (ArgFound)
          return false;

        ArgNo = I;
        ArgFound = true;
      }
    }

    if (!ArgFound)
      return false;

    // Collect the formal argument
    auto *Arg = F->getArg(ArgNo);
    if (ValueChecked.contains(Arg))
      return true;

    // Should be marked as "ptrnoalias", "assume_shape", "readonly" and
    // "noalias"
    if (!Arg->hasAttribute("ptrnoalias") ||
        !Arg->hasAttribute("assumed_shape") ||
        !Arg->onlyReadsMemory() ||
        !Arg->hasNoAliasAttr())
      return false;

    bool RestoreValue = NestedDV->getNotForDVCP();
    bool NewValue = RestoreValue || ForDVCP && !NestedDV->getNotForDVCP() &&
        HasBadSideCalls(Call, F, ArgNo);
    NestedDV->setNotForDVCP(NewValue);
    bool RV = collectNestedDopeVectorFieldAddress(NestedDV, Arg, GetTLI,
        ValueChecked, DL, ForDVCP, false /* AllowCheckForAllocSite */);
    NestedDV->setNotForDVCP(RestoreValue);
    return RV;
  };

  // Return true if the input GEP is used only by a BitCast for
  // data allocation
  auto GetCallForAllocation = [this](GEPOperator *GEP,
      std::function<const TargetLibraryInfo &(Function &F)> &GetTLI)
      -> CallBase* {

    if (!GEP->hasOneUser())
      return nullptr;

    auto *BC = dyn_cast<BitCastInst>(GEP->user_back());
    if (!BC)
      return nullptr;

    // NOTE: Most likely we will need to adjust this BitCast when opaque
    // pointers are available.
    return castingUsedForDataAllocation(BC, GetTLI);
  };

  // Return true if the input GEP is accessing a dope vector field and store
  // it. Special cases:
  //
  //   * DV_ArrayPtr: GEP could be used for allocate the array
  //
  //   * DV_PerDimensionArray: We need to trace if the GEP is used to access
  //         the extent, stride or lower bound
  //
  //   * DV_ExtentBase, DV_StrideBase or DV_LowerBoundBase: The GEP directly
  //         accesses the extent, stride or lower bound, collect the accesses
  auto CollectAccessFromGEP = [NestedDV, AllowCheckForAllocSite,
      &CollectAccessForPerDimensionArray, &CollectAccessForExtentStrideAndLB,
      &GetCallForAllocation] (GEPOperator *GEP, uint64_t DopeVectorIndex,
      std::function<const TargetLibraryInfo &(Function &F)> &GetTLI) -> bool {

    DopeVectorFieldType DVFieldType =
        DopeVectorAnalyzer::identifyDopeVectorField(*GEP, DopeVectorIndex);

    if (DVFieldType >= DopeVectorFieldType::DV_Invalid)
      return false;

    if (DVFieldType == DopeVectorFieldType::DV_Reserved)
      return true;

    // There is a chance that the field address 0 is used for allocating
    // the array
    if (DVFieldType == DopeVectorFieldType::DV_ArrayPtr) {
      if (auto *Call = GetCallForAllocation(GEP, GetTLI)) {
        if (!AllowCheckForAllocSite)
          return false;
        NestedDV->addAllocSite(Call);
        return true;
      }
    }

    // The GEP is used to access the array pointer, element size,
    // codimension, flags
    if (DVFieldType < DopeVectorFieldType::DV_PerDimensionArray) {
      auto *DVField = NestedDV->getDopeVectorField(DVFieldType);
      if (DVField->getIsBottom())
        return false;
     DVField->addFieldAddr(cast<Value>(GEP), NestedDV->getNotForDVCP());
    } else if (DVFieldType == DopeVectorFieldType::DV_PerDimensionArray) {
      // Check the accesses through the per dimension array
      if(!CollectAccessForPerDimensionArray(GEP))
        return false;
    } else {
      // If we reach this point then it means that it is a direct access to the
      // extent, stride and lower bound.
      if (!CollectAccessForExtentStrideAndLB(GEP, DVFieldType))
        return false;
    }
    return true;
  };

  // Return true if the input GEP is used for a accessing the dope vector
  // fields through a structure that contains a dope vector as field. For
  // example:
  //
  //    %92 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE",
  //          %"ARR_MOD$.btT_TESTTYPE"* %78, i64 0, i32 1, i32 6, i64 0, i32 1
  //
  // In the example above, assume that the type %"ARR_MOD$.btT_TESTTYPE" is a
  // structure where field 1 is a dope vector. The GEP %92 basically is
  // accessing the stride in the dope vector that is field 1 of
  // %"ARR_MOD$.btT_TESTTYPE".
  auto IsGEPUsedForAccessingField = [NestedDV](GEPOperator *GEP) {
    if (GEP->getNumIndices() < 3)
      return false;

    if (!GEP->hasAllConstantIndices())
      return false;

    auto ArrayIdx = getConstGEPIndex(*GEP, 1);
    if (!ArrayIdx || *ArrayIdx != 0)
      return false;

    // NOTE: This check is conservative, for now it accepts one level of nested
    // dope vector. Therefore index 2 in the GEP represents the access to the
    // dope vector.
    auto NestedDVIndx = getConstGEPIndex(*GEP, 2);
    if (!NestedDVIndx || *NestedDVIndx != NestedDV->getFieldNum())
      return false;

    // Find if the rest of the indices are accessing a dope vector field
    DopeVectorFieldType DVFieldType =
        DopeVectorAnalyzer::identifyDopeVectorField(*GEP, 2);

    return DVFieldType < DopeVectorFieldType::DV_Invalid;
  };

  //
  // Return 'true' if 'GEPO' stores a null value to a dope vector field.
  // Such stores are generally the product of a 'nullify' initialiation and
  // can be ignored for the purposes of dope vector analysis.
  //
  auto IsNullStoreToDVField = [](GEPOperator *GEPO) -> bool {
    if (GEPO->getNumIndices() != 2)
      return false;
    auto CI1 = dyn_cast<ConstantInt>(GEPO->getOperand(1));
    if (!CI1 || !CI1->isZero())
      return false;
    auto CI2 = dyn_cast<ConstantInt>(GEPO->getOperand(2));
    if (!CI2)
      return false;
    if (CI2->getZExtValue() > DV_PerDimensionArray)
      return false;
    if (!GEPO->hasOneUser())
      return false;
    auto SI = dyn_cast<StoreInst>(GEPO->user_back());
    if (!SI || SI->getPointerOperand() != GEPO)
      return false;
    auto CV = dyn_cast<Constant>(SI->getValueOperand());
    if (!CV || !CV->isNullValue())
      return false;
    return true;
  };

  //
  // Return 'true' if 'BC' is a bit cast between two identical dope vector
  // types.
  //
  auto IsNullDVBitCast = [](BitCastOperator *BC,
                            const DataLayout &DL) -> bool {
    uint32_t SArrayRank = 0;
    uint32_t DArrayRank = 0;
    Type *SElementType = nullptr;
    Type *DElementType = nullptr;
    auto STy = BC->getSrcTy();
    if (!STy->isPointerTy())
      return false;
    // Will need to be updated for opaque pointers.
    auto SPTy = STy->getNonOpaquePointerElementType();
    if (!isDopeVectorType(SPTy, DL, &SArrayRank, &SElementType))
      return false;
    auto DTy = BC->getDestTy();
    if (!DTy->isPointerTy())
      return false;
    // Will need to be updated for opaque pointers.
    auto DPTy = DTy->getNonOpaquePointerElementType();
    if (!isDopeVectorType(DPTy, DL, &DArrayRank, &DElementType))
      return false;

    // If typed pointers are enabled then SElementType and DElementType
    // shouldn't be nullptr.
    if (DPTy->getContext().supportsTypedPointers())
      assert((SElementType && DElementType) &&
             "Pointer address not collected when typed pointers are enabled");

    return SArrayRank == DArrayRank && SElementType == DElementType;
  };

  //
  // Return 'true' if User 'U' is a valid Use of 'V', which is a pointer
  // to a dope vector. 'ValueChecked' is used to avoid repeatedly analyzing
  // a Value that has already been analyzed.
  //
  std::function<bool(Value *V, User *U, const DataLayout &DL,
                NestedDopeVectorInfo *NestedDV, bool ForDVCP,
                SetVector<Value *> &ValueChecked)>
      GoodDVPUser = [&, this](Value *V, User *U,
                        const DataLayout &DL,
                        NestedDopeVectorInfo *NestedDV, bool ForDVCP,
                        SetVector<Value *> &ValueChecked) -> bool {
    // GEP should be accessing dope vector fields
    if (auto *GEP = dyn_cast<GEPOperator>(U)) {
      if (!CollectAccessFromGEP(GEP, 0, GetTLI))
        return false;
    } else if (auto *BC = dyn_cast<BitCastOperator>(U)) {
      // BitCast should be only used for allocating data
      if (IsNullDVBitCast(BC, DL)) {
         for (User *UU : BC->users())
           if (!GoodDVPUser(BC, UU, DL, NestedDV, ForDVCP, ValueChecked))
             return false;
         return true;
      }
      if (!AllowCheckForAllocSite)
        return false;
      CallBase *Call = castingUsedForDataAllocation(BC, GetTLI);
      if (!Call)
        return false;
      NestedDV->addAllocSite(Call);
    } else if (CallBase *Call = dyn_cast<CallBase>(U)) {
      // Calls should only load data
      if (!CollectAccessFromCall(Call, V, GetTLI, DL, NestedDV, ForDVCP,
                                 ValueChecked))
        return false;
    } else {
      return false;
    }
    return true;
  };

  if (!V)
    return false;

  if (!ValueChecked.insert(V))
    return true;

  // If the input Value is a GEP with more than 2 indices then there is a
  // chance that the GEP represents an access to a structure with
  // a field that is a dope vector (nested dope vector) and a field in
  // the dope vector.
  auto *GEPOp = dyn_cast<GEPOperator>(V);
  if (GEPOp && IsGEPUsedForAccessingField(GEPOp) &&
      (IsNullStoreToDVField(GEPOp) ||
      CollectAccessFromGEP(GEPOp, 2, GetTLI)))
    return true;

  // If we reach this point then it means that V represents a pointer to
  // a dope vector. Go through the users of V to identify how the fields of
  // the dope vector are used.
  for (auto *U : V->users()) {
    if (ValueChecked.contains(U))
      continue;
    if (!GoodDVPUser(V, U, DL, NestedDV, ForDVCP, ValueChecked))
      return false;
  }

  return true;
}

// Identify subscripts accessing the PtrAddr of the dope vector
// and place them in 'SIS'.
void DopeVectorInfo::identifyPtrAddrSubs(SubscriptInstSet &SIS) {
  for (unsigned I = 0; I < PtrAddr.numFieldAddrs(); ++I)
    for (User *U : PtrAddr.getFieldAddr(I)->users())
      if (auto *LI = dyn_cast<LoadInst>(U))
        for (User *W : LI->users())
          if (auto SI = dyn_cast<SubscriptInst>(W))
            if (SI && SI->getPointerOperand() == LI)
              SIS.insert(SI);
}

// Analyze that all the dope vector fields are used to load and store data
void NestedDopeVectorInfo::analyzeNestedDopeVector() {

  PtrAddr.analyzeUses();
  ElementSizeAddr.analyzeUses();
  CodimAddr.analyzeUses();
  FlagsAddr.analyzeUses();
  DimensionsAddr.analyzeUses();

  for (unsigned long I = 0; I < Rank; I++) {
    ExtentAddr[I].analyzeUses();
    StrideAddr[I].analyzeUses();
    LowerBoundAddr[I].analyzeUses();
  }

  // If the current dope vector is a copy dope vector then collect the
  // base from VBase.
  Value *CopyFromPtr = nullptr;
  if (IsCopyDopeVector)
    CopyFromPtr = VBase;

  // Load and stores collected, now validate the dope vector
  validateDopeVector(CopyFromPtr);
}

// Return true if the input BitCast operator is used for allocation
bool GlobalDopeVector::collectAndAnalyzeAllocSite(BitCastOperator *BC) {
  if (!BC || GlobalDVInfo->hasAllocSite())
    return false;

  CallBase *Call = castingUsedForDataAllocation(BC, GetTLI);

  if (!Call)
    return false;

  GlobalDVInfo->addAllocSite(Call);
  return true;
}

// Return true if the input GEPOperator is accessing a dope vector field,
// collect the uses and analyze that there is no ilegal use for it. Else
// return false.
bool
GlobalDopeVector::collectAndAnalyzeGlobalDopeVectorField(GEPOperator *GEP) {

  if (!GEP)
    return false;

  // Check which dope vector field is being accessed and
  // collect the information related to the load and stores
  DopeVectorFieldType DVFieldType =
      DopeVectorAnalyzer::identifyDopeVectorField(*GEP);

  if (DVFieldType >= DopeVectorFieldType::DV_Invalid)
    return false;

  if (DVFieldType == DopeVectorFieldType::DV_Reserved)
    return true;

  // If the field type is DV_PerDimensionArray or lower then check
  // the load and stores (analyzeUses).
  if (DVFieldType < DopeVectorFieldType::DV_PerDimensionArray) {
    DopeVectorFieldUse *DVField =
        GlobalDVInfo->getDopeVectorField(DVFieldType);
    if (!DVField || DVField->getIsBottom())
      return false;

    DVField->addFieldAddr(GEP);

    DVField->analyzeUses();
    if (DVField->getIsBottom())
      return false;
  } else if (DVFieldType > DopeVectorFieldType::DV_PerDimensionArray) {
    // If the GEPOperator directly accesses the extent, stride or lower bound
    // fields, then it means that the array entry should be accessed by a
    // subscript. We need to collect the subscript and check where the
    // data is used for load and store. Each subscript accesses a different
    // entry in the per dimension-array. To simplify the analysis, we are
    // going to collect and organize each subscript in the corresponding
    // array entry (collectSubscriptInformation) and then we are going to
    // analyze them (analyzeSubscriptUses).
    for (auto *U : GEP->users()) {
      // All entries in the subscript should be constant
      if (auto *SI = dyn_cast<SubscriptInst>(U)) {
        // Collect the array entry
        ConstantInt *Index = dyn_cast<ConstantInt>(SI->getIndex());
        if (!Index)
          return false;
        uint64_t ArrayEntry = Index->getZExtValue();
        if (ArrayEntry >= GlobalDVInfo->getRank())
          return false;

        // Find the DopeVectorField use corresponding to the array entry
        DopeVectorFieldUse *DVField =
            GlobalDVInfo->getDopeVectorField(DVFieldType, ArrayEntry);
        if (!DVField || DVField->getIsBottom())
          return false;

        // Store the current subscirpt
        DVField->addFieldAddr(GEP);
        DVField->collectSubscriptInformation(SI, DVFieldType,
                                             GlobalDVInfo->getRank());
        if (DVField->getIsBottom())
          return false;
      } else {
        return false;
      }
    }

    // Check that all subscripts used for collecting the extent, stride and
    // lower bound are used for loading and storing data.
    for (uint64_t I = 0, E = GlobalDVInfo->getRank(); I < E; I++) {
      auto *ExtentField =
        GlobalDVInfo->getDopeVectorField(DopeVectorFieldType::DV_ExtentBase,
                                         I);
      ExtentField->analyzeSubscriptsUses();
      if (ExtentField->getIsBottom())
        return false;

      auto *StrideField =
        GlobalDVInfo->getDopeVectorField(DopeVectorFieldType::DV_StrideBase,
                                         I);
      StrideField->analyzeSubscriptsUses();
      if (StrideField->getIsBottom())
        return false;

      auto *LowerBoundField = GlobalDVInfo->getDopeVectorField(
          DopeVectorFieldType::DV_LowerBoundBase, I);
      LowerBoundField->analyzeSubscriptsUses();
      if (LowerBoundField->getIsBottom())
        return false;
    }
  } else {
    // NOTE: If the DVFieldType is DV_PerDimensionArray it means that the
    // extent, stride and lower bound are being accessed. We may need
    // to extend the data colection to support this operation.
    return false;
  }

  return true;
}

extern Argument *isIPOPropagatable(const Value *V, const User *U) {
  if (isa<IntrinsicInst>(U))
    return nullptr;
  if (auto CB = dyn_cast<CallBase>(U)) {
    if (CB->isIndirectCall())
      return nullptr;
    Function *F = CB->getCalledFunction();
    if (!F || F->hasAddressTaken() || F->isDeclaration())
      return nullptr;
    if (auto ArgPos = getArgumentPosition(*CB, V))
      return F->getArg(*ArgPos);
  }
  return nullptr;
}

// Given a Value and the TargetLibraryInfo, check if it is a BitCast
// and is only used for data allocation and deallocation. Return the
// call to the data alloc function.
CallBase* GlobalDopeVector::castingUsedForDataAllocation(Value *Val,
    std::function<const TargetLibraryInfo &(Function &F)> &GetTLI) {
#if INTEL_FEATURE_SW_ADVANCED
    // If aggressive DVCP is disabled then collect the allocation sites only.
    if (!EnableAggressiveDVCP)
      return bitCastUsedForAllocationOnly(Val, GetTLI);
    else
#endif // INTEL_FEATURE_SW_ADVANCED
      return bitCastUsedForAllocation(Val, GetTLI);
}

// Collect the nested dope vectors and store the access to them. Return false
// if there is any illegal access, else return true.
bool GlobalDopeVector::collectNestedDopeVectorFromSubscript(
    SubscriptInst *SI, const DataLayout &DL, bool ForDVCP) {

  // Given a Value that is a GetElementPointerInst or a BitCastInst,
  // then find if it is accessing a structure, if so collect the field
  // number and type.
  //
  // This may need some extra work for with opaque pointers. A dope
  // vector with an opaque pointer might look as follows:
  //
  //   %opaque_dv = { p0*, i64, i64, i64, i64, i64, 1 x [i64, i64, i64] }
  //
  // This means that we don't know the pointer type for field 0.
  auto GetNestedDVTypeFromValue = [](Value *Val) ->
      std::pair<Type *, uint64_t> {

    std::pair<Type *, uint64_t> NullPair = std::make_pair(nullptr, 0);

    uint64_t RetIdx = 0;
    Type *RetType = nullptr;

    // If it is a BitCast, the destination is a pointer to a structure.
    // This is accessing field 0.
    if (auto BC = dyn_cast<BitCastInst>(Val)) {
      Type *DestTy = BC->getDestTy();
      Type *SrcTy = BC->getSrcTy();
      if (!SrcTy->isPointerTy() || !DestTy->isPointerTy())
        return NullPair;

      // NOTE: This may need to be updated for opaque pointers.
      SrcTy = SrcTy->getNonOpaquePointerElementType();
      if (!SrcTy->isStructTy() || SrcTy->getStructNumElements() == 0)
        return NullPair;

      Type *ZeroType = SrcTy->getStructElementType(0);
      DestTy = DestTy->getNonOpaquePointerElementType();
      if (ZeroType == DestTy)
        RetType = DestTy;
    } else if (auto GEP = dyn_cast<GetElementPtrInst>(Val)) {
      // If it is a GEP, then all indices are constants
      if (!GEP->hasAllConstantIndices())
        return NullPair;

      Type *SrcType = GEP->getSourceElementType();
      auto ArrayIdx = getConstGEPIndex(*(cast<GEPOperator>(GEP)), 1);
      if (!ArrayIdx || *ArrayIdx != 0)
        return NullPair;

      auto Idx = getConstGEPIndex(*(cast<GEPOperator>(GEP)), 2);
      if (!Idx)
        return NullPair;

      uint64_t FieldIdx = *Idx;

      if (FieldIdx >= SrcType->getStructNumElements())
        return NullPair;

      RetType = SrcType->getStructElementType(FieldIdx);
      RetIdx = FieldIdx;
    } else {
      llvm_unreachable("Trying to collect nested dope vector from an "
                       "invalid instruction");
    }

    return std::make_pair(RetType, RetIdx);
  };

  // Find the nested dope vector info, if not found then
  // create a new entry with (VBase, FieldNum).
  auto FindOrMakeNestedDopeVector = [&, this](Value *VBase,
                                              Type *DVType,
                                              uint64_t FieldNum) ->
                                              NestedDopeVectorInfo* {
    auto *NestedDV = getNestedDopeVector(VBase, FieldNum);
    if (!NestedDV) {
      NestedDV = new NestedDopeVectorInfo(Glob, DVType, FieldNum, VBase,
          true /* AllowMultipleFieldAddresses*/);
      NestedDopeVectors.insert(NestedDV);
    } else {
      assert(NestedDV->getLLVMStructType() == DVType &&
             "Nested dope vector type mismatch with the type found by the "
             "subscript analysis");
    }

    return NestedDV;
  };

  // Return a ConstantInt value for the stride of 'SI', if one can be
  // easily found.
  //
  auto GetConstantStride = [this](SubscriptInst *SI) -> ConstantInt * {
    Value *VS = SI->getStride();
    // If the stride is a literal constant, return it.
    if (auto CI0 = dyn_cast<ConstantInt>(VS))
      return CI0;
    // Otherwise, look for a LoadInst, fed by a SubscriptInst, fed by a
    // GEPOperator.
    auto LI = dyn_cast<LoadInst>(VS);
    if (!LI)
      return nullptr;
    auto SIS = dyn_cast<SubscriptInst>(LI->getPointerOperand());
    if (!SIS)
      return nullptr;
    // Is this a GEPOPerator indexing the stride of a global dope vector?
    auto GEPO = dyn_cast<GEPOperator>(SIS->getPointerOperand());
    if (!GEPO || GEPO->getPointerOperand() != Glob)
      return nullptr;
    if (DopeVectorAnalyzer::identifyDopeVectorField(*GEPO) != DV_StrideBase)
      return nullptr;
    // Get the dimension for the stride in the dope vector.
    auto CI = dyn_cast<ConstantInt>(SIS->getIndex());
    if (!CI)
      return nullptr;
    // Look up the stride value in the DopeVectorInfo.
    uint64_t Dim = CI->getZExtValue();
    DopeVectorInfo *DVI = getGlobalDopeVectorInfo();
    assert(DVI && "Expecting dope vector info");
    DopeVectorFieldUse *SF = DVI->getDopeVectorField(
        DopeVectorFieldType::DV_StrideBase, Dim);
    return SF->getConstantValue();
  };

  // If 'UU' is a byte-flattened GEP with I8 source element type and
  // pointer operand 'V', constant index X, and a single user, subtract X
  // from 'NE' and return the single user of that GEP. Otherwise, return UU.
  //
  auto ByteFlattenedGEPAdjustment = [](Value *V, User *UU,
                                       uint64_t &NE) -> User * {
    auto GEPI = dyn_cast<GetElementPtrInst>(UU);
    if (!GEPI || GEPI->getNumIndices() != 1 ||
        GEPI->getPointerOperand() != V || !GEPI->hasOneUser())
      return UU;
    LLVMContext &Ctx = GEPI->getContext();
    if (GEPI->getSourceElementType() != llvm::Type::getInt8Ty(Ctx))
      return UU;
    auto CI = dyn_cast<ConstantInt>(GEPI->getOperand(1));
    if (!CI)
      return UU;
    NE -= CI->getZExtValue();
    return GEPI->user_back();
  };

  // Return 'true' if 'CB' is a call to a memfunc with 'InPtr' pointing to
  // an argument of the memfunc that may write up to 'NE' bytes at that
  // argument without invalidating the dope vector analysis.
  //
  auto IsSafeIntrinMemFunc = [](CallBase *CB, Value *InPtr,
                               uint64_t NE) -> bool {
    if (CB->arg_size() != 4)
      return false;
    if (CB->getArgOperand(0) != InPtr)
      return false;
    auto CI = dyn_cast<ConstantInt>(CB->getArgOperand(2));
    if (!CI || CI->getZExtValue() > NE)
      return false;
    return true;
  };

  // Return 'true' if 'CB' is a call to an intrinsic with 'InPtr' pointing to
  // an argument of the intrinsic that may write up to 'NE' bytes at that
  // argument without invalidating the dope vector analysis.
  //
  auto IsSafeIntrinUser = [&](CallBase *CB, Value *InPtr, uint64_t NE) -> bool {
    auto II = dyn_cast<IntrinsicInst>(CB);
    if (!II)
      return false;
    switch (II->getIntrinsicID()) {
    case Intrinsic::memcpy:
    case Intrinsic::memset:
      return IsSafeIntrinMemFunc(CB, InPtr, NE);
    default:
      break;
    }
    return false;
  };

  // Return 'true' if 'CB' is a call to the Fortran runtime library libFunc
  // 'for_trim' with 'InPtr' pointing to its argument which may write up to
  // 'NE' bytes at that argument without invalidating the dope vector analysis.
  // Currently, the destination must be a local array large enough to hold
  // the result.
  //
  auto IsSafeLibFuncForTrim = [](CallBase *CB, Value *InPtr,
                                 uint64_t NE) -> bool {
    if (CB->arg_size() != 4)
      return false;
    if (CB->getArgOperand(2) != InPtr)
      return false;
    auto CI1 = dyn_cast<ConstantInt>(CB->getArgOperand(1));
    if (!CI1 || CI1->getZExtValue() > NE)
      return false;
    auto CI3 = dyn_cast<ConstantInt>(CB->getArgOperand(3));
    if (!CI3 || CI3->getZExtValue() > NE)
      return false;
    auto GEPI = dyn_cast<GetElementPtrInst>(CB->getArgOperand(0));
    if (!GEPI || !GEPI->hasAllZeroIndices())
      return false;
    auto AI = dyn_cast<AllocaInst>(GEPI->getPointerOperand());
    if (!AI)
      return false;
    auto AT = dyn_cast<ArrayType>(AI->getAllocatedType());
    if (!AT || AT->getNumElements() < NE)
      return false;
    return true;
  };

  //
  // Return 'true' if 'CB' is a call to the Fortran runtime library libFunc
  // 'for_concat' with 'InPtr' pointing to its argument without invalidating
  // the dope vector analysis. We are checking the source argument (argument
  // #0) in this implementation, which will not write any bytes to that
  // argument.
  //
  auto IsSafeLibFuncForConcat = [](CallBase *CB, Value *InPtr) -> bool {
    if (CB->arg_size() != 4)
      return false;
    if (CB->getArgOperand(0) != InPtr)
      return false;
    for (unsigned I = 1; I < CB->arg_size(); ++I)
      if (CB->getArgOperand(I) == InPtr)
        return false;
    return true;
  };

  //
  // Return 'true' if 'CB' is a call to a libFunc with 'InPtr' pointing to
  // an argument of the libFunc that may write up to 'NE' bytes at that
  // argument without invalidating the dope vector analysis.
  //
  auto IsSafeLibFuncUser = [&](CallBase *CB, Value *InPtr,
                               uint64_t NE) -> bool {
    Function *F = CB->getCalledFunction();
    if (!F)
      return false;
    LibFunc TheLibFunc;
    const TargetLibraryInfo &TLI = GetTLI(*const_cast<Function *>(F));
    if (!TLI.getLibFunc(F->getName(), TheLibFunc) || !TLI.has(TheLibFunc))
      return false;
    switch (TheLibFunc) {
    case LibFunc_for_trim:
      return IsSafeLibFuncForTrim(CB, InPtr, NE);
    case LibFunc_for_concat:
      return IsSafeLibFuncForConcat(CB, InPtr);
    default:
      break;
    }
    return false;
  };

  // Return 'true' if the User 'U' of 'V' is a GEP of i8* array type feeding
  // an intrinsic or a libFunc that uses it in a way that does not invalidate
  // dope vector analysis.
  //
  auto IsSafeIntrinOrLibFuncUser = [&](Value *V, User *U) -> bool {
    if (!U->hasOneUser())
      return false;
    auto GEPI = dyn_cast<GetElementPtrInst>(U->user_back());
    if (!GEPI)
      return false;
    if (GEPI->getPointerOperand() != U || !GEPI->hasAllZeroIndices())
      return false;
    Type *GEPITy = GEPI->getSourceElementType();
    auto GEPIArTy = dyn_cast<ArrayType>(GEPITy);
    if (!GEPIArTy)
      return false;
    LLVMContext &Ctx = GEPI->getContext();
    if (GEPIArTy->getElementType() != llvm::Type::getInt8Ty(Ctx))
      return false;
    uint64_t NE = GEPIArTy->getNumElements();
    for (User *UU : GEPI->users()) {
      auto VV = ByteFlattenedGEPAdjustment(GEPI, UU, NE);
      Value *InPtr = VV != UU ? UU : GEPI;
      auto CB = dyn_cast<CallBase>(VV);
      if (!CB)
        return false;
      if (IsSafeIntrinUser(CB, InPtr, NE))
        continue;
      if (IsSafeLibFuncUser(CB, InPtr, NE))
        continue;
      return false;
    }
    return true;
  };

  //
  // Return 'true' if 'A' can be propagated up to an AllocaInst. Use
  // 'ValueChecked' to ensure that we don't get in a recursive loop
  // by checking for a Value twice. As an example, consider:
  //
  //   subroutine bigloop(tau, krh)
  //   real tau(pcols, pver, nswbands)
  //   integer krh(pcols, pver)
  //   real, pointer :: h_ext(:,:)
  //   do id = 1, numphysprops
  //     call physprop_get(id, h_ext)
  //     call get_hygro_rad_props(krh, h_ext, tau)
  //   end do
  //   end subroutine
  //
  // Here if 'A' represents the second argument of 'physprop_get',
  // and 'h_ext' is represented by an AllocaInst in 'bigloop', we return
  // 'true' noting that 'h_ext' can be propagated down through
  // 'get_hygro_rad_props'.
  //
  std::function<bool(Argument *A, const DataLayout &DL,
                     NestedDopeVectorInfo *NDVInfo, bool ForDVCP,
                     SetVector<Value *> &ValueChecked)>
      CanPropUp = [&, this](Argument *A, const DataLayout &DL,
                      NestedDopeVectorInfo *NDVInfo, bool ForDVCP,
                      SetVector<Value *> &ValueChecked) -> bool {
    Function *F = A->getParent();
    unsigned FArgNo = A->getArgNo();
    ValueChecked.insert(A);
    if (F->hasAddressTaken() || F->isVarArg())
      return false;
    for (User *U : F->users()) {
      auto CB = dyn_cast<CallBase>(U);
      if (!CB || CB->getCalledFunction() != F)
        return false;
      Value *AA = CB->getArgOperand(FArgNo);
      if (auto C = dyn_cast<Constant>(AA)) {
        if (C->isNullValue())
          continue;
        return false;
      }
      if (auto BC = dyn_cast<BitCastInst>(AA)) {
        ValueChecked.insert(AA);
        AA = BC->getOperand(0);
      }
      if (auto FA = dyn_cast<Argument>(AA)) {
        if (!CanPropUp(FA, DL, NDVInfo, ForDVCP, ValueChecked))
          return false;
        continue;
      }
      if (auto AI = dyn_cast<AllocaInst>(AA)) {
       if (!collectNestedDopeVectorFieldAddress(NDVInfo, AI, GetTLI,
                ValueChecked, DL, ForDVCP, true /* AllowCheckForAllocSite */))
          return false;
        continue;
      }
      return false;
    }
    return true;
  };

  //
  // Return 'true' if 'U' is assigned through a pointer assigment to
  // a dummy argument. For example, in:
  //
  //   subroutine physprop_get(id, sw_hygro_ext)
  //   integer, intent(in) :: id
  //   real, pointer :: sw_hygro_ext(:,:)
  //   sw_hygro_ext => physprop(id)%sw_hygro_ext
  //   end subroutine physprop_get
  //
  // Let 'U' be 'physprop(id)%sw_hygro_ext'.  Note that it is assigned
  // through a pointer assignment to 'sw_hygro_ext', which is the second
  // argument of 'physprop_get'.
  //
  auto PropPtrAssignToArgument = [](User *U,
                                    const DataLayout &DL) -> Argument * {
    if (!U->hasOneUser())
      return nullptr;
    U = U->user_back();
    if (auto BC = dyn_cast<BitCastInst>(U)) {
      uint32_t SArrayRank;
      Type *SElementType = nullptr;
      Type *STy = BC->getSrcTy();
      if (!STy->isPointerTy())
        return nullptr;
      // Will need to be updated for opaque pointers.
      Type *SPETy = STy->getNonOpaquePointerElementType();
      if (!isDopeVectorType(SPETy, DL, &SArrayRank, &SElementType))
        return nullptr;

      // NOTE: This needs to be updated when opaque pointers support is
      // enabled. SElementType and DElementType need to be collected when
      // typed pointers are enabled.
      if (SPETy->getContext().supportsTypedPointers())
        assert(SElementType &&
               "Pointer address not collected for source pointer");

      uint32_t DArrayRank;
      Type *DElementType = nullptr;
      Type *DTy = BC->getDestTy();
      if (!DTy->isPointerTy())
        return nullptr;
      Type *DPETy = DTy->getNonOpaquePointerElementType();
      if (!isDopeVectorType(DPETy, DL, &DArrayRank, &DElementType))
        return nullptr;

      // NOTE: This needs to be updated when opaque pointers support is
      // enabled.
      if (DPETy->getContext().supportsTypedPointers())
        assert(DElementType &&
               "Pointer address not collected for destination pointer");

      if (SArrayRank != DArrayRank || SElementType != DElementType)
        return nullptr;
      if (!BC->hasOneUser())
        return nullptr;
      U = U->user_back();
    }
    auto LI = dyn_cast<LoadInst>(U);
    if (!LI)
      return nullptr;
    if (!LI->hasOneUser())
      return nullptr;
    auto SI = dyn_cast<StoreInst>(LI->user_back());
    if (!SI)
      return nullptr;
    if (SI->getValueOperand() != LI)
      return nullptr;
    auto A = dyn_cast<Argument>(SI->getPointerOperand());
    if (!A)
      return nullptr;
    for (User *U : A->users()) {
      if (auto IC = dyn_cast<ICmpInst>(U)) {
         if (IC->getPredicate() != ICmpInst::ICMP_EQ)
           return nullptr;
         if (IC->getOperand(0) != A)
           return nullptr;
         auto C = dyn_cast<Constant>(IC->getOperand(1));
         if (!C || !C->isNullValue())
           return nullptr;
      } else if (U != SI) {
         return nullptr;
      }
    }
    return A;
  };

  //
  // Return 'true' if the 'U' can be propagated through a pointer
  // assigment up the call chain to a AllocaInst and then down the
  // call chain to subscripts.
  //
  // As an example, consider the following Fortran code:
  //
  //   parameter n1=19, n2=1000, n3=100
  //   integer, parameter :: nswbands = 19
  //   integer, parameter :: pcols = 4
  //   integer, parameter :: pver = 26
  //   type :: physprop_type
  //   real, pointer :: sw_hygro_ext(:,:)
  //   endtype physprop_type
  //   type (physprop_type), pointer :: physprop(:)
  //
  //   subroutine physprop_get(id, sw_hygro_ext)
  //   integer, intent(in) :: id
  //   real, pointer :: sw_hygro_ext(:,:)
  //   sw_hygro_ext => physprop(id)%sw_hygro_ext
  //   end subroutine physprop_get
  //
  //   subroutine get_hygro_rad_props(krh, ext, tau)
  //   integer, intent(in) :: krh(pcols,pver)
  //   real, intent(in) :: ext(:,:)
  //   real, intent(out) :: tau(pcols,pver,nswbands)
  //   do iswband = 1, nswbands
  //     do icol = 1, pcols
  //       do ilev = 1, pver
  //         tau(icol,ilev,iswband) = &
  //           ext(krh(icol,ilev)+1,iswband) + ext(krh(icol,ilev)+1,iswband)
  //       end do
  //     end do
  //   end do
  //   end subroutine
  //
  //   subroutine bigloop(tau, krh)
  //   real tau(pcols, pver, nswbands)
  //   integer krh(pcols, pver)
  //   real, pointer :: h_ext(:,:)
  //   do id = 1, numphysprops
  //     call physprop_get(id, h_ext)
  //     call get_hygro_rad_props(krh, h_ext, tau)
  //   end do
  //   end subroutine
  //
  // Here, 'physprop(id)%sw_hygro_ext' is assigned to the pointer
  // 'sw_hygro_ext' in 'physprop_get', can be propagated up the call chain
  // to 'h_ext' in 'bigloop' and then propagated back down to 'ext' in
  // 'get_hygro_rad_props'.
  //
  auto CanPropThruPtrAssn = [&](User *U, const DataLayout &DL, bool ForDVCP,
                                NestedDopeVectorInfo *NDVInfo) -> bool {
    Argument *A = PropPtrAssignToArgument(U, DL);
    if (!A)
      return false;
    SetVector<Value *> ValueChecked;
    if (!CanPropUp(A, DL, NDVInfo, ForDVCP, ValueChecked))
      return false;
    return true;
  };

  // Recursive forward declaration
  std::function<bool(Value *, User *, SmallPtrSetImpl<Value *> &)>
    PropagatesThroughUser;

  //
  // Return 'true' if 'GEPI' is propagated down the call chain and terminates
  // with a LoadInst or StoreInst.
  //
  std::function<bool(GetElementPtrInst *, SmallPtrSetImpl<Value *> &)>
      PropagatesThroughGEPI = [&](GetElementPtrInst *GEPI,
                                  SmallPtrSetImpl<Value *> &Visited) -> bool {
    if (GEPI->getNumIndices() != 1)
      return false;
    for (User *U : GEPI->users()) {
      if (!Visited.count(U))
        continue;
      if (auto NewGEPI = dyn_cast<GetElementPtrInst>(U)) {
        if (!PropagatesThroughGEPI(NewGEPI, Visited))
          return false;
      } else if (!PropagatesThroughUser(GEPI, U, Visited)) {
        return false;
      }
    }
    return true;
  };

  //
  // Return 'true' if 'PHIN', which is a User of 'V' is propagated down the
  // call chain and terminates with a LoadInst or StoreInst.
  //
  auto PropagatesThroughPHINode = [&](Value *V, PHINode *PHIN,
                                      SmallPtrSetImpl<Value *> &Visited)
                                      -> bool {
     static const unsigned BackTraversalLimit = 3;
     if (Visited.count(PHIN))
       return true;
     Visited.insert(PHIN);
     //
     // Check the backedges of 'PHIN' and ensure that they reach back to
     // 'PHIN' using only byte-flattened GEPs or other PHINodes. This
     // is to ensure that any value derived from 'PHIN' is a byte offset
     // from the original value coming into 'PHIN'. 'LangRuleOutOfBoundsOK'
     // ensures that the offset does not index out of the original bounds.
     //
     for (Value *W : PHIN->incoming_values()) {
       if (W == V)
         continue;
       Value *WW = W;
       bool CompletedLoop = false;
       for (unsigned J = 0; J < BackTraversalLimit; ++J) {
         if (auto GEPI = dyn_cast<GetElementPtrInst>(WW)) {
           if (GEPI->getNumIndices() != 1)
             return false;
           WW = GEPI->getPointerOperand();
         } else if (auto NewPHIN = dyn_cast<PHINode>(WW)) {
           // NOTE: This traverses back through only one level of PHINode.
           // It could be generalized into a recursive search, but it is
           // sufficient for what we need at this time.
           bool FoundPHI = false;
           for (Value *WWW : NewPHIN->incoming_values())
             if (WWW == PHIN) {
               WW = PHIN;
               FoundPHI = true;
               break;
             }
           if (!FoundPHI)
             return false;
         }
         else {
           return false;
         }
         if (WW == PHIN) {
           CompletedLoop = true;
           break;
         }
       }
       if (!CompletedLoop)
         return false;
     }
     //
     // Walk forward through the uses of 'PHI' allowing the pointer
     // value to be offset through a series of byte flattened GEPs.
     //
     for (User *U : PHIN->users()) {
       if (auto GEPI = dyn_cast<GetElementPtrInst>(U)) {
         if (!PropagatesThroughGEPI(GEPI, Visited))
           return false;
       } else if (!PropagatesThroughUser(PHIN, U, Visited)) {
         return false;
       }
     }
     return true;
  };

  //
  // Recursive version of 'PropagatesToLoadOrStore' that uses a 'Visited'
  // set to exclude recursive descent on Values that have already been
  // evaluated. Two types of Values are placed in the 'Visited' set:
  // Arguments and PHINodes.
  //
  std::function<bool(Value *V, SmallPtrSetImpl<Value *> &Visited)>
      PropagatesToLoadOrStoreX = [&](Value *V,
                                     SmallPtrSetImpl<Value *> &Visited)
                                     -> bool {
    if (Visited.count(V))
      return true;
    Visited.insert(V);
    for (User *U: V->users())
      if (!PropagatesThroughUser(V, U, Visited))
        return false;
    return true;
  };

  //
  // Return 'true' if 'LI' is just stored as an integer or
  // floating point type. This ensures that it is not cast as
  // a pointer and used to dereference something else. If passed
  // at a call site, we follow it through the call chain.
  //
  auto PropagatesThroughLoadInst = [&](LoadInst *LI,
                                       SmallPtrSetImpl<Value *> &Visited)
                                       -> bool {
    for (User *U : LI->users()) {
      if (auto SI = dyn_cast<StoreInst>(U)) {
        Type *Ty = SI->getValueOperand()->getType();
        if (!Ty->isIntegerTy() && !Ty->isFloatingPointTy())
          return false;
      } else if (auto Arg = dyn_cast<Argument>(U)) {
        if (!PropagatesToLoadOrStoreX(Arg, Visited))
          return false;
      }
    }
    return true;
  };

  //
  // Return an AllocaInst that is used as a Fortran concatenation table for
  // for_concat(), if 'V' is an address stored into one of the entries in
  // that concatenation table. A concatenation table is an array of structures
  // each of which has two fields. The first field is a pointer to a string
  // and the second is the integer length of the string. The libFunc
  // for_concat takes a concatenation table as its first argument and
  // returns a pointer to the concatenated string in the second argument.
  //
  // For example, here is a series of instructions that allocates a 4 entry
  // concatenation table in %9, and assigns %89 to the string address of
  // the 4th entry in the table.
  //   %9 = alloca [4 x { i8*, i64 }], align 8
  //   %76 = getelementptr inbounds [4 x { i8*, i64 }], [4 x { i8*, i64 }]* %9,
  //       i64 0, i64 0
  //   %87 = call { i8*, i64 }* @llvm.intel.subscript.[...](i8 0, i64 1,
  //       i32 16, { i8*, i64 }* nonnull %76, i32 4)
  //   %88 = getelementptr inbounds { i8*, i64 }, { i8*, i64 }* %87, i64 0,
  //       i32 0
  //   store i8* %89, i8** %88, align 1
  //
  auto IsAddressInLocalConcatTable = [](Value *V) -> AllocaInst * {
    auto GEPI0 = dyn_cast<GetElementPtrInst>(V);
    if (!GEPI0 || GEPI0->getNumIndices() != 2 || !GEPI0->hasAllZeroIndices())
      return nullptr;
    auto SI = dyn_cast<SubscriptInst>(GEPI0->getPointerOperand());
    if (!SI || SI->getRank() != 0)
      return nullptr;
    auto CI1 = dyn_cast<ConstantInt>(SI->getLowerBound());
    if (!CI1 || CI1->getZExtValue() != 1)
      return nullptr;
    auto CI2 = dyn_cast<ConstantInt>(SI->getStride());
    if (!CI2 || CI2->getZExtValue() != 16)
      return nullptr;
    auto GEPI1 = dyn_cast<GetElementPtrInst>(SI->getPointerOperand());
    if (!GEPI1 || GEPI1->getNumIndices() != 2 || !GEPI1->hasAllZeroIndices())
      return nullptr;
    auto CI4 = dyn_cast<ConstantInt>(SI->getIndex());
    if (!CI4)
      return nullptr;
    auto AI = dyn_cast<AllocaInst>(GEPI1->getPointerOperand());
    if (!AI)
      return nullptr;
    auto ATy = dyn_cast<ArrayType>(AI->getAllocatedType());
    if (!ATy)
      return nullptr;
    if (CI4->getZExtValue() > ATy->getNumElements())
      return nullptr;
    auto STy = dyn_cast<StructType>(ATy->getElementType());
    if (!STy || STy->getNumElements() != 2)
      return nullptr;
    if (!STy->getElementType(0)->isPointerTy())
      return nullptr;
    if (!STy->getElementType(1)->isIntegerTy())
      return nullptr;
    return AI;
  };

  //
  // Return 'true' if 'SI' is a safe store. This means it is assigned only
  // to a local variable and then passed to a libFunc which will use the
  // value it points to. Therefore, the stored value does not escape.
  //
  auto IsSafeLocalStoreAssignment = [&](StoreInst *SI) -> bool {
     AllocaInst *AI = IsAddressInLocalConcatTable(SI->getPointerOperand());
     if (!AI || AI->getNumUses() > 2)
       return false;
     for (User *U : AI->users()) {
       // The path from the StoreInst to the AllocaInst in
       // IsAddressInLocalConcatTable() can be ignored, as it has already
       // been analyzed in that function. It terminates in a GetElementPtrInst.
       if (isa<GetElementPtrInst>(U))
         continue;
       Value *W = AI;
       Value *V = U;
       if (auto BC = dyn_cast<BitCastInst>(V)) {
         if (!BC->hasOneUser())
           return false;
         W = BC;
         V = BC->user_back();
       }
       auto CB = dyn_cast<CallBase>(V);
       if (!CB || !IsSafeLibFuncForConcat(CB, W))
         return false;
     }
     return true;
  };

  //
  // Return 'true' if 'U', which is a User of 'V' is propagated down the
  // call chain and terminates with a LoadInst or StoreInst.
  //
  PropagatesThroughUser = [&](Value *V, User *U,
                              SmallPtrSetImpl<Value *> &Visited) -> bool {
    if (auto SI = dyn_cast<StoreInst>(U)) {
      if (SI->getValueOperand() == V && !IsSafeLocalStoreAssignment(SI))
        return false;
      return true;
    }
    if (auto PHIN = dyn_cast<PHINode>(U)) {
      if (!PropagatesThroughPHINode(V, PHIN, Visited))
        return false;
    } else if (auto LI = dyn_cast<LoadInst>(U)) {
      if (!PropagatesThroughLoadInst(LI, Visited))
        return false;
    } else if (auto BC = dyn_cast<BitCastInst>(U)) {
      for (User *UU : BC->users()) {
        if (!PropagatesThroughUser(BC, UU, Visited))
          return false;
      }
    } else if (auto CB = dyn_cast<CallBase>(U)) {
      auto ArgPos = getArgumentPosition(*CB, V);
      if (!ArgPos)
        return false;
      auto F = dyn_cast<Function>(CB->getCalledOperand()->stripPointerCasts());
      if (!F || F->isVarArg())
        return false;
      Argument *Arg = F->getArg(*ArgPos);
      if (!PropagatesToLoadOrStoreX(Arg, Visited))
        return false;
    } else if (auto GEPI = dyn_cast<GetElementPtrInst>(U)) {
      // Allow for the special case of a zero-indexed GEP, which is
      // effectively a null operation.
      if (GEPI->getPointerOperand() != V || !GEPI->hasAllZeroIndices())
        return false;
      for (User *UU : GEPI->users()) {
        if (!PropagatesThroughUser(GEPI, UU, Visited))
          return false;
      }
    } else {
      return false;
    }
    return true;
  };

  //
  // Returns 'true' if 'V' is only used as the pointer operand of a simple load
  // or store instruction.
  //
  auto PropagatesToSimpleLoadOrStore = [](Value *V) -> bool {
    for (User *U : V->users()) {
      if (isa<LoadInst>(U))
        continue;
      if (auto SI = dyn_cast<StoreInst>(U)) {
        if (SI->getValueOperand() == V)
          return false;
        continue;
      }
      return false;
    }
    return true;
  };

  //
  // Return 'true' if 'V' is propagated down the call chain and terminates
  // with a LoadInst or StoreInst.
  //
  auto PropagatesToLoadOrStore = [&](Value *V) {
    if (PropagatesToSimpleLoadOrStore(V))
      return true;
#if INTEL_FEATURE_SW_ADVANCED
    SmallPtrSet<Value *, 10> Visited;
    if (!CheckOutOfBoundsOK)
      return false;
    if (getLangRuleOutOfBoundsOK())
      return false;
    if (!PropagatesToLoadOrStoreX(V, Visited))
      return false;
    return true;
#else // INTEL_FEATURE_SW_ADVANCED
    return false;
#endif // INTEL_FEATURE_SW_ADVANCED
  };

  // Return 'true' if 'U' is the user of a SubscriptInst that can be
  // properly collected. In that case, update 'AllocSiteFound'.
  //
  auto CanCollectNDVSubscriptUser = [&, this](Value *V, User *U,
                                              const DataLayout &DL,
                                              NestedDopeVectorInfo *NDVInfo,
                                              bool &AllocSiteFound,
                                              bool ForDVCP) -> bool {
    if (auto *Call = castingUsedForDataAllocation(U, GetTLI)) {
      // A BitCast can used for allocating the array for the
      // nested dope vector at field 0. It represents a field 0 access.
      if (AllocSiteFound)
        return false;

      // NOTE: This may not work with opaque pointers. We may want to revise
      // it in the future.
      BitCastInst *BC = cast<BitCastInst>(U);
      Type *SrcTy = BC->getSrcTy();
      if (!SrcTy->isPointerTy())
        return false;

      StructType *StrSource =
          dyn_cast<StructType>(SrcTy->getNonOpaquePointerElementType());

      if (!StrSource || StrSource->getNumElements() == 0)
        return false;

      Type *FieldZeroType = StrSource->getElementType(0);
      if (!isDopeVectorType(FieldZeroType, DL))
        return false;

      auto *NestedDVInfo = FindOrMakeNestedDopeVector(V, FieldZeroType, 0);
      assert(NestedDVInfo && "Nested dope vector 0 couldn't be found\n");

      NestedDVInfo->addAllocSite(Call);
    } else if (isa<GetElementPtrInst>(U) || isa<BitCastInst>(U)) {
      auto NestedDVTypePair = GetNestedDVTypeFromValue(U);
      if (!NestedDVTypePair.first)
        return false;

      if (isDopeVectorType(NestedDVTypePair.first, DL)) {
        auto *NestedDVInfo =
            FindOrMakeNestedDopeVector(V, NestedDVTypePair.first,
                                       NestedDVTypePair.second);
        assert(NestedDVInfo && "Nested dope vector couldn't be found\n");
        SetVector<Value *> ValueChecked;

        // Nested dope vector found, now collect the fields access
        if (collectNestedDopeVectorFieldAddress(NestedDVInfo, U, GetTLI,
                ValueChecked, DL, ForDVCP, true /* AllowCheckForAllocSite */))
          return true;
        if (CanPropThruPtrAssn(U, DL, ForDVCP, NestedDVInfo))
          return true;
        return false;
      } else {
        if (PropagatesToLoadOrStore(U))
          return true;
        if (IsSafeIntrinOrLibFuncUser(V, U))
          return true;
        return false;
      }
    } else {
      // Subscript used for something else
      return false;
    }
    return true;
  };

  // Return 'true' if each use of 'A' can be propagated to a CallBase that
  // calls a Function with IR that is not address taken, or can be collected
  // inside its Function as usual. Update 'AllocSiteFound' if the alloc site
  // for the dope vector is found.
  //
  std::function<bool(Argument *, const DataLayout &,
                NestedDopeVectorInfo *, bool, bool &)>
      PropagateArgument = [&](Argument *A, const DataLayout &DL,
                              NestedDopeVectorInfo *NDVInfo,
                              bool ForDVCP, bool &AllocSiteFound) -> bool {
    for (User *U : A->users()) {
      if (Argument *NewA = isIPOPropagatable(A, U)) {
        if (!PropagateArgument(NewA, DL, NDVInfo, ForDVCP, AllocSiteFound))
          return false;
      } else if (!CanCollectNDVSubscriptUser(A, U, DL, NDVInfo,
                                             AllocSiteFound, ForDVCP)) {
        return false;
      }
    }
    return true;
  };

  if (!SI)
    return false;
  ConstantInt *GlobalElementSize = GlobalDVInfo->getDopeVectorField(
      DopeVectorFieldType::DV_ElementSize)->getConstantValue();

  bool AllocSiteFound = false;

  // Unknown element size for the global dope vector, we can't collect the
  // information for nested dope vectors. The element size of the global
  // dope vector is needed to make sure that the stride in the subscript
  // instruction is collecting the right number of bits when accessing
  // each entry in the global array.
  if (!GlobalElementSize || GetConstantStride(SI) != GlobalElementSize)
    return false;

  // Traverse through the users of the array entry and find the information
  // related to the nested dope vectors
  for (User *U : SI->users()) {
    if (Argument *A = isIPOPropagatable(SI, U)) {
      if (!PropagateArgument(A, DL, nullptr, ForDVCP, AllocSiteFound))
        return false;
    } else if (!CanCollectNDVSubscriptUser(SI, U, DL, nullptr,
                                           AllocSiteFound, ForDVCP)) {
      return false;
    }
  }
  return true;
}

using NDVInfoVector = SetVector<NestedDopeVectorInfo *>;

void
GlobalDopeVector::mergeNestedDopeVectors() {

  // Load a subset 'NDVSubset' of the nested dope vectors for merging.
  // The set will all have the same 'FieldNum' and a non-nullptr 'VBase'.
  //
  auto LoadNDVSubset = [this](NDVInfoVector &NDVSubset) {
    Optional<uint64_t> FieldNum = None;
    NDVSubset.clear();
    for (auto *NestedDV : NestedDopeVectors) {
      if (NestedDV->getVBase()) {
        if (!FieldNum)
          FieldNum = NestedDV->getFieldNum();
        if (NestedDV->getFieldNum() == FieldNum)
          NDVSubset.insert(NestedDV);
      }
    }
  };

  // Return 'true' if 'NDVSubset' has a mismatch and the nested dope vectors
  // contained in it cannot be merged.
  //
  auto HasMismatch = [](NDVInfoVector &NDVSubset) -> bool {
    for (unsigned I = 0; I < NDVSubset.size(); ++I)
      for (unsigned J = I + 1; J < NDVSubset.size(); ++J)
        if (!NDVSubset[I]->matches(*NDVSubset[J]))
          return true;
    return false;
  };

  // Merge a subset 'NDVSubset' of the nested dope vectors. If the merge
  // is successful, a single nested dope vector will remain that summarizes
  // the subset, with a 'VBase' which is nullptr.
  //
  auto MergeNDVSubset = [this, HasMismatch](NDVInfoVector &NDVSubset) {
    if (!NDVSubset.size())
      return;
    if (NDVSubset.size() == 1) {
      NDVSubset[0]->nullifyVBase();
      return;
    }
    NestedDopeVectorInfo* MergeDV = NDVSubset[0];
    MergeDV->nullifyVBase();
    auto AR_Pass = DopeVectorInfo::AnalysisResult::AR_Pass;
    if (HasMismatch(NDVSubset)) {
      bool Passed = MergeDV->getAnalysisResult() == AR_Pass;
      for (auto NestedDV : NDVSubset) {
        if (NestedDV != MergeDV) {
          if (Passed && (NestedDV->getAnalysisResult() != AR_Pass)) {
            MergeDV->setAnalysisResult(NestedDV->getAnalysisResult());
            Passed = false;
          }
          NestedDopeVectors.remove(NestedDV);
          delete NestedDV;
        }
      }
      if (Passed) {
        auto CNM = DopeVectorInfo::AnalysisResult::AR_CouldNotMerge;
        MergeDV->setAnalysisResult(CNM);
      }
    } else {
      for (auto NestedDV : NDVSubset)
        if (NestedDV != MergeDV) {
          MergeDV->merge(*NestedDV);
          NestedDopeVectors.remove(NestedDV);
          delete NestedDV;
        }
    }
  };

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  auto GetFunction = [](Value *V) -> Function * {
    if (!V)
      return nullptr;
    if (auto I = dyn_cast<Instruction>(V))
      return I->getFunction();
    else if (auto A = dyn_cast<Argument>(V))
      return A->getParent();
    return nullptr;
  };

  // Dump the nested dope vectors. Use 'Banner' to indicate the point in
  // time the dump occurs.
  //
  auto DumpNestedDopeVectors = [this, GetFunction](const char Banner[]) {
    dbgs() << "[" << Banner << "] DUMPING NESTED DOPE VECTORS: BEGIN\n";
    for (auto *NestedDV : NestedDopeVectors) {
      dbgs() << "FIELD[" << NestedDV->getFieldNum() << "] "
             << NestedDV->getVBase();
      if (Function  *F = GetFunction(NestedDV->getVBase()))
        dbgs() << " IN " << F->getName() << "\n";
      else
        dbgs() << "\n";
      if (NestedDV->getVBase())
        NestedDV->getVBase()->dump();
      NestedDV->print(2);
    }
    dbgs() << "[" << Banner << "] DUMPING NESTED DOPE VECTORS: END\n";
  };
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  LLVM_DEBUG(DumpNestedDopeVectors("BEFORE"));
  NDVInfoVector NestedDVSubset;
  LoadNDVSubset(NestedDVSubset);
  while (NestedDVSubset.size()) {
    MergeNDVSubset(NestedDVSubset);
    LoadNDVSubset(NestedDVSubset);
  }
  for (auto *NestedDV : NestedDopeVectors) {
    NestedDV->validateAllocSite();
    NestedDV->validateSingleNonNullValue(DV_ElementSize);
    NestedDV->validateSingleNonNullValue(DV_Codim);
  }
  LLVM_DEBUG(DumpNestedDopeVectors("AFTER"));
}

// This function will collect the information from the nested dope vectors that
// are copied. A copy dope vector is a dope vector that was allocated and each
// field is a copy from another dope vector. For example:
//
//   %DV2 = alloca %"__DTRT_QNCA_a0$double*$rank3$"
//   %1 = getelementptr inbounds %"__DTRT_QNCA_a0$double*$rank3$",
//           %"__DTRT_QNCA_a0$double*$rank3$"* @DV1, i64 0, i32 0
//   %2 = load double*, double** %1
//   %3 = getelementptr inbounds %"__DTRT_QNCA_a0$double*$rank3$",
//           %"__DTRT_QNCA_a0$double*$rank3$"* %DV2, i64 0, i32 0
//   store double* %2, double** %3
//
// Assume that @DV1 is a global dope vector that was collected, analyzed and
// validated, then %DV2 will be a local copy since the fields of @DV1 are
// copied to it. The local copy is a new instantiation, therefore the analysis
// result won't affect the result from the original dope vector. The goal is
// to find these copy dope vectors and prove that the fields won't change
// in order to propagate the constants collected for them too.
void GlobalDopeVector::collectAndAnalyzeCopyNestedDopeVectors(
    const DataLayout &DL, bool ForDVCP) {

  // Collect the copy dope vector from the store instruction. From the
  // example above, assume that we are analyzing the store instruction,
  // then this function will return %DV2.
  auto GetLocalDopeVector = [](StoreInst *SI) -> AllocaInst * {
    auto *DVField = SI->getPointerOperand();
    auto *GEPDV = dyn_cast<GEPOperator>(DVField);
    if (!GEPDV)
      return nullptr;

    if (DopeVectorAnalyzer::identifyDopeVectorField(*GEPDV) !=
        DopeVectorFieldType::DV_ArrayPtr)
      return nullptr;

    AllocaInst *AI = dyn_cast<AllocaInst>(GEPDV->getOperand(0));
    return AI;
  };

  // Generate a new nested dope vector that represents a copy of the
  // current dope vector. The alloc site will be AI and VBase is the
  // pointer to the original dope vector.
  auto GenerateCopyNestedDV = [DL](AllocaInst *AI, Value *VBase,
                                   uint64_t FieldNum,
                                   Type *DVType) -> NestedDopeVectorInfo * {
    assert(isDopeVectorType(DVType, DL) && "Trying to make a copy dope vector "
                                           "from a non-dope vector type");


    auto *PtrAllocType = AI->getType();
    // NOTE: This won't work for opaque pointers. We need to address collecting
    // the dope vector for opaque pointers.
    auto *AllocMainType = PtrAllocType->getNonOpaquePointerElementType();
    if (AllocMainType != DVType)
      return nullptr;

    auto CopyNestedDV = new NestedDopeVectorInfo(AI, AllocMainType, FieldNum,
        VBase, true /* AllowMultipleFieldAddresses */,
        true /* IsCopyDopeVector */);

    CopyNestedDV->addAllocSite(AI);
    return CopyNestedDV;
  };

  // Traverse through each nested dope vector and collect any information
  // from the copies.
  for (auto *NestedDV : NestedDopeVectors) {
    if (NestedDV->getAnalysisResult() !=
        DopeVectorInfo::AnalysisResult::AR_Pass)
      continue;

    // Find where the pointer to the array is being copied
    SetVector<AllocaInst *> CopyNestedDopeVectorsAllocs;
    SetVector<NestedDopeVectorInfo *> CopyNestedDopeVectors;
    auto PtrAddrField =
        NestedDV->getDopeVectorField(DopeVectorFieldType::DV_ArrayPtr);

    assert(!PtrAddrField->getIsBottom() && "Dope vector field set to bottom "
                                           "when analysis passes");

    // Identify the alloc sites
    for (auto *LI : PtrAddrField->loads()) {

      GEPOperator *GEPO = dyn_cast<GEPOperator>(LI->getPointerOperand());
      // NOTE: Perhaps this should be an assert
      if (!GEPO)
        continue;

      Value *VBase = GEPO->getOperand(0);

      // The load will be used in a store instruction to another dope vector
      for (auto U : LI->users()) {
        if (auto *SI = dyn_cast<StoreInst>(U)) {
          if (SI->getValueOperand() != LI)
            continue;

          if (AllocaInst *AllocI = GetLocalDopeVector(SI)) {
            if (AllocI->getFunction() != LI->getFunction())
              continue;

            if (!CopyNestedDopeVectorsAllocs.insert(AllocI))
              continue;

            // Generate the copy and store it
            auto *CopyNestedDV = GenerateCopyNestedDV(AllocI, VBase,
                NestedDV->getFieldNum(), NestedDV->getLLVMStructType());
            if (!CopyNestedDV)
              continue;

            CopyNestedDopeVectors.insert(CopyNestedDV);
          }
        }
      }
    }

    // Traverse through each copy, collect where each field was accessed and
    // analyze it. If the analysis passes then we are going to merge the load
    // instructions.
    for (auto *LocalDV : CopyNestedDopeVectors) {
      SetVector<Value *> ValueChecked;
      if (collectNestedDopeVectorFieldAddress(LocalDV, LocalDV->getDVObject(),
          GetTLI, ValueChecked, DL, ForDVCP, true /* AllowCheckForAllocSite*/)) {
        LocalDV->analyzeNestedDopeVector();
        NestedDV->collectFromCopy(*LocalDV);
      }
      // Delete the pointer since we already copied the information to the
      // current dope vector.
      delete LocalDV;
    }
  }
}

// Return true if the pointer address of the current global dope vector is a
// pointer to another dope vector, or if it is a pointer to a structure where
// at least one field is a dope vector. Else, return false.
bool GlobalDopeVector::isCandidateForNestedDopeVectors(const DataLayout &DL) {
  assert(Glob == GlobalDVInfo->getDVObject() &&
         "Dope vector object mismatch with global");

  StructType *DVStruct = GlobalDVInfo->getLLVMStructType();
  assert(DVStruct && "Analyzing dope vector without the proper structure");

  if (!Glob->getType()->getContext().supportsTypedPointers())
    return false; // FIXME to support opaque pointers
  
  PointerType *PtrAddr = dyn_cast<PointerType>(DVStruct->getElementType(0));
  if (!PtrAddr)
    return false;

  // TODO: This needs to be updated to handle opaque pointers.
  StructType *StrTy =
      dyn_cast<StructType>(PtrAddr->getNonOpaquePointerElementType());
  if (!StrTy)
    return false;

  if (isDopeVectorType(StrTy, DL))
    return true;

  for (unsigned I = 0, E = StrTy->getNumElements(); I < E; I++)
    if (isDopeVectorType(StrTy->getElementType(I), DL))
      return true;

  return false;
}

// This function will check if there are nested dope vectors for the global
// dope vector, collect the information and analyze if there is any illegal
// access that could invalidate the data.
void
GlobalDopeVector::collectAndAnalyzeNestedDopeVectors(const DataLayout &DL,
                                                     bool ForDVCP) {

  // NOTE: This needs to be updated for opaque pointers
  auto GetResultTypeFromSubscript = [](SubscriptInst *SI) {
    Type *ResType = SI->getType();
    if (ResType->isPointerTy())
      ResType = ResType->getNonOpaquePointerElementType();

    return ResType;
  };

  // First check if there is any nested dope vector. At this point we know that
  // the global is a dope vector, but double check.
  assert(Glob == GlobalDVInfo->getDVObject() &&
         "Dope vector object mismatch with global");

  // If the global dope vector info is not valid then there is no point for
  // collecting the nested dope vectors
  if (GlobalDVInfo->getAnalysisResult() ==
      DopeVectorInfo::AnalysisResult::AR_Invalid)
    return;

  // Don't perform any analysis for nested dope vectors if the current
  // global doesn't qualify for it.
#if INTEL_FEATURE_SW_ADVANCED
  // If aggressive DVCP is disabled then prefer to use the analysis
  // process to decide if the nested dope vectors is enabled
  if (EnableAggressiveDVCP && !isCandidateForNestedDopeVectors(DL))
    return;
#else // INTEL_FEATURE_SW_ADVANCED
  if (!isCandidateForNestedDopeVectors(DL))
    return;
#endif // INTEL_FEATURE_SW_ADVANCED

  auto *PtrAddressField = GlobalDVInfo->getDopeVectorField(
      DopeVectorFieldType::DV_ArrayPtr);

  // If the pointer address is Bottom or is written then we bail out. There is
  // an use of the pointer address that invalidates the nested dope vectors.
  if (PtrAddressField->getIsBottom() ||
      PtrAddressField->getIsWritten())
    return;

  Type *SubscriptResultType = nullptr;
  bool NestedDVDataValid = true;

  // Go through all the places where the global array is loaded
  for (auto *LI : PtrAddressField->loads()) {
    for (auto *U : LI->users()) {

      // The user of the global array will be subscript
      if (auto *SI = dyn_cast<SubscriptInst>(U)) {

        // Collect the type that the subscript is loading.
        // The type should be the same accross all subscripts.
        Type *CurrSIType = GetResultTypeFromSubscript(SI);
        if (CurrSIType->isPointerTy()) {
          NestedDVDataValid = false;
          break;
        } else if (!SubscriptResultType) {
          SubscriptResultType = CurrSIType;
        } else if (SubscriptResultType != CurrSIType) {
          NestedDVDataValid = false;
          break;
        }

        // If the subscript is not accessing a structure then there
        // won't be nested dope vectors, still we need to check that
        // all the subscripts are accessing the same type
        if (!SubscriptResultType->isStructTy())
          continue;

        // Collect the possible nested dope vectors that the subscript
        // could be accessing
        if (!collectNestedDopeVectorFromSubscript(SI, DL, ForDVCP)) {
          NestedDVDataValid = false;
          break;
        }
      } else {
        NestedDVDataValid = false;
        break;
      }
    }

    if (!NestedDVDataValid)
      break;
  }

  NestedDVDataCollected = NestedDVDataValid;
  if (!NestedDVDataValid || NestedDopeVectors.empty())
    return;

  // Analyze the nested dope vectors if they were collected correctly
  for (auto *NestedDV : NestedDopeVectors)
    NestedDV->analyzeNestedDopeVector();

  mergeNestedDopeVectors();

  // Collect any information from the copy dope vectors to check if we
  // can also propagate the constants to it.
  collectAndAnalyzeCopyNestedDopeVectors(DL, ForDVCP);
}

// Validate that the data was collected correctly for the global dope vector
void GlobalDopeVector::validateGlobalDopeVector(const DataLayout &DL) {

  // If the global dope vector info is not valid then the analysis for
  // the fields of the global dope vector failed
  if (GlobalDVInfo->getAnalysisResult() !=
      DopeVectorInfo::AnalysisResult::AR_Pass) {
    AnalysisRes = GlobalDopeVector::AnalysisResult::AR_GlobalDVAnalysisFailed;
    return;
  }

  // If NestedDVDataCollected is false then it means that something happened
  // while collecting the nested dope vectors
  if (!NestedDVDataCollected) {
#if INTEL_FEATURE_SW_ADVANCED
    // If aggressive DVCP is not enabled and NestedDVDataCollected is false,
    // then we are going to consider the whole dope vector invalid
    if (!EnableAggressiveDVCP) {
      AnalysisRes =
          GlobalDopeVector::AnalysisResult::AR_IncompleteNestedDVData;
      return;
    }
#endif // INTEL_FEATURE_SW_ADVANCED

    // If the pointer address is candidate for nested dope vectors then disable
    // DVCP. This is conservative, in reality, if we have all the information
    // for the outer dope vector, then we can propagate that information.
    if (isCandidateForNestedDopeVectors(DL)) {
      AnalysisRes =
          GlobalDopeVector::AnalysisResult::AR_IncompleteNestedDVData;
      return;
    }
  }

  // Check that all nested dope vectors were validated correctly
  for (auto *NestedDV : NestedDopeVectors) {
    if (NestedDV->getAnalysisResult() !=
        DopeVectorInfo::AnalysisResult::AR_Pass) {
      AnalysisRes = GlobalDopeVector::AnalysisResult::AR_BadNestedDV;
      return;
    }
  }

  // Analysis pass
  AnalysisRes = GlobalDopeVector::AnalysisResult::AR_Pass;
}

void GlobalDopeVector::collectAndValidate(const DataLayout &DL,
                                          bool ForDVCP) {
#if INTEL_FEATURE_SW_ADVANCED
  // Aggressive DVCP is turned on by default, collect the option from opt if
  // it is turned off.
  if (EnableAggressiveDVCP)
    EnableAggressiveDVCP = EnableAgressiveDVCPOpt;
#endif // INTEL_FEATURE_SW_ADVANCED

  bool isOpaquePtr = !Glob->getParent()->getContext().supportsTypedPointers();
  for (auto *U : Glob->users()) {
    if (auto *BC = dyn_cast<BitCastOperator>(U)) {
      // The BitCast should only be used for data allocation and
      // should happen only once
      if (collectAndAnalyzeAllocSite(BC))
        continue;
    } else if (auto *GEP = dyn_cast<GEPOperator>(U)) {
      // The fields of the global dope vector are accessed through
      // a GEPOperator
      if (collectAndAnalyzeGlobalDopeVectorField(GEP))
        continue;
    } else if (auto *CB = dyn_cast<CallBase>(U)) {
      // This is needed in the opaque pointer case because we cannot 
      // find the DV allocate function through bitcasts in the 
      // opaque pointer case.
      if (isOpaquePtr && !GlobalDVInfo->hasAllocSite()) {
        if (isCallToAllocFunction(CB, GetTLI)) {
          GlobalDVInfo->addAllocSite(CB);
          continue;
        }
      }
    } else if (isa<LoadInst>(U) || isa<StoreInst>(U)) {
      // This is needed in the opaque pointer case because
      // GEP(X,0,0) instructions may be dead code eliminated.
      if (isOpaquePtr) {
        auto *DVField = GlobalDVInfo->getDopeVectorField(DV_ArrayPtr);
        if (DVField && !DVField->getIsBottom()) {
          DVField->addFieldAddr(Glob);
          bool isNotForDVCP = DVField->isAddrNotForDVCP(Glob);
          DVField->analyzeLoadOrStoreInstruction(U, Glob, isNotForDVCP);
          if (!DVField->getIsBottom())
            continue;
        }
      }
    }
    // Any other use is invalid
    getGlobalDopeVectorInfo()->invalidateDopeVectorInfo();
    break;
  }

  // Make sure that none of the fields the in the dope vector are
  // set to bottom, and that an alloc site was seen.
  getGlobalDopeVectorInfo()->validateDopeVector();
  getGlobalDopeVectorInfo()->validateAllocSite();

  // Collect any information related to the nested dope vectors
  collectAndAnalyzeNestedDopeVectors(DL, ForDVCP);

  // Validate that the data was collected correctly
  validateGlobalDopeVector(DL);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DopeVectorInfo::print(uint64_t Indent) {

  auto SimpleFieldUsePrint = [Indent](DopeVectorFieldUse &Field,
                                      uint64_t ArrayEntry,
                                      DopeVectorFieldType FieldType) -> void {

    std::string IndentString(Indent, ' ');
    StringRef IndentRef(IndentString);
    switch(FieldType) {
      case DopeVectorFieldType::DV_ArrayPtr:
        dbgs() << IndentString << "  [0] Array Pointer: ";
        break;
      case DopeVectorFieldType::DV_ElementSize:
        dbgs() << IndentString << "  [1] Element size: ";
        break;
      case DopeVectorFieldType::DV_Codim:
        dbgs() << IndentString << "  [2] Co-Dimension: ";
        break;
      case DopeVectorFieldType::DV_Flags:
        dbgs() << IndentString << "  [3] Flags: ";
        break;
      case DopeVectorFieldType::DV_Dimensions:
        dbgs() << IndentString << "  [4] Dimensions: ";
        break;
      case DopeVectorFieldType::DV_PerDimensionArray:
        dbgs() << IndentString << "  [6] Per-dimension array: ";
        break;
      case DopeVectorFieldType::DV_ExtentBase:
        dbgs() << IndentString << "  [6][" << ArrayEntry <<"] Extent: ";
        break;
      case DopeVectorFieldType::DV_StrideBase:
        dbgs() << IndentString << "  [6][" << ArrayEntry <<"] Stride: ";
        break;
      case DopeVectorFieldType::DV_LowerBoundBase:
        dbgs() << IndentString << "  [6][" << ArrayEntry <<"] Lower Bound: ";
        break;
      default:
        llvm_unreachable("Trying to access a field in the dope vector "
                         "that should not be accessed");
    }

    std::string Result = "";

    if (Field.getIsRead())
      Result = "Read";

    if (Field.getIsWritten()) {
      if (!Result.empty())
        Result += " | ";
      Result += "Written";
    }

    if (Field.getIsBottom()) {
      if (!Result.empty())
        Result += " | ";
      Result += "Bottom";
    }

    dbgs() << Result;

    if (Field.getConstantValue()) {
      if (!Result.empty())
        dbgs() << " | ";
      dbgs() << "Constant = " << *(Field.getConstantValue());
    }

    dbgs() << "\n";

  };

  std::string IndentString(Indent, ' ');
  StringRef IndentRef(IndentString);
  dbgs() << IndentRef << "Dope vector analysis result: ";

  // The following switch statement doesn't include a default case because
  // clang throws an error if all the enum cases are covered explicitly and
  // a default case is added.
  assert(AnalysisRes <= DopeVectorInfo::AnalysisResult::AR_Pass &&
         "Invalid dope vector analysis result");

  switch (AnalysisRes) {
    case DopeVectorInfo::AnalysisResult::AR_Top:
      dbgs() << "Incomplete data collection";
      break;
    case DopeVectorInfo::AnalysisResult::AR_Invalid:
      dbgs() << "Invalid data collection";
      break;
    case DopeVectorInfo::AnalysisResult::AR_ReadIllegality:
      dbgs() << "Invalid dope vector field read";
      break;
    case DopeVectorInfo::AnalysisResult::AR_WriteIllegality:
      dbgs() << "Invalid dope vector field write";
      break;
    case DopeVectorInfo::AnalysisResult::AR_NoAllocSite:
      dbgs() << "Alloc site wasn't found";
      break;
    case DopeVectorInfo::AnalysisResult::AR_AllocStoreIllegality:
      dbgs() << "Store won't execute with alloc site";
      break;
    case DopeVectorInfo::AnalysisResult::AR_NoSingleNonNullValue:
      dbgs() << "Single non-null value expected";
      break;
    case DopeVectorInfo::AnalysisResult::AR_CouldNotMerge:
      dbgs() << "Could not merge dope vectors with multiple allocations";
      break;
    case DopeVectorInfo::AnalysisResult::AR_Pass:
      dbgs() << "Pass";
      break;
  }
  dbgs() << "\n";

  dbgs() << IndentRef << "Constant propagation status:"
         << (ConstantsPropagated ? " " : " NOT ") << "performed\n";

  SimpleFieldUsePrint(PtrAddr, 0, DopeVectorFieldType::DV_ArrayPtr);
  SimpleFieldUsePrint(ElementSizeAddr, 0, DopeVectorFieldType::DV_ElementSize);
  SimpleFieldUsePrint(CodimAddr, 0, DopeVectorFieldType::DV_Codim);
  SimpleFieldUsePrint(FlagsAddr, 0, DopeVectorFieldType::DV_Flags);
  SimpleFieldUsePrint(DimensionsAddr, 0, DopeVectorFieldType::DV_Dimensions);
  for (uint64_t I = 0; I < Rank; I++) {
    SimpleFieldUsePrint(ExtentAddr[I], I, DopeVectorFieldType::DV_ExtentBase);
    SimpleFieldUsePrint(StrideAddr[I], I, DopeVectorFieldType::DV_StrideBase);
    SimpleFieldUsePrint(LowerBoundAddr[I], I,
        DopeVectorFieldType::DV_LowerBoundBase);
  }
}

void GlobalDopeVector::print() {
  dbgs() << "Global variable: " << Glob->getName() << "\n";
  dbgs() << "  LLVM Type: "
         << GlobalDVInfo->getLLVMStructType()->getName() << "\n";
  dbgs() << "  Global dope vector result: ";

  // The following switch statement doesn't include a default case because
  // clang throws an error if all the enum cases are covered explicitly and
  // a default case is added.
  assert(AnalysisRes <= GlobalDopeVector::AnalysisResult::AR_Pass &&
         "Invalid global dope vector analysis result");

  switch(AnalysisRes) {
    case GlobalDopeVector::AnalysisResult::AR_Top:
      dbgs() << "Analysis not performed";
      break;
    case GlobalDopeVector::AnalysisResult::AR_GlobalDVAnalysisFailed:
      dbgs() << "Failed to collect global dope vector info";
      break;
    case GlobalDopeVector::AnalysisResult::AR_IncompleteNestedDVData:
      dbgs() << "Failed to collect nested dope vectors' data";
      break;
    case GlobalDopeVector::AnalysisResult::AR_BadNestedDV:
      dbgs() << "At least one nested dope vector didn't pass analysis";
      break;
    case GlobalDopeVector::AnalysisResult::AR_Pass:
      dbgs() << "Pass";
      break;
  }

  dbgs() << "\n";
  GlobalDVInfo->print(2);
  if (NestedDopeVectors.size() > 0) {
    // Sort the result for consistent printing
    std::vector<std::tuple<uint64_t, StringRef, NestedDopeVectorInfo *>> NDV;
    for (auto NestedDV : NestedDopeVectors)
      NDV.push_back(std::make_tuple(NestedDV->getFieldNum(),
                                    NestedDV->getLLVMStructType()->getName(),
                                    NestedDV));
    std::sort(NDV.begin(), NDV.end());
    dbgs() << "  Nested dope vectors: " << NDV.size() << "\n";
    for (auto &OneNDV : NDV) {
      dbgs() << "    Field[" << std::get<0>(OneNDV) << "]: "
             << std::get<1>(OneNDV) << "\n";
      std::get<2>(OneNDV)->print(6);
    }
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

} // end namespace dvanalysis

} // end namespace llvm

