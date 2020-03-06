//===-----------DTransTypes.cpp - Type model for DTrans -------------------===//
//
// Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/DTransTypes.h"

#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

#include <map>

namespace llvm {
namespace dtrans {

//////////////////////////////////////////////////////////////////////////////
// Methods for DTransType
//////////////////////////////////////////////////////////////////////////////
DTransType *DTransType::getPointerElementType() const {
  assert(isPointerTy() && "Must be DTransPointerType");
  return cast<DTransPointerType>(this)->getPointerElementType();
}

DTransType *DTransType::getArrayElementType() const {
  assert(isArrayTy() && "Must be DTransArrayType");
  return cast<DTransArrayType>(this)->getElementType();
}

DTransType *DTransType::getVectorElementType() const {
  assert(isVectorTy() && "Must be DTransVectorType");
  return cast<DTransVectorType>(this)->getElementType();
}

llvm::Type *DTransType::getLLVMType() const {
  switch (ID) {
  case DTransAtomicTypeID:
    return cast<DTransAtomicType>(this)->getLLVMType();
  case DTransPointerTypeID:
    return cast<DTransPointerType>(this)->getLLVMType();
  case DTransStructTypeID:
    return cast<DTransStructType>(this)->getLLVMType();
  case DTransArrayTypeID:
    return cast<DTransSequentialType>(this)->getLLVMType();
  case DTransVectorTypeID:
    return cast<DTransSequentialType>(this)->getLLVMType();
  case DTransFunctionTypeID:
    return cast<DTransFunctionType>(this)->getLLVMType();
  }

  llvm_unreachable("Switch table not completely covered");
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DTransType::dump() const { print(llvm::dbgs()); }

void DTransType::print(raw_ostream &OS, bool Detailed) const {
  switch (ID) {
  case DTransAtomicTypeID:
    cast<DTransAtomicType>(this)->print(OS);
    break;
  case DTransPointerTypeID:
    cast<DTransPointerType>(this)->print(OS);
    break;
  case DTransStructTypeID:
    cast<DTransStructType>(this)->print(OS, Detailed);
    break;
  case DTransArrayTypeID:
    cast<DTransArrayType>(this)->print(OS);
    break;
  case DTransVectorTypeID:
    cast<DTransVectorType>(this)->print(OS);
    break;
  case DTransFunctionTypeID:
    cast<DTransFunctionType>(this)->print(OS);
    break;
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

bool DTransType::compare(const DTransType &Other) const {
  if (this == &Other)
    return true;

  if (getTypeID() != Other.getTypeID())
    return false;

  switch (ID) {
  case DTransAtomicTypeID:
    return cast<DTransAtomicType>(this)->compare(
        *(cast<DTransAtomicType>(&Other)));
  case DTransPointerTypeID:
    return cast<DTransPointerType>(this)->compare(
        *(cast<DTransPointerType>(&Other)));
  case DTransStructTypeID:
    return cast<DTransStructType>(this)->compare(
        *(cast<DTransStructType>(&Other)));
  case DTransArrayTypeID:
    return cast<DTransSequentialType>(this)->compare(
        *(cast<DTransSequentialType>(&Other)));
  case DTransVectorTypeID:
    return cast<DTransSequentialType>(this)->compare(
        *(cast<DTransSequentialType>(&Other)));
  case DTransFunctionTypeID:
    return cast<DTransFunctionType>(this)->compare(
        *(cast<DTransFunctionType>(&Other)));
  }
  llvm_unreachable("Switch table incomplete");
}

//////////////////////////////////////////////////////////////////////////////
// Methods for DTransAtomicType
//////////////////////////////////////////////////////////////////////////////
DTransAtomicType *DTransAtomicType::get(DTransTypeManager &TM, llvm::Type *Ty) {
  return TM.getOrCreateAtomicType(Ty);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DTransAtomicType::print(raw_ostream &OS) const { OS << *LLVMType; }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

//////////////////////////////////////////////////////////////////////////////
// Methods for DTransPointerType
//////////////////////////////////////////////////////////////////////////////
DTransPointerType *DTransPointerType::get(DTransTypeManager &TM,
                                          DTransType *PointeeTy) {
  return TM.getOrCreatePointerType(PointeeTy);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DTransPointerType::print(raw_ostream &OS) const {
  unsigned Level = 1;
  auto *BaseTI = PointeeType;
  while (BaseTI->isPointerTy()) {
    Level++;
    BaseTI = BaseTI->getPointerElementType();
  }

  BaseTI->print(OS, false);
  while (Level--)
    OS << '*';
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

//////////////////////////////////////////////////////////////////////////////
// Methods for DTransCompositeType
//////////////////////////////////////////////////////////////////////////////
bool DTransCompositeType::indexValid(unsigned Idx) const {
  if (auto *DtSTy = dyn_cast<DTransStructType>(this))
    return Idx < DtSTy->getNumFields();

  // Sequential types can be indexed by any integer.
  return true;
}

//////////////////////////////////////////////////////////////////////////////
// Methods for DTransStructType
//////////////////////////////////////////////////////////////////////////////

// Get or create a structure that will be used to map to the structure \p Ty.
DTransStructType *DTransStructType::get(DTransTypeManager &TM,
                                        llvm::StructType *Ty) {
  return TM.getOrCreateStructType(Ty);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DTransStructType::print(raw_ostream &OS, bool Detailed) const {
  if (getReconstructError())
    OS << "Metadata mismatch: ";

  // For compatibility with the way struct names are printed for
  // llvm::StructType, some names will be quoted.
  auto ShouldQuoteName = [](StringRef S) { return S.contains(':'); };

  if (!isLiteralStruct()) {
    assert(hasName() && "Non-literal structs should have names");
    if (ShouldQuoteName(getName()))
      OS << "%\"" << getName() << "\"";
    else
      OS << "%" << getName();
  } else {
    // Print a literal type as its contents
    OS << "{ ";
    bool First = true;
    for (auto &Field : getFields()) {
      if (!First)
        OS << ", ";
      Field.print(OS, false);
      First = false;
    }
    OS << " }";
  }

  if (Detailed) {
    // Print the structure members
    OS << "\n";
    if (getNumFields() == 0) {
      if (IsOpaque)
        OS << "  opaque";
      else
        OS << "  empty";
      return;
    }

    unsigned Number = 0;
    for (auto &Field : getFields()) {
      OS << format_decimal(Number++, 3) << ") ";
      Field.print(OS, false);
      OS << "\n";
    }
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

//////////////////////////////////////////////////////////////////////////////
// Methods for DTransArrayType
//////////////////////////////////////////////////////////////////////////////
DTransArrayType *DTransArrayType::get(DTransTypeManager &TM,
                                      DTransType *ElemType, uint64_t Num) {
  return TM.getOrCreateArrayType(ElemType, Num);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DTransArrayType::print(raw_ostream &OS) const {
  OS << "[" << getNumElements() << " x ";
  getTypeAtIndex(0)->print(OS, false);
  OS << "]";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

//////////////////////////////////////////////////////////////////////////////
// Methods for DTransVectorType
//////////////////////////////////////////////////////////////////////////////
DTransVectorType *DTransVectorType::get(DTransTypeManager &TM,
                                        DTransType *ElemType, uint64_t Num) {
  return TM.getOrCreateVectorType(ElemType, Num);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DTransVectorType::print(raw_ostream &OS) const {
  OS << "<" << getNumElements() << " x ";
  getTypeAtIndex(0)->print(OS, false);
  OS << ">";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

//////////////////////////////////////////////////////////////////////////////
// Methods for DTransFunctionType
//////////////////////////////////////////////////////////////////////////////
DTransFunctionType *
DTransFunctionType::get(DTransTypeManager &TM, dtrans::DTransType *DTRetTy,
                        SmallVectorImpl<dtrans::DTransType *> &ParamTypes,
                        bool IsVarArg) {
  return TM.getOrCreateFunctionType(DTRetTy, ParamTypes, IsVarArg);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DTransFunctionType::print(raw_ostream &OS) const {
  DTransType *DTRetTy = getReturnType();
  if (DTRetTy)
    DTRetTy->print(OS, false);
  else
    OS << "<UNKNOWN RET TYPE>";

  OS << " (";
  bool First = true;
  for (auto *Arg : args()) {
    if (!First)
      OS << ", ";
    if (Arg)
      Arg->print(OS, false);
    else
      OS << "<UNKNOWN ARG TYPE>";
    First = false;
  }

  if (isVarArg()) {
    if (!First)
      OS << ", ";
    OS << "...";
  }
  OS << ")";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

//////////////////////////////////////////////////////////////////////////////
// Methods for DTransFieldMember
//////////////////////////////////////////////////////////////////////////////
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DTransFieldMember::print(raw_ostream &OS, bool) const {
  // Warn if multiple types were collected for the field.
  if (DTTypes.size() > 1)
    OS << "<HAS CONFLICTS>\n";

  bool First = true;
  for (auto *DType : DTTypes) {
    if (!First)
      OS << "\n     ";
    DType->print(OS, false);
    First = false;
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

//////////////////////////////////////////////////////////////////////////////
// Methods for DTransTypeManager
//////////////////////////////////////////////////////////////////////////////
DTransTypeManager::~DTransTypeManager() {
  // This destructor destroys all the DTransTypes constructed with this type
  // manager. Note, it is important that the destructor members of DTransType
  // objects do not attempt to access any handles they contain to DTransType
  // objects because there is no enforced ordering when running the destructors
  // because all objects are being deleted.

  for (auto P : TypeInfoMap)
    DeleteType(P.second);
  TypeInfoMap.clear();

  for (auto P : PointerTypeInfoMap)
    DeleteType(P.second);
  PointerTypeInfoMap.clear();

  // The following cases can invoke 'delete' directly on the object because each
  // container only hold a single type, which is known at compile time.
  for (auto &P : ArrayTypeInfoMap)
    delete P.second;
  ArrayTypeInfoMap.clear();

  for (auto &P : VecTypeInfoMap)
    delete P.second;
  VecTypeInfoMap.clear();

  for (auto *FTy : FunctionTypeVec)
    delete FTy;
  FunctionTypeVec.clear();

  for (auto &P : StructTypeInfoMap)
    delete P.second;
  StructTypeInfoMap.clear();
}

// Invoke the appropriate delete method based on the object type.
void DTransTypeManager::DeleteType(DTransType *DTTy) {
  switch (DTTy->getTypeID()) {
  case DTransType::DTransAtomicTypeID:
    delete cast<DTransAtomicType>(DTTy);
    break;
  case DTransType::DTransPointerTypeID:
    delete cast<DTransPointerType>(DTTy);
    break;
  case DTransType::DTransStructTypeID:
    delete cast<DTransStructType>(DTTy);
    break;
  case DTransType::DTransArrayTypeID:
    delete cast<DTransArrayType>(DTTy);
    break;
  case DTransType::DTransVectorTypeID:
    delete cast<DTransVectorType>(DTTy);
    break;
  case DTransType::DTransFunctionTypeID:
    delete cast<DTransFunctionType>(DTTy);
    break;
  }
}

DTransAtomicType *DTransTypeManager::getOrCreateAtomicType(llvm::Type *Ty) {
  if (auto *Existing = findType(Ty))
    return cast<DTransAtomicType>(Existing);

  auto *DTType = new DTransAtomicType(Ty);
  TypeInfoMap.insert(std::make_pair(Ty, DTType));
  return DTType;
}

DTransPointerType *
DTransTypeManager::getOrCreatePointerType(DTransType *PointeeTy) {
  assert(PointeeTy && "PointeeTy must be not be nullptr");

  auto Existing = PointerTypeInfoMap.find(PointeeTy);
  if (Existing != PointerTypeInfoMap.end())
    return (*Existing).getSecond();

  auto DTPtrTy = new DTransPointerType(PointeeTy->getContext(), PointeeTy);
  PointerTypeInfoMap.insert(std::make_pair(PointeeTy, DTPtrTy));
  return DTPtrTy;
}

DTransStructType *
DTransTypeManager::getOrCreateStructType(llvm::StructType *StTy) {
  // Literal types should not be created by this routine because this
  // routine uses the structure name for mapping to existing types.
  assert(!StTy->isLiteral() &&
         "Cannot use getOrCreateStructType for literal types");

  if (auto *Existing = getStructType(StTy->getName()))
    return Existing;

  if (StTy->isOpaque()) {
    DTransStructType *DTransStTy = new DTransStructType(
        StTy->getContext(), StTy, std::string(StTy->getName()), 0,
        /*IsOpaque=*/true);
    StructTypeInfoMap.insert(std::make_pair(StTy->getName(), DTransStTy));

    return DTransStTy;
  }

  // Create placeholders for the field types. The actual types will be populated
  // later with calls to addResolvedType.
  SmallVector<DTransFieldMember, 16> Fields;
  for (unsigned Idx = 0, Num = StTy->getNumElements(); Idx < Num; ++Idx)
    Fields.push_back(DTransFieldMember(nullptr));

  DTransStructType *DTransStTy = new DTransStructType(StTy, Fields);
  StructTypeInfoMap.insert(std::make_pair(StTy->getName(), DTransStTy));
  return DTransStTy;
}

DTransStructType *DTransTypeManager::getStructType(StringRef Name) {
  auto It = StructTypeInfoMap.find(Name);
  if (It != StructTypeInfoMap.end())
    return cast<DTransStructType>(It->second);

  return nullptr;
}

DTransStructType *DTransTypeManager::getOrCreateLiteralStructType(
    LLVMContext &Ctx, ArrayRef<DTransType *> FieldTypes) {
  SmallVector<DTransFieldMember, 8> Fields;
  for (auto FTy : FieldTypes)
    Fields.push_back(DTransFieldMember(FTy));

  // Create a temporary instance of the literal struct type that can be used
  // for searching for a match to an existing type.
  std::unique_ptr<DTransStructType> DTransLitTy(
      new DTransStructType(Ctx, Fields));
  for (auto *Ty : LitStructTypeVec)
    if (Ty->compare(*DTransLitTy))
      return Ty;

  auto *NewTy = DTransLitTy.release();
  LitStructTypeVec.push_back(NewTy);
  return NewTy;
}

DTransArrayType *DTransTypeManager::getOrCreateArrayType(DTransType *ElemType,
                                                         uint64_t Num) {
  assert(ElemType && "getOrCreateArrayType must have non-null element type");
  auto Existing = ArrayTypeInfoMap.find(std::make_pair(ElemType, Num));
  if (Existing != ArrayTypeInfoMap.end())
    return (*Existing).getSecond();

  auto *DTArrTy = new DTransArrayType(ElemType->getContext(), ElemType, Num);
  ArrayTypeInfoMap.insert(
      std::make_pair(std::make_pair(ElemType, Num), DTArrTy));
  return DTArrTy;
}

DTransVectorType *DTransTypeManager::getOrCreateVectorType(DTransType *ElemType,
                                                           uint64_t Num) {
  assert(ElemType && "getOrCreateVectorType must have non-null element type");
  auto Existing = VecTypeInfoMap.find(std::make_pair(ElemType, Num));
  if (Existing != VecTypeInfoMap.end())
    return (*Existing).getSecond();

  auto *DTVecTy = new DTransVectorType(ElemType->getContext(), ElemType, Num);
  VecTypeInfoMap.insert(std::make_pair(std::make_pair(ElemType, Num), DTVecTy));
  return DTVecTy;
}

DTransFunctionType *DTransTypeManager::getOrCreateFunctionType(
    dtrans::DTransType *DTRetTy,
    SmallVectorImpl<dtrans::DTransType *> &ParamTypes, bool IsVarArg) {

  auto CompareFunctionTypes = [](DTransFunctionType *DTFnTy,
                                 DTransType *DTRetTy,
                                 SmallVectorImpl<DTransType *> &ParamTypes,
                                 bool IsVarArg) {
    if (DTFnTy->getNumArgs() != ParamTypes.size())
      return false;

    if (DTFnTy->isVarArg() != IsVarArg)
      return false;

    if (!DTFnTy->getReturnType() || !DTFnTy->getReturnType()->compare(*DTRetTy))
      return false;

    unsigned AI = 0;
    for (auto CurParamTy : DTFnTy->args()) {
      assert(ParamTypes[AI] && "Should be non-null when AllAnalyzed is true");
      if (!CurParamTy || !CurParamTy->compare(*ParamTypes[AI]))
        return false;
      ++AI;
    }

    return true;
  };

  // Check for an existing function type that matches the signature
  for (auto *DTFnTy : FunctionTypeVec)
    if (CompareFunctionTypes(DTFnTy, DTRetTy, ParamTypes, IsVarArg))
      return DTFnTy;

  auto *NewDTFnTy = new DTransFunctionType(Ctx, ParamTypes.size(), IsVarArg);
  NewDTFnTy->setReturnType(DTRetTy);
  unsigned AI = 0;
  for (auto ParamTy : ParamTypes)
    NewDTFnTy->setArgType(AI++, ParamTy);

  FunctionTypeVec.push_back(NewDTFnTy);
  return NewDTFnTy;
}

DTransType *DTransTypeManager::findType(llvm::Type *Ty) const {
  auto Existing = TypeInfoMap.find(Ty);
  if (Existing == TypeInfoMap.end())
    return nullptr;
  return (*Existing).getSecond();
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DTransTypeManager::printTypes() const {
  dbgs() << "\nStructTypes\n";
  dbgs() << "--------------------------------------------------------\n";

  // Create a map for holding the types sorted by name.
  std::map<std::string, DTransType *> SMap;
  for (auto &P : StructTypeInfoMap) {
    DTransStructType *StTy = P.second;
    if (StTy->hasName())
      SMap.insert(std::make_pair(std::string(StTy->getName()), P.second));
    else {
      std::string Name;
      raw_string_ostream NameStr(Name);
      P.second->print(NameStr, false);
      SMap.insert(std::make_pair(Name, P.second));
    }
  }

  for (auto &P : SMap) {
    dbgs() << "StructType: ";
    P.second->print(dbgs(), true);
    dbgs() << "\n";
  }

  dbgs() << "\n\nArrayTypes\n";
  dbgs() << "--------------------------------------------------------\n";
  printCollectionSorted(
      dbgs(), ArrayTypeInfoMap.begin(), ArrayTypeInfoMap.end(), "\n",
      [](const std::pair<std::pair<DTransType *, uint64_t>, DTransArrayType *>
             Elem) {
        std::string OutputVal;
        raw_string_ostream OutputStream(OutputVal);
        OutputStream << "ArrayType: " << *(Elem.second);
        OutputStream.flush();
        return OutputVal;
      });

  dbgs() << "\n\nPointerTypes\n";
  dbgs() << "--------------------------------------------------------\n";
  printCollectionSorted(
      dbgs(), PointerTypeInfoMap.begin(), PointerTypeInfoMap.end(), "\n",
      [](const std::pair<DTransType *, DTransPointerType *> Elem) {
        std::string OutputVal;
        raw_string_ostream OutputStream(OutputVal);
        OutputStream << "PointerType: " << *(Elem.second);
        OutputStream.flush();
        return OutputVal;
      });

  // TODO: Add printing of literal structures, vectors, and function types, if
  // useful.

  dbgs() << "\n\n";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

} // end namespace dtrans
} // end namespace llvm
