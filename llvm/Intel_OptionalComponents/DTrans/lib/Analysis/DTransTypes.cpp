//===-----------DTransTypes.cpp - Type model for DTrans -------------------===//
//
// Copyright (C) 2019-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/DTransTypes.h"

#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

#include <map>

namespace llvm {
namespace dtransOP {

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

uint32_t DTransType::getNumContainedElements() const {
  switch (ID) {
  default:
     return 0;
  case DTransStructTypeID:
    return cast<DTransStructType>(this)->getNumFields();
  case DTransArrayTypeID:
    return cast<DTransSequentialType>(this)->getNumElements();
  }
  llvm_unreachable("Switch table not completely covered");
}

// The base class just dispatches the request to one of the
// derived classes based on the actual type of this DTransType object.
MDNode *DTransType::createMetadataReference() const {
  switch (ID) {
  case DTransAtomicTypeID:
    return cast<DTransAtomicType>(this)->createMetadataReference();
  case DTransPointerTypeID:
    return cast<DTransPointerType>(this)->createMetadataReference();
  case DTransStructTypeID:
    return cast<DTransStructType>(this)->createMetadataReference();
  case DTransArrayTypeID:
    return cast<DTransSequentialType>(this)->createMetadataReference();
  case DTransVectorTypeID:
    return cast<DTransSequentialType>(this)->createMetadataReference();
  case DTransFunctionTypeID:
    return cast<DTransFunctionType>(this)->createMetadataReference();
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

// For atomic types the general form is:
//   !{<type> zeroinitializer, i32 <pointer level>}
// For 'void' types or 'metadata' types there is a special form:
//   !{!"void", i32 <pointer level>}
//   !{!"metadata", i32 <pointer level>}
MDNode *DTransAtomicType::createMetadataReference(unsigned PtrLevel) const {
  LLVMContext &Ctx = getContext();

  if (isVoidTy()) {
    // Special encoding for VoidTy.
    return MDNode::get(Ctx, {MDString::get(Ctx, "void"),
                             ConstantAsMetadata::get(ConstantInt::get(
                                 Type::getInt32Ty(Ctx), PtrLevel))});
  } else if (isMetadataTy()) {
    // Special encoding for MetadataTy.
    return MDNode::get(Ctx, {MDString::get(Ctx, "metadata"),
                             ConstantAsMetadata::get(ConstantInt::get(
                                 Type::getInt32Ty(Ctx), PtrLevel))});
  }

  return MDNode::get(
      Ctx, {ConstantAsMetadata::get(Constant::getNullValue(getLLVMType())),
            ConstantAsMetadata::get(
                ConstantInt::get(Type::getInt32Ty(Ctx), PtrLevel))});
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

// For pointer types the general form is:
//   !{!MDNode, i32 <pointer level>}
// to reference another encoded type, and specify the level of pointer
// indirection.
//
// However, some types will use an abbreviated form when the
// type can be encoded directly as the first operand of the
// metadata, such as:
//   !{float 0.0, i32 1} ; float*
//   !{%struct.test zeroinitializer, i32 2} ; %struct.test**
MDNode *DTransPointerType::createMetadataReference() const {
  LLVMContext &Ctx = getContext();
  unsigned PtrLevel = 1;
  DTransType *BaseTy = PointeeType;
  while (BaseTy->isPointerTy()) {
    ++PtrLevel;
    BaseTy = (cast<DTransPointerType>(BaseTy))->getPointerElementType();
  }

  if (BaseTy->isAtomicTy())
    return (cast<DTransAtomicType>(BaseTy))->createMetadataReference(PtrLevel);
  if (BaseTy->isStructTy() &&
      !(cast<DTransStructType>(BaseTy))->isLiteralStruct())
    return (cast<DTransStructType>(BaseTy))->createMetadataReference(PtrLevel);

  return MDNode::get(Ctx, {BaseTy->createMetadataReference(),
                           ConstantAsMetadata::get(ConstantInt::get(
                               Type::getInt32Ty(Ctx), PtrLevel))});
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

// Named structures have the form of a zero initialized instance and
// a level of pointer indirection:
//   !{%struct.test zeroinitializer, i32 2} ; %struct.test**
//
// Literal structures use the form:
//   !{!"L", i32 <numFields>, !FieldTy0_MD, !FieldTy1_MD, ... }
MDNode *DTransStructType::createMetadataReference(unsigned PtrLevel) const {
  LLVMContext &Ctx = getContext();

  if (isLiteralStruct()) {
    SmallVector<Metadata *, 16> MDOps;
    MDOps.emplace_back(MDString::get(Ctx, "L"));
    MDOps.emplace_back(ConstantAsMetadata::get(
        ConstantInt::get(Type::getInt32Ty(Ctx), getNumFields())));
    for (auto &FieldMember : elements()) {
      DTransType *FieldTy = FieldMember.getType();
      assert(FieldTy && "Field member type not set");
      MDOps.emplace_back(FieldTy->createMetadataReference());
    }
    return MDTuple::get(Ctx, MDOps);
  }

  return MDNode::get(
      Ctx, {ConstantAsMetadata::get(Constant::getNullValue(getLLVMType())),
            ConstantAsMetadata::get(
                ConstantInt::get(Type::getInt32Ty(Ctx), PtrLevel))});
}

// The general form of the metadata node is:
//   !{!"S", %struct.test zeroinitializer, i32 <numFields>,
//     !FieldTy0_MD, !FieldTy1_MD, ... }
// Opaque structures use a numFields value of -1.
MDNode *DTransStructType::createMetadataStructureDescriptor() const {
  assert(!isLiteralStruct() && "Method is for named structs only");

  LLVMContext &Ctx = getContext();
  SmallVector<Metadata *, 16> MDOps;
  MDOps.emplace_back(MDString::get(Ctx, "S"));
  MDOps.emplace_back(
      ConstantAsMetadata::get(Constant::getNullValue(getLLVMType())));
  if (isOpaque())
    MDOps.emplace_back(
        ConstantAsMetadata::get(ConstantInt::get(Type::getInt32Ty(Ctx), -1)));
  else
    MDOps.emplace_back(ConstantAsMetadata::get(
        ConstantInt::get(Type::getInt32Ty(Ctx), getNumFields())));

  for (auto &FieldMember : elements()) {
    DTransType *FieldTy = FieldMember.getType();
    assert(FieldTy && "Field member type not set");
    MDOps.emplace_back(FieldTy->createMetadataReference());
  }

  return MDTuple::get(Ctx, MDOps);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DTransStructType::print(raw_ostream &OS, bool Detailed) const {
  auto PrintStructBody = [](raw_ostream &OS,
                            const SmallVectorImpl<DTransFieldMember> &Fields) {
    bool First = true;
    OS << "{ ";
    for (auto &Field : Fields) {
      if (!First)
        OS << ", ";
      Field.print(OS, false);
      First = false;
    }
    OS << " }";
  };

  if (getReconstructError())
    OS << "Metadata mismatch: ";

  // For compatibility with the way llvm::StructType names are printed, names
  // with certain non-alpha numeric characters will be quoted.
  auto ShouldQuoteName = [](StringRef S) {
    for (auto C : S)
      if (!isalnum(static_cast<unsigned char>(C)) && C != '-' && C != '.' &&
          C != '_')
        return true;
    return false;
  };

  bool IsLiteral = isLiteralStruct();
  if (!IsLiteral) {
    assert(hasName() && "Non-literal structs should have names");
    if (ShouldQuoteName(getName()))
      OS << "%\"" << getName() << "\"";
    else
      OS << "%" << getName();
  } else {
    // Print a literal type as its contents
    PrintStructBody(OS, getFields());
  }

  if (Detailed) {
    // Print the structure members
    if (getNumFields() == 0) {
      if (IsOpaque)
        OS << " = type opaque";
      else
        OS << " = type {}";
      return;
    }

    OS << " = type ";
    PrintStructBody(OS, getFields());
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// For array types the form is:
//   !{!"A", i32 <numElements>, !MDNode}
// For vector types the form is:
//   !{!"V", i32 <numElements>, !MDNode}
// Where the referenced MDNode contains the element type.
MDNode *DTransSequentialType::createMetadataReference() const {
  LLVMContext &Ctx = getContext();
  return MDNode::get(Ctx, {MDString::get(Ctx, (isArrayTy() ? "A" : "V")),
                           ConstantAsMetadata::get(ConstantInt::get(
                               Type::getInt32Ty(Ctx), getNumElements())),
                           getElementType()->createMetadataReference()});
}

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

// For function types the form is:
//   !{!"F", i1 <isVarArg>, i32 <numArgs>, !RetTy_MD,
//     !ArgTy0_MD, !ArgTy1_MD, ...}
// Where references to other MDNodes are used for the return type, and each
// argument type.
MDNode *DTransFunctionType::createMetadataReference() const {
  LLVMContext &Ctx = getContext();
  SmallVector<Metadata *, 16> MDOps;
  MDOps.emplace_back(MDString::get(Ctx, "F"));
  MDOps.emplace_back(ConstantAsMetadata::get(
      ConstantInt::get(Type::getInt1Ty(Ctx), isVarArg() ? 1 : 0)));
  MDOps.emplace_back(ConstantAsMetadata::get(
      ConstantInt::get(Type::getInt32Ty(Ctx), getNumArgs())));
  DTransType *DTRetTy = getReturnType();
  assert(DTRetTy && "Function return type should have been set");
  MDOps.emplace_back(DTRetTy->createMetadataReference());
  for (auto *Arg : args()) {
    assert(Arg && "Function arg type should have been set");
    MDOps.emplace_back(Arg->createMetadataReference());
  }

  return MDTuple::get(Ctx, MDOps);
}

DTransFunctionType *
DTransFunctionType::get(DTransTypeManager &TM, DTransType *DTRetTy,
                        SmallVectorImpl<DTransType *> &ParamTypes,
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
    OS << "<HAS CONFLICTS> ";

  bool First = true;
  for (auto *DType : DTTypes) {
    // Structure fields should only have a single type. However, the data
    // structure allows multiple types to aid in debugging problems with
    // conflicting information in the metadata. Mark these additional types when
    // printing.
    if (!First)
      OS << " #";
    DType->print(OS, false);
    if (!First)
      OS << "#";
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

  for (auto &FTyNode : FunctionTypeNodes)
    delete FTyNode.getFunctionType();
  FunctionTypeNodes.clear();

  for (auto &P : StructTypeInfoMap)
    delete P.second;
  StructTypeInfoMap.clear();
  AllDTransTypes.clear();
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
  AllDTransTypes.push_back(DTType);
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
  AllDTransTypes.push_back(DTPtrTy);
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
    AllDTransTypes.push_back(DTransStTy);
    return DTransStTy;
  }

  // Create placeholders for the field types. The actual types will be populated
  // later with calls to addResolvedType.
  SmallVector<DTransFieldMember, 16> Fields;
  for (unsigned Idx = 0, Num = StTy->getNumElements(); Idx < Num; ++Idx)
    Fields.push_back(DTransFieldMember(nullptr));

  DTransStructType *DTransStTy = new DTransStructType(StTy, Fields);
  StructTypeInfoMap.insert(std::make_pair(StTy->getName(), DTransStTy));
  AllDTransTypes.push_back(DTransStTy);
  return DTransStTy;
}

DTransStructType *DTransTypeManager::getStructType(StringRef Name) const {
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
  AllDTransTypes.push_back(NewTy);
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
  AllDTransTypes.push_back(DTArrTy);
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
  AllDTransTypes.push_back(DTVecTy);
  return DTVecTy;
}

DTransFunctionType *DTransTypeManager::getOrCreateFunctionType(
    DTransType *DTRetTy, ArrayRef<DTransType *> ParamTypes,
    bool IsVarArg) {
  llvm::FoldingSetNodeID Profile;
  DTransFunctionTypeNode::generateProfile(DTRetTy, ParamTypes, IsVarArg,
                                          Profile);
  void *IP = nullptr;
  DTransFunctionTypeNode *N =
      FunctionTypeNodes.FindNodeOrInsertPos(Profile, IP);
  if (N)
    return N->getFunctionType();

  auto *NewDTFnTy = new DTransFunctionType(Ctx, ParamTypes.size(), IsVarArg);
  NewDTFnTy->setReturnType(DTRetTy);
  unsigned AI = 0;
  for (auto ParamTy : ParamTypes)
    NewDTFnTy->setArgType(AI++, ParamTy);

  DTransFunctionTypeNode *NewN =
      new (Allocator) DTransFunctionTypeNode(NewDTFnTy);
  FunctionTypeNodes.InsertNode(NewN, IP);
  AllDTransTypes.push_back(NewDTFnTy);
  return NewDTFnTy;
}

// A simple type is one that is not a pointer, and does not contain any
// elements that are pointers, or a named structure type that's already
// been created.
bool DTransTypeManager::isSimpleType(llvm::Type *Ty) const {
  if (auto *StTy = dyn_cast<StructType>(Ty)) {
    // If the struct type has already been created, a reference to the type
    // can be returned, so treat it as simple.
    if (!StTy->isLiteral() && getStructType(StTy->getName()))
      return true;

    for (llvm::Type *FieldTy : StTy->elements())
      if (!isSimpleType(FieldTy))
        return false;

    return true;
  }

  if (isa<ScalableVectorType>(Ty))
    // TODO: We don't support scalable vector types in our model. If needed in
    // the future, changes will need to be made to DTransVectorType to support
    // it.
    return false;

  return !dtrans::hasPointerType(Ty);
}

DTransType *DTransTypeManager::getOrCreateSimpleType(llvm::Type *Ty) {
  if (Ty->isPointerTy())
    return nullptr;

  if (isa<ArrayType>(Ty)) {
    // To support the construction of nested arrays, build a stack of types so
    // the DTransTypes can be constructed from the innermost to the outermost.
    SmallVector<llvm::ArrayType *, 4> TypeStack;
    llvm::Type *BaseTy = Ty;
    while (BaseTy->isArrayTy()) {
      TypeStack.push_back(cast<ArrayType>(BaseTy));
      BaseTy = BaseTy->getArrayElementType();

      // Pointers are not simple types. Cannot create the type.
      if (BaseTy->isPointerTy())
        return nullptr;
    }

    DTransType *LastTy = getOrCreateSimpleType(BaseTy);
    while (!TypeStack.empty()) {
      auto *ArrTy = TypeStack.back();
      TypeStack.pop_back();
      uint64_t Num = ArrTy->getNumElements();
      LastTy = getOrCreateArrayType(LastTy, Num);
    }

    return LastTy;
  }

  if (isa<ScalableVectorType>(Ty))
    // TODO: Scalable vector types not currently modeled.
    return nullptr;

  if (auto *VecTy = dyn_cast<VectorType>(Ty)) {
    llvm::Type *ElemType = VecTy->getElementType();
    if (ElemType->isPointerTy())
      return nullptr;

    return getOrCreateVectorType(getOrCreateSimpleType(ElemType),
                                 VecTy->getElementCount().getKnownMinValue());
  }

  if (auto *StTy = dyn_cast<StructType>(Ty)) {
    if (StTy->isLiteral()) {
      SmallVector<DTransType *, 4> FieldTypes;
      for (auto *ElemTy : StTy->elements()) {
        DTransType *DTy = getOrCreateSimpleType(ElemTy);
        if (!DTy)
          return nullptr;
        FieldTypes.push_back(DTy);
      }
      return getOrCreateLiteralStructType(StTy->getContext(), FieldTypes);
    }

    return getOrCreateStructType(StTy);
  }

  if (auto *FnTy = dyn_cast<FunctionType>(Ty)) {
    DTransType *RetTy = getOrCreateSimpleType(FnTy->getReturnType());
    if (!RetTy)
      return nullptr;

    SmallVector<DTransType *, 8> ParamTypes;
    for (auto *ElemTy : FnTy->params()) {
      DTransType *DTy = getOrCreateSimpleType(ElemTy);
      if (!DTy)
        return nullptr;
      ParamTypes.push_back(DTy);
    }

    return getOrCreateFunctionType(RetTy, ParamTypes, FnTy->isVarArg());
  }

  assert((Ty->isIntegerTy() || Ty->isFloatingPointTy() || Ty->isVoidTy() ||
          Ty->isMetadataTy() || Ty->isTokenTy()) &&
         "Primitive type must be based on scalar type");
  return getOrCreateAtomicType(Ty);
}

DTransType *DTransTypeManager::findType(llvm::Type *Ty) const {
  auto Existing = TypeInfoMap.find(Ty);
  if (Existing == TypeInfoMap.end())
    return nullptr;
  return (*Existing).getSecond();
}

std::vector<DTransStructType *>
DTransTypeManager::getIdentifiedStructTypes() const {
  std::vector<DTransStructType *> TypeList;
  std::transform(StructTypeInfoMap.begin(), StructTypeInfoMap.end(),
                 std::back_inserter(TypeList),
                 [](const llvm::StringMapEntry<DTransStructType *> &KV) {
                   return KV.second;
                 });
  return TypeList;
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
  dtrans::printCollectionSorted(
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
  dtrans::printCollectionSorted(
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

DTransType *DTransTypeBuilder::getVoidTy() {
  return TM.getOrCreateAtomicType(Type::getVoidTy(TM.getContext()));
}

DTransType *DTransTypeBuilder::getIntNTy(unsigned N) {
  return TM.getOrCreateAtomicType(Type::getIntNTy(TM.getContext(), N));
}

DTransPointerType *DTransTypeBuilder::getPointerToTy(DTransType *DTy) {
  return TM.getOrCreatePointerType(DTy);
}

DTransFunctionType *DTransTypeBuilder::getFunctionType(
    DTransType *DTRetTy, ArrayRef<DTransType *> ParamTypes, bool IsVarArg) {
  return TM.getOrCreateFunctionType(DTRetTy, ParamTypes, IsVarArg);
}

DTransStructType *DTransTypeBuilder::getStructTy(llvm::StructType *Ty) {
  return TM.getOrCreateStructType(Ty);
}

void DTransTypeBuilder::populateDTransStructType(
    DTransStructType *DStructTy, ArrayRef<DTransType *> FieldTypes) {
  for (unsigned I = 0, NumFields = DStructTy->getNumFields(); I < NumFields;
       ++I) {
    dtransOP::DTransFieldMember &Field = DStructTy->getField(I);
    Field.addResolvedType(FieldTypes[I]);
  }
}


} // namespace dtransOP
} // end namespace llvm
